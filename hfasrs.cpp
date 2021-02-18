#include "ovr2shp.h"

using namespace std;

// TODO extract MapInfo, Projection

/*
 * extract_proj
 *
 * Extract GCS + PCS from HFA structure
 *
 * Caveats:
 * - For convenience, initialization of the SRS is offloaded to OGR via the use of "SetWellKnownGCS"
 *   - **This is not solution as it only supports limited amount of well known GCS**
 *   - It will fail / results will be inaccurate if there is a unknown datum as the datum name is used to initialize the SRS
 *   - TODO initialize GCS + PCS with parameters instead of 'keys'. Some HFA Eprj_Datum node does not collect datum parameters, perhaps in this class we could treat it as a well known datum
 * - Supported Projections: UTM
 * 
 * @param hHFA 	HFAHandle  			HFA file handle
 * @param srs 	OGRSpatialReference& 		OGR spatial reference object
 */
bool extract_proj (HFAHandle hHFA, OGRSpatialReference& srs) {	
	const Eprj_MapInfo* mapInfo;
	const Eprj_ProParameters* projectionParams;
	const Eprj_Datum* datum;
	
	mapInfo = HFAGetMapInfo(hHFA);
	projectionParams = HFAGetProParameters(hHFA);
	datum = HFAGetDatum(hHFA);

	if (mapInfo == NULL || projectionParams == NULL) {
		// no map information present
		return false;
	}
	
	// GCS
	if (datum != NULL) {
		// set gcrs using the datum name
		char* datumName = datum->datumname;
		srs.SetWellKnownGeogCS(datumName);
	} else {
		// use spheroid as a fall back
		char* spheroidName = projectionParams->proSpheroid.sphereName;
		srs.SetWellKnownGeogCS(spheroidName);
	}

	// PCS	
	if (projectionParams->proNumber == 1) { //UTM 
		bool isNorth = projectionParams->proParams[3] >= 0;
		long zoneNum = projectionParams->proZone;
		srs.SetUTM(zoneNum, isNorth);
	}

	return true;
}
