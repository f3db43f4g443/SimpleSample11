#pragma once
#include "Render/DrawableGroup.h"
#include "Render/Prefab.h"

class CGlobalCfg
{
public:
	CReference<CDrawableGroup> pFaceEditTile;
	CReference<CDrawableGroup> pFaceSelectTile;
	CReference<CDrawableGroup> pFaceSelectRed;
	CReference<CDrawableGroup> pFaceSelectBullet;
	CReference<CDrawableGroup> pWorldSelectTile;

	void Load();

	DECLARE_GLOBAL_INST_REFERENCE( CGlobalCfg );
};