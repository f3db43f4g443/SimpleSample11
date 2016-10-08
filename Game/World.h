#pragma once
#include "Stage.h"
#include "Player.h"

class CWorld
{
public:
	CWorld();

	CPlayer* GetPlayer() { return m_pCurPlayer; }
	void SetPlayer( CPlayer* pPlayer ) { m_pCurPlayer = pPlayer; }

	void EnterStage( const char* szStageName, const SStageEnterContext& enterContext );
	void Update();
	void Stop();
private:
	CStage* m_pCurStage;
	CReference<CPlayer> m_pCurPlayer;
	map<string, SStageContext> m_mapStageContexts;

	bool m_bUpdating;
	string m_strEnterStage;
	SStageEnterContext m_stageEnterContext;
};