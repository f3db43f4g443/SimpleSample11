#pragma once
#include "Useable.h"

class CDoor : public CEntity
{
	friend void RegisterGameClasses();
public:
	CDoor( const SClassCreateContext& context );
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTickBeforeHitTest();
	void OnUse();
	TClassTrigger<CDoor> m_onUse;
	TClassTrigger<CDoor> m_tickBeforeHitTest;

	CReference<CEntity> m_pObj1;
	CReference<CEntity> m_pObj2;
	CReference<CUseable> m_pHit;
	float m_fDoorSize;
};