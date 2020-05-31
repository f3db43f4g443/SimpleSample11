#pragma once
#include "Common/StringUtil.h"
#include "Common/PriorityQueue.h"
#include "Render/DrawableGroup.h"
#include "BasicElems.h"

#define LEVEL_GRID_SIZE_X 24.0f
#define LEVEL_GRID_SIZE_Y 32.0f
#define LEVEL_GRID_SIZE CVector2( LEVEL_GRID_SIZE_X, LEVEL_GRID_SIZE_Y )

struct SLevelGridData
{
	SLevelGridData() { bBlocked = false; nNextStage = 0; }
	SLevelGridData( const SClassCreateContext& context ) {}
	bool bBlocked;
	int32 nNextStage;
};

struct SLevelNextStageData
{
	SLevelNextStageData( const SClassCreateContext& context ) {}
	TResourceRef<CPrefab> pNxtStage;
	int32 nOfsX, nOfsY;
	CString strKeyOrig;
	CString strKeyRedirect;
};

class CLevelIndicatorLayer : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CLevelIndicatorLayer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelIndicatorLayer ); }

	virtual void OnAddedToStage() override;
	void Update( class CMyLevel* pLevel );
	virtual void Render( CRenderContext2D& context ) override;
private:
	CElement2D& AddElem( int32 x, int32 y, const CVector2& ofs, void* pParams );
	vector<CElement2D> m_elems;
	vector<CVector4> m_vecBlockedParams;
	uint32 m_nTick;
	vector<CVector4> m_vecNxtStageParam[2];
	vector<CVector2> m_vecNxtStageOfs[2];
};

class CMyLevel;
class CLevelScript : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CLevelScript( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelScript ); }

	virtual void OnInit( CMyLevel* pLevel ) {}
	virtual void OnDestroy( CMyLevel* pLevel ) {}
	virtual void OnUpdate( CMyLevel* pLevel ) {}
	virtual void OnUpdate1( CMyLevel* pLevel ) {}
	virtual void OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir ) {}
	virtual void OnPlayerAction( int32 nMatchLen, int8 nType ) {}
private:
};

class CMyLevel : public CEntity
{
	friend void RegisterGameClasses_Level();
	friend class CLevelEdit;
	friend class CWorldCfgEditor;
	friend class CMasterLevel;
public:
	CMyLevel( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMyLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	struct SGrid
	{
		SGrid() : bCanEnter( true ), nNextStage( 0 ), nHitEft( 0 ), nMissEft( 0 ), nBlockEft( 0 ), pMounts( NULL ) {}
		bool bCanEnter;
		int8 nHitEft;
		int8 nMissEft;
		int8 nBlockEft;
		int32 nNextStage;
		TVector2<int32> blockOfs;
		CReference<CPawn> pPawn;
		LINK_LIST_REF_HEAD( pMounts, CPlayerMount, Mount )
	};
	TVector2<int32> GetSize() const { return TVector2<int32>( m_nWidth, m_nHeight ); }
	SGrid* GetGrid( const TVector2<int32>& p ) { return p.x >= 0 && p.y >= 0 && p.x < m_nWidth && p.y < m_nHeight ?
		&m_vecGrid[p.x + p.y * m_nWidth] : NULL; }
	const SLevelGridData* GetGridData( const TVector2<int32>& p ) const { return p.x >= 0 && p.y >= 0 && p.x < m_nWidth && p.y < m_nHeight ?
		&m_arrGridData[p.x + p.y * m_nWidth] : NULL; }
	const CVector2& GetCamPos() { return m_camPos; }
	CPlayer* GetPlayer() { return m_pPlayer; }
	bool IsBegin() { return m_bBegin; }
	bool IsComplete() { return m_bComplete; }

	void Begin();
	void End();
	void Fail() { m_bFailed = true; }
	CPawn* SpawnPawn( int32 n, int32 x, int32 y, int8 nDir, CPawn* pCreator = NULL, int32 nForm = 0 );
	bool AddPawn( CPawn* pPawn, const TVector2<int32>& pos, int8 nDir, CPawn* pCreator = NULL, int32 nForm = 0 );
	void RemovePawn( CPawn* pPawn );
	bool PawnMoveTo( CPawn* pPawn, const TVector2<int32>& ofs );
	void PawnMoveEnd( CPawn* pPawn );
	void PawnMoveBreak( CPawn* pPawn );
	void PawnDeath( CPawn* pPawn );
	bool PawnTransform( CPawn* pPawn, int32 nForm, const TVector2<int32>& ofs );
	CPickUp* FindPickUp( const TVector2<int32>& p, int32 w, int32 h );
	CPlayerMount* FindMount();
	CPawn* FindUseablePawn( const TVector2<int32>& p, int8 nDir, int32 w, int32 h );
	CPawn* GetPawnByName( const char* szName );
	void OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir );
	void OnPlayerAction( int32 nMatchLen, int8 nType );
	TVector2<int32> SimpleFindPath( const TVector2<int32>& begin, const TVector2<int32>& end, int32 nCheckFlag, vector<TVector2<int32> >* pVecPath = NULL );

