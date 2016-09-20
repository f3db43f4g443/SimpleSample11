#pragma once
#include "Game.h"
#include "Common/Camera2D.h"
#include "UICommon/UIManager.h"

class CEditor : public IGame
{
public:
	CEditor() : m_beforeRender( this, &CEditor::BeforeRender ) {}
	virtual void Start() override;
	virtual void Stop() override;
	virtual void Update() override;
	
	virtual void OnResize( const CVector2& size ) override;
	virtual void OnMouseDown( const CVector2& pos ) override;
	virtual void OnMouseUp( const CVector2& pos ) override;
	virtual void OnMouseMove( const CVector2& pos ) override;
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) override;
	virtual void OnChar( uint32 nChar ) override;

	CUIManager* GetUIMgr() { return m_pUIMgr; }
	void SetEditor( CUIElement* pElem );

	void BeforeRender() { m_pUIMgr->UpdateLayout(); }
	
	DECLARE_GLOBAL_INST_REFERENCE( CEditor )
private:
	CReference<CUIManager> m_pUIMgr;
	CReference<CUIElement> m_pCurShownElem;
	CCamera2D m_camera;

	TClassTrigger<CEditor> m_beforeRender;
};