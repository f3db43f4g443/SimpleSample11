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
	ePawnStateEventType_JumpTo,
	ePawnStateEventType_SpecialState,
	ePawnStateEventType_Interaction,
	ePawnStateEventType_Script,
};

struct SPawnStateEvent
{
	SPawnStateEvent( EPawnStateEventType eType, int32 nTick, int32 nParams[4], const char* szParam )
	: eType( eType ), nTick( nTick ), strParam( szParam ) { for( int i = 0; i < 4; i++ ) this->nParams[i] = nParams[i]; }
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

	ePawnStateTransitReason_JumpTo,
};

struct SPawnStateTransit
{
	SPawnStateTransit( const SClassCreateContext& context ) {}
	CString strToName;
	int32 nTo;
	bool bInverse;
	EPawnStateTransitCondition eCondition;
	CString strCondition;
};

struct SPawnStateTransit1
{
	SPawnStateTransit1( const SClassCreateContext& context ) {}
	CString strToName;
	int32 nTo;
	bool bInverse;
	TArray<CString> arrStrExclude;
	TArray<int32> arrExclude;
	EPawnStateTransitCondition eCondition;
	CString strCondition;
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
	TArray<CString> arrTags;
};

struct SMapIconData
{
	SMapIconData( const SClassCreateContext& context ) {}
	CVector2 ofs;
	CString strTag;
	CString strCondition;
	int32 nConditionValue;
	TArray<CString> arrFilter;
	bool bKeepSize;
	int8 nTexX, nTexY;
	int8 nDir;
};

class CPawnAI : public CEntity
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPawnAI( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CPawnAI ); }
	virtual void PreInit() {}
	virtual void OnInit() {}
	virtual void OnUpdate() {}
	virtual void OnUpdate0() {}
	virtual void OnUpdate1() {}
	virtual void OnRemovedFromLevel() {}
	virtual void OnLevelEnd() {}
	virtual bool OnPlayerTryToLeave() { return true; }
	virtual bool CanCheckAction( bool bScenario ) { return !bScenario; }
	virtual int32 CheckAction( int8& nCurDir ) { return -1; }
	virtual bool CheckAction1( CString& strState, int8& nCurDir ) { return false; }
	virtual int32 CheckStateTransits( int8& nCurDir ) { return -1; }
	virtual int32 CheckStateTransits1( int8& nCurDir, bool bFinished ) { return -1; }
	virtual void OnChangeState() {}
	virtual bool Damage( int32& nDamage, int8 nDamageType, const TVector2<int32>& damageOfs, class CPawn* pSource ) { return false; }
	virtual void Block( const TVector2<int32>& damageOfs ) {}
	virtual bool PreKill() { return true; }
	virtual bool IsIgnoreBlockStage() { return false; }
	virtual void TryPickUp() {}
	virtual void OnSetMounted( bool bMounted ) {}
	virtual int32 Signal( int32 i ) { return 0; }
	virtual TVector2<int32> HandleStealthDetect() { return TVector2<int32>( -1, -1 ); }
	virtual void HandleAlert( class CPawn* pTrigger, const TVector2<int32>& p ) {}
	virtual int32 GetIntValue( const char* szKey ) { return 0; }
	virtual void SetIntValue( const char* szKey, int32 nValue ) {}
	virtual void CreateIconData( CPrefabNode* pNode, const char* szCondition0, TArray<SMapIconData>& arrData ) const {}
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
	CString strInitState;
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
	friend class CPawnTool;
public:
	CLevelSpawnHelper( int8 nSpawnIndex, const char* szDeathKey, int32 nDeathState, int8 nDataType = 1 ) : CEntity(), m_nSpawnType( 0 ), m_nSpawnIndex( nSpawnIndex ), m_nDataType( nDataType ),
		m_strSpawnCondition( "" ), m_strDeathKey( szDeathKey ), m_nDeathState( nDeathState ), m_bSpawnDeath( false ), m_bInitState( false )
	{ SET_BASEOBJECT_ID( CLevelSpawnHelper ); }
	CLevelSpawnHelper( const SClassCreateContext& context ) : CEntity( context ), m_nSpawnIndex( -1 ) { SET_BASEOBJECT_ID( CLevelSpawnHelper ); }
	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	int8 GetSpawnType() const { return m_nSpawnType; }
	const CString& GetSpawnCondition() const { return m_strSpawnCondition; }
	const CString& GetDeathKey() const { return m_strDeathKey; }
