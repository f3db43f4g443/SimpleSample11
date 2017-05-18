#include "stdafx.h"
#include "Pickup.h"
#include "Player.h"
#include "MyLevel.h"

void CPickUp::OnAddedToStage()
{
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );
}

void CPickUp::PickUp( CPlayer* pPlayer )
{
	m_onPickedUp.Trigger( 0, pPlayer );
	Kill();
}

void CPickUp::Kill()
{
	if( m_pDeathEffect )
	{
		CMatrix2D mat = globalTransform;
		CMatrix2D mat1;
		mat1.Transform( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		( mat1 * mat ).Decompose( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		m_pDeathEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		m_pDeathEffect->SetState( 2 );
	}

	SetParentEntity( NULL );
}

void CPickUpCommon::PickUp( CPlayer* pPlayer )
{
	if( m_nHpRestore )
	{
		pPlayer->RestoreHp( m_nHpRestore );
	}

	CPickUp::PickUp( pPlayer );
}