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
	ePawnStateEventType_PickUp,
	ePawnStateEventType_Drop,
	ePawnStateEventType_Cost,
	ePawnStateEventType_UnMount,
	ePawnStateEventType_SetZ,
	ePawnStateEventType_Sound,
	ePawnStateEventType_Script,
};

struct SPawnStateEvent
{
	SPawnStateEvent( const SClassCreateContext& context ) {}
	EPawnStateEventType eType;
	int32 nTick;
	int32 nParams[4];
	CString strParam;
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
	CString strToName;
	int32 nTo;
	EPawnStateTransitCondition eCondition;
};

struct SPawnStateTransit1
{
	SPawnStateTransit1( const SClassCreateContext& context ) {}
	CString strToName;
	int32 nTo;
	TArray<CString> arrStrExclude;
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
	virtual bool CanCheckAction( bool bScenario ) { return !bScenario; }
	virtual int32 CheckAction( int8& nCurDir ) { return -1; }
};

class CPawnUsage : public CEntity
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPawnUsage( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CEntity ); }

	virtual void Init() {}
	virtual bool CheckUse( class CPlayer* pPlayer ) { return !m_bBeingUsed; }
	const char* GetUseAction() { return m_strUseAction; }

	virtual void BeginUse( class CPlayer* pPlayer ) { m_bBeingUsed = true; m_bUseHit = false; }
	virtual void UseHit( class CPlayer* pPlayer ) { m_bUseHit = true; }
	virtual void EndUse( class CPlayer* pPlayer ) { m_bBeingUsed = false; }
	virtual void Update() {}
private:
	CString m_strUseAction;

	bool m_bBeingUsed;
	bool m_bUseHit;
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
	CString strName;
	int32 nWidth, nHeight;
	int32 nDefaultState;
};

class ISignalObj
{
public:
	virtual int32 Signal( int32 i ) { return 0; }
};

class CLevelSpawnHelper : public CEntity
{
	friend void RegisterGameClasses_BasicElems();
	friend class CMyLevel;
	friend class CPawn;
public:
	CLevelSpawnHelper( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelSpawnHelper ); }
	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
private:
	int8 m_nDataType;
	CString m_strSpawnCondition;
	CString m_strDeathKey;
	int32 m_nDeathState;

	bool m_bSpawnDeath;
};

class CPawn : public CEntity, public ISignalObj
{
	friend class CMyLevel;
	friend class CLevelSpawnHelper;
	friend class CPawnTool;
	friend class CLevelToolsView;
	friend void RegisterGameClasses_BasicElems();
public:
	CPawn( const SClassCreateContext& context ) : CEntity( context ), m_nHp( m_nMaxHp ) { SET_BASEOBJECT_ID( CPawn ); }

	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	virtual void Init();
	virtual void Update();
	virtual void Update1();
	virtual int32 Damage( int32 nDamage, int8 nDamageType = 0 );
	int32 GetWidth() const { return m_nWidth; }
	int32 GetHeight() const { return m_nHeight; }
	class CMyLevel* GetLevel();
	const TVector2<int32>& GetPos() { return m_pos; }
	const TVector2<int32>& GetMoveTo() { return m_moveTo; }
	int32 GetPosX() { return m_pos.x; }
	int32 GetPosY() { return m_pos.y; }
	int32 GetToX() { return m_moveTo.x; }
	int32 GetToY() { return m_moveTo.y; }
	int8 GetCurDir() { return m_nCurDir; }
	int32 GetCurForm() { return m_nCurForm; }
	virtual SPawnState& GetCurState() { return m_arrSubStates[m_nCurState]; }
	int32 GetCurStateIndex() { return m_nCurState; }
	int32 GetCurStateTick() { return m_nCurStateTick; }
	int32 GetStateIndexByName( const char* szName );
	bool IsIgnoreBlockedExit() { return m_bIgnoreBlockedExit; }
	bool IsValidStateIndex( int32 i ) { return i >= 0 && i < m_arrSubStates.Size(); }
	int8 GetArmorType() { return m_nArmorType; }
	int32 GetHp() { return m_nHp; }
	int32 GetMaxHp() { return m_nMaxHp; }
	void SetHp( int32 nHp ) { m_nHp = nHp; }
	virtual SPawnHitSpawnDesc* GetHitSpawn( int32 nHit ) { return nHit >= 0 && nHit < m_arrHitSpawnDesc.Size() ? &m_arrHitSpawnDesc[nHit] : NULL; }
	const SPawnForm& GetForm( int32 n ) { return m_arrForms[n]; }
	CPawnUsage* GetUsage() { return m_pUsage; }
	virtual int32 Signal( int32 i ) override { m_trigger.Trigger( 0, (void*)i ); return 0; }
	void RegisterSignal( CTrigger* pTrigger ) { m_trigger.Register( 0, pTrigger ); }
	void RegisterKilled( CTrigger* pTrigger ) { m_trigger.Register( 1, pTrigger ); }
	bool IsKilled() { return m_nHp <= 0; }
	bool CanBeHit();
	void SetMounted( bool b, bool bMountHide ) { m_bMounted = b; m_bMountHide = b ? bMountHide : false; }
	void SetForceHide( bool bForceHide ) { m_bForceHide = bForceHide; }
	CPrefab* GetDamageEft() { return m_pDamageEft; }

