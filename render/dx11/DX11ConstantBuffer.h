#pragma once

#include "DX11Common.h"
#include "ConstantBuffer.h"

class CConstantBuffer : public IConstantBuffer
{
public:
	CConstantBuffer( ID3D11Device* pDevice, uint32 nSize, bool bShared = false, bool bUsePool = false );

	virtual void UpdateBuffer( uint32 nOffset, uint32 nSize, const void* data ) override;
	virtual void InvalidBuffer() override;
	void OnUnBound();
	void CommitChanges( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext );
	ID3D11Buffer* GetBuffer() { return m_pBuffer; }
	bool IsShared() { return m_bShared; }
private:
	CReference<ID3D11Buffer> m_pBuffer;
	uint8* m_pData;
	uint32 m_nUpdatedSize;
	uint8 m_nSharedPool;
	bool m_bShared;
	bool m_bUsePool;
};