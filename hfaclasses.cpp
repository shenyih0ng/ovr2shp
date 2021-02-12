#include "hfa_p.h"
#include "hfaclasses.h"

using namespace std;

/* 
 * get_xyCoords [utility]
 *
 * Extract xy coordinates from HFAField that has x&y attributes
 * Examples of such fields includes "center", "origin"
 * 
 * @param xy	HFAField
 * @param data	GByte*
 * @param dataPos	GInt32
 * @param dataSize	GInt32
 *
 * @return double[2]{x,y}
 */
double* get_xyCoords (HFAField* xy, GByte* data, GInt32 dataPos, GInt32 dataSize) {
	double* xyCoords = new double[2];

	void* pReturn;
	xy->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'p', &pReturn);
	int nByteOffset = ((GByte *) pReturn) - data;
	data += nByteOffset;
	dataPos += nByteOffset;
	dataSize -= nByteOffset;

	HFAType* xyType = xy->poItemObjectType;
	HFAField *coordX = xyType->papoFields[0];
	coordX->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &xyCoords[0]);

	int nInstBytes = coordX->GetInstBytes(data);
	data += nInstBytes;
	dataPos += nInstBytes;
	dataSize -= nInstBytes;

	HFAField *coordY = xyType->papoFields[1];
	coordY->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &xyCoords[1]);

	return xyCoords;
}

/*
 * HFAGeom()
 *
 * Construct an HFAGeom from HFAEntry.
 *
 * Caveats:
 * - Only extracts fields/nested fields that have a 'd' dtype (double)
 * 
 * @param node	HFAEntry representing a HFA "geometry" object e.g Rectangle2, Eant_Ellipse
 */
HFAGeom::HFAGeom (HFAEntry* node) {
	string CENTER_FIELD_NAME = "center";
	string ORIGIN_FIELD_NAME = "origin"; // temp (for Text2 compatbility)
	string ORIEN_FIELD_NAME = "orientation";

	int iField = 0;
	double fval;
	HFAField *poField;

	HFAType* ntype = node->GetPoType();
	GByte* data = node->GetData();
	GInt32 dataPos = node->GetDataPos();
	GInt32 dataSize = node->GetDataSize();
	
	while (iField < ntype->nFields) {
		poField = ntype->papoFields[iField];	
		char* fieldName = poField->pszFieldName;
		if (fieldName == CENTER_FIELD_NAME || fieldName == ORIGIN_FIELD_NAME) {
			center = get_xyCoords(poField, data, dataPos, dataSize);	
		} else if (poField->chItemType == 'd') {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 'd', &fval);
			if (fieldName == ORIEN_FIELD_NAME) {
				orientation = (double)fval;
			} else {
				fieldValues.insert(make_pair<string, double>(fieldName, (double)fval));	
			}
		}
		int nInstBytes = poField->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
		iField++;	
	}
}

/*
 * HFAAnnotation()
 *
 * Constructs an HFAAnnotation from HFAEntry
 *
 * @param node	HFAEntry representing a HFA "annotation" object e.g. Element_Eant, Element_2_Eant
 */
HFAAnnotation::HFAAnnotation(HFAEntry* node) {
	string NAME_FIELD = "name";
	string DESC_FIELD = "description";

	int iField = 0;
	char* fval;
	HFAField *poField;

	HFAType* ntype = node->GetPoType();
	GByte* data = node->GetData();
	GInt32 dataPos = node->GetDataPos();
	GInt32 dataSize = node->GetDataSize();

	while (iField < ntype->nFields) {
		poField = ntype->papoFields[iField];	
		char* fieldName = poField->pszFieldName;
		if (fieldName == NAME_FIELD || fieldName == DESC_FIELD) {
			poField->ExtractInstValue(NULL, 0, data, dataPos, dataSize, 's', &fval);
			if (fieldName == NAME_FIELD) { name = fval; }
			else { description = fval; }
		}
		int nInstBytes = poField->GetInstBytes(data);
		data += nInstBytes;
		dataPos += nInstBytes;
		dataSize -= nInstBytes;
		iField++;	
	}
}

/* 
 * HFAAnnotation
 *
 * Insertion operator overloading for HFAAnnotation display
 */
ostream& operator <<(ostream& os, const HFAAnnotation& ha) {
	os << "gtype: " << ha.geom->get_type() << endl;
	os << "n: " << ha.name << " [" << ha.description << "]" << endl;
	os << *ha.geom << " ";

	return os;	
}
