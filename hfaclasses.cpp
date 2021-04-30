#include <set>
#include <math.h>
#include <iomanip>

#include "ovr2shp.h"

using namespace std;

extern const string HFA_POLYLINE_COORDS_ATTR_NAME = "coords";
extern const string HFA_ANNOTATION_XFORM_ATTR_NAME = "xformMatrix";
extern const string HFA_XFORM_COEF_ATTR_NAME = "polycoefmtx";
extern const string HFA_XFORM_VECT_ATTR_NAME = "polycoefvector";

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

template <typename T, typename U>
pair<T,U> operator-(const pair<T,U>& l, double* r) {
	return {l.first - r[0], l.second - r[1]};
}

template <typename T, typename U>
pair<T,U> operator+(const pair<T,U>& l, double* r) {
	return {l.first + r[0], l.second + r[1]};
}

/************************************************************************/
/*                                                                      */
/*                           Utility Functions                          */
/*                                                                      */
/************************************************************************/

/*
 * to_str
 *
 * convert double to string with floating-point precision of 20
 *
 * @param double val
 * @return string
 */
string to_str (double val) {
	ostringstream ss;
	ss << setprecision(20);
	ss << val;

	return ss.str();
}

/*
 * to_ptWKT [utility]
 *
 * Construct Point wkt (well-known text) from x,y
 *
 * @param  x	 double
 * @param  y	 double
 * @return string  point wkt
 */
string to_ptWKT (double x, double y) {
	string wkt = "POINT(";
	wkt += to_str(x) + " " + to_str(y) + ")";

	return wkt;
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
			wkt += to_str(pt.first) + " " + to_str(pt.second);
			wkt += "))";
		} else {
			wkt += to_str(pt.first) + " " + to_str(pt.second) + ", ";
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
			wkt += to_str(pt.first) + " " + to_str(pt.second);
			wkt += ")";
		} else {
			wkt += to_str(pt.first) + " " + to_str(pt.second) + ", ";
		}
	}

	return wkt;
}

/*
 * rotate [utility]
 *
 * rotate anti-clockwise a (x, y) vector by a specified rad angle
 *
 * @param coord	pair<double, double> vector coordinates
 * @param rad   double  rotation angle
 *
 * @return pair<double, double> rotated vector coordinates
 */
pair<double, double> rotate (pair<double, double> coord,  double rad) {
	double costheta = cos(rad);
	double sintheta = sin(rad);

	return make_pair(costheta*coord.first - sintheta*coord.second, 
			costheta*coord.second + sintheta*coord.first);
}

/*
 * rotate [utility]
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
	
	if (targetField-> chItemType == 'o') {
		// offset for 'o' item type
		void* pReturn;
		targetField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'p', &pReturn);

		int nByteOffset = ((GByte *) pReturn) - data;
		data += nByteOffset;
		dataPos += nByteOffset;
		dataSize -= nByteOffset;
	}

	return targetField;
}

/*
 * get_matrix [utility]
 *
 * Extract BASEDATA matrix from HFAField 
 *
 * @param hf		HFAField* 
 * @param data		GByte*
 * @param dataPos 	GInt32
 * @param dataSize	GInt32
 *
 * @return vector<double>  1d representation of BASEDATA matrix
 */
