#include "stdafx.h"
#include "Stage.h"
#include "Render/Scene2DManager.h"
#include "Player.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "UICommon/UIViewport.h"
#include "MyLevel.h"
#include "Entities/StartPoint.h"

CStage::CStage( CWorld* pWorld ) : m_pWorld( pWorld ), m_pContext( NULL ), m_bStarted( false ), m_bLight( false ), m_pPlayer( NULL ), m_onPostProcess( this, &CStage::OnPostProcess )
{
	m_pEntityRoot = new CEntity;
	m_pFootprintMgr = NULL;
	//m_pFootprintMgr = new CFootprintMgr;
}

CStage::~CStage()
{
	Stop();
	m_pEntityRoot = NULL;
}

void CStage::Create( SStageContext* pContext )
{
	m_pContext = pContext;
	m_bLight = m_pContext->bLight;
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
		m_enterContext.pViewport->Set( m_pEntityRoot, &m_camera, m_bLight );

	if( m_pFootprintMgr && m_pFootprintMgr->GetFootprintRoot() )
	{
		static CFootprintDrawable* pRenderDrawable = NULL;
		if( !pRenderDrawable )
		{
			vector<char> content;
			GetFileContent( content, "materials/footprint_persistent.xml", true );
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			pRenderDrawable = new CFootprintDrawable;
			pRenderDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render" ) );
			doc.Clear();
		}
		CFootprintReceiver* pReceiver = new CFootprintReceiver( NULL, pRenderDrawable, NULL );
		pReceiver->SetPersistent( CRectangle( -1024, -512, 2048, 1024 ) );
		pReceiver->SetZOrder( -1 );
		AddFootprint( pReceiver );
	}
	m_bStarted = true;

	if( pPlayer )
	{
		auto pStartPoint = m_pEntityRoot->Get_ChildEntity()->GetChildByName_Fast<CStartPoint>( context.strStartPointName.c_str() );
		if( pStartPoint )
		{
			pPlayer->SetPosition( pStartPoint->GetPosition() );
			pPlayer->SetParentBeforeEntity( pStartPoint );
		}
	}

	CPostProcessPass::GetPostProcessPass( ePostProcessPass_PreGUI )->RegisterOnPostProcess( &m_onPostProcess );
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
	if( m_pFootprintMgr )
		pSceneMgr->RemoveFootprintMgr( m_pFootprintMgr );
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
		m_enterContext.pViewport->Set( m_pEntityRoot, &m_camera, m_bLight );
}

void CStage::AddEntity( CEntity* pEntity )
{
	pEntity->AddRef();
	pEntity->SetStage( this );
	pEntity->m_bIsChangingStage = true;
	if( pEntity->Get_HitProxy() )
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
	if( pEntity->Get_HitProxy() )
		m_hitTestMgr.Remove( pEntity );
	pEntity->SetStage( NULL );
	pEntity->m_bIsChangingStage = false;
	pEntity->Release();
}

CEntity* CStage::Pick( const CVector2& pos )
{
	SHitProxyCircle hitProxy;
	hitProxy.center = pos;
	hitProxy.fRadius = 0.01f;
	CMatrix2D transform;
	transform.Identity();
	vector<CHitProxy*> vecResults;
	m_hitTestMgr.HitTest( &hitProxy, transform, vecResults );

	CEntity* pEntity = NULL;
	uint32 nMinTraverseOrder = -1;
	for( int i = 0; i < vecResults.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( vecResults[i] );
		if( pEntity1->GetTraverseIndex() < nMinTraverseOrder )
		{
			nMinTraverseOrder = pEntity->GetTraverseIndex();
			pEntity = pEntity1;
		}
	}
	return pEntity;
}

void CStage::MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result )
{
	SHitProxyCircle hitProxy;
	hitProxy.center = pos;
	hitProxy.fRadius = 0.01f;
	CMatrix2D transform;
	transform.Identity();
	vector<CHitProxy*> vecResults;
	m_hitTestMgr.HitTest( &hitProxy, transform, vecResults );
	
	for( int i = 0; i < vecResults.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( vecResults[i] );
		result.push_back( pEntity );
	}
}

void CStage::MultiHitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CReference<CEntity> >& result, vector<SHitTestResult>* pResult )
{
	vector<CHitProxy*> tempResult;
	m_hitTestMgr.HitTest( pProxy, transform, tempResult, pResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i] );
		result.push_back( pEntity );
	}
}

CEntity* CStage::Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType, SRaycastResult* pResult )
{
	vector<SRaycastResult> result;
	m_hitTestMgr.Raycast( begin, end, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( hitType != eEntityEvent_Count && pEntity->GetHitType() != hitType )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

void CStage::MultiRaycast( const CVector2& begin, const CVector2& end, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult )
{
	vector<SRaycastResult> tempResult;
	m_hitTestMgr.Raycast( begin, end, tempResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i].pHitProxy );
		if( pResult )
			pResult->push_back( tempResult[i] );
		result.push_back( pEntity );
	}
}