	void BlockStage();
	void BlockExit( int32 n );
	bool IsExitBlocked( int32 n );
	bool IsGridBlockedExit( SGrid* pGrid, bool bIgnoreComplete = false );
	int32 GetNextLevelCount() { return m_arrNextStage.Size(); }
	int32 FindNextLevelIndex( const char* szLevelName );
	void Redirect( int32 n, int32 n1 );

	void BeginScenario();
	void EndScenario();
	bool IsScenario() { return m_bScenario; }
	void GetAllUseableGrid( vector<TVector2<int32> >& result );

	void Init();
	void Update();

	void RegisterBegin( CTrigger* pTrigger );
	void RegisterUpdate( CTrigger* pTrigger ) { m_trigger.Register( 0, pTrigger ); }
	void RegisterUpdate1( CTrigger* pTrigger ) { m_trigger.Register( 1, pTrigger ); }
private:
	void HandlePawnMounts( CPawn* pPawn, bool bRemove );
	void FlushSpawn();
	void InitScripts();
	int32 m_nWidth, m_nHeight;
	CReference<CEntity> m_pPawnRoot;
	CVector2 m_camPos;
	TArray<SLevelGridData> m_arrGridData;
	TArray<SLevelNextStageData> m_arrNextStage;
	TArray<TResourceRef<CPrefab> > m_arrSpawnPrefab;
	TResourceRef<CDrawableGroup> m_pTileDrawable;
	CString m_strInitScript;
	CString m_strBeginScript;
	CString m_strDestroyScript;

	bool m_bBegin;
	bool m_bScenario;
	bool m_bComplete;
	bool m_bFailed;
	bool m_bFullyEntered;
	bool m_bBlocked;
	vector<CReference<CLevelScript> > m_vecScripts;
	vector<SGrid> m_vecGrid;
	TClassTrigger<CMyLevel> m_onTick;
	CReference<CPlayer> m_pPlayer;
	CReference<CRenderObject2D> m_pTip;
	CReference<CLevelIndicatorLayer> m_pIndicatorLayer;
	struct SExitState
	{
		SExitState() : nRedirect( -1 ), bBlocked( false ) {}
		int32 nRedirect;
		bool bBlocked;
	};
	vector<SExitState> m_vecExitState;

	CEventTrigger<1> m_beginTrigger;
	CEventTrigger<2> m_trigger;
	struct
	{
		LINK_LIST_REF_HEAD( m_pPawns, CPawn, Pawn );
	} m_spawningPawns;
	vector<CReference<CPawnHit> > m_vecPawnHits;
	LINK_LIST_REF_HEAD( m_pPawns, CPawn, Pawn );
};

class CCutScene : public CEntity
{
	friend void RegisterGameClasses_Level();
	friend class CMasterLevel;
public:
	CCutScene( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CCutScene ); }
	void Begin();
	void End() {}
private:
	CString m_strScript;
	TResourceRef<CPrefab> m_pNextLevelPrefab;
	int32 m_nPlayerEnterX, m_nPlayerEnterY;
	int8 m_nPlayerEnterDir;
};

class CMainUI : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CMainUI( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMainUI ); }
	void OnAddedToStage() override;
	void Reset( const CVector2& inputOrig );
	void RefreshPlayerInput( vector<int8>& vecInput, int32 nMatchLen = -1, int8 nType = 0 );
	void InsertDefaultFinishAction();
	void OnPlayerAction( vector<int8>& vecInput, int32 nMatchLen, int8 nType );

	void BeginScenario();
	void EndScenario();
	bool IsScenario() { return m_bScenario; }
	void ScenarioText( int8 n, const char* sz, const CVector4& color, int32 nFinishDelay = 0, int32 nSpeed = 1 );
	bool IsScenarioTextFinished();
	void HeadText( const char* sz, const CVector4& color, int32 nTime = 0 );

	void Update();
	virtual void Render( CRenderContext2D& context ) override;
