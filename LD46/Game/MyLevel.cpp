#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "GlobalCfg.h"
#include "Common/Rand.h"
#include "MyGame.h"
#include "Entities/UtilEntities.h"
#include "Common/Algorithm.h"

void SLevelEnvDesc::OverlayToGrid( SLevelEnvGridDesc& grid, int32 nDist )
{
	for( int i = 0; i < arrGridDesc.Size(); i++ )
	{
		auto& desc = arrGridDesc[i];
		if( desc.nDist >= nDist )
		{
			if( i == 0 )
				grid.Overlay( desc );
			else
			{
				auto desc1 = arrGridDesc[i - 1];
				float t = ( nDist - desc1.nDist ) * 1.0f / ( desc.nDist - desc1.nDist );
				desc1.Interpolate( desc, t );
				grid.Overlay( desc1 );
			}
			return;
		}
	}
}

void CLevelEnvEffect::Init()
{
	SetRenderObject( NULL );
	m_elems.resize( 0 );
	vector<SLevelEnvGridDesc> vecGridDescs;
	vecGridDescs.resize( m_nWidth * m_nHeight );
	vector<int32> vecDist;
	vecDist.resize( m_nWidth * m_nHeight );
	vector<int32> vecJamDist;
	vecJamDist.resize( m_nWidth * m_nHeight );
	TVector2<int32> ofs[] = { { 1, 0 }, { -1, 0 }, { 2, 0 }, { -2, 0 }, { 0, 1 }, { 0, -1 }, { -1, 1 }, { -1, -1 }, { 1, 1 }, { 1, -1 } };
	vector<TVector2<int32> > q;
	GenDistField( &m_arrEnvMap[0], m_nWidth, m_nHeight, -1, vecJamDist, q, ofs, ELEM_COUNT( ofs ), false, true );
	q.resize( 0 );

	for( int k = 0; k < m_arrEnvDescs.Size(); k++ )
	{
		auto& arrGridDescs = m_arrEnvDescs[k];
		GenDistField( &m_arrEnvMap[0], m_nWidth, m_nHeight, k + 1, vecDist, q, ofs, ELEM_COUNT( ofs ), false, true );
		q.resize( 0 );

		for( int x = 0; x < m_nWidth; x++ )
		{
			for( int y = 0; y < m_nHeight; y++ )
			{
				auto nDist = vecDist[x + y * m_nWidth];
				if( nDist < 0 )
					continue;
				if( arrGridDescs.arrJamStrength.Size() )
				{
					auto nJamDist = vecJamDist[x + y * m_nWidth];
					if( nJamDist >= 0 && nJamDist < arrGridDescs.arrJamStrength.Size() )
						nDist += arrGridDescs.arrJamStrength[nJamDist];
				}
				if( nDist <= arrGridDescs.GetMaxDist() )
					arrGridDescs.OverlayToGrid( vecGridDescs[x + y * m_nWidth], nDist );
			}
		}
	}
	for( int x = 0; x < m_nWidth; x++ )
	{
		for( int y = 0; y < m_nHeight; y++ )
		{
			auto& gridDesc = vecGridDescs[x + y * m_nWidth];
			if( gridDesc.fBlendWeight )
			{
				gridDesc.Normalize();
				m_elems.resize( m_elems.size() + 1 );
				auto& elem = m_elems.back();
				elem.gridDesc = gridDesc;
				elem.fPhase = gridDesc.fRandomPhaseOfs * SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) + CVector2( x, y ).Dot( gridDesc.gridPhaseOfs );
				elem.fPhase -= floor( elem.fPhase );
				elem.origRect = CRectangle( x * m_gridSize.x + m_gridOfs.x, y * m_gridSize.y + m_gridOfs.y, m_gridSize.x, m_gridSize.y );
				elem.elem.texRect = CRectangle( 0, 0, 1, 1 );
				if( x == 0 )
					elem.origRect.SetLeft( -4096 );
				if( y == 0 )
					elem.origRect.SetTop( -4096 );
				if( x == m_nWidth - 1 )
					elem.origRect.SetRight( 4096 );
				if( y == m_nHeight - 1 )
					elem.origRect.SetBottom( 4096 );
			}
		}
	}
	for( auto& elem : m_elems )
	{
		elem.elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.elem.pInstData = elem.param;
	}
	SetLocalBound( CRectangle( -4096, -4096, 8192, 8192 ) );
}

void CLevelEnvEffect::UpdateRendered( double dTime )
{
	if( !GetResource() )
		return;
	if( m_fFade <= 0 )
		return;
	for( auto& elem : m_elems )
	{
		elem.fPhase += dTime / elem.gridDesc.fPeriod;
		elem.fPhase -= floor( elem.fPhase );
		float k = abs( elem.fPhase - 0.5f ) * 4 - 1;
		CVector2 ofs = elem.gridDesc.sizeDynamic * k;
		ofs = CVector2( floor( ofs.x * 0.5f + 0.5f ), floor( ofs.y * 0.5f + 0.5f ) ) * 2;
		elem.elem.rect = elem.origRect + elem.origRect.Offset( ofs );
		elem.param[0] = elem.gridDesc.param[0] + elem.gridDesc.paramDynamic[0] * k;
		elem.param[1] = elem.gridDesc.param[1] + elem.gridDesc.paramDynamic[1] * k;
		elem.param[0] = CVector4( 1, 1, 1, 0 ) + ( elem.param[0] - CVector4( 1, 1, 1, 0 ) ) * m_fFade;
		elem.param[1] = CVector4( 0, 0, 0, 0 ) + ( elem.param[1] - CVector4( 0, 0, 0, 0 ) ) * m_fFade;
		elem.param[0].w = floor( elem.param[0].w * 0.5f + 0.5f ) * 2;
		elem.param[1].w = floor( elem.param[1].w * 0.5f + 0.5f ) * 2;
	}
}

void CLevelEnvEffect::Render( CRenderContext2D & context )
{
	if( !GetResource() )
		return;
	if( m_fFade <= 0 )
		return;
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_elems )
	{
		elem.elem.worldMat = globalTransform;
		elem.elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem.elem, nGroup );
	}
	context.AddElementDummy( &m_dummyElem, nGroup );
}

void CLevelEnvEffect::Resize( const TRectangle<int32>& rect )
{
	vector<int8> vecOrigData;
	vecOrigData.resize( m_nWidth * m_nHeight );
	if( vecOrigData.size() )
		memcpy( &vecOrigData[0], &m_arrEnvMap[0], vecOrigData.size() );

	m_arrEnvMap.Resize( rect.width * rect.height );
	for( int i = 0; i < rect.width; i++ )
	{
		for( int j = 0; j < rect.height; j++ )
		{
			auto x = Max( 0, Min( m_nWidth - 1, i + rect.x ) );
			auto y = Max( 0, Min( m_nHeight - 1, j + rect.y ) );
			m_arrEnvMap[i + j * rect.width] = vecOrigData[x + y * m_nWidth];
		}
	}
	m_nWidth = rect.width;
	m_nHeight = rect.height;
	m_gridOfs = m_gridOfs + CVector2( rect.x, rect.y ) * m_gridSize;
}

void CLevelEnvEffect::Clear()
{
	if( m_arrEnvMap.Size() )
		memset( &m_arrEnvMap[0], 0, m_arrEnvMap.Size() );
}

void CLevelEnvEffect::Fill( int8 n, const TVector2<int32>& p )
{
	FloodFill( &m_arrEnvMap[0], m_nWidth, m_nHeight, p.x, p.y, n );
}


void CLevelIndicatorLayer::OnAddedToStage()
{
	SetRenderObject( NULL );
}

