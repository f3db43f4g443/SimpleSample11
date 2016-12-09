#pragma once
#include "UICommon/UITreeView.h"
#include "UICommon/UIButton.h"
#include "FaceEditItem.h"

class CFaceToolbox;
class CFaceEditItemUI : public CUIButton
{
public:
	static CFaceEditItemUI* Create( CFaceToolbox* pOwner, CFaceEditItem* pItem );

	CFaceEditItem* GetItem() { return m_pItem; }
protected:
	virtual void OnInited() override;
	void Refresh( CFaceEditItem* pItem );
	virtual void OnClick( const CVector2& mousePos ) override;

	CFaceToolbox* m_pOwner;
	CFaceEditItem* m_pItem;
	CReference<CUILabel> m_pName;
};

class CCharacter;
class CFaceToolbox : public CUIElement
{
public:
	void Refresh( CCharacter* pCharacter );
	CFaceEditItemUI* GetSelected() { return m_pSelected; }
	void SetSelected( CFaceEditItemUI* pSelected );
protected:
	virtual void OnInited() override;

	CReference<CUITreeView> m_pToolView;

	CReference<CUITreeView::CTreeViewContent> m_pCommonRoot;
	CReference<CUITreeView::CTreeViewContent> m_pOrgansRoot;
	CReference<CUITreeView::CTreeViewContent> m_pSkinsRoot;
	CReference<CUITreeView::CTreeViewContent> m_pMasksRoot;

	CReference<CFaceEditItemUI> m_pSelected;
};