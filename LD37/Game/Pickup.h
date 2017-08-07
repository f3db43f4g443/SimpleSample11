#pragma once
#include "Item.h"
#include "Entities/EffectObject.h"
#include "Entities/UtilEntities.h"

class CPickUp : public CEntity
{
	friend void RegisterGameClasses();
public:
	CPickUp( const SClassCreateContext& context ) : CEntity( context )
		, m_onRefreshText( this, &CPickUp::RefreshText )
		, m_onRefreshPrice( this, &CPickUp::RefreshPrice ) { SET_BASEOBJECT_ID( CPickUp ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void DisablePickUp();
	virtual void RefreshText() {}
	void RefreshPrice();
	uint32 GetPrice() { return m_nPrice; }
	void SetPrice( uint32 nPrice );
	virtual bool CanPickUp( class CPlayer* pPlayer ) { return true; }
	virtual void PickUp( class CPlayer* pPlayer );
	virtual void Kill();
	void RegisterPickupEvent( CTrigger* pTrigger ) { m_onPickedUp.Register( 0, pTrigger ); }
protected:
	CReference<CEffectObject> m_pDeathEffect;
	CReference<CSimpleText> m_pText;
	CReference<CSimpleText> m_pPriceText;
	CReference<CSimpleText> m_pPriceText1;
	CEventTrigger<1> m_onPickedUp;
	TClassTrigger<CPickUp> m_onRefreshText;
	TClassTrigger<CPickUp> m_onRefreshPrice;
	uint32 m_nPrice;
};

class CPickUpCommon : public CPickUp
{
	friend void RegisterGameClasses();
public:
	CPickUpCommon( const SClassCreateContext& context ) : CPickUp( context ) { SET_BASEOBJECT_ID( CPickUpCommon ); }
	virtual void PickUp( CPlayer* pPlayer ) override;

	uint8 GetClass();
protected:
	uint32 m_nHeal;
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

class CPickUpTemplate : public CPickUp
{
	friend void RegisterGameClasses();
public:
	CPickUpTemplate( const SClassCreateContext& context ) : CPickUp( context ), m_lightBaseColor( -1, -1, -1 ) { SET_BASEOBJECT_ID( CPickUpTemplate ); }

	void Set( CEntity* pEntity, uint32 nPrice );
	virtual bool CanPickUp( class CPlayer* pPlayer ) override;
	virtual void PickUp( CPlayer* pPlayer ) override;
	virtual void RefreshText() override;
private:
	CVector2 m_ofs;
	float m_fPricePercent;

	CReference<CEntity> m_pEntity;
	CReference<CRenderObject2D> m_pLight;
	CVector3 m_lightBaseColor;
};