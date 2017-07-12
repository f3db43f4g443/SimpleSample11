#include "stdafx.h"
#include "Tutorial.h"
#include "Stage.h"
#include "Player.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "GUI/Splash.h"
#include "GameState.h"
#include "LevelDesign.h"
#include "PlayerData.h"
#include "GUI/MainUI.h"
#include "MyGame.h"

void CTutorialStaticBeam::OnAddedToStage()
{
	CLightning::OnAddedToStage();
	Set( NULL, NULL, m_beginOfs, m_endOfs, -1, -1 );
}

void CTutorialScreen::OnAddedToStage()
{
	CChunkObject::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 0, 4, 4 );
	m_eft = CResourceManager::Inst()->CreateResource<CPrefab>( m_strEft.c_str() );
}

void CTutorialScreen::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CChunkObject::OnRemovedFromStage();
}

void CTutorialScreen::OnTick()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;

	CVector2 pos = pPlayer->GetPosition() / CMyLevel::GetBlockSize();

	switch( m_nState )
	{
	case 0:
		if( CRectangle( 6, 4, 4, 2 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 4, 8, 4 );
			m_nState = 1;
		}
		break;
	case 1:
		if( CRectangle( 1, 4, 3, 2 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 8, 12, 4 );
			m_nState = 2;
		}
		break;
	case 2:
		if( CRectangle( 1, 16, 2, 4 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 12, 16, 4 );
			m_nState = 3;
		}
		break;
	case 3:
		if( CRectangle( 25, 1, 6, 6 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 16, 17, 0 );
			m_nState = 4;
		}
		break;
	case 4:
		if( CRectangle( 8, 10, 16, 10 ).Contains( pos ) )
		{
			for( int i = 0; i < GetChunk()->nWidth * 4; i++ )
			{
				auto pEntity = SafeCast<CEntity>( m_eft->GetRoot()->CreateInstance() );
				pEntity->SetPosition( CVector2( x + ( i + 0.5f ) * 8, y + ( GetChunk()->nHeight - 0.5f ) * 32 ) );
				pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			}
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 17, 18, 0 );
			m_nState = 5;
		}
		break;
	case 5:
		if( pPlayer->GetWeapon() )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 18, 22, 8 );
			m_nState = 6;
		}
		break;
	case 6:
		if( CMyLevel::GetInst()->GetShakeStrength() >= 2.0f )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 22, 32, 10 );
			m_nState = 7;
		}
		break;
	case 7:
		if( CMyLevel::GetInst()->GetShakeStrength() <= 1.0f )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 22, 23, 0 );
			m_nState = 6;
		}
		break;
	}

	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CTutorialChest::OnAddedToStage()
{
	CChunkObject::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, 1, 0 );
	static_cast<CMultiFrameImage2D*>( m_e.GetPtr() )->SetFrames( 0, 1, 0 );
	m_pPickUp->SetParentEntity( NULL );
}

void CTutorialChest::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( m_onPickUp.IsRegistered() )
		m_onPickUp.Unregister();
	CChunkObject::OnRemovedFromStage();
}

void CTutorialChest::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}

void CTutorialChest::OnTick()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;

	if( m_nState == 0 )
	{
		CRectangle rect( m_pChunk->pos.x, m_pChunk->pos.y, m_pChunk->nWidth * CMyLevel::GetInst()->GetBlockSize(), m_pChunk->nHeight * CMyLevel::GetInst()->GetBlockSize() );
		if( rect.Contains( pPlayer->GetPosition() ) )
		{
			static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 1, 4, 4 );
			static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetPlaySpeed( 1, false );
			static_cast<CMultiFrameImage2D*>( m_e.GetPtr() )->SetFrames( 1, 4, 4 );
			static_cast<CMultiFrameImage2D*>( m_e.GetPtr() )->SetPlaySpeed( 1, false );
			m_nState = 1;
			GetStage()->RegisterAfterHitTest( 30, &m_onTick );
		}
		else
			GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	}
	else if( m_nState == 1 )
	{
		if( !m_pPickUp )
			return;
		if( !m_pPickUp->GetParentEntity() )
		{
			m_pPickUp->SetParentEntity( this );
			m_pPickUp->RegisterPickupEvent( &m_onPickUp );
		}
		else if( m_pPickUp->y < 120 )
			m_pPickUp->SetPosition( m_pPickUp->GetPosition() + CVector2( 0, 1 ) );

		m_pEft1->SetRotation( m_pEft1->r + 0.097f );
		m_pEft2->SetRotation( m_pEft2->r - 0.093f );

		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	}
}

