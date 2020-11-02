#pragma once
#include "Common/StringUtil.h"
#include "Common/PriorityQueue.h"
#include "Render/DrawableGroup.h"
#include "BasicElems.h"
#include <deque>
#include <set>

#define LEVEL_GRID_SIZE_X 24.0f
#define LEVEL_GRID_SIZE_Y 32.0f
#define LEVEL_GRID_SIZE CVector2( LEVEL_GRID_SIZE_X, LEVEL_GRID_SIZE_Y )

struct SLevelTileData
{
	SLevelTileData( const SClassCreateContext& context ) {}
	TResourceRef<CDrawableGroup> pTileDrawable;
	CRectangle texRect;
	bool bBlocked;
};

struct SLevelGridData
{
	SLevelGridData() { bBlocked = false; nNextStage = 0; }
	SLevelGridData( const SClassCreateContext& context ) {}
	bool bBlocked;
	int32 nTile;
	int32 nNextStage;
};

struct SLevelEnvGridDesc
{
	SLevelEnvGridDesc() { memset( this, 0, sizeof( SLevelEnvGridDesc ) ); }
	SLevelEnvGridDesc( const SClassCreateContext& context ) {}
	int32 nDist;
	CVector4 param[2];
	CVector4 paramDynamic[2];
	CVector2 sizeDynamic;
	float fPeriod;
	float fRandomPhaseOfs;
	CVector2 gridPhaseOfs;
	float fBlendWeight;

	void Interpolate( const SLevelEnvGridDesc& a, float t )
	{
		float t0 = 1 - t;
		nDist = nDist * t0 + a.nDist * t;
		param[0] = param[0] * t0 + a.param[0] * t;
		param[1] = param[1] * t0 + a.param[1] * t;
		paramDynamic[0] = paramDynamic[0] * t0 + a.paramDynamic[0] * t;
		paramDynamic[1] = paramDynamic[1] * t0 + a.paramDynamic[1] * t;
		sizeDynamic = sizeDynamic * t0 + a.sizeDynamic * t;
		fPeriod = fPeriod * t0 + a.fPeriod * t;
		fRandomPhaseOfs = fRandomPhaseOfs * t0 + a.fRandomPhaseOfs * t;
		gridPhaseOfs = gridPhaseOfs * t0 + a.gridPhaseOfs * t;
		fBlendWeight = fBlendWeight * t0 + a.fBlendWeight * t;
	}
	void Overlay ( const SLevelEnvGridDesc& a, float t = 1.0f )
	{
		float f = a.fBlendWeight * t;
		nDist = nDist + a.nDist * f;
		param[0] = param[0] + a.param[0] * f;
		param[1] = param[1] + a.param[1] * f;
		paramDynamic[0] = paramDynamic[0] + a.paramDynamic[0] * f;
		paramDynamic[1] = paramDynamic[1] + a.paramDynamic[1] * f;
		sizeDynamic = sizeDynamic + a.sizeDynamic * f;
		fPeriod = fPeriod + a.fPeriod * f;
		fRandomPhaseOfs = fRandomPhaseOfs + a.fRandomPhaseOfs * f;
		gridPhaseOfs = gridPhaseOfs + a.gridPhaseOfs * f;
		fBlendWeight = fBlendWeight + f;
	}
	void Normalize()
	{
		if( abs( fBlendWeight ) <= 0 )
			return;
		float f = 1.0f / fBlendWeight;
		nDist = nDist * f;
		param[0] = param[0] * f;
		param[1] = param[1] * f;
		paramDynamic[0] = paramDynamic[0] * f;
		paramDynamic[1] = paramDynamic[1] * f;
		sizeDynamic = sizeDynamic * f;
		fPeriod = fPeriod * f;
		fRandomPhaseOfs = fRandomPhaseOfs * f;
		gridPhaseOfs = gridPhaseOfs * f;
		fBlendWeight = 1;
	}
};

struct SLevelEnvDesc
{
	SLevelEnvDesc( const SClassCreateContext& context ) {}
	TArray<SLevelEnvGridDesc> arrGridDesc;
	TArray<int32> arrJamStrength;
	int32 GetMaxDist() { return arrGridDesc.Size() ? arrGridDesc[arrGridDesc.Size() - 1].nDist : -1; }
	void OverlayToGrid( SLevelEnvGridDesc& grid, int32 nDist );
};

