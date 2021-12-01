#pragma once
#include "UILabel.h"

class CUITextBox : public CUILabel
{
public:
	CUITextBox() { SetEnableMouseEvent( true ); }
	virtual CUIElement* CreateObject() override { return new CUITextBox; }
protected:
	virtual void OnStartDrag( const CVector2& mousePos ) override;
	virtual void OnDragged( const CVector2& mousePos ) override;
	virtual void OnSetFocused( bool bFocused ) override;
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) override {}
	virtual void OnChar( uint32 nChar ) override;
};