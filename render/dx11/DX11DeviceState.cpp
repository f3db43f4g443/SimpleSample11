#include "stdafx.h"
#include "DX11DeviceState.h"

CDeviceState::CDeviceState()
{
	blendFactor = CVector4( 0, 0, 0, 0 );
	sampleMask = 0xffffffff;
	stencilRef = 0;
	memset( vbStrides, 0, sizeof( vbStrides ) );
	memset( vbOffsets, 0, sizeof( vbOffsets ) );
	ibFormat = DXGI_FORMAT_UNKNOWN;
	ibOffset = 0;
	primitiveType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	nViews = 0;
	nViewports = 0;
	nSOTargets = 0;
}

CDeviceStateMgr::CDeviceStateMgr()
{
	memset( m_bStateDirty, 0, sizeof( m_bStateDirty ) );
	memset( m_nDirtyStateIndex, 0, sizeof( m_nDirtyStateIndex ) );
	m_nDirtyStates = 0;

	m_pCommitFuncs[eDeviceStatePrimitiveType] = &CDeviceStateMgr::CommitPrimitiveType;
	m_pCommitFuncs[eDeviceStateBlend] = &CDeviceStateMgr::CommitBlendState;
	m_pCommitFuncs[eDeviceStateDepthStencil] = &CDeviceStateMgr::CommitDepthStencilState;
	m_pCommitFuncs[eDeviceStateRasterizer] = &CDeviceStateMgr::CommitRasterizerState;
	m_pCommitFuncs[eDeviceStateInputLayout] = &CDeviceStateMgr::CommitInputLayout;
	m_pCommitFuncs[eDeviceStateRenderTargets] = &CDeviceStateMgr::CommitRenderTargets;
	m_pCommitFuncs[eDeviceStateSOTargets] = &CDeviceStateMgr::CommitSOTargets;
	m_pCommitFuncs[eDeviceStateViewports] = &CDeviceStateMgr::CommitViewports;
	for( int i = eDeviceStateVertexBuffer; i <= eDeviceStateVertexBufferLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitVertexBuffer;
	m_pCommitFuncs[eDeviceStateIndexBuffer] = &CDeviceStateMgr::CommitIndexBuffer;
	m_pCommitFuncs[eDeviceStateVertexShader] = &CDeviceStateMgr::CommitVertexShader;
	m_pCommitFuncs[eDeviceStateGeometryShader] = &CDeviceStateMgr::CommitGeometryShader;
	m_pCommitFuncs[eDeviceStatePixelShader] = &CDeviceStateMgr::CommitPixelShader;
	for( int i = eDeviceStateVSConstantBuffer; i <= eDeviceStateVSConstantBufferLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitVSConstantBuffer;
	for( int i = eDeviceStateGSConstantBuffer; i <= eDeviceStateGSConstantBufferLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitGSConstantBuffer;
	for( int i = eDeviceStatePSConstantBuffer; i <= eDeviceStatePSConstantBufferLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitPSConstantBuffer;
	for( int i = eDeviceStateVSShaderResource; i <= eDeviceStateVSShaderResourceLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitVSShaderResource;
	for( int i = eDeviceStateGSShaderResource; i <= eDeviceStateGSShaderResourceLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitGSShaderResource;
	for( int i = eDeviceStatePSShaderResource; i <= eDeviceStatePSShaderResourceLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitPSShaderResource;
	for( int i = eDeviceStateVSSampler; i <= eDeviceStateVSSamplerLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitVSSampler;
	for( int i = eDeviceStateGSSampler; i <= eDeviceStateGSSamplerLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitGSSampler;
	for( int i = eDeviceStatePSSampler; i <= eDeviceStatePSSamplerLast; i++ )
		m_pCommitFuncs[i] = &CDeviceStateMgr::CommitPSSampler;

	for( int i = 0; i < 3; i++ )
	{
		for( int j = 0; j < MAX_SHADER_RESOURCE_BIND_COUNT; j++ )
		{
			m_oldState.shaderResourceBoundStates[i][j].nSlot = j | ( i << 8 );
			m_curState.shaderResourceBoundStates[i][j].nSlot = j | ( i << 8 );
		}
	}
}

void CDeviceStateMgr::ClearShaderResourceBoundState( ID3D11DeviceContext* pDeviceContext, CShaderResource* pShaderResource )
{
	SShaderResourceBoundState* pState;

	while( pState = pShaderResource->Get_State(), pState != NULL )
	{
		pState->SetShaderResource( NULL );
		uint8* pStateIndex = (uint8*)&pState->nSlot;
		auto& deviceState = pStateIndex[2] ? m_curState : m_oldState;
		ID3D11ShaderResourceView** pSRV;
		switch( pStateIndex[1] )
		{
		case 0:
			pSRV = deviceState.vsShaderResourceViews + pStateIndex[0];
			break;
		case 1:
			pSRV = deviceState.gsShaderResourceViews + pStateIndex[0];
			break;
		case 2:
			pSRV = deviceState.psShaderResourceViews + pStateIndex[0];
			break;
		}
		ID3D11ShaderResourceView* &srv = *pSRV;
		srv = NULL;
		if( !pStateIndex[2] )
		{
			ID3D11ShaderResourceView* pSRV = NULL;
			switch( pStateIndex[1] )
			{
			case 0:
				pDeviceContext->VSSetShaderResources( pStateIndex[0], 1, &pSRV );
				break;
			case 1:
				pDeviceContext->GSSetShaderResources( pStateIndex[0], 1, &pSRV );
				break;
			case 2:
				pDeviceContext->PSSetShaderResources( pStateIndex[0], 1, &pSRV );
				break;
			}
		}
	}
}

void CDeviceStateMgr::ClearBufferBoundState( ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pVertexBuffer )
{
	for( int i = 0; i < MAX_VERTEX_BUFFER_BIND_COUNT; i++ )
	{
		if( pVertexBuffer == m_oldState.vertexBuffers[i] )
		{
			ID3D11Buffer* pBuffer = NULL;
			uint32 nStrides = 0;
			uint32 nOffsets = 0;
			pDeviceContext->IASetVertexBuffers( i, 1, &pBuffer, &nStrides, &nOffsets );
			m_oldState.vertexBuffers[i] = NULL;
			m_oldState.vbStrides[i] = 0;
			m_oldState.vbOffsets[i] = 0;
		}
		if( pVertexBuffer == m_curState.vertexBuffers[i] )
		{
			m_curState.vertexBuffers[i] = NULL;
			m_curState.vbStrides[i] = 0;
			m_curState.vbOffsets[i] = 0;
		}
	}
}

void CDeviceStateMgr::SetStateDirty( uint32 nState )
{
	if( !m_bStateDirty[nState] )
	{
		m_bStateDirty[nState] = true;
		if( nState != eDeviceStateRenderTargets )
			m_nDirtyStateIndex[m_nDirtyStates++] = nState;
	}
}

void CDeviceStateMgr::CommitStates( ID3D11DeviceContext* pDeviceContext )
{
	if( m_bStateDirty[eDeviceStateRenderTargets] )
	{
		m_bStateDirty[eDeviceStateRenderTargets] = false;
		auto func = m_pCommitFuncs[eDeviceStateRenderTargets];
		( this->*func )( pDeviceContext, eDeviceStateRenderTargets );
	}
	if( m_bStateDirty[eDeviceStateSOTargets] )
	{
		m_bStateDirty[eDeviceStateSOTargets] = false;
		auto func = m_pCommitFuncs[eDeviceStateSOTargets];
		( this->*func )( pDeviceContext, eDeviceStateSOTargets );
	}

	for( int i = 0; i < m_nDirtyStates; i++ )
	{
		uint32 nDirtyState = m_nDirtyStateIndex[i];
		m_bStateDirty[nDirtyState] = false;
		auto func = m_pCommitFuncs[nDirtyState];
		( this->*func )( pDeviceContext, nDirtyState );
	}
	m_nDirtyStates = 0;
}

void CDeviceStateMgr::CommitPrimitiveType( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.primitiveType == m_oldState.primitiveType )
		return;
	pDeviceContext->IASetPrimitiveTopology( m_curState.primitiveType );
	m_oldState.primitiveType = m_curState.primitiveType;
}

void CDeviceStateMgr::CommitBlendState( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.blendState == m_oldState.blendState && m_curState.blendFactor == m_oldState.blendFactor && m_curState.sampleMask == m_oldState.sampleMask )
		return;
	pDeviceContext->OMSetBlendState( m_curState.blendState, &m_curState.blendFactor.x, m_curState.sampleMask );
	m_oldState.blendState = m_curState.blendState;
	m_oldState.blendFactor = m_curState.blendFactor;
	m_oldState.sampleMask = m_curState.sampleMask;
}

void CDeviceStateMgr::CommitDepthStencilState( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.depthStencilState == m_oldState.depthStencilState && m_curState.stencilRef == m_oldState.stencilRef )
		return;
	pDeviceContext->OMSetDepthStencilState( m_curState.depthStencilState, m_curState.stencilRef );
	m_oldState.depthStencilState = m_curState.depthStencilState;
	m_oldState.stencilRef = m_curState.stencilRef;
}

void CDeviceStateMgr::CommitRasterizerState( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.rasterizerState == m_oldState.rasterizerState )
		return;
	pDeviceContext->RSSetState( m_curState.rasterizerState );
	m_oldState.rasterizerState = m_curState.rasterizerState;
}

void CDeviceStateMgr::CommitInputLayout( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.inputLayout == m_oldState.inputLayout )
		return;
	pDeviceContext->IASetInputLayout( m_curState.inputLayout );
	m_oldState.inputLayout = m_curState.inputLayout;
}

void CDeviceStateMgr::CommitRenderTargets( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.nViews == m_oldState.nViews && m_curState.depthStencilView == m_oldState.depthStencilView )
	{
		bool bDiff = false;
		for( int i = 0; i < m_curState.nViews; i++ )
		{
			if( m_curState.renderTargetViews[i] != m_oldState.renderTargetViews[i] )
			{
				bDiff = true;
				break;
			}
		}
		if( !bDiff )
			return;
	}
	ID3D11RenderTargetView* renderTargets[MAX_RENDER_TARGETS];
	for( int i = 0; i < m_curState.nViews; i++ )
	{
		renderTargets[i] = m_curState.renderTargetViews[i];
		m_oldState.renderTargetViews[i] = m_curState.renderTargetViews[i];
	}
	pDeviceContext->OMSetRenderTargets( m_curState.nViews, renderTargets, m_curState.depthStencilView );
	m_oldState.nViews = m_curState.nViews;
	m_oldState.depthStencilView = m_curState.depthStencilView;
}

void CDeviceStateMgr::CommitSOTargets( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.nSOTargets == m_oldState.nSOTargets )
	{
		bool bDiff = false;
		for( int i = 0; i < m_curState.nSOTargets; i++ )
		{
			if( m_curState.soTargets[i] != m_oldState.soTargets[i] || m_curState.soOffsets[i] != m_oldState.soOffsets[i] )
			{
				bDiff = true;
				break;
			}
		}
		if( !bDiff )
			return;
	}
	ID3D11Buffer* soTargets[MAX_SO_TARGETS];
	for( int i = 0; i < m_curState.nSOTargets; i++ )
	{
		soTargets[i] = m_curState.soTargets[i];
		m_oldState.soTargets[i] = m_curState.soTargets[i];
		m_oldState.soOffsets[i] = m_curState.soOffsets[i];
	}
	pDeviceContext->SOSetTargets( m_curState.nSOTargets, soTargets, m_curState.soOffsets );
	m_oldState.nSOTargets = m_curState.nSOTargets;
}

void CDeviceStateMgr::CommitViewports( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.nViewports == m_oldState.nViewports )
	{
		bool bDiff = false;
		for( int i = 0; i < m_curState.nViewports; i++ )
		{
			if( m_curState.viewports[i] != m_oldState.viewports[i] )
			{
				bDiff = true;
				break;
			}
		}
		if( !bDiff )
			return;
	}
	pDeviceContext->RSSetViewports( m_curState.nViewports, m_curState.viewports );
	m_oldState.nViewports = m_curState.nViewports;
	for( int i = 0; i < m_curState.nViewports; i++ )
	{
		m_oldState.viewports[i] = m_curState.viewports[i];
	}
}

void CDeviceStateMgr::CommitVertexBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStateVertexBuffer;
	if( m_curState.vertexBuffers[nIndex] == m_oldState.vertexBuffers[nIndex] &&
		m_curState.vbStrides[nIndex] == m_oldState.vbStrides[nIndex] &&
		m_curState.vbOffsets[nIndex] == m_oldState.vbOffsets[nIndex] )
		return;
	ID3D11Buffer* pBuffer = m_curState.vertexBuffers[nIndex];
	pDeviceContext->IASetVertexBuffers( nIndex, 1, &pBuffer, m_curState.vbStrides + nIndex, m_curState.vbOffsets + nIndex );
	m_oldState.vertexBuffers[nIndex] = m_curState.vertexBuffers[nIndex];
	m_oldState.vbStrides[nIndex] = m_curState.vbStrides[nIndex];
	m_oldState.vbOffsets[nIndex] = m_curState.vbOffsets[nIndex];
}

