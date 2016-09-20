#pragma once
#include "Entity.h"

class CHUDCircle : public CEntity
{
	friend class CHUDCircleDrawable;
public:
	CHUDCircle( float fRadius, float fWidth, const CVector4& color, const CVector4& color1, bool bPercent = false );

	void SetPercent( float fPercent ) { m_fPercent = fPercent; }
	virtual void Render( CRenderContext2D& context ) override;
private:
	CElement2D m_element2D;
	float m_fRadius;
	float m_fWidth;
	CVector4 m_color;
	CVector4 m_color1;
	float m_fPercent;

	float m_fTimeScale[4];
	float m_fTexRad[4];
};