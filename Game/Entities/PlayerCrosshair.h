#pragma once
#include "Entity.h"
#include "Render/DefaultDrawable2D.h"

class CPlayerCrosshair : public CEntity, public CDefaultDrawable2D
{
public:
	CPlayerCrosshair();

	virtual void Render( CRenderContext2D& context ) override;

	void SetPercent( const CVector3& percent ) { m_percent = percent; }
protected:
	virtual void OnApplyMaterial( CRenderContext2D& context ) override;
private:
	CElement2D m_element2D;
	CShaderParam m_dirX, m_dirY, m_minDot, m_radBegin, m_radEnd;

	CVector3 m_percent;
};