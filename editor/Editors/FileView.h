#pragma once
#include "UICommon/UITreeView.h"

class CFileView : public CUIElement
{
	friend class CFileSelectTreeFolder;
public:
	void SelectFolder( const char* szPath );
	void SelectFile( const char* szFileName );
	void Refresh();
	void RefreshFolders( CUITreeView::CTreeViewContent* pRoot, const char* szPath );
protected:
	virtual void OnInited() override;

	string m_strPath;
	string m_strFile;
	vector<string> m_exts;
	CReference<CUITreeView> m_pFolders;
	CReference<CUIScrollView> m_pFiles;
	CReference<CUILabel> m_pSelectedFile;
};