#include "stdafx.h"
#include "DX11ConstantBuffer.h"
#include "DX11RenderSystem.h"
#include <vector>
using namespace std;

#define MAX_CONSTANT_BUFFER_POOL_COUNT 13

class CConstantBufferPool
{
public:
	CConstantBufferPool( uint32 nSize ) : m_nSize( nSize ) {}
	void Alloc( ID3D11Device* pDevice, CReference<ID3D11Buffer>& outBuffer )
	{
		if( !m_vecFreeBuffers.size() )
		{
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
			UINT bindFlags = D3D11_BIND_CONSTANT_BUFFER;
			D3D11_BUFFER_DESC bufferDesc = {
				m_nSize, usage, bindFlags, 0, 0
			};
			pDevice->CreateBuffer( &bufferDesc, NULL, outBuffer.AssignPtr() );
		}
		else
		{
			outBuffer = m_vecFreeBuffers.back();
			m_vecFreeBuffers.pop_back();
		}
	}
	void Free( CReference<ID3D11Buffer>& outBuffer )
	{
		if( !outBuffer )
		{
			int a = 0;
		}
		m_vecFreeBuffers.push_back( outBuffer );
		outBuffer = NULL;
	}

	static CConstantBufferPool* GetPool( uint8 nPoolIndex )
	{
		if( nPoolIndex >= MAX_CONSTANT_BUFFER_POOL_COUNT )
			return NULL;
		static CConstantBufferPool* g_pPools[MAX_CONSTANT_BUFFER_POOL_COUNT] = { NULL };
		if( !g_pPools[nPoolIndex] )
		{
			uint32 nSharedSize = MAX_CONSTANT_BUFFER_SIZE;
			uint32 nSharedPool = 0;
			for( ; nSharedPool < MAX_CONSTANT_BUFFER_POOL_COUNT; nSharedSize >>= 1, nSharedPool++ )
			{
				g_pPools[nSharedPool] = new CConstantBufferPool( nSharedSize );
			}
		}
		return g_pPools[nPoolIndex];
	}
private:
	vector<CReference<ID3D11Buffer> > m_vecFreeBuffers;
	uint32 m_nSize;
};

CConstantBuffer::CConstantBuffer( ID3D11Device* pDevice, uint32 nSize, bool bShared, bool bUsePool )
	: IConstantBuffer( bShared? MAX_CONSTANT_BUFFER_SIZE: nSize ), m_bShared( bShared ), m_bUsePool( bUsePool ), m_nUpdatedSize( 0 ), m_nSharedPool( -1 )
{
	if( !bShared )
	{
		if( m_bUsePool )
		{
			uint32 nSharedSize = MAX_CONSTANT_BUFFER_SIZE / 2;
			uint32 nSharedPool = 1;
			while( nSharedSize >= m_nSize && nSharedPool < MAX_CONSTANT_BUFFER_POOL_COUNT )
			{
				nSharedSize >>= 1;
				nSharedPool++;
			}
			nSharedSize <<= 1;
			nSharedPool--;

			m_nSharedPool = nSharedPool;
		}
		else
		{
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
			UINT bindFlags = D3D11_BIND_CONSTANT_BUFFER;
			UINT cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
			D3D11_BUFFER_DESC bufferDesc = {
				nSize, usage, bindFlags, cpuAccessFlags, 0
			};
			pDevice->CreateBuffer( &bufferDesc, NULL, m_pBuffer.AssignPtr() );
		}
	}

	m_pData = (uint8*)malloc( nSize );
}

void CConstantBuffer::UpdateBuffer( uint32 nOffset, uint32 nSize, const void* data )
{
	if( nOffset >= m_nSize )
		return;
	nSize = MIN( nSize, m_nSize - nOffset );
	memcpy( m_pData + nOffset, data, nSize );
	m_nUpdatedSize = MAX( m_nUpdatedSize, nSize + nOffset );
}

void CConstantBuffer::InvalidBuffer()
{
	if( m_bShared )
		m_nUpdatedSize = 0;
}

void CConstantBuffer::OnUnBound()
{
	if( m_bUsePool )
	{
		if( m_pBuffer )
			CConstantBufferPool::GetPool( m_nSharedPool )->Free( m_pBuffer );
		m_nUpdatedSize = m_nSize;
	}
	else if( m_bShared )
	{
		if( m_pBuffer )
			CConstantBufferPool::GetPool( m_nSharedPool )->Free( m_pBuffer );
		m_nSharedPool = -1;
		m_nUpdatedSize = 0;
	}
}

void CConstantBuffer::CommitChanges( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext )
{
	if( !m_nUpdatedSize )
		return;

	if( m_bShared )
	{
		uint32 nSharedSize = MAX_CONSTANT_BUFFER_SIZE / 2;
		uint32 nSharedPool = 1;
		while( nSharedSize >= m_nUpdatedSize && nSharedPool < MAX_CONSTANT_BUFFER_POOL_COUNT )
		{
			nSharedSize >>= 1;
			nSharedPool++;
		}
		nSharedSize <<= 1;
		nSharedPool--;

		if( nSharedPool != m_nSharedPool )
		{
			if( m_nSharedPool < MAX_CONSTANT_BUFFER_POOL_COUNT )
				CConstantBufferPool::GetPool( m_nSharedPool )->Free( m_pBuffer );
			m_nSharedPool = nSharedPool;
			CConstantBufferPool::GetPool( nSharedPool )->Alloc( pDevice, m_pBuffer );
		}
		pDeviceContext->UpdateSubresource( m_pBuffer, 0, NULL, m_pData, nSharedSize, nSharedSize );
	}
	else
	{
		if( m_bUsePool )
		{
			if( !m_pBuffer )
				CConstantBufferPool::GetPool( m_nSharedPool )->Alloc( pDevice, m_pBuffer );
			uint32 nSharedSize = MAX_CONSTANT_BUFFER_SIZE >> m_nSharedPool;
			pDeviceContext->UpdateSubresource( m_pBuffer, 0, NULL, m_pData, nSharedSize, nSharedSize );
		}
		pDeviceContext->UpdateSubresource( m_pBuffer, 0, NULL, m_pData, m_nSize, m_nSize );
		m_nUpdatedSize = 0;
	}
}

IConstantBuffer* CRenderSystem::CreateConstantBuffer( uint32 nSize, bool bUsePool )
{
	return new CConstantBuffer( m_pd3dDevice, nSize, false, bUsePool );
}