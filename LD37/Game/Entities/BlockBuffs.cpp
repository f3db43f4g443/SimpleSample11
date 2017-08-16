#include "stdafx.h"
#include "BlockBuffs.h"
#include "Stage.h"
#include "Bullets.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"

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
			auto pBullet = SafeCast<CBulletWithBlockBuff>( m_strBulletPrefab->GetRoot()->CreateInstance() );
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

void CBlockBuffFire::OnTick()
{
	CReference<CBlockBuffFire> pTemp = this;

	auto pChunkObject = SafeCast<CBlockObject>( GetParentEntity() )->GetBlock()->pOwner->pChunkObject;
	pChunkObject->Damage( m_fDamage * GetStage()->GetElapsedTimePerTick() );

	if( !GetStage() )
		return;

	if( m_nExplosionTick )
		m_nExplosionTick--;
	if( m_nExplosionTick == 0 )
	{
		m_nExplosionTick = SRand::Inst().Rand( 110, 130 );

		auto pBlockObject = SafeCast<CBlockObject>( GetParentEntity() );
		auto pChunkObject = SafeCast<CChunkObject>( pBlockObject->GetParentEntity() );
		ForceUpdateTransform();
		CVector2 center = globalTransform.GetPosition();
		
		auto pExp = SafeCast<CExplosionWithBlockBuff>( m_strExplosionPrefab->GetRoot()->CreateInstance() );
		pExp->SetCreator( this );
		pExp->SetPosition( center );
		CBlockBuff::SContext context;
		context.nLife = m_nLife;
		context.fParams[0] = m_fDamage;
		pExp->Set( &context );
		pExp->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}

	if( GetStage() )
		CBlockBuff::OnTick();
}

void CBlockBuffFire::OnAdded( uint8 nReason, SContext * pContext )
{
	if( nReason == eAddedReason_New )
		m_nExplosionTick = SRand::Inst().Rand( 110, 130 );

	if( m_nLife * m_fDamage < pContext->nLife * pContext->fParams[0] )
	{
		m_nTotalLife = Max( m_nTotalLife, pContext->nLife );
		m_nLife = pContext->nLife;
		m_fDamage = pContext->fParams[0];
	}
}
