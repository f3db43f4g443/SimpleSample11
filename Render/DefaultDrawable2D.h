#pragma once
#include "Drawable2D.h"
#include "Material.h"

class TiXmlElement;
class CDefaultDrawable2D : public CDrawable2D
{
public:
	CDefaultDrawable2D();
	~CDefaultDrawable2D();

	void LoadXml( TiXmlElement* pRoot );
	void Flush( CRenderContext2D& context );
protected:
	virtual void OnApplyMaterial( CRenderContext2D& context ) {}
	virtual bool OnFlushElement( CRenderContext2D& context, CElement2D* pElement, bool bBreak ) { return false; }
	IBlendState* m_pBlendState;
	CMaterial m_material;
};
