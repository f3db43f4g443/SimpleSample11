#include "stdafx.h"
#include "Rope2D.h"
#include "xml.h"

void CRopeDrawable2D::Flush( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetBlendState( m_pBlendState );
	m_material.Apply( context );

	pRenderSystem->SetPrimitiveType( EPrimitiveType::TriangleStrip );
	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBStrip() );

	uint16 nMaxParticles = 0;
	uint32 nParticleMaxInst = 0;
	uint32 nParticleInstDataStride = 0;
	uint8* pParticleInst = NULL;
	if( m_pData )
	{
		nMaxParticles = m_pData->GetMaxParticles();
		nParticleMaxInst = m_material.GetMaxInst();
		nParticleInstDataStride = m_pData->GetInstanceSize();
		pParticleInst = (uint8*)alloca( nMaxParticles * m_param.m_nInstStride );
	}

	uint32 nRopeExtraInstData = m_material.GetExtraInstData();
	uint32 nRopeInstStride = ( 2 + nRopeExtraInstData ) * sizeof( CVector4 );
	float* fRopeInstData = (float*)alloca( m_nRopeMaxInst * nRopeInstStride );

	uint32 nContextInstDataSize[2];
	void* pContextInstData[2] = { fRopeInstData, pParticleInst };
	context.pInstanceDataSize = nContextInstDataSize;
	context.ppInstanceData = pContextInstData;

	if( pParticleInst )
	{
		while( m_pElement )
		{
			SRopeData* pRopeData = (SRopeData*)m_pElement->pInstData;
			uint32 nRopeData = pRopeData->data.size();
			if( nRopeData <= 1 )
			{
				m_pElement->OnFlushed();
				continue;
			}
			uint32 nSegmentsPerData = Min( pRopeData->nSegmentsPerData, CGlobalRenderResources::nStripInstanceCount - 1 );
			uint32 nRopeMaxInst = Min( m_nRopeMaxInst, ( CGlobalRenderResources::nStripInstanceCount - 1 ) / nSegmentsPerData + 1 );
			context.pCurElement = m_pElement;
			OnFlushElement( context, m_pElement );

			uint32 nParticleCount = 0;
			uint32 i1;
			SParticleInstanceData* pParticleInstanceData = pRopeData->pParticleInstData;
			if( !pParticleInstanceData )
			{
				m_pElement->OnFlushed();
				continue;
			}
			uint32 iBegin = pParticleInstanceData->nBegin;
			if( iBegin >= pParticleInstanceData->nEnd )
			{
				m_pElement->OnFlushed();
				continue;
			}

			float fLife = m_pData->GetLifeTime();
			if( m_param.m_paramTime.bIsBound )
				m_param.m_paramTime.Set( pRenderSystem, &pParticleInstanceData->fTime );
			if( m_param.m_paramLife.bIsBound )
				m_param.m_paramLife.Set( pRenderSystem, &fLife );

			for( i1 = 0; iBegin < pParticleInstanceData->nEnd && i1 < nMaxParticles; i1++, iBegin++ )
			{
				if( !m_pElement )
					break;

				uint32 i2 = iBegin < nMaxParticles ? iBegin : iBegin - nMaxParticles;
				uint8* pData = pParticleInst + m_param.m_nInstStride * i1;
				uint8* pInstData = (uint8*)pParticleInstanceData->pData + nParticleInstDataStride * i2;
				for( auto& param : m_param.m_shaderParams )
				{
					memcpy( pData + param.nDstOfs, pInstData + param.nSrcOfs, param.nSize );
				}
			}
			nParticleCount = i1;
			if( m_paramSegmentsPerData.bIsBound )
			{
				float fSegmentsPerData = nSegmentsPerData;
				m_paramSegmentsPerData.Set( pRenderSystem, &fSegmentsPerData );
			}

			int iData = 0;
			while( iData < nRopeData )
			{
				uint32 i;
				uint32 nOfs = 0;
				for( i = 0; i < nRopeMaxInst && iData < nRopeData; i++, iData++ )
				{
					auto& data = pRopeData->data[iData];
					float* pData = fRopeInstData + nOfs;

					CVector2& center = data.worldCenter;
					*pData++ = center.x;
					*pData++ = center.y;

					CVector2 dir;
					if( iData == 0 )
						dir = pRopeData->data[iData + 1].worldCenter - data.worldCenter;
					else if( iData == nRopeData - 1 )
						dir = data.worldCenter - pRopeData->data[iData - 1].worldCenter;
					else
						dir = pRopeData->data[iData + 1].worldCenter - pRopeData->data[iData - 1].worldCenter;
					dir.Normalize();
					dir = CVector2( -dir.y, dir.x ) * data.fWidth * 0.5f;
					*pData++ = dir.x;
					*pData++ = dir.y;

					*pData++ = data.tex0.x;
					*pData++ = data.tex1.x;
					*pData++ = data.tex0.y;
					*pData++ = m_pElement->depth;

					nOfs += 4 * ( 2 + nRopeExtraInstData );
				}

				uint32 nInstanceDataSize = i * nRopeInstStride;
				nContextInstDataSize[0] = nInstanceDataSize;
				uint32 nVertexCount = ( ( i - 1 ) * nSegmentsPerData + 1 ) * 2;

				for( uint32 i1 = 0; i1 < nParticleCount; i1 += nParticleMaxInst )
				{
					uint32 i2 = Min( nParticleMaxInst, nParticleCount - i1 );
					nContextInstDataSize[1] = i2 * m_param.m_nInstStride;
					pContextInstData[1] = pParticleInst + i1 * m_param.m_nInstStride;

					m_material.ApplyPerInstance( context );
					pRenderSystem->DrawInstanced( nVertexCount, i2, 0, 0 );
				}

				if( iData < nRopeData )
					iData--;
			}

			m_pElement->OnFlushed();
		}
	}
	else
	{
		uint32 nLastElemRenderedCount = 0;
		while( m_pElement )
		{
			uint32 i;
			uint32 nOfs = 0;
			uint32 nSegmentsPerData = 0;
			for( i = 0; i < m_nRopeMaxInst && m_pElement; )
			{
				while( m_pElement && i < m_nRopeMaxInst )
				{
					SRopeData* pRopeData = (SRopeData*)m_pElement->pInstData;
					uint32 nRopeData = pRopeData->data.size();
					if( nRopeData <= 1 )
					{
						m_pElement->OnFlushed();
						continue;
					}
					nSegmentsPerData = Max( Min( pRopeData->nSegmentsPerData, CGlobalRenderResources::nStripInstanceCount - 1 ), nSegmentsPerData );
					context.pCurElement = m_pElement;

					if( m_paramSegmentsPerData.bIsBound )
					{
						float fSegmentsPerData = nSegmentsPerData;
						m_paramSegmentsPerData.Set( pRenderSystem, &fSegmentsPerData );
					}

					uint32 iData = nLastElemRenderedCount;
					uint32 iDataEnd = Min( nRopeData, m_nRopeMaxInst - i );

					for( ; iData < iDataEnd; iData++, i++ )
					{
						auto& data = pRopeData->data[iData];
						float* pData = fRopeInstData + nOfs;

						CVector2& center = data.worldCenter;
						*pData++ = center.x;
						*pData++ = center.y;

						CVector2 dir;
						if( iData == 0 )
							dir = pRopeData->data[iData + 1].worldCenter - data.worldCenter;
						else if( iData == nRopeData - 1 )
							dir = data.worldCenter - pRopeData->data[iData - 1].worldCenter;
						else
							dir = pRopeData->data[iData + 1].worldCenter - pRopeData->data[iData - 1].worldCenter;
						dir.Normalize();
						dir = CVector2( -dir.y, dir.x ) * data.fWidth * 0.5f;
						*pData++ = dir.x;
						*pData++ = dir.y;

						*pData++ = data.tex0.x;
						*pData++ = data.tex1.x;
						*pData++ = data.tex0.y;
						*pData++ = iData == nRopeData - 1 ? -1 : m_pElement->depth;

						nOfs += 4 * ( 2 + nRopeExtraInstData );
					}

					if( iData == nRopeData )
					{
						m_pElement->OnFlushed();
						nLastElemRenderedCount = 0;
					}
					else
					{
						nLastElemRenderedCount = iData - 1;
					}

				}
			}

			if( i > 1 )
			{
				uint32 nInstanceDataSize = i * nRopeInstStride;
				nContextInstDataSize[0] = nInstanceDataSize;

				m_material.ApplyPerInstance( context );
				pRenderSystem->DrawInstanced( ( nSegmentsPerData + 1 ) * 2, i - 1, 0, 0 );
			}
		}
	}
	pRenderSystem->SetPrimitiveType( EPrimitiveType::TriangleList );

	m_material.UnApply( context );
}

