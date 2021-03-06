#include "stdafx.h"
#include "DX11Texture.h"
#include "DX11RenderSystem.h"
#include "DX11ShaderResource.h"

CTexture::CTexture( ID3D11Device* pDevice, ETextureType eType, int nDim1, int nDim2, int nDim3, int nMipLevels, EFormat eFormat, void* data,
	bool bIsDynamic, bool bBindRenderTarget, bool bBindDepthStencil )
	: ITexture( eType, nDim1, nDim2, nDim3, nMipLevels, eFormat, data, bIsDynamic, bBindRenderTarget, bBindDepthStencil )
{
	D3D11_USAGE usage;
	uint32 bindFlags = !bBindDepthStencil? D3D11_BIND_SHADER_RESOURCE: 0;
	bool bGenerateMips = false;
	if( !nMipLevels )
	{
		if( !bBindDepthStencil )
		{
			bGenerateMips = true;
			bBindRenderTarget = true;

			uint32 nMaxDim = Max( nDim1, Max( nDim2, nDim3 ) );
			while( nMaxDim )
			{
				nMipLevels++;
				nMaxDim >>= 1;
			}
		}
		else
			nMipLevels = 1;
	}
	uint32 cpuAccessFlags = 0;
	if( bIsDynamic )
	{
		usage = D3D11_USAGE_DYNAMIC;
		cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if( !bBindRenderTarget && !bBindDepthStencil && data )
		usage = D3D11_USAGE_IMMUTABLE;
	else
		usage = D3D11_USAGE_DEFAULT;
	if( bBindRenderTarget )
		bindFlags |= D3D11_BIND_RENDER_TARGET;
	if( bBindDepthStencil )
		bindFlags |= D3D11_BIND_DEPTH_STENCIL;
	uint32 pitch = FormatToLength( eFormat ) * nDim1;
	uint32 slicePitch = pitch * nDim2;
	DXGI_FORMAT format = GetDXGIFormat( eFormat );

	D3D11_SUBRESOURCE_DATA texData[16];
	switch( eType )
	{
	case ETextureType::Tex2D:
		{
			D3D11_TEXTURE2D_DESC desc = {
				nDim1,
				nDim2,
				nMipLevels,
				1,
				format,
				{ 1, 0 },
				usage,
				bindFlags,
				cpuAccessFlags,
				bGenerateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0
			};
			if( data )
			{
				for( int i = 0; i < nMipLevels; i++ )
				{
					texData[i].pSysMem = data;
					texData[i].SysMemPitch = pitch;
					texData[i].SysMemSlicePitch = 0;
				}
			}
			pDevice->CreateTexture2D( &desc, data? texData: NULL, (ID3D11Texture2D**)m_pTexture.AssignPtr() );
		}
		break;
	case ETextureType::TexCube:
		{
			D3D11_TEXTURE2D_DESC desc = {
				nDim1,
				nDim2,
				nMipLevels,
				6,
				format,
				{ 1, 0 },
				usage,
				bindFlags,
				cpuAccessFlags,
				D3D10_RESOURCE_MISC_TEXTURECUBE | ( bGenerateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0 )
			};
			if( data )
			{
				for( int i = 0; i < nMipLevels; i++ )
				{
					texData[i].pSysMem = data;
					texData[i].SysMemPitch = pitch;
					texData[i].SysMemSlicePitch = slicePitch;
				}
			}
			pDevice->CreateTexture2D( &desc, data? texData: NULL, (ID3D11Texture2D**)m_pTexture.AssignPtr() );
		}
		break;
	case ETextureType::Tex3D:
		{
			D3D11_TEXTURE3D_DESC desc = {
				nDim1,
				nDim2,
				nDim3,
				nMipLevels,
				format,
				usage,
				bindFlags,
				cpuAccessFlags,
				bGenerateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0
			};
			if( data )
			{
				for( int i = 0; i < nMipLevels; i++ )
				{
					texData[i].pSysMem = data;
					texData[i].SysMemPitch = pitch;
					texData[i].SysMemSlicePitch = slicePitch;
				}
			}
			pDevice->CreateTexture3D( &desc, data? texData: NULL, (ID3D11Texture3D**)m_pTexture.AssignPtr() );
		}
		break;
	}

	if( !bBindDepthStencil )
		m_pSRV = new CShaderResource( pDevice, m_pTexture, NULL, EShaderResourceType::Texture2D, this, bGenerateMips );
	if( m_desc.bBindRenderTarget )
		m_pRTV = new CRenderTarget( pDevice, m_pTexture, NULL, this );
	if( bBindDepthStencil )
		m_pDSV = new CDepthStencil( pDevice, m_pTexture, NULL, this );
	m_nActualMipLevel = nMipLevels;
}

uint32 CTexture::GetData( void ** ppData )
{
	CRenderSystem* pRenderSystem = static_cast<CRenderSystem*>( IRenderSystem::Inst() );
	auto pDevice = pRenderSystem->GetDevice();
	auto pDeviceContext = pRenderSystem->GetDeviceContext();

	CReference<ID3D11Resource> pResource;
	DXGI_FORMAT format = GetDXGIFormat( m_desc.eFormat );
	switch( m_desc.eType )
	{
	case ETextureType::Tex2D:
	{
		D3D11_TEXTURE2D_DESC desc = {
			m_desc.nDim1,
			m_desc.nDim2,
			1,
			1,
			format,
			{ 1, 0 },
			D3D11_USAGE_STAGING,
			0,
			D3D11_CPU_ACCESS_READ,
			0
		};
		pDevice->CreateTexture2D( &desc, NULL, (ID3D11Texture2D**)pResource.AssignPtr() );
		break;
	}
	case ETextureType::TexCube:
	{
		D3D11_TEXTURE2D_DESC desc = {
			m_desc.nDim1,
			m_desc.nDim2,
			1,
			1,
			format,
			{ 1, 0 },
			D3D11_USAGE_STAGING,
			0,
			D3D11_CPU_ACCESS_READ,
			0
		};
		pDevice->CreateTexture2D( &desc, NULL, (ID3D11Texture2D**)pResource.AssignPtr() );
		break;
	}
	case ETextureType::Tex3D:
	{
		D3D11_TEXTURE3D_DESC desc = {
			m_desc.nDim1,
			m_desc.nDim2,
			m_desc.nDim3,
			1,
			format,
			D3D11_USAGE_STAGING,
			0,
			D3D11_CPU_ACCESS_READ,
			0
		};
		pDevice->CreateTexture3D( &desc, NULL, (ID3D11Texture3D**)pResource.AssignPtr() );
	}
	break;
	}

	uint32 nSize = 0;
	uint8* pData;

	switch( m_desc.eType )
	{
	case ETextureType::Tex2D:
	{
		pDeviceContext->CopySubresourceRegion( pResource, 0, 0, 0, 0, m_pTexture, 0, NULL );
		D3D11_MAPPED_SUBRESOURCE resource;
		pDeviceContext->Map( pResource, 0, D3D11_MAP_READ, 0, &resource );
		nSize = resource.RowPitch * m_desc.nDim2;
		pData = (uint8*)malloc( nSize );
		memcpy( pData, resource.pData, nSize );
		pDeviceContext->Unmap( pResource, 0 );
		break;
	}
	case ETextureType::TexCube:
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		uint8* pData1 = NULL;
		for( int i = 0; i < 6; i++ )
		{
			pDeviceContext->CopySubresourceRegion( pResource, 0, 0, 0, 0, m_pTexture, i * m_nActualMipLevel, NULL );
			pDeviceContext->Map( pResource, 0, D3D11_MAP_READ, 0, &resource );
			if( !pData1 )
			{
				nSize = resource.RowPitch * m_desc.nDim2;
				pData = pData1 = (uint8*)malloc( nSize * 6 );
			}
			memcpy( pData1, resource.pData, nSize );
			pDeviceContext->Unmap( pResource, 0 );
			pData1 += nSize;
		}
		nSize *= 6;
		break;
	}
	case ETextureType::Tex3D:
	{
		pDeviceContext->CopySubresourceRegion( pResource, 0, 0, 0, 0, m_pTexture, 0, NULL );
		D3D11_MAPPED_SUBRESOURCE resource;
		pDeviceContext->Map( pResource, 0, D3D11_MAP_READ, 0, &resource );
		nSize = resource.DepthPitch * m_desc.nDim3;
		pData = (uint8*)malloc( nSize );
		memcpy( pData, resource.pData, nSize );
		pDeviceContext->Unmap( pResource, 0 );
		break;
	}
	}

	*ppData = pData;
	return uint32();
}

