#pragma once
#include "Character.h"
#include "Render/Animation.h"
#include "Render/Footprint.h"

class CSpiderBossLimb;
class CSpiderBoss : public CCharacter
{
public:
	CSpiderBoss();

	enum
	{
		eState_Idle,
		eState_Move,
		eState_Attack,
		eState_Hit,
		eState_Hanging,
		eState_Hanged,
		eState_HangedAttack,
		eState_HangedHit,
		eState_HangedThrow,
		eState_HangedThrowHit,
	};

	uint8 GetCurState() { return m_nState; }
	bool IsHitInHR() { return m_bHitInHR; }
	CFootprintReceiver* GetFootprint() { return m_pFootprint; }

	void BeginMove();
	void BeginAttack();
	void BeginHangUp();
	void BeginHangedAttack();
	void BeginHangedThrow();
	void Stop();
	void HangedStop();

	void OnLimbDestroyed() { if( m_nState < eState_Hanging ) m_nDestroyedLimbs++; }
protected:
	virtual void OnTickBeforeHitTest() override;
	void OnTickAfterHitTest();
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void Move();
	void Idle();
	void Attack();
	void HangUp();
	void HangedMove();
	void HangedAttack();
	void HangedThrow();

	void HangedFixPosition();
	void OnEventHit();
	void OnHit();

	uint8 m_nState;
	uint32 m_nMoveType;
	CVector2 m_moveTarget;
	float m_fHRTime;
	bool m_bHit;
	bool m_bHitInHR;
	uint32 m_nSkillCounter;
	uint32 m_nDestroyedLimbs;
	CReference<IAnimation> m_pAnim;
	CReference<CFootprintReceiver> m_pFootprint;
	float m_fTime;
	float m_fPlayerDizzyTime;
	float m_fHangLength1, m_fHangLength2, m_fHangLength3;
	CSpiderBossLimb* m_pLimbs[11];
	TClassTrigger<CSpiderBoss> m_tickAfterHitTest;
	TClassTrigger<CSpiderBoss> m_onHit;
};