void CDeviceStateMgr::CommitIndexBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.indexBuffer == m_oldState.indexBuffer && m_curState.ibFormat == m_oldState.ibFormat && m_curState.ibOffset == m_oldState.ibOffset )
		return;
	pDeviceContext->IASetIndexBuffer( m_curState.indexBuffer, m_curState.ibFormat, m_curState.ibOffset );
	m_oldState.indexBuffer = m_curState.indexBuffer;
	m_oldState.ibFormat = m_curState.ibFormat;
	m_oldState.ibOffset = m_curState.ibOffset;
}

void CDeviceStateMgr::CommitVertexShader( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.vertexShader == m_oldState.vertexShader )
		return;
	pDeviceContext->VSSetShader( m_curState.vertexShader, NULL, NULL );
	m_oldState.vertexShader = m_curState.vertexShader;
}

void CDeviceStateMgr::CommitGeometryShader( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.geometryShader == m_oldState.geometryShader )
		return;
	pDeviceContext->GSSetShader( m_curState.geometryShader, NULL, NULL );
	m_oldState.geometryShader = m_curState.geometryShader;
}

void CDeviceStateMgr::CommitPixelShader( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	if( m_curState.pixelShader == m_oldState.pixelShader )
		return;
	pDeviceContext->PSSetShader( m_curState.pixelShader, NULL, NULL );
	m_oldState.pixelShader = m_curState.pixelShader;
}

