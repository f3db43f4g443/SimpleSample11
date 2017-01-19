#pragma once
#include "Entity.h"
#include "Trigger.h"

class CCharacter : public CEntity
{
public:
	CCharacter();
	CCharacter( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual bool CanTriggerItem() { return false; }
	virtual void Awake() {}
	virtual void Kill() { SetParentEntity( NULL ); }
	virtual void Crush() { Kill(); }
	const CVector2& GetVelocity() { return m_velocity; }
protected:
	virtual void OnTickBeforeHitTest();
	virtual void OnTickAfterHitTest();

	CVector2 m_velocity;
private:
	TClassTrigger<CCharacter> m_tickBeforeHitTest;
	TClassTrigger<CCharacter> m_tickAfterHitTest;
};