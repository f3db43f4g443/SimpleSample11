#include "stdafx.h"
#include "Lv1Enemies.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"

void CManHead1::AIFunc()
{
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	//Launch
	m_moveTarget = GetPosition() + CVector2( 0, 200 );
	m_flyData.fStablity = 0.0f;
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
			context.nBulletPageSize = 27;

			CBarrage* pBarrage = new CBarrage( context );
			CVector2 pos = GetPosition();
			pBarrage->AddFunc( []( CBarrage* pBarrage )
			{
				for( int i = 0; i < 3; i++ )
				{
					float fSpeedX = SRand::Inst().Rand( -50.0f, 50.0f );
					for( int j = 0; j < 9; j++ )
					{
						float fAngle1 = ( j - 4 ) * 0.1f;
						float fSpeed = 250.0f + SRand::Inst().Rand( -10.0f, 10.0f );
						pBarrage->InitBullet( i * 9 + j, 0, -1, CVector2( 0, 0 ), CVector2( fSpeed * sin( fAngle1 ) + fSpeedX, fSpeed * -cos( fAngle1 ) ), CVector2( 0, 0 ) );
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
		m_moveTarget.x += SRand::Inst().Rand( -32.0f, 32.0f );
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
