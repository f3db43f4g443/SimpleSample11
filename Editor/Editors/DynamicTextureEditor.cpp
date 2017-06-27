#include "stdafx.h"
#include "DynamicTextureEditor.h"
#include "Render/CommonShader.h"
#include "Common/Utf8Util.h"
#include "UICommon/UIFactory.h"

void CDynamicTextureEditor::NewFile( const char * szFileName )
{
	m_pWidth->SetValue( 256 );
	m_pHeight->SetValue( 256 );
	m_pPrefabName->SetText( "" );
	m_pBaseTexName->SetText( "" );
	m_pExportFileName->SetText( "" );
	Super::NewFile( szFileName );
}

void CDynamicTextureEditor::Refresh()
{
	if( m_pRes )
	{
		m_pWidth->SetValue( m_pRes->GetDesc().nWidth );
		m_pHeight->SetValue( m_pRes->GetDesc().nHeight );
		m_pPrefabName->SetText( m_pRes->GetDesc().strPrefab.c_str() );
		m_pBaseTexName->SetText( m_pRes->GetDesc().strBaseTex.c_str() );
		m_pExportFileName->SetText( m_pRes->GetDesc().strBaseTex.c_str() );
	}
}

void CDynamicTextureEditor::OnInited()
{
	Super::OnInited();
	auto pTreeView = GetChildByName<CUITreeView>( "view" );

	m_pWidth = CCommonEdit::Create( "Width" );
	pTreeView->AddContent( m_pWidth );
	m_pHeight = CCommonEdit::Create( "Height" );
	pTreeView->AddContent( m_pHeight );
	m_pPrefabName = CFileNameEdit::Create( "Prefab", "pf" );
	pTreeView->AddContent( m_pPrefabName );
	m_pBaseTexName = CFileNameEdit::Create( "Base Tex", "bmp;jpg;png;tga" );
	pTreeView->AddContent( m_pBaseTexName );
	m_pExportFileName = CCommonEdit::Create( "Export" );
	pTreeView->AddContent( m_pExportFileName );

	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/button.xml" );
	CUIButton* pButton = static_cast<CUIButton*>( g_pRes->GetElement()->Clone() );
	pButton->Resize( CRectangle( 0, 0, 75, 20 ) );
	pButton->SetText( "Export Texture" );
	pTreeView->AddContent( pButton );
	m_onExportBaseTex.Set( this, &CDynamicTextureEditor::ExportBaseTex );
	pButton->Register( eEvent_Action, &m_onExportBaseTex );

	m_onRefreshPreview.Set( this, &CDynamicTextureEditor::RefreshPreview );
	m_onSave.Set( this, &CDynamicTextureEditor::Save );
	pTreeView->GetChildByName( "refresh" )->Register( eEvent_Action, &m_onRefreshPreview );
	pTreeView->GetChildByName( "save" )->Register( eEvent_Action, &m_onSave );
}

void CDynamicTextureEditor::RefreshPreview()
{
	m_pRes->RefreshBegin();
	m_pRes->SetSize( m_pWidth->GetValue<int32>(), m_pHeight->GetValue<int32>() );

	if( !m_pRes->SetPrefab( UnicodeToUtf8( m_pPrefabName->GetText() ).c_str() ) )
		m_pPrefabName->SetText( m_pRes->GetDesc().strPrefab.c_str() );
	if( !m_pRes->SetBaseTex( UnicodeToUtf8( m_pBaseTexName->GetText() ).c_str() ) )
		m_pBaseTexName->SetText( m_pRes->GetDesc().strBaseTex.c_str() );
	m_pRes->RefreshEnd();
}

void CDynamicTextureEditor::OnDebugDraw( IRenderSystem * pRenderSystem )
{
	CRectangle viewport = m_pViewport->GetCamera().GetViewport();
	CRectangle dstRect = viewport;
	CVector2 srcSize = CVector2( m_pRes->GetDesc().nWidth, m_pRes->GetDesc().nHeight );
	dstRect.SetSize( srcSize );

	m_pRes->GetShaderResource();
	CopyToRenderTarget( pRenderSystem, NULL, m_pRes->GetTexture(), dstRect, CRectangle( 0, 0, srcSize.x, srcSize.y ), viewport.GetSize(), srcSize );
}

void CDynamicTextureEditor::ExportBaseTex()
{
	m_pRes->ExportTex( UnicodeToUtf8( m_pExportFileName->GetText() ).c_str() );
	m_pBaseTexName->SetText( m_pRes->GetDesc().strBaseTex.c_str() );
}
