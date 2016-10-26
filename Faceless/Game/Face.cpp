#include "stdafx.h"
#include "Face.h"
#include "MyLevel.h"
#include "Stage.h"
#include "GlobalCfg.h"

void CFace::OnAddedToStage()
{
	m_pDefaultSkin = CSkinNMaskCfg::Inst().GetSkin( m_strDefaultSkinName.c_str() );

	m_pOrganRoot = GetChildByName_Fast<CEntity>( "organs" );
	
	auto pTileMapObject = GetChildByName_Fast<CEntity>( "tiles" );
	CReference<CTileMap2D> pTileMap = static_cast<CTileMap2D*>( pTileMapObject->GetRenderObject() );
	m_baseOffset = pTileMap->GetBaseOffset();
	m_gridScale = pTileMap->GetTileSize();
	m_nWidth = pTileMap->GetWidth() + 1;
	m_nHeight = pTileMap->GetHeight() + 1;
	m_grids.resize( m_nWidth * m_nHeight );

	m_pFaceEditTile = static_cast<CTileMap2D*>( CGlobalCfg::Inst().pFaceEditTile->CreateInstance() );
	AddChild( m_pFaceEditTile );
	m_pFaceEditTile->Set( m_gridScale, m_baseOffset - m_gridScale * 0.5f, m_nWidth, m_nHeight );
	m_pFaceEditTile->bVisible = false;
	m_pFaceSelectTile = static_cast<CTileMap2D*>( CGlobalCfg::Inst().pFaceSelectTile->CreateInstance() );
	AddChild( m_pFaceSelectTile );
	m_pFaceSelectTile->Set( pTileMap->GetTileSize(), pTileMap->GetBaseOffset(), pTileMap->GetWidth(), pTileMap->GetHeight() );
	m_pFaceSelectTile->bVisible = false;
	m_pGUIRoot = new CEntity();
	m_pGUIRoot->SetParentEntity( this );

	m_pSkinTile = new CTileMapSet( pTileMap->GetTileSize(), pTileMap->GetBaseOffset(), pTileMap->GetWidth(), pTileMap->GetHeight(),
		&CSkinNMaskCfg::Inst().tileMapSetData );
	pTileMapObject->SetRenderObject( m_pSkinTile );
	pTileMapObject->SetResource( NULL );

	for( int j = 0; j < m_nHeight; j++ )
	{
		for( int i = 0; i < m_nWidth; i++ )
		{
			auto pGrid = GetGrid( i, j );
			pGrid->bEnabled = !!pTileMap->GetUserData( i, j );
			if( pGrid->bEnabled )
				SetSkin( m_pDefaultSkin, i, j );
			
			RefreshEditTile( i, j, 0 );
		}
	}

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CFace::OnRemovedFromStage()
{
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
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
	pOrgan->SetPosition( CVector2( x + ( pOrgan->m_nWidth - 1 ) * 0.5f, y + ( pOrgan->m_nHeight - 1 ) * 0.5f ) * m_gridScale + m_baseOffset );

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

bool CFace::KillOrgan( COrgan * pOrgan )
{
	if( pOrgan->m_pFace != this )
		return false;

	for( int32 i = pOrgan->GetGridPos().x; i < pOrgan->GetGridPos().x + pOrgan->GetWidth(); i++ )
	{
		for( int32 j = pOrgan->GetGridPos().y; j < pOrgan->GetGridPos().y + pOrgan->GetHeight(); j++ )
		{
			auto pGrid = GetGrid( i, j );
			SetSkinHp( 0, i, j );
		}
	}

	RemoveOrgan( pOrgan );
	return true;
}

bool CFace::SetSkin( CSkin* pSkin, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || !pGrid->bEnabled )
		return false;
	pGrid->pSkin = pSkin;

	uint32 nMaxHp = pSkin ? pSkin->nMaxHp : 0;
	pGrid->nSkinHp = pGrid->nSkinMaxHp = nMaxHp;
	if( pGrid->pEffect )
	{
		pGrid->pEffect->RemoveThis();
		pGrid->pEffect = NULL;
	}

	if( pSkin )
		m_pSkinTile->EditTile( x, y, pSkin->strTileMapName.c_str(), pSkin->nTileMapEditData );
	else
		m_pSkinTile->EditTile( x, y, NULL, 0 );
	
	RefreshEditTile( x, y, 0 );
}

void CFace::SetSkinHp( uint32 nHp, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || !pGrid->bEnabled )
		return;

	uint32 nRow = pGrid->nSkinHp ? ( pGrid->nSkinHp * ( pGrid->pSkin->nEffectRows + 1 ) - 1 ) / pGrid->nSkinMaxHp + 1 : 0;
	pGrid->nSkinHp = Min( nHp, pGrid->nSkinMaxHp );
	uint32 nRow1 = pGrid->nSkinHp ? ( pGrid->nSkinHp * ( pGrid->pSkin->nEffectRows + 1 ) - 1 ) / pGrid->nSkinMaxHp + 1 : 0;
	if( pGrid->pSkin->pEffect && nRow != nRow1 )
	{
		if( nRow1 < pGrid->pSkin->nEffectRows )
		{
			if( !pGrid->pEffect )
			{
				pGrid->pEffect = static_cast<CMultiFrameImage2D*>( pGrid->pSkin->pEffect->CreateInstance() );
				pGrid->pEffect->SetZOrder( 1 );
				pGrid->pEffect->SetPosition( CVector2( x, y ) * m_gridScale + m_baseOffset );
				m_pSkinTile->AddChild( pGrid->pEffect );
			}
			pGrid->pEffect->SetFrames( ( pGrid->pSkin->nEffectRows - 1 - nRow1 ) * pGrid->pSkin->nEffectColumns,
				( pGrid->pSkin->nEffectRows - nRow1 ) * pGrid->pSkin->nEffectColumns, pGrid->pEffect->GetData()->fFramesPerSec );
		}
		else
		{
			if( pGrid->pEffect )
			{
				pGrid->pEffect->RemoveThis();
				pGrid->pEffect = NULL;
			}
		}
	}
}

