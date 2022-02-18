#pragma once
#include "Player.h"
#include "Bug.h"
#include "Common/StringUtil.h"
#include "Common/PriorityQueue.h"
#include "Common/Coroutine.h"
#include "Entities/AIObject.h"
#include "Render/DrawableGroup.h"

struct SLevelCamCtrlPoint
{
	SLevelCamCtrlPoint( const SClassCreateContext& context ) {}
	int8 nPointType;
	int8 nResetType;
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

enum ELevelCamCtrlPointSmootherType
{
	eLevelCamCtrlPointSmoother_IgnoreSmallForce,
};

struct SLevelCamCtrlPointSmoother
{
	SLevelCamCtrlPointSmoother( const SClassCreateContext& context ) {}
	int32 n;
	ELevelCamCtrlPointSmootherType nType;
	CVector4 params[2];
};

enum ELevelCamCtrlPoint1LimitorType
{
	eLevelCamCtrlPoint1Limitor_Rect,
};

struct SLevelCamCtrlPoint1Limitor
{
	SLevelCamCtrlPoint1Limitor( const SClassCreateContext& context ) {}
	int32 n1, n2;
	ELevelCamCtrlPoint1LimitorType nType;
	CVector4 params[2];
};

class CLightArea : public CCharacter
{
	friend void RegisterGameClasses_Level();
public:
	CLightArea( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CLightArea ); }

	virtual void OnRemovedFromStage() override;
	float GetRad() { return m_fRad; }
	float GetRad1() { return m_fRad1; }
	virtual void PostUpdate() override;
private:
	float m_fRad, m_fRad1;

	bool m_bAdded;
	CVector2 m_lastLightPos;
};

class CBlackRegion : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CBlackRegion( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CBlackRegion ); }

	void Init();
	virtual bool IsPreview() { return true; }
	virtual void OnPreview();
	float GetGridSize() { return 32.0f; }

	void PostUpdate( CPlayerCross* pPlayer, bool bPlayerAttached );
	void OnPlayerAttach( CPlayerCross* pPlayer ) {}
	void OnPlayerDetach( CPlayerCross* pPlayer ) {}
	void UpdateLight( CLightArea* pLightArea, const CVector2& p, bool bAdd );
	virtual void Render( CRenderContext2D& context ) override;
	void UpdateImages( const CRectangle& viewRect );
private:
	void UpdateCharIndicators( class CMyLevel* pLevel, const CVector2& cursorPos, const CRectangle& viewRect );
	void AddImage( const CRectangle& rect, const CRectangle& texRect, const CVector4& param );
	void AddImage1( const CRectangle& rect, const CVector4& param );

	bool m_bInited;
	TRectangle<int32> m_size;
	struct SGrid
	{
		int16 nLight;
		bool bExplored;
		TVector2<int32> tex;
		TVector2<int32> texSpeed;
	};
	vector<SGrid> m_vecGrid;
	vector<CElement2D> m_vecElems;
	vector<CVector4> m_vecParams;
	int8 m_nShowType;

	CReference<CCharacter> m_pLastTarget;
	CRectangle m_targetRect[2];
};

class CLevelEnvLayer : public CEntity
{
	friend class CLevelEnvLayerEdit;
	friend class CLevelToolsView;
	friend void RegisterGameClasses_Level();
public:
	CLevelEnvLayer( const SClassCreateContext& context ) : CEntity( context ), m_ctrlPoint1( context ), m_ctrlPoint2( context )
	{ SET_BASEOBJECT_ID( CLevelEnvLayer ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void InitCtrlPoints();
	void InitCtrlPointsState( float x, float y, float r, float s, bool bNoReset = false );
	void UpdateCtrlPoints();
	CVector4 GetCtrlPointsTrans();
	void ApplyForce( int32 nIndex, int8 nType, float fForceX, float fForceY, int32 nDuration, int8 nFadeType );
	int32 GetFadeTime() { return m_nFadeTime; }
private:
	SLevelCamCtrlPoint m_ctrlPoint1, m_ctrlPoint2;
	TArray<SLevelCamCtrlPoint> m_arrCtrlPoint;
	TArray<SLevelCamCtrlPointLink> m_arrCtrlLink;
	TArray<SLevelCamCtrlPointSmoother> m_arrCtrlSmoother;
	TArray<SLevelCamCtrlPoint1Limitor> m_arrCtrlLimitor;
	int32 m_nFadeTime;

	class CMyLevel* m_pLevel;
	bool m_bCtrlPointsInited;
	struct SCtrlPointState
	{
		int8 nPointType;
		CVector2 p;
		CVector2 v;
		CVector2 f1;
		CVector2 f2;
		int32 nCurFrame;
		vector<CVector2> vecFrames;
		CElement2D elemDebugDraw;
		CVector4 debugDrawParam[2];
	};
	CVector2 GetCtrlPointCurPos( SCtrlPointState& point );
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
};

class CLevelObjLayer : public CEntity, public ILevelObjLayer
{
	friend void RegisterGameClasses_Level();
public:
	CLevelObjLayer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelObjLayer ); }
	virtual bool GetBound( CRectangle& rect ) const override { return false; }
	virtual void Init() override {}
	virtual void InitFromTemplate( CEntity* p, const CRectangle& rect ) override {}
	virtual void Update() override {}
	virtual void UpdateScroll( const CVector4& camTrans ) override {}
	virtual bool IsPreview() { return true; }
	virtual void OnPreview();
};

