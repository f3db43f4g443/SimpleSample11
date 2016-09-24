#pragma once
#include "UIButton.h"

class CUICheckBox : public CUIButton
{
public:
	CUICheckBox() : m_bChecked( NULL ) {}
	enum
	{
		eState_Checked = eState_Button_Count,
		eState_Disable_Checked,
		eState_Unchecked,
		eState_Disable_Unchecked,

		eState_CheckBox_Count,
	};

	bool IsChecked() { return m_bChecked; }
	void SetChecked( bool bChecked );
	virtual CUIElement* CreateObject() override { return new CUICheckBox; }
	virtual void Render( CRenderContext2D& context ) override;
protected:
	virtual void OnClick( const CVector2& mousePos ) override;
private:
	bool m_bChecked;
};