#pragma once
#include "Stage.h"
#include "BasicElems.h"
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
	CVector2 displayOfs;

	TArray<CString> arrShowSnapShot;
	TArray<CVector2> arrGrids;
	TArray<CVector2> arrNxtStages;
	TArray<CVector2> arrConsoles;
	TArray<SMapIconData> arrIconData;
};

struct SRegionData
{
	SRegionData( const SClassCreateContext& context ) {}
	CString strName;
	TArray<SLevelData> arrLevelData;
	TResourceRef<CPrefab> pBlueprint;
	TResourceRef<CPrefab> pMap;
};

struct SWorldCfg
{
	SWorldCfg( const SClassCreateContext& context ) {}
	TArray<SRegionData> arrRegionData;

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
	TVector2<int32> GetLevelIndex( const char* szLevel );
private:
	SWorldCfg* m_pWorldCfg;

	map<string, TVector2<int32> > m_mapLevels;
};

class CWorld
{
public:
	CWorld();
	~CWorld();

	CPlayer* GetPlayer() { return m_pCurPlayer; }
	void SetPlayer( CPlayer* pPlayer ) { m_pCurPlayer = pPlayer; }
	void CreatePlayer();
	void Start( const char* szName, int32 nParam );

	SWorldCfg& GetWorldCfg() { return m_pWorldCfgFile->GetData(); }
	TVector2<int32> GetWorldCfgLevelIndex( const char* szLevel ) { return m_pWorldCfgFile->GetLevelIndex( szLevel ); }
	void EnterStage( SStageEnterContext& enterContext );
	void EnterStage( CPrefab* pStage, const TVector2<int32>& pos, int8 nDir );
	void RestartStage();
	void Update();
	void Stop();
	CStage* GetCurStage() { return m_pCurStage; }
	
	SSubStage* GetSubStage( uint32 nSlot ) { return nSlot < m_subStages.size()? &m_subStages[nSlot] : NULL; }
	uint32 PlaySubStage( const char* szSubStageName, CUIViewport* pViewport );
	void StopSubStage( uint32 nSlot );
private:
	CReference<CWorldCfgFile> m_pWorldCfgFile;
	CStage* m_pCurStage;
	CReference<CPlayer> m_pCurPlayer;
	map<string, SStageContext> m_mapStageContexts;
	vector<SSubStage> m_subStages;

	float m_fPercent;

	int32 m_nMainUISubStage;
	bool m_bUpdating;
	bool m_bChangeStage;
	SStageEnterContext m_stageEnterContext;
	CBufFile m_playerData;
	int32 m_nRestartTime;
};