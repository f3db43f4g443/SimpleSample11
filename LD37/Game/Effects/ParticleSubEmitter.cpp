#include "stdafx.h"
#include "ParticleSubEmitter.h"
#include "Common/MathUtil.h"

CParticleSubEmitter::CParticleSubEmitter( uint32 nSubEmitterCount, float fLifeTime, float fDirMin, float fDirMax,
	const CVector2& sizeMin, const CVector2& sizeMax,
	const CVector2& s0Min, const CVector2& s0Max,
	const CVector2& vMin, const CVector2& vMax,
	const CVector2& aMin, const CVector2& aMax, bool bIgnoreGlobalTransform,
	bool bRotates0, bool bRotatev, bool bRotatea )
	: m_fLifeTime( fLifeTime )
	, m_fDirMin( fDirMin ), m_fDirMax( fDirMax )
	, m_sizeMin( sizeMin ), m_sizeMax( sizeMax )
	, m_s0Min( s0Min ), m_s0Max( s0Max )
	, m_vMin( vMin ), m_vMax( vMax )
	, m_aMin( aMin ), m_aMax( aMax )
	, m_bIgnoreGlobalTransform( bIgnoreGlobalTransform )
	, m_bRotates0( bRotates0 ), m_bRotatev( bRotatev ), m_bRotatea( bRotatea )
{
	Set();
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

void CParticleSubEmitter::Set()
{
	m_subEmitters.resize( m_nSubEmitterCount );
	float dDir = ( m_fDirMax - m_fDirMin ) / m_nSubEmitterCount;
	for( int i = 0; i < m_nSubEmitterCount; i++ )
	{
		auto& emitter = m_subEmitters[i];
		float fDir = ( 0.5f + SRand::Inst<eRand_Render>().Rand( -0.5f, 0.5f ) * ( 1 - m_fDirFixed ) + i ) * dDir + m_fDirMin;
		CMatrix2D mat;
		mat.Rotate( fDir );

		emitter.s0.x = SRand::Inst<eRand_Render>().Rand( m_s0Min.x, m_s0Max.x );
		emitter.s0.y = SRand::Inst<eRand_Render>().Rand( m_s0Min.y, m_s0Max.y );
		emitter.v.x = SRand::Inst<eRand_Render>().Rand( m_vMin.x, m_vMax.x );
		emitter.v.y = SRand::Inst<eRand_Render>().Rand( m_vMin.y, m_vMax.y );
		emitter.a.x = SRand::Inst<eRand_Render>().Rand( m_aMin.x, m_aMax.x );
		emitter.a.y = SRand::Inst<eRand_Render>().Rand( m_aMin.y, m_aMax.y );
		emitter.size.x = SRand::Inst<eRand_Render>().Rand( m_sizeMin.x, m_sizeMax.x );
		emitter.size.y = SRand::Inst<eRand_Render>().Rand( m_sizeMin.y, m_sizeMax.y );
		if( m_bRotates0 )
			emitter.s0 = mat.MulVector2Dir( emitter.s0 );
		if( m_bRotatev )
			emitter.v = mat.MulVector2Dir( emitter.v );
		if( m_bRotatea )
			emitter.a = mat.MulVector2Dir( emitter.a );
	}

	m_vecEmitterIndices.resize( m_nSubEmitterCount );
	for( int i = 0; i < m_nSubEmitterCount; i++ )
		m_vecEmitterIndices[i] = i;
	m_nCurEmitter = 0;
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

	if( !m_nCurEmitter )
		SRand::Inst<eRand_Render>().Shuffle( m_vecEmitterIndices );
	auto& emitter = m_subEmitters[m_nCurEmitter];
	CVector2 pos = emitter.s0 + emitter.v * m_fTime + emitter.a * ( m_fTime * m_fTime * 0.5f );
	CVector2 dir = emitter.v + emitter.a * m_fTime;
	dir.Normalize();
	float fPercent = m_fTime / m_fLifeTime;
	float fSize = ( emitter.size.x + ( emitter.size.y - emitter.size.x ) * fPercent );
	dir = dir * fSize;
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

	CRectangle ext = data.origRect.Scale( fSize ).Offset( CVector2( pos.x, pos.y ) );
	data.extRect = data.extRect + ext;

	m_nCurEmitter++;
	if( m_nCurEmitter >= m_subEmitters.size() )
		m_nCurEmitter -= m_subEmitters.size();
	return true;
}