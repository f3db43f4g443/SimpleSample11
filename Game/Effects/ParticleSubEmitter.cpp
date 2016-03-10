#include "stdafx.h"
#include "ParticleSubEmitter.h"
#include "Common/MathUtil.h"

CParticleSubEmitter::CParticleSubEmitter( uint32 nSubEmitterCount, float fLifeTime, float fDirMin, float fDirMax,
	const CVector2& sizeMin, const CVector2& sizeMax,
	const CVector2& s0Min, const CVector2& s0Max,
	const CVector2& vMin, const CVector2& vMax,
	const CVector2& aMin, const CVector2& aMax, bool bIgnoreGlobalTransform )
	: m_fLifeTime( fLifeTime )
	, m_bIgnoreGlobalTransform( bIgnoreGlobalTransform )
{
	m_subEmitters.resize( nSubEmitterCount );
	float dDir = ( fDirMax - fDirMin ) / nSubEmitterCount;
	for( int i = 0; i < nSubEmitterCount; i++ )
	{
		auto& emitter = m_subEmitters[i];
		float fDir = SRand::Inst().Rand( 0.0f, dDir ) + fDirMin + dDir * i;
		CMatrix2D mat;
		mat.Rotate( fDir );

		emitter.s0.x = SRand::Inst().Rand( s0Min.x, s0Max.x );
		emitter.s0.y = SRand::Inst().Rand( s0Min.y, s0Max.y );
		emitter.v.x = SRand::Inst().Rand( vMin.x, vMax.x );
		emitter.v.y = SRand::Inst().Rand( vMin.y, vMax.y );
		emitter.a.x = SRand::Inst().Rand( aMin.x, aMax.x );
		emitter.a.y = SRand::Inst().Rand( aMin.y, aMax.y );
		emitter.size.x = SRand::Inst().Rand( sizeMin.x, sizeMax.x );
		emitter.size.y = SRand::Inst().Rand( sizeMin.y, sizeMax.y );
		emitter.s0 = mat.MulVector2Dir( emitter.s0 );
		emitter.v = mat.MulVector2Dir( emitter.v );
		emitter.a = mat.MulVector2Dir( emitter.a );
	}
}

CParticleSubEmitter::CParticleSubEmitter( uint32 nSubEmitterCount, float fLifeTime, SSubEmitter* pSubEmitters, bool bIgnoreGlobalTransform )
	: m_fLifeTime( fLifeTime )
	, m_bIgnoreGlobalTransform( bIgnoreGlobalTransform )
{
	m_subEmitters.resize( nSubEmitterCount );
	for( int i = 0; i < nSubEmitterCount; i++ )
	{
		m_subEmitters[i] = pSubEmitters[i];
	}
}

void CParticleSubEmitter::Update( SParticleInstanceData& data, float fTime, const CMatrix2D& transform )
{
	m_fTime += fTime;
}

bool CParticleSubEmitter::Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform )
{
	if( m_fTime >= m_fLifeTime || !m_subEmitters.size() )
	{
		data.isEmitting = false;
		return false;
	}

	auto& emitter = m_subEmitters[m_nCurEmitter];
	CVector2 pos = emitter.s0 + emitter.v * m_fTime + emitter.a * ( m_fTime * m_fTime * 0.5f );
	CVector2 dir = emitter.v + emitter.a * m_fTime;
	dir.Normalize();
	float fPercent = m_fTime / m_fLifeTime;
	dir = dir * ( emitter.size.x + ( emitter.size.y - emitter.size.x ) * fPercent );
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
	if( m_nCurEmitter >= m_subEmitters.size() )
		m_nCurEmitter -= m_subEmitters.size();
	return true;
}