void CDeviceStateMgr::CommitVSConstantBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStateVSConstantBuffer;
	if( m_curState.vsConstantBuffers[nIndex] == m_oldState.vsConstantBuffers[nIndex] )
		return;
	ID3D11Buffer* pBuffer = m_curState.vsConstantBuffers[nIndex];
	pDeviceContext->VSSetConstantBuffers( nIndex, 1, &pBuffer );
	m_oldState.vsConstantBuffers[nIndex] = m_curState.vsConstantBuffers[nIndex];
}

void CDeviceStateMgr::CommitGSConstantBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStateGSConstantBuffer;
	if( m_curState.gsConstantBuffers[nIndex] == m_oldState.gsConstantBuffers[nIndex] )
		return;
	ID3D11Buffer* pBuffer = m_curState.gsConstantBuffers[nIndex];
	pDeviceContext->GSSetConstantBuffers( nIndex, 1, &pBuffer );
	m_oldState.gsConstantBuffers[nIndex] = m_curState.gsConstantBuffers[nIndex];
}

void CDeviceStateMgr::CommitPSConstantBuffer( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStatePSConstantBuffer;
	if( m_curState.psConstantBuffers[nIndex] == m_oldState.psConstantBuffers[nIndex] )
		return;
	ID3D11Buffer* pBuffer = m_curState.psConstantBuffers[nIndex];
	pDeviceContext->PSSetConstantBuffers( nIndex, 1, &pBuffer );
	m_oldState.psConstantBuffers[nIndex] = m_curState.psConstantBuffers[nIndex];
}

