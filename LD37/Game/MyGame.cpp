#include "stdafx.h"
#include "MyGame.h"
#include "Scene2DManager.h"
#include "FileUtil.h"
#include "xml.h"
#include "Player.h"
#include "Enemy.h"
#include "Block.h"
#include "BlockItem.h"
#include "LevelScrollObj.h"
#include "CharacterMove.h"
#include "GUI/StageDirector.h"
#include "UICommon/UIFactory.h"
#include "ClassMetaData.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "GlobalCfg.h"
#include "LevelDesign.h"
#include "MainMenu.h"

#include "Entities/Enemies.h"
#include "Entities/EnemyCharacters.h"
#include "Entities/Neutral.h"
#include "Entities/Door.h"
#include "Entities/FallingObj.h"
#include "Entities/StartPoint.h"
#include "Entities/GlitchEffect.h"
#include "Entities/EffectObject.h"
#include "Entities/Decorator.h"
#include "Entities/UtilEntities.h"
#include "Entities/Barrage.h"
#include "Entities/Blocks/RandomBlocks.h"
#include "Entities/Blocks/SpecialBlocks.h"
#include "Entities/Blocks/LvBarriers.h"
#include "Entities/Blocks/lv1/SpecialLv1.h"
#include "Entities/Blocks/lv2/SpecialLv2.h"
#include "Entities/BlockItems/BlockItemsLv1.h"
#include "Entities/BlockItems/BlockItemsLv2.h"
#include "Entities/Bullets.h"
#include "Entities/BlockBuffs.h"
#include "Entities/Consumables.h"
#include "Entities/Enemies/Lv1Boss.h"
#include "Entities/Enemies/Lv1Enemies.h"
#include "Entities/Enemies/Lv2Enemies.h"
#include "Entities/Items/SpecialWeapons0.h"
#include "Entities/Tutorial.h"
#include "GUI/MainUI.h"
#include "GUI/ChunkUI.h"
#include "GUI/Splash.h"

#include "Bullet.h"
#include "Explosion.h"
#include "Lightning.h"
#include "Weapons.h"
#include "Pickup.h"

#include "Effects/ParticleTimeoutEmitter.h"
#include "Effects/ParticleSubEmitter.h"
#include "Effects/ParticleArrayEmitter.h"


CGame::CGame()
	: m_bStarted( false )
	, m_pCurGameState( NULL )
	, m_key( 128 )
	, m_keyDown( 128 )
	, m_keyUp( 128 )
	, m_bIsMouse( false )
	, m_bIsMouseDown( false )
	, m_bIsMouseUp( false )
	, m_bIsRightMouse( false )
	, m_bIsRightMouseDown( false )
	, m_bIsRightMouseUp( false )
	, m_beforeRender( this, &CGame::BeforeRender )
{
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
	if( m_keyDown.GetBit( '1' ) )
		CProfileMgr::Inst()->BeginProfile();
	if( m_keyDown.GetBit( '2' ) )
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
		m_keyDown.SetBit( nChar, true );
	if( !bKeyDown && m_key.GetBit( nChar ) )
		m_keyUp.SetBit( nChar, true );
	m_key.SetBit( nChar, bKeyDown );
}

void CGame::OnChar( uint32 nChar )
{
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->HandleChar( nChar );
}

void Game_ShaderImplement_Dummy();

