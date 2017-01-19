#pragma once
#include "Character.h"

class CEnemy : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CEnemy( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CEnemy ); }

	void Damage( uint32 nDmg );
	virtual void OnHitPlayer( class CPlayer* pPlayer );
private:
	int32 m_nHp;
	int32 m_nMaxHp;
};