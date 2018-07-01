#include "stdafx.h"
#include "Lv1Enemies.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Entities/Barrage.h"
#include "Entities/Bullets.h"
#include "Common/ResourceManager.h"

void CManHead1::AIFunc()
{
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
			context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
			context.nBulletPageSize = 15;

			CBarrage* pBarrage = new CBarrage( context );
			CVector2 pos = GetPosition();
			pBarrage->AddFunc( []( CBarrage* pBarrage )
			{
				for( int i = 0; i < 3; i++ )
				{
					float fSpeedX = SRand::Inst().Rand( -50.0f, 50.0f );
					for( int j = 0; j < 5; j++ )
					{
						float fAngle1 = ( j - 2 ) * 0.2f;
						float fSpeed = 150.0f + SRand::Inst().Rand( 0.0f, 50.0f );
						pBarrage->InitBullet( i * 5 + j, 0, -1, CVector2( 0, 0 ), CVector2( fSpeed * sin( fAngle1 ) + fSpeedX, fSpeed * -cos( fAngle1 ) ), CVector2( 0, 0 ) );
					}
					pBarrage->Yield( 10 );
				}
				pBarrage->StopNewBullet();
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( pos );
			pBarrage->Start();

			SDamageContext dmgContext;
			dmgContext.nDamage = 10;
			dmgContext.nType = 0;
			dmgContext.nSourceType = 0;
			dmgContext.hitPos = dmgContext.hitDir = CVector2( 0, 0 );
			dmgContext.nHitType = -1;
			Damage( dmgContext );
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
	m_flyData.fStablity = 0.35f;

	uint32 n = 10;
	while( 1 )
	{
		if( n == 10 )
		{
			CRectangle bound = CMyLevel::GetInst()->GetBoundWithLvBarrier();
			bound.x += 128;
			bound.width -= 256;
			bound.y += 128;
			bound.height -= 256;
			m_moveTarget = CVector2( x >= bound.x + bound.width * 0.5f ? SRand::Inst().Rand( bound.x, bound.x + bound.width * 0.35f ) : SRand::Inst().Rand( bound.x + bound.width * 0.65f, bound.GetRight() ),
				y >= bound.y + bound.height * 0.5f ? SRand::Inst().Rand( bound.y, bound.y + bound.height * 0.35f ) : SRand::Inst().Rand( bound.y + bound.width * 0.65f, bound.GetBottom() ) );
			n = 0;

			if( m_velocity.Length2() > 64 * 64 )
			{
				auto pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( GetPosition() );
				pBullet->SetVelocity( CVector2( m_velocity.y, -m_velocity.x ) * 0.5f );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
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
	context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
	context.nBulletPageSize = 75;

	CBarrage* pBarrage = new CBarrage( context );
	CVector2 pos = GetPosition();
	float fAngle0 = GetRotation() + PI * 0.5f;
	pBarrage->AddFunc( [fAngle0] ( CBarrage* pBarrage )
	{
		uint32 nBullet = 0;
		for( int i = 0; i < 5; i++ )
		{
			float fAngle = fAngle0 + ( i - 2 ) * 0.47f;
			pBarrage->InitBullet( nBullet++, 0, -1, CVector2( -sin( fAngle ), cos( fAngle ) ) * 8, CVector2( cos( fAngle ), sin( fAngle ) ) * 180, CVector2( 0, 0 ) );
		}

		for( int i = 0; i < 5; i++ )
		{
			pBarrage->Yield( 8 );

			for( int j = 0; j < 5; j++ )
			{
				if( !pBarrage->IsBulletAlive( j ) )
					continue;

				float fAngle = fAngle0 + ( j - 2 ) * 0.5f;
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
	context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
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
				float fSpeed = 95 + j * 5;
				float fOfs = ( 1 - j ) * 32.0f;

				for( int k = 0; k < 6; k++ )
				{
					float fAngle30 = fAngle20 + k * PI * 2 / 6;
					float fAngle31 = fAngle21 + k * PI * 2 / 6;
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( fOfs * -sin( fAngle30 ), fOfs * cos( fAngle30 ) ), CVector2( fSpeed * cos( fAngle30 ), fSpeed * sin( fAngle30 ) ), CVector2( 0, 0 ) );
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( fOfs * sin( fAngle31 ), fOfs * -cos( fAngle31 ) ), CVector2( fSpeed * cos( fAngle31 ), fSpeed * sin( fAngle31 ) ), CVector2( 0, 0 ) );
				}

				pBarrage->Yield( j == 2 ? 20 : 10 );
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

void CSpider::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_nStopTimeLeft = m_nMoveStopTime;
	m_nMoveDir = -1;
	auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	pImage->SetFrames( 0, 1, 0 );
	memset( m_moveData.bHitChannel, 0, sizeof( m_moveData.bHitChannel ) );
}

void CSpider::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CEnemy::OnTickAfterHitTest();
	UpdateMove();
	if( !GetStage() )
		return;
	UpdateFire();
}

void CSpider::UpdateMove()
{
	bool bMove = m_nStopTimeLeft == 0 && m_nMoveDir >= 0;
	if( bMove )
		GetRenderObject()->SetRotation( m_nMoveDir * PI * 0.5f );
	m_moveData.UpdateMove( this, bMove ? CVector2( cos( m_nMoveDir * PI * 0.5f ), sin( m_nMoveDir * PI * 0.5f ) ) : CVector2( 0, 0 ) );
	uint8 nAnimState = bMove ? 1 : 0;
	if( nAnimState != m_nAnimState )
	{
		m_nAnimState = nAnimState;
		auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		if( nAnimState == 0 )
			pImage->SetFrames( 0, 1, 0 );
		else
			pImage->SetFrames( 0, 6, 8 );
	}

	CEntity* pLandedEntity = NULL;
	float fDist2 = FLT_MAX;
	bool bMoveDir[4] = { false, false, false, false };
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

		auto pBlockObject = SafeCast<CBlockObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pBlockObject )
		{
			auto pChunkObject = pBlockObject->GetBlock()->pOwner->pChunkObject;
			if( pChunkObject && pChunkObject->GetName() == m_strWebName )
			{
				CRectangle rect( pChunkObject->globalTransform.GetPosition().x, pChunkObject->globalTransform.GetPosition().y,
					pChunkObject->GetChunk()->nWidth * CMyLevel::GetBlockSize(),
					pChunkObject->GetChunk()->nHeight * CMyLevel::GetBlockSize() );
				CVector2 dPos = rect.GetCenter() - GetPosition();
				float d = dPos.Length2();
				if( d < fDist2 )
				{
					fDist2 = d;
					pLandedEntity = pChunkObject;
				}

				if( abs( dPos.x ) > abs( dPos.y ) )
				{
					if( dPos.x > m_fMoveThreshold )
						bMoveDir[0] = true;
					else if( dPos.x < -m_fMoveThreshold )
						bMoveDir[2] = true;
				}
				else
				{
					if( dPos.y > m_fMoveThreshold )
						bMoveDir[1] = true;
					else if( dPos.y < -m_fMoveThreshold )
						bMoveDir[3] = true;
				}
			}
		}
	}
	if( !pLandedEntity )
	{
		Kill();
		return;
	}

	m_moveData.SetLandedEntity( pLandedEntity );
	bool bStartMove = false;
	if( bMove )
	{
		if( m_nMoveTimeLeft )
			m_nMoveTimeLeft--;
		if( m_nMoveTimeLeft == 0 )
			m_nStopTimeLeft = m_nMoveStopTime;
	}
	else
	{
		if( m_nStopTimeLeft )
			m_nStopTimeLeft--;
		if( !m_nStopTimeLeft )
		{
			bStartMove = true;
			m_nMoveTimeLeft = m_nMoveTime;
		}
	}
	if( bStartMove || m_nMoveDir < 0 || !bMoveDir[m_nMoveDir] )
	{
		m_nMoveDir = -1;
		uint8 nMoveDirs[4];
		uint8 nMoveDirCount = 0;
		for( int i = 0; i < 4; i++ )
		{
			if( bMoveDir[i] )
				nMoveDirs[nMoveDirCount++] = i;
		}

		if( nMoveDirCount )
			m_nMoveDir = nMoveDirs[SRand::Inst().Rand<uint8>( 0, nMoveDirCount )];
	}
}

void CSpider::UpdateFire()
{
	if( m_nFireCDLeft )
		m_nFireCDLeft--;
	if( m_nNextFireTime )
		m_nNextFireTime--;

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	CVector2 p = pPlayer->GetPosition() - globalTransform.GetPosition();

	if( !m_nFireCDLeft )
	{
		if( p.Length2() > m_fSight * m_fSight )
			m_nFireCDLeft = 30;
		else
		{
			m_nFireCDLeft = m_nFireCD;
			m_nStopTimeLeft = m_nFireStopTime;
			m_nAmmoLeft = m_nAmmoCount;
			m_nNextFireTime = m_nFirstFireTime;
		}
	}

	if( m_nAmmoLeft )
	{
		float fAngle = atan2( p.y, p.x );
		GetRenderObject()->SetRotation( fAngle );
		if( m_nNextFireTime )
			m_nNextFireTime--;

		if( !m_nNextFireTime )
		{
			if( m_fPredict > 0 )
			{
				CVector2 vel = pPlayer->GetVelocity();
				float v = vel.Normalize();
				p.Normalize();
				float sn = p.x * vel.y - p.y * vel.x;
				float sn1 = sn / m_fBulletSpeed * v;
				if( sn1 > -1.0f && sn1 < 1.0f )
					fAngle += asin( sn ) * m_fPredict;
			}

			for( int i = 0; i < m_nBulletCount; i++ )
			{
				auto pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() );
				float r = fAngle + ( i - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
				pBullet->SetRotation( r );
				pBullet->SetVelocity( CVector2( cos( r ), sin( r ) ) * m_fBulletSpeed );
				pBullet->SetLife( m_nBulletLife );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}

			CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );

			m_nAmmoLeft--;
			m_nNextFireTime = m_nFireInterval;
		}
	}
}

void CSpider1::OnRemovedFromStage()
{
	m_moveData.pLandedEntity = NULL;
	CEnemy::OnRemovedFromStage();
}

void CSpider1::Attach( CEntity * pEntity )
{
	m_moveData.pLandedEntity = pEntity;
	m_pSpiderSilk->Set( pEntity, NULL, CVector2( 0, 0 ), CVector2( 0, 0 ), -1, -1 );

	auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	pImage->SetFrames( 0, 1, 0 );
	m_nAnimState = 0;
}

void CSpider1::OnKnockbackPlayer( const CVector2& vec )
{
	if( m_nState == 2 )
		Crush();
	else
	{
		m_nState = 0;
		AttackHit();
	}
}

bool CSpider1::IsKnockback()
{
	return m_nState == 0;
}

void CSpider1::Kill()
{
	if( !m_bCrushed )
	{
		if( m_nState != 2 )
		{
			m_moveData.pLandedEntity = NULL;
			m_nState = 2;
			if( m_pSpiderSilk )
			{
				m_pSpiderSilk->SetParentEntity( NULL );
				m_pSpiderSilk = NULL;
			}
		}
		return;
	}

	float fAngle0 = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < 8; i++ )
	{
		float fAngle = ( i + 0.5f ) * PI / 4 + fAngle0;
		for( int j = 0; j < 3; j++ )
		{
			float fAngle1 = fAngle + SRand::Inst().Rand( 0.1f, 0.15f ) * ( j - 1 );
			CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
			auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
			pBullet->SetPosition( globalTransform.GetPosition() );
			pBullet->SetVelocity( dir * SRand::Inst().Rand( 140.0f, 160.0f ) );
			pBullet->SetRotation( atan2( dir.y, dir.x ) );
			pBullet->SetAngularVelocity( ( j - 1 ) * 3.0f );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
	}
	CEnemy::Kill();
}

void CSpider1::OnTickBeforeHitTest()
{
	CEnemy::OnTickBeforeHitTest();
	if( m_nState == 2 )
	{
		float dTime = GetStage()->GetElapsedTimePerTick();
		CVector2 v0 = GetVelocity();
		CVector2 v1 = v0 + CVector2( 0, -m_fGravity ) * dTime;
		CVector2 dPos = ( v0 + v1 ) * dTime * 0.5f;
		SetPosition( GetPosition() + dPos );
		SetVelocity( v1 );
	}
}

void CSpider1::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CEnemy::OnTickAfterHitTest();
	uint8 nAnimState;
	CEntity* pAttached = m_moveData.pLandedEntity;
	if( m_nState == 0 || m_nState == 1 )
	{
		if( !pAttached || !pAttached->GetStage() )
		{
			Kill();
			return;
		}
		nAnimState = m_nState == 0 ? 1 : 0;
		m_moveData.fMoveSpeed = m_nState == 0 ? m_fSpeed : m_fSpeed1;
		CVector2 moveDir = m_nState == 0 ? CVector2( 0, 1 ) : CVector2( 0, -1 );
		if( m_nState == 0 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer && pPlayer->y > globalTransform.GetPosition().y )
			{
				m_moveData.fMoveSpeed = m_fSpeed1;
				moveDir = CVector2( 0, -1 );
			}
		}
		SetVelocity( moveDir * m_moveData.fMoveSpeed );
		m_moveData.UpdateMove( this, moveDir );
		if( !GetStage() )
			return;
		if( x != pAttached->globalTransform.GetPosition().x )
		{
			Crush();
			return;
		}

		if( m_nState == 1 )
		{
			if( m_moveData.hits[0].pHitProxy )
				AttackHit();
		}
		else
		{
			if( m_nStateTime )
				m_nStateTime--;
			if( !m_nStateTime )
			{
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( pPlayer && m_rectAttack.Contains( globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() ) ) )
				{
					if( m_moveData.hits[0].pHitProxy && m_moveData.hits[0].normal.y < 0 )
						m_nState = 1;
					else
					{
						float fAngle = atan2( pPlayer->y - globalTransform.GetPosition().y, pPlayer->x - globalTransform.GetPosition().x );
						for( int j = 0; j < 3; j++ )
						{
							float fAngle1 = fAngle + SRand::Inst().Rand( 0.1f, 0.15f ) * ( j - 1 );
							CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
							auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
							pBullet->SetPosition( globalTransform.GetPosition() );
							pBullet->SetVelocity( dir * SRand::Inst().Rand( 140.0f, 160.0f ) );
							pBullet->SetRotation( atan2( dir.y, dir.x ) );
							pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
						}

						m_nStateTime = m_nBeginAttackTime;
					}

				}
			}
		}
	}
	else
	{
		nAnimState = 0;
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( m_moveData.bHitChannel[pEntity->GetHitType()] )
			{
				Crush();
				return;
			}
		}
	}

	if( nAnimState != m_nAnimState )
	{
		m_nAnimState = nAnimState;
		auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		if( nAnimState == 0 )
			pImage->SetFrames( 0, 1, 0 );
		else
			pImage->SetFrames( 0, 6, 8 );
	}
}

