#include "stdafx.h"
#include "PlayerAction.h"

CPlayerAction::CPlayerAction( const char* szName, EPlayerActionType eType )
	: m_strName( szName )
	, m_eType( eType )
	, m_nState( 0 )
{

}

bool CPlayerActionCommon::Do( CPlayer* pPlayer )
{
	if( !OnDo( pPlayer ) )
		return false;
	return true;
}

bool CPlayerActionCommon::Stop( CPlayer* pPlayer )
{
	return false;
}
