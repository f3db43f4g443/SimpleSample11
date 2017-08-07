#include "stdafx.h"
#include "Consumables.h"
#include "Player.h"
#include "Stage.h"

bool CConsumableHealing::Use( CPlayer * pPlayer )
{
	pPlayer->HealHp( m_fPercent * pPlayer->GetMaxHp() );
	return true;
}

bool CConsumableRepair::Use( CPlayer * pPlayer )
{
	CChunkObject* pChunkObject = pPlayer->GetCurRoom();
	if( !pChunkObject || !pChunkObject->GetMaxHp() )
		return false;
	pChunkObject->Repair( m_fPercent * pChunkObject->GetMaxHp() );
	return true;
}

bool CConsumableEffect::Use( CPlayer * pPlayer )
{
	m_pEffect->SetParentEntity( pPlayer );
	return true;
}

void CPlayerSpecialEffect::OnAddedToStage()
{
	CPlayer* pPlayer = SafeCast<CPlayer>( GetParentEntity() );
	pPlayer->IncSpecialFlag( m_nType );
	GetStage()->RegisterAfterHitTest( m_nTime, &m_onTick );
	SetAutoUpdateAnim( true );
}

void CPlayerSpecialEffect::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CPlayer* pPlayer = SafeCast<CPlayer>( GetParentEntity() );
	pPlayer->DecSpecialFlag( m_nType );
}