void CLevelIndicatorLayer::Update( CMyLevel* pLevel )
{
	auto& globalData = CGlobalCfg::Inst().lvIndicatorData;
	auto& pawnParamData = globalData.vecPawnParams[m_nTick % globalData.vecPawnParams.size()];
	auto& useParamData = globalData.vecUseParams[m_nTick % globalData.vecUseParams.size()];
	auto& mountParamData = globalData.vecMountParams[m_nTick % globalData.vecMountParams.size()];
	int32 nNextLevelParamCount[2];
	for( int k = 0; k < 2; k++ )
	{
		nNextLevelParamCount[k] = k ? globalData.nNextLevelParamCount : globalData.nNextLevelBlockedParamCount;
		m_vecNxtStageParam[k].resize( 2 * nNextLevelParamCount[k] );
		m_vecNxtStageOfs[k].resize( nNextLevelParamCount[k] );
		auto& nextLevelParamMin = k ? globalData.NextLevelParamMin : globalData.NextLevelBlockedParamMin;
		auto& nextLevelParamMax = k ? globalData.NextLevelParamMax : globalData.NextLevelBlockedParamMax;
		for( int i = 0; i < nNextLevelParamCount[k]; i++ )
		{
			float t = SRand::Inst().Rand( 0.0f, 1.0f );
			m_vecNxtStageParam[k][i * 2] = nextLevelParamMin.params[0] + ( nextLevelParamMax.params[0] - nextLevelParamMin.params[0] ) * t;
			m_vecNxtStageParam[k][i * 2 + 1] = nextLevelParamMin.params[1] + ( nextLevelParamMax.params[1] - nextLevelParamMin.params[1] ) * t;
			m_vecNxtStageOfs[k][i] = nextLevelParamMin.ofs + ( nextLevelParamMax.ofs - nextLevelParamMin.ofs ) * t;
			m_vecNxtStageOfs[k][i] = CVector2( floor( m_vecNxtStageOfs[k][i].x * 0.5f + 0.5f ), floor( m_vecNxtStageOfs[k][i].y * 0.5f + 0.5f ) ) * 2;
		}
	}

	m_elems.resize( 0 );
	m_vecBlockedParams.resize( 0 );
	auto levelSize = pLevel->GetSize();
	m_localBound = CRectangle( 0, 0, levelSize.x * LEVEL_GRID_SIZE_X, levelSize.y * LEVEL_GRID_SIZE_Y );
	SetBoundDirty();

	struct SBlocked
	{
		TVector2<int32> p;
		TVector2<int32> ofs;
		int8 n;
	};
	struct SHit
	{
		TVector2<int32> p;
		int8 n;
	};
	struct SMount
	{
		TVector2<int32> p;
		int8 nDir;
	};
	static vector<SBlocked> vecBlocked;
	static vector<SHit> vecHit;
	static vector<SHit> vecHitBlocked;
	static vector<SHit> vecHitBash;
	static vector<SHit> vecMiss;
	static vector<SHit> vecMissBash;
	static vector<TVector2<int32> > vecNxtStage[2];
	static vector<TVector2<int32> > vecPawn;
	static vector<TVector2<int32> > vecUse;
	static vector<SMount> vecMount;

	for( int i = 0; i < levelSize.x; i++ )
	{
		for( int j = 0; j < levelSize.y; j++ )
		{
			auto pGrid = pLevel->GetGrid( TVector2<int32>( i, j ) );
			if( pGrid )
			{
				if( pGrid->nBlockEft )
				{
					SBlocked blocked = { { i, j }, pGrid->blockOfs, pGrid->nBlockEft };
					vecBlocked.push_back( blocked );
					pGrid->nBlockEft--;
				}
				if( pGrid->nNextStage )
				{
					if( pLevel->IsGridBlockedExit( pGrid ) )
						vecNxtStage[0].push_back( TVector2<int32>( i, j ) );
					else
						vecNxtStage[1].push_back( TVector2<int32>( i, j ) );
				}
				if( !pLevel->IsScenario() )
				{
					if( pGrid->nHitEft )
					{
						SHit hit = { { i, j }, pGrid->nHitEft };
						vecHit.push_back( hit );
						pGrid->nHitEft--;
					}
					if( pGrid->nMissEft )
					{
						SHit hit = { { i, j }, pGrid->nMissEft };
						vecMiss.push_back( hit );
						pGrid->nMissEft--;
					}
					if( pGrid->nHitBlockedEft )
					{
						SHit hit = { { i, j }, pGrid->nHitBlockedEft };
						vecHitBlocked.push_back( hit );
						pGrid->nHitBlockedEft--;
					}
					if( pGrid->nHitBashEft )
					{
						SHit hit = { { i, j }, pGrid->nHitBashEft };
						vecHitBash.push_back( hit );
						pGrid->nHitBashEft--;
					}
					if( pGrid->nMissBashEft )
					{
						SHit hit = { { i, j }, pGrid->nMissBashEft };
						vecMissBash.push_back( hit );
						pGrid->nMissBashEft--;
					}
					if( pGrid->pPawn )
					{
						if( pGrid->pPawn->CanBeHit() )
							vecPawn.push_back( TVector2<int32>( i, j ) );
					}
					for( auto pMount = pGrid->Get_Mount(); pMount; pMount = pMount->NextMount() )
					{
						if( !pMount->IsEnabled() )
							continue;
						auto pPawn = pMount->GetPawn();
						auto nDir = pMount->GetEnterDir();
						if( nDir <= 1 && pPawn->GetCurDir() )
							nDir = 1 - nDir;
						SMount mount = { TVector2<int32>( i, j ), nDir };
						vecMount.push_back( mount );
					}
				}
			}
		}
	}

	for( int l = 0; l < 2; l++ )
	{
		auto& hits = l == 0 ? vecHitBash : vecHit;
		for( int k = 2; k >= 0; k-- )
		{
			for( auto& hit : hits )
			{
				int32 nParam = globalData.vecHitParams.size() - Max<int32>( 1, hit.n - k );
				auto& paramData = globalData.vecHitParams[nParam];

				m_elems.resize( m_elems.size() + 1 );
				auto& elem = m_elems.back();
				int32 nMaxOfsX = nParam * LEVEL_GRID_SIZE_X / globalData.vecHitParams.size() / 4;
				int32 nMaxOfsY = nParam * LEVEL_GRID_SIZE_Y / globalData.vecHitParams.size() / 4;
				float f = nParam * 1.0f / globalData.vecHitParams.size();
				f = 1 - f * f * 0.75f;
				int32 nSizeX = f * LEVEL_GRID_SIZE_X / 2;
				int32 nSizeY = f * LEVEL_GRID_SIZE_Y / 2;
				int32 nOfsX = SRand::Inst<eRand_Render>().Rand<int32>( -nMaxOfsX, nMaxOfsX + 1 + LEVEL_GRID_SIZE_X / 2 - nSizeX ) * 2;
				int32 nOfsY = SRand::Inst<eRand_Render>().Rand<int32>( -nMaxOfsX, nMaxOfsX + 1 + LEVEL_GRID_SIZE_Y / 2 - nSizeY ) * 2;
				nSizeX *= 2;
				nSizeY *= 2;
				elem.rect = CRectangle( hit.p.x * LEVEL_GRID_SIZE_X, hit.p.y * LEVEL_GRID_SIZE_Y, nSizeX, nSizeY )
					.Offset( paramData.ofs + CVector2( nOfsX, nOfsY ) );
				if( l == 0 )
				{
					auto center = CVector2( hit.p.x + 0.5f, hit.p.y + 0.5f ) * LEVEL_GRID_SIZE;
					elem.rect = ( elem.rect.Offset( center * -1 ) * 2 ).Offset( center );
				}
				elem.texRect = CRectangle( 0, 0, 1, 1 );
				elem.nInstDataSize = sizeof( CVector4 ) * 2;
				elem.pInstData = paramData.params;
			}
		}
	}
	for( auto& hit : vecHitBlocked )
	{
		auto& paramData = globalData.vecHitBlockedParams[globalData.vecHitBlockedParams.size() - hit.n];
		AddElem( hit.p.x, hit.p.y, paramData.ofs, paramData.params );
	}
	for( auto& hit : vecMiss )
	{
		auto& paramData = globalData.vecMissParams[globalData.vecMissParams.size() - hit.n];
		AddElem( hit.p.x, hit.p.y, paramData.ofs, paramData.params );
	}
	for( auto& hit : vecMissBash )
	{
		int32 n = globalData.vecMissParams.size() - hit.n;
		auto& paramData = globalData.vecMissParams[n];
		auto& rect = AddElem( hit.p.x, hit.p.y, paramData.ofs, paramData.params ).rect;
		rect = CRectangle( rect.x - n * 2, rect.y - n * 2, rect.width + n * 4, rect.height + n * 4 );
	}
	auto pPlayer = pLevel->GetPlayer();
	if( !pPlayer || !pPlayer->IsMounting() )
	{
		CVector2 dirs[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for( auto& mount : vecMount )
		{
			auto ofs = dirs[mount.nDir] * mountParamData.ofs.x;
			auto l = mountParamData.ofs.y;
			CRectangle mountBaseRect[] = { { LEVEL_GRID_SIZE_X * 2 - 6, ( LEVEL_GRID_SIZE_Y - l ) / 2, 6, l },
			{ 0, ( LEVEL_GRID_SIZE_Y - l ) / 2, 6, l },
			{ LEVEL_GRID_SIZE_X - l / 2, LEVEL_GRID_SIZE_Y - 6, l, 6 },
			{ LEVEL_GRID_SIZE_X - l / 2, 0, l, 6 } };
			m_elems.resize( m_elems.size() + 1 );
			auto& elem = m_elems.back();
			elem.rect = mountBaseRect[mount.nDir].Offset( ofs + CVector2( mount.p.x, mount.p.y ) * LEVEL_GRID_SIZE );
			elem.texRect = CRectangle( 0, 0, 1, 1 );
			elem.nInstDataSize = sizeof( CVector4 ) * 2;
			elem.pInstData = &mountParamData.params;
		}
		pLevel->GetAllUseableGrid( vecUse );
	}
	for( auto& p : vecUse )
	{
		auto ofs = useParamData.ofs;
		CRectangle useRect( ( LEVEL_GRID_SIZE_X - ofs.x ) / 2, ( LEVEL_GRID_SIZE_Y - ofs.y ) / 2, ofs.x, ofs.y );
		m_elems.resize( m_elems.size() + 1 );
		auto& elem = m_elems.back();
		elem.rect = useRect.Offset( CVector2( p.x, p.y ) * LEVEL_GRID_SIZE );
		elem.texRect = CRectangle( 0, 0, 1, 1 );
		elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.pInstData = &useParamData.params;
	}
	m_vecBlockedParams.resize( vecBlocked.size() * 2 );
	for( int i = 0; i < vecBlocked.size(); i++ )
	{
		auto& item = vecBlocked[i];
		auto pParams = &m_vecBlockedParams[i * 2];
		auto& paramData = globalData.vecBlockedParams1[globalData.vecBlockedParams1.size() - item.n];
		pParams[0] = paramData.params[0];
		pParams[1] = paramData.params[1];
		CVector2 ofs( pParams[0].w, pParams[1].w );
		pParams[0].w = ofs.x * item.ofs.x - ofs.y * item.ofs.y;
		pParams[1].w = ofs.x * item.ofs.y + ofs.y * item.ofs.x;
		ofs = paramData.ofs;
		ofs = CVector2( ofs.x * item.ofs.x - ofs.y * item.ofs.y, ofs.x * item.ofs.y + ofs.y * item.ofs.x );
		AddElem( item.p.x, item.p.y, ofs, pParams );
	}
	for( int i = 0; i < vecBlocked.size(); i++ )
	{
		auto& item = vecBlocked[i];
		auto& paramData = globalData.vecBlockedParams[globalData.vecBlockedParams.size() - item.n];
		auto ofs = paramData.ofs;
		ofs = CVector2( ofs.x * item.ofs.x - ofs.y * item.ofs.y, ofs.x * item.ofs.y + ofs.y * item.ofs.x );
		AddElem( item.p.x, item.p.y, ofs, paramData.params );
	}
	for( int k = 0; k < 2; k++ )
	{
		for( int i = nNextLevelParamCount[k] - 1; i >= 0; i-- )
		{
			for( auto& p : vecNxtStage[k] )
				AddElem( p.x, p.y, m_vecNxtStageOfs[k][i], &m_vecNxtStageParam[k][i * 2] );
		}
	}
	for( auto& p : vecPawn )
		AddElem( p.x, p.y, pawnParamData.ofs, pawnParamData.params );

	vecBlocked.resize( 0 );
	vecHit.resize( 0 );
	vecHitBlocked.resize( 0 );
	vecHitBash.resize( 0 );
	vecMiss.resize( 0 );
	vecMissBash.resize( 0 );
	for( int k = 0; k < 2; k++ )
		vecNxtStage[k].resize( 0 );
	vecPawn.resize( 0 );
	vecUse.resize( 0 );
	vecMount.resize( 0 );
	m_nTick++;
}

void CLevelIndicatorLayer::Render( CRenderContext2D& context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_elems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

CElement2D& CLevelIndicatorLayer::AddElem( int32 x, int32 y, const CVector2& ofs, void* pParams )
{
	m_elems.resize( m_elems.size() + 1 );
	auto& elem = m_elems.back();
	elem.rect = CRectangle( x * LEVEL_GRID_SIZE_X, y * LEVEL_GRID_SIZE_Y, LEVEL_GRID_SIZE_X, LEVEL_GRID_SIZE_Y ).Offset( ofs );
	elem.texRect = CRectangle( 0, 0, 1, 1 );
	elem.nInstDataSize = sizeof( CVector4 ) * 2;
	elem.pInstData = pParams;
	return elem;
}


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
	if( m_strDestroyScript.length() )
		CLuaMgr::Inst().Run( m_strDestroyScript );
	for( int i = m_vecScripts.size() - 1; i >= 0; i-- )
		m_vecScripts[i]->OnDestroy( this );
	while( m_spawningPawns.Get_Pawn() )
		RemovePawn( m_spawningPawns.Get_Pawn() );
	for( int i = 0; i < m_vecPawnHits.size(); i++ )
		RemovePawn( m_vecPawnHits[i] );
	m_vecPawnHits.resize( 0 );
	while( m_pPawns )
		RemovePawn( m_pPawns );
}

void CMyLevel::OnPreview()
{
	for( auto p0 = Get_TransformChild(); p0; p0 == p0->NextTransformChild() )
	{
		if( p0 == m_pPawnRoot || SafeCast<CPawnLayer>( p0 ) )
		{
			for( auto p = p0->Get_TransformChild(); p; p = p->NextTransformChild() )
			{
				auto pSpawnHelper = SafeCast<CLevelSpawnHelper>( p );
				if( pSpawnHelper )
				{
					pSpawnHelper->OnPreview();
					continue;
				}
				auto pPawn = SafeCast<CPawn>( p );
				if( pPawn )
				{
					pPawn->m_pos = pPawn->m_moveTo = TVector2<int32>( floor( pPawn->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pPawn->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
					pPawn->m_nCurDir = pPawn->m_nInitDir;
					pPawn->OnPreview();
				}
			}
		}
	}
	InitTiles();
}

void CMyLevel::Begin()
{
	FlushSpawn();
	m_pIndicatorLayer = SafeCast<CLevelIndicatorLayer>( CGlobalCfg::Inst().lvIndicatorData.pIndicatorPrefab->GetRoot()->CreateInstance() );
	m_pIndicatorLayer->SetParentBeforeEntity( m_pPawnRoot );
	m_bBegin = true;
	m_beginTrigger.Trigger( 0, this );
	m_beginTrigger.Clear();
	if( m_strBeginScript.length() )
		CLuaMgr::Inst().Run( m_strBeginScript );
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnBegin( this );
	GetStage()->GetMasterLevel()->GetMainUI()->OnLevelBegin();
}

void CMyLevel::End()
{
	GetStage()->GetMasterLevel()->GetMainUI()->ShowFailEft( false );
	GetStage()->GetMasterLevel()->GetMainUI()->ShowFreezeEft( 0 );
	m_beginTrigger.Clear();
	m_trigger.Clear();
	if( m_pIndicatorLayer )
	{
		m_pIndicatorLayer->SetParentEntity( NULL );
		m_pIndicatorLayer = NULL;
	}
}

void CMyLevel::Fail( int8 nFailType )
{
	if( nFailType == 1 )
	{
		auto p0 = TVector2<int32>( m_pPlayer->GetWidth(), m_pPlayer->GetHeight() ) + m_pPlayer->GetPos() + m_pPlayer->GetMoveTo();
		auto p = CVector2( p0.x, p0.y ) * LEVEL_GRID_SIZE * 0.5f + CVector2( 0, 32 );
		CVector2 ofs[] = { { 256, 256 }, { 256, -256 }, { -256, -256 }, { -256, 256 } };
		for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
		{
			auto pLightning = SafeCast<CLightningEffect>( CGlobalCfg::Inst().pFailLightningEffectPrefab->GetRoot()->CreateInstance() );
			pLightning->SetParentBeforeEntity( m_pPawnRoot );
			pLightning->SetPosition( p );
			auto p1 = TVector2<int32>( floor( ofs[i].x / 8 + 0.5f ), floor( ofs[i].y / 8 + 0.5f ) );
			pLightning->Set( p1, 0 );
		}
	}

	m_bFailed = true;
	GetStage()->GetMasterLevel()->GetMainUI()->ShowFailEft( true );
}

void CMyLevel::Freeze()
{
	m_bFreeze = true;
}

CPawn* CMyLevel::SpawnPawn( int32 n, int32 x, int32 y, int8 nDir, CPawn* pCreator, int32 nForm )
{
	if( n < 0 || n >= m_arrSpawnPrefab.Size() )
		return NULL;
	CReference<CPawn> pPawn = SafeCast<CPawn>( m_arrSpawnPrefab[n]->GetRoot()->CreateInstance() );
	if( !AddPawn( pPawn, TVector2<int32>( x, y ), nDir, pCreator, nForm ) )
		return NULL;
	pPawn->strCreatedFrom = m_arrSpawnPrefab[n]->GetName();
	return pPawn;
}

bool CMyLevel::AddPawn( CPawn* pPawn, const TVector2<int32>& pos, int8 nDir, CPawn* pCreator, int32 nForm )
{
	pPawn->bVisible = false;
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
		pPawn->bVisible = !pPawn->m_bForceHide;
		pPawnHit->SetParentEntity( m_pPawnRoot );
		pPawnHit->Init();
	}
	return true;
}

void CMyLevel::RemovePawn( CPawn* pPawn )
{
	HandlePawnMounts( pPawn, true );
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
			}
		}
		if( pPawn->m_moveTo != pPawn->m_pos )
		{
			for( int i = 0; i < pPawn->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn->m_nHeight; j++ )
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

	bool bOK = true;
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				bool bBlocked = false;
				auto pGrid = GetGrid( moveTo + TVector2<int32>( i, j ) );
				if( !pGrid || pGrid->pPawn && pGrid->pPawn != pPawn || !pGrid->bCanEnter )
					bBlocked = true;
				else if( pPawn == m_pPlayer && IsGridBlockedExit( pGrid ) )
					bBlocked = true;
				if( bBlocked )
				{
					bOK = false;
					auto pGrid1 = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
					pGrid1->nBlockEft = CGlobalCfg::Inst().lvIndicatorData.vecBlockedParams.size();
					pGrid1->blockOfs = ofs;
				}
			}
		}
	}
	if( !bOK )
		return false;

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
	if( pPawn->m_bIsEnemy )
	{
		CVector2 p[2] = { { 0, 0 }, { 0, 64 } };
		CEntity* pEntity[2] = { pPawn, m_pPlayer };
		for( int i = 0; i < 2; i++ )
		{
			auto pPawn = SafeCast<CPawn>( pEntity[i] );
			if( pPawn )
			{
				auto p0 = TVector2<int32>( pPawn->GetWidth(), pPawn->GetHeight() ) + pPawn->GetPos() + pPawn->GetMoveTo();
				p[i] = p[i] + CVector2( p0.x * 0.5f, p0.y * 0.5f ) * LEVEL_GRID_SIZE;
			}
			else
				p[i] = p[i] + pEntity[i]->globalTransform.GetPosition();
		}

		auto pLightning = SafeCast<CLightningEffect>( CGlobalCfg::Inst().pFailLightningEffectPrefab->GetRoot()->CreateInstance() );
		pLightning->SetParentBeforeEntity( m_pPawnRoot );
		pLightning->SetPosition( p[0] );
		auto ofs = p[1] - p[0];
		auto p1 = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
		pLightning->Set( p1, 0, 1.0f, 2.0f );

		Fail();
	}
	else
		RemovePawn( pPawn );
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
	auto d = ( r1 * 2 ).GetCenter() - ( r0 * 2 ).GetCenter();

	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		bool bOK = true;
		for( int i = r1.x; i < r1.GetRight(); i++ )
		{
			for( int j = r1.y; j < r1.GetBottom(); j++ )
			{
				bool bBlocked = false;
				auto pGrid = GetGrid( TVector2<int32>( i, j ) );
				if( !pGrid || pGrid->pPawn && pGrid->pPawn != pPawn || !pGrid->bCanEnter )
					bBlocked = true;
				else if( !pPawn->IsIgnoreBlockedExit() && IsGridBlockedExit( pGrid ) )
					bBlocked = true;
				if( bBlocked )
				{
					pGrid->nBlockEft = CGlobalCfg::Inst().lvIndicatorData.vecBlockedParams.size();
					pGrid->blockOfs = d;
					bOK = false;
				}
			}
		}
		if( !bOK )
			return false;
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