void CTutorialEft::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	m_moveTarget = GetPosition();
	SetVelocity( CVector2( 0, SRand::Inst().Rand( -16.0f, -128.0f ) ) );
	m_nTex = SRand::Inst().Rand( 0, 4 ) * 4;

	m_flyData.bHitChannel[eEntityHitType_WorldStatic] = m_flyData.bHitChannel[eEntityHitType_Platform] = true;
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nTex, m_nTex + 1, 0 );
}

void CTutorialEft::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CCharacter::OnTickAfterHitTest();

	m_flyData.UpdateMove( this, m_moveTarget );
	if( !GetStage() )
		return;

	if( m_flyData.bHit && m_flyData.dVelocity.y > 1.0f )
	{
		SetParentEntity( NULL );
		return;
	}

	CVector2 vel = GetVelocity();
	if( m_nState == 0 )
	{
		if( vel.y >= -16 )
		{
			m_nState = SRand::Inst().Rand( 0, 2 ) * 2 - 1;
			m_moveTarget = GetPosition() + CVector2( m_nState * 100, SRand::Inst().Rand( -15.0f, -5.0f ) );
		}
	}
	switch( m_nState )
	{
	case 0:
		break;
	case 1:
		if( vel.x > 50 || m_flyData.bHit )
		{
			m_moveTarget = GetPosition() + CVector2( -200, SRand::Inst().Rand( -15.0f, -5.0f ) );
			m_nState = -1;
		}
		break;
	case -1:
		if( vel.x < -50 || m_flyData.bHit )
		{
			m_moveTarget = GetPosition() + CVector2( 200, SRand::Inst().Rand( -15.0f, -5.0f ) );
			m_nState = 1;
		}
		break;
	}

	if( abs( vel.x ) > 20 )
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nTex + 2, m_nTex + 3, 0 );
	else if( m_nState == 0 )
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nTex, m_nTex + 1, 0 );
	else if( m_moveTarget.x > x )
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nTex + 1, m_nTex + 2, 0 );
	else
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nTex + 3, m_nTex + 4, 0 );
}