void CSpider1::AttackHit()
{
	float fAngle0 = SRand::Inst().Rand( -0.3f, 0.3f );
	for( int i = 0; i < 8; i++ )
	{
		float fAngle = ( i - 3.5f ) * 0.6f + fAngle0;
		CVector2 dir( sin( fAngle ), -cos( fAngle ) );
		auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
		pBullet->SetPosition( globalTransform.GetPosition() );
		pBullet->SetVelocity( dir * 150 );
		pBullet->SetAcceleration( CVector2( 0, -150 ) );
		pBullet->SetTangentDir( true );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}
	m_nState = 0;
	m_nStateTime = m_nBeginAttackTime;
}

void CSpider1Web::OnAddedToStage()
{
	if( !CMyLevel::GetInst() )
	{
		m_pSpider->SetParentEntity( NULL );
		m_pSpider = NULL;
		return;
	}
	ForceUpdateTransform();
	m_pSpider->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
	m_pSpider->SetPosition( globalTransform.GetPosition() + m_pSpider->GetPosition() );
	m_pSpider->Attach( this );
}

void CSpider1Web::OnRemovedFromStage()
{
	if( m_pSpider && m_pSpider->GetStage() )
		m_pSpider->Kill();
}

void CSpider2::OnAddedToStage()
{
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			CReference<CRenderObject2D> pRenderObject = GetParentEntity()->GetRenderObject();
			if( pRenderObject && pChunk->GetDecoratorRoot() )
			{
				pRenderObject->RemoveThis();
				pChunk->GetDecoratorRoot()->AddChild( pRenderObject );
				pRenderObject->SetPosition( GetParentEntity()->GetPosition() );
			}
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			break;
		}
	}
	CEnemyTemplate::OnAddedToStage();
}

