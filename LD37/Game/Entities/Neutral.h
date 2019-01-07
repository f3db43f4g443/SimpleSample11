#pragma once
#include "Enemy.h"
#include "Pickup.h"
#include "Common/StringUtil.h"
#include "CharacterMove.h"
#include "ItemDrop.h"

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
	float m_fKillVel;
	uint32 m_nKnockbackTimeLeft;
};

class CBulletEnemy : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CBulletEnemy( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CEnemyPhysics ); }
	virtual void OnAddedToStage() override;

	void SetAcceleration( const CVector2& a ) { m_a = a; }
	virtual void Kill() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	float m_fDeathTime;
	CReference<CRenderObject2D> m_pParticle;
	CVector2 m_a;
	uint32 m_nLife;
	CReference<CEntity> m_pExp;
	uint8 m_nMoveType;
	bool m_bKilled;
};

class CPickupCarrier : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CPickupCarrier( const SClassCreateContext& context ) : CCharacter( context ), m_onPickUp( this, &CPickupCarrier::Kill ) { SET_BASEOBJECT_ID( CPickupCarrier ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	CReference<CPickUp> m_pPickup;
	TClassTrigger<CPickupCarrier> m_onPickUp;
};

class CPickUpCarrierPhysics : public CPickupCarrier
{
	friend void RegisterGameClasses();
public:
	CPickUpCarrierPhysics( const SClassCreateContext& context ) : CPickupCarrier( context ), m_moveData( context ), m_flyData( context ), m_bAttracted( false ) { SET_BASEOBJECT_ID( CPickUpCarrierPhysics ); }
	virtual void OnAddedToStage() override;
protected:
	virtual void OnTickAfterHitTest() override;
	SCharacterPhysicsMovementData m_moveData;
	SCharacterPhysicsFlyData m_flyData;
	uint32 m_nLife;
	float m_fAttractDist;
	uint32 m_nPickUpTime;
	bool m_bIgnoreHit;

	bool m_bAttracted;
};

class CBonusStageReward : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBonusStageReward( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CBonusStageReward::OnTick )
	{ SET_BASEOBJECT_ID( CBonusStageReward ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Set( SItemDropContext& dropResult, uint32 nReward );
private:
	void OnTick();
	void OnPickUp();
	TResourceRef<CPrefab> m_pPrefab;
	TResourceRef<CDrawableGroup> m_pLinkDrawable;
	float m_l;
	float m_dl;
	float m_fAngularSpeed;
	uint32 m_nLife;
	TResourceRef<CPrefab> m_pRestorePrefab[4];
	TResourceRef<CPrefab> m_pMoneyPrefab[4];
	float m_fRewardSpeed;

	float m_r;
	float m_lCur;
	uint32 m_nRewards[4];
	CReference<CPickUp> m_pickups[6];
	CReference<CRenderObject2D> m_pLinkImgs[5];
	TClassTrigger<CBonusStageReward> m_onTick;
	vector<CFunctionTrigger> m_triggers;
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