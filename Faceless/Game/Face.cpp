#include "stdafx.h"
#include "Face.h"
#include "MyLevel.h"

void CFace::OnAddedToStage()
{
	m_pOrganRoot = GetChildByName_Fast<CEntity>( "organs" );
	
	auto pTileMapObject = GetChildByName_Fast<CEntity>( "tiles" );
	auto pTileMap = static_cast<CTileMap2D*>( pTileMapObject->GetRenderObject() );
	m_baseOffset = pTileMap->GetBaseOffset();
	m_gridScale = pTileMap->GetTileSize();
	m_nWidth = pTileMap->GetWidth() + 1;
	m_nHeight = pTileMap->GetHeight() + 1;
	m_grids.resize( m_nWidth * m_nHeight );

	m_pFaceEditTile = static_cast<CTileMap2D*>( CMyLevel::GetInst()->pFaceEditTile->CreateInstance() );
	AddChild( m_pFaceEditTile );
	m_pFaceEditTile->Set( m_gridScale, m_baseOffset - m_gridScale * 0.5f, m_nWidth, m_nHeight );
	m_pFaceEditTile->bVisible = false;

	for( int j = 0; j < m_nHeight; j++ )
	{
		for( int i = 0; i < m_nWidth; i++ )
		{
			auto pGrid = GetGrid( i, j );
			pGrid->bEnabled = !!pTileMap->GetUserData( i, j );
			if( pGrid->bEnabled )
				pGrid->nSkinHp.base = m_nDefaultSkinMaxHp;
			
			RefreshEditTile( i, j, 0 );
		}
	}

	m_pSkinTile = new CTileMapSet( pTileMap->GetTileSize(), pTileMap->GetBaseOffset(), pTileMap->GetWidth(), pTileMap->GetHeight(),
		&CSkinNMaskCfg::Inst().tileMapSetData );
	pTileMapObject->SetRenderObject( m_pSkinTile );
	pTileMapObject->SetResource( NULL );
}

void CFace::OnRemovedFromStage()
{

}

bool CFace::AddOrgan( COrgan* pOrgan, uint32 x, uint32 y )
{
	for( int i = x; i < x + pOrgan->GetWidth(); i++ )
	{
		for( int j = y; j < y + pOrgan->GetHeight(); j++ )
		{
			auto pGrid = GetGrid( x, y );
			if( !pGrid || pGrid->pOrgan || pGrid->nSkinHp <= 0 )
				return false;
		}
	}

	pOrgan->SetParentEntity( m_pOrganRoot );
	pOrgan->m_pFace = this;
	pOrgan->m_pos.x = x;
	pOrgan->m_pos.y = y;
	pOrgan->SetPosition( CVector2( x - pOrgan->m_nWidth + 1, y - pOrgan->m_nHeight + 1 ) * m_gridScale + m_baseOffset );

	for( int i = x; i < x + pOrgan->GetWidth(); i++ )
	{
		for( int j = y; j < y + pOrgan->GetHeight(); j++ )
		{
			auto pGrid = GetGrid( i, j );
			pGrid->pOrgan = pOrgan;
			RefreshEditTile( i, j, 0 );
		}
	}
	return true;
}

bool CFace::RemoveOrgan( COrgan* pOrgan )
{
	if( pOrgan->m_pFace != this )
		return false;

	for( int i = pOrgan->GetGridPos().x; i < pOrgan->GetGridPos().x + pOrgan->GetWidth(); i++ )
	{
		for( int j = pOrgan->GetGridPos().y; j < pOrgan->GetGridPos().y + pOrgan->GetHeight(); j++ )
		{
			auto pGrid = GetGrid( i, j );
			pGrid->pOrgan = NULL;
			RefreshEditTile( i, j, 0 );
		}
	}
	pOrgan->m_pFace = NULL;
	pOrgan->SetParentEntity( NULL );
	return true;
}

bool CFace::SetSkin( CSkin* pSkin, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || !pGrid->bEnabled )
		return false;
	pGrid->pSkin = pSkin;

	uint32 nMaxHp = pSkin ? pSkin->nMaxHp : m_nDefaultSkinMaxHp;
	uint32 nCurHp = pGrid->nSkinHp + ( pSkin ? pSkin->nHp : m_nDefaultSkinHp );
	pGrid->nSkinHp.base = nMaxHp;
	pGrid->nSkinHp.SetCurValue( nCurHp );

	if( pSkin )
	{
		float fPercent = -pGrid->nSkinHp.add2 * 1.0f / pGrid->nSkinHp.GetMaxValue();
		m_pSkinTile->EditTile( x, y, pSkin->strTileMapName.c_str(), pSkin->nTileMapEditData + ( pSkin->nTileMapEditDataCount - 1 ) * fPercent );
	}
	else
		m_pSkinTile->EditTile( x, y, NULL, 0 );
	
	RefreshEditTile( x, y, 0 );
}

void CFace::OnBeginEdit()
{
	m_pFaceEditTile->bVisible = true;
}

void CFace::OnEndEdit()
{
	m_pFaceEditTile->bVisible = false;
}

CRectangle CFace::GetFaceRect()
{
	return CRectangle( m_baseOffset - m_gridScale * 0.5f, m_baseOffset + m_gridScale * CVector2( m_nWidth + 0.5f, m_nHeight + 0.5f ) );
}

CRectangle CFace::GetKillBound()
{
	return CRectangle( m_baseOffset - m_gridScale * 4, m_baseOffset + m_gridScale * CVector2( m_nWidth + 4, m_nHeight + 4 ) );
}

void CFace::RefreshEditTile( uint32 x, uint32 y, uint8 nFlag )
{
	auto pGrid = GetGrid( x, y );
	uint16 nTile = pGrid->GetEditType() | ( nFlag << 2 );
	m_pFaceEditTile->SetTile( x, y, 1, &nTile );
}

bool CFace::IsEditValid( CFaceEditItem* pItem, const TVector2<int32>& pos )
{
	bool bIsEditValid = pItem->nType == eFaceEditType_Organ ? true : false;
	for( int i = 0; i < pItem->nWidth; i++ )
	{
		for( int j = 0; j < pItem->nHeight; j++ )
		{
			bool bIsValid = pItem->IsValidGrid( this, TVector2<int32>( i + pos.x, j + pos.y ) );
			if( pItem->nType == eFaceEditType_Organ )
				bIsEditValid = bIsEditValid && bIsValid;
			else
				bIsEditValid = bIsEditValid || bIsValid;
		}
	}
	return bIsEditValid;
}