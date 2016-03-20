#pragma once
#include "Game.h"
#include "Common/Camera2D.h"
#include "UI/UIManager.h"

class CEditor : public IGame
{
public:
	virtual void Start() override;
	virtual void Stop() override;
	virtual void Update() override;
	
	virtual void OnMouseDown( const CVector2& pos ) override;
	virtual void OnMouseUp( const CVector2& pos ) override;
	virtual void OnMouseMove( const CVector2& pos ) override;
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) override;
	virtual void OnChar( uint32 nChar ) override;
	
	DECLARE_GLOBAL_INST_REFERENCE( CEditor )
private:
	CReference<CUIManager> m_pUIMgr;
	CCamera2D m_camera;
};