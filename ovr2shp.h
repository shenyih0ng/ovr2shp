#include <map>
#include <set>
#include <vector>
#include <limits>
#include <stdio.h>
#include <iostream>
#include <filesystem>

#include "ogrsf_frmts.h" // GDAL vector drivers
#include "hfa_p.h"

using namespace std;
namespace fs = std::filesystem;

extern const string HFA_POLYLINE_COORDS_ATTR_NAME;
extern const string HFA_ANNOTATION_XFORM_ATTR_NAME;
extern const string HFA_XFORM_COEF_ATTR_NAME;
extern const string HFA_XFORM_VECT_ATTR_NAME;

/*
 * Prototypes
 *
 */

HFAEntry* find (HFAEntry* node, string name);

vector<pair<double, double>> rotate (vector<pair<double, double>> pts, double* center, double orientation);

bool extract_proj (HFAHandle hHFA, OGRSpatialReference& srs);

string to_polyWKT (vector<pair<double, double>> pts);

string to_linestrWKT (vector<pair<double, double>> pts);

/************************************************************************/
/*                                                                      */
/*                               HFAGeom                                */
/*                                                                      */
/*      Interface for all geometry/shape objects in the HFA structure   */
/*                                                                      */
/************************************************************************/

class HFAGeom {
	public:
		virtual ~HFAGeom (){}; // for dynamic_cast
	
		virtual vector<pair<double, double>> get_pts() const = 0;

		virtual void write(ostream&) const = 0;

		friend ostream& operator<<(ostream& os, const HFAGeom& hg){
			hg.write(os);

			return os;
		}
};

/************************************************************************/
/*                                                                      */
/*                              HFAEllipse                              */
/*                                                                      */
/*      	       Base class for Eant_Ellipse                      */
/*                                                                      */
/************************************************************************/

class HFAEllipse: public HFAGeom {
	double* center;
	double rotation;

	double semiMajorAxis;
	double semiMinorAxis;

	vector<pair<double, double>> get_unorientated_pts() const;

	public:
		HFAEllipse (HFAEntry* node) {
			center = new double[2];
			center[0] = node -> GetDoubleField("center.x");
			center[1] = node -> GetDoubleField("center.y");

			rotation = node->GetDoubleField("orientation");		

			semiMajorAxis = node->GetDoubleField("semiMajorAxis");
			semiMinorAxis = node->GetDoubleField("semiMinorAxis");
		};

		double* get_center() { return center; };

		double get_rotation() { return rotation; };

		double get_majX () { return semiMajorAxis; };

		double get_minX () { return semiMinorAxis; };
		
		vector<pair<double, double>> get_pts() const {
			return rotate(get_unorientated_pts(), center, rotation);
		}

		void write (ostream &os) const {
			os << "center: " << center[0] << ", " << center[1] << endl;
			os << "rotation: " << rotation << endl;
			os << "majx: " << semiMajorAxis << endl;
			os << "minx: " << semiMinorAxis << endl;
		}
};

/************************************************************************/
/*                                                                      */
/*                              HFARectangle                            */
/*                                                                      */
/*      	       Base class for Eant_Rectangle                    */
/*                                                                      */
/************************************************************************/

class HFARectangle: public HFAGeom {
	double* center;
	double rotation;

	double width;
	double height;
	
	vector<pair<double, double>> get_unorientated_pts() const;

	public:
		HFARectangle (HFAEntry*);

		double* get_center() { return center; };

		double get_rotation() { return rotation; };

		double get_width() { return width; };

		double get_height() { return height; };
		
		vector<pair<double, double>> get_pts() const {
			return rotate(get_unorientated_pts(), center, rotation);
		}
		
		void write (ostream &os) const {
			os << "center: " << center[0] << ", " << center[1] << endl;
			os << "rotation: " << rotation << endl;
			os << "width: " << width << " " << endl;
			os << "height: " << height << " " << endl;
		}
};

/************************************************************************/
/*                                                                      */
/*                              HFAPolyline                             */
/*                                                                      */
/*            	       Base class for Eant_Polyline                     */
/*                                                                      */
/************************************************************************/

class HFAPolyline: public HFAGeom {
	protected:
		vector<pair<double, double>> pts;
	
	public:
		HFAPolyline(HFAEntry* node);

		void write (ostream &os) const { return; }

		vector<pair<double, double>> get_pts() const { return pts; }
};

/************************************************************************/
/*                                                                      */
/*                         HFAPolygon (untested)                        */
/*                                                                      */
/*            	        Base class for Eant_Polygon                     */
/*                                                                      */
/************************************************************************/

class HFAPolygon: public HFAPolyline  {
	public:
		HFAPolygon(HFAEntry* node):HFAPolyline(node){
			// enclose line to turn it into a polygon
			pts.push_back(pts[0]);
		};
};

/************************************************************************/
/*                                                                      */
/*                                HFAText                               */
/*                                                                      */
/*            	         Base class for Eant_Text                       */
/*                                                                      */
/************************************************************************/

