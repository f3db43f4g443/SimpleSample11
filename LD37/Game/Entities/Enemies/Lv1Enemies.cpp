#include "stdafx.h"
#include "Lv1Enemies.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"

void CManHead1::AIFunc()
{
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	//Launch
	m_moveTarget = GetPosition() + CVector2( 0, 200 );
	m_flyData.fStablity = 0.4f;
	m_pAI->Yield( 1.0f, false );

	//fly
	uint32 nAttack = 0;
	while( 1 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( !pPlayer )
			return;

		nAttack++;
		if( nAttack == 6 )
		{
			nAttack = 0;
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBullet );
			context.nBulletPageSize = 21;

			CBarrage* pBarrage = new CBarrage( context );
			CVector2 pos = GetPosition();
			pBarrage->AddFunc( []( CBarrage* pBarrage )
			{
				for( int i = 0; i < 3; i++ )
				{
					float fSpeedX = SRand::Inst().Rand( -50.0f, 50.0f );
					for( int j = 0; j < 7; j++ )
					{
						float fAngle1 = ( j - 3 ) * 0.12f;
						float fSpeed = 250.0f + SRand::Inst().Rand( -25.0f, 25.0f );
						pBarrage->InitBullet( i * 7 + j, 0, -1, CVector2( 0, 0 ), CVector2( fSpeed * sin( fAngle1 ) + fSpeedX, fSpeed * -cos( fAngle1 ) ), CVector2( 0, 0 ) );
					}
					pBarrage->Yield( 10 );
				}
				pBarrage->StopNewBullet();
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( pos );
			pBarrage->Start();
		}

		CVector2 playerPos = pPlayer->GetPosition();
		m_moveTarget.x += SRand::Inst().Rand( 0.0f, 1.0f ) * 64.0f - 32.0f + ( playerPos.x - m_moveTarget.x ) * 0.05f;
		m_moveTarget.x = Min( Max( m_moveTarget.x, CMyLevel::GetInst()->GetBound().x + 64.0f ), CMyLevel::GetInst()->GetBound().GetRight() - 64.0f );
		m_moveTarget.y = playerPos.y + SRand::Inst().Rand( 200.0f, 240.0f );
		m_moveTarget.y = Min( Max( m_moveTarget.y, CMyLevel::GetInst()->GetBound().y + 64.0f ), CMyLevel::GetInst()->GetBound().GetBottom() - 64.0f );
		m_pAI->Yield( 0.5f, false );
	}
}

void CManHead1::OnTickAfterHitTest()
{
	m_flyData.UpdateMove( this, m_moveTarget );
	CEnemyTemplate::OnTickAfterHitTest();
}

void CManHead2::AIFunc()
{
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_flyData.fStablity = 0.35f;

	CRectangle bound = CMyLevel::GetInst()->GetBound();
	bound.x += 128;
	bound.width -= 256;
	bound.y += 128;
	bound.height -= 256;
	uint32 n = 8;
	while( 1 )
	{
		if( n == 8 )
		{
			m_moveTarget = CVector2( x >= bound.x + bound.width * 0.5f ? SRand::Inst().Rand( bound.x, bound.x + bound.width * 0.35f ) : SRand::Inst().Rand( bound.x + bound.width * 0.65f, bound.GetRight() ),
				y >= bound.y + bound.height * 0.5f ? SRand::Inst().Rand( bound.y, bound.y + bound.height * 0.35f ) : SRand::Inst().Rand( bound.y + bound.width * 0.65f, bound.GetBottom() ) );
			n = 0;

			if( m_velocity.Length2() > 64 * 64 )
			{
				auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( GetPosition() );
				pBullet->SetVelocity( CVector2( m_velocity.y, -m_velocity.x ) * 0.5f );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) ); pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( GetPosition() );
				pBullet->SetVelocity( CVector2( -m_velocity.y, m_velocity.x ) * 0.5f );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}
		}

		n++;
		m_pAI->Yield( 0, false );
	}
}

void CManHead2::OnTickAfterHitTest()
{
	m_flyData.UpdateMove( this, m_moveTarget );
	SetRotation( atan2( -m_velocity.x, m_velocity.y ) );
	CEnemyTemplate::OnTickAfterHitTest();
}

void CManHead3::AIFunc()
{
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_flyData.fStablity = 0.45f;

	m_moveTarget = GetPosition();
	while( 1 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
			m_moveTarget = pPlayer->GetPosition();

		CVector2 dPos = m_moveTarget - GetPosition();
		if( dPos.Length2() < 32.0f * 32.0f )
		{
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBullet );
			context.nBulletPageSize = 21;

			CBarrage* pBarrage = new CBarrage( context );
			CVector2 pos = GetPosition();
			pBarrage->AddFunc( []( CBarrage* pBarrage )
			{
				float fAngle0 = SRand::Inst().Rand( -PI, PI );
				uint32 nBullet = 0;
				for( int i = 0; i < 4; i++ )
				{
					float fAngle10 = fAngle0 + ( i - 0.5f ) * PI / 20;
					float fAngle11 = fAngle0 - ( i - 0.5f ) * PI / 20;
					for( int j = 0; j < 3; j++ )
					{
						float fAngle20 = fAngle10 + j * 0.03f;
						float fAngle21 = fAngle11 - j * 0.03f;
						float fSpeed = 175 + j * 25;
						float fOfs = ( j - 1 ) * 8.0f;

						for( int k = 0; k < 10; k++ )
						{
							pBarrage->InitBullet( nBullet++, 0, -1, CVector2( fOfs * sin( fAngle20 ), fOfs * -cos( fAngle20 ) ), CVector2( fSpeed * cos( fAngle20 ), fSpeed * sin( fAngle20 ) ), CVector2( 0, 0 ) );
							pBarrage->InitBullet( nBullet++, 0, -1, CVector2( fOfs * -sin( fAngle21 ), fOfs * cos( fAngle21 ) ), CVector2( fSpeed * cos( fAngle21 ), fSpeed * sin( fAngle21 ) ), CVector2( 0, 0 ) );
						}

						pBarrage->Yield( j == 2 ? 6 : 2 );
					}
				}

				pBarrage->StopNewBullet();
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( pos );
			pBarrage->Start();
		}

		m_pAI->Yield( 0, false );
	}
}

void CManHead3::OnTickAfterHitTest()
{
	m_flyData.UpdateMove( this, m_moveTarget );
	SetRotation( atan2( -m_velocity.x, m_velocity.y ) );
	CEnemyTemplate::OnTickAfterHitTest();
}
