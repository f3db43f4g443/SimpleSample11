#pragma once
#include "Entity.h"
#include "Trigger.h"

class CCharacter : public CEntity
{
public:
	CCharacter();

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest();
private:
	TClassTrigger<CCharacter> m_tickBeforeHitTest;
};