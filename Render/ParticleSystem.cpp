#include "stdafx.h"
#include "ParticleSystem.h"
#include "Rand.h"
#include "xml.h"
#include "Rope2D.h"
#include "FileUtil.h"
#include "ClassMetaData.h"

void SParticleSystemDataElement::SetFloat1( float minValue, float maxValue, uint8 nRandomType )
{
	nComponents = 1;
	*(float*)&dataMin = minValue;
	*(float*)&dataMax = maxValue;
	this->nRandomType = nRandomType;
}

void SParticleSystemDataElement::SetFloat2( const CVector2& minValue, const CVector2& maxValue, uint8 nRandomType )
{
	nComponents = 2;
	*(CVector2*)&dataMin = minValue;
	*(CVector2*)&dataMax = maxValue;
	this->nRandomType = nRandomType;
}

void SParticleSystemDataElement::SetFloat3( const CVector3& minValue, const CVector3& maxValue, uint8 nRandomType )
{
	nComponents = 3;
	*(CVector3*)&dataMin = minValue;
	*(CVector3*)&dataMax = maxValue;
	this->nRandomType = nRandomType;
}

void SParticleSystemDataElement::SetFloat4( const CVector4& minValue, const CVector4& maxValue, uint8 nRandomType )
{
	nComponents = 4;
	*(CVector4*)&dataMin = minValue;
	*(CVector4*)&dataMax = maxValue;
	this->nRandomType = nRandomType;
}

void SParticleSystemDataElement::SetCircle( float fMinRadius, float fMaxRadius, float fMinAngle, float fMaxAngle, uint8 nRandomType )
{
	nComponents = 2;
	*(CVector2*)&dataMax = CVector2( fMaxRadius, fMaxAngle );
	dataMin[1] = fMinAngle;
	dataMin[0] = fMinRadius / fMaxRadius;
	dataMin[0] *= dataMin[0];
	this->nRandomType = nRandomType;
}

void SParticleSystemDataElement::GenerateValue( void* pData, const CMatrix2D& transform )
{
	float* fData = (float*)pData;
	uint8 nRandomTypeBasic = nRandomType & eRandomType_Basic;
	switch( nRandomTypeBasic )
	{
	case eRandomType_PerComponent:
		for( int i = 0; i < nComponents; i++ )
		{
			fData[i] = SRand::Inst().Rand( dataMin[i], dataMax[i] );
		}
		break;
	case eRandomType_Lerp:
		{
			float fRand = SRand::Inst().Rand( 0.0f, 1.0f );
			for( int i = 0; i < nComponents; i++ )
			{
				fData[i] = dataMin[i] + ( dataMax[i] - dataMin[i] ) * fRand;
			}
		}
		break;
	case eRandomType_Slerp:
		{
			float fRand = SRand::Inst().Rand( 0.0f, 1.0f );
			float l1 = 0, l2 = 0, dot = 0;
			for( int i = 0; i < nComponents; i++ )
			{
				l1 += dataMin[i] * dataMin[i];
				l2 += dataMax[i] * dataMax[i];
				dot += dataMin[i] * dataMax[i];
			}
			float fAngle = dot / sqrt( l1 * l2 );
			float sn = sin( fAngle );
			float invSn = 1.0 / sn;
			float tAngle = fRand * fAngle;
			float coeff0 = sin( fAngle - tAngle ) * invSn;
			float coeff1 = sin( tAngle ) * invSn;

			for( int i = 0; i < nComponents; i++ )
			{
				fData[i] = dataMin[i] * coeff0 + dataMax[i] * coeff1;
			}
		}
		break;
	case eRandomType_Circle:
		{
			float r = sqrt( SRand::Inst().Rand( dataMin[0], 1.0f ) ) * dataMax[0];
			CVector2& vec2 = *(CVector2*)pData;
			float angle = SRand::Inst().Rand( dataMin[1], dataMax[1] );
			vec2.x = cos( angle ) * r;
			vec2.y = sin( angle ) * r;
		}
		break;
	default:
		break;
	}

	if( nRandomType & eRandomType_TransformToGlobalPos )
	{
		if( nComponents > 1 )
		{
			CVector2& vec2 = *(CVector2*)pData;
			vec2 = transform.MulVector2Pos( vec2 );
		}
	}
	else if( nRandomType & eRandomType_TransformToGlobalDir )
	{
		if( nComponents > 1 )
		{
			CVector2& vec2 = *(CVector2*)pData;
			vec2 = transform.MulVector2Dir( vec2 );
		}
		else
		{
			float& fData = *(float*)pData;
			fData = transform.MulVector2Dir( CVector2( fData, 0 ) ).Length();
		}
	}
}

CParticleSystemData::CParticleSystemData( EParticleSystemType eType, uint16 nMaxParticles, float lifeTime, float emitRate, uint8 emitType, bool bBatchAcrossInstances, uint8 nElements, SParticleSystemDataElement* pElements )
	: m_eType( eType ), m_nMaxParticles( nMaxParticles ), m_lifeTime( lifeTime ), m_emitRate( emitRate ), m_emitType( emitType ), m_bBatchAcrossInstances( bBatchAcrossInstances ), m_nElements( nElements ), m_pElements( pElements )
{
	Update();
}

