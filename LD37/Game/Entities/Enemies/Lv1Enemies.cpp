#include "stdafx.h"
#include "Lv1Enemies.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Entities/Barrage.h"
#include "Entities/Bullets.h"
#include "Render/Rope2D.h"
#include "Common/ResourceManager.h"
#include "Common/MathUtil.h"

void CManHead1::AIFunc()
{
	DEFINE_TEMP_REF_THIS();
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
	m_flyData.bHitChannel[eEntityHitType_WorldStatic] = m_flyData.bHitChannel[eEntityHitType_Platform] = m_flyData.bHitChannel[eEntityHitType_System] = true;
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
			SetVelocity( CVector2( GetVelocity().x, Min( -50.0f, GetVelocity().y ) ) );
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
			if( m_moveData.bHitChannel[pEntity->GetHitType()] && pManifold->normal.Length2() > 0 )
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

void CSpider2::OnRemovedFromStage()
{
	m_vecLinks.clear();
	CEnemyTemplate::OnRemovedFromStage();
}

void CSpider2::OnLinkRemoved( CEntity* pLink )
{
	for( int i = 0; i < m_vecLinks.size(); i++ )
	{
		if( m_vecLinks[i] == pLink )
		{
			m_vecLinks[i] = m_vecLinks.back();
			m_vecLinks.pop_back();
			return;
		}
	}
}

void CSpider2::AddShake( const CVector2& shake )
{
	m_shake = m_shake + shake;
}

void CSpider2::AIFunc()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.bHitChannel[eEntityHitType_System] = false;

	int32 nCreateLinkTick = 0;
	while( 1 )
	{
		m_pAI->Yield( 0, true );
		UpdateMovement();
		if( !GetStage() )
			return;

		if( m_vecLinks.size() < 6 )
		{
			if( nCreateLinkTick )
				nCreateLinkTick--;
			if( !nCreateLinkTick )
			{
				if( CreateLink() && m_vecLinks.size() < 4 )
					nCreateLinkTick = 20;
				else
					nCreateLinkTick = 4;
			}
		}
	}
}

void CSpider2::UpdateMovement()
{
	DEFINE_TEMP_REF_THIS();
	CVector2 acc( 0, 0 );
	CVector2 p = globalTransform.GetPosition();
	bool b = false;
	for( CEntity* pEntity : m_vecLinks )
	{
		CVector2 d = SafeCast<CSpider2Link>( pEntity )->GetTargetPos() - p;
		if( d.y > 0 )
			b = true;
		else
		{
			auto pTarget = SafeCast<CBlockObject>( SafeCast<CSpider2Link>( pEntity )->GetTarget() );
			if( pTarget )
			{
				if( pTarget->GetBlock()->pOwner->pChunkObject->GetName() == m_strWeb )
					b = true;
			}
		}
		float l = d.Normalize();
		float fForce = ( l - m_l0 ) * m_k;
		acc = acc + d * fForce;
	}
	acc.y -= m_fGravity;
	acc = acc + m_shake * m_fShake;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		CVector2 d = pPlayer->GetPosition() - globalTransform.GetPosition();
		SetRotation( atan2( d.y, d.x ) );
		if( d.Length2() < 500 * 500 )
		{
			float l = d.Normalize();
			acc = acc + d * ( Max( 0.0f, 150.0f - abs( l - 350.0f ) ) ) * ( m_vecLinks.size() * 1.2f );
			acc = acc * Max( 0.0f, 150 - Max( 0.0f, d.Dot( GetVelocity() ) ) ) / 150.0f;
		}
	}
	m_shake = CVector2( 0, 0 );
	m_moveData.UpdateMove( this, acc );

	if( !GetStage() )
		return;
	for( int i = 0; i < 3; i++ )
	{
		if( !m_moveData.hits[i].pHitProxy )
			break;
		if( m_moveData.hits[i].normal.y > 0 && !b )
		{
			Kill();
			return;
		}
	}
}