void CSpider2::OnRemovedFromStage()
{
	CEnemyTemplate::OnRemovedFromStage();
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
}

void CSpider2::Kill()
{
	if( m_bCrushed )
	{
		auto pWeb = SafeCast<CEntity>( m_pWeb->GetRoot()->CreateInstance() );
		pWeb->SetPosition( globalTransform.GetPosition() );
		pWeb->SetRotation( SRand::Inst().Rand( -PI, PI ) );
		pWeb->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
	CEnemy::Kill();
}

void CSpider2::AIFunc()
{
	while( 1 )
	{
		while( 1 )
		{
			m_pAI->Yield( 0.5f, true );
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 pos = pPlayer->GetPosition() - globalTransform.GetPosition();
				if( pos.Length2() < m_fSight * m_fSight )
					break;
			}
		}

		uint8 n = 0;
		while( 1 )
		{
			for( int i = 0; i < 60; i++ )
			{
				m_pAI->Yield( 0, true );
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( !pPlayer )
					break;
				CVector2 pos = pPlayer->GetPosition() - globalTransform.GetPosition();
				SetRotation( atan2( pos.y, pos.x ) );
			}

			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				break;

			CVector2 pos = pPlayer->GetPosition() - globalTransform.GetPosition();
			bool b1 = pos.Length2() < m_fSight1 * m_fSight1;
			b1 = b1 && SRand::Inst().Rand( 3, 6 ) >= n;
			if( !b1 )
				n++;
			else
				n = 0;

			float fAngle0 = atan2( pos.y, pos.x );
			auto pWeb = SafeCast<CEntity>( ( b1 ? m_pWeb : m_pWeb1 )->GetRoot()->CreateInstance() );
			pWeb->SetPosition( globalTransform.GetPosition() );
			pWeb->SetRotation( GetRotation() );
			pWeb->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );

			for( int i = 0; i < 400; i++ )
			{
				m_pAI->Yield( 0, true );
				pPlayer = GetStage()->GetPlayer();
				if( !pPlayer )
					break;
				pos = pPlayer->GetPosition() - globalTransform.GetPosition();
				SetRotation( atan2( pos.y, pos.x ) );
			}
			if( pos.Length2() >= m_fSight * m_fSight )
				break;
		}
	}
}

