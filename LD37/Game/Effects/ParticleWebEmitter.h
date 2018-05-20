#pragma once
#include "Effects/ParticleSubEmitter.h"

class CParticleWebEmitter : public CParticleSubEmitter
{
	friend void RegisterGameClasses();
public:
	CParticleWebEmitter( const struct SClassCreateContext& context ) : CParticleSubEmitter( context ) {}
	CParticleWebEmitter( uint32 nSubEmitterCount, float fLifeTime, float fScaleLon, float fScaleLat, bool bLoop, float fDirMin, float fDirMax,
		const CVector2& sizeMin = CVector2( 1, 0 ), const CVector2& sizeMax = CVector2( 1, 0 ),
		const CVector2& s0Min = CVector2( 0, 0 ), const CVector2& s0Max = CVector2( 0, 0 ),
		const CVector2& vMin = CVector2( 0, 0 ), const CVector2& vMax = CVector2( 0, 0 ),
		const CVector2& aMin = CVector2( 0, 0 ), const CVector2& aMax = CVector2( 0, 0 ), bool bIgnoreGlobalTransform = false )
		: CParticleSubEmitter( nSubEmitterCount, fLifeTime, fDirMin, fDirMax, sizeMin, sizeMax, s0Min, s0Max, vMin, vMax, aMin, aMax, bIgnoreGlobalTransform )
		, m_fScaleLon( fScaleLon ), m_fScaleLat( fScaleLat ), m_bLoop( bLoop ) {}
	CParticleWebEmitter( uint32 nSubEmitterCount, float fLifeTime, float fScaleLon, float fScaleLat, bool bLoop, SSubEmitter* pSubEmitters, bool bIgnoreGlobalTransform = false )
		: CParticleSubEmitter( nSubEmitterCount, fLifeTime, pSubEmitters, bIgnoreGlobalTransform ), m_fScaleLon( fScaleLon ), m_fScaleLat( fScaleLat ), m_bLoop( bLoop ) {}

	virtual bool Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform ) override;
private:
	float m_fScaleLon;
	float m_fScaleLat;
	float m_fTime1;
	float m_fTime2;
	float m_fTimeMin, m_fTimeMax;
	bool m_bLoop;
};