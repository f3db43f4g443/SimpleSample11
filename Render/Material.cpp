#include "stdafx.h"
#include "Material.h"
#include "RenderContext2D.h"
#include "ResourceManager.h"
#include "GlobalRenderResources.h"
#include "SystemShaderParams.h"
#include "Canvas.h"
#include "xml.h"

void CMaterial::IMaterialShader::_init( const char* szShaderName, IShader* pShader )
{
	GetMaterialShaders()[szShaderName] = this;

	auto& shaderInfo = pShader->GetShaderInfo();
	for( auto& item : shaderInfo.mapConstantBuffers )
	{
		auto& constantBufferDesc = item.second;
		for( auto& item1 : constantBufferDesc.mapVariables )
		{
			auto& variableDesc = item1.second;
			uint32 nSystemParamIndex = CSystemShaderParams::Inst()->GetParamIndex( variableDesc.strName.c_str() );
			if( nSystemParamIndex != -1 )
			{
				CShaderParam param;
				param.bIsBound = true;
				param.eShaderType = shaderInfo.eType;
				param.strName = variableDesc.strName;
				param.strConstantBufferName = constantBufferDesc.strName;
				param.nConstantBufferIndex = constantBufferDesc.nIndex;
				param.nOffset = variableDesc.nOffset;
				param.nSize = variableDesc.nSize;
				if( CSystemShaderParams::Inst()->IsPerInstance( nSystemParamIndex ) )
					vecShaderParamsPerInstance.push_back( pair<CShaderParam, uint32>( param, nSystemParamIndex ) );
				else
					vecShaderParams.push_back( pair<CShaderParam, uint32>( param, nSystemParamIndex ) );
			}
		}
	}
}

ISamplerState* CMaterial::GetMaterialSamplerStates( uint8 nFilter, uint8 nAddress )
{
	static ISamplerState* pSamplerStates[2][3] =
	{
		{
			ISamplerState::Get<ESamplerFilterPPP, ETextureAddressModeClamp, ETextureAddressModeClamp, ETextureAddressModeClamp>(),
			ISamplerState::Get<ESamplerFilterPPP, ETextureAddressModeWrap, ETextureAddressModeWrap, ETextureAddressModeWrap>(),
			ISamplerState::Get<ESamplerFilterPPP, ETextureAddressModeMirror, ETextureAddressModeMirror, ETextureAddressModeMirror>(),
		},
		{
			ISamplerState::Get<ESamplerFilterLLL, ETextureAddressModeClamp, ETextureAddressModeClamp, ETextureAddressModeClamp>(),
			ISamplerState::Get<ESamplerFilterLLL, ETextureAddressModeWrap, ETextureAddressModeWrap, ETextureAddressModeWrap>(),
			ISamplerState::Get<ESamplerFilterLLL, ETextureAddressModeMirror, ETextureAddressModeMirror, ETextureAddressModeMirror>(),
		}
	};
	return pSamplerStates[nFilter][nAddress];
}

void CMaterial::Load( IBufReader& buf )
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	m_nMaxInst = buf.Read<uint16>();
	m_nExtraInstData = buf.Read<uint8>();
	for( int i = 0; i < 3; i++ )
		buf.Read( m_strShaderName[i] );
	const char* szShaders[] = { m_strShaderName[0].c_str(), m_strShaderName[1].c_str(), m_strShaderName[2].c_str() };
	IShader *pShaders[(uint32)EShaderType::Count];
	GetShaders( szShaders, pShaders, m_pShaderBoundState, m_vecShaderParams, m_vecShaderParamsPerInstance );

	uint16 nConstantBuffer = buf.Read<uint16>();
	for( int i = 0; i < nConstantBuffer; i++ )
	{
		CShaderParamConstantBuffer paramConstantBuffer;
		paramConstantBuffer.Load( buf );

		IConstantBuffer* pBuffer = IRenderSystem::Inst()->CreateConstantBuffer( paramConstantBuffer.nSize, true );
		m_vecConstantBuffers.push_back( pair<CShaderParamConstantBuffer, CReference<IConstantBuffer> >( paramConstantBuffer, pBuffer ) );

		buf.Read( pBuffer->GetData(), pBuffer->GetSize() );
	}

	uint16 nShaderResources = buf.Read<uint16>();
	for( int i = 0; i < nShaderResources; i++ )
	{
		CShaderParamShaderResource param;
		param.Load( buf );

		EShaderResourceType eType = param.eType;
		if( eType == EShaderResourceType::Texture2D )
		{
			string strFileName;
			buf.Read( strFileName );
			auto pResource = CResourceManager::Inst()->CreateResource( strFileName.c_str() );
			if( !pResource )
				continue;
			if( pResource->GetResourceType() == CTextureFile::eResType )
			{
				CTextureFile* pTexture = static_cast<CTextureFile*>( pResource );
				m_vecDependentResources.push_back( pTexture );
				m_vecShaderResources.push_back( pair<CShaderParamShaderResource, IShaderResourceProxy* >( param, pTexture->GetTexture() ) );
			}
			else if( pResource->GetResourceType() == CDynamicTexture::eResType )
			{
				CDynamicTexture* pTexture = static_cast<CDynamicTexture*>( pResource );
				m_vecDependentResources.push_back( pTexture );
				m_vecShaderResources.push_back( pair<CShaderParamShaderResource, IShaderResourceProxy* >( param, pTexture ) );
			}
		}
	}
	
	uint16 nSamplers = buf.Read<uint16>();
	for( int i = 0; i < nSamplers; i++ )
	{
		CShaderParamSampler param;
		param.Load( buf );

		uint8 nFilter = buf.Read<uint8>();
		uint8 nAddress = buf.Read<uint8>();
		ISamplerState* pSamplerState = GetMaterialSamplerStates( nFilter, nAddress );

		m_vecSamplers.push_back( pair<CShaderParamSampler, ISamplerState* >( param, pSamplerState ) );
	}
}

