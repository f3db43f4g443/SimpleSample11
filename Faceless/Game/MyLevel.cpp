#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "Player.h"
#include "GUI/MainUI.h"
#include "GlobalCfg.h"

void CTurnBasedContext::AIFunc()
{
	static_cast<CMyLevel*>( GetParentEntity() )->Turn();
}

CMyLevel* CMyLevel::s_pLevel = NULL;
void CMyLevel::OnAddedToStage()
{
	CreateGrids();
	s_pLevel = this;
	m_nCurCharacterID = 0;

	if( !m_pTurnBasedContext )
	{
		m_pTurnBasedContext = new CTurnBasedContext;
		m_pTurnBasedContext->SetParentEntity( this );
	}
}

void CMyLevel::OnRemovedFromStage()
{
	if( s_pLevel == this )
		s_pLevel = NULL;
}

void CMyLevel::CreateGrids()
{
	auto pTileMap = static_cast<CTileMap2D*>( GetChildByName_Fast<CEntity>( "tiles" )->GetRenderObject() );
	m_baseOffset = pTileMap->GetBaseOffset();
	m_gridScale = pTileMap->GetTileSize();
	m_nWidth = pTileMap->GetWidth() + 1;
	m_nHeight = pTileMap->GetHeight() + 1;
	m_grids.resize( m_nWidth * m_nHeight );

	for( int j = 0; j < m_nHeight; j++ )
	{
		for( int i = 0; i < m_nWidth; i++ )
		{
			GetGrid( i, j )->nGridFlag = pTileMap->GetUserData( i, j );
		}
	}

	m_pWorldSelectTile = static_cast<CTileMap2D*>( CGlobalCfg::Inst().pFaceEditTile->CreateInstance() );
	AddChildBefore( m_pWorldSelectTile, pTileMap );
	m_pWorldSelectTile->Set( m_gridScale, m_baseOffset - m_gridScale * 0.5f, m_nWidth, m_nHeight );
	m_pWorldSelectTile->bVisible = false;
}

bool CMyLevel::AddCharacter( CCharacter* pCharacter, uint32 x, uint32 y )
{
	auto pGrid = GetGrid( x, y );
	if( !pGrid || ( pGrid->nGridFlag & eGridFlag_BlockMove ) || pGrid->pCharacter )
		return false;
	pGrid->pCharacter = pCharacter;
	m_characters.Insert( pCharacter );
	pCharacter->m_nCharacterStageID = m_nCurCharacterID++;
	pCharacter->m_grid = TVector2<int32>( x, y );
	pCharacter->m_pLevel = this;
	pCharacter->SetParentEntity( this );
	pCharacter->SetPosition( m_baseOffset + m_gridScale * CVector2( x, y ) );
	pCharacter->Face( pCharacter->m_nDir );
	return true;
}

bool CMyLevel::RemoveCharacter( CCharacter* pCharacter )
{
	if( pCharacter->m_pLevel != this )
		return false;
	m_characters.Remove( pCharacter );
	
	auto pGrid = GetGrid( pCharacter->m_grid.x, pCharacter->m_grid.y );
	pGrid->pCharacter = NULL;
	pCharacter->SetParentEntity( NULL );
}

bool CMyLevel::MoveCharacter( CCharacter* pCharacter, uint32 x, uint32 y )
{
	if( pCharacter->m_pLevel != this )
		return false;
	if( x == pCharacter->m_grid.x && y == pCharacter->m_grid.y )
		return false;
	
	auto pGrid = GetGrid( x, y );
	if( !pGrid || ( pGrid->nGridFlag & eGridFlag_BlockMove ) || pGrid->pCharacter )
		return false;
	pGrid->pCharacter = pCharacter;
	pGrid = GetGrid( pCharacter->m_grid.x, pCharacter->m_grid.y );
	pGrid->pCharacter = NULL;
	pCharacter->m_grid = TVector2<int32>( x, y );
	pCharacter->SetPosition( m_baseOffset + m_gridScale * CVector2( x, y ) );
	return true;
}

void CMyLevel::RayCast( TVector2<int32> src, TVector2<int32> target, function<bool( const TVector2<int32>& )> func )
{
	auto delta = target - src;
	TVector2<int32> dir1, dir2, pt;
	if( delta.x > 0 )
	{
		if( delta.y > 0 )
		{
			pt = TVector2<int32>( 1, 1 );
			dir1 = TVector2<int32>( 1, 0 );
			dir2 = TVector2<int32>( 0, 1 );
		}
		else
		{
			pt = TVector2<int32>( 1, -1 );
			dir1 = TVector2<int32>( 0, -1 );
			dir2 = TVector2<int32>( 1, 0 );
		}
	}
	else
	{
		if( delta.y > 0 )
		{
			pt = TVector2<int32>( -1, 1 );
			dir1 = TVector2<int32>( 0, 1 );
			dir2 = TVector2<int32>( -1, 0 );
		}
		else
		{
			pt = TVector2<int32>( -1, -1 );
			dir1 = TVector2<int32>( -1, 0 );
			dir2 = TVector2<int32>( 0, -1 );
		}
	}
	TVector2<int32> dir0 = dir1 + dir2;
	
	vector<CHitProxy*> vecOverlaps;
	auto curGrid = src;
	int32 s = delta.x * pt.y - delta.y * pt.x;
	while( true )
	{
		auto pGrid = GetGrid( curGrid.x, curGrid.y );
		if( !pGrid )
			break;
		if( !func( curGrid ) )
			break;
		if( curGrid.x == target.x && curGrid.y == target.y )
			break;

		if( s > 0 )
		{
			curGrid = curGrid + dir1;
			s += ( delta.x * dir1.y - delta.y * dir1.x ) * 2;
		}
		else if( s < 0 )
		{
			curGrid = curGrid + dir2;
			s += ( delta.x * dir2.y - delta.y * dir2.x ) * 2;
		}
		else
		{
			curGrid = curGrid + dir0;
			s += ( delta.x * dir0.y - delta.y * dir0.x ) * 2;
		}
	}
}

void CMyLevel::Turn()
{
	while( true )
	{
		m_pTurnBasedContext->Yield( 0, false );

		auto pCharacter = static_cast<CCharacter*>( m_characters.Front() );
		if( pCharacter )
		{
			uint32 nDelay = pCharacter->m_nDelay;
			auto pChars = m_characters.Head();
			if( pChars )
			{
				for( int i = 1; i <= m_characters.Size(); i++ )
					static_cast<CCharacter*>( pChars[i] )->m_nDelay -= nDelay;
			}

			pCharacter->OnTurn( m_pTurnBasedContext );
		}
	}
}