void CRopeDrawable2D::Load( IBufReader& buf )
{
	CParticleSystemDrawable::Load( buf );
	m_nRopeMaxInst = buf.Read<uint16>();
}

void CRopeDrawable2D::BindParams()
{
	CParticleSystemDrawable::BindParams();
	BindParamsNoParticleSystem();
}

void CRopeDrawable2D::BindParamsNoParticleSystem()
{
	IShader* pVS = m_material.GetShader( EShaderType::VertexShader );
	if( pVS )
		pVS->GetShaderInfo().Bind( m_paramSegmentsPerData, "g_segmentsPerData" );
}

void CRopeDrawable2D::Save( CBufFile& buf )
{
	CParticleSystemDrawable::Save( buf );
	buf.Write<uint16>( m_nRopeMaxInst );
}

void CRopeDrawable2D::LoadNoParticleSystem( IBufReader& buf )
{
	m_pBlendState = LoadBlendState( buf );
	m_material.Load( buf );

	m_nRopeMaxInst = m_material.GetMaxInst();
	BindParamsNoParticleSystem();
}

void CRopeDrawable2D::SaveNoParticleSystem( CBufFile& buf )
{
	SaveBlendState( buf, m_pBlendState );
	m_material.Save( buf );
}

void CRopeDrawable2D::LoadXml( TiXmlElement* pRoot )
{
	CParticleSystemDrawable::LoadXml( pRoot );
	auto pMaterial = pRoot->FirstChildElement( "material" );
	m_nRopeMaxInst = XmlGetAttr( pMaterial, m_pData? "rope_max_insts": "max_insts", 0 );
	IShader* pVS = m_material.GetShader( EShaderType::VertexShader );
	if( pVS )
		pVS->GetShaderInfo().Bind( m_paramSegmentsPerData, "g_segmentsPerData" );
}

