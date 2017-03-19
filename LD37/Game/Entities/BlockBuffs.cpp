#include "stdafx.h"
#include "BlockBuffs.h"
#include "Stage.h"

void CBlockBuffAcid::OnTick()
{
	CReference<CBlockBuffAcid> pTemp = this;

	auto pChunkObject = SafeCast<CBlockObject>( GetParentEntity() )->GetBlock()->pOwner->pChunkObject;
	pChunkObject->Damage( m_fDamage * GetStage()->GetElapsedTimePerTick() );

	if( GetStage() )
		CBlockBuff::OnTick();
}

void CBlockBuffAcid::OnAdded( uint8 nReason, SContext * pContext )
{
	if( m_nLife * m_fDamage < pContext->nLife * pContext->fParams[0] )
	{
		m_nLife = pContext->nLife;
		m_fDamage = pContext->fParams[0];
	}
}

void CBlockBuffAcid::OnRemoved( uint8 nReason )
{
	if( nReason == eRemovedReason_BlockDestroyed )
	{

	}
}
