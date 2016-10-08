#include "stdafx.h"
#include "DizzyRegion.h"
#include "Player.h"

void CDizzyRegion::CheckPlayerDizzy( CPlayer* pPlayer, const CVector2& hitPoint, const CVector2& normal, SPlayerDizzyContext& result )
{
	CVector2 force = normal;
	float l = force.Normalize() * m_fForceScale;
	if( l > m_fMaxForce )
		l = m_fMaxForce;
	result.pushForce = force * l;
	result.fPercent = l / m_fMaxForce;
}