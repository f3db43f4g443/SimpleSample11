#pragma once
#include "UIElement.h"

class CUIManager : public CUIElement
{
public:
	CUIManager() : m_pDragged( NULL ), m_pFocus( NULL ), m_mousePos( 0, 0 ), m_bMouseDown( false )
		, m_key( 128 ), m_keyDown( 128 ), m_keyUp( 128 ) { m_pMgr = this; }

	void AddElement( CUIElement* pElem );
	void RemoveElement( CUIElement* pElem );

	void OnUpdate();
	CUIElement* HandleMouseDown( const CVector2& mousePos );
	CUIElement* HandleMouseUp( const CVector2& mousePos );
	CUIElement* HandleMouseMove( const CVector2& mousePos );
	CUIElement* HandleMouseWheel( int32 nDelta );
	void HandleKey( uint32 nChar, bool bKeyDown, bool bAltDown );
	void HandleChar( uint32 nChar );
	bool IsKey( uint32 nKey ) { return m_key.GetBit( nKey ); }
	bool IsKeyUp( uint32 nKey ) { return m_keyUp.GetBit( nKey ); }
	bool IsKeyDown( uint32 nKey ) { return m_keyDown.GetBit( nKey ); }

	CUIElement* GetDragged() { if( m_pDragged && m_pDragged->m_pMgr != this ) m_pDragged = NULL; return m_pDragged; }
	CUIElement* GetFocus() { if( m_pFocus && m_pFocus->m_pMgr != this ) m_pFocus = NULL; return m_pFocus; }
	void SetFocus( CUIElement* pFocus );
	void DragDrop( CUIElement* pObj );
	CUIElement* GetDragDropObject() { return m_pDragDropObj; }

	void DoModal( CUIElement* pElement );
	void EndModal();
	
	void RegisterLayout( CUIElement* pElement );
	void UpdateLayout();

	const CVector2& GetMousePos() { return m_mousePos; }
protected:
	virtual void OnTransformUpdated() override { globalClip = m_localBound.Offset( globalTransform.GetPosition() ); }
private:
	CReference<CUIElement> m_pDragged;
	CReference<CUIElement> m_pFocus;
	CVector2 m_mousePos;
	bool m_bMouseDown;
	CReference<CUIElement> m_pDragDropObj;

	CBitArray m_key;
	CBitArray m_keyUp;
	CBitArray m_keyDown;

	vector<CReference<CUIElement> > m_modalElements;
	vector<CReference<CUIElement> > m_dirtyLayouts;
};