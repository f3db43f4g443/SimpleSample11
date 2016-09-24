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

void CUIViewport::Render( CRenderContext2D& context )
{
	if( m_pRenderer )
	{
		m_pRenderer->OnRender( context.pRenderSystem );
		m_events.Trigger( eEvent_Action, context.pRenderSystem );
		if( GetMgr() )
			context.AddElement( &m_elem );
	}
}

void CUIViewport::Flush( CRenderContext2D& context )
{
	CopyToRenderTarget( context.pRenderSystem, NULL, m_pRenderer->GetSubRendererTexture(), m_localBound.Offset( globalTransform.GetPosition() ),
		CRectangle( 0, 0, m_localBound.width, m_localBound.height ),
		GetMgr()->GetSize().GetSize(), m_localBound.GetSize(), m_elem.depth * context.mat.m22 + context.mat.m23 );
	m_pRenderer->ReleaseSubRendererTexture();
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
	CRectangle sceneBound = GetCamera().GetViewArea();
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
	
	if( pRoot )
	{
		pExternalCamera->SetViewport( CRectangle( 0, 0, m_localBound.width, m_localBound.height ) );
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
		m_pRenderer->OnResize( IRenderSystem::Inst(), CVector2( m_localBound.width, m_localBound.height ) );
	}
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
	m_pRenderer->OnResize( IRenderSystem::Inst(), CVector2( m_localBound.width, m_localBound.height ) );

	m_camera.SetViewport( CRectangle( 0, 0, m_localBound.width, m_localBound.height ) );
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
	if( m_pRenderer )
		m_pRenderer->OnResize( IRenderSystem::Inst(), CVector2( newRect.width, newRect.height ) );
	GetCamera().SetViewport( CRectangle( 0, 0, newRect.width, newRect.height ) );
	GetCamera().SetSize( newRect.width, newRect.height );
}