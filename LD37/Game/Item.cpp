#include "stdafx.h"
#include "Item.h"
#include "Player.h"

void CItemCommon::Add( CPlayer * pPlayer )
{
	if( m_nHp )
		pPlayer->ModifyHp( m_nHp );
	if( m_nSp )
		pPlayer->ModifyHp( m_nSp );
}

void CItemCommon::Remove( CPlayer * pPlayer )
{
	if( m_nHp )
		pPlayer->ModifyHp( -m_nHp );
	if( m_nSp )
		pPlayer->ModifyHp( -m_nSp );
}