	/*<-------------------For Script----------------------*/
	bool PlayState( const char* sz );
	bool PlayStateTurnBack( const char* sz );
	bool PlayStateSetDir( const char* sz, int8 nDir );
	CPawnAI* ChangeAI( const char* sz );
	const char* GetCurStateName();
	void RegisterSignalScript();
	void RegisterKilledScript();
	/*--------------------For Script--------------------->*/
	CString strCreatedFrom;
	int8 nTempFlag;
protected:
	virtual bool TransitTo( const char* szToName, int32 nTo, int32 nReason );
	virtual bool EnumAllCommonTransits( function<bool( SPawnStateTransit1&, int32 )> Func );
	virtual bool FilterCommonTransit( SPawnStateTransit1& transit, int32 nSource );
	virtual int32 GetDefaultState();
	bool ChangeState( int32 nNewState, bool bInit = false );
	virtual void ChangeState( SPawnState& state, int32 nStateSource, bool bInit );
	virtual void Update0();
	virtual TVector2<int32> OnHit( SPawnStateEvent& evt ) { return TVector2<int32>( 0, 0 ); }
	virtual bool CheckAction();
	bool CheckStateTransits( int32 nDefaultState );
	bool CheckStateTransits1( int32 nDefaultState, bool bFinished );
	virtual bool StateCost( int8 nType, int32 nCount ) { return false; }
	void OnKilled();

	bool m_bIsEnemy;
	bool m_bIgnoreHit;
	bool m_bIgnoreBlockedExit;
	int8 m_nInitDir;
	int8 m_nArmorType;
	int32 m_nWidth, m_nHeight;
	int32 m_nMaxHp;
	TArray<SPawnForm> m_arrForms;
	TArray<SPawnState> m_arrSubStates;
	TArray<SPawnHitSpawnDesc> m_arrHitSpawnDesc;
	TArray<SPawnStateTransit1> m_arrCommonStateTransits;
	CRectangle m_origRect, m_origTexRect;
	CReference<CPawnAI> m_pAI;
	CReference<CPawnUsage> m_pUsage;
	int32 m_nRenderOrder;
	CReference<CRenderObject2D> m_pHpBar;
	CString m_strKillScript;
	TResourceRef<CPrefab> m_pDamageEft;

	CReference<CLevelSpawnHelper> m_pSpawnHelper;
	CReference<CPawn> m_pCreator;
	TVector2<int32> m_pos;
	TVector2<int32> m_moveTo;
	int32 m_nCurForm;
	int8 m_nCurDir;
	bool m_bDamaged;
	bool m_bMounted;
	bool m_bMountHide;
	bool m_bForceHide;
	int32 m_nHp;
	int32 m_nCurState;
	int32 m_nCurStateSource;
	int32 m_nCurStateTick;
	CVector2 m_curStateBeginPos;
	CRectangle m_curStateRect;
	CRectangle m_curStateOrigTexRect;
	CRectangle m_hpBarOrigRect;
	CEventTrigger<2> m_trigger;
	LINK_LIST_REF( CPawn, Pawn );
};

