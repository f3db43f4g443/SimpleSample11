#include "stdafx.h"
#include "EditorRenderer.h"
#include "Common/ResourceManager.h"
#include "Render/Texture.h"
#include "Render/Font.h"
#include "Render/GlobalRenderResources.h"
#include "Render/RenderSystem.h"
#include "Render/RenderContext2D.h"
#include "Render/Scene2DManager.h"
#include "Render/CommonShader.h"

void CEditorRenderer::OnCreateDevice( IRenderSystem* pSystem )
{
	CResourceManager::Inst()->Register( new TResourceFactory<CTextureFile>() );
	CResourceManager::Inst()->Register( new TResourceFactory<CFontFile>() );
	CTextureFile::InitLoader();
	CFontFile::Init();

	CScreenVertexShader::Inst();

	CGlobalRenderResources::Inst()->Init( pSystem );
}

void CEditorRenderer::OnResize( IRenderSystem* pSystem, const CVector2& size )
{
	m_screenRes = size;
	CRenderTargetPool::GetSizeDependentPool().Clear();
}

void CEditorRenderer::OnUpdate( IRenderSystem* pSystem )
{
	double dLastTime = pSystem->GetLastTime();
	double dTotalTime = pSystem->GetTotalTime();
	const uint32 nFPS = 60;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );
	m_nUpdateFrames = nFrames;
}

void CEditorRenderer::OnRender( IRenderSystem* pSystem )
{
	CRenderContext2D context;
	context.pRenderSystem = pSystem;
	context.screenRes = m_screenRes;
	context.lightMapRes = CVector2( 0, 0 );
	context.dTime = pSystem->GetElapsedTime();
	context.nTimeStamp = m_nTimeStamp;
	m_nTimeStamp += m_nUpdateFrames;
	context.nFixedUpdateCount = m_nUpdateFrames;
	context.bInverseY = true;

	CScene2DManager* pSceneMgr = CScene2DManager::GetGlobalInst();
	pSceneMgr->UpdateDirty();
	
	for( auto pFootprintMgr = pSceneMgr->Get_FootprintMgr(); pFootprintMgr; pFootprintMgr = pFootprintMgr->NextFootprintMgr() )
	{
		pFootprintMgr->Update( context.dTime, pSystem );
	}
	pSceneMgr->UpdateDirty();

	pSystem->SetPrimitiveType( EPrimitiveType::TriangleList );
	
	context.eRenderPass = eRenderPass_Color;
	pSceneMgr->Render( context );

	pSystem->SetRenderTarget( pSystem->GetDefaultRenderTarget(), pSystem->GetDefaultDepthStencil() );
	pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ) );
	pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSceneMgr->Flush( context );

	while( context.pUpdatedObjects )
	{
		context.pUpdatedObjects->SetUpdated( false );
		context.pUpdatedObjects->RemoveFrom_UpdatedObject();
	}
	m_nUpdateFrames = 0;
}

IMPLEMENT_MATERIAL_SHADER( Default2DUIVertexShader, "Shader/Default2DUI.shader", "VSDefault", "vs_5_0" );