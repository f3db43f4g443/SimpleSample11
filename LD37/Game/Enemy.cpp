#include "stdafx.h"
#include "Enemy.h"
#include "Player.h"

void CEnemy::Damage( int32 nDmg )
{
	m_nHp -= nDmg;
	if( m_nHp <= 0 )
	{
		m_nHp = 0;
		Kill();
	}
}

void CEnemy::OnHitPlayer( class CPlayer* pPlayer, const CVector2& normal )
{
	CVector2 vec = normal;
	if( vec.Normalize() == 0 )
	{
		vec = pPlayer->globalTransform.GetPosition() - globalTransform.GetPosition();
		vec.Normalize();
	}
	if( pPlayer->IsRolling() )
	{
		if( !Knockback( vec ) )
			pPlayer->Knockback( vec * -1 );
	}
	else
		pPlayer->Knockback( vec * -1 );
}