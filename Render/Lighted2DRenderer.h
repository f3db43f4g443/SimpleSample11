#pragma once
#include "Renderer.h"
#include "RenderContext2D.h"
#include "Canvas.h"

class CLighted2DRenderer : public IRenderer
{
public:
	CLighted2DRenderer() : m_nUpdateFrames( 0 ), m_nTimeStamp( 0 ) {}
	virtual void OnCreateDevice( IRenderSystem* pSystem ) override;
	virtual void OnResize( IRenderSystem* pSystem, const CVector2& size ) override;
	virtual void OnDestroyDevice( IRenderSystem* pSystem ) override;
	
	virtual void OnUpdate( IRenderSystem* pSystem ) override;
	virtual void OnRender( IRenderSystem* pSystem ) override;

	const CVector2& GetScreenRes() { return m_screenRes; }
	const CVector2& GetLightMapRes() { return m_lightMapRes; }

	DECLARE_GLOBAL_INST_REFERENCE( CLighted2DRenderer )
private:
	void RenderScene( IRenderSystem* pSystem, IRenderTarget* pTarget );

	void RenderColorBuffer( CRenderContext2D& context );
	void RenderOcclusionBuffer( CRenderContext2D& context );
	void RenderLights( CRenderContext2D& context );
	void RenderGUI( CRenderContext2D& context );

	void RenderLightDirectional( IRenderSystem* pSystem, SDirectionalLight2D& light );
	void RenderShadowDirectional( IRenderSystem* pSystem, SDirectionalLight2D& light );
	void DrawSceneLightingDirectional( IRenderSystem* pSystem, SDirectionalLight2D& light );
	
	void RenderLightPointInstancing( IRenderSystem* pSystem, SPointLight2D* pLights );
	void RenderLightPoint( IRenderSystem* pSystem, vector<SPointLight2D*> vecLights );
	void RenderShadowPoint( IRenderSystem* pSystem, vector<SPointLight2D*> vecLights );
	void DrawSceneLightingPoint( IRenderSystem* pSystem, vector<SPointLight2D*> vecLights );

	CReference<ITexture> m_pColorBuffer;
	CReference<ITexture> m_pEmissionBuffer;
	CReference<ITexture> m_pLightAccumulationBuffer;

	CReference<ITexture> m_pOcclusionBuffer;
	CReference<ITexture> m_pDepthStencilHighRes;

	CReference<ITexture> m_pShadowBuffer[2];

	CVector2 m_screenRes;
	CVector2 m_lightMapRes;

	uint32 m_nTimeStamp;
	uint32 m_nUpdateFrames;
};