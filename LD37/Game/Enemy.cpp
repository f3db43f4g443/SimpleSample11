#include "stdafx.h"
#include "Enemy.h"
#include "Player.h"

void CEnemy::Damage( uint32 nDmg )
{
	m_nHp -= nDmg;
	if( m_nHp <= 0 )
	{
		m_nHp = 0;
		Kill();
	}
}

void CEnemy::OnHitPlayer( class CPlayer* pPlayer )
{
	Kill();
}