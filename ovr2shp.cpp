#include <set>
#include <vector>
#include <stdio.h>
#include "hfa_p.h"
#include "hfaclasses.h"

using namespace std;

/*
 * Goal: Convert .ovr to .shp/.geojson
 * 2. Extract coordinate system of annotation (if there is one)
 * 3. Parse it into a .shp/.geojson format
 */

const string EANT_DTYPE_NAME = "Element_Eant";
const string EANT_GROUP_DTYPE_NAME = "Element_2_Eant";

const string ELLI_DTYPE_NAME = "Eant_Ellipse";
const string RECT_DTYPE_NAME = "Rectangle2";

const unordered_map<string, HFAGeomFactory*> hfaGeomFactories = {
	{ELLI_DTYPE_NAME, new HFAEllipseFactory()},
	{RECT_DTYPE_NAME, new HFARectangleFactory()}
};

/*
 * display [utility]
 *
 * Tree view of HFA structure
 * - displays the name, dtype and size of HFA nodes
 *
 * @para node 	HFAEntry* start node
 * @para nIdent	int	  indentation prefix
 * */
void display (HFAEntry* node, int nIdent=0) {
	for (int i=0; i < nIdent; i++) {cout << "\t";}
	printf("%s<%s> %d\n", node->GetName(), node->GetType(), node->GetDataSize());
	if (node->GetChild() != NULL) {display(node->GetChild(), nIdent+1);}
	if (node->GetNext() != NULL) {display(node->GetNext(), nIdent);}
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
 * processEant
 *
 * Convert HFA nodes of type "Element_Eant"/"Element_2_Eant" to HFAAnnotation
 *
 * @param eantElements HFAEntry* HFA nodes of "Element_Eant"/"Element_2_Eant" types
 *
 * @return vector<HFAAnnotation*>
 *
 */
vector<HFAAnnotation*> processEant (vector<HFAEntry*> eantElements) {
	vector<HFAAnnotation*> annotations;
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
				annotations.push_back(hfaA);
			}
		}
	}
	return annotations;
}

int main (int argc, char* argv[]) {
	bool displayTree = false;
	const string displayFlag = "-d"; //temp

	const char *file_name = NULL;
	HFAHandle hHFA;
	HFAEntry* root;
	
	for (int i = 1; i < argc; i++){
		if (argv[i] == displayFlag) {
			displayTree = true;
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

	root = hHFA-> poRoot;
	if (displayTree) {
		cout << "---tree display---" << endl;
		display(root);
	}
	
	set<string> dtypes = {EANT_DTYPE_NAME, EANT_GROUP_DTYPE_NAME};
	vector<HFAEntry*> eantElements = find(root, dtypes);

	vector<HFAAnnotation*> annotations = processEant(eantElements);
	vector<HFAAnnotation*>::iterator it;
	for (it=annotations.begin(); it != annotations.end(); ++it) {
		HFAAnnotation* annotation = *it;
		cout << *annotation << endl;
	}
	
	return 1;
}