void CSpider2Web::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	SetAutoUpdateAnim( true );
}

void CSpider2Web::OnRemovedFromStage()
{
	m_hit.clear();
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CSpider2Web::OnTick()
{
	m_fCurDist += m_fSpeed * GetStage()->GetElapsedTimePerTick();
	float fDist = Min( m_fCurDist, m_fMaxDist );
	float fDist0 = Max( m_fCurDist - m_fWidth, 0.0f );
	float fSpawn = -1;
	if( m_fCurDist <= m_fMaxDist )
	{
		if( m_nSpawnCDLeft )
			m_nSpawnCDLeft--;
		if( !m_nSpawnCDLeft )
		{
			fSpawn = SRand::Inst().Rand<float>( 0.0f, m_nEmit );
			m_nSpawnCDLeft = m_nSpawnCD;
		}
	}
	if( fDist > fDist0 )
	{
		vector<CReference<CEntity> > result;
		vector<SHitTestResult> hitResult;
		for( int i = 0; i < m_nEmit; i++ )
		{
			float fAngle1 = GetRotation() + ( m_nType ? i * PI * 2 / m_nEmit : ( i - m_nEmit * 0.5f ) * m_fAngle );
			float fAngle2 = GetRotation() + ( m_nType ? ( i + 1 ) * PI * 2 / m_nEmit : ( i + 1 - m_nEmit * 0.5f ) * m_fAngle );
			CVector2 dir1( cos( fAngle1 ), sin( fAngle1 ) );
			CVector2 dir2( cos( fAngle2 ), sin( fAngle2 ) );
			SHitProxyPolygon hitProxy;
			if( fDist0 > 0 )
			{
				hitProxy.nVertices = 4;
				hitProxy.vertices[0] = dir1 * fDist0;
				hitProxy.vertices[1] = dir1 * fDist;
				hitProxy.vertices[2] = dir2 * fDist;
				hitProxy.vertices[3] = dir2 * fDist0;
			}
			else
			{
				hitProxy.nVertices = 3;
				hitProxy.vertices[0] = CVector2( 0, 0 );
				hitProxy.vertices[1] = dir1 * fDist;
				hitProxy.vertices[2] = dir2 * fDist;
			}
			hitProxy.CalcNormals();
			GetStage()->MultiHitTest( &hitProxy, globalTransform, result, &hitResult );

			float f = fSpawn - i;
			if( f >= 0 && f < 1 )
			{
				CVector2 pos = ( dir1 + ( dir2 - dir1 ) * f ) * fDist + globalTransform.GetPosition();
				SHitProxyCircle spawnHit;
				spawnHit.center = CVector2( 0, 0 );
				spawnHit.fRadius = m_fSpawnSize;
				auto pChunkObject = CMyLevel::GetInst()->TrySpawnAt( pos, &spawnHit );
				if( pChunkObject )
				{
					auto pEgg = SafeCast<CEntity>( m_pEggPrefab->GetRoot()->CreateInstance() );
					pEgg->SetPosition( pos - pChunkObject->globalTransform.GetPosition() );
					pEgg->SetParentEntity( pChunkObject );
				}
			}
		}

		for( int i = 0; i < hitResult.size(); i++ )
		{
			CEntity* pEntity = result[i];
			auto& res = hitResult[i];
			if( !pEntity->GetStage() )
				continue;
			if( m_hit.find( pEntity ) != m_hit.end() )
				continue;

			auto pCharacter = SafeCast<CCharacter>( pEntity );
			if( pCharacter && !SafeCast<CSpider2Egg>( pEntity ) && !pCharacter->IsKnockback() )
			{
				CVector2 p( pCharacter->globalTransform.GetPosition() - GetPosition() );
				p.Normalize();
				pCharacter->Knockback( p );
				m_hit.insert( pCharacter );
			}
		}
	}

	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
		{
			SetParentEntity( NULL );
			return;
		}
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CSpider2Egg::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			break;
		}
	}

	if( CMyLevel::GetInst() )
		GetRenderObject()->SetRenderParentAfter( CMyLevel::GetInst()->GetChunkEffectRoot() );
	auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	auto pImage1 = static_cast<CMultiFrameImage2D*>( m_pBase.GetPtr() );
	m_nRandomImg = SRand::Inst<eRand_Render>().Rand( 0, 4 );
	pImage->SetFrames( m_nRandomImg * 4, m_nRandomImg * 4 + 3, 4 );
	pImage1->SetFrames( m_nRandomImg * 4, m_nRandomImg * 4 + 3, 4 );
	pImage->SetPlaySpeed( 1.0f, false );
	pImage1->SetPlaySpeed( 1.0f, false );
	m_nLifeLeft = m_nLife;
}

