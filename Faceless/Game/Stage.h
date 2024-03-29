#pragma once
#include "Entity.h"
#include "Common/Trigger.h"
#include "Common/Camera2D.h"
#include "Render/Footprint.h"

struct SStageContext
{
	SStageContext() : nStageInsts( 0 ) {}
	uint32 nStageInsts;
	string strName;
	string strSceneResName;

	map<string, CReference<CResource> > mapDependentRes;
	void Load( IBufReader& buf );
	void Save( CBufFile& buf );
};

struct SStageEnterContext
{
	SStageEnterContext() : pViewport( NULL ) {}
	string strStartPointName;
	class CUIViewport* pViewport;
};

enum
{
	eStageEvent_Start,
	eStageEvent_Stop,
	eStageEvent_PlayerUpdated,

	eStageEvent_Count,
};

enum
{
	eGUIOption_Crosshair = 1,
	eGUIOption_HpBar = 2,
};

class CWorld;
class CPlayer;
class CStage
{
public:
	CStage( CWorld* pWorld );
	~CStage();

	CWorld* GetWorld() { return m_pWorld; }
	SStageContext* GetContext() { return m_pContext; }
	void Create( SStageContext* pContext );

	void Start( CPlayer* pPlayer, const SStageEnterContext& context );
	void Stop();

	void SetViewport( CUIViewport* pViewport );

	void AddEntity( CEntity* pEntity );
	void RemoveEntity( CEntity* pEntity );

	CEntity* Pick( const CVector2& pos );
	void MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result );
	CEntity* Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType = eEntityHitType_Count, SRaycastResult* pResult = NULL );
	void MultiRaycast( const CVector2& begin, const CVector2& end, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult = NULL );

	void RegisterStageEvent( uint32 nEvent, CTrigger* pTrigger ) { m_events.Register( nEvent, pTrigger ); }
	void RegisterBeforeHitTest( uint32 nTime, CTrigger* pTrigger ) { m_tickBeforeHitTest.Register( nTime, pTrigger ); }
	void RegisterAfterHitTest( uint32 nTime, CTrigger* pTrigger ) { m_tickAfterHitTest.Register( nTime, pTrigger ); }
	void TriggerEvent( uint32 nEvent ) { m_events.Trigger( nEvent, NULL ); }
	void Update();

	CEntity* GetRoot() { return m_pEntityRoot; }
	CPlayer* GetPlayer() { return m_pPlayer; }
	CCamera2D& GetCamera() { return m_camera; }
	CRenderObject2D* GetFootprintRoot() { return m_pFootprintMgr->GetFootprintRoot(); }
	void SetFootprintRoot( CRenderObject2D* pRenderObject ) { m_pFootprintMgr->SetFootprintRoot( pRenderObject ); }
	bool AddFootprint( CFootprintReceiver* pReceiver );

	void AddStartPoint( CEntity* pEntity ) { m_mapStartPoints[pEntity->GetName()] = pEntity; }
	CEntity* GetStartPoint( const char* szName )
	{
		auto itr = m_mapStartPoints.find( szName );
		if( itr != m_mapStartPoints.end() )
			return itr->second;
		return NULL;
	}

	CHitTestMgr& GetHitTestMgr() { return m_hitTestMgr; }
	float GetElapsedTimePerTick() { return 1.0f / 60; }
	CFootprintMgr* GetFootprintMgr() { return m_pFootprintMgr; }
private:
	CWorld* m_pWorld;
	SStageContext* m_pContext;
	bool m_bStarted;
	CHitTestMgr m_hitTestMgr;
	CReference<CEntity> m_pEntityRoot;
	CPlayer* m_pPlayer;
	CCamera2D m_camera;
	CReference<CFootprintMgr> m_pFootprintMgr;
	map<string, CReference<CEntity> > m_mapStartPoints;
	SStageEnterContext m_enterContext;

	CEventTrigger<eStageEvent_Count> m_events;
	CTimedTrigger<4096> m_tickBeforeHitTest;
	CTimedTrigger<4096> m_tickAfterHitTest;
};