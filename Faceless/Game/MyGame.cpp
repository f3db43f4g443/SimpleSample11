#include "stdafx.h"
#include "MyGame.h"
#include "Scene2DManager.h"
#include "FileUtil.h"
#include "xml.h"
#include "Player.h"
#include "GUI/StageDirector.h"
#include "UICommon/UIFactory.h"
#include "ClassMetaData.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "Face.h"
#include "GlobalCfg.h"

#include "Entities/StartPoint.h"
#include "Entities/GlitchEffect.h"
#include "Entities/EffectObject.h"

#include "Entities/OrganHpBar.h"

#include "Entities/Bullet.h"
#include "Entities/OrganActionSimpleShoot.h"
#include "Entities/OrganTargetorSimpleShoot.h"

#include "Effects/ParticleTimeoutEmitter.h"
#include "Effects/ParticleSubEmitter.h"

#include "SkinNMask.h"

CGame::CGame()
	: m_pWorld( NULL )
	, m_key( 128 )
	, m_keyDown( 128 )
	, m_keyUp( 128 )
	, m_beforeRender( this, &CGame::BeforeRender )
{
	m_screenResolution = CVector2( 800, 600 );
}

void CGame::Start()
{
	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );

	CGlobalCfg::Inst().Load();
	CSkinNMaskCfg::Inst().Load();
	COrganCfg::Inst().Load();

	CVector2 screenRes = IRenderSystem::Inst()->GetScreenRes();
	CUIManager* pUIManager = new CUIManager;
	m_pUIMgr = pUIManager;
	pUIManager->Resize( CRectangle( 0, 0, screenRes.x, screenRes.y ) );
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pUIManager );
	m_camera.SetPosition( screenRes.x / 2, screenRes.y / 2 );
	m_camera.SetSize( screenRes.x, screenRes.y );
	CScene2DManager::GetGlobalInst()->AddActiveCamera( &m_camera, m_pUIMgr );
	CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_beforeRender );

	CStageDirector* pStageDirector = CStageDirector::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/stage_director.xml" )->GetElement()->Clone( pStageDirector );
	m_pUIMgr->AddChild( pStageDirector );

	//CMainUI::Inst()->InitResources();
	m_pWorld = new CWorld;

	//CPointLightObject* pPointLight = new CPointLightObject( CVector4( 0.1f, 0, 500, -0.05f ), CVector3( 1, 1, 1 ), 10.0f, 0.2f, 0.4f );
	CPlayer* pPlayer = static_cast<CPlayer*>( CResourceManager::Inst()->CreateResource<CPrefab>( "data/lv0/player.pf" )->GetRoot()->CreateInstance() );
	m_pWorld->SetPlayer( pPlayer );

	pPlayer->AddFaceEditItem( CSkinNMaskCfg::Inst().GetSkin( "Skin0" ) );
	pPlayer->AddFaceEditItem( COrganCfg::Inst().GetOrganEditItem( "Organ0" ) );
	pPlayer->AddFaceEditItem( COrganCfg::Inst().GetOrganEditItem( "Organ1" ) );

	SStageEnterContext context;
	context.strStartPointName = "start1";
	m_pWorld->EnterStage( "data/lv0/scene0.pf", context );
	//CMainUI::Inst()->SetVisible( true );

	auto pLevel = CMyLevel::GetInst();
	auto pCharacterPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( "data/lv0/boss.pf" );
	auto pCharacter = static_cast<CCharacter*>( pCharacterPrefab->GetRoot()->CreateInstance() );
	pLevel->AddCharacter( pCharacter, 12, 1 );
	pCharacter = static_cast<CCharacter*>( pCharacterPrefab->GetRoot()->CreateInstance() );
	pLevel->AddCharacter( pCharacter, 20, 1 );
	pCharacter = static_cast<CCharacter*>( pCharacterPrefab->GetRoot()->CreateInstance() );
	pLevel->AddCharacter( pCharacter, 12, 8 );
	pCharacter = static_cast<CCharacter*>( pCharacterPrefab->GetRoot()->CreateInstance() );
	pLevel->AddCharacter( pCharacter, 20, 8 );
	pCharacter = static_cast<CCharacter*>( pCharacterPrefab->GetRoot()->CreateInstance() );
	pLevel->AddCharacter( pCharacter, 16, 8 );
}

