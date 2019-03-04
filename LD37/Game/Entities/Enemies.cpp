#include "stdafx.h"
#include "Enemies.h"
#include "Stage.h"
#include "Player.h"
#include "Bullet.h"
#include "MyLevel.h"
#include "Barrage.h"
#include "Common/ResourceManager.h"
#include "Rand.h"
#include "Block.h"

void CEnemyTemplate::OnAddedToStage()
{
	if( !CMyLevel::GetInst() )
		return;
	CEnemy::OnAddedToStage();
	m_pAI = new AI();
	m_pAI->SetParentEntity( this );
}

#define MOVE_LIM0 64
#define MOVE_LIM1 192

void CBoss::OnAddedToStage()
{
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab.c_str() );
	m_pBulletPrefab1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab1.c_str() );
	CEnemyTemplate::OnAddedToStage();
}

void CBoss::OnRemovedFromStage()
{
	CEnemyTemplate::OnRemovedFromStage();
}

void CBoss::AIFunc()
{
	m_pAI->Yield( 0, true );
	SafeCast<CChunkObject>( GetParentEntity() )->GetChunk()->fWeight = 1000;

	while( globalTransform.GetPosition().y > 600 )
		m_pAI->Yield( 1.0f, true );

	for( ;; )
	{
		{
			auto pPlayer = GetStage()->GetPlayer();
			if( !pPlayer || !pPlayer->GetHp() )
				return;
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBulletPrefab );
			context.vecBulletTypes.push_back( m_pBulletPrefab1 );
			context.nBulletPageSize = 81;

			CBarrageAutoStopHolder pBarrage = new CBarrage( context );
			pBarrage->AddFunc( []( CBarrage* pBarrage )
			{
				uint32 nBullet = 0;
				for( int i = 0; i < 9; i++ )
				{
					auto pPlayer = pBarrage->GetStage()->GetPlayer();
					if( !pPlayer )
						return;
					CVector2 dPos = pPlayer->GetPosition() - ( pBarrage->GetPosition() + CVector2( 0, -64 ) );
					float fAngle = atan2( dPos.y, dPos.x ) + SRand::Inst().Rand( -0.3f, 0.3f );
					for( int i1 = 0; i1 < 3; i1++ )
					{
						float fAngle1 = ( i1 - 1 ) * 0.5f + fAngle;
						CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
						for( int i2 = 0; i2 < 3; i2++ )
						{
							pBarrage->InitBullet( nBullet++, 1, -1, CVector2( 0, -64 ), dir * ( 200 + i2 * 100 ), CVector2( 0, 0 ), true );
						}
					}
					pBarrage->Yield( 15 );
				}
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( globalTransform.GetPosition() );
			pBarrage->Start();

			m_pAI->Yield( 5, true );
		}

		{
			auto pPlayer = GetStage()->GetPlayer();
			if( !pPlayer || !pPlayer->GetHp() )
				return;
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBulletPrefab );
			context.vecBulletTypes.push_back( m_pBulletPrefab1 );
			context.nBulletPageSize = 100;

			CBarrageAutoStopHolder pBarrage = new CBarrage( context );
			pBarrage->AddFunc( []( CBarrage* pBarrage )
			{
				uint32 nBullet = 0;
				for( int i = 0; i < 7; i++ )
				{
					auto pPlayer = pBarrage->GetStage()->GetPlayer();
					if( !pPlayer )
						return;
					CVector2 ofs[] = { { -32, 32 }, { 32, 32 } };
					float fAngle[2];
					for( int i = 0; i < 2; i++ )
					{
						CVector2 dPos = pPlayer->GetPosition() - ( pBarrage->GetPosition() + ofs[i] );
						float fAngle = atan2( dPos.y, dPos.x ) + SRand::Inst().Rand( -0.3f, 0.3f );
						CVector2 dir( cos( fAngle ), sin( fAngle ) );
						uint32 nParBullet = nBullet;
						pBarrage->InitBullet( nBullet++, 0, -1, ofs[i], dir * 300, CVector2( 0, 0 ), false, 0, i? -0.5f: 0.5f, 0 );
						for( int i1 = 0; i1 < 6; i1++ )
						{
							float fAngle1 = i1 * PI / 3;
							pBarrage->InitBullet( nBullet++, 0, nParBullet, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 40, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
						}
					}

					pBarrage->Yield( 30 );
				}
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( globalTransform.GetPosition() );
			pBarrage->Start();

			m_pAI->Yield( 5, true );
		}

		{
			auto pPlayer = GetStage()->GetPlayer();
			if( !pPlayer || !pPlayer->GetHp() )
				return;
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBulletPrefab );
			context.vecBulletTypes.push_back( m_pBulletPrefab1 );
			context.nBulletPageSize = 180;

			CBarrageAutoStopHolder pBarrage = new CBarrage( context );
			pBarrage->AddFunc( []( CBarrage* pBarrage )
			{
				uint32 nBullet = 0;
				float fAngle = SRand::Inst().Rand( -PI, PI );
				for( int i = 0; i < 45; i++ )
				{
					for( int i1 = 0; i1 < 4; i1++ )
					{
						float fAngle1 = fAngle + i1 * PI / 2;
						CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
						pBarrage->InitBullet( nBullet++, 1, -1, CVector2( 0, 0 ), dir * ( 180 + i1 * 5 ), CVector2( 0, 0 ), true );
					}
					pBarrage->Yield( 2 );
					fAngle += PI / 15;
				}
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( globalTransform.GetPosition() );
			pBarrage->Start();

			m_pAI->Yield( 5, true );
		}
	}
}

void CBoss::OnTickBeforeHitTest()
{
	CEnemyTemplate::OnTickBeforeHitTest();
}