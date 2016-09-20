#include "stdafx.h"
#include "SkinNMask.h"
#include "Face.h"
#include "Common/TabFile.h"

bool CSkin::IsValidGrid( CFace* pFace, const TVector2<int32>& pos )
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