void CParticleSystemData::InitInstanceData( SParticleInstanceData& data )
{
	data.pData = malloc( m_nMaxParticles * m_instanceSize );
	data.isEmitting = true;
	data.nBegin = data.nEnd = 0;
	data.fTime = 0;
	data.fEmitTime = 0;
}

void CParticleSystemData::Update()
{
	m_instanceSize = 4;
	for(int i = 0; i < m_nElements; i++) {
		m_pElements[i].nOffset = m_instanceSize;
		m_instanceSize += m_pElements[i].nComponents * 4;
	}
}

bool CParticleSystemData::AnimateInstanceData( SParticleInstanceData& data, const CMatrix2D& transform, float fDeltaTime, IParticleEmitter* pEmitter )
{
	data.fTime += fDeltaTime;
	if( !m_bBatchAcrossInstances )
	{
		if( m_lifeTime > 0 )
		{
			float fTimePeriod = m_lifeTime * 8;
			if( data.fTime >= fTimePeriod )
				data.fTime -= fTimePeriod;
			for( ; data.nBegin < data.nEnd; data.nBegin++ )
			{
				uint32 nIndex = data.nBegin >= m_nMaxParticles ? data.nBegin - m_nMaxParticles : data.nBegin;
				float dataTime = *(float*)( ( (uint8*)data.pData ) + nIndex * m_instanceSize );
				float dTime = data.fTime - dataTime;
				if( dTime < 0 )
					dTime += fTimePeriod;
				if( dTime < m_lifeTime )
					break;
			}
			if(data.nBegin >= m_nMaxParticles) {
				data.nBegin -= m_nMaxParticles;
				data.nEnd -= m_nMaxParticles;
			}
		}
	}
	else
	{
		for( int i = data.nBegin; i < data.nEnd; i++ )
		{
			uint32 nIndex = i >= m_nMaxParticles ? i - m_nMaxParticles : i;
			float& dataTime = *(float*)( ( (uint8*)data.pData ) + nIndex * m_instanceSize );
			dataTime += fDeltaTime;
		}
		if( m_lifeTime > 0 )
		{
			for( ; data.nBegin < data.nEnd; data.nBegin++ )
			{
				uint32 nIndex = data.nBegin >= m_nMaxParticles ? data.nBegin - m_nMaxParticles : data.nBegin;
				float dataTime = *(float*)( ( (uint8*)data.pData ) + nIndex * m_instanceSize );
				if( dataTime < m_lifeTime )
					break;
			}
			if(data.nBegin >= m_nMaxParticles) {
				data.nBegin -= m_nMaxParticles;
				data.nEnd -= m_nMaxParticles;
			}
		}
	}

	int nEmit = 0;
	if( data.isEmitting )
	{
		switch(m_emitType) {
		case 0:
			data.fEmitTime += fDeltaTime;
			nEmit = floor( data.fEmitTime * m_emitRate );
			data.fEmitTime -= nEmit / m_emitRate;
			break;
		case 1:
			data.isEmitting = false;
			nEmit = m_emitRate;
			break;
		}
	}
	if( nEmit > m_nMaxParticles - data.nEnd + data.nBegin )
		nEmit = m_nMaxParticles - data.nEnd + data.nBegin;

	int iEmit;
	for( iEmit = 0; iEmit < nEmit; )
	{
		int i1 = data.nEnd + iEmit;
		if(i1 >= m_nMaxParticles)
			i1 -= m_nMaxParticles;

		uint8* pData = (uint8*)data.pData + i1 * m_instanceSize;
		float fExtraTime = ( nEmit - iEmit - 1 ) / m_emitRate;
		*(float*)pData = ( m_bBatchAcrossInstances ? 0 : data.fTime ) + fExtraTime;
		if( pEmitter )
		{
			if( pEmitter->Emit( data, this, pData, transform ) )
				iEmit++;
			else
				nEmit--;
		}
		else
		{
			GenerateSingleParticle( data, pData, transform );
			iEmit++;
		}
	}
	data.nEnd += nEmit;
	return data.isEmitting;
}

