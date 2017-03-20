#include "stdafx.h"
#include "Bullets.h"
#include "BlockBuffs.h"
#include "Common/ResourceManager.h"

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
