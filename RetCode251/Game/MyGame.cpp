#include "stdafx.h"
#include "MyGame.h"
#include "Scene2DManager.h"
#include "FileUtil.h"
#include "xml.h"
#include "Player.h"
#include "UICommon/UIFactory.h"
#include "ClassMetaData.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "Common/DateTime.h"
#include "MyLevel.h"
#include "CharacterMove.h"
#include "GlobalCfg.h"
#include "Terrain.h"
#include "Entities/EffectObject.h"

#include "Effects/ParticleTimeoutEmitter.h"
#include "Effects/ParticleSubEmitter.h"

CGame::CGame()
	: m_bStarted( false )
	, m_pCurGameState( NULL )
	, m_key( 128 )
	, m_keyDown( 128 )
	, m_keyUp( 128 )
	, m_keyForceRelease( 128 )
	, m_bIsMouse( false )
	, m_bIsMouseDown( false )
	, m_bIsMouseUp( false )
	, m_bIsRightMouse( false )
	, m_bIsRightMouseDown( false )
	, m_bIsRightMouseUp( false )
	, m_beforeRender( this, &CGame::BeforeRender )
{
	m_keyDownTime.resize( 128 );
	m_keyUpTime.resize( 128 );
}

void CGame::Start()
{
	CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_beforeRender );

	CGlobalCfg::Inst().Load();
	m_bStarted = true;
	if( m_pCurGameState )
		m_pCurGameState->EnterState();
}

void CGame::Stop()
{
	SetCurState( NULL );
	m_trigger.Clear();
	m_bStarted = false;
}

#include "Profile.h"
void CGame::Update()
{
	if( m_keyDown.GetBit( VK_F1 ) )
		CProfileMgr::Inst()->BeginProfile();
	if( m_keyDown.GetBit( VK_F2 ) )
		CProfileMgr::Inst()->EndProfile();

	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	double dLastTime = pRenderSystem->GetLastTime();
	double dTotalTime = pRenderSystem->GetTotalTime();
	m_dTotalTime = dTotalTime;
	const uint32 nFPS = 60;
	const float fInvFPS = 1.0f / nFPS;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );

	if( m_pCurGameState )
		m_pCurGameState->UpdateInput();

	for( int i = 0; i < nFrames; i++ )
	{
		for( auto pObj = CScene2DManager::GetGlobalInst()->Get_AutoUpdateAnimObject(); pObj; )
		{
			auto pObj1 = pObj->NextAutoUpdateAnimObject();
			pObj->UpdateAnim( fInvFPS );
			pObj = pObj1;
		}
		if( m_pCurGameState )
			m_pCurGameState->UpdateFrame();
		m_trigger.UpdateTime();

		m_keyDown.Clear();
		m_keyUp.Clear();
		m_bIsMouseDown = m_bIsMouseUp = false;
		m_bIsRightMouseDown = m_bIsRightMouseUp = false;
	}
}

void CGame::SetCurState( IGameState * pGameState )
{
	if( pGameState == m_pCurGameState )
		return;
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->ExitState();
	m_pCurGameState = pGameState;
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->EnterState();
}

void CGame::OnResize( const CVector2& size )
{
	m_screenRes = size;
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->HandleResize( size );
}

void CGame::OnMouseDown( const CVector2& pos )
{
	if( !m_bIsMouse )
	{
		m_bIsMouse = true;
		m_bIsMouseDown = true;
	}
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->HandleMouseDown( pos );
}

void CGame::OnMouseUp( const CVector2& pos )
{
	if( m_bIsMouse )
	{
		m_bIsMouse = false;
		m_bIsMouseUp = true;
	}
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->HandleMouseUp( pos );
}

void CGame::OnRightMouseDown( const CVector2& pos )
{
	if( !m_bIsRightMouse )
	{
		m_bIsRightMouse = true;
		m_bIsRightMouseDown = true;
	}
}

void CGame::OnRightMouseUp( const CVector2& pos )
{
	if( m_bIsRightMouse )
	{
		m_bIsRightMouse = false;
		m_bIsRightMouseUp = true;
	}
}

void CGame::OnMouseMove( const CVector2& pos )
{
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->HandleMouseMove( pos );
}

void CGame::OnKey( uint32 nChar, bool bKeyDown, bool bAltDown )
{
	if( nChar == VK_DELETE && bKeyDown )
	{
		if( m_pCurGameState && m_bStarted )
			m_pCurGameState->HandleChar( 127 );
	}
	if( nChar >= 128 )
		return;
	if( bKeyDown && !m_key.GetBit( nChar ) )
	{
		m_keyDown.SetBit( nChar, true );
		m_keyDownTime[nChar] = GetCycleCount();
	}
	if( !bKeyDown && m_key.GetBit( nChar ) )
	{
		m_keyUp.SetBit( nChar, true );
		m_keyForceRelease.SetBit( nChar, false );
		m_keyUpTime[nChar] = GetCycleCount();
	}
	m_key.SetBit( nChar, bKeyDown );
}

void CGame::OnChar( uint32 nChar )
{
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->HandleChar( nChar );
}

void Game_ShaderImplement_Dummy();

void RegisterGameClasses_World();
void RegisterGameClasses_Character();
void RegisterGameClasses_Player();
void RegisterGameClasses_Interfaces();
void RegisterGameClasses_Level();
void RegisterGameClasses_CharacterMisc();
void RegisterGameClasses_LevelMisc();
void RegisterGameClasses_GUI();
void RegisterGameClasses_UtilEntities();

