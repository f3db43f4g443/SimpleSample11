#pragma once
#include "UICommon/UIViewport.h"
#include "FaceEditItem.h"

struct SSubStage;
class CFaceView : public CUIViewport
{
public:
	enum
	{
		eState_None,
		eState_Edit,
		eState_Action,
		eState_SelectFaceTarget
	};
	static CFaceView* Create( CUIElement* pElem );

	CFaceView() : m_pSubStage( NULL ), m_pFaceEditItem( NULL ), m_nState( eState_None ), m_bIsEditValid( false ), m_bGridMoved( false ), m_bFocused( false ) {}
	SSubStage* GetSubStage() { return m_pSubStage; }
	void SetSubStage( SSubStage* pSubStage ) { m_pSubStage = pSubStage; }
	void Select( CFaceEditItem* pItem ) { m_pFaceEditItem = pItem; OnMouseMove( CVector2( 0, 0 ) ); }
	void OnFocused( bool bFocused );

	uint8 GetState() { return m_nState; }
	void SetState( uint8 nState );
protected:
	virtual void OnInited() override;

	virtual void OnClick( const CVector2& mousePos ) override;
	virtual void OnMouseMove( const CVector2& mousePos ) override;
	virtual void OnStartDrag( const CVector2& mousePos ) override;
	virtual void OnDragged( const CVector2& mousePos ) override;
	virtual void OnStopDrag( const CVector2& mousePos ) override;

	bool TryEdit();

	TRectangle<int32> m_curSelectedRect;
	SSubStage* m_pSubStage;
	CFaceEditItem* m_pFaceEditItem;
	uint8 m_nState;
	bool m_bIsEditValid;
	bool m_bGridMoved;
	bool m_bFocused;
};