void CMaterial::Save( CBufFile& buf )
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	buf.Write( (uint16)m_nMaxInst );
	buf.Write( (uint8)m_nExtraInstData );
	for( int i = 0; i < 3; i++ )
		buf.Write( m_strShaderName[i] );

	buf.Write( (uint16)m_vecConstantBuffers.size() );
	for( auto& item : m_vecConstantBuffers )
	{
		item.first.Save( buf );
		IConstantBuffer* pBuffer = item.second;
		buf.Write( pBuffer->GetData(), pBuffer->GetSize() );
	}
	
	buf.Write( (uint16)m_vecShaderResources.size() );
	uint32 nResource = 0;
	for( auto& item : m_vecShaderResources )
	{
		item.first.Save( buf );
		EShaderResourceType eType = item.first.eType;
		if( eType == EShaderResourceType::Texture2D )
		{
			string strFileName = m_vecDependentResources[nResource++]->GetName();
			buf.Write( strFileName );
		}
	}
	
	buf.Write( (uint16)m_vecSamplers.size() );
	for( auto& item : m_vecSamplers )
	{
		item.first.Save( buf );
		bool bBreak = false;
		for( uint8 nFilter = 0; nFilter < 2; nFilter++ )
		{
			for( uint8 nAddress = 0; nAddress < 3; nAddress++ )
			{
				ISamplerState* pSamplerState = GetMaterialSamplerStates( nFilter, nAddress );
				if( pSamplerState == item.second )
				{
					bBreak = true;
					buf.Write( nFilter );
					buf.Write( nAddress );
					break;
				}
			}
			if( bBreak )
				break;
		}
		if( !bBreak )
			buf.Write( (uint16)0 );
	}
}