void CGame::Stop()
{
	//CMainUI::Inst()->SetVisible( false );
	
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
		if( m_keyDown.GetBit( 'A' ) )
			pPlayer->PlayerCommandMove( 0 );
		else if( m_keyDown.GetBit( 'S' ) )
			pPlayer->PlayerCommandMove( 1 );
		else if( m_keyDown.GetBit( 'D' ) )
			pPlayer->PlayerCommandMove( 2 );
		else if( m_keyDown.GetBit( 'W' ) )
			pPlayer->PlayerCommandMove( 3 );

		if( m_keyDown.GetBit( ' ' ) )
			pPlayer->PlayerCommandEndPhase();
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
	m_pUIMgr->HandleMouseDown( pos );
}

void CGame::OnMouseUp( const CVector2& pos )
{
	m_pUIMgr->HandleMouseUp( pos );
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
	REGISTER_ENUM_BEGIN( ERangeType )
		REGISTER_ENUM_ITEM( eRangeType_Normal )
	REGISTER_ENUM_END()

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
	
	REGISTER_CLASS_BEGIN( CCharacter )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strSubStageName )
		REGISTER_MEMBER( m_strFaceDataName )
		REGISTER_MEMBER( m_nMaxHp )
		REGISTER_MEMBER( m_nMaxMp )
		REGISTER_MEMBER( m_nMaxSp )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayer )
		REGISTER_BASE_CLASS( CCharacter )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CMyLevel )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_ENUM_BEGIN( ETargetType )
		REGISTER_ENUM_ITEM( eTargetType_None )
		REGISTER_ENUM_ITEM( eTergetType_Pos )
		REGISTER_ENUM_ITEM( eTargetType_Character )
	REGISTER_ENUM_END()
	
	REGISTER_CLASS_BEGIN( COrgan )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strOrganAction )
		REGISTER_MEMBER( m_strOrganTargetor )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_nInnerX )
		REGISTER_MEMBER( m_nInnerY )
		REGISTER_MEMBER( m_nInnerWidth )
		REGISTER_MEMBER( m_nInnerHeight )
		REGISTER_MEMBER( m_nMaxHp )
		REGISTER_MEMBER( m_nCost )
		REGISTER_MEMBER( m_nRangeType )
		REGISTER_MEMBER( m_nTargetType )
		REGISTER_MEMBER( m_nRange )
		REGISTER_MEMBER( m_nRange1 )
		REGISTER_MEMBER( m_bRangeExcludeSelf )
		REGISTER_MEMBER( m_nFramesRowCount )
		REGISTER_MEMBER( m_nFramesColumnCount )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( COrganAction )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( COrganTargetor )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( COrganHpBar )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBulletBase )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBullet )
		REGISTER_BASE_CLASS( CBulletBase )
		REGISTER_MEMBER( m_nDmg )
		REGISTER_MEMBER( m_fSpeed )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMissile )
		REGISTER_BASE_CLASS( CBulletBase )
		REGISTER_MEMBER( m_nDmg )
		REGISTER_MEMBER( m_fSpeed )
		REGISTER_MEMBER( m_eRangeType )
		REGISTER_MEMBER( m_nRange )
		REGISTER_MEMBER( m_nRange1 )
		REGISTER_MEMBER( m_bCanDmgOrgan )
		REGISTER_MEMBER( m_bCanDmgSkin )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( COrganActionSimpleShoot )
		REGISTER_BASE_CLASS( COrganAction )
		REGISTER_MEMBER( m_strBulletPrefab )
		REGISTER_MEMBER( m_nCount )
		REGISTER_MEMBER( m_fInterval )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( COrganTargetorSimpleShoot )
		REGISTER_BASE_CLASS( COrganTargetor )
		REGISTER_MEMBER( m_fFlyingSpeed )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CFace )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strDefaultSkinName )
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

}

void InitGame()
{
	Game_ShaderImplement_Dummy();
	RegisterGameClasses();

	CResourceManager::Inst()->Register( new TResourceFactory<CFaceData>() );
	CResourceManager::Inst()->RegisterExtension<CFaceData>( "f" );
}