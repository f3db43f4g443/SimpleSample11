#pragma once
#include "Render/ParticleSystem.h"

class CParticleTimeoutEmitter : public IParticleEmitter
{
	friend void RegisterGameClasses();
public:
	CParticleTimeoutEmitter( const struct SClassCreateContext& context ) {}
	
	virtual void Init( SParticleInstanceData& data ) override { m_fTimeLeft = m_fTime; }
	virtual void Update( SParticleInstanceData& data, float fTime, const CMatrix2D& transform ) override;
	virtual bool Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform ) override;
protected:
	float m_fTime;
	float m_fTimeLeft;
};