bool CSpider2::CreateLink()
{
	CVector2 beginCenter = globalTransform.GetPosition();

	float fAngle0 = SRand::Inst().Rand( -PI, PI );
	bool bLink[8];
	memset( bLink, 0, sizeof( bLink ) );
	for( int i = 0; i < m_vecLinks.size(); i++ )
	{
		auto pLink = SafeCast<CSpider2Link>( m_vecLinks[i].GetPtr() );
		CVector2 dPos = pLink->GetTargetPos() - globalTransform.GetPosition();
		float fAngle = atan2( dPos.y, dPos.x ) - fAngle0;
		int32 n = floor( fAngle / ( PI * 2 / ELEM_COUNT( bLink ) ) );
		while( n < 0 )
			n += ELEM_COUNT( bLink );
		while( n >= ELEM_COUNT( bLink ) )
			n -= ELEM_COUNT( bLink );
		bLink[n] = true;
	}
	float fAngles[8];
	int32 nAngles = 0;
	for( int i = 0; i < ELEM_COUNT( bLink ); i++ )
	{
		if( !bLink[i] )
			fAngles[nAngles++] = fAngle0 + ( i + 0.5f ) * PI * 2 / ELEM_COUNT( bLink );
	}
	SRand::Inst().Shuffle( fAngles, nAngles );

	for( int i = 0; i < nAngles; i++ )
	{
		float fAngle = fAngles[i];

		CVector2 dir( cos( fAngle ), sin( fAngle ) );
		float l = m_fLinkLen;
		CVector2 dCenter = dir * l;
		CVector2 endCenter = beginCenter + dCenter;
		CVector2 begin = beginCenter + dir * m_fLinkWidth * 0.5f;

		SHitProxyPolygon polygon;
		polygon.nVertices = 3;
		polygon.vertices[0] = CVector2( 0, 0 );
		polygon.vertices[1] = ( CVector2( -dir.y, dir.x ) - dir ) * m_fLinkWidth * 0.5f;
		polygon.vertices[2] = ( CVector2( dir.y, -dir.x ) - dir ) * m_fLinkWidth * 0.5f;
		polygon.CalcNormals();
		vector<CReference<CEntity> > result;
		vector<SRaycastResult> raycastResult;
		CMatrix2D trans;
		trans.Translate( begin.x, begin.y );
		GetStage()->MultiSweepTest( &polygon, trans, endCenter - begin, result, &raycastResult );

		CEntity* pHitEntity = NULL;
		CVector2 targetPos;
		CVector2 normal;

		for( auto& item : raycastResult )
		{
			auto pEntity = static_cast<CEntity*>( item.pHitProxy );
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
				if( pBlockObject )
				{
					if( pBlockObject->GetBlock()->eBlockType != eBlockType_Block )
						continue;
					pHitEntity = pBlockObject;
					targetPos = pBlockObject->globalTransform.GetPosition() + CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize();
					normal = item.normal;
					break;
				}
			}
		}
		if( pHitEntity )
		{
			if( normal.Dot( dir ) > -0.6f )
				continue;
			bool b = false;
			for( auto& item : m_vecLinks )
			{
				if( ( SafeCast<CSpider2Link>( item.GetPtr() )->GetTargetPos() - targetPos ).Length2() <= 50 * 50 )
				{
					b = true;
					break;
				}
			}
			if( b )
				continue;
			auto pLink = SafeCast<CSpider2Link>( m_pWebLinkDrawable->GetRoot()->CreateInstance() );
			pLink->SetOwner( this, pHitEntity, targetPos );
			m_vecLinks.push_back( pLink );
			return true;
		}
	}

	return false;
}

void CSpider2Link::SetOwner( CSpider2* pOwner, CEntity* pTarget, const CVector2& targetPos )
{
	m_pTarget->SetParentEntity( pTarget );
	m_pTarget->SetPosition( pTarget->globalTransform.MulTVector2PosNoScale( targetPos ) );
	SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
	m_pTarget->SetRenderParent( this );
	Set( pOwner, m_pTarget, CVector2( 0, 0 ), CVector2( 0, 0 ), -1, -1 );

	CRopeObject2D* pRope = static_cast<CRopeObject2D*>( GetRenderObject() );
	float tx = SRand::Inst().Rand( 0.0f, 1.0f ), ty = SRand::Inst().Rand( 0.0f, 1.0f );
	for( int i = 0; i < pRope->GetData().data.size(); i++ )
	{
		auto pParam = pRope->GetParam( i );
		pParam->z = tx;
		pParam->w = ty;
	}
	m_nFireCD = SRand::Inst().Rand( 100, 120 );
}

