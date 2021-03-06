#include <map>
#include <set>
#include <vector>
#include <stdio.h>
#include <iostream>

#include "ogrsf_frmts.h" // GDAL vector drivers
#include "hfa_p.h"

using namespace std;

/*
 * Prototypes
 */

HFAEntry* find (HFAEntry* node, string name);

bool extract_proj (HFAHandle hHFA, OGRSpatialReference& srs);

string to_polyWKT (vector<pair<double, double>> pts);

string to_linestrWKT (vector<pair<double, double>> pts);

vector<pair<double, double>> rotate (vector<pair<double, double>> pts, double* center, double orientation);

/*
 * HFAGeom
 *
 * Base class for all geometry/shape objects in the HFA structure
 *
 * Caveats:
 * - HFAGeom is meant for HFA geometry/shape related nodes that shares "center"/"origin"&"orientation" data member
 *   *hence, it currently only supports "Rectangle2", "Eant_Ellipse", "Text2"
 *
 */
class HFAGeom {
	double* center;
	double orientation;

	public:
		HFAGeom (HFAEntry* node) {
			center = new double[2];
			center[0] = node->GetDoubleField("center.x");
			center[1] = node->GetDoubleField("center.y");
			orientation = node->GetDoubleField("orientation");	
		}
		
		virtual ~HFAGeom (){}; // for dynamic_cast

		double* get_center() const { return center; };

		void set_center (double* nCenter) { center = nCenter; };

		double get_orien() const { return orientation; };

		void set_orien(double nOrien) { orientation = nOrien; };

		virtual string to_wkt () const { 
			string wkt = "POINT(";
			wkt +=  to_string(center[0]) + " " + to_string(center[1]);
			wkt += ")";
			
			return wkt;
		};

		virtual ostream &write(ostream& os) const { return os;}

		virtual vector<pair<double, double>> get_pts() const {
			vector<pair<double, double>> pts;
			pts.push_back(make_pair(center[0], center[1]));

			return pts;
		};

		friend ostream& operator<<(ostream& os, const HFAGeom& hg){
			os.precision(10);
			os << "ct: " << hg.center[0] << "," << hg.center[1] << " " << endl;
			os << "or: " << hg.orientation << " " << endl;
			return hg.write(os);
		}
};

/*
 * HFAEllipse
 *
 * Sublcass of HFAGeom for "Eant_Ellipse" node types
 *
 */
class HFAEllipse: public HFAGeom {
	double semiMajorAxis;
	double semiMinorAxis;

	vector<pair<double, double>> get_unorientated_pts() const;

	public:
		HFAEllipse (HFAEntry* node):HFAGeom(node){
			semiMajorAxis = node->GetDoubleField("semiMajorAxis");
			semiMinorAxis = node->GetDoubleField("semiMinorAxis");
		};
		double get_majX () { return semiMajorAxis; };
		double get_minX () { return semiMinorAxis; };
		
		vector<pair<double, double>> get_pts() const {
			return rotate(get_unorientated_pts(), get_center(), get_orien());
		}

		string to_wkt() const { 
			return to_polyWKT(get_pts());
		};

		ostream& write (ostream &os) const override{
			os << "majx: " << semiMajorAxis << endl;
			os << "minx: " << semiMinorAxis << endl;
			os << to_wkt() << endl;

			return os;
		}
};

/*
 * HFARectangle
 * 
 * Subclass of HFAGeom for "Rectangle2" node types
 *
 */
class HFARectangle: public HFAGeom {
	double width;
	double height;
	
	vector<pair<double, double>> get_unorientated_pts() const;

	public:
		HFARectangle (HFAEntry* node):HFAGeom(node){
			width = node->GetDoubleField("width");
			height = node->GetDoubleField("height");
		};
		
		// Returns the orientated pts	
		vector<pair<double, double>> get_pts() const {
			return rotate(get_unorientated_pts(), get_center(), get_orien());
		}
		
		string to_wkt () const {
			return to_polyWKT(get_pts());
		}

