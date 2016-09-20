#pragma once
#include "Stage.h"
#include "Player.h"
#include "GUI/StageDirector.h"

struct SSubStage
{
	SSubStage() : pStage( NULL ), pCharacter( NULL ), pFace( NULL ) {}
	CStage* pStage;
	CCharacter* pCharacter;
	class CFace* pFace;
	bool bPaused;
};

class CWorld
{
public:
	CWorld();

	CPlayer* GetPlayer() { return m_pCurPlayer; }
	void SetPlayer( CPlayer* pPlayer ) { m_pCurPlayer = pPlayer; }

	void EnterStage( const char* szStageName, SStageEnterContext& enterContext );
	void Update();
	void Stop();
	
	SSubStage* GetSubStage( uint32 nSlot ) { return nSlot < m_subStages.size()? &m_subStages[nSlot] : NULL; }
	uint32 PlaySubStage( CCharacter* pCharacter );
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