void CParticleSystemDrawable::Flush( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetBlendState( m_pBlendState );
	m_material.Apply( context );

	uint32 nMaxInst = m_material.GetMaxInst();
	uint32 nInstDataStride = m_pData->GetInstanceSize();
	uint8* pInst = (uint8*)alloca( nMaxInst * m_param.m_nInstStride );

	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	float fLife = m_pData->GetLifeTime();
	if( m_param.m_paramLife.bIsBound )
		m_param.m_paramLife.Set( pRenderSystem, &fLife );
	if( !m_pData->IsBatchAcrossInstances() )
	{
		while( m_pElement )
		{
			context.pCurElement = m_pElement;
			OnFlushElement( context, m_pElement );
			uint32 i1;
			SParticleInstanceData* pParticleInstanceData = (SParticleInstanceData*)m_pElement->pInstData;
			uint32 iBegin = pParticleInstanceData->nBegin;
			if( m_param.m_paramTime.bIsBound )
				m_param.m_paramTime.Set( pRenderSystem, &pParticleInstanceData->fTime );

			while( iBegin < pParticleInstanceData->nEnd )
			{
				for( i1 = 0; i1 < nMaxInst && iBegin < pParticleInstanceData->nEnd; i1++, iBegin++ )
				{
					if( !m_pElement )
						break;

					uint32 i2 = iBegin < m_pData->GetMaxParticles() ? iBegin : iBegin - m_pData->GetMaxParticles();
					uint8* pData = pInst + m_param.m_nInstStride * i1;
					uint8* pInstData = (uint8*)pParticleInstanceData->pData + nInstDataStride * i2;
					for( auto& param : m_param.m_shaderParams )
					{
						memcpy( pData + param.nDstOfs, pInstData + param.nSrcOfs, param.nSize );
					}
				}

				uint32 nInstanceDataSize = i1 * m_param.m_nInstStride;
				context.pInstanceDataSize = &nInstanceDataSize;
				context.ppInstanceData = (void**)&pInst;
				m_material.ApplyPerInstance( context );
				pRenderSystem->DrawInputInstanced( i1 );
			}

			m_pElement->OnFlushed();
		}
	}
	else
	{
		uint32 nLastElemRenderedCount = 0;
		while( m_pElement )
		{
			uint32 i1;
			for( i1 = 0; i1 < nMaxInst && m_pElement; )
			{
				while( m_pElement && i1 < nMaxInst )
				{
					context.pCurElement = m_pElement;
					SParticleInstanceData* pParticleInstanceData = (SParticleInstanceData*)m_pElement->pInstData;
					uint32 iBegin = pParticleInstanceData->nBegin + nLastElemRenderedCount;
					uint32 iEnd = pParticleInstanceData->nEnd;
					if( iEnd - iBegin + i1 > nMaxInst )
					{
						iEnd = nMaxInst + iBegin - i1;
						nLastElemRenderedCount = iEnd - pParticleInstanceData->nBegin;
					}
					else
						nLastElemRenderedCount = 0;
					float fDepth = m_pElement->depth;

					for( ; iBegin < iEnd; iBegin++, i1++ )
					{
						uint32 i2 = iBegin < m_pData->GetMaxParticles() ? iBegin : iBegin - m_pData->GetMaxParticles();
						uint8* pData = pInst + m_param.m_nInstStride * i1;
						uint8* pInstData = (uint8*)pParticleInstanceData->pData + nInstDataStride * i2;
						for( auto& param : m_param.m_shaderParams )
						{
							memcpy( pData + param.nDstOfs, pInstData + param.nSrcOfs, param.nSize );
						}
						if( m_param.m_nZOrderOfs != INVALID_32BITID )
							memcpy( pData + m_param.m_nZOrderOfs, &fDepth, sizeof( fDepth ) );
					}

					if( !nLastElemRenderedCount )
						m_pElement->OnFlushed();
				}

				if( i1 > 0 )
				{
					uint32 nInstanceDataSize = i1 * m_param.m_nInstStride;
					context.pInstanceDataSize = &nInstanceDataSize;
					context.ppInstanceData = (void**)&pInst;
					m_material.ApplyPerInstance( context );
					pRenderSystem->DrawInputInstanced( i1 );
				}
			}
		}
	}
	m_material.UnApply( context );
}

CParticleSystemObject::CParticleSystemObject( CParticleSystemInstance* pData, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, bool bGUI )
	: m_pColorDrawable( bGUI ? NULL : pDrawable )
	, m_pOcclusionDrawable( bGUI ? NULL : pOcclusionDrawable )
	, m_pGUIDrawable( bGUI ? pDrawable : NULL )
	, m_pInstanceData( pData )
{
	if( pData )
	{
		pData->Insert_ParticleSystemObject( this );
		m_element2D.pInstData = &pData->GetData();
	}
	m_localBound = rect;
}

CParticleSystemObject::~CParticleSystemObject()
{
	if( m_pInstanceData )
		RemoveFrom_ParticleSystemObject();
}

void CParticleSystemObject::OnStopped()
{
	if( m_pInstanceData )
	{
		RemoveFrom_ParticleSystemObject();
		m_pInstanceData = NULL;
	}
	RemoveThis();
}

