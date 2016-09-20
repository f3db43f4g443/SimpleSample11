#include "stdafx.h"
#include "TutorialGame0.h"
#include "Render/Scene2DManager.h"
#include "Common/ResourceManager.h"
#include "Entities/GlitchEffect.h"
#include "Entities/Barrage.h"
#include "Render/ParticleSystem.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "Common/MathUtil.h"

void CTutorialGame0Enemy::OnAddedToStage()
{
	m_pHREffectPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHREffect.c_str() );
	if( !m_pAI )
	{
		m_pAI = new AI;
		m_pAI->SetParentEntity( this );
	}
}

void CTutorialGame0Enemy::AIFunc()
{
	CGlitchEffect* pEffect = dynamic_cast<CEntity*>( GetRenderObject() )->GetChildByName<CGlitchEffect>( "glitch" );
	CEntity* pParticle = dynamic_cast<CEntity*>( GetRenderObject() )->GetChildByName<CEntity>( "particles" );
	CEntity* pHit = dynamic_cast<CEntity*>( GetRenderObject() )->GetChildByName<CEntity>( "hit" );
	TVector4<uint32> p0( 1, 1, 0, 0 );
	TVector4<uint32> p1( 1, 1, 1, 0 );
	TVector4<uint32> p2( 0, 1, 0, 0 );
	TVector4<uint32> p3( 0, 0, 1, 0 );
	pEffect->SetP( &p0.x );
	pEffect->SetMaxOfs( 1, 0.02f );
	for( auto pChild = pParticle->Get_Child(); pChild; pChild = pChild->NextChild() )
	{
		auto pParticle = dynamic_cast<CParticleSystemObject*>( pChild );
		if( pParticle )
			pParticle->GetInstanceData()->GetData().isEmitting = false;
	}
	pHit->bVisible = false;
	m_pAI->Yield( 0, false );

	do
	{
		float fTime = 0;
		do
		{
			fTime += GetStage()->GetGlobalElapsedTime();

			if( fTime >= 10 )
				pEffect->SetP( &p3.x );
			else if( fTime >= 8 )
			{
				pEffect->SetMaxOfs( 1, 0.02f );
				auto p = p3 * ( 100 * ( fTime - 8 ) ) + p2 * ( 100 * ( 10 - fTime ) );
				pEffect->SetP( &p.x );
			}
			else if( fTime >= 6 )
			{
				pEffect->SetMaxOfs( 1, 0 );
				pEffect->SetP( &p2.x );
			}
			else if( fTime >= 4 )
			{
				auto p = p2 * ( 100 * ( fTime - 4 ) ) + p1 * ( 100 * ( 6 - fTime ) );
				pEffect->SetP( &p.x );
			}
			else if( fTime >= 2 )
			{
				auto p = p1 * ( 100 * ( fTime - 2 ) ) + p0 * ( 100 * ( 4 - fTime ) );
				pEffect->SetP( &p.x );
			}

			if( fTime >= 12 )
			{
				break;
			}
			m_pAI->Yield( 0, false );
		} while( true );

		for( auto pChild = pParticle->Get_Child(); pChild; pChild = pChild->NextChild() )
		{
			auto pParticle = dynamic_cast<CParticleSystemObject*>( pChild );
			if( pParticle )
				pParticle->GetInstanceData()->GetData().isEmitting = true;
		}

		CPlayer* pPlayer;
		do
		{
			m_pAI->Yield( 0, false );

			fTime = GetStage()->GetGlobalElapsedTime();
			pPlayer = GetStage()->GetPlayer();
			if( !CommonMove( 250, fTime, pPlayer->GetPosition() - GetPosition(), 0 ) )
				break;
		} while( true );
	
		for( auto pChild = pParticle->Get_Child(); pChild; pChild = pChild->NextChild() )
		{
			auto pParticleChild = dynamic_cast<CParticleSystemObject*>( pChild );
			if( pParticleChild )
				pParticleChild->GetInstanceData()->GetData().isEmitting = false;
		}

		OnHitPlayer();
		m_pAI->Yield( 5, false );

		SetPosition( CVector2( SRand::Inst().Rand( -384.0f, 384.0f ), SRand::Inst().Rand( -384.0f, 384.0f ) ) );
	} while( true );
}

