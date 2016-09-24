#pragma once
#include "UILabel.h"

class CUIScrollBar : public CUILabel
{
public:
	CUIScrollBar() : m_bVertical( false ), m_dragRect( 0, 0, 0, 0 ), m_fPercent( 0 ), m_fThumbLengthPercent( 1 ) { SetEnableMouseEvent( true ); }

	virtual void LoadXml( TiXmlElement* pRoot ) override;
	virtual CUIElement* CreateObject() override { return new CUIScrollBar; }

	float GetPercent() { return m_fPercent; }
	void SetPercent( float fPercent );
	void SetThumbLengthPercent( float fPercent );
protected:
	virtual void OnResize( const CRectangle& oldRect, const CRectangle& newRect ) override;
	virtual void CopyData( CUIElement* pElement, bool bInit ) override;
	
	virtual void OnStartDrag( const CVector2& mousePos ) override;
	virtual void OnDragged( const CVector2& mousePos ) override;

	void SetPercentByMousePos( const CVector2& mousePos );
	void ResizeThumb();
private:
	bool m_bVertical;
	float m_fPercent;
	float m_fThumbLengthPercent;
	CRectangle m_dragRect;

	CVector2 m_dragPos;
	float m_fDragPercent;
	CRectangle m_thumbRect;
	CReference<CImageList> m_pThumb;
};