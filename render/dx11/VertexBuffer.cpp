#include "stdafx.h"
#include "DX11VertexBuffer.h"
#include "DX11RenderSystem.h"
#include "DX11ShaderResource.h"

CVertexBuffer::CVertexBuffer( ID3D11Device* pDevice, uint32 nElements, const SVertexBufferElement* pElements, uint32 nVertices, void* pData, bool bIsDynamic,
	bool bBindShaderResource, bool bBindStreamOutput, bool bIsInstance )
	: IVertexBuffer( nElements, pElements, nVertices, pData, bIsDynamic, bBindShaderResource, bBindStreamOutput )
{
	D3D11_USAGE usage;
	uint32 bindFlags = D3D11_BIND_VERTEX_BUFFER;
	uint32 cpuAccessFlags = 0;
	auto& desc = GetDesc();
	if( desc.bIsDynamic )
	{
		usage = D3D11_USAGE_DYNAMIC;
		cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if( !desc.bBindStreamOutput )
	{
		usage = D3D11_USAGE_IMMUTABLE;
	}
	else
		usage = D3D11_USAGE_DEFAULT;
	if( desc.bBindShaderResource )
		bindFlags |= D3D11_BIND_SHADER_RESOURCE;
	if( desc.bBindStreamOutput )
		bindFlags |= D3D11_BIND_STREAM_OUTPUT;
	D3D11_BUFFER_DESC bufferDesc = {
		desc.nVertices * desc.nStride,
		usage,
		bindFlags,
		cpuAccessFlags,
		0
	};
	D3D11_SUBRESOURCE_DATA bufferData = {
		pData, 0, 0
	};
	pDevice->CreateBuffer( &bufferDesc, pData ? &bufferData : NULL, m_pBuffer.AssignPtr() );
	if( desc.bBindShaderResource )
	{
		m_pShaderResource = new CShaderResource( pDevice, m_pBuffer, NULL, EShaderResourceType::Buffer, this );
	}
	if( desc.bBindStreamOutput )
	{
		m_pStreamOutput = new CStreamOutput( m_pBuffer, EStreamOutputType::VertexBuffer, this );
	}
}

IVertexBuffer* CRenderSystem::CreateVertexBuffer( uint32 nElements, const struct SVertexBufferElement* pElements, uint32 nVertices, void* pData, bool bIsDynamic,
	bool bBindShaderResource, bool bBindStreamOutput, bool bIsInstance )
{
	return new CVertexBuffer( m_pd3dDevice, nElements, pElements, nVertices, pData, bIsDynamic, bBindShaderResource, bBindStreamOutput, bIsInstance );
}