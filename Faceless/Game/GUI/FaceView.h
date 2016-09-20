#pragma once
#include "UICommon/UIViewport.h"
#include "FaceEditItem.h"

struct SSubStage;
class CFaceView : public CUIViewport
{
public:
	static CFaceView* Create( CUIElement* pElem );

	CFaceView() : m_pSubStage( NULL ), m_pFaceEditItem( NULL ), m_bIsEditValid( false ), m_bGridMoved( false ) {}
	SSubStage* GetSubStage() { return m_pSubStage; }
	void SetSubStage( SSubStage* pSubStage ) { m_pSubStage = pSubStage; }
	void Select( CFaceEditItem* pItem ) { m_pFaceEditItem = pItem; OnMouseMove( CVector2( 0, 0 ) ); }
protected:
	virtual void OnInited() override;

	virtual void OnMouseMove( const CVector2& mousePos ) override;
	virtual void OnStartDrag( const CVector2& mousePos ) override;
	virtual void OnDragged( const CVector2& mousePos ) override;
	virtual void OnStopDrag( const CVector2& mousePos ) override;

	bool TryEdit();

	TRectangle<int32> m_curSelectedRect;
	SSubStage* m_pSubStage;
	CFaceEditItem* m_pFaceEditItem;
	bool m_bIsEditValid;
	bool m_bGridMoved;
};