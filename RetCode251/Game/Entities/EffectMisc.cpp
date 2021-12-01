#include "stdafx.h"
#include "EffectMisc.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Render/Image2D.h"
#include "Render/DrawableGroup.h"

void CDropBombEffect::OnAddedToStage()
{
	GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
	Init();
}

void CDropBombEffect::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDropBombEffect::Init()
{
	int32 nMaxItems = m_widthParam.x / m_fImgWidth;
	m_vecElems.resize( nMaxItems );
	m_pOrigRenderObject = GetRenderObject();
	SetRenderObject( NULL );
	auto pImg = static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() );
	int32 nImgTiles = pImg->GetElem().rect.width / m_fImgWidth;
	for( auto& elem : m_vecElems )
	{
		elem.nTexX = SRand::Inst<eRand_Render>().Rand( 0, nImgTiles );
		elem.elem.nInstDataSize = pImg->GetParamCount() * sizeof( CVector4 );
		elem.elem.pInstData = pImg->GetParam();
	}
	SetLocalBound( CRectangle( -m_widthParam.x * 0.5f, 0, m_widthParam.x, m_height1Param.y ) );
}

void CDropBombEffect::Render( CRenderContext2D& context )
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

	for( auto& elem : m_vecElems )
	{
		elem.elem.worldMat = globalTransform;
		elem.elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem.elem, nGroup );
	}
}

void CDropBombEffect::OnTick()
{
	if( m_nTick == m_nLife )
	{
		SetParentEntity( NULL );
		return;
	}

	auto pImg = static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() );
	int32 nImgTiles = pImg->GetElem().rect.width / m_fImgWidth;
	float t = m_nTick / ( m_nLife - 1.0f );
	float t1 = t * ( 2 - t );
	float fWidth = m_widthParam.x + ( m_widthParam.y - m_widthParam.x ) * ( t + m_widthParam.z * ( t1 - t ) );
	float fHeight = m_heightParam.x + ( m_heightParam.y - m_heightParam.x ) * ( t + m_heightParam.z * ( t1 - t ) );
	float fHeight1 = m_height1Param.x + ( m_height1Param.y - m_height1Param.x ) * ( t + m_height1Param.z * ( t1 - t ) );
	int32 nItems1 = fWidth / m_fImgWidth;
	if( nItems1 < m_vecElems.size() )
	{
		int32 nRemove = m_vecElems.size() - nItems1;
		int32 nRemoveBegin = ( nItems1 + SRand::Inst<eRand_Render>().Rand( 0, nImgTiles ) ) / 2;
		for( int i = nRemoveBegin; i < nItems1; i++ )
			m_vecElems[i].nTexX = m_vecElems[i + nRemove].nTexX;
		m_vecElems.resize( nItems1 );
	}

	float i0 = ( m_vecElems.size() - 1 ) * 0.5f;
	auto w0 = pImg->GetElem().rect.width;
	auto h0 = pImg->GetElem().rect.height;
	auto texRect0 = pImg->GetElem().texRect;
	float fTexWidth = texRect0.width * m_fImgWidth / w0;
	for( int i = 0; i < m_vecElems.size(); i++ )
	{
		float t = abs( i - i0 ) / ( fWidth / m_fImgWidth );
		float h = fHeight1 + ( fHeight - fHeight1 ) * t;

		auto& elem = m_vecElems[i];
		auto& rect = elem.elem.rect;
		auto& texRect = elem.elem.texRect;
		rect = CRectangle( ( i - i0 - 0.5f ) * m_fImgWidth, 0, m_fImgWidth, h );
		texRect = CRectangle( texRect0.x + elem.nTexX * fTexWidth, texRect0.GetBottom() - texRect0.height * h / h0, fTexWidth, texRect0.height * h / h0 );

		elem.elem.nInstDataSize = pImg->GetParamCount() * sizeof( CVector4 );
		elem.elem.pInstData = pImg->GetParam();
	}
	m_nTick++;
}

void CSpinEffect::OnAddedToStage()
{
	GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
	if( m_bFront )
	{
		auto p = CMyLevel::GetEntityLevel( this );
		if( p )
			SetRenderParent( p );
	}
	else if( m_bBack )
	{
		auto p = CMyLevel::GetEntityCharacterRootInLevel( this );
		if( p )
			SetRenderParentAfter( p );
	}
	if( !m_bInited )
		Init( SRand::Inst<eRand_Render>().nSeed );
}

void CSpinEffect::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CSpinEffect::Init( int32 nSeed )
{
	m_bInited = true;
	m_pOrigRenderObject = GetRenderObject();
	m_nSeed = nSeed;
	m_nSpawnTick = m_nInitTime;
	int32 nMaxItems = ( m_nLoopImgCount + m_arrLoopAnim.Size() - 1 ) * m_nFrameSpeed;
	nMaxItems = ( nMaxItems - 1 + m_nSpawnInterval ) / m_nSpawnInterval;
	m_vecItems.resize( nMaxItems );
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	m_origRect = pImg->GetElem().rect;
	m_origTexRect = pImg->GetElem().texRect;
	SetRenderObject( NULL );
}

