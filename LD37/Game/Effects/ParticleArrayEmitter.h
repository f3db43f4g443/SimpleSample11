#pragma once
#include "Render/ParticleSystem.h"

class CParticleArrayEmitter : public IParticleEmitter
{
	friend void RegisterGameClasses();
public:
	CParticleArrayEmitter( const struct SClassCreateContext& context ) {}

	virtual void Init( SParticleInstanceData& data ) override { m_nEmitCount = 0; }
	virtual bool Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform ) override;
private:
	uint32 m_nWidth;
	uint32 m_nHeight;
	CVector2 m_base;
	CVector2 m_ofs;
	bool m_bIgnoreGlobalTransform;

	uint32 m_nEmitCount;
};