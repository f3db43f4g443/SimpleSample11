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
	pHead->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
}

void CWindow2::OnAddedToStage()
{
	for( auto pChild = m_pLinks[0]->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		m_nLinkCount++;
	m_pAI = new AI();
	m_pAI->SetParentEntity( this );
}

void CWindow2::AIFunc()
{
	static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 1, 0 );
	m_pMan->bVisible = false;
	for( int i = 0; i < 2; i++ )
	{
		m_pEye[i]->bVisible = true;
		m_pHead[i]->bVisible = false;
		m_pLinks[i]->bVisible = false;
	}

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
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetPlaySpeed( 1, false );
		for( int i = 0; i < 2; i++ )
		{
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetFrames( 0, 4, 8 );
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetPlaySpeed( 1, false );
		}
		m_pAI->Yield( 0.75f, false );
		m_pMan->MoveToTopmost( true );

		//open
		while( 1 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				break;

			float fDeathChance = Max( 0.0f, nAttackCount - 10.0f ) / ( nAttackCount + 10.0f );
			auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
			if( pChunkObject && pChunkObject->GetMaxHp() )
				fDeathChance = 1 - ( 1 - fDeathChance ) * Max( pChunkObject->GetHp() / pChunkObject->GetMaxHp() * 2 - 1, 0.0f );
			if( SRand::Inst().Rand( 0.0f, 1.0f ) < fDeathChance )
			{
				goto dead;
			}

			CVector2 playerPos = pPlayer->GetPosition();
			CVector2 dPos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
			if( !m_closeRect.Contains( dPos ) )
				break;

			//attack
			nAttackCount++;

			{
				CVector2 dPos1 = playerPos - m_pEye[0]->globalTransform.GetPosition();
				CVector2 dPos2 = playerPos - m_pEye[1]->globalTransform.GetPosition();
				float fAngle1 = atan2( dPos1.y, dPos1.x );
				float fAngle2 = atan2( dPos2.y, dPos2.x );
				CMatrix2D mat1;
				mat1.Rotate( fAngle1 );
				CMatrix2D mat2;
				mat2.Rotate( fAngle2 );
				dPos1.Normalize();
				dPos2.Normalize();

				for( int i = 0; i < 8; i++ )
				{
					float fBaseAngle = i * PI / 4;
					CVector2 vel0 = CVector2( cos( fBaseAngle ) * 80, sin( fBaseAngle ) * 40 );
					CVector2 vel = mat1.MulVector2Dir( vel0 );
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( m_pEye[0]->globalTransform.GetPosition() );
					pBullet->SetVelocity( vel + dPos1 * 180 );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					vel = mat2.MulVector2Dir( vel0 );
					pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( m_pEye[1]->globalTransform.GetPosition() );
					pBullet->SetVelocity( vel + dPos2 * 180 );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				CVector2 shakeDir[2] = { CVector2( cos( fAngle1 ), sin( fAngle1 ) ), CVector2( cos( fAngle2 ), sin( fAngle2 ) ) };
				CVector2 basePos[2] = { m_pEye[0]->GetPosition(), m_pEye[1]->GetPosition() };
				for( int i = 30; i >= 0; i-- )
				{
					for( int k = 0; k < 2; k++ )
					{
						CVector2 ofs = shakeDir[k] * cos( ( 30 - i ) * 1.1243232f ) * ( i / 30.0f ) * 8;
						ofs = CVector2( floor( ofs.x + 0.5f ), floor( ofs.y + 0.5f ) );
						m_pEye[k]->SetPosition( basePos[k] + ofs );
					}
					m_pAI->Yield( 0, false );
				}
			}

			m_pAI->Yield( 1.2f, false );
		}

		//closing
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetPlaySpeed( -1, false );
		for( int i = 0; i < 2; i++ )
		{
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetFrames( 0, 4, 8 );
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetPlayPercent( 1 );
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetPlaySpeed( -1, false );
		}
		m_pAI->Yield( 0.25f, false );
		m_pWindow->MoveToTopmost( true );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.75f, false );
	}
	//dead
dead:
	for( int i = 0; i < 2; i++ )
	{
		AIEye* pEye = new AIEye( i );
		m_pAIEye[i] = pEye;
		pEye->SetParentEntity( this );
	}
}

void CWindow2::AIFuncEye( uint8 nEye )
{
	struct SEyeKilled
	{

	};
	try
	{
		CMessagePump pump( m_pAIEye[nEye] );
		m_pHead[nEye]->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SEyeKilled*>() );

		m_pEye[nEye]->bVisible = false;
		m_pHead[nEye]->bVisible = true;
		m_pLinks[nEye]->bVisible = true;
		m_pHead[nEye]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pLinks[nEye]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );

		switch( SRand::Inst().Rand( 0, 3 ) )
		{
		case 0:
			AIFuncEye1( nEye );
			break;
		case 1:
			AIFuncEye2( nEye );
			break;
		case 2:
			AIFuncEye3( nEye );
			break;
		}
	}
	catch( SEyeKilled* e )
	{
		m_pLinks[nEye]->bVisible = false;
	}
}