class CLevelEnvEffect : public CEntity
{
	friend class CLevelToolsView;
	friend class CLevelEnvTool;
	friend void RegisterGameClasses_Level();
public:
	CLevelEnvEffect( const SClassCreateContext& context ) : CEntity( context ), m_fFade( 1 ) { SET_BASEOBJECT_ID( CLevelEnvEffect ); }

	virtual void OnAddedToStage() override { Init(); }
	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override { Init(); }
	void Init();
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	void Resize( const TRectangle<int32>& rect );
	void Clear();
	void Fill( int8 n, const TVector2<int32>& p );
	void SetFade( float fFade ) { m_fFade = fFade; }
	const char* GetCondition() const { return m_strCondition; }
private:
	TArray<SLevelEnvDesc> m_arrEnvDescs;
	TArray<int8> m_arrEnvMap;
	int32 m_nWidth, m_nHeight;
	CVector2 m_gridSize;
	CVector2 m_gridOfs;
	CString m_strCondition;

	struct SElem
	{
		SLevelEnvGridDesc gridDesc;
		float fPhase;
		CRectangle origRect;
		CElement2D elem;
		CVector4 param[2];
	};
	vector<SElem> m_elems;
	CElement2D m_dummyElem;
	float m_fFade;
};

struct SLevelNextStageData
{
	SLevelNextStageData( const SClassCreateContext& context ) {}
	TResourceRef<CPrefab> pNxtStage;
	int32 nOfsX, nOfsY;
	CString strKeyOrig;
	CString strKeyRedirect;
};

class CLevelStealthLayer : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CLevelStealthLayer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelStealthLayer ); }

	virtual void OnAddedToStage() override;
	void Update( class CMyLevel* pLevel );
	virtual void Render( CRenderContext2D& context ) override;
private:
	CElement2D& AddElem( int32 x, int32 y, const CVector2& ofs, void* pParams );
	vector<CElement2D> m_elems;
	uint32 m_nTick;
};

class CLevelIndicatorLayer : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CLevelIndicatorLayer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelIndicatorLayer ); }

	virtual void OnAddedToStage() override;
	void Update( class CMyLevel* pLevel );
	void UpdatePawnTarget( CPawn* pPawn, const TVector2<int32>& target );
	virtual void Render( CRenderContext2D& context ) override;
private:
	CElement2D& AddElem( int32 x, int32 y, const CVector2& ofs, void* pParams );
	TResourceRef<CPrefab> m_pPawnTargetEftPrefab;

	vector<CElement2D> m_elems;
	vector<CVector4> m_vecBlockedParams;
	uint32 m_nTick;
	vector<CVector4> m_vecNxtStageParam[2];
	vector<CVector2> m_vecNxtStageOfs[2];
	CReference<CLevelStealthLayer> m_pStealthLayer;
	struct SPawnTargetEft
	{
		TVector2<int32> target;
		CReference<CEntity> pEft;
	};
	map<CReference<CPawn>, TVector2<int32> > m_mapPawnTarget;
	map<CReference<CPawn>, SPawnTargetEft > m_mapPawnTargetEft;
};

class CMyLevel;
class CLevelScript : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CLevelScript( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelScript ); }

	virtual void OnInit( CMyLevel* pLevel ) {}
	virtual void OnBegin( CMyLevel* pLevel ) {}
	virtual void OnDestroy( CMyLevel* pLevel ) {}
	virtual void OnUpdate( CMyLevel* pLevel ) {}
	virtual void OnUpdate1( CMyLevel* pLevel ) {}
	virtual void OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir ) {}
	virtual void OnPlayerAction( int32 nMatchLen, int8 nType ) {}
	virtual void OnAlert( CPawn* pTriggeredPawn, const TVector2<int32>& pawnOfs ) {}
private:
};

class CPawnLayer : public CEntity
{
	friend void RegisterGameClasses_Level();
	friend class CPawnTool;
public:
	CPawnLayer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CPawnLayer ); }
	const CString& GetCondition() const { return m_strCondition; }