class CLevelBugIndicatorLayer : public CEntity, public ILevelObjLayer
{
	friend void RegisterGameClasses_Level();
public:
	CLevelBugIndicatorLayer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelBugIndicatorLayer ); }
	virtual void OnAddedToStage() override;
	virtual bool IsPreview() { return true; }
	virtual void OnPreview();
	virtual bool GetBound( CRectangle& rect ) const override { return false; }
	virtual void Init() override {}
	virtual void InitFromTemplate( CEntity* p, const CRectangle& rect ) override { m_texRect = ( (CLevelBugIndicatorLayer*)p )->m_texRect; }
	virtual void Update() override;
	virtual void UpdateScroll( const CVector4& camTrans ) override {}
	virtual void Render( CRenderContext2D& context ) override;
private:
	void UpdateImg( int32 i, const CVector2& origPos, const CVector4& color );
	CRectangle m_texRect;

	vector<CElement2D> m_vecElems;
	vector<CVector4> m_vecColors;
};

class CPortal : public CCharacter
{
	friend void RegisterGameClasses_Level();
public:
	CPortal( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CPortal ); }
	bool CheckTeleport( CCharacter* pPlayer );
	virtual void OnTickAfterHitTest() override;
private:
	bool m_bUp;
};

struct SBugLink
{
	SBugLink( const SClassCreateContext& context ) {}
	int32 a, b;
	TArray<int32> arrPath;
};

