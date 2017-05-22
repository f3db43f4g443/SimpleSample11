#include "stdafx.h"
#include "DX11RenderSystem.h"
#include "Renderer.h"
#include "DX11ConstantBuffer.h"
#include "DX11Shader.h"
#include "DX11ShaderResource.h"
#include "DX11RenderState.h"
#include "DX11Texture.h"
#include "DX11VertexBuffer.h"
#include "DX11IndexBuffer.h"
#include "Game.h"

#pragma comment(lib, "d3dcompiler.lib")
#ifdef _DEBUG
#pragma comment(lib, "d3dx11d.lib")
#else
#pragma comment(lib, "d3dx11.lib")
#endif
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dsound.lib")

#if _MSC_VER >= 1900
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

IRenderSystem::IRenderSystem()
	: m_pGame( NULL ), m_pRenderer( NULL ), m_dLastTime( 0 ), m_dTime( 0 ), m_fElapsedTime( 0 ), m_fTimeScale( 1 ), m_fTimeScaleA( 0 )
{
	
}

IRenderSystem* IRenderSystem::Inst()
{
	static CRenderSystem system;
	return &system;
}

void CRenderSystem::SetPrimitiveType( EPrimitiveType ePrimitiveType )
{
	m_stateMgr.GetState().primitiveType = (D3D11_PRIMITIVE_TOPOLOGY)ePrimitiveType;
	m_stateMgr.SetStateDirty( eDeviceStatePrimitiveType );
}

void CRenderSystem::SetBlendState( IBlendState* pState, const CVector4& blendFactor, uint32 nSampleMask )
{
	m_stateMgr.GetState().blendState = pState? static_cast<CBlendState*>( pState )->GetState(): NULL;
	m_stateMgr.GetState().blendFactor = blendFactor;
	m_stateMgr.GetState().sampleMask = nSampleMask;
	m_stateMgr.SetStateDirty( eDeviceStateBlend );
}

void CRenderSystem::SetDepthStencilState( IDepthStencilState* pState, uint32 nStencilRef )
{
	m_stateMgr.GetState().depthStencilState = pState? static_cast<CDepthStencilState*>( pState )->GetState(): NULL;
	m_stateMgr.GetState().stencilRef = nStencilRef;
	m_stateMgr.SetStateDirty( eDeviceStateDepthStencil );
}

void CRenderSystem::SetRasterizerState( IRasterizerState* pState )
{
	m_stateMgr.GetState().rasterizerState = pState? static_cast<CRasterizerState*>( pState )->GetState(): NULL;
	m_stateMgr.SetStateDirty( eDeviceStateRasterizer );
}

void CRenderSystem::SetRenderTargets( IRenderTarget** ppRenderTargets, uint32 nRenderTargets, IDepthStencil* pDepthStencil )
{
	nRenderTargets = MIN( nRenderTargets, MAX_RENDER_TARGETS );
	m_stateMgr.GetState().nViews = nRenderTargets;
	for( int i = 0; i < nRenderTargets; i++ )
		m_stateMgr.GetState().renderTargetViews[i] = static_cast<CRenderTarget*>( ppRenderTargets[i] )->GetRenderTargetView();
	m_stateMgr.GetState().depthStencilView = pDepthStencil? static_cast<CDepthStencil*>( pDepthStencil )->GetDepthStencilView(): NULL;
	m_stateMgr.SetStateDirty( eDeviceStateRenderTargets );
	
	for( int i = 0; i < nRenderTargets; i++ )
	{
		m_pUsingRenderTargets[i] = ppRenderTargets[i];
	}
	for( int i = nRenderTargets; i < m_nUsingRenderTargets; i++ )
	{
		m_pUsingRenderTargets[i] = NULL;
	}
	m_nUsingRenderTargets = nRenderTargets;
}

