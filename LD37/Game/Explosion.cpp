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
	if( m_pSound )
		m_pSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
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
	vector<CReference<CEntity> > result;
	vector<SHitTestResult> hitResult;

	switch( m_nRangeType )
	{
	case 0:
	{
		SHitProxyCircle circle;
		circle.fRadius = m_fInitRange + m_nHitFrame * m_fDeltaRange;
		circle.center = CVector2( 0, 0 );
		GetStage()->MultiHitTest( &circle, globalTransform, result, &hitResult );
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
		GetStage()->MultiHitTest( &polygon, globalTransform, result, &hitResult );
		break;
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

		CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
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

			int32 nDmg = m_nDamage + m_nHitFrame * m_nDeltaDamage;
			nDmg = Max( nDmg, 0 );
			if( nDmg )
			{
				CChunkObject::SDamageContext dmgContext = { nDmg, 0, eDamageSourceType_Bullet, hitDir * 8 };
				pChunkObject->Damage( dmgContext );
			}
			OnHit( pEntity );
			if( pEntity->GetStage() )
				m_hit[pEntity] = m_nHitInterval;
			continue;
		}

		if( !m_bHitCreator && pEntity == m_pCreator )
			continue;

		CPlayer* pPlayer = SafeCast<CPlayer>( pEntity );
		if( pPlayer )
		{
			if( pPlayer->IsHiding() ? m_bHitHidingPlayer : m_bHitPlayer )
			{
				int32 nDmg = m_nDamage1 + m_nHitFrame * m_nDeltaDamage1;
				nDmg = Max( nDmg, 0 );
				if( nDmg )
				{
					CCharacter::SDamageContext context;
					context.nDamage = nDmg;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = res.hitPoint1;
					context.hitDir = res.normal;
					context.nHitType = -1;
					pPlayer->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				OnHit( pEntity );
				if( pEntity->GetStage() )
					m_hit[pEntity] = m_nHitInterval;
			}
			continue;
		}

		CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
		if( pEnemy )
		{
			if( pEnemy->IsHiding() ? m_bHitHidingEnemy : m_bHitEnemy )
			{
				int32 nDmg = m_nDamage2 + m_nHitFrame * m_nDeltaDamage2;
				nDmg = Max( nDmg, 0 );
				if( nDmg )
				{
					CCharacter::SDamageContext context;
					context.nDamage = nDmg;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = res.hitPoint1;
					context.hitDir = res.normal;
					context.nHitType = -1;
					pEnemy->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				OnHit( pEntity );
				if( pEntity->GetStage() )
					m_hit[pEntity] = m_nHitInterval;
			}
			continue;
		}

		CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
		if( pCharacter )
		{
			if( pCharacter->IsHiding() ? m_bHitHidingNeutral : m_bHitNeutral )
			{
				int32 nDmg = m_nDamage2 + m_nHitFrame * m_nDeltaDamage2;
				nDmg = Max( nDmg, 0 );
				if( nDmg )
				{
					CCharacter::SDamageContext context;
					context.nDamage = nDmg;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = res.hitPoint1;
					context.hitDir = res.normal;
					context.nHitType = -1;
					pCharacter->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				OnHit( pEntity );
				if( pEntity->GetStage() )
					m_hit[pEntity] = m_nHitInterval;
			}
			continue;
		}
	}

	if( m_nHitInterval )
	{
		for( auto itr = m_hit.begin(); itr != m_hit.end(); )
		{
			itr->second;
			if( !itr->second )
				itr = m_hit.erase( itr );
			else
				itr++;
		}
	}

	m_nHitFrame++;
	if( m_nHitFrame >= m_nHitFrameCount )
	{
		if( !m_nHitInterval )
			return;
		m_nHitFrame = m_nHitFrameCount - 1;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}
