#include "stdafx.h"
#include "Lv2Enemies.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Entities/Barrage.h"
#include "Entities/Bullets.h"
#include "Common/ResourceManager.h"
#include "Common/MathUtil.h"

bool CCar::CanHit( CEntity * pEntity )
{
	if( m_pExcludeChunkObject )
	{
		auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject && pBlockObject->GetParent() == m_pExcludeChunkObject )
		{
			auto pBlock = pBlockObject->GetBlock();
			if( pBlock->nX >= m_excludeRect.x && pBlock->nY >= m_excludeRect.y && pBlock->nX < m_excludeRect.GetRight() && pBlock->nY < m_excludeRect.GetBottom() )
			{
				m_bHitExcludeChunk = true;
				return false;
			}
		}
	}
	return true;
}

void CCar::OnRemovedFromStage()
{
	CEnemy::OnRemovedFromStage();
	m_pExcludeChunkObject = NULL;
}

void CCar::Damage( SDamageContext & context )
{
	DEFINE_TEMP_REF_THIS();
	CEnemy::Damage( context );
	if( GetStage() && m_nHp <= m_nBurnHp && !m_pBurnEffect )
	{
		m_pBurnEffect = SafeCast<CEntity>( m_strBurnEffect->GetRoot()->CreateInstance() );
		m_pBurnEffect->SetParentEntity( this );
		m_pBurnEffect->SetRenderParentAfter( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
}

void CCar::Kill()
{
	auto pExplosion = SafeCast<CExplosion>( m_strExplosion->GetRoot()->CreateInstance() );
	pExplosion->SetPosition( GetPosition() );
	pExplosion->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );

	switch( m_nExpType )
	{
	case 0:
	{
		float r = SRand::Inst().Rand( -PI, PI );
		for( int i = 0; i < 24; i++ )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
			float fAngle = i * PI / 12 + r;
			pBullet->SetPosition( GetPosition() );
			pBullet->SetRotation( fAngle );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * ( 150 + 50 * ( i & 1 ) ) );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
		break;
	}
	case 1:
	{
		float r = SRand::Inst().Rand( -PI, PI );
		for( int i = 0; i < 6; i++ )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
			float fAngle = i * PI / 3 + r;
			pBullet->SetPosition( GetPosition() );
			pBullet->SetRotation( fAngle );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 100 );
			pBullet->SetLife( 200 );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
		break;
	}
	case 2:
	{
		SBarrageContext context;
		context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
		context.nBulletPageSize = 50;

		CBarrage* pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [] ( CBarrage* pBarrage )
		{
			float r = SRand::Inst().Rand( -PI, PI );
			int32 nBullet = 0;
			for( int i = 0; i < 2; i++ )
			{
				for( int j = 0; j < 25; j++ )
				{
					float v = j / 5.0f;
					v = ( v - floor( v ) ) * 2 - 1;
					v = v * v * 125 + 125;
					float fAngle = r + j * PI / 12;
					CVector2 dir( cos( fAngle ), sin( fAngle ) );
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), dir * v, CVector2( dir.y, -dir.x ) * v * ( i * 2 - 1 ) );
				}
				pBarrage->Yield( 15 );
			}
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->SetPosition( GetPosition() );
		pBarrage->Start();
		break;
	}
	}

	CEnemy::Kill();
}

