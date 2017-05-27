#include"stdafx.h"
#include "BlockItemsLv1.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Barrage.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Common/ResourceManager.h"
#include "Entities/Bullets.h"

void CPipe0::Trigger()
{
	SBarrageContext context;
	context.pCreator = GetParentEntity();
	context.vecBulletTypes.push_back( m_pPrefab );
	context.nBulletPageSize = 80;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [this]( CBarrage* pBarrage )
	{
		for( int i = 0; i < 20; i++ )
		{
			float fAngle = SRand::Inst().Rand( -0.2f, 0.2f );
			pBarrage->InitBullet( i * 4, -1, -1, CVector2( SRand::Inst().Rand( -12.0f, 12.0f ), 0 ), CVector2( 150 * sin( fAngle ), -150 * cos( fAngle ) ), CVector2( 0, 0 ), false, SRand::Inst().Rand( -PI, PI ), SRand::Inst().Rand( -6.0f, 6.0f ) );

			for( int j = 1; j <= 3; j++ )
				pBarrage->InitBullet( i * 4 + j, 0, i * 4, CVector2( SRand::Inst().Rand( -6.0f, 6.0f ), SRand::Inst().Rand( -6.0f, 6.0f ) ),
					CVector2( 0, 0 ), CVector2( 0, 0 ), true );

			pBarrage->Yield( 3 );
		}
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( globalTransform.GetPosition() );
	pBarrage->SetRotation( r );
	pBarrage->Start();
}

void CWindow::AIFunc()
{
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );
	if( !CMyLevel::GetInst() )
		return;

	while( 1 )
	{
		m_pAI->Yield( 0.5f, true );
		CRectangle rect;
		Get_HitProxy()->CalcBound( globalTransform, rect );
		if( CMyLevel::GetInst()->GetBound().GetBottom() > rect.GetBottom() )
			break;
	}

	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBullet1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );
	m_pHead[0] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead.c_str() );
	m_pHead[1] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead1.c_str() );
	m_pHead[2] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead2.c_str() );
	m_pHead[3] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead3.c_str() );
	CReference<CPrefab> pBullet1 = m_pBullet1;

	uint32 nAttackCount = 0;
	//alive
	while( 1 )
	{
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 1, 0 );
		m_pMan->bVisible = false;

		//closed
		while( 1 )
		{
			m_pAI->Yield( 0.5f, true );
			vector<CReference<CEntity> > hitEntities;
			GetStage()->MultiHitTest( Get_HitProxy(), globalTransform, hitEntities );
			bool bHit = false;
			for( CEntity* pEntity : hitEntities )
			{
				if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
				{
					bHit = true;
					break;
				}
			}
			if( bHit )
				continue;

			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
				if( m_openRect.Contains( pos ) )
					break;
			}

			auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
			if( pChunkObject && pChunkObject->GetMaxHp() && pChunkObject->GetHp() / pChunkObject->GetMaxHp() < 0.5f )
				break;
		}

		//opening
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlaySpeed( 1, false );
		m_pAI->Yield( 0.25f, false );
		m_pMan->bVisible = true;
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( 1, false );
		m_pAI->Yield( 0.75f, false );
		m_pMan->MoveToTopmost();

		//open
		while( 1 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				break;

			float fDeathChance = nAttackCount / ( nAttackCount + 1.0f );
			auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
			if( pChunkObject && pChunkObject->GetMaxHp() )
				fDeathChance = 1 - ( 1 - fDeathChance ) * ( pChunkObject->GetHp() / pChunkObject->GetMaxHp() );
			if( SRand::Inst().Rand( 0.0f, 1.0f ) < fDeathChance )
			{
				goto dead;
			}

			CVector2 playerPos = pPlayer->GetPosition();
			CVector2 dPos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
			if( !m_closeRect.Contains( dPos ) )
				break;

			//disappear
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 4, 8, 8 );
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( 1, false );
			m_pAI->Yield( 0.5f, false );
			m_pMan->bVisible = false;
			m_pAI->Yield( 0.5f, false );

			//attack
			nAttackCount++;
			for( int i = 0; i < 3; i++ )
			{
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( pPlayer )
					playerPos = pPlayer->GetPosition();
				dPos = playerPos - globalTransform.GetPosition();
				dPos = dPos + CVector2( SRand::Inst().Rand( -32.0f, 32.0f ), SRand::Inst().Rand( -32.0f, 32.0f ) );

				auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() );
				CVector2 acc = CVector2( 0, -100 );
				CVector2 velocity;

				float t = dPos.Length() / 250.0f * SRand::Inst().Rand( 0.9f, 1.1f ) + SRand::Inst().Rand( 0.1f, 0.4f );
				velocity.y = dPos.y / t - 0.5f * acc.y * t;
				velocity.x = dPos.x / t;

				pBullet->SetVelocity( velocity );
				pBullet->SetAcceleration( acc );
				pBullet->SetAngularVelocity( SRand::Inst().Rand( 4.0f, 8.0f ) * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 ) );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				pBullet->SetOnHit( [pBullet1]( CBullet* pThis, CEntity* pEntity )
				{
					SBarrageContext context;
					context.vecBulletTypes.push_back( pBullet1 );
					context.nBulletPageSize = 30;

					CBarrage* pBarrage = new CBarrage( context );
					CVector2 pos = pThis->globalTransform.GetPosition();
					pBarrage->AddFunc( []( CBarrage* pBarrage )
					{
						float fAngle0 = SRand::Inst().Rand( -PI, PI );
						for( int i = 0; i < 10; i++ )
						{
							for( int j = 0; j < 3; j++ )
							{
								float fAngle1 = fAngle0 + j * ( PI * 2 / 3 ) + SRand::Inst().Rand( -0.1f, 0.1f );
								float fSpeed = 200 - i * 3 + SRand::Inst().Rand( -20.0f, 20.0f );
								pBarrage->InitBullet( i * 3 + j, 0, -1, CVector2( 0, 0 ), CVector2( fSpeed * cos( fAngle1 ), fSpeed * sin( fAngle1 ) ), CVector2( 0, 0 ) );
							}
							pBarrage->Yield( 2 );
						}
						pBarrage->StopNewBullet();
					} );
					pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBarrage->SetPosition( pos );
					pBarrage->Start();
				} );

				m_pAI->Yield( 0.5f, false );
			}

			//reappear
			m_pMan->bVisible = true;
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 3, 7, 8 );
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlayPercent( 1 );
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( -1, false );
			m_pAI->Yield( 1.0f, false );
		}

		//closing
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.25f, false );
		m_pWindow->MoveToTopmost();
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.75f, false );
	}
	
	//dead
dead:
	if( m_pDeathEffect )
	{
		m_pDeathEffect->SetParentEntity( this );
		m_pDeathEffect->SetState( 2 );
	}
	if( m_pSpawner )
		m_pSpawner->SetEnabled( true );

	static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 8, 12, 2 );
	static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( 1, false );
	float fLevelHeight = CMyLevel::GetInst()->GetBoundWithLvBarrier().height;
	auto pHead = SafeCast<CEntity>( m_pHead[SRand::Inst().Rand( 0, 2 ) * 2 + ( globalTransform.GetPosition().y < fLevelHeight * 0.5f ? 0 : 1 )]->GetRoot()->CreateInstance() );
	pHead->SetPosition( globalTransform.GetPosition() );
	pHead->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
}
