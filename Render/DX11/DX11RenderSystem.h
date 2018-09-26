#pragma once
#include "DX11Common.h"
#include "RenderSystem.h"

#include "DX11DeviceState.h"
#include "DX11Texture.h"

class CRenderSystem : public IRenderSystem, public CTimedTrigger<4096>
{
public:
	CRenderSystem() : m_pd3dDevice( NULL ), m_pDeviceContext( NULL ), m_pCurShaderBoundState( NULL ), m_nUsingRenderTargets( 0 ), m_nUsingStreamOutputs( 0 ) {}
	virtual void CreateDevice( const SDeviceCreateContext& context ) override;
	virtual void Start() override;

	ID3D11Device* GetDevice() { return m_pd3dDevice; }
	ID3D11DeviceContext* GetDeviceContext() { return m_pDeviceContext; }

	virtual IVertexBuffer* CreateVertexBuffer( uint32 nElements, const struct SVertexBufferElement* pElements, uint32 nVertices, void* pData, bool bIsDynamic = false,
		bool bBindShaderResource = false, bool bBindStreamOutput = false, bool bIsInstance = false ) override;
	virtual IIndexBuffer* CreateIndexBuffer( uint32 nIndices, EFormat eFormat, void* pData, bool bIsDynamic = false, bool bBindStreamOutput = false ) override;
	virtual ITexture* CreateTexture( ETextureType eType, uint32 nDim1, uint32 nDim2, uint32 nDim3, uint32 nMipLevels, EFormat eFormat, void* data,
		bool bIsDynamic = false, bool bBindRenderTarget = false, bool bBindDepthStencil = false ) override;
	virtual IConstantBuffer* CreateConstantBuffer( uint32 nSize, bool bUsePool ) override;

	virtual ISound* CreateSound( const void* pBuffer, uint32 nSize, const SWaveFormat& format ) override;
	virtual ISoundTrack* CreateSoundTrack( ISound* pSound ) override;

	virtual IRenderTarget* GetDefaultRenderTarget() override { return &m_defaultRenderTarget; }
	virtual IDepthStencil* GetDefaultDepthStencil() override { return &m_defaultDepthStencil; }
	
	virtual IShader* LoadShader( IBufReader& buf, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc = NULL, uint32 nVertexBuffers = 0, uint32 nRasterizedStream = 0 ) override;

	virtual void GetRenderTargetTextures( CReference<ITexture>* ppTextures, uint32& nTextures, uint32 nArraySize ) override;

	virtual void SetPrimitiveType( EPrimitiveType ePrimitiveType ) override;
	virtual void SetBlendState( IBlendState* pState, const CVector4& blendFactor = CVector4( 0, 0, 0, 0 ), uint32 nSampleMask = 0xffffffff ) override;
	virtual void SetDepthStencilState( IDepthStencilState* pState, uint32 nStencilRef = 0 ) override;
	virtual void SetRasterizerState( IRasterizerState* pState ) override;

	virtual void SetRenderTargets( IRenderTarget** ppRenderTargets, uint32 nRenderTargets ) override;
	virtual void SetRenderTargets( IRenderTarget** ppRenderTargets, uint32 nRenderTargets, IDepthStencil* pDepthStencil ) override;
	virtual void SetSOTargets( IStreamOutput** ppSOTargets, uint32* pSOOffsets, uint32 nSOTargets ) override;
	virtual void SetViewports( SViewport* pViewports, uint32 nViewports ) override;

	virtual void SetVertexBuffer( uint32 nIndex, IVertexBuffer* pVertexBuffer ) override;
	virtual void SetIndexBuffer( IIndexBuffer* pIndexBuffer ) override;

	virtual void SetShaderBoundState( IShaderBoundState* &pShaderBoundState, IShader* pVertexShader, IShader* pPixelShader, const CVertexBufferDesc** ppVertexBufferDesc, uint32 nVertexBuffers, IShader* pGeometryShader = NULL, bool bSet = true ) override;

	virtual void SetConstantBuffer( EShaderType eShaderType, uint32 nIndex, IConstantBuffer* pConstantBuffer ) override;
	virtual IConstantBuffer* GetConstantBuffer( EShaderType eShaderType, uint32 nIndex ) override;
	virtual void SetShaderResource( EShaderType eShaderType, uint32 nIndex, IShaderResource* pShaderResource ) override;
	virtual void SetSampler( EShaderType eShaderType, uint32 nIndex, const ISamplerState* pSampler ) override;

	virtual void ClearRenderTarget( const CVector4& color, IRenderTarget* pRenderTarget = NULL ) override;
	virtual void ClearDepthStencil( bool bDepth, float fDepth, bool bStencil, uint8 nStencil, IDepthStencil* pDepthStencil = NULL ) override;
	virtual void Draw( uint32 VertexCount, uint32 StartVertexLocation ) override;
	virtual void DrawAuto() override;
	virtual void DrawIndexed( uint32 IndexCount, uint32 StartIndexLocation, int32 BaseVertexLocation ) override;
	virtual void DrawInstanced( uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation ) override;
	virtual void DrawIndexedInstanced( uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation ) override;
	virtual void DrawInput() override;
	virtual void DrawInputInstanced( uint32 nInstance ) override;

	virtual void CopyResource( ITexture* pDst, ITexture* pSrc ) override;
	virtual void UpdateSubResource( ITexture* pDst, void* pData, TVector3<uint32> vMin, TVector3<uint32> vMax, uint32 nRowPitch, uint32 nDepthPitch ) override;

	virtual void Lock( IVertexBuffer* pVertexBuffer, void** ppData ) override;
	virtual void Unlock( IVertexBuffer* pVertexBuffer ) override;
private:
	void CommitStates();

	IShader* CreateShader( EShaderType& shaderType, void* pShaderCode, uint32 nShaderCodeLength, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream );

	static LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );
	static void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
	static void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
	static bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

	static bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
	{
		return false;
	}

	static bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
		DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
	{
		return true;
	}
	static HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
		void* pUserContext );
	static HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	static void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
	static void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
	static void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
		float fElapsedTime, void* pUserContext );
private:
	ID3D11Device* m_pd3dDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDirectSound* m_pDSound;
	CDeviceStateMgr m_stateMgr;

	CRenderTarget m_defaultRenderTarget;
	CDepthStencil m_defaultDepthStencil;

	CReference<IRenderTarget> m_pUsingRenderTargets[MAX_RENDER_TARGETS];
	uint32 m_nUsingRenderTargets;
	CReference<IStreamOutput> m_pUsingStreamOutputs[MAX_SO_TARGETS];
	uint32 m_nUsingStreamOutputs;
	CReference<IVertexBuffer> m_pUsingVertexBuffers[MAX_VERTEX_BUFFER_BIND_COUNT];
	CReference<IIndexBuffer> m_pUsingIndexBuffer;
	class CShaderBoundState* m_pCurShaderBoundState;

	CReference<IShader> m_pUsingVertexShader;
	CReference<IShader> m_pUsingGeometryShader;
	CReference<IShader> m_pUsingPixelShader;
	CReference<IConstantBuffer> m_pSharedConstantBuffers[(uint32)EShaderType::Count][MAX_CONSTANT_BUFFER_BIND_COUNT];
	CReference<IConstantBuffer> m_pUsingConstantBuffers[(uint32)EShaderType::Count][MAX_CONSTANT_BUFFER_BIND_COUNT];
};