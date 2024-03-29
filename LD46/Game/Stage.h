#pragma once
#include "Entity.h"
#include "Common/Trigger.h"
#include "Common/Camera2D.h"

struct SStageContext
{
	SStageContext() : nStageInsts( 0 ), bLight( false ) {}
	uint32 nStageInsts;
	string strName;
	string strSceneResName;
	bool bLight;
	TRectangle<int32> rectWorldMapRegion;

	map<string, CReference<CResource> > mapDependentRes;
};

class CUIViewport;
struct SStageEnterContext
{
	SStageEnterContext() : pTarget( NULL ), nParam( 0 ), pViewport( NULL ) {}
	SStageContext* pTarget;
	int32 nParam;
	CUIViewport* pViewport;
};

enum
{
	eStageEvent_Start,
	eStageEvent_Stop,
	eStageEvent_UpdateHpBar,
	eStageEvent_PostHitTest,
	eStageEvent_PostUpdate,

	eStageEvent_Count,
};

enum
{
	eStageUpdatePhase_BeforeHitTest,
	eStageUpdatePhase_HitTest,
	eStageUpdatePhase_AfterHitTest,
};

enum
{
	eGUIOption_Crosshair = 1,
	eGUIOption_HpBar = 2,
};

class CWorld;
class CMasterLevel;
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
	void MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result, float fRad = 0 );
	void MultiHitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CReference<CEntity> >& result, vector<SHitTestResult>* pResult = NULL );
	CEntity* Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType = eEntityHitType_Count, SRaycastResult* pResult = NULL );
	void MultiRaycast( const CVector2& begin, const CVector2& end, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult = NULL );
	CEntity* SweepTest( SHitProxy* pHitProxy, const CMatrix2D& trans, const CVector2& sweepOfs, EEntityHitType hitType = eEntityHitType_Count, SRaycastResult* pResult = NULL, bool bIgnoreInverseNormal = false );
	CEntity* SweepTest( SHitProxy* pEntity, const CMatrix2D& trans, const CVector2& sweepOfs, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult* pResult = NULL, bool bIgnoreInverseNormal = false );
	CEntity* SweepTest( CEntity* pEntity, const CMatrix2D& trans, const CVector2& sweepOfs, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult* pResult = NULL, bool bIgnoreInverseNormal = false );
	void MultiSweepTest( SHitProxy* pHitProxy, const CMatrix2D& trans, const CVector2& sweepOfs, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult = NULL );

	void RegisterStageEvent( uint32 nEvent, CTrigger* pTrigger ) { m_events.Register( nEvent, pTrigger ); }
	void RegisterTick( uint32 nTime, CTrigger* pTrigger ) { m_tick.Register( nTime, pTrigger ); }
	void TriggerEvent( uint32 nEvent ) { m_events.Trigger( nEvent, NULL ); }
	void Update();
	void OnPostProcess( class CPostProcessPass* pPass );

	uint8 GetUpdatePhase() { return m_nUpdatePhase; }
	CEntity* GetRoot() { return m_pEntityRoot; }
	CMasterLevel* GetMasterLevel() { return m_pMasterLevel; }
	CCamera2D& GetCamera() { return m_camera; }
	CVector2 GetOrigCamSize() { return m_origCamSize; }
	float GetPixelScale() { return m_fCamScale * 2; }

	void AddStartPoint( CEntity* pEntity ) { m_mapStartPoints[pEntity->GetName().c_str()] = pEntity; }
	CEntity* GetStartPoint( const char* szName )
	{
		auto itr = m_mapStartPoints.find( szName );
		if( itr != m_mapStartPoints.end() )
			return itr->second;
		return NULL;
	}

	CHitTestMgr& GetHitTestMgr() { return m_hitTestMgr; }
	float GetElapsedTimePerTick() { return 1.0f / 60; }
private:
	CWorld* m_pWorld;
	SStageContext* m_pContext;
	bool m_bStarted;
	uint8 m_nUpdatePhase;
	bool m_bLight;
	CHitTestMgr m_hitTestMgr;
	CReference<CEntity> m_pEntityRoot;
	CReference<CMasterLevel> m_pMasterLevel;
	CCamera2D m_camera;
	CVector2 m_origCamSize;
	float m_fCamScale;
	map<string, CReference<CEntity> > m_mapStartPoints;
	SStageEnterContext m_enterContext;

	TClassTrigger1<CStage, CPostProcessPass*> m_onPostProcess;
	CEventTrigger<eStageEvent_Count> m_events;
	CTimedTrigger<4096> m_tick;
};