#pragma once
#include "Character.h"

class CEnemy : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CEnemy( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CEnemy ); }

	virtual void Damage( int32 nDmg ) override;
	virtual void OnHitPlayer( class CPlayer* pPlayer, const CVector2& normal );
	virtual void OnKnockbackPlayer( const CVector2 & vec ) {}
	int32 GetHp() { return m_nHp; }

	void SetDefence( float fDefence ) { m_fDefence = fDefence; }
protected:
	int32 m_nHp;
	float m_fDefence;
	int32 m_nKnockbackCostSp;
};

class CEnemyPart : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CEnemyPart( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CEnemy ); }

	virtual void Damage( int32 nDmg ) override;
};