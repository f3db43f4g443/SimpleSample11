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
#include "MyLevel.h"
#include "Render/Rope2D.h"
#include "Common/Rand.h"
#include "Entities/Enemies/Lv1Enemies.h"

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
	else if( SafeCast<CChunkObject>( pEntity ) )
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

void CWaterFall::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	m_nLifeLeft = m_nLife;
	m_fY0 = GetPosition().y;
	UpdateEft();
	UpdateAnim( m_nLife * GetStage()->GetElapsedTimePerTick() );
}

void CWaterFall::OnTickAfterHitTest()
{
	auto pEft = static_cast<CRopeObject2D*>( GetRenderObject() );
	SCharacterMovementData movementData;
	movementData.ResolvePenetration( this, CVector2( 0, -1 ), 0.5f );
	float fDist = CCharacterMoveUtil::StretchEx( this, 3, m_fMinLen, m_fMaxLen, m_fFall, movementData.bHitChannel );
	if( m_nState == 0 && fDist <= 0 )
	{
		pEft->GetInstanceData()->GetData().isEmitting = false;
		m_nState = 1;
	}
	if( m_nState > 0 )
	{
		if( m_nLife1 )
			m_nLife1--;
		if( m_nLifeLeft )
		{
			m_nLifeLeft--;
			if( !m_nLifeLeft )
			{
				SetParentEntity( NULL );
				return;
			}
		}
	}
	UpdateEft();

	if( m_nState == 0 || m_nLife1 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

			CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
			if( pCharacter && pCharacter != pPlayer )
			{
				if( !m_nDamage1 )
					continue;
				if( !SafeCast<CBullet>( pCharacter ) )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage1;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pManifold->hitPoint;
					context.hitDir = CVector2( 0, -1 );
					context.nHitType = -1;
					pCharacter->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				continue;
			}

			if( pPlayer && pPlayer->CanBeHit() && pEntity == pPlayer->GetCore() )
			{
				if( !m_nDamage && m_fKnockback <= 0 )
					continue;
				if( m_nDamage )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pManifold->hitPoint;
					context.hitDir = CVector2( 0, -1 );
					context.nHitType = -1;
					pPlayer->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				if( m_fKnockback > 0 && pPlayer->CanKnockback() )
				{
					CVector2 norm( 1, 0 );
					norm = ( pPlayer->globalTransform.GetPosition() - GetPosition() ).Dot( norm ) > 0 ? norm : norm * -1;
					pPlayer->Knockback( norm );
				}
			}
		}
	}

	CCharacter::OnTickAfterHitTest();
}