void CRenderSystem::SetSOTargets( IStreamOutput** ppSOTargets, uint32* pSOOffsets, uint32 nSOTargets )
{
	nSOTargets = MIN( nSOTargets, MAX_SO_TARGETS );
	m_stateMgr.GetState().nSOTargets = nSOTargets;
	for( int i = 0; i < nSOTargets; i++ )
	{
		m_stateMgr.GetState().soTargets[i] = static_cast<CStreamOutput*>( ppSOTargets[i] )->GetBuffer();
		m_stateMgr.GetState().soOffsets[i] = pSOOffsets? pSOOffsets[i]: 0;
	}
	m_stateMgr.SetStateDirty( eDeviceStateSOTargets );
	
	for( int i = 0; i < nSOTargets; i++ )
	{
		m_pUsingStreamOutputs[i] = ppSOTargets[i];
	}
	for( int i = nSOTargets; i < m_nUsingStreamOutputs; i++ )
	{
		m_pUsingStreamOutputs[i] = NULL;
	}
	m_nUsingStreamOutputs = nSOTargets;
}

void CRenderSystem::SetViewports( SViewport* pViewports, uint32 nViewports )
{
	nViewports = MIN( nViewports, MAX_VIEWPORTS );
	m_stateMgr.GetState().nViewports = nViewports;
	for( int i = 0; i < nViewports; i++ )
	{
		m_stateMgr.GetState().viewports[i].TopLeftX = pViewports[i].fX;
		m_stateMgr.GetState().viewports[i].TopLeftY = pViewports[i].fY;
		m_stateMgr.GetState().viewports[i].Width = pViewports[i].fWidth;
		m_stateMgr.GetState().viewports[i].Height = pViewports[i].fHeight;
		m_stateMgr.GetState().viewports[i].MinDepth = pViewports[i].fMinDepth;
		m_stateMgr.GetState().viewports[i].MaxDepth = pViewports[i].fMaxDepth;
	}
	m_stateMgr.SetStateDirty( eDeviceStateViewports );
}

void CRenderSystem::SetVertexBuffer( uint32 nIndex, IVertexBuffer* pVertexBuffer )
{
	if( nIndex >= MAX_VERTEX_BUFFER_BIND_COUNT )
		return;
	m_pUsingVertexBuffers[nIndex] = static_cast<CVertexBuffer*>( pVertexBuffer );
	m_stateMgr.GetState().vertexBuffers[nIndex] = static_cast<CVertexBuffer*>( pVertexBuffer )->GetBuffer();
	m_stateMgr.GetState().vbStrides[nIndex] = static_cast<CVertexBuffer*>( pVertexBuffer )->GetDesc().nStride;
	m_stateMgr.GetState().vbOffsets[nIndex] = 0;
	m_stateMgr.SetStateDirty( eDeviceStateVertexBuffer + nIndex );
}

void CRenderSystem::SetIndexBuffer( IIndexBuffer* pIndexBuffer )
{
	m_pUsingIndexBuffer = static_cast<CIndexBuffer*>( pIndexBuffer );
	m_stateMgr.GetState().indexBuffer = static_cast<CIndexBuffer*>( pIndexBuffer )->GetBuffer();
	m_stateMgr.GetState().ibFormat = GetDXGIFormat( static_cast<CIndexBuffer*>( pIndexBuffer )->GetDesc().eFormat );
	m_stateMgr.GetState().ibOffset = 0;
	m_stateMgr.SetStateDirty( eDeviceStateIndexBuffer );
}

void CRenderSystem::SetConstantBuffer( EShaderType eShaderType, uint32 nIndex, IConstantBuffer* pConstantBuffer )
{
	if( nIndex >= MAX_CONSTANT_BUFFER_BIND_COUNT )
		return;
	if( !pConstantBuffer )
		pConstantBuffer = m_pSharedConstantBuffers[(uint32)eShaderType][nIndex];
	auto& usingConstantBuffer = m_pUsingConstantBuffers[(uint32)eShaderType][nIndex];
	if( pConstantBuffer == usingConstantBuffer )
		return;
	static_cast<CConstantBuffer*>( usingConstantBuffer.GetPtr() )->OnUnBound();
	usingConstantBuffer = static_cast<CConstantBuffer*>( pConstantBuffer );
}