void CSpider2Egg::Kill()
{
	if( m_bCrushed )
	{
		SBarrageContext context;
		context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
		context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
		context.nBulletPageSize = 4;

		class _CBarrage : public CBarrage
		{
		public:
			_CBarrage( const SBarrageContext& context ) : CBarrage( context ), m_nState( 0 ), m_nTime( 0 ) {}
		protected:
			virtual void OnTickAfterHitTest() override
			{
				if( m_nState == 0 )
				{
					if( m_nTime == 0 )
					{
						InitBullet( 0, -1, -1, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false,
							SRand::Inst().Rand( -PI, PI ), SRand::Inst().Rand( -6.0f, 6.0f ) );
						for( int i = 1; i <= 3; i++ )
						{
							InitBullet( i, 1, 0, CVector2( SRand::Inst().Rand( -8.0f, 8.0f ), SRand::Inst().Rand( -8.0f, 8.0f ) ),
								CVector2( 0, 0 ), CVector2( 0, 0 ) );
						}
					}
				}
				m_nTime++;
				if( m_nState == 0 )
				{
					CPlayer* pPlayer = GetStage()->GetPlayer();
					if( pPlayer )
					{
						auto pContext = GetBulletContext( 0 );
						pContext->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
						CVector2 dPos = pPlayer->GetPosition() - ( pContext->p0 + GetPosition() );
						if( m_nTime >= 30 && dPos.Length2() <= ( 50 + m_nTime * 2 ) * ( 50 + m_nTime * 2 ) )
						{
							m_nState = 1;
							m_nTime = 8;
							m_dir = dPos;
							if( m_dir.Normalize() < 0.01f )
								m_dir = CVector2( 1, 0 );
							m_n1 = SRand::Inst().Rand( 0, 2 ) * 2 - 1;
						}
						else
						{
							dPos.Normalize();
							pContext->SetBulletMove( dPos * Min( m_nTime, 120u ), CVector2( 0, 0 ) );
						}
					}
				}
				if( m_nState >= 1 && m_nState <= 3 )
				{
					if( m_nTime == 8 )
					{
						auto pContext0 = GetBulletContext( 0 );
						auto pContext = GetBulletContext( m_nState );
						if( pContext->IsValid() && pContext->pEntity )
						{
							float fAngle = m_n1 * ( m_nState - 2 ) * 0.3f;
							CVector2 dir( cos( fAngle ), sin( fAngle ) );
							dir = CVector2( dir.x * m_dir.x - dir.y * m_dir.y, dir.x * m_dir.y + dir.y * m_dir.x );
							InitBullet( m_nState, 0, -1, pContext0->p0, dir * ( 250 - m_nState * 50 ), CVector2( 0, 0 ), false,
								atan2( dir.y, dir.x ), m_n1 * 5 );
						}
						m_nState++;
						m_nTime = 0;
					}
				}
				if( m_nState > 3 && m_nTime == 2 )
					StopNewBullet();

				CBarrage::OnTickAfterHitTest();
			}

			uint8 m_nState;
			uint32 m_nTime;
			CVector2 m_dir;
			int8 m_n1;
		};
		auto pBarrage = new _CBarrage( context );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->SetPosition( globalTransform.GetPosition() );
		pBarrage->Start();
	}
	CEnemy::Kill();
}