		double get_width() { return width; };
		double get_height() { return height; };

		ostream& write (ostream &os) const override{
			os << "width: " << width << " " << endl;
			os << "height: " << height << " " << endl;
			vector<pair<double, double>> pts = get_pts();
			vector<pair<double, double>>::iterator it;
			for (it = pts.begin(); it != pts.end(); ++it) {
				pair<double, double> pt = *it;
				os << "(" << pt.first << ", " << pt.second << ")" << endl;		
			}

			return os;
		}
};

/* 
 * HFAPolyline
 * 
 * Subclass of HFAGeom for "Polyline2" node types
 *
 */
class HFAPolyline: public HFAGeom {
	protected:
		vector<pair<double, double>> pts;
	
	public:
		HFAPolyline(HFAEntry* node);

		string to_wkt() const { 
			return to_linestrWKT(get_pts()); 
		}

		vector<pair<double, double>> get_pts() const { return pts; }

		ostream& write (ostream &os) const override{
			vector<pair<double, double>> pts = get_pts();
			vector<pair<double, double>>::iterator it;
			for (it = pts.begin(); it != pts.end(); ++it) {
				pair<double, double> pt = *it;
				os << "(" << pt.first << ", " << pt.second << ")" << endl;		
			}

			return os;
		}
};

/*
 * HFAPolygon
 *
 * [WARNING] untested due to lack of data
 *
 * Subclass of HFAPolyline for "Polygon2" node type
 *
 */
class HFAPolygon: public HFAPolyline  {
	public:
		HFAPolygon(HFAEntry* node):HFAPolyline(node){
			// enclose line to turn it into a polygon
			pts.push_back(pts[0]);
		};

		string to_wkt() const {
			return to_polyWKT(get_pts());
		}
};


/*
 * HFAText
 *
 * Subclass of HFAGeom for "Text2"
 *
 */
class HFAText: public HFAGeom {
	const char* text;

	public:
		HFAText(HFAEntry* node):HFAGeom(node) {
			double* origin = new double[2];
			origin[0] = node->GetDoubleField("origin.x");
			origin[1] = node->GetDoubleField("origin.y");
			set_center(origin);

			text = node->GetStringField("text.string");
		}

		const char* get_text () { return text; };

		ostream& write (ostream &os) const override{
			os << "textval: " << text << endl;
			os << to_wkt() << endl;

			return os;
		}
};


/*
 * HFAAnnotation
 *
 * Base class for "Element_Eant"/"Element_2_Eant"
 * 
 * Stores "name" and "description" values of a element annotation node and its geometry/shape node
 *
 */
class HFAAnnotation {
	const char* name;
	const char* description;
	const char* elmType;
	int elmTypeId;

	HFAGeom* geom;

	public:
		HFAAnnotation (HFAEntry* node) {
			name = node->GetStringField("name");
			description = node->GetStringField("description");  
			elmType = node->GetStringField("elmType");
			elmTypeId = node->GetIntField("elmType");
		}
		const char* get_name() { return name; };

		const char* get_desc() { return description; };

		const char* get_type() { return elmType; };

		int get_typeId () { return elmTypeId; };

		HFAGeom* get_geom() { return geom; };

		void set_geom(HFAGeom* g) { geom=g; };

		friend ostream& operator<<(ostream& os, const HFAAnnotation& ha) {
			os << "type: " << ha.elmType << " [" << ha.elmTypeId << "]" << endl;
			os << "n: " << ((ha.name == NULL) ? "" : ha.name);
			os << " [" << ((ha.description == NULL) ? "" : ha.description) << "]" << endl;
			os << *ha.geom << " ";

			return os;	
		}
};

/*
 * HFAAnnotationLayer
 *
 * Extract essential information within an ERDAS Annotation Layer from HFA File Handle & HFA Root node
 * - Projection
 * - Annotations
 *
 */
class HFAAnnotationLayer {
	HFAHandle hHFA;
	HFAEntry* root;
	
