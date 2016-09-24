#include "stdafx.h"
#include "UIScrollBar.h"
#include "Common/xml.h"

void CUIScrollBar::LoadXml( TiXmlElement* pRoot )
{
	CUILabel::LoadXml( pRoot );
	m_dragRect.SetLeft( m_localBound.GetLeft() + XmlGetAttr( pRoot, "dragRectLeft", m_dragRect.GetLeft() - m_localBound.GetLeft() ) );
	m_dragRect.SetTop( m_localBound.GetTop() + XmlGetAttr( pRoot, "dragRectTop", m_dragRect.GetTop() - m_localBound.GetTop() ) );
	m_dragRect.SetRight( m_localBound.GetRight() - XmlGetAttr( pRoot, "dragRectRight", m_localBound.GetRight() - m_dragRect.GetRight() ) );
	m_dragRect.SetBottom( m_localBound.GetBottom() - XmlGetAttr( pRoot, "dragRectBottom", m_localBound.GetBottom() - m_dragRect.GetBottom() ) );
	
	if( !m_pThumb )
	{
		m_thumbRect = m_dragRect;
		m_pThumb = new CImageList;
		AddChild( m_pThumb );
		m_pThumb->LoadXml( pRoot->FirstChildElement( "imageList_Thumb" ) );
	}

	m_bVertical = XmlGetAttr( pRoot, "vertical", m_bVertical ? 1 : 0 );
	m_fPercent = XmlGetAttr( pRoot, "percent", m_fPercent );
	m_fThumbLengthPercent = XmlGetAttr( pRoot, "thumb_percent", m_fThumbLengthPercent );
	ResizeThumb();
}

void CUIScrollBar::CopyData( CUIElement* pElement, bool bInit )
{
	CUILabel::CopyData( pElement, bInit );

	CUIScrollBar* pUIScrollBar = dynamic_cast<CUIScrollBar*>( pElement );
	pUIScrollBar->m_dragRect = m_dragRect;
	pUIScrollBar->m_thumbRect = m_thumbRect;
	pUIScrollBar->m_pThumb = new CImageList;
	pUIScrollBar->AddChild( pUIScrollBar->m_pThumb );
	m_pThumb->CopyData( pUIScrollBar->m_pThumb );
	
	pUIScrollBar->m_bVertical = m_bVertical;
	pUIScrollBar->m_fPercent = m_fPercent;
	pUIScrollBar->m_fThumbLengthPercent = m_fThumbLengthPercent;
	pUIScrollBar->ResizeThumb();
}

void CUIScrollBar::SetPercent( float fPercent )
{
	m_fPercent = Min( 1.0f, Max( 0.0f, fPercent ) );
	ResizeThumb();

	m_events.Trigger( eEvent_Action, &fPercent );
}

void CUIScrollBar::SetThumbLengthPercent( float fPercent )
{
	m_fThumbLengthPercent = fPercent;
	ResizeThumb();
}

void CUIScrollBar::OnResize( const CRectangle& oldRect, const CRectangle& newRect )
{
	CUILabel::OnResize( oldRect, newRect );
	m_dragRect.SetLeft( m_dragRect.GetLeft() + newRect.GetLeft() - oldRect.GetLeft() );
	m_dragRect.SetRight( m_dragRect.GetRight() + newRect.GetRight() - oldRect.GetRight() );
	m_dragRect.SetTop( m_dragRect.GetTop() + newRect.GetTop() - oldRect.GetTop() );
	m_dragRect.SetBottom( m_dragRect.GetBottom() + newRect.GetBottom() - oldRect.GetBottom() );

	ResizeThumb();
}

void CUIScrollBar::ResizeThumb()
{
	CRectangle newThumbRect = m_dragRect;
	float fLeft = ( 1 - m_fThumbLengthPercent ) * m_fPercent;
	float fRight = m_fPercent + ( 1 - m_fPercent ) * m_fThumbLengthPercent;
	if( m_bVertical )
	{
		fLeft = m_dragRect.y + m_dragRect.height * fLeft;
		fRight = m_dragRect.y + m_dragRect.height * fRight;
		newThumbRect.SetTop( (int32)fLeft );
		newThumbRect.SetBottom( (int32)fRight );
	}
	else
	{
		fLeft = m_dragRect.x + m_dragRect.width * fLeft;
		fRight = m_dragRect.x + m_dragRect.width * fRight;
		newThumbRect.SetLeft( (int32)fLeft );
		newThumbRect.SetRight( (int32)fRight );
	}

	if( m_pThumb )
		m_pThumb->Resize( m_thumbRect, newThumbRect );
	m_thumbRect = newThumbRect;
}

void CUIScrollBar::SetPercentByMousePos( const CVector2& mousePos )
{
	CVector2 localPos = mousePos - globalTransform.GetPosition();
	float fPercent;
	if( m_bVertical )
		fPercent = ( localPos.y - m_dragRect.y ) / m_dragRect.height;
	else
		fPercent = ( localPos.x - m_dragRect.x ) / m_dragRect.width;
	fPercent = ( fPercent - m_fThumbLengthPercent * 0.5f ) / ( 1 - m_fThumbLengthPercent );
	if( fPercent > 1 )
		fPercent = 1;
	if( fPercent < 0 )
		fPercent = 0;
	SetPercent( fPercent );
}

void CUIScrollBar::OnStartDrag( const CVector2& mousePos )
{
	CUILabel::OnStartDrag( mousePos );
	SetPercentByMousePos( mousePos );
}

void CUIScrollBar::OnDragged( const CVector2& mousePos )
{
	CUILabel::OnDragged( mousePos );
	SetPercentByMousePos( mousePos );
}