void CSpider2Egg::OnTickAfterHitTest()
{
	m_nLifeLeft--;
	if( !m_nLifeLeft )
	{
		Crush();
		return;
	}

	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		if( static_cast<CEntity*>( pManifold->pOtherHitProxy )->GetHitType() <= eEntityHitType_Platform )
		{
			Crush();
			return;
		}
	}

	auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	auto pImage1 = static_cast<CMultiFrameImage2D*>( m_pBase.GetPtr() );
	if( m_nLifeLeft <= m_nLife / 4 )
	{
		uint32 nFrame = m_nRandomImg * 4 + 2 + ( ( ( ( m_nLife / 8 - m_nLifeLeft ) / 3 ) & 1 ) > 0 ? 0 : 1 );
		pImage->SetFrames( nFrame, nFrame + 1, 0 );
		pImage1->SetFrames( nFrame, nFrame + 1, 0 );
	}
	else if( m_nLifeLeft <= m_nLife / 2 )
	{
		uint32 nFrame = m_nRandomImg * 4 + 2 + ( ( ( ( m_nLife / 4 - m_nLifeLeft ) / 6 ) & 1 ) > 0 ? 0 : 1 );
		pImage->SetFrames( nFrame, nFrame + 1, 0 );
		pImage1->SetFrames( nFrame, nFrame + 1, 0 );
	}
	else if( m_nLifeLeft <= m_nLife * 3 / 4 )
	{
		uint32 nFrame = m_nRandomImg * 4 + 2 + ( ( ( ( m_nLife / 2 - m_nLifeLeft ) / 6 ) & 3 ) > 0 ? 0 : 1 );
		pImage->SetFrames( nFrame, nFrame + 1, 0 );
		pImage1->SetFrames( nFrame, nFrame + 1, 0 );
	}

	CEnemy::OnTickAfterHitTest();
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
					auto pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
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
	if( m_pExplosion )
		m_pExplosion->SetParentEntity( NULL );
}

