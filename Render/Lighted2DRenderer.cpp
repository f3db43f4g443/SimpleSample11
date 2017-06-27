#include "stdafx.h"
#include "Lighted2DRenderer.h"
#include "RenderSystem.h"
#include "GlobalRenderResources.h"
#include "Texture.h"
#include "DrawableGroup.h"
#include "Font.h"
#include "Prefab.h"
#include "CommonShader.h"
#include "ResourceManager.h"
#include "Scene2DManager.h"
#include "PostProcess.h"

void CLighted2DRenderer::OnCreateDevice( IRenderSystem* pSystem )
{
	if( !m_bIsSubRenderer )
	{
		InitEngine();
		CGlobalRenderResources::Inst()->Init( pSystem );
	}

	m_lightMapRes = CVector2( 2048, 2048 );
}

void CLighted2DRenderer::OnResize( IRenderSystem* pSystem, const CVector2& size )
{
	m_screenRes = size;
	if( !m_bIsSubRenderer )
	{
		CRenderTargetPool::GetSizeDependentPool().Release( m_pTransmissionBuffer );
		CRenderTargetPool::GetSizeDependentPool().Clear();
	}
	else
	{
		m_sizeDependentPool.Release( m_pTransmissionBuffer );
		ReleaseSubRendererTexture();
		m_sizeDependentPool.Clear();
	}
}

