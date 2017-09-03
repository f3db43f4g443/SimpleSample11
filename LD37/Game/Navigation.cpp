#include "stdafx.h"
#include "Navigation.h"
#include "Common/FixedSizeAllocator.h"
#include "Stage.h"
#include "Common/Rand.h"

void CNavigationUnit::Set( bool bCanFly, float fMaxScanDist, uint32 nGridsPerStep )
{
	m_bCanFly = bCanFly;
	m_fMaxScanDist = fMaxScanDist;
	m_nGridsPerStep = nGridsPerStep;
	Reset();
}

void CNavigationUnit::Reset()
{
	m_trigger.Trigger( 0, NULL );
	for( auto pos : m_visited )
	{
		auto& grid = GetGrid( pos );
		grid.nType = INVALID_8BITID;
		grid.bClosed = false;
		grid.bInserted = false;
	}
	m_curTargetGrid = TVector2<int32>( -1, -1 );
	m_q.Reserve( m_q.GetAllNodes().capacity() );
	m_q.Clear();
	m_visited.reserve( m_visited.capacity() );
	m_visited.resize( 0 );
}

void CNavigationUnit::Clear()
{
	Reset();
	ClearPath();
	m_trigger.Clear();
	m_pTarget = NULL;
}

void CNavigationUnit::SetTarget( CEntity * pEntity )
{
	m_pTarget = pEntity;
	Reset();
}

void CNavigationUnit::Step( CCharacter * pChar )
{
	INavigationProvider* pProvider = pChar->GetStage()->GetNavigationProvider();
	if( !pProvider )
		return;
	auto gridSize = pProvider->GetGridSize();
	auto mapRect = pProvider->GetMapRect();
	if( gridSize != m_gridSize || mapRect != m_mapRect )
	{
		Reset();
		m_gridSize = gridSize;
		m_mapRect = mapRect;
		m_vecGrid.resize( mapRect.width * mapRect.height );
	}

	bool bReset = false;
	for( int i = 0; i < m_nGridsPerStep && m_q.Size(); i++ )
	{
		auto pGrid = static_cast<SGridData*>( m_q.Pop() );
		pGrid->bClosed = true;
		if( pGrid->pos == m_curTargetGrid )
		{
			m_trigger.Trigger( 1, pGrid );
			bReset = true;
			break;
		}
		if( !m_pTarget && ( pGrid->pos - m_curSrcGrid ).Length2() > m_fMaxScanDist * m_fMaxScanDist )
		{
			bReset = true;
			break;
		}

		m_trigger.Trigger( 0, pGrid );

		bool bFall = !m_bCanFly && pGrid->nType == 1 && pGrid->pos.y > 0
			&& CacheGridData( TVector2<int32>( pGrid->pos.x, pGrid->pos.y - 1 ), pProvider ).nType > 0;
		TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
		SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );
		for( int j = 0; j < ELEM_COUNT( ofs ); j++ )
		{
			TVector2<int32> pos1 = pGrid->pos;
			if( bFall && ofs[j].y != -1 )
				continue;

			float fDist = pGrid->fDist;
			while( 1 )
			{
				pos1 = pos1 + ofs[j];
				if( pos1.x < 0 || pos1.y < 0 || pos1.x >= mapRect.width || pos1.y >= mapRect.height )
					break;

				auto& grid1 = CacheGridData( pos1, pProvider );
				if( grid1.bClosed )
					break;

				if( grid1.nType == 0 )
					break;

				if( !m_bCanFly )
				{
					if( grid1.nType == 1 )
					{
						if( ofs[j].y == 1 )
							break;
						fDist += 1;
					}
					else
						fDist += 1;
				}
				else
					fDist += 1;

				if( grid1.fDist <= fDist )
					break;

				grid1.fDist = fDist;
				grid1.par = pGrid->pos;
				if( !grid1.bInserted )
				{
					m_q.Insert( &grid1 );
					grid1.bInserted = true;
				}
				else
					m_q.Modify( &grid1 );

				if( grid1.nType != pGrid->nType )
					break;
			}

		}
	}

	if( bReset || !m_q.Size() )
	{
		if( m_pTarget && !bReset )
			m_trigger.Trigger( 1, NULL );
		Reset();
		m_curSrcGrid = GetGridByPos( pChar->globalTransform.GetPosition() );
		if( m_pTarget )
			m_curTargetGrid = GetGridByPos( m_pTarget->globalTransform.GetPosition() );

		if( m_curSrcGrid.x >= 0 && m_curSrcGrid.y >= 0 && m_curSrcGrid.x < mapRect.width && m_curSrcGrid.y < mapRect.height )
		{
			auto& grid = CacheGridData( m_curSrcGrid, pProvider );
			grid.fDist = 0;
			m_q.Insert( &grid );
		}
	}
}

TVector2<int32> CNavigationUnit::GetGridByPos( const CVector2 & pos )
{
	TVector2<int32> result;
	result.x = floor( pos.x / m_gridSize.x ) - m_mapRect.x;
	result.y = floor( pos.y / m_gridSize.y ) - m_mapRect.y;
	return result;
}

CRectangle CNavigationUnit::GridToRect( const TVector2<int32>& grid )
{
	return CRectangle( ( grid.x + m_mapRect.x ) * m_gridSize.x, ( grid.y + m_mapRect.y ) * m_gridSize.y, m_gridSize.x, m_gridSize.y );
}

void CNavigationUnit::BuildPath( SGridData * pGridData, CCharacter* pCharacter )
{
	ClearPath();

	while( pGridData->par.x >= 0 )
	{
		m_curPath.push_back( pGridData->pos );
		pGridData = &GetGrid( pGridData->par );
	}
	if( !m_curPath.size() )
		return;

	bool hitTypeFilter[eEntityHitType_Count] = { true, false };
	if( pCharacter->GetStage()->SweepTest( pCharacter, pCharacter->globalTransform,
		GridToRect( m_curPath.back() ).GetCenter() - pCharacter->GetPosition(), hitTypeFilter ) )
	{
		m_curPath.push_back( GetGridByPos( pCharacter->GetPosition() ) );
	}
}

void CNavigationUnit::ClearPath()
{
	m_curPath.reserve( m_curPath.capacity() );
	m_curPath.clear();
}

CVector2 CNavigationUnit::FollowPath( CCharacter * pCharacter )
{
	if( !m_curPath.size() )
		return CVector2( 0, 0 );

	CRectangle targetRect = GridToRect( m_curPath.back() );
	CVector2 res = targetRect.GetCenter() - pCharacter->globalTransform.GetPosition();
	CRectangle charRect;
	pCharacter->Get_HitProxy()->CalcBound( pCharacter->globalTransform, charRect );
	if( targetRect.Contains( charRect ) )
		m_curPath.pop_back();
	res.Normalize();
	return res;
}

CNavigationUnit * CNavigationUnit::Alloc()
{
	return (CNavigationUnit*)TObjectPool<CNavigationUnit>::Inst().Alloc();
}

void CNavigationUnit::Free( CNavigationUnit * pUnit )
{
	TObjectPool<CNavigationUnit>::Inst().Free( pUnit );
}
