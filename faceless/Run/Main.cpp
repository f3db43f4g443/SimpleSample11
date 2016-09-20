#include "Game/stdafx.h"
#include "Game/MyGame.h"
#include "Render/SimpleRenderer.h"
#include <Windows.h>

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	InitGame();
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	pRenderSystem->SetRenderer( new CSimpleRenderer );
	pRenderSystem->SetGame( &CGame::Inst() );
	SDeviceCreateContext context;
	context.resolution = CVector2( 800, 600 );
	pRenderSystem->CreateDevice( context );
	pRenderSystem->Start();
	exit( 0 );
}