IConstantBuffer* CRenderSystem::GetConstantBuffer( EShaderType eShaderType, uint32 nIndex )
{
	if( nIndex >= MAX_CONSTANT_BUFFER_BIND_COUNT )
		return NULL;
	return m_pUsingConstantBuffers[(uint32)eShaderType][nIndex];
}

void CRenderSystem::SetShaderResource( EShaderType eShaderType, uint32 nIndex, IShaderResource* pShaderResource )
{
	auto pSRV = static_cast<CShaderResource*>( pShaderResource );
	if( pSRV )
		pSRV->OnBindSRV( m_pDeviceContext );
	switch( eShaderType )
	{
	case EShaderType::VertexShader:
		m_stateMgr.GetState().vsShaderResourceViews[nIndex] = pSRV? pSRV->GetSRV(): NULL;
		m_stateMgr.SetStateDirty( eDeviceStateVSShaderResource + nIndex );
		break;
	case EShaderType::GeometryShader:
		m_stateMgr.GetState().gsShaderResourceViews[nIndex] = pSRV? pSRV->GetSRV(): NULL;
		m_stateMgr.SetStateDirty( eDeviceStateGSShaderResource + nIndex );
		break;
	case EShaderType::PixelShader:
		m_stateMgr.GetState().psShaderResourceViews[nIndex] = pSRV? pSRV->GetSRV(): NULL;
		m_stateMgr.SetStateDirty( eDeviceStatePSShaderResource + nIndex );
		break;
	}
	m_stateMgr.GetState().shaderResourceBoundStates[(uint32)eShaderType][nIndex].SetShaderResource( pSRV );
}

void CRenderSystem::SetSampler( EShaderType eShaderType, uint32 nIndex, const ISamplerState* pSampler )
{
	switch( eShaderType )
	{
	case EShaderType::VertexShader:
		m_stateMgr.GetState().vsSamplerStates[nIndex] = static_cast<const CSamplerState*>( pSampler )->GetState();
		m_stateMgr.SetStateDirty( eDeviceStateVSSampler + nIndex );
		break;
	case EShaderType::GeometryShader:
		m_stateMgr.GetState().gsSamplerStates[nIndex] = static_cast<const CSamplerState*>( pSampler )->GetState();
		m_stateMgr.SetStateDirty( eDeviceStateGSSampler + nIndex );
		break;
	case EShaderType::PixelShader:
		m_stateMgr.GetState().psSamplerStates[nIndex] = static_cast<const CSamplerState*>( pSampler )->GetState();
		m_stateMgr.SetStateDirty( eDeviceStatePSSampler + nIndex );
		break;
	}
}

void CRenderSystem::ClearRenderTarget( const CVector4& color, IRenderTarget* pRenderTarget )
{
	ID3D11RenderTargetView* pRTV = pRenderTarget ? static_cast<CRenderTarget*>( pRenderTarget )->GetRenderTargetView() : m_stateMgr.GetState().renderTargetViews[0];
	if( pRTV )
		m_pDeviceContext->ClearRenderTargetView( pRTV, &color.x );
}

void CRenderSystem::ClearDepthStencil( bool bDepth, float fDepth, bool bStencil, uint8 nStencil, IDepthStencil* pDepthStencil )
{
	uint32 nFlag = 0;
	if( bDepth )
		nFlag |= D3D11_CLEAR_DEPTH;
	if( bStencil )
		nFlag |= D3D11_CLEAR_STENCIL;
	ID3D11DepthStencilView* pDSV = pDepthStencil ? static_cast<CDepthStencil*>( pDepthStencil )->GetDepthStencilView() : m_stateMgr.GetState().depthStencilView;
	if( pDSV )
		m_pDeviceContext->ClearDepthStencilView( pDSV, nFlag, fDepth, nStencil );
}

