#include <iostream>
#include <stdio.h>
#include <vector>
#include "hfa_p.h"

using namespace std;

/*
 * Goal: Convert .ovr to .shp/.geojson
 * 1. Extract all AntElement with its meta + geom data
 * 2. Extract coordinate system of annotation (if there is one)
 * 3. Parse it into a .shp/.geojson format
 */

string GEOM_TYPE[] = {"Rectangle2", "Eant_Ellipse"};

struct ellipse
{
	double* center;
	double majorAxis;
	double minorAxis;
	double orientation;
};

struct rectangle 
{
	double* center;
	double width;
	double height;
	double orientation;
};

// UTILITIES
void display (HFAEntry* node, int nIdent=0) {
	for (int i=0; i < nIdent; i++) {cout << "\t";}
	printf("%s<%s> %d\n", node->GetName(), node->GetType(), node->GetDataSize());
	if (node->GetChild() != NULL) {display(node->GetChild(), nIdent+1);}
	if (node->GetNext() != NULL) {display(node->GetNext(), nIdent);}
}

void display(ellipse e) {
	printf("center: (%f, %f) ", e.center[0], e.center[1]);
	printf("majX: %f minX: %f ori: %f\n", e.majorAxis, e.minorAxis, e.orientation);
}

void display(rectangle r) {
	printf("center: (%f, %f) ", r.center[0], r.center[1]);
	printf("w: %f h: %f ori: %f\n", r.width, r.height, r.orientation);
}

vector<HFAEntry*> find (HFAEntry* node, string dtype_name) {
	vector<HFAEntry*> nodes_found;
	vector<HFAEntry*> to_search;
	to_search.push_back(node);
	while(!to_search.empty()) {
		HFAEntry* curr_node = to_search.back();
		to_search.pop_back();
		if(curr_node->GetType() == dtype_name) {
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

// CORE

double* get_center (HFAField* HFACenter, GByte* data, GInt32 dataPos, GInt32 dataSize) {
	static double coords[2];

	void* pReturn;
	HFACenter->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'p', &pReturn);
	int nByteOffset = ((GByte *) pReturn) - data;
	data += nByteOffset;
	dataPos += nByteOffset;
	dataSize -= nByteOffset;

	HFAType* centerType = HFACenter->poItemObjectType;
	for (int jField = 0; jField < centerType->nFields; jField++) {
		double coordVal;
		HFAField *coord = centerType->papoFields[jField];
		coord->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &coordVal);
		coords[jField] = coordVal;

		int nInstBytes = coord->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
	}	

	return coords;
}

ellipse get_ellipse (HFAEntry* geom_node) {
	const string CENTER_FIELD_NAME = "center";
	const string MAJOR_AXIS_FIELD_NAME = "semiMajorAxis";
	const string MINOR_AXIS_FIELD_NAME = "semiMinorAxis";
	const string ORIENTATION_FIELD_NAME = "orientation";

	int iField = 0;
	double fval;
	HFAField *poField;
	ellipse eObj;

	HFAType* ntype = geom_node->GetPoType();
	GByte* data = geom_node->GetData();
	GInt32 dataPos = geom_node->GetDataPos();
	GInt32 dataSize = geom_node->GetDataSize();
	while (iField < ntype->nFields) {
		poField = ntype->papoFields[iField];	
		char* fieldName = poField->pszFieldName;
		if (fieldName == CENTER_FIELD_NAME){
			eObj.center = get_center(poField, data, dataPos, dataSize);
		} else if (fieldName == MAJOR_AXIS_FIELD_NAME) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			eObj.majorAxis = fval;
		} else if (fieldName == MINOR_AXIS_FIELD_NAME) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			eObj.minorAxis = fval;
		} else if (fieldName == ORIENTATION_FIELD_NAME) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			eObj.orientation = fval;
		}

		int nInstBytes = poField->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
		iField++;	
	}
	
	return eObj;
}

rectangle get_rectangle (HFAEntry* geom_node) {
	const string CENTER_FIELD_NAME = "center";
	const string WIDTH_FIELD_NAME = "width";
	const string HEIGHT_FIELD_NAME = "height";
	const string ORIENTATION_FIELD_NAME = "orientation";

	int iField = 0;
	double fval;
	HFAField *poField;
	rectangle rObj;

	HFAType* ntype = geom_node->GetPoType();
	GByte* data = geom_node->GetData();
	GInt32 dataPos = geom_node->GetDataPos();
	GInt32 dataSize = geom_node->GetDataSize();
	while (iField < ntype->nFields) {
		poField = ntype->papoFields[iField];	
		char* fieldName = poField->pszFieldName;
		if (fieldName == CENTER_FIELD_NAME){
			rObj.center = get_center(poField, data, dataPos, dataSize);
		} else if (fieldName == WIDTH_FIELD_NAME) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			rObj.width = fval;
		} else if (fieldName == HEIGHT_FIELD_NAME) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			rObj.height = fval;
		} else if (fieldName == ORIENTATION_FIELD_NAME) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			rObj.orientation = fval;
		}

		int nInstBytes = poField->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
		iField++;	
	}
	
	return rObj;
}


int main (int argc, char* argv[]) {
	const char *file_name = NULL;
	HFAHandle hHFA;
	HFAEntry* root;

	if (argc > 1) {
		file_name = argv[1];
		printf("f: %s\n", file_name);
		hHFA = HFAOpen(file_name, "r");
	}
	if (hHFA == NULL) {
		printf("[x] HFAOpen() failed.\n");
		exit(100);	
	}

	root = hHFA-> poRoot;
	cout << "---tree display---" << endl;
	display(root);

	vector<HFAEntry*> nodes = find(root, "Rectangle2");
	vector<HFAEntry*>::iterator nodes_it;
	for (nodes_it = nodes.begin(); nodes_it!=nodes.end(); ++nodes_it) {
		HFAEntry* n = *nodes_it;
		n->LoadData();
		rectangle r = get_rectangle(n);
		display(r);
	}

	return 1;
}
