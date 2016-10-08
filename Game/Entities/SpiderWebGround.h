#pragma once
#include "DizzyRegion.h"
#include "FlyingObject.h"
#include "Utils/PlayerDizzy.h"
#include "Render/ParticleSystem.h"
#include "Render/Footprint.h"

class CSpiderWebGround : public CFlyingObject
{
public:
	CSpiderWebGround( uint8 nType, float fDir );
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest() override;
	void OnTickAfterHitTest();
	virtual void OnKilled() override;
private:
	TClassTrigger<CSpiderWebGround> m_tickAfterHitTest;
	uint8 m_nType;
	float m_fDir;
	float m_fLife;
	float m_fMaxLife;
	float m_fDeathTime;
	CReference<CFootprintReceiver> m_pFootprint;
	CReference<CParticleSystemInstance> m_pParticleInst;
	CElement2D m_elemParticle;
	
	uint32 m_nPolygonCount;
	SHitProxyPolygon m_polygons[10];
	SPlayerDizzyEffectSpinning m_playerDizzy;

	static CFootprintUpdateDrawable* s_pUpdateDrawable;
};