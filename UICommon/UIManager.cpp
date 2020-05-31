#include "stdafx.h"
#include "UIManager.h"
#include "Render/Scene2DManager.h"

void CUIManager::AddElement( CUIElement* pElem )
{
	pElem->m_pMgr = this;
	pElem->ForEachChild( [this] ( CUIElement* pElem ) {
		AddElement( pElem );
		return true;
	} );
	if( pElem->m_bLayoutDirty )
	{
		RegisterLayout( pElem );
	}
}

void CUIManager::RemoveElement( CUIElement* pElem )
{
	pElem->ForEachChild( [this] ( CUIElement* pElem ) {
		if( pElem == m_pDragged )
		{
			pElem->OnStopDrag( CVector2( 0, 0 ) );
			m_pDragged = NULL;
		}
		if( pElem == GetFocus() )
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
	CReference<CUIElement> pElement = m_modalElements.size() ? m_modalElements.back()->MouseDown( mousePos ) : MouseDown( mousePos );
	SetFocus( pElement );
	if( pElement && pElement->GetMgr() != this )
		pElement = NULL;
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
	CUIElement* pElement = m_modalElements.size() ? m_modalElements.back()->MouseUp( mousePos ) : MouseUp( mousePos );
	if( pClicked && pClicked == pElement )
	{
		pClicked->OnClick( mousePos );
	}
	if( m_pDragDropObj )
	{
		m_pDragDropObj->RemoveThis();
		m_pDragDropObj = NULL;
	}
	return pElement;
}

CUIElement* CUIManager::HandleMouseMove( const CVector2& mousePos )
{
	m_mousePos = mousePos;
	CUIElement* pElement = m_modalElements.size() ? m_modalElements.back()->MouseMove( mousePos ) : MouseMove( mousePos );
	if( m_bMouseDown )
	{
		if( m_pDragged )
			m_pDragged->OnDragged( mousePos );
		if( m_pDragDropObj )
			m_pDragDropObj->SetPosition( m_mousePos );
	}
	return pElement;
}

void CUIManager::HandleKey( uint32 nChar, bool bKeyDown, bool bAltDown )
{
	auto pFocus = GetFocus();
	if( pFocus )
		pFocus->OnKey( nChar, bKeyDown, bAltDown );
}

void CUIManager::HandleChar( uint32 nChar )
{
	auto pFocus = GetFocus();
	if( pFocus )
		pFocus->OnChar( nChar );
}

void CUIManager::SetFocus( CUIElement* pFocus )
{
	DEFINE_TEMP_REF( pFocus );
	if( pFocus == GetFocus() )
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
		while( pElement && pElement != pParent )
		{
			pElement = dynamic_cast<CUIElement*>( pElement->GetParent() );
		}
		if( !pElement )
			return;
		pElement = pFocus;
		while( pElement != pParent )
		{
			pElement->SetActive( true );
			pElement = dynamic_cast<CUIElement*>( pElement->GetParent() );
		}
		pFocus->OnSetFocused( true );
	}
}

void CUIManager::DragDrop( CUIElement* pObj )
{
	if( !m_pDragged )
		return;
	if( m_pDragDropObj )
	{
		m_pDragDropObj->RemoveThis();
		m_pDragDropObj = NULL;
	}
	m_pDragged = NULL;
	m_pDragDropObj = pObj;
	AddChild( m_pDragDropObj );
	m_pDragDropObj->MoveToTopmost();
	m_pDragDropObj->SetPosition( m_mousePos );
}

void CUIManager::DoModal( CUIElement* pElement )
{
	pElement->SetVisible( true );
	SetFocus( pElement );
	m_modalElements.push_back( pElement );
}

void CUIManager::EndModal()
{
	if( !m_modalElements.size() )
		return;
	CUIElement* pElement = m_modalElements.back();
	pElement->SetVisible( false );
	m_modalElements.pop_back();
	SetFocus( m_modalElements.size() ? m_modalElements.back() : NULL );
}

void CUIManager::RegisterLayout( CUIElement* pElement )
{
	m_dirtyLayouts.push_back( pElement );
}

void CUIManager::UpdateLayout()
{
	if( !m_dirtyLayouts.size() )
		return;

	for( CUIElement* pElement : m_dirtyLayouts )
	{
		if( !pElement->m_bLayoutDirty || pElement->GetMgr() != this )
			continue;
		pElement->m_bLayoutDirty = false;
		pElement->DoLayout();
	}
	m_dirtyLayouts.clear();
	
	CScene2DManager::GetGlobalInst()->UpdateDirty();
}