#include "stdafx.h"
#include "Lighted2DRenderer.h"
#include "RenderSystem.h"
#include "GlobalRenderResources.h"
#include "Texture.h"
#include "Font.h"
#include "CommonShader.h"
#include "ResourceManager.h"
#include "Scene2DManager.h"
#include "PostProcess.h"

void CLighted2DRenderer::OnCreateDevice( IRenderSystem* pSystem )
{
	CResourceManager::Inst()->Register( new TResourceFactory<CTextureFile>() );
	CResourceManager::Inst()->Register( new TResourceFactory<CFontFile>() );
	CTextureFile::InitLoader();
	CFontFile::Init();

	CGlobalRenderResources::Inst()->Init( pSystem );

	m_lightMapRes = CVector2( 2048, 2048 );
}

void CLighted2DRenderer::OnResize( IRenderSystem* pSystem, const CVector2& size )
{
	m_screenRes = size;
	CRenderTargetPool::GetSizeDependentPool().Clear();
}

void CLighted2DRenderer::OnDestroyDevice( IRenderSystem* pSystem )
{

}

void CLighted2DRenderer::OnUpdate( IRenderSystem* pSystem )
{
	double dLastTime = pSystem->GetLastTime();
	double dTotalTime = pSystem->GetTotalTime();
	const uint32 nFPS = 60;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );
	m_nUpdateFrames = nFrames;
}

void CLighted2DRenderer::OnRender( IRenderSystem* pSystem )
{
	CRenderContext2D context;
	context.pRenderSystem = pSystem;
	context.screenRes = m_screenRes;
	context.lightMapRes = m_lightMapRes;
	context.dTime = pSystem->GetElapsedTime();
	context.nTimeStamp = m_nTimeStamp;
	m_nTimeStamp += m_nUpdateFrames;
	context.nFixedUpdateCount = m_nUpdateFrames;

	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->UpdateDirty();
	
	for( auto pFootprintMgr = pSceneMgr->Get_FootprintMgr(); pFootprintMgr; pFootprintMgr = pFootprintMgr->NextFootprintMgr() )
	{
		pFootprintMgr->Update( context.dTime, pSystem );
	}
	pSceneMgr->UpdateDirty();

	pSystem->SetPrimitiveType( EPrimitiveType::TriangleList );

	auto& sizeDependentPool = CRenderTargetPool::GetSizeDependentPool();
	auto& sizeIndependentPool = CRenderTargetPool::GetSizeIndependentPool();
	
	//Base pass
	sizeDependentPool.AllocRenderTarget( m_pColorBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	sizeDependentPool.AllocRenderTarget( m_pEmissionBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	RenderColorBuffer( context );

	sizeIndependentPool.AllocRenderTarget( m_pOcclusionBuffer, ETextureType::Tex2D, m_lightMapRes.x, m_lightMapRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	sizeIndependentPool.AllocRenderTarget( m_pDepthStencilHighRes, ETextureType::Tex2D, m_lightMapRes.x, m_lightMapRes.y, 1, 1, EFormat::EFormatD32FloatS8X24UInt, NULL, false, false, true );
	RenderOcclusionBuffer( context );
	sizeIndependentPool.Release( m_pDepthStencilHighRes );

	//Lights
	sizeDependentPool.AllocRenderTarget( m_pLightAccumulationBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR16G16B16A16Float, NULL, false, true );
	for( int i = 0; i < 2; i++ )
	{
		sizeIndependentPool.AllocRenderTarget( m_pShadowBuffer[i], ETextureType::Tex2D, m_lightMapRes.x, m_lightMapRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	}
	RenderLights( context );
	for( int i = 0; i < 2; i++ )
	{
		sizeIndependentPool.Release( m_pShadowBuffer[i] );
	}
	sizeIndependentPool.Release( m_pOcclusionBuffer );

	//Post process
	auto pPostProcessPass = CPostProcessPass::GetPostProcessPass( ePostProcessPass_PreGUI );

	class CPostProcessRenderScene : public CPostProcess
	{
	public:
		CPostProcessRenderScene( CLighted2DRenderer* pRenderer, IRenderSystem* pSystem )
			: m_pRenderer( pRenderer ), m_pSystem( pSystem ) {}
		virtual void Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget ) override
		{
			auto& sizeDependentPool = CRenderTargetPool::GetSizeDependentPool();
			IRenderTarget* pTarget = pFinalTarget;
			if( !pTarget )
			{
				sizeDependentPool.AllocRenderTarget( pPass->GetTarget(), ETextureType::Tex2D,
					m_pRenderer->m_screenRes.x, m_pRenderer->m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
				pTarget = pPass->GetTarget()->GetRenderTarget();
			}
			
			m_pRenderer->RenderScene( m_pSystem, pTarget );
			sizeDependentPool.Release( m_pRenderer->m_pColorBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pEmissionBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pLightAccumulationBuffer );
		}
		virtual bool IsForceFirstPass() override { return true; }
	private:
		CLighted2DRenderer* m_pRenderer;
		IRenderSystem* m_pSystem;
	};

	CPostProcessRenderScene renderScenePass( this, pSystem );
	CReference<ITexture> pTempTarget;
	pPostProcessPass->Register( &renderScenePass );
	pPostProcessPass->Process( pSystem, pTempTarget, pSystem->GetDefaultRenderTarget() );
	sizeDependentPool.Release( pTempTarget );

	RenderGUI( context );

	while( context.pUpdatedObjects )
	{
		context.pUpdatedObjects->SetUpdated( false );
		context.pUpdatedObjects->RemoveFrom_UpdatedObject();
	}
	m_nUpdateFrames = 0;
}

void CLighted2DRenderer::RenderColorBuffer( CRenderContext2D& context )
{
	context.eRenderPass = eRenderPass_Color;
	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->Render( context );

	IRenderSystem* pSystem = context.pRenderSystem;
	IRenderTarget* pTargets[2] = { m_pColorBuffer->GetRenderTarget(), m_pEmissionBuffer->GetRenderTarget() };
	pSystem->SetRenderTargets( pTargets, 2, pSystem->GetDefaultDepthStencil() );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pTargets[0] );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pTargets[1] );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSceneMgr->Flush( context );
}

void CLighted2DRenderer::RenderOcclusionBuffer( CRenderContext2D& context )
{
	context.eRenderPass = eRenderPass_Occlusion;
	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->Render( context );

	IRenderSystem* pSystem = context.pRenderSystem;
	pSystem->SetRenderTarget( m_pOcclusionBuffer->GetRenderTarget(), m_pDepthStencilHighRes->GetDepthStencil() );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 1 ) );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_lightMapRes.x, m_lightMapRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSceneMgr->Flush( context );
}

