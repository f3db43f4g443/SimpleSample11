#include "stdafx.h"
#include "UIButton.h"
#include "UIManager.h"

void CUIButton::Render( CRenderContext2D& context )
{
	if( IsEnabled() )
	{
		if( m_localBound.Contains( GetMgr()->GetMousePos() - globalTransform.GetPosition() ) )
		{
			if( IsDragged() )
				ShowImageList( eState_Down );
			else
				ShowImageList( eState_Over );
		}
		else
			ShowImageList( eState_Normal );
	}
	else
		ShowImageList( eState_Disabled );
}

void CUIButton::OnClick( const CVector2& mousePos )
{
	if( IsEnabled() )
		m_events.Trigger( eEvent_Action, NULL );
}