void RegisterGameClasses()
{
	RegisterGameClasses_World();
	RegisterGameClasses_Interfaces();

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
		REGISTER_DIFF_FUNC( DiffData )
		REGISTER_PATCH_FUNC( PatchData )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterFlyData )
		REGISTER_MEMBER( fMoveSpeed )
		REGISTER_MEMBER( fRollMaxTime )
		REGISTER_MEMBER( fRollMaxSpeed )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterSimpleWalkData )
		REGISTER_MEMBER( fMoveSpeed )
		REGISTER_MEMBER( fGravity )
		REGISTER_MEMBER( fMaxFallSpeed )
		REGISTER_MEMBER( fJumpSpeed )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterWalkData )
		REGISTER_MEMBER( fMoveSpeed )
		REGISTER_MEMBER( fMoveAcc )
		REGISTER_MEMBER( fStopAcc )
		REGISTER_MEMBER( gravity )
		REGISTER_MEMBER( fMaxFallSpeed )
		REGISTER_MEMBER( fMaxLandFallSpeed )
		REGISTER_MEMBER( fJumpMaxSpeed )
		REGISTER_MEMBER( fJumpMaxHoldTime )
		REGISTER_MEMBER( fAirAcc )
		REGISTER_MEMBER( fAirMaxSpeed )
		REGISTER_MEMBER( fFindFloorDist )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterPhysicsFlyData )
		REGISTER_MEMBER( fMaxAcc )
		REGISTER_MEMBER( fStablity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterPhysicsFlyData1 )
		REGISTER_MEMBER( fMaxAcc )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterCreepData )
		REGISTER_MEMBER( fSpeed )
		REGISTER_MEMBER( fTurnSpeed )
		REGISTER_MEMBER( fFallGravity )
		REGISTER_MEMBER( fMaxFallSpeed )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterSurfaceWalkData )
		REGISTER_MEMBER( fSpeed )
		REGISTER_MEMBER( fFallInitSpeed )
		REGISTER_MEMBER( fGravity )
		REGISTER_MEMBER( fMaxFallSpeed )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterPhysicsMovementData )
		REGISTER_MEMBER( fGravity )
		REGISTER_MEMBER( fMaxFallSpeed )
		REGISTER_MEMBER( fFriction )
		REGISTER_MEMBER( fBounceCoef )
		REGISTER_MEMBER( fBounceCoef1 )
		REGISTER_MEMBER( fRotCoef )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SCharacterVehicleMovementData )
		REGISTER_MEMBER( fBounce )
		REGISTER_MEMBER( fCrushSpeed )
		REGISTER_MEMBER( fFallGravity )
		REGISTER_MEMBER( fMaxFallSpeed )
	REGISTER_CLASS_END()
	
	REGISTER_ENUM_BEGIN( EEntityHitType )
		REGISTER_ENUM_ITEM( eEntityHitType_WorldStatic )
		REGISTER_ENUM_ITEM( eEntityHitType_Platform )
		REGISTER_ENUM_ITEM( eEntityHitType_Platform_1 )
		REGISTER_ENUM_ITEM( eEntityHitType_Player )
		REGISTER_ENUM_ITEM( eEntityHitType_FlyingObject )
		REGISTER_ENUM_ITEM( eEntityHitType_Sensor )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( CEntity )
		REGISTER_MEMBER( m_eHitType )
		REGISTER_MEMBER( m_bHitChannel )
		REGISTER_MEMBER( m_bPlatformChannel )
		REGISTER_MEMBER( m_boundForEditor )
		REGISTER_BASE_CLASS( CPrefabBaseNode )
		REGISTER_BASE_CLASS( CHitProxy )
	REGISTER_CLASS_END()
	
	REGISTER_ENUM_BEGIN( EBugFixType )
		REGISTER_ENUM_ITEM( eBugFixType_Common )
		REGISTER_ENUM_ITEM( eBugFixType_Melee )
		REGISTER_ENUM_ITEM( eBugFixType_Range )
		REGISTER_ENUM_ITEM( eBugFixType_System )
		REGISTER_ENUM_ITEM( eBugFixType_None )
	REGISTER_ENUM_END()
	
	REGISTER_CLASS_BEGIN( CBug )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nBugID )
		REGISTER_MEMBER( m_nGroup )
		REGISTER_MEMBER( m_nExp )
		REGISTER_MEMBER( m_nFixType )
		REGISTER_MEMBER( m_pSoundFixed )
		REGISTER_MEMBER( m_pSoundCleared )
		REGISTER_MEMBER_TAGGED_PTR( m_pFixedEft, eft )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CTerrain )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IEditorTiled )
		REGISTER_BASE_CLASS( ILevelObjLayer )
		REGISTER_MEMBER( m_nBlockIndexBegin )
		REGISTER_MEMBER( m_nBorder )
		REGISTER_MEMBER( m_tileSize )
		REGISTER_MEMBER( m_nTileX )
		REGISTER_MEMBER( m_nTileY )
		REGISTER_MEMBER( m_ofs )
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

	RegisterGameClasses_Character();
	RegisterGameClasses_Player();
	RegisterGameClasses_Level();
	RegisterGameClasses_CharacterMisc();
	RegisterGameClasses_LevelMisc();
	RegisterGameClasses_GUI();
	RegisterGameClasses_UtilEntities();
}

void InitGame()
{
	Game_ShaderImplement_Dummy();
	RegisterGameClasses();

	CResourceManager::Inst()->Register( new TResourceFactory<CWorldCfgFile>() );
	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );
}