#pragma once
#include "DX11Common.h"
#include "ShaderResource.h"
#include "LinkList.h"

class CShaderResource;
struct SShaderResourceBoundState
{
	SShaderResourceBoundState() : nSlot( 0 ), pShaderResource( NULL ) {}
	void SetShaderResource( CShaderResource* pNewShaderResource );

	uint32 nSlot;
	CShaderResource* pShaderResource;
	LINK_LIST( SShaderResourceBoundState, State )
};

class CTexture;
class CVertexBuffer;
class CIndexBuffer;
class CShaderResource : public IShaderResource
{
public:
	CShaderResource( ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, EShaderResourceType eType, CReferenceObject* pOrigObject, bool bNeedGenMip = false )
		: IShaderResource( eType, pOrigObject ), m_bNeedGenMip( bNeedGenMip ), m_pShaderResourceBoundStates( NULL )
	{
		pDevice->CreateShaderResourceView( pResource, pDesc, m_pSRV.AssignPtr() );
	}

	ID3D11ShaderResourceView* GetSRV() const { return m_pSRV; }
	void OnBindSRV( ID3D11DeviceContext* pDeviceContext );

	CTexture* GetTexture() const;
	CVertexBuffer* GetBuffer() const;
	void GenMip() { m_bNeedGenMip = true; }
private:
	CReference<ID3D11ShaderResourceView> m_pSRV;
	bool m_bNeedGenMip;

	LINK_LIST_HEAD( m_pShaderResourceBoundStates, SShaderResourceBoundState, State )
};

class CStreamOutput : public IStreamOutput
{
public:
	CStreamOutput( ID3D11Buffer* pBuffer, EStreamOutputType eType, CReferenceObject* pOrigObject ) : IStreamOutput( eType, pOrigObject ), m_pBuffer( pBuffer ) {}

	ID3D11Buffer* GetBuffer() const { return m_pBuffer; }
	CVertexBuffer* GetVertexBuffer() const;
	CIndexBuffer* GetIndexBuffer() const;
private:
	CReference<ID3D11Buffer> m_pBuffer;
};