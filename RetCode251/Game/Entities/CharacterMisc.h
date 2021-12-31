#pragma once
#include "Character.h"
#include "CharacterMove.h"
#include "Interfaces.h"
#include "Beam.h"

class CDamageArea : public CEntity
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CDamageArea( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CDamageArea ); }

	bool IsIgnoreDamageSource( int8 nType ) { return m_bIgnoreDamageSource[nType]; }
private:
	bool m_bIgnoreDamageSource[3];
};

class CCommonMoveableObject : public CCharacter
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CCommonMoveableObject( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CCommonMoveableObject ); m_bHasHitFilter = true; }
	virtual bool Damage( SDamageContext& context ) override;
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool ImpactHit( int32 nLevel, const CVector2& vec, CEntity* pEntity ) override;
	virtual bool CheckImpact( CEntity* pEntity, SRaycastResult& result, bool bCast );

	virtual int8 CheckPush( SRaycastResult& hit, const CVector2& dir, float& fDist, SPush& context, int32 nPusher ) override;
	virtual void HandlePush( const CVector2& dir, float fDist, int8 nStep ) override;
protected:
	bool CanHitPlatform();
	void SetImpactLevel( int32 nLevel, int32 nTick );
	CVector2 HandleCommonMove();
	void PostMove();
	void PostMove( int32 nTestEntities, CEntity** pTestEntities );
	float m_fGravity;
	float m_fFrac;
	float m_fMaxFallSpeed;
	float m_fAirborneFrac;
	int32 m_nBlockImpactLevel;
	int32 m_nImpactHitTime;

	CVector2 m_vel;
	float m_fImpactSpeed0;
	int32 m_nKickCounter;
	int32 m_nImpactLevel;
	int32 m_nImpactTick;
	SCharacterMovementData m_moveData;
};

class CCommonGrabbable : public CCharacter, public IGrabbable
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CCommonGrabbable( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CCommonGrabbable ); }

	virtual bool CheckGrab( class CPlayer* pPlayer, SGrabDesc& desc ) override;
	virtual void OnAttached( class CPlayer* pPlayer ) override;
	virtual void OnDetached( class CPlayer* pPlayer ) override;
protected:
	CVector2 m_grabOfs;
	float m_fDropThreshold;
	int8 m_nGrabDir;
	int8 m_nDetachType;

	bool m_bAttached;
};

class CLever : public CCommonGrabbable
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CLever( const SClassCreateContext& context ) : CCommonGrabbable( context ) { SET_BASEOBJECT_ID( CLever ); }
	virtual void OnAddedToStage() override;

	virtual void OnTickBeforeHitTest() override;
	virtual bool CheckGrab( class CPlayer* pPlayer, SGrabDesc& desc ) override;
private:
	TArray<CVector3> m_arrState;
	TArray<CVector4> m_arrTransfer;

	bool m_bInited;
	CVector3 m_state0;
	int32 m_nCurState;
	int32 m_nNxtState;
	int32 m_nStateTransferTime;
};

class CChunk : public CCharacter, public IEditorTiled
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CChunk( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CChunk ); }
	virtual void OnAddedToStage() override;
	virtual bool IsPreview() { return true; }
	virtual void OnPreview() { UpdateImages(); }
	virtual void Render( CRenderContext2D& context ) override;

	virtual CVector2 GetTileSize() const override { return m_tileSize; }
	virtual TVector2<int32> GetSize() const override { return TVector2<int32>( m_nTileX, m_nTileY ); }
	virtual TVector2<int32> GetMinSize() const override { return TVector2<int32>( 2, 2 ); }
	virtual CVector2 GetBaseOfs() const override { return m_ofs; }
	virtual void Resize( const TRectangle<int32>& size ) override
	{ m_ofs = m_ofs + CVector2( size.x, size.y ) * m_tileSize; m_nTileX = size.width; m_nTileY = size.height; }
	virtual void SetBaseOfs( const CVector2& ofs ) override { m_ofs = ofs; }