void CWindow2::AIFuncEye1( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	for( int k = 0; k < 2; k++ )
	{
		m_pEye[k]->ForceUpdateTransform();
		float fBaseAngle = SRand::Inst().Rand( -PI, PI );
		for( int i = 0; i < 8; i++ )
		{
			float fAngle = fBaseAngle + i * PI / 4;
			auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
			pBullet->SetPosition( m_pEye[k]->globalTransform.GetPosition() );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 200 );
			pBullet->SetRotation( fAngle );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
	}

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	float fForce = 1.0f;
	int32 nTickCount = SRand::Inst().Rand( 45, 60 );
	int32 nLastHp = pHead->GetHp();
	while( 1 )
	{
		if( GetStage()->GetPlayer() )
			target = GetStage()->GetPlayer()->GetPosition();
		CVector2 dPos = target - pHead->globalTransform.GetPosition();

		CVector2 force = dPos;
		float l = force.Normalize();
		force = force * Min( l * 10, 450.0f ) * fForce;
		CVector2 force1 = m_pEye[nEye]->GetPosition() - pHead->GetPosition();
		float l1 = force1.Normalize();
		force1 = force1 * ( 200 + l1 * 0.25f );
		force = force + force1;
		UpdateLink( nEye );
		pAI->Yield( 0, false );

		int32 nDeltaHp = nLastHp - pHead->GetHp();
		nLastHp = pHead->GetHp();
		fForce -= nDeltaHp / 25.0f;

		pHead->SetPosition( pHead->GetPosition() + force * GetStage()->GetElapsedTimePerTick() );
		pHead->SetRotation( atan2( dPos.y, dPos.x ) );
		fForce = Min( 1.0f, fForce + GetStage()->GetElapsedTimePerTick() * 1.0f );
		nTickCount = Max( nTickCount - 1, 0 );

		if( !nTickCount )
		{
			float fDist = dPos.Length();
			float fMinDist = 130;
			float fMaxDist = 250;
			float r = ( fDist - fMinDist ) / ( fMaxDist - fMinDist );
			if( SRand::Inst().Rand( 0.0f, 1.0f ) > r )
			{
				for( int i = 0; i < 9; i++ )
				{
					float fAngle = ( i - 3 + SRand::Inst().Rand( 0.25f, 0.75f ) ) * 0.18f + atan2( dPos.y, dPos.x );
					float fSpeed = SRand::Inst().Rand( 150.0f, 200.0f );
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * fSpeed );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				fForce = 0;
			}
			else
			{
				float fBaseAngle = SRand::Inst().Rand( -PI, PI );
				for( int i = 0; i < 6; i++ )
				{
					float fAngle = fBaseAngle + i * PI * 2 / 6;
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 150.0f );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				fForce = SRand::Inst().Rand( 0.4f, 0.6f );
			}
			nTickCount = SRand::Inst().Rand( 45, 60 );
		}

		pAI->Yield( 0, true );
	}
}

void CWindow2::AIFuncEye2( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	int32 nTickCount = SRand::Inst().Rand( 240, 300 );
	while( 1 )
	{
		while( 1 )
		{
			if( GetStage()->GetPlayer() )
				target = GetStage()->GetPlayer()->GetPosition();
			CVector2 dPos = target - pHead->globalTransform.GetPosition();

			CVector2 force = dPos;
			float l = force.Normalize();
			force = force * Min( l * 10, 80.0f ) * Min( nTickCount / 120.0f, 1.0f );
			CVector2 force1 = m_pEye[nEye]->GetPosition() - pHead->GetPosition();
			force = force + force1;
			pAI->Yield( 0, false );

			pHead->SetPosition( pHead->GetPosition() + force * GetStage()->GetElapsedTimePerTick() );
			pHead->SetRotation( atan2( dPos.y, dPos.x ) );
			nTickCount = Max( nTickCount - 1, 0 );

			UpdateLink( nEye );

			if( !nTickCount )
			{
				nTickCount = SRand::Inst().Rand( 240, 300 );
				break;
			}
			pAI->Yield( 0, true );
		}

		pHead->SetDefence( -4.0f );
		CVector2 pos0 = pHead->GetPosition();
		CVector2 dir = CVector2( cos( pHead->r ), sin( pHead->r ) );
		int i;
		bool bHit = true;
		for( i = 1; i <= 60; i++ )
		{
			pAI->Yield( 0, false );

			float f = i / 60.0f;
			pHead->SetPosition( pos0 + dir * ( f * 600 + ( f * f ) * 200 ) );
			UpdateLink( nEye );
			pAI->Yield( 0, true );

			bool bHit1 = false;
			bool bForceBreak = false;
			for( auto pManifold = pHead->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

				if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform )
				{
					auto pBlockObject = SafeCast<CBlockObject>( pEntity );
					if( pBlockObject && pBlockObject->GetBlock()->pOwner->bIsRoom )
					{
						bForceBreak = true;
						break;
					}
					bHit1 = true;
					break;
				}
			}
			if( bForceBreak )
				break;
			if( bHit1 )
			{
				if( !bHit )
					break;
			}
			else
				bHit = false;

			if( GetStage()->GetPlayer() )
				target = GetStage()->GetPlayer()->GetPosition();
			if( ( target - pHead->globalTransform.GetPosition() ).Dot( dir ) < 0 )
				break;
		}

		SBarrageContext context;
		context.vecBulletTypes.push_back( m_pBullet );
		context.nBulletPageSize = 100;

		pHead->ForceUpdateTransform();
		CBarrage* pBarrage = new CBarrage( context );
		CVector2 pos = pHead->globalTransform.GetPosition();
		float fAngle0 = pHead->r;
		pBarrage->AddFunc( [fAngle0] ( CBarrage* pBarrage )
		{
			CMatrix2D mat;
			mat.Rotate( fAngle0 );
			int32 nBullet = 0;
			for( int i = 0; i < 5; i++ )
			{
				for( int j = 0; j < 20; j++ )
				{
					float r = 150 + i * 25;
					CVector2 center( 50 + i * 15, 0 );
					CVector2 vel = center + CVector2( cos( ( j + i * 0.5f ) * PI / 10 ), sin( ( j + i * 0.5f ) * PI / 10 ) ) * r;
					vel = mat.MulVector2Dir( vel );
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), vel, CVector2( 0, 0 ) );
				}
				pBarrage->Yield( 3 );
			}
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->SetPosition( pos );
		pBarrage->Start();

		pHead->SetDefence( 0.0f );
		for( i--; i >= 0; i-- )
		{
			float f = i / 60.0f;
			pHead->SetPosition( pos0 + dir * ( f * 600 + ( f * f ) * 200 ) );
			UpdateLink( nEye );
			pAI->Yield( 0, false );
		}

		pAI->Yield( 0, true );
	}
}