void CSpinEffect::Render( CRenderContext2D& context )
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

	for( auto& elem : m_vecElems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

void CSpinEffect::OnTick()
{
	auto nMaxTime = ( m_nLoopImgCount + m_arrLoopAnim.Size() - 1 ) * m_nFrameSpeed;
	for( int i = m_nItemBegin; i < m_nItemEnd; i++ )
	{
		auto& item = m_vecItems[i % m_vecItems.size()];
		item.nTime++;
		if( item.nTime >= nMaxTime )
		{
			m_nItemBegin++;
			continue;
		}
	}

	SRand rnd;
	rnd.nSeed = m_nSeed;
	m_nSpawnTick++;
	auto nSpawn = m_nSpawnTick / m_nSpawnInterval;
	m_nSpawnTick -= m_nSpawnInterval * nSpawn;
	nSpawn = Min<int32>( nSpawn, m_vecItems.size() );
	for( int i = nSpawn - 1; i >= 0; i-- )
	{
		auto& item = m_vecItems[( m_nItemEnd++ ) % m_vecItems.size()];
		item.nTime = i * m_nSpawnInterval + m_nSpawnTick;
		item.nInitFrame = rnd.Rand( 0, m_nRows );
		item.initOfs = CVector2( m_spawnRect.x + rnd.Rand( 0.0f, m_spawnRect.width ), m_spawnRect.y + rnd.Rand( 0.0f, m_spawnRect.height ) );
		item.loopOfs = CVector2( m_loopOffset.x + rnd.Rand( 0.0f, m_loopOffset.width ), m_loopOffset.y + rnd.Rand( 0.0f, m_loopOffset.height ) );
	}
	m_nSeed = rnd.nSeed;

	if( m_nItemBegin >= m_vecItems.size() )
	{
		auto nBegin = m_nItemBegin % m_vecItems.size();
		m_nItemEnd -= m_nItemBegin - nBegin;
		m_nItemBegin = nBegin;
	}

	m_vecElems.resize( 0 );
	int32 nX = 0, nY = 0;
	CRectangle rect( 0, 0, 0, 0 );
	auto pImg = static_cast<CImage2D*>( m_pOrigRenderObject.GetPtr() );
	for( int j = 0; j < m_arrLoopAnim.Size(); j++ )
	{
		int32 nTime0 = j * m_nFrameSpeed;
		for( int i = m_nItemBegin; i < m_nItemEnd; i++ )
		{
			auto& item = m_vecItems[i % m_vecItems.size()];
			if( item.nTime < nTime0 || item.nTime >= nTime0 + m_nLoopImgCount * m_nFrameSpeed )
				continue;
			int32 nFrame = ( item.nTime - nTime0 ) / m_nFrameSpeed;
			int32 nFrameX = nX;
			int32 nFrameY = ( item.nInitFrame + nFrame + nY ) % m_nRows;

			m_vecElems.resize( m_vecElems.size() + 1 );
			auto& elem = m_vecElems.back();
			elem.rect = m_origRect.Offset( item.initOfs + item.loopOfs * nFrame );
			elem.texRect = m_origTexRect.Offset( CVector2( nFrameX, nFrameY ) * m_texOfs );
			rect = rect + elem.rect;

			elem.nInstDataSize = pImg->GetParamCount() * sizeof( CVector4 );
			elem.pInstData = pImg->GetParam();
		}
		if( m_arrLoopAnim[j] )
			nX++;
		else
			nY++;
	}
	m_localBound = rect;
	SetBoundDirty();
}

void RegisterGameClasses_EffectMisc()
{
	REGISTER_CLASS_BEGIN( CDropBombEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_fImgWidth )
		REGISTER_MEMBER( m_widthParam )
		REGISTER_MEMBER( m_heightParam )
		REGISTER_MEMBER( m_height1Param )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSpinEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nFrameSpeed )
		REGISTER_MEMBER( m_nRows )
		REGISTER_MEMBER( m_texOfs )
		REGISTER_MEMBER( m_nLoopImgCount )
		REGISTER_MEMBER( m_arrLoopAnim )
		REGISTER_MEMBER( m_loopOffset )
		REGISTER_MEMBER( m_nSpawnInterval )
		REGISTER_MEMBER( m_nInitTime )
		REGISTER_MEMBER( m_spawnRect )
		REGISTER_MEMBER( m_bFront )
		REGISTER_MEMBER( m_bBack )
	REGISTER_CLASS_END()
}