#include "stdafx.h"
#include "World.h"
#include "ResourceManager.h"
#include "MyLevel.h"
#include "GlobalCfg.h"
#include "FileUtil.h"


SLevelData* SWorldCfg::GetLevelData( const char * szLevel )
{
	if( !m_mapLevelData.size() )
	{
		for( int i = 0; i < arrRegionData.Size(); i++ )
		{
			auto& arrLevelData = arrRegionData[i].arrLevelData;
			for( int j = 0; j < arrLevelData.Size(); j++ )
			{
				m_mapLevelData[arrLevelData[j].pLevel.c_str()] = &arrLevelData[j];
			}
		}
	}
	return m_mapLevelData[szLevel];
}

void CWorldCfgFile::Create()
{
	vector<char> fileContent;
	GetFileContent( fileContent, GetName(), false );
	if( !fileContent.size() )
		return;
	CBufReader bufReader( &fileContent[0], fileContent.size() );
	m_pWorldCfg = (SWorldCfg*)CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>()->NewObjFromData( bufReader, true, NULL );
	for( int i = 0; i < m_pWorldCfg->arrRegionData.Size(); i++ )
	{
		auto& arrLevelData = m_pWorldCfg->arrRegionData[i].arrLevelData;
		for( int j = 0; j < arrLevelData.Size(); j++ )
		{
			m_mapLevels[arrLevelData[j].pLevel.c_str()] = TVector2<int32>( i, j );
		}
	}
	m_bCreated = true;
}

void CWorldCfgFile::Save( CBufFile& buf )
{
	CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>()->PackData( (uint8*)m_pWorldCfg, buf, true );
}

TVector2<int32> CWorldCfgFile::GetLevelIndex( const char* szLevel )
{
	auto itr = m_mapLevels.find( szLevel );
	if( itr == m_mapLevels.end() )
		return TVector2<int32>( -1, -1 );
	return itr->second;
}

CWorld::CWorld()
	: m_pCurStage( NULL )
	, m_bUpdating( false )
	, m_bChangeStage( false )
	, m_nMainUISubStage( -1 )
	, m_fPercent( 0 )
	, m_nRestartTime( 0 )
{
	m_pWorldCfgFile = CResourceManager::Inst()->CreateResource<CWorldCfgFile>( "stages/world.w" );
	auto& arrReg = GetWorldCfg().arrRegionData;
	for( int i = 0; i < arrReg.Size(); i++ )
		arrReg[i].pMap = CResourceManager::Inst()->CreateResource<CPrefab>( arrReg[i].pMap );
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
}

void CWorld::Start( const char* szName, int32 nParam )
{
	SStageEnterContext context;
	context.nParam = nParam;

	const char* szSubStageName = szName;
	auto itr = m_mapStageContexts.find( szSubStageName );
	if( itr == m_mapStageContexts.end() )
	{
		auto& context = m_mapStageContexts[szSubStageName];
		context.strName = szSubStageName;
		context.bLight = false;
		//Load
		context.strSceneResName = context.strName;
	}
	context.pTarget = &m_mapStageContexts[szSubStageName];

	EnterStage( context );
}

void CWorld::EnterStage( SStageEnterContext& enterContext )
{
	if( m_bUpdating )
	{
		m_bChangeStage = true;
		m_stageEnterContext = enterContext;
		return;
	}
	m_stageEnterContext = enterContext;
	if( m_pCurStage )
	{
		Stop();
	}
	if( !m_pCurPlayer )
		CreatePlayer();
	
	auto& context = *enterContext.pTarget;
	CStage* pStage = new CStage( this );
	m_pCurStage = pStage;
	pStage->Create( &context );
	enterContext.pViewport = CStageDirector::Inst()->GetMainStage();
	pStage->Start( m_pCurPlayer, enterContext );
	CStageDirector::Inst()->AfterPlayMainStage();
}

void CWorld::EnterStage( CPrefab* pStage, const TVector2<int32>& pos, int8 nDir )
{
	SStageEnterContext context;

	const char* szSubStageName = pStage->GetName();
	auto itr = m_mapStageContexts.find( szSubStageName );
	if( itr == m_mapStageContexts.end() )
	{
		auto& context = m_mapStageContexts[szSubStageName];
		context.strName = szSubStageName;
		context.bLight = false;
		//Load
		context.strSceneResName = context.strName;
	}
	context.pTarget = &m_mapStageContexts[szSubStageName];

	EnterStage( context );
}

void CWorld::RestartStage()
{
	EnterStage( m_stageEnterContext );
}

void CWorld::Update()
{
	if( m_bChangeStage )
	{
		EnterStage( m_stageEnterContext );
		m_bChangeStage = false;
	}

	m_bUpdating = true;

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
		context.bLight = false;
		//Load
		context.strSceneResName = context.strName;
	}

	CStage* pStage = new CStage( this );
	auto& context = m_mapStageContexts[szSubStageName];
	pStage->Create( &context );
	SStageEnterContext enterContext;
	pStage->Start( NULL, enterContext );
	pSubStage->pStage = pStage;
	pSubStage->bPaused = false;
	pViewport->SetGUICamera( pStage->GetRoot(), &pStage->GetCamera() );
	pSubStage->pViewport = pViewport;
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
	subStage.pViewport->SetGUICamera( NULL, NULL );
	subStage.pViewport = NULL;
}

void RegisterGameClasses_World()
{
	REGISTER_CLASS_BEGIN( SLevelData )
		REGISTER_MEMBER( pLevel )
		REGISTER_MEMBER( displayOfs )
		REGISTER_MEMBER( arrShowSnapShot )
		REGISTER_MEMBER( arrGrids )
		REGISTER_MEMBER( arrNxtStages )
		REGISTER_MEMBER( arrConsoles )
		REGISTER_MEMBER( arrIconData )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SRegionData )
		REGISTER_MEMBER( strName )
		REGISTER_MEMBER( arrLevelData )
		REGISTER_MEMBER( pBlueprint )
		REGISTER_MEMBER( pMap )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SWorldCfg )
		REGISTER_MEMBER( arrRegionData )
	REGISTER_CLASS_END()
}