#include "stdafx.h"
#include "ParticleArrayEmitter.h"

bool CParticleArrayEmitter::Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform )
{
	if( !m_nWidth || !m_nHeight )
		return false;
	uint32 nX = m_nEmitCount % m_nWidth;
	uint32 nY = m_nEmitCount / m_nWidth;
	CVector2 pos = m_base + m_ofs * CVector2( nX, nY );

	CMatrix2D mat;
	mat.Translate( pos.x, pos.y );

	if( m_bIgnoreGlobalTransform )
		pParticleSystemData->GenerateSingleParticle( data, pData, mat );
	else
	{
		CMatrix2D mat1 = transform * mat;
		pParticleSystemData->GenerateSingleParticle( data, pData, mat1 );
	}

	m_nEmitCount++;
	if( m_nEmitCount == m_nWidth * m_nHeight )
		m_nEmitCount = 0;
	return true;
}