private:
	CString m_strCondition;
};

class CMyLevel : public CEntity
{
	friend void RegisterGameClasses_Level();
	friend class CLevelEdit;
	friend class CWorldCfgEditor;
	friend class CMasterLevel;
	friend class CLevelToolsView;
	friend class CTileTool;
	friend class CPawnTool;
public:
	CMyLevel( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMyLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	struct SGrid
	{
		SGrid() : bCanEnter( true ), bBlockSight( false ), nNextStage( 0 ), nHitEft( 0 ), nMissEft( 0 ), nHitBlockedEft( 0 ), nHitBashEft( 0 ), nMissBashEft( 0 ),
			nBlockEft( 0 ), bStealthAlert( false ), bStealthDetect( false ), pMounts( NULL ) {}
		bool bCanEnter;
		bool bBlockSight;
		int8 nHitEft;
		int8 nMissEft;
		int8 nHitBlockedEft;
		int8 nHitBashEft;
		int8 nMissBashEft;
		int8 nBlockEft;
		bool bStealthAlert;
		bool bStealthDetect;
		int32 nNextStage;
		TVector2<int32> blockOfs;
		CReference<CPawn> pPawn0;
		CReference<CPawn> pPawn1;
		LINK_LIST_REF_HEAD( pMounts, CPlayerMount, Mount )
	};
	TVector2<int32> GetSize() const { return TVector2<int32>( m_nWidth, m_nHeight ); }
	SGrid* GetGrid( const TVector2<int32>& p ) { return p.x >= 0 && p.y >= 0 && p.x < m_nWidth && p.y < m_nHeight ?
		&m_vecGrid[p.x + p.y * m_nWidth] : NULL; }
	const SLevelGridData* GetGridData( const TVector2<int32>& p ) const { return p.x >= 0 && p.y >= 0 && p.x < m_nWidth && p.y < m_nHeight ?
		&m_arrGridData[p.x + p.y * m_nWidth] : NULL; }
	const char* GetRegionName() { return m_strRegion; }
	const CVector2& GetCamPos() { return m_camPos; }
	CPlayer* GetPlayer() { return m_pPlayer; }
	bool IsBegin() { return m_bBegin; }
	bool IsEnd() { return m_bEnd; }
	bool IsComplete() { return m_bComplete; }
	bool IsFailed() { return m_bFailed; }
	bool IsActionPreview() { return m_pActionPreviewCoroutine != NULL; }
	class CMasterLevel* GetMasterLevel();
	CEntity* GetPawnRoot() { return m_pPawnRoot; }
	CLevelEnvEffect* GetEnvEffect() { return m_pEnvEffect; }

	void Begin();
	void End();
	void Fail( int8 nFailType = 0 );
	void Freeze();
	void UnFreeze();
	bool IsFreeze() { return m_nFreeze > 0; }
	CPawn* SpawnPawn( int32 n, int32 x, int32 y, int8 nDir, const char* szRemaining = NULL, CPawn* pCreator = NULL, int32 nForm = 0 );
	bool AddPawn( CPawn* pPawn, const TVector2<int32>& pos, int8 nDir, CPawn* pCreator = NULL, int32 nForm = 0 );
	bool AddPawn1( CPawn* pPawn, int32 nState, int32 nStateTick, const TVector2<int32>& pos, const TVector2<int32>& moveTo, int8 nDir );
	void RemovePawn( CPawn* pPawn );
	bool IsGridMoveable( const TVector2<int32>& p, CPawn* pPawn, int8 nForceCheckType = 0 );
	bool IsGridBlockSight( const TVector2<int32>& p );
	bool PawnMoveTo( CPawn* pPawn, const TVector2<int32>& ofs, int8 nForceCheckType = 0, int32 nMoveFlag = 0, bool bBlockEft = true );
	void PawnMoveEnd( CPawn* pPawn );
	void PawnMoveBreak( CPawn* pPawn );
	void PawnDeath( CPawn* pPawn );
	bool PawnTransform( CPawn* pPawn, int32 nForm, const TVector2<int32>& ofs, bool bBlockEft = true );
	CPickUp* FindPickUp( const TVector2<int32>& p, int32 w, int32 h );
	CPlayerMount* FindMount( const TVector2<int32>& ofs );
	CPawn* FindUseablePawn( const TVector2<int32>& p, int8 nDir, int32 w, int32 h );
	CPawn* GetPawnByName( const char* szName );
	void OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir );
	void OnPlayerAction( int32 nMatchLen, int8 nType );
	TVector2<int32> SimpleFindPath( const TVector2<int32>& begin, const TVector2<int32>& end, int32 nCheckFlag, vector<TVector2<int32> >* pVecPath = NULL );
	TVector2<int32> FindPath1( const TVector2<int32>& begin, const TVector2<int32>& end, function<bool( SGrid*, const TVector2<int32>& )> FuncGrid, vector<TVector2<int32> >* pVecPath = NULL );
	void Alert( CPawn* pTriggeredPawn, const TVector2<int32>& pawnOfs );
	void Alert1();
	void BeginTracer( const char* sz, int32 nDelay );
	void BeginTracer1( int32 n, int32 nDelay );
	void EndTracer();

