#include "stdafx.h"
#include "Bullets.h"
#include "BlockBuffs.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"

void CBomb::OnAddedToStage()
{
	CBullet::OnAddedToStage();
	m_pExp->SetParentEntity( NULL );
}

void CBomb::OnHit( CEntity * pEntity )
{
	if( SafeCast<CBlockObject>( pEntity ) )
	{
		if( m_bExplodeOnHitBlock )
			Explode();
	}
	else if( SafeCast<CCharacter>( pEntity ) )
	{
		if( m_bExplodeOnHitChar )
			Explode();
	}
	else
	{
		if( m_bExplodeOnHitWorld )
			Explode();
	}
	m_pExp = NULL;

	CBullet::OnHit( pEntity );
}

void CBomb::Kill()
{
	if( m_bExplodeOnHitWorld )
		Explode();
	CBullet::Kill();
}

void CBomb::Explode()
{
	if( m_pExp )
	{
		auto pParent = SafeCast<CBarrage>( GetParentEntity() );
		if( pParent )
			m_pExp->SetPosition( globalTransform.GetPosition() );
		else
			m_pExp->SetPosition( GetPosition() );
		m_pExp->SetParentBeforeEntity( pParent ? (CEntity*)pParent : this );
		m_pExp = NULL;
	}
}

void CBulletWithBlockBuff::OnAddedToStage()
{
	CBullet::OnAddedToStage();
	m_pBlockBuff = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBlockBuff.c_str() );
}

void CBulletWithBlockBuff::OnHit( CEntity * pEntity )
{
	auto pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject && pBlockObject->GetStage() )
	{
		CBlockBuff::AddBuff( m_pBlockBuff, pBlockObject, &m_context );
	}
}

void CExplosionWithBlockBuff::OnAddedToStage()
{
	CExplosion::OnAddedToStage();
	m_pBlockBuff = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBlockBuff.c_str() );
}

void CExplosionWithBlockBuff::OnHit( CEntity * pEntity )
{
	auto pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject && pBlockObject->GetStage() )
	{
		CBlockBuff::AddBuff( m_pBlockBuff, pBlockObject, &m_context );
	}
}

void CExplosionKnockback::OnHit( CEntity * pEntity )
{
	CCharacter* pChar = SafeCast<CCharacter>( pEntity );
	if( pChar )
	{
		CVector2 dir = pChar->globalTransform.GetPosition() - globalTransform.GetPosition();
		dir.Normalize();
		pChar->Knockback( dir * m_fKnockbackStrength );
	}
	CExplosion::OnHit( pEntity );
}