#include "stdafx.h"
#include "SpecialLv1.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Bullets.h"
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
		pBullet->SetPosition( globalTransform.GetPosition() + CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
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
	pExplosion->SetPosition( globalTransform.GetPosition() + CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
	pExplosion->SetCreator( this );
	pExplosion->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}

void CGarbageBinGreen::Trigger()
{
	float fBaseAngle = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < m_nBulletCount; i++ )
	{
		auto pBullet = SafeCast<CBulletWithBlockBuff>( m_pPrefab->GetRoot()->CreateInstance() );
		pBullet->SetCreator( this );
		pBullet->SetPosition( globalTransform.GetPosition() + CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
		float fAngle = fBaseAngle + PI * 2 * i / m_nBulletCount;
		pBullet->SetRotation( fAngle );
		pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fMinSpeed, m_fMaxSpeed ) );
		pBullet->SetAcceleration( CVector2( 0, -m_fGravity ) );
		CBlockBuff::SContext context;
		context.nLife = m_nLife;
		context.nTotalLife = m_nLife;
		context.fParams[0] = m_fDamage;
		pBullet->Set( &context );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}

void CGarbageBinBlack::Trigger()
{
	float fBaseAngle = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < m_nCount; i++ )
	{
		auto pCharacter = SafeCast<CCharacter>( m_pPrefab->GetRoot()->CreateInstance() );
		float fAngle = fBaseAngle + PI * 2 * i / m_nCount;
		pCharacter->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fMinSpeed, m_fMaxSpeed ) );
		pCharacter->SetPosition( globalTransform.GetPosition() + CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
		pCharacter->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}
