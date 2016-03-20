#include "stdafx.h"
#include <Windows.h>
#include "Editor.h"
#include "EditorRenderer.h"
#include "Render/RenderSystem.h"
#include "Render/Scene2DManager.h"
#include "UI/UIFactory.h"
#include "Common/ResourceManager.h"

void CEditor::Start()
{
	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );

	CUIManager* pUIManager = new CUIManager;
	m_pUIMgr = pUIManager;
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pUIManager );
	m_camera.SetViewport( 0, 0, 800, 600 );
	m_camera.SetPosition( 400, 300 );
	m_camera.SetSize( 800, 600 );
	CScene2DManager::GetGlobalInst()->AddActiveCamera( &m_camera, m_pUIMgr );

	m_pUIMgr->AddChild( CResourceManager::Inst()->CreateResource<CUIResource>( "main.xml" )->GetElement()->Clone() );
}

void CEditor::Stop()
{
	CScene2DManager::GetGlobalInst()->RemoveActiveCamera( &m_camera );
	m_pUIMgr->RemoveThis();
	m_pUIMgr = NULL;
}

void CEditor::Update()
{

}

void CEditor::OnMouseDown( const CVector2& pos )
{
	m_pUIMgr->HandleMouseDown( pos );
}

void CEditor::OnMouseUp( const CVector2& pos )
{
	m_pUIMgr->HandleMouseUp( pos );
}

void CEditor::OnMouseMove( const CVector2& pos )
{
	m_pUIMgr->HandleMouseMove( pos );
}


void CEditor::OnKey( uint32 nChar, bool bKeyDown, bool bAltDown )
{
	if( nChar == VK_DELETE && bKeyDown )
		m_pUIMgr->HandleChar( nChar );
}

void CEditor::OnChar( uint32 nChar )
{
	m_pUIMgr->HandleChar( nChar );
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	pRenderSystem->SetRenderer( new CEditorRenderer );
	pRenderSystem->SetGame( &CEditor::Inst() );
	SDeviceCreateContext context;
	context.resolution = CVector2( 800, 600 );
	pRenderSystem->CreateDevice( context );
	pRenderSystem->Start();
	exit( 0 );
}