	void BlockStage();
	void BlockExit( int32 n );
	bool IsExitBlocked( int32 n );
	bool IsGridBlockedExit( SGrid* pGrid, bool bIgnoreComplete = false );
	SLevelNextStageData& GetNextLevelData( int32 n ) { return m_arrNextStage[n]; }
	int32 GetNextLevelCount() { return m_arrNextStage.Size(); }
	int32 FindNextLevelIndex( const char* szLevelName );
	void Redirect( int32 n, int32 n1 );

	void BeginScenario();
	void EndScenario();
	bool IsScenario() { return m_bScenario; }
	void GetAllUseableGrid( vector<TVector2<int32> >& result );

	void Init( bool bPreview = false );
	void Update();
	int32 UpdateActionPreview();
	void ActionPreviewPause();

	void RegisterBegin( CTrigger* pTrigger );
	void RegisterUpdate( CTrigger* pTrigger ) { m_trigger.Register( 0, pTrigger ); }
	void RegisterUpdate1( CTrigger* pTrigger ) { m_trigger.Register( 1, pTrigger ); }
	void RegisterAlwaysUpdate( CTrigger* pTrigger ) { m_trigger.Register( 2, pTrigger ); }

	void ScriptForEachEnemy();
private:
	void HandleSpawn( CLevelSpawnHelper* pSpawnHelper );
	void HandlePawnMounts( CPawn* pPawn, bool bRemove );
	void FlushSpawn();
	void InitTiles();
	void InitScripts();
	void UpdateActionPreviewFunc();
	int32 m_nWidth, m_nHeight;
	CString m_strRegion;
	CReference<CEntity> m_pPawnRoot;
	CVector2 m_camPos;
	TArray<SLevelTileData> m_arrTileData;
	TArray<SLevelGridData> m_arrGridData;
	TArray<SLevelNextStageData> m_arrNextStage;
	TArray<TResourceRef<CPrefab> > m_arrSpawnPrefab;
	TResourceRef<CDrawableGroup> m_pTileDrawable;
	CString m_strInitScript;
	CString m_strBeginScript;
	CString m_strDestroyScript;

	bool m_bBegin;
	bool m_bEnd;
	bool m_bScenario;
	bool m_bComplete;
	bool m_bFailed;
	bool m_bFullyEntered;
	bool m_bBlocked;
	int8 m_nFreeze;
	bool m_bStartBattle;
	vector<CReference<CLevelScript> > m_vecScripts;
	vector<SGrid> m_vecGrid;
	TClassTrigger<CMyLevel> m_onTick;
	CReference<CPlayer> m_pPlayer;
	CReference<CLevelIndicatorLayer> m_pIndicatorLayer;
	CReference<CLevelEnvEffect> m_pEnvEffect;
	struct SExitState
	{
		SExitState() : nRedirect( -1 ), bBlocked( false ) {}
		int32 nRedirect;
		bool bBlocked;
	};
	vector<SExitState> m_vecExitState;
	vector<CReference<CLevelSpawnHelper> > m_vecSpawner;
	int32 m_nTracerDelayLeft;
	int8 m_nTracerSpawnExit;
	CReference<CEntity> m_pTracerSpawnEffect;