private:
	int8 m_nSpawnType;
	int8 m_nDataType;
	bool m_bSpawnDeath;
	int32 m_nSpawnParam[2];
	CString m_strSpawnCondition;
	CString m_strDeathKey;
	int32 m_nDeathState;

	bool m_bInitState;
	int8 m_nSpawnIndex;
	int32 m_nStateParam[2];
	int32 m_nInitState;
	int32 m_nInitStateTick;
};

struct SInputTableItem
{
	SInputTableItem( const SClassCreateContext& context ) {}
	CString strInput;
	CString strStateName;
	int32 nStateIndex;
	CString strCharge;
	bool bInverse;
	int8 nActionGroup;

	struct SInputItr
	{
		SInputItr( const SInputTableItem& item ) : sz( item.strInput.c_str() ),  szCondition( NULL ), l( 0 ), lCondition( 0 ) {}
		const char* sz;
		const char* szCondition;
		int32 l;
		int32 lCondition;
		int32 Next();
	};
};

struct SStateInputTableItem
{
	SStateInputTableItem( const SClassCreateContext& context ) : input( context ) {}
	TArray<int32> arrStates;
	SInputTableItem input;
};

class CPawn : public CEntity, public ISignalObj
{
	friend class CMyLevel;
	friend class CMasterLevel;
	friend class CLevelSpawnHelper;
	friend class CPawnTool;
	friend class CLevelToolsView;
	friend void RegisterGameClasses_BasicElems();
public:
	CPawn( const SClassCreateContext& context ) : CEntity( context ), m_nCurStateCheckAction( -1 ), m_nHp( m_nMaxHp ) { SET_BASEOBJECT_ID( CPawn ); }

	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	virtual void LoadData( IBufReader& buf );
	virtual void SaveData( CBufFile& buf );
	virtual void Init();
	virtual void Update();
	virtual void Update1();
	bool IsActionPreview();
	void OnRemovedFromLevel();
	void OnLevelEnd();
	void OnLevelSave();
	bool OnPlayerTryToLeave();
	void UpdateAnimOnly();
	void ScriptDamage( int32 nDamage ) { Damage( nDamage ); }
	virtual int32 Damage( int32 nDamage, int8 nDamageType = 0, TVector2<int32> damageOfs = TVector2<int32>( 0, 0 ), CPawn* pSource = NULL );
	void SetDamaged( int8 nType, int32 ofsX, int32 ofsY );
	virtual void Block( TVector2<int32> damageOfs = TVector2<int32>( 0, 0 ) );
	int32 GetWidth() const { return m_nWidth; }
	int32 GetHeight() const { return m_nHeight; }
	class CMyLevel* GetLevel();
	CPawn* GetCreator() { return m_pCreator; }
	const TVector2<int32>& GetPos() { return m_pos; }
	const TVector2<int32>& GetMoveTo() { return m_moveTo; }
	TVector2<int32> GetCurStateDest( int32 nTick = 0 );
	bool IsIconOnly() const { return m_bIconOnly; }
	int32 GetPosX() { return m_pos.x; }
	int32 GetPosY() { return m_pos.y; }
	int32 GetToX() { return m_moveTo.x; }
	int32 GetToY() { return m_moveTo.y; }
	int32 GetCurStateDestX( int32 nTick = 0 ) { return GetCurStateDest( nTick ).x; }
	int32 GetCurStateDestY( int32 nTick = 0 ) { return GetCurStateDest( nTick ).y; }
	bool IsPosHidden() { return m_bPosHidden; }
	bool IsToHidden() { return m_bMoveToHidden; }
	bool IsEnemy() { return m_bIsEnemy; }
	bool IsDynamic() { return m_bIsDynamic; }
	bool IsAutoBlockStage();
	bool HasTag( const char* sz );
	const char* GetTag( const char* sz );
	int8 GetCurDir() { return m_nCurDir; }
	int32 GetCurForm() { return m_nCurForm; }
	int8 GetInitDir() const { return m_nInitDir; }
	int32 GetRenderOrder() { return m_nRenderOrder; }
	virtual SPawnState& GetCurState() { return m_arrSubStates[m_nCurState]; }
	int32 GetCurStateIndex() { return m_nCurState; }
	virtual int32 GetCurStateSource() { return 0; }
	int32 GetCurStateTick() { return m_nCurStateTick; }
	int32 GetStateIndexByName( const char* szName ) const;
	void SetInitState( int32 nState ) { m_bUseInitState = true; m_nInitState = nState; }
	void SetDefaultState( int32 nState ) { m_bUseDefaultState = true; m_nDefaultState = nState; }
	bool IsIgnoreBlockedExit() { return m_bIgnoreBlockedExit; }
	bool IsNextStageBlock() { return m_bNextStageBlock; }
	bool IsValidStateIndex( int32 i ) { return i >= 0 && i < (int32)m_arrSubStates.Size(); }
	int8 GetArmorType() { return m_nArmorType; }
	int32 GetHp() { return m_nHp; }
	int32 GetMaxHp() { return m_nMaxHp; }
	void SetHp( int32 nHp ) { m_nHp = nHp; }
	virtual SPawnHitSpawnDesc* GetHitSpawn( int32 nHit ) { return nHit >= 0 && nHit < m_arrHitSpawnDesc.Size() ? &m_arrHitSpawnDesc[nHit] : NULL; }
	const SPawnForm& GetForm( int32 n ) { return m_arrForms[n]; }
	CPawnUsage* GetUsage() { return m_pUsage; }
	virtual int32 Signal( int32 i ) override;
	void RegisterSignal( CTrigger* pTrigger ) { m_trigger.Register( 0, pTrigger ); }
	void RegisterKilled( CTrigger* pTrigger ) { m_trigger.Register( 1, pTrigger ); }
	void RegisterChangeState( CTrigger* pTrigger ) { m_trigger.Register( 2, pTrigger ); }
	bool IsKilled() { return m_nMaxHp > 0 && m_nHp <= 0; }
	int32 CheckHit( const TVector2<int32>& p, int8 nDamageType );
	bool CanBeHit( int8 nDamageType );
	bool IsBlockSight() { return m_bBlockSight; }
	bool IsIgnoreBullet() { return m_bIgnoreBullet; }
	void SetMounted( bool b, bool bMountHide, int8 nMoveType );
	void SetForceHide( bool bForceHide ) { m_bForceHide = bForceHide; }
	bool IsMounted() { return m_bMounted; }
	bool IsMountHide() { return m_bMountHide; }
	CPrefab* GetDamageEft() { return m_pDamageEft; }
	bool IsDamaged() { return m_bDamaged; }
	int8 GetDamageType() { return m_nDamageType; }
	int8 GetDamageOfsX() { return m_damageOfs.x; }
	int8 GetDamageOfsY() { return m_damageOfs.y; }
	int8 GetDamageOfsDir();
	int8 GetDamageOfsDir1( int32 x, int32 y );
	void SetHitSize( int32 w, int32 h ) { m_nHitWidth = w; m_nHitHeight = h; }
	virtual TArray<SInputTableItem>* GetControllingInputTable() { return NULL; }
	virtual TArray<SStateInputTableItem>* GetControllingStateInputTable() { return NULL; }
	void StateTransit( const char* szToName, int32 nTo, int8 nDir ) { m_nCurDir = nDir; TransitTo( szToName, nTo, -1 ); }
	bool HandleHit( SPawnStateEvent& evt );
	virtual TVector2<int32> HandleStealthDetect();
	virtual void HandleAlert( CPawn* pTrigger, const TVector2<int32>& p );
	enum
	{
		eSpecialState_Fall,
		eSpecialState_Frenzy,
		eSpecialState_Block,

