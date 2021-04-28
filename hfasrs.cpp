#include "ovr2shp.h"

using namespace std;

/*
 * getMapInfo
 *
 * Adapted from HFAGetMapInfo
 * - HFAGetMapInfo is reliant on HFA structure having a Band Node
 * - HFAGetMapInfo gets the mapInfo/projParams using the first Band Node. Thus the function cant be used directly on .ovr
 * - The above applies to HFAGetProParameters & HFAGetDatum
 *
 * @param hHFA 			HFAHandle 	HFA File Handle
 * @returns Eprj_MapInfo
 */
const Eprj_MapInfo* getMapInfo (HFAHandle hHFA) {
	HFAEntry* mapInfoEntry;
	Eprj_MapInfo* mapInfo;

	mapInfoEntry = find(hHFA->poRoot, "Map_Info");
	if (mapInfoEntry == NULL) {
		Log(WARN) << "No MapInfo found";
		return NULL;
	}

	mapInfo = (Eprj_MapInfo *) CPLCalloc(sizeof(Eprj_MapInfo),1);

	mapInfo->proName = CPLStrdup(mapInfoEntry->GetStringField("proName"));

	mapInfo->upperLeftCenter.x = mapInfoEntry->GetDoubleField("upperLeftCenter.x");
	mapInfo->upperLeftCenter.y = mapInfoEntry->GetDoubleField("upperLeftCenter.y");

	mapInfo->lowerRightCenter.x = mapInfoEntry->GetDoubleField("lowerRightCenter.x");
	mapInfo->lowerRightCenter.y = mapInfoEntry->GetDoubleField("lowerRightCenter.y");
	
	mapInfo->pixelSize.width = mapInfoEntry->GetDoubleField("pixelSize.width");
	mapInfo->pixelSize.height = mapInfoEntry->GetDoubleField("pixelSize.height");

	mapInfo->units = CPLStrdup(mapInfoEntry->GetStringField("units"));

	hHFA->pMapInfo = (void*) mapInfo;

	return mapInfo;
}


/*
 * getProjectionParams
 *
 * Adapted from HFAGetProParameters (see getMapInfo docstring for more details)
 *
 * @param hHFA 				HFAHandle 	HFA File Handle
 * @returns Eprj_ProParameters
 */
const Eprj_ProParameters* getProjectionParams(HFAHandle hHFA) {
	HFAEntry* projEntry;
	Eprj_ProParameters* projection;

	projEntry = find(hHFA->poRoot, "Projection");
	if (projEntry == NULL) {
		Log(WARN) << "No Projection found";
		return NULL;
	}

	projection = (Eprj_ProParameters *)CPLCalloc(sizeof(Eprj_ProParameters),1);

	projection->proType = (Eprj_ProType) projEntry->GetIntField("proType");
	projection->proNumber = projEntry->GetIntField("proNumber");
	projection->proExeName =CPLStrdup(projEntry->GetStringField("proExeName"));
	projection->proName = CPLStrdup(projEntry->GetStringField("proName"));
	projection->proZone = projEntry->GetIntField("proZone");

	for(int i = 0; i < 15; i++ )
	{
	    char	szFieldName[30];

	    sprintf( szFieldName, "proParams[%d]", i );
	    projection->proParams[i] = projEntry->GetDoubleField(szFieldName);
	}

	projection->proSpheroid.sphereName = CPLStrdup(projEntry->GetStringField("proSpheroid.sphereName"));
	projection->proSpheroid.a = projEntry->GetDoubleField("proSpheroid.a");
	projection->proSpheroid.b = projEntry->GetDoubleField("proSpheroid.b");
	projection->proSpheroid.eSquared = projEntry->GetDoubleField("proSpheroid.eSquared");
	projection->proSpheroid.radius = projEntry->GetDoubleField("proSpheroid.radius");

	hHFA->pProParameters = (void*) projection;

	return projection;
}

/*
 * getDatum
 *
 * Adapted from HFAGetDatum (see getMapInfo docstring for more details)
 *
 * @param hHFA 			HFAHandle 	HFA File Handle
 * @returns Eprj_Datum
 */
const Eprj_Datum* getDatum(HFAHandle hHFA) {
	HFAEntry* datumEntry;
	Eprj_Datum* datum;

	datumEntry = find(hHFA->poRoot, "Datum");
	if (datumEntry == NULL) {
		Log(WARN) << "No Datum found";
		return NULL;
	}

	datum = (Eprj_Datum *) CPLCalloc(sizeof(Eprj_Datum),1);
	datum->datumname = CPLStrdup(datumEntry->GetStringField("datumname"));
	datum->type = (Eprj_DatumType) datumEntry->GetIntField("type");

	for(int i = 0; i < 7; i++ )
	{
	    char	szFieldName[30];

	    sprintf( szFieldName, "params[%d]", i );
	    datum->params[i] = datumEntry->GetDoubleField(szFieldName);
	}

	datum->gridname = CPLStrdup(datumEntry->GetStringField("gridname"));

	hHFA->pDatum = (void *) datum;

	return datum;
}


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
 * @param root 	HFAEntry* 			Root node
 * @param srs 	OGRSpatialReference& 		OGR spatial reference object
 */
bool extract_proj (HFAHandle hHFA, OGRSpatialReference& srs) {	
	const Eprj_MapInfo* mapInfo;
	const Eprj_ProParameters* projectionParams;
	const Eprj_Datum* datum;
	
	mapInfo = getMapInfo(hHFA);
	projectionParams = getProjectionParams(hHFA);
	datum = getDatum(hHFA);

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
