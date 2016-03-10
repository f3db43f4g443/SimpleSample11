#include "StdAfx.h"
#include "DefaultDrawable2D.h"
#include "GlobalRenderResources.h"
#include "xml.h"

CDefaultDrawable2D::CDefaultDrawable2D()
	: m_pBlendState( NULL )
{
}

CDefaultDrawable2D::~CDefaultDrawable2D()
{
}

IBlendState* CDrawable2D::LoadBlendState( const char* szBlendType )
{
	IBlendState* pBlendState = NULL;
	if( !strcmp( szBlendType, "opaque" ) )
	{
		pBlendState = IBlendState::Get<>();
		m_bOpaque = true;
	}
	else if( !strcmp( szBlendType, "transparent" ) )
	{
		pBlendState = IBlendState::Get<false, false, 0xf, EBlendSrcAlpha, EBlendInvSrcAlpha, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>();
		m_bOpaque = false;
	}
	else if( !strcmp( szBlendType, "transparent1" ) )
	{
		pBlendState = IBlendState::Get<false, false, 0xf, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>();
		m_bOpaque = false;
	}
	else if( !strcmp( szBlendType, "add" ) )
	{
		pBlendState = IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>();
		m_bOpaque = false;
	}
	else if( !strcmp( szBlendType, "multiply" ) )
	{
		pBlendState = IBlendState::Get<false, false, 0xf, EBlendDestColor, EBlendZero, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>();
		m_bOpaque = false;
	}
	else if( !strcmp( szBlendType, "subtract" ) )
	{
		pBlendState = IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpRevSubtract, EBlendOne, EBlendOne, EBlendOpRevSubtract>();
		m_bOpaque = false;
	}
	else if( !strcmp( szBlendType, "exclude" ) )
	{
		pBlendState = IBlendState::Get<false, false, 0xf, EBlendInvDestColor, EBlendInvSrcColor, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>();
		m_bOpaque = false;
	}
	return pBlendState;
}

void CDefaultDrawable2D::LoadXml( TiXmlElement* pRoot )
{
	auto pBlend = pRoot->FirstChildElement( "blend" );
	const char* szBlendType = XmlGetAttr<const char*>( pBlend, "type", "opaque" );
	m_pBlendState = LoadBlendState( szBlendType );
	
	auto pMaterial = pRoot->FirstChildElement( "material" );
	m_material.LoadXml( pMaterial );
}

void CDefaultDrawable2D::Flush( CRenderContext2D& context )
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

				uint32 texX = pElement->texRect.x * 2048;
				float dTexX = 1.0 - pElement->texRect.width;
				uint32 texY = pElement->texRect.y * 2048;
				float dTexY = 1.0 - pElement->texRect.height;
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