void CParticleSystemObject::Render( CRenderContext2D& context )
{
	if( !m_pInstanceData )
		return;
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

CParticleSystemInstance::CParticleSystemInstance( CParticleSystemData* pData )
	: m_bPaused( false )
	, m_bAutoRestart( false )
	, m_pParticleSystemData( pData )
	, m_pParticleSystemObjects( NULL )
{
	pData->InitInstanceData( m_data );
}

void CParticleSystemInstance::OnStopped()
{
	while( m_pParticleSystemObjects )
	{
		m_pParticleSystemObjects->OnStopped();
	}
}

void SParticleSystemDataElement::Load( IBufReader& buf )
{
	buf.Read( szName );
	buf.Read( nComponents );
	buf.Read( nRandomType );
	buf.Read( dataMin, sizeof(float)* nComponents );
	buf.Read( dataMax, sizeof(float)* nComponents );
}

void SParticleSystemDataElement::Save( CBufFile& buf )
{
	buf.Write( szName );
	buf.Write( nComponents );
	buf.Write( nRandomType );
	buf.Write( dataMin, sizeof(float)* nComponents );
	buf.Write( dataMax, sizeof(float)* nComponents );
}

CParticleSystemData* CParticleSystemData::Load( IBufReader& buf )
{
	uint16 nMaxParticles = buf.Read<uint16>();
	uint16 eType = buf.Read<uint16>();
	float lifeTime = buf.Read<float>();
	float emitRate = buf.Read<float>();
	uint8 emitType = buf.Read<uint8>();
	bool bBatchAcrossInstances = buf.Read<uint8>();

	uint8 nElements = buf.Read<uint8>();
	SParticleSystemDataElement* elements = new SParticleSystemDataElement[nElements];
	for( int i = 0; i < nElements; i++ )
	{
		auto& elem = elements[i];
		elem.Load( buf );
	}

	return new CParticleSystemData( (EParticleSystemType)eType, nMaxParticles, lifeTime, emitRate, emitType, bBatchAcrossInstances, nElements, elements );
}

void CParticleSystemData::Save( CBufFile& buf )
{
	buf.Write<uint16>( m_nMaxParticles );
	buf.Write<uint16>( m_eType );
	buf.Write( m_lifeTime );
	buf.Write( m_emitRate );
	buf.Write( m_emitType );
	buf.Write( (uint8)m_bBatchAcrossInstances );
	buf.Write( m_nElements );
	for( int i = 0; i < m_nElements; i++ )
	{
		m_pElements[i].Save( buf );
	}
}

CParticleSystemData* CParticleSystemData::LoadXml( TiXmlElement* pRoot )
{
	uint32 nMaxParticles = XmlGetAttr( pRoot, "max_particles", 1 );
	EParticleSystemType eType;
	const char* szType = XmlGetAttr( pRoot, "type", "particle" );
	if( !strcmp( szType, "beam" ) )
		eType = eParticleSystemType_Beam;
	else
		eType = eParticleSystemType_Particle;
	float lifeTime = XmlGetAttr( pRoot, "lifetime", 1.0f );
	float emitRate = XmlGetAttr( pRoot, "emitrate", 1.0f );
	uint8 emitType = XmlGetAttr( pRoot, "emittype", 0 );
	bool bBatchAcrossInstances = eType == eParticleSystemType_Particle? XmlGetAttr( pRoot, "batchacrossinstances", 0 ): false;

	TiXmlElement* pElement;
	uint32 nElements = 0;
	for( pElement = pRoot->FirstChildElement( "element" ); pElement; pElement = pElement->NextSiblingElement( "element" ) )
		nElements++;
	SParticleSystemDataElement* elements = new SParticleSystemDataElement[nElements];
	nElements = 0;

	for( pElement = pRoot->FirstChildElement( "element" ); pElement; pElement = pElement->NextSiblingElement( "element" ) )
	{
		SParticleSystemDataElement& element = elements[nElements++];
		element.szName = XmlGetAttr( pElement, "name", "" );
		const char* szType = XmlGetAttr( pElement, "type", "float1" );
		uint8 nRandomType = 0;
		const char* szRandomType = XmlGetAttr( pElement, "random_type", "per_component" );
		
		bool bCircle = false;
		if( !strcmp( szRandomType, "per_component" ) )
		{
			nRandomType = SParticleSystemDataElement::eRandomType_PerComponent;
		}
		else if( !strcmp( szRandomType, "lerp" ) )
		{
			nRandomType = SParticleSystemDataElement::eRandomType_Lerp;
		}
		else if( !strcmp( szRandomType, "slerp" ) )
		{
			nRandomType = SParticleSystemDataElement::eRandomType_Slerp;
		}
		else if( !strcmp( szRandomType, "circle" ) )
		{
			nRandomType = SParticleSystemDataElement::eRandomType_Circle;
			bCircle = true;
			szType = "float2";
		}

		uint8 bTransformGlobalPos = XmlGetAttr( pElement, "global_pos", 0 );
		if( bTransformGlobalPos )
			nRandomType |= SParticleSystemDataElement::eRandomType_TransformToGlobalPos;
		else
		{
			uint8 bTransformGlobalDir = XmlGetAttr( pElement, "global_dir", 0 );
			if( bTransformGlobalDir )
				nRandomType |= SParticleSystemDataElement::eRandomType_TransformToGlobalDir;
		}

		if( !strcmp( szType, "float1" ) )
		{
			element.SetFloat1( XmlGetAttr( pElement, "min", 0.0f ), XmlGetAttr( pElement, "max", 0.0f ), nRandomType );
		}
		else if( !strcmp( szType, "float2" ) )
		{
			CVector2 vMin( XmlGetAttr( pElement, "min1", 0.0f ), XmlGetAttr( pElement, "min2", 0.0f ) );
			CVector2 vMax( XmlGetAttr( pElement, "max1", 0.0f ), XmlGetAttr( pElement, "max2", 0.0f ) );
			if( !bCircle )
				element.SetFloat2( vMin, vMax, nRandomType );
			else
				element.SetCircle( vMin.x, vMax.x, vMin.y, vMax.y, nRandomType );
		}
		else if( !strcmp( szType, "float3" ) )
		{
			CVector3 vMin( XmlGetAttr( pElement, "min1", 0.0f ), XmlGetAttr( pElement, "min2", 0.0f ), XmlGetAttr( pElement, "min3", 0.0f ) );
			CVector3 vMax( XmlGetAttr( pElement, "max1", 0.0f ), XmlGetAttr( pElement, "max2", 0.0f ), XmlGetAttr( pElement, "max3", 0.0f ) );
			element.SetFloat3( vMin, vMax, nRandomType );
		}
		else if( !strcmp( szType, "float4" ) )
		{
			CVector4 vMin( XmlGetAttr( pElement, "min1", 0.0f ), XmlGetAttr( pElement, "min2", 0.0f ), XmlGetAttr( pElement, "min3", 0.0f ), XmlGetAttr( pElement, "min4", 0.0f ) );
			CVector4 vMax( XmlGetAttr( pElement, "max1", 0.0f ), XmlGetAttr( pElement, "max2", 0.0f ), XmlGetAttr( pElement, "max3", 0.0f ), XmlGetAttr( pElement, "max4", 0.0f ) );
			element.SetFloat4( vMin, vMax, nRandomType );
		}
	}

	return new CParticleSystemData( eType, nMaxParticles, lifeTime, emitRate, emitType, bBatchAcrossInstances, nElements, elements );
}

void SParticleSystemShaderParam::Load( IBufReader& buf )
{
	uint16 nParams = buf.Read<uint16>();
	m_shaderParams.resize( nParams );
	for( int i = 0; i < nParams; i++ )
	{
		buf.Read( m_shaderParams[i] );
	}

	buf.Read( m_nZOrderOfs );
	buf.Read( m_nInstStride );
}

void SParticleSystemShaderParam::Save( CBufFile& buf )
{
	buf.Write<uint16>( m_shaderParams.size() );
	for( int i = 0; i < m_shaderParams.size(); i++ )
	{
		buf.Write( m_shaderParams[i] );
	}
	buf.Write( m_nZOrderOfs );
	buf.Write( m_nInstStride );
}

void SParticleSystemShaderParam::LoadXml( TiXmlElement* pRoot, IShader* pVS, CParticleSystemData* pData )
{
	auto pMaterial = pRoot->FirstChildElement( "material" );
	m_nZOrderOfs = INVALID_32BITID;
	m_nInstStride = XmlGetAttr( pMaterial, "inst_stride", 0 );
	if( pVS )
	{
		auto& info = pVS->GetShaderInfo();
		info.Bind( m_paramTime, "g_t" );
		info.Bind( m_paramLife, "g_life" );
	}

	auto pParams = pRoot->FirstChildElement( "params" );
	if( !pParams )
		return;
	map<string, pair<uint16, uint16> > mapParams;
	for( auto pElemParam = pParams->FirstChildElement(); pElemParam; pElemParam = pElemParam->NextSiblingElement() )
	{
		const char* szName = XmlGetAttr( pElemParam, "name", "" );
		uint16 nOfs = XmlGetAttr( pElemParam, "ofs", 0 );
		uint16 nSize = XmlGetAttr( pElemParam, "size", INVALID_16BITID );

		if( !strcmp( szName, "t" ) )
		{
			SParam param;
			param.nSrcOfs = 0;
			param.nDstOfs = nOfs;
			param.nSize = Min<uint16>( nSize, 4 );
			m_shaderParams.push_back( param );
		}
		else if( !strcmp( szName, "z" ) )
		{
			m_nZOrderOfs = nOfs;
		}
		else
			mapParams[szName] = pair<uint16, uint16>( nOfs, nSize );
	}

	uint8 nElements = pData->GetElementCount();
	auto pElements = pData->GetElements();
	for( int i = 0; i < nElements; i++ )
	{
		auto& element = pElements[i];
		auto itr = mapParams.find( element.szName );
		if( itr == mapParams.end() )
			continue;
		SParam param;
		param.nSrcOfs = element.nOffset;
		param.nDstOfs = itr->second.first;
		param.nSize = Min<uint16>( itr->second.second, element.nComponents * 4 );
		m_shaderParams.push_back( param );
	}
}

void CParticleSystemDrawable::Load( IBufReader& buf )
{
	m_pBlendState = LoadBlendState( buf );
	m_material.Load( buf );
	m_param.Load( buf );
	BindParams();
}

void CParticleSystemDrawable::Save( CBufFile& buf )
{
	SaveBlendState( buf, m_pBlendState );
	m_material.Save( buf );
	m_param.Save( buf );
}

void CParticleSystemDrawable::BindParams()
{
	IShader* pVS = m_material.GetShader( EShaderType::VertexShader );
	if( pVS )
	{
		auto& info = pVS->GetShaderInfo();
		info.Bind( m_param.m_paramTime, "g_t" );
		info.Bind( m_param.m_paramLife, "g_life" );
	}
}

void CParticleSystemDrawable::LoadXml( TiXmlElement* pRoot )
{
	auto pBlend = pRoot->FirstChildElement( "blend" );
	const char* szBlendType = XmlGetAttr<const char*>( pBlend, "type", "opaque" );
	m_pBlendState = LoadBlendState( szBlendType );
	
	auto pMaterial = pRoot->FirstChildElement( "material" );
	m_material.LoadXml( pMaterial );
	if( m_pData )
	{
		IShader* pVS = m_material.GetShader( EShaderType::VertexShader );
		m_param.LoadXml( pRoot, pVS, m_pData );
	}
}

CParticleSystemObject* CParticleSystem::CreateParticleSystemObject( CAnimationController* pAnimController, CParticleSystemInstance** pInst, function<ParticleRenderObjectAllocFunc> *pAlloc )
{
	if( m_pParticleSystemData->GetType() != eParticleSystemType_Particle )
		return NULL;
	CParticleSystemInstance* pInstance = new CParticleSystemInstance( m_pParticleSystemData );
	if( pInst )
	{
		pInstance->AddRef();
		*pInst = pInstance;
	}
	uint32 nColorPassObject = Max( m_vecColorPassDrawables.size(), m_vecOcclusionPassDrawables.size() );
	uint32 nGUIPassObject = m_vecGUIPassDrawables.size();
	CParticleSystemObject* pRenderObject = NULL;
	if( !nColorPassObject && !nGUIPassObject )
		return NULL;
	if( nColorPassObject + nGUIPassObject > 1 )
		pRenderObject = pAlloc ? ( *pAlloc )( pInstance, NULL, NULL, CRectangle( 0, 0, 0, 0 ), false ) : new CParticleSystemObject( pInstance, NULL, NULL, CRectangle( 0, 0, 0, 0 ) );

	for( int i = 0; i < nColorPassObject; i++ )
	{
		CParticleSystemDrawable* pColorPass = i < m_vecColorPassDrawables.size() ? m_vecColorPassDrawables[i] : NULL;
		CParticleSystemDrawable* pOcclusionPass = i < m_vecOcclusionPassDrawables.size() ? m_vecOcclusionPassDrawables[i] : NULL;
		CParticleSystemObject* pObject = pAlloc ? ( *pAlloc )( pInstance, pColorPass, pOcclusionPass, m_rect, false ) : new CParticleSystemObject( pInstance, pColorPass, pOcclusionPass, m_rect );
		if( pRenderObject )
			pRenderObject->AddChild( pObject );
		else
			pRenderObject = pObject;
	}

	for( int i = 0; i < nGUIPassObject; i++ )
	{
		CParticleSystemObject* pObject = pAlloc ? ( *pAlloc )( pInstance, m_vecGUIPassDrawables[i], NULL, m_rect, true ) : new CParticleSystemObject( pInstance, m_vecGUIPassDrawables[i], NULL, m_rect, true );
		if( pRenderObject )
			pRenderObject->AddChild( pObject );
		else
			pRenderObject = pObject;
	}

	if( !pAnimController )
		pAnimController = pRenderObject->GetAnimController();
	pAnimController->PlayAnim( pInstance );
	return pRenderObject;
}

CRopeObject2D* CParticleSystem::CreateBeamObject( CAnimationController* pAnimController, CParticleSystemInstance** pInst, function<RopeRenderObjectAllocFunc> *pAlloc )
{
	if( m_pParticleSystemData->GetType() != eParticleSystemType_Beam )
		return NULL;
	CParticleSystemInstance* pInstance = new CParticleSystemInstance( m_pParticleSystemData );
	if( pInst )
	{
		pInstance->AddRef();
		*pInst = pInstance;
	}
	uint32 nColorPassObject = Max( m_vecColorPassDrawables.size(), m_vecOcclusionPassDrawables.size() );
	uint32 nGUIPassObject = m_vecGUIPassDrawables.size();
	CRopeObject2D* pRenderObject = NULL;

	if( nColorPassObject )
	{
		CParticleSystemDrawable* pColorPass = m_vecColorPassDrawables.size() ? m_vecColorPassDrawables[0] : NULL;
		CParticleSystemDrawable* pOcclusionPass = m_vecOcclusionPassDrawables.size() ? m_vecOcclusionPassDrawables[0] : NULL;
		pRenderObject = pAlloc ? ( *pAlloc )( pInstance, pColorPass, pOcclusionPass, false ) : new CRopeObject2D( pColorPass, pOcclusionPass, pInstance );
	}
	else
	{
		pRenderObject = pAlloc ? ( *pAlloc )( pInstance, m_vecGUIPassDrawables[0], NULL, true ) : new CRopeObject2D( m_vecGUIPassDrawables[0], NULL, pInstance, true );
	}

	if( !pAnimController )
		pAnimController = pRenderObject->GetAnimController();
	pAnimController->PlayAnim( pInstance );
	pRenderObject->SetBoundExt( m_rect );
	return pRenderObject;
}

CParticleSystemInstance* CParticleSystem::CreateParticleSystemInst( CAnimationController* pAnimController, CElement2D* pElem )
{
	if( !m_vecColorPassDrawables.size() )
		return NULL;
	if( m_pParticleSystemData->GetType() != eParticleSystemType_Particle )
		return NULL;
	CParticleSystemInstance* pInstance = new CParticleSystemInstance( m_pParticleSystemData );
	pAnimController->PlayAnim( pInstance );
	if( pElem )
	{
		CParticleSystemDrawable* pColorPass = m_vecColorPassDrawables[0];
		pElem->SetDrawable( pColorPass );
		pElem->pInstData = &pInstance->GetData();
		pElem->rect = m_rect;
	}
	return pInstance;
}

void CParticleSystem::BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource )
{
	for( auto& item : m_vecColorPassDrawables )
		item->GetMaterial().BindShaderResource( eShaderType, szName, pShaderResource );
	for( auto& item : m_vecOcclusionPassDrawables )
		item->GetMaterial().BindShaderResource( eShaderType, szName, pShaderResource );
	for( auto& item : m_vecGUIPassDrawables )
		item->GetMaterial().BindShaderResource( eShaderType, szName, pShaderResource );
}

