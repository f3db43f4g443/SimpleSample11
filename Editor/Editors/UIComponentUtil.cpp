#include "stdafx.h"
#include "UIComponentUtil.h"
#include "Common/ResourceManager.h"
#include "UICommon/UIFactory.h"
#include "UICommon/UIManager.h"
#include "Common/FileUtil.h"
#include "Editor.h"

CUITreeView::CTreeViewContent* CTreeFolder::Create( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, const char* szName )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/treefolder.xml" );
	auto pTreeFolder = new CTreeFolder;
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

void CTreeFolder::OnInited()
{
	m_onSwitch.Set( this, &CTreeFolder::OnSwitch );
	Register( eEvent_Action, &m_onSwitch );
}

void CTreeFolder::OnSwitch()
{
	if( m_pContent->bFolded == IsChecked() )
		return;
	m_pTreeView->SetContentFolded( m_pContent, IsChecked() );
	if( GetMgr() )
		GetMgr()->SetFocus( this );
}

CBoolEdit* CBoolEdit::Create( const char* szName )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/booledit.xml" );
	auto pElem = new CBoolEdit;
	g_pRes->GetElement()->Clone( pElem );
	pElem->GetChildByName<CUILabel>( "label" )->SetText( szName );
	return pElem;
}

CCommonEdit* CCommonEdit::Create( const char* szName )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/commonedit.xml" );
	auto pElem = new CCommonEdit;
	g_pRes->GetElement()->Clone( pElem );
	pElem->GetChildByName<CUILabel>( "label" )->SetText( szName );
	return pElem;
}

CVectorEdit* CVectorEdit::Create( const char* szName, uint32 nCount )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/vectoredit.xml" );
	auto pElem = new CVectorEdit;
	g_pRes->GetElement()->Clone( pElem );
	pElem->GetChildByName<CUILabel>( "label" )->SetText( szName );
	pElem->m_nCount = Min( 4u, nCount );
	for( int i = nCount; i < 4; i++ )
		pElem->m_textBox[i]->SetVisible( false );
	return pElem;
}

float CVectorEdit::GetFloat()
{
	return m_textBox[0]->GetValue<float>();
}
CVector2 CVectorEdit::GetFloat2()
{
	CVector2 vec( 0, 0 );
	if( m_nCount != 2 )
		return vec;
	GetFloats( &vec.x );
	return vec;
}
CVector3 CVectorEdit::GetFloat3()
{
	CVector3 vec( 0, 0, 0 );
	if( m_nCount != 3 )
		return vec;
	GetFloats( &vec.x );
	return vec;
}
CVector4 CVectorEdit::GetFloat4()
{
	CVector4 vec( 0, 0, 0, 0 );
	if( m_nCount != 4 )
		return vec;
	GetFloats( &vec.x );
	return vec;
}

void CVectorEdit::SetFloat2( const CVector2& value )
{
	if( m_nCount != 2 )
		return;
	SetFloats( &value.x );
}

void CVectorEdit::SetFloat3( const CVector3& value )
{
	if( m_nCount != 3 )
		return;
	SetFloats( &value.x );
}

void CVectorEdit::SetFloat4( const CVector4& value )
{
	if( m_nCount != 4 )
		return;
	SetFloats( &value.x );
}

uint8 CVectorEdit::GetFloats( float* pValues )
{
	for( int i = 0; i < m_nCount; i++ )
	{
		pValues[i] = m_textBox[i]->GetValue<float>();
	}
	return m_nCount;
}
uint8 CVectorEdit::SetFloats( const float* pValues )
{
	for( int i = 0; i < m_nCount; i++ )
	{
		m_textBox[i]->SetValue( pValues[i] );
	}
	return m_nCount;
}

void CVectorEdit::OnInited()
{
	for( int i = 0; i < 4; i++ )
	{
		char szName[64];
		sprintf( szName, "textbox%d", i );
		m_textBox[i] = GetChildByName<CUITextBox>( szName );
		
		m_onEdit[i].Set( this, &CVectorEdit::OnEdit );
		m_textBox[i]->Register( eEvent_Action, &m_onEdit[i] );
	}
}

void CVectorEdit::OnEdit()
{
	m_events.Trigger( eEvent_Action, NULL );
}

CTextEdit* CTextEdit::Create( const char* szName )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/textedit.xml" );
	auto pElem = new CTextEdit;
	g_pRes->GetElement()->Clone( pElem );
	pElem->GetChildByName<CUILabel>( "label" )->SetText( szName );
	return pElem;
}

void CTextEdit::OnInited()
{
	m_onEditOK.Set( this, &CTextEdit::OnEditOK );
}

void CTextEdit::OnClick( const CVector2& mousePos )
{
	CTextEditDialog::Inst()->Show( GetText(), &m_onEditOK );
}

