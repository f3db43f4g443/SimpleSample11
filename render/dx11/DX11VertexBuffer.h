#pragma once

#include "DX11Common.h"
#include "VertexBuffer.h"

class CVertexBuffer : public IVertexBuffer
{
public:
	CVertexBuffer( ID3D11Device* pDevice, uint32 nElements, const SVertexBufferElement* pElements, uint32 nVertices, void* pData, bool bIsDynamic = false,
		bool bBindShaderResource = false, bool bBindStreamOutput = false, bool bIsInstance = false );

	ID3D11Buffer* GetBuffer() { return m_pBuffer; }
private:
	CReference<ID3D11Buffer> m_pBuffer;
};