	bool hasSRS = false;
	OGRSpatialReference srs;
	vector<HFAAnnotation*> annotations;

	GDALDataset *gdalDs;

	set<int> geomTypes;

	/*
	 * display [utility]
	 *
	 * Tree view of HFA structure
	 * - displays the name, dtype and size of HFA nodes
	 * - seg fault occurs when trying to recursively display StyleLibrary //TODO
	 *
	 * @param node 		HFAEntry* start node
	 * @param nIdent	int	  indentation prefix
	 * */
	void display_HFATree(HFAEntry* node, int nIdent);

	void write_to_shp (const char*, char*);

	public:
		HFAAnnotationLayer (HFAHandle);

		OGRSpatialReference get_srs () { return srs; };

		void set_srs (OGRSpatialReference new_srs) { 
			hasSRS = true;
			srs = new_srs; 
		};

		void set_srs (char* proj4srs) {
			hasSRS = true;
			srs.importFromProj4(proj4srs);	
		};
		
		vector<HFAAnnotation*> get_annos () const { return annotations; }

		void add_anno (HFAAnnotation* anno) { 
			annotations.push_back(anno); 
		};

		bool is_empty() { return annotations.empty(); }

		int get_num_annos() { return annotations.size(); }

		set<int> get_geomTypes () { return geomTypes; }

		void add_geomType (int nGeomType) { geomTypes.insert(nGeomType); }

		void to_shp (char* ofilename) {
			const char* shpDriverName = "ESRI Shapefile";
			write_to_shp(shpDriverName, ofilename);	
		};

		void to_gjson (char* ofilename) {
			//TODO GeoJSONs have a different write process compared to shapefiles
			const char* gjsonDriverName = "GeoJSON";
			cerr << "GeoJSON is not supported yet!" << endl;
		};

		void printTree () { display_HFATree(root, 0); }

		friend ostream& operator<<(ostream& os, const HFAAnnotationLayer& hal) {
			if (hal.hasSRS) {
				const char* srsDisplayOptions[] = {"FORMAT=WKT2_2018", "MULTILINE=YES", nullptr};
				char* wkt = nullptr;
				hal.srs.exportToWkt(&wkt, srsDisplayOptions);	
				os << wkt << endl;
				CPLFree(wkt);
			}

			vector<HFAAnnotation*> annos = hal.get_annos();
			vector<HFAAnnotation*>::const_iterator it;
			cout << "num_annos: " << annos.size() << endl;
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
};


/*
 * HFAGeomFactory
 *
 * Abstract factory interface for HFA geometry/shapes
 *
 */
class HFAGeomFactory {
	public:
		virtual HFAGeom* create(HFAEntry* node) { return new HFAGeom(node); }
};

/*
 * HFAEllipseFactory
 *
 * Factory for HFA Ellipse geometry nodes
 *
 */
class HFAEllipseFactory: public HFAGeomFactory {
	public:
		HFAEllipse* create(HFAEntry* node) { return new HFAEllipse(node); }
};

/*
 * HFARectangleFactory
 *
 * Factory for HFA Rectangle geometry nodes
 *
 */
class HFARectangleFactory: public HFAGeomFactory {
	public:
		HFARectangle* create(HFAEntry* node) { return new HFARectangle(node); }
};

/*
 * HFATextFactory
 *
 * Factory for HFA Text geometry(?) nodes
 *
 */
class HFATextFactory: public HFAGeomFactory {
	public:
		HFAText* create(HFAEntry* node) { return new HFAText(node); }
};

/*
 * HFAPolyline
 *
 * Factory for HFA Polyline geometry nodes
 *
 */
class HFAPolylineFactory: public HFAGeomFactory {
	public:
		HFAPolyline* create(HFAEntry* node) { return new HFAPolyline(node); }
};

/*
 * HFAPolygon
 *
 * Factory for HFA Polygon geometry nodes
 *
 */
class HFAPolygonFactory: public HFAGeomFactory {
	public:
		HFAPolygon* create(HFAEntry* node) { return new HFAPolygon(node); }
};
