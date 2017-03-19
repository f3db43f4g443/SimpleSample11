#include "stdafx.h"
#include "SpecialLv1.h"
#include "Stage.h"
#include "Player.h"
#include "Bullet.h"
#include "Explosion.h"
#include "MyLevel.h"
#include "Common/Rand.h"

void CGarbageBinRed::Trigger()
{
	float fBaseAngle = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < m_nBulletCount; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pPrefab->GetRoot()->CreateInstance() );
		pBullet->SetCreator( this );
		pBullet->SetPosition( globalTransform.GetPosition() );
		float fAngle = fBaseAngle + PI * 2 * i / m_nBulletCount;
		pBullet->SetRotation( fAngle );
		pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fMinSpeed, m_fMaxSpeed ) );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}

void CGarbageBinYellow::Trigger()
{
	auto pExplosion = SafeCast<CExplosion>( m_pPrefab->GetRoot()->CreateInstance() );
	pExplosion->SetPosition( globalTransform.GetPosition() );
	pExplosion->SetCreator( this );
	pExplosion->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}
