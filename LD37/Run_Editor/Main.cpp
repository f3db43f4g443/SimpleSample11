#include "../Game/stdafx.h"
#include <Windows.h>
#include "../Game/MyGame.h"
#include "Editor/Editor.h"
#include "Render/SimpleRenderer.h"
#include "Editor/HitProxyEdit.h"
#include "UICommon/UIFactory.h"

#include "GlobalCfg.h"

void InitEditor()
{
	CObjectDataEditMgr::Inst().Register<CHitProxy, CHitProxyDataEdit>();
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	SDeviceCreateContext context;
	context.resolution = CVector2( 1200, 600 );
	pRenderSystem->SetRenderer( new CSimpleRenderer );
	pRenderSystem->CreateDevice( context );

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

	pRenderSystem->SetGame( &CEditor::Inst() );
	pRenderSystem->Start();
	exit( 0 );
}
