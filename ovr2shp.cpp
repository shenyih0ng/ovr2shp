#include <cmath>
#include <vector>

#include "ogrsf_frmts.h"
#include "hfa_p_wo_port.h"
#include "hfaclasses.h"

#include "gnuplot-iostream.h"

using namespace std;

/*
 * Goal: Convert .ovr to .shp/.geojson
 * 2. Extract coordinate system of annotation (if there is one)
 * 3. Parse it into a .shp/.geojson format
 */

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
			cmd += g.file1d(pts) + "with circles title '" + (*it)->get_name() + "', \\\n";
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
	g << cmd << endl;	
}


int main (int argc, char* argv[]) {
	bool displayTree = false;
	bool plotAnno = false;
	const string displayTreeFlag = "-d"; //temp
	const string plotFlag = "-p"; //temp

	const char *file_name = NULL;
	HFAHandle hHFA;
	
	for (int i = 1; i < argc; i++){
		if (argv[i] == displayTreeFlag) {
			displayTree = true;
		} else if (argv[i] == plotFlag) {
			plotAnno = true;
		} else if (file_name == NULL) {
			file_name = argv[i];
		}
	}

	if (file_name != NULL) {
		printf("f: %s\n", file_name);
		hHFA = HFAOpen(file_name, "r");
	} else {
		exit(1);
	}

	if (hHFA == NULL) {
		printf("[x] HFAOpen() failed.\n");
		exit(100);	
	}

	HFAAnnotationLayer* hfaal = new HFAAnnotationLayer(hHFA);
	cout << *hfaal << endl;
	
	if (displayTree) {
		hfaal->printTree();
	}

	if (plotAnno) {
		vector<HFAAnnotation*> annotations = hfaal->get_annos();
		plot(annotations);
	}	
	
	return 1;
}
