#include "stdafx.h"
#include "Weapons.h"
#include "ResourceManager.h"
#include "Bullet.h"
#include "Player.h"
#include "Stage.h"
#include "Render/Scene2DManager.h"
#include "MyLevel.h"
#include "Common/Rand.h"

CPlayerWeaponShoot0::CPlayerWeaponShoot0( const SClassCreateContext& context )
	: CPlayerWeapon( context )
	, m_bIsFiring( false )
	, m_nFireCD( 0 )
	, m_strBulletName( context )
{
	SET_BASEOBJECT_ID( CPlayerWeaponShoot0 );
}

void CPlayerWeaponShoot0::BeginFire( CPlayer* pPlayer )
{
	m_bIsFiring = true;
}

void CPlayerWeaponShoot0::EndFire( CPlayer* pPlayer )
{
	m_bIsFiring = false;
}

void CPlayerWeaponShoot0::Update( CPlayer* pPlayer )
{
	CVector2 dPos = pPlayer->GetAimAt() - pPlayer->GetPosition();
	if( dPos.Dot( dPos ) < 0.01f )
		dPos = CVector2( 0, 1 );
	SetRotation( atan2( dPos.y, dPos.x ) );

	if( m_bIsFiring && !m_nFireCD )
	{
		CScene2DManager::GetGlobalInst()->UpdateDirty();

		if( !m_nBulletCount )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_strBulletName->GetRoot()->CreateInstance() );
			pBullet->SetPosition( GetGlobalTransform().MulVector2Pos( m_fireOfs ) );
			CVector2 velocity = GetGlobalTransform().MulVector2Dir( CVector2( m_fSpeed, 0 ) );
			pBullet->SetVelocity( velocity );
			pBullet->SetAcceleration( CVector2( 0, -m_fGravity ) );
			pBullet->SetRotation( atan2( velocity.y, velocity.x ) );
			pBullet->SetAngularVelocity( m_fAngularSpeed * ( m_fAngularSpeed ? 1 : -1 ) );
			pBullet->SetLife( m_nBulletLife );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			pBullet->SetCreator( pPlayer->GetCurRoom() ? pPlayer->GetCurRoom() : (CEntity*)pPlayer );
			pBullet->SetDamage( m_nDamage, m_nDamage1, m_nDamage2 );
		}
		else
		{
			float fAngle0 = GetRotation();

			for( int i = 0; i < m_nBulletCount; i++ )
			{
				CBullet* pBullet = SafeCast<CBullet>( m_strBulletName->GetRoot()->CreateInstance() );

				float fAngle = fAngle0;
				switch( m_nDistribution )
				{
				case 0:
					fAngle = m_nBulletCount > 1 ? fAngle0 + ( i / ( m_nBulletCount - 1 ) - 0.5f ) * m_fAngle : fAngle0;
					break;
				case 1:
					fAngle = fAngle0 + ( ( i + SRand::Inst().Rand( 0.0f, 1.0f ) ) / m_nBulletCount - 0.5f ) * m_fAngle;
					break;
				case 2:
					fAngle = fAngle0 + SRand::Inst().Rand( -m_fAngle * 0.5f, m_fAngle * 0.5f );
					break;
				}

				pBullet->SetPosition( GetGlobalTransform().MulVector2Pos( m_fireOfs ) );
				CVector2 velocity = CVector2( cos( fAngle ), sin( fAngle ) ) * m_fSpeed;
				pBullet->SetVelocity( velocity );
				pBullet->SetAcceleration( CVector2( 0, -m_fGravity ) );
				pBullet->SetRotation( atan2( velocity.y, velocity.x ) );
				pBullet->SetAngularVelocity( m_fAngularSpeed * ( m_fAngularSpeed ? 1 : -1 ) );
				pBullet->SetLife( m_nBulletLife );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
				pBullet->SetCreator( pPlayer->GetCurRoom() ? pPlayer->GetCurRoom() : (CEntity*)pPlayer );
				pBullet->SetDamage( m_nDamage, m_nDamage1, m_nDamage2 );
			}
		}

		CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );
		CMyLevel::GetInst()->pFireSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );

		m_nFireCD = m_nFireRate;
	}
	if( m_nFireCD )
		m_nFireCD--;
}

CPlayerWeaponLaser0::CPlayerWeaponLaser0( const SClassCreateContext & context )
	: CPlayerWeapon( context )
	, m_bIsFiring( false )
	, m_nFireCD( 0 )
	, m_strBulletName( context )
{
	SET_BASEOBJECT_ID( CPlayerWeaponLaser0 );
}

void CPlayerWeaponLaser0::BeginFire( CPlayer * pPlayer )
{
	m_bIsFiring = true;
	pPlayer->SetAimSpeed( m_fAimSpeed );
}

void CPlayerWeaponLaser0::EndFire( CPlayer * pPlayer )
{
	if( m_pLaser )
	{
		m_pLaser->SetParentEntity( NULL );
		m_pLaser = NULL;
	}
	m_bIsFiring = false;
	m_nFireCD = m_nFireRate;
	pPlayer->SetAimSpeed( 0 );
}

void CPlayerWeaponLaser0::Update( CPlayer * pPlayer )
{
	CVector2 dPos = pPlayer->GetAimAt() - pPlayer->GetPosition();
	if( dPos.Dot( dPos ) < 0.01f )
		dPos = CVector2( 0, 1 );
	SetRotation( atan2( dPos.y, dPos.x ) );

	if( m_bIsFiring && !m_pLaser && !m_nFireCD )
	{
		m_pLaser = SafeCast<CLightning>( m_strBulletName->GetRoot()->CreateInstance() );

		m_pLaser->SetWidth( m_fWidth, m_fHitWidth );
		m_pLaser->SetDamage( m_nDamage );
		m_pLaser->SetDamage1( m_nDamage1 );
		m_pLaser->SetDamage2( m_nDamage2 );
		m_pLaser->SetHitCD( m_nHitCD );

		m_pLaser->SetParentEntity( this );
		m_pLaser->SetRenderParent( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}

	if( m_pLaser )
	{
		m_pLaser->SetCreator( pPlayer->GetCurRoom() ? pPlayer->GetCurRoom() : (CEntity*)pPlayer );
		CVector2 ofs = m_fireOfs;
		ofs.Normalize();
		float fLen = Max( m_fWidth, Min( m_fRange, pPlayer->GetAimAtOfs().Length() ) );
		ofs = ofs * fLen;

		m_pLaser->Set( NULL, NULL, m_fireOfs, m_fireOfs + ofs, -1, -1 );

		CMyLevel::GetInst()->AddShakeStrength( m_fShakePerSec * GetStage()->GetElapsedTimePerTick() );
	}
	else if( m_nFireCD )
		m_nFireCD--;
}
