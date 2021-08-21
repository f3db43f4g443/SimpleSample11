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
#include "Common/Coroutine.h"
#include "GUI/WorldMap.h"
#include "GUI/ActionPreview.h"
#include "GUI/LogUI.h"
#include "Common/FileUtil.h"
#include "Entities/InteractionUI.h"
#include "CommonUtils.h"
#include <algorithm>

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
	float fFade = m_fFade * ( m_bScenarioFade ? m_fScenarioFade : 1 );
	if( fFade <= 0 )
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
		elem.param[0] = CVector4( 1, 1, 1, 0 ) + ( elem.param[0] - CVector4( 1, 1, 1, 0 ) ) * fFade;
		elem.param[1] = CVector4( 0, 0, 0, 0 ) + ( elem.param[1] - CVector4( 0, 0, 0, 0 ) ) * fFade;
		elem.param[0].w = floor( elem.param[0].w * 0.5f + 0.5f ) * 2;
		elem.param[1].w = floor( elem.param[1].w * 0.5f + 0.5f ) * 2;
	}
}

void CLevelEnvEffect::Render( CRenderContext2D & context )
{
	if( !GetResource() )
		return;
	float fFade = m_fFade * ( m_bScenarioFade ? m_fScenarioFade : 1 );
	if( fFade <= 0 )
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


void CLevelStealthLayer::OnAddedToStage()
{
	SetRenderObject( NULL );
}

void CLevelStealthLayer::Update( CMyLevel* pLevel )
{
	auto& globalData = CGlobalCfg::Inst().lvIndicatorData;
	auto& alertData = globalData.vecStealthAlertParams[m_nTick % globalData.vecStealthAlertParams.size()];
	auto& detectData = globalData.vecStealthDetectParams[m_nTick % globalData.vecStealthDetectParams.size()];
	auto& hiddenData = globalData.vecStealthHiddenParams[m_nTick % globalData.vecStealthHiddenParams.size()];
	auto& alertHiddenData = globalData.vecStealthAlertHiddenParams[m_nTick % globalData.vecStealthAlertHiddenParams.size()];
	auto& detectHiddenData = globalData.vecStealthDetectHiddenParams[m_nTick % globalData.vecStealthDetectHiddenParams.size()];

	m_elems.resize( 0 );
	auto levelSize = pLevel->GetSize();
	float y0 = -128.0f;
	float y1 = levelSize.y * LEVEL_GRID_SIZE_Y + 128.0f;
	CRectangle r0( -2048, y0, levelSize.x * LEVEL_GRID_SIZE_X + 4096, y1 - y0 );
	m_localBound = r0;
	SetBoundDirty();
	int32 yStep = 8;
	static vector<TVector2<int32> > vec0, vec1;
	int32 yBegin = ( ( m_nTick >> 3 ) & 3 ) * 2;
	for( int y = y0 + yBegin; y < y1; y += yStep )
	{
		float k2 = Min( y - y0, y1 - y ) * 2 / 128.0f;
		float k1 = Min( y - y0, y1 - y ) * 2 / ( y1 - y0 );
		float f1 = yStep * Max( 0.0f, Min( 1.0f, SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) + k1 * 2 - 1 ) );
		float f2 = yStep * Max( 0.0f, Min( 1.0f, SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) + k2 * 2 - 1 ) );
		int32 h = floor( f1 * 0.5f + 0.5f ) * 2;
		if( h <= 0 )
			continue;
		vec0.push_back( TVector2<int32>( y, h ) );
		int32 h1 = Min<int32>( h - 2, floor( f2 * 0.5f + 0.5f ) * 2 );
		if( h1 <= 0 )
			continue;
		vec1.push_back( TVector2<int32>( y + SRand::Inst<eRand_Render>().Rand( 0, ( h - h1 ) / 2 ) * 2, h1 ) );
	}
	static vector<TVector3<int32> > vecAlertEft;

	auto pPlayer = pLevel->GetPlayer();
	auto hidden1 = pPlayer->IsPosHidden() ? pPlayer->GetPos() : TVector2<int32>( -1, -1 );
	auto hidden2 = pPlayer->IsToHidden() ? pPlayer->GetMoveTo() : TVector2<int32>( -1, -1 );
	int32 n = 0;

	for( int j = 0; j < levelSize.y; j++ )
	{
		for( int i = 0; i < levelSize.x; i++ )
		{
			if( !!( ( i + j ) & 1 ) )
				continue;
			auto pGrid = pLevel->GetGrid( TVector2<int32>( i, j ) );
			if( pGrid && pGrid->nAlertEft )
			{
				auto& data = globalData.vecAlertEffectParams[globalData.vecAlertEffectParams.size() - pGrid->nAlertEft];
				pGrid->nAlertEft--;

				m_elems.resize( m_elems.size() + 1 );
				auto& elem = m_elems.back();
				elem.rect = CRectangle( ( i + 1 ) * LEVEL_GRID_SIZE_X - data.ofs.x, j * LEVEL_GRID_SIZE_Y,
					2 * data.ofs.x, LEVEL_GRID_SIZE_Y + data.ofs.y );
				elem.texRect = CRectangle( 0, 0, 1, 1 );
				elem.nInstDataSize = sizeof( CVector4 ) * 2;
				elem.pInstData = &data.params;
			}
		}
	}
	for( int j = 0; j < levelSize.y; j++ )
	{
		int32 n1;
		if( j < levelSize.y - 1 )
			for( n1 = n; n1 < vec1.size() && vec1[n1].x < ( j + 1 ) * LEVEL_GRID_SIZE_Y; n1++ );
		else
			n1 = vec1.size();
		for( int i = 0; i < levelSize.x; i++ )
		{
			if( !!( ( i + j ) & 1 ) )
				continue;
			bool bHidden = TVector2<int32>( i, j ) == hidden1 || TVector2<int32>( i, j ) == hidden2;
			auto pGrid = pLevel->GetGrid( TVector2<int32>( i, j ) );
			if( pGrid )
			{
				if( pGrid->bStealthDetect || pGrid->bStealthAlert || bHidden )
				{
					auto& data = bHidden ? ( pGrid->bStealthDetect ? detectHiddenData : ( pGrid->bStealthAlert ? alertHiddenData : hiddenData ) )
						: ( pGrid->bStealthDetect ? detectData : alertData );
					if( bHidden )
					{
						m_elems.resize( m_elems.size() + 1 );
						auto& elem = m_elems.back();
						elem.rect = CRectangle( i * LEVEL_GRID_SIZE_X, j * LEVEL_GRID_SIZE_Y, 2 * LEVEL_GRID_SIZE_X, LEVEL_GRID_SIZE_Y );
						elem.rect = elem.rect.Offset( data.ofs );
						elem.texRect = CRectangle( 0, 0, 1, 1 );
						elem.nInstDataSize = sizeof( CVector4 ) * 2;
						elem.pInstData = &data.params;
					}
					else
					{
						for( int k = n; k < n1; k++ )
						{
							m_elems.resize( m_elems.size() + 1 );
							auto& elem = m_elems.back();
							elem.rect = CRectangle( i * LEVEL_GRID_SIZE_X, vec1[k].x, 2 * LEVEL_GRID_SIZE_X, vec1[k].y );
							elem.rect = elem.rect.Offset( data.ofs );
							elem.texRect = CRectangle( 0, 0, 1, 1 );
							elem.nInstDataSize = sizeof( CVector4 ) * 2;
							elem.pInstData = &data.params;
						}
					}
				}
			}
		}
		n = n1;
	}

	for( int i = 0; i < vec1.size(); i++ )
	{
		auto& item = vec1[i];
		auto& backData2 = globalData.vecStealthBackParams2[( ( m_nTick + i * 2 ) >> 2 ) % globalData.vecStealthBackParams2.size()];
		m_elems.resize( m_elems.size() + 1 );
		auto& elem = m_elems.back();
		elem.rect = CRectangle( r0.x, item.x, r0.width, item.y );
		elem.rect = elem.rect.Offset( backData2.ofs );
		elem.texRect = CRectangle( 0, 0, 1, 1 );
		elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.pInstData = &backData2.params;
	}
	for( int i = 0; i < vec0.size(); i++ )
	{
		auto& item = vec0[i];
		auto& backData1 = globalData.vecStealthBackParams1[( ( m_nTick + i * 2 ) >> 2 ) % globalData.vecStealthBackParams1.size()];
		m_elems.resize( m_elems.size() + 1 );
		auto& elem = m_elems.back();
		elem.rect = CRectangle( r0.x, item.x, r0.width, item.y );
		elem.rect = elem.rect.Offset( backData1.ofs );
		elem.texRect = CRectangle( 0, 0, 1, 1 );
		elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.pInstData = &backData1.params;
	}
	vec0.resize( 0 );
	vec1.resize( 0 );
	m_nTick++;
}

void CLevelStealthLayer::Render( CRenderContext2D& context )
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

void CLevelIndicatorLayer::OnAddedToStage()
{
	SetRenderObject( NULL );
}

void CLevelIndicatorLayer::Update( CMyLevel* pLevel )
{
	auto& globalData = CGlobalCfg::Inst().lvIndicatorData;
	auto& pawnParamData = globalData.vecPawnParams[m_nTick % globalData.vecPawnParams.size()];
	auto& pawn0ParamData = globalData.vecPawn0Params[m_nTick % globalData.vecPawn0Params.size()];
	auto& pawnFallParamData = globalData.vecPawnFallParams[m_nTick % globalData.vecPawnFallParams.size()];
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
	static vector<TVector3<int32> > vecPawn;
	static vector<TVector2<int32> > vecPawn0;
	static vector<TVector2<int32> > vecUse;
	static vector<SMount> vecMount;

	auto pPlayer = pLevel->GetPlayer();
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
				if( pGrid->bCanEnter && pGrid->nNextStage )
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
						pGrid->nHitEft > 0 ? pGrid->nHitEft-- : pGrid->nHitEft++;
					}
					if( pGrid->nMissEft )
					{
						SHit hit = { { i, j }, pGrid->nMissEft };
						vecMiss.push_back( hit );
						pGrid->nMissEft > 0 ? pGrid->nMissEft-- : pGrid->nMissEft++;
					}
					if( pGrid->nHitBlockedEft )
					{
						SHit hit = { { i, j }, pGrid->nHitBlockedEft };
						vecHitBlocked.push_back( hit );
						pGrid->nHitBlockedEft > 0 ? pGrid->nHitBlockedEft-- : pGrid->nHitBlockedEft++;
					}
					if( pGrid->nHitBashEft )
					{
						SHit hit = { { i, j }, pGrid->nHitBashEft };
						vecHitBash.push_back( hit );
						pGrid->nHitBashEft > 0 ? pGrid->nHitBashEft-- : pGrid->nHitBashEft++;
					}
					if( pGrid->nMissBashEft )
					{
						SHit hit = { { i, j }, pGrid->nMissBashEft };
						vecMissBash.push_back( hit );
						pGrid->nMissBashEft > 0 ? pGrid->nMissBashEft-- : pGrid->nMissBashEft++;
					}
					if( pGrid->pPawn0 )
					{
						if( pGrid->pPawn0->CheckHit( TVector2<int32>( i, j ), 1000 ) > 0 )
							vecPawn.push_back( TVector3<int32>( i, j, pGrid->pPawn0->IsSpecialState( CPawn::eSpecialState_Fall ) ? 1 : 0 ) );
						else if( pGrid->pPawn0->CanBeHit( 1000 ) )
							vecPawn0.push_back( TVector2<int32>( i, j ) );
					}
					if( pPlayer && pLevel->IsGridMoveable( TVector2<int32>( i, j ), pPlayer ) )
					{
						for( auto pMount = pGrid->Get_Mount(); pMount; pMount = pMount->NextMount() )
						{
							if( !pMount->IsEnabled() || pMount->IsHidden() || !pPlayer->IsReadyForMount( pMount ) )
								continue;
							auto pPawn = pMount->GetPawn();
							auto nDir = pMount->GetEnterDir();
							if( nDir <= 1 && pPawn->GetCurDir() )
								nDir = 1 - nDir;
							TRectangle<int32> r( pPawn->GetPosX(), pPawn->GetPosY(), pPawn->GetWidth(), pPawn->GetHeight() );
							r = r.Offset( pMount->GetPawnOfs() );
							TRectangle<int32> r1( i, j, 2, 1 );
							auto n1 = r1.x < r.x ? 1 : ( r1.GetRight() > r.GetRight() ? -1 : 0 );
							auto n2 = r1.y < r.y ? 1 : ( r1.GetBottom() > r.GetBottom() ? -1 : 0 );
							if( n1 > 0 )
								nDir = n2 > 0 ? 4 : ( n2 < 0 ? 6 : 0 );
							else if( n1 < 0 )
								nDir = n2 > 0 ? 5 : ( n2 < 0 ? 7 : 1 );
							else if( n2 > 0 )
								nDir = 2;
							else if( n2 < 0 )
								nDir = 3;
							SMount mount = { TVector2<int32>( i, j ), nDir };
							vecMount.push_back( mount );
						}
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
				auto nFrame = abs( hit.n );
				int32 nParam = globalData.vecHitParams.size() - Max<int32>( 1, nFrame - k );
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
				if( hit.n < 0 )
					elem.rect.y += 40 - nParam * 2;
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
		auto nFrame = abs( hit.n );
		int32 nParam = globalData.vecHitBlockedParams.size() - nFrame;
		auto& paramData = globalData.vecHitBlockedParams[nParam];
		auto& elem = AddElem( hit.p.x, hit.p.y, paramData.ofs, paramData.params );
		if( hit.n < 0 )
			elem.rect.y += 32 - nParam * 2;
	}
	for( auto& hit : vecMiss )
	{
		auto nFrame = abs( hit.n );
		int32 nParam = globalData.vecMissParams.size() - nFrame;
		auto& paramData = globalData.vecMissParams[nParam];
		auto& elem = AddElem( hit.p.x, hit.p.y, paramData.ofs, paramData.params );
		if( hit.n < 0 )
			elem.rect.y += 32 - nParam * 2;
	}
	for( auto& hit : vecMissBash )
	{
		auto nFrame = abs( hit.n );
		int32 nParam = globalData.vecMissParams.size() - nFrame;
		auto& paramData = globalData.vecMissParams[nParam];
		auto& elem = AddElem( hit.p.x, hit.p.y, paramData.ofs, paramData.params );
		elem.rect = CRectangle( elem.rect.x - nParam * 2, elem.rect.y - nParam * 2, elem.rect.width + nParam * 4, elem.rect.height + nParam * 4 );
		if( hit.n < 0 )
			elem.rect.y += 32 - nParam * 2;
	}
	if( pPlayer )
	{
		CVector2 dirs[8] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 }, { 1, 0 }, { -1, 0 } };
		for( auto& mount : vecMount )
		{
			auto ofs = dirs[mount.nDir] * mountParamData.ofs.x;
			auto l = mountParamData.ofs.y;
			CRectangle mountBaseRect[] = { { LEVEL_GRID_SIZE_X * 2 - 6, ( LEVEL_GRID_SIZE_Y - l ) / 2, 6, l },
			{ 0, ( LEVEL_GRID_SIZE_Y - l ) / 2, 6, l },
			{ LEVEL_GRID_SIZE_X - l / 2, LEVEL_GRID_SIZE_Y - 6, l, 6 },
			{ LEVEL_GRID_SIZE_X - l / 2, 0, l, 6 },
			{ LEVEL_GRID_SIZE_X * 2 - 6, LEVEL_GRID_SIZE_Y - l, 6, l },
			{ 0, LEVEL_GRID_SIZE_Y - l, 6, l },
			{ LEVEL_GRID_SIZE_X * 2 - 6, 0, 6, l },
			{ 0, 0, 6, l }, };
			m_elems.resize( m_elems.size() + 1 );
			auto& elem = m_elems.back();
			elem.rect = mountBaseRect[mount.nDir].Offset( ofs + CVector2( mount.p.x, mount.p.y ) * LEVEL_GRID_SIZE );
			elem.texRect = CRectangle( 0, 0, 1, 1 );
			elem.nInstDataSize = sizeof( CVector4 ) * 2;
			elem.pInstData = &mountParamData.params;
		}

		if( pPlayer->IsReadyToUse() )
		{
			pLevel->GetAllUseableGrid( vecUse );
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
		}
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
	{
		auto& param = p.z ? pawnFallParamData : pawnParamData;
		AddElem( p.x, p.y, param.ofs, param.params );
	}
	for( auto& p : vecPawn0 )
	{
		auto& param = pawn0ParamData;
		AddElem( p.x, p.y, param.ofs, param.params );
	}

	vecBlocked.resize( 0 );
	vecHit.resize( 0 );
	vecHitBlocked.resize( 0 );
	vecHitBash.resize( 0 );
	vecMiss.resize( 0 );
	vecMissBash.resize( 0 );
	for( int k = 0; k < 2; k++ )
		vecNxtStage[k].resize( 0 );
	vecPawn.resize( 0 );
	vecPawn0.resize( 0 );
	vecUse.resize( 0 );
	vecMount.resize( 0 );
	m_nTick++;

	if( pPlayer && pPlayer->IsHidden() && !pLevel->IsScenario() )
	{
		m_pStealthLayer->bVisible = true;
		m_pStealthLayer->Update( pLevel );
	}
	else
	{
		m_pStealthLayer->bVisible = false;
		for( int i = 0; i < levelSize.x; i++ )
		{
			for( int j = 0; j < levelSize.y; j++ )
			{
				auto pGrid = pLevel->GetGrid( TVector2<int32>( i, j ) );
				if( pGrid )
					pGrid->nAlertEft = 0;
			}
		}
	}
	if( m_pStealthLayer->bVisible )
	{
		for( auto& item : m_mapPawnTarget )
		{
			auto& item1 = m_mapPawnTargetEft[item.first];
			if( item1.pEft && !item1.pEft->GetParentEntity() )
				item1.pEft = NULL;
			if( !item1.pEft || item1.target != item.second )
			{
				if( !item1.pEft )
				{
					item1.pEft = SafeCast<CEntity>( m_pPawnTargetEftPrefab->GetRoot()->CreateInstance() );
					item1.pEft->SetParentBeforeEntity( m_pStealthLayer );
				}
				auto pLightning = SafeCast<CLightningEffect>( item1.pEft.GetPtr() );
				auto p = item.first->GetPos() + item.first->GetMoveTo();
				auto pos = CVector2( p.x * 0.5f + item.first->GetWidth() * 0.5f, p.y * 0.5f + item.first->GetHeight() * 0.5f ) * LEVEL_GRID_SIZE;
				pos.y += 32;
				pLightning->SetPosition( pos );
				auto ofs = CVector2( item.second.x + 0.5f, item.second.y + 0.5f ) * LEVEL_GRID_SIZE - pos;
				auto p1 = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
				pLightning->Set( p1, 15 );
				item1.target = item.second;
			}
		}
		for( auto& item : m_mapPawnTargetEft )
		{
			if( m_mapPawnTarget.find( item.first ) == m_mapPawnTarget.end() )
			{
				if( item.second.pEft )
				{
					item.second.pEft->SetParentEntity( NULL );
					item.second.pEft = NULL;
				}
			}
		}
	}
	else
	{
		for( auto& item : m_mapPawnTargetEft )
		{
			if( item.second.pEft )
				item.second.pEft->SetParentEntity( NULL );
		}
		m_mapPawnTargetEft.clear();
	}
	m_mapPawnTarget.clear();
}

void CLevelIndicatorLayer::UpdatePawnTarget( CPawn* pPawn, const TVector2<int32>& target )
{
	m_mapPawnTarget[pPawn] = target;
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
		m_vecGrid[i].bBlockSight = m_arrGridData[i].bBlocked;
		m_vecGrid[i].nNextStage = m_arrGridData[i].nNextStage;
		m_vecGrid[i].nTile = m_arrGridData[i].nTile;
	}
}

void CMyLevel::OnRemovedFromStage()
{
	if( !m_bSnapShot )
	{
		while( m_spawningPawns.Get_Pawn() )
			RemovePawn( m_spawningPawns.Get_Pawn() );
		for( int i = 0; i < m_vecPawnHits.size(); i++ )
			RemovePawn( m_vecPawnHits[i] );
		m_vecPawnHits.resize( 0 );
		while( m_pPawns )
			RemovePawn( m_pPawns );
	}
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

int32 CMyLevel::CheckGrid( int32 x, int32 y, CPawn* pPawn, int8 nForceCheckType )
{
	auto pGrid = GetGrid( TVector2<int32>( x, y ) );
	if( !pGrid || !pGrid->bCanEnter )
		return 0;
	int32 n = 3;
	if( pGrid->pPawn0 )
	{
		bool bCheck = false;
		if( nForceCheckType == 1 )
		{
			if( pGrid->pPawn0.GetPtr() != pPawn->m_pCreator )
				bCheck = true;
		}
		else if( nForceCheckType == 2 )
		{
			if( pGrid->pPawn0.GetPtr() != pPawn->m_pCreator && !pGrid->pPawn0->IsSpecialState( CPawn::eSpecialState_Fall ) )
				bCheck = true;
		}
		else
			bCheck = true;
		if( bCheck )
			n = Min( n, pGrid->pPawn0->IsDynamic() ? 2 : 1 );
	}
	if( pGrid->pPawn1 && pGrid->pPawn1.GetPtr() != pGrid->pPawn0 )
	{
		bool bCheck = false;
		if( nForceCheckType == 1 )
		{
			if( pGrid->pPawn1.GetPtr() != pPawn->m_pCreator )
				bCheck = true;
		}
		else if( nForceCheckType == 2 )
		{
			if( pGrid->pPawn1.GetPtr() != pPawn->m_pCreator && !pGrid->pPawn1->IsSpecialState( CPawn::eSpecialState_Fall ) )
				bCheck = true;
		}
		else
			bCheck = true;
		if( bCheck )
			n = Min( n, pGrid->pPawn1->IsDynamic() ? 2 : 1 );
	}
	return n;
}

CMasterLevel* CMyLevel::GetMasterLevel()
{
	if( IsSnapShot() )
		return SafeCast<CMasterLevel>( GetParentEntity()->GetParentEntity() );
	return SafeCast<CMasterLevel>( GetParentEntity() );
}

void CMyLevel::SetEnvEffect( const char * sz )
{
	if( IsSnapShot() )
		return;
	CLevelEnvEffect* p = m_mapEnvEffects[sz];
	if( p != m_pEnvEffect )
	{
		if( p && GetMasterLevel() )
		{
			if( m_pEnvEffect )
				p->SetParentBeforeEntity( m_pEnvEffect );
			else
				p->SetParentBeforeEntity( GetMasterLevel()->GetMainUI() );
		}
		if( m_pEnvEffect )
			m_pEnvEffect->SetParentEntity( NULL );
		if( p && m_pEnvEffect )
		{
			p->CopyState( m_pEnvEffect );
		}
		m_pEnvEffect = p;
	}
}

void CMyLevel::Begin()
{
	if( GetMasterLevel() )
	{
		auto& worldData = GetMasterLevel()->GetWorldData().curFrame;
		if( worldData.nTracerSpawnDelay )
		{
			m_nTracerDelayLeft = worldData.nTracerSpawnDelay;
			auto grid = worldData.playerEnterPos;
			if( worldData.strTracerLevel.length() )
			{
				if( worldData.strTracerLevel == worldData.strCurLevel )
				{
					grid = worldData.tracerLevelEnterPos;
					worldData.strTracerLevel = "";
				}
				else
				{
					bool bOK = false;
					for( int i = 0; i < m_nWidth && !bOK; i++ )
					{
						for( int j = 0; j < m_nHeight; j++ )
						{
							if( !!( ( i ^ j ) & 1 ) )
								continue;
							auto nNext = GetGrid( TVector2<int32>( i, j ) )->nNextStage;
							if( nNext )
							{
								auto& nextData = GetNextLevelData( nNext - 1 );
								if( strcmp( nextData.pNxtStage->GetName(), worldData.strTracerLevel.c_str() ) )
									continue;
								TVector2<int32> p( i - nextData.nOfsX, j - nextData.nOfsY );
								if( p == worldData.tracerLevelEnterPos )
								{
									grid = TVector2<int32>( i, j );
									worldData.strTracerLevel = "";
									bOK = true;
									break;
								}
							}
						}
					}
					if( !bOK )
						m_nTracerDelayLeft = 0;
				}
			}

			if( m_nTracerDelayLeft )
			{
				m_tracerEnterPos = grid;
				auto pGrid = GetGrid( grid );
				m_nTracerSpawnExit = pGrid->nNextStage;
				m_pTracerSpawnEffect = SafeCast<CEntity>( CGlobalCfg::Inst().pTracerSpawnEftPrefab->GetRoot()->CreateInstance() );
				m_pTracerSpawnEffect->SetParentEntity( this );
				m_pTracerSpawnEffect->SetPosition( CVector2( grid.x + 1, grid.y ) * LEVEL_GRID_SIZE );
			}
		}
	}

	FlushSpawn();
	for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
		HandlePawnMounts( pPawn, false );
	for( auto& pPawn : m_vecPawnHits )
		HandlePawnMounts( pPawn, false );
	m_pIndicatorLayer = SafeCast<CLevelIndicatorLayer>( CGlobalCfg::Inst().lvIndicatorData.pIndicatorPrefab->GetRoot()->CreateInstance() );
	m_pIndicatorLayer->SetParentBeforeEntity( m_pPawnRoot );
	m_bBegin = true;
	m_beginTrigger.Trigger( 0, this );
	m_beginTrigger.Clear();
	if( m_strBeginScript.length() )
		CLuaMgr::Inst().Run( m_strBeginScript );
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnBegin( this );
	if( GetMasterLevel() )
		GetMasterLevel()->GetMainUI()->OnLevelBegin();
}

void CMyLevel::End()
{
	m_bEnd = true;
	if( m_pActionPreviewCoroutine )
	{
		m_pActionPreviewCoroutine->Resume();
		TCoroutinePool<0x10000>::Inst().Free( m_pActionPreviewCoroutine );
		m_pActionPreviewCoroutine = NULL;
	}

	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		if( pPawn != m_pPlayer )
			pPawn->OnLevelEnd();
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )
	for( int i = 0; i < m_vecPawnHits.size(); i++ )
	{
		auto p = m_vecPawnHits[i];
		if( p && p->GetParentEntity() )
		{
			p->OnLevelEnd();
		}
	}
	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		if( pPawn != m_pPlayer )
			pPawn->OnLevelSave();
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )
	for( int i = 0; i < m_vecPawnHits.size(); i++ )
	{
		auto p = m_vecPawnHits[i];
		if( p && p->GetParentEntity() )
			p->OnLevelSave();
	}
	if( GetMasterLevel() )
	{
		GetMasterLevel()->GetMainUI()->ShowFailEft( false );
		GetMasterLevel()->GetMainUI()->ShowFreezeEft( 0 );
	}
	m_beginTrigger.Clear();
	m_trigger.Clear();
	EndNoise();
	if( m_pIndicatorLayer )
	{
		m_pIndicatorLayer->SetParentEntity( NULL );
		m_pIndicatorLayer = NULL;
	}

	if( m_strDestroyScript.length() )
		CLuaMgr::Inst().Run( m_strDestroyScript );
	for( int i = m_vecScripts.size() - 1; i >= 0; i-- )
		m_vecScripts[i]->OnDestroy( this );
}

