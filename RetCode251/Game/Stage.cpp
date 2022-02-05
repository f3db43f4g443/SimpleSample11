#include "stdafx.h"
#include "Stage.h"
#include "Render/Scene2DManager.h"
#include "Player.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "UICommon/UIViewport.h"
#include "MyLevel.h"
#include "MyGame.h"
#include "PostEffects.h"

CStage::CStage( CWorld* pWorld ) : m_pWorld( pWorld ), m_pContext( NULL ), m_bStarted( false ), m_nLightType( 0 ), m_pPlayer( NULL ), m_onPostProcess( this, &CStage::OnPostProcess )
{
	m_pEntityRoot = new CEntity;
}

CStage::~CStage()
{
	Stop();
	m_pEntityRoot = NULL;
}

void CStage::Create( SStageContext* pContext )
{
	m_pContext = pContext;
	m_nLightType = m_pContext->nLightType;
	if( pContext )
	{
		if( !pContext->nStageInsts )
		{
			for( auto& item : m_pContext->mapDependentRes )
			{
				item.second = CResourceManager::Inst()->CreateResource( item.first.c_str() );
			}
		}
		pContext->nStageInsts++;
	}

	CPrefab* pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_pContext->strSceneResName.c_str() );
	if( pPrefab )
	{
		CReference<CRenderObject2D> pRenderObject = pPrefab->GetRoot()->CreateInstance();
		if( pRenderObject )
		{
			CEntity* pEntity = static_cast<CEntity*>( pRenderObject.GetPtr() );
			if( pEntity )
				pEntity->SetParentEntity( m_pEntityRoot );
		}
	}
}

void CStage::Start( CPlayer* pPlayer, const SStageEnterContext& context )
{
	if( m_bStarted )
		return;

	auto pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->GetRoot()->AddChild( m_pEntityRoot );
	AddEntity( m_pEntityRoot );

	m_enterContext = context;
	if( m_enterContext.pViewport )
	{
		if( CMasterLevel::GetInst() )
			m_enterContext.pViewport->Set( CMasterLevel::GetInst(), m_nLightType, &m_camera );
		else
			m_enterContext.pViewport->Set( m_pEntityRoot, m_nLightType, &m_camera );
	}
	auto viewSize = m_camera.GetViewArea().GetSize();
	m_origCamSize = viewSize;
	if( m_enterContext.pViewport )
	{
		//m_enterContext.pViewport->SetCustomRender( viewSize * 0.25f );
		//m_enterContext.pViewport->RegisterOnPostProcess( &m_onPostProcess );
	}

	m_bStarted = true;

	m_events.Trigger( eStageEvent_Start, NULL );
}

void CStage::Stop()
{
	if( !m_bStarted )
		return;
	if( m_onPostProcess.IsRegistered() )
		m_onPostProcess.Unregister();
	m_events.Trigger( eStageEvent_Stop, NULL );
	m_bStarted = false;
	m_mapStartPoints.clear();
	auto pSceneMgr = CScene2DManager::GetGlobalInst();
	if( m_enterContext.pViewport )
		m_enterContext.pViewport->Set( NULL, NULL, false );
	RemoveEntity( m_pEntityRoot );
	m_tickBeforeHitTest.Clear();
	m_tickAfterHitTest.Clear();

	if( m_pContext )
	{
		m_pContext->nStageInsts--;
		if( !m_pContext->nStageInsts )
		{
			for( auto& item : m_pContext->mapDependentRes )
			{
				item.second = NULL;
			}
		}
	}
}

void CStage::SetViewport( CUIViewport* pViewport )
{
	auto pSceneMgr = CScene2DManager::GetGlobalInst();
	if( m_enterContext.pViewport )
		m_enterContext.pViewport->Set( NULL, NULL, false );
	
	m_enterContext.pViewport = pViewport;
	if( pViewport )
		m_enterContext.pViewport->Set( m_pEntityRoot, m_nLightType, &m_camera );
}

void CStage::AddEntity( CEntity* pEntity )
{
	pEntity->AddRef();
	pEntity->SetStage( this );
	pEntity->m_bIsChangingStage = true;
	if( pEntity->AddToStageHitTest() && pEntity->Get_HitProxy() )
		m_hitTestMgr.Add( pEntity );
	pEntity->OnAddedToStage();
	pEntity->m_bIsChangingStage = false;

	CPlayer* pPlayer = SafeCast<CPlayer>( pEntity );
	if( pPlayer )
		m_pPlayer = pPlayer;

	pEntity->m_trigger.Trigger( eEntityEvent_AddedToStage, this );

	for( CEntity* pChild = pEntity->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		AddEntity( pChild );
	}
}