void CRenderSystem::Draw( uint32 VertexCount, uint32 StartVertexLocation )
{
	CommitStates();
	m_pDeviceContext->Draw( VertexCount, StartVertexLocation );
}

void CRenderSystem::DrawAuto()
{
	CommitStates();
	m_pDeviceContext->DrawAuto();
}

void CRenderSystem::DrawIndexed( uint32 IndexCount, uint32 StartIndexLocation, int32 BaseVertexLocation )
{
	CommitStates();
	m_pDeviceContext->DrawIndexed( IndexCount, StartIndexLocation, BaseVertexLocation );
}

void CRenderSystem::DrawInstanced( uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation )
{
	CommitStates();
	m_pDeviceContext->DrawInstanced( VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation );
}

void CRenderSystem::DrawIndexedInstanced( uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation )
{
	CommitStates();
	m_pDeviceContext->DrawIndexedInstanced( IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation );
}

void CRenderSystem::DrawInput()
{
	if( !m_pCurShaderBoundState )
		return;
	CVertexBuffer* pVertexBuffer = static_cast<CVertexBuffer*>( m_pUsingVertexBuffers[0].GetPtr() );
	CVertexBuffer* pInstanceBuffer = NULL;
	int32 nInstanceBufferIndex = m_pCurShaderBoundState->GetInstanceBufferIndex();
	if( nInstanceBufferIndex >= 0 )
	{
		pInstanceBuffer = static_cast<CVertexBuffer*>( m_pUsingVertexBuffers[nInstanceBufferIndex].GetPtr() );
		if( nInstanceBufferIndex == 0 )
			pVertexBuffer = static_cast<CVertexBuffer*>( m_pUsingVertexBuffers[1].GetPtr() );
	}

	if( !pVertexBuffer )
		return;

	if( pInstanceBuffer )
	{
		if( m_pUsingIndexBuffer )
			DrawIndexedInstanced( m_pUsingIndexBuffer->GetDesc().nIndices, pInstanceBuffer->GetDesc().nVertices, 0, 0, 0 );
		else
			DrawInstanced( pVertexBuffer->GetDesc().nVertices, pInstanceBuffer->GetDesc().nVertices, 0, 0 );
	}
	else
	{
		if( m_pUsingIndexBuffer )
			DrawIndexed( m_pUsingIndexBuffer->GetDesc().nIndices, 0, 0 );
		else if( pVertexBuffer->GetDesc().bBindStreamOutput )
			DrawAuto();
		else
			Draw( pVertexBuffer->GetDesc().nVertices, 0 );
	}
}

void CRenderSystem::DrawInputInstanced( uint32 nInstance )
{
	if( !m_pCurShaderBoundState )
		return;
	CVertexBuffer* pVertexBuffer = static_cast<CVertexBuffer*>( m_pUsingVertexBuffers[0].GetPtr() );
	if( !pVertexBuffer )
		return;
	
	if( m_pUsingIndexBuffer )
		DrawIndexedInstanced( m_pUsingIndexBuffer->GetDesc().nIndices, nInstance, 0, 0, 0 );
	else
		DrawInstanced( pVertexBuffer->GetDesc().nVertices, nInstance, 0, 0 );
}

