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