		eSpecialState_Effect_Shake,

		eSpecialState_Count,
	};
	void IncSpecialState( int32 n ) { m_nSpecialState[n]++; }
	void DecSpecialState( int32 n ) { m_nSpecialState[n]--; }
	bool IsSpecialState( int32 n ) { return m_nSpecialState[n] > 0 || m_nCurStateSpecialState[n] > 0; }
	bool HasStateTag( const char* sz );
	CVector2 GetHpBarOfs();
	void ResetState();
	bool IsLocked() { return m_bLocked; }
	void SetLocked( bool b ) { m_bLocked = b; }

	/*<-------------------For Script----------------------*/
	bool PlayState( const char* sz, int8 nType = 0 );
	bool PlayStateTurnBack( const char* sz, int8 nType = 0 );
	bool PlayStateSetDir( const char* sz, int8 nDir, int8 nType = 0 );
	bool PlayStateForceMove( const char* sz, int32 x, int32 y, int8 nDir, int8 nType = 0 );
	CPawnAI* GetAI() { return m_pAI; }
	CPawnAI* ChangeAI( const char* sz );
	const char* GetCurStateName();
	void RegisterSignalScript();
	void RegisterKilledScript();
	/*--------------------For Script--------------------->*/
	bool IsHideInEditor() const { return m_bHideInEditor; }
	virtual void CreateIconData( CPrefabNode* pNode, const char* szCondition0, TArray<SMapIconData>& arrData ) const;
	bool ChangeState( int32 nNewState, bool bInit = false );
	CLevelSpawnHelper* GetSpawnHelper() { return m_pSpawnHelper; }
	void SetSpawnHelper( CLevelSpawnHelper* p ) { m_pSpawnHelper = p; }
	void AutoCreateSpawnHelper();
	void SetTracerEffectDisabled( bool bDisabled );

