#pragma once
#include "Render/DrawableGroup.h"
#include "Render/Prefab.h"
#include "World.h"

class CGlobalCfg
{
public:
	void Load();

	string strCfgPath;
	string strEntry;
	TVector2<int32> playerEnterPos;
	int8 nPlayerEnterDir;
	string strWorldInitScript;

	CReference<CPrefab> pFailLightningEffectPrefab;
	CReference<CDrawableGroup> pFallEftDrawable;
	CReference<CPrefab> pBattleEftPrefab;
	CReference<CPrefab> pInterferenceStripEftPrefab;
	CReference<CPrefab> pTracerSpawnEftPrefab;
	CReference<CPrefab> pCommonLinkPrefab;

	map<string, CReference<CSoundFile> > mapSoundEffect;

	struct
	{
		float fTransferCamSpeed;
		int32 nTransferFadeOutFrameCount;
		vector<pair<CVector4, CVector4> > vecTransferMaskParams;
	} lvTransferData;
	struct SLvIndicatorData
	{
		struct SParamItem
		{
			CVector4 params[2];
			CVector2 ofs;
		};
		vector<SParamItem> vecPawnParams;
		vector<SParamItem> vecPawn0Params;
		vector<SParamItem> vecPawnFallParams;
		vector<SParamItem> vecUseParams;
		vector<SParamItem> vecMountParams;
		vector<SParamItem> vecHitParams;
		vector<SParamItem> vecHitBlockedParams;
		vector<SParamItem> vecMissParams;
		vector<SParamItem> vecBlockedParams;
		vector<SParamItem> vecBlockedParams1;
		vector<SParamItem> vecStealthBackParams1;
		vector<SParamItem> vecStealthBackParams2;
		vector<SParamItem> vecStealthAlertParams;
		vector<SParamItem> vecStealthDetectParams;
		vector<SParamItem> vecStealthHiddenParams;
		vector<SParamItem> vecStealthAlertHiddenParams;
		vector<SParamItem> vecStealthDetectHiddenParams;
		SParamItem NextLevelParamMin;
		SParamItem NextLevelParamMax;
		int32 nNextLevelParamCount;
		SParamItem NextLevelBlockedParamMin;
		SParamItem NextLevelBlockedParamMax;
		int32 nNextLevelBlockedParamCount;
		CReference<CPrefab> pIndicatorPrefab;
	} lvIndicatorData;
	struct SMainUIData
	{
		struct SActionEftFrame
		{
			struct SImgParam
			{
				CVector3& v() { return *( CVector3* )this; }
				float fOfs;
				float fLum;
				float fOpaque;
			};
			SImgParam params[6];
			int32 nTotalHeight, nMaxImgHeight;
		};
		vector<SActionEftFrame> vecActionEftFrames;
		struct SPlayerInputParam
		{
			CVector4 params[4];
			CVector2 ofs;
		};
		vector<SPlayerInputParam> vecPlayerInputParams;
	} MainUIData;
	vector<pair<CVector4, CVector4> > playerDamagedMask;
	vector<pair<CVector4, CVector4> > showSnapShotMask;
	vector<pair<CVector4, CVector4> > battleEffectMask;

	DECLARE_GLOBAL_INST_REFERENCE( CGlobalCfg );
};