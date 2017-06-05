#pragma once
#include "Entity.h"
#include "Common/Trigger.h"
#include "Common/StringUtil.h"

class CEffectObject : public CEntity
{
	friend void RegisterGameClasses();
public:
	CEffectObject( const SClassCreateContext& context );
	CEffectObject( float fTime, CVector2 velocity, float fAngularVelocity );

	void SetTime( float fTime ) { m_fTimeLeft = fTime; }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	float GetBirthTime() { return m_fBirthTime; }
	float GetDeathTime() { return m_fDeathTime; }

	void SetState( uint8 nState );
protected:
	virtual void OnTickBeforeHitTest();
private:
	TClassTrigger<CEffectObject> m_tickBeforeHitTest;
	
	CReference<CEntity> m_pStates[3];
	float m_fBirthTime;
	float m_fDeathTime;
	float m_fTimeLeft;
	CVector2 m_vel;
	float m_velA;
	uint8 m_nState;
};