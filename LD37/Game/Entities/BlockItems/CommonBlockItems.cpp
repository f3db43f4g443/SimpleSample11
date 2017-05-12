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
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
		if( m_detectRect1.Contains( pos ) )
		{
			GetStage()->RegisterAfterHitTest( m_nCD, &m_onTick );
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
		pEntity->SetPosition( globalTransform.GetPosition() + CVector2( m_rectSpawn.x + SRand::Inst().Rand( 0.0f, m_rectSpawn.width ), m_rectSpawn.y + SRand::Inst().Rand( 0.0f, m_rectSpawn.height ) ) );
		if( m_bRandomRotate )
			pEntity->SetRotation( SRand::Inst().Rand( -PI, PI ) );
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