void SShaderResourceBoundState::SetShaderResource( CShaderResource* pNewShaderResource )
{
	if( pShaderResource == pNewShaderResource )
		return;
	if( pShaderResource )
	{
		RemoveFrom_State();
		pShaderResource->Release();
	}
	pShaderResource = pNewShaderResource;
	if( pNewShaderResource )
	{
		pNewShaderResource->Insert_State( this );
		pNewShaderResource->AddRef();
	}
}

void CDeviceStateMgr::CommitVSShaderResource( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStateVSShaderResource;
	if( m_curState.vsShaderResourceViews[nIndex] != m_oldState.vsShaderResourceViews[nIndex] )
	{
		ID3D11ShaderResourceView* pSRV = m_curState.vsShaderResourceViews[nIndex];
		pDeviceContext->VSSetShaderResources( nIndex, 1, &pSRV );
		m_oldState.vsShaderResourceViews[nIndex] = m_curState.vsShaderResourceViews[nIndex];
	}

	SShaderResourceBoundState& oldState = m_oldState.shaderResourceBoundStates[0][nIndex];
	SShaderResourceBoundState& curState = m_curState.shaderResourceBoundStates[0][nIndex];
	oldState.SetShaderResource( curState.pShaderResource );
}

void CDeviceStateMgr::CommitGSShaderResource( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStateGSShaderResource;
	if( m_curState.gsShaderResourceViews[nIndex] != m_oldState.gsShaderResourceViews[nIndex] )
	{
		ID3D11ShaderResourceView* pSRV = m_curState.gsShaderResourceViews[nIndex];
		pDeviceContext->GSSetShaderResources( nIndex, 1, &pSRV );
		m_oldState.gsShaderResourceViews[nIndex] = m_curState.gsShaderResourceViews[nIndex];
	}

	SShaderResourceBoundState& oldState = m_oldState.shaderResourceBoundStates[1][nIndex];
	SShaderResourceBoundState& curState = m_curState.shaderResourceBoundStates[1][nIndex];
	oldState.SetShaderResource( curState.pShaderResource );
}

