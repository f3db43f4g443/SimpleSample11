#include "stdafx.h"
#include "Stage.h"
#include "Render/Scene2DManager.h"
#include "Player.h"
#include "GUI/MainUI.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"

CStage::CStage( CWorld* pWorld ) : m_pWorld( pWorld ), m_pContext( NULL ), m_bStarted( false ), m_pPlayer( NULL ), m_nGUIOption( 0xffffffff )
{
	memset( m_pCurPlayerActions, 0, sizeof( m_pCurPlayerActions ) );
	m_pEntityRoot = new CEntity;
	m_pFootprintMgr = new CFootprintMgr;
}

CStage::~CStage()
{
	Stop();
	m_pEntityRoot = NULL;
}

#include "Entities/Slime.h"
#include "Entities/SlimeCore.h"
#include "Entities/SlimeBlister.h"
#include "Entities/SlimeGenerator2.h"

void CStage::Create( SStageContext* pContext )
{
	m_pContext = pContext;
	for( auto& item : m_pContext->mapDependentRes )
	{
		item.second = CResourceManager::Inst()->CreateResource( item.first.c_str() );
	}
	CPrefab* pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_pContext->strSceneResName.c_str() );
	if( pPrefab )
	{
		CReference<CRenderObject2D> pRenderObject = pPrefab->GetRoot()->CreateInstance();
		if( pRenderObject )
		{
			CEntity* pEntity = dynamic_cast<CEntity*>( pRenderObject.GetPtr() );
			if( pEntity )
				pEntity->SetParentEntity( m_pEntityRoot );
		}
	}

	//test
	//{
	//	vector<char> content;
	//	GetFileContent( content, "materials/background.xml", true );
	//	TiXmlDocument doc;
	//	doc.LoadFromBuffer( &content[0] );
	//	CDefaultDrawable2D* pDrawable = new CDefaultDrawable2D;
	//	pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
	//	CDefaultDrawable2D* pDrawable1 = new CDefaultDrawable2D;
	//	pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
	//	CImage2D* pImage = new CImage2D( pDrawable, pDrawable1, CRectangle( -512, -512, 1024, 1024 ), CRectangle( 0, 0, 1, 1 ) );
	//	CEntity* pEntity = new CEntity;
	//	pEntity->SetRenderObject( pImage );
	//	pEntity->SetParentEntity( GetRoot() );
	//	
	//	//CDirectionalLightObject* pDirectionalLight = new CDirectionalLightObject( CVector2( -0.6f, -0.8f ), CVector3( 0.15f, 0.15f, 0.15f ), 8, 256.0f );
	//	//pEntity->AddChild( pDirectionalLight );

	//	CEntity* pEntity1 = new CEntity;
	//	pEntity1->SetParentEntity( pEntity );
	//	pEntity1->AddRect( CRectangle( -800, -800, 1600, 340 ) );
	//	pEntity1 = new CEntity;
	//	pEntity1->SetParentEntity( pEntity );
	//	pEntity1->AddRect( CRectangle( -800, -800, 340, 1600 ) );
	//	pEntity1 = new CEntity;
	//	pEntity1->SetParentEntity( pEntity );
	//	pEntity1->AddRect( CRectangle( 800, 800, -340, -1600 ) );
	//	pEntity1 = new CEntity;
	//	pEntity1->SetParentEntity( pEntity );
	//	pEntity1->AddRect( CRectangle( 800, 800, -1600, -340 ) );

	//	CSlimeGround* pSlimeGround = new CSlimeGround();
	//	pSlimeGround->SetParentEntity( pEntity );
	//	CRenderObject2D* pRenderObject = new CRenderObject2D;
	//	pSlimeGround->AddChild( pRenderObject );
	//	SetFootprintRoot( pRenderObject );
	//	//CSlimeBlister* pSlimeBlister = new CSlimeBlister;
	//	//pSlimeBlister->SetParentEntity( pSlimeGround );
	//	CSlimeGenerator2* pSlimeGenerator = new CSlimeGenerator2;
	//	pSlimeGenerator->SetParentEntity( pSlimeGround );
	//}
}

