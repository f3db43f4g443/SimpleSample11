#pragma once
#include "UIScrollView.h"

class CUITreeView : public CUIScrollView
{
public:
	class CTreeViewContent : public CUIScrollView::CContent
	{
	public:
		CTreeViewContent() : nType( 0 ), bFolded( false ), bHidden( false ), fChildrenIndent( 0 ), pParent( NULL ) {}
		uint8 nType;
		bool bFolded;
		bool bHidden;
		float fChildrenIndent;
		CTreeViewContent* pParent;
		CReference<CTreeViewContent> pTail;
	};
	virtual CUIElement* CreateObject() override { return new CUITreeView; }

	CContent* AddContentChild( CUIElement* pElement, CContent* pInContent, bool bHead = false );
	CContent* AddContentSibling( CUIElement* pElement, CContent* pInContent );

	void RemoveContentTree( CContent* pContent );
	void SetContentFolded( CContent* pContent, bool bFolded );
	void FocusContent( CUITreeView::CTreeViewContent* pContent );

	CTreeViewContent* GetPrevSibling( CTreeViewContent* pContent );
	CTreeViewContent* GetNextSibling( CTreeViewContent* pContent );
	CTreeViewContent* MoveUp( CTreeViewContent* pContent );
	CTreeViewContent* MoveDown( CTreeViewContent* pContent );
	CTreeViewContent* MoveLeft( CTreeViewContent* pContent );
	CTreeViewContent* MoveRight( CTreeViewContent* pContent );

	void MoveChild( CTreeViewContent* pContent, uint32 nIndex, uint32 nTo );
protected:
	virtual CContent* CreateContent() override { return new CTreeViewContent; }
	virtual void LoadContent( TiXmlElement* pRoot ) override;
	virtual void CopyContent( CUIScrollView* pCopyTo, bool bInit ) override;
	virtual void DoLayout() override;

	void LoadContentChild( TiXmlElement* pRoot, CContent* pAddBefore, float fChildrenIndent, float fLineSpace );
};