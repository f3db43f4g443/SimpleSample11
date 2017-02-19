#pragma once

#include "Math3D.h"

class IRenderSystem;
class IRenderer
{
public:
	virtual ~IRenderer() {}

	virtual void OnCreateDevice( IRenderSystem* pSystem ) {}
	virtual void OnResize( IRenderSystem* pSystem, const CVector2& size ) {}
	virtual void OnEnterBackground( IRenderSystem* pSystem ) {}
	virtual void OnDestroyDevice( IRenderSystem* pSystem ) {}

	virtual void OnUpdate( IRenderSystem* pSystem ) {}
	virtual void OnRender( IRenderSystem* pSystem ) {}

	virtual bool IsSubRenderer() { return false; }
	virtual class ITexture* GetSubRendererTexture() { return NULL; }
	virtual void FetchSubRendererTexture( class ITexture** ppTex ) { *ppTex = NULL; }
	virtual void ReleaseSubRendererTexture() {}

	virtual class CRenderTargetPool* GetRenderTargetPool() { return NULL; }

	void DebugDrawLine( IRenderSystem* pSystem, const CVector2& pt1, const CVector2& pt2, const CVector4& color );
	void DebugDrawTriangle( IRenderSystem* pSystem, const CVector2& pt1, const CVector2& pt2, const CVector2& pt3, const CVector4& color );
};