void CRopeDrawable2D::BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource )
{
	m_material.BindShaderResource( eShaderType, szName, pShaderResource );
}

CRopeObject2D::CRopeObject2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, CParticleSystemInstance* pData, bool bGUI )
	: CParticleSystemObject( pData, pDrawable, pOcclusionDrawable, CRectangle( 0, 0, 0, 0 ), bGUI )
	, m_boundExt( 0, 0, 0, 0 )
{
	m_element2D.pInstData = &m_data;
	if( m_pInstanceData )
	{
		m_data.pParticleInstData = &m_pInstanceData->GetData();
	}
}

void SRopeData::SetDataCount( uint32 nCount )
{
	data.resize( nCount );
}

void SRopeData::SetData( uint32 nData, const CVector2& center, float fWidth, const CVector2& tex0, const CVector2& tex1 )
{
	if( nData >= data.size() )
		return;

	auto& ropeData = data[nData];
	ropeData.center = center;
	ropeData.fWidth = fWidth;
	ropeData.tex0 = tex0;
	ropeData.tex1 = tex1;
}

void CRopeObject2D::SetData( uint32 nData, const CVector2& center, float fWidth, const CVector2& tex0, const CVector2& tex1, uint16 nTransformIndex )
{
	if( nData >= m_data.data.size() )
		return;

	auto& data = m_data.data[nData];
	data.center = center;
	data.fWidth = fWidth;
	data.tex0 = tex0;
	data.tex1 = tex1;

	if( nTransformIndex != INVALID_16BITID )
	{
		if( !data.pRefObj )
		{
			data.pRefObj = new CRenderObject2D;
			AddChild( data.pRefObj );
		}
		data.pRefObj->SetTransformIndex( nTransformIndex );
	}
	else
	{
		if( data.pRefObj )
		{
			data.pRefObj->RemoveThis();
			data.pRefObj = NULL;
		}
	}
}

bool SRopeData::CalcAABB( CRectangle & rect )
{
	bool bFirst = false;
	for( auto& data : data )
	{
		float fHalfWidth = data.fWidth * 0.5f;
		CRectangle rect1( data.worldCenter.x - fHalfWidth, data.worldCenter.y - fHalfWidth, fHalfWidth * 2, fHalfWidth * 2 );
		if( !bFirst )
		{
			rect = rect1;
			bFirst = true;
		}
		else
			rect = rect + rect1;
	}
	return bFirst;
}

void SRopeData::Update( const CMatrix2D& globalTransform )
{
	for( auto& data : data )
	{
		const CMatrix2D& worldMat = data.pRefObj ? ( data.nRefTransformIndex >= 0 ? data.pRefObj->GetTransform( data.nRefTransformIndex ) : data.pRefObj->globalTransform ) : globalTransform;
		data.worldCenter = worldMat.MulVector2Pos( data.center );
	}
}

bool CRopeObject2D::CalcAABB()
{
	CRectangle orig = globalAABB;

	if( !m_data.CalcAABB( globalAABB ) )
		globalAABB = CRectangle( globalTransform.GetPosition().x, globalTransform.GetPosition().y, 0, 0 );
	else
	{
		globalAABB.x += m_boundExt.x;
		globalAABB.y += m_boundExt.y;
		globalAABB.width += m_boundExt.width;
		globalAABB.height += m_boundExt.height;
	}

	for( CRenderObject2D* pChild = m_pRenderChildren; pChild; pChild = pChild->NextRenderChild() ) {
		globalAABB = globalAABB + pChild->globalAABB;
	}
	return !( orig == globalAABB );
}

void CRopeObject2D::Render( CRenderContext2D& context )
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