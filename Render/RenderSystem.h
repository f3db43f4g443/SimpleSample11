#pragma once

#include "Math3D.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Sound.h"
#include "RenderState.h"
#include "BufFile.h"

struct SViewport
{
	float fX;
	float fY;
	float fWidth;
	float fHeight;
	float fMinDepth;
	float fMaxDepth;
};

struct SDeviceCreateContext
{
	CVector2 resolution;
};

class IGame;
class IRenderer;
class IRenderSystem
{
public:
	IRenderSystem();
	virtual void CreateDevice( const SDeviceCreateContext& context ) = 0;
	virtual void Start() = 0;

	virtual IVertexBuffer* CreateVertexBuffer( uint32 nElements, const SVertexBufferElement* pElements, uint32 nVertices, void* pData, bool bIsDynamic = false,
		bool bBindShaderResource = false, bool bBindStreamOutput = false, bool bIsInstance = false ) = 0;
	virtual IIndexBuffer* CreateIndexBuffer( uint32 nIndices, EFormat eFormat, void* pData, bool bIsDynamic = false, bool bBindStreamOutput = false ) = 0;
	virtual ITexture* CreateTexture( ETextureType eType, uint32 nDim1, uint32 nDim2, uint32 nDim3, uint32 nMipLevels, EFormat eFormat, void* data,
		bool bIsDynamic = false, bool bBindRenderTarget = false, bool bBindDepthStencil = false ) = 0;
	virtual IConstantBuffer* CreateConstantBuffer( uint32 nSize, bool bUsePool ) = 0;

	virtual ISound* CreateSound( const void* pBuffer, uint32 nSize, const SWaveFormat& format ) = 0;
	virtual ISoundTrack* CreateSoundTrack( ISound* pSound ) = 0;

	virtual IRenderTarget* GetDefaultRenderTarget() = 0;
	virtual IDepthStencil* GetDefaultDepthStencil() = 0;
	
	virtual IShader* LoadShader( IBufReader& buf, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc = NULL, uint32 nVertexBuffers = 0, uint32 nRasterizedStream = 0 ) = 0;

	virtual void GetRenderTargetTextures( CReference<ITexture>* ppTextures, uint32& nTextures, uint32 nArraySize ) = 0;

	virtual void SetPrimitiveType( EPrimitiveType ePrimitiveType ) = 0;
	virtual void SetBlendState( IBlendState* pState, const CVector4& blendFactor = CVector4( 0, 0, 0, 0 ), uint32 nSampleMask = 0xffffffff ) = 0;
	virtual void SetDepthStencilState( IDepthStencilState* pState, uint32 nStencilRef = 0 ) = 0;
	virtual void SetRasterizerState( IRasterizerState* pState ) = 0;

	virtual void SetRenderTargets( IRenderTarget** ppRenderTargets, uint32 nRenderTargets ) = 0;
	virtual void SetRenderTargets( IRenderTarget** ppRenderTargets, uint32 nRenderTargets, IDepthStencil* pDepthStencil ) = 0;
	void SetRenderTarget( IRenderTarget* pRenderTarget, IDepthStencil* pDepthStencil )
	{
		if( pRenderTarget )
			SetRenderTargets( &pRenderTarget, 1, pDepthStencil );
		else
			SetRenderTargets( NULL, 0, pDepthStencil );
	}
	virtual void SetSOTargets( IStreamOutput** ppSOTargets, uint32* pSOOffsets, uint32 nSOTargets ) = 0;
	virtual void SetViewports( SViewport* pViewports, uint32 nViewports ) = 0;

	virtual void SetVertexBuffer( uint32 nIndex, IVertexBuffer* pVertexBuffer ) = 0;
	virtual void SetIndexBuffer( IIndexBuffer* pIndexBuffer ) = 0;

	virtual void SetShaderBoundState( IShaderBoundState* &pShaderBoundState, IShader* pVertexShader, IShader* pPixelShader, const CVertexBufferDesc** ppVertexBufferDesc, uint32 nVertexBuffers, IShader* pGeometryShader = NULL, bool bSet = true ) = 0;

	virtual void SetConstantBuffer( EShaderType eShaderType, uint32 nIndex, IConstantBuffer* pConstantBuffer ) = 0;
	virtual IConstantBuffer* GetConstantBuffer( EShaderType eShaderType, uint32 nIndex ) = 0;
	virtual void SetShaderResource( EShaderType eShaderType, uint32 nIndex, IShaderResource* pShaderResource ) = 0;
	virtual void SetSampler( EShaderType eShaderType, uint32 nIndex, const ISamplerState* pSampler ) = 0;

	virtual void ClearRenderTarget( const CVector4& color, IRenderTarget* pRenderTarget = NULL ) = 0;
	virtual void ClearDepthStencil( bool bDepth, float fDepth, bool bStencil, uint8 nStencil, IDepthStencil* pDepthStencil = NULL ) = 0;
	virtual void Draw( uint32 VertexCount, uint32 StartVertexLocation) = 0;
	virtual void DrawAuto() = 0;
	virtual void DrawIndexed( uint32 IndexCount, uint32 StartIndexLocation, int32 BaseVertexLocation ) = 0;
	virtual void DrawInstanced( uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation ) = 0;
	virtual void DrawIndexedInstanced( uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation ) = 0;
	virtual void DrawInput() = 0;
	virtual void DrawInputInstanced( uint32 nInstance ) = 0;

	virtual void CopyResource( ITexture* pDst, ITexture* pSrc ) = 0;
	virtual void UpdateSubResource( ITexture* pDst, void* pData, TVector3<uint32> vMin, TVector3<uint32> vMax, uint32 nRowPitch, uint32 nDepthPitch ) = 0;

	virtual void Lock( IVertexBuffer* pVertexBuffer, void** ppData ) = 0;
	virtual void Unlock( IVertexBuffer* pVertexBuffer ) = 0;

	void SetGame( IGame* pGame ) { m_pGame = pGame; }
	IGame* GetGame() { return m_pGame; }
	void SetRenderer( IRenderer* pRenderer ) { m_pRenderer = pRenderer; }
	IRenderer* GetRenderer() { return m_pRenderer; }
	
	double GetLastTime() { return m_dLastTime; }
	double GetTotalTime() { return m_dTime; }
	float GetElapsedTime() { return m_fElapsedTime; }
	void SetTimeScale( float fTimeScale, float a ) { m_fTimeScale = fTimeScale; m_fTimeScaleA = a; }
	const CVector2& GetScreenRes() { return m_screenRes; }
	
	static IRenderSystem* Inst();
protected:
	IGame* m_pGame;
	IRenderer* m_pRenderer;

	double m_dLastTime;
	double m_dTime;
	float m_fElapsedTime;
	float m_fTimeScale;
	float m_fTimeScaleA;
	CVector2 m_screenRes;
};