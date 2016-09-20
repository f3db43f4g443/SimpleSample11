#pragma once
#include "TileMap2D.h"
#include "FaceEditItem.h"

struct SSkinBaseInfo
{
	uint32 nMaxHp;
	uint32 nHp;

	string strTileMapName;
	uint32 nTileMapEditData;
	uint32 nTileMapEditDataCount;
};

class CSkin : public CFaceEditItem, public SSkinBaseInfo
{
public:
	CSkin() { nType = eFaceEditType_Skin; }
	
	virtual bool IsValidGrid( CFace* pFace, const TVector2<int32>& pos ) override;
	virtual void Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos ) override;
};

class CMask : public CFaceEditItem
{
	CMask() { nType = eFaceEditType_Mask; }

};

class CSkinNMaskCfg
{
public:
	STileMapSetData tileMapSetData;

	void Load();
	DECLARE_GLOBAL_INST_REFERENCE( CSkinNMaskCfg );
};