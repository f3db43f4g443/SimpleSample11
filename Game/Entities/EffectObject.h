#pragma once
#include "Entity.h"
#include "Common/Trigger.h"

class CEffectObject : public CEntity
{
	friend void RegisterGameClasses();
public:
	enum
	{
		eType_None,
		eType_Character,
		eType_FlyingObject
	};

	CEffectObject( float fTime, class CDynamicTexture* pTexture = NULL, uint8 nType = eType_None );
	CEffectObject( const SClassCreateContext& context );

	void SetTime( float fTime ) { m_fTimeLeft = fTime; }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Render( CRenderContext2D& context ) override;
protected:
	virtual void OnTickBeforeHitTest();
private:
	TClassTrigger<CEffectObject> m_tickBeforeHitTest;
	CDynamicTexture* m_pUpdateTexture;
	float m_fTimeLeft;
	uint8 m_nType;
};