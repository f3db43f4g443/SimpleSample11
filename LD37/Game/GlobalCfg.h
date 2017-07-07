#pragma once
#include "Render/DrawableGroup.h"
#include "Render/Prefab.h"
#include "Block.h"
#include "LevelGenerate.h"

class CGlobalCfg
{
public:
	map<string, string> mapPrefabPath;
	map<string, string> mapSoundPath;
	SLevelGenerateNodeLoadContext levelGenerateNodeContext;
	SLevelGenerateFileContext* pRootGenerateFile;

	string strTutorialLevelPrefab;
	string strMainLevelPrefab;
	string strMainMenuLevel;
	vector<string> vecLevels;

	void Load();

	DECLARE_GLOBAL_INST_REFERENCE( CGlobalCfg );
};