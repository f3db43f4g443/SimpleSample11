#include "stdafx.h"
#include "Enemy.h"
#include "Player.h"

void CEnemy::Damage( int32 nDmg )
{
	if( m_nHp )
	{
		nDmg = ceil( nDmg * ( 1 - m_fDefence ) );
		m_nHp -= nDmg;
		if( m_nHp <= 0 )
		{
			m_nHp = 0;
			Kill();
		}
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

	if( !IsKnockback() && pPlayer->CanKnockback() )
	{
		if( pPlayer->IsRolling() )
		{
			if( !Knockback( vec ) )
			{
				pPlayer->Knockback( vec * -1 );
				OnKnockbackPlayer( vec );
			}
		}
		else if( pPlayer->GetSp() >= m_nKnockbackCostSp )
		{
			if( !Knockback( vec ) )
			{
				pPlayer->Knockback( vec * -1 );
				OnKnockbackPlayer( vec );
			}
			else
				pPlayer->CostSp( m_nKnockbackCostSp );
		}
		else
		{
			pPlayer->Knockback( vec * -1 );
			OnKnockbackPlayer( vec );
		}
	}
}