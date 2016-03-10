#pragma once
#include "RenderObject2D.h"

class CDirectionalLightObject : public CRenderObject2D
{
public:
	CDirectionalLightObject();
	CDirectionalLightObject( const CVector2& Dir, float fIntensity, float fShadowScale, float fMaxShadowDist );

	virtual void Render( CRenderContext2D& context ) override;

	CVector2 Dir;
	float fIntensity;
	float fShadowScale;
	float fMaxShadowDist;
private:
	SDirectionalLight2D m_light;
};

class CPointLightObject : public CRenderObject2D
{
public:
	CPointLightObject();
	CPointLightObject( const CVector4& AttenuationIntensity, float fShadowScale, float fMaxRange, float fLightHeight );

	void RecalcBound() { m_localBound = CRectangle( -fMaxRange, -fMaxRange, fMaxRange, fMaxRange ); }
	virtual void Render( CRenderContext2D& context ) override;

	CVector4 AttenuationIntensity;
	float fShadowScale;
	float fMaxRange;
	float fLightHeight;
private:
	SPointLight2D m_light;
};