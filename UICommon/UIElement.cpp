#include "stdafx.h"
#include "UIElement.h"
#include "UIManager.h"
#include "Common/xml.h"

CUIElement* CUIElement::MouseDown( const CVector2& mousePos )
{
	CUIElement* pElement = NULL;
	if( globalClip.Contains( mousePos ) )
	{
		ForEachChild( [&mousePos, &pElement] ( CUIElement* pElem ) {
			if( !pElem->bVisible )
				return true;
			if( !pElem->globalAABB.Contains( mousePos ) )
				return true;

			pElement = pElem->MouseDown( mousePos );
			return pElement == NULL;
		} );
	}

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
	if( globalClip.Contains( mousePos ) )
	{
		ForEachChild( [&mousePos, &pElement] ( CUIElement* pElem ) {
			if( !pElem->bVisible )
				return true;
			if( !pElem->globalAABB.Contains( mousePos ) )
				return true;

			pElement = pElem->MouseUp( mousePos );
			return pElement == NULL;
		} );
	}

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
	if( globalClip.Contains( mousePos ) )
	{
		ForEachChild( [&mousePos, &pElement] ( CUIElement* pElem ) {
			if( !pElem->bVisible )
				return true;
			if( !pElem->globalAABB.Contains( mousePos ) )
				return true;

			pElement = pElem->MouseMove( mousePos );
			return pElement == NULL;
		} );
	}

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
	SetTransformDirty();
	m_localBound = rect;
	OnResize( oldRect, rect );
}

void CUIElement::SetVisible( bool bVisible )
{
	if( bVisible == this->bVisible )
		return;
	this->bVisible = bVisible;
	OnSetVisible( bVisible );
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
		MoveToTopmost( true );
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
	m_strName = XmlGetAttr( pRoot, "name", m_strName.c_str() );
	x = XmlGetAttr( pRoot, "x", x );
	y = XmlGetAttr( pRoot, "y", y );
	SetZOrder( XmlGetAttr( pRoot, "z", GetZOrder() ) );
	bVisible = !!XmlGetAttr( pRoot, "visible", bVisible ? 1 : 0 );
	Resize( CRectangle( XmlGetAttr( pRoot, "rectX", GetSize().x ),
		XmlGetAttr( pRoot, "rectY", GetSize().y ),
		XmlGetAttr( pRoot, "rectWidth", GetSize().width ),
		XmlGetAttr( pRoot, "rectHeight", GetSize().height ) ) );

	m_nFlag = XmlGetAttr( pRoot, "alignX", m_nFlag & 3 )
		| ( XmlGetAttr( pRoot, "alignY", ( m_nFlag >> 2 ) & 3 ) << 2 )
		| ( XmlGetAttr( pRoot, "enableMouseEvent", (uint32)IsEnableMouseEvent() ) << 4 );
	m_bEnabled = XmlGetAttr( pRoot, "enabled", m_bEnabled ? 1 : 0 );

	m_bClipChildren = !!XmlGetAttr( pRoot, "clipChildren", m_bClipChildren ? 1 : 0 );
	if( m_bClipChildren )
	{
		m_localClip.SetLeft( m_localBound.GetLeft() + XmlGetAttr( pRoot, "clipRectLeft", m_localClip.GetLeft() - m_localBound.GetLeft() ) );
		m_localClip.SetTop( m_localBound.GetTop() + XmlGetAttr( pRoot, "clipRectTop", m_localClip.GetTop() - m_localBound.GetTop() ) );
		m_localClip.SetRight( m_localBound.GetRight() - XmlGetAttr( pRoot, "clipRectRight", m_localBound.GetRight() - m_localClip.GetRight() ) );
		m_localClip.SetBottom( m_localBound.GetBottom() - XmlGetAttr( pRoot, "clipRectBottom", m_localBound.GetBottom() - m_localClip.GetBottom() ) );
	}
}

void CUIElement::CopyData( CUIElement* pElement, bool bInit )
{
	pElement->x = x;
	pElement->y = y;
	pElement->SetZOrder( GetZOrder() );
	pElement->bVisible = bVisible;
	pElement->m_strName = m_strName;
	pElement->m_localBound = m_localBound;
	pElement->m_nFlag = m_nFlag;
	pElement->m_bEnabled = m_bEnabled;
	pElement->m_bClipChildren = m_bClipChildren;
	pElement->m_localClip = m_localClip;
}

CUIElement* CUIElement::Clone( bool bInit )
{
	CUIElement* pElement = CreateObject();
	Clone( pElement, bInit );
	return pElement;
}

void CUIElement::Clone( CUIElement* pElement, bool bInit )
{
	CopyData( pElement, bInit );

	CRenderObject2D* pChildren = NULL;
	ForEachChild( [&pChildren, bInit] ( CUIElement* pElem ) {
		CUIElement* pChild = pElem->Clone( bInit );
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

	if( bInit )
		pElement->OnInited();
}

void CUIElement::Replace( CUIElement* pElement )
{
	pElement->GetParent()->AddChildBefore( this, pElement );
	SetPosition( pElement->GetPosition() );
	Resize( pElement->GetSize() );
	pElement->RemoveThis();
}

void CUIElement::DispatchMouseEvent( SUIMouseEvent& mouseEvent )
{
	m_events.Trigger( mouseEvent.nEvent, &mouseEvent );
	CUIElement* pParent = dynamic_cast<CUIElement*>( GetParent() );
	if( pParent && pParent != m_pMgr )
		pParent->DispatchMouseEvent( mouseEvent );
}

bool CUIElement::CalcAABB()
{
	CRectangle orig = globalAABB;
	globalAABB = m_localBound.Offset( globalTransform.GetPosition() );

	if( m_bClipChildren )
		globalAABB = globalAABB + m_localClip.Offset( globalTransform.GetPosition() );
	else
	{
		for( CRenderObject2D* pChild = m_pChildren; pChild; pChild = pChild->NextChild() ) {
			globalAABB = globalAABB + pChild->globalAABB;
		}
	}
	return !( orig == globalAABB );
}

void CUIElement::SetLayoutDirty()
{
	if( m_bLayoutDirty )
		return;
	m_bLayoutDirty = true;
	if( m_pMgr )
		m_pMgr->RegisterLayout( this );
}

void CUIElement::OnTransformUpdated()
{
	CUIElement* pUIElement = dynamic_cast<CUIElement*>( GetParent() );
	if( pUIElement )
	{
		globalClip = pUIElement->globalClip;
		if( m_bClipChildren )
			globalClip = globalClip * m_localClip.Offset( globalTransform.GetPosition() );
	}
}

void CUIElement::OnResize( const CRectangle& oldRect, const CRectangle& newRect )
{
	m_localClip.SetLeft( m_localClip.GetLeft() + newRect.GetLeft() - oldRect.GetLeft() );
	m_localClip.SetRight( m_localClip.GetRight() + newRect.GetRight() - oldRect.GetRight() );
	m_localClip.SetTop( m_localClip.GetTop() + newRect.GetTop() - oldRect.GetTop() );
	m_localClip.SetBottom( m_localClip.GetBottom() + newRect.GetBottom() - oldRect.GetBottom() );

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

void CUIElement::OnSetVisible( bool bVisible )
{
	m_events.Trigger( eEvent_SetVisible, (void*)bVisible );
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

void CUIElement::OnChar( uint32 nChar )
{
	m_events.Trigger( eEvent_Char, (void*)nChar );
}