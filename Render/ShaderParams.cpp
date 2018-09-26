#include "stdafx.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "RenderSystem.h"
#include "FileUtil.h"

void CShaderParam::InvalidConstantBuffer( IRenderSystem* pRenderSystem )
{
	pRenderSystem->GetConstantBuffer( eShaderType, nConstantBufferIndex )->InvalidBuffer();
}

void CShaderParam::Set( IRenderSystem* pRenderSystem, const void* pData, uint32 nDataLen, uint32 nDataOffset, IConstantBuffer* pBuffer )
{
	if( !pBuffer )
		pBuffer = pRenderSystem->GetConstantBuffer( eShaderType, nConstantBufferIndex );
	nDataOffset = MIN( nDataOffset, nSize );
	nDataLen = MIN( nDataLen, nSize - nDataOffset );
	pBuffer->UpdateBuffer( nDataOffset + nOffset, nDataLen, pData );
}

void CShaderParamConstantBuffer::Set( IRenderSystem* pRenderSystem, IConstantBuffer* pBuffer )
{
	pRenderSystem->SetConstantBuffer( eShaderType, nIndex, pBuffer );
}

void CShaderParamShaderResource::Set( IRenderSystem* pRenderSystem, IShaderResource* pShaderResource )
{
	pRenderSystem->SetShaderResource( eShaderType, nIndex, pShaderResource );
}

void CShaderParamSampler::Set( IRenderSystem* pRenderSystem, const ISamplerState* pState )
{
	pRenderSystem->SetSampler( eShaderType, nIndex, pState );
}

void CShaderParam::Load( IBufReader& buf )
{
	eShaderType = (EShaderType)buf.Read<uint8>();
	buf.Read( strName );
	buf.Read( strConstantBufferName );
	nConstantBufferIndex = buf.Read<uint8>();
	nOffset = buf.Read<uint16>();
	nSize = buf.Read<uint16>();
	bIsBound = true;
}

void CShaderParam::Save( CBufFile& buf )
{
	buf.Write( (uint8)eShaderType );
	buf.Write( strName );
	buf.Write( strConstantBufferName );
	buf.Write( (uint8)nConstantBufferIndex );
	buf.Write( (uint16)nOffset );
	buf.Write( (uint16)nSize );
}

void CShaderParamConstantBuffer::Load( IBufReader& buf )
{
	eShaderType = (EShaderType)buf.Read<uint8>();
	buf.Read( strName );
	nIndex = buf.Read<uint16>();
	nSize = buf.Read<uint16>();
	bIsBound = true;
}

void CShaderParamConstantBuffer::Save( CBufFile& buf )
{
	buf.Write( (uint8)eShaderType );
	buf.Write( strName );
	buf.Write( (uint16)nIndex );
	buf.Write( (uint16)nSize );
}

void CShaderParamShaderResource::Load( IBufReader& buf )
{
	eShaderType = (EShaderType)buf.Read<uint8>();
	buf.Read( strName );
	eType = (EShaderResourceType)buf.Read<uint8>();
	nIndex = buf.Read<uint16>();
	bIsBound = true;
}

void CShaderParamShaderResource::Save( CBufFile& buf )
{
	buf.Write( (uint8)eShaderType );
	buf.Write( strName );
	buf.Write( (uint8)eType );
	buf.Write( (uint16)nIndex );
}

void CShaderParamSampler::Load( IBufReader& buf )
{
	eShaderType = (EShaderType)buf.Read<uint8>();
	buf.Read( strName );
	nIndex = buf.Read<uint16>();
	bIsBound = true;
}

void CShaderParamSampler::Save( CBufFile& buf )
{
	buf.Write( (uint8)eShaderType );
	buf.Write( strName );
	buf.Write( (uint16)nIndex );
}

void CShaderInfo::Bind( CShaderParam& param, const char* szName, const char* szConstantBufferName )
{
	if( !szConstantBufferName || !szConstantBufferName[0] )
		szConstantBufferName = "$Globals";
	auto itr = mapConstantBuffers.find( szConstantBufferName );
	if( itr == mapConstantBuffers.end() )
		return;
	auto itr1 = itr->second.mapVariables.find( szName );
	if( itr1 == itr->second.mapVariables.end() )
		return;

	param.bIsBound = true;
	param.eShaderType = eType;
	param.strName = szName;
	param.strConstantBufferName = szConstantBufferName;
	param.nConstantBufferIndex = itr->second.nIndex;
	param.nOffset = itr1->second.nOffset;
	param.nSize = itr1->second.nSize;
}

void CShaderInfo::BindArray( CShaderParam* pParams, uint32 nParamCount, const char* szName, const char* szConstantBufferName )
{
	if( !nParamCount )
		return;

	Bind( pParams[0], szName, szConstantBufferName );
	if( !pParams[0].bIsBound )
		return;

	pParams[0].nSize = 16;
	for( int i = 1; i < nParamCount; i++ )
	{
		pParams[i] = pParams[0];
		pParams[i].nOffset = pParams[0].nOffset + 16 * i;
	}
}

