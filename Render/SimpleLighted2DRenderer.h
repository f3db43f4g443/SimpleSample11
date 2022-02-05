#pragma once
#include "Camera2D.h"
#include "Renderer.h"
#include "RenderContext2D.h"

struct SSimpleLighted2DRendererContext
{
	SSimpleLighted2DRendererContext() : pCamera( NULL ), pGUICamera( NULL ) {}
	CCamera2D* pCamera;
	CCamera2D* pGUICamera;
	CReference<CRenderObject2D> pRoot;
	CReference<CRenderObject2D> pGUIRoot;
};

class CSimpleLighted2DRenderer : public IRenderer
{
public:
	CSimpleLighted2DRenderer() : m_nUpdateFrames( 0 ), m_nTimeStamp( 0 ), m_bIsSubRenderer( false ) {}
	CSimpleLighted2DRenderer( const SSimpleLighted2DRendererContext& context ) : m_nUpdateFrames( 0 ), m_nTimeStamp( 0 ), m_bIsSubRenderer( true ), m_subRendererContext( context ) {}

	virtual void OnCreateDevice( IRenderSystem* pSystem ) override;
	virtual void OnResize( IRenderSystem* pSystem, const CVector2& size ) override;
	virtual void OnDestroyDevice( IRenderSystem* pSystem ) override;

	virtual void OnUpdate( IRenderSystem* pSystem ) override;
	virtual void OnRender( IRenderSystem* pSystem ) override;

	virtual bool IsSubRenderer() override { return m_bIsSubRenderer; }
	virtual ITexture* GetSubRendererTexture() override { return m_pSubRendererTexture; }
	virtual void FetchSubRendererTexture( ITexture** ppTex ) override;
	virtual void ReleaseSubRendererTexture() override { if( m_bIsSubRenderer ) m_sizeDependentPool.Release( m_pSubRendererTexture ); }
	virtual CRenderTargetPool* GetRenderTargetPool() override { return &m_sizeDependentPool; }
	const CVector2& GetScreenRes() { return m_screenRes; }
private:
	void RenderScene( IRenderSystem* pSystem, IRenderTarget* pTarget );

	void RenderColorBuffer( CRenderContext2D& context );
	void RenderLights( CRenderContext2D& context );
	void RenderGUI( CRenderContext2D& context );

	void RenderLightDirectional( CRenderContext2D& context, SDirectionalLight2D& light );
	void RenderLightPoint( CRenderContext2D& context, SPointLight2D& light );

	CReference<ITexture> m_pColorBuffer;
	CReference<ITexture> m_pEmissionBuffer;
	CReference<ITexture> m_pNormBuffer;
	CReference<ITexture> m_pLightAccumulationBuffer;
	CReference<ITexture> m_pDepthStencil;

	CVector2 m_screenRes;

	uint32 m_nTimeStamp;
	uint32 m_nUpdateFrames;

	bool m_bIsSubRenderer;
	SSimpleLighted2DRendererContext m_subRendererContext;
	SRenderGroup m_renderGroup[2];
	CReference<ITexture> m_pSubRendererTexture;
	CRenderTargetPool m_sizeDependentPool;
};
