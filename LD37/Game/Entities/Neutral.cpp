#include "stdafx.h"
#include "Neutral.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "EffectObject.h"
#include "Stage.h"
#include "Player.h"
#include "Bullet.h"
#include "Explosion.h"
#include "Barrage.h"
#include "EnemyCharacters.h"

void CSpike::OnAddedToStage()
{
	GetStage()->RegisterStageEvent( eStageEvent_PostHitTest, &m_onTick );
}

void CSpike::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CSpike::OnTick()
{
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pPlayer = SafeCast<CPlayer>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pPlayer )
		{
			pPlayer->Crush();
			continue;
		}

		auto pEnemyChar = SafeCast<CEnemyCharacter>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pEnemyChar )
		{
			pEnemyChar->Crush();
			continue;
		}
	}
}

bool CEnemyPhysics::Knockback( const CVector2 & vec )
{
	m_nKnockbackTimeLeft = m_nKnockbackTime;
	CVector2 vel = GetVelocity();
	CVector2 norm = vec;
	float l = norm.Normalize();
	vel = vel - norm * ( vel.Dot( norm ) * ( l + 1 ) );
	return true;
}

bool CEnemyPhysics::IsKnockback()
{
	return m_nKnockbackTimeLeft > 0;
}

void CEnemyPhysics::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	if( m_nLife )
	{
		m_nLife--;
		if( m_fKillVel > 0 && GetVelocity().Length2() < m_fKillVel * m_fKillVel )
			m_nLife /= 2;
		if( !m_nLife )
		{
			Kill();
			return;
		}
	}

	if( m_nKnockbackTimeLeft )
		m_nKnockbackTimeLeft--;
	m_moveData.UpdateMove( this );
}

void CBulletEnemy::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	SetRotation( atan2( GetVelocity().y, GetVelocity().x ) );
	if( m_pExp )
		m_pExp->SetParentEntity( NULL );
}

void CBulletEnemy::Kill()
{
	if( m_bKilled )
		return;

	if( m_pExp )
	{
		m_pExp->SetPosition( globalTransform.MulVector2Pos( m_pExp->GetPosition() ) );
		m_pExp->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		m_pExp = NULL;
	}

	if( m_pParticle )
		static_cast<CParticleSystemObject*>( m_pParticle.GetPtr() )->GetInstanceData()->GetData().isEmitting = false;
	if( m_fDeathTime < 0 )
	{
		CEnemy::Kill();
		return;
	}
	m_bKilled = true;
	SetRenderObject( NULL );
	SetTransparentRec( true );
	KillEffect();
}

void CBulletEnemy::OnTickBeforeHitTest()
{
	CVector2 vel = GetVelocity();
	CVector2 vel1 = vel + m_a * GetStage()->GetElapsedTimePerTick();
	CVector2 ofs( ( vel + vel1 ) * ( GetStage()->GetElapsedTimePerTick() * 0.5f ) );
	if( m_nMoveType == 1 )
	{
		vector<CReference<CEntity> > vecResult;
		vector<SRaycastResult> sweepResult;
		GetStage()->MultiSweepTest( Get_HitProxy(), globalTransform, ofs, vecResult, &sweepResult );
		for( auto& raycastResult : sweepResult )
		{
			auto pEntity = static_cast<CEntity*>( raycastResult.pHitProxy );
			if( pEntity == this )
				continue;
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform )
			{
				auto pBlockObject = SafeCast<CBlockObject>( pEntity );
				if( pBlockObject )
				{
					if( raycastResult.fDist == 0 )
					{
						pBlockObject->GetBlock()->pOwner->bForceStop = true;
					}
					else
					{
						Kill();
						return;
					}
				}
				else
				{
					Kill();
					return;
				}
			}
		}
	}
	SetPosition( GetPosition() + ofs );
	SetRotation( atan2( vel1.y, vel1.x ) );
	SetVelocity( vel1 );
	CEnemy::OnTickBeforeHitTest();
}

void CBulletEnemy::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	if( m_bKilled )
	{
		m_fDeathTime -= GetStage()->GetElapsedTimePerTick();
		if( m_fDeathTime <= 0 )
			SetParentEntity( NULL );
		return;
	}
	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
		{
			Kill();
			return;
		}
	}

	if( m_nMoveType == 0 )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform || pEntity->GetHitType() == eEntityHitType_System )
			{
				auto pBlockObject = SafeCast<CBlockObject>( pEntity );
				if( pBlockObject && pBlockObject->GetBlock()->eBlockType != eBlockType_Block )
					continue;
				Kill();
				return;
			}
		}
	}
}

void CPickupCarrier::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	if( m_pPickup )
		m_pPickup->RegisterPickupEvent( &m_onPickUp );
}

void CPickupCarrier::OnRemovedFromStage()
{
	if( m_onPickUp.IsRegistered() )
		m_onPickUp.Unregister();
	CCharacter::OnRemovedFromStage();
}

void CPickUpCarrierPhysics::OnAddedToStage()
{
	CPickupCarrier::OnAddedToStage();
	if( m_bIgnoreHit )
		memset( m_moveData.bHitChannel, 0, sizeof( m_flyData.bHitChannel ) );
	if( m_nPickUpTime )
		m_pPickup->SetPickUpEnabled( false );
}

