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
				pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
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
	if( m_pEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_pEffect->GetRoot()->CreateInstance() );
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
				StartScenario();
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
	m_pScenario = new CScenario();
	m_pScenario->SetParentEntity( this );
}

void CTutorialLevel::Scenario()
{
	CVector2 cam0 = CMyLevel::GetCamPos();
	CVector2 camTarget( 512, 0 );
	for( int i = 1; i <= 60; i++ )
	{
		float f = i / 60.0f;
		m_camPos = cam0 * ( 1 - f ) + camTarget * f;
		m_pScenario->Yield( 0, false );
	}

	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strSplash );
	CReference<CSplashRenderer> pRenderer = SafeCast<CSplashRenderer>( pPrefab->GetRoot()->CreateInstance() );
	pRenderer->SetZOrder( 10000 );
	pRenderer->SetParentEntity( this );

	float fMaxSpeed = 1200;
	float fSpeedPerTick = 10.0f;
	int32 nMaxTime = 60 * 100;
	int32 nHeightBegin = 480;
	int32 nHeightEnd = -640;

	int32 nScrollLen = 0;

	for( int i = 1; i <= nMaxTime; i++ )
	{
		int32 nFloodHeight = nHeightBegin + ( nHeightEnd - nHeightBegin ) * i / nMaxTime;
		int32 nCamHeight = 0;
		if( nFloodHeight < 0 )
		{
			nCamHeight = -nFloodHeight;
			nFloodHeight = 0;
		}

		float v = Min( fMaxSpeed, fSpeedPerTick * i );
		int32 d = v * GetStage()->GetElapsedTimePerTick();
		nScrollLen += d;

		pRenderer->GetSplash()->Set( nFloodHeight, -nScrollLen );
		m_camPos.y = nCamHeight;
		m_pScenario->Yield( 0, false );
	}

	CMainGameState::Inst().SetStageName( "" );
	CMainGameState::Inst().DelayResetStage();
}