bool CMaggot::Knockback( const CVector2 & vec )
{
	Kill();
	return true;
}

void CMaggot::OnKnockbackPlayer( const CVector2 & vec )
{
	CVector2 tangent( m_moveData.normal.y, -m_moveData.normal.x );
	float fTangent = tangent.Dot( vec );
	CVector2 vecKnockback = ( tangent * fTangent + m_moveData.normal ) * m_moveData.fFallInitSpeed * 5;
	if( m_moveData.bHitSurface )
		m_moveData.Fall( this, vecKnockback );
	else
		SetVelocity( GetVelocity() + vecKnockback );

	m_nKnockBackTimeLeft = m_nKnockbackTime;
}

bool CMaggot::IsKnockback()
{
	return m_nKnockBackTimeLeft > 0;
}

void CMaggot::Kill()
{
	if( m_pExplosion )
	{
		CBlockBuff::SContext context;
		context.nLife = m_nExplosionLife;
		context.nTotalLife = m_nExplosionLife;
		context.fParams[0] = m_fExplosionDmg;

		SafeCast<CExplosionWithBlockBuff>( m_pExplosion.GetPtr() )->Set( &context );
		m_pExplosion->SetPosition( GetPosition() );
		m_pExplosion->SetParentBeforeEntity( this );
		m_pExplosion = NULL;
	}
	CEnemy::Kill();
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

void CRat::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_flyData.bHitChannel[eEntityHitType_Platform] = false;

	m_nState = 0;
	int8 nDir = SRand::Inst().Rand( 0, 4 );
	CVector2 dirs[4] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	m_curMoveDir = dirs[nDir];
	m_nTick = SRand::Inst().Rand( 60, 120 );
	m_nAnimState = 0;
	auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	pImage->SetFrames( 8, 12, 12 );
	pImage->SetRotation( atan2( m_curMoveDir.y, m_curMoveDir.x ) );

	m_nTick = SRand::Inst().Rand( 60, 120 );
}

void CRat::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CEnemy::OnTickAfterHitTest();

	int8 nAnimState = m_nAnimState;
	CVector2 prePos = GetPosition();
	if( m_nTick )
		m_nTick--;
	if( m_nKnockBackTimeLeft )
		m_nKnockBackTimeLeft--;

	if( m_nState == 1 )
	{
		bool bJump = false;
		if( !m_nTick )
		{
			bJump = true;
			m_nTick = SRand::Inst().Rand( 60, 120 );
		}
		CVector2 fixedVelocity = m_walkData.UpdateMove( this, m_curMoveDir.x > 0 ? 1 : -1, bJump );
		if( !GetStage() )
			return;

		auto levelBound = CMyLevel::GetInst()->GetBound();
		if( x < levelBound.x || x > levelBound.GetRight() || y < levelBound.y )
		{
			Kill();
			return;
		}

		if( fixedVelocity.x == 0 )
			m_curMoveDir.x = -m_curMoveDir.x;

		CChunkObject* pCurRoom = NULL;
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CChunkObject* pChunkObject = SafeCast<CChunkObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
			if( pChunkObject && pChunkObject->GetChunk()->nMoveType )
			{
				CRectangle rect( pChunkObject->globalTransform.GetPosition().x, pChunkObject->globalTransform.GetPosition().y,
					pChunkObject->GetChunk()->nWidth * CMyLevel::GetBlockSize(),
					pChunkObject->GetChunk()->nHeight * CMyLevel::GetBlockSize() );
				if( rect.Contains( GetPosition() ) )
					pCurRoom = pChunkObject;
			}
		}

		if( pCurRoom )
		{
			m_flyData.Reset();
			m_flyData.SetLandedEntity( pCurRoom );
			m_flyData.fKnockbackTime = m_walkData.fKnockbackTime;
			m_flyData.vecKnockback = m_walkData.vecKnockback;
			m_nState = 0;
		}
	}
	else
	{
		if( !m_flyData.pLandedEntity )
		{
			for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				auto pChunkObject = SafeCast<CChunkObject>( pEntity );
				if( pChunkObject )
				{
					m_flyData.SetLandedEntity( pChunkObject );
					break;
				}
			}
		}
		if( m_flyData.pLandedEntity && !m_flyData.pLandedEntity->GetStage() )
			m_flyData.SetLandedEntity( NULL );
		if( m_flyData.pLandedEntity )
		{
			bool bHitChunkObject = false;
			for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				auto pChunkObject = SafeCast<CChunkObject>( pEntity );
				if( m_flyData.pLandedEntity == pChunkObject )
				{
					bHitChunkObject = true;
					break;
				}
			}
			if( !bHitChunkObject )
			{
				m_flyData.pLandedEntity = NULL;
			}
		}

		if( m_flyData.pLandedEntity )
		{
			if( !m_nTick )
			{
				int8 nDir = SRand::Inst().Rand( 0, 4 );
				CVector2 dirs[4] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
				m_curMoveDir = dirs[nDir];
				m_nTick = SRand::Inst().Rand( 60, 120 );
			}

			m_flyData.UpdateMove( this, m_curMoveDir );
			if( !GetStage() )
				return;

			if( m_flyData.finalMoveAxis.x == 0 )
				m_curMoveDir.x = -m_curMoveDir.x;
			if( m_flyData.finalMoveAxis.y == 0 )
				m_curMoveDir.y = -m_curMoveDir.y;
		}
		if( !m_flyData.pLandedEntity )
		{
			m_flyData.bHitChannel[eEntityHitType_Platform] = true;
			m_walkData.Reset();
			m_walkData.fKnockbackTime = m_flyData.fKnockbackTime;
			m_walkData.vecKnockback = m_flyData.vecKnockback;
			if( m_curMoveDir.x == 0 )
				m_curMoveDir = SRand::Inst().Rand( 0, 2 ) ? CVector2( -1, 0 ) : CVector2( 1, 0 );
			m_nState = 1;
		}
	}

	if( m_nState == 1 )
	{
		if( m_curMoveDir.x > 0 )
			m_nAnimState = 1;
		else
			m_nAnimState = -1;
		m_pRenderObject->SetRotation( 0 );
	}
	else
	{
		m_nAnimState = 0;
		m_pRenderObject->SetRotation( atan2( m_curMoveDir.y, m_curMoveDir.x ) );
	}

	auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	if( nAnimState != m_nAnimState )
	{
		if( m_nAnimState == 1 )
			pImage->SetFrames( 0, 4, 12 );
		else if( m_nAnimState == -1 )
			pImage->SetFrames( 4, 8, 12 );
		else
			pImage->SetFrames( 8, 12, 12 );
	}

	CVector2 curPos = GetPosition();
	m_velocity = ( curPos - prePos ) / GetStage()->GetElapsedTimePerTick();
}

