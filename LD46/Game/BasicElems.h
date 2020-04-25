#pragma once
#include "Entity.h"

enum EPawnStateEventType
{
	ePawnStateEventType_CheckAction,
	ePawnStateEventType_MoveBegin,
	ePawnStateEventType_MoveEnd,
	ePawnStateEventType_Hit,
	ePawnStateEventType_Death,
	ePawnStateEventType_Transform,
};

struct SPawnStateEvent
{
	SPawnStateEvent( const SClassCreateContext& context ) {}
	EPawnStateEventType eType;
	int32 nTick;
	int32 nParams[4];
};

enum EPawnStateTransitCondition
{
	ePawnStateTransitCondition_Finish,
	ePawnStateTransitCondition_Break,
	ePawnStateTransitCondition_Hit,
	ePawnStateTransitCondition_Killed,
};

struct SPawnStateTransit
{
	SPawnStateTransit( const SClassCreateContext& context ) {}
	int32 nTo;
	EPawnStateTransitCondition eCondition;
};

struct SPawnStateTransit1
{
	SPawnStateTransit1( const SClassCreateContext& context ) {}
	int32 nTo;
	TArray<int32> arrExclude;
	EPawnStateTransitCondition eCondition;
};

struct SPawnState
{
	SPawnState( const SClassCreateContext& context ) {}
	CString strName;
	int32 nForm;
	int32 nTotalTicks;
	int32 nTicksPerFrame;
	int32 nImgExtLeft, nImgExtRight, nImgExtTop, nImgExtBottom;
	int32 nImgTexBeginX, nImgTexBeginY;
	int32 nImgTexCols, nImgTexCount;
	TArray<SPawnStateEvent> arrEvts;
	TArray<SPawnStateTransit> arrTransits;
};

class CPawnAI : public CEntity
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPawnAI( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CPawnAI ); }
	virtual int32 CheckAction( int8& nCurDir ) { return -1; }
};

struct SPawnHitSpawnDesc
{
	SPawnHitSpawnDesc( const SClassCreateContext& context ) {}
	TResourceRef<CPrefab> pHit;
	int32 nOfsX, nOfsY;
	int8 nDir;
};

struct SPawnForm
{
	SPawnForm( const SClassCreateContext& context ) {}
	int32 nWidth, nHeight;
	int32 nDefaultState;
};

class CPawn : public CEntity
{
	friend class CMyLevel;
	friend void RegisterGameClasses_BasicElems();
public:
	CPawn( const SClassCreateContext& context ) : CEntity( context ), m_nHp( m_nMaxHp ) { SET_BASEOBJECT_ID( CPawn ); }

	virtual void Init();
	virtual void Update();
	virtual void Update1();
	void Damage( int32 nDamage );
	int32 GetWidth() { return m_nWidth; }
	int32 GetHeight() { return m_nHeight; }
	const TVector2<int32>& GetPos() { return m_pos; }
	const TVector2<int32>& GetMoveTo() { return m_moveTo; }
	int32 GetCurForm() { return m_nCurForm; }
	SPawnState& GetCurState() { return m_arrSubStates[m_nCurState]; }
	int32 GetHp() { return m_nHp; }

	const SPawnForm& GetForm( int32 n ) { return m_arrForms[n]; }
protected:
	int32 GetDefaultState();
	virtual void ChangeState( int32 nNewState, bool bInit = false );
	virtual TVector2<int32> OnHit( SPawnStateEvent& evt ) { return TVector2<int32>( 0, 0 ); }
	virtual int32 CheckAction();

	bool m_bIsEnemy;
	int32 m_nWidth, m_nHeight;
	int32 m_nMaxHp;
	TArray<SPawnForm> m_arrForms;
	TArray<SPawnState> m_arrSubStates;
	TArray<SPawnHitSpawnDesc> m_arrHitSpawnDesc;
	TArray<SPawnStateTransit1> m_arrCommonStateTransits;
	CRectangle m_origRect, m_origTexRect;
	CReference<CPawnAI> m_pAI;
	int32 m_nRenderOrder;
	CReference<CRenderObject2D> m_pHpBar;

	CReference<CPawn> m_pCreator;
	TVector2<int32> m_pos;
	TVector2<int32> m_moveTo;
	int32 m_nCurForm;
	int8 m_nCurDir;
	bool m_bDamaged;
	int32 m_nHp;
	int32 m_nCurState;
	int32 m_nCurStateTick;
	TVector2<int32> m_curStateBeginPos;
	CRectangle m_curStateRect;
	CRectangle m_hpBarOrigRect;
	LINK_LIST_REF( CPawn, Pawn );
};

#define ACTION_EFT_FRAMES 4
class CPlayer : public CPawn
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPlayer( const SClassCreateContext& context ) : CPawn( context ) { SET_BASEOBJECT_ID( CPlayer ); }

	virtual void Update() override;
protected:
	virtual void ChangeState( int32 nNewState, bool bInit = false ) override;
	virtual int32 CheckAction() override;
	CVector2 m_actionEftOfs[ACTION_EFT_FRAMES * 2];
	CVector4 m_actionEftParam[ACTION_EFT_FRAMES * 2];
	CReference<CRenderObject2D> m_pEft[2];

	int8 m_nMoveXInput;
	int8 m_nMoveYInput;
	int8 m_nAttackInput;
	int8 m_nActionEftFrame;
};

struct SHitGridDesc
{
	SHitGridDesc( const SClassCreateContext& context ) {}
	int32 nHitIndex;
	int32 nOfsX, nOfsY;
	int32 nDamage;
	int32 nFlag;
};

class CPawnHit : public CPawn
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPawnHit( const SClassCreateContext& context ) : CPawn( context ) { SET_BASEOBJECT_ID( CPawnHit ); }
	virtual void Update() override;
protected:
	virtual TVector2<int32> OnHit( SPawnStateEvent& evt ) override;
	void UpdateBeam();
	int8 m_nHitType;
	int32 m_nHitParam[2];
	TArray<SHitGridDesc> arrGridDesc;
	CRectangle m_beamRect, m_beamTexRect;
	int32 m_nBeamTotalTime;
	int32 m_nBeamTickPerFrame;
	int32 m_nBeamTexCount;
	TResourceRef<CPrefab> m_pDamageEft;

	bool m_bBeamStarted;
	int32 m_nBeamTick;
	int32 m_nBeamLen;
	CReference<CRenderObject2D> m_pBeamImg;
};
