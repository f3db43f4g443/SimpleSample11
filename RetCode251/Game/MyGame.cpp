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
	}

	m_keyDown.Clear();
	m_keyUp.Clear();
	m_bIsMouseDown = m_bIsMouseUp = false;
	m_bIsRightMouseDown = m_bIsRightMouseUp = false;
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
void RegisterGameClasses_Interfaces();
void RegisterGameClasses_Level();
void RegisterGameClasses_CharacterMisc();
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
	
	REGISTER_ENUM_BEGIN( EDamageType )
		REGISTER_ENUM_ITEM( eDamageHitType_None )
		REGISTER_ENUM_ITEM( eDamageHitType_Alert )
		REGISTER_ENUM_ITEM( eDamageHitType_Kick_Special )
		REGISTER_ENUM_ITEM( eDamageHitType_Kick_Begin )
		REGISTER_ENUM_ITEM( eDamageHitType_Kick_End )
		REGISTER_ENUM_ITEM( eDamageHitType_Kick_End_1 )
		REGISTER_ENUM_ITEM( eDamageHitType_Kick_End_2 )
		REGISTER_ENUM_ITEM( eDamageHitType_Kick_End_3 )
		REGISTER_ENUM_ITEM( eDamageHitType_Kick_End_4 )
	REGISTER_ENUM_END()
	
	REGISTER_ENUM_BEGIN( ECharacterEvent )
		REGISTER_ENUM_ITEM( eCharacterEvent_Update1 )
		REGISTER_ENUM_ITEM( eCharacterEvent_Update2 )
		REGISTER_ENUM_ITEM( eCharacterEvent_Kill )
		REGISTER_ENUM_ITEM( eCharacterEvent_ImpactLevelBegin )
		REGISTER_ENUM_ITEM( eCharacterEvent_ImpactLevelEnd )
	REGISTER_ENUM_END()
	
	REGISTER_CLASS_BEGIN( CCharacter )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nMaxHp )
		REGISTER_MEMBER( m_pKillEffect )
		REGISTER_MEMBER( m_pKillSound )
		REGISTER_MEMBER( m_pCrushEffect )
		REGISTER_MEMBER( m_bIgnoreDamageSource )
		REGISTER_MEMBER( m_bAlwaysBlockBullet )
		REGISTER_MEMBER( m_nDmgToPlayer )
		REGISTER_MEMBER( m_nKillImpactLevel )
		REGISTER_MEMBER( m_fWeight )
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

	REGISTER_CLASS_BEGIN( CPlayer )
		REGISTER_BASE_CLASS( CCharacter )

		REGISTER_MEMBER( m_fMoveSpeed )
		REGISTER_MEMBER( m_fAirMaxSpeed )
		REGISTER_MEMBER( m_fJumpSpeed )
		REGISTER_MEMBER( m_fMoveAcc )
		REGISTER_MEMBER( m_fStopAcc )
		REGISTER_MEMBER( m_fAirAcc )
		REGISTER_MEMBER( m_fGravity )
		REGISTER_MEMBER( m_fMove1Speed )
		REGISTER_MEMBER( m_fMove1Acc )
		REGISTER_MEMBER( m_fStop1Acc )
		REGISTER_MEMBER( m_kickVel )
		REGISTER_MEMBER( m_kickOffset )
		REGISTER_MEMBER( m_kickSpinBackVel )
		REGISTER_MEMBER( m_kickSpinSlideVel )
		REGISTER_MEMBER( m_fKickDashSpeed )
		REGISTER_MEMBER( m_nKickSpinDashSpeed )
		REGISTER_MEMBER( m_fSlideSpeed0 )
		REGISTER_MEMBER( m_fSlideAcc )
		REGISTER_MEMBER( m_fSlideSpeed )
		REGISTER_MEMBER( m_glideVel )
		REGISTER_MEMBER( m_fGlideVelTransfer )
		REGISTER_MEMBER( m_boostAcc )
		REGISTER_MEMBER( m_fBoostThresholdSpeed )
		REGISTER_MEMBER( m_fBoostMinSpeed )
		REGISTER_MEMBER( m_fFlySpeed )
		REGISTER_MEMBER( m_fFlySpeed1 )
		REGISTER_MEMBER( m_fFlyFuelConsume )
		REGISTER_MEMBER( m_fFlyFuelConsume1 )
		REGISTER_MEMBER( m_fDashSpeed )
		REGISTER_MEMBER( m_fDashSpeedY )
		REGISTER_MEMBER( m_fRollAcc )
		REGISTER_MEMBER( m_fRollAcc1 )
		REGISTER_MEMBER( m_fForceRollSpeed )
		REGISTER_MEMBER( m_fBackFlipSpeed )
		REGISTER_MEMBER( m_fBackFlipSpeedY )
		REGISTER_MEMBER( m_fBackFlipAcc )
		REGISTER_MEMBER( m_fGrabAttachSpeed )
		REGISTER_MEMBER( m_punchSpeed )
		REGISTER_MEMBER( m_fPunchBounceSpeed )
		REGISTER_MEMBER( m_fShieldJumpSpeed )
		REGISTER_MEMBER( m_nLandTime )
		REGISTER_MEMBER( m_nJumpHoldTime )
		REGISTER_MEMBER( m_fGravity )
		REGISTER_MEMBER( m_fGravity1 )
		REGISTER_MEMBER( m_fMaxFallSpeed )
		REGISTER_MEMBER( m_fFindFloorDist )
		REGISTER_MEMBER( m_fFallDmgBegin )
		REGISTER_MEMBER( m_fFallDmgPerHeight )
		REGISTER_MEMBER( m_fImpactLevelFallHeight )
		REGISTER_MEMBER( m_nMaxPunchFrame )
		REGISTER_MEMBER( m_nFireCD )
		REGISTER_MEMBER( m_nBombCD )
		REGISTER_MEMBER( m_nHpRecoverCD )
		REGISTER_MEMBER( m_nHpRecoverInterval )
		REGISTER_MEMBER( m_nMaxShield )
		REGISTER_MEMBER( m_nShieldRecoverCD )
		REGISTER_MEMBER( m_nShieldHitRecoverCD )
		REGISTER_MEMBER_TAGGED_PTR( m_pGrabDetect, grab )
		REGISTER_MEMBER_TAGGED_PTR( m_pShieldEffect[0], shield_0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pShieldEffect[1], shield_1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pShieldEffect[2], shield_2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pShieldEffect[3], shield_3 )
		REGISTER_MEMBER( m_pKick )
		REGISTER_MEMBER( m_pKickSpin )
		REGISTER_MEMBER( m_pBullet )
		REGISTER_MEMBER( m_pBomb )
		REGISTER_MEMBER( m_arrBulletOfs )
		REGISTER_MEMBER( m_bombOfs )
		REGISTER_MEMBER( m_nWalkAnimSpeed )
		REGISTER_MEMBER( m_fJumpAnimVel )
		REGISTER_MEMBER( m_nKickReadyTime )
		REGISTER_MEMBER( m_nKickAnimFrame1 )
		REGISTER_MEMBER( m_nKickAnimFrame2 )
		REGISTER_MEMBER( m_nKickSpinAnimSpeed )
		REGISTER_MEMBER( m_nKickSpinAnimRep )
		REGISTER_MEMBER( m_kickRevAnimFrame )
		REGISTER_MEMBER( m_kickStompAnimFrame )
		REGISTER_MEMBER( m_nKickDashDelay )
		REGISTER_MEMBER( m_nKickDashTime )
		REGISTER_MEMBER( m_nKickSpinDashTime )
		REGISTER_MEMBER( m_nKickSpinDashAnimSpeed )
		REGISTER_MEMBER( m_nKickRecoverTime )
		REGISTER_MEMBER( m_nSlideAnimSpeed )
		REGISTER_MEMBER( m_nSlideMaxFrame )
		REGISTER_MEMBER( m_nSlideDashTime )
		REGISTER_MEMBER( m_nSlideAirCD )
		REGISTER_MEMBER( m_nWalk1AnimSpeed )
		REGISTER_MEMBER( m_nStand1ReadyTime )
		REGISTER_MEMBER( m_nGlideFallTime )
		REGISTER_MEMBER( m_nDashGrabBeginFrame )
		REGISTER_MEMBER( m_nGrabMaxTime )
		REGISTER_MEMBER( m_nDashFallEndFrame )
		REGISTER_MEMBER( m_nStandUpAnimSpeed )
		REGISTER_MEMBER( m_nRollStopFrames )
		REGISTER_MEMBER( m_nRollRecoverAnimSpeed )
		REGISTER_MEMBER( m_fRollRotateSpeed )
		REGISTER_MEMBER( m_nBackFlipAnimSpeed )
		REGISTER_MEMBER( m_nBackFlip1AnimSpeed )
		REGISTER_MEMBER( m_nBackFlip2AnimSpeed )
		REGISTER_MEMBER( m_nBackFlip2Time )
		REGISTER_MEMBER_TAGGED_PTR( m_pHit[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pHit[1], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pHit[2], 2 )
		REGISTER_MEMBER( m_fHitOfs1 )
		REGISTER_MEMBER( m_fHitOfs2 )
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

	RegisterGameClasses_Level();
	RegisterGameClasses_CharacterMisc();
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