void CTutorialLevel::StartUp()
{
	const char* szFileName = "data/tutorial/tutorial";
	if( !IsFileExist( szFileName ) )
		return;
	vector<char> content;
	uint32 nSize = GetFileContent( content, szFileName, false );
	CBufReader buf( &content[0], nSize );
	SLevelDesignContext context( 32, 128 );
	context.Init();
	context.Load( buf );
	context.GenerateLevel( this );

	auto pBlockLayer = m_basements[0].layers[0].Get_BlockLayer();
	while( pBlockLayer->NextBlockLayer() )
		pBlockLayer = pBlockLayer->NextBlockLayer();
	m_pLastChunk = pBlockLayer->pParent->pOwner;

	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CTutorialLevel::OnPlayerKilled( CPlayer * pPlayer )
{
	auto pBlockLayer = m_basements[0].layers[0].Get_BlockLayer();
	if( pBlockLayer )
	{
		auto pChunk = pBlockLayer->pParent->pOwner;
		if( pChunk->nWidth == 32 && pChunk->nHeight == 8 && pChunk->pos.y < CMyLevel::GetBlockSize() )
		{
			if( !m_pScenario )
			{
				m_nFloorCrushY = pChunk->pos.y;
				StartScenario();
			}
			return;
		}
	}

	CMyLevel::OnPlayerKilled( pPlayer );
}

CVector2 CTutorialLevel::GetCamPos()
{
	if( !m_pScenario )
		return CMyLevel::GetCamPos();
	return m_camPos;
}

void CTutorialLevel::StartScenario()
{
	CPlayerData::Inst().bFinishedTutorial = true;
	CPlayerData::Inst().Save();
	CMainUI::GetInst()->SetSkipVisible( true );
	m_pScenario = new CScenario();
	m_pScenario->SetParentEntity( this );
}

void CTutorialLevel::OnTick()
{
	int32 nHeight = m_pLastChunk->pos.y + m_pLastChunk->nHeight * CMyLevel::GetBlockSize() / 2;

	for( int i = 0; i < ELEM_COUNT( m_scroll ); i++ )
	{
		for( int j = 0; j < 2; j++ )
		{
			CEntity* pEntity = ( j == 0 ? m_scroll[i] : m_scroll1[i] );
			auto pImage2D = static_cast<CImage2D*>( pEntity->GetRenderObject() );
			CRectangle rect = pImage2D->GetElem().rect;
			rect.height = Max( 0.0f, Min( 1024.0f, nHeight - pEntity->GetPosition().y ) );
			pImage2D->SetRect( rect );
			CRectangle texRect = pImage2D->GetElem().texRect;
			texRect.height = rect.height / 1024;
			texRect.y = 1 - texRect.height;
			pImage2D->SetTexRect( texRect );
		}
	}

	if( !m_pScenario && nHeight < 24 * 32 )
		AddShakeStrength( 2.0f );

	GetStage()->RegisterAfterHitTest( 1, &m_onTick );

	if( m_pScenario && CGame::Inst().IsKeyUp( VK_BACK ) )
	{
		CMainGameState::Inst().SetStageName( "" );
		CMainGameState::Inst().DelayResetStage( 0 );
	}
}

void CTutorialLevel::Scenario()
{
	CVector2 cam0 = CMyLevel::GetCamPos();
	CVector2 camTarget( 512, 256 );
	for( int i = 1; i <= 60; i++ )
	{
		float f = i / 60.0f;
		m_camPos = cam0 * ( 1 - f ) + camTarget * f;
		m_pScenario->Yield( 0, false );
	}

	CReference<CDrawableGroup> pBarDrawable = static_cast<CDrawableGroup*>( m_pFloor->GetResource() );
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strSplash );
	CReference<CSplashRenderer> pRenderer = SafeCast<CSplashRenderer>( pPrefab->GetRoot()->CreateInstance() );
	pRenderer->SetZOrder( 10000 );
	pRenderer->SetParentEntity( this );

	CReference<CPrefab> pPrefabFloorEft = CResourceManager::Inst()->CreateResource<CPrefab>( m_strFloorEft );

	float fMaxSpeed = 512;
	float fSpeedPerTick = 1;
	int32 nMaxTime = 60 * 120;
	int32 nTime1 = 60 * 30;
	int32 nHeightBegin = 768;
	int32 nHeightEnd = -800;
	int32 nBarPos[] = { 0, 4, 8, 12, 16, 24, 28, 32, 36, 40, 44, 48, 52, 56 };

	float fScrollLen = 0;
	int32 iBar = 0;

	for( int i = 1; i <= nMaxTime; i++ )
	{
		int32 nFloodHeight = nHeightBegin + ( nHeightEnd - nHeightBegin ) * i / nMaxTime;
		int32 nCamHeight = 256;
		nFloodHeight += 256;
		if( nFloodHeight < 0 )
		{
			nCamHeight += -nFloodHeight;
			nFloodHeight = 0;
		}

		float v = Min( fMaxSpeed, fSpeedPerTick * i );
		fScrollLen += v * GetStage()->GetElapsedTimePerTick();
		m_fBaseShake = v * 2 / fMaxSpeed;
		int32 nScrollLen = floor( fScrollLen );

		float y0 = ( nScrollLen + 1536 - nCamHeight ) % 2048 - 1536 + nCamHeight;
		float y1 = y0 + 512 - nCamHeight > 0 ? y0 - 1024 : y0 + 1024;
		for( int j = 0; j < 3; j++ )
		{
			m_scroll[j]->SetPosition( CVector2( m_scroll[j]->x, y0 ) );
			m_scroll1[j]->SetPosition( CVector2( m_scroll1[j]->x, y1 ) );
		}

		if( iBar < ELEM_COUNT( nBarPos ) )
		{
			int32 nCurBarY = nScrollLen - nBarPos[iBar] * 1024;
			m_pFloor->SetPosition( CVector2( m_pFloor->x, nCurBarY ) );
			
			if( nCurBarY >= m_nFloorCrushY )
			{
				int32 breakPos[2][2];
				float breakVel[2][4];
				int32 centerLen0 = SRand::Inst().Rand( 256, 512 );
				int32 nReservedLen = ( 1024 - centerLen0 ) * 3 / 8;
				breakPos[0][0] = SRand::Inst().Rand( 0, 1024 - centerLen0 - nReservedLen * 2 + 1 ) + nReservedLen;
				breakPos[0][1] = breakPos[0][0] + centerLen0;

				breakPos[1][0] = breakPos[0][0] + SRand::Inst().Rand( -64, 65 );
				breakPos[1][1] = breakPos[0][1] + SRand::Inst().Rand( -64, 65 );

				for( int j = 0; j < 4; j++ )
				{
					breakVel[0][j] = SRand::Inst().Rand( -240.0f, -480.0f );
					breakVel[1][j] = breakVel[0][j] + SRand::Inst().Rand( -80.0f, -160.0f );
				}

				auto pFloorEft = SafeCast<CEffectObject>( pPrefabFloorEft->GetRoot()->CreateInstance() );
				pFloorEft->SetState( 2 );
				pFloorEft->SetPosition( CVector2( 0, -80 ) );
				pFloorEft->SetParentBeforeEntity( m_pFloor );

				for( int k = 0; k < 2; k++ )
				{
					for( int j = 0; j < 3; j++ )
					{
						float x0 = j == 0 ? 0 : breakPos[k][j - 1];
						float x1 = j == 2 ? 1024 : breakPos[k][j];
						float len = x1 - x0;
						auto pImage = static_cast<CImage2D*>( pBarDrawable->CreateInstance() );
						pImage->SetRect( CRectangle( -len / 2, -16, len, 32 ) );
						pImage->SetTexRect( CRectangle( x0 / 1024, 0, len / 1024, 1 ) );

						float vel0 = breakVel[k][j];
						float vel1 = breakVel[k][j + 1];
						float vel = ( vel0 + vel1 ) * 0.5f;
						float velA = ( vel1 - vel0 ) / len;

						CEffectObject* pEffectObject = new CEffectObject( 8, CVector2( 0, vel ), velA );
						pEffectObject->AddChild( pImage );
						pEffectObject->SetPosition( CVector2( ( x0 + x1 ) / 2, m_nFloorCrushY - 16 - k * 32 ) );
						pEffectObject->SetParentBeforeEntity( m_pFloor );
					}
				}

				m_nHitShakeFrame[0] = 0;
				m_hitShakeVec[0] = CVector2( 0, -20 - m_fBaseShake * 50 );

				iBar++;
				if( iBar < ELEM_COUNT( nBarPos ) )
				{
					int32 nCurBarY = nScrollLen - nBarPos[iBar] * 1024;
					m_pFloor->SetPosition( CVector2( m_pFloor->x, nCurBarY ) );
				}
				else
					m_pFloor->bVisible = false;
			}
		}

		pRenderer->GetSplash()->Set( nFloodHeight, -nScrollLen );
		camTarget.y = nCamHeight;
		m_camPos = camTarget + UpdateCamShake();

		if( !SRand::Inst().Rand( 0, 30 ) )
		{
			m_nHitShakeFrame[1] = 0;
			m_hitShakeVec[1] = CVector2( ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 ) * ( 4 + m_fBaseShake * 7 ), 0 );
		}

		if( i == nMaxTime - 120 )
		{
			CMainGameState::Inst().SetStageName( "" );
			CMainGameState::Inst().DelayResetStage( 2.0f );
		}
		m_pScenario->Yield( 0, false );
	}
}

