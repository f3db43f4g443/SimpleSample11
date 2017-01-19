#include "stdafx.h"
#include "ParticleTimeoutEmitter.h"

void CParticleTimeoutEmitter::Update( SParticleInstanceData& data, float fTime, const CMatrix2D& transform )
{
	m_fTimeLeft -= fTime;
}

bool CParticleTimeoutEmitter::Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform )
{
	if( m_fTimeLeft <= 0 )
	{
		data.isEmitting = false;
		return false;
	}
	pParticleSystemData->GenerateSingleParticle( data, pData, transform );
	return true;
}