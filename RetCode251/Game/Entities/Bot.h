#pragma once
#include "Entities/CharacterMisc.h"

class CBot : public CCommonMoveableObject
{
	friend void RegisterGameClasses_Bot();
public:
	CBot( const SClassCreateContext& context ) : CCommonMoveableObject( context ) { SET_BASEOBJECT_ID( CBot ); }
	virtual void OnAddedToStage() override;
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	virtual bool IsEnemy() override { return true; }
	virtual bool Damage( SDamageContext& context ) override;
	virtual bool ImpactHit( int32 nLevel, const CVector2& vec, CEntity* pEntity ) override;
	virtual void Activate() override { m_bActivate = true; m_nDamageTick = m_nDeactivateTime; if( m_bHideOnInactivated ) bVisible = true; }
	virtual void Deactivate() override { m_bActivate = false; m_nDamageTick = 0; if( m_bHideOnInactivated ) bVisible = false; }
protected:
	virtual void Init();
	void UpdateModules( bool bActivated );
	int32 m_nDamageActivateTime;
	int32 m_nDeactivateTime;
	bool m_bActivateOnAlert;
	bool m_bActivateOnDamage;
	bool m_bHideOnInactivated;

	bool m_bInited;
	bool m_bActivate;
	bool m_bActivateOK;
	int32 m_nDamageTick;
};

class CBotTypeA : public CBot
{
	friend void RegisterGameClasses_Bot();
public:
	CBotTypeA( const SClassCreateContext& context ) : CBot( context ) { SET_BASEOBJECT_ID( CBotTypeA ); }
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	virtual void Kill() override {}
	virtual bool CanHit( CEntity* pEntity ) override { return pEntity != m_p1 && __super::CanHit( pEntity ); }
	virtual bool CheckImpact( CEntity* pEntity, SRaycastResult& result, bool bCast ) override { return pEntity != m_p1 && __super::CheckImpact( pEntity, result, bCast ); }

	virtual int8 CheckPush( SRaycastResult& hit, const CVector2& dir, float& fDist, SPush& context, int32 nPusher ) override;
	virtual void HandlePush( const CVector2& dir, float fDist, int8 nStep ) override;
protected:
	virtual void Init() override;
	bool HandlePenetration( CEntity** pTestEntities, int32 nTestEntities, CVector2* pGravity, CEntity* pLandedEntity = NULL, CVector2* pLandedOfs = NULL );
	void UpdateHit();
	int8 m_nShapeType;
	CVector4 m_shapeParams[2];
	int32 m_nStateTransitTime;
	int32 m_nStateTransitFrames;
	int32 m_nRespawnTime;
	CReference<CEntity> m_p1;

	bool m_bForceDeactivate;
	int32 m_nStateTransitCurTime;
	int32 m_nAnimTick;
	int32 m_nRespawnTick;
};

class CBotTypeALeap : public CBotTypeA
{
	friend void RegisterGameClasses_Bot();
public:
	CBotTypeALeap( const SClassCreateContext& context ) : CBotTypeA( context ), m_nDir( 1 ) { SET_BASEOBJECT_ID( CBotTypeALeap ); }
	virtual void OnTickAfterHitTest() override;
	virtual bool Damage( SDamageContext& context ) override;
private:
	bool AICanHitPlatform();
	void UpdateImg();
	CRectangle m_rectDetect;
	CRectangle m_texRect0;
	int32 m_nLeapCD;
	int32 m_nLeapReadyTime;
	CVector2 m_leapVel;
	int8 m_nSmartLeap;
	bool m_bPushLeap;
	int32 m_nLeapFixedVelTime;
	float m_fLeapAnimSpd[2];
	int32 m_nLeapDamage;
	CRectangle m_leapDmgRect;
	CVector2 m_leapHitDir;
	float m_fLandPointCheckOfs;

	bool m_bIsLeaping;
	int32 m_nDir;
	int32 m_nLeapCDLeft;
	int32 m_nLeapReadyTimeLeft;
	int32 m_nLeapFixedVelTimeLeft;
};

class CBotTypeAPatrol : public CBotTypeA
{
	friend void RegisterGameClasses_Bot();
public:
	CBotTypeAPatrol( const SClassCreateContext& context ) : CBotTypeA( context ), m_nDir( 1 ) { SET_BASEOBJECT_ID( CBotTypeAPatrol ); }
	virtual void OnTickAfterHitTest() override;
	virtual bool Damage( SDamageContext& context ) override;
private:
	bool FallDetect( const CVector2& gravityDir );
	bool CanFindFloor();
	void FindFloor( CEntity** pTestEntities, int32 nTestEntities, const CVector2& gravityDir );
	void UpdateImg();
	CRectangle m_rectDetect;
	CRectangle m_texRect0;
	int32 m_nAttackCD;
	int32 m_nAttackTime;
	int32 m_nAttackReadyTime;
	float m_fMoveSpeed;
	int32 m_nWalkAnimFrames;
	int32 m_nWalkAnimSpeed;
	CRectangle m_fallDetectRect;

	bool m_bIsWalking;
	bool m_bModuleActive;
	int32 m_nDir;
	int32 m_nAttackCDLeft;
	int32 m_nAttackTimeLeft;
	CReference<CCharacter> m_pLanded;
	CMatrix2D m_lastLandedEntityTransform;
	CVector2 m_groundNorm;
};