CVector2 CTutorialLevel::UpdateCamShake()
{
	CVector2 shake( cos( IRenderSystem::Inst()->GetTotalTime() * 1.3592987 * 60 ), cos( IRenderSystem::Inst()->GetTotalTime() * 1.4112051 * 60 ) );
	shake = shake * m_fBaseShake;
	CVector2 shake1( 0, 0 );

	for( int i = 0; i < ELEM_COUNT( m_nHitShakeFrame ); i++ )
	{
		CVector2 vec = m_hitShakeVec[i] * ( m_nHitShakeFrame[i] % 4 < 2 ? 1 : -1 );
		if( i == 0 )
			shake1 = shake1 + vec;
		else
			shake = shake + vec;
		shake = shake + CVector2( m_hitShakeVec[i].y, -m_hitShakeVec[i].x ) * sin( m_nHitShakeFrame[i] * 1.58792 ) * 0.2f;
		m_nHitShakeFrame[i]++;

		float l = m_hitShakeVec[i].Normalize();
		m_hitShakeVec[i] = m_hitShakeVec[i] * Max( l - 8, 0.0f );
	}
	shake = CVector2( floor( shake.x + 0.5f ), floor( shake.y + 0.5f ) );
	shake1 = CVector2( floor( shake1.x * 0.03125f + 0.5f ), floor( shake1.y * 0.03125f + 0.5f ) ) * 32;
	return shake + shake1;
}
