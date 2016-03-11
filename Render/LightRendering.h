#pragma once
#include "RenderObject2D.h"

class CDirectionalLightObject : public CRenderObject2D
{
public:
	CDirectionalLightObject();
	CDirectionalLightObject( const CVector2& Dir, const CVector3& baseColor, float fShadowScale, float fMaxShadowDist );

	virtual void Render( CRenderContext2D& context ) override;

	CVector2 Dir;
	float fShadowScale;
	float fMaxShadowDist;
	CVector3 baseColor;
private:
	SDirectionalLight2D m_light;
};

class CPointLightObject : public CRenderObject2D
{
public:
	CPointLightObject();
	CPointLightObject( const CVector4& AttenuationIntensity, const CVector3& baseColor, float fShadowScale, float fMaxRange, float fLightHeight );

	void RecalcBound() { m_localBound = CRectangle( -fMaxRange, -fMaxRange, fMaxRange, fMaxRange ); }
	virtual void Render( CRenderContext2D& context ) override;

	CVector4 AttenuationIntensity;
	float fShadowScale;
	float fMaxRange;
	float fLightHeight;
	CVector3 baseColor;
private:
	SPointLight2D m_light;
};