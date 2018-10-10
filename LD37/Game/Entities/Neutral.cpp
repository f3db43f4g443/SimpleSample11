#include "stdafx.h"
#include "Neutral.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "EffectObject.h"
#include "Stage.h"
#include "Player.h"
#include "Bullet.h"
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

void CFuelTank::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_pAwakeEffect = CResourceManager::Inst()->CreateResource<CPrefab>( m_strAwakeEffect.c_str() );
	m_pKillEffect = CResourceManager::Inst()->CreateResource<CPrefab>( m_strKillEffect.c_str() );
	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBullet1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );
	SetTransparent( true );
}

void CFuelTank::Awake()
{
	m_bAwake = true;
	m_nAwakeEffectCD = 0;
	m_velocity = CVector2( 0, 0 );
	m_fTotalMovedDist = 0;
}

void CFuelTank::Kill()
{
	auto localBound = Get_TransformChild()->GetLocalBound();
	CVector2 center = localBound.GetCenter();
	center = Get_TransformChild()->globalTransform.MulVector2Pos( center );
	if( m_pKillEffect )
	{
		auto pEffect = SafeCast<CEffectObject>( m_pKillEffect->GetRoot()->CreateInstance() );
		pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		pEffect->SetPosition( center );
		pEffect->SetState( 2 );
	}

	{
		SBarrageContext context;
		context.pCreator = this;
		context.vecBulletTypes.push_back( m_pBullet );
		context.vecBulletTypes.push_back( m_pBullet1 );
		context.nBulletPageSize = 13 * 6;

		CBarrage* pBarrage = new CBarrage( context );
		pBarrage->AddFunc( []( CBarrage* pBarrage )
		{
			for( int iFire = 0; iFire < 6; iFire++ )
			{
				int nBulletBegin = iFire * 13;
				float fDir0 = SRand::Inst().Rand<float>( PI * 0.25f, PI * 0.75f );
				pBarrage->InitBullet( nBulletBegin, 1, -1, CVector2( 0, 0 ), CVector2( cos( fDir0 ), sin( fDir0 ) ) * SRand::Inst().Rand( 300, 350 ), CVector2( 0, -400 ), false,
					0, 20, 0 );

				for( int i = 0; i < 12; i++ )
				{
					pBarrage->AddDelayAction( 90 + iFire * 2, [pBarrage, nBulletBegin]()
					{
						auto pBullet = pBarrage->GetBulletContext( nBulletBegin );
						pBullet->nNewBulletType = -1;
						pBullet->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
						pBullet->SetBulletMoveA( 0, 0, 0 );

						static CVector2 dir[] = { { 1.5f, 0.5f },{ 1.5f, 1.5f },{ 0.5f, 1.5f },{ -0.5f, 1.5f },{ -1.5f, 1.5f },{ -1.5f, 0.5f },{ -1.5f, -0.5f },{ -1.5f, -1.5f },{ -0.5f, -1.5f },{ 0.5f, -1.5f },{ 1.5f, -1.5f },{ 1.5f, -0.5f } };
						CVector2 baseVel( SRand::Inst().Rand( -50, 50 ), SRand::Inst().Rand( 100, 150 ) );
						for( int i = 0; i < 12; i++ )
						{
							pBarrage->InitBullet( nBulletBegin + i + 1, 0, nBulletBegin, CVector2( 0, 0 ), baseVel + dir[i] * 100, CVector2( 0, -300 ), false );
						}
					} );
				}
			}
			pBarrage->Yield( 110 );
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->SetPosition( center );
		pBarrage->Start();
	}

	CMyLevel::GetInst()->AddShakeStrength( 50 );
	CEnemy::Kill();
}

void CFuelTank::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();

	if( m_bAwake )
	{
		if( m_pAwakeEffect )
		{
			if( m_nAwakeEffectCD )
				m_nAwakeEffectCD--;
			if( !m_nAwakeEffectCD )
			{
				auto localBound = Get_TransformChild()->GetLocalBound();
				CVector2 center = CVector2( localBound.x + SRand::Inst().Rand( 0.0f, localBound.width ), localBound.y + SRand::Inst().Rand( 0.0f, localBound.height ) );
				center = Get_TransformChild()->globalTransform.MulVector2Pos( center );
				auto pEffect = SafeCast<CEffectObject>( m_pAwakeEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( center );
				pEffect->SetState( 2 );
				m_nAwakeEffectCD = m_nAwakeEffectInterval;
			}
		}

		CVector2 origPos = GetPosition();
		CVector2 dVel = globalTransform.MulVector2Dir( m_accleration ) * GetStage()->GetElapsedTimePerTick();
		CVector2 ofs = ( m_velocity + dVel * GetStage()->GetElapsedTimePerTick() * 0.5f ) * GetStage()->GetElapsedTimePerTick();
		m_velocity = m_velocity + dVel;
		CVector2 ofsNormalized = ofs;
		ofsNormalized.Normalize();

		vector<CReference<CEntity> > vecResult;
		vector<SRaycastResult> sweepResult;
		GetStage()->MultiSweepTest( Get_HitProxy(), globalTransform, ofs, vecResult, &sweepResult );
		for( auto& raycastResult : sweepResult )
		{
			auto pEntity = static_cast<CEntity*>( raycastResult.pHitProxy );
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
						m_velocity = CVector2( 0, 0 );
						SetPosition( origPos + ofsNormalized * raycastResult.fDist );

						CReference<CChunkObject> pChunkObject = pBlockObject->GetBlock()->pOwner->pChunkObject;
						pChunkObject->Damage( m_nMaxHitDamage, CChunkObject::eDamage_Crush );

						if( pChunkObject->GetChunk() )
						{
							Kill();
							return;
						}
						else
						{
							CReference<CEntity> temp = this;

							CCharacter::SDamageContext dmgContext;
							dmgContext.nDamage = m_nMaxHitDamage;
							dmgContext.nType = 0;
							dmgContext.nSourceType = 0;
							dmgContext.hitPos = dmgContext.hitDir = CVector2( 0, 0 );
							dmgContext.nHitType = -1;
							Damage( dmgContext );

							if( !GetParentEntity() )
								return;
						}
					}
				}
				else
				{
					Kill();
					return;
				}
			}
		}

		SetPosition( origPos + ofs );
		if( !CMyLevel::GetInst()->GetBound().Contains( globalTransform.GetPosition() ) )
		{
			Kill();
			return;
		}
		uint32 nPreFireCount = m_fTotalMovedDist / m_fFireMoveDist;
		m_fTotalMovedDist += ofs.Length();
		uint32 nCurFireCount = m_fTotalMovedDist / m_fFireMoveDist;
		CVector2 dir0 = m_accleration;
		dir0.Normalize();
		if( nCurFireCount > nPreFireCount )
		{
			for( int i = nPreFireCount; i < nCurFireCount; i++ )
			{
				CMyLevel::GetInst()->AddShakeStrength( 10 );
				CVector2 dir( SRand::Inst().Rand( -2.0f, 2.0f ), 1 );
				dir.Normalize();
				CVector2 dir1( dir.x, -dir.y );
				dir = CVector2( dir.x * dir0.x - dir.y * dir0.y, dir.x * dir0.y + dir.y * dir0.x );
				dir1 = CVector2( dir1.x * dir0.x - dir1.y * dir0.y, dir1.x * dir0.y + dir1.y * dir0.x );
				for( int i = 0; i < 4; i++ )
				{
					auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
					pBullet->SetPosition( globalTransform.MulVector2Pos( dir0 * ( i - 1.5f ) * 24 ) );
					pBullet->SetVelocity( globalTransform.MulVector2Dir( dir * 200 ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
					pBullet->SetPosition( globalTransform.MulVector2Pos( dir0 * ( i - 1.5f ) * 24 ) );
					pBullet->SetVelocity( globalTransform.MulVector2Dir( dir1 * 200 ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}
			}
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
}

void CBulletEnemy::Kill()
{
	if( m_bKilled )
		return;
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
	SetPosition( GetPosition() + ( vel + vel1 ) * ( GetStage()->GetElapsedTimePerTick() * 0.5f ) );
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
