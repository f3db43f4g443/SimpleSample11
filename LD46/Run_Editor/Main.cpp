#include "Game/stdafx.h"
#include <Windows.h>
#include "Game/MyGame.h"
#include "Editor/Editor.h"
#include "Render/SimpleRenderer.h"
#include "Editor/HitProxyEdit.h"
#include "Editor/LevelEdit.h"
#include "UICommon/UIFactory.h"

#include "GlobalCfg.h"

void InitEditor()
{
	CObjectDataEditMgr::Inst().Register<CHitProxy, CHitProxyDataEdit>();
	CObjectDataEditMgr::Inst().Register<CMyLevel, CLevelEdit>();
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	class CMyEditor : public CEditor
	{
	public:
		virtual void Start() override
		{
			CEditor::Start();
			//CGlobalCfg::Inst().Load();
		}
	};
	static CMyEditor g_editor;

	InitGame();
	InitEditor();

	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	pRenderSystem->SetRenderer( new CSimpleRenderer );
	pRenderSystem->SetGame( &CEditor::Inst() );
	SDeviceCreateContext context;
	context.resolution = CVector2( 1600, 800 );
	pRenderSystem->CreateDevice( context );
	pRenderSystem->Start();
	exit( 0 );
}
