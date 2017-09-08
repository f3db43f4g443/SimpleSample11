#include "stdafx.h"
#include "CommonBlockItems.h"
#include "Stage.h"
#include "Player.h"
#include "MyLevel.h"
#include "Block.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "Pickup.h"
#include "GlobalCfg.h"

void CDetectTrigger::OnAddedToStage()
{
	if( m_bEnabled )
		GetStage()->RegisterAfterHitTest( 15, &m_onTick );
}

void CDetectTrigger::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDetectTrigger::SetEnabled( bool bEnabled )
{
	if( m_bEnabled == bEnabled )
		return;
	m_bEnabled = bEnabled;
	if( bEnabled )
	{
		if( GetStage() && !m_onTick.IsRegistered() )
			GetStage()->RegisterAfterHitTest( 15, &m_onTick );
	}
	else
	{
		if( m_onTick.IsRegistered() )
			m_onTick.Unregister();
	}
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

	if( CheckTrigger() )
	{
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
	}
	GetStage()->RegisterAfterHitTest( 15, &m_onTick );
}

void CKillTrigger::OnAddedToStage()
{
	auto pPar = SafeCast<CChunkObject>( GetParentEntity() );
	if( pPar )
		pPar->RegisterKilledEvent( &m_onKilled );
	else
	{
		auto pChar = SafeCast<CCharacter>( GetParentEntity() );
		if( pChar )
			pChar->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onKilled );
	}
}

void CKillTrigger::OnRemovedFromStage()
{
	if( m_onKilled.IsRegistered() )
		m_onKilled.Unregister();
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
	uint32 nSpawnCount = m_nSpawnCount;
	if( m_nMaxCount )
		nSpawnCount = Min( nSpawnCount, m_nMaxCount - m_nCurCount );
	if( m_nTotalCount >= 0 )
		nSpawnCount = Min( nSpawnCount, (uint32)m_nTotalCount );
	if( !nSpawnCount )
		return;

	for( int i = 0; i < nSpawnCount; i++ )
	{
		auto pEntity = Spawn();
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

		pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		if( m_nMaxCount )
		{
			SSpawnedEntity* pSpawnedEntity = new SSpawnedEntity( this, pEntity );
			Insert_SpawnedEntity( pSpawnedEntity );
		}
	}
	if( m_nMaxCount )
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

void CRandomSpawner::OnAddedToStage()
{
	CSpawner::OnAddedToStage();
}

CEntity * CRandomSpawner::Spawn()
{
	float fTotalChance = 0;
	int32 nCount;
	CPrefab* pPrefabs[4] = { m_strPrefab0, m_strPrefab1, m_strPrefab2, m_strPrefab3 };
	for( nCount = 0; nCount < ELEM_COUNT( pPrefabs ); nCount++ )
	{
		if( !pPrefabs[nCount] )
			break;
		fTotalChance += m_fChances[nCount];
	}

	float r = SRand::Inst().Rand( 0.0f, fTotalChance );
	int n;
	for( n = 0; n < nCount - 1; n++ )
	{
		r -= m_fChances[n];
		if( r < 0 )
			break;
	}

	auto pEntity = SafeCast<CEntity>( pPrefabs[n]->GetRoot()->CreateInstance() );
	return pEntity;
}

void CKillSpawner::OnAddedToStage()
{
	CKillTrigger::OnAddedToStage();
}

void CKillSpawner::Trigger()
{
	float fTotalChance = 0;
	int32 nCount;
	CPrefab* pPrefabs[4] = { m_strPrefab0, m_strPrefab1, m_strPrefab2, m_strPrefab3 };
	for( nCount = 0; nCount < ELEM_COUNT( pPrefabs ); nCount++ )
	{
		if( !pPrefabs[nCount] )
			break;
		fTotalChance += m_fChances[nCount];
	}
	if( !nCount )
		return;

	int32 nSpawnCount = SRand::Inst().Rand( m_nMinCount, m_nMaxCount );
	for( int i = 0; i < nSpawnCount; i++ )
	{
		float r = SRand::Inst().Rand( 0.0f, fTotalChance );
		int n;
		for( n = 0; n < nCount - 1; n++ )
		{
			r -= m_fChances[n];
			if( r < 0 )
				break;
		}

		auto pEntity = SafeCast<CEntity>( pPrefabs[n]->GetRoot()->CreateInstance() );
		pEntity->SetPosition( globalTransform.MulVector2Pos(
			CVector2( m_rectSpawn.x + SRand::Inst().Rand( 0.0f, m_rectSpawn.width ), m_rectSpawn.y + SRand::Inst().Rand( 0.0f, m_rectSpawn.height ) ) ) );

		CVector2 vel( 1, 0 );
		CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
		if( pCharacter )
		{
			switch( m_nVelocityType )
			{
			case 0:
				break;
			case 1:
			{
				pCharacter->SetVelocity( globalTransform.MulVector2Dir( CVector2( SRand::Inst().Rand( m_vel1.x, m_vel2.x ), SRand::Inst().Rand( m_vel1.y, m_vel2.y ) ) ) );
				break;
			}
			case 2:
			{
				pCharacter->SetVelocity( globalTransform.MulVector2Dir( m_vel1 + ( m_vel2 - m_vel1 ) * SRand::Inst().Rand( 0.0f, 1.0f ) ) );
				break;
			}
			case 3:
			{

				float r = sqrt( SRand::Inst().Rand( m_vel1.x / m_vel2.x, 1.0f ) ) * m_vel2.x;
				float angle = SRand::Inst().Rand( m_vel1.y, m_vel2.y ) * PI / 180;
				pCharacter->SetVelocity( globalTransform.MulVector2Dir( CVector2( cos( angle ) * r, sin( angle ) * r ) ) );
				break;
			}
			}
			vel = pCharacter->GetVelocity();
		}
		if( m_bRandomRotate )
			pEntity->SetRotation( SRand::Inst().Rand( -PI, PI ) );
		else if( m_bTangentRotate )
			pEntity->SetRotation( atan2( vel.y, vel.x ) );

		pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
	}
}

void CShop::OnAddedToStage()
{
	int32 nPickUp = 0;
	for( auto pPickUp = m_pPickUpRoot->Get_ChildEntity(); pPickUp; pPickUp = pPickUp->NextChildEntity() )
	{
		if( !SafeCast<CPickUpTemplate>( pPickUp ) )
			continue;
		nPickUp++;
	}
	if( !nPickUp )
		return;

	SItemDropContext context;
	context.nDrop = nPickUp;
	auto pNode = CGlobalCfg::Inst().itemDropNodeContext.FindNode( m_strItemDrop );
	pNode->Generate( context );
	context.Drop();

	int32 i = 0;
	for( auto pPickUp = m_pPickUpRoot->Get_ChildEntity(); pPickUp; pPickUp = pPickUp->NextChildEntity() )
	{
		auto p = SafeCast<CPickUpTemplate>( pPickUp );
		if( !p )
			continue;
		CEntity* pItem = SafeCast<CEntity>( context.dropItems[i].pPrefab->GetRoot()->CreateInstance() );
		p->Set( pItem, context.dropItems[i].nPrice );
		i++;
	}
}
