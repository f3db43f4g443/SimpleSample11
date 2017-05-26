#include "stdafx.h"
#include "PlayerWeapon.h"
#include "Render/Image2D.h"
#include "Player.h"

void CPlayerWeapon::Face( bool bLeft )
{
	if( bLeft )
		static_cast<CImage2D*>( GetRenderObject() )->SetTexRect( m_texRectFaceLeft );
	else
		static_cast<CImage2D*>( GetRenderObject() )->SetTexRect( m_texRectFaceRight );
}

void CPlayerWeapon::Add( CPlayer * pPlayer )
{
	pPlayer->SetWeapon( this );
}

void CPlayerWeapon::Remove( CPlayer * pPlayer )
{
	pPlayer->SetWeapon( NULL );
}
