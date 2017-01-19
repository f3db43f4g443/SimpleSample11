#pragma once
#include "Game.h"
#include "World.h"
#include "Stage.h"
#include "Common/BitArray.h"
#include "Common/Trigger.h"
#include "UICommon/UIManager.h"

class CGame : public IGame
{
public:
	CGame();
	virtual void Start() override;
	virtual void Stop() override;
	virtual void Update() override;
	
	virtual void OnResize( const CVector2& size ) override;
	virtual void OnMouseDown( const CVector2& pos ) override;
	virtual void OnMouseUp( const CVector2& pos ) override;
	virtual void OnRightMouseDown( const CVector2& pos ) override;
	virtual void OnRightMouseUp( const CVector2& pos ) override;
	virtual void OnMouseMove( const CVector2& pos ) override;
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) override;
	virtual void OnChar( uint32 nChar ) override;

	CWorld* GetWorld() { return m_pWorld; }
	const CVector2& GetScreenResolution() { return m_screenResolution; }
	float GetElapsedTimePerTick() { return 1.0f / 60; }
	double GetTotalTime() { return m_dTotalTime; }
	int32 GetTimeStamp() { return m_trigger.GetTimeStamp(); }

	void Register( uint32 nTime, CTrigger* pTrigger ) { m_trigger.Register( nTime, pTrigger ); }
	
	void BeforeRender() { m_pUIMgr->UpdateLayout(); }

	DECLARE_GLOBAL_INST_REFERENCE( CGame )
private:
	CWorld* m_pWorld;
	CBitArray m_key;
	CBitArray m_keyUp;
	CBitArray m_keyDown;
	bool m_bIsMouse;
	bool m_bIsMouseDown;
	bool m_bIsMouseUp;
	bool m_bIsRightMouse;
	bool m_bIsRightMouseDown;
	bool m_bIsRightMouseUp;

	CVector2 m_screenResolution;

	CReference<CUIManager> m_pUIMgr;
	CCamera2D m_camera;
	TClassTrigger<CGame> m_beforeRender;

	double m_dTotalTime;
	CTimedTrigger<4096> m_trigger;
};

void InitGame();