void CSpider2Link::OnHit( CEntity* pEntity, const CVector2& hitPoint )
{
	if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
	{
		CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject )
		{
			if( pBlockObject->GetBlock()->eBlockType != eBlockType_Block )
				return;
			if( ( hitPoint - m_pTarget->globalTransform.GetPosition() ).Length2() < 32 * 32 )
				return;
			m_pTarget->Kill();
		}
	}
}

void CSpider2Link::OnTick()
{
	DEFINE_TEMP_REF_THIS();
	m_fFadeIn = Min( 1.0f, m_fFadeIn + m_fFadeInSpeed * GetStage()->GetElapsedTimePerTick() );
	if( m_bKilled )
	{
		m_fFadeOut = Min( 1.0f, m_fFadeOut + m_fFadeOutSpeed * GetStage()->GetElapsedTimePerTick() );
		if( m_fFadeOut >= 1 )
		{
			SetParentEntity( NULL );
			return;
		}
		UpdateRenderObject();
		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
		return;
	}
	CLightning::OnTick();
	if( !GetStage() || m_bKilled )
		return;

	if( m_fBeamLen > m_fMaxLen )
	{
		m_pTarget->Kill();
		return;
	}
	auto pBlock = SafeCast<CBlockObject>( m_pTarget->GetParentEntity() );
	if( pBlock )
	{
		auto pChunkObject = pBlock->GetBlock()->pOwner->pChunkObject;
		while( pChunkObject )
		{
			SafeCast<CSpider2>( m_pBegin.GetPtr() )->AddShake( pChunkObject->GetShake() );
			pChunkObject = SafeCast<CChunkObject>( pChunkObject->GetParentEntity() );
		}
	}

	if( m_nFireCD )
		m_nFireCD--;
	if( !m_nFireCD )
	{
		CVector2 p = m_pTarget->globalTransform.GetPosition();
		if( !m_nBullet )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 d = pPlayer->GetPosition() - p;
				if( d.Length2() < 320 * 320 )
				{
					m_nBullet = 3;
					m_nFireCD = 10;
					d.Normalize();
					m_d = d;
				}
			}
		}
		else
		{
			auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
			pBullet->SetPosition( p );
			pBullet->SetVelocity( m_d * ( 150 + m_nBullet * 15 ) );
			pBullet->SetRotation( atan2( m_d.y, m_d.x ) );
			pBullet->SetAngularVelocity( SRand::Inst().Rand( 3.0f, 6.0f ) * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 ) );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			m_nBullet--;
			m_nFireCD = m_nBullet ? 10 : SRand::Inst().Rand( 100, 140 );
		}
	}
}

void CSpider2Link::UpdateRenderObject()
{
	CRopeObject2D* pRope = static_cast<CRopeObject2D*>( GetRenderObject() );
	for( int i = 0; i < pRope->GetData().data.size(); i++ )
	{
		auto pParam = pRope->GetParam( i );
		pParam->x = Max( 0.0001f, m_fFadeIn );
		pParam->y = m_fFadeOut;
	}
	CLightning::UpdateRenderObject();
}

void CSpider2Link::OnBeginRemoved()
{
	m_bKilled = true;
	CLightning::OnBeginRemoved();
	if( m_pTarget )
	{
		m_pTarget->Kill();
		m_pTarget = NULL;
	}
}

