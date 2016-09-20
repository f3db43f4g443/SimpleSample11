#pragma once
#include "UICommon/UITreeView.h"
#include "UICommon/UICheckBox.h"

class CGameTreeFolder : public CUICheckBox
{
public:
	static CUITreeView::CTreeViewContent* Create( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, const char* szName );
protected:
	virtual void OnInited() override;
	void OnSwitch();

	CUITreeView* m_pTreeView;
	CUITreeView::CTreeViewContent* m_pContent;
	TClassTrigger<CGameTreeFolder> m_onSwitch;
};