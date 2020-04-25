#pragma once
#include "Stage.h"
#include "BasicElems.h"
#include "GUI/StageDirector.h"

struct SSubStage
{
	SSubStage() : pStage( NULL ), pViewport( NULL ), bPaused( false ) {}
	CStage* pStage;
	CUIViewport* pViewport;
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
	void Start();

	void EnterStage( SStageEnterContext& enterContext );
	void EnterStage( CPrefab* pStage, const TVector2<int32>& pos, int8 nDir );
	void RestartStage();
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

	float m_fPercent;

	int32 m_nMainUISubStage;
	bool m_bUpdating;
	bool m_bChangeStage;
	SStageEnterContext m_stageEnterContext;
	CBufFile m_playerData;
	int32 m_nRestartTime;
};