void CSpider2Link::OnEndRemoved()
{
	m_bKilled = true;
	if( m_pBegin )
	{
		SafeCast<CSpider2>( m_pBegin.GetPtr() )->OnLinkRemoved( this );
		m_begin = m_pBegin->globalTransform.MulVector2Pos( m_begin );
		m_begin = globalTransform.MulTVector2PosNoScale( m_begin );
		if( m_onBeginRemoved.IsRegistered() )
			m_onBeginRemoved.Unregister();
		m_pBegin = NULL;
	}
	CLightning::OnEndRemoved();
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
	m_nDir = m_nType == 0 ? SRand::Inst().Rand( -1, 2 ) : SRand::Inst().Rand( 0, 2 ) * 2 - 1;
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
	CVector2 vecKnockback = ( tangent * fTangent + m_moveData.normal ) * m_fKnockbackSpeed;
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

CFly* CMaggot::Morph()
{
	auto pFly = SafeCast<CFly>( m_pFly->GetRoot()->CreateInstance() );
	pFly->SetPosition( globalTransform.GetPosition() );
	pFly->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	Kill();
	return pFly;
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
			{
				if( m_nType == 0 )
					m_nDir = SRand::Inst().Rand( -1, 2 );
				else
				{
					CVector2 moveDir( m_moveData.normal.y, -m_moveData.normal.x );
					if( moveDir.y > 0 )
						m_nDir = 1;
					else if( moveDir.y < 0 )
						m_nDir = -1;
					else
						m_nDir = SRand::Inst().Rand( 0, 2 ) * 2 - 1;
				}
			}

			if( m_nType == 1 && SRand::Inst().Rand( 0.0f, 1.0f ) < m_fFallChance )
			{
				if( m_moveData.normal.y > 0.8f )
					m_moveData.Fall( this, CVector2( m_nDir * m_moveData.fSpeed, m_moveData.normal.y * m_moveData.fFallInitSpeed ) );
				else if( m_moveData.normal.y > -0.1f && m_moveData.normal.y < 0.1f )
				{
					CRectangle rect( x, y, 0, 32 );
					if( m_moveData.normal.x < 0 )
						rect.SetLeft( rect.GetRight() - 128 );
					else
						rect.width = 128;
					SHitProxyPolygon hitProxy( rect );
					CMatrix2D trans;
					trans.Identity();
					vector<CReference<CEntity> > result;
					GetStage()->MultiHitTest( &hitProxy, trans, result );
					bool bHit = false;
					for( CEntity* pEntity : result )
					{
						if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
						{
							bHit = true;
							break;
						}
					}

					if( bHit )
						m_moveData.Fall( this, CVector2( m_moveData.normal.x * m_moveData.fFallInitSpeed, m_moveData.fFallInitSpeed ) );
				}
			}
			if( m_moveData.bHitSurface && m_moveData.normal.y < -0.8f && SRand::Inst().Rand( 0.0f, 1.0f ) < m_fFallChance )
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
			pImage->SetFrames( 0, 4, m_nAnimSpeed );
			break;
		case 2:
			pImage->SetFrames( 4, 8, m_nAnimSpeed );
			break;
		case 3:
			pImage->SetFrames( 8, 12, m_nAnimSpeed );
			break;
		case 4:
			pImage->SetFrames( 12, 16, m_nAnimSpeed );
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

void CFly::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_r = SRand::Inst().Rand( 0.5f, 1.0f );
	m_nFireCDLeft = m_nFireCD;
	m_nCreateFlyGroupCD = SRand::Inst().Rand( 15, 30 );
}

void CFly::OnRemovedFromStage()
{
	if( m_pFlyGroup )
		SetFlyGroup( NULL );
	CEnemy::OnRemovedFromStage();
}

void CFly::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();

	m_flyData.fMaxAcc = Min( m_fMaxAcc, m_flyData.fMaxAcc + m_fAcc1 * GetStage()->GetElapsedTimePerTick() );
	CVector2 p = GetPosition();
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( m_pTarget )
	{
		if( m_pTarget->GetStage() == GetStage() )
		{
			p = m_pTarget->GetTransform( m_nTargetTransformIndex + 1 ).GetPosition();
			m_flyData.fStablity = 0.25f;
			m_flyData.UpdateMove( this, p );
		}
		else
			Set( NULL );
	}
	if( !m_pTarget )
	{
		float r = 128.0f;
		if( !m_pFlyGroup && CMyLevel::GetInst() )
		{
			m_nCreateFlyGroupCD--;
			if( !m_nCreateFlyGroupCD )
			{
				auto pFlyGroup = new CFlyGroup( m_nType );
				pFlyGroup->SetPosition( GetPosition() );
				pFlyGroup->SetParentEntity( CMyLevel::GetInst() );
				SetFlyGroup( pFlyGroup );
			}
		}
		if( m_pFlyGroup )
		{
			r = 100 + SafeCast<CFlyGroup>( m_pFlyGroup.GetPtr() )->GetCount() * 3;
			if( m_nType == 1 )
			{
				if( m_nTargetCD )
					m_nTargetCD--;
				if( !m_nTargetCD )
				{
					m_target = m_pFlyGroup->GetPosition();
					m_nTargetCD = SRand::Inst().Rand( 60, 120 );
				}
				p = m_target;
			}
			else
				p = m_pFlyGroup->GetPosition();
		}

		r *= m_r;
		CVector2 d = GetPosition() - p;
		float l = d.Normalize();
		m_flyData.fStablity = Min( 0.5f, l / r * 0.5f );
		m_flyData.UpdateMove( this, p );
	}

	if( m_nKnockBackTimeLeft )
		m_nKnockBackTimeLeft--;
	if( m_nFireCDLeft )
		m_nFireCDLeft--;
	if( pPlayer && !m_nFireCDLeft )
	{
		CVector2 d = pPlayer->GetPosition() - GetPosition();
		float l = d.Normalize();
		if( l <= m_fFireDist && l >= m_fFireMinDist )
		{
			if( m_pTarget )
			{
				d = pPlayer->GetPosition() - ( GetPosition() + m_pTarget->GetTransform( 0 ).GetPosition() ) * 0.5f;
				d.Normalize();
			}
			auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
			pBullet->SetPosition( globalTransform.GetPosition() );
			pBullet->SetRotation( atan2( d.y, d.x ) );
			pBullet->SetVelocity( d * 175 );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			m_nFireCDLeft = m_nFireCD;

			SDamageContext dmgContext;
			dmgContext.nDamage = 1;
			dmgContext.nType = 0;
			dmgContext.nSourceType = 0;
			dmgContext.hitPos = dmgContext.hitDir = CVector2( 0, 0 );
			dmgContext.nHitType = -1;
			Damage( dmgContext );
		}
	}
}