void CLighted2DRenderer::RenderLights( CRenderContext2D& context )
{
	context.eRenderPass = eRenderPass_Light;
	context.pRenderSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), m_pLightAccumulationBuffer->GetRenderTarget() );
	
	SDirectionalLight2D* pDirectionalLight;
	while( ( pDirectionalLight = context.Get_DirectionalLight() ) != NULL )
	{
		RenderLightDirectional( context.pRenderSystem, *pDirectionalLight );
		pDirectionalLight->RemoveFrom_DirectionalLight();
	}

	RenderLightPointInstancing( context.pRenderSystem, context.Get_PointLight() );
}

void CLighted2DRenderer::RenderGUI( CRenderContext2D& context )
{
	context.eRenderPass = eRenderPass_GUI;
	
	IRenderSystem* pSystem = context.pRenderSystem;
	pSystem->SetRenderTarget( pSystem->GetDefaultRenderTarget(), pSystem->GetDefaultDepthStencil() );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->Flush( context );
}

class CLightScenePixelShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLightScenePixelShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_tex0, "ColorMap" );
		GetShader()->GetShaderInfo().Bind( m_tex1, "EmissionMap" );
		GetShader()->GetShaderInfo().Bind( m_tex2, "LightMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pColorMap, IShaderResource* pEmissionMap, IShaderResource* pLightMap )
	{
		m_tex0.Set( pRenderSystem, pColorMap );
		m_tex1.Set( pRenderSystem, pEmissionMap );
		m_tex2.Set( pRenderSystem, pLightMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParamShaderResource m_tex0;
	CShaderParamShaderResource m_tex1;
	CShaderParamShaderResource m_tex2;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CLightScenePixelShader, "Shader/Light2D.shader", "PSScene", "ps_5_0" );

void CLighted2DRenderer::RenderScene( IRenderSystem* pSystem, IRenderTarget* pTarget )
{
	pSystem->SetRenderTarget( pTarget, NULL );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CLightScenePixelShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle dstRect( 0, 0, m_screenRes.x, m_screenRes.y );
	CRectangle srcRect( 0, 0, m_screenRes.x, m_screenRes.y );

	pVertexShader->SetParams( pSystem, dstRect, srcRect, dstRect.GetSize(), srcRect.GetSize() );
	pPixelShader->SetParams( pSystem, m_pColorBuffer->GetShaderResource(), m_pEmissionBuffer->GetShaderResource(), m_pLightAccumulationBuffer->GetShaderResource() );

	pSystem->DrawInput();
}