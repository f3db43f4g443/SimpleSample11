#include "stdafx.h"
#include "Editor.h"
#include "Render/RenderSystem.h"
#include "Render/Scene2DManager.h"
#include "UICommon/UIFactory.h"
#include "Common/ResourceManager.h"
#include "Editors/FileExplorer.h"
#include "Editors/MaterialEditor.h"
#include "Editors/DynamicTextureEditor.h"
#include "Editors/ParticleEditor.h"
#include "Editors/PrefabEditor.h"
#include "Common/FileUtil.h"

CEditor::CEditor()
	: m_beforeRender( this, &CEditor::BeforeRender )
{
	_inst() = this;
	RegisterEditor( CMaterialEditor::Inst(), "EditorRes/UI/material_editor.xml", "Material(.mtl)", "mtl" );
	RegisterEditor( CParticleEditor::Inst(), "EditorRes/UI/material_editor.xml", "Particle System(.pts)", "pts" );
	RegisterEditor( CPrefabEditor::Inst(), "EditorRes/UI/prefab_editor.xml", "Prefab(.pf)", "pf" );
	RegisterEditor( CDynamicTextureEditor::Inst(), "EditorRes/UI/dynamic_texture_editor.xml", "Dynamic Texture(.dtx)", "dtx" );
}

void CEditor::Start()
{
	CResourceManager::Inst()->Register( new TResourceFactory<CUIResource>() );

	CVector2 screenRes = IRenderSystem::Inst()->GetScreenRes();

	CUIManager* pUIManager = new CUIManager;
	m_pUIMgr = pUIManager;
	pUIManager->Resize( CRectangle( 0, 0, screenRes.x, screenRes.y ) );
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pUIManager );
	m_camera.SetPosition( screenRes.x / 2, screenRes.y / 2 );
	m_camera.SetSize( screenRes.x, screenRes.y );
	CScene2DManager::GetGlobalInst()->AddActiveCamera( &m_camera, m_pUIMgr );
	CScene2DManager::GetGlobalInst()->Register( CScene2DManager::eEvent_BeforeRender, &m_beforeRender );

	CFileExplorer* pFileEditor = CFileExplorer::Inst();
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/fileexplorer.xml" )->GetElement()->Clone( pFileEditor );
	m_pUIMgr->AddChild( pFileEditor );

	m_pDefaultToolResource = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/tool_default.xml" );

	for( auto& item : m_mapRegisteredEditors )
	{
		auto pEditor = item.second.pEditor;
		CResourceManager::Inst()->CreateResource<CUIResource>( item.second.strPath.c_str() )->GetElement()->Clone( pEditor );
		pEditor->bVisible = false;
		m_pUIMgr->AddChild( pEditor );
	}
	PostInit();
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
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	double dLastTime = pRenderSystem->GetLastTime();
	double dTotalTime = pRenderSystem->GetTotalTime();
	for( auto pObj = CScene2DManager::GetGlobalInst()->Get_AutoUpdateAnimObject(); pObj; pObj = pObj->NextAutoUpdateAnimObject() )
	{
		pObj->UpdateAnim( dTotalTime - dLastTime );
	}
	m_pUIMgr->OnUpdate();
}

void CEditor::PostInit()
{
	RegisterToolDefault( 0, [] () { CPrefabEditor::ExportAllText(); } );
	RegisterToolDefault( 1, [] () { CPrefabEditor::ImportAllText(); } );
}

void CEditor::OnResize( const CVector2& size )
{
	if( !m_pUIMgr )
		return;
	m_pUIMgr->Resize( CRectangle( 0, 0, size.x, size.y ) );
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

void CEditor::OnMouseWheel( int32 nDelta )
{
	m_pUIMgr->HandleMouseWheel( nDelta );
}

void CEditor::OnKey( uint32 nChar, bool bKeyDown, bool bAltDown )
{
	m_pUIMgr->HandleKey( nChar, bKeyDown, bAltDown );
	if( nChar == VK_DELETE && bKeyDown )
		m_pUIMgr->HandleChar( 127 );
	if( nChar >= 128 )
		return;
}

void CEditor::OnChar( uint32 nChar )
{
	m_pUIMgr->HandleChar( nChar );
}

void CEditor::RegisterToolDefault( int32 nIcon, function<void()> funcHandler )
{
	auto pIcon = static_cast<CUILabel*>( m_pDefaultToolResource->GetElement()->Clone() );
	pIcon->AddImage( 0, "EditorRes/Drawables/icons.xml", CRectangle( 0, 0, 32, 32 ),
		CRectangle( nIcon % 16, nIcon / 16, 0.0625, 0.0625 ), 0 );
	auto pHandler = new CFunctionTrigger( funcHandler );
	pHandler->bAutoDelete = true;
	pIcon->Register( CUIElement::eEvent_Action, pHandler );
}

void CEditor::SetEditor( CResourceEditor* pElem )
{
	if( m_pCurShownElem == pElem )
		return;
	if( m_pCurShownElem )
		m_pCurShownElem->SetVisible( false );
	m_pCurShownElem = pElem;
	if( pElem )
		pElem->SetVisible( true );
}

CResourceEditor* CEditor::SetEditor( const char* szTag )
{
	auto itr = m_mapRegisteredEditors.find( szTag );
	if( itr != m_mapRegisteredEditors.end() )
	{
		SetEditor( itr->second.pEditor );
		return itr->second.pEditor;
	}
	return NULL;
}

void CEditor::OpenFile( const char* szFile, const char* szParam )
{
	if( !szFile[0] )
		return;
	if( !IsFileExist( szFile ) )
		return;

	string strFile = szFile;
	string strParam = szParam;
	const char* szExt = GetFileExtension( strFile.c_str() );
	auto pEditor = SetEditor( szExt );
	if( pEditor )
		pEditor->SetFileName( strFile.c_str(), strParam.c_str() );
	else if( !strcmp( szExt, "wav" ) || !strcmp( szExt, "mp3" ) || !strcmp( szExt, "sf" ) )
	{
		ISoundTrack* pSoundTrack = CResourceManager::Inst()->CreateResource<CSoundFile>( strFile.c_str() )->CreateSoundTrack();
		pSoundTrack->Play( ESoundPlay_KeepRef );
	}
}