bool CFly::Knockback( const CVector2 & vec )
{
	SetVelocity( GetVelocity() + vec * 500 );
	m_nKnockBackTimeLeft = m_nKnockbackTime;
	return true;
}

bool CFly::IsKnockback()
{
	return m_nKnockBackTimeLeft > 0;
}

void CFly::SetFlyGroup( CFlyGroup* pFlyGroup )
{
	DEFINE_TEMP_REF_THIS();
	if( m_pFlyGroup )
	{
		SafeCast<CFlyGroup>( m_pFlyGroup.GetPtr() )->OnFlyRemoved();
		RemoveFrom_Fly();
	}
	m_pFlyGroup = pFlyGroup;
	if( pFlyGroup )
	{
		pFlyGroup->OnFlyAdded();
		pFlyGroup->Insert_Fly( this );
		m_nCreateFlyGroupCD = 39;
		m_nTargetCD = 0;
		m_target = GetPosition();
	}
}

void CFly::Set( CEntity * pTarget, uint16 nTransformIndex )
{
	SetFlyGroup( NULL );
	if( m_pTarget && !pTarget )
		m_flyData.fMaxAcc = Min( m_fMaxAcc, GetVelocity().Length() * 0.6f );
	m_pTarget = pTarget;
	m_nTargetTransformIndex = nTransformIndex;
	if( pTarget )
		m_nCreateFlyGroupCD = 60;
}

