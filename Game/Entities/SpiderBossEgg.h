#pragma once
#include "FlyingObject.h"
#include "Render/ParticleSystem.h"
#include "Render/Footprint.h"

class CSpiderBossEgg : public CFlyingObject
{
public:
	CSpiderBossEgg( CEntity* pOwner, const CVector2& targetPos, float fLifeTime );
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnKilled() override;
private:
	void OnPlayerAttack( SPlayerAttackContext* pContext );

	CReference<CEntity> m_pOwner;
	CReference<CParticleSystemInstance> m_pParticleInstWeb;
	CReference<CParticleSystemInstance> m_pParticleInstBlood;
	CReference<CFootprintReceiver> m_pFootprintWeb;
	CReference<CFootprintReceiver> m_pFootprintBlood;
	CElement2D m_elemFootprint;
	CElement2D m_elemFootprintWeb;
	CElement2D m_elemFootprintBlood;
	CVector4 m_param;
	CVector2 m_targetPos;
	float m_fLifeTime;
	float m_fKillTime;
	TClassTrigger1<CSpiderBossEgg, SPlayerAttackContext*> m_onPlayerAttack;
};