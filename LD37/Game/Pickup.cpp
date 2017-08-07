#include "stdafx.h"
#include "Pickup.h"
#include "Player.h"
#include "MyLevel.h"
#include "Stage.h"
#include "Render/LightRendering.h"

void CPickUp::OnAddedToStage()
{
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );

	if( m_pText && GetStage()->GetPlayer() )
	{
		GetStage()->GetPlayer()->RegisterItemChanged( &m_onRefreshText );
		GetStage()->GetPlayer()->RegisterMoneyChanged( &m_onRefreshPrice );
	}
	RefreshText();
	RefreshPrice();
}

void CPickUp::OnRemovedFromStage()
{
	if( m_onRefreshText.IsRegistered() )
		m_onRefreshText.Unregister();
	if( m_onRefreshPrice.IsRegistered() )
		m_onRefreshPrice.Unregister();
}

void CPickUp::DisablePickUp()
{
	SetTransparent( true );
}

void CPickUp::RefreshPrice()
{
	if( m_pPriceText )
	{
		if( m_nPrice )
		{
			bool bEnoughMoney = true;
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
				bEnoughMoney = pPlayer->GetMoney() >= m_nPrice;
			char szText[32];
			sprintf( szText, "%dG", m_nPrice );
			if( bEnoughMoney )
			{
				m_pPriceText->Set( szText );
				m_pPriceText1->Set( "" );
			}
			else
			{
				m_pPriceText1->Set( szText );
				m_pPriceText->Set( "" );
			}
		}
		else
		{
			m_pPriceText->Set( "" );
			m_pPriceText1->Set( "" );
		}
	}
}

void CPickUp::SetPrice( uint32 nPrice )
{
	m_nPrice = nPrice;
	RefreshPrice();
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
	if( m_nHeal )
		pPlayer->HealHp( m_nHeal );
	if( m_nHpRestore )
		pPlayer->RestoreHp( m_nHpRestore );
	if( m_nMoney )
		pPlayer->ModifyMoney( m_nMoney );

	CPickUp::PickUp( pPlayer );
}

uint8 CPickUpCommon::GetClass()
{
	if( m_nHeal )
		return 1;
	if( m_nHpRestore )
		return 2;
	if( m_nMoney )
		return 3;
	return 0;
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

void CPickUpTemplate::Set( CEntity * pEntity, uint32 nPrice )
{
	uint32 nClass = 0;
	auto pPickUpCommon = SafeCast<CPickUpCommon>( pEntity );
	if( pPickUpCommon )
	{
		nClass = pPickUpCommon->GetClass();
		pPickUpCommon->DisablePickUp();
	}

	auto pConsumable = SafeCast<CConsumable>( pEntity );
	if( pConsumable )
		nClass = 4;

	auto pLight = static_cast<CPointLightObject*>( m_pLight.GetPtr() );
	if( pLight )
	{
		if( m_lightBaseColor.x < 0 )
			m_lightBaseColor = pLight->baseColor;
		switch( nClass )
		{
		case 0:
			pLight->baseColor = CVector3( 1, 1, 1 ) * m_lightBaseColor;
			break;
		case 1:
			pLight->baseColor = CVector3( 0, 2, 0 ) * m_lightBaseColor;
			break;
		case 2:
			pLight->baseColor = CVector3( 0.8f, 1.6f, 0 ) * m_lightBaseColor;
			break;
		case 3:
			pLight->baseColor = CVector3( 1.3f, 1.3f, 0 ) * m_lightBaseColor;
			break;
		case 4:
			pLight->baseColor = CVector3( 0.5f, 0.5f, 1.7f ) * m_lightBaseColor;
			break;
		}
	}

	m_pEntity = pEntity;
	pEntity->SetPosition( m_ofs + pEntity->GetPosition() );
	pEntity->SetParentEntity( this );
	m_nPrice = ceil( nPrice * m_fPricePercent );

	if( GetStage() )
	{
		RefreshText();
		RefreshPrice();
	}
}

bool CPickUpTemplate::CanPickUp( CPlayer * pPlayer )
{
	auto pConsumable = SafeCast<CConsumable>( m_pEntity.GetPtr() );
	if( pConsumable )
	{
		if( pPlayer->CanAddConsumable( pConsumable ) < 0 )
			return false;
	}
	return true;
}

void CPickUpTemplate::PickUp( CPlayer * pPlayer )
{
	do
	{
		auto pItem = SafeCast<CItem>( m_pEntity.GetPtr() );
		if( pItem )
		{
			pPlayer->AddItem( pItem );
			break;
		}

		auto pConsumable = SafeCast<CConsumable>( m_pEntity.GetPtr() );
		if( pConsumable )
		{
			pPlayer->AddConsumable( pConsumable );
			break;
		}

		auto pPickUpCommon = SafeCast<CPickUpCommon>( m_pEntity.GetPtr() );
		if( pPickUpCommon )
		{
			pPickUpCommon->PickUp( pPlayer );
			break;
		}
	} while( 0 );

	CPickUp::PickUp( pPlayer );
}

void CPickUpTemplate::RefreshText()
{
	if( !m_pText )
		return;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	auto pItem = SafeCast<CItem>( m_pEntity.GetPtr() );
	if( !pItem )
		return;

	int32 nLevel = pPlayer->CheckItemLevel( pItem );
	char szText[32];
	if( nLevel > 0 )
		sprintf( szText, "LV%d", nLevel + 1 );
	else if( nLevel < 0 )
		strcpy( szText, "MAX" );
	else if( pItem->GetUpgrade() )
		sprintf( szText, "LV%d", 1 );
	else
		szText[0] = 0;
	m_pText->Set( szText );
}
