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

void CWindow2::AIFunc()
{
	if( !CMyLevel::GetInst() )
		return;

	static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 1, 0 );
	m_pMan->bVisible = false;
	for( int i = 0; i < 2; i++ )
	{
		m_pEye[i]->bVisible = true;
		m_pHead[i]->bVisible = false;
		m_pLinks[i]->bVisible = false;
	}

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

			//attack
			nAttackCount++;
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
				playerPos = pPlayer->GetPosition();

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
					CVector2 vel0 = CVector2( cos( fBaseAngle ) * 150, sin( fBaseAngle ) * 50 );
					CVector2 vel = mat1.MulVector2Dir( vel0 );
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( m_pEye[0]->globalTransform.GetPosition() );
					pBullet->SetVelocity( vel + dPos1 * 100 );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().x, pBullet->GetVelocity().y ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					vel = mat2.MulVector2Dir( vel0 );
					pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( m_pEye[1]->globalTransform.GetPosition() );
					pBullet->SetVelocity( vel + dPos2 * 100 );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().x, pBullet->GetVelocity().y ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}
			}

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
	for( int i = 0; i < 2; i++ )
	{
		AIEye* pEye = new AIEye( i );
		m_pAIEye[i] = pEye;
		pEye->SetParentEntity( this );
	}
}

void CWindow2::AIFuncEye( uint8 nEye )
{
	m_pEye[nEye]->bVisible = false;
	m_pHead[nEye]->bVisible = true;
	m_pLinks[nEye]->bVisible = true;

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

void CWindow2::AIFuncEye1( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	float fForce = 1.0f;
	int32 nTickCount = SRand::Inst().Rand( 60, 90 );
	while( 1 )
	{
		if( GetStage()->GetPlayer() )
			target = GetStage()->GetPlayer()->GetPosition();
		CVector2 dPos = target - pHead->globalTransform.GetPosition();

		CVector2 force = target - dPos;
		float l = force.Normalize();
		force = force * Min( l * 10, 400.0f ) * fForce;
		CVector2 force1 = m_pEye[nEye]->GetPosition() - pHead->GetPosition();
		force1 = force1 * 0.75f;
		force = force + force1;
		UpdateLink( nEye );
		pAI->Yield( 0, false );

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
				for( int i = 0; i < 7; i++ )
				{
					float fAngle = ( i - 3 + SRand::Inst().Rand( 0.25f, 0.75f ) ) * 0.2f + atan2( dPos.y, dPos.x );
					float fSpeed = SRand::Inst().Rand( 150.0f, 200.0f );
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * fSpeed );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().x, pBullet->GetVelocity().y ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				fForce = 0;
			}
			else
			{
				float fBaseAngle = SRand::Inst().Rand( -PI, PI );
				for( int i = 0; i < 3; i++ )
				{
					float fAngle = fBaseAngle + i * PI * 2 / 3;
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 150.0f );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().x, pBullet->GetVelocity().y ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				fForce = SRand::Inst().Rand( 0.75f, 0.9f );
			}
			nTickCount = SRand::Inst().Rand( 60, 90 );
		}

		pAI->Yield( 0, true );
	}
}

void CWindow2::AIFuncEye2( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	float fForce = 1.0f;
	int32 nTickCount = SRand::Inst().Rand( 120, 180 );
	while( 1 )
	{
		while( 1 )
		{
			if( GetStage()->GetPlayer() )
				target = GetStage()->GetPlayer()->GetPosition();
			CVector2 dPos = target - pHead->globalTransform.GetPosition();

			CVector2 force = target - dPos;
			float l = force.Normalize();
			force = force * Min( l * 10, 80.0f ) * fForce;
			CVector2 force1 = m_pEye[nEye]->GetPosition() - pHead->GetPosition();
			force = force + force1 * fForce;
			fForce += ( SRand::Inst().Rand( -1.0f, 1.0f ) + ( 1 - fForce ) ) * GetStage()->GetElapsedTimePerTick();
			pAI->Yield( 0, false );

			pHead->SetPosition( pHead->GetPosition() + force * GetStage()->GetElapsedTimePerTick() );
			pHead->SetRotation( atan2( dPos.y, dPos.x ) );
			fForce = Min( 1.0f, fForce + GetStage()->GetElapsedTimePerTick() * 1.0f );
			nTickCount = Max( nTickCount - 1, 0 );

			UpdateLink( nEye );

			if( !nTickCount )
			{
				nTickCount = SRand::Inst().Rand( 120, 180 );
				break;
			}
			pAI->Yield( 0, true );
		}

		CVector2 pos0 = pHead->GetPosition();
		CVector2 dir = CVector2( cos( pHead->r ), sin( pHead->r ) );
		int i;
		for( i = 1; i <= 60; i++ )
		{
			float f = i / 60.0f;
			pHead->SetPosition( pos0 + dir * ( f * 800 - ( f * f ) * 200 ) );
			pAI->Yield( 0, true );


			pAI->Yield( 0, false );
		}
	}
}

void CWindow2::AIFuncEye3( uint8 nEye )
{
}

void CWindow2::UpdateLink( uint8 nEye )
{
	CVector2 eyePos = m_pEye[nEye]->GetPosition();
	CVector2 headPos = m_pHead[nEye]->GetPosition();
	int i = 0;
	for( auto pChild = m_pLinks[nEye]->Get_Child(); pChild; pChild = pChild->NextChild(), i++ )
	{
		float f = ( i + 0.5f ) / m_nLinkCount;
		pChild->SetPosition( eyePos * f + headPos * ( 1 - f ) );
	}
}
