#include "stdafx.h"
#include "LevelEdit.h"
#include "MyLevel.h"
#include "UICommon/UIViewport.h"
#include "Editor/Editors/PrefabEditor.h"
#include "Common/Utf8Util.h"
#include "UICommon/UIFactory.h"
#include "LevelTools.h"
#include "Editor/Editor.h"

CLevelEdit::CLevelEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName )
	: CObjectDataEdit( pTreeView, pParent, pData, pMetaData, szName )
{
	auto pNode = CPrefabEditor::Inst()->GetCurNode();
	if( pNode->GetObjData() == m_pData )
	{
		auto pTools = static_cast<CUIButton*>( CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/button.xml" )->GetElement()->Clone() );
		pTreeView->AddContentChild( pTools, m_pContent, true );
		m_onLevelTools.Set( this, &CLevelEdit::OnLevelTools );
		pTools->Register( CUIElement::eEvent_Action, &m_onLevelTools );
		pTools->SetText( "Tools" );
	}
	const char* szWorld = CPrefabEditor::Inst()->GetParam( "world" );
	if( szWorld )
	{
		auto pWorld = static_cast<CUIButton*>( CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/button.xml" )->GetElement()->Clone() );
		pTreeView->AddContentChild( pWorld, m_pContent, true );
		m_onWorld.Set( this, &CLevelEdit::OnWorld );
		pWorld->Register( CUIElement::eEvent_Action, &m_onWorld );
		pWorld->SetText( "World" );
	}
}

void CLevelEdit::OnLevelTools()
{
	auto pNode = CPrefabEditor::Inst()->GetCurNode();
	if( pNode->GetObjData() != m_pData )
		return;
	CLevelToolsView::Inst()->Set( pNode, [] () {
		CPrefabEditor::Inst()->RefreshCurItem();
	} );
}

void CLevelEdit::OnWorld()
{
	const char* szWorld = CPrefabEditor::Inst()->GetParam( "world" );
	string str = "level=";
	str += CPrefabEditor::Inst()->GetFileName();
	CEditor::Inst().OpenFile( szWorld, str.c_str() );
}