void CMyLevel::Fail( int8 nFailType )
{
	if( IsActionPreview() )
	{
		m_pActionPreviewCoroutine->Yield( 2 );
		throw( 1 );
	}
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
		GetMasterLevel()->InterferenceStripEffect( 1, 1 );
	}

	m_bFailed = true;
	GetMasterLevel()->ForceEndScenario();
	GetMasterLevel()->GetMainUI()->ShowFailEft( true );
}

void CMyLevel::Freeze()
{
	m_nFreeze++;
}

void CMyLevel::UnFreeze()
{
	m_nFreeze--;
}

CPawn* CMyLevel::SpawnPawn( int32 n, int32 x, int32 y, int8 nDir, const char* szRemaining, CPawn* pCreator, int32 nForm )
{
	if( n < 0 || n >= m_arrSpawnPrefab.Size() )
		return NULL;
	CReference<CPawn> pPawn = SafeCast<CPawn>( m_arrSpawnPrefab[n]->GetRoot()->CreateInstance() );
	if( szRemaining && szRemaining[0] )
	{
		auto pSpawnHelper = new CLevelSpawnHelper( n, szRemaining, pPawn->GetStateIndexByName( "death" ) );
		pPawn->SetName( szRemaining );
		pSpawnHelper->m_nDataType = pPawn->m_nLevelDataType;
		pPawn->m_pSpawnHelper = pSpawnHelper;
	}
	pPawn->SetPosition( CVector2( x, y ) * LEVEL_GRID_SIZE );
	if( !AddPawn( pPawn, TVector2<int32>( x, y ), nDir, pCreator, nForm ) )
		return NULL;
	pPawn->strCreatedFrom = m_arrSpawnPrefab[n]->GetName();
	return pPawn;
}

CPawn* CMyLevel::SpawnPawn1( const char* szPrefab, int32 x, int32 y, int8 nDir, CPawn* pCreator, int32 nForm )
{
	CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
	if( !pPrefab || !pPrefab->GetRoot()->GetStaticDataSafe<CPawn>() )
		return NULL;
	CReference<CPawn> pPawn = SafeCast<CPawn>( pPrefab->GetRoot()->CreateInstance() );
	pPawn->SetPosition( CVector2( x, y ) * LEVEL_GRID_SIZE );
	if( !AddPawn( pPawn, TVector2<int32>( x, y ), nDir, pCreator, nForm ) )
		return NULL;
	pPawn->strCreatedFrom = pPrefab->GetName();
	return pPawn;
}

CPawn* CMyLevel::SpawnPreset( const char* szName )
{
	for( auto& pSpawner : m_vecSpawner )
	{
		if( pSpawner && pSpawner->m_nSpawnType >= 2 && pSpawner->GetName() == szName )
		{
			if( pSpawner->m_nSpawnType == 2 )
			{
				auto pPawn = HandleSpawn( pSpawner );
				if( pPawn )
				{
					pSpawner = NULL;
					return pPawn;
				}
			}
			else
			{
				CReference<CLevelSpawnHelper> pSpawnHelper = SafeCast<CLevelSpawnHelper>( pSpawner->GetInstanceOwnerNode()->CreateInstance() );
				return HandleSpawn( pSpawnHelper );
			}
		}
	}
	return NULL;
}

CPawn* CMyLevel::SpawnPreset1( const char* szName, int32 x, int32 y, int8 nDir, const char* szInitState )
{
	for( auto& pSpawner : m_vecSpawner )
	{
		if( pSpawner && pSpawner->m_nSpawnType >= 2 && pSpawner->GetName() == szName )
		{
			if( pSpawner->m_nSpawnType == 2 )
			{
				auto pPawn = HandleSpawn1( pSpawner, TVector2<int32>( x, y ), nDir, szInitState );
				if( pPawn )
				{
					pSpawner = NULL;
					return pPawn;
				}
			}
			else
			{
				CReference<CLevelSpawnHelper> pSpawnHelper = SafeCast<CLevelSpawnHelper>( pSpawner->GetInstanceOwnerNode()->CreateInstance() );
				return HandleSpawn1( pSpawnHelper, TVector2<int32>( x, y ), nDir, szInitState );
			}
		}
	}
	return NULL;
}

int32 CMyLevel::GetPresetSpawnX( const char* szName )
{
	for( auto& pSpawner : m_vecSpawner )
	{
		if( pSpawner && pSpawner->m_nSpawnType >= 2 && pSpawner->GetName() == szName )
		{
			return floor( pSpawner->x / LEVEL_GRID_SIZE_X + 0.5f );
		}
	}
	return -1;
}

int32 CMyLevel::GetPresetSpawnY( const char* szName )
{
	for( auto& pSpawner : m_vecSpawner )
	{
		if( pSpawner && pSpawner->m_nSpawnType >= 2 && pSpawner->GetName() == szName )
		{
			return floor( pSpawner->y / LEVEL_GRID_SIZE_Y + 0.5f );
		}
	}
	return -1;
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
				if( !pGrid || pGrid->pPawn0 || !pGrid->bCanEnter )
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
	pPawn->m_bPosHidden = pPawn->m_bMoveToHidden = false;
	pPawn->m_nCurDir = nDir;
	pPawn->m_pCreator = pCreator;
	if( !pPawnHit )
	{
		for( int i = 0; i < nPawnWidth; i++ )
		{
			for( int j = 0; j < nPawnHeight; j++ )
			{
				auto pGrid = GetGrid( pos + TVector2<int32>( i, j ) );
				pGrid->pPawn0 = pGrid->pPawn1 = pPawn;
			}
		}
		if( !m_pPlayer )
		{
			auto pPlayer = SafeCast<CPlayer>( pPawn );
			if( pPlayer && pPlayer->IsRealPlayer() )
				m_pPlayer = pPlayer;
		}
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

bool CMyLevel::AddPawn1( CPawn* pPawn, int32 nState, int32 nStateTick, const TVector2<int32>& pos, const TVector2<int32>& moveTo, int8 nDir )
{
	pPawn->bVisible = false;
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	int32 nPawnWidth = pPawn->m_nWidth;
	int32 nPawnHeight = pPawn->m_nHeight;
	int32 nForm = 0;
	if( pPawn->m_arrForms.Size() )
	{
		nForm = pPawn->m_arrSubStates[nState].nForm;
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
				if( !pGrid || pGrid->pPawn0 || !pGrid->bCanEnter )
					return false;
				pGrid = GetGrid( moveTo + TVector2<int32>( i, j ) );
				if( !pGrid || pGrid->pPawn0 || !pGrid->bCanEnter )
					return false;
			}
		}
	}

	auto pSpawnHelper = pPawn->m_pSpawnHelper;
	pSpawnHelper->m_bInitState = true;
	pSpawnHelper->m_nInitState = nState;
	pSpawnHelper->m_nInitStateTick = nStateTick;
	if( pPawn->m_arrForms.Size() )
	{
		pPawn->m_nCurForm = nForm;
		pPawn->m_nWidth = nPawnWidth;
		pPawn->m_nHeight = nPawnHeight;
	}
	pPawn->m_pos = pos;
	pPawn->m_moveTo = moveTo;
	pPawn->m_bPosHidden = pPawn->m_bMoveToHidden = false;
	pPawn->m_nCurDir = nDir;
	if( !pPawnHit )
	{
		for( int i = 0; i < nPawnWidth; i++ )
		{
			for( int j = 0; j < nPawnHeight; j++ )
			{
				auto pGrid = GetGrid( pos + TVector2<int32>( i, j ) );
				pGrid->pPawn0 = pGrid->pPawn1 = pPawn;
				pGrid = GetGrid( moveTo + TVector2<int32>( i, j ) );
				pGrid->pPawn0 = pGrid->pPawn1 = pPawn;
			}
		}
		if( !m_pPlayer )
		{
			auto pPlayer = SafeCast<CPlayer>( pPawn );
			if( pPlayer && pPlayer->IsRealPlayer() )
				m_pPlayer = pPlayer;
		}
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
	pPawn->OnRemovedFromLevel();
	HandlePawnMounts( pPawn, true );
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	if( !pPawnHit )
	{
		if( pPawn == m_pPlayer )
			m_pPlayer = NULL;
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
				if( pGrid->pPawn0 == pPawn )
					pGrid->pPawn0 = NULL;
				if( pGrid->pPawn1 == pPawn )
					pGrid->pPawn1 = NULL;
				if( !pGrid->pPawn0 && pGrid->pPawn1 )
					pGrid->pPawn0 = pGrid->pPawn1;
				if( !pGrid->pPawn1 && pGrid->pPawn0 )
					pGrid->pPawn1 = pGrid->pPawn0;
			}
		}
		if( pPawn->m_moveTo != pPawn->m_pos )
		{
			for( int i = 0; i < pPawn->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn->m_nHeight; j++ )
				{
					auto pGrid1 = GetGrid( pPawn->m_moveTo + TVector2<int32>( i, j ) );
					if( pGrid1->pPawn0 == pPawn )
						pGrid1->pPawn0 = NULL;
					if( pGrid1->pPawn1 == pPawn )
						pGrid1->pPawn1 = NULL;
					if( !pGrid1->pPawn0 && pGrid1->pPawn1 )
						pGrid1->pPawn0 = pGrid1->pPawn1;
					if( !pGrid1->pPawn1 && pGrid1->pPawn0 )
						pGrid1->pPawn1 = pGrid1->pPawn0;
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

bool CMyLevel::IsGridMoveable( const TVector2<int32>& p, CPawn* pPawn, int8 nForceCheckType )
{
	auto pGrid = GetGrid( p );
	if( !pGrid || !pGrid->bCanEnter )
		return false;
	if( pGrid->pPawn0 && pGrid->pPawn0 != pPawn && pGrid->pPawn1 != pPawn )
	{
		bool bOK = false;
		auto pPlayer = SafeCast<CPlayer>( pPawn );
		if( pPlayer && pGrid->pPawn0 == pPlayer->GetCurMountingPawn() )
			bOK = true;
		else
		{
			pPlayer = SafeCast<CPlayer>( pGrid->pPawn0.GetPtr() );
			if( pPlayer )
			{
				if( pPawn == pPlayer->GetCurMountingPawn() )
					bOK = true;
			}
		}
		if( !bOK )
		{
			if( nForceCheckType == 1 )
			{
				if( pGrid->pPawn0.GetPtr() != pPawn->m_pCreator )
					return false;
			}
			else if( nForceCheckType == 2 )
			{
				if( pGrid->pPawn0.GetPtr() != pPawn->m_pCreator && !pGrid->pPawn0->IsSpecialState( CPawn::eSpecialState_Fall ) )
					return false;
			}
			else
				return false;
		}
	}
	if( pPawn->IsNextStageBlock() && pGrid->nNextStage > 0 )
		return false;
	if( !pPawn->IsIgnoreBlockedExit() && IsGridBlockedExit( pGrid ) )
		return false;
	return true;
}

bool CMyLevel::IsGridBlockSight( const TVector2<int32>& p )
{
	auto pGrid = GetGrid( p );
	if( !pGrid || pGrid->bBlockSight )
		return true;
	if( pGrid->pPawn1 && pGrid->pPawn1->IsBlockSight() )
		return true;
	return false;
}

bool CMyLevel::PawnMoveTo( CPawn* pPawn, const TVector2<int32>& ofs, int8 nForceCheckType, int32 nMoveFlag, bool bBlockEft )
{
	if( !( pPawn->m_moveTo == pPawn->m_pos && ( ofs.x || ofs.y ) ) )
		return false;
	auto moveTo = pPawn->m_pos + ofs;

	bool bOK = true;
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	int8 nMountMoveType = nMoveFlag & 3;
	bool bHidden = !!( nMoveFlag & 4 );
	if( ( !pPawnHit || nForceCheckType ) && nMountMoveType != 2 )
	{
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				if( !IsGridMoveable( moveTo + TVector2<int32>( i, j ), pPawn, nForceCheckType ) )
				{
					bOK = false;
					if( bBlockEft )
					{
						auto pGrid1 = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
						pGrid1->nBlockEft = CGlobalCfg::Inst().lvIndicatorData.vecBlockedParams.size();
						pGrid1->blockOfs = ofs;
					}
				}
			}
		}
	}
	CPawn* pPawn1 = NULL;
	CPawnHit* pPawnHit1 = NULL;
	TVector2<int32> moveTo1;
	auto pPlayer = SafeCast<CPlayer>( pPawn );
	if( pPlayer )
		pPawn1 = pPlayer->GetCurMountingPawn();
	if( pPawn1 )
	{
		moveTo1 = pPawn1->m_pos + ofs;
		pPawnHit1 = SafeCast<CPawnHit>( pPawn1 );
		if( ( !pPawnHit1 || nForceCheckType ) && nMountMoveType != 1 )
		{
			for( int i = 0; i < pPawn1->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn1->m_nHeight; j++ )
				{
					if( !IsGridMoveable( moveTo1 + TVector2<int32>( i, j ), pPawn1, nForceCheckType ) )
					{
						bOK = false;
						if( bBlockEft )
						{
							auto pGrid1 = GetGrid( pPawn1->m_pos + TVector2<int32>( i, j ) );
							pGrid1->nBlockEft = CGlobalCfg::Inst().lvIndicatorData.vecBlockedParams.size();
							pGrid1->blockOfs = ofs;
						}
					}
				}
			}
		}
	}

	if( !bOK )
		return false;

	if( nMountMoveType != 2 )
	{
		pPawn->m_moveTo = moveTo;
		pPawn->m_bMoveToHidden = bHidden;
		if( !pPawnHit )
		{
			for( int i = 0; i < pPawn->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( moveTo + TVector2<int32>( i, j ) );
					pGrid->pPawn0 = pPawn;
					if( !pGrid->pPawn1 )
						pGrid->pPawn1 = pPawn;
				}
			}
		}
	}
	if( pPawn1 && nMountMoveType != 1 )
	{
		pPawn1->m_moveTo = moveTo1;
		if( !pPawnHit1 )
		{
			for( int i = 0; i < pPawn1->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn1->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( moveTo1 + TVector2<int32>( i, j ) );
					pGrid->pPawn1 = pPawn1;
					if( !pGrid->pPawn0 )
						pGrid->pPawn0 = pPawn1;
				}
			}
		}
	}
	return true;
}

void CMyLevel::PawnMoveEnd( CPawn* pPawn )
{
	ASSERT( pPawn->m_moveTo != pPawn->m_pos );

	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	CPawn* pPawn1 = NULL;
	CPawnHit* pPawnHit1 = NULL;
	auto pPlayer = SafeCast<CPlayer>( pPawn );
	if( pPlayer )
		pPawn1 = pPlayer->GetCurMountingPawn();
	if( pPawn1 )
		pPawnHit1 = SafeCast<CPawnHit>( pPawn1 );

	if( pPawn1 && !pPawnHit1 )
	{
		for( int i = 0; i < pPawn1->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn1->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn1->m_pos + TVector2<int32>( i, j ) );
				pGrid->pPawn0 = pGrid->pPawn1 = NULL;
			}
		}
	}
	if( !pPawnHit )
	{
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
				pGrid->pPawn0 = pGrid->pPawn1 = NULL;
			}
		}
	}
	if( pPawn1 && !pPawnHit1 )
	{
		for( int i = 0; i < pPawn1->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn1->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn1->m_moveTo + TVector2<int32>( i, j ) );
				pGrid->pPawn1 = pPawn1;
				if( !pGrid->pPawn0 )
					pGrid->pPawn0 = pPawn1;
			}
		}
	}
	if( !pPawnHit )
	{
		for( int i = 0; i < pPawn->m_nWidth; i++ )
		{
			for( int j = 0; j < pPawn->m_nHeight; j++ )
			{
				auto pGrid = GetGrid( pPawn->m_moveTo + TVector2<int32>( i, j ) );
				pGrid->pPawn0 = pPawn;
				if( !pGrid->pPawn1 )
					pGrid->pPawn1 = pPawn;
			}
		}
	}

	pPawn->m_pos = pPawn->m_moveTo;
	pPawn->m_bPosHidden = pPawn->m_bMoveToHidden;
	if( pPawn1 )
		pPawn1->m_pos = pPawn1->m_moveTo;
}

void CMyLevel::PawnMoveBreak( CPawn* pPawn )
{
	auto pPawnHit = SafeCast<CPawnHit>( pPawn );
	CPawn* pPawn1 = NULL;
	CPawnHit* pPawnHit1 = NULL;
	auto pPlayer = SafeCast<CPlayer>( pPawn );
	if( pPlayer )
		pPawn1 = pPlayer->GetCurMountingPawn();
	if( pPawn1 )
		pPawnHit1 = SafeCast<CPawnHit>( pPawn1 );
	
	if( pPawn1 && !pPawnHit1 )
	{
		if( pPawn1->m_moveTo != pPawn1->m_pos )
		{
			for( int i = 0; i < pPawn1->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn1->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( pPawn1->m_moveTo + TVector2<int32>( i, j ) );
					pGrid->pPawn0 = pGrid->pPawn1 = NULL;
				}
			}
		}
	}
	if( !pPawnHit )
	{
		if( pPawn->m_moveTo != pPawn->m_pos )
		{
			for( int i = 0; i < pPawn->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( pPawn->m_moveTo + TVector2<int32>( i, j ) );
					pGrid->pPawn0 = pGrid->pPawn1 = NULL;
				}
			}
		}
	}
	if( pPawn1 && pPawn1->m_moveTo != pPawn1->m_pos )
	{
		if( !pPawnHit1 )
		{
			for( int i = 0; i < pPawn1->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn1->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( pPawn1->m_pos + TVector2<int32>( i, j ) );
					pGrid->pPawn1 = pPawn1;
					if( !pGrid->pPawn0 )
						pGrid->pPawn0 = pPawn1;
				}
			}
		}
		pPawn1->m_moveTo = pPawn1->m_pos;
	}
	if( pPawn->m_moveTo != pPawn->m_pos )
	{
		if( !pPawnHit )
		{
			for( int i = 0; i < pPawn->m_nWidth; i++ )
			{
				for( int j = 0; j < pPawn->m_nHeight; j++ )
				{
					auto pGrid = GetGrid( pPawn->m_pos + TVector2<int32>( i, j ) );
					pGrid->pPawn0 = pPawn;
					if( !pGrid->pPawn1 )
						pGrid->pPawn1 = pPawn;
				}
			}
		}
		pPawn->m_moveTo = pPawn->m_pos;
		pPawn->m_bMoveToHidden = pPawn->m_bPosHidden;
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

bool CMyLevel::PawnTransform( CPawn* pPawn, int32 nForm, const TVector2<int32>& ofs, bool bBlockEft )
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
				if( !IsGridMoveable( TVector2<int32>( i, j ), pPawn ) )
				{
					bOK = false;
					if( bBlockEft )
					{
						auto pGrid = GetGrid( TVector2<int32>( i, j ) );
						pGrid->nBlockEft = CGlobalCfg::Inst().lvIndicatorData.vecBlockedParams.size();
						pGrid->blockOfs = d;
					}
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
				if( pGrid->pPawn0 == pPawn )
					pGrid->pPawn0 = NULL;
				if( pGrid->pPawn1 == pPawn )
					pGrid->pPawn1 = NULL;
				if( !pGrid->pPawn0 && pGrid->pPawn1 )
					pGrid->pPawn0 = pGrid->pPawn1;
				if( !pGrid->pPawn1 && pGrid->pPawn0 )
					pGrid->pPawn1 = pGrid->pPawn0;
			}
		}
		for( int i = r1.x; i < r1.GetRight(); i++ )
		{
			for( int j = r1.y; j < r1.GetBottom(); j++ )
			{
				auto pGrid = GetGrid( TVector2<int32>( i, j ) );
				pGrid->pPawn0 = pGrid->pPawn1 = pPawn;
			}
		}
	}
	return true;
}

bool CMyLevel::IsPawnInTile( CPawn* pPawn, int32 nTile )
{
	for( int k = 0; k < 2; k++ )
	{
		auto p = k == 0 ? pPawn->GetPos() : pPawn->GetMoveTo();
		for( int x = p.x; x < p.x + pPawn->GetWidth(); x++ )
		{
			for( int y = p.y; y < p.y + pPawn->GetHeight(); y++ )
			{
				auto pGrid = GetGridData( TVector2<int32>( x, y ) );
				if( !pGrid || pGrid->nTile != nTile )
					return false;
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

CPlayerMount* CMyLevel::FindMount( const TVector2<int32>& ofs )
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
				auto pPawn = pMount->GetPawn();
				if( ofs != TVector2<int32>( 0, 0 ) )
				{
					auto pGrid1 = GetGrid( TVector2<int32>( i, j ) + ofs );
					TRectangle<int32> r( pPawn->GetPosX(), pPawn->GetPosY(), pPawn->GetWidth(), pPawn->GetHeight() );
					r = r.Offset( pMount->GetPawnOfs() );
					TRectangle<int32> r0( i, j, 2, 1 );
					TRectangle<int32> r1( i + ofs.x, j + ofs.y, 2, 1 );
					r0 = r0 * r;
					r1 = r1 * r;
					if( !( ( r0.width <= 0 || r0.height <= 0 ) && r1.width > 0 && r1.height > 0 ) )
						continue;
				}
				else if( pMount->IsHidden() )
					continue;
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
		if( pGrid && pGrid->pPawn0 )
		{
			auto pUsage = pGrid->pPawn0->GetUsage();
			if( pUsage && pUsage->CheckUse( m_pPlayer ) )
				return pGrid->pPawn0;
		}
	}
	return NULL;
}

CPawn* CMyLevel::GetPawnByName( const char* szName )
{
	for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->GetLevel() == this && pPawn->GetName() == szName )
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

int32 CMyLevel::GetAllPawnsByNameScript( const char * szName )
{
	auto pLuaState = CLuaMgr::GetCurLuaState();
	pLuaState->NewTable();

	int32 n = 0;
	for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->GetLevel() == this && pPawn->GetName() == szName )
		{
			pLuaState->PushLua( pPawn );
			pLuaState->SetTableIndex( -2, ++n );
		}
	}
	for( auto& pPawn : m_vecPawnHits )
	{
		if( pPawn->GetName() == szName )
		{
			pLuaState->PushLua( pPawn );
			pLuaState->SetTableIndex( -2, ++n );
		}
	}
	for( auto pPawn = m_spawningPawns.Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->GetName() == szName )
		{
			pLuaState->PushLua( pPawn );
			pLuaState->SetTableIndex( -2, ++n );
		}
	}

	return 1;
}

int32 CMyLevel::GetAllPawnsByTagScript( const char* szTag )
{
	auto pLuaState = CLuaMgr::GetCurLuaState();
	pLuaState->NewTable();

	int32 n = 0;
	for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->GetLevel() == this && pPawn->HasTag( szTag ) )
		{
			pLuaState->PushLua( pPawn );
			pLuaState->SetTableIndex( -2, ++n );
		}
	}
	for( auto& pPawn : m_vecPawnHits )
	{
		if( pPawn->HasTag( szTag ) )
		{
			pLuaState->PushLua( pPawn );
			pLuaState->SetTableIndex( -2, ++n );
		}
	}
	for( auto pPawn = m_spawningPawns.Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
	{
		if( pPawn->HasTag( szTag ) )
		{
			pLuaState->PushLua( pPawn );
			pLuaState->SetTableIndex( -2, ++n );
		}
	}

	return 1;
}

