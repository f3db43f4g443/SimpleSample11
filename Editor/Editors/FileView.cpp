#include "stdafx.h"
#include "FileView.h"
#include "UIComponentUtil.h"
#include "UICommon/UIFactory.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"

class CFileSelectTreeFolder : public CTreeFolder
{
	friend class CFileView;
public:
	static CUITreeView::CTreeViewContent* Create( CFileView* pView, CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, const char* szName, const char* szFullPath )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/fileselect_treefolder.xml" );
		auto pTreeFolder = new CFileSelectTreeFolder;
		g_pRes->GetElement()->Clone( pTreeFolder );
		CUITreeView::CTreeViewContent* pContent;
		if( pParent )
			pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pTreeFolder, pParent ) );
		else
			pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContent( pTreeFolder ) );
		pTreeFolder->m_pTreeView = pTreeView;
		pTreeFolder->m_pContent = pContent;
		pTreeFolder->GetChildByName<CUIButton>( "label" )->SetText( szName );
		pTreeFolder->m_pView = pView;
		pTreeFolder->m_path = szFullPath;
		pContent->fChildrenIndent = 4;
		return pContent;
	}
protected:
	virtual void OnInited() override
	{
		CTreeFolder::OnInited();
		m_onSelect.Set( this, &CFileSelectTreeFolder::OnSelect );
		GetChildByName<CUIButton>( "label" )->Register( eEvent_Action, &m_onSelect );
	}
	void OnSelect()
	{
		m_pView->SelectFolder( m_path.c_str() );
	}

	CFileView* m_pView;
	string m_path;
	TClassTrigger<CFileSelectTreeFolder> m_onSelect;
};

class CFileSelectItem : public CUIButton
{
public:
	static void Create( CFileView* pFileView, CUIScrollView* pView, const char* szName )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/fileselect_item.xml" );
		auto pItem = new CFileSelectItem;
		g_pRes->GetElement()->Clone( pItem );
		pView->AddContent( pItem );
		pItem->m_pFileView = pFileView;
		pItem->m_name = szName;
		pItem->SetText( szName );
	}
	
protected:
	virtual void OnInited() override
	{
		m_onSelect.Set( this, &CFileSelectItem::OnSelect );
		Register( eEvent_Action, &m_onSelect );
	}
	void OnSelect()
	{
		m_pFileView->SelectFile( m_name.c_str() );
	}
private:
	CFileView* m_pFileView;
	string m_name;
	TClassTrigger<CFileSelectItem> m_onSelect;
};

void CFileView::OnInited()
{
	m_pFolders = GetChildByName<CUITreeView>( "folders" );
	m_pFiles = GetChildByName<CUIScrollView>( "files" );
	m_pSelectedFile = GetChildByName<CUILabel>( "filename" );
}

void CFileView::Refresh()
{
	m_pFolders->ClearContent();
	auto pContent = CFileSelectTreeFolder::Create( this, m_pFolders, NULL, "Root", "" );
	RefreshFolders( pContent, "" );

	while( pContent->pTail )
	{
		CUITreeView::CTreeViewContent* pChild;
		for( pChild = dynamic_cast<CUITreeView::CTreeViewContent*>( pContent->NextContent() );
			pChild != pContent->pTail; pChild = dynamic_cast<CUITreeView::CTreeViewContent*>( pChild->NextContent() ) )
		{
			auto pElement = dynamic_cast<CFileSelectTreeFolder*>( pChild->pElement.GetPtr() );
			if( !strncmp( pElement->m_path.c_str(), m_strPath.c_str(), pElement->m_path.length() ) )
				break;
			
			if( pChild->pTail )
				pChild = pChild->pTail;
		}

		if( pChild != pContent->pTail )
		{
			dynamic_cast<CFileSelectTreeFolder*>( pContent->pElement.GetPtr() )->SetChecked( false );
			pContent = pChild;
		}
		else
			break;
	}

	auto pElement = dynamic_cast<CFileSelectTreeFolder*>( pContent->pElement.GetPtr() );
	SelectFolder( pElement->m_path.c_str() );
}

void CFileView::SelectFolder( const char* szPath )
{
	m_pFiles->ClearContent();
	string strFind = szPath;
	strFind += "*";
	FindFiles( strFind.c_str(), [this, szPath] ( const char* szFileName )
	{
		if( m_exts.size() )
		{
			const char* szExt = GetFileExtension( szFileName );
			int i;
			for( i = 0; i < m_exts.size(); i++ )
			{
				if( m_exts[i] == szExt )
					break;
			}
			if( i >= m_exts.size() )
				return true;
		}
		string strFullPath = szPath;
		strFullPath += szFileName;
		CFileSelectItem::Create( this, m_pFiles, szFileName );
		return true;
	}, true, false );

	m_strPath = szPath;
}

void CFileView::SelectFile( const char* szFileName )
{
	m_strFile = m_strPath + szFileName;
	m_pSelectedFile->SetText( m_strFile.c_str() );
}

void CFileView::RefreshFolders( CUITreeView::CTreeViewContent* pRoot, const char* szPath )
{
	string strFind = szPath;
	strFind += "*";
	FindFiles( strFind.c_str(), [this, pRoot, szPath] ( const char* szFileName )
	{
		string strFullPath = szPath;
		strFullPath += szFileName;
		strFullPath += "/";
		auto pContent = CFileSelectTreeFolder::Create( this, m_pFolders, pRoot, szFileName, strFullPath.c_str() );
		dynamic_cast<CFileSelectTreeFolder*>( pContent->pElement.GetPtr() )->SetChecked( true );
		RefreshFolders( pContent, strFullPath.c_str() );
		return true;
	}, false, true );
}