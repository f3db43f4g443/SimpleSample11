#include "stdafx.h"
#include "Explosion.h"
#include "Stage.h"
#include "Character.h"
#include "Player.h"
#include "Enemy.h"

void CExplosion::OnAddedToStage()
{
	if( m_nLife )
		GetStage()->RegisterAfterHitTest( m_nLife, &m_onTick1 );
	uint32 nHitFrame = GetStage()->GetUpdatePhase() == eStageUpdatePhase_AfterHitTest ? m_nHitBeginFrame : m_nHitBeginFrame + 1;
	if( nHitFrame )
		GetStage()->RegisterAfterHitTest( nHitFrame, &m_onTick );
	else
	{
		ForceUpdateTransform();
		OnTick();
	}
	SetAutoUpdateAnim( true );
}

void CExplosion::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( m_onTick1.IsRegistered() )
		m_onTick1.Unregister();
	m_pCreator = NULL;
	m_hit.clear();
}

void CExplosion::OnTick()
{
	vector<CReference<CEntity> > hitResult;

	switch( m_nRangeType )
	{
	case 0:
	{
		SHitProxyCircle circle;
		circle.fRadius = m_fInitRange + m_nHitFrame * m_fDeltaRange;
		circle.center = CVector2( 0, 0 );
		GetStage()->MultiHitTest( &circle, globalTransform, hitResult );
		break;
	}
	default:
	{
		CRectangle rect;
		rect.x = -m_fInitRange - m_nHitFrame * m_fDeltaRange;
		rect.y = -m_fInitRange1 - m_nHitFrame * m_fDeltaRange1;
		rect.width = m_fInitRange2 + m_nHitFrame * m_fDeltaRange2 - rect.x;
		rect.height = m_fInitRange3 + m_nHitFrame * m_fDeltaRange3 - rect.y;
		SHitProxyPolygon polygon( rect );
		GetStage()->MultiHitTest( &polygon, globalTransform, hitResult );
		break;
	}
	}

	for( auto& pEntity : hitResult )
	{
		if( !pEntity->GetStage() )
			continue;
		if( m_hit.find( pEntity ) != m_hit.end() )
			continue;

		int32 nDmg = m_nDamage + m_nHitFrame * m_nDeltaDamage;
		nDmg = Max( nDmg, 0 );

		CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity.GetPtr() );
		if( pBlockObject )
		{
			auto pChunkObject = pBlockObject->GetBlock()->pOwner->pChunkObject;
			if( !m_bHitCreator && pChunkObject == m_pCreator )
				continue;
			if( pBlockObject->GetBlock()->eBlockType == eBlockType_Block )
			{
				if( !m_bHitBlock )
					continue;
			}
			else
			{
				if( !m_bHitWall )
					continue;
			}
			CVector2 hitDir = pChunkObject->globalTransform.GetPosition() - globalTransform.GetPosition();
			hitDir.Normalize();
			CChunkObject::SDamageContext dmgContext = { nDmg, 0, eDamageSourceType_Bullet, hitDir * 8 };
			pChunkObject->Damage( dmgContext );
			OnHit( pEntity );
			if( pEntity->GetStage() )
				m_hit.insert( pEntity );
			continue;
		}

		if( !m_bHitCreator && pEntity.GetPtr() == m_pCreator )
			continue;

		CPlayer* pPlayer = SafeCast<CPlayer>( pEntity.GetPtr() );
		if( pPlayer )
		{
			if( pPlayer->IsHiding() ? m_bHitHidingPlayer : m_bHitPlayer )
			{
				if( nDmg )
					pPlayer->Damage( nDmg );
				OnHit( pEntity );
				if( pEntity->GetStage() )
					m_hit.insert( pEntity );
			}
			continue;
		}

		CEnemy* pEnemy = SafeCast<CEnemy>( pEntity.GetPtr() );
		if( pEnemy )
		{
			if( pEnemy->IsHiding() ? m_bHitHidingEnemy : m_bHitEnemy )
			{
				if( nDmg )
					pEnemy->Damage( nDmg );
				OnHit( pEntity );
				if( pEntity->GetStage() )
					m_hit.insert( pEntity );
			}
			continue;
		}

		CCharacter* pCharacter = SafeCast<CCharacter>( pEntity.GetPtr() );
		if( pCharacter )
		{
			if( pCharacter->IsHiding() ? m_bHitHidingNeutral : m_bHitNeutral )
			{
				if( nDmg )
					pCharacter->Damage( nDmg );
				OnHit( pEntity );
				if( pEntity->GetStage() )
					m_hit.insert( pEntity );
			}
			continue;
		}
	}

	m_nHitFrame++;
	if( m_nHitFrame < m_nHitFrameCount )
		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}
