#include "ovr2shp.h"

#ifdef GPLOT
#include "gnuplot-iostream.h" // gnuplot
#endif

using namespace std;

string CURRSRC = "";

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
void plot(vector<HFAAnnotation *> annotations, int buffer = 1) {
    Gnuplot g;
    string cmd = "plot";

    vector<pair<double, double>> combined;
    vector<HFAAnnotation *>::iterator it;
    for (it = annotations.begin(); it != annotations.end(); ++it) {
        vector<pair<double, double>> pts = (*it)->get_geom()->get_pts();
        combined.insert(combined.end(), pts.begin(), pts.end());
        if (pts.size() == 1) {
            cmd += g.file1d(pts) + "title '" + (*it)->get_name() + "', \\\n";
        } else {
            cmd += g.file1d(pts) + "with lines title '" + (*it)->get_name() +
                   "', \\\n";
        }
    }

    double xmin, xmax, ymin, ymax;
    sort(combined.begin(), combined.end(), sortx);
    xmin = combined.begin()->first;
    xmax = (combined.end() - 1)->first;
    sort(combined.begin(), combined.end(), sorty);
    ymin = combined.begin()->second;
    ymax = (combined.end() - 1)->second;

    g << "set xrange[" << xmin - (xmax - xmin) * buffer << ":"
      << xmax + (xmax - xmin) * buffer << "]\n";
    g << "set yrange[" << ymin - (ymax - ymin) * buffer << ":"
      << ymax + (ymax - ymin) * buffer << "]\n";
    g << "set key font \",7\"\n";
    g << "set key outside\n";
    g << cmd << endl;
}
#endif

bool validate_ovr(fs::path file_path) {
    bool valid = file_path.extension() == ".ovr";
    if (!valid) {
        Log(ERROR) << file_path << " is not a .ovr file";
    }

    return valid;
}

bool validate_rMode(fs::path file_path) {
    if (fs::is_directory(file_path)) {
        Log(ERROR) << "READ mode only supports single .ovr files";
    }

    return validate_ovr(file_path);
}

bool is_file_valid(fs::path file_path, bool (*validate)(fs::path)) {
    if (!fs::exists(file_path)) {
        Log(ERROR) << file_path << " does not exists";
        return false;
    }

    return (*validate)(file_path);
}

void display(HFAHandle hHFA, HFAAnnotationLayer *hfaal, bool displayAnno,
             bool displayTree, bool displayDict, bool plotAnno) {
    // Display layer
    if (displayAnno) {
        cout << *hfaal << endl;
    }

    // Display HFA Tree Structure
    if (displayTree) {
        hfaal->printTree();
    }

    // Display HFA Data Dictionary
    if (displayDict) {
        HFADumpDictionary(hHFA, stdout);
    }

    // Plot annotation geometries/shapes
    if (plotAnno) {
        vector<HFAAnnotation *> annotations = hfaal->get_annos();
#ifdef GPLOT
        plot(annotations);
#else
        Log(ERROR) << "GNUPLOT is not included in this build";
#endif
    }
}

bool ovr2shp(fs::path file_path, fs::path output_dir, char *user_srs) {
    HFAHandle hHFA = HFAOpen(file_path.string().c_str(), "r");
    if (hHFA == NULL) {
        Log(ERROR) << "HFA driver failed to open " << file_path;
    }

    HFAAnnotationLayer *hfaal = new HFAAnnotationLayer(hHFA);
    if (hfaal->is_empty()) {
        return false;
    }

    if (user_srs != NULL) {
        hfaal->set_srs(user_srs);
        Log(INFO) << "user defined srs: " << user_srs;
    }

    fs::path shp_path = output_dir / file_path.stem() / file_path.stem();

    fs::create_directories(shp_path.parent_path());

    bool converted = hfaal->to_shp(shp_path);
    if (converted) {
        Log(INFO) << "Successfully converted ✓"
                  << "\n";
    } else {
        Log(WARN) << "Failed to convert ✗"
                  << "\n";
    }

    return converted;
}

int main(int argc, char *argv[]) {
    bool displayAnno = false, displayTree = false, displayDict = false,
         plotAnno = false, userDefinedSRS = false, convertSrc = false;

    const string displayAnnoFlag = "-d", displayTreeFlag = "-dt",
                 displayDictFlag = "-dd", plotFlag = "-p", srsFlag = "-srs",
                 outputDirFlag = "-o";

    char *user_srs = NULL; // proj4
    fs::path output_dir;
    fs::path src_path;

    for (int i = 1; i < argc; i++) {
        if (argv[i] == displayTreeFlag) {
            displayTree = true;
        } else if (argv[i] == displayAnnoFlag) {
            displayAnno = true;
        } else if (argv[i] == displayDictFlag) {
            displayDict = true;
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

    if (src_path.empty()) {
        Log(ERROR) << "No source input specified";
        exit(100);
    }

    if (convertSrc) {
        Log(INFO) << "mode: CONVERT";
        Log(WARN) << "display flags are ignored";

        if (fs::is_directory(src_path)) {
            Log(INFO) << "src: " << src_path << " "
                      << "out: " << output_dir;

            vector<fs::path> failed;
            fs::recursive_directory_iterator rDirIt(src_path);
            for (auto &p : rDirIt) {
                if (p.path().extension() == ".ovr") {
                    CURRSRC = p.path().string();
                    if (!ovr2shp(p.path(), output_dir, user_srs)) {
                        failed.push_back(p.path());
                    }
                }
            }

            if (!failed.empty()) {
                cout << "Failed to convert: " << endl;
                vector<fs::path>::const_iterator it = failed.begin();
                for (; it != failed.end(); it++) {
                    cout << "✗ " << (*it) << endl;
                }
            }
        } else if (is_file_valid(src_path, validate_ovr)) {
            Log(INFO) << "src: " << src_path << " "
                      << "out: " << output_dir;
            CURRSRC = src_path.string();
            ovr2shp(src_path, output_dir, user_srs);
        }
    } else if (is_file_valid(src_path, validate_rMode)) {
        Log(INFO) << "mode: READ";
        Log(INFO) << "src: " << src_path;

        CURRSRC = src_path.string();
        HFAHandle hHFA = HFAOpen(src_path.string().c_str(), "r");
        if (hHFA == NULL) {
            Log(ERROR) << "HFA driver failed to open " << src_path;
        }

        HFAAnnotationLayer *hfaal = new HFAAnnotationLayer(hHFA);

        display(hHFA, hfaal, displayAnno, displayTree, displayDict, plotAnno);
    }

    return 0;
}
