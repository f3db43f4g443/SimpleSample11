#pragma once
#include "Render/ParticleSystem.h"

class CParticleSubEmitter : public IParticleEmitter
{
	friend void RegisterGameClasses();
public:
	struct SSubEmitter
	{
		CVector2 s0, v, a, size;
	};

	CParticleSubEmitter( uint32 nSubEmitterCount, float fLifeTime, float fDirMin, float fDirMax,
		const CVector2& sizeMin = CVector2( 1, 0 ), const CVector2& sizeMax = CVector2( 1, 0 ),
		const CVector2& s0Min = CVector2( 0, 0 ), const CVector2& s0Max = CVector2( 0, 0 ),
		const CVector2& vMin = CVector2( 0, 0 ), const CVector2& vMax = CVector2( 0, 0 ),
		const CVector2& aMin = CVector2( 0, 0 ), const CVector2& aMax = CVector2( 0, 0 ), bool bIgnoreGlobalTransform = false,
		bool bRotates0 = true, bool bRotatev = true, bool bRotatea = true );
	CParticleSubEmitter( uint32 nSubEmitterCount, float fLifeTime, SSubEmitter* pSubEmitters, bool bIgnoreGlobalTransform = false );
	CParticleSubEmitter( class SClassCreateContext& context ) { Set(); }

	virtual void Init( SParticleInstanceData& data ) override { m_fTime = 0; m_nCurEmitter = 0; }
	virtual void Update( SParticleInstanceData& data, float fTime, const CMatrix2D& transform ) override;
	virtual bool Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform ) override;
protected:
	void Set();

	float m_fLifeTime;
	float m_fTime;
	bool m_bIgnoreGlobalTransform;
	uint32 m_nCurEmitter;
	vector<SSubEmitter> m_subEmitters;

	uint32 m_nSubEmitterCount;
	float m_fDirMin;
	float m_fDirMax;
	CVector2 m_sizeMin;
	CVector2 m_sizeMax;
	CVector2 m_s0Min;
	CVector2 m_s0Max;
	CVector2 m_vMin;
	CVector2 m_vMax;
	CVector2 m_aMin;
	CVector2 m_aMax;
	bool m_bRotates0;
	bool m_bRotatev;
	bool m_bRotatea;
};