#include "stdafx.h"
#include "SpecialWeapons0.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "MyLevel.h"
#include "Rand.h"
#include "Entities/Door.h"
#include "Entities/UtilEntities.h"
#include "Render/ParticleSystem.h"

void CDrill::OnAddedToStage()
{
	CBullet::OnAddedToStage();
	CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pParticle.GetPtr() );
	if( pParticle )
		pParticle->GetInstanceData()->GetData().isEmitting = false;
}

void CDrill::OnRemovedFromStage()
{
	CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pParticle.GetPtr() );
	if( pParticle )
		pParticle->GetInstanceData()->GetData().isEmitting = false;
	CBullet::OnRemovedFromStage();
}

void CDrill::OnTickBeforeHitTest()
{
	if( m_bDrilling && m_nLife )
		m_nLife++;
	CBullet::OnTickBeforeHitTest();
}

void CDrill::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
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

	CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pParticle.GetPtr() );
	if( pParticle )
		pParticle->GetInstanceData()->GetData().isEmitting = false;

	CVector2 vel = GetVelocity();
	float l = vel.Normalize();
	if( m_fSpeedNormal == 0 )
		m_fSpeedNormal = l;
	m_bDrilling = false;
	SetVelocity( vel * m_fSpeedNormal );
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity == m_pCreator )
			continue;
		CReference<CEntity> pTempRef = pEntity;

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

			if( pParticle )
				pParticle->GetInstanceData()->GetData().isEmitting = true;
			m_nHit++;
			m_nHitCDLeft = m_nHitCD;
			SetVelocity( vel * m_fSpeedHit1 * m_fSpeedNormal );
			m_bDrilling = true;
			if( m_nHit >= m_nDamage2 )
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

				if( pParticle )
					pParticle->GetInstanceData()->GetData().isEmitting = true;
				m_nHit++;
				m_nHitCDLeft = m_nHitCD;
				SetVelocity( vel * m_fSpeedHit1 * m_fSpeedNormal );
				m_bDrilling = true;
				if( m_nHit >= m_nDamage2 )
					Kill();

				return;
			}
		}
	}

	LINK_LIST_FOR_EACH_BEGIN( pManifold, m_pManifolds, SHitProxyManifold, Manifold )
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity == m_pCreator )
			continue;
		CReference<CEntity> pTempRef = pEntity;

		CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
		if( pEnemy && !pEnemy->IsIgnoreBullet() )
		{
			if( m_pCreator && pEnemy->IsOwner( m_pCreator ) )
				continue;
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
			m_nHitCDLeft = m_nHitCD;
			SetVelocity( vel * m_fSpeedHit2 * m_fSpeedNormal );
		}
	LINK_LIST_FOR_EACH_END( pManifold, m_pManifolds, SHitProxyManifold, Manifold )
}

void CBanknotePrinter::Equip( CPlayer * pPlayer )
{
}

void CBanknotePrinter::BeginFire( CPlayer * pPlayer )
{
	m_bIsFiring = true;
}

void CBanknotePrinter::EndFire( CPlayer * pPlayer )
{
	m_bIsFiring = false;
}

void CBanknotePrinter::Update( CPlayer * pPlayer )
{
	CVector2 dPos = pPlayer->GetAimAt() - pPlayer->GetPosition();
	if( dPos.Dot( dPos ) < 0.01f )
		dPos = CVector2( 0, 1 );
	SetRotation( atan2( dPos.y, dPos.x ) );

	if( m_bIsFiring && !m_nFireCD && m_vecBullets.size() < m_nMaxBullets )
	{
		ForceUpdateTransform();

		CBullet* pBullet = SafeCast<CBullet>( m_strBulletName->GetRoot()->CreateInstance() );
		pBullet->SetPosition( GetGlobalTransform().MulVector2Pos( m_fireOfs ) );
		CVector2 velocity = GetGlobalTransform().MulVector2Dir( CVector2( 0, SRand::Inst().Rand( m_fInitSpeed, m_fInitSpeed1 ) * ( dPos.x > 0 ? 1 : -1 ) ) );
		pBullet->SetVelocity( velocity );
		pBullet->SetTangentDir( true );
		pBullet->SetRotation( atan2( velocity.y, velocity.x ) );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		pBullet->SetDamage( m_nDamage, m_nDamage1, m_nDamage2 );
		CShakeObject* pShakeObject = new CShakeObject;
		pShakeObject->SetShakePerSec( m_fShakePerSecBullet );
		pShakeObject->SetParentEntity( pBullet );
		m_vecBullets.push_back( pBullet );

		if( m_strFireSound )
			m_strFireSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );

		m_nFireCD = m_nFireRate;
	}

	CVector2 dir( cos( r ), sin( r ) );
	for( int i = m_vecBullets.size() - 1; i >= 0; i-- )
	{
		auto& pBullet = m_vecBullets[i];
		if( !pBullet->GetParentEntity() )
		{
			if( i < m_vecBullets.size() - 1 )
				pBullet = m_vecBullets.back();
			m_vecBullets.pop_back();
			continue;
		}

		pBullet->SetCreator( pPlayer->GetCurRoom() ? pPlayer->GetCurRoom() : (CEntity*)pPlayer );
		CVector2 vel = pBullet->GetVelocity();
		if( !m_bIsFiring && !m_nFireCD )
		{
			float d = vel.Dot( dir );
			float l = vel.Length();
			float cs = d / l;
			pBullet->SetVelocity( dir * l );
			pBullet->SetAcceleration( CVector2( 0, 0 ) );
			pBullet->SetLife( m_nBulletLife );

			if( i < m_vecBullets.size() - 1 )
				pBullet = m_vecBullets.back();
			m_vecBullets.pop_back();
			m_nFireCD = m_nFireRate1;
			continue;
		}

		CVector2 dPos1 = pBullet->GetPosition() - pPlayer->GetPosition();
		float fRad = dPos1.Normalize();
		float g = vel.Length2() / Max( 1.0f, fRad );

		float fNorVel = vel.Dot( dPos1 );
		float fTanVel = vel.Dot( CVector2( dPos1.y, -dPos1.x ) );
		float fDir = fTanVel > 0 ? 1 : -1;
		CVector2 accDir( ( m_fOrbitRad - fRad ) * m_a - fNorVel * m_b - g, ( m_fTargetSpeed * fDir - fTanVel ) * m_c );
		accDir.Normalize();
		accDir = dPos1 * accDir.x + CVector2( dPos1.y, -dPos1.x ) * accDir.y;
		pBullet->SetAcceleration( accDir * m_fAcc );
	}
	if( m_nFireCD )
		m_nFireCD--;
}

void CBanknotePrinter::UnEquip( CPlayer * pPlayer )
{
	for( auto& pBullet : m_vecBullets )
	{
		if( pBullet->GetParentEntity() )
		{
			pBullet->SetAcceleration( CVector2( 0, 0 ) );
			pBullet->SetLife( m_nBulletLife );
		}
	}
	m_vecBullets.clear();
}