	CString strCreatedFrom;
	int8 nTempFlag;
protected:
	virtual void InitState();
	bool CheckTransitCondition( EPawnStateTransitCondition eCondition, const char* strCondition );
	virtual bool TransitTo( const char* szToName, int32 nTo, int32 nReason );
	virtual bool EnumAllCommonTransits( function<bool( SPawnStateTransit1&, int32 )> Func );
	virtual bool FilterCommonTransit( SPawnStateTransit1& transit, int32 nSource );
	virtual int32 GetDefaultState();
	virtual bool ChangeState( SPawnState& state, int32 nStateSource, bool bInit );
	virtual void Update0();
	virtual bool IsCurStateInterrupted() { return false; }
	virtual TVector2<int32> OnHit( SPawnStateEvent& evt ) { return TVector2<int32>( 0, 0 ); }
	virtual bool CheckAction( int32 nGroup );
	virtual bool CheckCanFinish() { return true; }
	bool CheckStateTransits( int32 nDefaultState, int8 nBreakFlag );
	bool CheckStateTransits1( int32 nDefaultState, bool bFinished );
	virtual bool StateCost( int8 nType, int32 nCount ) { return false; }
	void OnKilled();

	bool m_bIsEnemy;
	bool m_bIsDynamic;
	bool m_bBlockSight;
	bool m_bIgnoreBullet;
	bool m_bForceHit;
	bool m_bIgnoreBlockedExit;
	bool m_bNextStageBlock;
	bool m_bHideInEditor;
	int8 m_nInitDir;
	int8 m_nArmorType;
	bool m_bUseInitState;
	bool m_bUseDefaultState;
	int8 m_nLevelDataType;
	bool m_bIconOnly;
	int32 m_nInitState;
	int32 m_nDefaultState;
	int32 m_nWidth, m_nHeight;
	int32 m_nHitWidth, m_nHitHeight;
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
	int8 m_nMapIconType;
	bool m_bMapIconKeepSize[2];
	int8 m_nMapIconTexX[2], m_nMapIconTexY[2];
	CString m_strMapIconTag[2];
	TArray<CString> m_arrTags;

	CReference<CLevelSpawnHelper> m_pSpawnHelper;
	CReference<CPawn> m_pCreator;
	TVector2<int32> m_pos;
	TVector2<int32> m_moveTo;
	TVector2<int32> m_curStateOrigPos;
	int32 m_nCurForm;
	bool m_bPosHidden;
	bool m_bMoveToHidden;
	int8 m_nCurDir;
	bool m_bDamaged;
	int8 m_nDamageType;
	bool m_bMounted;
	bool m_bMountHide;
	bool m_bForceHide;
	bool m_bCurStateDirty;
	bool m_bTracerEffectDisabled;
	bool m_bLocked;
	int8 m_nCurStateCheckAction;
	TVector2<int32> m_damageOfs;
	int32 m_nHp;
	int32 m_nCurState;
	int32 m_nCurStateSource;
	int32 m_nCurStateTick;
	int32 m_nCurStateRenderOrder;
	CVector2 m_curStateBeginPos;
	CRectangle m_curStateRect;
	CRectangle m_curStateOrigTexRect;
	CRectangle m_hpBarOrigRect;
	int32 m_nSpecialState[eSpecialState_Count];
	int32 m_nCurStateSpecialState[eSpecialState_Count];
	CEventTrigger<3> m_trigger;
	LINK_LIST_REF( CPawn, Pawn );
};

