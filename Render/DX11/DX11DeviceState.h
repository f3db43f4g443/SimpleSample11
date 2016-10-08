#pragma once
#include "DX11Common.h"
#include "Math3D.h"
#include "Reference.h"
#include "DX11ShaderResource.h"

class CDeviceState
{
public:
	CDeviceState();

	ID3D11BlendState* blendState;
	CVector4 blendFactor;
	uint32 sampleMask;

	ID3D11DepthStencilState* depthStencilState;
	uint32 stencilRef;

	ID3D11RasterizerState* rasterizerState;

	ID3D11VertexShader* vertexShader;
	ID3D11Buffer* vsConstantBuffers[MAX_CONSTANT_BUFFER_BIND_COUNT];
	ID3D11ShaderResourceView* vsShaderResourceViews[MAX_SHADER_RESOURCE_BIND_COUNT];
	ID3D11SamplerState* vsSamplerStates[MAX_SAMPLER_COUNT];
	ID3D11GeometryShader* geometryShader;
	ID3D11Buffer* gsConstantBuffers[MAX_CONSTANT_BUFFER_BIND_COUNT];
	ID3D11ShaderResourceView* gsShaderResourceViews[MAX_SHADER_RESOURCE_BIND_COUNT];
	ID3D11SamplerState* gsSamplerStates[MAX_SAMPLER_COUNT];
	ID3D11PixelShader* pixelShader;
	ID3D11Buffer* psConstantBuffers[MAX_CONSTANT_BUFFER_BIND_COUNT];
	ID3D11ShaderResourceView* psShaderResourceViews[MAX_SHADER_RESOURCE_BIND_COUNT];
	ID3D11SamplerState* psSamplerStates[MAX_SAMPLER_COUNT];

	ID3D11Buffer* vertexBuffers[MAX_VERTEX_BUFFER_BIND_COUNT];
	uint32 vbStrides[MAX_VERTEX_BUFFER_BIND_COUNT];
	uint32 vbOffsets[MAX_VERTEX_BUFFER_BIND_COUNT];

	ID3D11Buffer* indexBuffer;
	DXGI_FORMAT ibFormat;
	uint32 ibOffset;

	ID3D11InputLayout* inputLayout;
	D3D11_PRIMITIVE_TOPOLOGY primitiveType;

	uint32 nViews;
	ID3D11RenderTargetView* renderTargetViews[MAX_RENDER_TARGETS];
	ID3D11DepthStencilView* depthStencilView;

	uint32 nViewports;
	D3D11_VIEWPORT viewports[MAX_VIEWPORTS];

	uint32 nSOTargets;
	ID3D11Buffer* soTargets[MAX_SO_TARGETS];
	uint32 soOffsets[MAX_SO_TARGETS];

	SShaderResourceBoundState shaderResourceBoundStates[3][MAX_SHADER_RESOURCE_BIND_COUNT];
};

enum
{
	eDeviceStatePrimitiveType,
	eDeviceStateBlend,
	eDeviceStateDepthStencil,
	eDeviceStateRasterizer,
	eDeviceStateInputLayout,
	eDeviceStateRenderTargets,
	eDeviceStateSOTargets,
	eDeviceStateViewports,
	eDeviceStateVertexBuffer,
	eDeviceStateVertexBufferLast = eDeviceStateVertexBuffer + MAX_VERTEX_BUFFER_BIND_COUNT - 1,
	eDeviceStateIndexBuffer,
	eDeviceStateVertexShader,
	eDeviceStateGeometryShader,
	eDeviceStatePixelShader,
	eDeviceStateVSConstantBuffer,
	eDeviceStateVSConstantBufferLast = eDeviceStateVSConstantBuffer + MAX_CONSTANT_BUFFER_BIND_COUNT - 1,
	eDeviceStateGSConstantBuffer,
	eDeviceStateGSConstantBufferLast = eDeviceStateGSConstantBuffer + MAX_CONSTANT_BUFFER_BIND_COUNT - 1,
	eDeviceStatePSConstantBuffer,
	eDeviceStatePSConstantBufferLast = eDeviceStatePSConstantBuffer + MAX_CONSTANT_BUFFER_BIND_COUNT - 1,
	eDeviceStateVSShaderResource,
	eDeviceStateVSShaderResourceLast = eDeviceStateVSShaderResource + MAX_SHADER_RESOURCE_BIND_COUNT - 1,
	eDeviceStateGSShaderResource,
	eDeviceStateGSShaderResourceLast = eDeviceStateGSShaderResource + MAX_SHADER_RESOURCE_BIND_COUNT - 1,
	eDeviceStatePSShaderResource,
	eDeviceStatePSShaderResourceLast = eDeviceStatePSShaderResource + MAX_SHADER_RESOURCE_BIND_COUNT - 1,
	eDeviceStateVSSampler,
	eDeviceStateVSSamplerLast = eDeviceStateVSSampler + MAX_SAMPLER_COUNT - 1,
	eDeviceStateGSSampler,
	eDeviceStateGSSamplerLast = eDeviceStateGSSampler + MAX_SAMPLER_COUNT - 1,
	eDeviceStatePSSampler,
	eDeviceStatePSSamplerLast = eDeviceStatePSSampler + MAX_SAMPLER_COUNT - 1,

	eDeviceStateCount
};

class CDeviceStateMgr
{
public:
	CDeviceStateMgr();

	void OnSwapChainResized( ID3D11RenderTargetView* pRenderTarget, ID3D11DepthStencilView* pDepthStencil, const D3D11_VIEWPORT* viewport )
	{
		m_oldState.nViews = m_curState.nViews = 1;
		m_oldState.renderTargetViews[0] = m_curState.renderTargetViews[0] = pRenderTarget;
		m_oldState.depthStencilView = m_curState.depthStencilView = pDepthStencil;
		m_oldState.nViewports = m_curState.nViewports = 1;
		if( viewport )
			m_oldState.viewports[0] = m_curState.viewports[0] = *viewport;
	}

	void ClearShaderResourceBoundState( ID3D11DeviceContext* pDeviceContext, CShaderResource* pShaderResource );
	void ClearBufferBoundState( ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pVertexBuffer );
	void SetStateDirty( uint32 nState );
	void CommitStates( ID3D11DeviceContext* pDeviceContext );
	CDeviceState& GetState() { return m_curState; }
private:
	CDeviceState m_oldState;
	CDeviceState m_curState;
	bool m_bStateDirty[eDeviceStateCount];
	uint32 m_nDirtyStateIndex[eDeviceStateCount];
	uint32 m_nDirtyStates;

	typedef void( CDeviceStateMgr::*CommitFunc ) ( ID3D11DeviceContext*, uint32 );
	CommitFunc m_pCommitFuncs[eDeviceStateCount];

	void CommitPrimitiveType( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitBlendState( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitDepthStencilState( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitRasterizerState( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitInputLayout( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitRenderTargets( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitSOTargets( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitViewports( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitVertexBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitIndexBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitVertexShader( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitGeometryShader( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitPixelShader( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitVSConstantBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitGSConstantBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitPSConstantBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitVSShaderResource( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitGSShaderResource( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitPSShaderResource( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitVSSampler( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitGSSampler( ID3D11DeviceContext* pDeviceContext, uint32 nState );
	void CommitPSSampler( ID3D11DeviceContext* pDeviceContext, uint32 nState );
};