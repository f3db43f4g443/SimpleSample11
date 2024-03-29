#include "stdafx.h"
#include "UIViewport.h"
#include "UIManager.h"
#include "Render/SimpleRenderer.h"
#include "Render/SimpleLighted2DRenderer.h"
#include "Render/Lighted2DRenderer.h"
#include "Render/Scene2DManager.h"
#include "Render/CommonShader.h"
#include "Render/LightRendering.h"
#include "Common/xml.h"
#include "Common/FileUtil.h"

CUIViewport::CUIViewport()
	: m_pRenderer( NULL )
	, m_pExternalCamera( NULL )
	, m_pExternalGUICamera( NULL )
	, m_texSize( 0, 0 )
	, m_nLightType( 0 )
	, m_bCustomRender( false )
{
	m_elem.SetDrawable( this );
}

CUIViewport::~CUIViewport()
{
	if( m_pRoot )
		m_pRoot->RemoveThis();
	if( m_pRenderer )
	{
		m_pRenderer->OnDestroyDevice( IRenderSystem::Inst() );
		delete m_pRenderer;
	}
}

ITexture* CUIViewport::GetTexture()
{
	if( m_pRenderer )
		return m_pRenderer->GetSubRendererTexture();
	return NULL;
}

void CUIViewport::ReleaseTexture()
{
	if( m_pRenderer )
		m_pRenderer->ReleaseSubRendererTexture();
}

void CUIViewport::ReserveTexSize( const CVector2& size )
{
	CVector2 newSize = CVector2( Max( size.x, m_texSize.x ), Max( size.y, m_texSize.y ) );
	if( newSize == m_texSize )
		return;

	m_texSize = newSize;
	if( m_pRenderer )
		m_pRenderer->OnResize( IRenderSystem::Inst(), m_texSize );
}