void CCar::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CEnemy::OnTickAfterHitTest();
	CVector2 dir( cos( r ), sin( r ) );
	SetVelocity( GetVelocity() + dir * ( m_fAcc * GetStage()->GetElapsedTimePerTick() ) );
	m_moveData.UpdateMove( this );
	if( !GetStage() )
		return;

	if( !m_bHitExcludeChunk )
		m_pExcludeChunkObject = NULL;
	m_bHitExcludeChunk = false;

	if( !m_pExcludeChunkObject && !m_bSpawned )
	{
		if( m_bDoorOpen )
		{
			if( m_nSpawnManTimeLeft )
			{
				m_nSpawnManTimeLeft--;
				float fOpenAngle[4] = { 2.0f, -2.0f, -2.0f, 2.0f };
				for( int i = 0; i < 4; i++ )
				{
					m_pDoors[i]->SetRotation( fOpenAngle[i] * ( m_nSpawnManTime - m_nSpawnManTimeLeft ) / m_nSpawnManTime );
				}
			}
			if( !m_nSpawnManTimeLeft )
			{
				int8 nValidDoor = 0;
				int8 validDoors[4];
				for( int i = 0; i < 4; i++ )
				{
					CVector2 spawnPos = globalTransform.MulVector2Pos( m_spawnManOfs[i] );
					SHitProxyCircle circle;
					circle.fRadius = m_spawnManRadius;
					circle.center = spawnPos;

					vector<CReference<CEntity> > hitEntities;
					GetStage()->MultiHitTest( &circle, CMatrix2D::GetIdentity(), hitEntities );
					bool bHit = false;
					for( CEntity* pEntity : hitEntities )
					{
						if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
						{
							bHit = true;
							break;
						}
					}
					if( bHit )
						continue;

					auto pEntity = SafeCast<CEntity>( m_strMan->GetRoot()->CreateInstance() );
					pEntity->SetPosition( spawnPos );
					pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				}

				m_bSpawned = true;
			}
		}
		else if( GetVelocity().Length2() < m_fSpawnManSpeed * m_fSpawnManSpeed || m_pBurnEffect )
		{
			m_bDoorOpen = true;
			m_nSpawnManTimeLeft = m_nSpawnManTime;
		}
	}

	int32 nDmg = ceil( m_moveData.fDamage * m_fHitDamage );
	if( m_pBurnEffect )
	{
		if( m_nBurnDamageCD )
			m_nBurnDamageCD--;
		if( !m_nBurnDamageCD )
		{
			m_nBurnDamageCD = m_nBurnDamageInterval;
			nDmg += m_nBurnDamage;
		}
	}

	if( nDmg )
	{
		SDamageContext context;
		context.nDamage = nDmg;
		context.nType = 0;
		context.nSourceType = 0;
		context.hitPos = CVector2( 0, 0 );
		context.hitDir = CVector2( 0, 0 );
		context.nHitType = -1;
		Damage( context );
	}
}

void CThrowBox::Kill()
{
	CVector2 pos = globalTransform.GetPosition();
	float fAngle = SRand::Inst().Rand( -PI, PI );
	CVector2 dir( cos( fAngle ), sin( fAngle ) );
	CVector2 dirs[4] = { { dir.x, dir.y }, { -dir.y, dir.x }, { -dir.x, -dir.y }, { dir.y, -dir.x } };
	for( int i = 0; i < 4; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
		pBullet->SetPosition( pos );
		pBullet->SetVelocity( dirs[i] * 150 );
		pBullet->SetLife( 60 );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}
	CEnemy::Kill();
}

void CWheel::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_nDir = SRand::Inst().Rand( -1, 2 );
}

bool CWheel::Knockback( const CVector2& vec )
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

void CWheel::Kill()
{
	for( int i = 0; i < 12; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
		pBullet->SetPosition( GetPosition() );
		pBullet->SetRotation( SRand::Inst().Rand( -PI, PI ) );
		pBullet->SetVelocity( CVector2( cos( pBullet->r ), sin( pBullet->r ) ) * SRand::Inst().Rand( 150.0f, 200.0f ) );
		pBullet->SetLife( SRand::Inst().Rand( 60, 90 ) );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}

	CEnemy::Kill();
}

void CWheel::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CVector2 lastPos = GetPosition();
	m_moveData.UpdateMove( this, m_nDir );
	if( !GetStage() )
		return;
	CVector2 dPos = GetPosition() - lastPos;
	float dRot = m_fRotSpeed * m_nDir;
	GetRenderObject()->SetRotation( GetRenderObject()->r + dRot );

	if( !m_nAIStepTimeLeft )
	{
		if( m_moveData.bHitSurface )
		{
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

	if( m_nAIStepTimeLeft )
		m_nAIStepTimeLeft--;
	if( m_nKnockBackTimeLeft )
		m_nKnockBackTimeLeft--;
	CEnemy::OnTickAfterHitTest();
}

void CSawBlade::OnKnockbackPlayer( const CVector2 & vec )
{
	CCharacter::SDamageContext context;
	memset( &context, 0, sizeof( context ) );
	context.nDamage = m_nDamage;
	GetStage()->GetPlayer()->Damage( context );
}

void CSawBlade::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.UpdateMove( this );
	if( !GetStage() )
		return;
	float dRot = m_fRotSpeed;
	GetRenderObject()->SetRotation( GetRenderObject()->r + dRot );

	CEnemy::OnTickAfterHitTest();
}

