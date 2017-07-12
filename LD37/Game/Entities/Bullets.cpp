#include "stdafx.h"
#include "Bullets.h"
#include "BlockBuffs.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "Block.h"
#include "Entities/Door.h"

void CBomb::OnAddedToStage()
{
	CBullet::OnAddedToStage();
	m_pExp->SetParentEntity( NULL );
}

void CBomb::OnHit( CEntity * pEntity )
{
	if( SafeCast<CBlockObject>( pEntity ) )
	{
		if( m_bExplodeOnHitBlock )
			Explode();
	}
	else if( SafeCast<CCharacter>( pEntity ) )
	{
		if( m_bExplodeOnHitChar )
			Explode();
	}
	else
	{
		if( m_bExplodeOnHitWorld )
			Explode();
	}
	m_pExp = NULL;

	CBullet::OnHit( pEntity );
}

void CBomb::Kill()
{
	if( m_bExplodeOnHitWorld )
		Explode();
	CBullet::Kill();
}

void CBomb::Explode()
{
	if( m_pExp )
	{
		auto pParent = SafeCast<CBarrage>( GetParentEntity() );
		if( pParent )
			m_pExp->SetPosition( globalTransform.GetPosition() );
		else
			m_pExp->SetPosition( GetPosition() );
		m_pExp->SetParentBeforeEntity( pParent ? (CEntity*)pParent : this );
		m_pExp = NULL;
	}
}

void CBulletWithBlockBuff::OnAddedToStage()
{
	CBullet::OnAddedToStage();
	m_pBlockBuff = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBlockBuff.c_str() );
}

void CBulletWithBlockBuff::OnHit( CEntity * pEntity )
{
	auto pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject && pBlockObject->GetStage() )
	{
		CBlockBuff::AddBuff( m_pBlockBuff, pBlockObject, &m_context );
	}
}

void CExplosionWithBlockBuff::OnAddedToStage()
{
	CExplosion::OnAddedToStage();
	m_pBlockBuff = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBlockBuff.c_str() );
}

void CExplosionWithBlockBuff::OnHit( CEntity * pEntity )
{
	auto pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject && pBlockObject->GetStage() )
	{
		CBlockBuff::AddBuff( m_pBlockBuff, pBlockObject, &m_context );
	}
}

void CExplosionKnockback::OnHit( CEntity * pEntity )
{
	CCharacter* pChar = SafeCast<CCharacter>( pEntity );
	if( pChar )
	{
		CVector2 dir = pChar->globalTransform.GetPosition() - globalTransform.GetPosition();
		dir.Normalize();
		pChar->Knockback( dir * m_fKnockbackStrength );
	}
	CExplosion::OnHit( pEntity );
}

void CPlayerBulletMultiHit::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( m_bKilled )
	{
		m_fDeathTime -= GetStage()->GetElapsedTimePerTick();
		if( m_fDeathTime <= 0 )
		{
			SetParentEntity( NULL );
		}
		return;
	}

	if( !m_bound.Contains( globalTransform.GetPosition() ) )
	{
		CVector2 globalPos = globalTransform.GetPosition();
		globalPos.x = Min( m_bound.GetRight(), Max( m_bound.x, globalPos.x ) );
		globalPos.y = Min( m_bound.GetBottom(), Max( m_bound.y, globalPos.y ) );
		globalTransform.SetPosition( globalPos );
		Kill();
		return;
	}

	if( m_nHitCDLeft )
		m_nHitCDLeft--;
	if( m_nHitCDLeft )
		return;

	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity == m_pCreator )
			continue;
		CReference<CEntity> pTempRef = pEntity;

		CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
		if( pEnemy )
		{
			CReference<CEntity> pTempRef = pEntity;

			CCharacter::SDamageContext context;
			context.nDamage = m_nDamage;
			context.nType = 0;
			context.nSourceType = 0;
			context.hitPos = pManifold->hitPoint;
			context.hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
			context.nHitType = -1;
			pEnemy->Damage( context );
			if( m_pDmgEft )
				m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );

			OnHit( pEnemy );
			m_nHit++;
			m_nHitCDLeft = m_nHitCD;
			if( m_nHit >= m_nDamage2 )
				Kill();
			return;
		}

		CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject && pBlockObject->GetBlock()->eBlockType == eBlockType_Block )
		{
			auto pChunkObject = pBlockObject->GetBlock()->pOwner->pChunkObject;
			if( pChunkObject == m_pCreator )
				continue;
			CVector2 hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
			hitDir.Normalize();
			CReference<CEntity> pTempRef = pEntity;
			CChunkObject::SDamageContext dmgContext = { m_nDamage1, 0, eDamageSourceType_Bullet, hitDir * 8 };
			pChunkObject->Damage( dmgContext );
			OnHit( pBlockObject );
			if( m_bMultiHitBlock )
			{
				m_nHit++;
				m_nHitCDLeft = m_nHitCD;
				if( m_nHit >= m_nDamage2 )
					Kill();
			}
			else
				Kill();
			return;
		}

		CDoor* pDoor = SafeCast<CDoor>( pEntity );
		if( pDoor && !pDoor->IsOpen() )
		{
			auto pChunkObject = SafeCast<CChunkObject>( pDoor->GetParentEntity() );
			if( pChunkObject )
			{
				if( pChunkObject == m_pCreator )
					continue;
				CVector2 hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
				hitDir.Normalize();
				CReference<CEntity> pTempRef = pEntity;
				CChunkObject::SDamageContext dmgContext = { m_nDamage1, 0, eDamageSourceType_Bullet, hitDir * 8 };
				pChunkObject->Damage( dmgContext );
				OnHit( pChunkObject );
				if( m_bMultiHitBlock )
				{
					m_nHit++;
					m_nHitCDLeft = m_nHitCD;
					if( m_nHit >= m_nDamage2 )
						Kill();
				}
				else
					Kill();
				return;
			}
		}
	}
}
