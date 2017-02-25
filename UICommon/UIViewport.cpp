#include "stdafx.h"
#include "UIViewport.h"
#include "UIManager.h"
#include "Render/SimpleRenderer.h"
#include "Render/Lighted2DRenderer.h"
#include "Render/Scene2DManager.h"
#include "Render/CommonShader.h"
#include "Render/LightRendering.h"
#include "Common/xml.h"
#include "Common/FileUtil.h"

CUIViewport::CUIViewport()
	: m_pRenderer( NULL )
	, m_pExternalCamera( NULL )
	, m_texSize( 0, 0 )
	, m_bLight( false )
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

void CUIViewport::ReserveTexSize( const CVector2 & size )
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
		auto origRect = GetCamera().GetViewArea();
		if( m_bLight )
		{
			CVector2 size = m_bCustomRender ? m_customRes : m_texSize;
			GetCamera().SetViewArea( origRect.Offset( CVector2( m_texSize.x - m_localBound.width, m_texSize.y - m_localBound.height ) * 0.5f ) );
			GetCamera().SetSize( size.x, size.y );
		}

		m_pRenderer->OnRender( context.pRenderSystem );

		if( m_bCustomRender )
		{
			auto pPostProcessPass = &m_customRender;
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

		GetCamera().SetViewArea( origRect );
		m_events.Trigger( eEvent_Action, context.pRenderSystem );
		if( GetMgr() )
			context.AddElement( &m_elem );
	}
}

void CUIViewport::Flush( CRenderContext2D& context )
{
	if( m_bCustomRender )
	{
		CopyToRenderTarget( context.pRenderSystem, NULL, m_pCustomTarget, m_localBound.Offset( globalTransform.GetPosition() ),
			CRectangle( 0, 0, m_localBound.width, m_localBound.height ),
			GetMgr()->GetSize().GetSize(), m_texSize, m_elem.depth * context.mat.m22 + context.mat.m23 );

		auto& sizeDependentPool = *m_pRenderer->GetRenderTargetPool();
		sizeDependentPool.Release( m_pCustomTarget );
	}
	else
	{
		CopyToRenderTarget( context.pRenderSystem, NULL, m_pRenderer->GetSubRendererTexture(), m_localBound.Offset( globalTransform.GetPosition() ),
			CRectangle( 0, 0, m_localBound.width, m_localBound.height ),
			GetMgr()->GetSize().GetSize(), m_texSize, m_elem.depth * context.mat.m22 + context.mat.m23 );
		m_pRenderer->ReleaseSubRendererTexture();
	}
	m_elem.OnFlushed();
}

void CUIViewport::DebugDrawLine( IRenderSystem* pRenderSystem, const CVector2& begin, const CVector2& end, const CVector4& color )
{
	const CRectangle& rect = GetCamera().GetViewArea();
	CVector2 pt1 = ( begin - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
	CVector2 pt2 = ( end - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
	m_pRenderer->DebugDrawLine( pRenderSystem, pt1, pt2, color );
}

void CUIViewport::DebugDrawTriangles( IRenderSystem* pRenderSystem, uint32 nVert, CVector2* pVert, const CVector4& color )
{
	const CRectangle& rect = GetCamera().GetViewArea();
	for( int i = 0; i + 2 < nVert; i += 3 )
	{
		CVector2 pt1 = ( pVert[i] - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
		CVector2 pt2 = ( pVert[i + 1] - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
		CVector2 pt3 = ( pVert[i + 2] - rect.GetCenter() ) * CVector2( 2.0f / rect.width, 2.0f / rect.height );
		m_pRenderer->DebugDrawTriangle( pRenderSystem, pt1, pt2, pt3, color );
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

void CUIViewport::Set( CRenderObject2D* pRoot, CCamera2D* pExternalCamera, bool bLight )
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
	m_bLight = bLight;
	
	if( pRoot )
	{
		if( !m_bLight )
			pExternalCamera->SetViewport( CRectangle( 0, 0, m_localBound.width, m_localBound.height ) );
		else
			pExternalCamera->SetViewport( 0, 0, 0, 0 );
		pExternalCamera->SetSize( m_localBound.width, m_localBound.height );
		pExternalCamera->SetPosition( 0, 0 );

		if( bLight )
		{
			SLighted2DSubRendererContext context;
			context.pCamera = pExternalCamera;
			context.pRoot = pRoot;
			m_pRenderer = new CLighted2DRenderer( context );
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

	if( !m_bLight )
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
	if( !m_bLight )
		GetCamera().SetViewport( CRectangle( 0, 0, newRect.width, newRect.height ) );
	else
		GetCamera().SetViewport( 0, 0, 0, 0 );
	GetCamera().SetSize( newRect.width, newRect.height );
}