void CStage::Start( CPlayer* pPlayer, const SStageEnterContext& context )
{
	if( m_bStarted )
		return;

	auto pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->GetRoot()->AddChild( m_pEntityRoot );
	AddEntity( m_pEntityRoot );

	m_camera.SetViewport( 0, 0, 800, 600 );
	m_camera.SetPosition( 0, 0 );
	m_camera.SetSize( 800, 600 );
	pSceneMgr->AddActiveCamera( &m_camera, m_pEntityRoot );
	pSceneMgr->AddFootprintMgr( m_pFootprintMgr );

	if( m_pFootprintMgr->GetFootprintRoot() )
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

	pPlayer->SetParentEntity( m_pEntityRoot );
	CEntity* pEntity = GetStartPoint( context.strStartPointName.c_str() );
	if( !pEntity )
	{
		if( m_mapStartPoints.size() )
			pEntity = m_mapStartPoints.begin()->second;
	}
	if( pEntity )
	{
		CScene2DManager::GetGlobalInst()->UpdateDirty();
		pPlayer->SetPosition( pEntity->globalTransform.GetPosition() );
	}
	else
		pPlayer->SetPosition( CVector2( 0, 0 ) );
	SetGUIOption( m_nGUIOption );
	m_events.Trigger( eStageEvent_Start, NULL );
}

void CStage::Stop()
{
	if( !m_bStarted )
		return;
	m_events.Trigger( eStageEvent_Stop, NULL );
	m_bStarted = false;
	m_mapStartPoints.clear();
	auto pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->RemoveActiveCamera( &m_camera );
	pSceneMgr->RemoveFootprintMgr( m_pFootprintMgr );
	RemoveEntity( m_pEntityRoot );
	m_tickBeforeHitTest.Clear();
	m_tickAfterHitTest.Clear();

	if( m_pContext )
	{
		for( auto& item : m_pContext->mapDependentRes )
		{
			item.second = NULL;
		}
	}
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

	CPlayer* pPlayer = dynamic_cast<CPlayer*>( pEntity );
	if( pPlayer )
		m_pPlayer = pPlayer;

	for( CEntity* pChild = pEntity->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		AddEntity( pChild );
	}
}

void CStage::RemoveEntity( CEntity* pEntity )
{
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
		CEntity* pEntity1 = dynamic_cast<CEntity*>( vecResults[i] );
		if( !pEntity1 )
			continue;
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
		CEntity* pEntity = dynamic_cast<CEntity*>( vecResults[i] );
		if( !pEntity )
			continue;
		result.push_back( pEntity );
	}
}

CEntity* CStage::Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType, SRaycastResult* pResult )
{
	vector<SRaycastResult> result;
	m_hitTestMgr.Raycast( begin, end, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( result[i].pHitProxy );
		if( !pEntity )
			continue;
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
		CEntity* pEntity = dynamic_cast<CEntity*>( tempResult[i].pHitProxy );
		if( !pEntity )
			continue;
		if( pResult )
			pResult->push_back( tempResult[i] );
		result.push_back( pEntity );
	}
}

void CStage::Update()
{
	if( m_pPlayer )
	{
		m_fGlobalElapsedTime = m_pPlayer->GetHorrorReflexTimeScale() * GetElapsedTimePerTick();
		m_fFlyingObjectElapsedTime = m_pPlayer->GetHorrorReflexBulletTimeScale() * GetElapsedTimePerTick();
	}
	else
	{
		m_fGlobalElapsedTime = m_fFlyingObjectElapsedTime = GetElapsedTimePerTick();
	}
	m_tickBeforeHitTest.UpdateTime();
	CScene2DManager::GetGlobalInst()->UpdateDirty();

	m_pEntityRoot->BeforeHitTest( 0 );
	m_hitTestMgr.Update();
	m_tickAfterHitTest.UpdateTime();
	
	CScene2DManager::GetGlobalInst()->UpdateDirty();
	if( m_pPlayer )
	{
		const CMatrix2D& globalTransform = m_pPlayer->GetGlobalTransform();
		m_camera.SetPosition( globalTransform.m02, globalTransform.m12 );
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

void CStage::SetGUIOption( uint32 nOption )
{
	m_nGUIOption = nOption;
	if( !m_pPlayer )
		return;

	bool bCrosshair = nOption & eGUIOption_Crosshair;
	m_pPlayer->GetCrosshair()->bVisible = bCrosshair;

	bool bHpBar = nOption & eGUIOption_HpBar;
	CMainUI::Inst()->SetHpBarVisible( bHpBar );
}