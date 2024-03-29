#pragma once
#include "Render/DrawableGroup.h"
#include "Render/Prefab.h"

class CGlobalCfg
{
public:
	vector<int32> vecLevelExp;
	vector<CVector4> vecAttackLevelColor;

	void Load();
	int32 GetLevelByExp( int32 nExp );

	DECLARE_GLOBAL_INST_REFERENCE( CGlobalCfg );
};