void CParticleSystem::Load( IBufReader& buf )
{
	buf.Read( m_rect );

	m_pParticleSystemData = CParticleSystemData::Load( buf );

	uint16 nColorPass = buf.Read<uint16>();
	for( int i = 0; i < nColorPass; i++ )
	{
		m_vecColorPassDrawables.push_back( LoadDrawable( buf ) );
	}

	uint16 nOcclusionPass = buf.Read<uint16>();
	for( int i = 0; i < nOcclusionPass; i++ )
	{
		m_vecOcclusionPassDrawables.push_back( LoadDrawable( buf ) );
	}

	uint16 nGUIPass = buf.Read<uint16>();
	for( int i = 0; i < nGUIPass; i++ )
	{
		m_vecGUIPassDrawables.push_back( LoadDrawable( buf ) );
	}
}

CParticleSystemDrawable* CParticleSystem::CreateDrawable()
{
	switch( m_pParticleSystemData->GetType() )
	{
	case eParticleSystemType_Particle:
		{
			CParticleSystemDrawable* pDrawable = m_pDrawableFunc ? ( *m_pDrawableFunc )( m_pParticleSystemData ) : new CParticleSystemDrawable( m_pParticleSystemData );
			return pDrawable;
		}
		break;
	case eParticleSystemType_Beam:
		{
			CRopeDrawable2D* pDrawable = m_pDrawableFunc ? static_cast<CRopeDrawable2D*>( ( *m_pDrawableFunc )( m_pParticleSystemData ) ) : new CRopeDrawable2D( m_pParticleSystemData );
			return pDrawable;
		}
		break;
	}
	return NULL;
}

