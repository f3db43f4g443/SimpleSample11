#include "stdafx.h"
#include "BlockBuffs.h"
#include "Stage.h"
#include "Bullets.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"

void CBlockBuffAcid::OnAddedToStage()
{
	CBlockBuff::OnAddedToStage();
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBulletPrefab.c_str() );
}

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
		m_nTotalLife = Max( m_nTotalLife, pContext->nLife );
		m_nLife = pContext->nLife;
		m_fDamage = pContext->fParams[0];
	}
}

void CBlockBuffAcid::OnRemoved( uint8 nReason )
{
	if( nReason == eRemovedReason_BlockDestroyed )
	{
		auto pBlockObject = SafeCast<CBlockObject>( GetParentEntity() );
		auto pChunkObject = SafeCast<CChunkObject>( pBlockObject->GetParentEntity() );
		uint32 nBulletCount = floor( m_nLife / m_fLifePercentCostPerBullet / m_nTotalLife );
		if( !nBulletCount )
			return;

		ForceUpdateTransform();
		CVector2 center = globalTransform.GetPosition();
		for( int i = 0; i < nBulletCount; i++ )
		{
			auto pBullet = SafeCast<CBulletWithBlockBuff>( m_pBulletPrefab->GetRoot()->CreateInstance() );
			pBullet->SetCreator( this );
			pBullet->SetPosition( center );
			float fAngle = SRand::Inst().Rand( -PI, PI );
			pBullet->SetRotation( fAngle );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fBulletVelocityMin, m_fBulletVelocityMax ) );
			pBullet->SetAcceleration( CVector2( 0, -m_fBulletGravity ) );
			CBlockBuff::SContext context;
			context.nLife = m_nTotalLife * m_fNewBulletLifePercent;
			context.fParams[0] = m_fDamage;
			pBullet->Set( &context );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
	}
}
