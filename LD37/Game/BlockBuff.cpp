#include "stdafx.h"
#include "BlockBuff.h"
#include "Stage.h"
#include "MyLevel.h"

CBlockBuff* CBlockBuff::AddBuff( CPrefab* pPrefab, CBlockObject* pBlock, SContext* pContext )
{
	if( pBlock->GetBlock()->bImmuneToBlockBuff )
		return NULL;
	auto pBlockBuff = pPrefab->GetRoot()->GetStaticData<CBlockBuff>();
	if( !pBlockBuff )
		return NULL;

	return pBlockBuff->Add( pPrefab, pBlock, pContext );
}

CBlockBuff* CBlockBuff::Add( CPrefab* pPrefab, CBlockObject* pBlock, SContext* pContext ) const
{
	if( !m_bMulti )
	{
		for( auto pChild = pBlock->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
		{
			auto pInst = SafeCast<CBlockBuff>( pChild );
			if( pInst && pInst->GetName() == GetName() )
			{
				pInst->OnAdded( eAddedReason_Update, pContext );
				return pInst;
			}
		}
	}

	auto pInst = SafeCast<CBlockBuff>( pPrefab->GetRoot()->CreateInstance() );
	pInst->SetPosition( CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
	pInst->SetZOrder( -1 );
	pInst->SetParentEntity( pBlock );
	pInst->OnAdded( eAddedReason_New, pContext );
	return pInst;
}

void CBlockBuff::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
	GetParentEntity()->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onParentRemoved );
}

void CBlockBuff::OnRemovedFromStage()
{
	if( !m_bIsRemoving )
		OnRemoved( eRemovedReason_Default );
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( m_onParentRemoved.IsRegistered() )
		m_onParentRemoved.Unregister();
}

void CBlockBuff::OnTick()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
	UpdateAnim( GetStage()->GetElapsedTimePerTick() );
	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
		{
			OnRemoved( eRemovedReason_Timeout );
			SetParentEntity( NULL );
		}
	}
}