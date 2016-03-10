#include "stdafx.h"
#include "ParticleWebEmitter.h"
#include "Common/MathUtil.h"

bool CParticleWebEmitter::Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform )
{
	if( m_fTime >= m_fLifeTime || !m_subEmitters.size() )
	{
		data.isEmitting = false;
		return false;
	}
	float fPercent = m_fTime / m_fLifeTime;

	uint32 nEmittersSize = m_subEmitters.size();
	uint32 nEmitter = m_nCurEmitter;
	CVector2 pos, dir;
	uint32 iEmitter = nEmitter >> 1;
	bool bRight = nEmitter & 1;
	uint32 n = m_bLoop ? nEmittersSize : nEmittersSize - 1;
	if( iEmitter >= n )
	{
		iEmitter -= n;
		bRight = !bRight;
	}

	auto& emitter1 = m_subEmitters[iEmitter];
	auto& emitter2 = m_subEmitters[iEmitter < nEmittersSize - 1 ? iEmitter + 1 : 0];
	CVector2 pos1, pos2;
	float fTime1 = m_fTime * SRand::Inst().Rand( 0.5f, 1.0f );
	if( bRight )
	{
		pos1 = emitter2.s0 + emitter2.v * m_fTime + emitter2.a * ( m_fTime * m_fTime * 0.5f );
		pos2 = emitter1.s0 + emitter1.v * fTime1 + emitter1.a * ( fTime1 * fTime1 * 0.5f );
	}
	else
	{
		pos1 = emitter1.s0 + emitter1.v * m_fTime + emitter1.a * ( m_fTime * m_fTime * 0.5f );
		pos2 = emitter2.s0 + emitter2.v * fTime1 + emitter2.a * ( fTime1 * fTime1 * 0.5f );
	}
	pos = pos1;
	dir = pos2 - pos1;
	dir = dir * m_fScaleLat;

	CMatrix2D mat;
	mat.m00 = dir.x;
	mat.m01 = -dir.y;
	mat.m02 = pos.x;
	mat.m10 = dir.y;
	mat.m11 = dir.x;
	mat.m12 = pos.y;
	mat.m20 = 0;
	mat.m21 = 0;
	mat.m22 = 1.0f;
	if( m_bIgnoreGlobalTransform )
		pParticleSystemData->GenerateSingleParticle( data, pData, mat );
	else
	{
		CMatrix2D mat1 = transform * mat;
		pParticleSystemData->GenerateSingleParticle( data, pData, mat1 );
	}

	m_nCurEmitter++;
	uint32 nPeriod = m_bLoop ? nEmittersSize * 4 : nEmittersSize * 4 - 4;
	if( m_nCurEmitter >= nPeriod )
		m_nCurEmitter -= nPeriod;
	return true;
}