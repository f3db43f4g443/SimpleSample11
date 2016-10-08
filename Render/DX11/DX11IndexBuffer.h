#pragma once

#include "DX11Common.h"
#include "IndexBuffer.h"

class CIndexBuffer : public IIndexBuffer
{
public:
	CIndexBuffer( ID3D11Device* pDevice, uint32 nIndices, EFormat eFormat, void* pData, bool bIsDynamic = false, bool bBindStreamOutput = false );

	ID3D11Buffer* GetBuffer() { return m_pBuffer; }
private:
	CReference<ID3D11Buffer> m_pBuffer;
};