#pragma once

struct SPlayerDizzyEffectSpinning
{
	SPlayerDizzyEffectSpinning( float fRadMin[4] = NULL, float fRadMax[4] = NULL, float fTimeScaleMin[4] = NULL, float fTimeScaleMax[4] = NULL );
	float m_fRad[4], m_fTimeScale[4];

	void AddPlayerDizzy( class CPlayer* pPlayer, float fPercent );
};