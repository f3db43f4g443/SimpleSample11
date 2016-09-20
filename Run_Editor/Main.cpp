#include "Game/stdafx.h"
#include <Windows.h>
#include "Game/MyGame.h"
#include "Editor/Editor.h"
#include "Editor/EditorRenderer.h"
#include "Lighted2DRenderer.h"
#include "Editor/HitProxyEdit.h"

void InitEditor()
{
	CObjectDataEditMgr::Inst().Register<CHitProxy, CHitProxyDataEdit>();
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	InitGame();
	InitEditor();

	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	pRenderSystem->SetRenderer( new CEditorRenderer );
	pRenderSystem->SetGame( &CEditor::Inst() );
	SDeviceCreateContext context;
	context.resolution = CVector2( 1200, 600 );
	pRenderSystem->CreateDevice( context );
	pRenderSystem->Start();
	exit( 0 );
}
