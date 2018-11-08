#include "stdafx.h"
#include "Bullet.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "xml.h"
#include "FileUtil.h"
#include "MyLevel.h"
#include "Entities/Barrage.h"
#include "Block.h"
#include "Entities/Door.h"
#include "ParticleSystem.h"

void CBullet::Kill()
{
	auto pParent = SafeCast<CBarrage>( GetParentEntity() );
	if( m_pDeathEffect )
	{
		CMatrix2D mat = globalTransform;
		CMatrix2D mat1;
		mat1.Transform( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		( mat1 * mat ).Decompose( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		m_pDeathEffect->SetParentBeforeEntity( pParent ? (CEntity*)pParent : this );
		m_pDeathEffect->SetState( 2 );
	}

	if( m_pParticle )
	{
		static_cast<CParticleSystemObject*>( m_pParticle.GetPtr() )->GetInstanceData()->GetData().isEmitting = false;
	}

	m_fDeathTime = m_fDeathFramesPerSec <= 0 ? 0 : ( m_nDeathFrameEnd - m_nDeathFrameBegin ) / m_fDeathFramesPerSec;
	if( m_fDeathTime <= 0 )
	{
		SetParentEntity( NULL );
		return;
	}

	m_bKilled = true;
	if( m_nDeathFrameEnd > m_nDeathFrameBegin )
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nDeathFrameBegin, m_nDeathFrameEnd, m_fDeathFramesPerSec );
	else
		GetRenderObject()->bVisible = false;
	if( pParent )
	{
		globalTransform.Decompose( x, y, r, s );
		SetTransformIndex( -1 );
		SetParentBeforeEntity( pParent );
	}
}

void CBullet::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );

	switch( m_nBoundType )
	{
	case 0:
		m_bound = CMyLevel::GetInst()->GetLargeBound();
		break;
	case 1:
	case 3:
		m_bound = CMyLevel::GetInst()->GetBound();
		break;
	default:
	{
		CRectangle rect1 = CMyLevel::GetInst()->GetBound();
		CRectangle rect2 = CMyLevel::GetInst()->GetLargeBound();
		m_bound = CRectangle( rect2.x, rect1.y, rect2.width, rect1.height );
		break;
	}
	}
}

void CBullet::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
	if( !m_bKilled )
	{
		CVector2 newVelocity = m_velocity + m_acc * GetStage()->GetElapsedTimePerTick();
		SetPosition( GetPosition() + ( m_velocity + newVelocity ) * ( GetStage()->GetElapsedTimePerTick() * 0.5f ) );
		m_velocity = newVelocity;
		if( m_bTangentDir )
			SetRotation( atan2( m_velocity.y, m_velocity.x ) );
		else
			SetRotation( r + m_fAngularVelocity * GetStage()->GetElapsedTimePerTick() );
	}
	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
			Kill();
	}
}

