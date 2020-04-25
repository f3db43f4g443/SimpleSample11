#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "GlobalCfg.h"
#include "Common/Rand.h"
#include "MyGame.h"

void CMyLevel::OnAddedToStage()
{
	m_vecGrid.resize( m_nWidth * m_nHeight );
	for( int i = 0; i < m_vecGrid.size(); i++ )
	{
		m_vecGrid[i].bCanEnter = !m_arrGridData[i].bBlocked;
		m_vecGrid[i].nNextStage = m_arrGridData[i].nNextStage;
	}
}

void CMyLevel::OnRemovedFromStage()
{
	while( m_spawningPawns.Get_Pawn() )
		RemovePawn( m_spawningPawns.Get_Pawn() );
	for( int i = 0; i < m_vecPawnHits.size(); i++ )
		RemovePawn( m_vecPawnHits[i] );
	m_vecPawnHits.resize( 0 );
	while( m_pPawns )
		RemovePawn( m_pPawns );
}

bool CMyLevel::AddPawn( CPawn* pPawn, const TVector2<int32>& pos, int8 nDir, CPawn* pCreator, int32 nForm )
{
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	int32 nPawnWidth = pPawn->m_nWidth;
	int32 nPawnHeight = pPawn->m_nHeight;
	if( pPawn->m_arrForms.Size() )
	{
		nPawnWidth = pPawn->m_arrForms[nForm].nWidth;
		nPawnHeight = pPawn->m_arrForms[nForm].nHeight;
	}
	if( !pPawnHit )
	{
		for( int i = 0; i < nPawnWidth; i++ )
		{
			for( int j = 0; j < nPawnHeight; j++ )
			{
				auto pGrid = GetGrid( pos + TVector2<int32>( i, j ) );
				if( !pGrid || pGrid->pPawn || !pGrid->bCanEnter )
					return false;
			}
		}
	}

	if( pPawn->m_arrForms.Size() )
	{
		pPawn->m_nCurForm = nForm;
		pPawn->m_nWidth = nPawnWidth;
		pPawn->m_nHeight = nPawnHeight;
	}
	pPawn->m_pos = pPawn->m_moveTo = pos;
	pPawn->m_nCurDir = nDir;
	pPawn->m_pCreator = pCreator;
	if( !pPawnHit )
	{
		for( int i = 0; i < nPawnWidth; i++ )
		{
			for( int j = 0; j < nPawnHeight; j++ )
			{
				auto pGrid = GetGrid( pos + TVector2<int32>( i, j ) );
				pGrid->pPawn = pPawn;
			}
		}
		auto pPlayer = SafeCast<CPlayer>( pPawn );
		if( pPlayer )
			m_pPlayer = pPlayer;
		m_spawningPawns.Insert_Pawn( pPawn );
	}
	else
	{
		m_vecPawnHits.push_back( pPawnHit );
		pPawnHit->SetParentEntity( m_pPawnRoot );
		pPawnHit->Init();
	}
	return true;
}

void CMyLevel::RemovePawn( CPawn* pPawn )
{
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		if( pPawn == m_pPlayer )
			m_pPlayer = NULL;
		auto pGrid = GetGrid( pPawn->m_pos );
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
				ASSERT( pGrid->pPawn == pPawn );
				pGrid->pPawn = NULL;
				if( pPawn->m_moveTo != pPawn->m_pos )
				{
					auto pGrid1 = GetGrid( pPawn->m_moveTo + TVector2<int32>( i, j ) );
					pGrid1->pPawn = NULL;
				}
			}
		}
	}

	pPawn->m_pCreator = NULL;
	if( pPawn->GetParentEntity() )
		pPawn->SetParentEntity( NULL );
	if( !pPawnHit )
		pPawn->RemoveFrom_Pawn();
}

bool CMyLevel::PawnMoveTo( CPawn* pPawn, const TVector2<int32>& ofs )
{
	ASSERT( pPawn->m_moveTo == pPawn->m_pos && ( ofs.x || ofs.y ) );
	auto moveTo = pPawn->m_pos + ofs;

	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( moveTo + TVector2<int32>( i, j ) );
				if( !pGrid || pGrid->pPawn && pGrid->pPawn != pPawn || !pGrid->bCanEnter )
					return false;
				if( pPawn == m_pPlayer && pGrid->nNextStage > 0 && !m_bComplete )
					return false;
			}
		}
	}

	pPawn->m_moveTo = moveTo;
	if( !pPawnHit )
	{
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( moveTo + TVector2<int32>( i, j ) );
				pGrid->pPawn = pPawn;
			}
		}
	}
	return true;
}

