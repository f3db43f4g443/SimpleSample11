#include "stdafx.h"
#include "Stage.h"
#include "BasicElems.h"
#include "Render/Scene2DManager.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "UICommon/UIViewport.h"
#include "MyLevel.h"
#include "MyGame.h"
#include "GlobalCfg.h"

CStage::CStage( CWorld* pWorld ) : m_pWorld( pWorld ), m_pContext( NULL ), m_bStarted( false ), m_bLight( false ), m_onPostProcess( this, &CStage::OnPostProcess )
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
			auto pLevel = SafeCast<CMasterLevel>( pEntity );
			if( pLevel )
				m_pMasterLevel = pLevel;
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

	m_bStarted = true;

	if( pPlayer )
	{
		if( m_pMasterLevel )
		{
			m_pMasterLevel->Init( pPlayer );
			CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( CGlobalCfg::Inst().strEntry.c_str() );
			m_pMasterLevel->Begin( pPrefab, CGlobalCfg::Inst().playerEnterPos, CGlobalCfg::Inst().nPlayerEnterDir );
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
	RemoveEntity( m_pEntityRoot );
	m_tick.Clear();

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

	if( m_pMasterLevel == pEntity )
		m_pMasterLevel = NULL;
	
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

void CStage::MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result, float fRad )
{
	SHitProxyCircle hitProxy;
	hitProxy.center = pos;
	hitProxy.fRadius = fRad;
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
		if( hitType != eEntityHitType_Count && pEntity->GetHitType() != hitType )
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

CEntity* CStage::SweepTest( SHitProxy* pHitProxy, const CMatrix2D& trans, const CVector2& sweepOfs, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult * pResult, bool bIgnoreInverseNormal )
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

CEntity * CStage::SweepTest( CEntity* pEntity, const CMatrix2D& trans, const CVector2& sweepOfs, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult * pResult, bool bIgnoreInverseNormal )
{
	SHitProxy* pHitProxy = pEntity->Get_HitProxy();
	if( !pHitProxy )
		return NULL;
	vector<SRaycastResult> result;
	m_hitTestMgr.SweepTest( pHitProxy, trans, sweepOfs, result );
	CEntity* p = NULL;
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		pEntity1->AddRef();
	}
	for( int i = 0; i < result.size(); i++ )
	{
		if( result[i].pHitProxy == pEntity )
			continue;
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		if( !hitTypeFilter[pEntity1->GetHitType()] )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( !pEntity1->CanHit( pEntity ) || !pEntity->CanHit1( pEntity1, result[i] ) )
			continue;
		if( pResult )
			*pResult = result[i];
		p = pEntity1;
		break;
	}
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		pEntity1->Release();
	}
	return p;
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
		
	/*PROFILE_BEGIN( HitTest )
	m_nUpdatePhase = eStageUpdatePhase_HitTest;
	m_pEntityRoot->BeforeHitTest( 0 );
	PROFILE_BEGIN( HitTest1 )
	m_hitTestMgr.Update();
	PROFILE_END( HitTest1 )
	PROFILE_END( HitTest )
	PROFILE_BEGIN( Tick2 )
	m_events.Trigger( eStageEvent_PostHitTest, NULL );
	m_nUpdatePhase = eStageUpdatePhase_AfterHitTest;
	PROFILE_END( Tick2 )*/

	if( m_pMasterLevel )
	{
		m_pMasterLevel->Update();
		auto camPos = m_pMasterLevel->GetCamPos();
		m_camera.SetPosition( camPos.x, camPos.y );
	}
	
	PROFILE_BEGIN( UpdateDirty2 )
	CScene2DManager::GetGlobalInst()->UpdateDirty();
	PROFILE_END( UpdateDirty2 )
	m_events.Trigger( eStageEvent_PostUpdate, NULL );
	m_tick.UpdateTime();

	CProfileMgr::Inst()->OnFrameMove();
}

void CStage::OnPostProcess( CPostProcessPass* pPass )
{
	/*CMyLevel* pLevel = CMyLevel::GetInst();
	if( m_pPlayer && pLevel )
	{
		int32 nShakeType = CPlayerData::Inst().nShakeType;

		float fHurt = Min( 1.0f, m_pPlayer->GetInvicibleTimeLeft() / 0.25f );
		CVector2 vecKnockback;
		float fKnockbackLen;
		if( m_pPlayer->IsHooked() )
		{
			vecKnockback = CVector2( 0, 0 );
			fKnockbackLen = 1;
		}
		else
		{
			vecKnockback = m_pPlayer->GetKnockback();
			fKnockbackLen = vecKnockback.Length();
		}

		CVector4 texOfs[3];
		CVector4 weightsBase[3][3] =
		{
			{
				{ 1, 1, 1, 1 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 },
			},
			{
				{ 0, 0, 0, 1 },
				{ 0, 0, 0, 0 },
				{ 1, 1, 1, 0 },
			},
			{
				{ 1, 0, 0, 1 },
				{ 0, 0, 1, 0 },
				{ 0, 1, 0, 0 },
			}
		};
		CVector4 weights1[3] =
		{
			{ 1.0f, 0, 0, 1 },
			{ 0.0f, 0, 0.25f, 0 },
			{ 0.0f, 0.25f, 0, 0 },
		};
		CVector4 weights2[3] =
		{
			{ 0.5f, 0.5f, 0.5f, 1 },
			{ 0.5f, 0.5f, 0.5f, 0 },
			{ 0.5f, 0.5f, 0.5f, 0 },
		};
		CVector4 weights[3];
		for( int i = 0; i < 3; i++ )
		{
			weights[i] = weightsBase[nShakeType][i] * ( 1 - fHurt ) * ( 1 - fKnockbackLen ) + weights1[i] * fHurt + weights2[i] * ( 1 - fHurt ) * fKnockbackLen;
		}

		CVector2 camShake = CVector2( cos( IRenderSystem::Inst()->GetTotalTime() * 1.3592987 * 60 ), cos( IRenderSystem::Inst()->GetTotalTime() * 1.4112051 * 60 ) )
			* ( Min( pLevel->GetShakeStrength() / 64.0f, 1.0f ) + fHurt );
		camShake = camShake + vecKnockback * 10;
		CVector2 ofs = camShake;

		CVector2 ofs1 = CVector2( ofs.y, -ofs.x );
		for( int i = 0; i < 3; i++ )
		{
			texOfs[i] = CVector4( ofs.x, ofs.y, ofs1.x, ofs1.y ) * i;
		}

		static CPostProcessDizzyEffect effect;
		effect.SetPriority( 1 );
		effect.SetTexOfs( texOfs );
		effect.SetWeights( weights );
		pPass->Register( &effect );
	}
*/
}