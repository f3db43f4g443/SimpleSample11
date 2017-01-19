#include "stdafx.h"
#include "PlayerWeapon.h"
#include"Render/Image2D.h"

void CPlayerWeapon::Face( bool bLeft )
{
	if( bLeft )
		static_cast<CImage2D*>( GetRenderObject() )->SetTexRect( m_texRectFaceLeft );
	else
		static_cast<CImage2D*>( GetRenderObject() )->SetTexRect( m_texRectFaceRight );
}