void CWindow2::AIFuncEye3( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	int32 nTickCount = SRand::Inst().Rand( 400, 500 );
	uint8 nState = 0;
	float fLinear = 0;
	float fRadius = 0;
	float fFireCD = 200.0f;
	while( 1 )
	{
		pAI->Yield( 0, false );

		float fTargetLinear;
		float fTargetRadius;

		if( nTickCount )
			nTickCount--;
		switch( nState )
		{
		case 0:
			fTargetLinear = 200.0f;
			fTargetRadius = 100.0f;
			if( !nTickCount )
			{
				nTickCount = SRand::Inst().Rand( 300, 400 );
				pHead->SetDefence( -4.0f );
				nState = 1;
			}
			break;
		case 1:
			fTargetLinear = 600.0f;
			fTargetRadius = 250.0f;
			if( !nTickCount )
			{
				nTickCount = SRand::Inst().Rand( 600, 700 );
				pHead->SetDefence( 0.0f );
				nState = 0;
			}
			break;
		}

		if( fLinear < fTargetLinear )
			fLinear = Min( fLinear + GetStage()->GetElapsedTimePerTick() * ( 50.0f + fTargetLinear - fLinear ), fTargetLinear );
		else
			fLinear = Max( fLinear - GetStage()->GetElapsedTimePerTick() * 50.0f, fTargetLinear );
		if( fRadius < fTargetRadius )
			fRadius = Min( fRadius + GetStage()->GetElapsedTimePerTick() * 100.0f, fTargetRadius );
		else
			fRadius = Max( fRadius - GetStage()->GetElapsedTimePerTick() * 100.0f, fTargetRadius );

		pHead->SetRotation( pHead->r + ( nEye == 1 ? -fLinear / fRadius : fLinear / fRadius ) * GetStage()->GetElapsedTimePerTick() );
		pHead->SetPosition( CVector2( cos( pHead->r ), sin( pHead->r ) ) * fRadius );
		UpdateLink( nEye );

		fFireCD -= fLinear * GetStage()->GetElapsedTimePerTick();
		if( fFireCD <= 0 )
		{
			pAI->Yield( 0, true );
			if( nState == 0 )
			{
				auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
				pBullet->SetPosition( pHead->globalTransform.GetPosition() );
				pBullet->SetVelocity( CVector2( cos( pHead->r ), sin( pHead->r ) ) * 150.0f );
				pBullet->SetRotation( pHead->r );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				fFireCD = 30;
			}
			else
			{
				for( int i = 0; i < 5; i++ )
				{
					float fAngle = pHead->r + ( i - 2 ) * 0.45f;
					float fSpeed = 180 - abs( i - 2 ) * 15;
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * fSpeed );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}
				fFireCD = 120;
			}
		}
	}

}

void CWindow2::UpdateLink( uint8 nEye )
{
	CVector2 eyePos = m_pEye[nEye]->GetPosition();
	CVector2 headPos = m_pHead[nEye]->GetPosition();
	int i = 0;
	for( auto pChild = m_pLinks[nEye]->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild(), i++ )
	{
		float f = ( i + 0.5f ) / m_nLinkCount;
		pChild->SetPosition( eyePos * f + headPos * ( 1 - f ) );
	}
}
