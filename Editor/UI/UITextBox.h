#pragma once
#include "UILabel.h"

class CUITextBox : public CUILabel
{
public:
	CUITextBox() { SetEnableMouseEvent( true ); }
	virtual CUIElement* CreateObject() override { return new CUITextBox; }
protected:
	virtual void OnMouseDown( const CVector2& mousePos ) override;
	virtual void OnSetFocused( bool bFocused ) override;
	virtual void OnChar( uint32 nChar ) override;
};