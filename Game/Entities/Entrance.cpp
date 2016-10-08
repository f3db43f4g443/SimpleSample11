#include "stdafx.h"
#include "Entrance.h"
#include "Stage.h"
#include "Player.h"

CEntrance::CEntrance( const char* szStage, const char* szStartPoint, const char* szText, float fTime, float fCircleSize )
	: CFastUseable( szText, fTime, fCircleSize )
	, m_strStageName( szStage )
	, m_strStartPointName( szStartPoint )
{
}

CEntrance::CEntrance( const SClassCreateContext& context )
	: CFastUseable( context )
	, m_strStageName( context )
	, m_strStartPointName( context )
{
}

void CEntrance::OnUse()
{
	SetEnabled( false );
	GetStage()->GetPlayer()->DelayChangeStage( m_strStageName.c_str(), m_strStartPointName.c_str() );
}