void CTutorialGame0Enemy::OnHitPlayer()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer->IsInHorrorReflex() )
	{
		dynamic_cast<CTutorialGame0*>( m_pGame )->Damage( 1, 0, 0 );
	}
	else
	{
		TTempEntityHolder<CGlitchEffect> pEffect = dynamic_cast<CGlitchEffect*>( m_pHREffectPrefab->GetRoot()->CreateInstance() );
		pEffect->SetParentEntity( m_pGame->GetGUIRoot() );
		TVector4<uint32> p0[] = { { 0, 0, 1, 0 }, { 1, 0, 0, 0 } };
		TVector4<uint32> p1[] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 } };
		pEffect->SetP( &p0[0].x );

		SBarrageContext context;
		context.vecObjects.push_back( this );
		context.vecObjects.push_back( pPlayer );
		context.vecBulletTypes.push_back( dynamic_cast<CTutorialGame0*>( m_pGame )->pBulletSmallPrefab );
		context.vecBulletTypes.push_back( dynamic_cast<CTutorialGame0*>( m_pGame )->pBulletBigPrefab );
		context.nBulletPageSize = 32 * 3 * 8;
		TTempEntityHolder<CBarrage> pBarrage( new CBarrage( context ) );
		pBarrage->AddFunc( [] ( CBarrage* pBarrage )
		{
			uint32 nBullet = 0;

			float fAngle = SRand::Inst().Rand( -PI, PI );
			while( true )
			{
				float dAngle0 = SRand::Inst().Rand<float>( 0, PI / 4 );
				float dAngle[16];
				for( int i = 0; i < 8; i++ )
				{
					dAngle[i] = dAngle0;
					dAngle[i + 8] = dAngle0 - PI / 4;
				}

				for( int iWave = 0; iWave < 18; iWave++ )
				{
					int32 nBulletBegin = nBullet;

					float fPercent = Min( 1.0f, Max( 0.0f, ( iWave - 6 ) / 6.0f ) );
					float fPercent1 = Min( 1.0f, Max( 0.0f, ( iWave - 12 ) / 6.0f ) );
					fPercent -= fPercent1;
					
					float l = sin( ( pBarrage->GetCurFrame() % 240 ) * PI * 2 / 240 ) * 50.0f;
					for( int i = 0; i < 8; i++ )
					{
						pBarrage->InitBullet( nBullet, -1, -1, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false, i * PI / 4 + fAngle + dAngle[i] * fPercent1, dAngle[i] * fPercent, 0 );
						pBarrage->InitBullet( nBullet + 1, 0, nBullet, CVector2( 0, l ), CVector2( 100, 0 ), CVector2( 0, 0 ), false, 0, 0, 0 );

						pBarrage->InitBullet( nBullet + 2, -1, -1, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false, i * PI / 4 + fAngle + dAngle[i + 8] * fPercent1, dAngle[i + 8] * fPercent, 0 );
						pBarrage->InitBullet( nBullet + 3, 0, nBullet + 2, CVector2( 0, -l ), CVector2( 100, 0 ), CVector2( 0, 0 ), false, 0, 0, 0 );
						nBullet += 4;
					}

					pBarrage->AddDelayAction( 60, [=] () {
						for( int i = 0; i < 16; i++ )
						{
							pBarrage->GetBulletContext( i * 2 + nBulletBegin )->SetBulletMoveA( 0, 0 );
						}
					} );

					pBarrage->AddDelayAction( 360, [=] () {
						for( int i = 0; i < 32; i++ )
						{
							pBarrage->DestroyBullet( i + nBulletBegin );
						}
					} );

					pBarrage->Yield( 20 );
				}

				fAngle = NormalizeAngle( fAngle + dAngle0 );
			}
		} );
		
		pBarrage->SetParentBeforeEntity( pEffect );
		pBarrage->Start();
		float fTime = 0;
		m_pAI->Yield( 0, false );

		CVector2 targetPos[4] = 
		{
			{ 150, -150 }, { -150, 150 }, { 150, 150 }, { -150, -150 }
		};
		int i;
		for( i = 0; i < 4 && pPlayer->IsInHorrorReflex(); i++ )
		{
			float fTime1 = 0;
			TTempEntityHolder<CGlitchEffect> pTargetEffect = dynamic_cast<CGlitchEffect*>(
				dynamic_cast<CTutorialGame0*>( m_pGame )->pTargetEffectPrefab->GetRoot()->CreateInstance() );
			pTargetEffect->SetPosition( targetPos[i] );
			pTargetEffect->SetParentBeforeEntity( pEffect );
			pTargetEffect->SetP( &p1[0].x );
			CFunctionTrigger1<SPlayerAttackContext*> trigger( [this, &fTime1] ( SPlayerAttackContext* pContext ) {
				pContext->nResult |= SPlayerAttackContext::eResult_Hit;
				if( fTime1 >= 1.0f )
					m_pAI->Throw( pContext );
			} );
			pTargetEffect->RegisterEntityEvent( eEntityEvent_PlayerAttack, &trigger );
			
			try
			{
				while( pPlayer->IsInHorrorReflex() )
				{
					fTime += GetStage()->GetElapsedTimePerTick();
					float t = Min( 1.0f, Max( 0.0f, fTime * 2 ) );
					auto p = p0[1] * ( 100 * t ) + p0[0] * ( 100 * ( 1 - t ) );
					pEffect->SetP( &p.x );
					
					fTime1 += GetStage()->GetElapsedTimePerTick();
					float t1 = Min( 1.0f, Max( 0.0f, fTime1 ) );
					p = p1[1] * ( 100 * t ) + p1[0] * ( 100 * ( 1 - t ) );
					pTargetEffect->SetP( &p.x );

					m_pAI->Yield( 0, false );
				}
			}
			catch( SPlayerAttackContext* e )
			{
			}
		}

		if( i == 4 )
		{
			pPlayer->AddBreakoutValue( 100 );
		}
	}
}

