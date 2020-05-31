#include "stdafx.h"
#include "MyGame.h"
#include "Scene2DManager.h"
#include "FileUtil.h"
#include "xml.h"
#include "UICommon/UIFactory.h"
#include "ClassMetaData.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "Common/DateTime.h"
#include "MyLevel.h"
#include "GlobalCfg.h"

#include "Entities/EffectObject.h"

CGame::CGame()
	: m_bStarted( false )
	, m_bRestart( false )
	, m_pCurGameState( NULL )
	, m_key( 128 )
	, m_keyDown( 128 )
	, m_keyUp( 128 )
	, m_keyHolding( 128 )
	, m_char( 128 )
	, m_char1( 128 )
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
	m_screenRes = IRenderSystem::Inst()->GetScreenRes();
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
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	double dLastTime = pRenderSystem->GetLastTime();
	double dTotalTime = pRenderSystem->GetTotalTime();
	m_dTotalTime = dTotalTime;
	const uint32 nFPS = 60;
	const float fInvFPS = 1.0f / nFPS;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );

	if( m_pCurGameState )
		m_pCurGameState->UpdateInput();

	for( int i = 0; i < nFrames && !m_bRestart; i++ )
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
		m_char.Clear();
		m_bIsMouseDown = m_bIsMouseUp = false;
		m_bIsRightMouseDown = m_bIsRightMouseUp = false;
	}
	if( m_bRestart )
	{
		SetCurState( NULL );
		m_trigger.Clear();
		SetCurState( &CMainGameState::Inst() );
		m_bRestart = false;
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
		m_keyHolding.SetBit( nChar, false );
		m_char1.SetBit( nChar, false );
		m_keyUpTime[nChar] = GetCycleCount();
	}
	m_key.SetBit( nChar, bKeyDown );
	if( ( nChar == VK_UP || nChar == VK_DOWN || nChar == VK_LEFT || nChar == VK_RIGHT ) && bKeyDown )
		OnChar( nChar );
}

void CGame::OnChar( uint32 nChar )
{
	if( nChar < 128 )
	{
		if( nChar >= 'a' && nChar <= 'z' )
			nChar -= 'a' - 'A';
		m_char.SetBit( nChar, true );
		if( !m_char1.GetBit( nChar ) )
			m_char1.SetBit( nChar, true );
		else
			m_keyHolding.SetBit( nChar, true );
	}
	if( m_pCurGameState && m_bStarted )
		m_pCurGameState->HandleChar( nChar );
}

void Game_ShaderImplement_Dummy();

void RegisterGameClasses_World();
void RegisterGameClasses_BasicElems();
void RegisterGameClasses_Level();
void RegisterGameClasses_MiscElem();
void RegisterGameClasses_UtilEntities();
void RegisterGameClasses_PawnAI();
void RegisterGlobalLuaCFunc();
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
		REGISTER_DIFF_FUNC( DiffData )
		REGISTER_PATCH_FUNC( PatchData )
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
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetVisible )
		REGISTER_LUA_CFUNCTION( FindChildEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEffectObject )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[0], birth )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[1], stand )
		REGISTER_MEMBER_TAGGED_PTR( m_pStates[2], death )
		REGISTER_MEMBER( m_fBirthTime )
		REGISTER_MEMBER( m_fDeathTime )
	REGISTER_CLASS_END()

	RegisterGameClasses_World();
	RegisterGameClasses_BasicElems();
	RegisterGameClasses_Level();
	RegisterGameClasses_MiscElem();
	RegisterGameClasses_UtilEntities();
	RegisterGameClasses_PawnAI();
	RegisterGlobalLuaCFunc();
}

void InitGame()
{
	Game_ShaderImplement_Dummy();
	RegisterGameClasses();

	CResourceManager::Inst()->Register( new TResourceFactory<CWorldCfgFile>() );
	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );
}