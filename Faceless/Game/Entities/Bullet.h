#pragma once
#include "Entity.h"

class CBullet : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBullet( const SClassCreateContext& context )
		: CEntity( context )
		, m_bActive( false )
		, m_tickBeforeHitTest( this, &CBullet::TickBeforeHitTest )
		, m_tickAfterHitTest( this, &CBullet::TickAfterHitTest ) {
		SET_BASEOBJECT_ID( CBullet );
	}

	void SetActive( bool bActive ) { m_bActive = bActive; }
	void SetVelocity( const CVector2& velocity ) { m_velocity = velocity; }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	uint32 GetDmg() { return m_nDmg; }
	float GetSpeed() { return m_fSpeed; }
protected:
	virtual void TickBeforeHitTest();
	virtual void TickAfterHitTest();

	class CFace* m_pFace;
	bool m_bActive;
	CVector2 m_velocity;

	uint32 m_nDmg;
	float m_fSpeed;

	TClassTrigger<CBullet> m_tickBeforeHitTest;
	TClassTrigger<CBullet> m_tickAfterHitTest;
};