void CMyLevel::PawnMoveEnd( CPawn* pPawn )
{
	ASSERT( pPawn->m_moveTo != pPawn->m_pos );

	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
				ASSERT( pGrid->pPawn == pPawn );
				pGrid->pPawn = NULL;
			}
		}
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn->m_moveTo + TVector2<int32>( i, j ) );
				pGrid->pPawn = pPawn;
			}
		}
	}

	pPawn->m_pos = pPawn->m_moveTo;
}

void CMyLevel::PawnMoveBreak( CPawn* pPawn )
{
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		if( pPawn->m_moveTo != pPawn->m_pos )
		{
			for( int i = 0; i < pPawn->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( pPawn->m_moveTo + TVector2<int32>( i, j ) );
					ASSERT( pGrid->pPawn == pPawn );
					pGrid->pPawn = NULL;
				}
			}
			for( int i = 0; i < pPawn->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
					pGrid->pPawn = pPawn;
				}
			}
			pPawn->m_moveTo = pPawn->m_pos;
		}
	}
}

void CMyLevel::PawnDeath( CPawn* pPawn )
{
	RemovePawn( pPawn );
	if( pPawn->m_bIsEnemy )
		m_bFailed = true;
}

bool CMyLevel::PawnTransform( CPawn* pPawn, int32 nForm, const TVector2<int32>& ofs )
{
	if( pPawn->m_moveTo != pPawn->m_pos )
		PawnMoveEnd( pPawn );
	auto& newForm = pPawn->m_arrForms[nForm];
	TRectangle<int32> r0( 0, 0, pPawn->m_nWidth, pPawn->m_nHeight );
	TRectangle<int32> r1( ofs.x, ofs.y, newForm.nWidth, newForm.nHeight );
	if( pPawn->m_nCurDir )
		r1 = TRectangle<int32>( r0.x + r0.GetRight() - r1.GetRight(), r1.y, r1.width, r1.height );
	r0 = r0.Offset( pPawn->m_moveTo );
	r1 = r1.Offset( pPawn->m_moveTo );

	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		for( int i = r1.x; i < r1.GetRight(); i++ )
		{
			for( int j = r1.y; j < r1.GetBottom(); j++ )
			{
				auto pGrid = GetGrid( TVector2<int32>( i, j ) );
				if( !pGrid || pGrid->pPawn && pGrid->pPawn != pPawn || !pGrid->bCanEnter )
					return false;
				if( pPawn == m_pPlayer && pGrid->nNextStage > 0 && !m_bComplete )
					return false;
			}
		}
	}

	pPawn->m_nCurForm = nForm;
	pPawn->m_nWidth = r1.width;
	pPawn->m_nHeight = r1.height;
	pPawn->m_pos = pPawn->m_moveTo = TVector2<int32>( r1.x, r1.y );
	if( !pPawnHit )
	{
		for( int i = r0.x; i < r0.GetRight(); i++ )
		{
			for( int j = r0.y; j < r0.GetBottom(); j++ )
			{
				auto pGrid = GetGrid( TVector2<int32>( i, j ) );
				ASSERT( pGrid->pPawn == pPawn );
				pGrid->pPawn = NULL;
			}
		}
		for( int i = r1.x; i < r1.GetRight(); i++ )
		{
			for( int j = r1.y; j < r1.GetBottom(); j++ )
			{
				auto pGrid = GetGrid( TVector2<int32>( i, j ) );
				pGrid->pPawn = pPawn;
			}
		}
	}
	return true;
}

void CMyLevel::Init()
{
	CReference<CPrefab> pBarrierPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( "barrier.pf" );
	for( int x = 0; x < m_nWidth; x++ )
	{
		for( int y = 0; y < m_nHeight; y++ )
		{
			auto& grid = m_arrGridData[x + y * m_nWidth];
			if( grid.nSpawn > 0 && grid.nSpawn <= m_arrSpawnPrefab.Size() )
			{
				CReference<CPawn> pPawn = SafeCast<CPawn>( m_arrSpawnPrefab[grid.nSpawn - 1]->GetRoot()->CreateInstance() );
				AddPawn( pPawn, TVector2<int32>( x, y ), 1 );
			}
			if( grid.nNextStage > 0 )
			{
				auto pBarrier = SafeCast<CPawn>( pBarrierPrefab->GetRoot()->CreateInstance() );
				AddPawn( pBarrier, TVector2<int32>( x, y ), 0 );
				pBarrier->ChangeState( 1 );
				m_vecBarriers.push_back( pBarrier );
			}
			if( grid.bBlocked == false && !( ( x + y ) & 1 ) && m_pTileDrawable )
			{
				auto pImage = m_pTileDrawable->CreateInstance();
				pImage->SetPosition( CVector2( x, y ) * LEVEL_GRID_SIZE );
				AddChildAfter( pImage, m_pPawnRoot );
			}
		}
	}
	/*CReference<CPrefab> pEnemyPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( "enemy.pf" );
	for( int i = 0; i < 5; i++ )
	{
		int32 y = SRand::Inst().Rand( 0, 2 ) + i * 2;
		int32 x = SRand::Inst().Rand( 5, 15 ) * 2;
		if( !!( y & 1 ) )
			x++;
		auto pPawn = SafeCast<CPawn>( pEnemyPrefab->GetRoot()->CreateInstance() );
		AddPawn( pPawn, TVector2<int32>( x, y ), 1 );
	}*/
}

