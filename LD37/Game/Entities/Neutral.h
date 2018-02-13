#pragma once
#include "Enemy.h"
#include "Pickup.h"
#include "Common/StringUtil.h"
#include "CharacterMove.h"

class CEnemyPhysics : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CEnemyPhysics( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CEnemyPhysics ); }
	virtual bool Knockback( const CVector2& vec ) override;
	virtual bool IsKnockback() override;
protected:
	virtual void OnTickAfterHitTest() override;
	SCharacterPhysicsMovementData m_moveData;
	uint32 m_nKnockbackTime;
	uint32 m_nLife;

	uint32 m_nKnockbackTimeLeft;
};

class CBulletEnemy : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CBulletEnemy( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CEnemyPhysics ); }
	virtual void OnAddedToStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	CVector2 m_a;
};

class CPickupCarrier : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CPickupCarrier( const SClassCreateContext& context ) : CCharacter( context ), m_onPickUp( this, &CPickupCarrier::Kill ) { SET_BASEOBJECT_ID( CPickupCarrier ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	CReference<CPickUp> m_pPickup;
	TClassTrigger<CPickupCarrier> m_onPickUp;
};

class CPickUpCarrierPhysics : public CPickupCarrier
{
	friend void RegisterGameClasses();
public:
	CPickUpCarrierPhysics( const SClassCreateContext& context ) : CPickupCarrier( context ), m_moveData( context ), m_flyData( context ), m_bAttracted( false ) { SET_BASEOBJECT_ID( CPickUpCarrierPhysics ); }
protected:
	virtual void OnTickAfterHitTest() override;
	SCharacterPhysicsMovementData m_moveData;
	SCharacterPhysicsFlyData m_flyData;
	uint32 m_nLife;
	float m_fAttractDist;

	bool m_bAttracted;
};

class CSpike : public CEntity
{
public:
	CSpike( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CSpike::OnTick ) { SET_BASEOBJECT_ID( CSpike ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	void OnTick();
private:
	TClassTrigger<CSpike> m_onTick;
};

class CFuelTank : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CFuelTank( const SClassCreateContext& context ) : CEnemy( context ), m_strAwakeEffect( context ), m_strKillEffect( context ), m_strBullet( context ), m_strBullet1( context ), m_bAwake( false ) { SET_BASEOBJECT_ID( CFuelTank ); }
	
	virtual void OnAddedToStage() override;
	virtual void Awake() override;
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;
private:
	uint32 m_nAwakeEffectInterval;
	CString m_strAwakeEffect;
	CString m_strKillEffect;
	CString m_strBullet;
	CString m_strBullet1;
	CVector2 m_accleration;
	uint32 m_nMaxHitDamage;
	float m_fFireMoveDist;

	bool m_bAwake;
	uint32 m_nAwakeEffectCD;
	CVector2 m_velocity;
	float m_fTotalMovedDist;
	CReference<CPrefab> m_pAwakeEffect;
	CReference<CPrefab> m_pKillEffect;
	CReference<CPrefab> m_pBullet;
	CReference<CPrefab> m_pBullet1;
};