CParticleSystemDrawable* CParticleSystem::LoadDrawable( IBufReader& buf )
{
	CParticleSystemDrawable* pDrawable = CreateDrawable();
	if( pDrawable )
		pDrawable->Load( buf );
	return pDrawable;
}

void CParticleSystem::Save( CBufFile& buf )
{
	buf.Write( m_rect );

	m_pParticleSystemData->Save( buf );

	buf.Write<uint16>( m_vecColorPassDrawables.size() );
	for( auto& item : m_vecColorPassDrawables )
	{
		item->Save( buf );
	}
	
	buf.Write<uint16>( m_vecOcclusionPassDrawables.size() );
	for( auto& item : m_vecOcclusionPassDrawables )
	{
		item->Save( buf );
	}
	
	buf.Write<uint16>( m_vecGUIPassDrawables.size() );
	for( auto& item : m_vecGUIPassDrawables )
	{
		item->Save( buf );
	}
}

void CParticleSystem::LoadXml( TiXmlElement* pRoot )
{
	m_rect.x = XmlGetAttr( pRoot, "x", 0.0f );
	m_rect.y = XmlGetAttr( pRoot, "y", 0.0f );
	m_rect.width = XmlGetAttr( pRoot, "width", 0.0f );
	m_rect.height = XmlGetAttr( pRoot, "height", 0.0f );

	m_pParticleSystemData = CParticleSystemData::LoadXml( pRoot->FirstChildElement( "data" ) );

	TiXmlElement* pColorPass = pRoot->FirstChildElement( "color_passes" );
	if( pColorPass )
	{
		for( TiXmlElement* pElem = pColorPass->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement() )
		{
			m_vecColorPassDrawables.push_back( LoadDrawable( pElem ) );
		}
	}

	TiXmlElement* pOcclusionPass = pRoot->FirstChildElement( "occlusion_passes" );
	if( pOcclusionPass )
	{
		for( TiXmlElement* pElem = pOcclusionPass->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement() )
		{
			m_vecOcclusionPassDrawables.push_back( LoadDrawable( pElem ) );
		}
	}

	TiXmlElement* pGUIPass = pRoot->FirstChildElement( "gui_passes" );
	if( pGUIPass )
	{
		for( TiXmlElement* pElem = pGUIPass->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement() )
		{
			m_vecGUIPassDrawables.push_back( LoadDrawable( pElem ) );
		}
	}
}

