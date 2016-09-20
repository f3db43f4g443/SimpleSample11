#include "stdafx.h"
#include "MyGame.h"
#include "Image2D.h"
#include "LightRendering.h"
#include "DefaultDrawable2D.h"
#include "Scene2DManager.h"
#include "FileUtil.h"
#include "xml.h"
#include "Player.h"
#include "GUI/MainUI.h"
#include "Animation.h"
#include "TextureAtlas.h"
#include "ClassMetaData.h"
#include "ClassMetaData.h"
#include "Common/Rand.h"
#include "Useable.h"
#include "Entities/StartPoint.h"
#include "Entities/Entrance.h"
#include "Entities/Door.h"
#include "Entities/TutorialGame.h"
#include "Entities/TutorialGame0.h"
#include "Entities/GlitchEffect.h"
#include "Entities/EffectObject.h"
#include "Entities/CommonBullet.h"

#include "Effects/ParticleTimeoutEmitter.h"
#include "Effects/ParticleSubEmitter.h"

CGame::CGame()
	: m_pWorld( NULL )
	, m_key( 128 )
	, m_keyDown( 128 )
	, m_keyUp( 128 )
{
	m_screenResolution = CVector2( 800, 600 );
}

void CGame::Start()
{
	CMainUI::Inst()->InitResources();
	m_pWorld = new CWorld;

	CPointLightObject* pPointLight = new CPointLightObject( CVector4( 0.1f, 0, 500, -0.05f ), CVector3( 1, 1, 1 ), 10.0f, 0.2f, 0.4f );
	CPlayer* pPlayer = new CPlayer;
	pPlayer->SetRenderObject( pPointLight );
	pPlayer->AddCircle( 32, CVector2( 0, 0 ) );
	m_pWorld->SetPlayer( pPlayer );

	SStageEnterContext context;
	context.strStartPointName = "start1";
	//m_pWorld->EnterStage( "data/lv0/scene.pf", context );
	m_pWorld->EnterStage( "data/lv0/game0/game0.pf", context );
	CMainUI::Inst()->SetVisible( true );
}

void CGame::Stop()
{
	CMainUI::Inst()->SetVisible( false );
	
	m_pWorld->Stop();
	m_trigger.Clear();
	delete m_pWorld;
}

void CGame::Update()
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	double dLastTime = pRenderSystem->GetLastTime();
	double dTotalTime = pRenderSystem->GetTotalTime();
	m_dTotalTime = dTotalTime;
	const uint32 nFPS = 60;
	const float fInvFPS = 1.0f / nFPS;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );

	CPlayer* pPlayer = m_pWorld->GetPlayer();
	const float invsqrt2 = sqrt( 0.5f );
	if( pPlayer )
	{
		CVector2 moveAxis;
		moveAxis.x = (int)m_key.GetBit( 'D' ) - (int)m_key.GetBit( 'A' );
		moveAxis.y = (int)m_key.GetBit( 'W' ) - (int)m_key.GetBit( 'S' );
		moveAxis.Normalize();
		pPlayer->Move( moveAxis.x, moveAxis.y );
	}

	for( int i = 0; i < nFrames; i++ )
	{
		m_trigger.UpdateTime();
		for( auto pObj = CScene2DManager::GetGlobalInst()->Get_AutoUpdateAnimObject(); pObj; pObj = pObj->NextAutoUpdateAnimObject() )
		{
			pObj->UpdateAnim( fInvFPS );
		}
		m_pWorld->Update();
	}

	/*if( m_keyDown.GetBit( 'J' ) )
		pPlayer->EnterHorrorReflex( 0 );
	if( m_keyDown.GetBit( 'K' ) )
		pPlayer->EnterHorrorReflex( 1 );
	if( m_keyDown.GetBit( 'L' ) )
		pPlayer->EnterHorrorReflex( 2 );*/
	if( m_keyDown.GetBit( 'J' ) )
		pPlayer->EnterHorrorReflex( 0 );

	if( m_keyDown.GetBit( 'K' ) )
		pPlayer->Action();
	if( m_keyUp.GetBit( 'K' ) )
		pPlayer->StopAction();

	m_keyDown.Clear();
	m_keyUp.Clear();
}

void CGame::OnKey( uint32 nChar, bool bKeyDown, bool bAltDown )
{
	if( nChar >= 128 )
		return;
	if( nChar >= 'a' && nChar <= 'z' )
		nChar -= 'a' - 'A';
	if( bKeyDown && !m_key.GetBit( nChar ) )
		m_keyDown.SetBit( nChar, true );
	if( !bKeyDown && m_key.GetBit( nChar ) )
		m_keyUp.SetBit( nChar, true );
	m_key.SetBit( nChar, bKeyDown );
}

void Game_ShaderImplement_Dummy();

