#include "stdafx.h"
#include "CommonBlockItems.h"
#include "Stage.h"
#include "Player.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"

void CDetectTrigger::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 15, &m_onTick );

	m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab.c_str() );
}

void CDetectTrigger::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDetectTrigger::OnTick()
{
	if( m_nFireCountLeft )
	{
		m_nFireCountLeft--;
		GetStage()->RegisterAfterHitTest( m_nFireCountLeft ? m_nFireCD : m_nCD, &m_onTick );
		Trigger();
		return;
	}

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
		if( m_detectRect1.Contains( pos ) )
		{
			m_nFireCountLeft = m_nFireCount;
			if( m_nFireCountLeft )
				m_nFireCountLeft--;
			GetStage()->RegisterAfterHitTest( m_nFireCountLeft ? m_nFireCD : m_nCD, &m_onTick );
			Trigger();
			return;
		}

		if( m_detectRect.Contains( pos ) )
		{
			GetStage()->RegisterAfterHitTest( 1, &m_onTick );
			return;
		}
	}
	GetStage()->RegisterAfterHitTest( 15, &m_onTick );
}

void CSpawner::OnRemovedFromStage()
{
	while( m_pSpawnedEntities )
	{
		auto pSpawnedEntity = m_pSpawnedEntities;
		pSpawnedEntity->RemoveFrom_SpawnedEntity();
		delete pSpawnedEntity;
	}
	m_nCurCount = 0;
	CDetectTrigger::OnRemovedFromStage();
}

void CSpawner::Trigger()
{
	uint32 nSpawnCount = Min( m_nSpawnCount, m_nMaxCount - m_nCurCount );
	if( m_nTotalCount >= 0 )
		nSpawnCount = Min( nSpawnCount, (uint32)m_nTotalCount );
	if( !nSpawnCount )
		return;

	for( int i = 0; i < nSpawnCount; i++ )
	{
		auto pEntity = SafeCast<CEntity>( m_pPrefab->GetRoot()->CreateInstance() );
		pEntity->SetPosition( globalTransform.MulVector2Pos(
			CVector2( m_rectSpawn.x + SRand::Inst().Rand( 0.0f, m_rectSpawn.width ), m_rectSpawn.y + SRand::Inst().Rand( 0.0f, m_rectSpawn.height ) ) ) );
		if( m_bRandomRotate )
			pEntity->SetRotation( SRand::Inst().Rand( -PI, PI ) );

		switch( m_nVelocityType )
		{
		case 0:
			break;
		case 1:
		{
			CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
			if( pCharacter )
				pCharacter->SetVelocity( globalTransform.MulVector2Dir( CVector2( SRand::Inst().Rand( m_vel1.x, m_vel2.x ), SRand::Inst().Rand( m_vel1.y, m_vel2.y ) ) ) );
			break;
		}
		case 2:
		{
			CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
			if( pCharacter )
				pCharacter->SetVelocity( globalTransform.MulVector2Dir( m_vel1 + ( m_vel2 - m_vel1 ) * SRand::Inst().Rand( 0.0f, 1.0f ) ) );
			break;
		}
		case 3:
		{
			CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
			if( pCharacter )
			{
				float r = sqrt( SRand::Inst().Rand( m_vel1.x / m_vel2.x, 1.0f ) ) * m_vel2.x;
				float angle = SRand::Inst().Rand( m_vel1.y, m_vel2.y ) * PI / 180;
				pCharacter->SetVelocity( globalTransform.MulVector2Dir( CVector2( cos( angle ) * r, sin( angle ) * r ) ) );
			}
			break;
		}
		}

		pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
		SSpawnedEntity* pSpawnedEntity = new SSpawnedEntity( this, pEntity );
		Insert_SpawnedEntity( pSpawnedEntity );
	}
	m_nCurCount += nSpawnCount;
	if( m_nTotalCount >= 0 )
		m_nTotalCount -= nSpawnCount;
}

void CSpawner::OnSpawnedEntityDeath( SSpawnedEntity * pSpawnedEntity )
{
	m_nCurCount--;
	pSpawnedEntity->RemoveFrom_SpawnedEntity();
	delete pSpawnedEntity;
}

CSpawner::SSpawnedEntity::SSpawnedEntity( CSpawner * pOwner, CEntity * pSpawnedEntity )
	: onTick( this, &SSpawnedEntity::OnDeath )
	, pOwner( pOwner )
	, pSpawnedEntity( pSpawnedEntity )
{
	pSpawnedEntity->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &onTick );
}