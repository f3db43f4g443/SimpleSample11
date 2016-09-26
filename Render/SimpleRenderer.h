#pragma once
#include "Renderer.h"
#include "RenderContext2D.h"
#include "Canvas.h"

struct SSimpleSubRendererContext
{
	SSimpleSubRendererContext() : pCamera( NULL ), bInvertY( true ) {}
	CCamera2D* pCamera;
	bool bInvertY;
	CReference<CRenderObject2D> pRoot;
};

class CSimpleRenderer : public IRenderer
{
public:
	CSimpleRenderer( bool bInvertY = true ) : m_nUpdateFrames( 0 ), m_nTimeStamp( 0 ), m_bIsSubRenderer( false ) { m_subRendererContext.bInvertY = bInvertY; }
	CSimpleRenderer( const SSimpleSubRendererContext& context ) : m_nUpdateFrames( 0 ), m_nTimeStamp( 0 ), m_bIsSubRenderer( true ), m_subRendererContext( context ) {}

	virtual void OnCreateDevice( IRenderSystem* pSystem ) override;
	virtual void OnResize( IRenderSystem* pSystem, const CVector2& size ) override;
	virtual void OnDestroyDevice( IRenderSystem* pSystem ) override;

	virtual void OnUpdate( IRenderSystem* pSystem ) override;
	virtual void OnRender( IRenderSystem* pSystem ) override;

	virtual bool IsSubRenderer() override { return m_bIsSubRenderer; }
	virtual ITexture* GetSubRendererTexture() override { return m_pSubRendererTexture; }
	virtual void ReleaseSubRendererTexture() override { if( m_bIsSubRenderer ) m_sizeDependentPool.Release( m_pSubRendererTexture ); }

	const CVector2& GetScreenRes() { return m_screenRes; }
private:
	CVector2 m_screenRes;

	uint32 m_nTimeStamp;
	uint32 m_nUpdateFrames;

	bool m_bIsSubRenderer;
	SSimpleSubRendererContext m_subRendererContext;
	SRenderGroup m_renderGroup[2];
	CReference<ITexture> m_pSubRendererTexture;
	CReference<ITexture> m_pDepthStencil;
	CRenderTargetPool m_sizeDependentPool;
};