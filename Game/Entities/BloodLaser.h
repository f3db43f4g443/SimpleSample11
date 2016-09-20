#pragma once
#include "Entity.h"

class CBloodLaser : public CEntity
{
public:
	CBloodLaser( const CVector2& end, float fWidth, float fDuration = 0, float fRotSpeed = 0, CEntity* pOwner = NULL );
	~CBloodLaser();
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Render( CRenderContext2D& context ) override;
private:
	void OnTickBeforeHitTest();
	void OnTickAfterHitTest();
	
	CReference<CEntity> m_pOwner;
	bool m_bAlive;
	float m_fTime;
	float m_fDuration;
	float m_fRotSpeed;
	float m_fDeathTime;
	CVector2 m_width;
	CVector2 m_end;
	TClassTrigger<CBloodLaser> m_tickBeforeHitTest;
	TClassTrigger<CBloodLaser> m_tickAfterHitTest;
};