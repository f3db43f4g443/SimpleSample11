#pragma once
#include "Render/DrawableGroup.h"
#include "Render/Prefab.h"
#include "Block.h"
#include "LevelGenerate.h"
#include "ItemDrop.h"

class CGlobalCfg
{
public:
	map<string, string> mapPrefabPath;
	SLevelGenerateNodeLoadContext levelGenerateNodeContext;
	SLevelGenerateFileContext* pRootGenerateFile;

	string strTutorialLevelPrefab;
	string strMainLevelPrefab;
	string strMainMenuLevel;
	vector<string> vecLevels;
	vector<uint32> vecLvEntries;

	SItemDropNodeLoadContext itemDropNodeContext;
	CBonusStageDrop bonusStageDrop;

	vector<pair<uint32, float> > vecCombo2PointMul;
	vector<pair<uint32, uint32> > vecPoint2Reward;
	void GetComboLevel( uint32 nCombo, int32& nLevel, float& fPercent );
	float Combo2PointMul( uint32 nCombo );
	uint32 Point2Reward( uint32 nPoint );

	void Load();

	DECLARE_GLOBAL_INST_REFERENCE( CGlobalCfg );
};