private:
	void UpdatePos();
	void UpdateInputItem( int32 nItem );
	void Effect0();
	void Effect1();
	CReference<CEntity> m_pHeadText;
	CReference<CEntity> m_pScenarioText[2];

	CRectangle m_origRect;
	CVector2 m_playerInputOrig;
	int32 m_nPlayerActionFrame;
	vector<CElement2D> m_vecPlayerActionElems;
	vector<CVector4> m_vecPlayerActionElemParams;
	struct SInputItem
	{
		vector<int8> vec;
		vector<CElement2D> vecElems;
		int32 nMatchLen;
	};
	vector<SInputItem> m_vecInputItems;
	bool m_bScenario;
	int8 m_nLastScenarioText;
	int32 m_nScenarioTextFinishDelay;
	int32 m_nHeadTextTime;
};


struct SWorldDataFrame
{
	struct SLevelData
	{
		map<string, int32> mapDataInt;
	};
	string strCurLevel;
	string strLastLevel;
	map<string, SLevelData> mapLevelData;
	map<string, int32> mapDataInt;
	TVector2<int32> playerEnterPos;
	int8 nPlayerEnterDir;
	CBufFile playerData;
};

struct SWorldData
{
	SWorldData() : bCheckPoint( false ), bCheckPointCurLvl( false ) {}
	SWorldDataFrame curFrame;
	SWorldDataFrame curLvlBackup;
	SWorldDataFrame checkPoint;
	bool bCheckPoint;
	bool bCheckPointCurLvl;

	SWorldDataFrame::SLevelData& GetCurLevelData() { return curFrame.mapLevelData[curFrame.strCurLevel]; }
	void OnEnterLevel( const char* szCurLevel, CPlayer* pPlayer, const TVector2<int32>& playerPos, int8 nPlayerDir );
	void OnReset( CPlayer* pPlayer );
	void CheckPoint( CPlayer* pPlayer );
	void OnRestoreToCheckpoint( CPlayer* pPlayer );
	void ClearKeys();
	SWorldDataFrame::SLevelData curLevelDataBackup;
	map<string, int32> mapDataIntBackup;
};

class CMasterLevel : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CMasterLevel( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMasterLevel ); }

	void Init( CPlayer* pPlayer );
	void Begin( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir );
	void TransferTo( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir );
	void ResetCurLevel();
	CMainUI* GetMainUI() { return m_pMainUI; }
	CMyLevel* GetCurLevel() { return m_pCurLevel; }
	CCutScene* GetCurCutScene() { return m_pCurCutScene; }
	SWorldDataFrame::SLevelData& GetCurLevelData();
	const char* GetCurLevelName() { return m_worldData.curFrame.strCurLevel.c_str(); }
	const char* GetLastLevelName() { return m_worldData.curFrame.strLastLevel.c_str(); }
	void BeginScenario();
	void EndScenario();
	bool IsScenario();
	void RunScenarioScriptText( const char* sz );
	void RunScenarioScript();

	void CheckPoint();
	void RestoreToCheckpoint();
	int32 EvaluateKeyInt( const char* str );
	void SetKeyInt( const char* str, int32 n );
	void ClearKeys() { m_worldData.ClearKeys(); }
	void ScriptTransferTo( const char* szName, int32 nPlayerX, int32 nPlayerY, int8 nPlayerDir );

	CVector2 GetCamPos();
	void OnPlayerDamaged();
	void Update();
private:
	void ResetMainUI();
	CReference<CMainUI> m_pMainUI;
	CReference<CRenderObject2D> m_pLevelFadeMask;

	SWorldData m_worldData;
	CReference<CPlayer> m_pPlayer;
	CReference<CPrefab> m_pCurLevelPrefab;
	CReference<CMyLevel> m_pCurLevel;
	CReference<CMyLevel> m_pLastLevel;
	CReference<CCutScene> m_pCurCutScene;
	CReference<CPrefab> m_pLastLevelPrefab;
	int32 m_nPlayerDamageFrame;

	int8 m_nTransferType;
	CVector2 m_transferOfs;
	CVector2 m_camTransferBegin;
	int32 m_nTransferAnimTotalFrames;
	int32 m_nTransferAnimFrames;
	int32 m_nTransferFadeOutFrames;
	CReference<CLuaState> m_pScenarioScript;
	string m_strScriptTransferTo;
	int32 m_nScriptTransferPlayerX;
	int32 m_nScriptTransferPlayerY;
	int32 m_nScriptTransferPlayerDir;
};