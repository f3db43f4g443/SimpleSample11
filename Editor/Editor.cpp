#include "stdafx.h"
#include "Editor.h"
#include "Render/RenderSystem.h"
#include "Render/Scene2DManager.h"
#include "UICommon/UIFactory.h"
#include "Common/ResourceManager.h"
#include "Editors/FileExplorer.h"
#include "Editors/MaterialEditor.h"
#include "Editors/ParticleEditor.h"
#include "Editors/PrefabEditor.h"
#include "Common/FileUtil.h"

void CEditor::Start()
{
	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );

	CVector2 screenRes = IRenderSystem::Inst()->GetScreenRes();

	CUIManager* pUIManager = new CUIManager;
	m_pUIMgr = pUIManager;
	pUIManager->Resize( CRectangle( 0, 0, screenRes.x, screenRes.y ) );
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pUIManager );
	m_camera.SetViewport( 0, 0, screenRes.x, screenRes.y );
	m_camera.SetPosition( screenRes.x / 2, screenRes.y / 2 );
	m_camera.SetSize( screenRes.x, screenRes.y );
	CScene2DManager::GetGlobalInst()->AddActiveCamera( &m_camera, m_pUIMgr );
	CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_beforeRender );

	CFileExplorer* pFileEditor = CFileExplorer::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/fileexplorer.xml" )->GetElement()->Clone( pFileEditor );
	m_pUIMgr->AddChild( pFileEditor );

	CMaterialEditor* pMaterialEditor = CMaterialEditor::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/material_editor.xml" )->GetElement()->Clone( pMaterialEditor );
	pMaterialEditor->bVisible = false;
	m_pUIMgr->AddChild( pMaterialEditor );

	CParticleEditor* pParticleEditor = CParticleEditor::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/material_editor.xml" )->GetElement()->Clone( pParticleEditor );
	pParticleEditor->bVisible = false;
	m_pUIMgr->AddChild( pParticleEditor );

	CPrefabEditor* pPrefabEditor = CPrefabEditor::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/prefab_editor.xml" )->GetElement()->Clone( pPrefabEditor );
	pPrefabEditor->bVisible = false;
	m_pUIMgr->AddChild( pPrefabEditor );
}

void CEditor::Stop()
{
	if( m_beforeRender.IsRegistered() )
		m_beforeRender.Unregister();
	CScene2DManager::GetGlobalInst()->RemoveActiveCamera( &m_camera );
	m_pUIMgr->RemoveThis();
	m_pUIMgr = NULL;
}

void CEditor::Update()
{

}

void CEditor::OnResize( const CVector2& size )
{
	if( !m_pUIMgr )
		return;
	m_pUIMgr->Resize( CRectangle( 0, 0, size.x, size.y ) );
	m_camera.SetViewport( 0, 0, size.x, size.y );
	m_camera.SetPosition( size.x / 2, size.y / 2 );
	m_camera.SetSize( size.x, size.y );
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
		m_pUIMgr->HandleChar( 127 );
}

void CEditor::OnChar( uint32 nChar )
{
	m_pUIMgr->HandleChar( nChar );
}

void CEditor::SetEditor( CUIElement* pElem )
{
	if( m_pCurShownElem == pElem )
		return;
	if( m_pCurShownElem )
		m_pCurShownElem->SetVisible( false );
	m_pCurShownElem = pElem;
	if( pElem )
		pElem->SetVisible( true );
}