class CMyLevel : public CEntity
{
	friend void RegisterGameClasses_Level();
	friend class CLevelBugIndicatorLayer;
	friend class CLevelToolsView;
	friend class CMasterLevel;
public:
	CMyLevel( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMyLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual bool IsPreview() { return true; }
	virtual void OnPreview();

	void Init();
	void Begin();
	void End();
	void PlayerEnter( CPlayerCross* pPlayerCross, CCharacter* pControlled );
	void PlayerLeave();
	void OnAddCharacter( CCharacter* p );
	void OnRemoveCharacter( CCharacter* p );
	bool IsBegin() { return m_bBegin; }
	CPlayerCross* GetPlayer() { return m_pPlayer; }
	CCharacter* GetControlled() { return m_pControlled; }

	CEntity* GetStartPoint() { return m_pStartPoint; }
	int32 GetLevelZ() const { return m_nLevelZ; }
	CRectangle GetSize() const { return m_size; }
	void SetSize( const CRectangle& size ) { m_size = size; }
	CVector2 GetGravityDir();
	bool CheckOutOfBound( CEntity* p );
	CBlackRegion* GetBlackRegion() { return m_pBlackRegion; }
	CLevelEnvLayer* GetEnv() { return m_pCurEnvLayer; }
	void ChangeToEnvLayer( CLevelEnvLayer* pEnv );
	uint8 GetUpdatePhase() { return m_nUpdatePhase; }
	float GetDeltaTime();
	int32 GetDeltaTick() { return m_nDeltaTick; }
	int32 GetSlowMotionScale() { return T_SCL; }
	float GetPlayerPickRad() { return 64.0f; }

	CEntity* Pick( const CVector2& pos, int8 nGroup = 0 );
	void MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result, int8 nGroup = 0 );
	CEntity* DoHitTest( SHitProxy* pProxy, const CMatrix2D& transform, bool hitTypeFilter[eEntityHitType_Count], SHitTestResult* pResult = NULL, int8 nGroup = 0 );
	void MultiHitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CReference<CEntity> >& result, vector<SHitTestResult>* pResult = NULL, int8 nGroup = 0 );
	CEntity* Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType = eEntityHitType_Count, SRaycastResult* pResult = NULL, int8 nGroup = 0 );
	void MultiRaycast( const CVector2& begin, const CVector2& end, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult = NULL, int8 nGroup = 0 );
	CEntity* SweepTest( SHitProxy* pHitProxy, const CMatrix2D& trans, const CVector2& sweepOfs, float fSideThreshold, EEntityHitType hitType = eEntityHitType_Count, SRaycastResult* pResult = NULL, bool bIgnoreInverseNormal = false, int8 nGroup = 0 );
	CEntity* SweepTest( SHitProxy* pEntity, const CMatrix2D& trans, const CVector2& sweepOfs, float fSideThreshold, EEntityHitType hitType, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult* pResult = NULL, bool bIgnoreInverseNormal = false, int8 nGroup = 0 );
	CEntity* SweepTest( CEntity* pEntity, const CMatrix2D& trans, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult* pResult = NULL, bool bIgnoreInverseNormal = false, int8 nGroup = 0 );
	void MultiSweepTest( SHitProxy* pHitProxy, const CMatrix2D& trans, const CVector2& sweepOfs, float fSideThreshold, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult = NULL, int8 nGroup = 0 );
	CHitTestMgr& GetHitTestMgr( int8 nGroup ) { return m_hitTestMgr[nGroup]; }
	bool CheckTeleport( CCharacter* pPlayer, const CVector2& transferOfs );
	float Push( CCharacter* pCharacter, const CVector2& dir, float fDist );
	float Push( CCharacter* pCharacter, CCharacter::SPush& context, const CVector2& dir, float fDist, int32 nTested, CEntity** pTested, CMatrix2D* matTested, float fSideThreshold );

	CCharacter* FindAttach();
	CCharacter* TryAttach( CCharacter* pChar0 );
	void TryDetach();

	void OnBugDetected( CBug* pBug );
	void ResetBug( CBug* pBug );
	void CheckBugs( bool bTest );
	struct SEditorBugListItem
	{
		CReference<CPrefabNode> p;
		CReference<CPrefabNode> par;
		vector<int32> vecPath;
	};
	void EditorFixBugListLoad( vector<SEditorBugListItem>& vecAllBugs );
	void EditorFixBugListSave( vector<SEditorBugListItem>& vecAllBugs );

	void Update();
	void Update1();
	int8 GetUpdateType() { return m_nUpdateType; }
	static CMyLevel* GetEntityLevel( CEntity* pEntity );
	static CEntity* GetEntityRootInLevel( CEntity* pEntity );
	static CCharacter* GetEntityCharacterRootInLevel( CEntity* pEntity, bool bFindResetable = false );
private:
	void DisableLights( CRenderObject2D* p );
	void BuildBugList();
	void ScanBug( CEntity* p );
	int32 m_nLevelZ;
	CRectangle m_size;
	TArray<SBugLink> m_arrBugLink;
	CReference<CEntity> m_pStartPoint;
	CReference<CBlackRegion> m_pBlackRegion;
	CReference<CLevelEnvLayer> m_pCurEnvLayer;
	CReference<CLevelBugIndicatorLayer> m_pBugIndicatorLayer;

	int32 m_nDeltaTick;
	int32 m_nTick0;
	CHitTestMgr m_hitTestMgr[2];
	CReference<CPlayerCross> m_pPlayer;
	CReference<CCharacter> m_pControlled;
	int8 m_nUpdateType;
	uint8 m_nUpdatePhase;
	bool m_bInited;
	bool m_bBegin;
	bool m_bEnd;

	vector<CReference<CEntity> > m_vecAllLayers;
	struct SBugListItem
	{
		SBugListItem() : nFirstChild( -1 ), nNxtSib( -1 ), nParent( -1 ), bDetected( false ) {}
		CReference<CBug> pBug;
		CVector2 origPos;
		int32 nGroup;
		int32 nFirstChild;
		int32 nNxtSib;
		int32 nParent;
		bool bDetected;
		CReference<CEntity> pParentLinkEft;
	};
	bool m_bBugListReady;
	vector<SBugListItem> m_vecBugListItems;
	map<int32, TVector2<int32> > m_mapBugListGroupRange;

	TClassTrigger<CMyLevel> m_onTick;

	LINK_LIST_REF_HEAD_ARR( m_pCharacters, 2, CCharacter, Character )
};

