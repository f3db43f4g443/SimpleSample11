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

class CGameDropDownBox : public CUILabel
{
	friend class CGameDropDownScrollView;
public:
	struct SItem
	{
		string name;
		void* pData;
	};

	static CGameDropDownBox* Create( SItem* pItems, uint32 nItems );
	static CGameDropDownBox* Create( const char* szName, SItem* pItems, uint32 nItems );
	uint32 GetItemCount() { return m_items.size(); }
	SItem* GetItem( uint32 nIndex );
	SItem* GetSelectedItem();
	void SetItems( SItem* pItems, uint32 nItems );
	void SetSelectedItem( uint32 nIndex, bool bTrigger = true );
	void SetSelectedItem( const char* szName, bool bTrigger = true );
protected:
	virtual void OnInited() override;
	void OnClick();
private:
	TClassTrigger<CGameDropDownBox> m_onClick;
	CUIButton* m_pBtn;

	uint32 m_nSelectedItem;
	vector<SItem> m_items;
	map<string, uint32> m_itemIndex;
};