CEntity * CStage::SweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, EEntityHitType hitType, SRaycastResult * pResult, bool bIgnoreInverseNormal )
{
	vector<SRaycastResult> result;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( hitType != eEntityEvent_Count && pEntity->GetHitType() != hitType )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

CEntity * CStage::SweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult * pResult, bool bIgnoreInverseNormal )
{
	vector<SRaycastResult> result;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( !hitTypeFilter[pEntity->GetHitType()] )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

void CStage::MultiSweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, vector<CReference<CEntity>>& result, vector<SRaycastResult>* pResult )
{
	vector<SRaycastResult> tempResult;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, tempResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i].pHitProxy );
		if( pResult )
			pResult->push_back( tempResult[i] );
		result.push_back( pEntity );
	}
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
	CMyLevel* pLevel = CMyLevel::GetInst();
	if( m_pPlayer && pLevel )
	{
		CVector2 camPos = m_pPlayer->GetCam();
		CMyLevel::GetInst()->UpdateBack0Position( m_pPlayer->GetPosition() );
		//CVector2 camShake = CVector2( cos( IRenderSystem::Inst()->GetTotalTime() * 1.3592987 * 60 ), cos( IRenderSystem::Inst()->GetTotalTime() * 1.4112051 * 60 ) )
		//	* Min( pLevel->GetShakeStrength() / 25.0f, 1.0f ) * 2.0f;

		m_camera.SetPosition( floor( camPos.x + 0.5f ), floor( camPos.y + 0.5f ) );
		//m_camera.SetExtraOfs( CVector2( floor( camShake.x + 0.5f ), floor( camShake.y + 0.5f ) ) );
	}
	PROFILE_END( Update )

	CProfileMgr::Inst()->OnFrameMove();
}

void CStage::OnPostProcess( CPostProcessPass* pPass )
{
	CMyLevel* pLevel = CMyLevel::GetInst();
	if( m_pPlayer && pLevel )
	{
		float fHurt = Min( 1.0f, m_pPlayer->GetInvicibleTimeLeft() / 0.25f );
		CVector2 vecKnockback = m_pPlayer->GetKnockback();
		float fKnockbackLen = vecKnockback.Length();

		CVector4 texOfs[5];
		CVector4 weights[5] =
		{
			{ 0.0f, 1, 0, 0.2f },
			{ 0.0f, 0, 1, 0.2f },
			{ 1.0f, 0, 0, 0.2f },
			{ 0.0f, 0, 0, 0.2f },
			{ 0.0f, 0, 0, 0.2f }
		};
		CVector4 weights1[5] =
		{
			{ 0.0f, 0.25f, 0, 0.2f },
			{ 0.0f, 0, 0, 0.2f },
			{ 1.0f, 0, 0, 0.2f },
			{ 0.0f, 0, 0, 0.2f },
			{ 0.0f, 0, 0.25f, 0.2f }
		};
		CVector4 weights2[5] =
		{
			{ 0.5f, 0.5f, 0.5f, 0.2f },
			{ 0.5f, 0.5f, 0.5f, 0.2f },
			{ 0.5f, 0.5f, 0.5f, 0.2f },
			{ 0.0f, 0, 0, 0.2f },
			{ 0.0f, 0.0f, 0, 0.2f }
		};
		for( int i = 0; i < 5; i++ )
		{
			weights[i] = weights[i] * ( 1 - fHurt ) * ( 1 - fKnockbackLen ) + weights1[i] * fHurt + weights2[i] * ( 1 - fHurt ) * fKnockbackLen;
		}

		CVector2 camShake = CVector2( cos( IRenderSystem::Inst()->GetTotalTime() * 1.3592987 * 60 ), cos( IRenderSystem::Inst()->GetTotalTime() * 1.4112051 * 60 ) )
			* ( Min( pLevel->GetShakeStrength() / 64.0f, 1.0f ) + fHurt );
		camShake = camShake + vecKnockback * 5;
		CVector2 ofs = camShake;

		CVector2 ofs1 = CVector2( ofs.y, -ofs.x );
		for( int i = 0; i < 5; i++ )
		{
			texOfs[i] = CVector4( ofs.x, ofs.y, ofs1.x, ofs1.y ) * ( i - 2 );
		}

		static CPostProcessDizzyEffect effect;
		effect.SetPriority( 1 );
		effect.SetTexOfs( texOfs );
		effect.SetWeights( weights );
		pPass->Register( &effect );
	}

}

bool CStage::AddFootprint( CFootprintReceiver* pReceiver )
{
	if( !GetFootprintRoot() )
		return false;
	GetFootprintRoot()->AddChild( pReceiver );
	pReceiver->SetMgr( m_pFootprintMgr );

	return true;
}