void CTextEdit::OnEditOK( const wchar_t* sz )
{
	if( sz )
		SetText( sz );
	m_events.Trigger( CUIElement::eEvent_Action, NULL );
}

CDropTargetEdit* CDropTargetEdit::Create( const char* szName )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/droptargetedit.xml" );
	auto pElem = new CDropTargetEdit;
	g_pRes->GetElement()->Clone( pElem );
	pElem->GetChildByName<CUILabel>( "label" )->SetText( szName );
	return pElem;
}

void CDropTargetEdit::OnMouseUp( const CVector2& mousePos )
{
	CUILabel::OnMouseUp( mousePos );
	auto pDragDropObj = GetMgr()->GetDragDropObject();
	if( pDragDropObj )
		m_events.Trigger( eEvent_Action, pDragDropObj );
}

void CDropTargetEdit::OnInited()
{
	m_onClear.Set( this, &CDropTargetEdit::OnClear );
	auto pBtn = GetChildByName<CUIButton>( "clear" );
	if( pBtn )
		pBtn->Register( eEvent_Action, &m_onClear );
}

CFileNameEdit* CFileNameEdit::Create( const char* szName, const char* szExt )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/filenameedit.xml" );
	auto pElem = new CFileNameEdit;
	g_pRes->GetElement()->Clone( pElem );
	pElem->GetChildByName<CUILabel>( "label" )->SetText( szName );
	pElem->m_strExt = szExt;
	return pElem;
}

void CFileNameEdit::OnInited()
{
	m_onEdit.Set( this, &CFileNameEdit::OnEdit );
	m_onEditOK.Set( this, &CFileNameEdit::OnEditOK );
	GetChildByName( ".." )->Register( eEvent_Action, &m_onEdit );
}

void CFileNameEdit::OnEdit()
{
	CFileSelectDialog::Inst()->Show( m_strExt.c_str(), &m_onEditOK );
}

void CFileNameEdit::OnEditOK( const char* szText )
{
	if( szText )
	{
		SetText( szText );
		m_events.Trigger( eEvent_Action, NULL );
	}
}

CDropDownBox* CDropDownBox::Create( SItem* pItems, uint32 nItems )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/dropdownbox.xml" );
	auto pElem = new CDropDownBox;
	pElem->m_items.resize( nItems );
	for( int i = 0; i < nItems; i++ )
	{
		pElem->m_items[i] = pItems[i];
		pElem->m_itemIndex[pItems[i].name] = i;
	}
	g_pRes->GetElement()->Clone( pElem );
	return pElem;
}

CDropDownBox* CDropDownBox::Create( const char* szName, SItem* pItems, uint32 nItems )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/dropdownbox1.xml" );
	auto pElem = new CDropDownBox;
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

void CDropDownBox::OnInited()
{
	SetSelectedItem( 0u );
	m_onClick.Set( this, &CDropDownBox::OnClick );
	m_pBtn = GetChildByName<CUIButton>( "btn" );
	m_pBtn->Register( eEvent_Action, &m_onClick );
}

class CDropDownScrollView : public CUIScrollView
{
public:
	CDropDownScrollView() : m_pDropDownBox( NULL ) {}

	class CScrollViewItem : public CUIButton
	{
	public:
		static CScrollViewItem* Create( CDropDownScrollView* pView, uint32 nIndex )
		{
			static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/dropdownscrollviewitem.xml" );
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
		CDropDownScrollView* m_pView;
		TClassTrigger<CScrollViewItem> m_onClick;
	};

