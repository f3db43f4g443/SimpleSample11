#pragma once
#include "Character.h"

class CEnemy : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CEnemy( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CEnemy ); }

	virtual void OnAddedToStage() { m_nMaxHp = m_nHp; CCharacter::OnAddedToStage(); }
	virtual void Damage( SDamageContext& context ) override;
	virtual void OnHitPlayer( class CPlayer* pPlayer, const CVector2& normal );
	virtual void OnKnockbackPlayer( const CVector2 & vec ) {}
	virtual bool CanOpenDoor() override { return GetHitType() == eEntityHitType_Enemy; }
	int32 GetHp() { return m_nHp; }
	void SetHp( int32 nHp ) { m_nHp = nHp; }
	int32 GetMaxHp() { return m_nMaxHp; }

	virtual bool IsOwner( CEntity* pEntity ) { return pEntity == this; }

	void SetDefence( float fDefence ) { m_fDefence = fDefence; }
protected:
	int32 m_nHp;
	float m_fDefence;
	int32 m_nKnockbackCostSp;
	uint8 m_nHitType;

	int32 m_nMaxHp;
};

class CEnemyPart : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CEnemyPart( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CEnemyPart ); }

	virtual void Damage( SDamageContext& context ) override;

	virtual bool IsOwner( CEntity* pEntity ) override;
};