enum
{
	ePlayerEquipment_Melee,
	ePlayerEquipment_Ranged,
	ePlayerEquipment_Ability,

	ePlayerEquipment_Large,
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
	void Drop( class CPlayer* pPlayer, const TVector2<int32>& pos, int8 nDir, int32 nPickupState = -1 );
	void PrePickedUp( CPawn* pPickUp );
	class CPickUp* GetPickUp();

	int8 GetEquipmentType() { return m_nEquipType; }
	const CString& GetEquipmentName() { return m_strEquipmentName; }
	int8 GetAmmoType() { return m_nAmmoType; }
	int32 GetAmmo() { return m_nAmmo; }
	int32 GetMaxAmmo() { return m_nMaxAmmo; }
	void SetAmmo( int32 n ) { m_nAmmo = n; }
	int32 GetIcon() { return m_nIcon; }
	int32 GetAmmoIconWidth() { return m_nAmmoIconWidth; }

	void LoadData( IBufReader& buf );
	void SaveData( CBufFile& buf );
private:
	CString m_strEquipmentName;
	int8 m_nEquipType;
	int8 m_nAmmoType;
	int32 m_nAmmo, m_nMaxAmmo;
	int32 m_nIcon;
	int32 m_nAmmoIconWidth;
	TArray<SPawnState> m_arrSubStates;
	TArray<SPawnHitSpawnDesc> m_arrHitSpawnDesc;
	TArray<SPawnStateTransit1> m_arrCommonStateTransits;
	TArray<SInputTableItem> m_inputTable;
	TArray<SStateInputTableItem> m_stateInputTable;
	CRectangle m_origRect, m_origTexRect;
	CString m_strOnBlock;
	CReference<CRenderObject2D> m_pEft[2];
	CReference<CRenderObject2D> m_pSpecialRenderObject;

	CReference<CPawn> m_pPickUp;
	CReference<CRenderObject2D> m_pOrigRenderObject;
};

class CPlayerMount : public CEntity
{
	friend void RegisterGameClasses_BasicElems();
	friend class CMyLevel;
public:
	CPlayerMount( const SClassCreateContext& context ) : CEntity( context ), m_levelPos( -1, -1 ) { SET_BASEOBJECT_ID( CPlayerMount ); }

	virtual bool IsPreview() override { return true; }
	virtual void OnPreview() override;
	void Init();
	bool IsEnabled();
	bool IsHidden() { return m_bHidden; }
	bool IsEnablePreview() { return m_bEnablePreview; }
	void SetEnabled( bool bEnabled ) { m_bDisabled = !bEnabled; }
	int8 GetEnterDir() { return m_nEnterDir; }
	bool CheckMount( class CPlayer* pPlayer );
	void Mount( class CPlayer* pPlayer );
	CPlayerEquipment* GetEquipment();
	const CString& GetCostEquipment() { return m_strCostEquipment; }
	int8 GetCostEquipmentType() { return m_nCostEquipType; }
	int32 GetNeedEquipmentCharge() { return m_nNeedEquipCharge; }
	CPawn* GetPawn();
	TVector2<int32> GetPawnOfs() { return TVector2<int32>( m_nPawnOfsX, m_nPawnOfsY ); }
private:
	bool m_bDisabled;
	bool m_bHidden;
	bool m_bAnimPlayerOriented;
	bool m_bUseMountRenderOrder;
	bool m_bShowPawnOnMount;
	bool m_bNeedLevelComplete;
	bool m_bEnablePreview;
	int8 m_nEnterDir;
	int8 m_nCostEquipType;
	int32 m_nOfsX, m_nOfsY;
	int32 m_nPawnOfsX, m_nPawnOfsY;
	CReference<CPlayerEquipment> m_pEquipment;
	CString m_strEntryState;
	CString m_strCostEquipment;
	int32 m_nNeedStateIndex;
	int32 m_nNeedEquipCharge;