CPickUp* CMyLevel::FindPickUp( const TVector2<int32>& p, int32 w, int32 h )
{
	TRectangle<int32> a( p.x, p.y, w, h );
	for( CPawnHit* pHit : m_vecPawnHits )
	{
		auto pPickUp = SafeCast<CPickUp>( pHit );
		if( pPickUp && pPickUp->IsPickUpReady() )
		{
			TRectangle<int32> b( pPickUp->m_pos.x, pPickUp->m_pos.y, pPickUp->m_nWidth, pPickUp->m_nHeight );
			auto c = a * b;
			if( c.width > 0 && c.height > 0 )
				return pPickUp;
		}
	}
	return NULL;
}

CPlayerMount* CMyLevel::FindMount()
{
	auto p = m_pPlayer->GetPos();
	TRectangle<int32> a( p.x, p.y, m_pPlayer->m_nWidth, m_pPlayer->m_nHeight );
	for( int i = a.x; i < a.GetRight(); i++ )
	{
		for( int j = a.y; j < a.GetBottom(); j++ )
		{
			auto pGrid = GetGrid( TVector2<int32>( i, j ) );
			for( auto pMount = pGrid->Get_Mount(); pMount; pMount = pMount->NextMount() )
			{
				if( pMount->IsEnabled() && pMount->CheckMount( m_pPlayer ) )
					return pMount;
			}
		}
	}
	return NULL;
}

CPawn* CMyLevel::FindUseablePawn( const TVector2<int32>& p, int8 nDir, int32 w, int32 h )
{
	TRectangle<int32> a( p.x, p.y, w, h );
	for( CPawnHit* pHit : m_vecPawnHits )
	{
		auto pUsage = pHit->GetUsage();
		if( pUsage )
		{
			TRectangle<int32> b( pHit->m_pos.x, pHit->m_pos.y, pHit->m_nWidth, pHit->m_nHeight );
			auto c = a * b;
			if( c.width > 0 && c.height > 0 )
			{
				if( pUsage->CheckUse( m_pPlayer ) )
					return pHit;
			}
		}
	}
	int i = nDir == 1 ? p.x - 1 : p.x + w;
	for( int j = p.y; j < p.y + h; j++ )
	{
		auto pGrid = GetGrid( TVector2<int32>( i, j ) );
		if( pGrid && pGrid->pPawn )
		{
			auto pUsage = pGrid->pPawn->GetUsage();
			if( pUsage && pUsage->CheckUse( m_pPlayer ) )
				return pGrid->pPawn;
		}
	}
	return NULL;
}

CPawn* CMyLevel::GetPawnByName( const char* szName )
{
	for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->GetName() == szName )
			return pPawn;
	}
	for( auto& pPawn : m_vecPawnHits )
	{
		if( pPawn->GetName() == szName )
			return pPawn;
	}
	for( auto pPawn = m_spawningPawns.Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->GetName() == szName )
			return pPawn;
	}
	return NULL;
}

void CMyLevel::OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir )
{
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnPlayerChangeState( state, nStateSource, nDir );
}

void CMyLevel::OnPlayerAction( int32 nMatchLen, int8 nType )
{
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnPlayerAction( nMatchLen, nType );
}

TVector2<int32> CMyLevel::SimpleFindPath( const TVector2<int32>& begin, const TVector2<int32>& end, int32 nCheckFlag, vector<TVector2<int32> >* pVecPath )
{
	vector<int8> vec;
	vec.resize( m_nWidth * m_nHeight );
	for( int i = 0; i < m_nWidth; i++ )
	{
		for( int j = 0; j < m_nHeight; j++ )
		{
			auto pGrid = GetGrid( TVector2<int32>( i, j ) );
			if( !pGrid->bCanEnter )
				vec[i + j * m_nWidth] = 1;
			else if( !!( nCheckFlag & 1 ) && pGrid->pPawn && !pGrid->pPawn->nTempFlag )
				vec[i + j * m_nWidth] = 1;
			else if( !!( nCheckFlag & 2 ) && IsGridBlockedExit( pGrid ) )
				vec[i + j * m_nWidth] = 1;
		}
	}
	vector<TVector2<int32> > q;
	vector<int32> par;
	q.push_back( begin );
	par.push_back( -1 );
	vec[begin.x + begin.y * m_nWidth] = 1;
	TVector2<int32> ofs[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 }, { -1, 1 }, { -1, -1 } };
	bool bFind = false;
	for( int32 i = 0; i < q.size(); i++ )
	{
		auto p = q[i];
		SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );
		for( int j = 0; j < ELEM_COUNT( ofs ); j++ )
		{
			auto p1 = p + ofs[j];
			if( p1 == end )
			{
				q.push_back( p1 );
				par.push_back( i );
				bFind = true;
				break;
			}
			if( p1.x < 0 || p1.y < 0 || p1.x >= m_nWidth || p1.y >= m_nHeight )
				continue;
			if( vec[p1.x + p1.y * m_nWidth] )
				continue;
			vec[p1.x + p1.y * m_nWidth] = 1;
			q.push_back( p1 );
			par.push_back( i );
		}
		if( bFind )
			break;
	}

	TVector2<int32> result( -1, -1 );
	if( bFind )
	{
		for( int i = q.size() - 1; i > 0; i = par[i] )
		{
			result = q[i];
			if( pVecPath )
				pVecPath->push_back( result );
		}
	}
	return result;
}

void CMyLevel::BlockStage()
{
	m_bBlocked = true;
}

void CMyLevel::BlockExit( int32 n )
{
	if( n < 0 || n >= m_vecExitState.size() )
		return;
	m_vecExitState[n].bBlocked = true;
}

bool CMyLevel::IsExitBlocked( int32 n )
{
	if( n < 0 || n >= m_vecExitState.size() )
		return true;
	return m_vecExitState[n].bBlocked;
}

bool CMyLevel::IsGridBlockedExit( SGrid* pGrid, bool bIgnoreComplete )
{
	if( !pGrid->nNextStage )
		return false;
	if( !bIgnoreComplete )
	{
		if( !m_bComplete )
			return true;
	}
	auto nNextStage = pGrid->nNextStage - 1;
	if( nNextStage >= m_vecExitState.size() )
		return true;
	return m_vecExitState[nNextStage].bBlocked;
}

int32 CMyLevel::FindNextLevelIndex( const char* szLevelName )
{
	for( int i = 0; i < m_arrNextStage.Size(); i++ )
	{
		if( 0 == strcmp( m_arrNextStage[i].pNxtStage->GetName(), szLevelName ) )
			return i;
	}
	return -1;
}

void CMyLevel::Redirect( int32 n, int32 n1 )
{
	m_vecExitState[n].nRedirect = n1;
}

void CMyLevel::BeginScenario()
{
	m_bScenario = true;
}

void CMyLevel::EndScenario()
{
	m_bScenario = false;
}

void CMyLevel::GetAllUseableGrid( vector<TVector2<int32>>& result )
{
	for( CPawnHit* pHit : m_vecPawnHits )
	{
		auto pUsage = pHit->GetUsage();
		if( pUsage && pUsage->CheckUse( m_pPlayer ) )
		{
			for( int i = 0; i < pHit->m_nWidth; i++ )
			{
				for( int j = 0; j < pHit->m_nHeight; j++ )
				{
					result.push_back( TVector2<int32>( i, j ) + pHit->m_pos );
				}
			}
		}
		auto pPickUp = SafeCast<CPickUp>( pHit );
		if( pPickUp && pPickUp->IsPickUpReady() )
		{
			for( int i = 0; i < pHit->m_nWidth; i++ )
			{
				for( int j = 0; j < pHit->m_nHeight; j++ )
				{
					result.push_back( TVector2<int32>( i, j ) + pHit->m_pos );
				}
			}
		}
	}
}

