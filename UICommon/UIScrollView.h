#pragma once
#include "UIScrollBar.h"

class CUIScrollView : public CUILabel
{
public:
	CUIScrollView();
	~CUIScrollView() { ClearContent(); }

	class CContent : public CReferenceObject
	{
	public:
		CContent() : fIndent( 0 ), fLineSpace( 0 ) {}
		CReference<CUIElement> pElement;
		float fIndent;
		float fLineSpace;

		LINK_LIST( CContent, Content );
	};

	virtual void LoadXml( TiXmlElement* pRoot ) override;
	virtual CUIElement* CreateObject() override { return new CUIScrollView; }

	CContent* AddContent( CUIElement* pElement );
	CContent* AddContentBefore( CUIElement* pElement, CContent* pInContent );
	CContent* AddContentAfter( CUIElement* pElement, CContent* pInContent );
	void RemoveContent( CContent* pContent );
	CContent* FindContent( CUIElement* pElement );
	void ClearContent();
	const CRectangle& GetContentClip() { return m_pClipElement->GetLocalClip(); }
protected:
	virtual CContent* CreateContent() { return new CContent; }
	virtual void CopyData( CUIElement* pElement, bool bInit ) override;
	virtual void LoadContent( TiXmlElement* pRoot );
	virtual void CopyContent( CUIScrollView* pCopyTo, bool bInit );

	void SetPercentX( float fPercent ) { if( m_pBarX ) m_pBarX->SetPercent( fPercent ); }
	void SetPercentY( float fPercent ) { if( m_pBarY ) m_pBarY->SetPercent( fPercent ); }
	void FocusPoint( const CVector2& p );

	virtual void OnInited() override;
	virtual void DoLayout() override;

	virtual void OnResize( const CRectangle& oldRect, const CRectangle& newRect ) override;
private:
	void OnScrollX();
	void OnScrollY();

	uint8 m_nLayoutType;
	
	CReference<CUIElement> m_pClipElement;
	CReference<CUIElement> m_pContentPanel;
	CReference<CUIScrollBar> m_pBarX;
	CReference<CUIScrollBar> m_pBarY;

	TClassTrigger<CUIScrollView> m_onScrollX;
	TClassTrigger<CUIScrollView> m_onScrollY;

	CRectangle m_contentSize;
protected:
	CContent m_contentTail;
	LINK_LIST_HEAD( m_pContent, CContent, Content );
};