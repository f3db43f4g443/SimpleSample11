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

namespace _DefaultDrawable2D_cpp
{
	enum
	{
		eDrawableBlend_Opaque,
		eDrawableBlend_Transparent,
		eDrawableBlend_Transparent1,
		eDrawableBlend_Add,
		eDrawableBlend_Multiply,
		eDrawableBlend_Subtract,
		eDrawableBlend_Exclude,
		eDrawableBlend_Min,
		eDrawableBlend_Blend1,

		eDrawableBlend_Count,
	};

	IBlendState** GetBlendStates()
	{
		static IBlendState* pBlendStates[] = 
		{
			IBlendState::Get<>(),
			IBlendState::Get<false, false, 0xf, EBlendSrcAlpha, EBlendInvSrcAlpha, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>(),
			IBlendState::Get<false, false, 0xf, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>(),
			IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>(),
			IBlendState::Get<false, false, 0xf, EBlendDestColor, EBlendZero, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>(),
			IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpRevSubtract, EBlendOne, EBlendOne, EBlendOpRevSubtract>(),
			IBlendState::Get<false, false, 0xf, EBlendInvDestColor, EBlendInvSrcColor, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>(),
			IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpMin, EBlendOne, EBlendOne, EBlendOpMin>(),
			IBlendState::Get<false, false, 0xf, EBlendSrcAlpha, EBlendSrcColor, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>(),
		};
		return pBlendStates;
	}
}
using namespace _DefaultDrawable2D_cpp;

IBlendState* CDrawable2D::GetBlendState( uint16 nType )
{
	m_bOpaque = nType == 0;
	if( nType >= eDrawableBlend_Count )
		return NULL;
	return GetBlendStates()[nType];
}

IBlendState* CDrawable2D::LoadBlendState( IBufReader& buf )
{
	uint16 nType = buf.Read<uint16>();
	return GetBlendState( nType );
}

uint16 CDrawable2D::GetBlendStateIndex( IBlendState* pState )
{
	if( !pState )
		return INVALID_16BITID;
	uint16 nType = 0;
	for( int i = 0; i < eDrawableBlend_Count; i++ )
	{
		if( pState == GetBlendStates()[i] )
		{
			nType = i;
			break;
		}
	}
	return nType;
}

void CDrawable2D::SaveBlendState( CBufFile& buf, IBlendState* pBlendState )
{
	buf.Write( GetBlendStateIndex( pBlendState ) );
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

void CDefaultDrawable2D::Load( IBufReader& buf )
{
	m_pBlendState = LoadBlendState( buf );
	m_material.Load( buf );
	if( !m_pBlendState )
		m_material1.PixelCopy( m_material );
}

void CDefaultDrawable2D::Save( CBufFile& buf )
{
	SaveBlendState( buf, m_pBlendState );
	m_material.Save( buf );
}

void CDefaultDrawable2D::LoadXml( TiXmlElement* pRoot )
{
	auto pBlend = pRoot->FirstChildElement( "blend" );
	const char* szBlendType = XmlGetAttr<const char*>( pBlend, "type", "opaque" );
	m_pBlendState = LoadBlendState( szBlendType );
	
	auto pMaterial = pRoot->FirstChildElement( "material" );
	m_material.LoadXml( pMaterial );
}

void CDefaultDrawable2D::BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource )
{
	m_material.BindShaderResource( eShaderType, szName, pShaderResource );
}

void CDefaultDrawable2D::Flush( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	if( m_pBlendState )
	{
		pRenderSystem->SetBlendState( m_pBlendState );
		m_material.Apply( context );
		OnApplyMaterial( context );

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

					float fMax1 = fabsf( mat.m00 ) > fabsf( mat.m10 ) ? mat.m00 : mat.m10;
					float fMax2 = fabsf( mat.m00 ) > fabsf( mat.m10 ) ? mat.m11 : -mat.m01;
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
	else
	{
		pRenderSystem->SetBlendState( IBlendState::Get<>() );
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

					float fMax1 = fabsf( mat.m00 ) > fabsf( mat.m10 ) ? mat.m00 : mat.m10;
					float fMax2 = fabsf( mat.m00 ) > fabsf( mat.m10 ) ? mat.m11 : -mat.m01;
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

				context.RTImage( 0 );
				m_material.Apply( context );
				OnApplyMaterial( context );

				m_material.ApplyPerInstance( context );
				pRenderSystem->DrawInputInstanced( i1 );

				m_material.UnApply( context );

				context.RTImage( 1 );
				m_material1.Apply( context );

				m_material1.ApplyPerInstance( context );
				pRenderSystem->DrawInputInstanced( i1 );

				m_material1.UnApply( context );
				context.RTImage( 2 );
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

				context.RTImage( 0 );
				m_material.Apply( context );
				OnApplyMaterial( context );

				m_material.ApplyPerInstance( context );
				pRenderSystem->DrawInput();

				m_material.UnApply( context );

				context.RTImage( 1 );
				m_material1.Apply( context );

				m_material1.ApplyPerInstance( context );
				pRenderSystem->DrawInput();

				m_material1.UnApply( context );
				context.RTImage( 2 );

				m_pElement->OnFlushed();
			}
			context.pCurElement = NULL;
		}
	}
}