void CMyLevel::Init()
{
	m_vecExitState.resize( m_arrNextStage.Size() );
	InitTiles();
	vector<CReference<CPawnLayer> > vecPawnLayers;
	for( auto p0 = Get_ChildEntity(); p0; p0 = p0->NextChildEntity() )
	{
		auto pPawnLayer = SafeCast<CPawnLayer>( p0 );
		if( pPawnLayer )
			vecPawnLayers.push_back( pPawnLayer );
	}
	for( CPawnLayer* p : vecPawnLayers )
	{
		if( !p->GetCondition().length() || GetStage()->GetMasterLevel()->EvaluateKeyInt( p->GetCondition() ) )
		{
			while( p->Get_ChildEntity() )
				p->Get_ChildEntity()->SetParentEntity( m_pPawnRoot );
		}
		p->SetParentEntity( NULL );
	}

	vector<CReference<CLevelSpawnHelper> > vecSpawnHelpers;
	vector<CReference<CPawn> > vecPawnsToSpawn;
	vector<CReference<CRenderObject2D> > vecOthers;

	for( auto pChild = m_pPawnRoot->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		auto pSpawnHelper = SafeCast<CLevelSpawnHelper>( pChild );
		if( pSpawnHelper )
		{
			vecSpawnHelpers.push_back( pSpawnHelper );
			continue;
		}
		auto pPawn = SafeCast<CPawn>( pChild );
		if( pPawn )
			vecPawnsToSpawn.push_back( pPawn );
		else
			vecOthers.push_back( pChild );
	}
	for( int i = vecOthers.size() - 1; i >= 0; i-- )
	{
		auto& p = vecOthers[i];
		auto pEntity = SafeCast<CEntity>( p.GetPtr() );
		if( pEntity )
			pEntity->SetParentAfterEntity( m_pPawnRoot );
		else
		{
			p->RemoveThis();
			AddChildAfter( p, m_pPawnRoot );
		}
	}
	for( auto& pPawn : vecPawnsToSpawn )
	{
		TVector2<int32> pos( floor( pPawn->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pPawn->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
		AddPawn( pPawn, pos, pPawn->m_nInitDir );
	}
	for( auto& pSpawnHelper : vecSpawnHelpers )
	{
		if( pSpawnHelper->m_strSpawnCondition.length() )
		{
			if( !GetStage()->GetMasterLevel()->EvaluateKeyInt( pSpawnHelper->m_strSpawnCondition ) )
			{
				pSpawnHelper->SetParentEntity( NULL );
				continue;
			}
		}
		CReference<CPawn> pPawn = SafeCast<CPawn>( pSpawnHelper->GetRenderObject() );
		TVector2<int32> pos( floor( pSpawnHelper->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pSpawnHelper->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
		auto nDir = pPawn->m_nInitDir;
		auto pos0 = pos;
		auto nDir0 = nDir;

		if( pSpawnHelper->m_nDataType == 1 )
		{
			auto& mapDeadPawn = GetStage()->GetMasterLevel()->GetCurLevelData().mapDataDeadPawn;
			auto itr = mapDeadPawn.find( pSpawnHelper->GetName().c_str() );
			if( itr != mapDeadPawn.end() )
			{
				if( !pPawn->IsValidStateIndex( pSpawnHelper->m_nDeathState ) )
				{
					pSpawnHelper->SetParentEntity( NULL );
					continue;
				}
				bool b = true;
				if( !SafeCast<CPawnHit>( pPawn.GetPtr() ) )
				{
					for( int i = 0; i < pPawn->m_nWidth && b; i++ )
					{
						for( int j = 0; j < pPawn->m_nHeight; j++ )
						{
							auto pGrid = GetGrid( itr->second.p + TVector2<int32>( i, j ) );
							if( !pGrid || pGrid->pPawn || !pGrid->bCanEnter || pGrid->nNextStage )
							{
								b = false;
								break;
							}
						}
					}
				}
				if( b )
				{
					pos = itr->second.p;
					nDir = itr->second.nDir;
				}
				pSpawnHelper->m_bSpawnDeath = true;
			}
		}
		else if( pSpawnHelper->m_strDeathKey.length() && GetStage()->GetMasterLevel()->EvaluateKeyInt( pSpawnHelper->m_strDeathKey ) )
		{
			if( !pPawn->IsValidStateIndex( pSpawnHelper->m_nDeathState ) )
			{
				pSpawnHelper->SetParentEntity( NULL );
				continue;
			}
			pSpawnHelper->m_bSpawnDeath = true;
		}

		pPawn->SetParentEntity( m_pPawnRoot );
		pPawn->SetPosition( pSpawnHelper->GetPosition() );
		if( !AddPawn( pPawn, pos, nDir ) && ( pos0 == pos || !AddPawn( pPawn, pos0, nDir0 ) ) )
		{
			pPawn->SetParentEntity( NULL );
			continue;
		}
		pSpawnHelper->ClearRenderObject();
		pPawn->SetName( pSpawnHelper->GetName() );
		pPawn->strCreatedFrom = pSpawnHelper->GetResource()->GetName();
		pPawn->m_pSpawnHelper = pSpawnHelper;
		pSpawnHelper->SetParentEntity( NULL );
	}

	FlushSpawn();
	InitScripts();
}

void CMyLevel::Update()
{
	if( m_bBegin && !m_bFreeze )
	{
		if( CGame::Inst().IsKeyDown( 'R' ) || CGame::Inst().IsKeyDown( 'r' ) )
		{
			GetStage()->GetMasterLevel()->JumpBack( 0 );
			return;
		}
		if( CGame::Inst().IsKeyDown( 'T' ) || CGame::Inst().IsKeyDown( 't' ) )
		{
			GetStage()->GetMasterLevel()->JumpBack( 1 );
			return;
		}
		if( CGame::Inst().IsKeyDown( 'V' ) || CGame::Inst().IsKeyDown( 'v' ) )
		{
			GetStage()->GetMasterLevel()->JumpBack( 2 );
			return;
		}
		if( m_bFailed )
			return;
	}
	FlushSpawn();
	for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
		HandlePawnMounts( pPawn, false );

	if( m_bBegin && !m_bFreeze )
	{
		m_pPlayer->Update();
		LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
		{
			if( pPawn != m_pPlayer )
				pPawn->Update();
		}
		LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )

		for( int i = 0; i < m_vecPawnHits.size(); i++ )
		{
			auto p = m_vecPawnHits[i];
			if( p->GetParentEntity() )
			{
				p->Update();
			}
		}
		m_bBlocked = false;
		for( int i = 0; i < m_vecExitState.size(); i++ )
			m_vecExitState[i].bBlocked = false;
		m_trigger.Trigger( 0, this );
		for( auto& pScript : m_vecScripts )
			pScript->OnUpdate( this );

		for( int i = 0; i < m_vecExitState.size(); i++ )
		{
			if( !m_vecExitState[i].bBlocked )
			{
				if( m_arrNextStage[i].strKeyOrig.length() &&
					!GetStage()->GetMasterLevel()->EvaluateKeyInt( m_arrNextStage[i].strKeyOrig ) )
				{
					m_vecExitState[i].bBlocked = true;
					continue;
				}
				auto nRedirect = m_vecExitState[i].nRedirect;
				if( nRedirect >= 0 && nRedirect < m_vecExitState.size() )
				{
					if( m_arrNextStage[nRedirect].strKeyRedirect.length() &&
						!GetStage()->GetMasterLevel()->EvaluateKeyInt( m_arrNextStage[nRedirect].strKeyRedirect ) )
					{
						m_vecExitState[i].bBlocked = true;
						continue;
					}
				}
			}
		}

		m_pPlayer->Update1();
		LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
		{
			if( pPawn != m_pPlayer )
				pPawn->Update1();
		}
		LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )

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
		m_trigger.Trigger( 1, this );
		for( auto& pScript : m_vecScripts )
			pScript->OnUpdate1( this );
	}
	if( m_bBegin )
		m_trigger.Trigger( 2, this );

	if( m_bBlocked )
		m_bComplete = false;
	else
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
	}

	if( m_pIndicatorLayer )
		m_pIndicatorLayer->Update( this );

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
			Fail();
		else if( m_bComplete && m_pPlayer->m_pos == m_pPlayer->m_moveTo )
		{
			for( int i = 0; i < m_pPlayer->m_nWidth; i++ )
			{
				for( int j = 0; j < m_pPlayer->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( m_pPlayer->m_pos + TVector2<int32>( i, j ) );
					auto nNextStage = pGrid->nNextStage - 1;
					if( nNextStage >= 0 && nNextStage < m_arrNextStage.Size() )
					{
						if( m_vecExitState[nNextStage].bBlocked )
							continue;
						auto nRedirect = m_vecExitState[nNextStage].nRedirect;
						if( nRedirect >= 0 && nRedirect < m_arrNextStage.Size() )
							nNextStage = nRedirect;
						auto& nxt = m_arrNextStage[nNextStage];
						if( m_bFullyEntered )
							GetStage()->GetMasterLevel()->TransferTo( nxt.pNxtStage, m_pPlayer->m_pos - TVector2<int32>( nxt.nOfsX, nxt.nOfsY ), m_pPlayer->m_nCurDir );
						return;
					}
				}
			}
			m_bFullyEntered = true;
		}
	}
}

void CMyLevel::RegisterBegin( CTrigger* pTrigger )
{
	if( m_bBegin )
	{
		pTrigger->Run( this );
		return;
	}
	m_beginTrigger.Register( 0, pTrigger );
}

void CMyLevel::HandlePawnMounts( CPawn* pPawn, bool bRemove )
{
	for( auto pChild = pPawn->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto pMount = SafeCast<CPlayerMount>( pChild );
		if( !pMount )
			continue;
		auto pGrid = GetGrid( pMount->m_levelPos );
		if( pGrid )
			pMount->RemoveFrom_Mount();
		if( !bRemove )
		{
			pMount->m_levelPos = pPawn->GetMoveTo() + TVector2<int32>( pPawn->m_nCurDir ? pPawn->m_nWidth - 2 - pMount->m_nOfsX : pMount->m_nOfsX, pMount->m_nOfsY );
			pGrid = GetGrid( pMount->m_levelPos );
			if( pGrid )
				pGrid->Insert_Mount( pMount );
		}
	}
}

void CMyLevel::FlushSpawn()
{
	while( m_spawningPawns.Get_Pawn() )
	{
		auto p = m_spawningPawns.Get_Pawn();
		p->RemoveFrom_Pawn();
		p->bVisible = !p->m_bForceHide;
		p->SetParentEntity( m_pPawnRoot );
		p->Init();
		Insert_Pawn( p );
	}
}

void CMyLevel::InitTiles()
{
	for( int x = 0; x < m_nWidth; x++ )
	{
		for( int y = 0; y < m_nHeight; y++ )
		{
			auto& grid = m_arrGridData[x + y * m_nWidth];
			if( !( ( x + y ) & 1 ) )
			{
				if( m_arrTileData.Size() )
				{
					auto& tileData = m_arrTileData[Min<int32>( grid.nTile, m_arrTileData.Size() - 1 )];
					if( tileData.pTileDrawable )
					{
						auto pImage = static_cast<CImage2D*>( tileData.pTileDrawable->CreateInstance() );
						pImage->SetPosition( CVector2( x, y ) * LEVEL_GRID_SIZE );
						auto rect = pImage->GetElem().rect;
						rect.width *= tileData.texRect.width;
						rect.height *= tileData.texRect.height;
						pImage->SetRect( rect );
						pImage->SetTexRect( tileData.texRect );
						AddChildAfter( pImage, m_pPawnRoot );
					}
				}
				else
				{
					if( grid.bBlocked == false && m_pTileDrawable )
					{
						auto pImage = m_pTileDrawable->CreateInstance();
						pImage->SetPosition( CVector2( x, y ) * LEVEL_GRID_SIZE );
						AddChildAfter( pImage, m_pPawnRoot );
					}
				}
			}
		}
	}
}

