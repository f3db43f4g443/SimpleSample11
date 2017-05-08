#include "stdafx.h"
#include "MyGame.h"
#include "Scene2DManager.h"
#include "FileUtil.h"
#include "xml.h"
#include "Player.h"
#include "Enemy.h"
#include "Block.h"
#include "BlockItem.h"
#include "CharacterMove.h"
#include "GUI/StageDirector.h"
#include "UICommon/UIFactory.h"
#include "ClassMetaData.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "GlobalCfg.h"
#include "LevelDesign.h"

#include "Entities/Enemies.h"
#include "Entities/EnemyCharacters.h"
#include "Entities/Neutral.h"
#include "Entities/Door.h"
#include "Entities/FallingObj.h"
#include "Entities/StartPoint.h"
#include "Entities/GlitchEffect.h"
#include "Entities/EffectObject.h"
#include "Entities/Barrage.h"
#include "Entities/Blocks/RandomBlocks.h"
#include "Entities/Blocks/SpecialBlocks.h"
#include "Entities/Blocks/LvBarriers.h"
#include "Entities/Blocks/lv1/SpecialLv1.h"
#include "Entities/BlockItems/BlockItemsLv1.h"
#include "Entities/Bullets.h"
#include "Entities/BlockBuffs.h"
#include "Entities/Enemies/Lv1Enemies.h"
#include "GUI/MainUI.h"
#include "GUI/ChunkUI.h"

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
		m_trigger.UpdateTime();
		for( auto pObj = CScene2DManager::GetGlobalInst()->Get_AutoUpdateAnimObject(); pObj; pObj = pObj->NextAutoUpdateAnimObject() )
		{
			pObj->UpdateAnim( fInvFPS );
		}
		if( m_pCurGameState )
			m_pCurGameState->UpdateFrame();
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
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayer )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_walkData )
		REGISTER_MEMBER( m_flyData )
		REGISTER_MEMBER_TAGGED_PTR( m_pCore, core );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CEnemy )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nHp )
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
		REGISTER_MEMBER( m_strPrefab )
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
		REGISTER_MEMBER( m_nBlockSize )
		REGISTER_MEMBER( m_fFallDistPerSpeedFrame )
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot, chunks );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot1, chunks1 );
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkEffectRoot, chunkeffects );
		REGISTER_MEMBER_TAGGED_PTR( m_pClickToStart, start/clicktostart );
		REGISTER_MEMBER_TAGGED_PTR( m_pCrosshair, crosshair );
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMainUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBar, hp );
		REGISTER_MEMBER_TAGGED_PTR( m_pShake, shake );
		REGISTER_MEMBER_TAGGED_PTR( m_pMinimap, minimap );
		REGISTER_MEMBER_TAGGED_PTR( m_pShakeSmallBars[0], minimap/shake );
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
		REGISTER_MEMBER_TAGGED_PTR( m_pDeathEffect, deatheft );
		REGISTER_MEMBER_TAGGED_PTR( m_pParticle, particle );
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CBulletWithBlockBuff )
		REGISTER_BASE_CLASS( CBullet )
		REGISTER_MEMBER( m_strBlockBuff )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CExplosion )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_nHitBeginFrame )
		REGISTER_MEMBER( m_nHitFrameCount )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDeltaDamage )
		REGISTER_MEMBER( m_fInitRange )
		REGISTER_MEMBER( m_fDeltaRange )
		REGISTER_MEMBER( m_bHitPlayer )
		REGISTER_MEMBER( m_bHitEnemy )
		REGISTER_MEMBER( m_bHitNeutral )
		REGISTER_MEMBER( m_bHitBlock )
		REGISTER_MEMBER( m_bHitWall )
		REGISTER_MEMBER( m_bHitHidingPlayer )
		REGISTER_MEMBER( m_bHitHidingEnemy )
		REGISTER_MEMBER( m_bHitHidingNeutral )
		REGISTER_MEMBER( m_bHitCreator )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CExplosionWithBlockBuff )
		REGISTER_BASE_CLASS( CExplosion )
		REGISTER_MEMBER( m_strBlockBuff )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickUp )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pDeathEffect, deatheft );
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPickUpCommon )
		REGISTER_BASE_CLASS( CPickUp )
		REGISTER_MEMBER( m_nHpRestore )
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
		REGISTER_MEMBER( m_fWidth )
		REGISTER_MEMBER( m_fHitWidth )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerWeapon )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_texRectFaceRight )
		REGISTER_MEMBER( m_texRectFaceLeft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerWeaponShoot0 )
		REGISTER_BASE_CLASS( CPlayerWeapon )
		REGISTER_MEMBER( m_strBulletName )
		REGISTER_MEMBER( m_fSpeed )
		REGISTER_MEMBER( m_nFireRate )
		REGISTER_MEMBER( m_fireOfs )
		REGISTER_MEMBER( m_nBulletLife )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlockObject )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChunkObject )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strEffect )
		REGISTER_MEMBER( m_nMaxHp )
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
		REGISTER_MEMBER( m_strBullet1 )
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

	REGISTER_CLASS_BEGIN( CRandomChunkTiled )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
		REGISTER_MEMBER( m_bBlockTypeMask )
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

	REGISTER_CLASS_BEGIN( CLvBarrier1Core )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
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

	REGISTER_CLASS_BEGIN( CBlockBuff )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_bMulti )
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
		REGISTER_MEMBER( m_strPrefab )
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
		REGISTER_MEMBER( m_strBullet )
		REGISTER_MEMBER( m_strBullet1 )
		REGISTER_MEMBER( m_strHead )
		REGISTER_MEMBER( m_strHead1 )
		REGISTER_MEMBER( m_strHead2 )
		REGISTER_MEMBER( m_strHead3 )
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

	REGISTER_CLASS_BEGIN( CEffectObject )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[0], birth )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[1], stand )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[2], death )
		REGISTER_MEMBER( m_fBirthTime )
		REGISTER_MEMBER( m_fDeathTime )
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