void CMaterial::LoadXml( TiXmlElement* pRoot )
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();

	m_nMaxInst = XmlGetAttr<uint32>( pRoot, "max_insts", 0 );
	m_nExtraInstData = XmlGetAttr<uint32>( pRoot, "extra_inst_data", 0 );
	const char* szShaders[] = { XmlGetAttr<const char*>( pRoot, "vertex_shader", "" ),
		XmlGetAttr<const char*>( pRoot, "geometry_shader", "" ),
		XmlGetAttr<const char*>( pRoot, "pixel_shader", "" ),
	};
	for( int i = 0; i < ELEM_COUNT( szShaders ); i++ )
		m_strShaderName[i] = szShaders[i];
	IShader *pShaders[(uint32)EShaderType::Count];
	GetShaders( szShaders, pShaders, m_pShaderBoundState, m_vecShaderParams, m_vecShaderParamsPerInstance );

	auto pConstantBuffers = pRoot->FirstChildElement( "constant_buffers" );
	if( pConstantBuffers )
	{
		for( auto pConstantBuffer = pConstantBuffers->FirstChildElement(); pConstantBuffer; pConstantBuffer = pConstantBuffer->NextSiblingElement() )
		{
			const char* szName = XmlGetAttr<const char*>( pConstantBuffer, "name", "$Globals" );
			uint32 nShaderType = XmlGetAttr<uint32>( pConstantBuffer, "shader_type" );
			if( nShaderType >= (uint32)EShaderType::Count )
				nShaderType = 0;
			if( !pShaders[nShaderType] )
				continue;
			auto& shaderInfo = pShaders[nShaderType]->GetShaderInfo();
			CShaderParamConstantBuffer paramConstantBuffer;
			shaderInfo.Bind( paramConstantBuffer, szName );
			if( !paramConstantBuffer.bIsBound )
				continue;

			IConstantBuffer* pBuffer = IRenderSystem::Inst()->CreateConstantBuffer( paramConstantBuffer.nSize, true );
			m_vecConstantBuffers.push_back( pair<CShaderParamConstantBuffer, CReference<IConstantBuffer> >( paramConstantBuffer, pBuffer ) );
			
			for( auto pParam = pConstantBuffer->FirstChildElement(); pParam; pParam = pParam->NextSiblingElement() )
			{
				const char* szParamName = XmlGetAttr<const char*>( pParam, "name", "" );
				CShaderParam param;
				shaderInfo.Bind( param, szParamName, szName );
				if( !param.bIsBound )
					continue;
				uint32 nArrayIndex = XmlGetAttr( pParam, "array_index", -1 );
				if( nArrayIndex != -1 )
				{
					param.nSize = Min( param.nSize, 16u );
					param.nOffset += nArrayIndex * sizeof( CVector4 );
				}
				
				const char* szType = XmlGetAttr<const char*>( pParam, "type", "float" );
				if( !strcmp( szType, "float" ) )
				{
					float data = XmlGetAttr<float>( pParam, "data" );
					param.Set( pRenderSystem, &data, sizeof( data ), 0, pBuffer );
				}
				else if( !strcmp( szType, "float2" ) )
				{
					const char* szData = XmlGetAttr<const char*>( pParam, "data" );
					CVector2 data;
					sscanf( szData, "%f,%f", &data.x, &data.y );
					param.Set( pRenderSystem, &data, sizeof( data ), 0, pBuffer );
				}
				else if( !strcmp( szType, "float3" ) )
				{
					const char* szData = XmlGetAttr<const char*>( pParam, "data" );
					CVector3 data;
					sscanf( szData, "%f,%f,%f", &data.x, &data.y, &data.z );
					param.Set( pRenderSystem, &data, sizeof( data ), 0, pBuffer );
				}
				else if( !strcmp( szType, "float4" ) )
				{
					const char* szData = XmlGetAttr<const char*>( pParam, "data" );
					CVector4 data;
					sscanf( szData, "%f,%f,%f,%f", &data.x, &data.y, &data.z, &data.w );
					param.Set( pRenderSystem, &data, sizeof( data ), 0, pBuffer );
				}
			}
		}
	}

	auto pShaderResources = pRoot->FirstChildElement( "shader_resources" );
	if( pShaderResources )
	{
		for( auto pShaderResource = pShaderResources->FirstChildElement(); pShaderResource; pShaderResource = pShaderResource->NextSiblingElement() )
		{
			const char* szName = XmlGetAttr<const char*>( pShaderResource, "name", "" );
			uint32 nShaderType = XmlGetAttr<uint32>( pShaderResource, "shader_type", (uint32)EShaderType::PixelShader );
			if( nShaderType >= (uint32)EShaderType::Count )
				nShaderType = 0;
			if( !pShaders[nShaderType] )
				continue;
			auto& shaderInfo = pShaders[nShaderType]->GetShaderInfo();
			CShaderParamShaderResource param;
			shaderInfo.Bind( param, szName );
			if( !param.bIsBound )
				continue;

			const char* szType = XmlGetAttr<const char*>( pShaderResource, "type", "texture" );
			if( !strcmp( szType, "texture" ) )
			{
				const char* szFileName = XmlGetAttr<const char*>( pShaderResource, "filename" );
				CTextureFile* pTexture = CResourceManager::Inst()->CreateResource<CTextureFile>( szFileName );
				if( !pTexture )
					continue;
				m_vecDependentResources.push_back( pTexture );
				m_vecShaderResources.push_back( pair<CShaderParamShaderResource, IShaderResourceProxy* >( param, pTexture->GetTexture() ) );
			}
		}
	}

	auto pSamplers = pRoot->FirstChildElement( "samplers" );
	if( pSamplers )
	{
		for( auto pSampler = pSamplers->FirstChildElement(); pSampler; pSampler = pSampler->NextSiblingElement() )
		{
			const char* szName = XmlGetAttr<const char*>( pSampler, "name", "" );
			uint32 nShaderType = XmlGetAttr<uint32>( pSampler, "shader_type", (uint32)EShaderType::PixelShader );
			if( nShaderType >= (uint32)EShaderType::Count )
				nShaderType = 0;
			if( !pShaders[nShaderType] )
				continue;
			auto& shaderInfo = pShaders[nShaderType]->GetShaderInfo();
			CShaderParamSampler param;
			shaderInfo.Bind( param, szName );
			if( !param.bIsBound )
				continue;

			ISamplerState* pSamplerState = NULL;
			const char* szFilter = XmlGetAttr<const char*>( pSampler, "filter", "linear" );
			const char* szAddress = XmlGetAttr<const char*>( pSampler, "address", "clamp" );
			if( !strcmp( szFilter, "point" ) )
			{
				if( !strcmp( szAddress, "clamp" ) )
					pSamplerState = ISamplerState::Get<ESamplerFilterPPP, ETextureAddressModeClamp, ETextureAddressModeClamp, ETextureAddressModeClamp>();
				else if( !strcmp( szAddress, "wrap" ) )
					pSamplerState = ISamplerState::Get<ESamplerFilterPPP, ETextureAddressModeWrap, ETextureAddressModeWrap, ETextureAddressModeWrap>();
				else if( !strcmp( szAddress, "mirror" ) )
					pSamplerState = ISamplerState::Get<ESamplerFilterPPP, ETextureAddressModeMirror, ETextureAddressModeMirror, ETextureAddressModeMirror>();
			}
			else if( !strcmp( szFilter, "linear" ) )
			{
				if( !strcmp( szAddress, "clamp" ) )
					pSamplerState = ISamplerState::Get<ESamplerFilterLLL, ETextureAddressModeClamp, ETextureAddressModeClamp, ETextureAddressModeClamp>();
				else if( !strcmp( szAddress, "wrap" ) )
					pSamplerState = ISamplerState::Get<ESamplerFilterLLL, ETextureAddressModeWrap, ETextureAddressModeWrap, ETextureAddressModeWrap>();
				else if( !strcmp( szAddress, "mirror" ) )
					pSamplerState = ISamplerState::Get<ESamplerFilterLLL, ETextureAddressModeMirror, ETextureAddressModeMirror, ETextureAddressModeMirror>();
			}
			if( !pSamplerState )
				continue;

			m_vecSamplers.push_back( pair<CShaderParamSampler, ISamplerState* >( param, pSamplerState ) );
		}
	}
}

