#pragma once
#include "Render/DrawableGroup.h"
#include "Render/Prefab.h"
#include "World.h"

class CGlobalCfg
{
public:
	void Load();

	DECLARE_GLOBAL_INST_REFERENCE( CGlobalCfg );
};