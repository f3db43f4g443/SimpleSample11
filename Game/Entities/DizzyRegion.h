#pragma once
#include "Entity.h"

class CDizzyRegion : public CEntity
{
public:
	CDizzyRegion( float fForceScale, float fMaxForce ) : m_fForceScale( fForceScale ), m_fMaxForce( fMaxForce ) {}
	virtual void CheckPlayerDizzy( CPlayer* pPlayer, const CVector2& hitPoint, const CVector2& normal, SPlayerDizzyContext& result ) override;
private:
	float m_fForceScale;
	float m_fMaxForce;
};