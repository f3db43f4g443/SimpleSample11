#include "stdafx.h"
#include "World.h"
#include "ResourceManager.h"
#include "MyLevel.h"

CWorld::CWorld()
	: m_pCurStage( NULL )
	, m_bUpdating( false )
	, m_nMainUISubStage( -1 )
{
	CStageDirector::Inst()->OnWorldCreated( this );
}

CWorld::~CWorld()
{
	CStageDirector::Inst()->OnWorldDestroyed( this );
}

void CWorld::CreatePlayer()
{
	if( m_pCurPlayer )
	{
		m_pCurPlayer->SetParentEntity( NULL );
		SetPlayer( NULL );
	}

	CPlayer* pPlayer = static_cast<CPlayer*>( CResourceManager::Inst()->CreateResource<CPrefab>( "player.pf" )->GetRoot()->CreateInstance() );
	SetPlayer( pPlayer );
	//auto pWeapon = SafeCast<CPlayerWeapon>( CResourceManager::Inst()->CreateResource<CPrefab>( "weapon.pf" )->GetRoot()->CreateInstance() );
	//pPlayer->AddItem( pWeapon );
}

void CWorld::EnterStage( const char* szStageName, SStageEnterContext& enterContext )
{
	if( m_bUpdating )
	{
		m_strEnterStage = szStageName;
		m_stageEnterContext = enterContext;
		return;
	}

	if( m_pCurStage )
	{
		//if( m_pCurStage->GetContext()->strName == szStageName )
			//return;

		Stop();
	}
	auto itr = m_mapStageContexts.find( szStageName );
	if( itr == m_mapStageContexts.end() )
	{
		auto& context = m_mapStageContexts[szStageName];
		context.strName = szStageName;
		context.bLight = true;
		//Load
		context.strSceneResName = context.strName;
	}

	StopSubStage( m_nMainUISubStage );
	m_nMainUISubStage = PlaySubStage( "ui.pf", CStageDirector::Inst()->GetSubStageView() );

	CreatePlayer();
	
	auto& context = m_mapStageContexts[szStageName];
	CStage* pStage = new CStage( this );
	m_pCurStage = pStage;
	pStage->Create( &context );
	enterContext.pViewport = CStageDirector::Inst()->OnPlayMainStage( pStage );
	pStage->Start( m_pCurPlayer, enterContext );
	CStageDirector::Inst()->AfterPlayMainStage();
}

void CWorld::Update()
{
	if( m_strEnterStage.length() )
	{
		EnterStage( m_strEnterStage.c_str(), m_stageEnterContext );
		m_strEnterStage = "";
	}

	m_bUpdating = true;

	if( CMyLevel::GetInst() )
		CMyLevel::GetInst()->UpdateBlocksMovement();
	if( m_pCurStage )
		m_pCurStage->Update();

	for( int i = 0; i < m_subStages.size(); i++ )
	{
		if( m_subStages[i].pStage )
			m_subStages[i].pStage->Update();
	}
	m_bUpdating = false;
}

void CWorld::Stop()
{
	for( int i = 0; i < m_subStages.size(); i++ )
		StopSubStage( i );
	if( m_pCurStage )
	{
		CStageDirector::Inst()->OnStopMainStage( m_pCurStage );
		m_pCurStage->Stop();
		delete m_pCurStage;
		m_pCurStage = NULL;
	}
}

uint32 CWorld::PlaySubStage( const char* szSubStageName, CUIViewport* pViewport )
{
	SSubStage* pSubStage = NULL;
	uint32 nSlot;
	for( int i = 0; i < m_subStages.size(); i++ )
	{
		if( !m_subStages[i].pStage )
		{
			nSlot = i;
			pSubStage = &m_subStages[i];
			break;
		}
	}
	if( !pSubStage )
	{
		m_subStages.resize( m_subStages.size() + 1 );
		nSlot = m_subStages.size() - 1;
		pSubStage = &m_subStages.back();
	}

	auto itr = m_mapStageContexts.find( szSubStageName );
	if( itr == m_mapStageContexts.end() )
	{
		auto& context = m_mapStageContexts[szSubStageName];
		context.strName = szSubStageName;
		//Load
		context.strSceneResName = context.strName;
	}

	CStage* pStage = new CStage( this );
	auto& context = m_mapStageContexts[szSubStageName];
	pStage->Create( &context );
	SStageEnterContext enterContext;
	enterContext.pViewport = pViewport;
	pStage->Start( NULL, enterContext );
	pSubStage->pStage = pStage;
	pSubStage->bPaused = false;
	return nSlot;
}

void CWorld::StopSubStage( uint32 nSlot )
{
	if( nSlot >= m_subStages.size() )
		return;
	auto& subStage = m_subStages[nSlot];
	if( !subStage.pStage )
		return;
	subStage.pStage->Stop();
	subStage.pStage = NULL;
}