void CTutorialGame0Enemy::OnUpdate( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime )
{
}

CTutorialGame0::CTutorialGame0( const SClassCreateContext& context )
	: CTutorialGame( context )
	, m_strBackground( context )
	, m_strBackgroundEffect( context )
	, m_strBackgroundEffect1( context )
	, m_strEnemyPrefab( context )
	, m_strHpBar( context )
	, m_strBulletSmall( context )
	, m_strBulletBig( context )
	, m_strTargetEffect( context )
	, m_effectTexture( 400, 300, EFormat::EFormatR8G8B8A8UNorm )
	, m_actionAttack( this, "a", 5, 16, 300, 280, 20, 0.25f )
{
	m_effectTexture.SetRoot( m_pElementRoot );
	m_effectTexture.SetClear( true );
	m_effectTexture.GetCamera().SetSize( 800, 600 );
	m_effectTexture.SetRenderPass( eRenderPass_Occlusion );
}

void CTutorialGame0::OnAddedToStage()
{
	CTutorialGame::OnAddedToStage();
	GetStage()->SetPlayerAction( 0, &m_actionAttack );
	
	pBackgroundPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBackground.c_str() );
	pBackgroundPrefab->GetRoot()->BindShaderResource( EShaderType::PixelShader, "Texture1", &m_effectTexture );
	pBackgroundEffectPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBackgroundEffect.c_str() );
	pBackgroundEffect1Prefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBackgroundEffect1.c_str() );
	pBackgroundEffect1Prefab->GetRoot()->BindShaderResource( EShaderType::PixelShader, "Texture1", &m_effectTexture );
	pEnemyPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strEnemyPrefab.c_str() );
	pEnemyPrefab->GetRoot()->BindShaderResource( EShaderType::PixelShader, "Texture1", &m_effectTexture );
	
	pBulletSmallPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBulletSmall.c_str() );
	pBulletSmallPrefab->GetRoot()->BindShaderResource( EShaderType::PixelShader, "Texture1", &m_effectTexture );
	pBulletBigPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBulletBig.c_str() );
	pBulletBigPrefab->GetRoot()->BindShaderResource( EShaderType::PixelShader, "Texture1", &m_effectTexture );
	pTargetEffectPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strTargetEffect.c_str() );
}

void CTutorialGame0::OnRemovedFromStage()
{
	GetStage()->SetPlayerAction( 0, NULL );
	pBackgroundPrefab->GetRoot()->BindShaderResource( EShaderType::PixelShader, "Texture1", NULL );
	pEnemyPrefab->GetRoot()->BindShaderResource( EShaderType::PixelShader, "Texture1", NULL );
	CTutorialGame::OnRemovedFromStage();
}

void CTutorialGame0::StartGame()
{
	auto pElement = dynamic_cast<CTutorialGameElement*>( pBackgroundPrefab->GetRoot()->CreateInstance() );
	AddGameElement( pElement );

	pElement = dynamic_cast<CTutorialGameElement*>( pEnemyPrefab->GetRoot()->CreateInstance() );
	AddGameElement( pElement );
	
	m_pGUIRoot->AddChild( pBackgroundEffectPrefab->GetRoot()->CreateInstance() );
	m_pHREffect = pBackgroundEffect1Prefab->GetRoot()->CreateInstance();
	m_pGUIRoot->AddChild( m_pHREffect );
	m_nCurHp = 100;
	m_nMaxHp = 100;
	m_pHpBar = dynamic_cast<CTutorialGameHpBar*>( CResourceManager::Inst()->CreateResource<CPrefab>( m_strHpBar.c_str() )->GetRoot()->CreateInstance() );
	m_pHpBar->SetPosition( CVector2( -360, -260 ) );
	m_pHpBar->SetParentEntity( m_pGUIRoot );
	m_pHpBar->SetHp( m_nCurHp, m_nMaxHp );

	GetStage()->SetGUIOption( 0 );
}

void CTutorialGame0::OnPlayerUpdated()
{
	CTutorialGame::OnPlayerUpdated();
	m_effectTexture.GetCamera().SetPosition( m_camCenter.x, m_camCenter.y );

	CPlayer* pPlayer = GetStage()->GetPlayer();
	m_pHREffect->bVisible = pPlayer->IsInHorrorReflex();
	if( m_pHREffect->bVisible )
	{
		float fTimeScale = pPlayer->GetHorrorReflexTimeScale();
		dynamic_cast<CImage2D*>( m_pHREffect.GetPtr() )->GetParam()[0].x = 1 - fTimeScale;
	}
}

void CTutorialGame0::Damage( uint32 nHp, uint32 nMp, uint32 nSp )
{
	m_nCurHp = Max( 0u, m_nCurHp - nHp );
	m_pHpBar->SetHp( m_nCurHp, m_nMaxHp );
}