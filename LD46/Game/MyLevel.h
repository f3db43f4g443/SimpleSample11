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

struct SLevelCamCtrlPoint
{
	SLevelCamCtrlPoint( const SClassCreateContext& context ) {}
	float fWeight;
	float fDamping;
	CVector2 orig;
	CVector2 g1, g2;
	TArray<CVector3> arrPath;
	TArray<CVector3> arrTangent;

	void Offset( const CVector2& p )
	{
		orig = orig + p;
		for( int i = 0; i < arrPath.Size(); i++ )
		{
			arrPath[i].x += p.x;
			arrPath[i].y += p.y;
		}
	}
};

struct SLevelCamCtrlPointLink
{
	SLevelCamCtrlPointLink( const SClassCreateContext& context ) {}
	int32 n1, n2;
	CVector2 ofs1, ofs2;
	float fStrength1;
	float fStrength2;
	
	float l0;
};

class CLevelEnvEffect : public CEntity
{
	friend class CLevelToolsView;
	friend class CLevelEnvTool;
	friend void RegisterGameClasses_Level();
public:
	CLevelEnvEffect( const SClassCreateContext& context ) : CEntity( context ), m_ctrlPoint1( context ), m_ctrlPoint2( context ),
		m_fFade( 1 ) { SET_BASEOBJECT_ID( CLevelEnvEffect ); }

	virtual void OnAddedToStage() override { Init(); }
	virtual void OnRemovedFromStage() override { m_pLevel = NULL; }
	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override { Init(); }
	void Init();
	void SetLevel( class CMyLevel* pLevel ) { m_pLevel = pLevel; }
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	void Resize( const TRectangle<int32>& rect );
	void Clear();
	void Fill( int8 n, const TVector2<int32>& p );
	void SetFade( float fFade ) { m_fFade = fFade; }
	void ScenarioFade( bool b ) { m_bScenarioFade = b; }
	const char* GetCondition() const { return m_strCondition; }
	void CopyState( CLevelEnvEffect* p1 ) { m_fFade = p1->m_fFade; m_bScenarioFade = p1->m_bScenarioFade; }

	float GetScenarioFade() { return m_fScenarioFade; }
	CVector3 GetGamma() { return m_gamma; }
	CVector3 GetColorTranspose( int32 n ) { return m_colorTranspose[n]; }
	bool IsOverrideBackColor() { return m_bOverrideBackColor; }
	bool IsCustomBattleEffectBackColor() { return m_bCustomBattleEffectBackColor; }
	CVector3 GetBackColor() { return m_backColor; }
	CVector3 GetBattleEffectBackColor() { return m_battleEffectBackColor; }

	bool IsCtrlPointValid() { return m_bCtrlPointValid; }
	void InitCtrlPoints();
	void InitCtrlPointsState( float x, float y, float r, float s, bool bNoReset = false );
	void UpdateCtrlPoints();
	CVector4 GetCtrlPointsTrans();

	void OnPlayerAction( CPlayer* pPlayer );
	void OnPlayerMoveBegin( CPlayer* pPlayer );
	void OnPlayerMoveEnd( CPlayer* pPlayer );
	void OnEnemyMoveBegin( CPawn* pEnemy );
	void OnEnemyMoveEnd( CPawn* pEnemy );
	void OnLevelAlert( CPawn* pPawn, int32 x, int32 y );

	float GetCtrlPointOrigX( int32 nIndex );
	float GetCtrlPointOrigY( int32 nIndex );
	void ApplyForce( int32 nIndex, int8 nType, float fForceX, float fForceY, int32 nDuration, int8 nFadeType );
private:
	CVector3 m_gamma;
	CVector3 m_colorTranspose[3];
	bool m_bOverrideBackColor;
	bool m_bCustomBattleEffectBackColor;
	bool m_bCtrlPointValid;
	CVector3 m_backColor;
	CVector3 m_battleEffectBackColor;
	TArray<SLevelEnvDesc> m_arrEnvDescs;
	TArray<int8> m_arrEnvMap;
	int32 m_nWidth, m_nHeight;
	CVector2 m_gridSize;
	CVector2 m_gridOfs;
	float m_fScenarioFade;
	CString m_strCondition;
	float m_fCtrlPointTransRemoveRot;
	SLevelCamCtrlPoint m_ctrlPoint1, m_ctrlPoint2;
	TArray<SLevelCamCtrlPoint> m_arrCtrlPoint;
	TArray<SLevelCamCtrlPointLink> m_arrCtrlLink;
	CString m_strCommonEvtScript;

