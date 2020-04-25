#pragma once
#include "Game.h"
#include "World.h"
#include "Stage.h"
#include "Common/BitArray.h"
#include "Common/Trigger.h"
#include "UICommon/UIManager.h"
#include "GameState.h"

class CGame : public IGame
{
public:
	CGame();
	virtual void Start() override;
	virtual void Stop() override;
	virtual void Update() override;
	void RestartLater() { m_bRestart = true; }
	void SetCurState( IGameState* pGameState );

	virtual void OnResize( const CVector2& size ) override;
	virtual void OnMouseDown( const CVector2& pos ) override;
	virtual void OnMouseUp( const CVector2& pos ) override;
	virtual void OnRightMouseDown( const CVector2& pos ) override;
	virtual void OnRightMouseUp( const CVector2& pos ) override;
	virtual void OnMouseMove( const CVector2& pos ) override;
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) override;
	virtual void OnChar( uint32 nChar ) override;

	CVector2 GetScreenRes() { return m_screenRes; }
	float GetElapsedTimePerTick() { return 1.0f / 60; }
	double GetTotalTime() { return m_dTotalTime; }
	int32 GetTimeStamp() { return m_trigger.GetTimeStamp(); }
	float GetKeyHoldTime( uint32 nKey ) { return IsKeyUp( nKey ) ? ( m_keyUpTime[nKey] - m_keyDownTime[nKey] ) / 1000000.0f : 0; }

	bool IsKey( uint32 nKey ) { return m_key.GetBit( nKey ); }
	bool IsKeyUp( uint32 nKey ) { return m_keyUp.GetBit( nKey ); }
	bool IsKeyDown( uint32 nKey ) { return m_keyDown.GetBit( nKey ); }
	bool IsKeyHolding( uint32 nKey ) { return m_keyHolding.GetBit( nKey ); }
	bool IsChar( uint32 nKey ) { return m_char.GetBit( nKey ); }
	bool IsMouse() { return m_bIsMouse; }
	bool IsMouseUp() { return m_bIsMouseUp; }
	bool IsMouseDown() { return m_bIsMouseDown; }
	bool IsRightMouse() { return m_bIsRightMouse; }
	bool IsRightMouseUp() { return m_bIsRightMouseUp; }
	bool IsRightMouseDown() { return m_bIsRightMouseDown; }

	void Register( uint32 nTime, CTrigger* pTrigger ) { m_trigger.Register( nTime, pTrigger ); }

	void BeforeRender() { if( m_pCurGameState ) m_pCurGameState->BeforeRender(); }

	DECLARE_GLOBAL_INST_REFERENCE( CGame )
private:
	bool m_bStarted;
	bool m_bRestart;
	IGameState* m_pCurGameState;

	CBitArray m_key;
	CBitArray m_keyUp;
	CBitArray m_keyDown;
	CBitArray m_keyHolding;
	CBitArray m_char;
	CBitArray m_char1;
	vector<uint64> m_keyDownTime;
	vector<uint64> m_keyUpTime;
	bool m_bIsMouse;
	bool m_bIsMouseDown;
	bool m_bIsMouseUp;
	bool m_bIsRightMouse;
	bool m_bIsRightMouseDown;
	bool m_bIsRightMouseUp;
	CVector2 m_screenRes;

	TClassTrigger<CGame> m_beforeRender;

	double m_dTotalTime;
	CTimedTrigger<4096> m_trigger;
};

void InitGame();