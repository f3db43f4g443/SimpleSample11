#include "stdafx.h"
#include "World.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"

SLevelData* SWorldCfg::GetLevelData( const char* szLevel )
{
	if( !m_mapLevelData.size() )
	{
		for( int i = 0; i < arrLevelData.Size(); i++ )
		{
			m_mapLevelData[arrLevelData[i].pLevel.c_str()] = &arrLevelData[i];
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
	for( int i = 0; i < m_pWorldCfg->arrLevelData.Size(); i++ )
	{
		m_mapLevels[m_pWorldCfg->arrLevelData[i].pLevel.c_str()] = i;
	}
	m_bCreated = true;
}

void CWorldCfgFile::Save( CBufFile & buf )
{
	CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>()->PackData( (uint8*)m_pWorldCfg, buf, true );
}

int32 CWorldCfgFile::GetLevelIndex( const char* szLevel )
{
	auto itr = m_mapLevels.find( szLevel );
	if( itr == m_mapLevels.end() )
		return -1;
	return itr->second;
}

void SWorldSaveData::Load( IBufReader& buf )
{
	int32 nVersion = 0;
	buf.Read( nVersion );
	buf.Read( nDay );
	buf.Read( nLastPlayerExp );
	buf.Read( nPlayerExp );
	int32 n;
	buf.Read( n );
	for( int i = 0; i < n; i++ )
	{
		string str;
		buf.Read( str );
		int32 nState = buf.Read<int32>();
		mapBugState[str] = nState;
	}
}

void SWorldSaveData::Save( CBufFile & buf )
{
	int32 nVersion = 0;
	buf.Write( nVersion );
	buf.Write( nDay );
	buf.Write( nLastPlayerExp );
	buf.Write( nPlayerExp );
	int32 n = mapBugState.size();
	buf.Write( n );
	for( auto& item : mapBugState )
	{
		buf.Write( item.first );
		buf.Write( item.second );
	}
}

int32 SWorldSaveData::GetBugState( const char* szLevelName, const char* szName )
{
	string str = szLevelName;
	str += ':';
	str += szName;
	auto itr = mapBugState.find( str );
	if( itr == mapBugState.end() )
		return 0;
	return itr->second;
}

void SWorldSaveData::DetectBug( const char* szLevelName, const char* szName )
{
	string str = szLevelName;
	str += ':';
	str += szName;
	mapBugState[str] = Max( mapBugState[str], 1 );
}

void SWorldSaveData::FixBug( const char* szLevelName, const char* szName, int32 nExp )
{
	string str = szLevelName;
	str += ':';
	str += szName;
	mapBugState[str] = 2;
	nPlayerExp += nExp;
}

CWorld::CWorld()
	: m_pCurStage( NULL )
	, m_bUpdating( false )
	, m_nMainUISubStage( -1 )
	, m_fPercent( 0.5f )
	, m_nRestartTime( 0 )
{
	auto& context = m_mapStageContexts["1.pf"];
	context.strName = "1.pf";
	context.nLightType = 0;
	context.strSceneResName = context.strName;

	m_pWorldCfgFile = CResourceManager::Inst()->CreateResource<CWorldCfgFile>( "levels/world.w" );
	CStageDirector::Inst()->OnWorldCreated( this );
	LoadWorldData();
}

CWorld::~CWorld()
{
	CStageDirector::Inst()->OnWorldDestroyed( this );
}

void CWorld::LoadWorldData()
{
	vector<char> content;
	GetFileContent( content, "save/a", false );
	if( content.size() )
	{
		CBufReader buf( &content[0], content.size() );
		m_worldData.Load( buf );
	}
}

void CWorld::SaveWorldData()
{
	CBufFile buf;
	m_worldData.Save( buf );
	SaveFile( "save/a", buf.GetBuffer(), buf.GetBufLen() );
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

void CWorld::EnterStage( const char* szStageName, SStageEnterContext& enterContext )
{
	if( m_bUpdating )
	{
		m_strEnterStage = szStageName;
		m_stageEnterContext = enterContext;
		return;
	}

	m_strCurStage = szStageName;
	m_stageEnterContext = enterContext;
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

	//StopSubStage( m_nMainUISubStage );
	//m_nMainUISubStage = PlaySubStage( "ui.pf", CStageDirector::Inst()->GetMainStage() );

	//CreatePlayer();
	
	auto& context = m_mapStageContexts[szStageName];
	CStage* pStage = new CStage( this );
	m_pCurStage = pStage;
	pStage->Create( &context );
	enterContext.pViewport = CStageDirector::Inst()->GetMainStage();
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

	if( m_pCurStage )
		m_pCurStage->Update();

	for( int i = 0; i < m_subStages.size(); i++ )
	{
		if( m_subStages[i].pStage )
			m_subStages[i].pStage->Update();
	}
	m_bUpdating = false;
	if( m_pCurStage && !m_pCurStage->GetPlayer() )
	{
		m_nRestartTime++;
		if( m_nRestartTime == 180 )
			m_strEnterStage = m_strCurStage;
	}
	else
		m_nRestartTime = 0;
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
		REGISTER_MEMBER( arrOverlapLevel )
		REGISTER_MEMBER( displayOfs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SWorldCfg )
		REGISTER_MEMBER( arrLevelData )
	REGISTER_CLASS_END()
}