void CBullet::OnTickAfterHitTest()
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
		if( m_nBoundType == 3 )
		{
			if( x < m_bound.x && m_velocity.x < 0 || x > m_bound.GetRight() && m_velocity.x > 0 )
				m_velocity.x = -m_velocity.x;
			if( y < m_bound.y && m_velocity.y < 0 || y > m_bound.GetBottom() && m_velocity.y > 0 )
				m_velocity.y = -m_velocity.y;
		}
		else
		{
			CVector2 globalPos = globalTransform.GetPosition();
			globalPos.x = Min( m_bound.GetRight(), Max( m_bound.x, globalPos.x ) );
			globalPos.y = Min( m_bound.GetBottom(), Max( m_bound.y, globalPos.y ) );
			globalTransform.SetPosition( globalPos );
			Kill();
			return;
		}
	}

	switch( m_nType )
	{
	case 0:
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pEntity == m_pCreator )
				continue;

			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
				if( pBlockObject )
				{
					auto pChunk = pBlockObject->GetBlock()->pOwner->pChunkObject;
					if( pPlayer && pPlayer->IsHiding() && pChunk == pPlayer->GetCurRoom() )
					{
						CReference<CEntity> pTempRef = pEntity;
						pChunk->Damage( m_nDamage1 );
						OnHit( pBlockObject );
						Kill();
						return;
					}
				}
				/*else
				{
					Kill();
					return;
				}*/
			}

			CDoor* pDoor = SafeCast<CDoor>( pEntity );
			if( pDoor && !pDoor->IsOpen() )
			{
				auto pChunk = SafeCast<CChunkObject>( pDoor->GetParentEntity() );
				if( pPlayer && pPlayer->IsHiding() && pChunk == pPlayer->GetCurRoom() )
				{
					CReference<CEntity> pTempRef = pEntity;
					pChunk->Damage( m_nDamage1 );
					OnHit( pChunk );
					Kill();
					return;
				}
			}

			if( pEntity && pPlayer && pPlayer->CanBeHit() && pEntity == pPlayer->GetCore() )
			{
				CReference<CEntity> pTempRef = pEntity;

				SDamageContext context;
				context.nDamage = m_nDamage;
				context.nType = 0;
				context.nSourceType = 0;
				context.hitPos = pManifold->hitPoint;
				context.hitDir = m_velocity;
				context.nHitType = -1;
				pPlayer->Damage( context );
				if( m_pDmgEft )
					m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );

				OnHit( pPlayer );
				Kill();
				return;
			}
		}
	}
	break;
	case 1:
	{
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pEntity == m_pCreator )
				continue;
			CReference<CEntity> pTempRef = pEntity;

			CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
			if( pEnemy )
			{
				if( m_pCreator && pEnemy->IsOwner( m_pCreator ) )
					continue;
				CReference<CEntity> pTempRef = pEntity;

				SDamageContext context;
				context.nDamage = m_nDamage;
				context.nType = 0;
				context.nSourceType = 0;
				context.hitPos = pManifold->hitPoint;
				context.hitDir = m_velocity;
				context.nHitType = -1;
				pEnemy->Damage( context );
				if( m_pDmgEft )
					m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );

				OnHit( pEnemy );
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
					Kill();
					return;
				}
			}

			/*if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				Kill();
				return;
			}*/
		}
	}
	break;
	default:
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pEntity == m_pCreator )
				continue;
			CReference<CEntity> pTempRef = pEntity;

			CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
			if( pCharacter && pCharacter != pPlayer )
			{
				if( m_pCreator )
				{
					auto pEnemy = SafeCast<CEnemy>( pEntity );
					if( pEnemy && pEnemy->IsOwner( m_pCreator ) )
						continue;
				}

				if( !m_nDamage2 )
					continue;
				if( SafeCast<CBullet>( pCharacter ) )
					continue;
				CReference<CEntity> pTempRef = pEntity;

				SDamageContext context;
				context.nDamage = m_nDamage2;
				context.nType = 0;
				context.nSourceType = 0;
				context.hitPos = pManifold->hitPoint;
				context.hitDir = m_velocity;
				context.nHitType = -1;
				pCharacter->Damage( context );
				if( m_pDmgEft )
					m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );

				OnHit( pCharacter );
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
				CChunkObject::SDamageContext dmgContext = { m_nDamage, 0, eDamageSourceType_Bullet, hitDir * 8 };
				pChunkObject->Damage( dmgContext );
				OnHit( pBlockObject );
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
					CChunkObject::SDamageContext dmgContext = { m_nDamage, 0, eDamageSourceType_Bullet, hitDir * 8 };
					pChunkObject->Damage( dmgContext );
					OnHit( pChunkObject );
					Kill();
					return;
				}
			}

			if( pEntity && pPlayer && pPlayer->CanBeHit() && pEntity == pPlayer->GetCore() )
			{
				if( !m_nDamage1 )
					continue;
				CReference<CEntity> pTempRef = pEntity;

				SDamageContext context;
				context.nDamage = m_nDamage1;
				context.nType = 0;
				context.nSourceType = 0;
				context.hitPos = pManifold->hitPoint;
				context.hitDir = m_velocity;
				context.nHitType = -1;
				pPlayer->Damage( context );
				if( m_pDmgEft )
					m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );

				OnHit( pPlayer );
				Kill();
				return;
			}
		}
	}
		break;
	}
}

