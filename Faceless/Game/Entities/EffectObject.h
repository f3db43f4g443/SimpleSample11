#pragma once
#include "Entity.h"
#include "Common/Trigger.h"
#include "Common/StringUtil.h"

class CEffectObject : public CEntity
{
	friend void RegisterGameClasses();
public:
	CEffectObject( const SClassCreateContext& context );

	void SetTime( float fTime ) { m_fTimeLeft = fTime; }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void SetState( uint8 nState );
protected:
	virtual void OnTickBeforeHitTest();
private:
	TClassTrigger<CEffectObject> m_tickBeforeHitTest;
	uint8 m_nType;
	
	CReference<CEntity> m_pStates[3];
	float m_fBirthTime;
	float m_fDeathTime;
	float m_fTimeLeft;
	uint8 m_nState;
};