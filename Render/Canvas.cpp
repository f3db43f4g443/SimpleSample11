#include "stdafx.h"
#include "Canvas.h"
#include "RenderSystem.h"
#include "Scene2DManager.h"
#include "Game.h"

CCanvas::CCanvas( bool bSizeDependent, uint32 nWidth, uint32 nHeight, EFormat eFormat, EDepthStencilType eDepthStencil, bool bMip )
	: m_pRenderTargetPool( bSizeDependent ? &CRenderTargetPool::GetSizeDependentPool() : &CRenderTargetPool::GetSizeIndependentPool() )
	, m_textureDesc( ETextureType::Tex2D, nWidth, nHeight, 1, bMip? 0: 1, eFormat, false, true, false )
	, m_depthStencilDesc( ETextureType::Tex2D, nWidth, nHeight, 1, 0, EFormat::EFormatD32FloatS8X24UInt, false, false, true )
	, m_eRenderPass( eRenderPass_Color )
	, m_eDepthStencilType( eDepthStencil )
	, m_clearColor( 0, 0, 0, 0 )
{
	m_cam.SetViewport( 0, 0, nWidth, nHeight );
	m_cam.SetPosition( 0, 0 );
	m_cam.SetSize( nWidth, nHeight );
}

CCanvas::~CCanvas()
{
	ReleaseTexture();
}

void CCanvas::SetSize( const CVector2& size )
{
	if( m_textureDesc.nDim1 == size.x && m_textureDesc.nDim2 == size.y )
		return;
	m_textureDesc.nDim1 = m_depthStencilDesc.nDim1 = size.x;
	m_textureDesc.nDim2 = m_depthStencilDesc.nDim2 = size.y;
	ReleaseTexture();
}

void CCanvas::Render( CRenderContext2D& context )
{
	CRenderContext2D context1( context );
	context1.rectScene = m_cam.GetViewArea();
	context1.rectViewport = m_cam.GetViewport();
	context1.renderGroup = &m_renderGroup;
	context1.eRenderPass = m_eRenderPass;
	
	context1.Render( m_pRoot );

	IRenderSystem* pSystem = context.pRenderSystem;
	bool bClear = !m_pTexture;
	m_pRenderTargetPool->AllocRenderTarget( m_pTexture, m_textureDesc );
	
	IDepthStencil* pDepthStencil = NULL;
	switch( m_eDepthStencilType )
	{
	case eDepthStencilType_UseDefault:
		pDepthStencil = pSystem->GetDefaultDepthStencil();
		break;
	case eDepthStencilType_Create:
		m_pRenderTargetPool->AllocRenderTarget( m_pDepthStencil, m_depthStencilDesc );
		pDepthStencil = m_pDepthStencil->GetDepthStencil();
	default:
		break;
	}

	pSystem->SetRenderTarget( m_pTexture->GetRenderTarget(), pDepthStencil );
	if( bClear )
	{
		pSystem->ClearRenderTarget( m_clearColor, m_pTexture->GetRenderTarget() );
		pSystem->ClearDepthStencil( true, 1.0f, true, 0 );
	}
	SViewport viewport = {
		context1.rectViewport.x,
		context1.rectViewport.y,
		context1.rectViewport.width,
		context1.rectViewport.height,
		0,
		1
	};
	pSystem->SetViewports( &viewport, 1 );

	if( context1.GetElemCount() )
	{
		context1.FlushElements();
	}

	while( context1.pUpdatedObjects )
	{
		CRenderObject2D* pObject = context1.pUpdatedObjects;
		pObject->RemoveFrom_UpdatedObject();
		pObject->InsertTo_UpdatedObject( context.pUpdatedObjects );
	}
}

void CCanvas::ReleaseTexture()
{
	if( m_pRenderTargetPool )
	{
		m_pRenderTargetPool->Release( m_pTexture );
		m_pRenderTargetPool->Release( m_pDepthStencil );
	}
}

CDynamicTexture::CDynamicTexture( uint32 nWidth, uint32 nHeight, EFormat eFormat )
	: m_texCanvas( false, nWidth, nHeight, eFormat, CCanvas::eDepthStencilType_None )
	, m_bClear( false )
	, m_fTotalTime( 0 )
	, m_nTimeStamp( 0 )
{
	CRenderObject2D* pRoot = new CRenderObject2D;
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pRoot );
	m_texCanvas.SetRoot( pRoot );
}

CDynamicTexture::CDynamicTexture( CRenderObject2D* pRoot, uint32 nWidth, uint32 nHeight, EFormat eFormat )
	: m_texCanvas( false, nWidth, nHeight, eFormat, CCanvas::eDepthStencilType_None )
	, m_bClear( false )
	, m_fTotalTime( 0 )
	, m_nTimeStamp( 0 )
{
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pRoot );
	m_texCanvas.SetRoot( pRoot );
}

CDynamicTexture::~CDynamicTexture()
{
	m_texCanvas.GetRoot()->RemoveThis();
}

void CDynamicTexture::SetRoot( CRenderObject2D* pRoot )
{
	if( m_texCanvas.GetRoot() == m_pDefaultRoot )
	{
		m_pDefaultRoot->RemoveThis();
		m_pDefaultRoot = NULL;
	}
	m_texCanvas.SetRoot( pRoot );
}

void CDynamicTexture::Update( float fTime )
{
	int32 timeStamp = IRenderSystem::Inst()->GetGame()->GetTimeStamp();
	if( timeStamp == m_nTimeStamp )
		return;
	m_nTimeStamp = timeStamp;

	CRenderObject2D* pRoot = m_texCanvas.GetRoot();
	for( CRenderObject2D* pRenderObject = pRoot->Get_Child(); pRenderObject; pRenderObject = pRenderObject->NextChild() )
	{
		pRenderObject->UpdateAnim( fTime );
	}
}

void CDynamicTexture::Render( CRenderContext2D& context )
{
	double totalTime = IRenderSystem::Inst()->GetTotalTime();
	if( totalTime == m_fTotalTime )
		return;
	if( m_bClear )
		m_texCanvas.ReleaseTexture();
	m_fTotalTime = totalTime;
	m_texCanvas.Render( context );
}

IShaderResource* CDynamicTexture::GetShaderResource()
{
	ITexture* pTex = m_texCanvas.GetTexture();
	if( pTex )
		return pTex->GetShaderResource();
	return NULL;
}