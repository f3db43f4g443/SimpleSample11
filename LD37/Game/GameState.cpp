#include "stdafx.h"
#include "GameState.h"
#include "MyGame.h"
#include "MyLevel.h"
#include "Render/Scene2DManager.h"
#include "Common/ResourceManager.h"
#include "UICommon/UIFactory.h"

CUIMgrGameState::CUIMgrGameState()
{
	CUIManager* pUIManager = new CUIManager;
	m_pUIMgr = pUIManager;
	pUIManager->Resize( CRectangle( 0, 0, 800, 600 ) );
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pUIManager );
}

void CUIMgrGameState::EnterState()
{
	CVector2 screenRes = IRenderSystem::Inst()->GetScreenRes();
	m_pUIMgr->Resize( CRectangle( 0, 0, screenRes.x, screenRes.y ) );

	m_camera.SetPosition( screenRes.x / 2, screenRes.y / 2 );
	m_camera.SetSize( screenRes.x, screenRes.y );
	CScene2DManager::GetGlobalInst()->AddActiveCamera( &m_camera, m_pUIMgr );
}

void CUIMgrGameState::ExitState()
{
	CScene2DManager::GetGlobalInst()->RemoveActiveCamera( &m_camera );
}

void CUIMgrGameState::HandleResize( const CVector2 & size )
{
	if( !m_pUIMgr )
		return;
	m_pUIMgr->Resize( CRectangle( 0, 0, size.x, size.y ) );
	m_camera.SetPosition( size.x / 2, size.y / 2 );
	m_camera.SetSize( size.x, size.y );
}

void CUIMgrGameState::HandleMouseDown( const CVector2 & pos )
{
	m_pUIMgr->HandleMouseDown( pos );
}

void CUIMgrGameState::HandleMouseUp( const CVector2 & pos )
{
	m_pUIMgr->HandleMouseUp( pos );
}

void CUIMgrGameState::HandleMouseMove( const CVector2 & mousePos )
{
	m_pUIMgr->HandleMouseMove( mousePos );
}

void CUIMgrGameState::HandleChar( uint32 nChar )
{
	m_pUIMgr->HandleChar( nChar );
}

CMainGameState::CMainGameState() : m_pWorld( NULL )
{
	CStageDirector* pStageDirector = CStageDirector::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/stage_director.xml" )->GetElement()->Clone( pStageDirector );
	m_pUIMgr->AddChild( pStageDirector );
}

void CMainGameState::EnterState()
{
	CUIMgrGameState::EnterState();

	m_pWorld = new CWorld;

	//CPointLightObject* pPointLight = new CPointLightObject( CVector4( 0.1f, 0, 500, -0.05f ), CVector3( 1, 1, 1 ), 10.0f, 0.2f, 0.4f );
	CPlayer* pPlayer = static_cast<CPlayer*>( CResourceManager::Inst()->CreateResource<CPrefab>( "player.pf" )->GetRoot()->CreateInstance() );
	m_pWorld->SetPlayer( pPlayer );
	auto pWeapon = SafeCast<CPlayerWeapon>( CResourceManager::Inst()->CreateResource<CPrefab>( "weapon.pf" )->GetRoot()->CreateInstance() );
	pPlayer->SetWeapon( pWeapon );

	SStageEnterContext context;
	context.strStartPointName = "start";
	m_pWorld->EnterStage( m_strStage.length() ? m_strStage.c_str() : "scene0.pf", context );
}

void CMainGameState::ExitState()
{
	CUIMgrGameState::ExitState();
	m_pWorld->Stop();
	delete m_pWorld;
}

void CMainGameState::UpdateInput()
{
	CGame* pGame = &CGame::Inst();
	if( CMyLevel::GetInst()->IsLevelDesignTest() )
	{
		if( pGame->IsKeyDown( VK_BACK ) )
		{
			CGame::Inst().SetCurState( &CLevelDesignGameState::Inst() );
			return;
		}
	}

	CPlayer* pPlayer = m_pWorld->GetPlayer();
	if( pPlayer )
	{
		CVector2 moveAxis;
		moveAxis.x = (int)pGame->IsKey( 'D' ) - (int)pGame->IsKey( 'A' );
		moveAxis.y = (int)pGame->IsKey( 'W' ) - (int)pGame->IsKey( 'S' );
		moveAxis.Normalize();
		pPlayer->Move( moveAxis.x, moveAxis.y );

		if( pGame->IsMouseDown() )
		{
			CMyLevel::GetInst()->Start();
			pPlayer->BeginFire();
		}
		if( pGame->IsMouseUp() )
			pPlayer->EndFire();

		if( pGame->IsKeyDown( ' ' ) )
			pPlayer->BeginRepair();
		if( pGame->IsKeyUp( ' ' ) )
			pPlayer->EndRepair();

		if( pGame->IsRightMouseDown() )
		{
			pPlayer->Roll();
		}
	}
}

void CMainGameState::UpdateFrame()
{
	m_pWorld->Update();
}

void CMainGameState::DelayResetStage()
{
	m_pWorld->GetPlayer()->DelayChangeStage( m_strStage.length() ? m_strStage.c_str() : "scene0.pf", "start" );
}