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
	auto localBound = Get_Child()->GetLocalBound();
	CVector2 center = localBound.GetCenter();
	center = Get_Child()->globalTransform.MulVector2Pos( center );
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

	CMyLevel::GetInst()->pExpSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
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
				CMyLevel::GetInst()->pExpSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
				auto localBound = Get_Child()->GetLocalBound();
				CVector2 center = CVector2( localBound.x + SRand::Inst().Rand( 0.0f, localBound.width ), localBound.y + SRand::Inst().Rand( 0.0f, localBound.height ) );
				center = Get_Child()->globalTransform.MulVector2Pos( center );
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
							Damage( m_nMaxHitDamage );
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
