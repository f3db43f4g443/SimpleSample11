#pragma once
#include "Render/PostProcess.h"
#include "Common/Math3D.h"

class CPostProcessPixelUpsample : public CPostProcess
{
public:
	void Set( const CVector4& colorCenter, const CVector4& colorEdge, float fEdgeTexPow )
	{
		m_colorCenter = colorCenter;
		m_colorEdge = colorEdge;
		m_fEdgeTexPow = fEdgeTexPow;
	}
	virtual void Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget ) override;
private:
	CVector4 m_colorCenter;
	CVector4 m_colorEdge;
	float m_fEdgeTexPow;
};