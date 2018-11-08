#include "stdafx.h"
#include "Enemy.h"
#include "Player.h"

void CEnemy::Damage( SDamageContext& context )
{
	context.nHitType = m_nHitType;
	int32 nDmg = context.nDamage;
	if( m_nHp )
	{
		nDmg = ceil( nDmg * ( 1 - m_fDefence ) );
		m_nHp -= nDmg;
		context.nDamage = nDmg;
		if( m_nHp <= 0 )
		{
			context.nDamage += m_nHp;
			m_nHp = 0;
			m_onHpChanged.Trigger( 0, this );
			Kill();
		}
		else
			m_onHpChanged.Trigger( 0, this );
	}
	else
		context.nDamage = 0;
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
		if( pPlayer->IsRolling() || pPlayer->GetSpecialFlag( CPlayer::eSpecialFlag_Strength ) )
		{
			if( !Knockback( vec ) )
			{
				pPlayer->Knockback( vec * -1 );
				OnKnockbackPlayer( vec );
			}
		}
		else if( pPlayer->CheckCostSp( m_nKnockbackCostSp, 1 ) )
		{
			if( !Knockback( vec ) )
			{
				pPlayer->Knockback( vec * -1 );
				OnKnockbackPlayer( vec );
			}
			else
				pPlayer->CostSp( m_nKnockbackCostSp, 1 );
		}
		else
		{
			pPlayer->Knockback( vec * -1 );
			OnKnockbackPlayer( vec );
		}
	}
}

void CEnemyPart::Damage( SDamageContext& context )
{
	CEnemy* pEnemy = SafeCast<CEnemy>( GetParentEntity() );
	if( pEnemy )
	{
		uint8 nHitType = m_nHitType;
		context.nDamage = ceil( context.nDamage * ( 1 - m_fDefence ) );
		pEnemy->Damage( context );
		context.nHitType = nHitType;
	}
}

bool CEnemyPart::IsOwner( CEntity* pEntity )
{
	if( pEntity == this )
		return true;
	auto pParent = SafeCast<CEnemy>( GetParentEntity() );
	if( pParent )
		return pParent->IsOwner( pEntity );
	return false;
}
