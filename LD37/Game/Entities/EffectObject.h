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
	void SetVelocity( const CVector2& vel ) { m_vel = vel; }
	void SetAcceleration( const CVector2& a ) { m_a = a; }
	void SetAnimTimeScale( float fTimeScale ) { m_fAnimTimeScale = fTimeScale; }
protected:
	virtual void OnTickBeforeHitTest();
private:
	TClassTrigger<CEffectObject> m_tickBeforeHitTest;
	
	CReference<CEntity> m_pStates[3];
	TResourceRef<CSoundFile> m_strSound;
	float m_fBirthTime;
	float m_fDeathTime;
	float m_fTimeLeft;
	CVector2 m_vel;
	float m_velA;
	CVector2 m_a;
	uint8 m_nState;

	float m_fAnimTimeScale;
};