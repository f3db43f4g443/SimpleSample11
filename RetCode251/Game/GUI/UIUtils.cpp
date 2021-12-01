#include "stdafx.h"
#include "UIUtils.h"
#include "Common/ResourceManager.h"
#include "UICommon/UIFactory.h"
#include "UICommon/UIManager.h"

CUITreeView::CTreeViewContent* CGameTreeFolder::Create( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, const char* szName )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/treefolder.xml" );
	auto pTreeFolder = new CGameTreeFolder;
	g_pRes->GetElement()->Clone( pTreeFolder );
	CUITreeView::CTreeViewContent* pContent;
	if( pParent )
		pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pTreeFolder, pParent ) );
	else
		pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContent( pTreeFolder ) );
	pTreeFolder->m_pTreeView = pTreeView;
	pTreeFolder->m_pContent = pContent;
	pTreeFolder->GetChildByName<CUILabel>( "label" )->SetText( szName );
	pContent->fChildrenIndent = 4;
	return pContent;
}

void CGameTreeFolder::OnInited()
{
	m_onSwitch.Set( this, &CGameTreeFolder::OnSwitch );
	Register( eEvent_Action, &m_onSwitch );
}

void CGameTreeFolder::OnSwitch()
{
	if( m_pContent->bFolded == IsChecked() )
		return;
	m_pTreeView->SetContentFolded( m_pContent, IsChecked() );
	if( GetMgr() )
		GetMgr()->SetFocus( this );
}