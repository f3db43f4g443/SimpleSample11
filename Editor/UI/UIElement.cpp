#include "stdafx.h"
#include "UIElement.h"
#include "UIManager.h"
#include "Common/xml.h"

CUIElement* CUIElement::MouseDown( const CVector2& mousePos )
{
	CUIElement* pElement = NULL;
	ForEachChild( [&mousePos, &pElement] ( CUIElement* pElem ) {
		if( !pElem->bVisible )
			return true;
		if( !pElem->globalAABB.Contains( mousePos ) )
			return true;

		pElement = pElem->MouseDown( mousePos );
		return pElement == NULL;
	} );

	if( !pElement )
	{
		if( IsEnableMouseEvent() && m_localBound.Contains( mousePos - globalTransform.GetPosition() ) )
		{
			pElement = this;
			OnMouseDown( mousePos );
		}
	}

	return pElement;
}

CUIElement* CUIElement::MouseUp( const CVector2& mousePos )
{
	CUIElement* pElement = NULL;
	ForEachChild( [&mousePos, &pElement] ( CUIElement* pElem ) {
		if( !pElem->globalAABB.Contains( mousePos ) )
			return true;

		pElement = pElem->MouseUp( mousePos );
		return pElement == NULL;
	} );

	if( !pElement )
	{
		if( IsEnableMouseEvent() && m_localBound.Contains( mousePos - globalTransform.GetPosition() ) )
		{
			pElement = this;
			OnMouseUp( mousePos );
		}
	}

	return pElement;
}

CUIElement* CUIElement::MouseMove( const CVector2& mousePos )
{
	CUIElement* pElement = NULL;
	ForEachChild( [&mousePos, &pElement] ( CUIElement* pElem ) {
		if( !pElem->globalAABB.Contains( mousePos ) )
			return true;

		pElement = pElem->MouseMove( mousePos );
		return pElement == NULL;
	} );

	if( !pElement )
	{
		if( IsEnableMouseEvent() && m_localBound.Contains( mousePos - globalTransform.GetPosition() ) )
		{
			pElement = this;
			OnMouseMove( mousePos );
		}
	}

	return pElement;
}

void CUIElement::Resize( const CRectangle& rect )
{
	CRectangle oldRect = m_localBound;

	ForEachChild( [&rect, &oldRect] ( CUIElement* pElem ) {
		uint32 nAlignType = pElem->GetAlignType();
		CRectangle newRect = pElem->GetSize();
		
		switch( nAlignType & 3 )
		{
		case eUIAlignType_Center:
			newRect.x += rect.GetCenterX() - oldRect.GetCenterX();
			break;
		case eUIAlignType_Left:
			newRect.x += rect.GetLeft() - oldRect.GetLeft();
			break;
		case eUIAlignType_Right:
			newRect.x += rect.GetRight() - oldRect.GetRight();
			break;
		case eUIAlignType_Both:
			newRect.SetLeft( newRect.GetLeft() + rect.GetLeft() - oldRect.GetLeft() );
			newRect.SetRight( newRect.GetRight() + rect.GetRight() - oldRect.GetRight() );
			break;
		}
		switch( ( nAlignType >> 2 ) & 3 )
		{
		case eUIAlignType_Center:
			newRect.y += rect.GetCenterY() - oldRect.GetCenterY();
			break;
		case eUIAlignType_Left:
			newRect.y += rect.GetTop() - oldRect.GetTop();
			break;
		case eUIAlignType_Right:
			newRect.y += rect.GetBottom() - oldRect.GetBottom();
			break;
		case eUIAlignType_Both:
			newRect.SetTop( newRect.GetTop() + rect.GetTop() - oldRect.GetTop() );
			newRect.SetBottom( newRect.GetBottom() + rect.GetBottom() - oldRect.GetBottom() );
			break;
		}

		pElem->Resize( newRect );
		return true;
	} );

	m_localBound = rect;
	OnResize( oldRect, rect );
}

bool CUIElement::IsFocused()
{
	return m_pMgr && m_pMgr->GetFocus() == this;
}

bool CUIElement::IsDragged()
{
	return m_pMgr && m_pMgr->GetDragged() == this;
}

void CUIElement::Focus()
{
	m_pMgr->SetFocus( this );
}

void CUIElement::ForEachChild( function<bool( CUIElement* pElem )> func )
{
	LINK_LIST_FOR_EACH_BEGIN( pRenderObject, m_pChildren, CRenderObject2D, Child )
		if( !pRenderObject->GetParent() )
			continue;
		CUIElement* pElem = dynamic_cast<CUIElement*>( pRenderObject );
		if( !pElem )
			continue;
		if( !func( pElem ) )
			break;
	LINK_LIST_FOR_EACH_END( pRenderObject, m_pChildren, CRenderObject2D, Child )
}