void CDeviceStateMgr::CommitPSShaderResource( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStatePSShaderResource;
	if( m_curState.psShaderResourceViews[nIndex] != m_oldState.psShaderResourceViews[nIndex] )
	{
		ID3D11ShaderResourceView* pSRV = m_curState.psShaderResourceViews[nIndex];
		pDeviceContext->PSSetShaderResources( nIndex, 1, &pSRV );
		m_oldState.psShaderResourceViews[nIndex] = m_curState.psShaderResourceViews[nIndex];
	}

	SShaderResourceBoundState& oldState = m_oldState.shaderResourceBoundStates[2][nIndex];
	SShaderResourceBoundState& curState = m_curState.shaderResourceBoundStates[2][nIndex];
	oldState.SetShaderResource( curState.pShaderResource );
}

void CDeviceStateMgr::CommitVSSampler( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStateVSSampler;
	if( m_curState.vsSamplerStates[nIndex] == m_oldState.vsSamplerStates[nIndex] )
		return;
	ID3D11SamplerState* pState = m_curState.vsSamplerStates[nIndex];
	pDeviceContext->VSSetSamplers( nIndex, 1, &pState );
	m_oldState.vsSamplerStates[nIndex] = m_curState.vsSamplerStates[nIndex];
}

void CDeviceStateMgr::CommitGSSampler( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStateGSSampler;
	if( m_curState.gsSamplerStates[nIndex] == m_oldState.gsSamplerStates[nIndex] )
		return;
	ID3D11SamplerState* pState = m_curState.gsSamplerStates[nIndex];
	pDeviceContext->GSSetSamplers( nIndex, 1, &pState );
	m_oldState.gsSamplerStates[nIndex] = m_curState.gsSamplerStates[nIndex];
}

void CDeviceStateMgr::CommitPSSampler( ID3D11DeviceContext* pDeviceContext, uint32 nState )
{
	uint32 nIndex = nState - eDeviceStatePSSampler;
	if( m_curState.psSamplerStates[nIndex] == m_oldState.psSamplerStates[nIndex] )
		return;
	ID3D11SamplerState* pState = m_curState.psSamplerStates[nIndex];
	pDeviceContext->PSSetSamplers( nIndex, 1, &pState );
	m_oldState.psSamplerStates[nIndex] = m_curState.psSamplerStates[nIndex];
}
