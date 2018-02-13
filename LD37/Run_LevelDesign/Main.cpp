#include "../Game/stdafx.h"
#include "../Game/MyGame.h"
#include "Render/SimpleRenderer.h"
#include "Common/Rand.h"
#include "Common/DateTime.h"
#include <Windows.h>

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	uint64 nLocalTime = GetLocalTime();
	SRand::Inst().nSeed = (uint32)nLocalTime;
	SRand::Inst<eRand_Render>().nSeed = (uint32)( nLocalTime  >> 32 );

	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	SDeviceCreateContext context;
	context.resolution = CVector2( 1280, 960 );
	pRenderSystem->SetRenderer( new CSimpleRenderer );
	pRenderSystem->CreateDevice( context );

	InitGame();
	CGame::Inst().SetCurState( &CLevelDesignGameState::Inst() );
	pRenderSystem->SetGame( &CGame::Inst() );
	pRenderSystem->Start();
	exit( 0 );
}
