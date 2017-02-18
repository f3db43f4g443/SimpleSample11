#include "stdafx.h"
#include "Weapons.h"
#include "ResourceManager.h"
#include "Bullet.h"
#include "Player.h"
#include "Stage.h"
#include "Render/Scene2DManager.h"
#include "MyLevel.h"

CPlayerWeaponShoot0::CPlayerWeaponShoot0( const SClassCreateContext& context )
	: CPlayerWeapon( context )
	, m_bIsFiring( false )
	, m_nFireCD( 0 )
	, m_strBulletName( context )
{
	SET_BASEOBJECT_ID( CPlayerWeaponShoot0 );
}

void CPlayerWeaponShoot0::OnAddedToStage()
{
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBulletName.c_str() );
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
		CPlayerBullet* pPlayerBullet = static_cast<CPlayerBullet*>( m_pBulletPrefab->GetRoot()->CreateInstance() );
		pPlayerBullet->SetPosition( GetGlobalTransform().MulVector2Pos( m_fireOfs ) );
		CVector2 velocity = GetGlobalTransform().MulVector2Dir( CVector2( m_fSpeed, 0 ) );
		pPlayerBullet->SetVelocity( velocity );
		pPlayerBullet->SetRotation( atan2( velocity.y, velocity.x ) );
		pPlayerBullet->SetLife( m_nBulletLife );
		pPlayerBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		pPlayerBullet->SetCreator( pPlayer->GetCurRoom() ? pPlayer->GetCurRoom() : (CEntity*)pPlayer );
		CMyLevel::GetInst()->AddShakeStrength( 5 );
		CMyLevel::GetInst()->pFireSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );

		m_nFireCD = m_nFireRate;
	}
	if( m_nFireCD )
		m_nFireCD--;
}