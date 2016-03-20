#pragma once
#include "Render/RenderObject2D.h"
#include "Common/Trigger.h"
#include <functional>

enum
{
	eUIAlignType_Center = 0,
	eUIAlignType_Left = 1,
	eUIAlignType_Right = 2,
	eUIAlignType_Both = 3,

	eUIAlignType_Mask = 0xf,

	eUIFlag_MouseEvent = 0x10,
};

class TiXmlElement;
struct SUIMouseEvent;
class CUIElement : public CRenderObject2D
{
	friend class CUIManager;
public:
	CUIElement() : m_pMgr( NULL ), m_nFlag( 0 ), m_bActive( false ) {}

	enum
	{
		eEvent_MouseDown,
		eEvent_MouseUp,
		eEvent_MouseMove,
		eEvent_StartDrag,
		eEvent_Dragged,
		eEvent_StopDrag,

		eEvent_Action,
		eEvent_Resize,
		eEvent_SetEnabled,
		eEvent_SetActive,
		eEvent_SetFocused,

		eEvent_Count,
	};

	class CUIManager* GetMgr() { return m_pMgr; }
	const char* GetName() { return m_strName.c_str(); }

	CUIElement* MouseDown( const CVector2& mousePos );
	CUIElement* MouseUp( const CVector2& mousePos );
	CUIElement* MouseMove( const CVector2& mousePos );

	bool IsEnabled() { return m_bEnabled; }
	void SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; OnSetEnabled( bEnabled ); }
	bool IsActive() { return m_bActive; }
	bool IsFocused();
	bool IsDragged();
	void Focus();

	void Resize( const CRectangle& rect );
	const CRectangle& GetSize() { return m_localBound; }
	bool IsEnableMouseEvent() { return m_nFlag & eUIFlag_MouseEvent; }
	void SetEnableMouseEvent( bool bEnable ) { m_nFlag = ( m_nFlag & ~eUIFlag_MouseEvent ) | ( bEnable? eUIFlag_MouseEvent: 0 ); }
	uint32 GetAlignType() { return m_nFlag & eUIAlignType_Mask; }
	void SetAlignType( uint32 eType ) { m_nFlag = ( m_nFlag & ~eUIAlignType_Mask ) | ( eType & eUIAlignType_Mask ); }

	virtual void LoadXml( TiXmlElement* pRoot );
	virtual CUIElement* CreateObject() { return new CUIElement; }
	CUIElement* Clone();

	void ForEachChild( function<bool( CUIElement* pElem )> func );

	void Register( uint32 iEvent, CTrigger* pTrigger ) { m_events.Register( iEvent, pTrigger ); }
	void DispatchMouseEvent( SUIMouseEvent& mouseEvent );
protected:
	virtual void OnResize( const CRectangle& oldRect, const CRectangle& newRect );
	virtual void OnMouseDown( const CVector2& mousePos );
	virtual void OnMouseUp( const CVector2& mousePos );
	virtual void OnMouseMove( const CVector2& mousePos );
	virtual void OnClick( const CVector2& mousePos ) {}
	virtual void OnStartDrag( const CVector2& mousePos );
	virtual void OnDragged( const CVector2& mousePos );
	virtual void OnStopDrag( const CVector2& mousePos );
	
	virtual void OnSetEnabled( bool bEnabled );
	virtual void OnSetActive( bool bActive );
	virtual void OnSetFocused( bool bFocused );

	virtual void OnChar( uint32 nChar ) {}

	void SetActive( bool bActive );

	virtual void CopyData( CUIElement* pElement );

	virtual void OnAdded() override;
	virtual void OnRemoved() override;

	CEventTrigger<eEvent_Count> m_events;
private:
	class CUIManager* m_pMgr;
	string m_strName;
	uint32 m_nFlag;
	bool m_bActive;
	bool m_bEnabled;
};

struct SUIMouseEvent
{
	uint32 nEvent;
	CReference<CUIElement> pTarget;
	CVector2 mousePos;
};