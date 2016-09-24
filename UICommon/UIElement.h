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
	CUIElement() : m_pMgr( NULL ), m_nFlag( 0 ), m_bActive( false ), m_bEnabled( true ), m_bClipChildren( false ), m_bLayoutDirty( false ), m_localClip( 0, 0, 0, 0 ) {}

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
		eEvent_SetVisible,
		eEvent_SetEnabled,
		eEvent_SetActive,
		eEvent_SetFocused,

		eEvent_Char,

		eEvent_Count,
	};

	class CUIManager* GetMgr() { return m_pMgr; }
	const char* GetName() { return m_strName.c_str(); }

	CUIElement* MouseDown( const CVector2& mousePos );
	CUIElement* MouseUp( const CVector2& mousePos );
	CUIElement* MouseMove( const CVector2& mousePos );

	void Action( void* pContext = NULL ) { m_events.Trigger( eEvent_Action, pContext ); }

	void SetVisible( bool bVisible );
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
	bool IsClipChildren() { return m_bClipChildren; }
	const CRectangle& GetLocalClip() { return m_localClip; }
	void SetLocalClip( bool bClip, const CRectangle& rect ) { m_bClipChildren = bClip; m_localClip = rect; SetTransformDirty(); }

	virtual void LoadXml( TiXmlElement* pRoot );
	virtual CUIElement* CreateObject() { return new CUIElement; }
	CUIElement* Clone( bool bInit = true );
	void Clone( CUIElement* pElement, bool bInit = true );
	void Replace( CUIElement* pElement );

	void ForEachChild( function<bool( CUIElement* pElem )> func );
	template<typename T = CUIElement>
	T* GetChildByName( const char* szName )
	{
		for( auto pRenderObject = m_pChildren; pRenderObject; pRenderObject = pRenderObject->NextChild() )
		{
			T* pElem = dynamic_cast<T*>( pRenderObject );
			if( !pElem )
				continue;
			if( pElem->m_strName == szName )
				return pElem;
		}
		return NULL;
	}

	void Register( uint32 iEvent, CTrigger* pTrigger ) { m_events.Register( iEvent, pTrigger ); }
	void DispatchMouseEvent( SUIMouseEvent& mouseEvent );

	virtual bool CalcAABB() override;
	void SetLayoutDirty();

	CRectangle globalClip;
protected:
	virtual void OnTransformUpdated() override;

	virtual void OnInited() {}

	virtual void OnResize( const CRectangle& oldRect, const CRectangle& newRect );
	virtual void OnMouseDown( const CVector2& mousePos );
	virtual void OnMouseUp( const CVector2& mousePos );
	virtual void OnMouseMove( const CVector2& mousePos );
	virtual void OnClick( const CVector2& mousePos ) {}
	virtual void OnStartDrag( const CVector2& mousePos );
	virtual void OnDragged( const CVector2& mousePos );
	virtual void OnStopDrag( const CVector2& mousePos );
	
	virtual void OnSetVisible( bool bVisible );
	virtual void OnSetEnabled( bool bEnabled );
	virtual void OnSetActive( bool bActive );
	virtual void OnSetFocused( bool bFocused );

	virtual void OnChar( uint32 nChar );

	virtual void DoLayout() {}

	void SetActive( bool bActive );

	virtual void CopyData( CUIElement* pElement, bool bInit );

	virtual void OnAdded() override;
	virtual void OnRemoved() override;

	CEventTrigger<eEvent_Count> m_events;
private:
	class CUIManager* m_pMgr;
	string m_strName;
	uint32 m_nFlag;
	bool m_bActive;
	bool m_bEnabled;

	bool m_bLayoutDirty;
	bool m_bClipChildren;
	CRectangle m_localClip;
};

struct SUIMouseEvent
{
	uint32 nEvent;
	CReference<CUIElement> pTarget;
	CVector2 mousePos;
};