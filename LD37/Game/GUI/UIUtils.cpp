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

CGameDropDownBox* CGameDropDownBox::Create( SItem* pItems, uint32 nItems )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/dropdownbox.xml" );
	auto pElem = new CGameDropDownBox;
	pElem->m_items.resize( nItems );
	for( int i = 0; i < nItems; i++ )
	{
		pElem->m_items[i] = pItems[i];
		pElem->m_itemIndex[pItems[i].name] = i;
	}
	g_pRes->GetElement()->Clone( pElem );
	return pElem;
}

CGameDropDownBox* CGameDropDownBox::Create( const char* szName, SItem* pItems, uint32 nItems )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/dropdownbox1.xml" );
	auto pElem = new CGameDropDownBox;
	pElem->m_items.resize( nItems );
	for( int i = 0; i < nItems; i++ )
	{
		pElem->m_items[i] = pItems[i];
		pElem->m_itemIndex[pItems[i].name] = i;
	}
	g_pRes->GetElement()->Clone( pElem );
	pElem->GetChildByName<CUILabel>( "label" )->SetText( szName );
	return pElem;
}

void CGameDropDownBox::OnInited()
{
	SetSelectedItem( 0u );
	m_onClick.Set( this, &CGameDropDownBox::OnClick );
	m_pBtn = GetChildByName<CUIButton>( "btn" );
	m_pBtn->Register( eEvent_Action, &m_onClick );
}

class CGameDropDownScrollView : public CUIScrollView
{
public:
	CGameDropDownScrollView() : m_pDropDownBox( NULL ) {}

	class CScrollViewItem : public CUIButton
	{
	public:
		static CScrollViewItem* Create( CGameDropDownScrollView* pView, uint32 nIndex )
		{
			static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/dropdownscrollviewitem.xml" );
			auto pElem = new CScrollViewItem;
			pElem->m_pView = pView;
			pElem->m_nIndex = nIndex;
			g_pRes->GetElement()->Clone( pElem );
			return pElem;
		}
	protected:
		virtual void OnInited() override
		{
			SetText( m_pView->m_pDropDownBox->GetItem( m_nIndex )->name.c_str() );
			m_onClick.Set( this, &CScrollViewItem::OnClick );
			Register( eEvent_Action, &m_onClick );
		}
		void OnClick()
		{
			m_pView->OnSelect( m_nIndex );
		}
	private:
		uint32 m_nIndex;
		CGameDropDownScrollView* m_pView;
		TClassTrigger<CScrollViewItem> m_onClick;
	};

	void Set( CGameDropDownBox* pDropDownBox )
	{
		if( pDropDownBox->GetMgr() != GetMgr() )
		{
			DEFINE_TEMP_REF_THIS();
			RemoveThis();
			pDropDownBox->GetMgr()->AddChild( this );
		}
		ClearContent();
		m_pDropDownBox = pDropDownBox;
		CRectangle rect = GetSize();
		rect.width = pDropDownBox->GetSize().width;
		Resize( rect );

		CVector2 pos( pDropDownBox->GetSize().x, pDropDownBox->GetSize().GetBottom() );
		pos = pos + pDropDownBox->globalTransform.GetPosition();
		if( pos.y + rect.height > GetMgr()->GetSize().height - 10 )
			pos.y -= pDropDownBox->GetSize().height + rect.height;
		SetPosition( pos );

		uint32 i = 0;
		for( auto& item : pDropDownBox->m_items )
		{
			CScrollViewItem* pScrollViewItem = CScrollViewItem::Create( this, i++ );
			AddContent( pScrollViewItem );
			CRectangle rect1 = pScrollViewItem->GetSize();
			rect1.width = GetContentClip().width;
			rect1.height = 16;
			pScrollViewItem->Resize( rect1 );
		}
		SetVisible( true );
		GetMgr()->SetFocus( this );
	}
	void OnSelect( uint32 nIndex )
	{
		m_pDropDownBox->SetSelectedItem( nIndex );
		GetMgr()->SetFocus( m_pDropDownBox );
	}

	static CGameDropDownScrollView* Inst()
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/dropdownscrollview.xml" );
		static CGameDropDownScrollView* g_pInst = NULL;
		if( !g_pInst )
		{
			g_pInst = new CGameDropDownScrollView;
			g_pRes->GetElement()->Clone( g_pInst );
			g_pInst->SetZOrder( 1 );
		}
		return g_pInst;
	}
protected:
	virtual void OnSetActive( bool bActive ) override
	{
		if( !bActive )
			SetVisible( false );
	}
private:
	CGameDropDownBox* m_pDropDownBox;
};

void CGameDropDownBox::OnClick()
{
	CGameDropDownScrollView::Inst()->Set( this );
}

CGameDropDownBox::SItem* CGameDropDownBox::GetItem( uint32 nIndex )
{
	return nIndex < m_items.size() ? &m_items[nIndex] : NULL;
}
CGameDropDownBox::SItem* CGameDropDownBox::GetSelectedItem()
{
	return &m_items[m_nSelectedItem];
}

void CGameDropDownBox::SetItems( SItem * pItems, uint32 nItems )
{
	m_items.resize( nItems );
	for( int i = 0; i < nItems; i++ )
	{
		m_items[i] = pItems[i];
		m_itemIndex[pItems[i].name] = i;
	}
	SetSelectedItem( 0u );
}

void CGameDropDownBox::SetSelectedItem( uint32 nIndex, bool bTrigger )
{
	if( nIndex >= m_items.size() )
		return;
	m_nSelectedItem = nIndex;
	SetText( m_items[nIndex].name.c_str() );
	if( bTrigger )
		m_events.Trigger( eEvent_Action, NULL );
}
void CGameDropDownBox::SetSelectedItem( const char* szName, bool bTrigger )
{
	auto itr = m_itemIndex.find( szName );
	if( itr != m_itemIndex.end() )
	{
		m_nSelectedItem = itr->second;
		SetText( m_items[m_nSelectedItem].name.c_str() );
		if( bTrigger )
			m_events.Trigger( eEvent_Action, NULL );
	}
}