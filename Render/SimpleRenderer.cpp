#include "stdafx.h"
#include "SimpleRenderer.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Font.h"
#include "Prefab.h"
#include "GlobalRenderResources.h"
#include "RenderSystem.h"
#include "RenderContext2D.h"
#include "Scene2DManager.h"
#include "CommonShader.h"
#include "DrawableGroup.h"

void CSimpleRenderer::OnCreateDevice( IRenderSystem* pSystem )
{
	if( !m_bIsSubRenderer )
	{
		InitEngine();
		CGlobalRenderResources::Inst()->Init( pSystem );
	}
}

void CSimpleRenderer::OnResize( IRenderSystem* pSystem, const CVector2& size )
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

void CSimpleRenderer::OnDestroyDevice( IRenderSystem* pSystem )
{
	if( m_bIsSubRenderer )
	{
		ReleaseSubRendererTexture();
		m_sizeDependentPool.Clear();
	}
}

void CSimpleRenderer::OnUpdate( IRenderSystem* pSystem )
{
	double dLastTime = pSystem->GetLastTime();
	double dTotalTime = pSystem->GetTotalTime();
	const uint32 nFPS = 60;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );
	m_nUpdateFrames = nFrames;
	
	for( auto pObj = CScene2DManager::GetGlobalInst()->Get_AutoUpdateAnimObject(); pObj; pObj = pObj->NextAutoUpdateAnimObject() )
	{
		pObj->UpdateAnim( dTotalTime - dLastTime );
	}
}

void CSimpleRenderer::OnRender( IRenderSystem* pSystem )
{
	CRenderContext2D context;
	context.pRenderSystem = pSystem;
	context.screenRes = m_screenRes;
	context.lightMapRes = CVector2( 0, 0 );
	context.dTime = pSystem->GetElapsedTime();
	context.nTimeStamp = m_nTimeStamp;
	m_nTimeStamp += m_nUpdateFrames;
	context.nFixedUpdateCount = m_nUpdateFrames;
	context.bInverseY = m_subRendererContext.bInvertY;

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
		pSceneMgr->Trigger( CScene2DManager::eEvent_BeforeRender );
	}
	
	context.eRenderPass = eRenderPass_Color;
	if( m_bIsSubRenderer )
		pSceneMgr->Render( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
	else
		pSceneMgr->Render( context );
	
	auto& sizeDependentPool = m_bIsSubRenderer ? m_sizeDependentPool : CRenderTargetPool::GetSizeDependentPool();
	auto& sizeIndependentPool = CRenderTargetPool::GetSizeIndependentPool();

	if( m_bIsSubRenderer )
	{
		sizeDependentPool.AllocRenderTarget( m_pSubRendererTexture, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
		sizeDependentPool.AllocRenderTarget( m_pDepthStencil, ETextureType::Tex2D, m_screenRes.x, m_screenRes.y, 1, 1, EFormat::EFormatD32FloatS8X24UInt, NULL, false, false, true );
	}

	pSystem->SetRenderTarget( m_bIsSubRenderer ? m_pSubRendererTexture->GetRenderTarget() : pSystem->GetDefaultRenderTarget(),
		m_bIsSubRenderer ? m_pDepthStencil->GetDepthStencil() : pSystem->GetDefaultDepthStencil() );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ) );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );
	
	if( m_bIsSubRenderer )
		pSceneMgr->Flush( context, m_subRendererContext.pCamera, m_subRendererContext.pRoot, m_renderGroup );
	else
		pSceneMgr->Flush( context );

	if( m_bIsSubRenderer )
		sizeDependentPool.Release( m_pDepthStencil );

	while( context.pUpdatedObjects )
	{
		context.pUpdatedObjects->SetUpdated( false );
		context.pUpdatedObjects->RemoveFrom_UpdatedObject();
	}
	m_nUpdateFrames = 0;
}