void CRenderSystem::CommitStates()
{
	for( int i = 0; i < m_nUsingRenderTargets; i++ )
	{
		CTexture* pTexture = static_cast<CRenderTarget*>( m_pUsingRenderTargets[i].GetPtr() )->GetTexture();
		if( pTexture )
		{
			m_stateMgr.ClearShaderResourceBoundState( m_pDeviceContext, static_cast<CShaderResource*>( pTexture->GetShaderResource() ) );
			auto& desc = pTexture->GetDesc();
			if( !desc.nMipLevels && !desc.bBindDepthStencil )
			{
				CShaderResource* pShaderResource = static_cast<CShaderResource*>( pTexture->GetShaderResource() );
				if( pShaderResource )
					pShaderResource->GenMip();
			}
		}
	}
	for( int i = 0; i < m_nUsingStreamOutputs; i++ )
	{
		CVertexBuffer* pVertexBuffer = static_cast<CStreamOutput*>( m_pUsingStreamOutputs[i].GetPtr() )->GetVertexBuffer();
		if( pVertexBuffer )
		{
			if( pVertexBuffer->GetBuffer() )
				m_stateMgr.ClearBufferBoundState( m_pDeviceContext, pVertexBuffer->GetBuffer() );
			if( pVertexBuffer->GetShaderResource() )
				m_stateMgr.ClearShaderResourceBoundState( m_pDeviceContext, static_cast<CShaderResource*>( pVertexBuffer->GetShaderResource() ) );
		}
	}

	if( m_pUsingVertexShader )
	{
		uint32 nConstantBuffers = m_pUsingVertexShader->GetShaderInfo().nMaxConstantBuffer;
		for( int i = 0; i < nConstantBuffers; i++ )
		{
			CConstantBuffer* pConstantBuffer = static_cast<CConstantBuffer*>( m_pUsingConstantBuffers[(uint32)EShaderType::VertexShader][i].GetPtr() );
			pConstantBuffer->CommitChanges( m_pd3dDevice, m_pDeviceContext );
			m_stateMgr.GetState().vsConstantBuffers[i] = pConstantBuffer->GetBuffer();
			m_stateMgr.SetStateDirty( eDeviceStateVSConstantBuffer + i );
		}
	}

	if( m_pUsingGeometryShader )
	{
		uint32 nConstantBuffers = m_pUsingGeometryShader->GetShaderInfo().nMaxConstantBuffer;
		for( int i = 0; i < nConstantBuffers; i++ )
		{
			CConstantBuffer* pConstantBuffer = static_cast<CConstantBuffer*>( m_pUsingConstantBuffers[(uint32)EShaderType::GeometryShader][i].GetPtr() );
			pConstantBuffer->CommitChanges( m_pd3dDevice, m_pDeviceContext );
			m_stateMgr.GetState().gsConstantBuffers[i] = pConstantBuffer->GetBuffer();
			m_stateMgr.SetStateDirty( eDeviceStateGSConstantBuffer + i );
		}
	}

	if( m_pUsingPixelShader )
	{
		uint32 nConstantBuffers = m_pUsingPixelShader->GetShaderInfo().nMaxConstantBuffer;
		for( int i = 0; i < nConstantBuffers; i++ )
		{
			CConstantBuffer* pConstantBuffer = static_cast<CConstantBuffer*>( m_pUsingConstantBuffers[(uint32)EShaderType::PixelShader][i].GetPtr() );
			pConstantBuffer->CommitChanges( m_pd3dDevice, m_pDeviceContext );
			m_stateMgr.GetState().psConstantBuffers[i] = pConstantBuffer->GetBuffer();
			m_stateMgr.SetStateDirty( eDeviceStatePSConstantBuffer + i );
		}
	}

	m_stateMgr.CommitStates( m_pDeviceContext );
}

BOOL InitDSound( HWND hwnd, IDirectSound* &lpds )
{
	HRESULT hr;
	// Create DirectSound.
	if( FAILED( hr = DirectSoundCreate( NULL, &lpds, NULL ) ) )
		return FALSE;

	// Set cooperative level.
	if( FAILED( hr = lpds->SetCooperativeLevel( hwnd, DSSCL_PRIORITY ) ) )
		return FALSE;
	return TRUE;
}