void CPickUpCarrierPhysics::OnTickAfterHitTest()
{
	if( m_nPickUpTime )
	{
		m_nPickUpTime--;
		if( !m_nPickUpTime )
			m_pPickup->SetPickUpEnabled( true );
	}
	if( !m_bAttracted )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
			m_bAttracted = ( GetPosition() - pPlayer->GetPosition() ).Length2() < m_fAttractDist * m_fAttractDist;
	}

	CPickupCarrier::OnTickAfterHitTest();
	if( m_bAttracted )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		m_flyData.UpdateMove( this, pPlayer ? pPlayer->GetPosition() : GetPosition() );
	}
	else
	{
		if( m_nLife )
		{
			m_nLife--;
			if( !m_nLife )
			{
				Kill();
				return;
			}
		}

		m_moveData.UpdateMove( this ); 
	}
}

void CBonusStageReward::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CBonusStageReward::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	for( auto& trigger : m_triggers )
	{
		if( trigger.IsRegistered() )
			trigger.Unregister();
	}
	m_triggers.clear();
}

void CBonusStageReward::Set( SItemDropContext & dropResult, uint32 nReward )
{
	m_r = PI * ( 1.5f - 1.0f / dropResult.nDrop );
	m_lCur = 0;
	for( int i = 0; i < dropResult.nDrop; i++ )
	{
		auto pPickUp = SafeCast<CPickUpTemplate>( m_pPrefab->GetRoot()->CreateInstance() );
		pPickUp->Set( SafeCast<CEntity>( dropResult.dropItems[i].pPrefab->GetRoot()->CreateInstance() ), 0 );
		pPickUp->SetParentEntity( this );
		float r = m_r + i * PI * 2 / dropResult.nDrop;
		m_pickups[i + 1] = pPickUp;

		auto pImg = static_cast<CImage2D*>( m_pLinkDrawable->CreateInstance() );
		auto rect = pImg->GetElem().rect;
		rect.x = 0;
		rect.width = 0;
		pImg->SetRect( rect );
		pImg->SetRotation( r );
		pImg->SetZOrder( -1 );
		AddChild( pImg );
		m_pLinkImgs[i] = pImg;
	}

	m_triggers.resize( dropResult.nDrop + 1 );
	for( int i = 0; i <= dropResult.nDrop; i++ )
	{
		m_triggers[i].Set( [i, this] () {
			m_pickups[i] = NULL;
			OnPickUp();
		} );
		m_pickups[i]->RegisterPickupEvent( &m_triggers[i] );
	}

	for( int i = 0; i < 3; i++ )
		m_nRewards[i] = ( nReward >> ( i * 2 ) ) & 3;
	m_nRewards[3] = nReward >> 6;

	int32 nCount = log2( nReward + 8 ) * 5;
	for( int i = 0; i < 4; i++ )
		nCount -= m_nRewards[i];
	int8 index[3] = { 1, 2, 3 };
	while( nCount > 0 )
	{
		int8 iIndex;
		SRand::Inst().Shuffle( index, ELEM_COUNT( index ) );
		for( iIndex = 0; iIndex < ELEM_COUNT( index ); iIndex++ )
		{
			if( m_nRewards[index[iIndex]] )
				break;
		}
		if( iIndex >= ELEM_COUNT( index ) )
			break;
		int32 i = index[iIndex];
		m_nRewards[i]--;
		m_nRewards[i - 1] += 4;
		nCount -= 3;
	}
}

void CBonusStageReward::OnTick()
{
	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
		{
			OnPickUp();
			return;
		}
	}

	for( int i = 0; i < 4; i++ )
	{
		if( m_nRewards[i] )
		{
			auto pReward = SafeCast<CCharacter>( ( SRand::Inst().Rand( 0, 2 ) ? m_pRestorePrefab[i] : m_pMoneyPrefab[i] )->GetRoot()->CreateInstance() );
			float fDir = SRand::Inst().Rand( -PI, PI );
			pReward->SetPosition( globalTransform.GetPosition() );
			pReward->SetVelocity( CVector2( cos( fDir ), sin( fDir ) ) * m_fRewardSpeed );
			pReward->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			m_nRewards[i]--;
			break;
		}
	}

	m_r += m_fAngularSpeed * ( m_lCur / m_l ) * GetStage()->GetElapsedTimePerTick();
	m_lCur = Min( m_l, m_lCur + m_dl * GetStage()->GetElapsedTimePerTick() );
	int32 n = m_triggers.size() - 1;
	for( int i = 0; i < n; i++ )
	{
		float r = m_r + i * PI * 2 / n;
		m_pickups[i + 1]->SetPosition( CVector2( cos( r ), sin( r ) ) * m_lCur );
		m_pLinkImgs[i]->SetRotation( r );

		auto pImg = static_cast<CImage2D*>( m_pLinkImgs[i].GetPtr() );
		auto rect = pImg->GetElem().rect;
		rect.width = m_lCur;
		pImg->SetRect( rect );
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CBonusStageReward::OnPickUp()
{
	for( int i = 0; i < m_triggers.size(); i++ )
	{
		auto& trigger = m_triggers[i];
		if( trigger.IsRegistered() )
			trigger.Unregister();
		auto& pPickUp = m_pickups[i];
		if( pPickUp )
		{
			pPickUp->Kill();
			pPickUp = NULL;
		}
	}
	for( int i = 0; i < ELEM_COUNT( m_pLinkImgs ); i++ )
	{
		if( !m_pLinkImgs[i] )
			break;
		m_pLinkImgs[i]->RemoveThis();
		m_pLinkImgs[i] = NULL;
	}
	SetParentEntity( NULL );
}