struct SInputTableItem
{
	SInputTableItem( const SClassCreateContext& context ) {}
	CString strInput;
	CString strStateName;
	int32 nStateIndex;
	bool bInverse;
};

struct SStateInputTableItem
{
	SStateInputTableItem( const SClassCreateContext& context ) : input( context ) {}
	TArray<int32> arrStates;
	SInputTableItem input;
};

enum
{
	ePlayerEquipment_Melee,
	ePlayerEquipment_Ranged,
	ePlayerEquipment_Ability,

	ePlayerEquipment_Count,

	ePlayerStateSource_Mount,
	ePlayerStateSource_Count,
};

class CPlayerEquipment : public CEntity
{
	friend void RegisterGameClasses_BasicElems();
	friend class CPlayer;
public:
	CPlayerEquipment( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CPlayerEquipment ); }

	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	void Init();
	void Drop( class CPlayer* pPlayer, const TVector2<int32>& pos, int8 nDir );
	void PrePickedUp( CPawn* pPickUp );

	int32 GetAmmo() { return m_nAmmo; }
	int32 GetMaxAmmo() { return m_nMaxAmmo; }
	void SetAmmo( int32 n ) { m_nAmmo = n; }
	int32 GetIcon() { return m_nIcon; }
	int32 GetAmmoIconWidth() { return m_nAmmoIconWidth; }

	void LoadData( IBufReader& buf );
	void SaveData( CBufFile& buf );
private:
	int8 m_nEquipType;
	int32 m_nAmmo, m_nMaxAmmo;
	int32 m_nIcon;
	int32 m_nAmmoIconWidth;
	TArray<SPawnState> m_arrSubStates;
	TArray<SPawnHitSpawnDesc> m_arrHitSpawnDesc;
	TArray<SPawnStateTransit1> m_arrCommonStateTransits;
	TArray<SInputTableItem> m_inputTable;
	TArray<SStateInputTableItem> m_stateInputTable;
	CRectangle m_origRect, m_origTexRect;
	CReference<CRenderObject2D> m_pEft[2];

	CReference<CPawn> m_pPickUp;
	CReference<CRenderObject2D> m_pOrigRenderObject;
};

class CPlayerMount : public CEntity
{
	friend void RegisterGameClasses_BasicElems();
	friend class CMyLevel;
public:
	CPlayerMount( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CPlayerMount ); }

	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	void Init();
	bool IsEnabled();
	void SetEnabled( bool bEnabled ) { m_bDisabled = !bEnabled; }
	int8 GetEnterDir() { return m_nEnterDir; }
	bool CheckMount( class CPlayer* pPlayer );
	void Mount( class CPlayer* pPlayer );
	CPawn* GetPawn();
private:
	bool m_bDisabled;
	bool m_bAnimPlayerOriented;
	bool m_bShowPawnOnMount;
	int8 m_nEnterDir;
	int32 m_nOfsX, m_nOfsY;
	CReference<CPlayerEquipment> m_pEquipment;
	CString m_strEntryState;
	int32 m_nNeedStateIndex;

	TVector2<int32> m_levelPos;
	LINK_LIST_REF( CPlayerMount, Mount )
};