CParticleSystemDrawable* CParticleSystem::LoadDrawable( TiXmlElement* pElem )
{
	CParticleSystemDrawable* pDrawable = CreateDrawable();
	if( pDrawable )
		pDrawable->LoadXml( pElem );
	return pDrawable;
}

void CParticleFile::Create()
{
	if( strcmp( GetFileExtension( GetName() ), "pts" ) )
		return;
	vector<char> content;
	if( GetFileContent( content, GetName(), false ) == INVALID_32BITID )
		return;
	CBufReader buf( &content[0], content.size() );
	m_particleSystem.Load( buf );
	m_bCreated = true;
}

CParticleSystemObject* CParticleFile::CreateInstance( CAnimationController* pAnimController )
{
	CParticleSystemObject* pObj;
	if( m_particleSystem.GetData()->GetType() == eParticleSystemType_Particle )
	{
		pObj = m_particleSystem.CreateParticleSystemObject( pAnimController );
	}
	else
	{
		pObj = m_particleSystem.CreateBeamObject( pAnimController );
		CRopeObject2D* pRopeObject = static_cast<CRopeObject2D*>( pObj );
		pRopeObject->SetDataCount( 2 );
		pRopeObject->SetData( 0, CVector2( 0, 0 ), 8, CVector2( 0, 0 ), CVector2( 1, 0 ) );
		pRopeObject->SetData( 1, CVector2( 64, 0 ), 8, CVector2( 0, 1 ), CVector2( 1, 1 ) );
	}
	if( !pAnimController && pObj )
		pObj->SetAutoUpdateAnim( true );
	return pObj;
}