void CLighted2DRenderer::OnDestroyDevice( IRenderSystem* pSystem )
{
	if( m_bIsSubRenderer )
	{
		ReleaseSubRendererTexture();
		m_sizeDependentPool.Clear();
	}
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

	pSystem->SetPrimitiveType( EPrimitiveType::TriangleList );
	
	if( !m_bIsSubRenderer )
	{
		CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
		pSceneMgr->UpdateDirty();
	
		for( auto pFootprintMgr = pSceneMgr->Get_FootprintMgr(); pFootprintMgr; pFootprintMgr = pFootprintMgr->NextFootprintMgr() )
		{
			pFootprintMgr->Update( context.dTime, pSystem );
		}
		pSceneMgr->UpdateDirty();
		pSceneMgr->Trigger( CScene2DManager::eEvent_BeforeRender, &context );
	}

	auto& sizeDependentPool = m_bIsSubRenderer ? m_sizeDependentPool : CRenderTargetPool::GetSizeDependentPool();
	auto& sizeIndependentPool = CRenderTargetPool::GetSizeIndependentPool();
	
	//Base pass
	sizeDependentPool.AllocRenderTarget( m_pColorBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	sizeDependentPool.AllocRenderTarget( m_pEmissionBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	if( m_bIsSubRenderer )
		sizeDependentPool.AllocRenderTarget( m_pDepthStencil, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatD32FloatS8X24UInt, NULL, false, false, true );
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

	//Post process
	auto pPostProcessPass = CPostProcessPass::GetPostProcessPass( ePostProcessPass_PreGUI );

	class CPostProcessRenderScene : public CPostProcess
	{
	public:
		CPostProcessRenderScene( CLighted2DRenderer* pRenderer, IRenderSystem* pSystem )
			: m_pRenderer( pRenderer ), m_pSystem( pSystem ) {}
		virtual void Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget ) override
		{
			auto& sizeDependentPool = pPass->GetRenderTargetPool();
			IRenderTarget* pTarget = pFinalTarget;
			if( !pTarget )
			{
				sizeDependentPool.AllocRenderTarget( pPass->GetTarget(), ETextureType::Tex2D,
					m_pRenderer->m_screenRes.x, m_pRenderer->m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
				pTarget = pPass->GetTarget()->GetRenderTarget();
			}

			if( !m_pRenderer->m_pTransmissionBuffer )
			{
				sizeDependentPool.AllocRenderTarget( m_pRenderer->m_pTransmissionBuffer, ETextureType::Tex2D,
					m_pRenderer->m_screenRes.x, m_pRenderer->m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
				m_pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), m_pRenderer->m_pTransmissionBuffer->GetRenderTarget() );
				m_pRenderer->m_cameraOfs = CVector2( 0, 0 );
			}
			else
				m_pRenderer->m_cameraOfs = m_pRenderer->m_curCameraPos - m_pRenderer->m_lastCameraPos;
			m_pRenderer->m_lastCameraPos = m_pRenderer->m_curCameraPos;

			sizeDependentPool.AllocRenderTarget( m_pRenderer->m_pTransmissionBuffer1, ETextureType::Tex2D,
				m_pRenderer->m_screenRes.x, m_pRenderer->m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
			sizeDependentPool.AllocRenderTarget( m_pRenderer->m_pTransmissionBufferTemp, ETextureType::Tex2D,
				m_pRenderer->m_screenRes.x, m_pRenderer->m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
			
			m_pRenderer->RenderScene( m_pSystem, pTarget );
			sizeDependentPool.Release( m_pRenderer->m_pColorBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pEmissionBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pLightAccumulationBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pTransmissionBuffer1 );
			sizeDependentPool.Release( m_pRenderer->m_pTransmissionBufferTemp );
		}
		virtual bool IsForceFirstPass() override { return true; }
	private:
		CLighted2DRenderer* m_pRenderer;
		IRenderSystem* m_pSystem;
	};

	if( m_bIsSubRenderer )
		sizeDependentPool.AllocRenderTarget( m_pSubRendererTexture, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	CPostProcessRenderScene renderScenePass( this, pSystem );
	CReference<ITexture> pTempTarget;
	pPostProcessPass->Register( &renderScenePass );
	pPostProcessPass->SetRenderTargetPool( &sizeDependentPool );
	pPostProcessPass->Process( pSystem, pTempTarget, m_pSubRendererTexture ? m_pSubRendererTexture->GetRenderTarget() : pSystem->GetDefaultRenderTarget() );
	sizeDependentPool.Release( pTempTarget );
	sizeIndependentPool.Release( m_pOcclusionBuffer );

	RenderGUI( context );
	if( m_bIsSubRenderer )
		sizeDependentPool.Release( m_pDepthStencil );

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
	if( m_bIsSubRenderer )
	{
		pSceneMgr->Render( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
		m_curCameraPos = m_subRendererContext.pCamera->GetViewArea().GetCenter();
	}
	else
	{
		pSceneMgr->Render( context );
		m_curCameraPos = pSceneMgr->GetCamera()->GetViewArea().GetCenter();
	}

	IRenderSystem* pSystem = context.pRenderSystem;
	IRenderTarget* pTargets[2] = { m_pColorBuffer->GetRenderTarget(), m_pEmissionBuffer->GetRenderTarget() };
	pSystem->SetRenderTargets( pTargets, 2, m_bIsSubRenderer ? m_pDepthStencil->GetDepthStencil() : pSystem->GetDefaultDepthStencil() );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pTargets[0] );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pTargets[1] );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );
	
	if( m_bIsSubRenderer )
		pSceneMgr->Flush( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
	else
		pSceneMgr->Flush( context );
}

void CLighted2DRenderer::RenderOcclusionBuffer( CRenderContext2D& context )
{
	context.eRenderPass = eRenderPass_Occlusion;
	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	if( m_bIsSubRenderer )
		pSceneMgr->Render( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
	else
		pSceneMgr->Render( context );

	IRenderSystem* pSystem = context.pRenderSystem;
	pSystem->SetRenderTarget( m_pOcclusionBuffer->GetRenderTarget(), m_pDepthStencilHighRes->GetDepthStencil() );
	pSystem->ClearRenderTarget( CVector4( 1, 1, 1, 0 ) );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_lightMapRes.x, m_lightMapRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	if( m_bIsSubRenderer )
		pSceneMgr->Flush( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
	else
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
	pSystem->SetRenderTarget( m_pSubRendererTexture ? m_pSubRendererTexture->GetRenderTarget() : pSystem->GetDefaultRenderTarget(),
		m_bIsSubRenderer ? m_pDepthStencil->GetDepthStencil() : pSystem->GetDefaultDepthStencil() );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	if( m_bIsSubRenderer )
		pSceneMgr->Flush( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
	else
		pSceneMgr->Flush( context );
}

void IRenderer::DebugDrawLine( IRenderSystem* pSystem, const CVector2& pt1, const CVector2& pt2, const CVector4& color )
{
	pSystem->SetPrimitiveType( EPrimitiveType::LineList );
	
	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	CVector2* pVertexData;
	pSystem->Lock( CGlobalRenderResources::Inst()->GetVBDebug(), (void**)&pVertexData );
	pVertexData[0] = pt1;
	pVertexData[1] = pt2;
	pSystem->Unlock( CGlobalRenderResources::Inst()->GetVBDebug() );
	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBDebug() );
	
	auto pVertexShader = CDebugDrawShader::Inst();
	auto pPixelShader = COneColorPixelShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );
	pPixelShader->SetParams( pSystem, color );

	pSystem->Draw( 2, 0 );

	pSystem->SetPrimitiveType( EPrimitiveType::TriangleList );
}

void IRenderer::DebugDrawTriangle( IRenderSystem* pSystem, const CVector2& pt1, const CVector2& pt2, const CVector2& pt3, const CVector4& color )
{
	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	CVector2* pVertexData;
	pSystem->Lock( CGlobalRenderResources::Inst()->GetVBDebug(), (void**)&pVertexData );
	pVertexData[0] = pt1;
	pVertexData[1] = pt2;
	pVertexData[2] = pt3;
	pSystem->Unlock( CGlobalRenderResources::Inst()->GetVBDebug() );
	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBDebug() );
	
	auto pVertexShader = CDebugDrawShader::Inst();
	auto pPixelShader = COneColorPixelShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );
	pPixelShader->SetParams( pSystem, color );

	pSystem->Draw( 3, 0 );
}