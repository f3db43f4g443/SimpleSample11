#pragma once
#include "Entity.h"
#include "Trigger.h"

class CCharacter : public CEntity
{
	friend void RegisterGameClasses();
public:
	CCharacter();
	CCharacter( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual bool CanTriggerItem() { return false; }
	virtual void Awake() {}
	virtual void Kill();
	virtual void Crush() { Kill(); }
	virtual bool Knockback( const CVector2& vec ) { return false; }
	virtual bool IsKnockback() { return false; }
	virtual void Damage( int32 nDmg ) {}
	virtual bool IsHiding() { return false; }
	const CVector2& GetVelocity() { return m_velocity; }
	void SetVelocity( const CVector2& velocity ) { m_velocity = velocity; }
protected:
	virtual void OnTickBeforeHitTest();
	virtual void OnTickAfterHitTest();

	TResourceRef<CPrefab> m_pKillEffect;

	CVector2 m_velocity;
private:
	TClassTrigger<CCharacter> m_tickBeforeHitTest;
	TClassTrigger<CCharacter> m_tickAfterHitTest;
};