	class ICoroutine* m_pActionPreviewCoroutine;
	CEventTrigger<1> m_beginTrigger;
	CEventTrigger<3> m_trigger;
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
	void Reset( const CVector2& inputOrig, const CVector2& iconOrig, bool bClearAllEfts = true );
	void OnLevelBegin();
	void RefreshPlayerInput( vector<int8>& vecInput, int32 nMatchLen, int8 nChargeKey, int8 nType );
	void InsertDefaultFinishAction();
	void OnPlayerAction( vector<int8>& vecInput, int32 nMatchLen, int8 nChargeKey, int8 nType );

	void BeginScenario();
	void EndScenario();
	bool IsScenario() { return m_bScenario; }
	void ScenarioText( int8 n, const char* sz, const CVector4& color, int32 nFinishDelay = 0, int32 nSpeed = 1, const char* szSound = "", int32 nSoundInterval = 1 );
	bool IsScenarioTextFinished();
	void HeadText( const char* sz, const CVector4& color, int32 nTime = 0, const char* szSound = "", int32 nSoundInterval = 1 );
	void ShowFailEft( bool b );
	void ShowFreezeEft( int32 nLevel );
	void ClearLabels();
	void SetLabel( int32 nIndex, int32 x, int32 y );

	void Update();
	virtual void Render( CRenderContext2D& context ) override;
private:
	void UpdatePos();
	void UpdateInputItem( int32 nItem );
	void UpdateIcons();
	void Effect0();
	void Effect1();
	void RecordEffect();
	void FailEffect();
	void FreezeEffect( int32 nLevel );
	CReference<CEntity> m_pHeadText;
	CReference<CEntity> m_pScenarioText[2];
	CReference<CEntity> m_pFailTips[3];
	CReference<CEntity> m_pIcons[ePlayerEquipment_Count];
	CReference<CEntity> m_pLabelsRoot;
	CReference<CEntity> m_pAmmoCount;

	CRectangle m_origRect;
	CVector2 m_playerInputOrig;
	CVector2 m_iconOrig;
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
	float m_fLabelX;
	vector<CReference<CRenderObject2D> > m_vecLabels;
	bool m_bScenario;
	int8 m_nLastScenarioText;
	int32 m_nScenarioTextFinishDelay;
	int32 m_nHeadTextTime;
	int32 m_nRecordEftFrames;
	int32 m_nFreezeLevel;
};

#define MAX_SCENARIO_RECORDS 256
struct SWorldDataFrame
{
	SWorldDataFrame() : bForceAllVisible( false ), nTracerSpawnDelay( 0 ) {}
	void Load( IBufReader& buf, int32 nVersion );
	void Save( CBufFile& buf );
	struct SPawnData
	{
		SPawnData() { memset( this, 0, sizeof( SPawnData ) ); }
		int32 nState;
		int32 nStateTick;
		TVector2<int32> p;
		TVector2<int32> p1;
		int8 nDir;
		int8 nSpawnIndex;
		int16 nEmpty;
	};
	struct SLevelData
	{
		SLevelData() : bVisited( false ) {}
		bool bVisited;
		map<string, int32> mapDataInt;
		map<string, string> mapDataString;
		map<string, SPawnData> mapDataDeadPawn;
	};
	struct SLevelMark
	{
		string strLevelName;
		TVector2<int32> ofs;
	};
	struct SScenarioRecord
	{
		int8 nType;
		CVector4 color;
		string str;
	};

	string strCurLevel;
	string strLastLevel;
	map<string, SLevelData> mapLevelData;
	map<string, int32> mapDataInt;
	map<string, string> mapDataString;
	map<string, int32> mapDataIntStatic;
	map<string, string> mapDataStringStatic;
	TVector2<int32> playerEnterPos;
	int8 nPlayerEnterDir;
	bool bForceAllVisible;
	CBufFile playerData;
	set<string> unlockedRegionMaps;
	map<string, SLevelMark> mapLevelMarks;
	string strTracer;
	int32 nTracerSpawnDelay;
	deque<SScenarioRecord> vecScenarioRecords;
};

