#pragma once
#include "SlimeCore.h"

class CSlime1 : public CSlime
{
public:
	CSlime1( CSlimeGround* pSlimeGround, const CVector2& velocity, float fSize ) : CSlime( pSlimeGround, velocity, fSize ) {}
	virtual void ChangeVelocity( bool bExplode = false ) override;
};

class CSlimeCore1 : public CSlimeCore
{
public:
	CSlimeCore1();
	virtual void Clear() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	void OnHit();
	void OnEventHit() { m_bHit = true; }
	virtual void OnKilled() override;
	
	CVector2 m_velocity;
	float m_fAttackTime;
	uint8 m_nState;
	bool m_bHit;
	CReference<IAnimation> m_pCurAnim;
	TClassTrigger<CSlimeCore1> m_onHit;
};

class CSlimeCore2 : public CSlimeCore
{
public:
	CSlimeCore2();
	virtual void Clear() override;
	virtual float GetVelocityWeight() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	void BeginAttack();
	void UpdateAttackEffect();
	void OnHit();
	virtual void OnKilled() override;

	CVector2 m_velocity;
	float m_fAttackTime;
	bool m_bOK;
	uint8 m_nAttackType;

	CReference<CRenderObject2D> m_pAttackEffects[2];
};

class CSlimeCore3 : public CSlimeCore
{
public:
	CSlimeCore3();
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
private:
	void OnEventHit() { m_bHit = true; }
	void OnHit();
	virtual void OnKilled() override;

	float m_fAttackCD;
	bool m_bIsAttack;
	bool m_bHit;
	CReference<IAnimation> m_pCurAnim;

	TClassTrigger<CSlimeCore3> m_onHit;
};

class CSlimeCoreGenerator1 : public CSlimeCoreGenerator
{
public:
	CSlimeCoreGenerator1( CSlimeGround* pSlimeGround );
};