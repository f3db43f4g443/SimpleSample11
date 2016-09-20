#pragma once
#include "Entity.h"
#include "Trigger.h"

class CFlyingObject : public CEntity
{
public:
	CFlyingObject();
	CFlyingObject( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void Kill();
	bool IsAlive() { return m_bIsAlive; }
protected:
	virtual void OnKilled() {}
	virtual void OnTickBeforeHitTest();
private:
	TClassTrigger<CFlyingObject> m_tickBeforeHitTest;
	bool m_bIsAlive;
};