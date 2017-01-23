#include "stdafx.h"
#include "BlockItem.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "MyLevel.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"

void CBlockItemTrigger::OnAddedToStage()
{
	CBlockItem::OnAddedToStage();
	GetStage()->RegisterStageEvent( eStageEvent_PostHitTest, &m_onTick );
}

void CBlockItemTrigger::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CBlockItem::OnRemovedFromStage();
}

void CBlockItemTrigger::OnTick()
{
	if( m_nTriggerCDLeft )
		m_nTriggerCDLeft--;
	for( auto pManifold = m_pTriggerArea->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

		auto pPlayer = SafeCast<CPlayer>( pEntity );
		if( pPlayer )
			CheckHit( pPlayer );

		auto pEnemy = SafeCast<CEnemy>( pEntity );
		if( pEnemy )
			CheckHit( pEnemy );
	}
	if( m_eTriggerType != eBlockItemTriggerType_Step )
	{
		m_vecHitCharacters.clear();
		for( auto pManifold = m_pTriggerArea->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

			auto pPlayer = SafeCast<CPlayer>( pEntity );
			if( pPlayer )
				m_vecHitCharacters.push_back( pPlayer );

			auto pEnemy = SafeCast<CEnemy>( pEntity );
			if( pEnemy )
				m_vecHitCharacters.push_back( pEnemy );
		}
	}
}

void CBlockItemTrigger::CheckHit( CCharacter* pCharacter )
{
	switch( m_eTriggerType )
	{
	case eBlockItemTriggerType_Step:
		Trigger( pCharacter );
		break;
	case eBlockItemTriggerType_HitDeltaPos:
	case eBlockItemTriggerType_HitVelocity:
	{
		for( auto& item : m_vecHitCharacters )
		{
			if( item == pCharacter )
				return;
		}
		Trigger( pCharacter );
		break;
	}
	default:
		break;
	}
}

bool CBlockItemTrigger::Trigger( CCharacter* pCharacter )
{
	if( m_nTriggerCDLeft )
		return false;
	if( !pCharacter->CanTriggerItem() )
		return false;

	CVector2 dir( 1, 0 );
	switch( m_eTriggerType )
	{
	case eBlockItemTriggerType_Step:
		break;
	case eBlockItemTriggerType_HitDeltaPos:
	{
		dir = globalTransform.GetPosition() - pCharacter->globalTransform.GetPosition();
	}
		break;
	case eBlockItemTriggerType_HitVelocity:
	{
		dir = pCharacter->GetVelocity();
	}
		break;
	default:
		return false;
	}

	m_nTriggerCDLeft = m_nTriggerCD;
	OnTrigged( pCharacter, dir );
	return true;
}

void CBlockItemTrigger1::OnAddedToStage()
{
	CBlockItemTrigger::OnAddedToStage();
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBulletPrefab1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );
}

void CBlockItemTrigger1::OnTrigged( CCharacter* pCharacter, const CVector2& dir )
{
	uint32 nAmmoCount = m_nAmmoCount;
	uint32 nFireRate = m_nFireRate;
	uint32 nBulletCount = m_nBulletCount;
	float fBulletSpeed = m_fBulletSpeed;
	float fBulletAngle = m_fBulletAngle;
	CVector2 p = dir;

	SBarrageContext context;

	auto pEnemy = SafeCast<CEnemy>( pCharacter );
	if( pEnemy )
		context.vecBulletTypes.push_back( m_pBulletPrefab );
	else
		context.vecBulletTypes.push_back( m_pBulletPrefab1 );
	context.nBulletPageSize = m_nBulletCount * m_nAmmoCount;
	context.pCreator = SafeCast<CChunkObject>( GetParentEntity() );

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [=]( CBarrage* pBarrage )
	{
		uint32 nBullet = 0;
		for( int i = 0; i < nAmmoCount; i++ )
		{
			for( int i = 0; i < nBulletCount; i++ )
			{
				float r = atan2( p.y, p.x ) + ( i - ( nBulletCount - 1 ) * 0.5f ) * fBulletAngle;
				pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), CVector2( cos( r ), sin( r ) ) * fBulletSpeed, CVector2( 0, 0 ) );
			}

			pBarrage->Yield( nFireRate );
		}
		pBarrage->Yield( 5 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( pEnemy ? CMyLevel::eBulletLevel_Enemy : CMyLevel::eBulletLevel_Player ) );
	pBarrage->SetPosition( globalTransform.GetPosition() );
	pBarrage->Start();
}

void CBlockItemTrigger2::OnAddedToStage()
{
	CBlockItemTrigger::OnAddedToStage();
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBulletPrefab1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );
}

void CBlockItemTrigger2::OnTrigged( CCharacter * pCharacter, const CVector2 & dir )
{
	auto pEnemy = SafeCast<CEnemy>( pCharacter );
	if( !pEnemy )
		return;

	SBarrageContext context;
	context.nBulletPageSize = 100;
	context.vecBulletTypes.push_back( m_pBulletPrefab );
	context.vecBulletTypes.push_back( m_pBulletPrefab1 );
	context.pCreator = SafeCast<CChunkObject>( GetParentEntity() );

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [=] ( CBarrage* pBarrage )
	{
		uint32 nBullet = 0;

		CPlayer* pPlayer = GetStage()->GetPlayer();
		CVector2 dPos = globalTransform.GetPosition() - pPlayer->globalTransform.GetPosition();
		float fAngle0 = atan2( dPos.y, dPos.x );
		pBarrage->InitBullet( nBullet++, -1, -1, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false, fAngle0, 0, 0 );
		for( int i = 0; i < 5; i++ )
		{
			float fAngle = 0.3f * ( i - 2 );
			pBarrage->InitBullet( nBullet++, 1, 0, CVector2( 0, 0 ), CVector2( cos( fAngle ), sin( fAngle ) ) * 400, CVector2( cos( fAngle ), sin( fAngle ) ) * -400, false );
		}
		pBarrage->Yield( 60 );
		for( int i = 0; i < 5; i++ )
		{
			pBarrage->GetBulletContext( i + 1 )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
		}
		pBarrage->Yield( 60 );

		for( int i = 0; i < 5; i++ )
		{
			uint32 nBullet0 = nBullet;

		}

		for( int i = 0; i < nAmmoCount; i++ )
		{
			for( int i = 0; i < nBulletCount; i++ )
			{
				float r = atan2( p.y, p.x )+( i-( nBulletCount-1 ) * 0.5f ) * fBulletAngle;
				pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), CVector2( cos( r ), sin( r ) ) * fBulletSpeed, CVector2( 0, 0 ) );
			}

			pBarrage->Yield( nFireRate );
		}
		pBarrage->Yield( 5 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( pEnemy ? CMyLevel::eBulletLevel_Enemy : CMyLevel::eBulletLevel_Player ) );
	pBarrage->SetPosition( globalTransform.GetPosition() );
	pBarrage->Start();
}
