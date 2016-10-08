#pragma once
#include "FlyingObject.h"
#include "Render/ParticleSystem.h"
#include "Render/Footprint.h"

class CSpiderBossBullet : public CFlyingObject
{
public:
	CSpiderBossBullet( CEntity* pOwner, uint32 nType );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	void OnTickAfterHitTest();
	virtual void OnKilled() override;
private:
	uint32 m_nType;
	CReference<CParticleSystemInstance> m_pParticle;
	float m_fKillTimeLeft;
	CVector2 m_dir;
	float m_fMovedDist;
	CReference<CEntity> m_pOwner;
	TClassTrigger<CSpiderBossBullet> m_tickAfterHitTest;
};