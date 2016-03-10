#pragma once
#include "Render/PostProcess.h"
#include "Common/Math3D.h"

class CPostProcessInvertColor : public CPostProcess
{
public:
	virtual void Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget ) override;

	void SetPercent( float fPercent ) { m_fPercent = fPercent; }
private:
	float m_fPercent;
};

class CPostProcessDizzyEffect : public CPostProcess
{
public:
	virtual void Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget ) override;

	void SetInvertPercent( float fPercent ) { m_fInvertPercent = fPercent; }
	void SetTexOfs( CVector4* pTexOfs ) { memcpy( m_texofs, pTexOfs, sizeof( m_texofs ) ); }
	void SetWeights( CVector4* pWeights ) { memcpy( m_weights, pWeights, sizeof( m_weights ) ); }
private:
	float m_fInvertPercent;
	CVector4 m_texofs[5];
	CVector4 m_weights[5];
};

class CPostProcessCrackEffect : public CPostProcess
{
public:
	CPostProcessCrackEffect() : m_fElapsedTime( NULL ), m_pMesh( NULL ) {}
	~CPostProcessCrackEffect();
	virtual void Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget ) override;

	bool HasEffect();
	void Update( float fElapsedTime ) { m_fElapsedTime += fElapsedTime; }
	void Reset();
private:
	float m_fElapsedTime;

	class CCrackMeshGenerator* m_pMesh;
};