void CRenderSystem::CreateDevice( const SDeviceCreateContext& context )
{// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// DXUT will create and use the best device (either D3D9 or D3D11) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set DXUT callbacks
	DXUTSetCallbackMsgProc( MsgProc, this );
	DXUTSetCallbackKeyboard( OnKeyboard, this );
	DXUTSetCallbackFrameMove( OnFrameMove, this );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings, this );

	DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable, this );

	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable, this );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice, this );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain, this );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain, this );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice, this );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender, this );
	DXUTSetIsInGammaCorrectMode( false );

	DXUTInit( true, false, NULL );
	DXUTSetCursorSettings( true, true );
	DXUTCreateWindow( L"SimpleSample11" );

	// Only require 10-level hardware, change to D3D_FEATURE_LEVEL_11_0 to require 11-class hardware
	// Switch to D3D_FEATURE_LEVEL_9_x for 10level9 hardware
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, context.resolution.x, context.resolution.y );
	InitDSound( DXUTGetHWND(), m_pDSound );
}

void CRenderSystem::Start()
{
	m_pGame->Start();
	DXUTMainLoop();
	m_pGame->Stop();
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK CRenderSystem::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext )
{
	HRESULT hr;
	CRenderSystem* pThis = (CRenderSystem*)pUserContext;
	pThis->m_pd3dDevice = pd3dDevice;

	for( int i = 0; i < (int)EShaderType::Count; i++ )
	{
		for( int j = 0; j < MAX_CONSTANT_BUFFER_BIND_COUNT; j++ )
		{
			pThis->m_pSharedConstantBuffers[i][j] = new CConstantBuffer( pd3dDevice, MAX_CONSTANT_BUFFER_SIZE, true );
			pThis->m_pUsingConstantBuffers[i][j] = pThis->m_pSharedConstantBuffers[i][j];
		}
	}

	pThis->m_pRenderer->OnCreateDevice( pThis );
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK CRenderSystem::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;

	CRenderSystem* pThis = (CRenderSystem*)pUserContext;
	D3D11_VIEWPORT viewport =
	{
		0,
		0,
		pBackBufferSurfaceDesc->Width,
		pBackBufferSurfaceDesc->Height,
		0,
		1
	};
	pThis->m_stateMgr.OnSwapChainResized( NULL, NULL, &viewport );
	pThis->m_screenRes = CVector2( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	pThis->m_pRenderer->OnResize( pThis, pThis->m_screenRes );
	if( pThis->m_pGame )
		pThis->m_pGame->OnResize( pThis->m_screenRes );
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK CRenderSystem::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext )
{
	CRenderSystem* pThis = (CRenderSystem*)pUserContext;
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pThis->m_defaultRenderTarget.SetRenderTargetView( pRTV );
	pThis->m_defaultDepthStencil.SetDepthStencilView( pDSV );
	pThis->m_stateMgr.OnSwapChainResized( pRTV, pDSV, NULL );

	float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );

	// Clear the depth stencil
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	pThis->m_pDeviceContext = pd3dImmediateContext;
	pThis->m_pRenderer->OnRender( pThis );

	static DWORD dwTimefirst = GetTickCount();
	if( GetTickCount() - dwTimefirst > 5000 )
	{
		dwTimefirst = GetTickCount();
	}

	pThis->m_defaultRenderTarget.SetRenderTargetView( NULL );
	pThis->m_defaultDepthStencil.SetDepthStencilView( NULL );
	pThis->m_stateMgr.OnSwapChainResized( NULL, NULL, NULL );
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK CRenderSystem::OnD3D11ReleasingSwapChain( void* pUserContext )
{
	CRenderSystem* pThis = (CRenderSystem*)pUserContext;
	pThis->m_pRenderer->OnEnterBackground( pThis );
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK CRenderSystem::OnD3D11DestroyDevice( void* pUserContext )
{
	CRenderSystem* pThis = (CRenderSystem*)pUserContext;
	pThis->m_pRenderer->OnDestroyDevice( pThis );
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK CRenderSystem::ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	if( pDeviceSettings->ver == DXUT_D3D9_DEVICE )
	{
		IDirect3D9* pD3D = DXUTGetD3D9Object( );
		D3DCAPS9 Caps;
		pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal, pDeviceSettings->d3d9.DeviceType, &Caps );

		// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
		// then switch to SWVP.
		if( ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
			Caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
		{
			pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// Debugging vertex shaders requires either REF or software vertex processing 
		// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
		if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
		{
			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
			pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
#endif
#ifdef DEBUG_PS
		pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
	}

	return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK CRenderSystem::OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	CRenderSystem* pThis = (CRenderSystem*)pUserContext;

	if( pThis->m_fTimeScale != 1 || pThis->m_fTimeScaleA != 0 )
	{
		float fTimeScale0 = pThis->m_fTimeScale;
		float fTimeScale1 = fTimeScale0 + fElapsedTime * pThis->m_fTimeScaleA;
		if( fTimeScale1 >= 1 )
		{
			fElapsedTime = fElapsedTime - ( 1 - fTimeScale0 ) * ( 1 - fTimeScale0 ) * 0.5f / pThis->m_fTimeScaleA;

			pThis->m_fTimeScale = 1;
			pThis->m_fTimeScaleA = 0;
		}
		else if( fTimeScale1 <= 0 )
		{
			fElapsedTime = fTimeScale0 * fTimeScale0 * 0.5f / -pThis->m_fTimeScaleA;
			pThis->m_fTimeScale = 0;
			pThis->m_fTimeScaleA = 0;
		}
		else
		{
			fElapsedTime = fElapsedTime * ( fTimeScale0 + fTimeScale1 ) * 0.5f;
			pThis->m_fTimeScale = fTimeScale1;
		}
	}
	fElapsedTime = Min( fElapsedTime, 0.1f );

	pThis->m_fElapsedTime = fElapsedTime;
	pThis->m_dTime = pThis->m_dLastTime + pThis->m_fElapsedTime;
	pThis->m_pGame->Update();
	pThis->m_pRenderer->OnUpdate( pThis );
	
	double dLastTime = pThis->GetLastTime();
	double dTotalTime = pThis->GetTotalTime();
	uint32 nFrames = floor( dTotalTime * 1000 ) - floor( dLastTime * 1000 );
	for( int i = 0; i < nFrames; i++ )
		pThis->UpdateTime();
	pThis->m_dLastTime = pThis->m_dTime;
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK CRenderSystem::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext )
{
    int iMouseX = ( short )LOWORD( lParam );
    int iMouseY = ( short )HIWORD( lParam );
	CRenderSystem* pThis = (CRenderSystem*)pUserContext;

    switch( uMsg )
    {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
			if( pThis->m_pGame )
				pThis->m_pGame->OnMouseDown( CVector2( iMouseX, iMouseY ) );
            return TRUE;

        case WM_LBUTTONUP:
			if( pThis->m_pGame )
				pThis->m_pGame->OnMouseUp( CVector2( iMouseX, iMouseY ) );
            return TRUE;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			if( pThis->m_pGame )
				pThis->m_pGame->OnRightMouseDown( CVector2( iMouseX, iMouseY ) );
			return TRUE;

		case WM_RBUTTONUP:
			if( pThis->m_pGame )
				pThis->m_pGame->OnRightMouseUp( CVector2( iMouseX, iMouseY ) );
			return TRUE;

        case WM_MOUSEMOVE:
			if( pThis->m_pGame )
				pThis->m_pGame->OnMouseMove( CVector2( iMouseX, iMouseY ) );
            return TRUE;

		case WM_CHAR:
			if( pThis->m_pGame )
				pThis->m_pGame->OnChar( wParam );
            return TRUE;
    }
	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK CRenderSystem::OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	CRenderSystem* pThis = (CRenderSystem*)pUserContext;
	pThis->m_pGame->OnKey( nChar, bKeyDown, bAltDown );
}