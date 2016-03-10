#pragma once

#include "Math3D.h"

class IRenderSystem;
class IRenderer
{
public:
	virtual void OnCreateDevice( IRenderSystem* pSystem ) {}
	virtual void OnResize( IRenderSystem* pSystem, const CVector2& size ) {}
	virtual void OnEnterBackground( IRenderSystem* pSystem ) {}
	virtual void OnDestroyDevice( IRenderSystem* pSystem ) {}

	virtual void OnUpdate( IRenderSystem* pSystem ) {}
	virtual void OnRender( IRenderSystem* pSystem ) {}
};