protected:
	void Init();
	void UpdateImages();
	int8 m_nTypeX, m_nTypeY;
	int8 m_nTex1Type;
	int32 m_nTexX, m_nTexY;
	int32 m_nTexX1, m_nTexY1;
	CVector2 m_tileSize;
	int32 m_nTileX, m_nTileY;
	CVector2 m_ofs;
	CRectangle m_hitSize;
	float m_fHpPerTile;
	bool m_bNoHit;

	bool m_bInited;
	CRectangle m_origTexRect;
	vector<CElement2D> m_vecElems;
	int32 m_nParamCount;
	CVector4 m_params[2];
};

class CChunkPortal : public CChunk
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CChunkPortal( const SClassCreateContext& context ) : CChunk( context ) { SET_BASEOBJECT_ID( CChunkPortal ); }
	bool CheckTeleport( CPlayer* pPlayer );
	virtual void OnTickAfterHitTest() override;
private:
	bool m_bUp;
	CVector4 m_colorDefault;
	CVector4 m_colorInvalid;
	CVector4 m_colorValid;
};

struct SChunk1EditGroup
{
	SChunk1EditGroup( const SClassCreateContext& context ) {}
	int8 nAutoTypeCol, nAutoTypeRow;
	int32 nTexX, nTexY;
	CRectangle texRect;
};

class CChunk1 : public CCharacter, public IEditorTiled
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CChunk1( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CChunk1 ); }
	virtual void OnAddedToStage() override;
	virtual bool IsPreview() { return true; }
	virtual void OnPreview() { UpdateImages(); }
	virtual void Render( CRenderContext2D& context ) override;

	virtual CVector2 GetTileSize() const override { return m_tileSize; }
	virtual TVector2<int32> GetSize() const override { return TVector2<int32>( m_nTileX, m_nTileY ); }
	virtual TVector2<int32> GetMinSize() const override { return TVector2<int32>( 1, 1 ); }
	virtual CVector2 GetBaseOfs() const override { return m_ofs; }
	virtual void Resize( const TRectangle<int32>& size ) override;
	virtual void SetBaseOfs( const CVector2& ofs ) override { m_ofs = ofs; }

	int32 GetEditGroupCount() { return m_arrEditGroup.Size(); }
	int32 GetEditGroupIndex() { return m_nEditGroup; }
	SChunk1EditGroup& GetEditGroup() { return m_arrEditGroup[m_nEditGroup]; }
	void SetEditGroup( int32 n );
	int32 GetColData( int32 i ) { return m_arrCols[i]; }
	int32 GetRowData( int32 i ) { return m_arrRows[i]; }
	void SetColType( int32 i, int32 n );
	void SetRowType( int32 i, int32 n );
private:
	void Init();
	void UpdateImages();
	TArray<SChunk1EditGroup> m_arrEditGroup;
	int32 m_nEditGroup;
	TArray<int32> m_arrCols;
	TArray<int32> m_arrRows;
	TArray<int32> m_arrData;
	CVector2 m_tileSize;
	int32 m_nTileX, m_nTileY;
	CVector2 m_ofs;
	CRectangle m_hitSize;
	float m_fHpPerTile;
	bool m_bNoHit;

	bool m_bInited;
	vector<CElement2D> m_vecElems;
};

class CAlertTrigger : public CCharacter
{
	friend void RegisterGameClasses_CharacterMisc();
	friend class CLevelEditObjectQuickToolAlertTriggerResize;
public:
	CAlertTrigger( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CAlertTrigger ); }
	bool IsTriggered();
	virtual void OnTickAfterHitTest() override;
private:
	CRectangle m_alertRect;
	CVector2 m_alertVel;
	CVector4 m_color0;
	CVector4 m_colorTriggered;

	bool m_bDetected;
	bool m_bTriggered;
};

