#pragma once
#include "Render/ParticleSystem.h"

class CParticleArrayEmitter : public IParticleEmitter
{
	friend void RegisterGameClasses();
public:
	CParticleArrayEmitter( const struct SClassCreateContext& context ) {}
	CParticleArrayEmitter( uint32 nWidth, uint32 nHeight, const CVector2& base, const CVector2& ofs, bool bIgnoreGlobalTransform )
		: m_nWidth( nWidth ), m_nHeight( nHeight ), m_base( base ), m_ofs( ofs ), m_bIgnoreGlobalTransform( bIgnoreGlobalTransform ){}

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