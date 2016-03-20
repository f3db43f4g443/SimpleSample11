#pragma once
#include "UIElement.h"

class CUIManager : public CUIElement
{
public:
	CUIManager() : m_pDragged( NULL ), m_pFocus( NULL ), m_mousePos( 0, 0 ), m_bMouseDown( false ) { m_pMgr = this; }

	void AddElement( CUIElement* pElem );
	void RemoveElement( CUIElement* pElem );

	CUIElement* HandleMouseDown( const CVector2& mousePos );
	CUIElement* HandleMouseUp( const CVector2& mousePos );
	CUIElement* HandleMouseMove( const CVector2& mousePos );
	void HandleChar( uint32 nChar );

	CUIElement* GetDragged() { return m_pDragged; }
	CUIElement* GetFocus() { return m_pFocus; }
	void SetFocus( CUIElement* pFocus );

	const CVector2& GetMousePos() { return m_mousePos; }
private:
	CUIElement* m_pDragged;
	CUIElement* m_pFocus;
	CVector2 m_mousePos;
	bool m_bMouseDown;
};