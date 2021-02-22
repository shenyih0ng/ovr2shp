#include <set>
#include <math.h>

#include "ovr2shp.h"

using namespace std;

/*
 * HFA_GEOM_FACTORIES
 *
 * Supported Eants Geometries
 * {<elmType_enum>, <HFAGeomFactory>}
 */
const map<int, HFAGeomFactory*> HFA_GEOM_FACTORIES = {
	{10, new HFATextFactory()},
	{13, new HFARectangleFactory()},
	{14, new HFAEllipseFactory()},
	{16, new HFAPolylineFactory()}
};

//TEMP
const map<int, const char*> HFA_GEOM_MAPPING = {
	{10, "EANT_TEXT"},
	{13, "EANT_RECTANGLE"},
	{14, "EANT_ELLIPSE"},
	{16, "EANT_POLYLINE"}
};

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

template <typename T, typename U>
pair<T,U> operator-(const pair<T,U>& l, double* r) {
	return {l.first - r[0], l.second - r[1]};
}

template <typename T, typename U>
pair<T,U> operator+(const pair<T,U>& l, double* r) {
	return {l.first + r[0], l.second + r[1]};
}

/*
 * rotate
 *
 * @return vector<pair<double, double>> orientated points
 */
vector<pair<double, double>> rotate(vector<pair<double, double>> pts, double* center, double orientation){
	vector<pair<double, double>> orientated;

	vector<pair<double, double>>::iterator it;
	for (it = pts.begin(); it != pts.end(); it++) {
		pair<double, double> vect_coord = rotate(*it - center, orientation) + center;
		orientated.push_back(vect_coord);
	}
	return orientated;
}

/*
 * to_polyWKT [utility]
 *
 * Construct Polygon wkt (well-known text) from points
 *
 * @param  pts	 vector<pair<double, double>> 
 * @return string  polygon wkt
 */
