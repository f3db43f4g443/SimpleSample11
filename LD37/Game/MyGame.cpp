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

#include "Entities/Enemies.h"
#include "Entities/EnemyCharacters.h"
#include "Entities/Neutral.h"
#include "Entities/Door.h"
#include "Entities/FallingObj.h"
#include "Entities/StartPoint.h"
#include "Entities/GlitchEffect.h"
#include "Entities/EffectObject.h"
#include "Entities/Barrage.h"
#include "GUI/MainUI.h"
#include "GUI/ChunkUI.h"

#include "Bullet.h"
#include "Lightning.h"
#include "Weapons.h"
#include "Pickup.h"

#include "Effects/ParticleTimeoutEmitter.h"
#include "Effects/ParticleSubEmitter.h"
#include "Effects/ParticleArrayEmitter.h"


CGame::CGame()
	: m_pWorld( NULL )
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
	m_screenResolution = CVector2( 1280, 960 );
}

void CGame::Start()
{
	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );

	CGlobalCfg::Inst().Load();

	CVector2 screenRes = IRenderSystem::Inst()->GetScreenRes();
	CUIManager* pUIManager = new CUIManager;
	m_pUIMgr = pUIManager;
	pUIManager->Resize( CRectangle( 0, 0, 800, 600 ) );
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pUIManager );

	CStageDirector* pStageDirector = CStageDirector::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/stage_director.xml" )->GetElement()->Clone( pStageDirector );
	m_pUIMgr->AddChild( pStageDirector );
	pUIManager->Resize( CRectangle( 0, 0, screenRes.x, screenRes.y ) );

	m_camera.SetPosition( screenRes.x / 2, screenRes.y / 2 );
	m_camera.SetSize( screenRes.x, screenRes.y );
	CScene2DManager::GetGlobalInst()->AddActiveCamera( &m_camera, m_pUIMgr );
	CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_beforeRender );

	m_pWorld = new CWorld;

	//CPointLightObject* pPointLight = new CPointLightObject( CVector4( 0.1f, 0, 500, -0.05f ), CVector3( 1, 1, 1 ), 10.0f, 0.2f, 0.4f );
	CPlayer* pPlayer = static_cast<CPlayer*>( CResourceManager::Inst()->CreateResource<CPrefab>( "player.pf" )->GetRoot()->CreateInstance() );
	m_pWorld->SetPlayer( pPlayer );
	auto pWeapon = SafeCast<CPlayerWeapon>( CResourceManager::Inst()->CreateResource<CPrefab>( "weapon.pf" )->GetRoot()->CreateInstance() );
	pPlayer->SetWeapon( pWeapon );

	SStageEnterContext context;
	context.strStartPointName = "start";
	m_pWorld->EnterStage( "scene0.pf", context );
}

void CGame::Stop()
{
	m_pWorld->Stop();
	m_trigger.Clear();
	delete m_pWorld;
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

	CPlayer* pPlayer = m_pWorld->GetPlayer();
	if( pPlayer )
	{
		CVector2 moveAxis;
		moveAxis.x = (int)m_key.GetBit( 'D' ) - (int)m_key.GetBit( 'A' );
		moveAxis.y = (int)m_key.GetBit( 'W' ) - (int)m_key.GetBit( 'S' );
		moveAxis.Normalize();
		pPlayer->Move( moveAxis.x, moveAxis.y );

		if( m_bIsMouseDown )
		{
			CMyLevel::GetInst()->Start();
			pPlayer->BeginFire();
		}
		if( m_bIsMouseUp )
			pPlayer->EndFire();

		if( m_keyDown.GetBit( ' ' ) )
			pPlayer->BeginRepair();
		if( m_keyUp.GetBit( ' ' ) )
			pPlayer->EndRepair();

		if( m_bIsRightMouseDown )
		{
			pPlayer->Roll();
		}
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

	m_keyDown.Clear();
	m_keyUp.Clear();
	m_bIsMouseDown = m_bIsMouseUp = false;
	m_bIsRightMouseDown = m_bIsRightMouseUp = false;
}

void CGame::OnResize( const CVector2& size )
{
	if( !m_pUIMgr )
		return;
	m_pUIMgr->Resize( CRectangle( 0, 0, size.x, size.y ) );
	m_camera.SetPosition( size.x / 2, size.y / 2 );
	m_camera.SetSize( size.x, size.y );
}

void CGame::OnMouseDown( const CVector2& pos )
{
	if( !m_bIsMouse )
	{
		m_bIsMouse = true;
		m_bIsMouseDown = true;
	}
	m_pUIMgr->HandleMouseDown( pos );
}

void CGame::OnMouseUp( const CVector2& pos )
{
	if( m_bIsMouse )
	{
		m_bIsMouse = false;
		m_bIsMouseUp = true;
	}
	m_pUIMgr->HandleMouseUp( pos );
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
	m_pUIMgr->HandleMouseMove( pos );
}

void CGame::OnKey( uint32 nChar, bool bKeyDown, bool bAltDown )
{
	if( nChar == VK_DELETE && bKeyDown )
		m_pUIMgr->HandleChar( 127 );
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

void CGame::OnChar( uint32 nChar )
{
	m_pUIMgr->HandleChar( nChar );
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
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_nSpawnHeight )
		REGISTER_MEMBER( m_nBlockSize )
		REGISTER_MEMBER( m_fFallDistPerSpeedFrame )
		REGISTER_MEMBER_TAGGED_PTR( m_pChunkRoot, chunks );
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
		REGISTER_MEMBER_TAGGED_PTR( m_pDeathEffect, deatheft );
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

	REGISTER_CLASS_BEGIN( CRandomChunk1 )
		REGISTER_BASE_CLASS( CChunkObject )
		REGISTER_MEMBER( m_nHpPerSize )
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
}

void InitGame()
{
	Game_ShaderImplement_Dummy();
	RegisterGameClasses();
}