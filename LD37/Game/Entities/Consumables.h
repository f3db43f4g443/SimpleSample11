#pragma once
#include "Item.h"

class CConsumableHealing : public CConsumable
{
	friend void RegisterGameClasses();
public:
	CConsumableHealing( const SClassCreateContext& context ) : CConsumable( context ) { SET_BASEOBJECT_ID( CConsumableHealing ); }
	virtual bool Use( CPlayer* pPlayer ) override;
private:
	float m_fPercent;
};

class CConsumableRepair : public CConsumable
{
	friend void RegisterGameClasses();
public:
	CConsumableRepair( const SClassCreateContext& context ) : CConsumable( context ) { SET_BASEOBJECT_ID( CConsumableRepair ); }
	virtual bool Use( CPlayer* pPlayer ) override;
private:
	float m_fPercent;
};

class CPlayerSpecialEffect : public CEntity
{
	friend void RegisterGameClasses();
public:
	CPlayerSpecialEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CPlayerSpecialEffect::OnTick ) { SET_BASEOBJECT_ID( CPlayerSpecialEffect ); }
	virtual void OnAddedToStage();
	virtual void OnRemovedFromStage();
private:
	void OnTick() { SetParentEntity( NULL ); }

	uint8 m_nType;
	uint32 m_nTime;
	TClassTrigger<CPlayerSpecialEffect> m_onTick;
};

class CConsumableEffect : public CConsumable
{
	friend void RegisterGameClasses();
public:
	CConsumableEffect( const SClassCreateContext& context ) : CConsumable( context ) { SET_BASEOBJECT_ID( CConsumableEffect ); }
	virtual void OnAddedToStage() { m_pEffect->SetParentEntity( NULL ); }
	virtual bool Use( CPlayer* pPlayer ) override;
private:
	CReference<CEntity> m_pEffect;
};