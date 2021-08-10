#pragma once
#include "FileView.h"
#include "UIComponentUtil.h"

class CFileExplorer : public CFileView
{
public:
	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CFileExplorer )
protected:
	virtual void OnInited() override;

	void AddTool( CUIElement* pElem );
private:
	void OnOpen();
	void OnNew();
	void OnTools();

	CReference<CUITextBox> m_pNewFileName;
	CReference<CDropDownBox> m_pNewFileType;
	CReference<CUIElement> m_pToolsPanel;
	TClassTrigger<CFileExplorer> m_onOpen;
	TClassTrigger<CFileExplorer> m_onNew;
	TClassTrigger<CFileView> m_onRefresh;
	TClassTrigger<CFileExplorer> m_onTools;

	int32 m_nToolY;
};