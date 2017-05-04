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

#define ENTER_SIGHT_HEIGHT 600

void CEnemyTemplate::OnAddedToStage()
{
	if( !CMyLevel::GetInst() )
		return;
	CEnemy::OnAddedToStage();
	m_pAI = new AI();
	m_pAI->SetParentEntity( this );
}

void CEnemy1::OnAddedToStage()
{
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab.c_str() );
	CEnemyTemplate::OnAddedToStage();
}

void CEnemy1::AIFunc()
{
	m_pAI->Yield( 0, true );

	for( ;; )
	{
		while( true )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer && ( pPlayer->globalTransform.GetPosition() - globalTransform.GetPosition() ).Length2() < m_fSight * m_fSight )
				break;

			m_pAI->Yield( 0.5f, true );
		}

		for( int i = 0; i < m_nAmmoCount; i++ )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				return;
			CVector2 p = pPlayer->GetPosition() - globalTransform.GetPosition();
			for( int i = 0; i < m_nBulletCount; i++ )
			{
				auto pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() );
				float r = atan2( p.y, p.x ) + ( i - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
				pBullet->SetRotation( atan2( p.y, p.x ) );
				pBullet->SetVelocity( CVector2( cos( r ), sin( r ) ) * m_fBulletSpeed );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}

			CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );
			m_pAI->Yield( m_fFireRate, true );
		}

		m_pAI->Yield( m_fFireInterval, true );
	}
}

void CEnemy2::OnAddedToStage()
{
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab.c_str() );
	CEnemyTemplate::OnAddedToStage();
}

void CEnemy2::AIFunc()
{
	m_pAI->Yield( 0, true );

	SBarrageContext context;
	context.vecBulletTypes.push_back( m_pBulletPrefab );
	context.nBulletPageSize = 12;

	for( ;; )
	{
		while( true )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer && ( pPlayer->globalTransform.GetPosition() - globalTransform.GetPosition() ).Length2() < m_fSight * m_fSight )
				break;

			m_pAI->Yield( 0.5f, true );
		}

		CBarrageAutoStopHolder pBarrage = new CBarrage( context );
		pBarrage->AddFunc( []( CBarrage* pBarrage )
		{
			float fAngle0 = SRand::Inst().Rand( -PI, PI );
			float fAngle1 = SRand::Inst().Rand( 0.5f, 1.5f );
			CMatrix2D matA;
			matA.Rotate( fAngle0 );
			CMatrix2D matB;
			matB.Rotate( fAngle0 + fAngle1 );
			for( int i = 0; i < 12; i++ )
			{
				float fBaseAngle = i * PI / 6;
				CVector2 vel0 = CVector2( cos( fBaseAngle ) * 350, sin( fBaseAngle ) * 80 );
				CVector2 vel = matA.MulVector2Dir( vel0 );
				pBarrage->InitBullet( i * 2, 0, -1, CVector2( 0, 0 ), vel, vel * -0.5f, true );
				vel = matB.MulVector2Dir( vel0 );
				pBarrage->InitBullet( i * 2 + 1, 0, -1, CVector2( 0, 0 ), vel, vel * -0.5f, true );
			}

			CMyLevel::GetInst()->AddShakeStrength( 10 );
			pBarrage->Yield( 120 );

			CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
			if( !pPlayer )
				return;
			for( int i = 0; i < 24; i++ )
			{
				pBarrage->GetBulletContext( i )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
				pBarrage->GetBulletContext( i )->SetBulletMove( CVector2( 0, 0 ), ( pPlayer->GetPosition() - pBarrage->GetPosition() - pBarrage->GetBulletContext( i )->p0 ) * 2 );
			}
			CMyLevel::GetInst()->AddShakeStrength( 10 );
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->SetPosition( globalTransform.GetPosition() );
		pBarrage->Start();

		m_pAI->Yield( 10, true );
	}
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
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
		pPlayer->DelayChangeStage( "scene0.pf", "start" );
	CEnemyTemplate::OnRemovedFromStage();
}

void CBoss::AIFunc()
{
	m_pAI->Yield( 0, true );
	SafeCast<CChunkObject>( GetParentEntity() )->GetChunk()->fWeight = 1000;

	while( globalTransform.GetPosition().y > ENTER_SIGHT_HEIGHT )
		m_pAI->Yield( 1.0f, true );

	for( ;; )
	{
		{
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