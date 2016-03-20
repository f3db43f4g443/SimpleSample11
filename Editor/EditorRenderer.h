#pragma once
#include "Render/Renderer.h"

class CEditorRenderer : public IRenderer
{
public:
	CEditorRenderer() : m_nUpdateFrames( 0 ), m_nTimeStamp( 0 ) {}

	virtual void OnCreateDevice( IRenderSystem* pSystem ) override;
	virtual void OnResize( IRenderSystem* pSystem, const CVector2& size ) override;

	virtual void OnUpdate( IRenderSystem* pSystem ) override;
	virtual void OnRender( IRenderSystem* pSystem ) override;
private:
	CVector2 m_screenRes;

	uint32 m_nTimeStamp;
	uint32 m_nUpdateFrames;
};