void CMaterial::Apply( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetShaderBoundState( m_pShaderBoundState, NULL, NULL, NULL, 0 );
	for( auto& item : m_vecConstantBuffers )
	{
		item.first.Set( pRenderSystem, item.second );
	}
	for( auto& item : m_vecShaderResources )
	{
		item.first.Set( pRenderSystem, item.second->GetShaderResource() );
	}
	for( auto& item : m_vecSamplers )
	{
		item.first.Set( pRenderSystem, item.second );
	}
	for( auto& item : m_vecShaderParams )
	{
		context.SetSystemShaderParam( item.first, item.second );
	}
}

void CMaterial::ApplyPerInstance( CRenderContext2D& context )
{
	for( auto& item : m_vecShaderParamsPerInstance )
	{
		context.SetSystemShaderParam( item.first, item.second );
	}
}

void CMaterial::UnApply( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	for( auto& item : m_vecConstantBuffers )
	{
		item.first.Set( pRenderSystem, NULL );
	}
}

void CMaterial::BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource )
{
	for( auto& item : m_vecShaderResources )
	{
		if( item.first.strName == szName )
		{
			item.second = pShaderResource;
			return;
		}
	}

	IShader* pShader = GetShader( EShaderType::PixelShader );
	if( pShader )
	{
		CShaderParamShaderResource param;
		pShader->GetShaderInfo().Bind( param, szName );
		if( !param.bIsBound )
			return;
		m_vecShaderResources.push_back( pair<CShaderParamShaderResource, IShaderResourceProxy* >( param, pShaderResource ) );
	}
}

void CMaterial::GetShaders( const char** szShaders, IShader** pShaders, IShaderBoundState* &pShaderBoundState,
	vector< pair<CShaderParam, uint32> >& vecParams, vector< pair<CShaderParam, uint32> >& vecParamsPerInstance )
{
	string szName = "";
	for( int i = 0; i < (int)EShaderType::Count; i++ )
	{
		if( i )
			szName += "#";
		szName += szShaders[i];
		pShaders[i] = CGlobalShader::GetShaderByName( szShaders[i] );

		if( pShaders[i] )
		{
			auto itr = IMaterialShader::GetMaterialShaders().find( szShaders[i] );
			if( itr != IMaterialShader::GetMaterialShaders().end() )
			{
				IMaterialShader* pShader = itr->second;
				for( auto& item : pShader->vecShaderParams )
				{
					vecParams.push_back( item );
				}
				for( auto& item : pShader->vecShaderParamsPerInstance )
				{
					vecParamsPerInstance.push_back( item );
				}
			}
		}
	}

	static map<string, IShaderBoundState*> g_mapShaderBoundStates;
	IShaderBoundState* &pState = g_mapShaderBoundStates[szName];
	auto pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	if( pShaders[(uint32)EShaderType::VertexShader] )
	{
		IRenderSystem::Inst()->SetShaderBoundState( pState, pShaders[(uint32)EShaderType::VertexShader], pShaders[(uint32)EShaderType::PixelShader],
			&pDesc, 1, pShaders[(uint32)EShaderType::GeometryShader], false );
	}
	pShaderBoundState = pState;
}