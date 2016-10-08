#pragma once
#include "Slime.h"
#include "EffectObject.h"
#include "Render/Sound.h"
#include "Render/ParticleSystem.h"

class CSlimeBlister : public CCharacter
{
	friend class CSlimeBlisterDrawable;
public:
	CSlimeBlister();
	virtual void Render( CRenderContext2D& context ) override;
protected:
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void OnTickBeforeHitTest() override;
	void OnTickAfterHitTest();
	void OnPlayerAttack( SPlayerAttackContext* pContext );
private:
	void SetMoveTarget();

	struct SSlime
	{
		SSlime() : pRenderObject( NULL ), velocity( 0, 0 ) {}
		CRenderObject2D* pRenderObject;
		CVector2 velocity;
	};
	vector<SSlime> m_vecSlimes;
	CCanvas m_canvas;
	bool m_bBurst;
	bool m_bCanBeHit;
	uint32 m_nSlimeCount;
	float m_fBleedCD;
	CVector4 m_param;
	uint32 m_nState;
	float m_fStateTime;
	CVector2 m_velocity;
	uint32 m_nMoveCount;

	CReference<CImage2D> m_pHitImage;
	CReference<CEffectObject> m_pEffect;
	CReference<ISoundTrack> m_pSound;
	CReference<CParticleSystemInstance> m_pParticleInst;

	class CParticleSystem *m_pParticleSystem, *m_pParticleSystem1;

	TClassTrigger<CSlimeBlister> m_tickAfterHitTest;
	TClassTrigger1<CSlimeBlister, SPlayerAttackContext*> m_onPlayerAttack;
};