void RegisterGameClasses()
{
	REGISTER_CLASS_BEGIN( SHitProxyCircle )
		REGISTER_MEMBER( fRadius )
		REGISTER_MEMBER( center )
	REGISTER_CLASS_END()
	REGISTER_CLASS_BEGIN( SHitProxyPolygon )
		REGISTER_MEMBER( nVertices )
		REGISTER_MEMBER( vertices )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN_ABSTRACT( CHitProxy )
		REGISTER_MEMBER( m_bBulletMode )
		REGISTER_MEMBER( m_bTransparent )
		REGISTER_PACK_FUNC( PackData )
		REGISTER_UNPACK_FUNC( UnpackData )
	REGISTER_CLASS_END()
	
	REGISTER_ENUM_BEGIN( EEntityHitType )
		REGISTER_ENUM_ITEM( eEntityHitType_WorldStatic )
		REGISTER_ENUM_ITEM( eEntityHitType_Platform )
		REGISTER_ENUM_ITEM( eEntityHitType_Enemy )
		REGISTER_ENUM_ITEM( eEntityHitType_Player )
		REGISTER_ENUM_ITEM( eEntityHitType_FlyingObject )
		REGISTER_ENUM_ITEM( eEntityHitType_Sensor )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( CEntity )
		REGISTER_MEMBER( m_eHitType )
		REGISTER_BASE_CLASS( CPrefabBaseNode )
		REGISTER_BASE_CLASS( CHitProxy )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEffectObject )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_fTimeLeft )
		REGISTER_MEMBER( m_nType )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCommonBullet )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nDmgHp )
		REGISTER_MEMBER( m_nDmgMp )
		REGISTER_MEMBER( m_nDmgSp )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CStartPoint )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CUseable )
		REGISTER_MEMBER( m_fTime )
		REGISTER_MEMBER( m_fCircleSize )
		REGISTER_MEMBER( m_strText )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CFastUseable )
		REGISTER_BASE_CLASS( CUseable )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDoor )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CEntrance )
		REGISTER_BASE_CLASS( CFastUseable )
		REGISTER_MEMBER( m_strStageName )
		REGISTER_MEMBER( m_strStartPointName )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CTutorialGame )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strCrosshair )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGameBullet )
		REGISTER_BASE_CLASS( CCommonBullet )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGame0 )
		REGISTER_BASE_CLASS( CTutorialGame )
		REGISTER_MEMBER( m_strBackground )
		REGISTER_MEMBER( m_strBackgroundEffect )
		REGISTER_MEMBER( m_strBackgroundEffect1 )
		REGISTER_MEMBER( m_strEnemyPrefab )
		REGISTER_MEMBER( m_strHpBar )
		REGISTER_MEMBER( m_strBulletSmall )
		REGISTER_MEMBER( m_strBulletBig )
		REGISTER_MEMBER( m_strTargetEffect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGame0Enemy )
		REGISTER_BASE_CLASS( CTutorialGameElement )
		REGISTER_MEMBER( m_strHREffect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGameFall )
		REGISTER_BASE_CLASS( CTutorialGame )
		REGISTER_MEMBER( m_strBackground )
		REGISTER_MEMBER( m_strCoin )
		REGISTER_MEMBER( m_strChest )
		REGISTER_MEMBER( m_strParticle0 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGameElement )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGameHpBar )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_fHeightPerHp )
		REGISTER_MEMBER( m_strElem )
		REGISTER_MEMBER( m_strElem1 )
		REGISTER_MEMBER( m_strEffect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGameFlyingObject )
		REGISTER_BASE_CLASS( CTutorialGameElement )
		REGISTER_MEMBER( m_fFlyingSpeed )
		REGISTER_MEMBER( m_fLife )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialGameHitObject )
		REGISTER_BASE_CLASS( CTutorialGameFlyingObject )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CGlitchEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nPrefabCount )
		REGISTER_MEMBER( m_nMaxNodes )
		REGISTER_MEMBER( m_effectRect )
		REGISTER_MEMBER( m_strMaterial0 )
		REGISTER_MEMBER( m_strMaterial1 )
		REGISTER_MEMBER( m_strMaterial2 )
		REGISTER_MEMBER( m_strMaterial3 )
		REGISTER_MEMBER( m_p )
		REGISTER_MEMBER( m_baseTex )
		REGISTER_MEMBER( m_texSize )
		REGISTER_MEMBER( m_fMaxTexOfs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CParticleTimeoutEmitter )
		REGISTER_BASE_CLASS( IParticleEmitter )
		REGISTER_MEMBER( m_fTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CParticleSubEmitter )
		REGISTER_BASE_CLASS( IParticleEmitter )
		REGISTER_MEMBER( m_nSubEmitterCount )
		REGISTER_MEMBER( m_fLifeTime )
		REGISTER_MEMBER( m_fDirMin )
		REGISTER_MEMBER( m_fDirMax )
		REGISTER_MEMBER( m_sizeMin )
		REGISTER_MEMBER( m_sizeMax )
		REGISTER_MEMBER( m_s0Min )
		REGISTER_MEMBER( m_s0Max )
		REGISTER_MEMBER( m_vMin )
		REGISTER_MEMBER( m_vMax )
		REGISTER_MEMBER( m_aMin )
		REGISTER_MEMBER( m_aMax )
		REGISTER_MEMBER( m_bIgnoreGlobalTransform )
		REGISTER_MEMBER( m_bRotates0 )
		REGISTER_MEMBER( m_bRotatev )
		REGISTER_MEMBER( m_bRotatea )
	REGISTER_CLASS_END()
}

void InitGame()
{
	Game_ShaderImplement_Dummy();
	RegisterGameClasses();
}