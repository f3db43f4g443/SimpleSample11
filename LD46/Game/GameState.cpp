#include "stdafx.h"
#include "GameState.h"
#include "MyGame.h"
#include "MyLevel.h"
#include "Render/Scene2DManager.h"
#include "Common/ResourceManager.h"
#include "UICommon/UIFactory.h"
#include "GlobalCfg.h"
#include "GUI/StageDirector.h"
#include "GUI/Menu.h"

CUIMgrGameState::CUIMgrGameState()
{
	CUIManager* pUIManager = new CUIManager;
	m_pUIMgr = pUIManager;
	pUIManager->Resize( CRectangle( 0, 0, 1152, 720 ) );
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

	m_pWorld = new CWorld();
	m_nState = 0;
	m_pWorld->Start( "start.pf", 0 );
	auto pStartMenu = SafeCast<CStartMenu>( m_pWorld->GetCurStage()->GetRoot()->Get_ChildEntity() );
	pStartMenu->Init();
}

void CMainGameState::ExitState()
{
	CUIMgrGameState::ExitState();
	m_pWorld->Stop();
	delete m_pWorld;
}

void CMainGameState::UpdateInput()
{
}

void CMainGameState::UpdateFrame()
{
	if( m_nState == 0 )
	{
		auto pStartMenu = SafeCast<CStartMenu>( m_pWorld->GetCurStage()->GetRoot()->Get_ChildEntity() );
		auto n = pStartMenu->Update();
		if( n == eStartMenuResult_Continue )
		{
			m_nState = 1;
			m_pWorld->Start( "main.pf", 1 );
		}
		else if( n == eStartMenuResult_NewGame )
		{
			m_nState = 1;
			m_pWorld->Start( "main.pf", 0 );
		}
		else if( n == eStartMenuResult_Quit )
			exit( 0 );
	}
	m_pWorld->Update();
}