ITexture* CRenderSystem::CreateTexture( ETextureType eType, uint32 nDim1, uint32 nDim2, uint32 nDim3, uint32 nMipLevels, EFormat eFormat, void* data,
	bool bIsDynamic, bool bBindRenderTarget, bool bBindDepthStencil )
{
	return new CTexture( m_pd3dDevice, eType, nDim1, nDim2, nDim3, nMipLevels, eFormat, data, bIsDynamic, bBindRenderTarget, bBindDepthStencil );
}

void CRenderSystem::CopyResource( ITexture* pDst, ITexture* pSrc )
{
	m_pDeviceContext->CopyResource( static_cast<CTexture*>( pDst )->GetTexture(), static_cast<CTexture*>( pSrc )->GetTexture() );
}

void CRenderSystem::UpdateSubResource( ITexture* pDst, void* pData, TVector3<uint32> vMin, TVector3<uint32> vMax, uint32 nRowPitch, uint32 nDepthPitch )
{
	D3D11_BOX box = { vMin.x, vMin.y, vMin.z, vMax.x, vMax.y, vMax.z };
	m_pDeviceContext->UpdateSubresource( static_cast<CTexture*>( pDst )->GetTexture(),
		0,
		&box,
		pData,
		nRowPitch,
		nDepthPitch );
}