class CMasterLevel : public CEntity
{
	friend void RegisterGameClasses_Level();
public:
	CMasterLevel( const SClassCreateContext& context ) : CEntity( context ), m_camTrans( 0, 0, 0, 1 ) { SET_BASEOBJECT_ID( CMasterLevel ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void NewGame();
	void TransferTo( const char* szNewLevel, int8 nTransferType, int32 nTransferParam );
	void OpenTestConsole();
	CMyLevel* CreateLevel( CPrefab* pPrefab );
	void CreateCurLevel();
	void BeginCurLevel();
	void EndCurLevel();
	void Update();
	void Kill();
	float GetKillFade() { return m_nKillTickLeft * 1.0f / m_nKillTick; }
	CCharacter* GetPlayer() { return m_pPlayer; }
	CMyLevel* GetCurLevel() { return m_pCurLevel; }
	CVector2 GetCtrlPointFollowPlayerPos( const CVector2& p );
	CVector4 GetCamTrans();
	void OnAttached() {}
	void OnDetached() {}
	enum
	{
		eTest_None,

		eTest_Static,
		eTest_Scan_0,
		eTest_Scan_1,
		eTest_Scan_2,
	};
	int32 GetTestState();
	CRectangle GetTestRect();
	CRectangle GetTestRect( int32 nState, int8 nDir, const CVector2& orig );
	void BeginTest( int32 nType, int8 nDir );
	void UpdateTest();
	void EndTest();
	void BeginAlert( const CRectangle& rect, const CVector2& vel );
	void UpdateAlert();
	void EndAlert();
	bool IsAlert() { return m_bAlert; }
	CRectangle GetAlertRect() { return m_bAlert ? m_alertRect : CRectangle( 0, 0, 0, 0 ); }
	
	const char* CheckTeleport( bool bUp );
	bool CheckTeleport( const char* sz, int8 bUp );
	void TryTeleport( bool bUp ) { m_bTryTeleport = true; m_bTeleportUp = bUp; }

	static CMasterLevel* GetInst() { return s_pInst; }
private:
	CLevelEnvLayer* CurEnv();
	void UpdateCtrlPoints( float fWeight );
	void BufferLevels();
	void TransferFunc();
	void TestConsoleFunc();
	void UpdateMasks( float fFade );
	void UpdateTestMasks( int32 nType, int8 nDir, const CVector2& orig, float fEnterTest );
	CVector4 m_dmgParam[3];
	CVector4 m_testConsoleParam[3];
	CVector4 m_testParam[3];
	CString m_strBeginLevel;
	TResourceRef<CPrefab> m_pPlayerPrefab;
	CReference<CRenderObject2D> m_pLayer1;
	int32 m_nKillTick;
	CRectangle m_testRect[4];
	float m_fTestScanSpeed[3];
	CVector2 m_backOfsScale;
	CReference<CEntity> m_pLevelFadeMask;
	CReference<CLevelEnvLayer> m_pDefaultEnvLayer;
	CReference<CEntity> m_pMask1;
	CReference<CEntity> m_pMask2;
	CReference<CEntity> m_pBack;
	CReference<CEntity> m_pTestUI;
	CReference<CRenderObject2D> m_pTestUIItems[5];
	CReference<CEntity> m_pTestPlayerCross;
	TResourceRef<CSoundFile> m_pSoundFileAlert;

	CVector4 m_camTrans;
	CVector2 m_camTransPlayerOfs;
	int32 m_nKillTickLeft;
	CReference<CPlayerCross> m_pPlayer;
	CReference<CMyLevel> m_pCurLevel;
	CReference<CMyLevel> m_pLastLevel;
	CReference<CPrefab> m_pCurLevelPrefab;
	CReference<CPrefab> m_pLastLevelPrefab;
	CReference<CLevelEnvLayer> m_pCurEnvLayer;
	class ICoroutine* m_pTransferCoroutine;
	class ICoroutine* m_pTestConsoleCoroutine;
	bool m_bTryTeleport;
	bool m_bTeleportUp;
	int8 m_nTransferType;
	CReference<CPrefab> m_pTransferTo;
	CVector2 m_transferOfs;
	int32 m_nTransferParam;
	int32 m_nTestState;
	CVector2 m_testOrig;
	int8 m_nTestDir;
	CVector4 m_maskParams[3];
	bool m_bAlert;
	CRectangle m_alertRect;
	CVector2 m_alertVel;
	CReference<ISoundTrack> m_pAlertSound;

	map<string, CReference<CPrefab> > m_mapBufferedLevelPrefabs;
	map<string, CReference<CMyLevel> > m_mapBufferedLevels;
	static CMasterLevel* s_pInst;
};