#define ACTION_EFT_FRAMES 4
class CPlayer : public CPawn
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPlayer( const SClassCreateContext& context ) : CPawn( context ) { SET_BASEOBJECT_ID( CPlayer ); }

	void Reset();
	void LoadData( IBufReader& buf );
	void SaveData( CBufFile& buf );
	virtual void Init() override;
	virtual void Update() override;

	virtual int32 Damage( int32 nDamage, int8 nDamageType ) override;
	bool TryPickUp();
	bool TryDrop();
	void Equip( CPlayerEquipment* pEquipment );
	void UnEquip( CPlayerEquipment* pEquipment );
	CPlayerEquipment* GetEquipment( int8 n ) { return m_pCurEquipment[n]; }
	void Mount( CPawn* pPawn, CPlayerEquipment* pMount, const char* szState, bool bAnimPlayerOriented, bool bMountHide );
	void UnMount();
	bool IsMounting() { return m_pCurMount != NULL; }
	virtual SPawnState& GetCurState() override;
	virtual SPawnHitSpawnDesc* GetHitSpawn( int32 nHit ) override;
	SInputTableItem* GetCurInputResult();
	void EnableDefaultEquipment();
	void RestoreAmmo();

	void SetInputSequence( const char* szInput );
	vector<int8>& ParseInputSequence();
protected:
	virtual bool TransitTo( const char* szToName, int32 nTo, int32 nReason );
	virtual bool EnumAllCommonTransits( function<bool( SPawnStateTransit1&, int32 )> Func ) override;
	virtual void ChangeState( SPawnState& state, int32 nStateSource, bool bInit ) override;
	virtual void Update0() override;
	virtual bool CheckAction() override;
	bool HandleInput();
	bool CheckInputTableItem( SInputTableItem& item );
	bool ExecuteInputtableItem( SInputTableItem& item, int32 nStateSource );
	void FlushInput( int32 nMatchLen, int8 nType );
	virtual bool StateCost( int8 nType, int32 nCount ) override;
	CPlayerEquipment* GetStateSource( int8 nType );
	TArray<SInputTableItem> m_inputTable;
	TArray<SStateInputTableItem> m_stateInputTable;
	CVector2 m_actionEftOfs[ACTION_EFT_FRAMES * 2];
	CVector4 m_actionEftParam[ACTION_EFT_FRAMES * 2];
	CReference<CRenderObject2D> m_pEft[2];
	CReference<CPlayerEquipment> m_pDefaultEquipment;

	bool m_bActionStop;
	bool m_bEnableDefaultEquipment;
	int32 m_nTickInputOnActionStop;
	CReference<CPlayerEquipment> m_pCurEquipment[ePlayerEquipment_Count];
	CReference<CRenderObject2D> m_pOrigRenderObject;
	CReference<CRenderObject2D> m_pCurEft[2];
	CReference<CRenderObject2D> m_pNewRenderObject;
	CReference<CRenderObject2D> m_pNewEft[2];
	vector<int8> m_vecInputQueues;
	vector<int8> m_parsedInputSequence;
	CReference<CPlayerEquipment> m_pCurStateSource;
	CReference<CPawn> m_pCurUsingPawn;
	CReference<CPlayerEquipment> m_pCurMount;
	CReference<CPawn> m_pCurMountingPawn;
	bool m_bMountAnimPlayerOriented;
	int8 m_nDirBeforeMounting;
	int8 m_nActionEftFrame;
	/*int8 m_nMoveXInput;
	int8 m_nMoveYInput;
	int8 m_nAttackInput;*/
};

struct SHitGridDesc
{
	SHitGridDesc( const SClassCreateContext& context ) {}
	int32 nHitIndex;
	int32 nOfsX, nOfsY;
	int32 nDamage;
	int8 nDamageType;
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

	bool m_bBeamStarted;
	int32 m_nBeamTick;
	int32 m_nBeamLen;
	CReference<CRenderObject2D> m_pBeamImg;
};

class CPickUp : public CPawnHit
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPickUp( const SClassCreateContext& context ) : CPawnHit( context ) { SET_BASEOBJECT_ID( CPickUp ); }
	virtual void OnPreview() override;
	virtual void Init() override;
	bool IsPickUpReady();
	void PickUp( CPlayer* pPlayer );
	void PreDrop( CPlayerEquipment* pEquipment ) { m_pEquipment = pEquipment; m_bDropped = true; }
protected:
	virtual int32 GetDefaultState() override;
	CReference<CPlayerEquipment> m_pEquipment;
	CString m_strScript;

	bool m_bDropped;
};