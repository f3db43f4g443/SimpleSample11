#pragma once
#include "PlayerDebuff.h"
#include "FlyingObject.h"
#include "Utils/PlayerDizzy.h"
#include "Render/Rope2D.h"

class CPlayerDebuffSpiderWebObject : public CFlyingObject
{
public:
	CPlayerDebuffSpiderWebObject();

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void CreateBullets();
	void DestroyBullets();
	float CheckHitPlayer( CPlayer* pPlayer );
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnKilled() override;
private:
	void UpdateBulletDisplay( uint32 i );

	CVector4 m_param;
	float m_fFade;
	CReference<CParticleSystemInstance> m_pParticleInst;
	CElement2D m_elemParticle;
	CReference<CRopeObject2D> m_pBullet[3];
	float m_bulletLen[3];
	CVector2 m_bulletDir[3];

	static class CFootprintUpdateDrawable* s_pUpdateDrawable;
};

class CPlayerDebuffSpiderWeb : public CPlayerDebuff
{
public:
	CPlayerDebuffSpiderWeb( uint32 nObjectCount );

	virtual void OnAdded( CPlayerDebuff* pAddedDebuff ) override;
	virtual void OnRemoved() override;
	virtual void OnEnterHorrorReflex() override;
	virtual void OnEndHorrorReflex( float fSpRecover ) override;
	virtual bool UpdateAfterHitTest() override;
private:
	vector<CReference<CPlayerDebuffSpiderWebObject> > m_pObjects;
	uint32 m_nObjectCount;

	SPlayerDizzyEffectSpinning m_playerDizzy;
};