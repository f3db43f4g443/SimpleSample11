#include "stdafx.h"
#include "World.h"

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
}

void CWorld::EnterStage( const char* szStageName, const SStageEnterContext& enterContext )
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

		m_pCurStage->Stop();
		delete m_pCurStage;
		m_pCurStage = NULL;
	}
	auto itr = m_mapStageContexts.find( szStageName );
	if( itr == m_mapStageContexts.end() )
	{
		auto& context = m_mapStageContexts[szStageName];
		context.strName = szStageName;
		//Load
		context.strSceneResName = context.strName;
	}
	
	auto& context = m_mapStageContexts[szStageName];
	CStage* pStage = new CStage( this );
	m_pCurStage = pStage;
	pStage->Create( &context );
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
	m_bUpdating = false;
}

void CWorld::Stop()
{
	if( m_pCurStage )
	{
		m_pCurStage->Stop();
		delete m_pCurStage;
		m_pCurStage = NULL;
	}
}