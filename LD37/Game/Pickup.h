#pragma once
#include "Item.h"
#include "Entities/EffectObject.h"
#include "Entities/UtilEntities.h"

class CPickUp : public CEntity
{
	friend void RegisterGameClasses();
public:
	CPickUp( const SClassCreateContext& context ) : CEntity( context ), m_onRefreshText( this, &CPickUp::RefreshText ) { SET_BASEOBJECT_ID( CPickUp ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual void RefreshText() {}
	virtual void PickUp( class CPlayer* pPlayer );
	virtual void Kill();
	void RegisterPickupEvent( CTrigger* pTrigger ) { m_onPickedUp.Register( 0, pTrigger ); }
protected:
	CReference<CEffectObject> m_pDeathEffect;
	CReference<CSimpleText> m_pText;
	CEventTrigger<1> m_onPickedUp;
	TClassTrigger<CPickUp> m_onRefreshText;
};

class CPickUpCommon : public CPickUp
{
	friend void RegisterGameClasses();
public:
	CPickUpCommon( const SClassCreateContext& context ) : CPickUp( context ) { SET_BASEOBJECT_ID( CPickUpCommon ); }
	virtual void PickUp( CPlayer* pPlayer ) override;
protected:
	uint32 m_nHpRestore;
	uint32 m_nMoney;
};

class CPickUpItem : public CPickUp
{
	friend void RegisterGameClasses();
public:
	CPickUpItem( const SClassCreateContext& context ) : CPickUp( context ) { SET_BASEOBJECT_ID( CPickUpCommon ); }
	virtual void PickUp( CPlayer* pPlayer ) override;
	virtual void RefreshText() override;
protected:
	CReference<CItem> m_pItem;
};