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
#include "Bullet.h"
#include "Explosion.h"

void CDetectTrigger::OnAddedToStage()
{
	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	auto p = GetRenderObject();
	if( p && pChunkObject )
	{
		auto p1 = pChunkObject->GetDecoratorRoot();
		if( p1 )
		{
			auto pEntity = SafeCast<CEntity>( p );
			if( pEntity )
			{
				pEntity->SetParentEntity( p1 );
			}
			else
			{
				p->RemoveThis();
				p1->AddChild( p );
			}
			p->SetPosition( GetPosition() );
			p->SetRotation( GetRotation() );
			p->SetRenderParent( this );
		}
	}
	if( m_nFrames1 && m_nFrames2 )
	{
		m_fFramesPerSec = static_cast<CMultiFrameImage2D*>( GetRenderObject() )->GetFramesPerSec();
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nFrames1, m_nFrames2, m_fFramesPerSec );
	}
	if( m_bEnabled )
	{
		if( m_bBeginCD )
		{
			if( m_nFrames1 && m_nFrames2 )
			{
				static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, m_nFrames1, m_nFrames1 / ( m_nCD * GetStage()->GetElapsedTimePerTick() ) );
				static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetPlaySpeed( 1, false );
			}
			GetStage()->RegisterAfterHitTest( m_nCD, &m_onTick );
		}
		else
			GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	}
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
		Trigger();
		return;
	}
	if( m_nFrames1 && m_nFrames2 )
	{
		if( !static_cast<CMultiFrameImage2D*>( GetRenderObject() )->IsLoop() )
		{
			static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nFrames1, m_nFrames2, m_fFramesPerSec );
			static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetPlaySpeed( 1, true );
		}
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

void CDetectTrigger::Trigger()
{
	GetStage()->RegisterAfterHitTest( m_nFireCountLeft ? m_nFireCD : m_nCD, &m_onTick );
	if( !m_nFireCountLeft && m_nFrames1 && m_nFrames2 )
	{
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, m_nFrames1, m_nFrames1 / ( m_nCD * GetStage()->GetElapsedTimePerTick() ) );
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetPlaySpeed( 1, false );
	}
}

void CKillTrigger::OnAddedToStage()
{
	auto pChunk = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunk )
	{
		pChunk->RegisterKilledEvent( &m_onKilled );
		return;
	}

	auto pChar = SafeCast<CCharacter>( GetParentEntity() );
	if( pChar )
	{
		pChar->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onKilled );
		return;
	}

	auto pPickup = SafeCast<CPickUp>( GetParentEntity() );
	if( pPickup )
	{
		pPickup->RegisterBeforePickupEvent( &m_onKilled );
		return;
	}
}

void CKillTrigger::OnRemovedFromStage()
{
	if( m_onKilled.IsRegistered() )
		m_onKilled.Unregister();
}

void CDamageTriggerEnemy::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	if( m_fChunkHpPercentBegin > m_fChunkHpPercentEnd )
	{
		auto pChunk = SafeCast<CChunkObject>( GetParentEntity() );
		if( pChunk )
		{
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			pChunk->RegisterDamagedEvent( &m_onChunkDamaged );
			m_bChunk = true;
		}
	}
}

void CDamageTriggerEnemy::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	if( m_onChunkDamaged.IsRegistered() )
		m_onChunkDamaged.Unregister();
	CEnemy::OnRemovedFromStage();
}

void CDamageTriggerEnemy::Damage( SDamageContext & context )
{
	DEFINE_TEMP_REF_THIS();
	int32 n1 = ceil( m_nHp * m_nTriggerCount * 1.0f / m_nMaxHp );
	CEnemy::Damage( context );
	int32 n2 = ceil( m_nHp * m_nTriggerCount * 1.0f / m_nMaxHp );
	n1 = Max( 0, Min( m_nTriggerCount, n1 ) );
	n2 = Max( 0, Min( m_nTriggerCount, n2 ) );

	for( int i = n1; i > n2; i-- )
	{
		for( int j = 0; j < m_nFireCount; j++ )
			Trigger();
	}
}

void CDamageTriggerEnemy::Kill()
{
	ForceUpdateTransform();
	for( int j = 0; j < m_nKillFireCount; j++ )
		Trigger();
	CEnemy::Kill();
}

void CDamageTriggerEnemy::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		auto eHitType = pEntity->GetHitType();
		if( eHitType == eEntityHitType_WorldStatic || eHitType == eEntityHitType_Platform )
		{
			CVector2 p = globalTransform.GetPosition();
			SetParentAfterEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			SetPosition( p );
			SCharacterMovementData moveData;
			moveData.ResolvePenetration( this );

			SDamageContext context;
			context.nDamage = m_nHp;
			context.nType = 0;
			context.nSourceType = 0;
			context.hitPos = context.hitDir = CVector2( 0, 0 );
			context.nHitType = -1;
			Damage( context );
			return;
		}
	}

	if( m_killRect.Contains( globalTransform.GetPosition() ) )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer && ( pPlayer->GetPosition() - globalTransform.GetPosition() ).Length2() <= m_fKillPlayerDist * m_fKillPlayerDist )
		{
			SDamageContext context;
			context.nDamage = m_nHp;
			context.nType = 0;
			context.nSourceType = 0;
			context.hitPos = context.hitDir = CVector2( 0, 0 );
			context.nHitType = -1;
			Damage( context );
			return;
		}
	}
}

