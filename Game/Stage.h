#pragma once
#include "Entity.h"
#include "Common/Trigger.h"
#include "Common/Camera2D.h"
#include "Render/Footprint.h"

class CPlayer;
class CStage
{
public:
	CStage();
	~CStage();

	void Start();
	void Stop();

	void AddEntity( CEntity* pEntity );
	void RemoveEntity( CEntity* pEntity );

	CEntity* Pick( const CVector2& pos );
	void MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result );
	CEntity* Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType = eEntityHitType_Count, SRaycastResult* pResult = NULL );
	void MultiRaycast( const CVector2& begin, const CVector2& end, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult = NULL );

	void RegisterBeforeHitTest( uint32 nTime, CTrigger* pTrigger ) { m_tickBeforeHitTest.Register( nTime, pTrigger ); }
	void RegisterAfterHitTest( uint32 nTime, CTrigger* pTrigger ) { m_tickAfterHitTest.Register( nTime, pTrigger ); }
	void Update();

	CEntity* GetRoot() { return m_pEntityRoot; }
	CPlayer* GetPlayer() { return m_pPlayer; }
	CRenderObject2D* GetFootprintRoot() { return m_pFootprintMgr->GetFootprintRoot(); }
	void SetFootprintRoot( CRenderObject2D* pRenderObject ) { m_pFootprintMgr->SetFootprintRoot( pRenderObject ); }
	bool AddFootprint( CFootprintReceiver* pReceiver );

	CHitTestMgr& GetHitTestMgr() { return m_hitTestMgr; }
	float GetElapsedTimePerTick() { return 1.0f / 60; }
	float GetGlobalElapsedTime() { return m_fGlobalElapsedTime; }
	float GetFlyingObjectElapsedTime() { return m_fFlyingObjectElapsedTime; }
	CFootprintMgr* GetFootprintMgr() { return m_pFootprintMgr; }

	CVector2 GetGravityDir() { return CVector2( 0, -1 );  }
private:
	bool m_bStarted;
	CHitTestMgr m_hitTestMgr;
	CReference<CEntity> m_pEntityRoot;
	CPlayer* m_pPlayer;
	CCamera2D m_camera;
	float m_fGlobalElapsedTime;
	float m_fFlyingObjectElapsedTime;
	CReference<CFootprintMgr> m_pFootprintMgr;

	CTimedTrigger<4096> m_tickBeforeHitTest;
	CTimedTrigger<4096> m_tickAfterHitTest;
};