	CMyLevel* m_pLevel;
	struct SCtrlPointState
	{
		CVector2 p;
		CVector2 v;
		CVector2 f1;
		CVector2 f2;
		int32 nCurFrame;
		vector<CVector2> vecFrames;
		CElement2D elemDebugDraw;
		CVector4 debugDrawParam[2];
		CVector2 GetCurPos() { if( vecFrames.size() ) return p + vecFrames[nCurFrame]; return p; }
	};
	vector<SCtrlPointState> m_vecCtrlPointStates;
	struct SExtraForce
	{
		int32 nIndex;
		int8 nType;
		CVector2 force;
		int32 nDuration;
		int8 nFadeType;

		int32 nTimeLeft;
	};
	vector<SExtraForce> m_vecExtraForces;
	CReference<CLuaState> m_pCommonEvt;

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
	bool m_bScenarioFade;
	bool m_bCtrlPointsInited;
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
	virtual void OnPlayerAction( vector<int8>& vecInput, int32 nMatchLen, int8 nType ) {}
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
			nBlockEft( 0 ), bStealthAlert( false ), bStealthDetect( false ), nAlertEft( 0 ), pMounts( NULL ) {}
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
		int8 nAlertEft;
		int32 nNextStage;
		TVector2<int32> blockOfs;
		CReference<CPawn> pPawn0;
		CReference<CPawn> pPawn1;
		int32 nTile;
		CReference<CRenderObject2D> pTile;
		LINK_LIST_REF_HEAD( pMounts, CPlayerMount, Mount )
	};
	TVector2<int32> GetSize() const { return TVector2<int32>( m_nWidth, m_nHeight ); }
	SGrid* GetGrid( const TVector2<int32>& p ) { return p.x >= 0 && p.y >= 0 && p.x < m_nWidth && p.y < m_nHeight ?
		&m_vecGrid[p.x + p.y * m_nWidth] : NULL; }
	const SLevelGridData* GetGridData( const TVector2<int32>& p ) const { return p.x >= 0 && p.y >= 0 && p.x < m_nWidth && p.y < m_nHeight ?
		&m_arrGridData[p.x + p.y * m_nWidth] : NULL; }
	int32 CheckGrid( int32 x, int32 y, CPawn* pPawn, int8 nForceCheckType );
	CRectangle GetMainAreaSize() const;
	const char* GetRegionName() { return m_strRegion; }
	const CVector2& GetCamPos() { return m_camPos; }
	CPlayer* GetPlayer() { return m_pPlayer; }
	bool IsBegin() { return m_bBegin; }
	bool IsEnd() { return m_bEnd; }
	bool IsComplete() { return m_bComplete; }
	bool IsFailed() { return m_bFailed; }
	bool IsActionPreview() { return m_pActionPreviewCoroutine != NULL; }
	bool IsSnapShot() { return m_bSnapShot; }
	class CMasterLevel* GetMasterLevel();
	CEntity* GetPawnRoot() { return m_pPawnRoot; }
	CLevelEnvEffect* GetEnvEffect() { return m_pEnvEffect; }
	void SetEnvEffect( const char* sz );

	void Begin();
	void End();
	void Fail( int8 nFailType = 0 );
	void Freeze();
	void UnFreeze();
	bool IsFreeze() { return m_nFreeze > 0; }
	CPawn* SpawnPawn( int32 n, int32 x, int32 y, int8 nDir, const char* szRemaining = NULL, CPawn* pCreator = NULL, int32 nForm = 0 );
	CPawn* SpawnPawn1( const char* szPrefab, int32 x, int32 y, int8 nDir, CPawn* pCreator = NULL, int32 nForm = 0 );
	CPawn* SpawnPreset( const char* szName );
	CPawn* SpawnPreset1( const char* szName, int32 x, int32 y, int8 nDir, const char* szInitState = NULL );
	int32 GetPresetSpawnX( const char* szName );
	int32 GetPresetSpawnY( const char* szName );
	bool AddPawn( CPawn* pPawn, const TVector2<int32>& pos, int8 nDir, CPawn* pCreator = NULL, int32 nForm = 0 );
	bool AddPawn1( CPawn* pPawn, int32 nState, int32 nStateTick, const TVector2<int32>& pos, const TVector2<int32>& moveTo, int8 nDir );
	void RemovePawn( CPawn* pPawn );
	bool IsGridMoveable( const TVector2<int32>& p, CPawn* pPawn, int8 nForceCheckType = 0 );
	bool IsGridBlockSight( const TVector2<int32>& p );
	bool PawnMoveTo( CPawn* pPawn, const TVector2<int32>& ofs, int8 nForceCheckType = 0, int32 nMoveFlag = 0, bool bBlockEft = true );
	void PawnMoveEnd( CPawn* pPawn );
	void PawnMoveBreak( CPawn* pPawn, bool bInState = false );
	void PawnDeath( CPawn* pPawn );
	bool PawnTransform( CPawn* pPawn, int32 nForm, const TVector2<int32>& ofs, bool bBlockEft = true );
	bool IsPawnInTile( CPawn* pPawn, int32 nTile );
	CPickUp* FindPickUp( const TVector2<int32>& p, int32 w, int32 h );
	CPlayerMount* FindMount( const TVector2<int32>& ofs );
	CPawn* FindUseablePawn( const TVector2<int32>& p, int8 nDir, int32 w, int32 h );
	CPawn* GetPawnByName( const char* szName );
	int32 GetAllPawnsByNameScript( const char* szName );
	int32 GetAllPawnsByTagScript( const char* szTag );
	CPawn* GetPawnByGrid( int32 x, int32 y );
	void OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir );
	void OnPlayerAction( vector<int8>& vecInput, int32 nMatchLen, int8 nType );
	TVector2<int32> SimpleFindPath( const TVector2<int32>& begin, const TVector2<int32>& end, int32 nCheckFlag,
		vector<TVector2<int32> >* pVecPath = NULL, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
	TVector2<int32> FindPath1( const TVector2<int32>& begin, const TVector2<int32>& end, function<bool( SGrid*, const TVector2<int32>& )> FuncGrid,
		vector<TVector2<int32> >* pVecPath = NULL, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
	TVector2<int32> Search( const TVector2<int32>& begin, function<int8( SGrid*, const TVector2<int32>& )> FuncGrid,
		vector<TVector2<int32> >* pVecPath = NULL, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
	void Alert( CPawn* pTriggeredPawn, const TVector2<int32>& p );
	void Alert1();
	void BeginTracer( const char* sz, int32 nDelay );
	void BeginTracer1( int32 n, int32 nDelay );
	void EndTracer();
	void BlockTracer();
	void SetTracerDelay( int32 n );
	int32 GetTracerDelayLeft() { return m_nTracerDelayLeft; }
	int8 GetTracerSpawnExit() { return m_nTracerSpawnExit; }
	TVector2<int32> GetTracerEnterPos() { return m_tracerEnterPos; }
	void BeginNoise( const char* szSound );
	void EndNoise();
	bool IsNoise() { return m_pNoise != NULL; }

	void BlockStage();
	void BlockExit( int32 n );
	bool IsExitBlocked( int32 n );
	bool IsGridBlockedExit( SGrid* pGrid, bool bIgnoreComplete = false );
	int32 GetGridExit( int32 x, int32 y );
	SLevelNextStageData& GetNextLevelData( int32 n ) { return m_arrNextStage[n]; }
	int32 GetNextLevelCount() { return m_arrNextStage.Size(); }
	int32 FindNextLevelIndex( const char* szLevelName );
	TVector2<int32> GetPlayerEnterPos();
	void Redirect( int32 n, int32 n1 );
	void ReplaceTiles( int32 n0, int32 n1 );

	void BeginScenario();
	void EndScenario();
	bool IsScenario() { return m_bScenario; }
	void GetAllUseableGrid( vector<TVector2<int32> >& result );

	void Init( int8 nType = 0 );
	void Update();
	int32 UpdateActionPreview();
	void ActionPreviewPause();
	void OnCheckPoint();
	bool OnPlayerTryToLeave( const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType, int32 nTransferParam );

	void RegisterBegin( CTrigger* pTrigger );
	void RegisterUpdate( CTrigger* pTrigger ) { m_trigger.Register( 0, pTrigger ); }
	void RegisterUpdate1( CTrigger* pTrigger ) { m_trigger.Register( 1, pTrigger ); }
	void RegisterAlwaysUpdate( CTrigger* pTrigger ) { m_trigger.Register( 2, pTrigger ); }

	void ScriptForEachPawn();
	void ScriptForEachEnemy();
private:
	CPawn* HandleSpawn( CLevelSpawnHelper* pSpawnHelper );
	CPawn* HandleSpawn1( CLevelSpawnHelper* pSpawnHelper, const TVector2<int32>& p, int32 nDir, const char* szInitState = NULL );
	void HandlePawnMounts( CPawn* pPawn, bool bRemove, CEntity* pRoot = NULL );
	void FlushSpawn();
	void InitTiles();
	void InitTile( const TVector2<int32>& p );
	void InitScripts();
	void UpdateActionPreviewFunc();
	int32 m_nWidth, m_nHeight;
	int32 m_nDepth;
	CRectangle m_rectMainArea;
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
	CString m_strBGM;
	int32 m_nBGMPriority;

	bool m_bBegin;
	bool m_bEnd;
	bool m_bScenario;
	bool m_bComplete;
	bool m_bFailed;
	bool m_bFullyEntered;
	bool m_bBlocked;
	int8 m_nFreeze;
	bool m_bStartBattle;
	bool m_bSnapShot;
	vector<CReference<CLevelScript> > m_vecScripts;
	vector<SGrid> m_vecGrid;
	TClassTrigger<CMyLevel> m_onTick;
	CReference<CPlayer> m_pPlayer;
	CReference<CLevelIndicatorLayer> m_pIndicatorLayer;
	CReference<CLevelEnvEffect> m_pEnvEffect;
	map<string, CReference<CLevelEnvEffect> > m_mapEnvEffects;
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
	TVector2<int32> m_tracerEnterPos;
	CReference<CEntity> m_pTracerSpawnEffect;
	CReference<ISoundTrack> m_pNoise;

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
	void ScenarioWaitInput();
	bool IsScenarioWaitInput() { return m_bScenarioWaitInput; }
	void FinishWaitInput() { m_bScenarioWaitInput = false; }
	bool IsScenarioTextFinished();
	bool IsScenarioTextFinished0();
	void HeadText( const char* sz, const CVector4& color, int32 nTime = 0, const char* szSound = "", int32 nSoundInterval = 1, bool bImportant = false );
	void ShowFailEft( bool b );
	void ShowFreezeEft( int32 nLevel );
	void ClearLabels();
	void SetLabel( int32 nIndex, int32 x, int32 y, int32 nCounter = 0 );
	void SetLabelCounter( int32 nIndex, int32 nCounter );

	void Update();
	void UpdateEffect();
	virtual void Render( CRenderContext2D& context ) override;
private:
	void UpdatePos();
	void UpdateInputItem( int32 nItem );
	void UpdateIcons();
	void Effect0();
	void Effect1();
	void RecordEffect();
	void FailEffect( int8 nType0 = 0 );
	void FreezeEffect( int32 nLevel );
	CReference<CEntity> m_pHeadText;
	CReference<CEntity> m_pScenarioText[2];
	CReference<CEntity> m_pFailTips[3];
	CReference<CEntity> m_pIcons[ePlayerEquipment_Count];
	CReference<CEntity> m_pLabelsRoot;
	CReference<CEntity> m_pLabelsCounter;
	CReference<CEntity> m_pAmmoCount;

	CRectangle m_origRect;
	CVector2 m_playerInputOrig;
	CVector2 m_iconOrig;
	CVector2 m_pos0;
	int8 m_nPlayerActionType;
	int32 m_nPlayerActionFrame;
	struct SPlayerActionItem
	{
		int32 nSeed;
		float y0a, y0b;
		float y1a[3], y1b[3];
		CVector4 params[3][2];
	};
	int32 m_nPlayerActionSeed;
	SPlayerActionItem m_playerActionItems[3];
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
	vector<CReference<CEntity> > m_vecLabelCounters;
	bool m_bDrawEffectFirst;
	bool m_bScenario;
	int8 m_nLastScenarioText;
	bool m_bScenarioWaitInput;
	bool m_bResetFreezeEft;
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
		int8 bIsAlive;
		int8 nEmpty;
	};
	struct SLevelData
	{
		SLevelData() : bVisited( false ), bIgnoreGlobalClearKeys( false ) {}
		bool bVisited;
		bool bIgnoreGlobalClearKeys;
		map<string, int32> mapDataInt;
		map<string, string> mapDataString;
		map<string, SPawnData> mapDataDeadPawn;
		void Load( IBufReader& buf, int32 nVersion );
		void Save( CBufFile& buf );
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

	struct SLevelSnapShot
	{
		SLevelData levelData;
		map<string, int32> mapDataInt;
		map<string, string> mapDataString;
		map<string, int32> mapDataIntStatic;
		map<string, string> mapDataStringStatic;
		int8 bValid;

		SLevelSnapShot() : bValid( 0 ) {}
		void Clear()
		{
			bValid = 0;
			mapDataInt.clear();
			mapDataString.clear();
			mapDataIntStatic.clear();
			mapDataStringStatic.clear();
			levelData.mapDataDeadPawn.clear();
			levelData.mapDataInt.clear();
			levelData.mapDataString.clear();
		}
		void Load( IBufReader& buf, int32 nVersion );
		void Save( CBufFile& buf );
	};
	SLevelSnapShot curLevelSnapShot;
	map<string, SWorldDataFrame::SLevelSnapShot> mapClearedSnapShot;

	TVector2<int32> playerEnterPos;
	int8 nPlayerEnterDir;
	bool bForceAllVisible;
	CBufFile playerData;
	CBufFile pawnData;
	string strGlobalBGM;
	int32 nGlobalBGMPriority;
	vector<CBufFile> vecPlayerDataStack;
	set<string> unlockedRegionMaps;
	map<string, SLevelMark> mapLevelMarks;
	string strTracer;
	int32 nTracerSpawnDelay;
	string strTracerLevel;
	TVector2<int32> tracerLevelEnterPos;
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
	map<string, SWorldDataFrame::SLevelSnapShot> mapSnapShotCur;
	map<string, SWorldDataFrame::SLevelSnapShot> mapSnapShotCheckPoint;
	static constexpr int32 nMaxFrameCount = 10;

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );
	const char* GetCurLevel() { return curFrame.strCurLevel.c_str(); }
	SWorldDataFrame::SLevelData& GetLevelData( const char* szLevel ) { return curFrame.mapLevelData[szLevel]; }
	SWorldDataFrame::SLevelData& GetCurLevelData() { return curFrame.mapLevelData[curFrame.strCurLevel]; }
	void OnEnterLevel( const char* szCurLevel, CPlayer* pPlayer, const TVector2<int32>& playerPos, int8 nPlayerDir, vector<CReference<CPawn> >* pVecPawns, bool bClearSnapShot, int8 nPlayerDataOpr = 0 );
	void OnReset( CPlayer* pPlayer, vector<CReference<CPawn> >& vecPawns );
	void OnRetreat( CPlayer* pPlayer, vector<CReference<CPawn> >& vecPawns );
	void CheckPoint( CPlayer* pPlayer );
	void OnRestoreToCheckpoint( CPlayer* pPlayer, vector<CReference<CPawn> >& vecPawns );
	void ClearKeys();
	void Respawn();
	void RespawnLevel( const char* szLevel );
	void ClearByPrefix( const char* sz );
	void SetLevelIgnoreGlobalClearKeys( const char* szLevel, bool b );
	void UnlockRegionMap( const char* szRegion );
	void GetScenarioRecords( function<void( int8, const CVector4&, const char* )> Func );
	void OnScenarioText( int8 n, const char* sz, const CVector4& color );

	template <typename T>
	static void ClearKeys( T& t, vector<string>& vecTemp )
	{
		for( auto& pair : t )
		{
			if( pair.first.find( '%' ) == string::npos )
				vecTemp.push_back( pair.first );
		}
		for( auto& key : vecTemp )
			t.erase( key );
		vecTemp.resize( 0 );
	}
	template <typename T>
	static void ClearKeysByPrefix( T& t, const char* sz, vector<string>& vecTemp )
	{
		auto itr1 = t.lower_bound( sz );
		auto itr2 = t.upper_bound( sz );
		for( ; itr1 != itr2; itr1++ )
		{
			if( itr1->first.find( '%' ) == string::npos )
				vecTemp.push_back( itr1->first );
		}
		for( auto& key : vecTemp )
			t.erase( key );
		vecTemp.resize( 0 );
	}
};

class CMasterLevel : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CMasterLevel( const SClassCreateContext& context ) : CEntity( context ), m_levelTrans( 0, 0, 0, 1 ) { SET_BASEOBJECT_ID( CMasterLevel ); }

	virtual void OnAddedToStage() override;
	void NewGame( CPlayer* pPlayer, CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir );
	void Continue( CPlayer* pPlayer, IBufReader& buf );
	void Save();
	bool TransferTo( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType = 0, int32 nTransferParam = 0 );
	void JumpBack( int8 nType );
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
	int32 EvaluateKeyInt( const char* str ) { return EvaluateKeyIntLevelData( str, GetCurLevelData() ); }
	int32 EvaluateKeyIntLevelData( const char* str, SWorldDataFrame::SLevelData& levelData );
	const char* EvaluateKeyString( const char* str ) { return EvaluateKeyStringLevelData( str, GetCurLevelData() ); }
	const char* EvaluateKeyStringLevelData( const char* str, SWorldDataFrame::SLevelData& levelData );
	void SetKeyInt( const char* str, int32 n ) { SetKeyIntLevelData( str, n, m_worldData.GetCurLevelData() ); }
	void SetKeyIntLevelData( const char* str, int32 n, SWorldDataFrame::SLevelData& levelData );
	void SetKeyString( const char* str, const char* szValue ) { SetKeyStringLevelData( str, szValue, m_worldData.GetCurLevelData() ); }
	void SetKeyStringLevelData( const char* str, const char* szValue, SWorldDataFrame::SLevelData& levelData );
	void ClearKeys() { m_worldData.ClearKeys(); }
	void ClearSnapShot() { m_bClearSnapShot = true; }
	void Respawn() { m_worldData.Respawn(); }
	void RespawnLevel( const char* szLevel ) { m_worldData.RespawnLevel( szLevel ); }
	void ClearByPrefix( const char* sz ) { m_worldData.ClearByPrefix( sz ); }
	void SetLevelIgnoreGlobalClearKeys( const char* szLevel, bool b ) { m_worldData.SetLevelIgnoreGlobalClearKeys( szLevel, b ); }
	void PushPlayerData() { m_nTransferPlayerDataOpr = 1; }
	void PopPlayerData() { m_nTransferPlayerDataOpr = -1; }
	void TransferTo1( CPrefab* pLevelPrefab, const TVector2<int32>& playerPos, int8 nPlayerDir, int8 nTransferType = 0, int32 nTransferParam = 0 );
	void TransferBy( int32 nNxtStage, int8 nTransferType = 0, int32 nTransferParam = 0 );
	void ScriptTransferTo( const char* szName, int32 nPlayerX, int32 nPlayerY, int8 nPlayerDir, int8 nTransferType = 0, int32 nTransferParam = 0 );
	void ScriptTransferOprFunc();
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
	CEntity* GotoInteractionUI( const char* szName );
	CEntity* GetInteractionUI() { return m_pInteractionUI; }
	void BlackOut( int32 nFrame1, int32 nFrame2 );
	bool IsBlackOut() { return m_nBlackOutFrame1 > 0; }
	void InterferenceStripEffect( int8 nType, float fSpeed );
	void SetGlobalBGM( const char* sz, int32 nPriority );
	void OnSetEnvEffect( CLevelEnvEffect* pEffect );

	CVector2 GetCamPos();
	void OnPlayerDamaged();
	void Update();
private:
	int32 CalcSnapShotMaskParam( CVector4* pParams );
	void UpdateMarkLayer();
	void UpdateBackground();
	void UpdateColorAdjust( bool bJump = false );
	void UpdateCtrlPoints( bool bUpdate, float fWeight = 1 );
	void UpdateLevelTrans();
	void UpdateLevelEnvTrans( CMyLevel* pLevel );
	void CheckBGM();
	void UpdateBGM();
	const char* GetCurBGM();
	void UpdateBattleEffect();
	void RefreshSnapShot();
	bool UpdateSnapShot( const char* sz );
	void HideAllSnapShot();
	void RemoveAllSnapShot();

	void ResetMainUI();
	void RefreshMainUI();
	void BeginCurLevel();
	void EndCurLevel();
	void TransferFunc();
	void TransferFuncEnterLevel( vector<CReference<CPawn> >* pTransferPawn = NULL );
	void TransferFuncLevel2Level();
	void TransferFuncLevel2Level0( bool bFade );
	void TransferFuncLevel2Level1();
	void TransferFuncLevel2Level2();
	void TransferFuncLevel2Level3();
	void TransferFuncLevel2Level4_5( int8 bUp );
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
	CReference<CRenderObject2D> m_pBackMask;
	CReference<CEntity> m_pLevelFadeMask;
	CReference<CEntity> m_pSnapShotRoot;
	CReference<CRenderObject2D> m_pBattleEffect;
	CReference<CRenderObject2D> m_pUpsampleLayer;
	CReference<CEntity> m_pMarkLayer;
	CReference<CEntity> m_pMenu;
	CReference<CEntity> m_pMenuItem[5];
	CReference<CRenderObject2D> m_pColorAdjust;
	CReference<CRenderObject2D> m_pMenuSelected;
	CReference<CEntity> m_pWorldMap;
	CReference<CEntity> m_pActionPreview;
	CReference<CEntity> m_pLogUI;
	TResourceRef<CSoundFile> m_pBlackOutSound;
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
	int32 m_nShowSnapShotFrame;
	int32 m_nBattleEffectFrame;
	int32 m_nPlayerDamageFrame;
	map<string, CReference<CMyLevel> > m_mapSnapShot;
	set<string> m_setShowingSnapShot;
	vector<CReference<CRenderObject2D> > m_vecBackScenarioMask;
	struct SMark
	{
		CVector2 p;
	};
	vector<SMark> m_vecMarks;
	vector<CReference<CRenderObject2D> > m_vecMarkImgs;
	bool m_bMarkDirty;
	bool m_bInteractionUIInit;
	CReference<CEntity> m_pInteractionUI;
	CReference<CPawn> m_pInteractionUIPawn;
	string m_strInteractionUI;
	int32 m_nBlackOutFrame1, m_nBlackOutFrame2;
	CReference<CEntity> m_pInterferenceStripEffect;
	CVector4 m_backParam[2];
	struct SColorAdjust
	{
		CVector3 gamma;
		CVector3 colorTranspose[3];
		bool IsZero()
		{
			if( gamma.x != 0 || gamma.y != 0 || gamma.z != 0 )
				return false;
			for( int i = 0; i < 3; i++ )
			{
				if( colorTranspose[i].x != 0 || colorTranspose[i].y != 0 || colorTranspose[i].z != 0 )
					return false;
			}
			return true;
		}
	};
	SColorAdjust m_curColorAdjust;
	CVector4 m_levelTrans;

	class ICoroutine* m_pTransferCoroutine;
	CReference<CPrefab> m_pTransferTo;
	TVector2<int32> m_transferPos;
	int8 m_nTransferDir;
	int8 m_nTransferType;
	bool m_bClearSnapShot;
	int8 m_nTransferPlayerDataOpr;
	int32 m_nTransferParam;
	CVector2 m_transferCurCamPos;
	CReference<CEntity> m_pTransferEft;
	string m_strUpdatingSnapShot;
	/*CVector2 m_transferOfs;
	CVector2 m_camTransferBegin;
	int32 m_nTransferAnimTotalFrames;
	int32 m_nTransferAnimFrames;
	int32 m_nTransferFadeOutTotalFrames;
	int32 m_nTransferFadeOutFrames;*/

	string m_strBGM;
	string m_strFadeOutBGM;
	CReference<ISoundTrack> m_pBGMSoundTrack;
	CReference<ISoundTrack> m_pBGMSoundTrackFadeOut;
	float m_fBGMFadeOut;
	float m_fBGMFadeIn;
	float m_fBGMFadeInSpeed;
	CReference<ISoundTrack> m_pSpecialEftSoundTrack;

	CReference<CLuaState> m_pScenarioScript;
	CReference<CPrefab> m_pScriptTransferTo;
	int32 m_nScriptTransferPlayerX;
	int32 m_nScriptTransferPlayerY;
	int32 m_nScriptTransferPlayerDir;
	int8 m_nScriptTransferType;
	int32 m_nScriptTransferParam;
	CReference<CLuaState> m_pScriptTransferOpr;
};