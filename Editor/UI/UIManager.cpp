#include "stdafx.h"
#include "UIManager.h"

void CUIManager::AddElement( CUIElement* pElem )
{
	pElem->m_pMgr = this;
	pElem->ForEachChild( [this] ( CUIElement* pElem ) {
		AddElement( pElem );
		return true;
	} );
}

void CUIManager::RemoveElement( CUIElement* pElem )
{
	pElem->ForEachChild( [this] ( CUIElement* pElem ) {
		if( pElem == m_pDragged )
		{
			pElem->OnStopDrag( CVector2( 0, 0 ) );
			m_pDragged = NULL;
		}
		if( pElem == m_pFocus )
			SetFocus( NULL );
		RemoveElement( pElem );
		return true;
	} );
	pElem->m_pMgr = NULL;
}

CUIElement* CUIManager::HandleMouseDown( const CVector2& mousePos )
{
	m_mousePos = mousePos;
	if( m_bMouseDown )
		return NULL;
	m_bMouseDown = true;
	CUIElement* pElement = MouseDown( mousePos );
	SetFocus( pElement );
	m_pDragged = pElement;
	if( m_pDragged )
		m_pDragged->OnStartDrag( mousePos );
	return pElement;
}

CUIElement* CUIManager::HandleMouseUp( const CVector2& mousePos )
{
	m_mousePos = mousePos;
	if( !m_bMouseDown )
		return NULL;
	m_bMouseDown = false;

	CUIElement* pClicked = m_pDragged;
	if( m_pDragged )
		m_pDragged->OnStopDrag( mousePos );
	m_pDragged = NULL;
	CUIElement* pElement = MouseUp( mousePos );
	if( pClicked && pClicked == pElement )
	{
		pClicked->OnClick( mousePos );
	}
	return pElement;
}

CUIElement* CUIManager::HandleMouseMove( const CVector2& mousePos )
{
	m_mousePos = mousePos;
	CUIElement* pElement = MouseMove( mousePos );
	if( m_bMouseDown )
	{
		if( m_pDragged )
			m_pDragged->OnDragged( mousePos );
	}
	return pElement;
}

void CUIManager::HandleChar( uint32 nChar )
{
	if( m_pFocus )
		m_pFocus->OnChar( nChar );
}

void CUIManager::SetFocus( CUIElement* pFocus )
{
	if( pFocus == m_pFocus )
		return;

	CUIElement* pParent;
	if( m_pFocus && pFocus )
	{
		CRenderObject2D* pRenderObject = CRenderObject2D::FindCommonParent( pFocus, m_pFocus );
		if( !pRenderObject )
			return;
		pParent = dynamic_cast<CUIElement*>( pRenderObject );
		if( !pParent )
			return;
	}
	else
		pParent = this;

	CUIElement* pElement;
	if( m_pFocus )
	{
		m_pFocus->OnSetFocused( false );
		pElement = m_pFocus;
		while( pElement != pParent )
		{
			pElement->SetActive( false );
			pElement = dynamic_cast<CUIElement*>( pElement->GetParent() );
		}
	}

	m_pFocus = pFocus;
	if( pFocus )
	{
		pElement = pFocus;
		while( pElement != pParent )
		{
			pElement->SetActive( true );
			pElement = dynamic_cast<CUIElement*>( pElement->GetParent() );
		}
		pFocus->OnSetFocused( true );
	}
}