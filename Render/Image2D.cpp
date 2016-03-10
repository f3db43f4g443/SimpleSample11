#include "StdAfx.h"
#include "Image2D.h"

CImage2D::CImage2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, const CRectangle& texRect, bool bGUI )
	: m_pColorDrawable( bGUI ? NULL : pDrawable ), m_pOcclusionDrawable( bGUI ? NULL : pOcclusionDrawable ), m_pGUIDrawable( bGUI ? pDrawable : NULL )
{
	m_element2D.rect = rect;
	m_element2D.texRect = texRect;
	m_element2D.pInstData = this;
	m_localBound = rect;
}

void CImage2D::Render( CRenderContext2D& context )
{
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( m_pColorDrawable )
		{
			m_element2D.SetDrawable( m_pColorDrawable );
			m_element2D.worldMat = globalTransform;
			context.AddElement( &m_element2D );
		}
		else if( m_pGUIDrawable )
		{
			m_element2D.SetDrawable( m_pGUIDrawable );
			m_element2D.worldMat = globalTransform;
			context.AddElement( &m_element2D, 1 );
		}
		break;
	case eRenderPass_Occlusion:
		if( m_pOcclusionDrawable )
		{
			m_element2D.SetDrawable( m_pOcclusionDrawable );
			m_element2D.worldMat = globalTransform;
			context.AddElement( &m_element2D );
		}
		break;
	default:
		break;
	}
}