void CWaterFall::UpdateEft()
{
	auto pEft = static_cast<CRopeObject2D*>( GetRenderObject() );
	auto pRect = static_cast<SHitProxyPolygon*>( Get_HitProxy() );
	if( pRect->nVertices != 4 )
		return;
	float fMinX = FLT_MAX, fMaxX = -FLT_MAX, fMinY = FLT_MAX, fMaxY = -FLT_MAX;
	for( int i = 0; i < 4; i++ )
	{
		fMinX = Min( fMinX, pRect->vertices[i].x );
		fMaxX = Max( fMaxX, pRect->vertices[i].x );
		fMinY = Min( fMinY, pRect->vertices[i].y );
		fMaxY = Max( fMaxY, pRect->vertices[i].y );
	}
	float fLen = fMaxY - fMinY;
	float t = m_nLifeLeft * 1.0f / m_nLife;
	if( fLen > 32 )
	{
		pEft->SetDataCount( 4 );
		pEft->SetData( 0, CVector2( 0, 0 ), m_fWidth, CVector2( 0, 0 ), CVector2( 1, 2 * Max( 1 - ( m_fY0 - GetPosition().y ) / ( fLen * 0.125f ), 0.0f ) ) );
		pEft->SetData( 1, CVector2( 0, -16 ), m_fWidth, CVector2( 0, 16 / m_fTexYTileLen ), CVector2( 1, 1 ) );
		pEft->SetData( 2, CVector2( 0, -( fLen - 16 ) ), m_fWidth, CVector2( 0, ( fLen - 16 ) / m_fTexYTileLen ), CVector2( 1, 1 ) );
		pEft->SetData( 3, CVector2( 0, -fLen ), m_fWidth, CVector2( 0, fLen / m_fTexYTileLen ), CVector2( 1, 0 ) );
	}
	else
	{
		pEft->SetDataCount( 3 );
		pEft->SetData( 0, CVector2( 0, 0 ), m_fWidth, CVector2( 0, 0 ), CVector2( 1, 2 * Max( 1 - ( m_fY0 - GetPosition().y ) / ( fLen * 0.125f ), 0.0f ) ) );
		pEft->SetData( 1, CVector2( 0, -fLen * 0.5f / 2 ), m_fWidth, CVector2( 0, fLen * 0.5f / m_fTexYTileLen ), CVector2( 1, 1 ) );
		pEft->SetData( 2, CVector2( 0, -fLen ), m_fWidth, CVector2( 0, fLen / m_fTexYTileLen ), CVector2( 1, 0 ) );

	}
	pEft->SetTransformDirty();
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

void CThrowObj::OnTickBeforeHitTest()
{
	CEnemy::OnTickBeforeHitTest();
	if( SafeCast<CCharacter>( GetParentEntity() ) )
		return;
	m_nCurLife++;
	SetPosition( GetPosition() + m_velocity * GetStage()->GetElapsedTimePerTick() );
}

void CThrowObj::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	if( SafeCast<CCharacter>( GetParentEntity() ) )
		return;
	if( m_nCurLife >= m_nLife1 )
	{
		Kill();
		return;
	}

	CRectangle rect = CMyLevel::GetInst()->GetBoundWithLvBarrier();
	CRectangle rect0;
	Get_HitProxy()->CalcBound( globalTransform, rect0 );
	if( !rect.Contains( rect0 ) )
	{
		Kill();
		return;
	}

	if( m_nCurLife >= m_nLife )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				Kill();
				return;
			}
		}
	}
}

void CWaterSplash::OnAddedToStage()
{
	uint32 nFrame = m_nFrameOffset * SRand::Inst().Rand( 0u, m_nFrameRows );
	auto pImg = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	pImg->SetFrames( pImg->GetFrameBegin() + nFrame, pImg->GetFrameEnd() + nFrame, pImg->GetFramesPerSec() );
	m_nDeathFrameBegin += nFrame;
	m_nDeathFrameEnd += nFrame;
	CBullet::OnAddedToStage();
}

void CWaterSplash::OnTickAfterHitTest()
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

	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity == m_pCreator )
			continue;
		CReference<CEntity> pTempRef = pEntity;

		CMaggot* pMaggot = SafeCast<CMaggot>( pEntity );
		if( pMaggot )
		{
			pMaggot->Morph();
			OnHit( pMaggot );
			Kill();
			return;
		}

		if( m_nDamage )
		{
			CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
			if( pEnemy )
			{
				if( m_nDamage )
				{
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
				}
				OnHit( pEnemy );
				Kill();
				return;
			}
		}

		CDoor* pDoor = SafeCast<CDoor>( pEntity );
		if( pDoor && !pDoor->IsOpen() )
			pDoor->OpenForFrame( 3 );

		if( m_nDamage1 || m_fKnockback > 0 )
		{
			CPlayer* pPlayer = SafeCast<CPlayer>( pEntity );
			if( pPlayer )
			{
				if( m_fKnockback > 0 )
				{
					CVector2 dir = GetVelocity();
					dir.Normalize();
					pPlayer->Knockback( dir * m_fKnockback );
				}
				if( m_nDamage1 )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage1;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pManifold->hitPoint;
					context.hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
					context.nHitType = -1;
					pPlayer->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				OnHit( pPlayer );
				Kill();
				return;
			}
		}
	}
}
