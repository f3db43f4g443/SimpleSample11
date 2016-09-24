#pragma once
#include "UILabel.h"

class CUIButton : public CUILabel
{
public:
	CUIButton() { SetEnableMouseEvent( true ); }

	enum
	{
		eState_Over = eState_Label_Count,
		eState_Down,
		eState_Button_Count
	};

	virtual CUIElement* CreateObject() override { return new CUIButton; }
	virtual void Render( CRenderContext2D& context ) override;
protected:
	virtual void OnClick( const CVector2& mousePos ) override;
};