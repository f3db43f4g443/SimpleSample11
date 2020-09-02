#include "stdafx.h"
#include "LevelEdit.h"
#include "MyLevel.h"
#include "UICommon/UIViewport.h"
#include "Editor/Editors/PrefabEditor.h"
#include "Common/Utf8Util.h"
#include "UICommon/UIFactory.h"
#include "LevelTools.h"

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