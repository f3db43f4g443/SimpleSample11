#include "stdafx.h"
#include "UICheckBox.h"
#include "UIManager.h"

void CUICheckBox::Render( CRenderContext2D& context )
{
	CUIButton::Render( context );
	if( m_bChecked )
	{
		SetImageListVisible( eState_Unchecked, false );
		SetImageListVisible( eState_Disable_Unchecked, false );
		if( IsEnabled() )
		{
			SetImageListVisible( eState_Checked, true );
			SetImageListVisible( eState_Disable_Checked, false );
		}
		else
		{
			SetImageListVisible( eState_Checked, false );
			SetImageListVisible( eState_Disable_Checked, true );
		}
	}
	else
	{
		SetImageListVisible( eState_Checked, false );
		SetImageListVisible( eState_Disable_Checked, false );
		if( IsEnabled() )
		{
			SetImageListVisible( eState_Unchecked, true );
			SetImageListVisible( eState_Disable_Unchecked, false );
		}
		else
		{
			SetImageListVisible( eState_Unchecked, false );
			SetImageListVisible( eState_Disable_Unchecked, true );
		}
	}
}

void CUICheckBox::SetChecked( bool bChecked, bool bTriggerEvent )
{
	if( bChecked == m_bChecked )
		return;
	m_bChecked = bChecked;
	if( bTriggerEvent )
		m_events.Trigger( eEvent_Action, NULL );
}

void CUICheckBox::OnClick( const CVector2& mousePos )
{
	if( IsEnabled() )
	{
		m_bChecked = !m_bChecked;
		m_events.Trigger( eEvent_Action, NULL );
	}
}