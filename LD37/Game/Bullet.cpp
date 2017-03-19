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

void CBullet::SetVelocity( const CVector2& velocity )
{
	m_velocity = velocity;
}

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

	m_fDeathTime = m_fDeathFramesPerSec <= 0 ? 0 : ( m_nDeathFrameEnd - m_nDeathFrameBegin ) / m_fDeathFramesPerSec;
	if( m_fDeathTime <= 0 )
	{
		SetParentEntity( NULL );
		return;
	}

	m_bKilled = true;
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nDeathFrameBegin, m_nDeathFrameEnd, m_fDeathFramesPerSec );
	if( pParent )
	{
		globalTransform.Decompose( x, y, r, s );
		SetParentBeforeEntity( pParent );
	}
}

void CBullet::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );
}

void CBullet::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
	if( !m_bKilled )
		SetPosition( GetPosition() + m_velocity * GetStage()->GetElapsedTimePerTick() );
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
	pPlayer->Damage( 1 );
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
		pEnemy->Damage( m_nDmg );
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
		pChunkObject->AddHitShake( hitDir * 8 );
		pChunkObject->Damage( m_nDmg );
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
			pChunkObject->AddHitShake( hitDir * 8 );
			pChunkObject->Damage( m_nDmg );
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