bool CRat::Knockback( const CVector2 & vec )
{
	if( m_nState == 1 )
		m_curMoveDir = CVector2( vec.x > 0 ? 1 : -1, 0 );
	else
	{
		int8 a = vec.y > vec.x;
		int8 b = vec.y > -vec.x;
		int8 n = a + b * 2;
		CVector2 dirs[4] = { { 0, -1 }, { -1, 0 }, { 1, 0 }, { 0, 1 } };
		m_curMoveDir = dirs[n];
	}

	m_nKnockBackTimeLeft = m_nKnockbackTime;
	return true;
}

void CRat::OnKnockbackPlayer( const CVector2 & vec )
{
	SDamageContext context;
	context.nDamage = 5;
	context.nType = 0;
	context.nSourceType = 0;
	context.hitPos = context.hitDir = CVector2( 0, 0 );
	context.nHitType = -1;
	Damage( context );
}

bool CRat::IsKnockback()
{
	return m_nKnockBackTimeLeft > 0;
}

void CRat::Kill()
{
	SBarrageContext context;
	context.vecBulletTypes.push_back( m_strPrefab.GetPtr() );
	context.nBulletPageSize = 9;

	CBarrage* pBarrage = new CBarrage( context );
	CVector2 pos = GetPosition();
	float fAngle0 = atan2( m_curMoveDir.y, m_curMoveDir.x );
	pBarrage->AddFunc( [fAngle0] ( CBarrage* pBarrage )
	{
		pBarrage->Yield( 4 );
		int32 iBullet = 0;
		float fSpeed = 175;
		for( int i = 0; i < 5; i++ )
		{
			float fAngle1 = fAngle0 + ( i - 2 ) * 0.2f;
			pBarrage->InitBullet( iBullet++, 0, -1, CVector2( 0, 0 ), CVector2( fSpeed * cos( fAngle1 ), fSpeed * sin( fAngle1 ) ), CVector2( 0, 0 ) );
		}
		pBarrage->Yield( 8 );
		for( int i = 0; i < 4; i++ )
		{
			float fAngle1 = fAngle0 + ( i - 1.5f ) * 0.2f;
			pBarrage->InitBullet( iBullet++, 0, -1, CVector2( 0, 0 ), CVector2( fSpeed * cos( fAngle1 ), fSpeed * sin( fAngle1 ) ), CVector2( 0, 0 ) );
		}
		pBarrage->Yield( 4 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( pos );
	pBarrage->Start();

	CEnemy::Kill();
}
