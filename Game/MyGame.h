#pragma once
#include "Game.h"
#include "Stage.h"
#include "Common/BitArray.h"
#include "Common/Trigger.h"

class CGame : public IGame
{
public:
	CGame();
	virtual void Start() override;
	virtual void Stop() override;
	virtual void Update() override;

	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) override;

	CStage* GetStage() { return m_pStage; }
	const CVector2& GetScreenResolution() { return m_screenResolution; }
	float GetElapsedTimePerTick() { return 1.0f / 60; }
	double GetTotalTime() { return m_dTotalTime; }
	int32 GetTimeStamp() { return m_trigger.GetTimeStamp(); }

	void Register( uint32 nTime, CTrigger* pTrigger ) { m_trigger.Register( nTime, pTrigger ); }

	DECLARE_GLOBAL_INST_REFERENCE( CGame )
private:
	CStage* m_pStage;
	CBitArray m_key;
	CBitArray m_keyUp;
	CBitArray m_keyDown;

	CVector2 m_screenResolution;

	double m_dTotalTime;
	CTimedTrigger<4096> m_trigger;
};