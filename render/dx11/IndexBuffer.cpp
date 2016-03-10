#include "stdafx.h"
#include "DX11IndexBuffer.h"
#include "DX11RenderSystem.h"
#include "DX11ShaderResource.h"

CIndexBuffer::CIndexBuffer( ID3D11Device* pDevice, uint32 nIndices, EFormat eFormat, void* pData, bool bIsDynamic, bool bBindStreamOutput )
	: IIndexBuffer( nIndices, eFormat, pData, bIsDynamic, bBindStreamOutput )
{
	D3D11_USAGE usage;
	uint32 bindFlags = D3D11_BIND_INDEX_BUFFER;
	uint32 cpuAccessFlags = 0;
	if( m_desc.bIsDynamic)
	{
		usage = D3D11_USAGE_DYNAMIC;
		cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if( !m_desc.bBindStreamOutput )
		usage = D3D11_USAGE_IMMUTABLE;
	else
		usage = D3D11_USAGE_DEFAULT;

	D3D11_BUFFER_DESC bufferDesc = {
		m_desc.nIndices * m_desc.nOffset,
		usage,
		bindFlags,
		cpuAccessFlags,
		0
	};
	D3D11_SUBRESOURCE_DATA bufferData = {
		pData, 0, 0
	};
	pDevice->CreateBuffer( &bufferDesc, pData? &bufferData: NULL, m_pBuffer.AssignPtr() );

	if( m_desc.bBindStreamOutput )
	{
		m_pStreamOutput = new CStreamOutput( m_pBuffer, EStreamOutputType::IndexBuffer, this );
	}
}

IIndexBuffer* CRenderSystem::CreateIndexBuffer( uint32 nIndices, EFormat eFormat, void* pData, bool bIsDynamic, bool bBindStreamOutput )
{
	return new CIndexBuffer( m_pd3dDevice, nIndices, eFormat, pData, bIsDynamic, bBindStreamOutput );
}