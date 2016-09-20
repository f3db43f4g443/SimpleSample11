#pragma once
#include "FileView.h"
#include "UIComponentUtil.h"

class CFileExplorer : public CFileView
{
public:
	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CFileExplorer )
protected:
	virtual void OnInited() override;
private:
	void OnOpen();
	void OnNew();

	CReference<CUITextBox> m_pNewFileName;
	CReference<CDropDownBox> m_pNewFileType;
	TClassTrigger<CFileExplorer> m_onOpen;
	TClassTrigger<CFileExplorer> m_onNew;
	TClassTrigger<CFileView> m_onRefresh;
};