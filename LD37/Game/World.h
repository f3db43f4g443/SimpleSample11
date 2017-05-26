#pragma once
#include "Stage.h"
#include "Player.h"
#include "GUI/StageDirector.h"

struct SSubStage
{
	SSubStage() : pStage( NULL ), bPaused( false ){}
	CStage* pStage;
	bool bPaused;
};

class CWorld
{
public:
	CWorld();
	~CWorld();

	CPlayer* GetPlayer() { return m_pCurPlayer; }
	void SetPlayer( CPlayer* pPlayer ) { m_pCurPlayer = pPlayer; }
	void CreatePlayer();

	void EnterStage( const char* szStageName, SStageEnterContext& enterContext );
	void Update();
	void Stop();
	
	SSubStage* GetSubStage( uint32 nSlot ) { return nSlot < m_subStages.size()? &m_subStages[nSlot] : NULL; }
	uint32 PlaySubStage( const char* szSubStageName, CUIViewport* pViewport );
	void StopSubStage( uint32 nSlot );
private:
	CStage* m_pCurStage;
	CReference<CPlayer> m_pCurPlayer;
	map<string, SStageContext> m_mapStageContexts;
	vector<SSubStage> m_subStages;

	bool m_bUpdating;
	string m_strEnterStage;
	SStageEnterContext m_stageEnterContext;
};