#include <vector>
#include <stdio.h>
#include <iostream>
#include <unordered_map>

#define GDAL_INCLUDED
#include "ogrsf_frmts.h" // GDAL vector drivers
#include "hfa_p.h"

using namespace std;

/*
 * Prototypes
 *
 */

bool extract_proj (HFAHandle hHFA, OGRSpatialReference& srs);

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
		double* get_center() const { return center; };
		double get_orien() const { return orientation; };
		unordered_map<string, double> get_fieldValues () { return fieldValues; };

		virtual string get_type () { return "bGeom"; };

		virtual string to_wkt () { 
			// TODO
			return ""; 
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
		
		string to_wkt() { 
			//TODO
			return ""; 
		};

		string get_type () { return "ellipse"; };
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
	
	vector<pair<double, double>> get_unorientated_pts() const;

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
		
		// Returns the orientated pts	
		vector<pair<double, double>> get_pts() const;
		
		string to_wkt ();

		double get_width() { return width; };
		double get_height() { return height; };
		string get_type () { return "rectangle"; };
		string * get_fieldNames()
		{
			static string fieldNames[] = {"width", "height"};
			return fieldNames;
		}

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

		friend ostream& operator<<(ostream& os, const HFAAnnotation& ha) {
			os << "gtype: " << ha.geom->get_type() << endl;
			os << "n: " << ha.name << " [" << ha.description << "]" << endl;
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

	/*
	 * display [utility]
	 *
	 * Tree view of HFA structure
	 * - displays the name, dtype and size of HFA nodes
	 *
	 * @param node 		HFAEntry* start node
	 * @param nIdent	int	  indentation prefix
	 * */
	void display_HFATree(HFAEntry* node, int nIdent=0) {
		//TODO i have no idea why there is a segmentation fault for LineStyle_0 (hypothesis: something to with the number of fields + when offsetting memory space it leads to a seg fault) the error differs from file to file (its probably time to look into some error handling)
		string _avoid = "LineStyle_0";

		for (int i=0; i < nIdent; i++) {cout << "\t";}
		printf("%s<%s> %d \n", node->GetName(), node->GetType(), node->GetDataSize());

		if (node->GetName() != _avoid) {
			node->LoadData();	
			HFAType* ntype = node->GetPoType();	
			HFAField* poField;
			GByte* data = node->GetData();
			GInt32 dataPos = node->GetDataPos();
			GInt32 dataSize = node->GetDataSize();

			if (ntype != NULL) {	
				int iField=0;
				while (iField < ntype->nFields) {
					poField = ntype->papoFields[iField];
				
					for (int i=0; i < nIdent + 1; i++) {cout << "\t";}
					printf("+ %s<%c>\n", poField->pszFieldName , poField->chItemType); // display field names;
					
					int nInstBytes = poField->GetInstBytes(data);
					data += nInstBytes;
					dataPos += nInstBytes;
					dataSize -= nInstBytes;
					iField++;
				}
			}
		}
		
		//temp
		string mapinfo = "Map_Info";
		if (node->GetName() == mapinfo) {
			for (int i=0; i < nIdent + 1; i++) {cout << "\t";}
			cout << "projName: " << node->GetStringField("proName") << " ";

			cout << "ulx: " << node->GetDoubleField("upperLeftCenter.x") << " ";
			cout << "uly: " << node->GetDoubleField("upperLeftCenter.y") << " ";

			cout << "lrx: " << node->GetDoubleField("lowerRightCenter.x") << " ";
			cout << "lry: " << node->GetDoubleField("lowerRightCenter.y") << " ";

			cout << "psizeW: " << node->GetDoubleField("pixelSize.width") << " ";
			cout << "psizeH: " << node->GetDoubleField("pixelSize.height") << endl;
		}

		if (node->GetChild() != NULL) {display_HFATree(node->GetChild(), nIdent+1);}
		if (node->GetNext() != NULL) {display_HFATree(node->GetNext(), nIdent);}
	}

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

		void to_shp (char* ofilename) {
			const char* shpDriverName = "ESRI Shapefile";
			write_to_shp(shpDriverName, ofilename);	
		};

		void to_gjson (char* ofilename) {
			//TODO GeoJSONs have a different write process compared to shapefiles
			const char* gjsonDriverName = "GeoJSON";
			cerr << "GeoJSON is not supported yet!" << endl;
		};

		void printTree () { display_HFATree(root); }

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