void CMyLevel::InitScripts()
{
	if( m_strInitScript.length() )
		CLuaMgr::Inst().Run( m_strInitScript );

	function<void( CEntity* )> Func;
	Func = [this, &Func] ( CEntity* pEntity ) {
		for( auto pChild = pEntity->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
		{
			Func( pChild );
			auto pScript = SafeCast<CLevelScript>( pChild );
			if( pScript )
			{
				pScript->OnInit( this );
				m_vecScripts.push_back( pScript );
			}
		}
	};
	Func( this );
}

void CCutScene::Begin()
{
	GetStage()->GetMasterLevel()->RunScenarioScriptText( m_strScript );
}

void CMainUI::OnAddedToStage()
{
	auto size = GetStage()->GetCamera().GetViewArea().GetSize();
	m_localBound = CRectangle( -size.x * 0.5f, -size.y * 0.5f, size.x, size.y );
	auto pRenderObject = static_cast<CImage2D*>( GetRenderObject() );
	m_origRect = pRenderObject->GetElem().rect;
	SetRenderObject( NULL );
	m_vecInputItems.resize( CGlobalCfg::Inst().MainUIData.vecPlayerInputParams.size() );
	for( int i = 0; i < 3; i++ )
		m_pFailTips[i]->bVisible = false;
}

void CMainUI::Reset( const CVector2& inputOrig, const CVector2& iconOrig, bool bClearAllEfts )
{
	m_playerInputOrig = inputOrig;
	m_iconOrig = iconOrig;
	for( int i = 0; i < ELEM_COUNT( m_pIcons ); i++ )
		m_pIcons[i]->bVisible = false;
	for( auto& item : m_vecInputItems )
	{
		item.vec.resize( 0 );
		item.vecElems.resize( 0 );
	}
	if( bClearAllEfts )
	{
		m_nRecordEftFrames = 0;
		EndScenario();
		m_nHeadTextTime = 0;
		auto pText = SafeCast<CTypeText>( m_pHeadText.GetPtr() );
		pText->Set( "" );
	}
	UpdatePos();
}

void CMainUI::OnLevelBegin()
{
	m_nRecordEftFrames = 120;
}

void CMainUI::RefreshPlayerInput( vector<int8>& vecInput, int32 nMatchLen, int8 nType )
{
	auto& item = m_vecInputItems[0];
	if( nMatchLen >= 0 && !vecInput.size() )
	{
		item.vec.resize( 1 );
		item.vec[0] = nType == 2 ? 14 : 12;
	}
	else
	{
		item.vec.resize( vecInput.size() + ( nType == 1 ? 1 : 0 ) );
		if( nType == 1 )
		{
			item.vec.back() = 13;
			if( nMatchLen > 0 )
				nMatchLen++;
		}
		for( int i = 0; i < vecInput.size(); i++ )
		{
			auto n = vecInput[i];
			if( n == 1 )
				item.vec[i] = 0;
			else if( n == ( 1 | 2 ) )
				item.vec[i] = 1;
			else if( n == 2 )
				item.vec[i] = 2;
			else if( n == ( 2 | 4 ) )
				item.vec[i] = 3;
			else if( n == 4 )
				item.vec[i] = 4;
			else if( n == ( 4 | 8 ) )
				item.vec[i] = 5;
			else if( n == 8 )
				item.vec[i] = 6;
			else if( n == ( 8 | 1 ) )
				item.vec[i] = 7;
			else if( n < 0 )
				item.vec[i] = 7 - n;
		}
	}

	item.nMatchLen = nMatchLen;
	item.vecElems.resize( item.vec.size() );
	UpdateInputItem( 0 );
}

void CMainUI::InsertDefaultFinishAction()
{
	m_nPlayerActionFrame = CGlobalCfg::Inst().MainUIData.vecActionEftFrames.size();
	for( int i = m_vecInputItems.size() - 1; i >= 2; i-- )
		m_vecInputItems[i] = m_vecInputItems[i - 1];
	m_vecInputItems[1].nMatchLen = 0;
	m_vecInputItems[1].vec.resize( 4 );
	for( int i = 0; i < 4; i++ )
		m_vecInputItems[1].vec[i] = 15;
	m_vecInputItems[1].vecElems.resize( 4 );
	for( int i = 1; i < m_vecInputItems.size(); i++ )
		UpdateInputItem( i );
}

void CMainUI::OnPlayerAction( vector<int8>& vecInput, int32 nMatchLen, int8 nType )
{
	if( IsScenario() )
		return;
	if( nType != 2 )
		m_nPlayerActionFrame = CGlobalCfg::Inst().MainUIData.vecActionEftFrames.size();
	RefreshPlayerInput( vecInput, nMatchLen, nType );

	for( int i = m_vecInputItems.size() - 1; i >= 1; i-- )
		m_vecInputItems[i] = m_vecInputItems[i - 1];
	m_vecInputItems[0].nMatchLen = -1;
	m_vecInputItems[0].vec.resize( 0 );
	m_vecInputItems[0].vecElems.resize( 0 );
	for( int i = 1; i < m_vecInputItems.size(); i++ )
		UpdateInputItem( i );
}

void CMainUI::BeginScenario()
{
	Reset( m_playerInputOrig, m_iconOrig, false );
	m_bScenario = true;
	m_nLastScenarioText = -1;
}

void CMainUI::EndScenario()
{
	m_bScenario = false;
	for( int i = 0; i < 2; i++ )
		SafeCast<CTypeText>( m_pScenarioText[i].GetPtr() )->Set( "" );
}

void CMainUI::ScenarioText( int8 n, const char* sz, const CVector4& color, int32 nFinishDelay, int32 nSpeed, const char* szSound, int32 nSoundInterval )
{
	if( n < 0 || n >= 2 )
		return;
	auto pText = SafeCast<CTypeText>( m_pScenarioText[n].GetPtr() );
	pText->SetParam( color );
	pText->SetTypeInterval( nSpeed );
	pText->Set( sz, n & 1 );
	pText->SetTypeSound( szSound, nSoundInterval );
	m_nLastScenarioText = n;
	m_nScenarioTextFinishDelay = nFinishDelay;
}

bool CMainUI::IsScenarioTextFinished()
{
	if( !m_bScenario || m_nLastScenarioText < 0 )
		return true;
	if( m_nScenarioTextFinishDelay )
		return false;
	auto pText = SafeCast<CTypeText>( m_pScenarioText[m_nLastScenarioText].GetPtr() );
	return pText->IsFinished();
}

void CMainUI::HeadText( const char* sz, const CVector4& color, int32 nTime, const char* szSound, int32 nSoundInterval )
{
	auto pText = SafeCast<CTypeText>( m_pHeadText.GetPtr() );
	pText->SetParam( color );
	pText->Set( sz, 2 );
	pText->SetTypeSound( szSound, nSoundInterval );
	m_nHeadTextTime = nTime;
}

void CMainUI::ShowFailEft( bool b )
{
	if( !b )
	{
		for( int i = 0; i < 3; i++ )
			m_pFailTips[i]->bVisible = false;
		return;
	}
	auto& worldData = GetStage()->GetMasterLevel()->GetWorldData();
	bool bFailTips[3] = { worldData.nCurFrameCount > 0, worldData.nCurFrameCount > 1, worldData.pCheckPoint != NULL };
	if( bFailTips[2] && !bFailTips[0] )
		swap( bFailTips[2], bFailTips[0] );
	if( bFailTips[2] && !bFailTips[1] )
		swap( bFailTips[2], bFailTips[1] );
	CVector2 p( -64, -16 );
	for( int i = 0; i < 3; i++ )
	{
		if( !bFailTips[i] )
		{
			m_pFailTips[i]->bVisible = false;
			continue;
		}
		m_pFailTips[i]->bVisible = true;
		m_pFailTips[i]->SetPosition( p );
		p.x += 160;
	}
}

void CMainUI::ShowFreezeEft( int32 nLevel )
{
	m_nFreezeLevel = nLevel;
}

void CMainUI::Update()
{
	UpdatePos();
	UpdateIcons();
	m_vecPlayerActionElems.resize( 0 );
	m_vecPlayerActionElemParams.resize( 0 );
	auto pCurLevel = GetStage()->GetMasterLevel()->GetCurLevel();
	if( !m_bScenario && m_nPlayerActionFrame )
	{
		Effect0();
	}
	if( m_nRecordEftFrames )
	{
		RecordEffect();
		m_nRecordEftFrames--;
	}
	else if( pCurLevel && pCurLevel->IsFailed() )
	{
		RecordEffect();
		FailEffect();
	}
	else if( m_nFreezeLevel )
		FreezeEffect( m_nFreezeLevel );

	if( m_bScenario )
	{
		if( pCurLevel )
			Effect1();
	}
	for( int i = 0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.pInstData = &m_vecPlayerActionElemParams[i * 2];
	}

	auto pText = SafeCast<CTypeText>( m_pHeadText.GetPtr() );
	if( m_nHeadTextTime )
	{
		m_nHeadTextTime--;
		if( !m_nHeadTextTime )
			pText->Set( "" );
	}
	pText->Update();
	if( m_bScenario )
	{
		for( int k = 0; k < 2; k++ )
			SafeCast<CTypeText>( m_pScenarioText[k].GetPtr() )->Update();
	}
	if( !IsScenarioTextFinished() )
	{
		if( CGame::Inst().IsKeyDown( VK_RETURN ) )
		{
			auto pText = SafeCast<CTypeText>( m_pScenarioText[m_nLastScenarioText].GetPtr() );
			pText->ForceFinish();
			m_nScenarioTextFinishDelay = 0;
		}
		else if( m_nScenarioTextFinishDelay > 0 )
		{
			auto pText = SafeCast<CTypeText>( m_pScenarioText[m_nLastScenarioText].GetPtr() );
			if( pText->IsFinished() )
				m_nScenarioTextFinishDelay--;
		}
	}
}

void CMainUI::Render( CRenderContext2D& context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_vecPlayerActionElems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
	for( auto& item : m_vecInputItems )
	{
		for( auto& elem : item.vecElems )
		{
			elem.worldMat = globalTransform;
			elem.SetDrawable( pDrawables[nPass] );
			context.AddElement( &elem, nGroup );
		}
	}
}

void CMainUI::UpdatePos()
{
	auto p = GetStage()->GetMasterLevel()->GetCamPos();
	auto pPlayer = GetStage()->GetWorld()->GetPlayer();
	if( !m_bScenario && pPlayer && pPlayer->GetStage() )
		p.y = pPlayer->GetMoveTo().y * LEVEL_GRID_SIZE_Y + pPlayer->GetLevel()->y;
	p.y += m_playerInputOrig.y;
	SetPosition( p );
	if( m_bScenario )
	{
		auto pLevel = GetStage()->GetMasterLevel()->GetCurLevel();
		if( pLevel )
		{
			auto levelSize = pLevel->GetSize();
			auto camSize = GetStage()->GetCamera().GetViewArea().GetSize();
			CRectangle rect0( -camSize.x * 0.5f + 144, pLevel->y - y - 64, camSize.x - 288, levelSize.y * LEVEL_GRID_SIZE_Y + 128 );
			m_pScenarioText[0]->SetPosition( CVector2( rect0.x, rect0.y ) );
			m_pScenarioText[1]->SetPosition( CVector2( rect0.GetRight(), rect0.GetBottom()
				- SafeCast<CTypeText>( m_pScenarioText[1].GetPtr() )->GetInitTextBound().height ) );
		}
	}
}

void CMainUI::UpdateInputItem( int32 nItem )
{
	auto& item = m_vecInputItems[nItem];
	auto& param = CGlobalCfg::Inst().MainUIData.vecPlayerInputParams[nItem];
	auto p0 = m_playerInputOrig;
	p0.y = 0;
	if( nItem == 0 )
	{
		auto pPlayer = GetStage()->GetWorld()->GetPlayer();
		if( pPlayer && pPlayer->GetStage() )
			p0.x = pPlayer->GetMoveTo().x * LEVEL_GRID_SIZE_X + pPlayer->GetLevel()->x - x;
	}
	for( int i = 0; i < item.vec.size(); i++ )
	{
		auto& elem = item.vecElems[i];
		elem.rect = CRectangle( p0.x - ( item.vec.size() - i ) * 16, p0.y - nItem * 16, 16, 16 ).Offset( param.ofs );
		elem.texRect = CRectangle( 0.0625f * item.vec[i], 0, 0.0625f, 0.0625f );
		elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.pInstData = param.params + ( item.nMatchLen >= 0 && item.vec.size() - i > item.nMatchLen ? 2 : 0 );
	}
}

void CMainUI::UpdateIcons()
{
	if( m_bScenario || GetStage()->GetMasterLevel()->GetTransferType() )
	{
		for( int i = 0; i < ELEM_COUNT( m_pIcons ); i++ )
			m_pIcons[i]->bVisible = false;
		return;
	}
	auto pPlayer = GetStage()->GetWorld()->GetPlayer();
	if( pPlayer )
	{
		for( int i = 0; i < ePlayerEquipment_Count; i++ )
		{
			auto pEquipment = pPlayer->GetEquipment( i );
			if( pEquipment )
			{
				int32 nIcon = pEquipment->GetIcon();
				auto pIcon = m_pIcons[i];
				pIcon->bVisible = true;
				pIcon->SetPosition( CVector2( m_iconOrig.x, pIcon->y ) );
				auto texRect = static_cast<CImage2D*>( pIcon->GetRenderObject() )->GetElem().texRect;
				texRect.x = nIcon % 4 * 0.25f;
				texRect.y = nIcon / 4 * 0.125f;
				static_cast<CImage2D*>( pIcon->GetRenderObject() )->SetTexRect( texRect );
				if( i == ePlayerEquipment_Ranged )
				{
					auto w = pEquipment->GetAmmoIconWidth() * pEquipment->GetAmmo() / pEquipment->GetMaxAmmo();
					auto rect = static_cast<CImage2D*>( m_pAmmoCount->GetRenderObject() )->GetElem().rect;
					rect.SetLeft( rect.GetRight()- w );
					CRectangle texRect1( nIcon % 4 * 0.25f, nIcon / 4 * 0.125f, 0.25f, 0.125f );
					texRect1.SetLeft( texRect1.GetRight() - texRect1.width * rect.width / 64 );
					texRect1.SetTop( texRect1.GetBottom() - texRect1.height * rect.height / 32 );
					static_cast<CImage2D*>( m_pAmmoCount->GetRenderObject() )->SetRect( rect );
					static_cast<CImage2D*>( m_pAmmoCount->GetRenderObject() )->SetTexRect( texRect1 );
					m_pAmmoCount->SetBoundDirty();
				}
			}
			else
				m_pIcons[i]->bVisible = false;
		}
	}
}

void CMainUI::Effect0()
{
	auto& vecFrameCfg = CGlobalCfg::Inst().MainUIData.vecActionEftFrames;
	int32 nFrame = vecFrameCfg.size() - m_nPlayerActionFrame;
	m_nPlayerActionFrame--;

	auto& frameData = vecFrameCfg[nFrame];
	auto camSize = GetStage()->GetCamera().GetViewArea().GetSize();
	CRectangle rect0( -camSize.x * 0.5f, -frameData.nTotalHeight, camSize.x, frameData.nTotalHeight );
	auto nSize0 = m_vecPlayerActionElems.size();
	m_vecPlayerActionElems.resize( nSize0 + 1 );
	m_vecPlayerActionElems.back().rect = rect0;

	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		if( elem.rect.height > frameData.nMaxImgHeight )
		{
			int32 h = SRand::Inst().Rand<int32>( 1, elem.rect.height / 4 ) * 2;
			int32 h1 = elem.rect.height - h;
			if( SRand::Inst<eRand_Render>().Rand( 0, 2 ) )
				swap( h, h1 );
			auto rect1 = elem.rect;
			elem.rect.height = h;
			rect1.SetTop( elem.rect.GetBottom() );
			m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
			auto& elem1 = m_vecPlayerActionElems.back();
			elem1.rect = rect1;
			i--;
		}
	}
	m_vecPlayerActionElemParams.resize( m_vecPlayerActionElems.size() * 2 );
	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto t = 1.0f - ( m_vecPlayerActionElems[i].rect.GetCenterY() - rect0.y ) / rect0.height;
		auto v1 = frameData.params[0].v() * ( ( 1 - t ) * ( 1 - t ) ) + frameData.params[1].v() * ( 2 * t * ( 1 - t ) ) + frameData.params[2].v() * ( t * t );
		auto v2 = frameData.params[3].v() * ( ( 1 - t ) * ( 1 - t ) ) + frameData.params[4].v() * ( 2 * t * ( 1 - t ) ) + frameData.params[5].v() * ( t * t );
		auto v = v1 + ( v2 - v1 ) * CVector3( SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ),
			SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ), SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) );
		float fOfs = floor( v.x * 0.5f + 0.5f ) * 2;
		float fLum = Max( 0.0f, v.y );
		CVector3 add;
		int32 k = SRand::Inst<eRand_Render>().Rand( 0, 5 );
		if( k == 0 )
		{
			if( fLum >= 2 )
				add = CVector3( fLum - 2, 1, 1 );
			else
				add = CVector3( 0, fLum * 0.5f, fLum * 0.5f );
			add = add * 0.35f + CVector3( fLum, fLum, fLum ) * 0.25f;
		}
		else if( k == 1 )
		{
			if( fLum >= 1 )
				add = CVector3( 1, ( fLum - 1 ) * 0.5f, ( fLum - 1 ) * 0.5f );
			else
				add = CVector3( fLum, 0, 0 );
		}
		else
			add = CVector3( fLum / 3, fLum / 3, fLum / 3 );
		CVector3 mul = CVector3( 1, 1, 1 ) - add * v.z;
		add = add * CVector3( 0.75f, 0.73f, 0.5f );
		m_vecPlayerActionElemParams[i * 2] = CVector4( mul.x, mul.y, mul.z, fOfs );
		m_vecPlayerActionElemParams[i * 2 + 1] = CVector4( add.x, add.y, add.z, 0 );
	}

	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		elem.texRect.y = SRand::Inst<eRand_Render>().Rand<int32>( m_origRect.height / 4, floor( ( m_origRect.height - elem.rect.height ) * 0.5f ) ) * 2;
	}
	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		float w = SRand::Inst().Rand( m_origRect.width / 2, m_origRect.width );
		w = floor( w * 0.5f + 0.5f ) * 2;
		if( elem.rect.width > w )
		{
			auto w1 = elem.rect.width - w;
			if( SRand::Inst<eRand_Render>().Rand( 0, 2 ) )
				swap( w, w1 );
			auto rect1 = elem.rect;
			auto tex1 = elem.texRect;
			elem.rect.width = w;
			rect1.SetLeft( elem.rect.GetRight() );
			auto pInstData = elem.pInstData;
			m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
			m_vecPlayerActionElemParams.resize( m_vecPlayerActionElemParams.size() + 2 );
			auto& elem1 = m_vecPlayerActionElems.back();
			elem1.rect = rect1;
			elem1.texRect = tex1;
			elem1.nInstDataSize = sizeof( CVector4 ) * 2;
			elem1.pInstData = pInstData;
			m_vecPlayerActionElemParams[m_vecPlayerActionElemParams.size() - 2] = m_vecPlayerActionElemParams[i * 2];
			m_vecPlayerActionElemParams[m_vecPlayerActionElemParams.size() - 1] = m_vecPlayerActionElemParams[i * 2 + 1];
			i--;
		}
	}
	int32 nTexScale = nFrame * 2 + 1;
	while( nTexScale & ( nTexScale - 1 ) )
		nTexScale = nTexScale & ( nTexScale - 1 );
	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		elem.texRect = CRectangle( SRand::Inst<eRand_Render>().Rand<int32>( 0, floor( ( m_origRect.width - elem.rect.width / nTexScale ) * 0.5f ) ) * 2,
			elem.texRect.y, elem.rect.width / nTexScale, elem.rect.height );
		elem.texRect = CRectangle( elem.texRect.x / m_origRect.width, elem.texRect.y / m_origRect.height,
			elem.texRect.width / m_origRect.width, elem.texRect.height / m_origRect.height );
	}
}

