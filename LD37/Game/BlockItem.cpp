#include "stdafx.h"
#include "BlockItem.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "MyLevel.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"

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

		CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
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

		uint32 nParentBullet[5] = { 1, 2, 3, 4, 5 };
		SRand::Inst().Shuffle( nParentBullet, ELEM_COUNT( nParentBullet ) );
		for( int i = 0; i < 5; i++ )
		{
			uint32 nBullet0 = nBullet;
			uint32 nParent = nParentBullet[i];

			pPlayer = pBarrage->GetStage()->GetPlayer();
			if( !pPlayer )
				goto end;
			CVector2 dPos = pBarrage->GetTransform( nParent ).MulTVector2PosNoScale( pPlayer->GetPosition() );
			CVector2 dir = dPos;
			dir.Normalize();
			CVector2 norm( dir.y, -dir.x );


			struct
			{
				CVector2 keyPoints[5][8];
				uint32 nKeyPointsIndex[5][8];
			} temp;
			uint32 nKeyPoints[5] = { 1, 2, 3, 5, 8 };
			temp.keyPoints[0][0] = CVector2( 0, 0 );
			for( int i = 1; i < 5; i++ )
			{
				for( int j = 0; j < nKeyPoints[i]; j++ )
				{
					temp.keyPoints[i][j] = dir * i * 64 + norm * ( j - ( nKeyPoints[i] - 1 ) * 0.5f ) * 32;
				}
			}

			uint32 nSplitIndex[4][5];
			for( int i = 0; i < 4; i++ )
			{
				for( uint32 j = 0; j < nKeyPoints[i]; j++ )
				{
					nSplitIndex[i][j] = j;
				}
				for( uint32 j = 0; j < nKeyPoints[i + 1] - nKeyPoints[i]; j++ )
				{
					uint32 a = SRand::Inst().Rand( j, nKeyPoints[i] );
					uint32 temp = nSplitIndex[i][j];
					nSplitIndex[i][j] = nSplitIndex[i][a];
					nSplitIndex[i][a] = temp;
				}
			}
			for( int j = 0; j < 8; j++ )
			{
				temp.nKeyPointsIndex[4][j] = j;
			}
			for( int i = 4; i > 0; i-- )
			{
				int j1 = 0;
				uint32 nIndexMap[8];
				for( int j = 0; j < nKeyPoints[i - 1]; j++ )
				{
					nIndexMap[j1++] = j;
					for( int k = 0; k < nKeyPoints[i] - nKeyPoints[i - 1]; k++ )
					{
						if( j == nSplitIndex[i - 1][k] )
						{
							nIndexMap[j1++] = j;
							break;
						}
					}
				}
				for( int j = 0; j < 8; j++ )
				{
					temp.nKeyPointsIndex[i - 1][j] = nIndexMap[temp.nKeyPointsIndex[i][j] ];
				}
			}

			const uint32 nWave = 16;
			for( int i = 0; i < nWave; i++ )
			{
				uint32 nBaseBullet = nBullet0 + i * 8;
				pBarrage->InitBullet( nBaseBullet, 0, nParent, CVector2( 0, 0 ), dir * 128, CVector2( 0, 0 ), true );

				for( int i = 1; i <= 4; i++ )
				{
					pBarrage->AddDelayAction( 15 * i - 3, [=]()
					{
						for( int iBullet = 0; iBullet < 8; iBullet++ )
						{
							uint32 nB = temp.nKeyPointsIndex[i - 1][iBullet];
							uint32 nB1 = temp.nKeyPointsIndex[i][iBullet];
							if( iBullet > 0 )
							{
								if( nB1 - temp.nKeyPointsIndex[i][iBullet - 1] > nB - temp.nKeyPointsIndex[i - 1][iBullet - 1] )
								{
									auto pContext0 = pBarrage->GetBulletContext( nBaseBullet + iBullet - 1 );
									pBarrage->InitBullet( nBaseBullet + iBullet, pContext0->nNewBulletType, nParent, pContext0->p0,
										CVector2( 0, 0 ), CVector2( 0, 0 ), true );
								}
							}
							else
							{
								SBulletContext* pContext = pBarrage->GetBulletContext( nBaseBullet + iBullet );
								if( !pContext->pEntity )
									pContext->nNewBulletType = -1;
							}
							SBulletContext* pContext = pBarrage->GetBulletContext( nBaseBullet + iBullet );
							if( pContext->IsValid() )
								pContext->MoveTowards( temp.keyPoints[i][iBullet], 3 );
						}
					} );

					pBarrage->AddDelayAction( 15 * i, [=] ()
					{
						for( int iBullet = 0; iBullet < 8; iBullet++ )
						{
							SBulletContext* pContext = pBarrage->GetBulletContext( nBaseBullet + iBullet );
							if( pContext->IsValid() )
								pContext->SetBulletMove(  dir * 256, CVector2( 0, 0 ) );
						}
					} );

				}
				pBarrage->Yield( 8 );

			}
			nBullet += 16 * 8;
		}

		end:
		pBarrage->Yield( 8 );
		for( int i = 1; i <= 5; i++ )
			pBarrage->GetBulletContext( i )->nNewBulletType = -1;
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( pEnemy ? CMyLevel::eBulletLevel_Enemy : CMyLevel::eBulletLevel_Player ) );
	pBarrage->SetPosition( globalTransform.GetPosition() );
	pBarrage->Start();
}