vector<double> get_matrix (HFAField* hf, GByte* data, GInt32 dataPos, GInt32 dataSize) {
        GInt32 nRows, nColumns;
	GInt16 nBaseItemType;
   	
       	// extract BASEDATA meta	
        memcpy( &nRows, data+8, 4 );
        HFAStandard( 4, &nRows );
        memcpy( &nColumns, data+12, 4 );
        HFAStandard( 4, &nColumns );
        memcpy( &nBaseItemType, data+16, 2 );
        HFAStandard( 2, &nBaseItemType );
	
	vector<double> pts;	
	for (int idx=0; idx < nColumns*nRows; idx++) {
		double _val;
		hf->ExtractInstValue(NULL, idx, data, dataPos, dataSize, 'd', &_val);
		pts.push_back(_val);
	}

	return pts;
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

/************************************************************************/
/*                                                                      */
/*                              HFAEllipse                              */
/*                                                                      */
/************************************************************************/

/*
 * Constructor for HFAEllipse
 *
 */
HFAEllipse::HFAEllipse(HFAEntry* node) {
	center = new double[2];
	center[0] = node -> GetDoubleField("center.x");
	center[1] = node -> GetDoubleField("center.y");

	rotation = node->GetDoubleField("orientation");

	semiMajorAxis = node->GetDoubleField("semiMajorAxis");
	semiMinorAxis = node->GetDoubleField("semiMinorAxis");
}

/*
 * get_unorientated_pts
 *
 * - discretizes ellipse to polygon without rotation
 *
 * @returns vector<pair<double, double>>
 */
vector<pair<double, double>> HFAEllipse::get_unorientated_pts () const {
	vector<pair<double, double>> pts;

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

/************************************************************************/
/*                                                                      */
/*                             HFARectangle                             */
/*                                                                      */
/************************************************************************/

/*
 * Constructor for HFARectangle
 *
 */
HFARectangle::HFARectangle(HFAEntry* node) {
	center = new double[2];
	center[0] = node -> GetDoubleField("center.x");
	center[1] = node -> GetDoubleField("center.y");

	rotation = node->GetDoubleField("orientation");

	width = node->GetDoubleField("width");
	height = node->GetDoubleField("height");
}

/* 
 * get_unorientated_points
 *
 * - get the 4 unorientated corners of the rectangle
 * - follows LinearRing coordinate standard (first and last coordinates are the same)
 *
 * @return vector<pair<double, double>> four unorientated corners
 */
vector<pair<double, double>> HFARectangle::get_unorientated_pts() const {
	vector<pair<double, double>> pts;

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

/************************************************************************/
/*                                                                      */
/*                              HFAPolyline                             */
/*                                                                      */
/************************************************************************/

/*
 * Constructor for HFAPolyline
 *
 * BASEDATA Matrix
 * [
 * 	[x1,y1],
 * 	[x2,y2],
 * 	[x3,y3]
 * ]
 * shape: 3x2 (<BASEDATA nColumns> x <BASEDATA nRows>)
 *
 */
HFAPolyline::HFAPolyline(HFAEntry* node){
	GByte* data = node->GetData();
	GInt32 dataPos = node->GetDataPos();
	GInt32 dataSize = node->GetDataSize();

	HFAField* polyCoords = get_field(node->GetPoType(), 
			HFA_POLYLINE_COORDS_ATTR_NAME, data, dataPos, dataSize);

	HFAField* vectCoords = get_field(polyCoords->poItemObjectType, 
			HFA_POLYLINE_COORDS_ATTR_NAME, data, dataPos, dataSize);	
	
	vector<double> coordsMtx = get_matrix(vectCoords, data, dataPos, dataSize);
	for (int i = 0; i < coordsMtx.size(); i+=2) {
		pts.push_back(make_pair(coordsMtx[i], coordsMtx[i+1]));
	}	
}

/************************************************************************/
/*                                                                      */
/*                             HFAAnnotation                            */
/*                                                                      */
/************************************************************************/

/*
 * Constructor for HFAAnnotation
 *
 */
HFAAnnotation::HFAAnnotation (HFAEntry* node) {
	id = node->GetIntField("id");
	name = node->GetStringField("name");
	description = node->GetStringField("description");  
	elmType = node->GetStringField("elmType");
	elmTypeId = node->GetIntField("elmType");

	// get xform matrix
	GByte* data = node->GetData();
	GInt32 dataPos = node->GetDataPos();
	GInt32 dataSize = node->GetDataSize();

	HFAField* xformMatrix = get_field(node->GetPoType(),
			HFA_ANNOTATION_XFORM_ATTR_NAME, data, dataPos, dataSize);
	
	// copy
	GByte* _data = data;
	GInt32 _dataPos = dataPos;
	GInt32 _dataSize = dataSize;

	HFAField* polyVect = get_field(xformMatrix->poItemObjectType,
			HFA_XFORM_VECT_ATTR_NAME, data, dataPos, dataSize);
	vector<double> vects = get_matrix(polyVect, data, dataPos, dataSize);

	if (vects.size() == 2) {
		xform[4] = vects[0];
		xform[5] = vects[1];
	} else {
		Log(ERROR) << "unexpected xform.polycoefvect size of "
			   << vects.size()
			   << " , expected 2";
	}

	HFAField* polyCoef = get_field(xformMatrix->poItemObjectType,
			HFA_XFORM_COEF_ATTR_NAME, _data, _dataPos, _dataSize);
	vector<double> coefs = get_matrix(polyCoef, _data, _dataPos, _dataSize);

	if (coefs.size() == 4) {
		for (int i = 0; i < 4; i++) {
			xform[i] = coefs[i];
		}
	} else {
		Log(ERROR) << "unexpected xform.polycoefmtx size of "
			   << coefs.size()
			   << " , expected 4";
	}
}

/*
 * get_pts
 *
 * apply transformation matrix on shape coordinates
 * 
 * [warning] only tested on Eant_Rectangle
 *
 * @return vector<pair<double, double>> transformed shape coordinates
 *
 */
vector<pair<double, double>> HFAAnnotation::get_pts() const {
	vector<pair<double, double>> tCoords;

	vector<pair<double, double>> geom_coords = geom->get_pts();
	vector<pair<double, double>>::const_iterator it = geom_coords.begin();
	for (;it != geom_coords.end(); it++) {
		double tX = ((*it).first * xform[0]) +
			((*it).second * xform[2]) +
			xform[4];
		double tY = ((*it).first * xform[1]) +
			((*it).second * xform[3]) +
			xform[5];

		tCoords.push_back(make_pair(tX, tY));
	}

	return tCoords;
}

/*
 * get_wkt
 *
 * @return string WKT of annotation shape
 *
 */
string HFAAnnotation::get_wkt() const {
	string wkt;
	vector<pair<double, double>> geom_pts = get_pts();
	switch(elmTypeId) {
		case 10:
			wkt = to_ptWKT(geom_pts[0].first, geom_pts[0].second);
			break;
		case 16:
			wkt = to_linestrWKT(geom_pts);
			break;
		default:
			wkt = to_polyWKT(geom_pts);
			break;
	}

	return wkt;
}

/************************************************************************/
/*                                                                      */
/*                           HFAAnnotationLayer                         */
/*                                                                      */
/************************************************************************/

/*
 * _loadData [utility]
 *
 * wrapper for HFAEntry->LoadData()
 *
 * @param hfaEntry  HFAEntry*
 */
bool _loadData (HFAEntry* hfaEntry) {
	hfaEntry->LoadData();
	bool loaded = hfaEntry->GetData() != NULL;
	if (!loaded) {
		Log(ERROR) << "Corrupted HFAEntry node found";
	}

	return loaded;
}

/*
 * extract_annotations [utility]
 *
 * finds annotation nodes (Element_X_Eant) and insert it into annotation layer
 *
 * @param eant 	  HFAEntry*
 * @param annos   vector<HFAAnnotation*>&	ref to annotation layer annotations
 * @param hfaal   HFAAnnotationLayer*		pointer to annotation layer instance
 */
void extract_annotations (HFAEntry* eant, vector<HFAAnnotation*>& annos, HFAAnnotationLayer* hfaal) {
	if (_loadData(eant)) {
		int elmType = eant->GetIntField("elmType");
		map<int, HFAGeomFactory*>::const_iterator factoryEntry = HFA_GEOM_FACTORIES.find(elmType);

		if (elmType != 0 && factoryEntry != HFA_GEOM_FACTORIES.end()) {
			HFAEntry* hfaAGeomChild = eant->GetChild();
			if (_loadData(hfaAGeomChild)) {
				HFAAnnotation* hfaA = new HFAAnnotation(eant);

				HFAGeom* hfaAGeom = factoryEntry->second->create(hfaAGeomChild);
				hfaA->set_geom(hfaAGeom);

				annos.push_back(hfaA);
				hfaal->add_geomType(elmType); // add geomtype as metadata of a layer
			}
		}
	}

	if (eant->GetChild() != NULL) {
		extract_annotations(eant->GetChild(), annos, hfaal);
	}	

	if (eant->GetNext() != NULL) {
		extract_annotations(eant->GetNext(), annos, hfaal);
	}
}

/*
 * Constructor for HFAAnnotationLayer
 *
 */
HFAAnnotationLayer::HFAAnnotationLayer(HFAHandle hHFA) {
	hasSRS = extract_proj(hHFA, srs);
	root = hHFA->poRoot;

	HFAEntry* hfaElmList = find(root, "ElementList");
	if (hfaElmList != NULL) {
		extract_annotations(hfaElmList->GetChild(), annotations, this);
	}

	if (annotations.empty()) {
		Log(WARN) << "No annotation elements found";
	}
}

/*
 * Insertion operator overload
 *
 */
ostream& operator<<(ostream& os, const HFAAnnotationLayer& hal) {
	if (hal.hasSRS) {
		const char* srsDisplayOptions[] = {"FORMAT=WKT2_2018", "MULTILINE=YES", nullptr};
		char* wkt = nullptr;
		hal.srs.exportToWkt(&wkt, srsDisplayOptions);	
		os << wkt << endl;
		CPLFree(wkt);
	}

	vector<HFAAnnotation*> annos = hal.get_annos();
	vector<HFAAnnotation*>::const_iterator it;
	os << "num_annos: " << annos.size() << endl;
	for (it = annos.begin(); it != annos.end(); ++it) {
		HFAAnnotation* anno = *it;
		os << *anno << endl;
	}
	
	set<int>::const_iterator sIt;
	os << "geomTypes: ";
	for (sIt = hal.geomTypes.begin(); sIt != hal.geomTypes.end(); ++sIt) {
		os << (*sIt) << " ";
	}
	os << endl;	

	return os;	
}

/*
 * display_HFATree
 *
 * Tree view of HFA structure
 * - displays the name, dtype and size of HFA nodes
 * - TODO seg fault occurs when trying to recursively display StyleLibrary
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
 * createOGRField [utility]
 *
 * create field in OGRLayer
 *
 * @param layer       OGRLayer*
 * @param fieldName   const char*
 * @param dtype       OGRFieldType 	data type of field
 */
void createOGRField (OGRLayer* layer, const char* fieldName, OGRFieldType dtype) {
	OGRFieldDefn field(fieldName, dtype);
	if (dtype == OFTString) {
		field.SetWidth(254);
	}

	if (layer->CreateField(&field) != OGRERR_NONE) {
		Log(ERROR) << "Failed to create "
			   << fieldName << " field "
			   << "in .shp";
	}
}

/*
 * write_to_shp
 *
 * - write/export HFAAnnotationLayer to ShapeFile (.shp)
 *
 * Caveats
 * - Setting the layer name does not work expected (layer name turns out to be the base name of the output file name/path)
 * - Field width will be truncated to 254 when set to 256 (i guess the max field width is 254)  
 *
 * @param driverName	const char* 	GDAL vector driver name
 * @param ofilename	string		Output file name/path
 *
 */
bool HFAAnnotationLayer::write_to_shp (const char* driverName, fs::path dst) {
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driverName);
	if (driver == NULL) {
		Log(ERROR) << "Cannot find " << driverName << "driver";
		return false;
	}

	vector<GDALDataset*> gdalDatasets;
	map<int, OGRLayer*> layers;

	for (set<int>::iterator gtIt = geomTypes.begin(); gtIt != geomTypes.end(); ++gtIt) {
		map<int, const char*>::const_iterator gmapIt = HFA_GEOM_MAPPING.find(*gtIt);
		const char* geomName = gmapIt->second;
		
		fs::path geom_dst = dst;
		geom_dst += geomName;
		geom_dst += ".shp";

		GDALDataset* ds = driver->Create(geom_dst.c_str(), 0, 0, 0, GDT_Unknown,NULL);
		if (ds == NULL) {
			Log(ERROR) << "Unable to create file "
				   << geom_dst;
			continue;
		}

		OGRLayer* l;		
		OGRwkbGeometryType lgeomType;

		if ((*gtIt) == 10) {
			lgeomType = wkbPoint;
		} else if ((*gtIt) == 16){
			lgeomType = wkbLineString;
		} else {
			lgeomType = wkbPolygon;
		}

		l = ds->CreateLayer(NULL,((hasSRS == false) ? NULL: new OGRSpatialReference(get_srs())), lgeomType, NULL);
		
		createOGRField(l, "eleId", OFTInteger64);
		createOGRField(l, "name", OFTString);
		createOGRField(l, "desc", OFTString);
	
		if ((*gtIt) == 10) {
			createOGRField(l, "text", OFTString);
		}

		layers.insert(make_pair((*gtIt), l));
		gdalDatasets.push_back(ds);
	}
	
	vector<HFAAnnotation*>::const_iterator it;
	for (it = annotations.begin(); it != annotations.end(); ++it) {
		OGRFeature* feat;
		OGRGeometry* geom;

		feat = OGRFeature::CreateFeature(layers[(*it)->get_typeId()]->GetLayerDefn());
		feat->SetField("eleId", (*it)->get_id());
		feat->SetField("name", (*it)->get_name());
		feat->SetField("desc", (*it)->get_desc());

		HFAGeom* hfaGeom = (*it)->get_geom();	

		if ((*it)->get_typeId() == 10) {
			HFAText* hfaText = dynamic_cast<HFAText*>(hfaGeom);
			feat->SetField("text", hfaText->get_text());
		}
		
		string wktStr = (*it)->get_wkt();
		OGRGeometryFactory::createFromWkt(wktStr.c_str(), NULL, &geom);
		feat->SetGeometry(geom);

		if (layers[(*it)->get_typeId()]->CreateFeature(feat) != OGRERR_NONE) {
			Log(ERROR) << "Failed to create feature in Shapefile";
			return false;
		}

		OGRFeature::DestroyFeature(feat);
	}
	
	for (int dsIdx = 0; dsIdx < gdalDatasets.size(); dsIdx++) {	
		GDALClose(gdalDatasets[dsIdx]);
	}

	return true;
}