void CMainUI::Effect1()
{
	auto pLevel = GetStage()->GetMasterLevel()->GetCurLevel();
	auto levelSize = pLevel->GetSize();
	auto camSize = GetStage()->GetCamera().GetViewArea().GetSize();
	CRectangle rect0( -camSize.x * 0.5f, pLevel->y - y - 64, camSize.x, levelSize.y * LEVEL_GRID_SIZE_Y + 128 );
	auto nSize0 = m_vecPlayerActionElems.size();
	m_vecPlayerActionElems.resize( nSize0 + 1 );
	m_vecPlayerActionElems.back().rect = rect0;
	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		int32 h = SRand::Inst<eRand_Render>().Rand( 4, 16 + 1 ) * 2;
		if( elem.rect.height >= h * 2 )
		{
			int32 h1 = elem.rect.height - h;
			if( SRand::Inst<eRand_Render>().Rand( 0, 2 ) )
				swap( h, h1 );
			auto rect = elem.rect;
			rect.height = h;
			elem.rect.SetTop( rect.GetBottom() );
			m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
			m_vecPlayerActionElems.back().rect = rect;
			i--;
		}
	}
	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		int32 w = SRand::Inst<eRand_Render>().Rand( 100, 200 + 1 ) * 2;
		if( elem.rect.width >= w * 2 )
		{
			int32 w1 = elem.rect.width - w;
			if( SRand::Inst<eRand_Render>().Rand( 0, 2 ) )
				swap( w, w1 );
			auto rect = elem.rect;
			rect.width = w;
			elem.rect.SetLeft( rect.GetRight() );
			m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
			m_vecPlayerActionElems.back().rect = rect;
			i--;
		}
	}

	static SRand rand0 = SRand::Inst<eRand_Render>();
	SRand rand1 = rand0;
	for( int k = 0; k < 1; k++ )
		rand0.Rand();
	for( int k = 0; k < SRand::Inst<eRand_Render>().Rand( rect0.height / 80, rect0.height / 70 + 1 ); k++ )
	{
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		m_vecPlayerActionElems.back().rect = m_vecPlayerActionElems[k + nSize0].rect;
		int32 w = rand1.Rand( rect0.width / 16, rect0.width / 8 ) * 2;
		int32 h = rand1.Rand( 1, 4 ) * 2;
		m_vecPlayerActionElems[k + nSize0].rect = CRectangle( rect0.x + rand1.Rand<int32>( 0, ( rect0.width - w ) / 2 + 1 ) * 2,
			rect0.y + rand1.Rand<int32>( 0, ( rect0.height - h ) / 2 + 1 ) * 2, w, h );
	}

	m_vecPlayerActionElemParams.resize( m_vecPlayerActionElems.size() * 2 );
	for( int i = nSize0; i < m_vecPlayerActionElems.size(); i++ )
	{
		auto& elem = m_vecPlayerActionElems[i];
		if( elem.rect.height >= 8 )
		{
			elem.texRect.width = elem.rect.width / 32;
			elem.texRect.height = elem.rect.height / 4;
		}
		else
		{
			elem.texRect.width = elem.texRect.height = 2;
		}
		elem.texRect = CRectangle( SRand::Inst<eRand_Render>().Rand<int32>( 0, floor( ( m_origRect.width - elem.texRect.width ) * 0.5f ) ) * 2,
			m_origRect.height * 0.375f + SRand::Inst<eRand_Render>().Rand<int32>( 0, floor( ( m_origRect.height / 8 - elem.texRect.height ) * 0.5f ) ) * 2,
			elem.texRect.width, elem.texRect.height );
		elem.texRect = CRectangle( elem.texRect.x / m_origRect.width, elem.texRect.y / m_origRect.height,
			elem.texRect.width / m_origRect.width, elem.texRect.height / m_origRect.height );

		float t = ( elem.rect.GetCenterY() - rect0.y ) / rect0.height + SRand::Inst().Rand( -0.005f, 0.005f );
		float l[2] = { t - 0.05f, t + 0.05f };
		for( int i = 0; i < 2; i++ )
		{
			float k = l[i] - 0.5f;
			l[i] = Max( 0.0f, Min( 1.0f, ( 1 - k * k * 4 ) * 1.3f ) );
		}
		float l1[2] = { l[0], l[1] };
		int8 nType = 1;
		if( elem.rect.height < 8 )
		{
			nType = 0;
			auto k = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			l1[1 - k] = ( l1[0] + l1[1] ) * 0.5f;
			l1[k] = 0;
		}
		m_vecPlayerActionElemParams[i * 2] = CVector4( l1[0], l1[1] + l1[0], l1[1], 0 ) + CVector4( 1, 1, 1, 0 ) * nType;
		m_vecPlayerActionElemParams[i * 2 + 1] = CVector4( l[0] * 1.2f, l[1], l[1] * 0.75f, 0 ) * ( nType ? 0.11f : 0 )
			+ CVector4( l1[0], l1[1], l1[1], 0 ) * ( 1 - nType ) * 0.1f;
		m_vecPlayerActionElemParams[i * 2].w = ( floor( ( l[0] + l[1] ) * 12 ) * 2 );
	}
}

void CMainUI::RecordEffect()
{
	auto camSize = GetStage()->GetCamera().GetViewArea().GetSize();
	CRectangle r0( -camSize.x * 0.5f + 64, -16, 20, 30 );
	auto& worldData = GetStage()->GetMasterLevel()->GetWorldData();
	if( worldData.pCheckPoint )
	{
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem = m_vecPlayerActionElems.back();
		elem.rect = r0;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.5f, 0.5f, 0.5f, 4 ) );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.4f, 0.1f, 0.35f, 4 ) );
		r0.x += 40;
	}
	for( int i = 0; i < worldData.nCurFrameCount; i++ )
	{
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem = m_vecPlayerActionElems.back();
		elem.rect = r0;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.5f, 0.5f, 0.5f, 4 ) );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.4f, 0.45f, 0.17f, 4 ) );
		r0.x += 40;
	}
}

void CMainUI::FailEffect()
{
	auto camSize = GetStage()->GetCamera().GetViewArea().GetSize();
	CRectangle r0( -camSize.x * 0.5f, -28, camSize.x, 28 );

	static SRand rand0_1 = SRand::Inst<eRand_Render>();
	SRand rand1 = rand0_1;
	if( !SRand::Inst<eRand_Render>().Rand( 0, 8 ) )
		rand0_1.Rand();
	for( int n = rand1.Rand( -6, 4 ); n > 0; n-- )
	{
		CRectangle rect = r0;
		rect.width = rand1.Rand( 20, 70 ) * 4;
		rect.x = rand1.Rand<int32>( r0.x / 2, ( r0.GetRight() - rect.width ) / 2 + 1 ) * 2;
		rect.y -= rand1.Rand( 0, 4 ) * 2;
		int32 nOfs0 = rand1.Rand( 0, 4 );
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem = m_vecPlayerActionElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );

		CVector3 color( rand1.Rand( 0.0f, 1.0f ), rand1.Rand( 0.0f, 1.0f ), rand1.Rand( 0.0f, 1.0f ) );
		float f = Min( color.x, Min( color.y, color.z ) );
		color = ( color - CVector3( f, f, f ) ) * rand1.Rand( 1.0f, 1.3f );
		m_vecPlayerActionElemParams.push_back( CVector4( color.x, color.y, color.z, nOfs0 * 2 ) );
		m_vecPlayerActionElemParams.push_back( CVector4( color.x, color.y, color.z, 0 ) * rand1.Rand( 0.01f, 0.2f ) );
	}

	static SRand rand0_2 = SRand::Inst<eRand_Render>();
	rand1 = rand0_2;
	if( !SRand::Inst<eRand_Render>().Rand( 0, 16 ) )
		rand0_2.Rand();
	int8 nType = rand1.Rand( -4, 3 );
	if( nType == 0 || nType == 1 )
	{
		CRectangle rect = r0;
		rect.y -= rand1.Rand( 100, 200 ) * 2;
		rect.SetBottom( rand1.Rand<int32>( r0.y / 2, r0.GetBottom() / 2 ) * 2 );
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem = m_vecPlayerActionElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );

		CVector3 color( rand1.Rand( 0.0f, 0.025f ), rand1.Rand( 0.0f, 0.025f ), rand1.Rand( 0.0f, 0.025f ) );
		m_vecPlayerActionElemParams.push_back( CVector4( 1, 1, 1, rand1.Rand( -3, 4 ) * 2 ) );
		m_vecPlayerActionElemParams.push_back( CVector4( -color.x, -color.y, -color.z, rand1.Rand( -3, 4 ) * 2 ) );
	}
	else if( nType == 2 )
	{
		CRectangle rect = r0;
		rect.y -= rand1.Rand( 100, 200 ) * 2;
		rect.SetBottom( rand1.Rand<int32>( r0.y / 2, r0.GetBottom() / 2 ) * 2 );
		rect.x = rand1.Rand( -16, 64 ) * 2;
		rect.width = rand1.Rand( 18, 56 ) * 2;
		int32 nCount = ( r0.GetRight() - rect.x ) / rect.width;
		int8 nDir = rand1.Rand( 0, 2 );
		if( nDir )
			rect.x = r0.x + r0.GetRight() - rect.GetRight();
		float fOfsY = rand1.Rand( -3, 4 ) * 2;
		CVector3 color( rand1.Rand( 0.0f, 0.07f ), rand1.Rand( 0.0f, 0.07f ), rand1.Rand( 0.0f, 0.07f ) );
		for( int i = 0; i < nCount; i++ )
		{
			m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
			auto& elem = m_vecPlayerActionElems.back();
			elem.rect = rect;
			elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
			m_vecPlayerActionElemParams.resize( m_vecPlayerActionElemParams.size() + 2 );
			m_vecPlayerActionElemParams.push_back( CVector4( 1, 1, 1, ( nDir ? rect.width : -rect.width ) * ( i + 1 ) ) );
			auto color1 = color * -i;
			m_vecPlayerActionElemParams.push_back( CVector4( color1.x, color1.y, color1.z, -fOfsY * ( i + 1 ) ) );
			rect.x += nDir ? -rect.width : rect.width;
		}
	}

	CRectangle rect = r0;
	int32 nOfs0 = SRand::Inst<eRand_Render>().Rand( 0, 4 );
	rect.x += nOfs0 * 8;
	m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
	auto& elem = m_vecPlayerActionElems.back();
	elem.rect = rect;
	elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
	m_vecPlayerActionElemParams.push_back( CVector4( 0.95f, 0.88f, 0.88f, nOfs0 * 2 ) );
	m_vecPlayerActionElemParams.push_back( CVector4( 0.1f, -0.05f, -0.05f, 0 ) );
}