string to_polyWKT (vector<pair<double, double>> pts) {
	string wkt = "POLYGON ((";
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
 * to_linestrWKT [utility]
 *
 * Construct LineString wkt (well-known text) from points
 *
 * @param  pts	 vector<pair<double, double>> 
 * @return string  polygon wkt
 */
string to_linestrWKT (vector<pair<double, double>> pts) {
	string wkt = "LINESTRING(";
	vector<pair<double, double>>::const_iterator it;
	for (it = pts.begin(); it != pts.end(); ++it) {
		pair<double, double> pt = *it;
		if (it == pts.end() -1) {
			wkt += to_string(pt.first) + " " + to_string(pt.second);
			wkt += ")";
		} else {
			wkt += to_string(pt.first) + " " + to_string(pt.second) + ", ";
		}
	}

	return wkt;
}

/*
 * HFAEllipse
 *
 * get_unorientated_pts
 * - discretizes ellipse to polygon without rotation
 *
 * @returns vector<pair<double, double>>
 */
vector<pair<double, double>> HFAEllipse::get_unorientated_pts () const {
	vector<pair<double, double>> pts;

	double* center = get_center();	
	int seg = max((int)floor(sqrt(((semiMajorAxis + semiMinorAxis) / 2) * 20)), 8);
	double shift = (44/7.0f)/seg; // (44/7) -> approx of 2pi
	double theta = 0.0; 
	for (int i = 0; i < seg; ++i) { 
	    theta += shift; 
	    pts.push_back(make_pair(center[0]+(semiMajorAxis*cos(theta)),
				    center[1]+(semiMinorAxis*sin(theta))));
	} 

	pts.push_back(pts[0]);
	return pts;
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

/*
 * get_field [utility]
 *
 * Traverse down HFAType structure to get requested HFAField while incrementing memory/data offset
 *
 * @param ntype		HFAType*
 * @param tFieldName	string		query field name
 * @params
 * 	data
 * 	dataPos
 * 	dataSize
 * @returns HFAField*
 */
HFAField* get_field (HFAType* ntype, string tFieldName, GByte*& data, GInt32& dataPos, GInt32& dataSize) {
	HFAField* targetField;	

	int iField = 0;
	HFAField* currField;
	while (iField < ntype->nFields) {
		currField = ntype->papoFields[iField];	
		char* fieldName = currField->pszFieldName;
		if (fieldName == tFieldName) {
			targetField = currField;
			break;
		};

		int nInstBytes = currField->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
		iField++;	
	}

	return targetField;
}

/*
 * HFAPolyline
 *
 * Constructor for HFAPolyline
 *
 */
HFAPolyline::HFAPolyline(HFAEntry* node):HFAGeom(node){
	string COORD_FIELD_NAME = "coords";

	GByte* data = node->GetData();
	GInt32 dataPos = node->GetDataPos();
	GInt32 dataSize = node->GetDataSize();

	HFAField* polyCoords = get_field(node->GetPoType(), 
			COORD_FIELD_NAME, data, dataPos, dataSize);

	// mem offset for "o" dtype
	void* pReturn;
	polyCoords->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'p', &pReturn);
	int nByteOffset = ((GByte *) pReturn) - data;
	data += nByteOffset;
	dataPos += nByteOffset;
	dataSize -= nByteOffset;

	HFAField* vectCoords = get_field(polyCoords->poItemObjectType, 
			COORD_FIELD_NAME, data, dataPos, dataSize);	
	
	// retrieve BASEDATA meta and matrix values
        GInt32 nRows, nColumns;
	GInt16 nBaseItemType;
    
        memcpy( &nRows, data+8, 4 );
        HFAStandard( 4, &nRows );
        memcpy( &nColumns, data+12, 4 );
        HFAStandard( 4, &nColumns );
        memcpy( &nBaseItemType, data+16, 2 );
        HFAStandard( 2, &nBaseItemType );
	
	for (int r=0; r < nRows; r++) {
		pair<double, double> coord;
		double x, y;
		vectCoords->ExtractInstValue(NULL, r*nRows+0, data, dataPos, dataSize, 'd', &x);
		vectCoords->ExtractInstValue(NULL, r*nRows+1, data, dataPos, dataSize, 'd', &y);
		coord = make_pair(x,y);

		pts.push_back(coord);
	}
}

/*
 * find [utility]
 *
 * Retrieve HFA node of specified name using recursive search
 *
 * @param node 	HFAEntry* start node
 * @param name  string target name
 *
 * @return HFAEntry* HFAEntry of specified name
 *
 */
HFAEntry* find (HFAEntry* node, string name) {
	if(node->GetName() == name) {
		return node;
	}
	
	HFAEntry* tgNode = NULL;

	if (node->GetChild() != NULL) {
		tgNode = find(node->GetChild(), name);
		if (tgNode != NULL) {
			return tgNode;
		}
	}

	if (node->GetNext() != NULL) {
		tgNode = find(node->GetNext(), name);
		if (tgNode != NULL) {
			return tgNode;
		}
	}

	return NULL;
}

/*
 * find_eants
 *
 * Find all Eants that currently supported
 *
 * @param eant 		HFAEntry*	 	start node (first child of ElementList)
 * @param tgEants	vector<HFAEntry*>&	collection of extracted Eants that are supported
 *
 */
void find_eants (HFAEntry* eant, vector<HFAEntry*>& tgEants) {
	eant->LoadData();
	int elmType = eant->GetIntField("elmType");

	if (elmType != 0 && HFA_GEOM_FACTORIES.find(elmType) != HFA_GEOM_FACTORIES.end()) {
		tgEants.push_back(eant);		
	}

	if (eant->GetChild() != NULL) {
		find_eants(eant->GetChild(), tgEants);
	}	

	if (eant->GetNext() != NULL) {
		find_eants(eant->GetNext(), tgEants);
	}
}

/*
 * HFAAnnotationLayer()
 *
 * Constructs an HFAAnnotationLayer from HFAHandle
 *
 * @param handle   HFAHandle HFA File Handle
 */
HFAAnnotationLayer::HFAAnnotationLayer(HFAHandle hHFA) {
	hasSRS = extract_proj(hHFA, srs);
	if (!hasSRS) {
		cerr << "[warn] no map info found/unsupported projections in file" << endl;
	}

	root = hHFA->poRoot;	
	HFAEntry* hfaElmList = find(root, "ElementList");
	
	if (hfaElmList == NULL) {
		cout << "[exit] no Eants(annotations) found!" << endl;
		exit(100);
	}

	vector<HFAEntry*> elmEants;
	find_eants(hfaElmList->GetChild(), elmEants);

	vector<HFAEntry*>::iterator it;
	for (it = elmEants.begin(); it != elmEants.end(); ++it) {
		HFAAnnotation* hfaA = new HFAAnnotation(*it);
		map<int, HFAGeomFactory*>::const_iterator gIt = HFA_GEOM_FACTORIES.find(
				(*it)->GetIntField("elmType"));
		add_geomType(gIt->first); // add geomtype as metadata of alayer
		HFAEntry* hfaAGeomChild = (*it)->GetChild();
		HFAGeomFactory* gFactory = gIt->second;
		HFAGeom* hfaAGeom = gFactory->create(hfaAGeomChild);
		hfaA->set_geom(hfaAGeom);

		add_anno(hfaA);
	}
}

/*
 * HFAAnnotationLayer
 *
 * display_HFATree
 *
 * Tree view of HFA structure
 * - displays the name, dtype and size of HFA nodes
 * - seg fault occurs when trying to recursively display StyleLibrary //TODO
 *
 * @param node 		HFAEntry* start node
 * @param nIdent	int	  indentation prefix
 * */
void HFAAnnotationLayer::display_HFATree(HFAEntry* node, int nIdent) {
	string _avoid = "StyleLibrary";
	bool avoid = (node->GetName() == _avoid);

	static char indentSpaces[128];
	for (int i = 0; i < nIdent; i++) {
		indentSpaces[i] = ' ';
	}
	indentSpaces[nIdent] = '\0';

	fprintf( stdout, "%s%s(%s) @ %d + %d @ %d\n", indentSpaces,
		     node->GetName(), node->GetType(),
		     node->GetFilePos(),
		     node->GetDataSize(), node->GetDataPos() );

	if (avoid) {
		fprintf(stdout, "%s__omitted__\n\n", indentSpaces);
	} else {
		// field values
		strcat(indentSpaces, "- ");
		node->DumpFieldValues(stdout, indentSpaces);
		fprintf(stdout, "\n");
	}

	if (node->GetChild() != NULL && (!avoid)) {
		display_HFATree(node->GetChild(), nIdent+1);
	}
	if (node->GetNext() != NULL) {display_HFATree(node->GetNext(), nIdent);}
}

/*
 * HFAAnnotationLayer
 *
 * write_to_shp
 * - write/export HFAAnnotationLayer to ShapeFile (.shp)
 *
 * Caveats
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

	set<int> gTypes = get_geomTypes();
	GDALDataset* gdalDatasets[gTypes.size()];
	map<int, OGRLayer*> layers;

	int c = 0;
	for (set<int>::iterator gtIt = gTypes.begin(); gtIt != gTypes.end(); ++gtIt) {
		map<int, const char*>::const_iterator gmapIt = HFA_GEOM_MAPPING.find(*gtIt);
		const char* geomName = gmapIt->second;

		string _fileName = ofilename;
		size_t ldirSep = _fileName.find_last_of('/');
		string _fileBaseName = _fileName.substr(ldirSep);
		size_t fextSep = _fileBaseName.find_first_of('.');
		_fileName.insert(fextSep + ldirSep, geomName);

		GDALDataset* ds = driver->Create(_fileName.c_str(), 0, 0, 0, GDT_Unknown,NULL);
		OGRLayer* l;		
		OGRwkbGeometryType lgeomType;

		if ((*gtIt) == 10) {
			lgeomType = wkbPoint;
		} else if ((*gtIt) == 16){
			lgeomType = wkbLineString;
		}else {
			lgeomType = wkbPolygon;
		}

		l = ds->CreateLayer(NULL,((hasSRS == false) ? NULL: new OGRSpatialReference(get_srs())), lgeomType, NULL);
		
		// Layer Fields
		
		OGRFieldDefn nameField("name", OFTString);
		nameField.SetWidth(254);
		if (l->CreateField(&nameField) != OGRERR_NONE) {
			cout << "failed creating name field" << endl;
			exit(1);
		}

		OGRFieldDefn descField("desc", OFTString);
		descField.SetWidth(254);
		if (l->CreateField(&descField) != OGRERR_NONE) {
			cout << "failed creating description field" << endl;
			exit(1);
		}

		if ((*gtIt) == 10) {
			OGRFieldDefn textField("text", OFTString);
			textField.SetWidth(254);
			if (l->CreateField(&textField) != OGRERR_NONE) {
				cout << "failed creating text field" << endl;
				exit(1);
			}
		}

		layers.insert(make_pair((*gtIt), l));
		gdalDatasets[c] = ds;
		c++;
	}
	

	vector<HFAAnnotation*> annotations = get_annos();	
	vector<HFAAnnotation*>::const_iterator it;
	for (it = annotations.begin(); it != annotations.end(); ++it) {
		OGRFeature* feat;
		OGRGeometry* geom;

		feat = OGRFeature::CreateFeature(layers[(*it)->get_typeId()]->GetLayerDefn());
		feat->SetField("name", (*it)->get_name());
		feat->SetField("desc", (*it)->get_desc());

		HFAGeom* hfaGeom = (*it)->get_geom();	

		if ((*it)->get_typeId() == 10) {
			HFAText* hfaText = dynamic_cast<HFAText*>(hfaGeom);
			feat->SetField("text", hfaText->get_text());
		}
		
		string wktStr = hfaGeom->to_wkt();
		const char* wkt = wktStr.c_str();

		OGRGeometryFactory::createFromWkt(wkt, NULL, &geom);
		feat->SetGeometry(geom);

		if (layers[(*it)->get_typeId()]->CreateFeature(feat) != OGRERR_NONE) {
			cout << "failed to create feature" << endl;
		}

		OGRFeature::DestroyFeature(feat);
	}
	
	for (int dsIdx = 0; dsIdx < gTypes.size(); dsIdx++) {	
		GDALClose(gdalDatasets[dsIdx]);
	}
}
