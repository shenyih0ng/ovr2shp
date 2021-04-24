#include <cmath>
#include <vector>

#include "ovr2shp.h"

#ifdef GPLOT
#include "gnuplot-iostream.h" // gnuplot
#endif

using namespace std;

#ifdef GPLOT
/*
 * Helper sort functions for plotting
 *
 */
bool sortx(pair<double, double> i, pair<double, double> j) {
	return i.first < j.first;
}

bool sorty(pair<double, double> i, pair<double, double> j) {
	return i.second < j.second;
}

/*
 * plot[utility]
 *
 * Plot HFAAAnnotations geometry/shapes using Gnuplot
 * 
 * @param annotations 	vector<HFAAnnotation*> vector of HFAAnnotation objects
 * @param buffer	"zoom"
 */
void plot (vector<HFAAnnotation*> annotations, int buffer=1) {
	Gnuplot g;
	string cmd = "plot";

	vector<pair<double, double>> combined;
	vector<HFAAnnotation*>::iterator it;
	for (it = annotations.begin(); it != annotations.end(); ++it) {
		vector<pair<double, double>> pts = (*it)->get_geom()->get_pts();
		combined.insert(combined.end(), pts.begin(), pts.end());
		if (pts.size() == 1) {
			cmd += g.file1d(pts) + "title '" + (*it)->get_name() + "', \\\n";
		} else {
			cmd += g.file1d(pts) + "with lines title '" + (*it)->get_name() + "', \\\n";
		}
	}

	double xmin, xmax, ymin, ymax;
	sort(combined.begin(), combined.end(), sortx);
	xmin = combined.begin()->first;
	xmax = (combined.end()-1)->first;
	sort(combined.begin(), combined.end(), sorty);
	ymin = combined.begin()->second;
	ymax = (combined.end()-1)->second;

	g << "set xrange[" << xmin - (xmax-xmin)*buffer << ":" << xmax + (xmax-xmin)*buffer << "]\n";
	g << "set yrange[" << ymin - (ymax-ymin)*buffer << ":" << ymax + (ymax-ymin)*buffer << "]\n";
	g << "set key font \",7\"\n";
	g << "set key outside\n";
	g << cmd << endl;	
}
#endif

bool validate_ovr (fs::path file_path) {
	bool valid = file_path.extension() == ".ovr";
	if (!valid) {
		cout << "[err] " << file_path << " is not a .ovr file" << endl;
	}

	return valid;
}

bool validate_rMode (fs::path file_path) {
	bool valid = !fs::is_directory(file_path) && validate_ovr(file_path);
	if (!valid) {
		cout << "[err] invalid source input for READ mode" << endl;
		cout << "\t- READ mode only support single .ovr file" << endl;
	}

	return valid;
}

bool is_file_valid (fs::path file_path, bool (*validate)(fs::path)) {
	if (!fs::exists(file_path)) {
		cout << "[x] " << file_path << " does not exists" << endl;
		return false;
	}

	return (*validate)(file_path);
}

void display (HFAAnnotationLayer* hfaal, bool displayAnno, bool displayTree, bool plotAnno) {
	// Display layer
	if (displayAnno) {
		cout << *hfaal << endl;
	}
		
	// Display HFA Tree Structure	
	if (displayTree) { 
		hfaal->printTree(); 
	}
	
	// Plot annotation geometries/shapes
	if (plotAnno) {
		vector<HFAAnnotation*> annotations = hfaal->get_annos();
		#ifdef GPLOT
		plot(annotations);
		#else
		cout << "[err] gnuplot is not included in the build" << endl;
		#endif
	}	
}

HFAAnnotationLayer* open (fs::path file_path) {
	cout << "[info] opening " << file_path << endl;

	HFAHandle hHFA = HFAOpen(file_path.c_str(), "r");
	if (hHFA == NULL) {
		cout << "[error] cannot open " << file_path << endl;	
		return nullptr;
	}

	HFAAnnotationLayer* hfaal = new HFAAnnotationLayer(hHFA);

	return hfaal;
}

void ovr2shp (fs::path file_path, fs::path output_dir, char* user_srs) {
	cout << "[info] converting " << file_path << " ..." << endl;

	HFAAnnotationLayer* hfaal = open(file_path);

	if (user_srs != NULL) {
		hfaal->set_srs(user_srs);
		cout << "[info] user defined srs: " << user_srs << endl;
	}

	fs::path shp_path = output_dir / 
		file_path.stem() / 
		file_path.stem();

	fs::create_directories(shp_path.parent_path());

	bool converted = hfaal->to_shp(shp_path);
	if (converted) {
		cout << "[info] ✓ success: " << file_path;
		cout << " -> " << shp_path.parent_path();
		cout << endl;
	} else {
		cout << "[err] failed to convert ";
		cout << file_path << endl;
	}
	cout << endl;
}

int main (int argc, char* argv[]) {
	bool displayAnno = false, 
	     displayTree = false, 
	     plotAnno = false, 
	     userDefinedSRS = false, 
	     convertSrc = false;

	const string displayAnnoFlag = "-d",
	       displayTreeFlag = "-t", 
	       plotFlag = "-p", 
	       srsFlag = "-srs", 
	       outputDirFlag = "-o";

	char* user_srs = NULL; //proj4
	fs::path output_dir;
	fs::path src_path;

	for (int i = 1; i < argc; i++){
		if (argv[i] == displayTreeFlag) {
			displayTree = true;
		} else if (argv[i] == displayAnnoFlag) {
			displayAnno = true;
		} else if (argv[i] == plotFlag) {
			plotAnno = true;
		} else if (argv[i] == srsFlag) {
			userDefinedSRS = true;
			i++;
			user_srs = argv[i];
		} else if (argv[i] == outputDirFlag) {
			i++;
			output_dir = argv[i];
			convertSrc = true;
		} else if (src_path.empty()) {
			src_path = argv[i];
		}
	}

	GDALAllRegister();
	
	if (convertSrc) {
		cout << "[info] mode: CONVERT" << endl;
		cout << "[warn] display flags are ignored!" << endl;
		cout << "output directory: " << output_dir << endl;

		if (fs::is_directory(src_path)) {
			cout << "source directory: " << src_path << endl;
			cout << endl;

			fs::recursive_directory_iterator rDirIt(src_path);
			for (auto& p : rDirIt) {
				if (p.path().extension() == ".ovr") {
					ovr2shp(p.path(), output_dir, user_srs);
				}
			}
		} else if (is_file_valid(src_path, validate_ovr)) {
			cout << "source file: " << src_path << endl;
			cout << endl;

			ovr2shp(src_path, output_dir, user_srs);
		}
	} else if (is_file_valid(src_path, validate_rMode)) {
		cout << "[info] mode: READ" << endl;
		cout << "source file: " << src_path << endl;
		cout << endl;

		HFAAnnotationLayer* hfaal = open(src_path);
		
		display(hfaal, displayAnno, displayTree, plotAnno);
	}
	
	return 0;
}
