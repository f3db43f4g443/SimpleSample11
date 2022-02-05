#include "stdafx.h"
#include <Windows.h>
#undef Yield
#include "Game/MyGame.h"
#include "Editor/Editor.h"
#include "Render/SimpleRenderer.h"
#include "Editor/HitProxyEdit.h"
#include "Editor/LevelEdit.h"
#include "Editor/LevelEnvLayerEdit.h"
#include "Editor/LevelTools.h"
#include "Editor/WorldCfgEditor.h"
#include "Editor/Editors/ResourceEditor.h"
#include "UICommon/UIFactory.h"

#include "GlobalCfg.h"

void InitEditor()
{
	CResourceEditor::LightType() = 0;
	CObjectDataEditMgr::Inst().Register<CHitProxy, CHitProxyDataEdit>();
	CObjectDataEditMgr::Inst().Register<CLevelEnvLayer, CLevelEnvLayerEdit>();
	CObjectDataEditMgr::Inst().Register<CMyLevel, CLevelEdit>();
	CEditor::Inst().RegisterEditor( CWorldCfgEditor::Inst(), "EditorRes/UI/world_editor.xml", "World Config(.w)", "w" );
}

void RegisterToolsets();
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	class CMyEditor : public CEditor
	{
	public:
		virtual void Start() override
		{
			CEditor::Start();
			CLevelToolsView* pToolsView = CLevelToolsView::Inst();
			CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/view.xml" )->GetElement()->Clone( pToolsView );
			m_pUIMgr->AddChild( pToolsView );
			RegisterToolsets();
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
