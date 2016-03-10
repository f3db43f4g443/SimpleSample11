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