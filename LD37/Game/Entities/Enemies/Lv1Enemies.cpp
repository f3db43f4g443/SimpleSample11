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

			Damage( 10 );
		}

		CVector2 playerPos = pPlayer->GetPosition();
		m_moveTarget.x += SRand::Inst().Rand( 0.0f, 1.0f ) * 64.0f - 32.0f + ( playerPos.x - m_moveTarget.x ) * 0.05f;
		m_moveTarget.x = Min( Max( m_moveTarget.x, CMyLevel::GetInst()->GetBoundWithLvBarrier().x + 64.0f ), CMyLevel::GetInst()->GetBoundWithLvBarrier().GetRight() - 64.0f );
		m_moveTarget.y = playerPos.y + SRand::Inst().Rand( 200.0f, 240.0f );
		m_moveTarget.y = Min( Max( m_moveTarget.y, CMyLevel::GetInst()->GetBoundWithLvBarrier().y + 64.0f ), CMyLevel::GetInst()->GetBoundWithLvBarrier().GetBottom() - 64.0f );
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

	CRectangle bound = CMyLevel::GetInst()->GetBoundWithLvBarrier();
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

void CManHead3::Kill()
{
	SBarrageContext context;
	context.vecBulletTypes.push_back( m_pBullet );
	context.nBulletPageSize = 75;

	CBarrage* pBarrage = new CBarrage( context );
	CVector2 pos = GetPosition();
	float fAngle0 = GetRotation() + PI * 0.5f;
	pBarrage->AddFunc( [fAngle0] ( CBarrage* pBarrage )
	{
		uint32 nBullet = 0;
		for( int i = 0; i < 5; i++ )
		{
			float fAngle = fAngle0 + ( i - 2 ) * 0.45f;
			pBarrage->InitBullet( nBullet++, 0, -1, CVector2( -sin( fAngle ), cos( fAngle ) ) * 8, CVector2( 120 * cos( fAngle ), 120 * sin( fAngle ) ), CVector2( 0, 0 ) );
		}

		for( int i = 0; i < 5; i++ )
		{
			pBarrage->Yield( 10 );

			for( int j = 0; j < 5; j++ )
			{
				if( !pBarrage->IsBulletAlive( j ) )
					continue;

				float fAngle = fAngle0 + ( j - 2 ) * 0.45f;
				CVector2 pos( ( 20 * ( i + 1 ) ) * cos( fAngle ) - 8 * sin( fAngle ), ( 20 * ( i + 1 ) ) * sin( fAngle ) + 8 * cos( fAngle ) );
				for( int k = 0; k < 3; k++ )
				{
					float fAngle1 = fAngle0 + ( j - 2 ) * 0.08f + ( k - 1 ) * ( 0.12f + i * 0.09f - ( j - 2 ) * ( j - 2 ) * 0.021f );
					pBarrage->InitBullet( nBullet++, 0, -1, pos, CVector2( 250 * cos( fAngle1 ), 250 * sin( fAngle1 ) ), CVector2( 0, 0 ) );
				}
			}
		}

		for( int i = 0; i < 5; i++ )
		{
			pBarrage->DestroyBullet( i );
		}
		pBarrage->Yield( 1 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( pos );
	pBarrage->Start();

	CEnemyTemplate::Kill();
}

void CManHead3::AIFunc()
{
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_flyData.bHitChannel[eEntityHitType_WorldStatic] = m_flyData.bHitChannel[eEntityHitType_Platform] = true;
	SetVelocity( CVector2( 0, 50 ) );
	m_flyData.fStablity = 1.0f;
	m_flyData.fMaxAcc = 0.0f;

	m_moveTarget = GetPosition();
	while( 1 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
			m_moveTarget = pPlayer->GetPosition();

		CVector2 dPos = m_moveTarget - GetPosition();
		if( dPos.Length2() < 64.0f * 64.0f )
		{
			Kill();
			return;
		}
		m_flyData.fMaxAcc = Min( 400.0f, m_flyData.fMaxAcc + GetStage()->GetElapsedTimePerTick() * 200.0f );
		m_flyData.fStablity = Max( 0.0f, m_flyData.fStablity - GetStage()->GetElapsedTimePerTick() * 0.5f );

		m_pAI->Yield( 0, false );
	}
}

void CManHead3::OnTickAfterHitTest()
{
	CReference<CManHead3> pTemp = this;
	m_flyData.UpdateMove( this, m_moveTarget );
	if( !GetStage() )
		return;

	if( m_flyData.bHit )
	{
		Kill();
		return;
	}
	SetRotation( atan2( -m_velocity.x, m_velocity.y ) );
	CEnemyTemplate::OnTickAfterHitTest();
}

void CManHead4::Kill()
{
	SBarrageContext context;
	context.vecBulletTypes.push_back( m_pBullet );
	context.nBulletPageSize = 144;

	CBarrage* pBarrage = new CBarrage( context );
	CVector2 pos = GetPosition();
	pBarrage->AddFunc( [] ( CBarrage* pBarrage )
	{
		float fAngle0 = SRand::Inst().Rand( -PI, PI );
		uint32 nBullet = 0;
		for( int i = 0; i < 4; i++ )
		{
			float fAngle10 = fAngle0 + ( i - 0.5f ) * PI / 12;
			float fAngle11 = fAngle0 - ( i - 0.5f ) * PI / 12;
			for( int j = 0; j < 3; j++ )
			{
				float fAngle20 = fAngle10 - j * 0.04f;
				float fAngle21 = fAngle11 + j * 0.04f;
				float fSpeed = 190 + j * 10;
				float fOfs = ( 1 - j ) * 32.0f;

				for( int k = 0; k < 6; k++ )
				{
					float fAngle30 = fAngle20 + k * PI * 2 / 6;
					float fAngle31 = fAngle21 + k * PI * 2 / 6;
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( fOfs * -sin( fAngle30 ), fOfs * cos( fAngle30 ) ), CVector2( fSpeed * cos( fAngle30 ), fSpeed * sin( fAngle30 ) ), CVector2( 0, 0 ) );
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( fOfs * sin( fAngle31 ), fOfs * -cos( fAngle31 ) ), CVector2( fSpeed * cos( fAngle31 ), fSpeed * sin( fAngle31 ) ), CVector2( 0, 0 ) );
				}

				pBarrage->Yield( j == 2 ? 10 : 5 );
			}
		}

		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( pos );
	pBarrage->Start();

	CEnemyTemplate::Kill();
}

void CManHead4::AIFunc()
{
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	SetVelocity( CVector2( 0, 0 ) );
	float g = 300.0f;

	while( 1 )
	{
		float dTime = GetStage()->GetElapsedTimePerTick();
		CVector2 v0 = GetVelocity();
		CVector2 v1 = v0 + CVector2( 0, -g ) * dTime;
		CVector2 dPos = ( v0 + v1 ) * dTime * 0.5f;
		SetPosition( GetPosition() + dPos );
		SetVelocity( v1 );

		m_pAI->Yield( 0, false );
	}
}

void CManHead4::OnTickAfterHitTest()
{
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic
			|| pEntity->GetHitType() == eEntityHitType_Platform
			|| pEntity->GetHitType() == eEntityHitType_Player )
		{
			Kill();
			return;
		}
	}

	CEnemyTemplate::OnTickAfterHitTest();
}

bool CRoach::Knockback( const CVector2& vec )
{
	if( !m_creepData.bHitWall )
		return false;
	m_creepData.Knockback( 0.25f, vec * 750 );
	m_bKnockedback = true;
	return true;
}

bool CRoach::IsKnockback()
{
	return m_creepData.fKnockbackTime > 0;
}

void CRoach::AIFunc()
{
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_nDir = SRand::Inst().Rand( 0, 2 ) * 2 - 1;
	m_bKnockedback = false;

	uint32 nStep = SRand::Inst().Rand( 0u, m_nFireRate );

	while( 1 )
	{
		do
		{
			m_pAI->Yield( SRand::Inst().Rand( m_fAIStepTimeMin, m_fAIStepTimeMax ), false );
		} while( !m_creepData.bHitWall );

		m_nDir = SRand::Inst().Rand( 0, 2 ) * 2 - 1;

		if( m_bKnockedback )
			m_bKnockedback = false;
		else
		{
			nStep++;
			if( nStep == m_nFireRate )
			{
				nStep = 0;

				CVector2 dir( cos( GetRotation() ), sin( GetRotation() ) );
				CVector2 ofs[3] = { CVector2( 0, -8 ), CVector2( 8, 0 ), CVector2( 0, 8 ) };
				for( int i = 0; i < 3; i++ )
				{
					auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
					pBullet->SetPosition( globalTransform.GetPosition() + dir * ofs[i].x + CVector2( dir.y, -dir.x ) * ofs[i].y );
					pBullet->SetRotation( atan2( dir.y, dir.x ) );
					pBullet->SetVelocity( CVector2( dir ) * 150 );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}
			}
		}
	}
}

void CRoach::OnTickAfterHitTest()
{
	CReference<CRoach> pTemp = this;
	m_creepData.UpdateMove( this, m_nDir );
	if( !GetStage() )
		return;
	CEnemyTemplate::OnTickAfterHitTest();
}

void CMaggot::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_nDir = SRand::Inst().Rand( -1, 2 );
	m_nAIStepTimeLeft = SRand::Inst().Rand( m_fAIStepTimeMin, m_fAIStepTimeMax ) / GetStage()->GetElapsedTimePerTick();
}