void CUIElement::SetActive( bool bActive )
{
	m_bActive = bActive;
	if( bActive )
		GetParent()->MoveToTopmost( this, true );
	OnSetActive( bActive );
}

void CUIElement::OnAdded()
{
	CUIElement* pElement = dynamic_cast<CUIElement*>( GetParent() );
	if( pElement && pElement->m_pMgr )
		pElement->m_pMgr->AddElement( this );
}

void CUIElement::OnRemoved()
{
	if( m_pMgr )
		m_pMgr->RemoveElement( this );
}

void CUIElement::LoadXml( TiXmlElement* pRoot )
{
	m_strName = XmlGetAttr( pRoot, "name", "" );
	x = XmlGetAttr( pRoot, "x", 0.0f );
	y = XmlGetAttr( pRoot, "y", 0.0f );
	m_localBound.x = XmlGetAttr( pRoot, "rectX", 0.0f );
	m_localBound.y = XmlGetAttr( pRoot, "rectY", 0.0f );
	m_localBound.width = XmlGetAttr( pRoot, "rectWidth", 0.0f );
	m_localBound.height = XmlGetAttr( pRoot, "rectHeight", 0.0f );

	m_nFlag = XmlGetAttr( pRoot, "alignX", 0 )
		| ( XmlGetAttr( pRoot, "alignY", 0 ) << 2 )
		| ( XmlGetAttr( pRoot, "enableMouseEvent", (uint32)IsEnableMouseEvent() ) << 4 );
	m_bEnabled = XmlGetAttr( pRoot, "enabled", 1 );
}

void CUIElement::CopyData( CUIElement* pElement )
{
	pElement->x = x;
	pElement->y = y;
	pElement->m_strName = m_strName;
	pElement->m_localBound = m_localBound;
	pElement->m_nFlag = m_nFlag;
	pElement->m_bEnabled = m_bEnabled;
}

CUIElement* CUIElement::Clone()
{
	CUIElement* pElement = CreateObject();
	CopyData( pElement );

	CRenderObject2D* pChildren = NULL;
	ForEachChild( [&pChildren] ( CUIElement* pElem ) {
		CUIElement* pChild = pElem->Clone();
		pChild->AddRef();
		pChild->InsertTo_Child( pChildren );
		return true;
	} );
	while( pChildren )
	{
		CUIElement* pChild = static_cast<CUIElement*>( pChildren );
		pChild->RemoveFrom_Child();
		pElement->AddChild( pChild );
		pChild->Release();
	}

	return pElement;
}

void CUIElement::DispatchMouseEvent( SUIMouseEvent& mouseEvent )
{
	m_events.Trigger( eEvent_Resize, &mouseEvent );
	CUIElement* pParent = dynamic_cast<CUIElement*>( GetParent() );
	if( pParent && pParent != m_pMgr )
		pParent->DispatchMouseEvent( mouseEvent );
}

void CUIElement::OnResize( const CRectangle& oldRect, const CRectangle& newRect )
{
	CRectangle rect = oldRect;
	m_events.Trigger( eEvent_Resize, &rect );
}

void CUIElement::OnMouseDown( const CVector2& mousePos )
{
	SUIMouseEvent mouseEvent = { eEvent_MouseDown, this, mousePos };
	DispatchMouseEvent( mouseEvent );
}

void CUIElement::OnMouseUp( const CVector2& mousePos )
{
	SUIMouseEvent mouseEvent = { eEvent_MouseUp, this, mousePos };
	DispatchMouseEvent( mouseEvent );
}

void CUIElement::OnMouseMove( const CVector2& mousePos )
{
	SUIMouseEvent mouseEvent = { eEvent_MouseMove, this, mousePos };
	DispatchMouseEvent( mouseEvent );
}

void CUIElement::OnStartDrag( const CVector2& mousePos )
{
	SUIMouseEvent mouseEvent = { eEvent_StartDrag, this, mousePos };
	m_events.Trigger( eEvent_StartDrag, &mouseEvent );
}

void CUIElement::OnDragged( const CVector2& mousePos )
{
	SUIMouseEvent mouseEvent = { eEvent_Dragged, this, mousePos };
	m_events.Trigger( eEvent_Dragged, &mouseEvent );
}

void CUIElement::OnStopDrag( const CVector2& mousePos )
{
	SUIMouseEvent mouseEvent = { eEvent_StopDrag, this, mousePos };
	m_events.Trigger( eEvent_StopDrag, &mouseEvent );
}

void CUIElement::OnSetEnabled( bool bEnabled )
{
	m_events.Trigger( eEvent_SetEnabled, (void*)bEnabled );
}

void CUIElement::OnSetActive( bool bActive )
{
	m_events.Trigger( eEvent_SetActive, (void*)bActive );
}

void CUIElement::OnSetFocused( bool bFocused )
{
	m_events.Trigger( eEvent_SetFocused, (void*)bFocused );
}