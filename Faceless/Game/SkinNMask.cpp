#include "stdafx.h"
#include "SkinNMask.h"
#include "Face.h"
#include "Common/TabFile.h"
#include "Common/ResourceManager.h"

bool CSkin::IsValidGrid( CFace* pFace, const TRectangle<int32>& editRect, const TVector2<int32>& pos )
{
	auto pGrid = pFace->GetGrid( pos.x, pos.y );
	if( !pGrid )
		return false;
	return pGrid->bEnabled;
}

void CSkin::Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos )
{
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			pFace->SetSkin( this, pos.x + i, pos.y + j );
		}
	}
}

void CSkinNMaskCfg::Load()
{
	{
		CTabFile tabFile;
		tabFile.Load( "configs/tilemapset.txt" );

		for( int i = 0; i < tabFile.GetRowCount(); i++ )
		{
			const char* szName = tabFile.Get( "Name", i, "" );
			const char* szResource = tabFile.Get( "Resource", i, "" );
			auto& item = tileMapSetData.items[szName];
			item.strName = szName;
			item.strResource = szResource;
		}
	}

	{
		CTabFile tabFile;
		tabFile.Load( "configs/skin.txt" );

		for( int i = 0; i < tabFile.GetRowCount(); i++ )
		{
			CSkin* pSkin = new CSkin;
			pSkin->strName = tabFile.Get( "Name", i, "" );
			pSkin->nCost = tabFile.Get( "Cost", i, 0 );
			pSkin->nWidth = tabFile.Get( "Width", i, 0 );
			pSkin->nHeight = tabFile.Get( "Height", i, 0 );
			pSkin->nMaxHp = tabFile.Get( "Hp", i, 0 );
			pSkin->strTileMapName = tabFile.Get( "TileMapName", i, "" );
			pSkin->nTileMapEditData = tabFile.Get( "TileMapEditData", i, 0 );
			pSkin->strEffectName = tabFile.Get( "EffectName", i, "" );
			if( pSkin->strEffectName.length() )
				pSkin->pEffect = CResourceManager::Inst()->CreateResource<CDrawableGroup>( pSkin->strEffectName.c_str() );
			pSkin->nEffectRows = tabFile.Get( "EffectRows", i, 0 );
			pSkin->nEffectColumns = tabFile.Get( "EffectColumns", i, 0 );

			mapSkins[pSkin->strName] = pSkin;
		}
	}
}

void CSkinNMaskCfg::Unload()
{
	tileMapSetData.items.clear();
	for( auto& item : mapSkins )
	{
		delete item.second;
		item.second = NULL;
	}
	mapSkins.clear();
}