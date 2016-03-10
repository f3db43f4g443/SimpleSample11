#pragma once
#include "FlyingObject.h"
#include "Render/ParticleSystem.h"

class CPlayer;
class CEnemyBullet : public CFlyingObject
{
public:
	CEnemyBullet( CEntity* pOwner, CVector2 velocity, CVector2 acceleration, float fSize, float fLife, int32 nDmgHp, int32 nDmgMp, int32 nDmgSp, float fKillTime );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest();
	virtual void OnKilled() override;

	virtual void OnHitPlayer( CPlayer* pPlayer );
private:
	TClassTrigger<CEnemyBullet> m_tickAfterHitTest;
	CReference<CEntity> m_pOwner;
	CReference<CParticleSystemInstance> m_pParticle;
	int32 m_nDmgHp, m_nDmgMp, m_nDmgSp;
	float m_fLifeLeft;
	float m_fKillTimeLeft;
	CVector2 m_velocity;
	CVector2 m_acceleration;
};