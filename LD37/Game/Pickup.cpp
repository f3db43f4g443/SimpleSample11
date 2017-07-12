#include "stdafx.h"
#include "Pickup.h"
#include "Player.h"
#include "MyLevel.h"
#include "Stage.h"

void CPickUp::OnAddedToStage()
{
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );

	if( m_pText && GetStage()->GetPlayer() )
	{
		GetStage()->GetPlayer()->RegisterItemChanged( &m_onRefreshText );
	}
	RefreshText();
}

void CPickUp::OnRemovedFromStage()
{
	if( m_onRefreshText.IsRegistered() )
		m_onRefreshText.Unregister();
}

void CPickUp::PickUp( CPlayer* pPlayer )
{
	DEFINE_TEMP_REF_THIS()
	Kill();
	m_onPickedUp.Trigger( 0, pPlayer );
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
		pPlayer->RestoreHp( m_nHpRestore );
	if( m_nMoney )
		pPlayer->AddMoney( m_nMoney );

	CPickUp::PickUp( pPlayer );
}

void CPickUpItem::PickUp( CPlayer* pPlayer )
{
	pPlayer->AddItem( m_pItem );
	CPickUp::PickUp( pPlayer );
}

void CPickUpItem::RefreshText()
{
	if( !m_pText )
		return;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;

	int32 nLevel = pPlayer->CheckItemLevel( m_pItem );
	m_pItem->NextItem();
	char szText[32];
	if( nLevel > 0 )
		sprintf( szText, "LV%d", nLevel + 1 );
	else if( nLevel < 0 )
		strcpy( szText, "MAX" );
	else if( m_pItem->GetUpgrade() )
		sprintf( szText, "LV%d", 1 );
	else
		szText[0] = 0;
	m_pText->Set( szText );
}