void CMainUI::FreezeEffect( int32 nLevel )
{
	auto r0 = GetStage()->GetCamera().GetViewArea();
	r0 = r0.Offset( globalTransform.GetPosition() * -1 );

	static SRand rd0 = SRand::Inst<eRand_Render>();
	SRand::Inst<eRand_Render>().Rand();
	SRand rand0 = rd0;

	if( nLevel >= 10 )
	{
		CRectangle rect( -160, -96, 384, 64 );
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem = m_vecPlayerActionElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0.125f, 0.25f, 0.75f, 0.125f );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.1f, 0, 0, 0 ) );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.32f, 0.03f, 0.1f, 0 ) );


		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem1 = m_vecPlayerActionElems.back();
		elem1.rect = CRectangle( r0.x, -98, r0.width, 64 );
		elem1.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.1f, 0, 0, 0 ) );
		m_vecPlayerActionElemParams.push_back( CVector4( 0.35f, 0, 0, 0 ) );
	}

	{
		static SRand rd1 = SRand::Inst<eRand_Render>();
		SRand::Inst<eRand_Render>().Rand();
		SRand rand1 = rd1;
		if( !SRand::Inst<eRand_Render>().Rand( 0, 16 ) )
			rd1.Rand();
		static SRand rd2 = SRand::Inst<eRand_Render>();
		SRand::Inst<eRand_Render>().Rand();
		SRand rand2 = rd2;
		if( !SRand::Inst<eRand_Render>().Rand( 0, 4 ) )
			rd2.Rand();

		for( int n = rand1.Rand( nLevel, nLevel * 2 + 1 ); n > 0; n-- )
		{
			CRectangle rect = r0;
			rect.height = rand0.Rand( 14, 22 ) * 2;
			rect.width = rand0.Rand( 50, 80 ) * 4;
			rect.x = rand0.Rand<int32>( r0.x / 2, ( r0.GetRight() - rect.width ) / 2 + 1 ) * 2;
			rect.y = rand0.Rand<int32>( r0.y / 2, ( r0.GetBottom() - rect.height ) / 2 + 1 ) * 2;

			CVector2 ofs( rand2.Rand( 0, 4 ), rand2.Rand( 0, 4 ) );

			m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
			auto& elem = m_vecPlayerActionElems.back();
			elem.rect = rect;
			elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );

			CVector3 color( rand0.Rand( 0.0f, 1.0f ), rand0.Rand( 0.0f, 1.0f ), rand0.Rand( 0.0f, 1.0f ) );
			float f = Min( color.x, Min( color.y, color.z ) );
			color = ( color - CVector3( f, f, f ) ) * rand0.Rand( 1.0f, 1.3f );
			auto color1 = ( CVector3( 1, 1, 1 ) - color ) * rand0.Rand( 0.9f, 1.1f );
			color = color * CVector3( 0.4f, 0.3f, 0.2f );
			m_vecPlayerActionElemParams.push_back( CVector4( color1.x, color1.y, color1.z, ofs.x * 2 ) );
			m_vecPlayerActionElemParams.push_back( CVector4( color.x, color.y, color.z, ofs.y * 2 ) );
		}
	}
	if( nLevel >= 8 )
		FailEffect();
}

void SWorldData::OnEnterLevel( const char* szCurLevel, CPlayer* pPlayer, const TVector2<int32>& playerPos, int8 nPlayerDir )
{
	curFrame.strLastLevel = curFrame.strCurLevel;
	curFrame.strCurLevel = szCurLevel;
	curFrame.playerEnterPos = playerPos;
	curFrame.nPlayerEnterDir = nPlayerDir;
	curFrame.playerData.Clear();
	pPlayer->SaveData( curFrame.playerData );

	SWorldDataFrame* p = NULL;
	if( nCurFrameCount >= nMaxFrameCount )
	{
		p = backupFrames.front();
		backupFrames.pop_front();
		backupFrames.push_back( p );
	}
	else
	{
		if( nCurFrameCount >= backupFrames.size() )
			backupFrames.push_back( new SWorldDataFrame );
		p = backupFrames[nCurFrameCount++];
	}
	*p = curFrame;
}

void SWorldData::OnReset( CPlayer* pPlayer )
{
	auto p = nCurFrameCount ? backupFrames[nCurFrameCount - 1]: pCheckPoint;
	ASSERT( p );
	curFrame = *p;
	curFrame.playerData.ResetCurPos();
	pPlayer->LoadData( curFrame.playerData );
}

void SWorldData::OnRetreat( CPlayer* pPlayer )
{
	if( nCurFrameCount > 1 || pCheckPoint && nCurFrameCount )
		nCurFrameCount--;
	return OnReset( pPlayer );
}

void SWorldData::CheckPoint( CPlayer* pPlayer )
{
	if( !pCheckPoint )
		pCheckPoint = new SWorldDataFrame;
	nCurFrameCount = 0;
	*pCheckPoint = curFrame;
	pCheckPoint->playerEnterPos = pPlayer->GetPos();
	pCheckPoint->nPlayerEnterDir = pPlayer->GetCurDir();
	pCheckPoint->playerData.Clear();
	pPlayer->SaveData( pCheckPoint->playerData );
}

void SWorldData::OnRestoreToCheckpoint( CPlayer* pPlayer )
{
	if( !pCheckPoint )
		return OnReset( pPlayer );
	nCurFrameCount = 0;
	curFrame = *pCheckPoint;
	curFrame.playerData.ResetCurPos();
	pPlayer->LoadData( curFrame.playerData );
}

void SWorldData::ClearKeys()
{
	curFrame.mapDataInt.clear();
	curFrame.mapLevelData.clear();
}

void SWorldData::Respawn()
{
	for( auto& item : curFrame.mapLevelData )
		item.second.mapDataDeadPawn.clear();
}

void CMasterLevel::Init( CPlayer* pPlayer )
{
	m_pPlayer = pPlayer;
	CLuaMgr::Inst().Run( CGlobalCfg::Inst().strWorldInitScript.c_str() );
}

void CMasterLevel::Begin( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir )
{
	auto p = pLevelPrefab->GetRoot()->CreateInstance();
	auto pLevel = SafeCast<CMyLevel>( p );
	if( pLevel )
	{
		m_pCurLevelPrefab = pLevelPrefab;
		m_worldData.OnEnterLevel( pLevelPrefab->GetName(), m_pPlayer, playerPos, nPlayerDir );
		m_pCurLevel = pLevel;
		pLevel->SetParentBeforeEntity( m_pLevelFadeMask );
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
		pLevel->Init();
		pLevel->AddPawn( m_pPlayer, playerPos, nPlayerDir );
		ResetMainUI();
		pLevel->Begin();
	}
	else
	{
		ResetMainUI();
		auto pCutScene = SafeCast<CCutScene>( p );
		ASSERT( pCutScene );
		m_pCurCutScene = pCutScene;
		pCutScene->SetParentBeforeEntity( m_pLevelFadeMask );
		pCutScene->Begin();
	}
}

void CMasterLevel::TransferTo( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir )
{
	ResetMainUI();
	m_nPlayerDamageFrame = 0;
	auto p = pLevelPrefab->GetRoot()->CreateInstance();
	auto pLevel = SafeCast<CMyLevel>( p );
	if( pLevel )
	{
		if( !m_pCurCutScene )
		{
			EndCurLevel();
		}
		else
		{
			m_pCurCutScene->End();
			m_pCurCutScene->SetParentEntity( NULL );
		}
		m_pLastLevelPrefab = m_pCurLevelPrefab;
		m_pCurLevelPrefab = pLevelPrefab;

		m_worldData.OnEnterLevel( pLevelPrefab->GetName(), m_pPlayer, playerPos, nPlayerDir );
		m_pLastLevel = m_pCurLevel;
		m_pCurLevel = pLevel;
		pLevel->SetParentAfterEntity( m_pLevelFadeMask );
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
		pLevel->Init();

		if( !m_pCurCutScene )
		{
			auto d = m_pPlayer->GetPos() - playerPos;
			m_transferOfs = CVector2( d.x, d.y ) * LEVEL_GRID_SIZE;
			pLevel->SetPosition( m_transferOfs );
			m_camTransferBegin = m_pLastLevel->GetCamPos();
			auto dVisualTransfer = m_transferOfs + m_pCurLevel->GetCamPos() - m_camTransferBegin;
			m_nTransferAnimFrames = ceil( dVisualTransfer.Length() / CGlobalCfg::Inst().lvTransferData.fTransferCamSpeed );
			m_nTransferAnimTotalFrames = m_nTransferAnimFrames;
			m_nTransferFadeOutFrames = m_nTransferFadeOutTotalFrames = CGlobalCfg::Inst().lvTransferData.nTransferFadeOutFrameCount;
			m_nTransferType = 1;
		}
		else
		{
			m_pCurCutScene = NULL;
			m_nTransferAnimFrames = m_nTransferAnimTotalFrames = CGlobalCfg::Inst().lvTransferData.nTransferFadeOutFrameCount;
			m_nTransferFadeOutFrames = m_nTransferFadeOutTotalFrames = 0;
			m_nTransferType = 2;
		}
		auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
		pParams[0] = pParams[1] = CVector4( 0, 0, 0, 0 );
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetFade( 0 );
	}
	else
	{
		auto pCutScene = SafeCast<CCutScene>( p );
		ASSERT( pCutScene );
		if( !m_pCurCutScene )
		{
			EndCurLevel();
			m_pCurLevel->SetRenderParentAfter( m_pLevelFadeMask );
			m_nTransferAnimFrames = m_nTransferAnimTotalFrames = 0;
			m_nTransferFadeOutFrames = m_nTransferFadeOutTotalFrames = CGlobalCfg::Inst().lvTransferData.nTransferFadeOutFrameCount * 4;
			auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
			pParams[0] = CVector4( 1, 1, 1, 0 );
			pParams[1] = CVector4( 0, 0, 0, 0 );
			m_pCurCutScene = pCutScene;
			m_nTransferType = 3;
		}
		else
		{
			m_pCurCutScene->End();
			m_pCurCutScene->SetParentEntity( NULL );
			m_pCurCutScene = pCutScene;
			pCutScene->SetParentBeforeEntity( m_pLevelFadeMask );
			pCutScene->Begin();
		}
	}
}

void CMasterLevel::JumpBack( int8 nType )
{
	ASSERT( !m_pLastLevel && !m_pCurCutScene );
	EndCurLevel();
	m_pCurLevel->SetParentEntity( NULL );

	if( nType == 0 )
		m_worldData.OnReset( m_pPlayer );
	else if( nType == 1 )
		m_worldData.OnRetreat( m_pPlayer );
	else
		m_worldData.OnRestoreToCheckpoint( m_pPlayer );

	if( m_worldData.curFrame.strCurLevel != m_pCurLevelPrefab->GetName() )
		m_pCurLevelPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_worldData.curFrame.strCurLevel.c_str() );
	auto pLevel = SafeCast<CMyLevel>( m_pCurLevelPrefab->GetRoot()->CreateInstance() );
	m_pCurLevel = pLevel;
	pLevel->SetParentBeforeEntity( m_pLevelFadeMask );
	if( pLevel->GetEnvEffect() )
		pLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
	pLevel->Init();
	pLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
	ResetMainUI();
	pLevel->Begin();
}

SWorldDataFrame::SLevelData& CMasterLevel::GetCurLevelData()
{
	return m_worldData.curFrame.mapLevelData[m_pCurLevelPrefab->GetName()];
}

void CMasterLevel::BeginScenario()
{
	if( m_pCurLevel )
		m_pCurLevel->BeginScenario();
	m_pMainUI->BeginScenario();
}

