#include "stdafx.h"
#include "PlayerDizzy.h"
#include "Player.h"
#include "MyGame.h"
#include "Common/MathUtil.h"

SPlayerDizzyEffectSpinning::SPlayerDizzyEffectSpinning( float fRadMin[4], float fRadMax[4], float fTimeScaleMin[4], float fTimeScaleMax[4] )
{
	static float g_radMin[4] = { 45, 27, 18, 9 };
	static float g_radMax[4] = { 55, 33, 22, 11 };
	static float g_timeScaleMin[4] = { 1.5f, 2.5f, 4.0f, 6.5f };
	static float g_timeScaleMax[4] = { 1.0f, 2.0f, 3.0f, 5.0f };
	if( !fRadMin )
		fRadMin = g_radMin;
	if( !fRadMax )
		fRadMax = g_radMax;
	if( !fTimeScaleMin )
		fTimeScaleMin = g_timeScaleMin;
	if( !fTimeScaleMax )
		fTimeScaleMax = g_timeScaleMax;
	
	for( int i = 0; i < 4; i++ )
	{
		m_fRad[i] = SRand::Inst().Rand( fRadMin[i], fRadMax[i] );
		m_fTimeScale[i] = SRand::Inst().Rand( fTimeScaleMin[i], fTimeScaleMax[i] ) * ( SRand::Inst().Rand() & 1 ? 1 : -1 );
	}
}

void SPlayerDizzyEffectSpinning::AddPlayerDizzy( CPlayer* pPlayer, float fPercent )
{
	double time = CGame::Inst().GetTotalTime();
	SPlayerDizzyContext context;
	context.fPercent = fPercent;
	context.pushForce = CVector2( 0, 0 );
	for( int i = 0; i < ELEM_COUNT( m_fTimeScale ); i++ )
	{
		double t = time * m_fTimeScale[i];
		context.pushForce = context.pushForce + CVector2( cos( t ), sin( t ) ) * ( m_fRad[i] * fPercent );
	}
	pPlayer->AddDizzyThisFrame( context );
}