void CUIViewport::Render( CRenderContext2D& context )
{
	if( m_pRenderer )
	{
		CRectangle dstRect = m_localBound.Offset( globalTransform.GetPosition() );
		CRectangle srcRect = CRectangle( 0, 0, m_localBound.width, m_localBound.height );
		CRectangle clipRect = dstRect * globalClip;
		if( clipRect.width <= 0 || clipRect.height <= 0 )
			return;

		auto origRect = GetCamera().GetViewArea();
		if( m_nLightType == 2 )
		{
			CVector2 size = m_bCustomRender ? m_customRes : m_texSize;
			GetCamera().SetViewArea( origRect.Offset( CVector2( m_texSize.x - m_localBound.width, m_texSize.y - m_localBound.height ) * 0.5f ) );
			GetCamera().SetSize( size.x, size.y );
		}
		if( m_pExternalGUICamera )
			m_pExternalGUICamera->SetSize( GetCamera().GetViewArea().width, GetCamera().GetViewArea().height );
		auto origViewport = GetCamera().GetViewport();
		if( m_bCustomRender )
			GetCamera().SetViewport( 0, 0, m_customRes.x, m_customRes.y );

		m_pRenderer->OnRender( context.pRenderSystem );

		if( m_bCustomRender )
		{
			GetCamera().SetViewport( origViewport );
			auto pPostProcessPass = &m_customRender;
			if( pPostProcessPass->PreProcess( context.pRenderSystem ) )
			{
				auto& sizeDependentPool = *m_pRenderer->GetRenderTargetPool();
				sizeDependentPool.AllocRenderTarget( m_pCustomTarget, ETextureType::Tex2D,
					m_texSize.x, m_texSize.y, 1, 1, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
				CReference<ITexture> pTempTarget;
				m_pRenderer->FetchSubRendererTexture( pTempTarget.AssignPtr() );
				pPostProcessPass->SetRenderTargetPool( &sizeDependentPool );
				pPostProcessPass->SetFinalViewport( TRectangle<int32>( 0, 0, m_texSize.x, m_texSize.y ) );
				pPostProcessPass->Process( context.pRenderSystem, pTempTarget, m_pCustomTarget->GetRenderTarget() );
				sizeDependentPool.Release( pTempTarget );
			}
		}

		GetCamera().SetViewArea( origRect );
		m_events.Trigger( eEvent_Action, context.pRenderSystem );
		if( GetMgr() )
			context.AddElement( &m_elem );
	}
}

void CUIViewport::Flush( CRenderContext2D& context )
{
	CRectangle dstRect = m_localBound.Offset( globalTransform.GetPosition() );
	CRectangle srcRect = CRectangle( 0, 0, m_localBound.width, m_localBound.height );
	CRectangle clipRect = dstRect * globalClip;
	srcRect.x += clipRect.x - dstRect.x;
	srcRect.y += clipRect.y - dstRect.y;
	srcRect.width += clipRect.width - dstRect.width;
	srcRect.height += clipRect.height - dstRect.height;
	dstRect = clipRect;

	if( m_bCustomRender && m_pCustomTarget )
	{
		CopyToRenderTarget( context.pRenderSystem, NULL, m_pCustomTarget, dstRect, srcRect,
			GetMgr()->GetSize().GetSize(), m_texSize, m_elem.depth * context.mat.m22 + context.mat.m23, m_pBlend );

		auto& sizeDependentPool = *m_pRenderer->GetRenderTargetPool();
		sizeDependentPool.Release( m_pCustomTarget );
	}
	else
	{
		CopyToRenderTarget( context.pRenderSystem, NULL, m_pRenderer->GetSubRendererTexture(), dstRect, srcRect,
			GetMgr()->GetSize().GetSize(), m_texSize, m_elem.depth * context.mat.m22 + context.mat.m23, m_pBlend );
		m_pRenderer->ReleaseSubRendererTexture();
	}
	m_elem.OnFlushed();
}

void CUIViewport::RenderToTexture( CReference<ITexture>& outTex )
{
	CRenderContext2D context;
	context.pRenderSystem = IRenderSystem::Inst();
	Render( context );

	if( m_bCustomRender && m_pCustomTarget )
		outTex = m_pCustomTarget;
	else
		m_pRenderer->FetchSubRendererTexture( outTex.AssignPtr() );
}

void CUIViewport::DebugDrawLine( IRenderSystem* pRenderSystem, const CVector2& begin, const CVector2& end, const CVector4& color, float z )
{
	const CRectangle& rect = GetCamera().GetViewArea();
	CVector2 pt1 = ( begin - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
	CVector2 pt2 = ( end - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
	IRenderer::DebugDrawLine( pRenderSystem, pt1, pt2, color, z,
		IBlendState::Get<false, false, 0xf, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>() );
}

void CUIViewport::DebugDrawTriangles( IRenderSystem* pRenderSystem, uint32 nVert, CVector2* pVert, const CVector4& color, float z )
{
	const CRectangle& rect = GetCamera().GetViewArea();
	for( int i = 0; i + 2 < nVert; i += 3 )
	{
		CVector2 pt1 = ( pVert[i] - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
		CVector2 pt2 = ( pVert[i + 1] - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
		CVector2 pt3 = ( pVert[i + 2] - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
		IRenderer::DebugDrawTriangle( pRenderSystem, pt1, pt2, pt3, color, z,
			IBlendState::Get<false, false, 0xf, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>() );
	}
}

CVector2 CUIViewport::GetScenePos( const CVector2& mousePos )
{
	CRectangle bound = m_localBound.Offset( globalTransform.GetPosition() );
	CRectangle sceneBound = GetCamera().GetOrigViewArea();
	CVector2 vec;
	vec.x = ( mousePos.x - bound.x ) / bound.width * sceneBound.width + sceneBound.x;
	vec.y = ( bound.height - mousePos.y + bound.y ) / bound.height * sceneBound.height + sceneBound.y;
	return vec;
}

void CUIViewport::Set( CRenderObject2D* pRoot, int8 nLightType, CCamera2D* pExternalCamera, IBlendState* pBlend )
{
	if( m_pRoot )
	{
		m_pRoot->RemoveThis();
		m_pRoot = NULL;
	}
	if( m_pRenderer )
	{
		m_pRenderer->OnDestroyDevice( IRenderSystem::Inst() );
		delete m_pRenderer;
		m_pRenderer = NULL;
	}

	m_pExternalRoot = pRoot;
	m_pExternalCamera = pExternalCamera;
	m_nLightType = nLightType;
	m_pBlend = pBlend;
	m_bOpaque = m_pBlend == NULL;
	
	if( pRoot )
	{
		if( m_nLightType == 2 )
			pExternalCamera->SetViewport( CRectangle( 0, 0, m_localBound.width, m_localBound.height ) );
		else
			pExternalCamera->SetViewport( 0, 0, 0, 0 );
		pExternalCamera->SetSize( m_localBound.width, m_localBound.height );
		pExternalCamera->SetPosition( 0, 0 );

		if( m_nLightType == 2 )
		{
			SLighted2DSubRendererContext context;
			context.pCamera = pExternalCamera;
			context.pRoot = pRoot;
			context.pGUIRoot = m_pGUIRoot;
			context.pGUICamera = m_pExternalGUICamera;
			m_pRenderer = new CLighted2DRenderer( context );
		}
		else if( nLightType == 1 )
		{
			SSimpleLighted2DRendererContext context;
			context.pCamera = &GetCamera();
			context.pRoot = GetRoot();
			context.pGUIRoot = m_pGUIRoot;
			context.pGUICamera = m_pExternalGUICamera;
			m_pRenderer = new CSimpleLighted2DRenderer( context );
		}
		else
		{
			SSimpleSubRendererContext context;
			context.pCamera = pExternalCamera;
			context.pRoot = pRoot;
			context.bInvertY = false;
			m_pRenderer = new CSimpleRenderer( context );
		}
		m_pRenderer->OnCreateDevice( IRenderSystem::Inst() );
		m_pRenderer->OnResize( IRenderSystem::Inst(), m_texSize );
	}
}

void CUIViewport::SetLight( int8 nLightType )
{
	if( m_pRenderer )
	{
		m_pRenderer->OnDestroyDevice( IRenderSystem::Inst() );
		delete m_pRenderer;
		m_pRenderer = NULL;
	}
	m_nLightType = nLightType;
	if( nLightType == 2 )
	{
		SLighted2DSubRendererContext context;
		context.pCamera = &GetCamera();
		context.pRoot = GetRoot();
		context.pGUIRoot = m_pGUIRoot;
		context.pGUICamera = m_pExternalGUICamera;
		m_pRenderer = new CLighted2DRenderer( context );
	}
	else if( nLightType == 1 )
	{
		SSimpleLighted2DRendererContext context;
		context.pCamera = &GetCamera();
		context.pRoot = GetRoot();
		context.pGUIRoot = m_pGUIRoot;
		context.pGUICamera = m_pExternalGUICamera;
		m_pRenderer = new CSimpleLighted2DRenderer( context );
	}
	else
	{
		SSimpleSubRendererContext context;
		context.pCamera = &GetCamera();
		context.pRoot = GetRoot();
		context.bInvertY = false;
		m_pRenderer = new CSimpleRenderer( context );
	}
	m_pRenderer->OnCreateDevice( IRenderSystem::Inst() );
	m_pRenderer->OnResize( IRenderSystem::Inst(), m_texSize );
}

void CUIViewport::SetGUICamera( CRenderObject2D* pRoot, CCamera2D* pCam )
{
	m_pGUIRoot = pRoot;
	m_pExternalGUICamera = pCam;
	if( m_nLightType && m_pRenderer )
	{
		static_cast<CLighted2DRenderer*>( m_pRenderer )->SetGUICamera( pCam, pRoot );
	}
}

void CUIViewport::SetCustomRender( const CVector2& customRes )
{
	m_bCustomRender = true;
	m_customRes = customRes;
	if( m_pRenderer )
		m_pRenderer->OnResize( IRenderSystem::Inst(), m_customRes );
}

void CUIViewport::OnInited()
{
	if( !m_pRoot )
		m_pRoot = new CRenderObject2D;
	auto pRoot = GetRoot();
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pRoot );

	SLighted2DSubRendererContext context;
	context.pCamera = &m_camera;
	context.pRoot = pRoot;
	m_pRenderer = new CLighted2DRenderer( context );
	m_pRenderer->OnCreateDevice( IRenderSystem::Inst() );
	ReserveTexSize( CVector2( m_localBound.width, m_localBound.height ) );

	if( m_nLightType != 2 )
		m_camera.SetViewport( CRectangle( 0, 0, m_localBound.width, m_localBound.height ) );
	else
		m_camera.SetViewport( 0, 0, 0, 0 );
	m_camera.SetSize( m_localBound.width, m_localBound.height );
	m_camera.SetPosition( 0, 0 );

	/*vector<char> content;
	GetFileContent( content, "materials/background.xml", true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	CDefaultDrawable2D* pDrawable = new CDefaultDrawable2D;
	pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
	CDefaultDrawable2D* pDrawable1 = new CDefaultDrawable2D;
	pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
	CImage2D* pImage = new CImage2D( pDrawable, pDrawable1, CRectangle( -1024, -512, 2048, 1024 ), CRectangle( 0, 0, 1, 1 ) );
	m_pRoot->AddChild( pImage );
	CPointLightObject* pPointLight = new CPointLightObject( CVector4( 0.1f, 0, 500, -0.05f ), CVector3( 1, 1, 1 ), 10.0f, 0.2f, 0.4f );
	m_pRoot->AddChild( pPointLight );*/
}

void CUIViewport::OnResize( const CRectangle& oldRect, const CRectangle& newRect )
{
	ReserveTexSize( CVector2( newRect.width, newRect.height ) );
	if( m_nLightType != 2 )
		GetCamera().SetViewport( CRectangle( 0, 0, newRect.width, newRect.height ) );
	else
		GetCamera().SetViewport( 0, 0, 0, 0 );
	GetCamera().SetSize( newRect.width, newRect.height );
}