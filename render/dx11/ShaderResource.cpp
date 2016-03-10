#include "stdafx.h"
#include "DX11ShaderResource.h"
#include "DX11Texture.h"
#include "DX11VertexBuffer.h"
#include "DX11IndexBuffer.h"

void CShaderResource::OnBindSRV( ID3D11DeviceContext* pDeviceContext )
{
	if( m_bNeedGenMip )
	{
		pDeviceContext->GenerateMips( m_pSRV );
		m_bNeedGenMip = false;
	}
}

CTexture* CShaderResource::GetTexture() const
{
	return m_eType == EShaderResourceType::Buffer ? NULL : static_cast<CTexture*>( m_pOrigObject );
}

CVertexBuffer* CShaderResource::GetBuffer() const
{
	return m_eType == EShaderResourceType::Buffer ? static_cast<CVertexBuffer*>( m_pOrigObject ): NULL;
}

CVertexBuffer* CStreamOutput::GetVertexBuffer() const
{
	return m_eType == EStreamOutputType::VertexBuffer ? static_cast<CVertexBuffer*>( m_pOrigObject ) : NULL;
}

CIndexBuffer* CStreamOutput::GetIndexBuffer() const
{
	return m_eType == EStreamOutputType::IndexBuffer ? static_cast<CIndexBuffer*>( m_pOrigObject ) : NULL;
}