#include <iostream>
#include <unordered_map>

using namespace std;

/*
 * HFAGeom
 *
 * Base class for all geometry/shape objects in the HFA structure
 *
 * Caveats:
 * - HFAGeom is meant for HFA geometry/shape related nodes that shares "center"/"origin"&"orientation" data member
 *   *hence, it currently only supports "Rectangle2", "Eant_Ellipse", "Text2"
 * - it only keeps track of data fields of type "d"(double)
 *
 */
class HFAGeom {
	double* center;
	double orientation;
	unordered_map<string, double> fieldValues;
	public:
		HFAGeom (HFAEntry*);
		double* get_center() { return center; };
		double get_orien() { return orientation; };
		unordered_map<string, double> get_fieldValues () { return fieldValues; };
		string* get_fieldNames()
		{
			static string fieldNames[] = {"center", "orientation"};
			return fieldNames;
		};

		virtual ostream &write(ostream& os) const { return os;}

		friend ostream& operator<<(ostream& os, const HFAGeom& hg){
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
	public:
		HFAEllipse (HFAEntry* node):HFAGeom(node){
			unordered_map<string, double> fvalues = HFAGeom::get_fieldValues();
			string* fieldNames = get_fieldNames();
			for (int idx=0; idx < fieldNames->size(); idx++) {
				string fname = fieldNames[idx];
				if (fname == "semiMajorAxis") { semiMajorAxis=fvalues.at(fname); }
				else if (fname == "semiMinorAxis") { semiMinorAxis=fvalues.at(fname); }
			}
		};
		double get_majX () { return semiMajorAxis; };
		double get_minX () { return semiMinorAxis; };
		string* get_fieldNames()
		{
			static string fieldNames[] = {"semiMajorAxis", "semiMinorAxis"};
			return fieldNames;
		}
		ostream& write (ostream &os) const override{
			os << "majx: " << semiMajorAxis << endl;
			os << "minx: " << semiMinorAxis << endl;

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
	public:
		HFARectangle (HFAEntry* node):HFAGeom(node){
			unordered_map<string, double> fvalues = HFAGeom::get_fieldValues();
			string* fieldNames = get_fieldNames();
			for (int idx=0; idx < fieldNames->size(); idx++) {
				string fname = fieldNames[idx];
				if (fname == "width") { width=fvalues.at(fname); }
				else if (fname == "height") { height=fvalues.at(fname); }
			}
		};
		double get_width() { return width; };
		double get_height() { return height; };
		string * get_fieldNames()
		{
			static string fieldNames[] = {"width", "height"};
			return fieldNames;
		}
		ostream& write (ostream &os) const override{
			os << "width: " << width << " " << endl;
			os << "height: " << height << " " << endl;

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
	char* name;
	char* description;
	HFAGeom* geom;
	public:
		HFAAnnotation (HFAEntry*);
		char* get_name() { return name; };
		char* get_desc() { return description; };
		HFAGeom* get_geom() { return geom; };
		void set_geom(HFAGeom* g) { geom=g; };
		friend ostream& operator<<(ostream&, const HFAAnnotation&);
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
