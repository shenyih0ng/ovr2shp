#include <set>
#include <math.h>

#include "ovr2shp.h"

using namespace std;

const string EANT_DTYPE_NAME = "Element_Eant";
const string EANT_GROUP_DTYPE_NAME = "Element_2_Eant";

const string ELLI_DTYPE_NAME = "Eant_Ellipse";
const string RECT_DTYPE_NAME = "Rectangle2";

const unordered_map<string, HFAGeomFactory*> hfaGeomFactories = {
        {ELLI_DTYPE_NAME, new HFAEllipseFactory()},
        {RECT_DTYPE_NAME, new HFARectangleFactory()}
};

/* 
 * get_xyCoords [utility]
 *
 * Extract xy coordinates from HFAField that has x&y attributes
 * Examples of such fields includes "center", "origin"
 * 
 * @param xy	HFAField
 * @param data	GByte*
 * @param dataPos	GInt32
 * @param dataSize	GInt32
 *
 * @return double[2]{x,y}
 */
double* get_xyCoords (HFAField* xy, GByte* data, GInt32 dataPos, GInt32 dataSize) {
	double* xyCoords = new double[2];

	void* pReturn;
	xy->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'p', &pReturn);
	int nByteOffset = ((GByte *) pReturn) - data;
	data += nByteOffset;
	dataPos += nByteOffset;
	dataSize -= nByteOffset;

	HFAType* xyType = xy->poItemObjectType;
	HFAField *coordX = xyType->papoFields[0];
	coordX->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &xyCoords[0]);

	int nInstBytes = coordX->GetInstBytes(data);
	data += nInstBytes;
	dataPos += nInstBytes;
	dataSize -= nInstBytes;

	HFAField *coordY = xyType->papoFields[1];
	coordY->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &xyCoords[1]);

	return xyCoords;
}

/*
 * rotate [utility]
 *
 * rotate clockwise a (x, y) vector by a specified rad angle
 *
 * @param coord	pair<double, double> vector coordinates
 * @param rad   double  rotation angle
 *
 * @return pair<double, double> rotated vector coordinates
 */
pair<double, double> rotate (pair<double, double> coord,  double rad) {
	double costheta = cos(rad);
	double sintheta = sin(rad);

	return make_pair(costheta*coord.first + sintheta*coord.second, 
			costheta*coord.second - sintheta*coord.first);
}

/*
 * find [utility]
 *
 * Retrieve HFA nodes of specified type using recursive search
 *
 * @param node 	HFAEntry* start node
 * @param dtype_names set specified types
 *
 * @return vector<HFAEntry*> collection of HFAEntry of specified type
 *
 */
vector<HFAEntry*> find (HFAEntry* node, set<string> dtype_names) {
	vector<HFAEntry*> nodes_found;
	vector<HFAEntry*> to_search;
	to_search.push_back(node);
	while(!to_search.empty()) {
		HFAEntry* curr_node = to_search.back();
		to_search.pop_back();
		if(dtype_names.find(curr_node->GetType()) != dtype_names.end()) {
			nodes_found.push_back(curr_node);
		}
		if (curr_node->GetNext() != NULL) {
			to_search.push_back(curr_node->GetNext());
		}
		if (curr_node->GetChild() != NULL) {
			to_search.push_back(curr_node->GetChild());
		}
	}

	return nodes_found;
}

/*
 * find[utility]
 *
 * Overloaded
 *
 * @param node HFAEntry* start node
 * @param dtype_name string specified type
 *
 */
vector<HFAEntry*> find (HFAEntry* node, string dtype_name) {
	set<string> dtype = {dtype_name};
	return find(node, dtype);
}

/*
 * HFAGeom()
 *
 * Construct an HFAGeom from HFAEntry.
 *
 * Caveats:
 * - Only extracts fields/nested fields that have a 'd' dtype (double)
 * 
 * @param node	HFAEntry representing a HFA "geometry" object e.g Rectangle2, Eant_Ellipse
 */
