#pragma once

#include "DX11Common.h"
#include "Texture.h"

class CTexture : public ITexture
{
public:
	CTexture( ID3D11Device* pDevice, ETextureType eType, int nDim1, int nDim2, int nDim3, int nMipLevels, EFormat eFormat, void* data,
		bool bIsDynamic = false, bool bBindRenderTarget = false, bool bBindDepthStencil = false );

	virtual uint32 GetData( void** ppData ) override;
	ID3D11Resource* GetTexture() { return m_pTexture; }
private:
	CReference<ID3D11Resource> m_pTexture;
	uint32 m_nActualMipLevel;
};

class CRenderTarget : public IRenderTarget
{
public:
	CRenderTarget() : IRenderTarget( NULL ) {}
	CRenderTarget( ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ITexture* pTexture )
		: IRenderTarget( pTexture )
	{
		pDevice->CreateRenderTargetView( pResource, pDesc, m_pRTV.AssignPtr() );
	}
	
	CTexture* GetTexture() { return static_cast<CTexture*>( m_pTexture ); }
	ID3D11RenderTargetView* GetRenderTargetView() { return m_pRTV; }
	void SetRenderTargetView( ID3D11RenderTargetView* pRTV ) { m_pRTV = pRTV; }
private:
	CReference<ID3D11RenderTargetView> m_pRTV;
};

class CDepthStencil : public IDepthStencil
{
public:
	CDepthStencil() : IDepthStencil( NULL ) {}
	CDepthStencil( ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ITexture* pTexture )
		: IDepthStencil( pTexture )
	{
		pDevice->CreateDepthStencilView( pResource, pDesc, m_pDSV.AssignPtr() );
	}
	
	CTexture* GetTexture() { return static_cast<CTexture*>( m_pTexture ); }
	ID3D11DepthStencilView* GetDepthStencilView() { return m_pDSV; }
	void SetDepthStencilView( ID3D11DepthStencilView* pDSV ) { m_pDSV = pDSV; }
private:
	CReference<ID3D11DepthStencilView> m_pDSV;
};