void CStage::RemoveEntity( CEntity* pEntity )
{
	pEntity->m_trigger.Trigger( eEntityEvent_RemovedFromStage, NULL );

	for( CEntity* pChild = pEntity->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		RemoveEntity( pChild );
	}

	if( m_pPlayer == pEntity )
		m_pPlayer = NULL;
	
	pEntity->m_bIsChangingStage = true;
	pEntity->OnRemovedFromStage();
	pEntity->SetTraverseIndex( -1 );
	if( pEntity->AddToStageHitTest() && pEntity->Get_HitProxy() )
		m_hitTestMgr.Remove( pEntity );
	pEntity->SetStage( NULL );
	pEntity->m_bIsChangingStage = false;
	pEntity->Release();
}

#include "Common/Profile.h"
void CStage::Update()
{
	PROFILE_BEGIN( Update )
	PROFILE_BEGIN( Tick1 )
	m_nUpdatePhase = eStageUpdatePhase_BeforeHitTest;
	m_tickBeforeHitTest.UpdateTime();
	PROFILE_END( Tick1 )
	PROFILE_BEGIN( UpdateDirty1 )
	CScene2DManager::GetGlobalInst()->UpdateDirty();
	PROFILE_END( UpdateDirty1 )

	/*auto pGame = &CGame::Inst();
	if( pGame->IsKey( 'A' ) )
		m_pWorld->ChangePercent( 0.04f );
	if( pGame->IsKey( 'D' ) )
		m_pWorld->ChangePercent( -0.04f );*/
	m_events.Trigger( eStageEvent_UpdateHpBar, NULL );
	PROFILE_BEGIN( HitTest )
	m_nUpdatePhase = eStageUpdatePhase_HitTest;
	m_pEntityRoot->BeforeHitTest( 0 );
	PROFILE_BEGIN( HitTest1 )
	m_hitTestMgr.Update();
	PROFILE_END( HitTest1 )
	PROFILE_END( HitTest )
	PROFILE_BEGIN( Tick2 )
	m_events.Trigger( eStageEvent_PostHitTest, NULL );
	m_nUpdatePhase = eStageUpdatePhase_AfterHitTest;
	m_tickAfterHitTest.UpdateTime();
	PROFILE_END( Tick2 )
	
	PROFILE_BEGIN( UpdateDirty2 )
	CScene2DManager::GetGlobalInst()->UpdateDirty();
	PROFILE_END( UpdateDirty2 )
	CMasterLevel* pMasterLevel = CMasterLevel::GetInst();
	if( pMasterLevel )
	{
		pMasterLevel->Update();
	}
	CRectangle rect( -m_origCamSize.x * 0.5f, -m_origCamSize.y * 0.5f, m_origCamSize.x, m_origCamSize.y );
	if( pMasterLevel )
	{
		auto trans = pMasterLevel->GetCamTrans();
		CVector2 ofs( trans.x, trans.y );
		float r = trans.z;
		float s = trans.w;
		rect = rect * s;
		rect = rect.Offset( ofs );
		m_camera.SetRotation( r );
	}
	else
		m_camera.SetRotation( 0 );
	m_camera.SetViewArea( rect );

	m_events.Trigger( eStageEvent_PostUpdate, NULL );
	PROFILE_END( Update )

	CProfileMgr::Inst()->OnFrameMove();
}

void CStage::OnPostProcess( CPostProcessPass* pPass )
{
	static CPostProcessPixelUpsample effect;
	effect.SetPriority( 1 );
	CVector4 colorCenter( 1, 1, 1, 1 );
	CVector4 colorEdge( 1, 1, 1, 1 );
	/*float k = ( abs( ( CGame::Inst().GetTimeStamp() & 511 ) - 256 ) - 128 ) / 128.0f;
	colorCenter = colorCenter - CVector4( 0.25f, 0.2f, 0.15f, 0 ) * k;
	colorEdge = colorEdge + CVector4( 0.25f, 0.2f, 0.15f, 0 ) * k;*/
	float fPow = 2.5f;
	CMasterLevel* pMasterLevel = CMasterLevel::GetInst();
	if( pMasterLevel )
	{
		auto pPlayer = pMasterLevel->GetPlayer();
		if( pPlayer->IsKilled() )
		{
			float t = pMasterLevel->GetKillFade();
			colorEdge = CVector4( 1, 0, 0, 1 );
			colorCenter = CVector4( 1, 0, 0, 1 ) + CVector4( 0, 1, 1, 0 ) * t;
			fPow = 2.5f;
		}
		else
		{
			float t = pPlayer->GetHp() * 1.0f / pPlayer->GetMaxHp();
			float t1 = pPlayer->GetFallCritical();
			if( t < 1 )
			{
				colorEdge = CVector4( 1, 0, 0, 1 ) + CVector4( 0, 1, 1, 0 ) * t;
			}
			if( t1 > 0 )
				colorCenter = colorCenter + CVector4( 1, 1, 1, 0 ) * t1 * ( 2 - t1 );
			if( t < 1 || t1 > 0 )
				fPow = 2.5f * pow( 4, t );
		}
	}
	effect.Set( colorCenter, colorEdge, fPow );
	pPass->Register( &effect );
}