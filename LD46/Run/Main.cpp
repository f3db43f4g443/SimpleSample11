#include "Game/stdafx.h"
#include "Game/MyGame.h"
#include "Render/SimpleRenderer.h"
#include "Common/DateTime.h"
#include "Common/Rand.h"
#include <Windows.h>

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	SRand::Inst().nSeed = (uint32)GetLocalTime();
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	SDeviceCreateContext context;
	context.szWindowName = L"P0";
	context.resolution = CVector2( 1152, 720 );
	pRenderSystem->SetRenderer( new CSimpleRenderer );
	pRenderSystem->CreateDevice( context );

	InitGame();
	CGame::Inst().SetCurState( &CMainGameState::Inst() );
	pRenderSystem->SetGame( &CGame::Inst() );
	pRenderSystem->Start();
	exit( 0 );
}