void CParticleFile::UpdateDependencies()
{
	ClearDependency();
	vector<CParticleSystemDrawable*>* drawables[] = { &m_particleSystem.m_vecColorPassDrawables, &m_particleSystem.m_vecOcclusionPassDrawables, &m_particleSystem.m_vecGUIPassDrawables };
	for( int i = 0; i < ELEM_COUNT( drawables ); i++ )
	{
		auto& vecDrawables = *drawables[i];
		for( auto pDrawable : vecDrawables )
		{
			auto& material = pDrawable->GetMaterial();
			for( auto& res : material.GetDependentResources() )
				AddDependency( res );
		}
	}
}

void CParticleSystemObject::CopyData( CParticleSystemObject* pObj )
{
	auto pInstanceData = GetInstanceData();
	auto pEmitter = pInstanceData->GetEmitter();
	if( pEmitter )
	{
		auto pClassData = CClassMetaDataMgr::Inst().GetClassData( pEmitter );
		CBufFile buf;
		pClassData->PackData( (uint8*)pEmitter, buf, false );
		auto pEmitter1 = (IParticleEmitter*)pClassData->NewObjFromData( buf, false );
		pObj->GetInstanceData()->SetEmitter( pEmitter1 );
	}
}

void CParticleSystemObject::LoadExtraData( IBufReader& buf )
{
	if( !buf.GetBytesLeft() )
		return;
	auto pInstanceData = GetInstanceData();
	string className;
	buf.Read( className );
	auto pClassData = CClassMetaDataMgr::Inst().GetClassData( className.c_str() );
	auto pEmitter = (IParticleEmitter*)pClassData->NewObjFromData( buf, true );
	pInstanceData->SetEmitter( pEmitter );
}

void CParticleSystemObject::SaveExtraData( CBufFile& buf )
{
	auto pInstanceData = GetInstanceData();
	auto pEmitter = pInstanceData->GetEmitter();
	auto pClassData = pEmitter ? CClassMetaDataMgr::Inst().GetClassData( pEmitter ) : NULL;
	string className = pClassData ? pClassData->strClassName : "";
	buf.Write( className );
	pClassData->PackData( (uint8*)pEmitter, buf, true );
}