#include "stdafx.h"
#include "FallingObj.h"
#include "Stage.h"
#include "Player.h"
#include "EnemyCharacters.h"
#include "Block.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"

void CFallingObj::OnTick()
{
	float fDeltaTime = GetStage()->GetElapsedTimePerTick();
	float fDeltaSpeed = m_fGravity * fDeltaTime;
	float fSpeed = fDeltaSpeed + m_fCurSpeed;
	if( fSpeed > m_fMaxSpeed )
	{
		fSpeed = m_fMaxSpeed;
		fDeltaSpeed = fSpeed - m_fCurSpeed;
	}

	float t = fDeltaSpeed / m_fGravity;
	float t1 = fDeltaTime - t;
	float ofs = t * 0.5f * ( fSpeed + m_fCurSpeed ) + t1 * fSpeed;
	SetPosition( CVector2( x, y - ofs ) );
	m_fCurSpeed = fSpeed;
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
}

void CFallingObj::OnTick1()
{
	if( !CMyLevel::GetInst()->GetBound().Contains( globalTransform.GetPosition() ) )
	{
		Destroy();
		return;
	}

	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

		CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject )
		{
			Destroy();
			return;
		}

		if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
		{
			Destroy();
			return;
		}

		CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
		if( pEnemy )
		{
			pEnemy->Damage( m_nHitDmg );
			Destroy();
			return;
		}

		CPlayer* pPlayer = SafeCast<CPlayer>( pEntity );
		if( pPlayer )
		{
			pPlayer->Damage();
			Destroy();
			return;
		}
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick1 );
}

void CFallingObj::Fall( float fInitFallSpeed )
{
	m_bFalling = true;
	m_fCurSpeed = fInitFallSpeed;
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
	GetStage()->RegisterAfterHitTest( 1, &m_onTick1 );
}

void CFallingObjHolder::OnAddedToStage()
{
	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunkObject )
	{
		pChunkObject->RegisterKilledEvent( &m_onChunkKilled );
		pChunkObject->RegisterCrushedEvent( &m_onChunkCrushed );
	}
}

void CFallingObjHolder::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	if( m_onChunkCrushed.IsRegistered() )
		m_onChunkCrushed.Unregister();
}

void CFallingObjHolder::OnKilled()
{
	float fInitFallSpeed = ( GetLastPos().y - GetPosition().y ) / GetStage()->GetElapsedTimePerTick();
	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunkObject )
	{
		fInitFallSpeed = pChunkObject->GetChunk()->GetFallSpeed();
	}

	auto pParentEntity = GetParentEntity();
	while( SafeCast<CBlockObject>( pParentEntity->GetParentEntity() ) )
		pParentEntity = pParentEntity->GetParentEntity();
	m_pFallingObj->ForceUpdateTransform();
	m_pFallingObj->SetParentAfterEntity( GetParentEntity() );
	m_pFallingObj->globalTransform.Decompose( m_pFallingObj->x, m_pFallingObj->y, m_pFallingObj->r, m_pFallingObj->s );

	m_pFallingObj->Fall( fInitFallSpeed );
	SetParentEntity( NULL );
}

void CFallingObjHolder::OnCrushed()
{
	m_pFallingObj->ForceUpdateTransform();
	m_pFallingObj->SetParentAfterEntity( GetParentEntity() );
	m_pFallingObj->globalTransform.Decompose( m_pFallingObj->x, m_pFallingObj->y, m_pFallingObj->r, m_pFallingObj->s );

	m_pFallingObj->Destroy();
	SetParentEntity( NULL );
}

void CFallingSpike::OnAddedToStage()
{
	CFallingObj::OnAddedToStage();
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
}

void CFallingSpike::Destroy()
{
	float fAngle0 = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < 6; i++ )
	{
		auto pBullet = SafeCast<CEnemyBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
		pBullet->SetPosition( globalTransform.GetPosition() );
		float r = fAngle0 + i * PI / 3;
		pBullet->SetRotation( r );
		pBullet->SetVelocity( CVector2( cos( r ), sin( r ) ) * 200 );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}

	SetParentEntity( NULL );
}