struct SWorldData
{
	SWorldData() : pCheckPoint( NULL ), nCurFrameCount( 0 ) {}
	~SWorldData() { if( pCheckPoint ) delete pCheckPoint; for( auto p : backupFrames ) delete p; }
	SWorldDataFrame curFrame;
	SWorldDataFrame* pCheckPoint;
	deque<SWorldDataFrame*> backupFrames;
	int32 nCurFrameCount;
	static constexpr int32 nMaxFrameCount = 10;

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );
	const char* GetCurLevel() { return curFrame.strCurLevel.c_str(); }
	SWorldDataFrame::SLevelData& GetLevelData( const char* szLevel ) { return curFrame.mapLevelData[szLevel]; }
	SWorldDataFrame::SLevelData& GetCurLevelData() { return curFrame.mapLevelData[curFrame.strCurLevel]; }
	void OnEnterLevel( const char* szCurLevel, CPlayer* pPlayer, const TVector2<int32>& playerPos, int8 nPlayerDir );
	void OnReset( CPlayer* pPlayer );
	void OnRetreat( CPlayer* pPlayer );
	void CheckPoint( CPlayer* pPlayer );
	void OnRestoreToCheckpoint( CPlayer* pPlayer );
	void ClearKeys();
	void Respawn();
	void RespawnLevel( const char* szLevel );
	void UnlockRegionMap( const char* szRegion );
	void GetScenarioRecords( function<void( int8, const CVector4&, const char* )> Func );
	void OnScenarioText( int8 n, const char* sz, const CVector4& color );
};

class CMasterLevel : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CMasterLevel( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMasterLevel ); }

	virtual void OnAddedToStage() override;
	void NewGame( CPlayer* pPlayer, CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir );
	void Continue( CPlayer* pPlayer, IBufReader& buf );
	void Save();
	void TransferTo( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType = 0 );
	void JumpBack( int8 nType );
	void Fall( const char* szTargetRegion );
	bool IsTransfer() { return m_pTransferCoroutine != NULL; }
	CMainUI* GetMainUI() { return m_pMainUI; }
	CMyLevel* GetCurLevel() { return m_pCurLevel; }
	CCutScene* GetCurCutScene() { return m_pCurCutScene; }
	SWorldData& GetWorldData() { return m_worldData; }
	SWorldDataFrame::SLevelData& GetCurLevelData();
	const char* GetCurLevelName() { return m_worldData.curFrame.strCurLevel.c_str(); }
	const char* GetLastLevelName() { return m_worldData.curFrame.strLastLevel.c_str(); }
	void BeginScenario();
	void EndScenario();
	void ForceEndScenario();
	bool IsScenario();
	void RunScenarioScriptText( const char* sz );
	void RunScenarioScript();

	void CheckPoint( bool bRefresh = false, bool bIgnoreSave = false );
	int32 EvaluateKeyInt( const char* str ) { return EvaluateKeyIntLevelData( str, m_worldData.GetCurLevelData() ); }
	int32 EvaluateKeyIntLevelData( const char* str, SWorldDataFrame::SLevelData& levelData );
	const char* EvaluateKeyString( const char* str ) { return EvaluateKeyStringLevelData( str, m_worldData.GetCurLevelData() ); }
	const char* EvaluateKeyStringLevelData( const char* str, SWorldDataFrame::SLevelData& levelData );
	void SetKeyInt( const char* str, int32 n ) { SetKeyIntLevelData( str, n, m_worldData.GetCurLevelData() ); }
	void SetKeyIntLevelData( const char* str, int32 n, SWorldDataFrame::SLevelData& levelData );
	void SetKeyString( const char* str, const char* szValue ) { SetKeyStringLevelData( str, szValue, m_worldData.GetCurLevelData() ); }
	void SetKeyStringLevelData( const char* str, const char* szValue, SWorldDataFrame::SLevelData& levelData );
	void ClearKeys() { m_worldData.ClearKeys(); }
	void Respawn() { m_worldData.Respawn(); }
	void RespawnLevel( const char* szLevel ) { m_worldData.RespawnLevel( szLevel ); }
	void TransferTo1( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType = 0 );
	void ScriptTransferTo( const char* szName, int32 nPlayerX, int32 nPlayerY, int8 nPlayerDir, int8 nTransferType = 0 );
	void UnlockRegionMap( const char* szRegion ) { m_worldData.UnlockRegionMap( szRegion ); }
	void ShowWorldMap( bool bShow, int8 nType = 0 );
	void AddLevelMark( const char* szKey, const char* szLevel, int32 x, int32 y );
	bool HasLevelMark( const char* szKey );
	void RemoveLevelMark( const char* szKey );
	void ShowActionPreview( bool bShow );
	void ShowLogUI( bool bShow, int8 nPage = -1, int32 nIndex = 0 );
	void ShowDoc( int32 nIndex ) { ShowLogUI( true, 0, nIndex ); }
	void ShowMenu( bool bShow, int8 nCurPage );
	void SwitchMenuPage( int8 nPage );
	bool IsMenuShow() { return m_pMenu->bVisible; }
	bool IsMenuPageEnabled( int8 nPage );
	CEntity* ShowInteractionUI( CPawn* pPawn, const char* szName );
	CEntity* GetInteractionUI() { return m_pInteractionUI; }
	void BlackOut( int32 nFrame1, int32 nFrame2 );
	void InterferenceStripEffect( int8 nType, float fSpeed );

	CVector2 GetCamPos();
	void OnPlayerDamaged();
	void Update();