	TVector2<int32> m_levelPos;
	LINK_LIST_REF( CPlayerMount, Mount )
};

#define ACTION_EFT_FRAMES 4
class CPlayer : public CPawn
{
	friend void RegisterGameClasses_BasicElems();
	friend class CActionPreview;
public:
	CPlayer( const SClassCreateContext& context ) : CPawn( context ) { SET_BASEOBJECT_ID( CPlayer ); }

	virtual void OnRemovedFromStage() override;
	virtual void OnPreview() override;
	void Reset( int8 nFlag );
	virtual void LoadData( IBufReader& buf ) override;
	virtual void SaveData( CBufFile& buf ) override;
	virtual void Init() override;
	virtual void Update() override;
	void UpdateInputOnly();
	CPlayer* InitActionPreviewLevel( class CMyLevel* pLevel, const TVector2<int32>& pos );

	virtual int32 Damage( int32 nDamage, int8 nDamageType, TVector2<int32> hitOfs = TVector2<int32>( 0, 0 ), CPawn* pSource = NULL ) override;
	virtual void Block( TVector2<int32> damageOfs = TVector2<int32>( 0, 0 ) ) override;
	int8 TryPickUp( int32 nParam );
	bool TryDrop( int8 nType = 0, int32 nPickupState = -1 );
	bool TryDropIndex( int32 nIndex, int32 nPickupState = -1 );
	void Drop( int32 n );
	void DropAll();
	void Equip( CPlayerEquipment* pEquipment );
	void UnEquip( CPlayerEquipment* pEquipment, int32 nPickupState = -1 );
	CPlayerEquipment* GetEquipment( int8 n ) { return m_pCurEquipment[n]; }
	const char* GetEquipmentName( int8 n );
	void Mount( CPawn* pPawn, CPlayerEquipment* pMount, const char* szState, bool bAnimPlayerOriented, bool bMountHide, bool bUseMountRenderOrder, bool bMountEnablePreview );
	void UnMount( const char* szAction = "", int8 nActionDirType = 0, int8 nMoveType = 0 );
	void ForceUnMount();
	CPawn* GetCurMountingPawn() { return m_pCurMountingPawn; }
	bool IsReadyForMount( CPlayerMount* pMount );
	bool IsReadyToUse() { return !m_pCurMount && !m_pCurEquipment[ePlayerEquipment_Large]; }
	virtual SPawnState& GetCurState() override;
	virtual int32 GetCurStateSource() override { return m_nCurStateSource; }
	virtual SPawnHitSpawnDesc* GetHitSpawn( int32 nHit ) override;
	SInputTableItem* GetCurInputResult();
	void EnableDefaultEquipment();
	void DisableDefaultEquipment();
	bool HasEquipment( int8 n, const char* szName );
	void RemoveEquipment( int8 n );
	bool RestoreAmmo( int8 nAmmoType = 0, int32 nMaxAmmo = 0 );
	void BeginControl( CPawn* pPawn );
	void EndControl();
	bool ControllingPawnCheckAction( int32 nActionGroup );
	bool ControllingPawnCheckStateInput( int32 nReason );
	CPawn* GetControllingPawn() { return m_pControllingPawn; }
	void BeginStealth( int32 nMaxValue );
	void CancelStealth();
	int32 GetStealthValue() { return m_nStealthValue; }
	void UpdateStealthValue( int32 n );
	bool IsHidden() { return m_nStealthValue > 0 || m_bPosHidden || m_bMoveToHidden; }
	bool IsRealPlayer() { return m_bIsRealPlayer; }