HFAGeom::HFAGeom (HFAEntry* node) {
	string CENTER_FIELD_NAME = "center";
	string ORIGIN_FIELD_NAME = "origin"; // temp (for Text2 compatbility)
	string ORIEN_FIELD_NAME = "orientation";

	int iField = 0;
	double fval;
	HFAField *poField;

	HFAType* ntype = node->GetPoType();
	GByte* data = node->GetData();
	GInt32 dataPos = node->GetDataPos();
	GInt32 dataSize = node->GetDataSize();
	
	while (iField < ntype->nFields) {
		poField = ntype->papoFields[iField];	
		char* fieldName = poField->pszFieldName;
		if (fieldName == CENTER_FIELD_NAME || fieldName == ORIGIN_FIELD_NAME) {
			center = get_xyCoords(poField, data, dataPos, dataSize);	
		} else if (poField->chItemType == 'd') {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			if (fieldName == ORIEN_FIELD_NAME) {
				orientation = (double)fval;
			} else {
				fieldValues.insert(make_pair<string, double>(fieldName, (double)fval));	
			}
		}
		int nInstBytes = poField->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
		iField++;	
	}
}

/*
 * HFARectangle
 *
 * get_unorientated_points
 * - get the 4 unorientated corners of the rectangle
 * - follows LinearRing coordinate standard (first and last coordinates are the same)
 *
 * @return vector<pair<double, double>> four unorientated corners
 */
vector<pair<double, double>> HFARectangle::get_unorientated_pts() const {
	vector<pair<double, double>> pts;
	double* center = get_center();
	int ydir[2] = {1, -1};
	int xdir[2] = {-1, 1};
	for (int i = 0; i <= 1; i++) {
		for (int j = 0; j <= 1; j++) {
		 	pts.push_back(make_pair(
				center[0] + (xdir[j]*(width/2)), 
				center[1] + (ydir[i]*(height/2))));
		}
		int temp = xdir[1];
		xdir[1] = xdir[0];
		xdir[0] = temp;
	}
	pts.push_back(pts[0]);
	return pts;	
}

template <typename T, typename U>
pair<T,U> operator-(const pair<T,U>& l, double* r) {
	return {l.first - r[0], l.second - r[1]};
}

template <typename T, typename U>
pair<T,U> operator+(const pair<T,U>& l, double* r) {
	return {l.first + r[0], l.second + r[1]};
}

/*
 * HFARectangle
 *
 * get_pts
 * - get the 4 orientated corners of the rectangle
 *
 * @return vector<pair<double, double>> four orientated corners
 */
vector<pair<double, double>> HFARectangle::get_pts() const {
	double* center = get_center();
	double orientation = get_orien();
	vector<pair<double, double>> orientated;
	vector<pair<double, double>> unorientated = get_unorientated_pts();
	vector<pair<double, double>>::iterator it;
	for (it = unorientated.begin(); it != unorientated.end(); it++) {
		pair<double, double> vect_coord = rotate(*it - center, orientation) + center;
		orientated.push_back(vect_coord);
	}
	return orientated;
}

/*
 * HFARectangle
 *
 * to_wkt
 * - Construct wkt (well-known text) from orientated points of HFARectangle
 *
 * @return string  polygon wkt of HFARectangle
 */
string HFARectangle::to_wkt() {
	string wkt = "POLYGON ((";
	vector<pair<double, double>> pts = get_pts();
	vector<pair<double, double>>::const_iterator it;
	for (it = pts.begin(); it != pts.end(); ++it) {
		pair<double, double> pt = *it;
		if (it == pts.end() -1) {
			wkt += to_string(pt.first) + " " + to_string(pt.second);
			wkt += "))";
		} else {
			wkt += to_string(pt.first) + " " + to_string(pt.second) + ", ";
		}
	}

	return wkt;
}

/*
 * HFAAnnotation()
 *
 * Constructs an HFAAnnotation from HFAEntry
 *
 * @param node	HFAEntry representing a HFA "annotation" object e.g. Element_Eant, Element_2_Eant
 */
HFAAnnotation::HFAAnnotation(HFAEntry* node) {
	string NAME_FIELD = "name";
	string DESC_FIELD = "description";

	int iField = 0;
	char* fval;
	HFAField *poField;

	HFAType* ntype = node->GetPoType();
	GByte* data = node->GetData();
	GInt32 dataPos = node->GetDataPos();
	GInt32 dataSize = node->GetDataSize();

	while (iField < ntype->nFields) {
		poField = ntype->papoFields[iField];	
		char* fieldName = poField->pszFieldName;
		if (fieldName == NAME_FIELD || fieldName == DESC_FIELD) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 's', &fval);
			if (fieldName == NAME_FIELD) { name = fval; }
			else { description = fval; }
		}
		int nInstBytes = poField->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
		iField++;	
	}
}

