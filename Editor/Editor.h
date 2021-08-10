#pragma once
#include "Game.h"
#include "Common/Camera2D.h"
#include "UICommon/UIManager.h"
#include "UICommon/UIFactory.h"

class CEditor : public IGame
{
public:
	CEditor();
	virtual void Start() override;
	virtual void Stop() override;
	virtual void Update() override;
	
	void PostInit();
	virtual void OnResize( const CVector2& size ) override;
	virtual void OnMouseDown( const CVector2& pos ) override;
	virtual void OnMouseUp( const CVector2& pos ) override;
	virtual void OnMouseMove( const CVector2& pos ) override;
	virtual void OnMouseWheel( int32 nDelta ) override;
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) override;
	virtual void OnChar( uint32 nChar ) override;

	struct SRegisteredEditor
	{
		class CResourceEditor* pEditor;
		string strPath;
		string strDesc;
	};
	void RegisterEditor( class CResourceEditor* pEditor, const char* szPath, const char* szDesc, const char* szTag )
	{
		auto& item = m_mapRegisteredEditors[szTag];
		item.pEditor = pEditor;
		item.strPath = szPath;
		item.strDesc = szDesc;
	}
	void RegisterToolDefault( int32 nIcon, function<void()> funcHandler );

	CUIManager* GetUIMgr() { return m_pUIMgr; }
	void SetEditor( CResourceEditor* pElem );
	CResourceEditor* SetEditor( const char* szTag );
	map<string, SRegisteredEditor>& GetRegisteredEditors() { return m_mapRegisteredEditors; }
	void OpenFile( const char* szFile, const char* szParam = "" );

	void BeforeRender() { m_pUIMgr->UpdateLayout(); }
	
	static CEditor& Inst()
	{
		return *_inst();
	}
protected:
	static CEditor*& _inst()
	{
		static CEditor* pEditor = NULL;
		return pEditor;
	}
	CReference<CUIManager> m_pUIMgr;
	CReference<CUIElement> m_pCurShownElem;
	CCamera2D m_camera;

	TClassTrigger<CEditor> m_beforeRender;

	CReference<CUIResource> m_pDefaultToolResource;

	map<string, SRegisteredEditor> m_mapRegisteredEditors;
};