	void SetInputSequence( const char* szInput );
	static void SetInputSequence( const char* szInput, vector<int8>& result );
	vector<int8>& ParseInputSequence();
protected:
	virtual void InitState();
	virtual bool TransitTo( const char* szToName, int32 nTo, int32 nReason );
	virtual bool EnumAllCommonTransits( function<bool( SPawnStateTransit1&, int32 )> Func ) override;
	virtual bool ChangeState( SPawnState& state, int32 nStateSource, bool bInit ) override;
	virtual void Update0() override;
	virtual bool IsCurStateInterrupted() override;
	virtual bool CheckAction( int32 nGroup ) override;
	virtual bool CheckCanFinish() override;
	bool HandleInput( int32 nActionGroup );
	int32 CheckInputTableItem( SInputTableItem& item );
	const char* CheckInputTableItem1( SInputTableItem& item, int32& len );
	bool ExecuteInputtableItem( SInputTableItem& item, int32 nStateSource );
	void FlushInput( int32 nMatchLen, int8 nChargeKey, int8 nType );
	virtual bool StateCost( int8 nType, int32 nCount ) override;
	void ActionPreviewAddInputItem( int8 nType, SInputTableItem* pItem );
	bool ActionPreviewWaitInput( bool bJumpTo );
	void ActionPreviewInput( int8 nType, int32 nIndex );

	CPlayerEquipment* GetStateSource( int8 nType );
	bool m_bIsRealPlayer;
	CString m_strScriptDamaged;
	CString m_strScriptInputOverflow;
	TArray<SInputTableItem> m_inputTable;
	TArray<SStateInputTableItem> m_stateInputTable;
	CVector2 m_actionEftOfs[ACTION_EFT_FRAMES * 2];
	CVector4 m_actionEftParam[ACTION_EFT_FRAMES * 2];
	CReference<CRenderObject2D> m_pEft[2];
	CReference<CPlayerEquipment> m_pDefaultEquipment;

	bool m_bActionStop;
	int8 m_nCurActionGroup;
	bool m_bEnableDefaultEquipment;
	uint8 m_nChargeKeyDown;
	int32 m_nStealthValue;
	int32 m_nMaxStealthValue;
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
	CReference<CPawn> m_pControllingPawn;
	bool m_bMountAnimPlayerOriented;
	bool m_bUseMountRenderOrder;
	bool m_bForceUnMount;
	bool m_bMountEnablePreview;
	int8 m_nDirBeforeMounting;
	int8 m_nActionEftFrame;

	int8 m_nActionPreviewType;
	vector<SInputTableItem*> m_vecActionPreviewInputItem[ePlayerStateSource_Count];
	int32 m_nActionPreviewIndex;
	CString m_strActionPreviewCharge;
	string m_strDelayedChargeInput;
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
	int8 nHitType;
	int8 nDamageType;
	int32 nFlag;
};

class CPawnHit : public CPawn
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPawnHit( const SClassCreateContext& context ) : CPawn( context ) { SET_BASEOBJECT_ID( CPawnHit ); }
	const TVector2<int32>& GetHitOfs() { return m_hitOfs; }
	void SetHitOfs( const TVector2<int32>& ofs ) { m_hitOfs = ofs; }
protected:
	virtual TVector2<int32> OnHit( SPawnStateEvent& evt ) override;
	int8 m_nHitType;
	int32 m_nHitParam[2];
	TArray<SHitGridDesc> arrGridDesc;
	TResourceRef<CPrefab> m_pBeamPrefab[3];

	TVector2<int32> m_hitOfs;
};

class CPickUp : public CPawnHit
{
	friend void RegisterGameClasses_BasicElems();
public:
	CPickUp( const SClassCreateContext& context ) : CPawnHit( context ) { SET_BASEOBJECT_ID( CPickUp ); }
	virtual void OnPreview() override;
	virtual void LoadData( IBufReader& buf ) override;
	virtual void SaveData( CBufFile& buf ) override;
	virtual void Init() override;
	virtual void Update() override;
	CPlayerEquipment* GetEquipment() { return m_pEquipment; }
	bool IsPickUpReady();
	bool PickUp( CPlayer* pPlayer );
	bool PickUp1( CPlayer* pPlayer );
	void PreDrop( CPlayerEquipment* pEquipment, int32 nDropState ) { m_pEquipment = pEquipment; SetInitState( nDropState ); m_bDropped = true; }
protected:
	virtual int32 GetDefaultState() override;
	CReference<CPlayerEquipment> m_pEquipment;
	CString m_strScript;
	int32 m_nLife;

	bool m_bDropped;
	int32 m_nLifeLeft;
};