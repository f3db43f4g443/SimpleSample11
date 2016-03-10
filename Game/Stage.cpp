#include "stdafx.h"
#include "Stage.h"
#include "Render/Scene2DManager.h"
#include "Player.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"

CStage::CStage() : m_pPlayer( NULL ), m_bStarted( false )
{
	m_pEntityRoot = new CEntity;
	m_pFootprintMgr = new CFootprintMgr;
}

CStage::~CStage()
{
	Stop();
	m_pEntityRoot = NULL;
}

void CStage::Start()
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
}

void CStage::Stop()
{
	if( !m_bStarted )
		return;
	m_bStarted = false;
	auto pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->RemoveActiveCamera( &m_camera );
	pSceneMgr->RemoveFootprintMgr( m_pFootprintMgr );
	m_pEntityRoot->SetParentEntity( NULL );
	m_tickBeforeHitTest.Clear();
	m_tickAfterHitTest.Clear();
}

void CStage::AddEntity( CEntity* pEntity )
{
	pEntity->AddRef();
	pEntity->SetStage( this );
	if( pEntity->Get_HitProxy() )
		m_hitTestMgr.Add( pEntity );
	pEntity->OnAddedToStage();

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

	pEntity->OnRemovedFromStage();
	pEntity->SetTraverseIndex( -1 );
	if( pEntity->Get_HitProxy() )
		m_hitTestMgr.Remove( pEntity );
	pEntity->SetStage( NULL );
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