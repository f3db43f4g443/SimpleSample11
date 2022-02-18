#pragma once
#include "Stage.h"
#include "Player.h"
#include "GUI/StageDirector.h"

struct SSubStage
{
	SSubStage() : pStage( NULL ), pViewport( NULL ), bPaused( false ) {}
	CStage* pStage;
	CUIViewport* pViewport;
	bool bPaused;
};

struct SLevelData
{
	SLevelData( const SClassCreateContext& context ) {}
	TResourceRef<CPrefab> pLevel;
	TArray<CString> arrOverlapLevel;
	CVector2 displayOfs;
};

struct SWorldCfg
{
	SWorldCfg( const SClassCreateContext& context ) {}
	TArray<SLevelData> arrLevelData;

	SLevelData* GetLevelData( const char* szLevel );
	CVector2 GetLevelDisplayOfs( const char* szLevel ) { return GetLevelData( szLevel )->displayOfs; }
	map<string, SLevelData*> m_mapLevelData;
};


enum EWorldCfgFileVersion
{
	EWorldCfgFileVersion_Cur = 0,
};

class CWorldCfgFile : public CResource
{
	friend class CWorldCfgEditor;
public:
	enum EType
	{
		eResType = eGameResType_WorldCfgFile,
	};

	CWorldCfgFile( const char* name, int32 type ) : CResource( name, type ), m_pWorldCfg( NULL ) {}
	~CWorldCfgFile() { if( m_pWorldCfg ) delete m_pWorldCfg; }
	void Create();
	void Save( CBufFile& buf );

	SWorldCfg& GetData() { return *m_pWorldCfg; }
	int32 GetLevelIndex( const char* szLevel );
private:
	SWorldCfg* m_pWorldCfg;

	map<string, int32 > m_mapLevels;
};

struct SWorldSaveData
{
	SWorldSaveData() : nDay( 0 ), nLastPlayerExp( 0 ), nPlayerExp( 0 ) {}
	int32 nDay;
	int32 nLastPlayerExp;
	int32 nPlayerExp;
	map<string, int32> mapBugState;

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	void NewDay() { nDay++; nLastPlayerExp = nPlayerExp; }
	int32 GetBugState( const char* szLevelName, const char* szName );
	void DetectBug( const char* szLevelName, const char* szName );
	void FixBug( const char* szLevelName, const char* szName, int32 nExp );
};

class CWorld
{
public:
	CWorld();
	~CWorld();

	SWorldSaveData& GetWorldData() { return m_worldData; }
	void LoadWorldData();
	void SaveWorldData();
	CPlayerCross* GetPlayer() { return m_pCurPlayer; }
	void SetPlayer( CPlayerCross* pPlayer ) { m_pCurPlayer = pPlayer; }
	void CreatePlayer();

	void EnterStage( const char* szStageName, SStageEnterContext& enterContext );
	void Update();
	void Stop();
	float GetPercent() { return m_fPercent; }
	void ChangePercent( float f ) { m_fPercent = Max( 0.0f, Min( 1.0f, m_fPercent + f ) ); }
	SWorldCfg& GetWorldCfg() { return m_pWorldCfgFile->GetData(); }
	int32 GetWorldCfgLevelIndex( const char* szLevel ) { return m_pWorldCfgFile->GetLevelIndex( szLevel ); }
	
	SSubStage* GetSubStage( uint32 nSlot ) { return nSlot < m_subStages.size()? &m_subStages[nSlot] : NULL; }
	uint32 PlaySubStage( const char* szSubStageName, CUIViewport* pViewport );
	void StopSubStage( uint32 nSlot );
private:
	CReference<CWorldCfgFile> m_pWorldCfgFile;
	SWorldSaveData m_worldData;
	CStage* m_pCurStage;
	CReference<CPlayerCross> m_pCurPlayer;
	map<string, SStageContext> m_mapStageContexts;
	vector<SSubStage> m_subStages;

	float m_fPercent;
	int32 m_nMainUISubStage;
	bool m_bUpdating;
	string m_strEnterStage;
	SStageEnterContext m_stageEnterContext;
	string m_strCurStage;
	int32 m_nRestartTime;
};