class CCharacterTriggerSpawn : public CEntity
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CCharacterTriggerSpawn( const SClassCreateContext& context ) : CEntity( context ), m_onTrigger( this, &CCharacterTriggerSpawn::Trigger ) { SET_BASEOBJECT_ID( CCharacterTriggerSpawn ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void Trigger();
	ECharacterEvent m_eEvent;
	TResourceRef<CPrefab> m_pPrefab;

	TClassTrigger<CCharacterTriggerSpawn> m_onTrigger;
};

struct SBulletDesc
{
	SBulletDesc( const SClassCreateContext& context ) {}
	TResourceRef<CPrefab> pPrefab;
	CVector2 ofs;
	CVector2 vel;
};

class CTurret : public CEntity, public IBotModule
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CTurret( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CTurret ); }
	virtual void InitModule() override {}
	virtual void UpdateModule( bool bActivated ) override;
private:
	bool Detect();
	void Fire();
	float m_fSightRange;
	float m_fSightAngle;
	float m_fRotAngle;
	float m_fRotSpeed;
	int8 m_nScanType;
	TResourceRef<CPrefab> m_pBullet;//
	TArray<CVector2> m_arrBulletOfs;//
	float m_fBulletVel;//
	TArray<SBulletDesc> m_arrBulletDesc;
	int32 m_nBulletCount;
	float m_fBulletAngle;
	int32 m_nFireCount;
	int32 m_nFireInterval;
	int32 m_nReloadTime;
	int32 m_nActivateTime;
	int32 m_nDetectActivateTime;
	TResourceRef<CPrefab> m_pStaticEftPreActivePrefab;
	TResourceRef<CPrefab> m_pStaticEftPrefab;
	CVector2 m_staticEftOfs;

	int32 m_nFireCountLeft;
	int32 m_nFireCD;
	int32 m_nActivateTimeLeft;
	int32 m_nDetectActivateTimeLeft;
	float m_fCurRot;
	int8 m_nStaticEft;
	int8 m_fCurRot1;
	CReference<CEntity> m_pStaticEft;
};

class CGate : public CCharacter
{
	friend void RegisterGameClasses_CharacterMisc();
	friend class CLevelEditObjectQuickToolGate;
public:
	CGate( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CGate ); }
	virtual bool IsPreview() { return true; }
	virtual void OnPreview() { InitImages(); }
	virtual void OnAddedToStage() override;
	virtual void OnTickBeforeHitTest() override;
	virtual void Render( CRenderContext2D& context ) override;
	virtual void HandlePush( const CVector2& dir, float fDist, int8 nStep ) override;
	virtual bool Damage( SDamageContext& context ) override;
protected:
	void Init();
	void InitImages();
	void UpdateImages();
	void UpdateHit();
	int8 m_nDir;
	bool m_bStartOpen;
	int8 m_nTriggerType;
	float m_fMaxLength;
	float m_fOpenLenPerTick;
	float m_fCloseLenPerTick;
	int32 m_nTriggerTime;

	bool m_bInited;
	CRectangle m_rect0;
	CRectangle m_texRect0;
	CRectangle m_texRect1;
	CRectangle m_initHit;
	float m_fCurLength;
	int32 m_nTriggerTimeLeft;
	CElement2D m_elems[4];
};

class CAlertBeam : public CBeam
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CAlertBeam( const SClassCreateContext& context ) : CBeam( context ) { SET_BASEOBJECT_ID( CAlertBeam ); }
protected:
	virtual void OnTickAfterHitTest() override;
	virtual void HandleHit( CEntity* pEntity, const CVector2& hitPoint ) override;
	virtual void UpdateImages() override;
	int32 m_nAlertTime;
	CVector2 m_alertTexOfs;

	int32 m_nAlertTimeLeft;
};

class CEnemy1 : public CCharacter
{
	friend void RegisterGameClasses_CharacterMisc();
public:
	CEnemy1( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CEnemy1 ); }
	virtual void OnAddedToStage() override;
private:
	virtual void OnTickAfterHitTest() override;
	int8 m_nVelType;
	int32 m_nVelPeriod;
	CVector4 m_velFactor;

	CVector2 m_initVel;
	CVector2 m_initDir;
	int32 m_nTick;
};