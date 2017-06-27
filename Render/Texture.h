#pragma once

#include "Resource.h"
#include "ShaderResource.h"
#include "LinkList.h"

#include <map>
#include <string>

using namespace std;

enum class ETextureType
{
	Tex2D,
	TexCube,
	Tex3D
};

struct CTextureDesc
{
	CTextureDesc() {}
	CTextureDesc( ETextureType eType, int nDim1, int nDim2, int nDim3, int nMipLevels, EFormat eFormat,
		bool bIsDynamic = false, bool bBindRenderTarget = false, bool bBindDepthStencil = false )
		:eType( eType ), nDim1( nDim1 ), nDim2( nDim2 ), nDim3( nDim3 ), nMipLevels( nMipLevels ), eFormat( eFormat ),
		bIsDynamic( bIsDynamic ), bBindRenderTarget( bBindRenderTarget ), bBindDepthStencil( bBindDepthStencil ), bEmpty( false ) {}

	bool operator < ( const CTextureDesc& rhs ) const
	{
		return memcmp( this, &rhs, sizeof( CTextureDesc ) ) < 0;
	}
	bool operator == ( const CTextureDesc& rhs ) const
	{
		return memcmp( this, &rhs, sizeof( CTextureDesc ) ) == 0;
	}

	ETextureType eType;
	uint32 nDim1, nDim2, nDim3;
	uint32 nMipLevels;
	EFormat eFormat;

	bool bIsDynamic;
	bool bBindRenderTarget;
	bool bBindDepthStencil;
	bool bEmpty;
};

class ITexture;
class IRenderTarget
{
public:
	IRenderTarget( ITexture* pTexture ) : m_pTexture( pTexture ) {}
	
	void AddRef();
	void Release();
protected:
	ITexture* m_pTexture;
};

class IDepthStencil
{
public:
	IDepthStencil( ITexture* pTexture ) : m_pTexture( pTexture ) {}
	
	void AddRef();
	void Release();
protected:
	ITexture* m_pTexture;
};

class ITexture : public CReferenceObject, public IShaderResourceProxy
{
public:
	ITexture( ETextureType eType, int nDim1, int nDim2, int nDim3, int nMipLevels, EFormat eFormat, void* data,
		bool bIsDynamic = false, bool bBindRenderTarget = false, bool bBindDepthStencil = false )
		: m_desc( eType, nDim1, nDim2, nDim3, nMipLevels, eFormat, bIsDynamic, bBindRenderTarget, bBindDepthStencil ), m_pSRV( NULL ), m_pRTV( NULL ), m_pDSV( NULL ) {}
	~ITexture()
	{
		if( m_pSRV )
			delete m_pSRV;
		if( m_pRTV )
			delete m_pRTV;
		if( m_pDSV )
			delete m_pDSV;
	}

	const CTextureDesc& GetDesc() { return m_desc; }
	virtual uint32 GetData( void** ppData ) = 0;

	virtual IShaderResource* GetShaderResource() override { return m_pSRV; }
	IRenderTarget* GetRenderTarget() { return m_pRTV; }
	IDepthStencil* GetDepthStencil() { return m_pDSV; }
protected:
	CTextureDesc m_desc;
	IShaderResource* m_pSRV;
	IRenderTarget* m_pRTV;
	IDepthStencil* m_pDSV;
};

inline void IRenderTarget::AddRef()
{
	if( m_pTexture )
		m_pTexture->AddRef();
}
inline void IRenderTarget::Release()
{
	if( m_pTexture )
		m_pTexture->Release();
}

inline void IDepthStencil::AddRef()
{
	if( m_pTexture )
		m_pTexture->AddRef();
}
inline void IDepthStencil::Release()
{
	if( m_pTexture )
		m_pTexture->Release();
}

class CRenderTargetPool
{
public:
	void Clear();
	void AllocRenderTarget( CReference<ITexture>& pTexture, const CTextureDesc& desc );
	void AllocRenderTarget( CReference<ITexture>& pTexture, ETextureType eType, uint32 nDim1, uint32 nDim2, uint32 nDim3, uint32 nMipLevels, EFormat eFormat, void* data,
		bool bIsDynamic = false, bool bBindRenderTarget = false, bool bBindDepthStencil = false );
	void Release( CReference<ITexture>& pTexture );

	static CRenderTargetPool& GetSizeDependentPool()
	{
		static CRenderTargetPool g_inst;
		return g_inst;
	}
	static CRenderTargetPool& GetSizeIndependentPool()
	{
		static CRenderTargetPool g_inst;
		return g_inst;
	}
private:
	struct SItem
	{
		CReference<ITexture> pTexture;
		LINK_LIST( SItem, Item )
	};
	map<CTextureDesc, SItem*> m_mapItems;
};

class CTextureFile : public CResource
{
public:
	enum EType
	{
		eResType = eEngineResType_Texture,
	};
	CTextureFile( const char* name, int32 type ) : CResource( name, type ) {}
	void Create();

	ITexture* GetTexture() { return m_pTexture; }
	static void InitLoader();

	static bool SaveTexFile( const char* szFileName, void* pData, uint32 nWidth, uint32 nHeight );
private:
	CReference<ITexture> m_pTexture;
};
