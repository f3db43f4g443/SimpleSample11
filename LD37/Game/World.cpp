#include "stdafx.h"
#include "World.h"
#include "ResourceManager.h"
#include "MyLevel.h"

CWorld::CWorld()
	: m_pCurStage( NULL )
	, m_bUpdating( false )
{
	//test
	/*SStageContext& createContext = m_mapStageContexts["test"];
	createContext.strName = "test";
	createContext.mapDependentRes["growl1.wav"] = NULL;
	createContext.mapDependentRes["growl2.wav"] = NULL;
	createContext.mapDependentRes["growl3.wav"] = NULL;
	createContext.mapDependentRes["atk1.wav"] = NULL;
	createContext.mapDependentRes["atk2.wav"] = NULL;
	createContext.mapDependentRes["atk3.wav"] = NULL;
	createContext.mapDependentRes["death1.wav"] = NULL;
	createContext.mapDependentRes["death2.wav"] = NULL;
	createContext.mapDependentRes["death3.wav"] = NULL;
	createContext.mapDependentRes["a.wav"] = NULL;
	createContext.mapDependentRes["b1.wav"] = NULL;
	createContext.mapDependentRes["b2.wav"] = NULL;
	createContext.mapDependentRes["b3.wav"] = NULL;
	createContext.mapDependentRes["c.wav"] = NULL;*/

	CStageDirector::Inst()->OnWorldCreated( this );
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
		//Load
		context.strSceneResName = context.strName;
	}

	StopSubStage( 0 );
	PlaySubStage( "ui.pf", CStageDirector::Inst()->GetSubStageView() );
	
	auto& context = m_mapStageContexts[szStageName];
	CStage* pStage = new CStage( this );
	m_pCurStage = pStage;
	pStage->Create( &context );
	enterContext.pViewport = CStageDirector::Inst()->OnPlayMainStage( pStage );
	pStage->Start( m_pCurPlayer, enterContext );
}

void CWorld::Update()
{
	if( m_strEnterStage.length() )
	{
		EnterStage( m_strEnterStage.c_str(), m_stageEnterContext );
		m_strEnterStage = "";
	}

	m_bUpdating = true;
	if( m_pCurStage )
		m_pCurStage->Update();

	if( CMyLevel::GetInst() )
		CMyLevel::GetInst()->UpdateBlocksMovement();

	for( int i = 0; i < m_subStages.size(); i++ )
	{
		if( m_subStages[i].pStage )
			m_subStages[i].pStage->Update();
	}
	m_bUpdating = false;
}

void CWorld::Stop()
{
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
	if( nSlot <= m_subStages.size() )
		return;
	auto& subStage = m_subStages[nSlot];
	if( !subStage.pStage )
		return;
	subStage.pStage->Stop();
	subStage.pStage = NULL;
}