void CShaderInfo::Bind( CShaderParamConstantBuffer& param, const char* szName )
{
	auto itr = mapConstantBuffers.find( szName );
	if( itr == mapConstantBuffers.end() )
		return;

	param.bIsBound = true;
	param.eShaderType = eType;
	param.strName = szName;
	param.nIndex = itr->second.nIndex;
	param.nSize = itr->second.nSize;
}

void CShaderInfo::Bind( CShaderParamShaderResource& param, const char* szName )
{
	auto itr = mapShaderResources.find( szName );
	if( itr == mapShaderResources.end() )
		return;

	param.bIsBound = true;
	param.eShaderType = eType;
	param.strName = szName;
	param.nIndex = itr->second.nIndex;
	param.eType = itr->second.eType;
}

void CShaderInfo::BindArray( CShaderParamShaderResource* pParams, uint32 nParamCount, const char* szName )
{
	char* szBuf = (char*)alloca( strlen( szName ) + 32 );
	for( int i = 0; i < nParamCount; i++ )
	{
		sprintf( szBuf, "%s[%d]", szName, i );
		Bind( pParams[i], szBuf );
	}
}

void CShaderInfo::Bind( CShaderParamSampler& param, const char* szName )
{
	auto itr = mapSamplerDescs.find( szName );
	if( itr == mapSamplerDescs.end() )
		return;

	param.bIsBound = true;
	param.eShaderType = eType;
	param.strName = szName;
	param.nIndex = itr->second.nIndex;
}

void CShaderInfo::Load( IBufReader& buf )
{
	eType = (EShaderType)buf.Read<uint32>();
	nMaxConstantBuffer = 0;
	nMaxShaderResource = 0;
	nMaxSampler = 0;
	
	uint16 nConstantBuffers = buf.Read<uint16>();
	for( int i = 0; i < nConstantBuffers; i++ )
	{
		string strName;
		buf.Read( strName );
		auto& constantBuffer = mapConstantBuffers[strName];
		constantBuffer.strName = strName;
		buf.Read( constantBuffer.nIndex );
		buf.Read( constantBuffer.nSize );
		nMaxConstantBuffer = Max( nMaxConstantBuffer, constantBuffer.nIndex + 1 );

		uint16 nVariables = buf.Read<uint16>();
		for( int j = 0; j < nVariables; j++ )
		{
			string strName1;
			buf.Read( strName1 );
			auto& variable = constantBuffer.mapVariables[strName1];
			variable.strName = strName1;
			buf.Read( variable.nOffset );
			buf.Read( variable.nSize );
		}
	}

	uint16 nShaderResources = buf.Read<uint16>();
	for( int i = 0; i < nShaderResources; i++ )
	{
		string strName;
		buf.Read( strName );
		auto& shaderResource = mapShaderResources[strName];
		shaderResource.strName = strName;
		shaderResource.eType = (EShaderResourceType)buf.Read<uint16>();
		buf.Read( shaderResource.nIndex );
		nMaxShaderResource = Max( nMaxShaderResource, shaderResource.nIndex + 1 );
	}

	uint16 nSamplers = buf.Read<uint16>();
	for( int i = 0; i < nSamplers; i++ )
	{
		string strName;
		buf.Read( strName );
		auto& sampler = mapSamplerDescs[strName];
		sampler.strName = strName;
		buf.Read( sampler.nIndex );
		nMaxSampler = Max( nMaxSampler, sampler.nIndex + 1 );
	}

	buf.Read( nMaxTarget );
}

void CShaderInfo::Save( CBufFile& buf )
{
	buf.Write( (uint32)eType );

	uint16 nConstantBuffers = mapConstantBuffers.size();
	buf.Write( nConstantBuffers );
	for( auto& item : mapConstantBuffers )
	{
		auto& constantBuffer = item.second;
		buf.Write( constantBuffer.strName );
		buf.Write( constantBuffer.nIndex );
		buf.Write( constantBuffer.nSize );
		uint16 nVariables = constantBuffer.mapVariables.size();
		buf.Write( nVariables );
		for( auto& item1 : constantBuffer.mapVariables )
		{
			auto& variable = item1.second;
			buf.Write( variable.strName );
			buf.Write( variable.nOffset );
			buf.Write( variable.nSize );
		}
	}

	uint16 nShaderResources = mapShaderResources.size();
	buf.Write( nShaderResources );
	for( auto& item : mapShaderResources )
	{
		SShaderResourceDesc& shaderResource = item.second;
		buf.Write( shaderResource.strName );
		buf.Write( (uint16)shaderResource.eType );
		buf.Write( shaderResource.nIndex );
	}

	uint16 nSamplers = mapSamplerDescs.size();
	buf.Write( nSamplers );
	for( auto& item : mapSamplerDescs )
	{
		SSamplerDesc& sampler = item.second;
		buf.Write( sampler.strName );
		buf.Write( sampler.nIndex );
	}

	buf.Write( nMaxTarget );
}