void CMasterLevel::EndScenario()
{
	m_pMainUI->EndScenario();
	if( m_pCurLevel )
		m_pCurLevel->EndScenario();
}

bool CMasterLevel::IsScenario()
{
	return m_pMainUI->IsScenario();
}

void CMasterLevel::RunScenarioScriptText( const char* sz )
{
	auto pLuaState = CLuaState::GetCurLuaState();
	auto pCoroutine = pLuaState->CreateCoroutine( sz );
	m_pScenarioScript = pCoroutine;
	BeginScenario();
}

void CMasterLevel::RunScenarioScript()
{
	auto pLuaState = CLuaState::GetCurLuaState();
	auto pCoroutine = pLuaState->CreateCoroutineAuto();
	ASSERT( pCoroutine );
	m_pScenarioScript = pCoroutine;
	BeginScenario();
}

void CMasterLevel::CheckPoint()
{
	m_worldData.CheckPoint( m_pPlayer );
}

int32 CMasterLevel::EvaluateKeyInt( const char* str )
{
	if( str[0] == ':' )
	{
		CLuaMgr::Inst().Run( str + 1, 1 );
		return CLuaMgr::Inst().PopLuaValue<int32>();
	}
	else if( str[0] == '=' )
	{
		string strScript = "return ";
		strScript += str + 1;
		CLuaMgr::Inst().Run( strScript.c_str(), 1 );
		return CLuaMgr::Inst().PopLuaValue<int32>();
	}

	if( str[0] == '$' )
	{
		str++;
		auto& curLevelData = m_worldData.GetCurLevelData();
		auto itr = curLevelData.mapDataInt.find( str );
		if( itr == curLevelData.mapDataInt.end() )
			return 0;
		return itr->second;
	}
	else
	{
		auto itr = m_worldData.curFrame.mapDataInt.find( str );
		if( itr == m_worldData.curFrame.mapDataInt.end() )
			return 0;
		return itr->second;
	}
}

void CMasterLevel::SetKeyInt( const char* str, int32 n )
{
	if( str[0] == '$' )
	{
		str++;
		auto& curLevelData = m_worldData.GetCurLevelData();
		curLevelData.mapDataInt[str] = n;
	}
	else
		m_worldData.curFrame.mapDataInt[str] = n;
}

void CMasterLevel::ScriptTransferTo( const char* szName, int32 nPlayerX, int32 nPlayerY, int8 nPlayerDir )
{
	m_strScriptTransferTo = szName;
	m_nScriptTransferPlayerX = nPlayerX;
	m_nScriptTransferPlayerY = nPlayerY;
	m_nScriptTransferPlayerDir = nPlayerDir;
}

CVector2 CMasterLevel::GetCamPos()
{
	if( m_nTransferType == 1 )
	{
		if( m_nTransferAnimFrames )
		{
			float t = 1 - m_nTransferAnimFrames * 1.0f / m_nTransferAnimTotalFrames;
			auto p = m_camTransferBegin + ( m_pCurLevel->GetCamPos() - m_camTransferBegin ) * t;
			p = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
			return p;
		}
	}
	else if( m_pCurCutScene && m_pCurCutScene->GetParentEntity() )
		return CVector2( 0, 0 );
	return m_pCurLevel->GetCamPos();
}

void CMasterLevel::OnPlayerDamaged()
{
	m_nPlayerDamageFrame = CGlobalCfg::Inst().playerDamagedMask.size();
}

void CMasterLevel::Update()
{
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	if( m_nTransferType )
	{
		auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
		if( m_nTransferAnimFrames )
		{
			ASSERT( m_nTransferType == 1 || m_nTransferType == 2 );
			int32 nFrame = m_nTransferAnimTotalFrames - m_nTransferAnimFrames;
			m_nTransferAnimFrames--;
			float t = 1 - m_nTransferAnimFrames * 1.0f / m_nTransferAnimTotalFrames;
			if( nFrame < maskParams.size() )
			{
				pParams[0] = maskParams[nFrame].first;
				pParams[1] = maskParams[nFrame].second;
			}
			else
			{
				float a = Min( 1.0f, t * 1.05f );
				float b = Max( 0.0f, t * 1.05f - 0.05f );
				pParams[0] = CVector4( b, a, a, 0 );
				pParams[1] = CVector4( 0, 0, 0, 0 );
			}
			if( m_nTransferType == 1 )
			{
				auto p = m_transferOfs * ( 1 - t );
				p = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
				m_pCurLevel->SetPosition( p );
				m_pLastLevel->SetPosition( p - m_transferOfs );
			}
			if( m_pCurLevel->GetEnvEffect() )
				m_pCurLevel->GetEnvEffect()->SetFade( t );

			if( !m_nTransferAnimFrames )
			{
				pParams[0] = CVector4( 1, 1, 1, 0 );
				pParams[1] = CVector4( 0, 0, 0, 0 );
				if( m_nTransferType == 1 )
				{
					m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );
					m_pLastLevel->RemovePawn( m_pPlayer );
					m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
					m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
				}
				else
				{
					ASSERT( !m_pLastLevel );
					m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
					m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
					m_nTransferType = 0;
					ResetMainUI();
					m_pCurLevel->Begin();
				}
			}
		}
		else
		{
			ASSERT( m_nTransferType == 1 || m_nTransferType == 3 );
			int32 nFrame = maskParams.size() - m_nTransferFadeOutFrames;
			m_nTransferFadeOutFrames--;
			float t = m_nTransferFadeOutFrames * 1.0f / m_nTransferFadeOutTotalFrames;
			if( nFrame >= 0 && nFrame < maskParams.size() )
			{
				pParams[0] = maskParams[nFrame].first;
				pParams[1] = maskParams[nFrame].second;
			}
			else
			{
				float a = t * 1.5f;
				float b = Max( 0.0f, t * 1.5f - 0.5f );
				pParams[0] = CVector4( a, b, b, 1 );
				pParams[1] = CVector4( 0, 0, 0, 0 );
			}
			CMyLevel* pLevel = m_nTransferType == 1 ? m_pLastLevel : m_pCurLevel;
			if( pLevel->GetEnvEffect() )
				pLevel->GetEnvEffect()->SetFade( t );

			if( !m_nTransferFadeOutFrames )
			{
				pParams[0] = CVector4( 0, 0, 0, 0 );
				pParams[1] = CVector4( 0, 0, 0, 0 );
				if( m_nTransferType == 1 )
				{
					m_pLastLevel->SetParentEntity( NULL );
					m_pLastLevel = NULL;
					m_nTransferType = 0;
					ResetMainUI();
					m_pCurLevel->Begin();
				}
				else
				{
					m_pCurLevel->SetParentEntity( NULL );
					m_pCurLevel = NULL;
					m_nTransferType = 0;
					m_pCurCutScene->SetParentBeforeEntity( m_pLevelFadeMask );
					m_pCurCutScene->Begin();
				}
			}
		}
	}
	else if( m_nPlayerDamageFrame )
	{
		auto nFrame = CGlobalCfg::Inst().playerDamagedMask.size() - m_nPlayerDamageFrame;
		m_nPlayerDamageFrame--;
		auto& maskParam = CGlobalCfg::Inst().playerDamagedMask[nFrame];
		pParams[0] = maskParam.first;
		pParams[1] = maskParam.second;
	}
	else
	{
		pParams[0] = CVector4( 0, 0, 0, 0 );
		pParams[1] = CVector4( 0, 0, 0, 0 );
	}

	if( m_pCurLevel )
		m_pCurLevel->Update();
	m_pMainUI->Update();
	if( m_pScenarioScript )
	{
		if( !m_pScenarioScript->Resume( 0, 0 ) )
		{
			m_pScenarioScript = NULL;
			EndScenario();
		}
	}
	if( m_strScriptTransferTo.length() )
	{
		auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strScriptTransferTo.c_str() );
		TransferTo( pPrefab, TVector2<int32>( m_nScriptTransferPlayerX, m_nScriptTransferPlayerY ), m_nScriptTransferPlayerDir );
		m_strScriptTransferTo = "";
	}
	else if( m_nTransferType == 0 && !m_pScenarioScript && m_pCurCutScene )
	{
		TransferTo( m_pCurCutScene->m_pNextLevelPrefab, TVector2<int32>( m_pCurCutScene->m_nPlayerEnterX, m_pCurCutScene->m_nPlayerEnterY ),
			m_pCurCutScene->m_nPlayerEnterDir );
	}
}

void CMasterLevel::ResetMainUI()
{
	if( !m_pCurLevel )
		return;
	CRectangle lvRect( 0, 0, m_pCurLevel->GetSize().x * LEVEL_GRID_SIZE_X, m_pCurLevel->GetSize().y * LEVEL_GRID_SIZE_Y );
	lvRect = lvRect.Offset( m_pCurLevel->GetCamPos() * -1 );
	m_pMainUI->Reset( CVector2( lvRect.GetRight() + 64, 144 ), CVector2( lvRect.x - 64, 144 ) );
}

void CMasterLevel::EndCurLevel()
{
	m_pScenarioScript = NULL;
	EndScenario();
	m_pCurLevel->End();
}

void RegisterGameClasses_Level()
{
	REGISTER_CLASS_BEGIN( SLevelTileData )
		REGISTER_MEMBER( pTileDrawable )
		REGISTER_MEMBER( texRect )
		REGISTER_MEMBER( bBlocked )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelGridData )
		REGISTER_MEMBER( bBlocked )
		REGISTER_MEMBER( nTile )
		REGISTER_MEMBER( nNextStage )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelEnvGridDesc )
		REGISTER_MEMBER( nDist )
		REGISTER_MEMBER( param )
		REGISTER_MEMBER( paramDynamic )
		REGISTER_MEMBER( sizeDynamic )
		REGISTER_MEMBER( fPeriod )
		REGISTER_MEMBER( fRandomPhaseOfs )
		REGISTER_MEMBER( gridPhaseOfs )
		REGISTER_MEMBER( fBlendWeight )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelEnvDesc )
		REGISTER_MEMBER( arrGridDesc )
		REGISTER_MEMBER( arrJamStrength )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelEnvEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_arrEnvDescs )
		REGISTER_MEMBER_BEGIN( m_arrEnvMap )
			MEMBER_ARG( editor_hide, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_gridSize )
		REGISTER_MEMBER( m_gridOfs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelNextStageData )
		REGISTER_MEMBER( pNxtStage )
		REGISTER_MEMBER( nOfsX )
		REGISTER_MEMBER( nOfsY )
		REGISTER_MEMBER( strKeyOrig )
		REGISTER_MEMBER( strKeyRedirect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelIndicatorLayer )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelScript )
		REGISTER_BASE_CLASS( CEntity )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strCondition )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMyLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_strRegion )
		REGISTER_MEMBER( m_camPos )
		REGISTER_MEMBER( m_arrTileData )
		REGISTER_MEMBER_BEGIN( m_arrGridData )
			MEMBER_ARG( editor_hide, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_arrNextStage )
		REGISTER_MEMBER( m_arrSpawnPrefab )
		REGISTER_MEMBER( m_pTileDrawable )
		REGISTER_MEMBER_TAGGED_PTR( m_pPawnRoot, 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEnvEffect, env )
		REGISTER_MEMBER_BEGIN( m_strInitScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strBeginScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strDestroyScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()

		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SpawnPawn )
		REGISTER_LUA_CFUNCTION( Fail )
		REGISTER_LUA_CFUNCTION( Freeze )
		REGISTER_LUA_CFUNCTION( GetPawnByName )
		REGISTER_LUA_CFUNCTION( Redirect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCutScene )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_BEGIN( m_strScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_pNextLevelPrefab )
		REGISTER_MEMBER( m_nPlayerEnterX )
		REGISTER_MEMBER( m_nPlayerEnterY )
		REGISTER_MEMBER( m_nPlayerEnterDir )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMainUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pHeadText, text )
		REGISTER_MEMBER_TAGGED_PTR( m_pScenarioText[0], text_s0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pScenarioText[1], text_s1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pFailTips[0], tips_0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pFailTips[1], tips_1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pFailTips[2], tips_2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pIcons[0], icon_0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pIcons[1], icon_1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pIcons[2], icon_2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pAmmoCount, icon_1/ammo )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( ScenarioText )
		REGISTER_LUA_CFUNCTION( IsScenarioTextFinished )
		REGISTER_LUA_CFUNCTION( HeadText )
		REGISTER_LUA_CFUNCTION( ShowFreezeEft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMasterLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pMainUI, main_ui )
		REGISTER_MEMBER_TAGGED_PTR( m_pLevelFadeMask, mask )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( GetMainUI )
		REGISTER_LUA_CFUNCTION( CheckPoint )
		REGISTER_LUA_CFUNCTION( EvaluateKeyInt )
		REGISTER_LUA_CFUNCTION( SetKeyInt )
		REGISTER_LUA_CFUNCTION( ClearKeys )
		REGISTER_LUA_CFUNCTION( Respawn )
		REGISTER_LUA_CFUNCTION( ScriptTransferTo )
	REGISTER_CLASS_END()
}