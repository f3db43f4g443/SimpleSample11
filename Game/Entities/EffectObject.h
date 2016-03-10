#pragma once
#include "Entity.h"
#include "Common/Trigger.h"

class CEffectObject : public CEntity
{
public:
	enum
	{
		eType_None,
		eType_Character,
		eType_FlyingObject
	};

	CEffectObject( float fTime, uint8 nType = eType_None );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnTickBeforeHitTest();
private:
	TClassTrigger<CEffectObject> m_tickBeforeHitTest;
	float m_fTimeLeft;
	uint8 m_nType;
};