/*
 * HFAAnnotationLayer()
 *
 * Constructs an HFAAnnotationLayer from HFAHandle
 *
 * @param handle   HFAHandle HFA File Handle
 */
HFAAnnotationLayer::HFAAnnotationLayer(HFAHandle handle) {
	hHFA = handle;

	// extract projection/crs
	hasSRS = extract_proj(hHFA, srs);
	if (!hasSRS) {
		cerr << "[warn] no map info found/unsupported projections in file" << endl;
	}

	root = hHFA->poRoot;	
	set<string> dtypes = {EANT_DTYPE_NAME, EANT_GROUP_DTYPE_NAME};
	
	// add annotations
	vector<HFAEntry*> eantElements = find(root, dtypes);
	vector<HFAEntry*>::iterator it;

	for (it=eantElements.begin(); it != eantElements.end(); ++it) {
		HFAEntry* eantElement = *it;
		if (eantElement->GetChild() != NULL) {
			const char* cType = eantElement->GetChild()->GetType();
			unordered_map<string, HFAGeomFactory*>::const_iterator gIt = hfaGeomFactories.find(cType);
			if (gIt != hfaGeomFactories.end()) {
				eantElement->LoadData();
				HFAAnnotation* hfaA = new HFAAnnotation(eantElement);
				HFAEntry* child = eantElement->GetChild();
				child->LoadData();

				HFAGeom* annoGeom;
				HFAGeomFactory* factory = gIt->second;
				annoGeom = factory->create(child);
				hfaA->set_geom(annoGeom);

				add_anno(hfaA); // add to layer
			}
		}
	}		
}


/*
 * HFAAnnotationLayer
 *
 * write_to_shp
 * - write/export HFAAnnotationLayer to ShapeFile (.shp)
 *
 * Caveats
 * - Currently only supports OGRPolygon & Polygon wkts
 * - Setting the layer name does not work expected (layer name turns out to be the base name of the output file name/path)
 * - Field width will be truncated to 254 when set to 256 (i guess the max field width is 254)  
 *
 * @param driverName	const char* 	GDAL vector driver name
 * @param ofilename	string		Output file name/path
 */
void HFAAnnotationLayer::write_to_shp (const char* driverName, char* ofilename) {
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driverName);
	if (driver == NULL) {
		cout << driverName << " driver not found" << endl;
		exit(1);
	}

	OGRLayer* layer;
	gdalDs = driver->Create(ofilename, 0, 0, 0, GDT_Unknown, NULL);
	
	if (hasSRS) {
		layer = gdalDs->CreateLayer(NULL, new OGRSpatialReference(get_srs()), wkbPolygon, NULL);
	} else {
		layer = gdalDs->CreateLayer(NULL, NULL, wkbPolygon, NULL);
	}
	
	OGRFieldDefn nameField("name", OFTString);
	nameField.SetWidth(254);
	if (layer->CreateField(&nameField) != OGRERR_NONE) {
		cout << "failed creating name field" << endl;
		exit(1);
	}

	OGRFieldDefn descField("desc", OFTString);
	descField.SetWidth(254);
	if (layer->CreateField(&descField) != OGRERR_NONE) {
		cout << "failed creating description field" << endl;
		exit(1);
	}

	vector<HFAAnnotation*> annotations = get_annos();	
	vector<HFAAnnotation*>::const_iterator it;
	for (it = annotations.begin(); it != annotations.end(); ++it) {
		OGRFeature* feat;
		OGRPolygon polygon;

		feat = OGRFeature::CreateFeature(layer->GetLayerDefn());
		feat->SetField("name", (*it)->get_name());
		feat->SetField("desc", (*it)->get_desc());

		HFAGeom* hfaGeom = (*it)->get_geom();
		string wktStr = hfaGeom->to_wkt();
		const char* wkt = wktStr.c_str();
		polygon.importFromWkt(&wkt);
		feat->SetGeometry(&polygon);

		if (layer->CreateFeature(feat) != OGRERR_NONE) {
			cout << "failed to create feature" << endl;
		}

		OGRFeature::DestroyFeature(feat);
	}

	GDALClose(gdalDs);	
}
