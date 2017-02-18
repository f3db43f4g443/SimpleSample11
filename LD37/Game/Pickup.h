#pragma once
#include "Entity.h"
#include "Entities/EffectObject.h"

class CPickUp : public CEntity
{
	friend void RegisterGameClasses();
public:
	CPickUp( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CPickUp ); }

	virtual void OnAddedToStage() override;

	virtual void PickUp( class CPlayer* pPlayer );
	virtual void Kill();
protected:
	CReference<CEffectObject> m_pDeathEffect;
};

class CPickUpCommon : public CPickUp
{
	friend void RegisterGameClasses();
public:
	CPickUpCommon( const SClassCreateContext& context ) : CPickUp( context ) { SET_BASEOBJECT_ID( CPickUpCommon ); }
	virtual void PickUp( CPlayer* pPlayer ) override;
protected:
	uint32 m_nHpRestore;
};