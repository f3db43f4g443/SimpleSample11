#pragma once
#include "Character.h"
#include "Render/Animation.h"

class CSpider : public CCharacter
{
public:
	CSpider();
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void BeginMove();
	void BeginAttack();
	void Stop();
	void Move();
	void Idle();
	void Attack();

	void OnHit();

	void OnPlayerAttack( SPlayerAttackContext* pContext );

	uint8 m_nState;
	uint16 m_nHeadBoneIndex;
	CReference<IAnimation> m_pAnim;
	float m_fTime;
	TClassTrigger<CSpider> m_onHit;
	TClassTrigger1<CSpider, SPlayerAttackContext*> m_onPlayerAttack;
};