void CFlyGroup::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	m_vel.x = SRand::Inst().Rand( 50.0f, 80.0f ) * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 );
	m_vel.y = SRand::Inst().Rand( 50.0f, 80.0f ) * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 );
	if( m_nType == 1 )
		m_vel = m_vel * 20;
	m_nTime = SRand::Inst().Rand( 0, 30 ) + 1;
}

void CFlyGroup::OnRemovedFromStage()
{
	while( m_pFlies )
		m_pFlies->SetFlyGroup( NULL );
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CFlyGroup::Merge( CFlyGroup * pFlyGroup1 )
{
	int32 n = 0;
	uint32 nCount1 = pFlyGroup1->GetCount();
	if( m_nCount + nCount1 <= 32 )
		n = nCount1;
	else
		n = nCount1 - ( m_nCount + nCount1 ) / 2;
	for( int i = 0; i < n; i++ )
		pFlyGroup1->Get_Fly()->SetFlyGroup( this );
	if( n >= nCount1 )
		pFlyGroup1->SetParentEntity( NULL );
}

void CFlyGroup::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );

	if( m_nTime )
		m_nTime--;
	if( !m_nTime )
	{
		if( m_nCount < 32 )
		{
			SHitProxyCircle circle;
			circle.center = CVector2( 0, 0 );
			circle.fRadius = 64;
			vector<CReference<CEntity> > result;
			GetStage()->MultiHitTest( &circle, globalTransform, result );
			for( CEntity* pEntity : result )
			{
				auto pFly = SafeCast<CFly>( pEntity );
				if( pFly )
				{
					if( pFly->GetTarget() || pFly->GetType() != m_nType )
						continue;
					auto pGroup = SafeCast<CFlyGroup>( pFly->GetGroup() );
					if( pGroup )
					{
						if( pGroup != this )
							Merge( pGroup );
						pFly = NULL;
					}
				}
				else
				{
					auto pMaggot = SafeCast<CMaggot>( pEntity );
					if( pMaggot )
						pFly = pMaggot->Morph();
				}

				if( pFly )
					pFly->SetFlyGroup( this );
				if( m_nCount >= 32 )
					break;
			}
		}
		m_nTime = 30;
	}

	SetPosition( GetPosition() + m_vel * GetStage()->GetElapsedTimePerTick() );
	if( m_nType == 0 )
	{
		auto bound = CMyLevel::GetInst()->GetBound();
		bound.SetSize( CVector2( Max( 0.0f, bound.width - 200 ), Max( 0.0f, bound.height - 200 ) ) );
		if( x < bound.x && m_vel.x < 0 || x > bound.GetRight() && m_vel.x > 0 )
			m_vel.x = -m_vel.x;
		if( y < bound.y && m_vel.y < 0 || y > bound.GetBottom() && m_vel.y > 0 )
			m_vel.y = -m_vel.y;
	}
	else
	{
		CVector2 target = CMyLevel::GetInst()->GetBound().GetCenter();
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
			target = pPlayer->GetPosition();
		CRectangle bound( target.x - 256.0f, target.y + 256.0f, 512.0f, 128.0f );
		bound = CMyLevel::GetInst()->GetBoundWithLvBarrier() * bound;
		if( x < bound.x && m_vel.x < 0 || x > bound.GetRight() && m_vel.x > 0 )
			m_vel.x = -m_vel.x;
		if( y < bound.y && m_vel.y < 0 || y > bound.GetBottom() && m_vel.y > 0 )
			m_vel.y = -m_vel.y;
	}

	if( !m_nCount )
	{
		if( m_nLife )
			m_nLife--;
		if( !m_nLife )
			SetParentEntity( NULL );
	}
	else
		m_nLife = 300;
}

void CRat::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_flyData.bHitChannel[eEntityHitType_System] = false;

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
		CVector2 fixedVelocity = m_walkData.UpdateMove( this, CVector2( 0, 0 ), m_curMoveDir.x > 0 ? 1 : -1, bJump );
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
			m_flyData.bHitChannel[eEntityHitType_System] = true;
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

