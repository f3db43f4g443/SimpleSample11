#pragma once
#include "Entity.h"

class CBloodLaser : public CEntity
{
public:
	CBloodLaser( const CVector2& end, float fWidth );
	~CBloodLaser();
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Render( CRenderContext2D& context ) override;
private:
	void OnTickBeforeHitTest();
	void OnTickAfterHitTest();

	bool m_bAlive;
	float m_fTime;
	float m_fDeathTime;
	CVector2 m_width;
	CVector2 m_end;
	TClassTrigger<CBloodLaser> m_tickBeforeHitTest;
	TClassTrigger<CBloodLaser> m_tickAfterHitTest;
};