	void Set( CDropDownBox* pDropDownBox )
	{
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

	static CDropDownScrollView* Inst()
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/dropdownscrollview.xml" );
		static CDropDownScrollView* g_pInst = NULL;
		if( !g_pInst )
		{
			g_pInst = new CDropDownScrollView;
			g_pRes->GetElement()->Clone( g_pInst );
			g_pInst->SetZOrder( 1 );
			CEditor::Inst().GetUIMgr()->AddChild( g_pInst );
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
	CDropDownBox* m_pDropDownBox;
};

void CDropDownBox::OnClick()
{
	CDropDownScrollView::Inst()->Set( this );
}

CDropDownBox::SItem* CDropDownBox::GetItem( uint32 nIndex )
{
	return nIndex < m_items.size() ? &m_items[nIndex] : NULL;
}
CDropDownBox::SItem* CDropDownBox::GetSelectedItem()
{
	return &m_items[m_nSelectedItem];
}
void CDropDownBox::SetSelectedItem( uint32 nIndex, bool bTrigger )
{
	if( nIndex >= m_items.size() )
		return;
	m_nSelectedItem = nIndex;
	SetText( m_items[nIndex].name.c_str() );
	if( bTrigger )
		m_events.Trigger( eEvent_Action, NULL );
}
void CDropDownBox::SetSelectedItem( const char* szName, bool bTrigger )
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

CDropDownBox* CDropDownBox::CreateShaderSelectBox( const char* szName, uint8 nType )
{
	static vector<SItem> items[(uint32)EShaderType::Count];
	static bool bInited = false;
	if( !bInited )
	{
		bInited = true;
		for( int i = 0; i < ELEM_COUNT( items ); i++ )
		{
			SItem selectItem;
			selectItem.name = "(none)";
			selectItem.pData = NULL;
			items[i].push_back( selectItem );
		}

		auto mapShaders = CGlobalShader::GetShaders();
		for( auto& item : mapShaders )
		{
			IShader* pShader = item.second;
			EShaderType eType = pShader->GetShaderInfo().eType;
			SItem selectItem;
			selectItem.name = item.first;
			selectItem.pData = pShader;
			items[(uint32)eType].push_back( selectItem );
		}
	}

	return Create( szName, &items[nType][0], items[nType].size() );
}

void CTextEditDialog::Show( const wchar_t* sz, CTrigger* pOnOK )
{
	m_pOnOK = pOnOK;
	SetText( sz );
	GetMgr()->DoModal( this );
}

CTextEditDialog* CTextEditDialog::Inst()
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/texteditdialog.xml" );
	static CTextEditDialog* g_pInst = NULL;
	if( !g_pInst )
	{
		g_pInst = new CTextEditDialog;
		g_pRes->GetElement()->Clone( g_pInst );
		g_pInst->SetZOrder( 1 );
		CEditor::Inst().GetUIMgr()->AddChild( g_pInst );
	}
	return g_pInst;
}

void CTextEditDialog::OnInited()
{
	CUITextBox::OnInited();
	SetVisible( false );
	m_pOnOK = NULL;
	m_onOk.Set( this, &CTextEditDialog::OnOk );
	m_onCancel.Set( this, &CTextEditDialog::OnCancel );
	GetChildByName( "btn_ok" )->Register( eEvent_Action, &m_onOk );
	GetChildByName( "btn_cancel" )->Register( eEvent_Action, &m_onCancel );
}

void CTextEditDialog::OnOk()
{
	GetMgr()->EndModal();
	m_pOnOK->Run( (void*)GetText() );
}

void CTextEditDialog::OnCancel()
{
	GetMgr()->EndModal();
	m_pOnOK->Run( NULL );
}

CFileSelectDialog* CFileSelectDialog::Inst()
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/fileselectdialog.xml" );
	static CFileSelectDialog* g_pInst = NULL;
	if( !g_pInst )
	{
		g_pInst = new CFileSelectDialog;
		g_pRes->GetElement()->Clone( g_pInst );
		g_pInst->SetZOrder( 1 );
		CEditor::Inst().GetUIMgr()->AddChild( g_pInst );
	}
	return g_pInst;
}

void CFileSelectDialog::OnInited()
{
	CFileView::OnInited();
	SetVisible( false );
	m_pOnSelected = NULL;
	m_onOk.Set( this, &CFileSelectDialog::OnOk );
	m_onClear.Set( this, &CFileSelectDialog::OnClear );
	m_onCancel.Set( this, &CFileSelectDialog::OnCancel );
	GetChildByName( "btn_ok" )->Register( eEvent_Action, &m_onOk );
	GetChildByName( "btn_clear" )->Register( eEvent_Action, &m_onClear );
	GetChildByName( "btn_cancel" )->Register( eEvent_Action, &m_onCancel );
}

void CFileSelectDialog::Show( const char* szExt, CTrigger* pOnSelected )
{
	string strExt = szExt;
	m_exts.clear();
	if( strExt.length() )
	{
		for( int i = 0; i < strExt.length(); )
		{
			int j;
			for( j = i; j < strExt.length(); j++ )
			{
				if( strExt[j] == ';' )
					break;
			}
			if( j > i )
				m_exts.push_back( strExt.substr( i, j - i ) );
			i = j + 1;
		}
	}

	m_pOnSelected = pOnSelected;
	m_strFile = "";
	Refresh();
	GetMgr()->DoModal( this );
}

void CFileSelectDialog::OnOk()
{
	if( !m_strFile.length() )
		return;
	GetMgr()->EndModal();
	m_pOnSelected->Run( (void*)m_strFile.c_str() );
}

void CFileSelectDialog::OnClear()
{
	GetMgr()->EndModal();
	m_pOnSelected->Run( "" );
}

void CFileSelectDialog::OnCancel()
{
	GetMgr()->EndModal();
	m_pOnSelected->Run( NULL );
}