void CMyLevel::Update()
{
	if( CGame::Inst().IsKeyDown( 'R' ) || CGame::Inst().IsKeyDown( 'r' ) )
	{
		GetStage()->GetWorld()->RestartStage();
		return;
	}
	if( m_bFailed )
	{
		if( !m_pTip )
		{
			static CReference<CDrawableGroup> pDrawable = CResourceManager::Inst()->CreateResource<CDrawableGroup>( "tips.mtl" );
			m_pTip = pDrawable->CreateInstance();
			m_pTip->SetPosition( m_camPos + CVector2( 0, 200 ) );
			AddChild( m_pTip );
		}
		return;
	}

	while( m_spawningPawns.Get_Pawn() )
	{
		auto p = m_spawningPawns.Get_Pawn();
		p->RemoveFrom_Pawn();
		p->SetParentEntity( m_pPawnRoot );
		p->Init();
		Insert_Pawn( p );
	}
	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		pPawn->Update();
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn );

	for( int i = 0; i < m_vecPawnHits.size(); i++ )
	{
		auto p = m_vecPawnHits[i];
		if( p->GetParentEntity() )
			p->Update();
	}
		
	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		pPawn->Update1();
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn );

	for( int i = 0; i < m_vecPawnHits.size(); i++ )
	{
		auto p = m_vecPawnHits[i];
		if( p->GetParentEntity() )
			p->Update1();
	}
	int i1 = 0;
	for( int i = 0; i < m_vecPawnHits.size(); i++ )
	{
		auto p = m_vecPawnHits[i];
		if( p->GetParentEntity() )
		{
			if( i1 < i )
				m_vecPawnHits[i1] = m_vecPawnHits[i];
			i1++;
		}
	}
	m_vecPawnHits.resize( i1 );

	if( !m_bComplete )
	{
		m_bComplete = true;
		for( auto pPawn = Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
		{
			if( pPawn->m_bIsEnemy && pPawn->m_nHp > 0 )
			{
				m_bComplete = false;
				break;
			}
		}
		if( m_bComplete )
		{
			for( auto pPawn = m_spawningPawns.Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
			{
				if( pPawn->m_bIsEnemy && pPawn->m_nHp > 0 )
				{
					m_bComplete = false;
					break;
				}
			}
		}

		if( m_bComplete )
		{
			for( CPawn* pBarrier : m_vecBarriers )
				pBarrier->ChangeState( 2 );
		}
	}
	//complete eft

	m_pPawnRoot->SortChildrenRenderOrder( [] ( CRenderObject2D* a, CRenderObject2D* b ) {
		auto pPawn1 = static_cast<CPawn*>( a );
		auto pPawn2 = static_cast<CPawn*>( b );
		auto n1 = pPawn1->m_pos.y + pPawn1->m_moveTo.y;
		auto n2 = pPawn2->m_pos.y + pPawn2->m_moveTo.y;
		if( n1 < n2 )
			return true;
		if( n2 < n1 )
			return false;
		return pPawn1->m_nRenderOrder > pPawn2->m_nRenderOrder;
	} );
	if( m_pPlayer && !m_bFailed )
	{
		if( m_pPlayer->m_nHp <= 0 )
			m_bFailed = true;
		else if( m_bComplete && m_pPlayer->m_pos == m_pPlayer->m_moveTo )
		{
			for( int i = 0; i < m_pPlayer->m_nWidth; i++ )
			{
				for( int j = 0; j < m_pPlayer->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( m_pPlayer->m_pos + TVector2<int32>( i, j ) );
					if( pGrid->nNextStage > 0 && pGrid->nNextStage <= m_arrNextStage.Size() )
					{
						auto& nxt = m_arrNextStage[pGrid->nNextStage - 1];
						GetStage()->GetWorld()->EnterStage( nxt.pNxtStage, m_pPlayer->m_pos - TVector2<int32>( nxt.nOfsX, nxt.nOfsY ), m_pPlayer->m_nCurDir );
						return;
					}
				}
			}
		}
	}
}