class HFAText: public HFAGeom {
	double* origin;
	const char* text;

	public:
		HFAText(HFAEntry* node) {
			origin = new double[2];
			origin[0] = node->GetDoubleField("origin.x");
			origin[1] = node->GetDoubleField("origin.y");

			text = node->GetStringField("text.string");
		}

		double* get_origin() { return origin; };

		const char* get_text () { return text; };

		vector<pair<double, double>> get_pts() const {
			vector<pair<double, double>> pts;
			pts.push_back(make_pair(origin[0], origin[1]));

			return pts;
		}

		void write (ostream &os) const {
			os << "origin: " << origin[0] << ", " << origin[1] << endl;
			os << "textval: " << text << endl;
		}
};


/************************************************************************/
/*                                                                      */
/*                          HFAAnnotation                               */
/*                                                                      */
/*            Base class for Element_Eant/Element_2_Eant                */
/*           Stores annotation (not shape) level metadata.              */
/*                                                                      */
/************************************************************************/

class HFAAnnotation {
	int id;
	const char* name;
	const char* description;
	const char* elmType;
	int elmTypeId;

	HFAGeom* geom;
		
	/*
	 * coord_vect (1x3)
	 * {x, y, z}
	 *
	 * xform (3x3)
	 * { xform[0] xform[1], -- coef
	 *   xform[2] xform[3], -- coef
	 *   xform[4] xform[5]  -- vect }
	 *
	 */
	double xform[6];

	public:
		HFAAnnotation (HFAEntry*);
 
		int get_id() { return id; };

		const char* get_name() { return name; };

		const char* get_desc() { return description; };

		const char* get_type() { return elmType; };

		double* get_xform() { return xform; };

		int get_typeId () { return elmTypeId; };

		HFAGeom* get_geom() { return geom; };

		void set_geom(HFAGeom* g) { geom=g; };
		
		vector<pair<double, double>> get_pts() const;

		string get_wkt() const;

		friend ostream& operator<<(ostream& os, const HFAAnnotation& ha) {
			os << "id: " << ha.id << endl;
			os << "type: " << ha.elmType << " [" << ha.elmTypeId << "]" << endl;
			os << "name: " << ((ha.name == NULL) ? "" : ha.name) << endl;
			os << "description: " << ((ha.description == NULL) ? "" : ha.description) << endl;

			os.precision(numeric_limits<long double>::digits10 + 1);
			os << "xform: " << endl;
			for (int i = 0; i < 6; i+=2) {
				os << "\t" << ha.xform[i];
				os << " " << ha.xform[i+1];
				os << endl;
			}
			os << endl;

			os << *ha.geom;
			os << ha.get_wkt();
			os << endl;

			return os;	
		}
};

/************************************************************************/
/*                                                                      */
/*                       HFAAnnotationLayer                             */
/*                                                                      */
/*            Base class to store all annotations in a .ovr             */
/*           Stores file level metadata (projection/map info)           */
/*                                                                      */
/************************************************************************/

class HFAAnnotationLayer {
	HFAHandle hHFA;
	HFAEntry* root;
	
	bool hasSRS = false;
	OGRSpatialReference srs;
	vector<HFAAnnotation*> annotations;

	GDALDataset *gdalDs;

	set<int> geomTypes;

	void display_HFATree(HFAEntry* node, int nIdent);

	bool write_to_shp (const char*, fs::path);

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

		bool to_shp (fs::path dst) {
			const char* shpDriverName = "ESRI Shapefile";
			return write_to_shp(shpDriverName, dst);	
		};

		bool to_gjson (char* ofilename) {
			//TODO  @NotImplemented
			const char* gjsonDriverName = "GeoJSON";
			cerr << "GeoJSON is not supported yet!" << endl;
			return false;
		};

		void printTree () { display_HFATree(root, 0); }

		friend ostream& operator<<(ostream&, const HFAAnnotationLayer&);
};

/************************************************************************/
/*                                                                      */
/*                       HFA Geometry Factories                         */
/*                                                                      */
/************************************************************************/

/*
 * HFAGeomFactory
 *
 * Factory Interface
 *
 */
class HFAGeomFactory {
	public:
		virtual HFAGeom* create(HFAEntry*) = 0;
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
 * Factory for HFA Text geometry nodes
 *
 */
class HFATextFactory: public HFAGeomFactory {
	public:
		HFAText* create(HFAEntry* node) { return new HFAText(node); }
};

/*
 * HFAPolylineFactory
 *
 * Factory for HFA Polyline geometry nodes
 *
 */
class HFAPolylineFactory: public HFAGeomFactory {
	public:
		HFAPolyline* create(HFAEntry* node) { return new HFAPolyline(node); }
};

/*
 * HFAPolygonFactory
 *
 * Factory for HFA Polygon geometry nodes
 *
 */
class HFAPolygonFactory: public HFAGeomFactory {
	public:
		HFAPolygon* create(HFAEntry* node) { return new HFAPolygon(node); }
};
