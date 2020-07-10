#pragma once
#include "BasicElems.h"
#include "MyLevel.h"

class CLevelScriptCustom : public CLevelScript, public ISignalObj
{
	friend void RegisterGameClasses_MiscElem();
public:
	CLevelScriptCustom( const SClassCreateContext& context ) : CLevelScript( context ) { SET_BASEOBJECT_ID( CLevelScriptCustom ); }

	virtual void OnInit( CMyLevel* pLevel ) override;
	virtual void OnBegin( CMyLevel* pLevel ) override;
	virtual void OnDestroy( CMyLevel* pLevel ) override;
	virtual void OnUpdate( CMyLevel* pLevel ) override;
	virtual void OnUpdate1( CMyLevel* pLevel ) override;
	virtual void OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir ) override;
	virtual void OnPlayerAction( int32 nMatchLen, int8 nType ) override;
	virtual int32 Signal( int32 i ) override;
private:
	CString m_strInit;
	CString m_strBegin;
	CString m_strDestroy;
	CString m_strUpdate;
	CString m_strUpdate1;
	CString m_strPlayerChangeState;
	CString m_strPlayerAction;
	CString m_strSignal;
};

class CCommonLink : public CEntity
{
	friend void RegisterGameClasses_MiscElem();
public:
	CCommonLink( const SClassCreateContext& context ) : CEntity( context )
		, m_onBegin( this, &CCommonLink::Begin ), m_onUpdate( this, &CCommonLink::Update )
		, m_onSrcKilled( this, &CCommonLink::OnKilled ), m_onDstKilled( this, &CCommonLink::OnKilled ) { SET_BASEOBJECT_ID( CCommonLink ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void Begin();
	void Update();
	void OnKilled();
	TObjRef<CEntity> m_pSrc;
	TObjRef<CEntity> m_pDst;
	CVector2 m_srcOfs;
	CVector2 m_dstOfs;
	int32 m_nEftInterval;
	int32 m_nEftLife;
	float m_fEftStrength;
	TResourceRef<CPrefab> m_pLightningPrefab;

	int32 m_nTick;
	TClassTrigger<CCommonLink> m_onBegin;
	TClassTrigger<CCommonLink> m_onUpdate;
	TClassTrigger<CCommonLink> m_onSrcKilled;
	TClassTrigger<CCommonLink> m_onDstKilled;
	CReference<ISoundTrack> m_pSound;
};

class CPawnUsageCommon : public CPawnUsage
{
	friend void RegisterGameClasses_MiscElem();
public:
	CPawnUsageCommon( const SClassCreateContext& context ) : CPawnUsage( context ) { SET_BASEOBJECT_ID( CPawnUsageCommon ); }
	virtual void UseHit( class CPlayer* pPlayer ) override;
private:
	int8 m_nType;
};

class CPawnUsageButton : public CPawnUsage
{
	friend void RegisterGameClasses_MiscElem();
public:
	CPawnUsageButton( const SClassCreateContext& context ) : CPawnUsage( context ) { SET_BASEOBJECT_ID( CPawnUsageButton ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override { m_pTarget = NULL; CPawnUsage::OnRemovedFromStage(); }
	virtual void UseHit( class CPlayer* pPlayer ) override;
	virtual void Update() override;
private:
	TObjRef<CEntity> m_pTarget;
	int32 m_nSignal;
	int32 m_nEftFrames0;
	CVector4 m_eftParam0;
	int32 m_nEftFrames1;
	CVector4 m_eftParam1;

	CVector4 m_param0;
	bool m_bEftFramesType;
	int32 m_nEftFramesLeft;
};

class CPawnAIAutoDoor : public CPawnAI
{
	friend void RegisterGameClasses_MiscElem();
public:
	CPawnAIAutoDoor( const SClassCreateContext& context ) : CPawnAI( context ) { SET_BASEOBJECT_ID( CPawnAIAutoDoor ); }
	virtual bool CanCheckAction( bool bScenario ) override { return true; }
	virtual int32 CheckAction( int8& nCurDir ) override;
};

class CConsole : public CPawn
{
	friend void RegisterGameClasses_MiscElem();
public:
	CConsole( const SClassCreateContext& context ) : CPawn( context ) { SET_BASEOBJECT_ID( CConsole ); }
	virtual void Update() override;
	virtual int32 Signal( int32 i ) override;
private:
	void RunDefault();
	CString m_strDefaultScript;
	CString m_strExtraScript;

	CReference<CLuaState> m_pDefault;
	CReference<CLuaState> m_pExtra;
};

class CFallPoint : public CPawnHit
{
	friend void RegisterGameClasses_MiscElem();
public:
	CFallPoint( const SClassCreateContext& context ) : CPawnHit( context ) { SET_BASEOBJECT_ID( CFallPoint ); }
	virtual void Init() override;
	virtual void Update() override;
private:
	int32 m_nNxtStage;
	CString m_strKey;
	TResourceRef<CPrefab> m_pEft;
	CString m_strSound;

	bool m_bVisited;
};

class CClimbPoint : public CPawnHit
{
	friend void RegisterGameClasses_MiscElem();
public:
	CClimbPoint( const SClassCreateContext& context ) : CPawnHit( context ) { SET_BASEOBJECT_ID( CClimbPoint ); }
	virtual void Init() override;
	virtual int32 Signal( int32 i ) override;
private:
	virtual int32 GetDefaultState() override;
	int32 m_nNxtStage;
	CString m_strKey;
	CReference<CPlayerMount> m_pMount[2];

	bool m_bReady;
};

class CSmoke : public CPawnHit
{
	friend void RegisterGameClasses_MiscElem();
public:
	CSmoke( const SClassCreateContext& context ) : CPawnHit( context ) { SET_BASEOBJECT_ID( CSmoke ); }
	virtual void Init() override;
	virtual void Update() override;
	virtual void Render( CRenderContext2D& context ) override;
	virtual void UpdateRendered( double dTime ) override;
	virtual void OnPreview() override;
private:
	void InitImages();
	void UpdateImages();
	bool m_bPreview;

	CVector4 m_origParam[2];
	float m_t;
	float m_fAnimSpeed;
	float m_fSplitOfs[2];
	struct SItem
	{
		CVector2 tex;
		float fTexSpeed;
		CVector2 ofs;
	};
	SItem m_items[3];
	CElement2D m_elems[3];
	CVector4 m_params[6];
};

class CElevator : public CLevelScript, public ISignalObj
{
	friend void RegisterGameClasses_MiscElem();
public:
	CElevator( const SClassCreateContext& context ) : CLevelScript( context ) { SET_BASEOBJECT_ID( CElevator ); }
	virtual void OnAddedToStage() override;
	virtual void OnInit( CMyLevel* pLevel ) override;
	virtual void OnUpdate1( CMyLevel* pLevel ) override;
	virtual int32 Signal( int32 i ) override;
private:
	void OnCurFloorChanged();
	int32 m_nRedirect;
	int32 m_nFloorBegin, m_nFloorCount;
	int32 m_nEftFrames;
	CVector4 m_eftParam;
	CVector4 m_invalidParam;

	CRectangle m_origTexRect;
	CVector4 m_param0;
	int32 m_nEftFramesLeft;
	CMyLevel* m_pLevel;
	int32 m_nCurFloor;
};

class CProjector : public CLevelScript, public ISignalObj
{
	friend void RegisterGameClasses_MiscElem();
public:
	CProjector( const SClassCreateContext& context ) : CLevelScript( context ) { SET_BASEOBJECT_ID( CProjector ); }
	virtual void OnAddedToStage() override { if( m_p ) m_p->bVisible = false; if( m_p1 ) m_p1->bVisible = false; }
	virtual void OnUpdate1( CMyLevel* pLevel ) override;
	void SetTarget( const CVector2& target );
	bool IsActivated();
	CVector2 GetProjSrc() { return m_p1->GetPosition(); }
	virtual int32 Signal( int32 i ) override;
private:
	int8 m_nFixedTarget;
	CVector2 m_projTargetOfs;
	TArray<CVector2> m_arrFixedTargets;
	TArray<float> m_arrProjPos;
	CReference<CEntity> m_p;
	CReference<CEntity> m_p1;

	bool m_bSetTarget;
	CVector2 m_target;
};

class CTutorialMoving : public CLevelScript, public ISignalObj
{
	friend void RegisterGameClasses_MiscElem();
public:
	CTutorialMoving( const SClassCreateContext& context ) : CLevelScript( context ) { SET_BASEOBJECT_ID( CTutorialMoving ); }
	virtual void OnAddedToStage() override { m_p->bVisible = m_p1->bVisible = false; }
	virtual void OnUpdate1( CMyLevel* pLevel ) override;
	virtual int32 Signal( int32 i ) override;
private:
	CReference<CEntity> m_p;
	CReference<CEntity> m_p1;
	CReference<CEntity> m_pImgs[8];
	CReference<CRenderObject2D> m_pImg1;
	CReference<CEntity> m_pImg2[3];

	bool m_bStarted;
	int8 m_nState;
	int32 m_nStateTick;
};

class CTutorialFollowing : public CLevelScript, public ISignalObj
{
	friend void RegisterGameClasses_MiscElem();
public:
	CTutorialFollowing( const SClassCreateContext& context ) : CLevelScript( context ) { SET_BASEOBJECT_ID( CTutorialFollowing ); }
	virtual void OnUpdate1( CMyLevel* pLevel ) override;
	virtual void OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir ) override;
	virtual void OnPlayerAction( int32 nMatchLen, int8 nType ) override;
	virtual int32 Signal( int32 i ) override;
private:
	void Succeed();
	void Fail( const TVector2<int32>& pos );
	void UpdateStep();
	CRenderObject2D* CreateImg();
	CRenderObject2D* CreateErrImg();
	void UpdateStateImg();
	bool m_bNoStop;
	int8 m_nMaxShowStepCount;
	int32 m_nBeginX, m_nBeginY;
	CString m_str;
	CString m_strStepScript;
	CString m_strFinishedScript;
	CString m_strFailedScript;
	TResourceRef<CPrefab> m_pPrefab;
	TResourceRef<CPrefab> m_pFailEffect;
	CReference<CProjector> m_pProjector;
	TObjRef<CEntity> m_pStateImg;

	bool m_bEnabled;
	bool m_bError;
	int8 m_nState;
	int32 m_nCurStep;
	int32 m_nCurShowStep;
	int32 m_nEndTick;
	int32 m_nErrorTick;
	TVector2<int32> m_curPos;
	vector<TVector2<int32> > m_vecError;
	vector<CReference<CRenderObject2D> > m_vecImgs;
	CReference<CEntity> m_pFailEftObj;
};