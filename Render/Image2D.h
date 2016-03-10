#pragma once
#include "RenderObject2D.h"

class CImage2D : public CRenderObject2D
{
public:
	CImage2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, const CRectangle& texRect, bool bGUI = false );

	void SetColorDrawable( CDrawable2D* pDrawable ) { m_pColorDrawable = pDrawable; }
	void SetOcclusionDrawable( CDrawable2D* pDrawable ) { m_pOcclusionDrawable = pDrawable; }
	void SetGUIDrawable( CDrawable2D* pDrawable ) { m_pGUIDrawable = pDrawable; }
	void SetRect( CRectangle& rect ) { m_element2D.rect = rect; }
	void SetTexRect( CRectangle& texRect ) { m_element2D.texRect = texRect; }
	void SetInstData( void* pInstData, uint32 nInstDataSize ) { m_element2D.pInstData = pInstData; m_element2D.nInstDataSize = nInstDataSize; }

	virtual void Render( CRenderContext2D& context ) override;
protected:
	CElement2D m_element2D;
	CDrawable2D* m_pColorDrawable;
	CDrawable2D* m_pOcclusionDrawable;
	CDrawable2D* m_pGUIDrawable;
};
