#include "stdafx.h"
#include "FaceEditor.h"
#include "UICommon/UIFactory.h"
#include "Editor/Editors/UIComponentUtil.h"

void CFaceEditor::NewFile( const char * szFileName )
{
	SetSelectedEditItem( NULL );
	if( m_pFace )
	{
		m_pFace->SetParentEntity( NULL );
		m_pFace = NULL;
	}
	if( m_pStage )
	{
		m_pStage->Stop();
		delete m_pStage;
		m_pStage = NULL;
	}

	Super::NewFile( szFileName );
}

void CFaceEditor::Refresh()
{
	if( m_pFace )
	{
		m_pFace->SetParentEntity( NULL );
		m_pFace = NULL;
	}
	if( m_pStage )
	{
		m_pStage->Stop();
		delete m_pStage;
		m_pStage = NULL;
	}
	m_pEditPanel->SetVisible( false );
	m_pCreatePanel->SetVisible( false );

	if( m_pRes )
	{
		if( m_pRes->GetPrefab() )
		{
			m_pStage = new CStage( NULL );
			SStageEnterContext context;
			context.pViewport = m_pViewport;
			m_pStage->Start( NULL, context );

			m_pFace = static_cast<CFace*>( m_pRes->GetPrefab()->GetRoot()->CreateInstance() );
			m_pFace->SetParentEntity( m_pStage->GetRoot() );
			m_pFace->LoadExtraData( m_pRes->GetData() );

			m_pEditPanel->SetVisible( true );
		}
		else
		{
			m_pCreatePanel->SetVisible( true );
		}
	}
}

void CFaceEditor::OnInited()
{
	Super::OnInited();

	m_pCreatePanel = GetChildByName<CUIElement>( "create" );
	m_pEditPanel = GetChildByName<CUIElement>( "edit" );
	m_pToolView = m_pEditPanel->GetChildByName<CUITreeView>( "toolbox" );

	m_onSave.Set( this, CFaceEditor::Save );
	m_pToolView->GetChildByName<CUIButton>( "save" )->Register( eEvent_Action, &m_onSave );
}

void CFaceEditor::CreateViewport()
{
	m_pFaceView = CFaceView::Create( GetChildByName<CUIViewport>( "viewport" ) );
	m_pFaceView->SetState( CFaceView::eState_Edit );
	m_pViewport = m_pFaceView;
}

void CFaceEditor::RefreshPreview()
{
	if( !m_pRes )
		return;
	m_pRes->RefreshBegin();
	m_pRes->SetPrefab( m_pFacePrefab );
	m_pRes->GetData().Clear();
	if( m_pFace )
		m_pFace->SaveExtraData( m_pRes->GetData() );
	m_pRes->RefreshEnd();
}

void CFaceEditor::OnDebugDraw( IRenderSystem * pRenderSystem )
{
}

void CFaceEditor::OnSetVisible( bool bVisible )
{
	if( bVisible )
	{
		CSkinNMaskCfg::Inst().Load();
		COrganCfg::Inst().Load();
		RefreshToolbox();
	}
	Super::OnSetVisible( bVisible );
	if( !bVisible )
	{
		ClearToolbox();
		CSkinNMaskCfg::Inst().Unload();
		COrganCfg::Inst().Unload();
	}
}

void CFaceEditor::OnViewportStartDrag( SUIMouseEvent * pEvent )
{
	if( !m_pSelected )
		Super::OnViewportStartDrag( pEvent );
}

void CFaceEditor::OnViewportDragged( SUIMouseEvent * pEvent )
{
	if( !m_pSelected )
		Super::OnViewportDragged( pEvent );
}

void CFaceEditor::OnViewportStopDrag( SUIMouseEvent * pEvent )
{
	if( !m_pSelected )
		Super::OnViewportStopDrag( pEvent );
}

void CFaceEditor::OnViewportChar( uint32 nChar )
{
}

void CFaceEditor::RefreshToolbox()
{
	m_pOrgansRoot = CTreeFolder::Create( m_pToolView, NULL, "Organs" );
	m_pSkinsRoot = CTreeFolder::Create( m_pToolView, NULL, "Skins" );
	m_pMasksRoot = CTreeFolder::Create( m_pToolView, NULL, "Masks" );

	for( auto& item : CSkinNMaskCfg::Inst().mapSkins )
	{
		auto pItem = CFaceEditItemUI::Create( this, item.second );
		m_pToolView->AddContentChild( pItem, m_pSkinsRoot );
	}
	for( auto& item : COrganCfg::Inst().mapOrganEditItems )
	{
		auto pItem = CFaceEditItemUI::Create( this, &item.second );
		m_pToolView->AddContentChild( pItem, m_pOrgansRoot );
	}
}

void CFaceEditor::ClearToolbox()
{
	SetSelectedEditItem( NULL );
	m_pToolView->ClearContent();
	m_pOrgansRoot = NULL;
	m_pSkinsRoot = NULL;
	m_pMasksRoot = NULL;
}

void CFaceEditor::SetSelectedEditItem( CFaceEditItemUI * pSelected )
{
	m_pSelected = pSelected;
	if( pSelected )
		m_pFaceView->Select( pSelected->GetItem() );
	else
		m_pFaceView->Select( NULL );
}

CFaceEditor::CFaceEditItemUI * CFaceEditor::CFaceEditItemUI::Create( CFaceEditor * pOwner, CFaceEditItem * pItem )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/faceedititem.xml" );
	auto pUIItem = new CFaceEditItemUI;
	g_pRes->GetElement()->Clone( pUIItem );
	pUIItem->m_pOwner = pOwner;
	pUIItem->Refresh( pItem );
	return pUIItem;
}