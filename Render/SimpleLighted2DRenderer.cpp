#include "stdafx.h"
#include "SimpleLighted2DRenderer.h"
#include "GlobalRenderResources.h"
#include "Scene2DManager.h"
#include "PostProcess.h"

void CSimpleLighted2DRenderer::OnCreateDevice( IRenderSystem* pSystem )
{
	if( !m_bIsSubRenderer )
	{
		InitEngine();
		CGlobalRenderResources::Inst()->Init( pSystem );
	}
}

void CSimpleLighted2DRenderer::OnResize( IRenderSystem* pSystem, const CVector2& size )
{
	m_screenRes = size;
	if( !m_bIsSubRenderer )
		CRenderTargetPool::GetSizeDependentPool().Clear();
	else
	{
		ReleaseSubRendererTexture();
		m_sizeDependentPool.Clear();
	}
}

void CSimpleLighted2DRenderer::OnDestroyDevice( IRenderSystem* pSystem )
{
	if( m_bIsSubRenderer )
	{
		ReleaseSubRendererTexture();
		m_sizeDependentPool.Clear();
	}
}

void CSimpleLighted2DRenderer::OnUpdate( IRenderSystem* pSystem )
{
	double dLastTime = pSystem->GetLastTime();
	double dTotalTime = pSystem->GetTotalTime();
	const uint32 nFPS = 60;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );
	m_nUpdateFrames = nFrames;
}

void CSimpleLighted2DRenderer::OnRender( IRenderSystem* pSystem )
{
	CRenderContext2D context;
	context.pRenderSystem = pSystem;
	context.screenRes = m_screenRes;
	context.lightMapRes = CVector2( 0, 0 );
	context.dTime = pSystem->GetElapsedTime();
	context.nTimeStamp = m_nTimeStamp;
	m_nTimeStamp += m_nUpdateFrames;
	context.nFixedUpdateCount = m_nUpdateFrames;
	context.bInverseY = false;

	pSystem->SetPrimitiveType( EPrimitiveType::TriangleList );

	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	if( !m_bIsSubRenderer )
	{
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

	sizeDependentPool.AllocRenderTarget( m_pColorBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	sizeDependentPool.AllocRenderTarget( m_pEmissionBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR16G16B16A16Float, NULL, false, true );
	sizeDependentPool.AllocRenderTarget( m_pNormBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	if( m_bIsSubRenderer )
		sizeDependentPool.AllocRenderTarget( m_pDepthStencil, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatD32FloatS8X24UInt, NULL, false, false, true );
	RenderColorBuffer( context );

	sizeDependentPool.AllocRenderTarget( m_pLightAccumulationBuffer, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR16G16B16A16Float, NULL, false, true );
	RenderLights( context );

	//Post process
	auto pPostProcessPass = CPostProcessPass::GetPostProcessPass( ePostProcessPass_PreGUI );

	class CPostProcessRenderScene : public CPostProcess
	{
	public:
		CPostProcessRenderScene( CSimpleLighted2DRenderer* pRenderer, IRenderSystem* pSystem )
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

			m_pRenderer->RenderScene( m_pSystem, pTarget );
			sizeDependentPool.Release( m_pRenderer->m_pColorBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pEmissionBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pNormBuffer );
			sizeDependentPool.Release( m_pRenderer->m_pLightAccumulationBuffer );
		}
		virtual bool IsForceFirstPass() override { return true; }
	private:
		CSimpleLighted2DRenderer* m_pRenderer;
		IRenderSystem* m_pSystem;
	};

	if( m_bIsSubRenderer )
		sizeDependentPool.AllocRenderTarget( m_pSubRendererTexture, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
	CPostProcessRenderScene renderScenePass( this, pSystem );
	CReference<ITexture> pTempTarget;
	pPostProcessPass->Register( &renderScenePass );
	pPostProcessPass->SetRenderTargetPool( &sizeDependentPool );
	pPostProcessPass->SetDepthStencil( m_bIsSubRenderer ? m_pDepthStencil->GetDepthStencil() : pSystem->GetDefaultDepthStencil() );
	pPostProcessPass->Process( pSystem, pTempTarget, m_pSubRendererTexture ? m_pSubRendererTexture->GetRenderTarget() : pSystem->GetDefaultRenderTarget() );
	sizeDependentPool.Release( pTempTarget );

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

void CSimpleLighted2DRenderer::FetchSubRendererTexture( ITexture** ppTex )
{
	*ppTex = m_pSubRendererTexture;
	( *ppTex )->AddRef();
	m_pSubRendererTexture = NULL;
}

void CSimpleLighted2DRenderer::RenderColorBuffer( CRenderContext2D& context )
{
	context.eRenderPass = eRenderPass_Color;
	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	if( m_bIsSubRenderer )
	{
		pSceneMgr->Render( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
		if( m_subRendererContext.pGUICamera )
		{
			pSceneMgr->Render( context, m_subRendererContext.pGUICamera, m_subRendererContext.pGUIRoot, m_renderGroup );
		}
	}
	else
	{
		pSceneMgr->Render( context );
	}

	IRenderSystem* pSystem = context.pRenderSystem;
	IRenderTarget* pTargets[3] = { m_pColorBuffer->GetRenderTarget(), m_pEmissionBuffer->GetRenderTarget(), m_pNormBuffer->GetRenderTarget() };
	pSystem->SetRenderTargets( pTargets, 3, m_bIsSubRenderer ? m_pDepthStencil->GetDepthStencil() : pSystem->GetDefaultDepthStencil() );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pTargets[0] );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pTargets[1] );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pTargets[2] );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	if( m_bIsSubRenderer )
		pSceneMgr->Flush( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
	else
		pSceneMgr->Flush( context );
}

void CSimpleLighted2DRenderer::RenderLights( CRenderContext2D& context )
{
	context.eRenderPass = eRenderPass_Light;
	context.pRenderSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), m_pLightAccumulationBuffer->GetRenderTarget() );

	SDirectionalLight2D* pDirectionalLight;
	while( ( pDirectionalLight = context.Get_DirectionalLight() ) != NULL )
	{
		RenderLightDirectional( context, *pDirectionalLight );
		pDirectionalLight->RemoveFrom_DirectionalLight();
	}
	SPointLight2D* pPointLight;
	while( ( pPointLight = context.Get_PointLight() ) != NULL )
	{
		RenderLightPoint( context, *pPointLight );
		pPointLight->RemoveFrom_PointLight();
	}
}

void CSimpleLighted2DRenderer::RenderGUI( CRenderContext2D& context )
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
		pSceneMgr->Flush( context, m_subRendererContext.pGUICamera ? m_subRendererContext.pGUICamera : m_subRendererContext.pCamera,
			m_subRendererContext.pGUIRoot ? m_subRendererContext.pGUIRoot : m_subRendererContext.pRoot, m_renderGroup );
	else
		pSceneMgr->Flush( context );
}