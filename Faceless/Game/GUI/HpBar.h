#pragma once
#include "Render/RenderObject2D.h"
#include "Render/Drawable2D.h"
#include "Render/Material.h"
#include "Common/Trigger.h"

class CHpBar : public CRenderObject2D
{
public:
	CHpBar( const CRectangle& rect, float fSkew, float fBlur, const CVector4& color, bool bIsBorder = false, bool bIsPanel = true );

	float GetSkew() { return m_fSkew; }
	float GetBlur() { return m_fBlur; }
	const CVector4& GetColor() { return m_color; }
	virtual void Render( CRenderContext2D& context ) override;

	void UpdateHp( float fPercent );
	void Tick();

	void OnHide();
	
	static void Init();
private:
	CElement2D m_element2D;
	CHpBar* m_pTail;
	bool m_bIsBorder;
	float m_fSkew;
	float m_fBlur;
	CVector4 m_color;
	CRectangle m_maxRect;
	TClassTrigger<CHpBar> m_tick;
};