void CFace::DamageSkin( uint32 nDmg, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || !pGrid->bEnabled )
		return;

	SetSkinHp( Max( 0, (int)( pGrid->nSkinHp - nDmg ) ), x, y );
	if( !pGrid->nSkinHp )
	{
		if( pGrid->pOrgan )
			KillOrgan( pGrid->pOrgan );
	}
}

void CFace::OnBeginEdit()
{
	m_pFaceEditTile->bVisible = true;
}

void CFace::OnEndEdit()
{
	m_pFaceEditTile->bVisible = false;
}

void CFace::OnBeginSelectTarget()
{
	m_pFaceSelectTile->bVisible = true;

	m_pSelectEffect = CGlobalCfg::Inst().pFaceSelectRed->CreateInstance();
	m_pSelectEffect->SetAutoUpdateAnim( true );
	m_pSelectEffect->SetZOrder( 1 );
	m_pGUIRoot->AddChild( m_pSelectEffect );
	UpdateSelectGrid( TVector2<int32>( 0, 0 ) );
}

void CFace::OnEndSelectTarget()
{
	m_pSelectEffect->RemoveThis();
	m_pFaceSelectTile->bVisible = false;
}

void CFace::UpdateSelectGrid( TVector2<int32> grid )
{
	if( m_pSelectEffect )
		m_pSelectEffect->SetPosition( CVector2( grid.x, grid.y ) * m_gridScale + m_baseOffset );
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

void CFace::RefreshSelectTile( uint32 x, uint32 y, uint8 nType )
{
	auto pGrid = GetGrid( x, y );
	m_pFaceEditTile->EditTile( x, y, nType );
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

void CFace::OnTickAfterHitTest()
{
	if( m_nAwakeFrames )
		m_nAwakeFrames--;

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}
