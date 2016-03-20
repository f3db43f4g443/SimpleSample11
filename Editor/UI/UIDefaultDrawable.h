#pragma once
#include "Render/DefaultDrawable2D.h"

class CUIDefaultDrawable : public CDefaultDrawable2D
{
public:
	void LoadXml( TiXmlElement* pRoot );
	virtual void Flush( CRenderContext2D& context ) override;
private:
	CVector2 m_texSize;
};