bool CMaggot::Knockback( const CVector2 & vec )
{
	if( !m_moveData.bHitSurface )
		return false;
	CVector2 tangent( m_moveData.normal.y, -m_moveData.normal.x );
	float fTangent = tangent.Dot( vec );
	CVector2 vecKnockback = tangent * fTangent + m_moveData.normal;
	m_moveData.Fall( this, vecKnockback * m_moveData.fFallInitSpeed * 5 );

	m_nKnockBackTimeLeft = m_nKnockbackTime;

	return true;
}

bool CMaggot::IsKnockback()
{
	return m_nKnockBackTimeLeft > 0;
}

void CMaggot::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.UpdateMove( this, m_nDir );
	if( !GetStage() )
		return;

	if( !m_nAIStepTimeLeft )
	{
		if( m_moveData.bHitSurface )
		{
			float fDir = 0;
			for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				if( SafeCast<CMaggot>( pEntity ) )
				{
					fDir += CVector2( -m_moveData.normal.y, m_moveData.normal.x ).Dot( pManifold->normal );
				}
			}

			if( fDir > 0 )
				m_nDir = 1;
			else if( fDir < 0 )
				m_nDir = -1;
			else
				m_nDir = SRand::Inst().Rand( -1, 2 );

			if( m_moveData.normal.y < -0.8f && SRand::Inst().Rand( 0.0f, 1.0f ) < m_fFallChance )
			{
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( pPlayer )
				{
					CVector2 dPos = pPlayer->GetPosition() - globalTransform.GetPosition();
					if( dPos.y < 0 )
					{
						float t = sqrt( -2 * dPos.y / m_moveData.fGravity );
						float vx = dPos.x / t;
						if( abs( vx ) < m_moveData.fFallInitSpeed * 2 )
						{
							m_moveData.Fall( this, CVector2( vx, 0 ) );
						}
					}
				}

			}
		}

		m_nAIStepTimeLeft = SRand::Inst().Rand( m_fAIStepTimeMin, m_fAIStepTimeMax ) / GetStage()->GetElapsedTimePerTick();
	}

	uint8 newAnimState = 0;
	if( m_moveData.bHitSurface )
	{
		GetRenderObject()->SetRotation( atan2( -m_moveData.normal.x, m_moveData.normal.y ) );
		if( m_nDir == 1 )
			newAnimState = 1;
		else if( m_nDir == -1 )
			newAnimState = 2;
		else
			newAnimState = 0;
	}
	else
	{
		GetRenderObject()->SetRotation( 0 );
		if( GetVelocity().x > 0 )
			newAnimState = 3;
		else
			newAnimState = 4;
	}

	if( newAnimState != m_nAnimState )
	{
		auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		switch( newAnimState )
		{
		case 0:
			pImage->SetFrames( 0, 1, 0 );
			break;
		case 1:
			pImage->SetFrames( 0, 4, 6 );
			break;
		case 2:
			pImage->SetFrames( 4, 8, 6 );
			break;
		case 3:
			pImage->SetFrames( 8, 12, 6 );
			break;
		case 4:
			pImage->SetFrames( 12, 16, 6 );
			break;
		default:
			break;
		}
		m_nAnimState = newAnimState;
	}

	if( m_nAIStepTimeLeft )
		m_nAIStepTimeLeft--;
	if( m_nKnockBackTimeLeft )
		m_nKnockBackTimeLeft--;
	CEnemy::OnTickAfterHitTest();
}
