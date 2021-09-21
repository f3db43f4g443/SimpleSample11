#include "Game/stdafx.h"
#include "Game/MyGame.h"
#include "Render/SimpleRenderer.h"
#include "Common/DateTime.h"
#include "Common/Rand.h"
#include <Windows.h>
#include "Platform_Common.h"

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	SRand::Inst().nSeed = (uint32)GetLocalTime();
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	SDeviceCreateContext context;
	context.szWindowName = L"P0";
	int nFullWidth = GetSystemMetrics( SM_CXSCREEN );
	int nFullHeight = GetSystemMetrics( SM_CYSCREEN );
	context.resolution = CVector2( nFullWidth, nFullHeight );
	context.bFullWindow = true;
	pRenderSystem->SetRenderer( new CSimpleRenderer );
	pRenderSystem->CreateDevice( context );

	Init_PlatformSDK();

	InitGame();
	CGame::Inst().SetCurState( &CMainGameState::Inst() );
	pRenderSystem->SetGame( &CGame::Inst() );
	pRenderSystem->Start();

	Shutdown_PlatformSDK();
	exit( 0 );
}