void CEnemyBullet::OnTickAfterHitTest()
{
	CBullet::OnTickAfterHitTest();
	if( m_bKilled )
		return;
	CPlayer* pPlayer = GetStage()->GetPlayer();

	if( !CMyLevel::GetInst()->GetLargeBound().Contains( globalTransform.GetPosition() ) )
	{
		Kill();
		return;
	}

	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity == m_pCreator )
			continue;

		if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
		{
			CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
			if( pBlockObject )
			{
				auto pChunk = pBlockObject->GetBlock()->pOwner->pChunkObject;
				if( pPlayer && pPlayer->IsHiding() && pChunk == pPlayer->GetCurRoom() )
				{
					pChunk->Damage( 1 );
					Kill();
					return;
				}
			}
			else
			{
				Kill();
				return;
			}
		}

		CDoor* pDoor = SafeCast<CDoor>( pEntity );
		if( pDoor && !pDoor->IsOpen() )
		{
			auto pChunk = SafeCast<CChunkObject>( pDoor->GetParentEntity() );
			if( pPlayer && pPlayer->IsHiding() && pChunk == pPlayer->GetCurRoom() )
			{
				pChunk->Damage( 5 );
				Kill();
				return;
			}
		}

		if( pEntity && pPlayer && pPlayer->CanBeHit() && pEntity == pPlayer->GetCore() )
		{
			OnHitPlayer( pPlayer );
			Kill();
			return;
		}
	}
}

void CEnemyBullet::OnHitPlayer( CPlayer* pPlayer )
{
	SDamageContext context;
	context.nDamage = 1;
	context.nType = 0;
	context.nSourceType = 0;
	context.hitPos = context.hitDir = CVector2( 0, 0 );
	context.nHitType = -1;
	pPlayer->Damage( context );
}

void CPlayerBullet::OnTickAfterHitTest()
{
	CBullet::OnTickAfterHitTest();
	if( m_bKilled )
		return;
	if( !CMyLevel::GetInst()->GetLargeBound().Contains( globalTransform.GetPosition() ) )
	{
		Kill();
		return;
	}

	CPlayer* pPlayer = GetStage()->GetPlayer();
	CReference<CEntity> temp = this;
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		OnHit( pEntity );
		if( m_bKilled || !GetStage() )
			return;
	}
}

void CPlayerBullet::OnHit( CEntity* pEntity )
{
	CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
	if( pEnemy )
	{
		SDamageContext context;
		context.nDamage = m_nDmg;
		context.nType = 0;
		context.nSourceType = 0;
		context.hitPos = context.hitDir = CVector2( 0, 0 );
		context.nHitType = -1;
		pEnemy->Damage( context );

		Kill();
		return;
	}

	CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject && pBlockObject->GetBlock()->eBlockType == eBlockType_Block )
	{
		auto pChunkObject = pBlockObject->GetBlock()->pOwner->pChunkObject;
		if( pChunkObject == m_pCreator )
			return;
		CVector2 hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
		hitDir.Normalize();
		CChunkObject::SDamageContext dmgContext = { m_nDamage, 0, eDamageSourceType_Bullet, hitDir * 8 };
		pChunkObject->Damage( dmgContext );
		Kill();
		return;
	}
	else if( pEntity == m_pCreator )
		return;

	CDoor* pDoor = SafeCast<CDoor>( pEntity );
	if( pDoor && !pDoor->IsOpen() )
	{
		auto pChunkObject = SafeCast<CChunkObject>( pDoor->GetParentEntity() );
		if( pChunkObject )
		{
			if( pChunkObject == m_pCreator )
				return;
			CVector2 hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
			hitDir.Normalize();
			CChunkObject::SDamageContext dmgContext = { m_nDamage, 0, eDamageSourceType_Bullet, hitDir * 8 };
			pChunkObject->Damage( dmgContext );
			Kill();
			return;
		}
	}

	if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
	{
		Kill();
		return;
	}
}