CPawn* CMyLevel::GetPawnByGrid( int32 x, int32 y )
{
	auto pGrid = GetGrid( TVector2<int32>( x, y ) );
	if( !pGrid )
		return NULL;
	return pGrid->pPawn0;
}

void CMyLevel::OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir )
{
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnPlayerChangeState( state, nStateSource, nDir );
}

void CMyLevel::OnPlayerAction( vector<int8>& vecInput, int32 nMatchLen, int8 nType )
{
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnPlayerAction( vecInput, nMatchLen, nType );
}

TVector2<int32> CMyLevel::SimpleFindPath( const TVector2<int32>& begin, const TVector2<int32>& end, int32 nCheckFlag, vector<TVector2<int32> >* pVecPath, TVector2<int32>* pOfs, int32 nOfs )
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
			else if( !!( nCheckFlag & 1 ) && ( pGrid->pPawn0 && !pGrid->pPawn0->nTempFlag || pGrid->pPawn1 && !pGrid->pPawn1->nTempFlag ) )
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
	if( !pOfs )
	{
		SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );
		pOfs = ofs;
		nOfs = ELEM_COUNT( ofs );
	}
	bool bFind = false;
	for( int32 i = 0; i < q.size(); i++ )
	{
		auto p = q[i];
		for( int j = 0; j < nOfs; j++ )
		{
			auto p1 = p + pOfs[j];
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

TVector2<int32> CMyLevel::FindPath1( const TVector2<int32>& begin, const TVector2<int32>& end, function<bool( SGrid*, const TVector2<int32>& )> FuncGrid, vector<TVector2<int32>>* pVecPath, TVector2<int32>* pOfs, int32 nOfs )
{
	vector<int8> vec;
	vec.resize( m_nWidth * m_nHeight );
	for( int i = 0; i < m_nWidth; i++ )
	{
		for( int j = 0; j < m_nHeight; j++ )
		{
			auto pGrid = GetGrid( TVector2<int32>( i, j ) );
			if( !FuncGrid( pGrid, TVector2<int32>( i, j ) ) )
				vec[i + j * m_nWidth] = 1;
		}
	}
	vector<TVector2<int32> > q;
	vector<int32> par;
	q.push_back( begin );
	par.push_back( -1 );
	vec[begin.x + begin.y * m_nWidth] = 1;
	TVector2<int32> ofs[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 }, { -1, 1 }, { -1, -1 } };
	if( !pOfs )
	{
		SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );
		pOfs = ofs;
		nOfs = ELEM_COUNT( ofs );
	}
	bool bFind = false;
	for( int32 i = 0; i < q.size(); i++ )
	{
		auto p = q[i];
		for( int j = 0; j < nOfs; j++ )
		{
			auto p1 = p + pOfs[j];
			if( p1.x < 0 || p1.y < 0 || p1.x >= m_nWidth || p1.y >= m_nHeight )
				continue;
			if( vec[p1.x + p1.y * m_nWidth] )
				continue;
			if( p1 == end )
			{
				q.push_back( p1 );
				par.push_back( i );
				bFind = true;
				break;
			}
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

TVector2<int32> CMyLevel::Search( const TVector2<int32>& begin, function<int8( SGrid*, const TVector2<int32>& )> FuncGrid, vector<TVector2<int32> >* pVecPath, TVector2<int32>* pOfs, int32 nOfs )
{
	vector<int8> vec;
	vec.resize( m_nWidth * m_nHeight );
	vector<TVector2<int32> > q;
	vector<int32> par;
	q.push_back( begin );
	par.push_back( -1 );
	vec[begin.x + begin.y * m_nWidth] = 1;
	TVector2<int32> ofs[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 }, { -1, 1 }, { -1, -1 } };
	if( !pOfs )
	{
		SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );
		pOfs = ofs;
		nOfs = ELEM_COUNT( ofs );
	}
	bool bFind = false;
	for( int32 i = 0; i < q.size(); i++ )
	{
		auto p = q[i];
		for( int j = 0; j < nOfs; j++ )
		{
			auto p1 = p + pOfs[j];
			auto pGrid = GetGrid( TVector2<int32>( p1.x, p1.y ) );
			if( !pGrid || vec[p1.x + p1.y * m_nWidth] || !pGrid->bCanEnter )
				continue;
			auto nResult = FuncGrid( pGrid, p1 );
			if( nResult == -1 )
			{
				vec[p1.x + p1.y * m_nWidth] = 1;
				continue;
			}
			if( nResult == 1 )
			{
				q.push_back( p1 );
				par.push_back( i );
				bFind = true;
				break;
			}
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

void CMyLevel::Alert( CPawn* pTriggeredPawn, const TVector2<int32>& p )
{
	if( !IsNoise() )
	{
		for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
		{
			if( pPawn->GetLevel() )
				pPawn->HandleAlert( pTriggeredPawn, p );
		}
		for( auto& pPawn : m_vecPawnHits )
		{
			if( pPawn->GetLevel() )
				pPawn->HandleAlert( pTriggeredPawn, p );
		}
		auto pGrid = GetGrid( p );
		if( pGrid )
			pGrid->nAlertEft = CGlobalCfg::Inst().lvIndicatorData.vecAlertEffectParams.size();
	}
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnAlert( pTriggeredPawn, p );
}

void CMyLevel::Alert1()
{
	m_bStartBattle = true;
	//if( m_pPlayer->GetStealthValue() )
		//m_pPlayer->UpdateStealthValue( -10000 );
	for( CLevelScript* pScript : m_vecScripts )
		pScript->OnAlert( NULL, TVector2<int32>( 0, 0 ) );
}

void CMyLevel::BeginTracer( const char* sz, int32 nDelay )
{
	EndTracer();
	auto& worldData = GetMasterLevel()->GetWorldData().curFrame;
	worldData.strTracer = sz;
	worldData.nTracerSpawnDelay = nDelay;
	worldData.strTracerLevel = "";
}

void CMyLevel::BeginTracer1( int32 n, int32 nDelay )
{
	if( n < 0 || n >= m_arrSpawnPrefab.Size() )
		return;
	BeginTracer( m_arrSpawnPrefab[n], nDelay );
}

void CMyLevel::EndTracer()
{
	auto& worldData = GetMasterLevel()->GetWorldData().curFrame;
	worldData.strTracer = "";
	worldData.nTracerSpawnDelay = 0;
	worldData.strTracerLevel = "";
	m_nTracerDelayLeft = 0;
	m_nTracerSpawnExit = 0;
	if( m_pTracerSpawnEffect )
	{
		m_pTracerSpawnEffect->SetParentEntity( NULL );
		m_pTracerSpawnEffect = NULL;
	}
}

void CMyLevel::BlockTracer()
{
	auto& worldData = GetMasterLevel()->GetWorldData().curFrame;
	if( !worldData.nTracerSpawnDelay || worldData.strTracerLevel.length() && worldData.strTracerLevel != worldData.strCurLevel )
		return;
	worldData.strTracerLevel = worldData.strCurLevel;
	worldData.tracerLevelEnterPos = m_tracerEnterPos;
}

void CMyLevel::SetTracerDelay( int32 n )
{
	auto& worldData = GetMasterLevel()->GetWorldData().curFrame;
	if( !worldData.strTracer.length() )
		return;
	worldData.nTracerSpawnDelay = n;
	if( m_bBegin )
		m_nTracerDelayLeft = n;
}

void CMyLevel::BeginNoise( const char* szSound )
{
	m_pNoise = PlaySoundLoop( szSound );
}

void CMyLevel::EndNoise()
{
	if( m_pNoise )
	{
		m_pNoise->FadeOut( 0.05f );
		m_pNoise = NULL;
	}
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

int32 CMyLevel::GetGridExit( int32 x, int32 y )
{
	auto pGrid = GetGrid( TVector2<int32>( x, y ) );
	if( !pGrid )
		return 0;
	return pGrid->nNextStage;
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

TVector2<int32> CMyLevel::GetPlayerEnterPos()
{
	return GetMasterLevel()->GetWorldData().curFrame.playerEnterPos;
}

void CMyLevel::Redirect( int32 n, int32 n1 )
{
	m_vecExitState[n].nRedirect = n1;
}

void CMyLevel::ReplaceTiles( int32 n0, int32 n1 )
{
	auto& tileData = m_arrTileData[n1];
	for( int x = 0; x < m_nWidth; x++ )
	{
		for( int y = 0; y < m_nHeight; y++ )
		{
			auto& grid = m_arrGridData[x + y * m_nWidth];
			if( grid.nTile == n0 )
			{
				auto pGrid = GetGrid( TVector2<int32>( x, y ) );
				pGrid->nTile = n1;
				pGrid->bCanEnter = !tileData.bBlocked;
				pGrid->bBlockSight = tileData.bBlocked;
				InitTile( TVector2<int32>( x, y ) );
			}
		}
	}
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

void CMyLevel::Init( int8 nType )
{
	if( nType == 1 )
		m_bSnapShot = true;
	if( nType != 1 )
	{
		m_vecExitState.resize( m_arrNextStage.Size() );
		InitTiles();
	}
	vector<CReference<CLevelEnvEffect> > vecEnvEft;
	vector<CReference<CPawnLayer> > vecPawnLayers;
	for( auto p0 = Get_ChildEntity(); p0; p0 = p0->NextChildEntity() )
	{
		auto pPawnLayer = SafeCast<CPawnLayer>( p0 );
		if( pPawnLayer )
		{
			vecPawnLayers.push_back( pPawnLayer );
			continue;
		}
		if( nType != 1 )
		{
			auto pEnvEft = SafeCast<CLevelEnvEffect>( p0 );
			if( pEnvEft )
			{
				vecEnvEft.push_back( pEnvEft );
				m_mapEnvEffects[pEnvEft->GetName().c_str()] = pEnvEft;
			}
		}
	}

	auto pMasterLevel = GetMasterLevel();
	if( nType != 1 )
	{
		for( CLevelEnvEffect* pEnvEft : vecEnvEft )
		{
			if( m_pEnvEffect )
			{
				pEnvEft->SetParentEntity( NULL );
				continue;
			}
			auto szCondition = pEnvEft->GetCondition();
			if( szCondition[0] && pMasterLevel )
			{
				if( !pMasterLevel->EvaluateKeyInt( szCondition ) )
				{
					pEnvEft->SetParentEntity( NULL );
					continue;
				}
			}
			m_pEnvEffect = pEnvEft;
		}
	}

	for( CPawnLayer* p : vecPawnLayers )
	{
		if( !p->GetCondition().length() || pMasterLevel && pMasterLevel->EvaluateKeyInt( p->GetCondition() ) )
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
		else if( nType != 1 )
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
		if( !pPawn->IsIconOnly() )
		{
			int32 nForm = 0;
			if( pPawn->m_bUseInitState && pPawn->m_arrForms.Size() )
				nForm = pPawn->m_arrSubStates[pPawn->m_nInitState].nForm;
			AddPawn( pPawn, pos, pPawn->m_nInitDir, NULL, nForm );
		}
		else
			pPawn->SetParentEntity( NULL );
	}
	for( auto& pSpawnHelper : vecSpawnHelpers )
	{
		if( pSpawnHelper->m_strSpawnCondition.length() && pMasterLevel )
		{
			if( !pMasterLevel->EvaluateKeyInt( pSpawnHelper->m_strSpawnCondition ) )
			{
				if( pSpawnHelper->m_nSpawnType != 2 )
				{
					pSpawnHelper->SetParentEntity( NULL );
					continue;
				}
			}
			else if( pSpawnHelper->m_nSpawnType == 2 )
			{
				HandleSpawn( pSpawnHelper );
				continue;
			}
		}
		if( pSpawnHelper->m_nSpawnType >= 1 )
		{
			m_vecSpawner.push_back( pSpawnHelper );
			pSpawnHelper->SetParentEntity( NULL );
			pSpawnHelper->m_nStateParam[0] = pSpawnHelper->m_nSpawnParam[1];
			continue;
		}
		HandleSpawn( pSpawnHelper );
	}
	if( pMasterLevel )
	{
		for( auto& item : pMasterLevel->GetCurLevelData().mapDataDeadPawn )
		{
			auto n = item.second.nSpawnIndex;
			if( n >= 0 && n < m_arrSpawnPrefab.Size() )
			{
				CReference<CPawn> pPawn = SafeCast<CPawn>( m_arrSpawnPrefab[n]->GetRoot()->CreateInstance() );
				if( pPawn->m_nLevelDataType == 2 )
				{
					auto nDeath = pPawn->GetStateIndexByName( "death" );
					if( nDeath < 0 )
						continue;
					auto pSpawnHelper = new CLevelSpawnHelper( n, item.first.c_str(), nDeath );
					pSpawnHelper->m_nDataType = 2;
					pSpawnHelper->m_bSpawnDeath = true;
					pPawn->SetName( item.first.c_str() );
					pPawn->m_pSpawnHelper = pSpawnHelper;
					if( !AddPawn1( pPawn, item.second.nState, item.second.nStateTick, item.second.p, item.second.p1, item.second.nDir ) )
						continue;
				}
				else
				{
					auto nDeath = pPawn->GetStateIndexByName( "death" );
					if( nDeath < 0 )
						continue;
					auto pSpawnHelper = new CLevelSpawnHelper( n, item.first.c_str(), nDeath );
					pSpawnHelper->m_bSpawnDeath = true;
					pPawn->SetName( item.first.c_str() );
					pPawn->m_pSpawnHelper = pSpawnHelper;
					if( !AddPawn( pPawn, item.second.p, item.second.nDir ) )
						continue;
				}
				pPawn->strCreatedFrom = m_arrSpawnPrefab[n]->GetName();
			}
		}
	}
	FlushSpawn();
	InitScripts();

	m_pPawnRoot->SortChildrenRenderOrder( [] ( CRenderObject2D* a, CRenderObject2D* b ) {
		auto pPawn1 = static_cast<CPawn*>( a );
		auto pPawn2 = static_cast<CPawn*>( b );
		return pPawn1->m_nCurStateRenderOrder < pPawn2->m_nCurStateRenderOrder;
	} );

	if( nType == 1 )
	{
		while( Get_ChildEntity() )
			Get_ChildEntity()->SetParentEntity( NULL );
		RemoveAllChild();
		m_pPawnRoot->SetParentEntity( this );
		InitTiles();
	}
	else if( nType == 2 )
	{
		struct _STemp
		{
			static uint32 Func( void* pThis )
			{
				( (CMyLevel*)pThis )->UpdateActionPreviewFunc();
				return 1;
			}
		};
		m_pActionPreviewCoroutine = TCoroutinePool<0x10000>::Inst().Alloc();
		m_pActionPreviewCoroutine->Create( &_STemp::Func, this );
	}
}

void CMyLevel::Update()
{
	auto pMasterLevel = GetMasterLevel();
	if( m_bBegin && !m_nFreeze )
	{
		if( pMasterLevel )
		{
			if( CGame::Inst().IsKeyDown( 'R' ) )
			{
				if( !m_bFailed )
					Fail();
				else
					pMasterLevel->JumpBack( 0 );
				return;
			}
			if( CGame::Inst().IsKeyDown( 'T' ) )
			{
				pMasterLevel->JumpBack( 1 );
				return;
			}
			if( CGame::Inst().IsKeyDown( 'Y' ) )
			{
				pMasterLevel->JumpBack( 2 );
				return;
			}
			if( m_bFailed && CGame::Inst().IsAnyInputDown() )
			{
				pMasterLevel->JumpBack( 0 );
				pMasterLevel->GetCurLevel()->Update();
				return;
			}
		}
		if( m_bFailed )
			return;

		for( CLevelSpawnHelper* p : m_vecSpawner )
		{
			if( p && p->m_nSpawnType == 1 )
			{
				if( !p->m_nStateParam[0] )
				{
					CReference<CLevelSpawnHelper> pSpawnHelper = SafeCast<CLevelSpawnHelper>( p->GetInstanceOwnerNode()->CreateInstance() );
					HandleSpawn( pSpawnHelper );
					p->m_nStateParam[0] = p->m_nSpawnParam[0];
				}
				p->m_nStateParam[0]--;
			}
		}
	}
	FlushSpawn();
	for( auto& grid : m_vecGrid )
	{
		grid.bStealthAlert = false;
		grid.bStealthDetect = false;
	}
	for( auto pPawn = m_pPawns; pPawn; pPawn = pPawn->NextPawn() )
	{
		HandlePawnMounts( pPawn, false );
		auto pawnTarget = pPawn->HandleStealthDetect();
		if( pawnTarget.x >= 0 )
			m_pIndicatorLayer->UpdatePawnTarget( pPawn, pawnTarget );
	}
	for( auto& pPawn : m_vecPawnHits )
	{
		HandlePawnMounts( pPawn, false );
		auto pawnTarget = pPawn->HandleStealthDetect();
		if( pawnTarget.x >= 0 )
			m_pIndicatorLayer->UpdatePawnTarget( pPawn, pawnTarget );
	}

	if( m_bBegin && !m_nFreeze )
	{
		if( m_pPlayer )
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
		if( m_nTracerDelayLeft )
		{
			if( m_nTracerDelayLeft <= 5 )
				SafeCast<CTracerSpawnEffect>( m_pTracerSpawnEffect.GetPtr() )->Kill();
			m_nTracerDelayLeft--;
			if( !m_nTracerDelayLeft )
			{
				auto worldData = GetMasterLevel()->GetWorldData().curFrame;
				auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( worldData.strTracer.c_str() );
				CReference<CPawn> pPawn = SafeCast<CPawn>( pPrefab->GetRoot()->CreateInstance() );
				if( !AddPawn( pPawn, m_tracerEnterPos, worldData.nPlayerEnterDir ) )
				{
					if( m_pPlayer->GetPos() == m_tracerEnterPos || m_pPlayer->GetMoveTo() == m_tracerEnterPos )
						Fail();
					else
						m_nTracerDelayLeft = 1;
				}
				else
				{
					pPawn->SetName( "__TRACER" );
					pPawn->strCreatedFrom = pPrefab->GetName();
				}
			}
		}
		if( m_pTracerSpawnEffect )
		{
			SafeCast<CTracerSpawnEffect>( m_pTracerSpawnEffect.GetPtr() )->Update();
			if( !m_pTracerSpawnEffect->GetParentEntity() )
				m_pTracerSpawnEffect = NULL;
		}

		m_bBlocked = false;
		for( int i = 0; i < m_vecExitState.size(); i++ )
			m_vecExitState[i].bBlocked = false;
		if( m_nTracerSpawnExit > 0 )
		{
			if( m_nTracerDelayLeft )
				BlockExit( m_nTracerSpawnExit - 1 );
			else
			{
				auto pPawn = GetPawnByName( "__TRACER" );
				if( pPawn && !pPawn->IsKilled() )
					BlockExit( m_nTracerSpawnExit - 1 );
			}
		}
		m_trigger.Trigger( 0, this );
		for( auto& pScript : m_vecScripts )
			pScript->OnUpdate( this );

		for( int i = 0; i < m_vecExitState.size(); i++ )
		{
			if( !m_vecExitState[i].bBlocked )
			{
				if( m_arrNextStage[i].strKeyOrig.length() &&
					!pMasterLevel->EvaluateKeyInt( m_arrNextStage[i].strKeyOrig ) )
				{
					m_vecExitState[i].bBlocked = true;
					continue;
				}
				auto nRedirect = m_vecExitState[i].nRedirect;
				if( nRedirect >= 0 && nRedirect < m_vecExitState.size() )
				{
					if( m_arrNextStage[nRedirect].strKeyRedirect.length() &&
						!pMasterLevel->EvaluateKeyInt( m_arrNextStage[nRedirect].strKeyRedirect ) )
					{
						m_vecExitState[i].bBlocked = true;
						continue;
					}
				}
			}
		}

		if( m_pActionPreviewCoroutine )
			m_pActionPreviewCoroutine->Yield( 0 );
		if( m_pPlayer )
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

	if( m_pPlayer )
	{
		bool bDetect = false;
		if( m_pPlayer->IsHidden() )
		{
			auto p1 = m_pPlayer->GetPos();
			auto p2 = m_pPlayer->GetMoveTo();

			bool bAlert = false;
			for( int i = 0; i < m_pPlayer->GetWidth() && !bAlert; i++ )
			{
				for( int j = 0; j < m_pPlayer->GetHeight(); j++ )
				{
					auto pGrid = GetGrid( p1 + TVector2<int32>( i, j ) );
					if( pGrid->bStealthAlert && !m_pPlayer->m_bPosHidden )
					{
						bAlert = true;
						break;
					}
					pGrid = GetGrid( p2 + TVector2<int32>( i, j ) );
					if( pGrid->bStealthAlert && !m_pPlayer->m_bMoveToHidden )
					{
						bAlert = true;
						break;
					}
				}
			}
			if( bAlert )
				m_pPlayer->m_bPosHidden = m_pPlayer->m_bMoveToHidden = false;

			for( int i = 0; i < m_pPlayer->GetWidth() && !bDetect; i++ )
			{
				for( int j = 0; j < m_pPlayer->GetHeight(); j++ )
				{
					auto pGrid = GetGrid( p1 + TVector2<int32>( i, j ) );
					if( pGrid->bStealthDetect && !m_pPlayer->m_bPosHidden )
					{
						bDetect = true;
						break;
					}
					pGrid = GetGrid( p2 + TVector2<int32>( i, j ) );
					if( pGrid->bStealthDetect && !m_pPlayer->m_bMoveToHidden )
					{
						bDetect = true;
						break;
					}
				}
			}
			m_pPlayer->UpdateStealthValue( bDetect ? -1 : 1 );
		}
		if( !m_bStartBattle && !m_pPlayer->IsHidden() )
		{
			for( auto pPawn = Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
			{
				if( pPawn->IsAutoBlockStage() )
				{
					m_bStartBattle = true;
					break;
				}
			}
			if( !m_bStartBattle )
			{
				for( auto pPawn = m_spawningPawns.Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
				{
					if( pPawn->IsAutoBlockStage() )
					{
						m_bStartBattle = false;
						break;
					}
				}
			}
		}
	}

	if( m_bBlocked )
		m_bComplete = false;
	else
	{
		m_bComplete = true;
		if( m_bStartBattle )
		{
			for( auto pPawn = Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
			{
				if( pPawn->IsAutoBlockStage() && pPawn->m_nHp > 0 )
				{
					m_bComplete = false;
					break;
				}
			}
			if( m_bComplete )
			{
				for( auto pPawn = m_spawningPawns.Get_Pawn(); pPawn; pPawn = pPawn->NextPawn() )
				{
					if( pPawn->IsAutoBlockStage() && pPawn->m_nHp > 0 )
					{
						m_bComplete = false;
						break;
					}
				}
			}
		}
	}

	if( m_pIndicatorLayer )
		m_pIndicatorLayer->Update( this );

	m_pPawnRoot->SortChildrenRenderOrder( [] ( CRenderObject2D* a, CRenderObject2D* b ) {
		auto pPawn1 = static_cast<CPawn*>( a );
		auto pPawn2 = static_cast<CPawn*>( b );
		return pPawn1->m_nCurStateRenderOrder < pPawn2->m_nCurStateRenderOrder;
	} );
	if( m_pPlayer && !m_bFailed )
	{
		if( m_pPlayer->m_nHp <= 0 )
			Fail();
		else if( m_bComplete && !m_bScenario && m_pPlayer->m_pos == m_pPlayer->m_moveTo )
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
							pMasterLevel->TransferTo( nxt.pNxtStage, m_pPlayer->m_pos - TVector2<int32>( nxt.nOfsX, nxt.nOfsY ), m_pPlayer->m_nCurDir );
						return;
					}
				}
			}
			m_bFullyEntered = true;
		}
	}
}

int32 CMyLevel::UpdateActionPreview()
{
	return m_pActionPreviewCoroutine->Resume();
}

void CMyLevel::ActionPreviewPause()
{
	m_pActionPreviewCoroutine->Yield( 1 );
	if( IsEnd() )
		throw( 1 );
}

bool CMyLevel::OnPlayerTryToLeave( const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType, int32 nTransferParam )
{
	int32 nExcludeTile = -1;
	if( nTransferType == 4 || nTransferType == 5 )
		nExcludeTile = nTransferParam;

	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		if( nExcludeTile >= 0 && IsPawnInTile( pPawn, nExcludeTile ) )
			continue;
		if( !pPawn->OnPlayerTryToLeave() )
			return false;
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )
	for( int i = 0; i < m_vecPawnHits.size(); i++ )
	{
		auto p = m_vecPawnHits[i];
		if( p->GetParentEntity() )
		{
			if( nExcludeTile >= 0 && IsPawnInTile( p, nExcludeTile ) )
				continue;
			if( !p->OnPlayerTryToLeave() )
				return false;
		}
	}
	return true;
}

void CMyLevel::OnCheckPoint()
{
	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		if( pPawn != m_pPlayer )
			pPawn->OnLevelSave();
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )
	for( int i = 0; i < m_vecPawnHits.size(); i++ )
	{
		auto p = m_vecPawnHits[i];
		if( p->GetParentEntity() )
			p->OnLevelSave();
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

void CMyLevel::ScriptForEachPawn()
{
	auto pLuaState = CLuaState::GetCurLuaState();
	bool b = false;
	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		if( pPawn->GetLevel() == this )
		{
			pLuaState->PushLuaIndex( -1 );
			pLuaState->PushLua( pPawn );
			pLuaState->Call( 1, 1 );
			if( !pLuaState->PopLuaValue<bool>() )
			{
				b = true;
				break;
			}
		}
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )
	if( b )
		return;
	for( CPawn* p : m_vecPawnHits )
	{
		if( p->GetLevel() == this )
		{
			pLuaState->PushLuaIndex( -1 );
			pLuaState->PushLua( p );
			pLuaState->Call( 1, 1 );
			if( !pLuaState->PopLuaValue<bool>() )
				return;
		}
	}
}

void CMyLevel::ScriptForEachEnemy()
{
	auto pLuaState = CLuaState::GetCurLuaState();
	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pPawns, CPawn, Pawn )
	{
		if( pPawn->GetLevel() == this && pPawn->m_bIsEnemy )
		{
			pLuaState->PushLuaIndex( -1 );
			pLuaState->PushLua( pPawn );
			pLuaState->Call( 1, 1 );
			if( !pLuaState->PopLuaValue<bool>() )
				break;
		}
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pPawns, CPawn, Pawn )
}

CPawn* CMyLevel::HandleSpawn( CLevelSpawnHelper* pSpawnHelper )
{
	CReference<CPawn> pPawn = SafeCast<CPawn>( pSpawnHelper->GetRenderObject() );
	if( pPawn->IsIconOnly() )
	{
		pSpawnHelper->SetParentEntity( NULL );
		return NULL;
	}
	TVector2<int32> pos( floor( pSpawnHelper->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pSpawnHelper->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
	auto nDir = pPawn->m_nInitDir;
	return HandleSpawn1( pSpawnHelper, pos, nDir );
}

CPawn* CMyLevel::HandleSpawn1( CLevelSpawnHelper* pSpawnHelper, const TVector2<int32>& p, int32 nDir, const char* szInitState )
{
	CReference<CPawn> pPawn = SafeCast<CPawn>( pSpawnHelper->GetRenderObject() );
	if( szInitState )
	{
		auto nInitState = pPawn->GetStateIndexByName( szInitState );
		if( nInitState >= 0 )
			pPawn->SetInitState( nInitState );
	}
	auto pos = p;
	auto pos0 = pos;
	auto nDir0 = nDir;
	bool bInitState = false;
	auto moveTo = pos;
	int32 nInitState = 0;
	int32 nInitStateTick = 0;
	if( pSpawnHelper->m_nDataType >= 2 )
	{
		auto& mapDeadPawn = GetMasterLevel()->GetCurLevelData().mapDataDeadPawn;
		auto itr = mapDeadPawn.find( pSpawnHelper->GetName().c_str() );
		if( itr != mapDeadPawn.end() )
		{
			auto nWidth = pPawn->m_nWidth;
			auto nHeight = pPawn->m_nHeight;
			if( pPawn->m_arrForms.Size() )
			{
				auto nForm = pPawn->m_arrSubStates[itr->second.nState].nForm;
				nWidth = pPawn->m_arrForms[nForm].nWidth;
				nHeight = pPawn->m_arrForms[nForm].nHeight;
			}
			bool b = true;
			if( !SafeCast<CPawnHit>( pPawn.GetPtr() ) )
			{
				for( int i = 0; i < nWidth && b; i++ )
				{
					for( int j = 0; j < nHeight; j++ )
					{
						auto pGrid = GetGrid( itr->second.p + TVector2<int32>( i, j ) );
						if( !pGrid || pGrid->pPawn0 || !pGrid->bCanEnter || pGrid->nNextStage )
						{
							b = false;
							break;
						}
						pGrid = GetGrid( itr->second.p1 + TVector2<int32>( i, j ) );
						if( !pGrid || pGrid->pPawn0 || !pGrid->bCanEnter || pGrid->nNextStage )
						{
							b = false;
							break;
						}
					}
				}
			}
			if( b )
			{
				bInitState = true;
				pos = itr->second.p;
				moveTo = itr->second.p1;
				nInitState = itr->second.nState;
				nInitStateTick = itr->second.nStateTick;
				nDir = itr->second.nDir;
			}
			if( pSpawnHelper->m_nDataType == 2 && !itr->second.bIsAlive )
				pSpawnHelper->m_bSpawnDeath = true;
		}
	}
	else if( pSpawnHelper->m_nDataType == 1 )
	{
		auto& mapDeadPawn = GetMasterLevel()->GetCurLevelData().mapDataDeadPawn;
		auto itr = mapDeadPawn.find( pSpawnHelper->GetName().c_str() );
		if( itr != mapDeadPawn.end() )
		{
			if( !pPawn->IsValidStateIndex( pSpawnHelper->m_nDeathState ) )
			{
				pSpawnHelper->SetParentEntity( NULL );
				return NULL;
			}
			bool b = true;
			if( !SafeCast<CPawnHit>( pPawn.GetPtr() ) )
			{
				for( int i = 0; i < pPawn->m_nWidth && b; i++ )
				{
					for( int j = 0; j < pPawn->m_nHeight; j++ )
					{
						auto pGrid = GetGrid( itr->second.p + TVector2<int32>( i, j ) );
						if( !pGrid || pGrid->pPawn0 || !pGrid->bCanEnter || pGrid->nNextStage )
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
	else if( pSpawnHelper->m_strDeathKey.length() && GetMasterLevel()->EvaluateKeyInt( pSpawnHelper->m_strDeathKey ) )
		pSpawnHelper->m_bSpawnDeath = true;
	if( pSpawnHelper->m_bSpawnDeath )
	{
		if( !pPawn->IsValidStateIndex( pSpawnHelper->m_nDeathState ) )
		{
			pSpawnHelper->SetParentEntity( NULL );
			return NULL;
		}
	}

	pPawn->SetParentEntity( m_pPawnRoot );
	pPawn->SetPosition( pSpawnHelper->GetPosition() );
	pSpawnHelper->ClearRenderObject();
	pPawn->SetName( pSpawnHelper->GetName() );
	pPawn->m_pSpawnHelper = pSpawnHelper;
	pSpawnHelper->SetParentEntity( NULL );

	bool bSucceed = false;
	if( bInitState && AddPawn1( pPawn, nInitState, nInitStateTick, pos, moveTo, nDir ) )
		bSucceed = true;
	else
	{
		int32 nForm = 0;
		if( pPawn->m_bUseInitState && pPawn->m_arrForms.Size() )
			nForm = pPawn->m_arrSubStates[pPawn->m_nInitState].nForm;

		if( AddPawn( pPawn, pos, nDir, NULL, nForm ) )
			bSucceed = true;
		else if( pos0 != pos && AddPawn( pPawn, pos0, nDir0, NULL, nForm ) )
			bSucceed = true;
		else if( pSpawnHelper->m_nSpawnType >= 2 && pSpawnHelper->m_nSpawnParam[0] )
		{
			TVector2<int32> ofs[] = { { 2, 0 }, { 1, 1 }, { -1, 1 }, { -2, 0 }, { -1, -1 }, { -1, 1 } };
			int32 k0 = SRand::Inst().Rand( 0, 6 );
			int32 nMaxDist = Max( m_nWidth / 2, m_nHeight );
			if( pSpawnHelper->m_nSpawnParam[0] > 0 )
				nMaxDist = Min( pSpawnHelper->m_nSpawnParam[0], nMaxDist );
			for( int32 i = 1; i <= nMaxDist && !bSucceed; i++ )
			{
				for( int32 j = 0; j < i && !bSucceed; j++ )
				{
					for( int32 k = 0; k < 6 && !bSucceed; k++ )
					{
						auto p = pos + ofs[( k + k0 ) % 6] * ( i - j ) + ofs[( k + k0 + 1 ) % 6] * j;
						if( AddPawn( pPawn, p, nDir, NULL, nForm ) )
							bSucceed = true;
					}
				}
			}
		}
	}
	if( !bSucceed )
	{
		pPawn->SetParentEntity( NULL );
		return NULL;
	}
	pPawn->strCreatedFrom = pSpawnHelper->GetResource()->GetName();
	return pPawn;
}

void CMyLevel::HandlePawnMounts( CPawn* pPawn, bool bRemove, CEntity* pRoot )
{
	if( !pRoot )
		pRoot = pPawn;
	for( auto pChild = pRoot->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
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
		HandlePawnMounts( pPawn, bRemove, pMount );
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
			InitTile( TVector2<int32>( x, y ) );
		}
	}
}

void CMyLevel::InitTile( const TVector2<int32>& p )
{
	auto& grid = m_arrGridData[p.x + p.y * m_nWidth];
	auto pGrid = GetGrid( p );
	if( !( ( p.x + p.y ) & 1 ) )
	{
		if( m_arrTileData.Size() )
		{
			auto& tileData = m_arrTileData[Min<int32>( pGrid->nTile, m_arrTileData.Size() - 1 )];
			CRenderObject2D* pRenderObject;
			if( tileData.pTileDrawable )
			{
				auto pImage = static_cast<CImage2D*>( tileData.pTileDrawable->CreateInstance() );
				pImage->SetPosition( CVector2( p.x, p.y ) * LEVEL_GRID_SIZE );
				auto rect = pImage->GetElem().rect;
				rect.width *= tileData.texRect.width;
				rect.height *= tileData.texRect.height;
				pImage->SetRect( rect );
				pImage->SetTexRect( tileData.texRect );
				pRenderObject = pImage;
			}
			else
				pRenderObject = new CRenderObject2D;
			if( pGrid->pTile )
			{
				AddChildAfter( pRenderObject, pGrid->pTile );
				pGrid->pTile->RemoveThis();
			}
			else
				AddChildAfter( pRenderObject, m_pPawnRoot );
			pGrid->pTile = pRenderObject;
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

void CMyLevel::UpdateActionPreviewFunc()
{
	try
	{
		while( !IsEnd() )
		{
			Update();
		}
	}
	catch( int32 i )
	{

	}
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
	m_fLabelX = m_pLabelsRoot->x;
	m_pLabelsRoot->SetRenderObject( NULL );
	m_pLabelsCounter->SetParentEntity( NULL );
}

void CMainUI::Reset( const CVector2& inputOrig, const CVector2& iconOrig, bool bClearAllEfts )
{
	m_playerInputOrig = inputOrig;
	m_iconOrig = iconOrig;
	m_pLabelsRoot->bVisible = false;
	for( int i = 0; i < ELEM_COUNT( m_pIcons ); i++ )
	{
		if( m_pIcons[i] )
			m_pIcons[i]->bVisible = false;
	}
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

void CMainUI::RefreshPlayerInput( vector<int8>& vecInput, int32 nMatchLen, int8 nChargeKey, int8 nType )
{
	auto& item = m_vecInputItems[0];
	if( nMatchLen >= 0 && !vecInput.size() )
	{
		item.vec.resize( 1 );
		item.vec[0] = nType == 2 ? 14 : 12;
	}
	else
	{
		item.vec.resize( vecInput.size() );
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
		if( nChargeKey )
		{
			int8 n1 = -1;
			if( ( nChargeKey & ( 1 | 2 ) ) == ( 1 | 2 ) )
				n1 = 1;
			else if( ( nChargeKey & ( 2 | 4 ) ) == ( 2 | 4 ) )
				n1 = 3;
			else if( ( nChargeKey & ( 4 | 8 ) ) == ( 4 | 8 ) )
				n1 = 5;
			else if( ( nChargeKey & ( 8 | 1 ) ) == ( 8 | 1 ) )
				n1 = 7;
			else if( ( nChargeKey & 1 ) == 1 )
				n1 = 0;
			else if( ( nChargeKey & 2 ) == 2 )
				n1 = 2;
			else if( ( nChargeKey & 4 ) == 4 )
				n1 = 4;
			else if( ( nChargeKey & 8 ) == 8 )
				n1 = 6;
			if( n1 >= 0 )
			{
				item.vec.push_back( n1 + 16 );
				if( nMatchLen >= 0 )
					nMatchLen++;
			}
			for( int k = 0; k < 4; k++ )
			{
				if( !!( nChargeKey & ( 16 << k ) ) )
				{
					item.vec.push_back( 24 + k );
					if( nMatchLen >= 0 )
						nMatchLen++;
				}
			}
		}
		if( nType == 1 )
		{
			item.vec.push_back( 13 );
			if( nMatchLen > 0 )
				nMatchLen++;
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

void CMainUI::OnPlayerAction( vector<int8>& vecInput, int32 nMatchLen, int8 nChargeKey, int8 nType )
{
	if( IsScenario() )
		return;
	if( nType != 2 )
		m_nPlayerActionFrame = CGlobalCfg::Inst().MainUIData.vecActionEftFrames.size();
	RefreshPlayerInput( vecInput, nMatchLen, nChargeKey, nType );

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
	GetStage()->GetMasterLevel()->GetWorldData().OnScenarioText( n, sz, color );
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

void CMainUI::HeadText( const char* sz, const CVector4& color, int32 nTime, const char* szSound, int32 nSoundInterval, bool bImportant )
{
	auto pText = SafeCast<CTypeText>( m_pHeadText.GetPtr() );
	pText->SetParam( color );
	pText->Set( sz, 2 );
	pText->SetTypeSound( szSound, nSoundInterval );
	m_nHeadTextTime = nTime;
	if( bImportant )
		GetStage()->GetMasterLevel()->GetWorldData().OnScenarioText( 2, sz, color );
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
	if( !nLevel )
		m_bResetFreezeEft = true;
}

void CMainUI::ClearLabels()
{
	m_pLabelsRoot->RemoveAllChild();
	m_vecLabels.resize( 0 );
	m_vecLabelCounters.resize( 0 );
}

void CMainUI::SetLabel( int32 nIndex, int32 x, int32 y, int32 nCounter )
{
	if( x >= 0 )
	{
		if( nIndex >= m_vecLabels.size() )
			m_vecLabels.resize( nIndex + 1 );
		if( nIndex >= m_vecLabelCounters.size() )
			m_vecLabelCounters.resize( nIndex + 1 );
		if( !m_vecLabels[nIndex] )
		{
			auto pImg = static_cast<CImage2D*>( static_cast<CDrawableGroup*>( m_pLabelsRoot->GetResource() )->CreateInstance() );
			m_vecLabels[nIndex] = pImg;
			pImg->SetRect( CRectangle( -1 - ( nIndex / 8 ), -1 - ( nIndex % 8 ), 1, 1 ) * 32 );
			m_pLabelsRoot->AddChild( pImg );
		}
		auto pImg = static_cast<CImage2D*>( m_vecLabels[nIndex].GetPtr() );
		pImg->SetTexRect( CRectangle( x, y, 1, 1 ) / 8 );
		if( nCounter )
		{
			if( !m_vecLabelCounters[nIndex] )
			{
				auto p = SafeCast<CEntity>( m_pLabelsCounter->GetInstanceOwnerNode()->CreateInstance() );
				m_vecLabelCounters[nIndex] = p;
				p->SetPosition( CVector2( -1 - ( nIndex / 8 ), -1 - ( nIndex % 8 ) ) * 32 );
				p->SetParentBeforeEntity( m_vecLabels[nIndex] );
			}
			char buf[64];
			sprintf( buf, "%d", nCounter );
			SafeCast<CSimpleText>( m_vecLabelCounters[nIndex].GetPtr() )->Set( buf );
		}
		else
		{
			if( m_vecLabelCounters[nIndex] )
			{
				m_vecLabelCounters[nIndex]->SetParentEntity( NULL );
				m_vecLabelCounters[nIndex] = NULL;
			}
		}
	}
	else
	{
		if( nIndex < m_vecLabels.size() && m_vecLabels[nIndex] )
		{
			if( m_vecLabels[nIndex] )
			{
				m_vecLabels[nIndex]->RemoveThis();
				m_vecLabels[nIndex] = NULL;
			}
			if( m_vecLabelCounters[nIndex] )
			{
				m_vecLabelCounters[nIndex]->SetParentEntity( NULL );
				m_vecLabelCounters[nIndex] = NULL;
			}
		}
	}
}

void CMainUI::SetLabelCounter( int32 nIndex, int32 nCounter )
{
	if( nIndex >= m_vecLabels.size() || !m_vecLabels[nIndex] )
		return;
	if( nCounter )
	{
		if( !m_vecLabelCounters[nIndex] )
		{
			auto p = SafeCast<CEntity>( m_pLabelsCounter->GetInstanceOwnerNode()->CreateInstance() );
			m_vecLabelCounters[nIndex] = p;
			p->SetPosition( CVector2( -1 - ( nIndex / 8 ), -1 - ( nIndex % 8 ) ) * 32 );
			p->SetParentBeforeEntity( m_vecLabels[nIndex] );
		}
		char buf[64];
		sprintf( buf, "%d", nCounter );
		SafeCast<CSimpleText>( m_vecLabelCounters[nIndex].GetPtr() )->Set( buf );
	}
	else
	{
		if( m_vecLabelCounters[nIndex] )
		{
			m_vecLabelCounters[nIndex]->SetParentEntity( NULL );
			m_vecLabelCounters[nIndex] = NULL;
		}
	}
}

void CMainUI::Update()
{
	UpdatePos();
	UpdateIcons();
	UpdateEffect();
	UpdateInputItem( 0 );

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
		if( CGame::Inst().IsKeyDown( VK_RETURN ) || CGame::Inst().IsKeyDown( ' ' ) )
		{
			auto pText = SafeCast<CTypeText>( m_pScenarioText[m_nLastScenarioText].GetPtr() );
			if( !pText->IsFinished() && !pText->IsForceFinish() )
				pText->ForceFinish();
			else
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

void CMainUI::UpdateEffect()
{
	m_vecPlayerActionElems.resize( 0 );
	m_vecPlayerActionElemParams.resize( 0 );
	auto pCurLevel = GetStage()->GetMasterLevel()->GetCurLevel();
	if( GetStage()->GetMasterLevel()->IsBlackOut() )
		FailEffect( 1 );
	else
	{
		if( !m_bScenario && m_nPlayerActionFrame )
		{
			Effect0();
		}
		if( pCurLevel && pCurLevel->IsFailed() )
		{
			RecordEffect();
			FailEffect();
		}
		else
		{
			if( m_nFreezeLevel )
				FreezeEffect( m_nFreezeLevel );
			if( m_nRecordEftFrames )
			{
				RecordEffect();
				m_nRecordEftFrames--;
			}
		}
	}
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
			if( elem.rect.width <= 0 )
				break;
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
	if( !m_bScenario && pPlayer && pPlayer->GetLevel() )
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
	auto viewArea = GetStage()->GetCamera().GetViewArea();
	int32 nGlitch = 0;
	int32 nElem = 0;
	for( int i = 0; i < item.vec.size(); i++ )
	{
		auto rect = CRectangle( p0.x - ( item.vec.size() - i ) * 16, p0.y - nItem * 16, 16, 16 ).Offset( param.ofs );
		if( nItem == 0 )
			rect.x += item.vec.size() * 8;
		if( nItem == 0 && rect.GetRight() <= -viewArea.width / 2 || rect.x >= viewArea.width / 2 )
		{
			nGlitch++;
			continue;
		}
		auto& elem = item.vecElems[nElem++];
		elem.rect = rect;
	}
	if( !nGlitch )
	{
		for( int i = 0; i < item.vec.size(); i++ )
		{
			auto& elem = item.vecElems[i];

			elem.texRect = CRectangle( item.vec[i] % 16 * 0.0625f, item.vec[i] / 16 * 0.0625f, 0.0625f, 0.0625f );
			elem.nInstDataSize = sizeof( CVector4 ) * 2;
			elem.pInstData = param.params + ( item.nMatchLen >= 0 && item.vec.size() - i > item.nMatchLen ? 2 : 0 );
		}
		return;
	}

	int8* result = (int8*)alloca( item.vec.size() );
	for( int i = 0; i < item.vec.size(); i++ )
	{
		if( i >= nElem )
			result[i] = 0;
		else if( i < nGlitch )
			result[i] = 1;
		else
			result[i] = 2;
	}
	SRand::Inst<eRand_Render>().Shuffle( result, item.vec.size() );
	nElem = 0;
	for( int i = 0; i < item.vec.size(); i++ )
	{
		if( result[i] == 0 )
			continue;
		auto& elem = item.vecElems[nElem++];
		if( result[i] == 1 )
			elem.texRect = CRectangle( 0.75f, 0.0625f, 0.0625f, 0.0625f );
		else
			elem.texRect = CRectangle( item.vec[i] % 16 * 0.0625f, item.vec[i] / 16 * 0.0625f, 0.0625f, 0.0625f );

		elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.pInstData = param.params + ( item.nMatchLen >= 0 && item.vec.size() - i > item.nMatchLen ? 2 : 0 );
	}
	for( ; nElem < item.vec.size(); nElem++ )
	{
		auto& elem = item.vecElems[nElem++];
		elem.rect = CRectangle( 0, 0, 0, 0 );
	}
}

void CMainUI::UpdateIcons()
{
	if( m_bScenario || GetStage()->GetMasterLevel()->IsTransfer() )
	{
		for( int i = 0; i < ELEM_COUNT( m_pIcons ); i++ )
		{
			if( m_pIcons[i] )
				m_pIcons[i]->bVisible = false;
		}
		return;
	}
	auto pPlayer = GetStage()->GetWorld()->GetPlayer();
	if( pPlayer )
	{
		for( int i = 0; i < ePlayerEquipment_Count; i++ )
		{
			if( !m_pIcons[i] )
				continue;
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
	m_pLabelsRoot->bVisible = true;
	m_pLabelsRoot->SetPosition( CVector2( m_iconOrig.x + m_fLabelX, m_pLabelsRoot->y ) );
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
		m_vecPlayerActionElemParams[i * 2].w = ( floor( ( l[0] + l[1] ) ) * 2 ) * ( SRand::Inst<eRand_Render>().Rand( 0, 2 ) * 2 - 1 );
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

void CMainUI::FailEffect( int8 nType0 )
{
	auto camSize = GetStage()->GetCamera().GetViewArea().GetSize();
	CRectangle r0( -camSize.x * 0.5f, -28, camSize.x, 28 );

	static SRand rand0_1 = SRand::Inst<eRand_Render>();
	SRand rand1 = rand0_1;
	if( !SRand::Inst<eRand_Render>().Rand( 0, 8 ) )
		rand0_1.Rand();
	for( int n = nType0? SRand::Inst<eRand_Render>().Rand( 15, 45 ) : rand1.Rand( -6, 4 ); n > 0; n-- )
	{
		CRectangle rect = r0;
		rect.width = rand1.Rand( 20, 70 ) * 4;
		rect.x = rand1.Rand<int32>( r0.x / 2, ( r0.GetRight() - rect.width ) / 2 + 1 ) * 2;
		rect.y -= nType0? rand1.Rand( 0, 100 ) * 2 : rand1.Rand( 0, 4 ) * 2;
		if( nType0 )
			rect.height = rand1.Rand( 1, 6 ) * 2;
		int32 nOfs0 = rand1.Rand( 0, 4 );
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem = m_vecPlayerActionElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );

		CVector3 color( rand1.Rand( 0.0f, 1.0f ), rand1.Rand( 0.0f, 1.0f ), rand1.Rand( 0.0f, 1.0f ) );
		if( nType0 )
		{
			color = color * 0.5f + CVector3( 0.5f, 0.5f, 0.5f );
			float f = 1 - Max( color.x, Max( color.y, color.z ) );
			color = ( color + CVector3( f, f, f ) ) * rand1.Rand( 0.8f, 1.1f );
		}
		else
		{
			float f = Min( color.x, Min( color.y, color.z ) );
			color = ( color - CVector3( f, f, f ) ) * rand1.Rand( 1.0f, 1.3f );
		}
		m_vecPlayerActionElemParams.push_back( CVector4( color.x, color.y, color.z, nOfs0 * 2 ) );
		m_vecPlayerActionElemParams.push_back( nType0 ? CVector4( 0, 0, 0, 0 ) :
			CVector4( color.x, color.y, color.z, 0 ) * rand1.Rand( 0.01f, 0.2f ) );
	}

	static SRand rand0_2 = SRand::Inst<eRand_Render>();
	rand1 = rand0_2;
	if( !SRand::Inst<eRand_Render>().Rand( 0, 16 ) )
		rand0_2.Rand();
	int8 nType = nType0 ? rand1.Rand( 1, 3 ) : rand1.Rand( -4, 3 );
	CRectangle rect0 = r0;
	rect0.y -= rand1.Rand( 100, 200 ) * 2;
	rect0.SetBottom( rand1.Rand<int32>( r0.y / 2, r0.GetBottom() / 2 ) * 2 );
	if( nType0 )
	{
		rect0.y -= 50;
		rect0.height += 100;
	}
	if( nType == 2 )
	{
		auto rect = rect0;
		int8 nDir = rand1.Rand( 0, 2 );
		if( nType0 )
		{
			float x0 = 0;
			auto pPlayer = GetStage()->GetWorld()->GetPlayer();
			if( pPlayer && pPlayer->GetLevel() )
				x0 = ( pPlayer->GetMoveTo().x + 1 ) * LEVEL_GRID_SIZE_X + pPlayer->GetLevel()->x - GetPosition().x;

			rect.x = rand1.Rand( 32, 64 ) * 2;
			rect.width = rand1.Rand( 18, 56 ) * 2;
			if( nDir )
				rect.x = x0 - rect.GetRight();
			else
				rect.x += x0;
		}
		else
		{
			rect.x = rand1.Rand( -16, 64 ) * 2;
			rect.width = rand1.Rand( 18, 56 ) * 2;
			if( nDir )
				rect.x = r0.x + r0.GetRight() - rect.GetRight();
		}
		int32 nCount = ( r0.GetRight() - rect.x ) / rect.width;
		float fOfsY = rand1.Rand( -3, 4 ) * 2;
		CVector3 color( rand1.Rand( 0.0f, 0.07f ), rand1.Rand( 0.0f, 0.07f ), rand1.Rand( 0.0f, 0.07f ) );
		for( int i = 0; i < nCount; i++ )
		{
			m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
			auto& elem = m_vecPlayerActionElems.back();
			elem.rect = rect;
			elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
			m_vecPlayerActionElemParams.push_back( CVector4( 1, 1, 1, ( nDir ? rect.width : -rect.width ) * ( i + 1 ) ) );
			auto color1 = color * -i;
			m_vecPlayerActionElemParams.push_back( CVector4( color1.x, color1.y, color1.z, -fOfsY * ( i + 1 ) ) );
			rect.x += nDir ? -rect.width : rect.width;
		}
	}
	if( nType == 0 || nType == 1 || nType0 )
	{
		auto rect = rect0;
		m_vecPlayerActionElems.resize( m_vecPlayerActionElems.size() + 1 );
		auto& elem = m_vecPlayerActionElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );

		CVector3 color( rand1.Rand( 0.0f, 0.025f ), rand1.Rand( 0.0f, 0.025f ), rand1.Rand( 0.0f, 0.025f ) );
		if( nType0 )
		{
			float f = Min( color.x, Min( color.y, color.z ) );
			color = color - CVector3( f, f, f );
			m_vecPlayerActionElemParams.push_back( CVector4( 1 - color.x * 3, 1 - color.y * 3, 1 - color.z * 3, rand1.Rand( -3, 4 ) * 2 ) );
			m_vecPlayerActionElemParams.push_back( CVector4( 0, 0, 0, rand1.Rand( -3, 4 ) * 2 ) );
		}
		else
		{
			m_vecPlayerActionElemParams.push_back( CVector4( 1, 1, 1, rand1.Rand( -3, 4 ) * 2 ) );
			m_vecPlayerActionElemParams.push_back( CVector4( -color.x, -color.y, -color.z, rand1.Rand( -3, 4 ) * 2 ) );
		}
	}

	if( nType0 )
	{
		for( int i = 0; i < m_vecPlayerActionElemParams.size(); i += 2 )
		{
			m_vecPlayerActionElemParams[i] = m_vecPlayerActionElemParams[i] * CVector4( 2.6f, 2.6f, 2.6f, 1 ) - CVector4( 1.4f, 1.4f, 1.4f, 1 );
			m_vecPlayerActionElemParams[i + 1] = m_vecPlayerActionElemParams[i + 1] - CVector4( 0.13f, 0.13f, 0.13f, 0 );
		}
	}
	else
	{
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
}

void CMainUI::FreezeEffect( int32 nLevel )
{
	auto r0 = GetStage()->GetCamera().GetViewArea();
	r0 = r0.Offset( globalTransform.GetPosition() * -1 );

	static SRand rd0 = SRand::Inst<eRand_Render>();
	if( m_bResetFreezeEft )
	{
		rd0 = SRand::Inst<eRand_Render>();
		SRand::Inst<eRand_Render>().Rand();
		m_bResetFreezeEft = false;
	}
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

void SWorldDataFrame::SLevelData::Load( IBufReader& buf, int32 nVersion )
{
	buf.Read( bVisited );
	if( nVersion >= 5 )
		buf.Read( bIgnoreGlobalClearKeys );
	int32 n1 = buf.Read<int32>();
	for( int i1 = 0; i1 < n1; i1++ )
	{
		string key1;
		buf.Read( key1 );
		buf.Read( mapDataInt[key1] );
	}
	if( nVersion )
	{
		n1 = buf.Read<int32>();
		for( int i1 = 0; i1 < n1; i1++ )
		{
			string key1;
			buf.Read( key1 );
			buf.Read( mapDataString[key1] );
		}
	}
	n1 = buf.Read<int32>();
	for( int i1 = 0; i1 < n1; i1++ )
	{
		string key1;
		buf.Read( key1 );
		buf.Read( mapDataDeadPawn[key1] );
	}
}

void SWorldDataFrame::SLevelData::Save( CBufFile& buf )
{
	buf.Write( bVisited );
	buf.Write( bIgnoreGlobalClearKeys );
	buf.Write( mapDataInt.size() );
	for( auto& item1 : mapDataInt )
	{
		buf.Write( item1.first );
		buf.Write( item1.second );
	}
	buf.Write( mapDataString.size() );
	for( auto& item1 : mapDataString )
	{
		buf.Write( item1.first );
		buf.Write( item1.second );
	}
	buf.Write( mapDataDeadPawn.size() );
	for( auto& item1 : mapDataDeadPawn )
	{
		buf.Write( item1.first );
		buf.Write( item1.second );
	}
}

void SWorldDataFrame::SLevelSnapShot::Load( IBufReader& buf, int32 nVersion )
{
	levelData.Load( buf, nVersion );
	int32 n;
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		buf.Read( mapDataInt[key] );
	}
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		buf.Read( mapDataString[key] );
	}
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		buf.Read( mapDataIntStatic[key] );
	}
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		buf.Read( mapDataStringStatic[key] );
	}
}

void SWorldDataFrame::SLevelSnapShot::Save( CBufFile & buf )
{
	levelData.Save( buf );
	buf.Write( mapDataInt.size() );
	for( auto& item : mapDataInt )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
	buf.Write( mapDataString.size() );
	for( auto& item : mapDataString )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
	buf.Write( mapDataIntStatic.size() );
	for( auto& item : mapDataIntStatic )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
	buf.Write( mapDataStringStatic.size() );
	for( auto& item : mapDataStringStatic )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
}

void SWorldDataFrame::Load( IBufReader& buf, int32 nVersion )
{
	buf.Read( strCurLevel );
	buf.Read( strLastLevel );
	buf.Read( playerEnterPos );
	buf.Read( nPlayerEnterDir );
	buf.Read( bForceAllVisible );
	buf.Read( playerData );
	if( nVersion >= 10 )
		buf.Read( pawnData );
	if( nVersion >= 9 )
	{
		buf.Read( strGlobalBGM );
		buf.Read( nGlobalBGMPriority );
	}

	int32 n;
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		auto& item = mapLevelData[key];
		item.Load( buf, nVersion );
	}
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		buf.Read( mapDataInt[key] );
	}
	if( nVersion >= 1 )
	{
		buf.Read( n );
		for( int i = 0; i < n; i++ )
		{
			string key;
			buf.Read( key );
			buf.Read( mapDataString[key] );
		}
	}
	if( nVersion >= 2 )
	{
		buf.Read( n );
		for( int i = 0; i < n; i++ )
		{
			string key;
			buf.Read( key );
			buf.Read( mapDataIntStatic[key] );
		}
		buf.Read( n );
		for( int i = 0; i < n; i++ )
		{
			string key;
			buf.Read( key );
			buf.Read( mapDataStringStatic[key] );
		}
	}
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		unlockedRegionMaps.insert( key );
	}
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string key;
		buf.Read( key );
		auto& item = mapLevelMarks[key];
		buf.Read( item.strLevelName );
		buf.Read( item.ofs );
	}
	if( nVersion >= 4 )
	{
		buf.Read( strTracer );
		buf.Read( nTracerSpawnDelay );
		if( nVersion >= 6 )
		{
			buf.Read( strTracerLevel );
			buf.Read( tracerLevelEnterPos );
		}
	}
	if( nVersion >= 3 )
	{
		buf.Read( n );
		vecScenarioRecords.resize( n );
		for( int i = 0; i < n; i++ )
		{
			buf.Read( vecScenarioRecords[i].nType );
			buf.Read( vecScenarioRecords[i].color );
			buf.Read( vecScenarioRecords[i].str );
		}
	}
	if( nVersion >= 8 )
	{
		buf.Read( n );
		vecPlayerDataStack.resize( n );
		for( int i = 0; i < n; i++ )
			buf.Read( vecPlayerDataStack[i] );
	}
}

void SWorldDataFrame::Save( CBufFile& buf )
{
	buf.Write( strCurLevel );
	buf.Write( strLastLevel );
	buf.Write( playerEnterPos );
	buf.Write( nPlayerEnterDir );
	buf.Write( bForceAllVisible );
	buf.Write( playerData );
	buf.Write( pawnData );
	buf.Write( strGlobalBGM );
	buf.Write( nGlobalBGMPriority );

	buf.Write( mapLevelData.size() );
	for( auto& item : mapLevelData )
	{
		buf.Write( item.first );
		item.second.Save( buf );
	}
	buf.Write( mapDataInt.size() );
	for( auto& item : mapDataInt )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
	buf.Write( mapDataString.size() );
	for( auto& item : mapDataString )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
	buf.Write( mapDataIntStatic.size() );
	for( auto& item : mapDataIntStatic )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
	buf.Write( mapDataStringStatic.size() );
	for( auto& item : mapDataStringStatic )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
	buf.Write( unlockedRegionMaps.size() );
	for( auto& item : unlockedRegionMaps )
		buf.Write( item );
	buf.Write( mapLevelMarks.size() );
	for( auto& item : mapLevelMarks )
	{
		buf.Write( item.first );
		buf.Write( item.second.strLevelName );
		buf.Write( item.second.ofs );
	}
	buf.Write( strTracer );
	buf.Write( nTracerSpawnDelay );
	buf.Write( strTracerLevel );
	buf.Write( tracerLevelEnterPos );
	buf.Write( vecScenarioRecords.size() );
	for( auto& item : vecScenarioRecords )
	{
		buf.Write( item.nType );
		buf.Write( item.color );
		buf.Write( item.str );
	}
	buf.Write( vecPlayerDataStack.size() );
	for( auto& item : vecPlayerDataStack )
	{
		buf.Write( item );
	}
}

void SWorldData::Load( IBufReader& buf )
{
	int32 nVersion = buf.Read<int32>();
	buf.Read( nCurFrameCount );
	for( int i = 0; i < nCurFrameCount; i++ )
	{
		auto pFrame = new SWorldDataFrame;
		pFrame->Load( buf, nVersion );
		backupFrames.push_back( pFrame );
		if( nVersion >= 7 )
		{
			buf.Read( pFrame->curLevelSnapShot.bValid );
			if( pFrame->curLevelSnapShot.bValid )
				pFrame->curLevelSnapShot.Load( buf, nVersion );

			int32 n;
			buf.Read( n );
			for( int i = 0; i < n; i++ )
			{
				string key;
				buf.Read( key );
				auto& item = pFrame->mapClearedSnapShot[key];
				item.bValid = true;
				item.Load( buf, nVersion );
			}
		}
	}
	if( buf.Read<int8>() )
	{
		pCheckPoint = new SWorldDataFrame;
		pCheckPoint->Load( buf, nVersion );
	}
	if( nVersion >= 7 )
	{
		int32 n;
		buf.Read( n );
		for( int i = 0; i < n; i++ )
		{
			string key;
			buf.Read( key );
			auto& item = mapSnapShotCur[key];
			item.bValid = true;
			item.Load( buf, nVersion );
		}
		buf.Read( n );
		for( int i = 0; i < n; i++ )
		{
			string key;
			buf.Read( key );
			auto& item = mapSnapShotCheckPoint[key];
			item.bValid = true;
			item.Load( buf, nVersion );
		}
	}
}

void SWorldData::Save( CBufFile& buf )
{
	int32 nVersion = 10;
	buf.Write( nVersion );
	buf.Write( nCurFrameCount );
	for( int i = 0; i < nCurFrameCount; i++ )
	{
		auto pFrame = backupFrames[i];
		pFrame->Save( buf );

		buf.Write( pFrame->curLevelSnapShot.bValid );
		if( pFrame->curLevelSnapShot.bValid )
			pFrame->curLevelSnapShot.Save( buf );

		buf.Write( pFrame->mapClearedSnapShot.size() );
		for( auto& item : pFrame->mapClearedSnapShot )
		{
			buf.Write( item.first );
			item.second.Save( buf );
		}
	}
	if( pCheckPoint )
	{
		buf.Write<int8>( 1 );
		pCheckPoint->Save( buf );
	}
	else
		buf.Write<int8>( 0 );

	buf.Write( mapSnapShotCur.size() );
	for( auto& item : mapSnapShotCur )
	{
		buf.Write( item.first );
		item.second.Save( buf );
	}
	buf.Write( mapSnapShotCheckPoint.size() );
	for( auto& item : mapSnapShotCheckPoint )
	{
		buf.Write( item.first );
		item.second.Save( buf );
	}
}

void SWorldData::OnEnterLevel( const char* szCurLevel, CPlayer* pPlayer, const TVector2<int32>& playerPos, int8 nPlayerDir, vector<CReference<CPawn> >* pVecPawns, bool bClearSnapShot, int8 nPlayerDataOpr )
{
	if( bClearSnapShot )
	{
		curFrame.mapClearedSnapShot = mapSnapShotCur;
		mapSnapShotCur.clear();
	}
	else
	{
		curFrame.mapClearedSnapShot.clear();
		if( curFrame.strCurLevel.length() )
		{
			auto& snapshot = mapSnapShotCur[curFrame.strCurLevel];
			snapshot.bValid = 1;
			snapshot.levelData = curFrame.mapLevelData[curFrame.strCurLevel];
			snapshot.mapDataInt = curFrame.mapDataInt;
			snapshot.mapDataIntStatic = curFrame.mapDataIntStatic;
			snapshot.mapDataString = curFrame.mapDataString;
			snapshot.mapDataStringStatic = curFrame.mapDataStringStatic;
		}
	}

	curFrame.strLastLevel = curFrame.strCurLevel;
	curFrame.strCurLevel = szCurLevel;
	curFrame.playerEnterPos = playerPos;
	curFrame.nPlayerEnterDir = nPlayerDir;
	GetCurLevelData().bVisited = true;
	if( nPlayerDataOpr == -1 )
	{
		pPlayer->LoadData( curFrame.vecPlayerDataStack.back() );
		curFrame.vecPlayerDataStack.pop_back();
	}
	curFrame.playerData.Clear();
	pPlayer->SaveData( curFrame.playerData );
	if( nPlayerDataOpr == 1 )
		curFrame.vecPlayerDataStack.push_back( curFrame.playerData );
	curFrame.pawnData.Clear();
	if( pVecPawns )
	{
		auto& vec = *pVecPawns;
		curFrame.pawnData.Write( vec.size() - 1 );
		for( CPawn* pPawn : vec )
		{
			if( pPawn != pPlayer )
			{
				CBufFile buf;
				buf.Write( pPawn->strCreatedFrom );
				pPawn->SaveData( buf );
				curFrame.pawnData.Write( buf );
			}
		}
	}
	else
		curFrame.pawnData.Write( 0 );

	auto itr = mapSnapShotCur.find( szCurLevel );
	if( itr != mapSnapShotCur.end() )
		curFrame.curLevelSnapShot = itr->second;
	else
		curFrame.curLevelSnapShot.Clear();

	SWorldDataFrame* p = NULL;
	if( nCurFrameCount >= nMaxFrameCount )
	{
		p = backupFrames.front();
		backupFrames.pop_front();
		auto p1 = backupFrames.front();
		auto n = Min<int32>( p->vecScenarioRecords.size(), MAX_SCENARIO_RECORDS - p1->vecScenarioRecords.size() );
		for( int i = p->vecScenarioRecords.size() - 1; i >= (int32)p->vecScenarioRecords.size() - n; i-- )
			p1->vecScenarioRecords.push_front( p->vecScenarioRecords[i] );
		backupFrames.push_back( p );
	}
	else
	{
		if( nCurFrameCount >= backupFrames.size() )
			backupFrames.push_back( new SWorldDataFrame );
		p = backupFrames[nCurFrameCount++];
	}
	*p = curFrame;
	curFrame.vecScenarioRecords.clear();
}

void SWorldData::OnReset( CPlayer* pPlayer, vector<CReference<CPawn> >& vecPawns )
{
	auto p = nCurFrameCount ? backupFrames[nCurFrameCount - 1]: pCheckPoint;
	ASSERT( p );
	curFrame = *p;
	curFrame.vecScenarioRecords.clear();
	curFrame.playerData.ResetCurPos();
	pPlayer->LoadData( curFrame.playerData );
	curFrame.pawnData.ResetCurPos();
	int32 nPawn = 0;
	curFrame.pawnData.Read( nPawn );
	if( nPawn )
	{
		vecPawns.resize( nPawn );
		for( int i = 0; i < nPawn; i++ )
		{
			CBufFile buf;
			curFrame.pawnData.Read( buf );
			string str;
			buf.Read( str );

			auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( str.c_str() );
			auto pPawn = SafeCast<CPawn>( pPrefab->GetRoot()->CreateInstance() );
			pPawn->LoadData( buf );
			vecPawns[i] = pPawn;
		}
	}
}

void SWorldData::OnRetreat( CPlayer* pPlayer, vector<CReference<CPawn> >& vecPawns )
{
	if( nCurFrameCount > 1 || pCheckPoint && nCurFrameCount )
	{
		nCurFrameCount--;
		if( !nCurFrameCount )
		{
			mapSnapShotCur = mapSnapShotCheckPoint;
		}
		else if( curFrame.mapClearedSnapShot.size() )
		{
			mapSnapShotCur = curFrame.mapClearedSnapShot;
		}
		else
		{
			auto& snapShot = backupFrames[nCurFrameCount - 1]->curLevelSnapShot;
			if( !snapShot.bValid )
				mapSnapShotCur.erase( curFrame.strLastLevel );
			else
				mapSnapShotCur[curFrame.strLastLevel] = snapShot;
		}
	}
	OnReset( pPlayer, vecPawns );
}

void SWorldData::CheckPoint( CPlayer* pPlayer )
{
	auto pCheckPoint0 = pCheckPoint;
	pCheckPoint = new SWorldDataFrame;
	*pCheckPoint = curFrame;
	pCheckPoint->playerEnterPos = pPlayer->GetPos();
	pCheckPoint->nPlayerEnterDir = pPlayer->GetCurDir();
	pCheckPoint->playerData.Clear();
	pPlayer->SaveData( pCheckPoint->playerData );
	for( int iFrame = nCurFrameCount - 1; iFrame >= -1; iFrame-- )
	{
		auto p = iFrame == -1 ? pCheckPoint0 : backupFrames[iFrame];
		if( !p )
			break;
		auto n = Min<int32>( p->vecScenarioRecords.size(), MAX_SCENARIO_RECORDS - pCheckPoint->vecScenarioRecords.size() );
		for( int i = p->vecScenarioRecords.size() - 1; i >= (int32)p->vecScenarioRecords.size() - n; i-- )
			pCheckPoint->vecScenarioRecords.push_front( p->vecScenarioRecords[i] );
		if( pCheckPoint->vecScenarioRecords.size() >= MAX_SCENARIO_RECORDS )
			break;
	}
	nCurFrameCount = 0;
	curFrame.vecScenarioRecords.clear();
	mapSnapShotCheckPoint = mapSnapShotCur;
	if( pCheckPoint0 )
		delete pCheckPoint0;
}

void SWorldData::OnRestoreToCheckpoint( CPlayer* pPlayer, vector<CReference<CPawn> >& vecPawns )
{
	if( !pCheckPoint )
		return OnReset( pPlayer, vecPawns );
	nCurFrameCount = 0;
	mapSnapShotCur = mapSnapShotCheckPoint;
	curFrame = *pCheckPoint;
	curFrame.playerData.ResetCurPos();
	pPlayer->LoadData( curFrame.playerData );
	curFrame.pawnData.ResetCurPos();
	int32 nPawn = 0;
	curFrame.pawnData.Read( nPawn );
	if( nPawn )
	{
		vecPawns.resize( nPawn );
		for( int i = 0; i < nPawn; i++ )
		{
			CBufFile buf;
			curFrame.pawnData.Read( buf );
			string str;
			buf.Read( str );

			auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( str.c_str() );
			auto pPawn = SafeCast<CPawn>( pPrefab->GetRoot()->CreateInstance() );
			pPawn->LoadData( buf );
			vecPawns[i] = pPawn;
		}
	}

	curFrame.vecScenarioRecords.clear();
}

void SWorldData::ClearKeys()
{
	vector<string> vecTemp;
	ClearKeys( curFrame.mapDataInt, vecTemp );
	ClearKeys( curFrame.mapDataString, vecTemp );
	for( auto& item : curFrame.mapLevelData )
	{
		if( item.second.bIgnoreGlobalClearKeys )
			continue;
		ClearKeys( item.second.mapDataDeadPawn, vecTemp );
		ClearKeys( item.second.mapDataInt, vecTemp );
		ClearKeys( item.second.mapDataString, vecTemp );
	}
}

void SWorldData::Respawn()
{
	vector<string> vecTemp;
	for( auto& item : curFrame.mapLevelData )
	{
		if( item.second.bIgnoreGlobalClearKeys )
			continue;
		if( item.first != curFrame.strCurLevel )
			ClearKeys( item.second.mapDataDeadPawn, vecTemp );
	}
}

void SWorldData::RespawnLevel( const char* szLevel )
{
	auto itr = curFrame.mapLevelData.find( szLevel );
	if( itr == curFrame.mapLevelData.end() )
		return;

	if( itr->first != curFrame.strCurLevel )
	{
		vector<string> vecTemp;
		ClearKeys( itr->second.mapDataDeadPawn, vecTemp );
	}
}

void SWorldData::ClearByPrefix( const char* sz )
{
	string strLevelPrefix = "stages/";
	strLevelPrefix += sz;
	auto itr1 = curFrame.mapLevelData.lower_bound( strLevelPrefix.c_str() );
	auto itr2 = curFrame.mapLevelData.upper_bound( strLevelPrefix.c_str() );
	vector<string> vecTemp;
	ClearKeysByPrefix( curFrame.mapDataInt, sz, vecTemp );
	ClearKeysByPrefix( curFrame.mapDataString, sz, vecTemp );
	for( ; itr1 != itr2; itr1++ )
	{
		if( itr1->second.bIgnoreGlobalClearKeys )
			continue;
		ClearKeys( itr1->second.mapDataDeadPawn, vecTemp );
		ClearKeys( itr1->second.mapDataInt, vecTemp );
		ClearKeys( itr1->second.mapDataString, vecTemp );
	}
}

void SWorldData::SetLevelIgnoreGlobalClearKeys( const char* szLevel, bool b )
{
	curFrame.mapLevelData[szLevel].bIgnoreGlobalClearKeys = b;
}

void SWorldData::UnlockRegionMap( const char* szRegion )
{
	curFrame.unlockedRegionMaps.insert( szRegion );
}

void SWorldData::GetScenarioRecords( function<void( int8, const CVector4&, const char* )> Func )
{
	int32 nRecord = 0;
	for( int i = curFrame.vecScenarioRecords.size() - 1; i >= 0; i-- )
	{
		auto& item = curFrame.vecScenarioRecords[i];
		Func( item.nType, item.color, item.str.c_str() );
		nRecord++;
		if( nRecord >= MAX_SCENARIO_RECORDS )
			return;
	}
	for( int iFrame = nCurFrameCount - 1; iFrame >= 0; iFrame-- )
	{
		auto pFrame = backupFrames[iFrame];
		for( int i = pFrame->vecScenarioRecords.size() - 1; i >= 0; i-- )
		{
			auto& item = pFrame->vecScenarioRecords[i];
			Func( item.nType, item.color, item.str.c_str() );
			nRecord++;
			if( nRecord >= MAX_SCENARIO_RECORDS )
				return;
		}
	}
	if( pCheckPoint )
	{
		for( int i = pCheckPoint->vecScenarioRecords.size() - 1; i >= 0; i-- )
		{
			auto& item = pCheckPoint->vecScenarioRecords[i];
			Func( item.nType, item.color, item.str.c_str() );
			nRecord++;
			if( nRecord >= MAX_SCENARIO_RECORDS )
				return;
		}
	}
}

void SWorldData::OnScenarioText( int8 n, const char* sz, const CVector4& color )
{
	SWorldDataFrame::SScenarioRecord record = { n, color, sz };
	curFrame.vecScenarioRecords.push_back( record );
}

void CMasterLevel::OnAddedToStage()
{
	m_pMenu->bVisible = false;
	m_pWorldMap->bVisible = false;
	m_pActionPreview->bVisible = false;
	m_pLogUI->bVisible = false;
}

void CMasterLevel::NewGame( CPlayer* pPlayer, CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir )
{
	m_pPlayer = pPlayer;
	CLuaMgr::Inst().Run( CGlobalCfg::Inst().strWorldInitScript.c_str() );
	auto p = pLevelPrefab->GetRoot()->CreateInstance();
	auto pLevel = SafeCast<CMyLevel>( p );
	if( pLevel )
	{
		m_pCurLevelPrefab = pLevelPrefab;
		m_worldData.OnEnterLevel( pLevelPrefab->GetName(), m_pPlayer, playerPos, nPlayerDir, NULL, false );
		m_pCurLevel = pLevel;
		pLevel->SetParentBeforeEntity( m_pLevelFadeMask );
		pLevel->Init();
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
		pLevel->AddPawn( m_pPlayer, playerPos, nPlayerDir );
		BeginCurLevel();
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

void CMasterLevel::Continue( CPlayer* pPlayer, IBufReader& buf )
{
	m_pPlayer = pPlayer;
	m_worldData.Load( buf );
	JumpBack( 0 );
}

void CMasterLevel::Save()
{
	CBufFile buf;
	m_worldData.Save( buf );
	SaveFile( "save/a", buf.GetBuffer(), buf.GetBufLen() );
}

bool CMasterLevel::TransferTo( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType, int32 nTransferParam )
{
	if( m_pCurLevel && !m_pCurLevel->OnPlayerTryToLeave( playerPos, nPlayerDir, nTransferType, nTransferParam ) )
		return false;
	ResetMainUI();
	m_nPlayerDamageFrame = 0;
	m_transferPos = playerPos;
	m_nTransferDir = nPlayerDir;
	m_nTransferType = nTransferType;
	m_nTransferParam = nTransferParam;
	m_pTransferTo = pLevelPrefab;

	struct _STemp
	{
		static uint32 Func( void* pThis )
		{
			( ( CMasterLevel* )pThis )->TransferFunc();
			return 1;
		}
	};
	m_pTransferCoroutine = TCoroutinePool<0x10000>::Inst().Alloc();
	m_pTransferCoroutine->Create( &_STemp::Func, this );
	m_pTransferCoroutine->Resume();
	if( m_pTransferCoroutine->GetState() == ICoroutine::eState_Stopped )
	{
		TCoroutinePool<0x10000>::Inst().Free( m_pTransferCoroutine );
		m_pTransferCoroutine = NULL;
	}
	return true;
}

void CMasterLevel::JumpBack( int8 nType )
{
	ASSERT( !m_pLastLevel && !m_pCurCutScene );
	if( m_pCurLevel )
	{
		EndCurLevel();
		m_pCurLevel->SetParentEntity( NULL );
	}

	vector<CReference<CPawn> > vecPawns;
	if( nType == 0 )
		m_worldData.OnReset( m_pPlayer, vecPawns );
	else if( nType == 1 )
	{
		m_worldData.OnRetreat( m_pPlayer, vecPawns );
		if( !m_worldData.nCurFrameCount )
			RemoveAllSnapShot();
	}
	else
	{
		m_worldData.OnRestoreToCheckpoint( m_pPlayer, vecPawns );
		RemoveAllSnapShot();
	}
	RefreshMainUI();

	if( !m_pCurLevelPrefab || m_worldData.curFrame.strCurLevel != m_pCurLevelPrefab->GetName() )
		m_pCurLevelPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_worldData.curFrame.strCurLevel.c_str() );
	auto pLevel = SafeCast<CMyLevel>( m_pCurLevelPrefab->GetRoot()->CreateInstance() );
	m_pCurLevel = pLevel;
	pLevel->SetParentBeforeEntity( m_pLevelFadeMask );
	pLevel->Init();
	if( pLevel->GetEnvEffect() )
		pLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
	pLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
	for( CPawn* pPawn : vecPawns )
		pLevel->AddPawn( pPawn, pPawn->m_pos, pPawn->m_nCurDir );
	BeginCurLevel();
}

SWorldDataFrame::SLevelData& CMasterLevel::GetCurLevelData()
{
	if( m_strUpdatingSnapShot.length() )
		return m_worldData.mapSnapShotCur[m_strUpdatingSnapShot.c_str()].levelData;
	return m_worldData.GetCurLevelData();
}

void CMasterLevel::BeginScenario()
{
	m_worldData.OnScenarioText( -1, "", CVector4( 0, 0, 0, 0 ) );
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

void CMasterLevel::ForceEndScenario()
{
	if( !IsScenario() )
		return;
	m_pScenarioScript = NULL;
	EndScenario();
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

void CMasterLevel::CheckPoint( bool bRefresh, bool bIgnoreSave )
{
	m_pCurLevel->OnCheckPoint();
	m_worldData.CheckPoint( m_pPlayer );
	if( bRefresh )
	{
		m_pScriptTransferTo = NULL;
		m_nScriptTransferType = -1;
	}
	if( !bIgnoreSave )
		Save();
}

int32 CMasterLevel::EvaluateKeyIntLevelData( const char* str, SWorldDataFrame::SLevelData& levelData )
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

	if( str[0] == '&' )
	{
		if( 0 == strncmp( &str[1], "dead&", 5 ) )
		{
			auto itr = levelData.mapDataDeadPawn.find( &str[6] );
			return itr == levelData.mapDataDeadPawn.end() ? 0 : 1;
		}
	}
	if( str[0] == '$' )
	{
		str++;
		auto itr = levelData.mapDataInt.find( str );
		if( itr == levelData.mapDataInt.end() )
			return 0;
		return itr->second;
	}
	else if( str[0] == '%' )
	{
		str++;
		auto& data = m_strUpdatingSnapShot.length() ? m_worldData.mapSnapShotCur[m_strUpdatingSnapShot.c_str()].mapDataIntStatic : m_worldData.curFrame.mapDataIntStatic;
		auto itr = data.find( str );
		if( itr == data.end() )
			return 0;
		return itr->second;
	}
	else
	{
		auto& data = m_strUpdatingSnapShot.length() ? m_worldData.mapSnapShotCur[m_strUpdatingSnapShot.c_str()].mapDataInt : m_worldData.curFrame.mapDataInt;
		auto itr = data.find( str );
		if( itr == data.end() )
			return 0;
		return itr->second;
	}
}

const char* CMasterLevel::EvaluateKeyStringLevelData( const char* str, SWorldDataFrame::SLevelData& levelData )
{
	if( str[0] == ':' )
	{
		CLuaMgr::Inst().Run( str + 1, 1 );
		return CLuaMgr::Inst().PopLuaValue<const char*>();
	}
	else if( str[0] == '=' )
	{
		string strScript = "return ";
		strScript += str + 1;
		CLuaMgr::Inst().Run( strScript.c_str(), 1 );
		return CLuaMgr::Inst().PopLuaValue<const char*>();
	}

	if( str[0] == '$' )
	{
		str++;
		auto itr = levelData.mapDataString.find( str );
		if( itr == levelData.mapDataString.end() )
			return "";
		return itr->second.c_str();
	}
	else if( str[0] == '%' )
	{
		str++;
		auto& data = m_strUpdatingSnapShot.length() ? m_worldData.mapSnapShotCur[m_strUpdatingSnapShot.c_str()].mapDataStringStatic : m_worldData.curFrame.mapDataStringStatic;
		auto itr = data.find( str );
		if( itr == data.end() )
			return "";
		return itr->second.c_str();
	}
	else
	{
		auto& data = m_strUpdatingSnapShot.length() ? m_worldData.mapSnapShotCur[m_strUpdatingSnapShot.c_str()].mapDataString : m_worldData.curFrame.mapDataString;
		auto itr = data.find( str );
		if( itr == data.end() )
			return "";
		return itr->second.c_str();
	}
}

void CMasterLevel::SetKeyIntLevelData( const char* str, int32 n, SWorldDataFrame::SLevelData& levelData )
{
	if( m_strUpdatingSnapShot.length() )
		return;
	if( str[0] == '$' )
	{
		str++;
		levelData.mapDataInt[str] = n;
	}
	else if( str[0] == '%' )
	{
		str++;
		m_worldData.curFrame.mapDataIntStatic[str] = n;
	}
	else
		m_worldData.curFrame.mapDataInt[str] = n;
}

void CMasterLevel::SetKeyStringLevelData( const char* str, const char* szValue, SWorldDataFrame::SLevelData& levelData )
{
	if( m_strUpdatingSnapShot.length() )
		return;
	if( str[0] == '$' )
	{
		str++;
		levelData.mapDataString[str] = szValue;
	}
	else if( str[0] == '%' )
	{
		str++;
		m_worldData.curFrame.mapDataStringStatic[str] = szValue;
	}
	else
		m_worldData.curFrame.mapDataString[str] = szValue;
}

void CMasterLevel::TransferTo1( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType, int32 nTransferParam )
{
	m_pScriptTransferTo = pLevelPrefab;
	m_nScriptTransferPlayerX = playerPos.x;
	m_nScriptTransferPlayerY = playerPos.y;
	m_nScriptTransferPlayerDir = nPlayerDir;
	m_nScriptTransferType = nTransferType;
	m_nScriptTransferParam = nTransferParam;
	m_pScriptTransferOpr = NULL;
}

void CMasterLevel::TransferBy( int32 nNxtStage, int8 nTransferType, int32 nTransferParam )
{
	auto& nxtStage = m_pCurLevel->GetNextLevelData( nNxtStage );
	TransferTo1( nxtStage.pNxtStage, m_pPlayer->GetMoveTo() -
		TVector2<int32>( nxtStage.nOfsX, nxtStage.nOfsY ), m_pPlayer->GetCurDir(), nTransferType, nTransferParam );
}

void CMasterLevel::ScriptTransferTo( const char* szName, int32 nPlayerX, int32 nPlayerY, int8 nPlayerDir, int8 nTransferType, int32 nTransferParam )
{
	m_pScriptTransferTo = CResourceManager::Inst()->CreateResource<CPrefab>( szName );
	m_nScriptTransferPlayerX = nPlayerX;
	m_nScriptTransferPlayerY = nPlayerY;
	m_nScriptTransferPlayerDir = nPlayerDir;
	m_nScriptTransferType = nTransferType;
	m_nScriptTransferParam = nTransferParam;
	m_pScriptTransferOpr = NULL;
}

void CMasterLevel::ScriptTransferOprFunc()
{
	auto pCoroutine = CLuaMgr::GetCurLuaState()->CreateCoroutineAuto();
	ASSERT( pCoroutine );
	m_pScriptTransferOpr = pCoroutine;
}

void CMasterLevel::ShowWorldMap( bool bShow, int8 nType )
{
	auto pWorldMap = SafeCast<CWorldMapUI>( m_pWorldMap.GetPtr() );
	if( bShow )
	{
		if( !pWorldMap->Show( nType ) )
		{
			if( nType == 1 )
			{
				BlackOut( 10, 0 );
				m_pPlayer->SetHp( m_pPlayer->GetMaxHp() );
				Respawn();
				CheckPoint( true );
			}
			return;
		}
		if( nType == 0 )
			ShowMenu( true, eMenuPage_Map );
		pWorldMap->bVisible = true;
		pWorldMap->SetPosition( GetCamPos() );
		m_pCurLevel->Freeze();
	}
	else
	{
		ShowMenu( false, 0 );
		pWorldMap->bVisible = false;
		m_pCurLevel->UnFreeze();
	}
}

void CMasterLevel::AddLevelMark( const char* szKey, const char* szLevel, int32 x, int32 y )
{
	auto& mark = m_worldData.curFrame.mapLevelMarks[szKey];
	mark.strLevelName = szLevel;
	mark.ofs = TVector2<int32>( x, y );
}

bool CMasterLevel::HasLevelMark( const char* szKey )
{
	return m_worldData.curFrame.mapLevelMarks.find( szKey ) != m_worldData.curFrame.mapLevelMarks.end();
}

void CMasterLevel::RemoveLevelMark( const char* szKey )
{
	m_worldData.curFrame.mapLevelMarks.erase( szKey );
}

void CMasterLevel::ShowActionPreview( bool bShow )
{
	auto pActionPreview = SafeCast<CActionPreview>( m_pActionPreview.GetPtr() );
	if( bShow )
	{
		pActionPreview->bVisible = true;
		pActionPreview->Show( true );
		ShowMenu( true, eMenuPage_ActionPreview );
		pActionPreview->bVisible = true;
		pActionPreview->SetPosition( GetCamPos() );
		m_pCurLevel->Freeze();
	}
	else
	{
		ShowMenu( false, 0 );
		pActionPreview->Show( false );
		pActionPreview->bVisible = false;
		m_pCurLevel->UnFreeze();
	}
}

void CMasterLevel::ShowLogUI( bool bShow, int8 nPage, int32 nIndex )
{
	auto pLogUI = SafeCast<CLogUI>( m_pLogUI.GetPtr() );
	if( bShow )
	{
		pLogUI->bVisible = true;
		pLogUI->Show( nPage, nIndex );
		ShowMenu( true, eMenuPage_Log );
		pLogUI->bVisible = true;
		pLogUI->SetPosition( GetCamPos() );
		m_pCurLevel->Freeze();
	}
	else
	{
		ShowMenu( false, 0 );
		pLogUI->bVisible = false;
		m_pCurLevel->UnFreeze();
	}
}

void CMasterLevel::ShowMenu( bool bShow, int8 nCurPage )
{
	if( bShow )
	{
		m_pMenu->bVisible = true;
		m_pMenu->SetPosition( GetCamPos() );
		m_nEnabledPageCount = 0;
		m_nMenuPage = nCurPage;
		m_nMenuPageItemIndex = -1;
		const char* szMenuItemName[] = { "Action", "Map", "Log", "4", "5" };
		for( int i = 0; i < 5; i++ )
		{
			if( IsMenuPageEnabled( i ) )
			{
				auto pText = SafeCast<CSimpleText>( m_pMenuItem[m_nEnabledPageCount].GetPtr() );
				pText->Set( szMenuItemName[i] );
				pText->bVisible = true;
				if( i == nCurPage )
				{
					m_nMenuPageItemIndex = m_nEnabledPageCount;
					m_pMenuSelected->SetPosition( pText->GetPosition() );
				}
				m_enabledPages[m_nEnabledPageCount++] = i;
			}
		}
		for( int i = m_nEnabledPageCount; i < ELEM_COUNT( m_pMenuItem ); i++ )
			m_pMenuItem[i]->bVisible = false;
	}
	else
		m_pMenu->bVisible = false;
}

void CMasterLevel::SwitchMenuPage( int8 nPage )
{
	if( m_nMenuPage == eMenuPage_ActionPreview )
		ShowActionPreview( false );
	else if( m_nMenuPage == eMenuPage_Map )
		ShowWorldMap( false );
	else if( m_nMenuPage == eMenuPage_Log )
		ShowLogUI( false );
	if( nPage < 0 )
	{
		ShowMenu( false, 0 );
		return;
	}
	m_nMenuPage = nPage;
	if( m_nMenuPage == eMenuPage_ActionPreview )
		ShowActionPreview( true );
	else if( m_nMenuPage == eMenuPage_Map )
		ShowWorldMap( true );
	else if( m_nMenuPage == eMenuPage_Log )
		ShowLogUI( true );
}

bool CMasterLevel::IsMenuPageEnabled( int8 nPage )
{
	if( nPage == eMenuPage_ActionPreview )
		return true;
	if( nPage == eMenuPage_Map )
		return SafeCast<CWorldMapUI>( m_pWorldMap.GetPtr() )->IsEnabled();
	if( nPage == eMenuPage_Log )
		return true;
	return false;
}

CEntity* CMasterLevel::ShowInteractionUI( CPawn* pPawn, const char* szName )
{
	if( m_pInteractionUI )
		return NULL;
	m_pInteractionUIPawn = pPawn;
	m_strInteractionUI = szName;

	CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strInteractionUI.c_str() );
	if( pPrefab && pPrefab->GetRoot()->GetStaticDataSafe<CInteractionUI>() )
	{
		auto p = SafeCast<CInteractionUI>( pPrefab->GetRoot()->CreateInstance() );
		m_pInteractionUI = p;
		m_bInteractionUIInit = true;
	}
	else
	{
		m_strInteractionUI = "";
		m_pInteractionUIPawn = NULL;
	}
	return m_pInteractionUI;
}

CEntity* CMasterLevel::GotoInteractionUI( const char* szName )
{
	if( m_pInteractionUI )
	{
		m_pInteractionUI->SetParentEntity( NULL );
		m_pInteractionUI = NULL;
		m_strInteractionUI = "";
	}

	m_strInteractionUI = szName;
	CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strInteractionUI.c_str() );
	if( pPrefab && pPrefab->GetRoot()->GetStaticDataSafe<CInteractionUI>() )
	{
		auto p = SafeCast<CInteractionUI>( pPrefab->GetRoot()->CreateInstance() );
		m_pInteractionUI = p;
		p->SetZOrder( 1 );
		p->SetParentEntity( this );
		p->SetPosition( GetCamPos() );
		p->Init( m_pInteractionUIPawn );
	}
	else
	{
		m_strInteractionUI = "";
		m_pInteractionUIPawn = NULL;
	}
	return m_pInteractionUI;
}

void CMasterLevel::BlackOut( int32 nFrame1, int32 nFrame2 )
{
	m_nBlackOutFrame1 = nFrame1;
	m_nBlackOutFrame2 = nFrame2;
}

void CMasterLevel::InterferenceStripEffect( int8 nType, float fSpeed )
{
	if( m_pInterferenceStripEffect )
	{
		m_pInterferenceStripEffect->SetParentEntity( NULL );
		m_pInterferenceStripEffect = NULL;
		auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
		pParams[1] = CVector4( 0, 0, 0, 0 );
	}
	if( nType )
	{
		auto pEft = SafeCast<CInterferenceStripEffect>( CGlobalCfg::Inst().pInterferenceStripEftPrefab->GetRoot()->CreateInstance() );
		m_pInterferenceStripEffect = pEft;
		pEft->SetParentBeforeEntity( GetCurLevel()->m_pPawnRoot );

		auto view = GetStage()->GetCamera().GetViewArea();
		CRectangle lvRect( 0, 0, m_pCurLevel->GetSize().x * LEVEL_GRID_SIZE_X, m_pCurLevel->GetSize().y * LEVEL_GRID_SIZE_Y );
		pEft->Init( view, lvRect, fSpeed );
		m_pLevelFadeMask->bVisible = true;
		m_pSnapShotMask->bVisible = false;
		auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
		pParams[0] = CVector4( 0, 0, 0, 0 );
		pParams[1] = CVector4( 0.07f, 0.07f, 0.07f, 0 );
	}
}

void CMasterLevel::SetGlobalBGM( const char* sz, int32 nPriority )
{
	auto& data = m_worldData.curFrame;
	data.strGlobalBGM = sz;
	data.nGlobalBGMPriority = nPriority;
	CheckBGM();
}

CVector2 CMasterLevel::GetCamPos()
{
	if( m_pTransferCoroutine )
		return m_transferCurCamPos;
	if( !m_pCurLevel )
		return CVector2( 0, 0 );
	return m_pCurLevel->GetCamPos();
}

void CMasterLevel::OnPlayerDamaged()
{
	m_nPlayerDamageFrame = CGlobalCfg::Inst().playerDamagedMask.size();
}

void CMasterLevel::Update()
{
	int32 k = 1;
	if( m_nBlackOutFrame1 )
		m_nBlackOutFrame1--;
	if( m_nBlackOutFrame1 )
		k = 0;
	else if( m_nBlackOutFrame2 )
	{
		k += m_nBlackOutFrame2;
		m_nBlackOutFrame2 = 0;
	}
	if( !k && m_pCurLevel && m_pCurLevel->IsBegin() && !m_pCurLevel->IsEnd() && !m_pCurLevel->IsFailed() && !m_pCurLevel->IsFreeze() )
		m_pPlayer->UpdateInputOnly();
	if( !k )
		m_pMainUI->UpdateEffect();
	for( ; k > 0; k-- )
	{
		int32 nShowSnapShotFrame = 0;
		auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
		auto pParams1 = static_cast<CImage2D*>( m_pSnapShotMask.GetPtr() )->GetParam();
		if( m_pTransferCoroutine )
		{
			m_pLevelFadeMask->bVisible = true;
			m_pSnapShotMask->bVisible = false;
			m_pTransferCoroutine->Resume();
			if( m_pTransferCoroutine->GetState() == ICoroutine::eState_Stopped )
			{
				TCoroutinePool<0x10000>::Inst().Free( m_pTransferCoroutine );
				m_pTransferCoroutine = NULL;
			}
		}
		else if( m_nPlayerDamageFrame )
		{
			auto nFrame = CGlobalCfg::Inst().playerDamagedMask.size() - m_nPlayerDamageFrame;
			m_nPlayerDamageFrame--;
			auto& maskParam = CGlobalCfg::Inst().playerDamagedMask[nFrame];
			m_pLevelFadeMask->bVisible = false;
			m_pSnapShotMask->bVisible = true;
			pParams1[0] = maskParam.first;
			pParams1[1] = maskParam.second;
		}
		else if( m_pInterferenceStripEffect )
		{
			m_pLevelFadeMask->bVisible = true;
			m_pSnapShotMask->bVisible = false;
			pParams[0] = CVector4( 0, 0, 0, 0 );
			pParams[1] = CVector4( 0.07f, 0.07f, 0.07f, 0 );
		}
		else if( !IsScenario() )
		{
			auto& cfg = CGlobalCfg::Inst().showSnapShotMask;
			nShowSnapShotFrame = m_nShowSnapShotFrame;
			auto& maskParam = cfg[nShowSnapShotFrame];
			m_pLevelFadeMask->bVisible = false;
			m_pSnapShotMask->bVisible = true;
			pParams1[0] = maskParam.first;
			pParams1[1] = maskParam.second;
			if( m_pCurLevel && !m_pCurLevel->IsEnd() && !m_pCurLevel->IsFreeze() )
			{
				nShowSnapShotFrame++;
				if( nShowSnapShotFrame >= cfg.size() )
					nShowSnapShotFrame = 0;
			}
		}
		else
		{
			m_pLevelFadeMask->bVisible = true;
			m_pSnapShotMask->bVisible = false;
			pParams[0] = CVector4( 0, 0, 0, 0 );
			pParams[1] = CVector4( 0, 0, 0, 0 );
		}
		m_nShowSnapShotFrame = nShowSnapShotFrame;

		m_pBattleEffect->bVisible = false;
		if( m_pCurLevel )
		{
			m_pBattleEffect->bVisible = m_pCurLevel->IsBegin() && !m_pCurLevel->IsComplete() && !m_pCurLevel->IsEnd();
			if( !m_pCurLevel->IsFreeze() && !m_pCurLevel->IsFailed() )
			{
				if( m_pBattleEffect->bVisible )
					UpdateBattleEffect();
			}
		}

		if( m_pCurLevel && !m_pCurLevel->IsEnd() )
			m_pCurLevel->Update();
		m_pMainUI->Update();

		if( m_pScenarioScript && !m_pMenu->bVisible )
		{
			if( !m_pScenarioScript->Resume( 0, 0 ) )
			{
				m_pScenarioScript = NULL;
				EndScenario();
			}
		}
		if( m_pInteractionUI && m_bInteractionUIInit )
		{
			auto p = SafeCast<CInteractionUI>( m_pInteractionUI.GetPtr() );
			p->SetZOrder( 1 );
			p->SetParentEntity( this );
			p->SetPosition( GetCamPos() );
			p->Init( m_pInteractionUIPawn );
			m_pCurLevel->Freeze();
			m_bInteractionUIInit = false;
		}

		if( m_pScriptTransferTo )
		{
			TransferTo( m_pScriptTransferTo, TVector2<int32>( m_nScriptTransferPlayerX, m_nScriptTransferPlayerY ), m_nScriptTransferPlayerDir, m_nScriptTransferType, m_nScriptTransferParam );
			m_pScriptTransferTo = NULL;
			m_nScriptTransferType = 0;
		}
		else if( m_nScriptTransferType == -1 )
		{
			m_nScriptTransferType = 0;
			JumpBack( m_nScriptTransferParam == 1 ? 0 : 2 );
		}
		else if( !m_pTransferCoroutine && !m_pScenarioScript && m_pCurCutScene )
		{
			TransferTo( m_pCurCutScene->m_pNextLevelPrefab, TVector2<int32>( m_pCurCutScene->m_nPlayerEnterX, m_pCurCutScene->m_nPlayerEnterY ),
				m_pCurCutScene->m_nPlayerEnterDir );
		}
		else
		{
			if( !m_pTransferCoroutine && m_pCurLevel && m_pCurLevel->IsBegin() && !m_pCurLevel->m_nFreeze )
			{
				if( CGame::Inst().IsKeyDown( 'M' ) )
					ShowWorldMap( true );
				else if( CGame::Inst().IsKeyDown( 'N' ) )
					ShowActionPreview( true );
				else if( CGame::Inst().IsKeyDown( 'B' ) )
					ShowLogUI( true );
			}
			else if( m_pWorldMap->bVisible )
			{
				auto pWorldMap = SafeCast<CWorldMapUI>( m_pWorldMap.GetPtr() );
				pWorldMap->Update();
				if( CGame::Inst().IsKeyDown( 'M' ) )
					ShowWorldMap( false );
			}
			else if( m_pActionPreview->bVisible )
			{
				auto pActionPreview = SafeCast<CActionPreview>( m_pActionPreview.GetPtr() );
				pActionPreview->Update();
				if( CGame::Inst().IsKeyDown( 'N' ) )
					ShowActionPreview( false );
			}
			else if( m_pLogUI->bVisible )
			{
				auto pLogUI = SafeCast<CLogUI>( m_pLogUI.GetPtr() );
				pLogUI->Update();
				if( CGame::Inst().IsKeyDown( 'B' ) )
					ShowLogUI( false );
			}
			if( m_pMenu->bVisible )
			{
				if( CGame::Inst().IsKeyDown( VK_ESCAPE ) )
					SwitchMenuPage( -1 );
				else if( m_nMenuPageItemIndex >= 0 )
				{
					auto nPageIndex = m_nMenuPageItemIndex;
					if( CGame::Inst().IsKeyDown( 'Q' ) )
					{
						nPageIndex--;
						if( nPageIndex < 0 )
							nPageIndex = m_nEnabledPageCount - 1;
					}
					if( CGame::Inst().IsKeyDown( 'E' ) )
					{
						nPageIndex++;
						if( nPageIndex >= m_nEnabledPageCount )
							nPageIndex = 0;
					}
					if( nPageIndex != m_nMenuPageItemIndex )
						SwitchMenuPage( m_enabledPages[nPageIndex] );
				}
			}
			else if( m_pInteractionUI )
			{
				auto p = m_pInteractionUI;
				bool b = true;
				if( m_pInteractionUIPawn && !m_pInteractionUIPawn->GetStage() )
					b = false;
				else
					b = SafeCast<CInteractionUI>( p.GetPtr() )->Update( m_pInteractionUIPawn );
				if( !b && p == m_pInteractionUI.GetPtr() )
				{
					m_pInteractionUI->SetParentEntity( NULL );
					m_pInteractionUI = NULL;
					m_pInteractionUIPawn = NULL;
					m_strInteractionUI = "";
					if( m_pCurLevel )
						m_pCurLevel->UnFreeze();
				}
			}
			else if( CGame::Inst().IsKeyDown( VK_ESCAPE ) )
				ShowLogUI( true );
		}
		CGame::Inst().ClearInputEvent();
		m_nTransferPlayerDataOpr = 0;
	}

	if( m_pCurLevel )
	{
		if( m_pCurLevel->GetEnvEffect() )
		{
			if( IsScenario() || m_pPlayer && m_pPlayer->IsHidden() )
				m_pCurLevel->GetEnvEffect()->ScenarioFade( true );
			else
				m_pCurLevel->GetEnvEffect()->ScenarioFade( false );
		}
		if(!m_pCurLevel->IsFreeze() )
		{
			if( m_pInterferenceStripEffect )
				SafeCast<CInterferenceStripEffect>( m_pInterferenceStripEffect.GetPtr() )->Update();
		}
	}
	UpdateBGM();
}

void CMasterLevel::CheckBGM()
{
	string strNewBGM;
	if( m_pCurCutScene )
		strNewBGM = "";
	else if( m_pLastLevel )
	{
		strNewBGM = GetCurBGM();
		if( strNewBGM != m_strBGM )
			strNewBGM = "";
	}
	else
		strNewBGM = GetCurBGM();
	if( strNewBGM == m_strBGM )
		return;

	if( strNewBGM.length() && m_strFadeOutBGM == strNewBGM )
	{
		swap( m_fBGMFadeIn, m_fBGMFadeOut );
		swap( m_pBGMSoundTrack, m_pBGMSoundTrackFadeOut );
	}
	else
	{
		if( m_strBGM.length() )
		{
			if( m_pBGMSoundTrackFadeOut )
			{
				m_pBGMSoundTrackFadeOut->FadeOut( 0.1f );
				m_pBGMSoundTrackFadeOut = NULL;
			}
			m_pBGMSoundTrackFadeOut = m_pBGMSoundTrack;
			m_fBGMFadeOut = m_fBGMFadeIn;
		}
		if( strNewBGM.length() )
		{
			CreateBGM( m_pBGMSoundTrack, strNewBGM.c_str() );
			m_fBGMFadeIn = 0;
			m_fBGMFadeInSpeed = 0.5f;
		}
		else
			m_pBGMSoundTrack = NULL;
	}
	m_strBGM = strNewBGM;
}

void CMasterLevel::UpdateBGM()
{
	float fFadeOutSpeed = 0.5f;
	float fFadeInSpeed = m_fBGMFadeInSpeed;
	bool bMuteCurBGM = false;
	float fTargetVolume = 1.0f;

	if( m_nBlackOutFrame1 )
	{
		if( !m_pSpecialEftSoundTrack )
		{
			m_pSpecialEftSoundTrack = CGlobalCfg::Inst().pBlackOutSound->CreateSoundTrack();
			m_pSpecialEftSoundTrack->Play( ESoundPlay_KeepRef | ESoundPlay_Loop );
		}
	}
	else
	{
		if( m_pSpecialEftSoundTrack )
		{
			m_pSpecialEftSoundTrack->FadeOut( 0.05f );
			m_pSpecialEftSoundTrack = NULL;
		}
	}

	if( m_nBlackOutFrame1 || m_pCurLevel && ( m_pCurLevel->IsFailed() || m_pCurLevel->IsBegin() && m_pCurLevel->IsFreeze() ) )
	{
		fFadeOutSpeed = 20.0f;
		m_fBGMFadeInSpeed = fFadeInSpeed = 20.0f;
		bMuteCurBGM = true;
	}
	else if( IsScenario() )
	{
		fFadeOutSpeed = 1.0f;
		m_fBGMFadeInSpeed = fFadeInSpeed = 0.25f;
		bMuteCurBGM = true;
	}
	if( m_pCurLevel && m_pCurLevel->m_pNoise )
	{
		fFadeOutSpeed = Max( fFadeOutSpeed, 20.0f );
		fFadeInSpeed = Max( fFadeInSpeed, 20.0f );
		fTargetVolume = 0.03f;
	}

	if( m_pBGMSoundTrackFadeOut )
	{
		m_fBGMFadeOut -= fFadeOutSpeed * 1.0f / 60.0f;
		if( m_fBGMFadeOut <= 0 )
		{
			m_fBGMFadeOut = 0;
			m_pBGMSoundTrackFadeOut->Stop();
			m_pBGMSoundTrackFadeOut = NULL;
		}
		else
			m_pBGMSoundTrackFadeOut->SetVolumeDB( ( m_fBGMFadeOut - 1 ) * 40 );
	}
	if( m_pBGMSoundTrack )
	{
		if( bMuteCurBGM )
		{
			m_fBGMFadeIn = Max( 0.0f, m_fBGMFadeIn - fFadeOutSpeed * 1.0f / 60.0f );
			m_pBGMSoundTrack->SetVolumeDB( ( m_fBGMFadeIn - 1 ) * 40 );
			if( m_fBGMFadeIn <= 0 )
				m_pBGMSoundTrack->Stop();
		}
		else
		{
			if( m_fBGMFadeIn == 0 )
				m_pBGMSoundTrack->Resume();
			if( m_fBGMFadeIn < fTargetVolume )
				m_fBGMFadeIn = Min( fTargetVolume, m_fBGMFadeIn + fFadeInSpeed * 1.0f / 60.0f );
			else
				m_fBGMFadeIn = Max( fTargetVolume, m_fBGMFadeIn - fFadeOutSpeed * 1.0f / 60.0f );
			m_pBGMSoundTrack->SetVolumeDB( ( m_fBGMFadeIn - 1 ) * 40 );
			if( m_fBGMFadeIn >= fTargetVolume )
				m_fBGMFadeInSpeed = 0.5f;
		}
	}
}

const char* CMasterLevel::GetCurBGM()
{
	auto& data = m_worldData.curFrame;
	const char* szBGM = data.strGlobalBGM.c_str();
	int32 n = data.strGlobalBGM.length() ? data.nGlobalBGMPriority : 0x80000000;
	if( m_pCurLevel && m_pCurLevel->m_strBGM.length() )
	{
		if( m_pCurLevel->m_nBGMPriority >= n )
		{
			n = m_pCurLevel->m_nBGMPriority;
			szBGM = m_pCurLevel->m_strBGM.c_str();
		}
	}
	return szBGM;
}

void CMasterLevel::UpdateBattleEffect()
{
	auto pParams = static_cast<CImage2D*>( m_pBattleEffect.GetPtr() )->GetParam();
	auto& cfg = CGlobalCfg::Inst().battleEffectMask;
	auto& maskParam = cfg[m_nBattleEffectFrame];
	pParams[0] = maskParam.first;
	pParams[1] = maskParam.second;
	if( m_pCurLevel && !m_pCurLevel->IsEnd() && !m_pCurLevel->IsFreeze() )
	{
		m_nBattleEffectFrame++;
		if( m_nBattleEffectFrame >= cfg.size() )
			m_nBattleEffectFrame = 0;
	}
	CRectangle lvRect( m_pCurLevel->x, m_pCurLevel->y, m_pCurLevel->GetSize().x * LEVEL_GRID_SIZE_X, m_pCurLevel->GetSize().y * LEVEL_GRID_SIZE_Y );
	lvRect.SetSize( lvRect.GetSize() + CVector2( 64, 64 ) );
	static_cast<CImage2D*>( m_pBattleEffect.GetPtr() )->SetRect( lvRect );
}

void CMasterLevel::RefreshSnapShot()
{
	auto& itr = m_mapSnapShot.find( GetCurLevelName() );
	if( itr != m_mapSnapShot.end() )
	{
		if( itr->second->GetStage() )
			itr->second->SetParentEntity( NULL );
		m_mapSnapShot.erase( itr );
	}
	auto pCurLevel = m_pCurLevel;
	auto worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	auto pLevelData = worldCfg.GetLevelData( GetCurLevelName() );
	if( !pLevelData )
		return;
	for( int i = 0; i < pLevelData->arrShowSnapShot.Size(); i++ )
	{
		auto sz = pLevelData->arrShowSnapShot[i].c_str();
		if( !UpdateSnapShot( sz ) )
			continue;
		if( m_mapSnapShot.find( sz ) != m_mapSnapShot.end() )
			m_setShowingSnapShot.insert( sz );
	}

	m_pSnapShotRoot->SortChildrenRenderOrder( [] ( CRenderObject2D* a, CRenderObject2D* b ) {
		auto p1 = static_cast<CMyLevel*>( a );
		auto p2 = static_cast<CMyLevel*>( b );
		return p1->m_nDepth > p2->m_nDepth;
	} );
}

bool CMasterLevel::UpdateSnapShot( const char* sz )
{
	auto& pSnapShot = m_mapSnapShot[sz];
	if( !pSnapShot )
	{
		auto itr = m_worldData.mapSnapShotCur.find( sz );
		if( itr != m_worldData.mapSnapShotCur.end() )
		{
			CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( sz );
			if( pPrefab->GetRoot()->GetStaticDataSafe<CMyLevel>()->m_nDepth > m_pCurLevel->m_nDepth )
			{
				bool bFound = false;
				for( int i = 0; i < m_pCurLevel->m_arrNextStage.Size(); i++ )
				{
					if( m_pCurLevel->m_arrNextStage[i].pNxtStage == pPrefab.GetPtr() )
					{
						bFound = true;
						break;
					}
				}
				if( !bFound )
				{
					m_mapSnapShot.erase( sz );
					return false;
				}
			}
			pSnapShot = SafeCast<CMyLevel>( pPrefab->GetRoot()->CreateInstance() );
			auto pCurLevel = m_pCurLevel;
			m_pCurLevel = pSnapShot;
			pSnapShot->SetParentEntity( m_pSnapShotRoot );
			m_strUpdatingSnapShot = sz;
			pSnapShot->Init( 1 );
			m_pCurLevel = pCurLevel;
			m_strUpdatingSnapShot = "";
		}
		else
		{
			m_mapSnapShot.erase( sz );
			return false;
		}
	}
	if( pSnapShot->m_nDepth > m_pCurLevel->m_nDepth )
	{
		bool bFound = false;
		for( int i = 0; i < m_pCurLevel->m_arrNextStage.Size(); i++ )
		{
			if( m_pCurLevel->m_arrNextStage[i].pNxtStage == pSnapShot->GetInstanceOwner() )
			{
				bFound = true;
				break;
			}
		}
		if( !bFound )
			return false;
	}
	pSnapShot->SetParentEntity( m_pSnapShotRoot );
	auto worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	pSnapShot->SetPosition( worldCfg.GetLevelDisplayOfs( sz ) - worldCfg.GetLevelDisplayOfs( GetCurLevelName() ) );
	pSnapShot->y -= ( m_pCurLevel->m_nDepth - pSnapShot->m_nDepth ) * 8;
	return true;
}

void CMasterLevel::HideAllSnapShot()
{
	for( auto& item : m_setShowingSnapShot )
	{
		m_mapSnapShot[item]->SetParentEntity( NULL );
	}
	m_setShowingSnapShot.clear();
}

void CMasterLevel::RemoveAllSnapShot()
{
	for( auto& item : m_mapSnapShot )
	{
		item.second->SetParentEntity( NULL );
	}
	m_mapSnapShot.clear();
	m_setShowingSnapShot.clear();
}

void CMasterLevel::ResetMainUI()
{
	if( !m_pCurLevel )
		return;
	CRectangle lvRect( 0, 0, m_pCurLevel->GetSize().x * LEVEL_GRID_SIZE_X, m_pCurLevel->GetSize().y * LEVEL_GRID_SIZE_Y );
	lvRect = lvRect.Offset( m_pCurLevel->GetCamPos() * -1 );
	m_pMainUI->Reset( CVector2( lvRect.GetRight() + 64, 144 ), CVector2( lvRect.x - 64, 144 ) );
}

void CMasterLevel::RefreshMainUI()
{
	CLuaMgr::Inst().Run( "OnRefreshMainUI()" );
}

void CMasterLevel::BeginCurLevel()
{
	ResetMainUI();
	if( m_pScriptTransferOpr )
	{
		m_pScriptTransferOpr->Resume( 0, 0 );
		m_pScriptTransferOpr = NULL;
	}
	m_pCurLevel->Begin();
	RefreshSnapShot();
	CheckBGM();
}

void CMasterLevel::EndCurLevel()
{
	HideAllSnapShot();
	if( m_pInteractionUI )
	{
		m_strInteractionUI = "";
		m_pInteractionUIPawn = NULL;
		m_pInteractionUI->SetParentEntity( NULL );
		m_pInteractionUI = NULL;
	}
	m_pScenarioScript = NULL;
	InterferenceStripEffect( 0, 0 );
	EndScenario();
	m_pCurLevel->End();
}

void CMasterLevel::TransferFunc()
{
	auto pTransferTo = m_pTransferTo;
	m_pTransferTo = NULL;
	auto p = pTransferTo->GetRoot()->CreateInstance();
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
		m_pCurLevelPrefab = pTransferTo;
		m_pLastLevel = m_pCurLevel;
		m_pCurLevel = pLevel;
		if( m_bClearSnapShot )
		{
			m_bClearSnapShot = false;
			RemoveAllSnapShot();
		}
		CheckBGM();

		if( !m_pCurCutScene )
		{
			if( m_nTransferType == 1 )
				TransferFuncLevel2Level1();
			else if( m_nTransferType == 2 )
				TransferFuncLevel2Level2();
			else if( m_nTransferType == 3 )
				TransferFuncLevel2Level3();
			else if( m_nTransferType == 4 )
				TransferFuncLevel2Level4_5( false );
			else if( m_nTransferType == 5 )
				TransferFuncLevel2Level4_5( true );
			else if( m_nTransferType == -1 )
				TransferFuncLevel2Level0( true );
			else if( m_nTransferType == -2 )
				TransferFuncLevel2Level0( false );
			else
				TransferFuncLevel2Level();
		}
		else
		{
			m_pCurCutScene = NULL;
			TransferFuncCut2Level();
		}
	}
	else
	{
		auto pCutScene = SafeCast<CCutScene>( p );
		ASSERT( pCutScene );
		if( !m_pCurCutScene )
		{
			EndCurLevel();
			m_pCurLevel->SetRenderParentAfter( m_pLevelFadeMask );
			m_pCurCutScene = pCutScene;
			CheckBGM();
			TransferFuncLevel2Cut();
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

void CMasterLevel::TransferFuncEnterLevel( vector<CReference<CPawn> >* pTransferPawn )
{
	m_worldData.OnEnterLevel( m_pCurLevelPrefab->GetName(), m_pPlayer, m_transferPos, m_nTransferDir, pTransferPawn, m_bClearSnapShot, m_nTransferPlayerDataOpr );
	m_nTransferPlayerDataOpr = 0;
	Save();
}

void CMasterLevel::TransferFuncLevel2Level()
{
	TransferFuncEnterLevel();
	m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
	auto d = m_pPlayer->GetPos() - m_transferPos;
	auto transferOfs = CVector2( d.x, d.y ) * LEVEL_GRID_SIZE;
	m_pCurLevel->SetPosition( transferOfs );
	auto camTransferBegin = m_pLastLevel->GetCamPos();
	auto dVisualTransfer = transferOfs + m_pCurLevel->GetCamPos() - camTransferBegin;
	int32 nTransferAnimFrames = ceil( ( dVisualTransfer.Length() * 0.5f + 128 ) / CGlobalCfg::Inst().lvTransferData.fTransferCamSpeed );
	int32 nTransferAnimTotalFrames = nTransferAnimFrames;
	int32 nTransferFadeOutFrames = CGlobalCfg::Inst().lvTransferData.nTransferFadeOutFrameCount;
	int32 nTransferFadeOutTotalFrames = nTransferFadeOutFrames;
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	pParams[0] = pParams[1] = CVector4( 0, 0, 0, 0 );
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetFade( 0 );

	auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
	for( ; nTransferAnimFrames > 0; nTransferAnimFrames-- )
	{
		if( nTransferAnimFrames )
		{
			float t = 1 - nTransferAnimFrames * 1.0f / nTransferAnimTotalFrames;
			auto p = camTransferBegin + ( m_pCurLevel->GetCamPos() - camTransferBegin ) * t;
			m_transferCurCamPos = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
		}

		m_pTransferCoroutine->Yield( 0 );
		int32 nFrame = nTransferAnimTotalFrames - nTransferAnimFrames;
		float t = 1 - ( nTransferAnimFrames - 1 ) * 1.0f / nTransferAnimTotalFrames;
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
		auto p = transferOfs * ( 1 - t );
		p = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
		m_pCurLevel->SetPosition( p );
		m_pLastLevel->SetPosition( p - transferOfs );
		if( m_pCurLevel->GetEnvEffect() )
			m_pCurLevel->GetEnvEffect()->SetFade( t );
	}
	m_transferCurCamPos = m_pCurLevel->GetCamPos();
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );
	m_pLastLevel->RemovePawn( m_pPlayer );
	m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
	m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );

	for( ; nTransferFadeOutFrames; nTransferFadeOutFrames-- )
	{
		m_pTransferCoroutine->Yield( 0 );
		int32 nFrame = maskParams.size() - nTransferFadeOutFrames;
		float t = ( nTransferFadeOutFrames - 1 ) * 1.0f / nTransferFadeOutTotalFrames;
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
		CMyLevel* pLevel = m_pLastLevel;
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetFade( t );
	}
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pLastLevel->SetParentEntity( NULL );
	m_pLastLevel = NULL;
	BeginCurLevel();
}

void CMasterLevel::TransferFuncLevel2Level0( bool bFade )
{
	TransferFuncEnterLevel();
	m_pLastLevel->RemovePawn( m_pPlayer );
	m_pLastLevel->SetParentEntity( NULL );
	m_pLastLevel = NULL;

	m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	if( bFade )
	{
		pParams[0] = pParams[1] = CVector4( 0, 0, 0, 0 );
		int32 nTransferAnimFrames = CGlobalCfg::Inst().lvTransferData.nTransferFadeOutFrameCount;
		int32 nTransferAnimTotalFrames = nTransferAnimFrames;
		if( m_pCurLevel->GetEnvEffect() )
			m_pCurLevel->GetEnvEffect()->SetFade( 0 );

		m_transferCurCamPos = m_pCurLevel->GetCamPos();
		auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
		for( ; nTransferAnimFrames > 0; nTransferAnimFrames-- )
		{
			m_pTransferCoroutine->Yield( 0 );
			int32 nFrame = nTransferAnimTotalFrames - nTransferAnimFrames;
			float t = 1 - ( nTransferAnimFrames - 1 ) * 1.0f / nTransferAnimTotalFrames;
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
			if( m_pCurLevel->GetEnvEffect() )
				m_pCurLevel->GetEnvEffect()->SetFade( t );
		}
	}
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
	m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
	BeginCurLevel();
}

void CMasterLevel::TransferFuncLevel2Level1()
{
	TransferFuncEnterLevel();
	auto pCurLevel = m_pCurLevel;
	m_pCurLevel = NULL;

	auto d = m_pPlayer->GetPos() - m_transferPos;
	auto transferOfs = CVector2( d.x, d.y ) * LEVEL_GRID_SIZE - CVector2( 0, 64 );
	pCurLevel->SetPosition( transferOfs );
	auto camTransferBegin = m_pLastLevel->GetCamPos();
	int32 nTransferAnimFrames = 80;
	int32 nTransferAnimTotalFrames = nTransferAnimFrames;
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );

	m_transferCurCamPos = camTransferBegin;
	m_pTransferEft = new CEntity;
	m_pTransferEft->SetParentBeforeEntity( m_pLevelFadeMask );
	m_pTransferEft->SetRenderObject( CGlobalCfg::Inst().pFallEftDrawable->CreateInstance() );
	auto pImg = static_cast<CImage2D*>( m_pTransferEft->GetRenderObject() );
	pImg->SetPosition( m_pLastLevel->GetPosition() + CVector2( m_pPlayer->GetPosX(), m_pPlayer->GetPosY() ) * LEVEL_GRID_SIZE );
	m_pLastLevel->RemovePawn( m_pPlayer );

	auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
	CRectangle rects[] = { { 0, 0, 2, 3 }, { 0, -1, 2, 3 }, { 0, -1, 2, 3 }, { -1, -2, 3, 3 }, { -1, -2, 3, 2 }, { -1, -2, 3, 1 }, { 0, -2, 2, 3 } };
	CRectangle texRects[] = { { 18, 15, 2, 3 }, { 18, 18, 2, 3 }, { 18, 21, 2, 3 }, { 20, 21, 3, 3 }, { 20, 24, 3, 2 }, { 20, 26, 3, 1 }, { 21, 27, 2, 3 } };
	int32 nFrames[] = { 12, 12, 12, 40, 12, 60, 60 };
	int32 nFadeOutTotalFrames = 20;
	int32 nFadeOutFrame = nFadeOutTotalFrames;
	for( int i = 0; i < ELEM_COUNT( rects ); i++ )
	{
		auto rect = rects[i] * 32;
		auto texRect = texRects[i] / 32;
		if( m_worldData.curFrame.nPlayerEnterDir )
		{
			rect = CRectangle( LEVEL_GRID_SIZE_X * m_pPlayer->GetWidth() - rect.GetRight(), rect.y, rect.width, rect.height );
			texRect = CRectangle( 2 - texRect.GetRight(), texRect.y, texRect.width, texRect.height );
		}
		pImg->SetRect( rect );
		pImg->SetTexRect( texRect );
		if( i == 5 )
		{
			m_pLastLevel->SetParentEntity( NULL );
			m_pLastLevel = NULL;
			m_pCurLevel = pCurLevel;
			m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
			m_pCurLevel->Init();
			if( m_pCurLevel->GetEnvEffect() )
				m_pCurLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );

			if( m_pCurLevel->GetEnvEffect() )
				m_pCurLevel->GetEnvEffect()->SetFade( 0 );
		}

		for( int k = 0; k < nFrames[i]; k++ )
		{
			m_pTransferCoroutine->Yield( 0 );
			if( i >= 2 && i < 5 )
			{
				if( nFadeOutFrame )
				{
					int32 nFrame = maskParams.size() - nFadeOutFrame;
					nFadeOutFrame--;
					float t = nFadeOutFrame * 1.0f / nFadeOutTotalFrames;
					if( nFrame >= 0 && nFrame < maskParams.size() )
					{
						pParams[0] = maskParams[nFrame].first;
						pParams[1] = maskParams[nFrame].second;
					}
					else
					{
						float a = t * 1.5f;
						float b = Max( 0.0f, t * 1.5f - 0.5f );
						pParams[0] = CVector4( a, b, b, 0 );
						pParams[1] = CVector4( 0, 0, 0, 0 );
					}
					CMyLevel* pLevel = m_pLastLevel;
					if( pLevel->GetEnvEffect() )
						pLevel->GetEnvEffect()->SetFade( t );
					pLevel->SetPosition( pLevel->GetPosition() + CVector2( 0, 12 ) );
				}
				else
				{
					pParams[0] = CVector4( 0, 0, 0, 0 );
					pParams[1] = CVector4( 0, 0, 0, 0 );
					camTransferBegin.y += 6;
					m_transferCurCamPos = camTransferBegin;
				}
			}
			if( i >= 5 && nTransferAnimFrames )
			{
				int32 nFrame = nTransferAnimTotalFrames - nTransferAnimFrames;
				nTransferAnimFrames--;
				float t = 1 - nTransferAnimFrames * 1.0f / nTransferAnimTotalFrames;
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
				auto p = transferOfs * ( 1 - t );
				p = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
				m_pCurLevel->SetPosition( p );
				m_pTransferEft->SetPosition( p - transferOfs );
				if( m_pCurLevel->GetEnvEffect() )
					m_pCurLevel->GetEnvEffect()->SetFade( t );
				auto cam = camTransferBegin + ( m_pCurLevel->GetCamPos() - camTransferBegin ) * t;
				m_transferCurCamPos = CVector2( floor( cam.x * 0.5f + 0.5f ), floor( cam.y * 0.5f + 0.5f ) ) * 2;
			}
		}
	}

	m_pTransferEft->SetParentEntity( NULL );
	m_pTransferEft = NULL;
	m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
	m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	BeginCurLevel();
}

void CMasterLevel::TransferFuncLevel2Level2()
{
	TransferFuncEnterLevel();
	auto pCurLevel = m_pCurLevel;
	m_pCurLevel = NULL;

	auto d = m_pPlayer->GetPos() - m_transferPos;
	auto transferOfs = CVector2( d.x, d.y ) * LEVEL_GRID_SIZE + CVector2( m_nTransferDir ? -48 : 48, 64 );
	pCurLevel->SetPosition( transferOfs );
	auto camTransferBegin = m_pLastLevel->GetCamPos();
	int32 nTransferAnimFrames = 80;
	int32 nTransferAnimTotalFrames = nTransferAnimFrames;
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );

	m_transferCurCamPos = camTransferBegin;
	m_pTransferEft = new CEntity;
	m_pTransferEft->SetParentBeforeEntity( m_pLevelFadeMask );
	m_pLastLevel->RemovePawn( m_pPlayer );
	m_pPlayer->SetParentEntity( m_pTransferEft );
	m_pPlayer->SetPosition( m_pLastLevel->GetPosition() + m_pPlayer->GetPosition() );

	auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
	int32 nFadeOutTotalFrames = 60;
	int32 nFadeOutFrame = nFadeOutTotalFrames;
	while( nFadeOutFrame )
	{
		m_pTransferCoroutine->Yield( 0 );
		m_pPlayer->UpdateAnimOnly();
		int32 nFrame = maskParams.size() - nFadeOutFrame;
		nFadeOutFrame--;
		float t = nFadeOutFrame * 1.0f / nFadeOutTotalFrames;
		if( nFrame >= 0 && nFrame < maskParams.size() )
		{
			pParams[0] = maskParams[nFrame].first;
			pParams[1] = maskParams[nFrame].second;
		}
		else
		{
			float a = t * 1.5f;
			float b = Max( 0.0f, t * 1.5f - 0.5f );
			pParams[0] = CVector4( a, b, b, 0 );
			pParams[1] = CVector4( 0, 0, 0, 0 );
		}
		CMyLevel* pLevel = m_pLastLevel;
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetFade( t );
		if( !( nFadeOutFrame & 1 ) )
			pLevel->SetPosition( pLevel->GetPosition() + CVector2( 0, -2 ) );
	}
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	for( int i = 0; i < 30; i++ )
	{
		m_pTransferCoroutine->Yield( 0 );
		m_pPlayer->UpdateAnimOnly();
		camTransferBegin.y += 2;
		m_transferCurCamPos = camTransferBegin;
	}

	m_pLastLevel->SetParentEntity( NULL );
	m_pLastLevel = NULL;
	m_pCurLevel = pCurLevel;
	m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );

	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetFade( 0 );
	while( nTransferAnimFrames )
	{
		m_pTransferCoroutine->Yield( 0 );
		m_pPlayer->UpdateAnimOnly();
		int32 nFrame = nTransferAnimTotalFrames - nTransferAnimFrames;
		nTransferAnimFrames--;
		float t = 1 - nTransferAnimFrames * 1.0f / nTransferAnimTotalFrames;
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
		auto p = transferOfs * ( 1 - t );
		p = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
		m_pCurLevel->SetPosition( p );
		m_pTransferEft->SetPosition( p - transferOfs );
		if( m_pCurLevel->GetEnvEffect() )
			m_pCurLevel->GetEnvEffect()->SetFade( t );
		auto cam = camTransferBegin + ( m_pCurLevel->GetCamPos() - camTransferBegin ) * t;
		m_transferCurCamPos = CVector2( floor( cam.x * 0.5f + 0.5f ), floor( cam.y * 0.5f + 0.5f ) ) * 2;
	}

	m_pPlayer->SetParentEntity( NULL );
	m_pTransferEft->SetParentEntity( NULL );
	m_pTransferEft = NULL;
	m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
	m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	BeginCurLevel();
}

void CMasterLevel::TransferFuncLevel2Level3()
{
	TransferFuncEnterLevel();
	auto pCurLevel = m_pCurLevel;
	m_pCurLevel = NULL;

	auto d = m_pPlayer->GetPos() - m_transferPos;
	int16* params = (int16*)&m_nTransferParam;
	CVector2 transferOfs( params[0], params[1] );
	transferOfs = ( CVector2( d.x, d.y ) + transferOfs ) * LEVEL_GRID_SIZE;
	pCurLevel->SetPosition( transferOfs );
	auto camTransferBegin = m_pLastLevel->GetCamPos();
	int32 nTransferAnimFrames = 80;
	int32 nTransferAnimTotalFrames = nTransferAnimFrames;
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );

	m_transferCurCamPos = camTransferBegin;
	m_pTransferEft = new CEntity;
	m_pTransferEft->SetParentBeforeEntity( m_pLevelFadeMask );
	m_pLastLevel->RemovePawn( m_pPlayer );
	m_pPlayer->SetParentEntity( m_pTransferEft );
	m_pPlayer->SetPosition( m_pLastLevel->GetPosition() + m_pPlayer->GetPosition() );

	auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
	int32 nFadeOutTotalFrames = 80;
	int32 nFadeOutFrame = nFadeOutTotalFrames;
	while( nFadeOutFrame )
	{
		m_pTransferCoroutine->Yield( 0 );
		m_pPlayer->UpdateAnimOnly();
		int32 nFrame = maskParams.size() - nFadeOutFrame;
		nFadeOutFrame--;
		float t = nFadeOutFrame * 1.0f / nFadeOutTotalFrames;
		if( nFrame >= 0 && nFrame < maskParams.size() )
		{
			pParams[0] = maskParams[nFrame].first;
			pParams[1] = maskParams[nFrame].second;
		}
		else
		{
			float a = t * 1.5f;
			float b = Max( 0.0f, t * 1.5f - 0.5f );
			pParams[0] = CVector4( a, b, b, 0 );
			pParams[1] = CVector4( 0, 0, 0, 0 );
		}
		CMyLevel* pLevel = m_pLastLevel;
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetFade( t );
	}
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	for( int i = 0; i < 60; i++ )
	{
		m_pTransferCoroutine->Yield( 0 );
		m_pPlayer->UpdateAnimOnly();
	}

	m_pLastLevel->SetParentEntity( NULL );
	m_pLastLevel = NULL;
	m_pCurLevel = pCurLevel;
	m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );

	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetFade( 0 );
	while( nTransferAnimFrames )
	{
		m_pTransferCoroutine->Yield( 0 );
		m_pPlayer->UpdateAnimOnly();
		int32 nFrame = nTransferAnimTotalFrames - nTransferAnimFrames;
		nTransferAnimFrames--;
		float t = 1 - nTransferAnimFrames * 1.0f / nTransferAnimTotalFrames;
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
		auto p = transferOfs * ( 1 - t );
		p = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
		m_pCurLevel->SetPosition( p );
		m_pTransferEft->SetPosition( p - transferOfs );
		if( m_pCurLevel->GetEnvEffect() )
			m_pCurLevel->GetEnvEffect()->SetFade( t );
		auto cam = camTransferBegin + ( m_pCurLevel->GetCamPos() - camTransferBegin ) * t;
		m_transferCurCamPos = CVector2( floor( cam.x * 0.5f + 0.5f ), floor( cam.y * 0.5f + 0.5f ) ) * 2;
	}

	m_pPlayer->SetParentEntity( NULL );
	m_pTransferEft->SetParentEntity( NULL );
	m_pTransferEft = NULL;
	m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
	m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	BeginCurLevel();
}

void CMasterLevel::TransferFuncLevel2Level4_5( int8 bUp )
{
	auto pCurLevel = m_pCurLevel;
	m_pCurLevel = NULL;

	auto d = m_pPlayer->GetPos() - m_transferPos;
	auto transferOfs = CVector2( d.x, d.y ) * LEVEL_GRID_SIZE;
	pCurLevel->SetPosition( transferOfs );
	auto camTransferBegin = m_pLastLevel->GetCamPos();
	int32 nTransferAnimFrames = 50;
	int32 nTransferAnimTotalFrames = nTransferAnimFrames;
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );

	m_transferCurCamPos = camTransferBegin;
	m_pTransferEft = new CEntity;
	m_pTransferEft->SetParentBeforeEntity( m_pLevelFadeMask );
	vector<CReference<CPawn> > vecTransferPawn;
	for( int y = m_pLastLevel->m_nHeight - 1; y >= 0; y-- )
	{
		for( int x = 0; x < m_pLastLevel->m_nWidth; x++ )
		{
			auto& grid = m_pLastLevel->m_arrGridData[x + y * m_pLastLevel->m_nWidth];
			if( grid.nTile == m_nTransferParam )
			{
				auto pGrid = m_pLastLevel->GetGrid( TVector2<int32>( x, y ) );
				auto pTile = pGrid->pTile;
				if( pTile )
				{
					pTile->RemoveThis();
					m_pTransferEft->AddChild( pTile );
				}
			}
		}
	}

	vecTransferPawn.push_back( m_pPlayer.GetPtr() );
	LINK_LIST_FOR_EACH_BEGIN( pPawn, m_pLastLevel->Get_Pawn(), CPawn, Pawn )
	{
		if( m_pLastLevel->IsPawnInTile( pPawn, m_nTransferParam ) )
		{
			if( pPawn != m_pPlayer )
				vecTransferPawn.push_back( pPawn );
		}
	}
	LINK_LIST_FOR_EACH_END( pPawn, m_pLastLevel->Get_Pawn(), CPawn, Pawn )
	for( CPawn* pPawn : m_pLastLevel->m_vecPawnHits )
	{
		if( m_pLastLevel->IsPawnInTile( pPawn, m_nTransferParam ) )
			vecTransferPawn.push_back( pPawn );
	}

	std::stable_sort( vecTransferPawn.begin(), vecTransferPawn.end(), [] ( CPawn* pPawn1, CPawn* pPawn2 ) {
		auto n1 = ( pPawn1->m_pos.y + pPawn1->m_moveTo.y ) * 64 - pPawn1->m_nRenderOrder;
		auto n2 = ( pPawn2->m_pos.y + pPawn2->m_moveTo.y ) * 64 - pPawn2->m_nRenderOrder;
		return n1 > n2;
	} );
	for( CPawn* pPawn : vecTransferPawn )
	{
		m_pLastLevel->RemovePawn( pPawn );
		pPawn->SetParentEntity( m_pTransferEft );
		pPawn->SetPosition( m_pLastLevel->GetPosition() + pPawn->GetPosition() );
		pPawn->m_pos = pPawn->m_pos - d;
	}
	TransferFuncEnterLevel( &vecTransferPawn );

	auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
	int32 nFadeOutTotalFrames = 25;
	int32 nFadeOutFrame = nFadeOutTotalFrames;
	
	{
		float s1 = 0;
		float v1 = 0;
		for( int i = 0; ; i++ )
		{
			if( nFadeOutFrame )
			{
				m_pTransferCoroutine->Yield( 0 );
				m_pPlayer->UpdateAnimOnly();
				int32 nFrame = maskParams.size() - nFadeOutFrame;
				nFadeOutFrame--;
				float t = nFadeOutFrame * 1.0f / nFadeOutTotalFrames;
				if( nFrame >= 0 && nFrame < maskParams.size() )
				{
					pParams[0] = maskParams[nFrame].first;
					pParams[1] = maskParams[nFrame].second;
				}
				else
				{
					float a = t * 1.5f;
					float b = Max( 0.0f, t * 1.5f - 0.5f );
					pParams[0] = CVector4( a, b, b, 0 );
					pParams[1] = CVector4( 0, 0, 0, 0 );
				}
				CMyLevel* pLevel = m_pLastLevel;
				if( pLevel->GetEnvEffect() )
					pLevel->GetEnvEffect()->SetFade( t );
				if( !nFadeOutFrame )
				{
					pParams[0] = CVector4( 0, 0, 0, 0 );
					pParams[1] = CVector4( 0, 0, 0, 0 );
				}
			}

			float s0;
			if( i < 60 )
				s0 = 0.5f * i * i / 60 * 12;
			else if( i < 120 )
				s0 = ( 0.5f * 60 + ( i - 60 ) ) * 12;
			else
			{
				auto i1 = Max( 0, 180 - i );
				s0 = ( 120 - 0.5f * i1 * i1 / 60 ) * 12;
			}
			auto v2 = ( v1 + ( s0 - s1 ) * 0.005f ) * 0.95f;
			s1 += ( v2 + v1 ) * 0.5f;
			v1 = v2;
			if( i >= 180 && abs( s1 - s0 ) < 1 && abs( v1 ) < 1 )
				break;

			m_pTransferEft->SetPosition( CVector2( 0, floor( ( s0 - s1 ) * 0.5f + 0.5f ) * ( bUp ? 2 : -2 ) ) );
			m_pTransferCoroutine->Yield( 0 );
			m_pPlayer->UpdateAnimOnly();
		}
	}

	m_pLastLevel->SetParentEntity( NULL );
	m_pLastLevel = NULL;
	m_pCurLevel = pCurLevel;
	m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );

	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetFade( 0 );
	while( nTransferAnimFrames )
	{
		m_pTransferCoroutine->Yield( 0 );
		m_pPlayer->UpdateAnimOnly();
		int32 nFrame = nTransferAnimTotalFrames - nTransferAnimFrames;
		nTransferAnimFrames--;
		float t = 1 - nTransferAnimFrames * 1.0f / nTransferAnimTotalFrames;
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
		auto p = transferOfs * ( 1 - t );
		p = CVector2( floor( p.x * 0.5f + 0.5f ), floor( p.y * 0.5f + 0.5f ) ) * 2;
		m_pCurLevel->SetPosition( p );
		m_pTransferEft->SetPosition( p - transferOfs );
		if( m_pCurLevel->GetEnvEffect() )
			m_pCurLevel->GetEnvEffect()->SetFade( t );
		auto cam = camTransferBegin + ( m_pCurLevel->GetCamPos() - camTransferBegin ) * t;
		m_transferCurCamPos = CVector2( floor( cam.x * 0.5f + 0.5f ), floor( cam.y * 0.5f + 0.5f ) ) * 2;
	}

	for( CPawn* pPawn : vecTransferPawn )
		pPawn->SetParentEntity( NULL );
	m_pTransferEft->SetParentEntity( NULL );
	m_pTransferEft = NULL;
	m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
	for( CPawn* pPawn : vecTransferPawn )
		m_pCurLevel->AddPawn( pPawn, pPawn->GetPos(), pPawn->GetCurDir() );
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	BeginCurLevel();
}

void CMasterLevel::TransferFuncCut2Level()
{
	TransferFuncEnterLevel();
	m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetRenderParentBefore( m_pMainUI );
	int32 nTransferAnimFrames = CGlobalCfg::Inst().lvTransferData.nTransferFadeOutFrameCount;
	int32 nTransferAnimTotalFrames = nTransferAnimFrames;
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	pParams[0] = pParams[1] = CVector4( 0, 0, 0, 0 );
	if( m_pCurLevel->GetEnvEffect() )
		m_pCurLevel->GetEnvEffect()->SetFade( 0 );

	m_transferCurCamPos = m_pCurLevel->GetCamPos();
	auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
	for( ; nTransferAnimFrames > 0; nTransferAnimFrames-- )
	{
		m_pTransferCoroutine->Yield( 0 );
		int32 nFrame = nTransferAnimTotalFrames - nTransferAnimFrames;
		float t = 1 - ( nTransferAnimFrames - 1 ) * 1.0f / nTransferAnimTotalFrames;
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
		if( m_pCurLevel->GetEnvEffect() )
			m_pCurLevel->GetEnvEffect()->SetFade( t );
	}
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	ASSERT( !m_pLastLevel );
	m_pCurLevel->SetRenderParentBefore( m_pLevelFadeMask );
	m_pCurLevel->AddPawn( m_pPlayer, m_worldData.curFrame.playerEnterPos, m_worldData.curFrame.nPlayerEnterDir );
	BeginCurLevel();
}

void CMasterLevel::TransferFuncLevel2Cut()
{
	int32 nTransferFadeOutFrames = CGlobalCfg::Inst().lvTransferData.nTransferFadeOutFrameCount * 4;
	int32 nTransferFadeOutTotalFrames = nTransferFadeOutFrames;
	auto pParams = static_cast<CImage2D*>( m_pLevelFadeMask.GetPtr() )->GetParam();
	pParams[0] = CVector4( 1, 1, 1, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );

	m_transferCurCamPos = m_pCurLevel->GetCamPos();
	auto& maskParams = CGlobalCfg::Inst().lvTransferData.vecTransferMaskParams;
	for( ; nTransferFadeOutFrames; nTransferFadeOutFrames-- )
	{
		m_pTransferCoroutine->Yield( 0 );
		int32 nFrame = maskParams.size() - nTransferFadeOutFrames;
		float t = ( nTransferFadeOutFrames - 1 ) * 1.0f / nTransferFadeOutTotalFrames;
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
		CMyLevel* pLevel = m_pCurLevel;
		if( pLevel->GetEnvEffect() )
			pLevel->GetEnvEffect()->SetFade( t );
	}
	m_transferCurCamPos = CVector2( 0, 0 );
	pParams[0] = CVector4( 0, 0, 0, 0 );
	pParams[1] = CVector4( 0, 0, 0, 0 );
	m_pCurLevel->SetParentEntity( NULL );
	m_pCurLevel = NULL;
	m_pCurCutScene->SetParentBeforeEntity( m_pLevelFadeMask );
	m_pCurCutScene->Begin();
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
		REGISTER_MEMBER( m_fScenarioFade )
		REGISTER_MEMBER_BEGIN( m_strCondition )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelNextStageData )
		REGISTER_MEMBER( pNxtStage )
		REGISTER_MEMBER( nOfsX )
		REGISTER_MEMBER( nOfsY )
		REGISTER_MEMBER( strKeyOrig )
		REGISTER_MEMBER( strKeyRedirect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelStealthLayer )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelIndicatorLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_pPawnTargetEftPrefab )
		REGISTER_MEMBER_TAGGED_PTR( m_pStealthLayer, 1 )
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
		REGISTER_MEMBER( m_nDepth )
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
		REGISTER_MEMBER_BEGIN( m_strInitScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strBeginScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strDestroyScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_strBGM )
		REGISTER_MEMBER( m_nBGMPriority )

		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( CheckGrid )
		REGISTER_LUA_CFUNCTION( SpawnPawn )
		REGISTER_LUA_CFUNCTION( SpawnPawn1 )
		REGISTER_LUA_CFUNCTION( SpawnPreset )
		REGISTER_LUA_CFUNCTION( SpawnPreset1 )
		REGISTER_LUA_CFUNCTION( GetPresetSpawnX )
		REGISTER_LUA_CFUNCTION( GetPresetSpawnY )
		REGISTER_LUA_CFUNCTION( RemovePawn )
		REGISTER_LUA_CFUNCTION( IsFailed )
		REGISTER_LUA_CFUNCTION( IsSnapShot )
		REGISTER_LUA_CFUNCTION( SetEnvEffect )
		REGISTER_LUA_CFUNCTION( Fail )
		REGISTER_LUA_CFUNCTION( Freeze )
		REGISTER_LUA_CFUNCTION( GetPawnByName )
		REGISTER_LUA_CFUNCTION_RETUNWR( GetAllPawnsByNameScript )
		REGISTER_LUA_CFUNCTION_RETUNWR( GetAllPawnsByTagScript )
		REGISTER_LUA_CFUNCTION( GetPawnByGrid )
		REGISTER_LUA_CFUNCTION( Redirect )
		REGISTER_LUA_CFUNCTION( ReplaceTiles )
		REGISTER_LUA_CFUNCTION( ScriptForEachPawn )
		REGISTER_LUA_CFUNCTION( ScriptForEachEnemy )
		REGISTER_LUA_CFUNCTION( BeginTracer )
		REGISTER_LUA_CFUNCTION( BeginTracer1 )
		REGISTER_LUA_CFUNCTION( EndTracer )
		REGISTER_LUA_CFUNCTION( BlockTracer )
		REGISTER_LUA_CFUNCTION( SetTracerDelay )
		REGISTER_LUA_CFUNCTION( GetTracerSpawnExit )
		REGISTER_LUA_CFUNCTION( BeginNoise )
		REGISTER_LUA_CFUNCTION( EndNoise )
		REGISTER_LUA_CFUNCTION( IsExitBlocked )
		REGISTER_LUA_CFUNCTION( GetGridExit )
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
		REGISTER_MEMBER_TAGGED_PTR( m_pLabelsRoot, labels )
		REGISTER_MEMBER_TAGGED_PTR( m_pLabelsCounter, label_counter )
		REGISTER_MEMBER_TAGGED_PTR( m_pAmmoCount, icon_1/ammo )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( ScenarioText )
		REGISTER_LUA_CFUNCTION( IsScenarioTextFinished )
		REGISTER_LUA_CFUNCTION( HeadText )
		REGISTER_LUA_CFUNCTION( ShowFreezeEft )
		REGISTER_LUA_CFUNCTION( ClearLabels )
		REGISTER_LUA_CFUNCTION( SetLabel )
		REGISTER_LUA_CFUNCTION( SetLabelCounter )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMasterLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pMainUI, main_ui )
		REGISTER_MEMBER_TAGGED_PTR( m_pLevelFadeMask, mask )
		REGISTER_MEMBER_TAGGED_PTR( m_pSnapShotRoot, sn )
		REGISTER_MEMBER_TAGGED_PTR( m_pSnapShotMask, mask0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pBattleEffect, battle_eft )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenu, menu )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[0], menu/t1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[1], menu/t2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[2], menu/t3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[3], menu/t4 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[4], menu/t5 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuSelected, menu/selected )
		REGISTER_MEMBER_TAGGED_PTR( m_pWorldMap, world_map )
		REGISTER_MEMBER_TAGGED_PTR( m_pActionPreview, action_preview )
		REGISTER_MEMBER_TAGGED_PTR( m_pLogUI, log )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( GetMainUI )
		REGISTER_LUA_CFUNCTION( GetCurLevelName )
		REGISTER_LUA_CFUNCTION( GetLastLevelName )
		REGISTER_LUA_CFUNCTION( IsScenario )
		REGISTER_LUA_CFUNCTION( CheckPoint )
		REGISTER_LUA_CFUNCTION( EvaluateKeyInt )
		REGISTER_LUA_CFUNCTION( EvaluateKeyString )
		REGISTER_LUA_CFUNCTION( SetKeyInt )
		REGISTER_LUA_CFUNCTION( SetKeyString )
		REGISTER_LUA_CFUNCTION( ClearKeys )
		REGISTER_LUA_CFUNCTION( ClearSnapShot )
		REGISTER_LUA_CFUNCTION( Respawn )
		REGISTER_LUA_CFUNCTION( RespawnLevel )
		REGISTER_LUA_CFUNCTION( ClearByPrefix )
		REGISTER_LUA_CFUNCTION( SetLevelIgnoreGlobalClearKeys )
		REGISTER_LUA_CFUNCTION( PushPlayerData )
		REGISTER_LUA_CFUNCTION( PopPlayerData )
		REGISTER_LUA_CFUNCTION( TransferBy )
		REGISTER_LUA_CFUNCTION( ScriptTransferTo )
		REGISTER_LUA_CFUNCTION( ScriptTransferOprFunc )
		REGISTER_LUA_CFUNCTION( UnlockRegionMap )
		REGISTER_LUA_CFUNCTION( ShowWorldMap )
		REGISTER_LUA_CFUNCTION( ShowDoc )
		REGISTER_LUA_CFUNCTION( IsMenuShow )
		REGISTER_LUA_CFUNCTION( AddLevelMark )
		REGISTER_LUA_CFUNCTION( HasLevelMark )
		REGISTER_LUA_CFUNCTION( RemoveLevelMark )
		REGISTER_LUA_CFUNCTION( ShowInteractionUI )
		REGISTER_LUA_CFUNCTION( GotoInteractionUI )
		REGISTER_LUA_CFUNCTION( GetInteractionUI )
		REGISTER_LUA_CFUNCTION( BlackOut )
		REGISTER_LUA_CFUNCTION( InterferenceStripEffect )
		REGISTER_LUA_CFUNCTION( SetGlobalBGM )
	REGISTER_CLASS_END()
}