void CDamageTriggerEnemy::OnChunkKilled()
{
	if( m_nHp > 0 )
	{
		SDamageContext context;
		context.nDamage = m_nHp;
		context.nType = 0;
		context.nSourceType = 0;
		context.hitPos = context.hitDir = CVector2( 0, 0 );
		context.nHitType = -1;
		Damage( context );
	}
}

void CDamageTriggerEnemy::OnChunkDamaged( SDamageContext * pContext )
{
	auto pChunk = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunk )
	{
		float fPercent = ( pChunk->GetHp() / pChunk->GetMaxHp() - m_fChunkHpPercentEnd ) / ( m_fChunkHpPercentBegin - m_fChunkHpPercentEnd );
		fPercent = Min( 1.0f, Max( 0.0f, fPercent ) );
		int32 nHp = m_nMaxHp * fPercent;
		if( nHp < m_nHp )
		{
			SDamageContext context;
			context.nDamage = m_nHp - nHp;
			context.nType = 0;
			context.nSourceType = 0;
			context.hitPos = context.hitDir = CVector2( 0, 0 );
			context.nHitType = -1;
			Damage( context );
		}
	}
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
	if( nSpawnCount && m_bCheckHit )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto hitType = static_cast<CEntity*>( pManifold->pOtherHitProxy )->GetHitType();
			if( hitType <= eEntityHitType_Platform || hitType == eEntityHitType_System )
			{
				nSpawnCount = 0;
				break;
			}
		}
	}
	if( nSpawnCount )
	{
		for( int i = 0; i < nSpawnCount; i++ )
		{
			auto pEntity = Spawn();
			CVector2 pos( m_rectSpawn.x + SRand::Inst().Rand( 0.0f, m_rectSpawn.width ), m_rectSpawn.y + SRand::Inst().Rand( 0.0f, m_rectSpawn.height ) );
			if( !m_nSpawnType )
				pos = globalTransform.MulVector2Pos( pos );
			pEntity->SetPosition( pos );
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
			if( m_nSpawnType )
				pEntity->SetParentEntity( this );
			else
			{
				auto pBullet = SafeCast<CBullet>( pEntity );
				if( pBullet )
				{
					if( pBullet->GetBulletType() == 0 )
						pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					else
						pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
				}
				else if( SafeCast<CExplosion>( pEntity ) )
					pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
				else
					pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			}
			if( m_nMaxCount )
			{
				SSpawnedEntity* pSpawnedEntity = new SSpawnedEntity( this, pEntity );
				Insert_SpawnedEntity( pSpawnedEntity );
			}

			m_onSpawn.Trigger( 0, pEntity );
		}
	}

	if( m_nMaxCount )
		m_nCurCount += nSpawnCount;
	if( m_nTotalCount >= 0 )
		m_nTotalCount -= nSpawnCount;
	if( m_nSpawnType )
	{
		bool b = true;
		if( m_nMaxCount && m_nMaxCount <= m_nCurCount )
			b = false;
		else if( m_nTotalCount == 0 )
			b = false;
		if( !b )
			return;
		CDetectTrigger::Trigger();
	}
	else
		CDetectTrigger::Trigger();
}

void CSpawner::OnSpawnedEntityDeath( SSpawnedEntity * pSpawnedEntity )
{
	if( m_nSpawnType && m_nMaxCount == m_nCurCount && m_nTotalCount != 0 )
		CDetectTrigger::Trigger();
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
	ForceUpdateTransform();
	int32 nSpawnCount = SRand::Inst().Rand( m_nMinCount, m_nMaxCount + 1 );
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

		auto pBullet = SafeCast<CBullet>( pEntity );
		if( pBullet )
		{
			if( pBullet->GetBulletType() == 0 )
				pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			else
				pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
		else if( SafeCast<CExplosion>( pEntity ) )
			pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		else
			pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
	}
}

void CDamageSpawnEnemy::Trigger()
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

	int32 nSpawnCount = SRand::Inst().Rand( m_nMinCount, m_nMaxCount + 1 );
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

		auto pBullet = SafeCast<CBullet>( pEntity );
		if( pBullet )
		{
			if( pBullet->GetBulletType() == 0 )
				pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			else
				pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
		else if( SafeCast<CExplosion>( pEntity ) )
			pEntity->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		else
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
