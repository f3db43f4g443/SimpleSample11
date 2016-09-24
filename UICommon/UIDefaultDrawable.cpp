#include "stdafx.h"
#include "UIDefaultDrawable.h"
#include "Common/xml.h"

void CUIDefaultDrawable::LoadXml( TiXmlElement* pRoot )
{
	CDefaultDrawable2D::LoadXml( pRoot );
	m_texSize = CVector2( 2048.0f / XmlGetAttr( pRoot, "texWidth", 0 ), 2048.0f / XmlGetAttr( pRoot, "texHeight", 0 ) );
}

void CUIDefaultDrawable::Flush( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetBlendState( m_pBlendState );
	m_material.Apply( context );
	OnApplyMaterial( context );

	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	if( m_material.GetMaxInst() )
	{
		uint32 nMaxInst = m_material.GetMaxInst();
		uint32 nExtraInstData = m_material.GetExtraInstData();
		uint32 nInstStride = ( 2 + nExtraInstData ) * sizeof( CVector4 );
		float* fInstData = (float*)alloca( nMaxInst * nInstStride );

		while( m_pElement )
		{
			uint32 i1;
			uint32 nOfs = 0;
			for( i1 = 0;; )
			{
				CElement2D* pElement = m_pElement;

				CRectangle newRect = pElement->rect.Offset( pElement->worldMat.GetPosition() );
				CRectangle clippedRect = newRect * *(CRectangle*)pElement->pInstData;
				if( clippedRect.width <= 0 || clippedRect.height <= 0 )
				{
					pElement->OnFlushed();
					if( !m_pElement )
						break;
					continue;
				}
				CRectangle& texRect = pElement->texRect;
				CRectangle clippedTexRect = CRectangle( texRect.x + texRect.width * ( clippedRect.x - newRect.x ) / newRect.width,
					texRect.y + texRect.height * ( clippedRect.y - newRect.y ) / newRect.height,
					texRect.width * clippedRect.width / newRect.width, texRect.height * clippedRect.height / newRect.height );

				float* pData = fInstData + nOfs;
				*pData++ = clippedRect.GetCenterX();
				*pData++ = clippedRect.GetCenterY();
				*pData++ = clippedRect.GetSizeX() / 2;
				*pData++ = 0;

				uint32 texX = clippedTexRect.x * m_texSize.x;
				float dTexX = 1.0 - clippedTexRect.width * m_texSize.x / 2048;
				uint32 texY = clippedTexRect.y * m_texSize.y;
				float dTexY = 1.0 - clippedTexRect.height * m_texSize.y / 2048;
				*pData++ = texX + dTexX;
				*pData++ = texY + dTexY;

				float ratio = clippedRect.GetSizeY() / clippedRect.GetSizeX();
				*pData++ = ratio;
				*pData++ = pElement->depth;

				uint32 nDataSize = Min( pElement->nInstDataSize, sizeof( CVector4 ) * nExtraInstData );
				if( nDataSize )
					memcpy( pData, pElement->pInstData, nDataSize );

				nOfs += 4 * ( 2 + nExtraInstData );
				i1++;

				bool bBreak = i1 >= nMaxInst || pElement->NextElement() == NULL;
				bBreak = OnFlushElement( context, pElement, bBreak ) || bBreak;
				pElement->OnFlushed();

				if( bBreak )
					break;
			}
		
			uint32 nInstanceDataSize = i1 * nInstStride;
			context.pInstanceDataSize = &nInstanceDataSize;
			context.ppInstanceData = (void**)&fInstData;
			m_material.ApplyPerInstance( context );
			pRenderSystem->DrawInputInstanced( i1 );
		}
	}
	else
	{
		context.pInstanceDataSize = NULL;
		context.ppInstanceData = NULL;
		while( m_pElement )
		{
			context.pCurElement = m_pElement;
			OnFlushElement( context, m_pElement, false );
			m_material.ApplyPerInstance( context );
			pRenderSystem->DrawInput();
			m_pElement->OnFlushed();
		}
		context.pCurElement = NULL;
	}

	m_material.UnApply( context );
}