void CBloodDrop::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			int32 nWidth = pChunk->GetChunk()->nWidth;
			int32 nHeight = pChunk->GetChunk()->nHeight;
			int32 nGridX = floor( GetPosition().x / CMyLevel::GetBlockSize() );
			int32 nGridY = floor( GetPosition().y / CMyLevel::GetBlockSize() );
			for( ; nGridY > 0; nGridY-- )
			{
				if( pChunk->GetBlock( nGridX, nGridY - 1 )->eBlockType < eBlockType_Block )
					break;
			}

			m_fTargetY = nGridY * CMyLevel::GetBlockSize();
			m_pRenderObject->SetPosition( CVector2( 0, m_fTargetY - y ) );
			break;
		}
	}

	GetRenderObject()->bVisible = false;
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	auto texRect = pImage->GetElem().texRect;
	texRect.x = SRand::Inst().Rand( 0, 8 ) * 0.125f;
	pImage->SetTexRect( texRect );

	if( CMyLevel::GetInst() && m_fTargetY == 0 )
		GetRenderObject()->SetRenderParentAfter( CMyLevel::GetInst()->GetChunkRoot1() );
	m_pKilled->SetParentEntity( NULL );
	auto pEft = static_cast<CRopeObject2D*>( m_pEft->GetRenderObject() );
	pEft->SetDataCount( 0 );
	m_pEft->SetAutoUpdateAnim( true );
}

void CBloodDrop::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	auto pEft = static_cast<CRopeObject2D*>( m_pEft->GetRenderObject() );
	auto& data = pEft->GetData();

	float fLen = y - m_fTargetY;
	m_l += m_fSpeed * GetStage()->GetElapsedTimePerTick();
	float l = floor( m_l );
	float l0 = Min( fLen, l );
	if( l0 <= 32 )
	{
		pEft->SetDataCount( 2 );
		pEft->SetData( 0, CVector2( 0, 0 ), 32, CVector2( 0, 0 ), CVector2( 1, 2 ) );
		pEft->SetData( 1, CVector2( 0, -l0 ), 32, CVector2( 0, l0 / 256 ), CVector2( 1, Min( 1.0f, Max( 0.0f, ( l - 32 ) / 32 ) ) ) );
	}
	else
	{
		pEft->SetDataCount( 3 );
		pEft->SetData( 0, CVector2( 0, 0 ), 32, CVector2( 0, 0 ), CVector2( 1, 2 ) );
		pEft->SetData( 1, CVector2( 0, -32 ), 32, CVector2( 0, 32.0f / 256 ), CVector2( 1, Min( 1.0f, Max( 0.0f, ( l - 32 ) / 32 ) ) ) );
		pEft->SetData( 2, CVector2( 0, -l0 ), 32, CVector2( 0, l0 / 256 ), CVector2( 1, Min( 1.0f, Max( 0.0f, ( l - fLen ) / 32 ) ) ) );
	}
	pEft->SetTransformDirty();

	if( l - fLen >= m_fLen1 )
	{
		CEffectObject* pEffectObject = new CEffectObject( 1.0f, CVector2( 0, 0 ), 0 );
		pEffectObject->SetParentBeforeEntity( this );
		pEffectObject->SetPosition( GetPosition() );
		m_pEft->SetParentEntity( pEffectObject );
		m_pEft = NULL;
		pEft->GetInstanceData()->GetData().isEmitting = false;
		m_nState = 1;
		Kill();
	}
	else
	{
		int32 nFrame = Min<int32>( 7, floor( ( l - fLen ) * 8 / m_fLen1 ) );
		if( nFrame >= 0 )
		{
			auto pImage = static_cast<CImage2D*>( GetRenderObject() );
			pImage->bVisible = true;
			auto texRect = pImage->GetElem().texRect;
			texRect.y = nFrame / 8.0f;
			pImage->SetTexRect( texRect );
		}
	}
}

void CBloodDrop::Kill()
{
	if( m_nState >= 1 )
	{
		m_pKilled->SetPosition( globalTransform.GetPosition() );
		m_pKilled->y += m_fTargetY - y;
		m_pKilled->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
	CEnemy::Kill();
}
