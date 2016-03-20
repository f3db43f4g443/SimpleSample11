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

				CMatrix2D mat( pElement->rect.GetSizeX() / 2, 0, pElement->rect.GetCenterX(),
					0, pElement->rect.GetSizeY() / 2, pElement->rect.GetCenterY(),
					0, 0, 1 );
				mat = pElement->worldMat * mat;
				float* pData = fInstData + nOfs;
				*pData++ = mat.m02;
				*pData++ = mat.m12;
				*pData++ = mat.m00;
				*pData++ = mat.m10;

				uint32 texX = pElement->texRect.x * m_texSize.x;
				float dTexX = 1.0 - pElement->texRect.width * m_texSize.x / 2048;
				uint32 texY = pElement->texRect.y * m_texSize.y;
				float dTexY = 1.0 - pElement->texRect.height * m_texSize.y / 2048;
				*pData++ = texX + dTexX;
				*pData++ = texY + dTexY;

				float fMax1 = fabsf( mat.m00 ) > fabsf( mat.m10 )? mat.m00: mat.m10;
				float fMax2 = fabsf( mat.m00 ) > fabsf( mat.m10 )? mat.m11: -mat.m01;
				float ratio = fMax2 / fMax1;
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