void RegisterGameClasses()
{
	REGISTER_CLASS_BEGIN( SAttribute )
		REGISTER_MEMBER( base )
	REGISTER_CLASS_END()

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
		REGISTER_MEMBER( fRollMaxTime )
		REGISTER_MEMBER( fRollMaxSpeed )
		REGISTER_MEMBER( fFindFloorDist )
		REGISTER_MEMBER( fSlideDownSpeed )
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
	
	REGISTER_CLASS_BEGIN( CCharacter )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_pKillEffect )
		REGISTER_MEMBER( m_pCrushEffect )
		REGISTER_MEMBER( m_pKillSound )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CDamageEft )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_prefabs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayer )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_hp )
		REGISTER_MEMBER( m_sp )
		REGISTER_MEMBER( m_nSpRegenPerFrame )
		REGISTER_MEMBER( m_nSpRegenPerFrameSlidingDown )
		REGISTER_MEMBER( m_nRollSpCost )
		REGISTER_MEMBER( m_walkData )
		REGISTER_MEMBER( m_flyData )
		REGISTER_MEMBER_TAGGED_PTR( m_pCore, core );
		REGISTER_MEMBER_TAGGED_PTR( m_pCurRoomChunkUI, chunk_ui );
		REGISTER_MEMBER_TAGGED_PTR( m_pBlockDetectUI, block_ui );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CEnemy )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nHp )
		REGISTER_MEMBER( m_fDefence )
		REGISTER_MEMBER( m_nKnockbackCostSp )
		REGISTER_MEMBER( m_nHitType )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CEnemyPart )
		REGISTER_BASE_CLASS( CEnemy )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEnemy1 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_fFireInterval )
		REGISTER_MEMBER( m_nAmmoCount )
		REGISTER_MEMBER( m_fFireRate )
		REGISTER_MEMBER( m_nBulletCount )
		REGISTER_MEMBER( m_fBulletSpeed )
		REGISTER_MEMBER( m_fBulletAngle )
		REGISTER_MEMBER( m_fSight )
		REGISTER_MEMBER( m_fShakePerFire )
		REGISTER_MEMBER( m_strPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEnemy2 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_fSight )
		REGISTER_MEMBER( m_strPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBoss )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strPrefab )
		REGISTER_MEMBER( m_strPrefab1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pLeft, l );
		REGISTER_MEMBER_TAGGED_PTR( m_pRight, r );
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CEnemyCharacter )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_walkData )
		REGISTER_MEMBER( m_flyData )
		REGISTER_MEMBER( m_fClimbSpeed )
		REGISTER_MEMBER( m_nFireCD )
		REGISTER_MEMBER( m_nFireStopTime )
		REGISTER_MEMBER( m_nFireInterval )
		REGISTER_MEMBER( m_nAmmoCount )
		REGISTER_MEMBER( m_nBulletCount )
		REGISTER_MEMBER( m_fBulletSpeed )
		REGISTER_MEMBER( m_fBulletAngle )
		REGISTER_MEMBER( m_fSight )
		REGISTER_MEMBER( m_fShakePerFire )
		REGISTER_MEMBER( m_fPredict )
		REGISTER_MEMBER( m_strPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEnemyCharacterLeader )
		REGISTER_BASE_CLASS( CEnemyCharacter )
		REGISTER_MEMBER( m_fRadius )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCop )
		REGISTER_BASE_CLASS( CEnemyCharacter )
		REGISTER_MEMBER( m_fMaxScanDist )
		REGISTER_MEMBER( m_nGridsPerStep )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CThug )
		REGISTER_BASE_CLASS( CEnemyCharacter )
		REGISTER_MEMBER( m_fMaxScanDist )
		REGISTER_MEMBER( m_nGridsPerStep )
		REGISTER_MEMBER( m_fThrowSpeed )
		REGISTER_MEMBER( m_throwObjOfs )
		REGISTER_MEMBER( m_fThrowDist )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CFuelTank )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_nAwakeEffectInterval )
		REGISTER_MEMBER( m_strAwakeEffect )
		REGISTER_MEMBER( m_strKillEffect )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_accleration )
		REGISTER_MEMBER( m_nMaxHitDamage )
		REGISTER_MEMBER( m_fFireMoveDist )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CMyLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_bIsLevelDesignTest )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_nSpawnHeight )
		REGISTER_MEMBER( m_fFallDistPerSpeedFrame )
		REGISTER_MEMBER( m_pBlockRT )
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot, chunks );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot1, chunks1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkEffectRoot, chunkeffects );
		REGISTER_MEMBER_TAGGED_PTR( m_pCrosshair, crosshair );
		REGISTER_MEMBER_TAGGED_PTR( m_pBack0, back0 );
		REGISTER_MEMBER_TAGGED_PTR( m_pScrollObjRoot[0], building/scroll0 );
		REGISTER_MEMBER_TAGGED_PTR( m_pScrollObjRoot[1], building/scroll1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pScrollObjRoot[2], building/scroll2 );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMainUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pB, b );
		REGISTER_MEMBER_TAGGED_PTR( m_pC, c );
		REGISTER_MEMBER_TAGGED_PTR( m_pRB, rb );
		REGISTER_MEMBER_TAGGED_PTR( m_pRT, rt );
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarRoot, c/hpbar );
		REGISTER_MEMBER_TAGGED_PTR( m_pSpBarRoot, c/spbar );
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBar[0], c/hpbar/hp_1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBar[1], c/hpbar/hp_2 );
		REGISTER_MEMBER_TAGGED_PTR( m_pHpStoreBar[0], c/hpbar/hp1_1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pHpStoreBar[1], c/hpbar/hp1_2 );
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarBack[0], c/hpbar/hp_b1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBarBack[1], c/hpbar/hp_b2 );
		REGISTER_MEMBER_TAGGED_PTR( m_pSpBar, c/spbar/sp );
		REGISTER_MEMBER_TAGGED_PTR( m_pSpBarBack[0], c/spbar/sp_a );
		REGISTER_MEMBER_TAGGED_PTR( m_pSpBarBack[1], c/spbar/sp_b );
		REGISTER_MEMBER_TAGGED_PTR( m_pMoney, b/money );
		REGISTER_MEMBER_TAGGED_PTR( m_pShake, rb/shake );
		REGISTER_MEMBER_TAGGED_PTR( m_pMinimap, rb/minimap );
		REGISTER_MEMBER_TAGGED_PTR( m_pUse, use );
		REGISTER_MEMBER_TAGGED_PTR( m_pUseText, use/text );
		REGISTER_MEMBER_TAGGED_PTR( m_pSkip, rt/skip );
		REGISTER_MEMBER_TAGGED_PTR( m_pShakeSmallBars[0], rb/minimap/shake );
		REGISTER_MEMBER_TAGGED_PTR( m_pConsumableSlot[0], b/consumables/0 );
		REGISTER_MEMBER_TAGGED_PTR( m_pConsumableSlot[1], b/consumables/1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pConsumableSlot[2], b/consumables/2 );
		REGISTER_MEMBER_TAGGED_PTR( m_pConsumableSlot[3], b/consumables/3 );
		REGISTER_MEMBER_TAGGED_PTR( m_pConsumableSlot[4], b/consumables/4 );
		REGISTER_MEMBER_TAGGED_PTR( m_pConsumableSlot[5], b/consumables/5 );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChunkUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[0], frame/00 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[1], frame/01 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[2], frame/02 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[3], frame/10 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[4], frame/12 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[5], frame/20 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[6], frame/21 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[7], frame/22 );
		REGISTER_MEMBER_TAGGED_PTR( m_pRepairImg, repair );
		REGISTER_MEMBER_TAGGED_PTR( m_pBlockBulletEffect, blockbullet );
		REGISTER_MEMBER_TAGGED_PTR( m_pRepairEffect, particle );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CBlockDetectUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_fMaxDetectRange )
		REGISTER_MEMBER( m_fFadeDist )
		REGISTER_MEMBER( m_fFadeInSpeed )
		REGISTER_MEMBER( m_pUIDrawable )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN_ABSTRACT( CBarrage )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CBullet )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nDeathFrameBegin )
		REGISTER_MEMBER( m_nDeathFrameEnd )
		REGISTER_MEMBER( m_fDeathFramesPerSec )
		REGISTER_MEMBER( m_nType )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_nDamage2 )
		REGISTER_MEMBER( m_nBoundType )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_pDmgEft )
		REGISTER_MEMBER_TAGGED_PTR( m_pDeathEffect, deatheft );
		REGISTER_MEMBER_TAGGED_PTR( m_pParticle, particle );
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CBomb )
		REGISTER_BASE_CLASS( CBullet )
		REGISTER_MEMBER( m_bExplodeOnHitWorld )
		REGISTER_MEMBER( m_bExplodeOnHitBlock )
		REGISTER_MEMBER( m_bExplodeOnHitChar )
		REGISTER_MEMBER_TAGGED_PTR( m_pExp, exp );
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CThrowObj )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_nLife1 )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CBulletWithBlockBuff )
		REGISTER_BASE_CLASS( CBullet )
		REGISTER_MEMBER( m_strBlockBuff )
		REGISTER_MEMBER( m_context )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CExplosion )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_nHitBeginFrame )
		REGISTER_MEMBER( m_nHitFrameCount )
		REGISTER_MEMBER( m_nHitInterval )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDeltaDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_nDeltaDamage1 )
		REGISTER_MEMBER( m_nDamage2 )
		REGISTER_MEMBER( m_nDeltaDamage2 )
		REGISTER_MEMBER( m_nRangeType )
		REGISTER_MEMBER( m_fInitRange )
		REGISTER_MEMBER( m_fDeltaRange )
		REGISTER_MEMBER( m_fInitRange1 )
		REGISTER_MEMBER( m_fDeltaRange1 )
		REGISTER_MEMBER( m_fInitRange2 )
		REGISTER_MEMBER( m_fDeltaRange2 )
		REGISTER_MEMBER( m_fInitRange3 )
		REGISTER_MEMBER( m_fDeltaRange3 )
		REGISTER_MEMBER( m_bHitPlayer )
		REGISTER_MEMBER( m_bHitEnemy )
		REGISTER_MEMBER( m_bHitNeutral )
		REGISTER_MEMBER( m_bHitBlock )
		REGISTER_MEMBER( m_bHitWall )
		REGISTER_MEMBER( m_bHitHidingPlayer )
		REGISTER_MEMBER( m_bHitHidingEnemy )
		REGISTER_MEMBER( m_bHitHidingNeutral )
		REGISTER_MEMBER( m_bHitCreator )
		REGISTER_MEMBER( m_pDmgEft )
		REGISTER_MEMBER( m_pSound )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CExplosionWithBlockBuff )
		REGISTER_BASE_CLASS( CExplosion )
		REGISTER_MEMBER( m_strBlockBuff )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CExplosionKnockback )
		REGISTER_BASE_CLASS( CExplosion )
		REGISTER_MEMBER( m_fKnockbackStrength )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CPlayerBulletMultiHit )
		REGISTER_BASE_CLASS( CBullet )
		REGISTER_MEMBER( m_nHitCD )
		REGISTER_MEMBER( m_bMultiHitBlock )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickUp )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pDeathEffect, deatheft );
		REGISTER_MEMBER_TAGGED_PTR( m_pText, text );
		REGISTER_MEMBER_TAGGED_PTR( m_pPriceText, price );
		REGISTER_MEMBER_TAGGED_PTR( m_pPriceText1, price1 );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickUpCommon )
		REGISTER_BASE_CLASS( CPickUp )
		REGISTER_MEMBER( m_nHeal )
		REGISTER_MEMBER( m_nHpRestore )
		REGISTER_MEMBER( m_nMoney )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickUpItem )
		REGISTER_BASE_CLASS( CPickUp )
		REGISTER_MEMBER_TAGGED_PTR( m_pItem, item );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickUpTemplate )
		REGISTER_BASE_CLASS( CPickUp )
		REGISTER_MEMBER( m_ofs )
		REGISTER_MEMBER( m_fPricePercent )
		REGISTER_MEMBER_TAGGED_PTR( m_pLight, light );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickupCarrier )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER_TAGGED_PTR( m_pPickup, pickup );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickUpCarrierPhysics )
		REGISTER_BASE_CLASS( CPickupCarrier )
		REGISTER_MEMBER( m_moveData )
		REGISTER_MEMBER( m_flyData )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_fAttractDist )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CEnemyBullet )
		REGISTER_BASE_CLASS( CBullet )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPlayerBullet )
		REGISTER_BASE_CLASS( CBullet )
		REGISTER_MEMBER( m_nDmg )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CLightning )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nType )
		REGISTER_MEMBER( m_fWidth )
		REGISTER_MEMBER( m_fHitWidth )
		REGISTER_MEMBER( m_fTexYTileLen )
		REGISTER_MEMBER( m_nHitFrameBegin )
		REGISTER_MEMBER( m_nHitFrameCount )
		REGISTER_MEMBER( m_nHitCD )
		REGISTER_MEMBER( m_bBurst )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_fHitWidthPerFrame )
		REGISTER_MEMBER( m_fBeginLen )
		REGISTER_MEMBER( m_fEndLen )
		REGISTER_MEMBER( m_fBeginTexLen )
		REGISTER_MEMBER( m_fEndTexLen )
		REGISTER_MEMBER( m_bIsBeam )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_nDamage2 )
		REGISTER_MEMBER( m_fKnockback )
		REGISTER_MEMBER( m_pDmgEft )
		REGISTER_MEMBER_TAGGED_PTR( m_pBeginEft, begin );
		REGISTER_MEMBER_TAGGED_PTR( m_pEndEft, end );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CItem )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strKey )
		REGISTER_MEMBER( m_strUpgrade )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CItemCommon )
		REGISTER_BASE_CLASS( CItem )
		REGISTER_MEMBER( m_nHp )
		REGISTER_MEMBER( m_nSp )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CConsumable )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_pIcon )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CConsumableHealing )
		REGISTER_BASE_CLASS( CConsumable )
		REGISTER_MEMBER( m_fPercent )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CConsumableRepair )
		REGISTER_BASE_CLASS( CConsumable )
		REGISTER_MEMBER( m_fPercent )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CConsumableEffect )
		REGISTER_BASE_CLASS( CConsumable )
		REGISTER_MEMBER_TAGGED_PTR( m_pEffect, effect );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerSpecialEffect )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nType )
		REGISTER_MEMBER( m_nTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerWeapon )
		REGISTER_BASE_CLASS( CItem )
		REGISTER_MEMBER( m_texRectFaceRight )
		REGISTER_MEMBER( m_texRectFaceLeft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerWeaponShoot0 )
		REGISTER_BASE_CLASS( CPlayerWeapon )
		REGISTER_MEMBER( m_strBulletName )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_nDamage2 )
		REGISTER_MEMBER( m_nBulletCount )
		REGISTER_MEMBER( m_fAngle )
		REGISTER_MEMBER( m_nDistribution )
		REGISTER_MEMBER( m_fSpeed )
		REGISTER_MEMBER( m_fGravity )
		REGISTER_MEMBER( m_nFireRate )
		REGISTER_MEMBER( m_fShakePerFire )
		REGISTER_MEMBER( m_fireOfs )
		REGISTER_MEMBER( m_fAngularSpeed )
		REGISTER_MEMBER( m_nBulletLife )
		REGISTER_MEMBER( m_strFireSound )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerWeaponLaser0 )
		REGISTER_BASE_CLASS( CPlayerWeapon )
		REGISTER_MEMBER( m_strBulletName )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_nDamage2 )
		REGISTER_MEMBER( m_nHitCD )
		REGISTER_MEMBER( m_fWidth )
		REGISTER_MEMBER( m_fHitWidth )
		REGISTER_MEMBER( m_fRange )
		REGISTER_MEMBER( m_nFireRate )
		REGISTER_MEMBER( m_fAimSpeed )
		REGISTER_MEMBER( m_fireOfs )
		REGISTER_MEMBER( m_fShakePerSec )
		REGISTER_MEMBER( m_strFireSound )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDrill )
		REGISTER_BASE_CLASS( CBullet )
		REGISTER_MEMBER( m_nHitCD )
		REGISTER_MEMBER( m_fSpeedHit1 )
		REGISTER_MEMBER( m_fSpeedHit2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBanknotePrinter )
		REGISTER_BASE_CLASS( CPlayerWeapon )
		REGISTER_MEMBER( m_strBulletName )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_nDamage2 )
		REGISTER_MEMBER( m_nMaxBullets )
		REGISTER_MEMBER( m_nBulletLife )
		REGISTER_MEMBER( m_fInitSpeed )
		REGISTER_MEMBER( m_fInitSpeed1 )
		REGISTER_MEMBER( m_fAcc )
		REGISTER_MEMBER( m_fTargetSpeed )
		REGISTER_MEMBER( m_fOrbitRad )
		REGISTER_MEMBER( m_a )
		REGISTER_MEMBER( m_b )
		REGISTER_MEMBER( m_c )
		REGISTER_MEMBER( m_nFireRate )
		REGISTER_MEMBER( m_nFireRate1 )
		REGISTER_MEMBER( m_fShakePerSecBullet )
		REGISTER_MEMBER( m_fireOfs )
		REGISTER_MEMBER( m_strFireSound )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockObject )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChunkObject )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strEffect )
		REGISTER_MEMBER( m_strSoundEffect )
		REGISTER_MEMBER( m_strBlockRTDrawable )
		REGISTER_MEMBER( m_nMaxHp )
		REGISTER_MEMBER( m_nCrushCost )
		REGISTER_MEMBER_TAGGED_PTR( m_p1, 1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pDamagedEffectsRoot, dmgeft );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCharacterChunk )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER_TAGGED_PTR( m_pCharacter, character );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSpecialChunk )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_bKillTrigger )
		REGISTER_MEMBER( m_nTriggerImpact )
		REGISTER_MEMBER( m_strBullet )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSpecialChunk1 )
		REGISTER_BASE_CLASS( CSpecialChunk )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSpecialChunk2 )
		REGISTER_BASE_CLASS( CSpecialChunk )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSpecialChunk3 )
		REGISTER_BASE_CLASS( CSpecialChunk )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CExplosiveChunk )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nMaxKillHp )
		REGISTER_MEMBER( m_nDeathDamage )
		REGISTER_MEMBER( m_nDeathDamageInterval )
		REGISTER_MEMBER( m_nKillEffectInterval )
		REGISTER_MEMBER( m_strKillEffect )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CBarrel )
		REGISTER_BASE_CLASS( CExplosiveChunk )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strExp )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CBarrel1 )
		REGISTER_BASE_CLASS( CExplosiveChunk )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_strExp )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomEnemyRoom )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_strRes )
		REGISTER_MEMBER( m_strDoor )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunk )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_texScale )
		REGISTER_MEMBER( m_texOfs )
		REGISTER_MEMBER( m_dmgTexScale )
		REGISTER_MEMBER( m_dmgTexOfs )
		REGISTER_MEMBER( m_nHpPerSize )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunk0 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunkTiledSimple )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_texRect1 )
		REGISTER_MEMBER( m_texRect2 )
		REGISTER_MEMBER( m_texRect3 )
		REGISTER_MEMBER( m_texRect4 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunkTiled )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_bBlockTypeMask )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunkTiled1 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_sizeX )
		REGISTER_MEMBER( m_sizeY )
		REGISTER_MEMBER( m_texRects )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunk1 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_nAltX )
		REGISTER_MEMBER( m_nAltY )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunk2 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerLength )
		REGISTER_MEMBER( m_bVertical )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_texRect1 )
		REGISTER_MEMBER( m_nTexRect1X )
		REGISTER_MEMBER( m_nTexRect1Y )
		REGISTER_MEMBER( m_texRect1End )
		REGISTER_MEMBER( m_nTexRect1EndX )
		REGISTER_MEMBER( m_nTexRect1EndY )
		REGISTER_MEMBER( m_texRect2 )
		REGISTER_MEMBER( m_nTexRect2X )
		REGISTER_MEMBER( m_nTexRect2Y )
		REGISTER_MEMBER( m_texRect2End )
		REGISTER_MEMBER( m_nTexRect2EndX )
		REGISTER_MEMBER( m_nTexRect2EndY )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomChunk3 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_bBlockTypeMask )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTriggerChunk )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nTriggerType )
		REGISTER_MEMBER( m_nTriggerImpact )
		REGISTER_MEMBER( m_strPrefab )
		REGISTER_MEMBER( m_strPrefab1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDefaultRandomRoom )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomRoom1 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLvFloor1 )
		REGISTER_BASE_CLASS( CRandomChunkTiled )
		REGISTER_MEMBER( m_strCrate )
		REGISTER_MEMBER( m_strItemDrop )
		REGISTER_MEMBER( m_fWeights )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLvFloor2 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_strItemDrop )
		REGISTER_MEMBER( m_fWeights )
		REGISTER_MEMBER( m_pPrefab )
		REGISTER_MEMBER( m_pPrefab1 )
		REGISTER_MEMBER( m_pCrate )
		REGISTER_MEMBER( m_pItemDropPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLvBarrier1Core )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_strBullet2 )
		REGISTER_MEMBER( m_fOpenDist )
		REGISTER_MEMBER( m_fCloseDist )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLvBarrier1 )
		REGISTER_BASE_CLASS( CRandomChunkTiled )
		REGISTER_MEMBER( m_strCore )
		REGISTER_MEMBER( m_strWall )
		REGISTER_MEMBER( m_strKillEffect )
		REGISTER_MEMBER( m_nKillEffectInterval )
		REGISTER_MEMBER( m_nDeathTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLvBarrierReward1 )
		REGISTER_BASE_CLASS( CRandomChunkTiled )
		REGISTER_MEMBER( m_strItemDrop )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CGarbageBinRed )
		REGISTER_BASE_CLASS( CTriggerChunk )
		REGISTER_MEMBER( m_nBulletCount )
		REGISTER_MEMBER( m_fMinSpeed )
		REGISTER_MEMBER( m_fMaxSpeed )
		REGISTER_MEMBER( m_fShake )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CGarbageBinYellow )
		REGISTER_BASE_CLASS( CTriggerChunk )
		REGISTER_MEMBER( m_fShake )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CGarbageBinGreen )
		REGISTER_BASE_CLASS( CTriggerChunk )
		REGISTER_MEMBER( m_nBulletCount )
		REGISTER_MEMBER( m_fMinSpeed )
		REGISTER_MEMBER( m_fMaxSpeed )
		REGISTER_MEMBER( m_fGravity )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_fDamage )
		REGISTER_MEMBER( m_fShake )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CGarbageBinBlack )
		REGISTER_BASE_CLASS( CTriggerChunk )
		REGISTER_MEMBER( m_nCount )
		REGISTER_MEMBER( m_bSetAngle )
		REGISTER_MEMBER( m_fMinSpeed )
		REGISTER_MEMBER( m_fMaxSpeed )
		REGISTER_MEMBER( m_fShake )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CHousePart )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_pEntrancePrefabs )
		REGISTER_MEMBER( m_pExp )
	REGISTER_CLASS_END()
			
	REGISTER_CLASS_BEGIN( CHouse )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_pThrowObjPrefabs )
		REGISTER_MEMBER( m_nThrowObjMin )
		REGISTER_MEMBER( m_nThrowObjMax )
		REGISTER_MEMBER( m_pInitCharPrefabs )
		REGISTER_MEMBER( m_fInitCharPerGrid )
		REGISTER_MEMBER( m_pExp )
		REGISTER_MEMBER( m_pExpEft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockBuff )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_bMulti )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockBuff::SContext )
		REGISTER_MEMBER( nLife )
		REGISTER_MEMBER( nTotalLife )
		REGISTER_MEMBER( fParams )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockBuffAcid )
		REGISTER_BASE_CLASS( CBlockBuff )
		REGISTER_MEMBER( m_strBulletPrefab )
		REGISTER_MEMBER( m_fNewBulletLifePercent )
		REGISTER_MEMBER( m_fLifePercentCostPerBullet )
		REGISTER_MEMBER( m_fBulletVelocityMin )
		REGISTER_MEMBER( m_fBulletVelocityMax )
		REGISTER_MEMBER( m_fBulletGravity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockBuffFire )
		REGISTER_BASE_CLASS( CBlockBuff )
		REGISTER_MEMBER( m_strExplosionPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockItem )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_ENUM_BEGIN( EBlockItemTriggerType )
		REGISTER_ENUM_ITEM( eBlockItemTriggerType_Step )
		REGISTER_ENUM_ITEM( eBlockItemTriggerType_HitDeltaPos )
		REGISTER_ENUM_ITEM( eBlockItemTriggerType_HitVelocity )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( CBlockItemTrigger )
		REGISTER_BASE_CLASS( CBlockItem )
		REGISTER_MEMBER( m_nTriggerCD )
		REGISTER_MEMBER( m_eTriggerType )
		REGISTER_MEMBER_TAGGED_PTR( m_pTriggerArea, trigger )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockItemTrigger1 )
		REGISTER_BASE_CLASS( CBlockItemTrigger )
		REGISTER_MEMBER( m_nAmmoCount )
		REGISTER_MEMBER( m_nFireRate )
		REGISTER_MEMBER( m_nBulletCount )
		REGISTER_MEMBER( m_fBulletSpeed )
		REGISTER_MEMBER( m_fBulletAngle )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockItemTrigger2 )
		REGISTER_BASE_CLASS( CBlockItemTrigger )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_strLightning )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelScrollObj )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER_TAGGED_PTR( m_pEffect, eft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLv1Boss )
		REGISTER_BASE_CLASS( CLevelScrollObj )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_strBullet2 )
		REGISTER_MEMBER( m_strBullet3 )
		REGISTER_MEMBER( m_strLaser )
		REGISTER_MEMBER( m_strBulletEye )
		REGISTER_MEMBER( m_strBulletShockwave )
		REGISTER_MEMBER( m_strTentacle )
		REGISTER_MEMBER( m_strTentacleHole )
		REGISTER_MEMBER( m_strWorm1 )
		REGISTER_MEMBER( m_strWorm2 )
		REGISTER_MEMBER( m_strExpKnockbackName )
		REGISTER_MEMBER( m_strExpKnockback1 )
		REGISTER_MEMBER( m_strTransparentChunkName )
		REGISTER_MEMBER( m_strTentacleName1 )
		REGISTER_MEMBER( m_strExplosive0 )
		REGISTER_MEMBER( m_strExplosive1 )
		REGISTER_MEMBER( m_strExplosive2 )
		REGISTER_MEMBER( m_strExplosive3 )
		REGISTER_MEMBER( m_strParticle0 )
		REGISTER_MEMBER( m_strParticle1 )
		REGISTER_MEMBER( m_strParticle2 )
		REGISTER_MEMBER( m_strParticle3 )
		REGISTER_MEMBER( m_strKillEffect )
		REGISTER_MEMBER_TAGGED_PTR( m_pBoss, boss )
		REGISTER_MEMBER_TAGGED_PTR( m_pFaceEye[0], boss/face/eye_l )
		REGISTER_MEMBER_TAGGED_PTR( m_pFaceEye[1], boss/face/eye_r )
		REGISTER_MEMBER_TAGGED_PTR( m_pFaceNose, boss/face/nose )
		REGISTER_MEMBER_TAGGED_PTR( m_pFaceMouth, boss/face/mouth )
		REGISTER_MEMBER_TAGGED_PTR( m_pEyeHole[0], boss/eye_l_hole )
		REGISTER_MEMBER_TAGGED_PTR( m_pEyeHole[1], boss/eye_r_hole )
		REGISTER_MEMBER_TAGGED_PTR( m_pEye[0], boss/eye_l_hole/eye_l )
		REGISTER_MEMBER_TAGGED_PTR( m_pEye[1], boss/eye_r_hole/eye_r )
		REGISTER_MEMBER_TAGGED_PTR( m_pEyeLink[0], boss/eye_l_hole/link )
		REGISTER_MEMBER_TAGGED_PTR( m_pEyeLink[1], boss/eye_r_hole/link )
		REGISTER_MEMBER_TAGGED_PTR( m_pNose, boss/nose )
		REGISTER_MEMBER_TAGGED_PTR( m_pTongueHole, boss/tongue_hole )
		REGISTER_MEMBER_TAGGED_PTR( m_pTongue, boss/tongue_hole/tongue )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CLv1BossWorm1 )
		REGISTER_BASE_CLASS( CEnemy )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CLv1BossWorm2Seg )
		REGISTER_BASE_CLASS( CEnemyPart )
		REGISTER_MEMBER_TAGGED_PTR( m_pLayers[0], layer1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pLayers[1], layer2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pLayers[2], layer3 )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CLv1BossWorm2 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strSeg )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_strBullet2 )
		REGISTER_MEMBER( m_strLaser )
		REGISTER_MEMBER( m_strLaser1 )
		REGISTER_MEMBER( m_strDestroyEft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLv1BossBullet1 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_flyData )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSpike )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDoor )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CFallingObj )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_fGravity )
		REGISTER_MEMBER( m_fMaxSpeed )
		REGISTER_MEMBER( m_nHitDmg )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CFallingObjHolder )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pFallingObj, falling )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CFallingSpike )
		REGISTER_BASE_CLASS( CFallingObj )
		REGISTER_MEMBER( m_strBullet )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDetectTrigger )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_detectRect )
		REGISTER_MEMBER( m_detectRect1 )
		REGISTER_MEMBER( m_nCD )
		REGISTER_MEMBER( m_nFireCount )
		REGISTER_MEMBER( m_nFireCD )
		REGISTER_MEMBER( m_strPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CKillTrigger )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSpawner )
		REGISTER_BASE_CLASS( CDetectTrigger )
		REGISTER_MEMBER( m_nMaxCount )
		REGISTER_MEMBER( m_nTotalCount )
		REGISTER_MEMBER( m_nSpawnCount )
		REGISTER_MEMBER( m_bRandomRotate )
		REGISTER_MEMBER( m_rectSpawn )
		REGISTER_MEMBER( m_nVelocityType )
		REGISTER_MEMBER( m_vel1 )
		REGISTER_MEMBER( m_vel2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRandomSpawner )
		REGISTER_BASE_CLASS( CSpawner )
		REGISTER_MEMBER( m_strPrefab0 )
		REGISTER_MEMBER( m_strPrefab1 )
		REGISTER_MEMBER( m_strPrefab2 )
		REGISTER_MEMBER( m_strPrefab3 )
		REGISTER_MEMBER( m_fChances )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CKillSpawner )
		REGISTER_BASE_CLASS( CKillTrigger )
		REGISTER_MEMBER( m_bRandomRotate )
		REGISTER_MEMBER( m_bTangentRotate )
		REGISTER_MEMBER( m_rectSpawn )
		REGISTER_MEMBER( m_nVelocityType )
		REGISTER_MEMBER( m_vel1 )
		REGISTER_MEMBER( m_vel2 )
		REGISTER_MEMBER( m_nMinCount )
		REGISTER_MEMBER( m_nMaxCount )
		REGISTER_MEMBER( m_strPrefab0 )
		REGISTER_MEMBER( m_strPrefab1 )
		REGISTER_MEMBER( m_strPrefab2 )
		REGISTER_MEMBER( m_strPrefab3 )
		REGISTER_MEMBER( m_fChances )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CShop )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strItemDrop )
		REGISTER_MEMBER_TAGGED_PTR( m_pPickUpRoot, pickup )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPipe0 )
		REGISTER_BASE_CLASS( CDetectTrigger )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CWindow )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_openRect )
		REGISTER_MEMBER( m_closeRect )
		REGISTER_MEMBER_TAGGED_PTR( m_pWindow, window )
		REGISTER_MEMBER_TAGGED_PTR( m_pMan, man )
		REGISTER_MEMBER_TAGGED_PTR( m_pDeathEffect, deatheft );
		REGISTER_MEMBER_TAGGED_PTR( m_pSpawner, spawner );
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_strHead )
		REGISTER_MEMBER( m_strHead1 )
		REGISTER_MEMBER( m_strHead2 )
		REGISTER_MEMBER( m_strHead3 )
	REGISTER_CLASS_END()
			

	REGISTER_CLASS_BEGIN( CWindow2 )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_openRect )
		REGISTER_MEMBER( m_closeRect )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pWindow, window )
		REGISTER_MEMBER_TAGGED_PTR( m_pMan, man )
		REGISTER_MEMBER_TAGGED_PTR( m_pEye[0], man/eye1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEye[1], man/eye2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pHead[0], head1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pHead[1], head2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pLinks[0], link1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pLinks[1], link2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCarSpawner )
		REGISTER_BASE_CLASS( CDetectTrigger )
		REGISTER_MEMBER( m_carRect )
		REGISTER_MEMBER( m_carRect1 )
		REGISTER_MEMBER( m_spawnVel )
		REGISTER_MEMBER( m_pCarPrefabs )
		REGISTER_MEMBER( m_nSpawnCounts )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CHouseEntrance )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_spawnRect )
		REGISTER_MEMBER( m_spawnRect1 )
		REGISTER_MEMBER( m_nDir )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CManHead1 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_flyData )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CManHead2 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_flyData )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CManHead3 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_flyData )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CManHead4 )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strBullet )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRoach )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_creepData )
		REGISTER_MEMBER( m_fAIStepTimeMin )
		REGISTER_MEMBER( m_fAIStepTimeMax )
		REGISTER_MEMBER( m_nFireRate )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMaggot )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_moveData )
		REGISTER_MEMBER( m_fAIStepTimeMin )
		REGISTER_MEMBER( m_fAIStepTimeMax )
		REGISTER_MEMBER( m_fFallChance )
		REGISTER_MEMBER( m_nKnockbackTime )
		REGISTER_MEMBER_TAGGED_PTR( m_pExplosion, explosion )
		REGISTER_MEMBER( m_nExplosionLife )
		REGISTER_MEMBER( m_fExplosionDmg )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRat )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_walkData )
		REGISTER_MEMBER( m_flyData )
		REGISTER_MEMBER( m_strPrefab )
		REGISTER_MEMBER( m_nKnockbackTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCar )
		REGISTER_BASE_CLASS( CEnemy )
		REGISTER_MEMBER( m_moveData )
		REGISTER_MEMBER( m_fAcc )
		REGISTER_MEMBER( m_fHitDamage )
		REGISTER_MEMBER( m_nBurnHp )
		REGISTER_MEMBER( m_nBurnDamage )
		REGISTER_MEMBER( m_nBurnDamageInterval )
		REGISTER_MEMBER( m_strBurnEffect )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strExplosion )
		REGISTER_MEMBER( m_strMan )
		REGISTER_MEMBER_TAGGED_PTR( m_pDoors[0], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDoors[1], 2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDoors[2], 3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDoors[3], 4 )
		REGISTER_MEMBER( m_spawnManOfs )
		REGISTER_MEMBER( m_spawnManRadius )
		REGISTER_MEMBER( m_nSpawnManTime )
		REGISTER_MEMBER( m_fSpawnManSpeed )
		REGISTER_MEMBER( m_nExpType )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CThrowBox )
		REGISTER_BASE_CLASS( CThrowObj )
		REGISTER_MEMBER( m_pBullet )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSplashElem )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nBaseHeight )
		REGISTER_MEMBER( m_nScrollHeight )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSplash )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nBlendHeight )
		REGISTER_MEMBER( m_strElem )
		REGISTER_MEMBER( m_elemRect )
		REGISTER_MEMBER( m_nTileX )
		REGISTER_MEMBER( m_nTileY )
		REGISTER_MEMBER_TAGGED_PTR( m_pElemLayer, elems )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSplashRenderer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strSplash )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialStaticBeam )
		REGISTER_BASE_CLASS( CLightning )
		REGISTER_MEMBER( m_beginOfs )
		REGISTER_MEMBER( m_endOfs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialScreen )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_strEft )
		REGISTER_MEMBER_TAGGED_PTR( m_pTips, tips )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialChest )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER_TAGGED_PTR( m_e, e )
		REGISTER_MEMBER_TAGGED_PTR( m_pPickUp, pickup )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft1, pickup/eft1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft2, pickup/eft2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialEft )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_flyData )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialLevel )
		REGISTER_BASE_CLASS( CMyLevel )
		REGISTER_MEMBER( m_strSplash )
		REGISTER_MEMBER( m_strFloorEft )
		REGISTER_MEMBER_TAGGED_PTR( m_scroll[0], building/scroll00 )
		REGISTER_MEMBER_TAGGED_PTR( m_scroll[1], building/scroll10 )
		REGISTER_MEMBER_TAGGED_PTR( m_scroll[2], building/scroll20 )
		REGISTER_MEMBER_TAGGED_PTR( m_scroll1[0], building/scroll01 )
		REGISTER_MEMBER_TAGGED_PTR( m_scroll1[1], building/scroll11 )
		REGISTER_MEMBER_TAGGED_PTR( m_scroll1[2], building/scroll21 )
		REGISTER_MEMBER_TAGGED_PTR( m_pFloor, building/floor )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEffectObject )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[0], birth )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[1], stand )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[2], death )
		REGISTER_MEMBER( m_fBirthTime )
		REGISTER_MEMBER( m_fDeathTime )
		REGISTER_MEMBER( m_strSound )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDecorator )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDecoratorRandomTex )
		REGISTER_BASE_CLASS( CDecorator )
		REGISTER_MEMBER( m_texSize )
		REGISTER_MEMBER( m_fTexelSize )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDecorator9Patch )
		REGISTER_BASE_CLASS( CDecorator )
		REGISTER_MEMBER( m_fX1 )
		REGISTER_MEMBER( m_fX2 )
		REGISTER_MEMBER( m_fY1 )
		REGISTER_MEMBER( m_fY2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDecoratorFiber )
		REGISTER_BASE_CLASS( CDecorator )
		REGISTER_MEMBER( m_texSize )
		REGISTER_MEMBER( m_fTexelSize )
		REGISTER_MEMBER( m_fWidth )
		REGISTER_MEMBER( m_bVertical )
		REGISTER_MEMBER( m_nAlignment )
		REGISTER_MEMBER( m_fMaxHeightPercent )
		REGISTER_MEMBER( m_fMinHeightPercent )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDecoratorDirt )
		REGISTER_BASE_CLASS( CDecorator )
		REGISTER_MEMBER( m_texSize )
		REGISTER_MEMBER( m_fTexelSize )
		REGISTER_MEMBER( m_nMaxTexelSize )
		REGISTER_MEMBER( m_nMinTexelSize )
		REGISTER_MEMBER( m_fPercent )
		REGISTER_MEMBER( m_nMaskCols )
		REGISTER_MEMBER( m_nMaskRows )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDecoratorTile )
		REGISTER_BASE_CLASS( CDecorator )
		REGISTER_MEMBER( m_nTexCols )
		REGISTER_MEMBER( m_nTexRows )
		REGISTER_MEMBER( m_nTileSize )
		REGISTER_MEMBER( m_nTileBegin )
		REGISTER_MEMBER( m_nTileCount )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTexRectRandomModifier )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nCols )
		REGISTER_MEMBER( m_nRows )
		REGISTER_MEMBER( m_fWidth )
		REGISTER_MEMBER( m_fHeight )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CAnimFrameRandomModifier )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nFrameCount )
		REGISTER_MEMBER( m_nRandomCount )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CRopeAnimator )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nFrameCount )
		REGISTER_MEMBER( m_nFrameLen )
		REGISTER_MEMBER( m_bLoop )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSimpleText )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockRTEft )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nBeginFrame )
		REGISTER_MEMBER( m_nLife )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CStartPoint )
		REGISTER_BASE_CLASS( CEntity )
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

	REGISTER_CLASS_BEGIN( CMainMenu )
		REGISTER_BASE_CLASS( CRandomChunkTiled )
		REGISTER_MEMBER( m_strButton )
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

	REGISTER_CLASS_BEGIN( CParticleArrayEmitter )
		REGISTER_BASE_CLASS( IParticleEmitter )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_base )
		REGISTER_MEMBER( m_ofs )
		REGISTER_MEMBER( m_bIgnoreGlobalTransform )
	REGISTER_CLASS_END()

			
	REGISTER_CLASS_BEGIN( CChunkPreview )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChunkEdit )
		REGISTER_BASE_CLASS( CChunkPreview )
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[0], frame/00 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[1], frame/01 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[2], frame/02 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[3], frame/10 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[4], frame/12 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[5], frame/20 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[6], frame/21 );
		REGISTER_MEMBER_TAGGED_PTR( m_pFrameImg[7], frame/22 );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDesignLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strChunkEditPrefab )
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot[0], chunks );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot[1], chunks1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot[2], chunks2 );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkEditRoot[0], chunkedit );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkEditRoot[1], chunkedit1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkEditRoot[2], chunkedit2 );
	REGISTER_CLASS_END()
}

void InitGame()
{
	Game_ShaderImplement_Dummy();
	RegisterGameClasses();

	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );
}