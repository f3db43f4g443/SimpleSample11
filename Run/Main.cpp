#include "Game/stdafx.h"
#include "Game/MyGame.h"
#include "Lighted2DRenderer.h"
#include <Windows.h>
#include "Common/Rand.h"
#include "Common/DateTime.h"

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	SRand::Inst().nSeed = GetLocalTime();

	InitGame();
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	pRenderSystem->SetRenderer( new CLighted2DRenderer );
	pRenderSystem->SetGame( &CGame::Inst() );
	SDeviceCreateContext context;
	context.resolution = CVector2( 800, 600 );
	pRenderSystem->CreateDevice( context );
	pRenderSystem->Start();
	exit( 0 );
}
