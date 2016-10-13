#pragma once
#include "Render/TileMap2D.h"
#include "Render/DrawableGroup.h"
#include "FaceEditItem.h"

struct SSkinBaseInfo
{
	uint32 nMaxHp;

	string strTileMapName;
	uint32 nTileMapEditData;

	string strEffectName;
	CReference<CDrawableGroup> pEffect;
	uint32 nEffectRows;
	uint32 nEffectColumns;
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

	map<string, CSkin*> mapSkins;

	void Load();
	
	CSkin* GetSkin( const char* szName )
	{
		auto itr = mapSkins.find( szName );
		if( itr == mapSkins.end() )
			return NULL;
		return itr->second;
	}

	DECLARE_GLOBAL_INST_REFERENCE( CSkinNMaskCfg );
};