private:
	void ResetMainUI();
	void RefreshMainUI();
	void EndCurLevel();
	void TransferFunc();
	void TransferFuncLevel2Level();
	void TransferFuncLevel2Level0();
	void TransferFuncLevel2Level1();
	void TransferFuncLevel2Level2();
	void TransferFuncLevel2Level3();
	void TransferFuncCut2Level();
	void TransferFuncLevel2Cut();
	enum
	{
		eMenuPage_ActionPreview,
		eMenuPage_Map,
		eMenuPage_Log,
		eMenuPage_3,
		eMenuPage_4
	};

	CReference<CMainUI> m_pMainUI;
	CReference<CRenderObject2D> m_pLevelFadeMask;
	CReference<CEntity> m_pMenu;
	CReference<CEntity> m_pMenuItem[5];
	CReference<CRenderObject2D> m_pMenuSelected;
	CReference<CEntity> m_pWorldMap;
	CReference<CEntity> m_pActionPreview;
	CReference<CEntity> m_pLogUI;
	int8 m_nMenuPage;
	int8 m_nEnabledPageCount;
	int8 m_nMenuPageItemIndex;
	int8 m_enabledPages[5];

	SWorldData m_worldData;
	CReference<CPlayer> m_pPlayer;
	CReference<CPrefab> m_pCurLevelPrefab;
	CReference<CMyLevel> m_pCurLevel;
	CReference<CMyLevel> m_pLastLevel;
	CReference<CCutScene> m_pCurCutScene;
	CReference<CPrefab> m_pLastLevelPrefab;
	int32 m_nPlayerDamageFrame;

	CReference<CEntity> m_pInteractionUI;
	CReference<CPawn> m_pInteractionUIPawn;
	string m_strInteractionUI;
	bool m_bInteractionUIInit;
	int32 m_nBlackOutFrame1, m_nBlackOutFrame2;
	CReference<CEntity> m_pInterferenceStripEffect;

	class ICoroutine* m_pTransferCoroutine;
	CReference<CPrefab> m_pTransferTo;
	TVector2<int32> m_transferPos;
	int8 m_nTransferDir;
	int8 m_nTransferType;
	CVector2 m_transferCurCamPos;
	CReference<CEntity> m_pTransferEft;
	/*CVector2 m_transferOfs;
	CVector2 m_camTransferBegin;
	int32 m_nTransferAnimTotalFrames;
	int32 m_nTransferAnimFrames;
	int32 m_nTransferFadeOutTotalFrames;
	int32 m_nTransferFadeOutFrames;*/

	CReference<CLuaState> m_pScenarioScript;
	CReference<CPrefab> m_pScriptTransferTo;
	int32 m_nScriptTransferPlayerX;
	int32 m_nScriptTransferPlayerY;
	int32 m_nScriptTransferPlayerDir;
	int8 m_nScriptTransferType;
};