bool CGear::Knockback( const CVector2& vec )
{
	SetVelocity( vec * m_fMoveSpeed );
	m_nKnockBackTimeLeft = m_nKnockbackTime;
}

void CGear::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.UpdateMove( this );
	if( !GetStage() )
		return;

	if( m_velocity.Length2() == 0 )
	{
		for( int i = 0; i < 8; i++ )
		{
			auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
			pBullet->SetPosition( GetPosition() );
			pBullet->SetRotation( i * PI / 4 );
			pBullet->SetVelocity( CVector2( cos( pBullet->r ), sin( pBullet->r ) ) * 180.0f );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}

		float fAngle = SRand::Inst().Rand( -PI, PI );
		CVector2 dir( cos( fAngle ), sin( fAngle ) );
		dir = dir * SRand::Inst().Rand( 0.0f, 1024.0f );
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
		{
			CVector2 dPos = pPlayer->GetPosition() - GetPosition();
			dir = dir + dPos;
		}

		CVector2 dirs[8] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0.707f, 0.707f }, { -0.707f, 0.707f }, { 0.707f, -0.707f }, { -0.707f, -0.707f } };
		float fMaxDot = -1000000;
		uint32 maxi = 0;
		for( int i = 0; i < 8; i++ )
		{
			float fDot = dirs[i].Dot( dir );
			if( fDot > fMaxDot )
			{
				fMaxDot = fDot;
				maxi = i;
			}
		}

		SetVelocity( dirs[maxi] * m_fMoveSpeed );
		if( dir.x > 0 )
			m_nTargetDir = m_nTargetDir >= 3 ? 0 : m_nTargetDir + 1;
		else
			m_nTargetDir = m_nTargetDir <= 0 ? 3 : m_nTargetDir - 1;
	}

	float dAngle = NormalizeAngle( m_nTargetDir * PI * 0.5f - GetRotation() );
	float dAngle1 = m_fRotSpeed * GetStage()->GetElapsedTimePerTick();
	if( abs( dAngle ) <= dAngle1 )
		SetRotation( m_nTargetDir * PI * 0.5f );
	else if( dAngle > 0 )
		SetRotation( GetRotation() + dAngle1 );
	else
		SetRotation( GetRotation() - dAngle1 );
	
	if( m_nKnockBackTimeLeft )
		m_nKnockBackTimeLeft--;
	CEnemy::OnTickAfterHitTest();
}

void CExplosiveBall::OnAddedToStage()
{
	m_moveTarget = GetPosition();
	CEnemy::OnAddedToStage();
}

void CExplosiveBall::OnKnockbackPlayer( const CVector2 & vec )
{
	GetStage()->GetPlayer()->Knockback( vec * -2.5f );
}

void CExplosiveBall::Kill()
{
	SBarrageContext context;
	context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
	context.nBulletPageSize = 4 * 6;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [] ( CBarrage* pBarrage )
	{
		CVector2 ofs[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
		int32 nBullet = 0;
		for( int i = 0; i < 6; i++ )
		{
			for( int j = 0; j < 4; j++ )
			{
				pBarrage->InitBullet( i * 4 + j, 0, -1, ofs[j] * i * 60, ofs[j] * i * 240, CVector2( 0, 0 ) );
			}
			pBarrage->Yield( 15 );
		}
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( GetPosition() );
	pBarrage->Start();

	CEnemy::Kill();
}

void CExplosiveBall::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.UpdateMove( this, m_moveTarget );
	if( !GetStage() )
		return;

	if( !m_nAITickLeft )
	{
		float fAngle = SRand::Inst().Rand( -PI, PI );
		CVector2 dir( cos( fAngle ), sin( fAngle ) );
		m_moveTarget = m_moveTarget + dir * 32;
		m_nAITickLeft = m_nAITick;
	}

	if( m_nAITickLeft )
		m_nAITickLeft--;
	CEnemy::OnTickAfterHitTest();
}