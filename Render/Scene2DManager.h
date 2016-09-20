#pragma once

#include "Element2D.h"
#include "Drawable2D.h"
#include "RenderObject2D.h"
#include "SortedList.h"
#include "Camera2D.h"
#include "RenderContext2D.h"
#include "RenderSystem.h"
#include "Footprint.h"

#include <set>
#include <map>
using namespace std;

class CScene2DManager
{
public:
	CScene2DManager();
	~CScene2DManager();

	void AddDrawable( CDrawable2D* pDrawable );
	void DeleteDrawable( CDrawable2D* pDrawable );

	void AddDirtyRenderObject( CRenderObject2D* pRenderObject );
	void AddDirtyAABB( CRenderObject2D* pRenderObject );

	void AddActiveCamera( CCamera2D* pCamera, CRenderObject2D* pRoot );
	void RemoveActiveCamera( CCamera2D* pCamera );

	void UpdateDirty();
	void Render( CRenderContext2D& context );
	void Render( CRenderContext2D& context, CCamera2D* pCamera, CRenderObject2D* pRoot, SRenderGroup* pRenderGroup );
	void Flush( CRenderContext2D& context );
	void Flush( CRenderContext2D& context, CCamera2D* pCamera, CRenderObject2D* pRoot, SRenderGroup* pRenderGroup );

	void AddElement( CElement2D* pElement );

	void AddFootprintMgr( CFootprintMgr* pMgr ) { Insert_FootprintMgr( pMgr ); }
	void RemoveFootprintMgr( CFootprintMgr* pMgr ) { pMgr->Clear(); Remove_FootprintMgr( pMgr ); }

	CRenderObject2D* GetRoot() { return m_pRoot; }

	enum
	{
		eEvent_BeforeRender,

		eEvent_Count,
	};
	void Register( uint32 iEvent, CTrigger* pTrigger ) { m_trigger.Register( iEvent, pTrigger ); }
	void Trigger( uint32 iEvent ) { m_trigger.Trigger( iEvent, NULL ); }

	static CScene2DManager* GetGlobalInst();
private:
	void _init();
	void _flush( CRenderContext2D& context, uint32 nGroup, CCamera2D* pCamera, CRenderObject2D* pRoot, SRenderGroup* pRenderGroup );

	set<CDrawable2D*> m_pDrawables;
	TSortedList<CReference<CRenderObject2D>, CRenderObject2D::PointerDepth> m_dirtySceneNodes;
	TSortedList<CReference<CRenderObject2D>, CRenderObject2D::PointerDepth> m_dirtyAABBSceneNodes;
	CReference<CRenderObject2D> m_pRoot;

	struct SCameraLess
	{	// functor for operator<
		bool operator()(const CCamera2D* _Left, const CCamera2D* _Right) const
		{
			if( _Left->GetPriority() < _Right->GetPriority() )
				return true;
			if( _Left->GetPriority() > _Right->GetPriority() )
				return false;
			return (_Left < _Right);
		}
	};

	struct SCameraContext
	{
		CReference<CRenderObject2D> pRoot;
		SRenderGroup renderGroup[2];
	};
	map<CCamera2D*, SCameraContext, SCameraLess> m_activeCams;

	CEventTrigger<eEvent_Count> m_trigger;

	LINK_LIST_REF_HEAD( m_pFootprintMgrs, CFootprintMgr, FootprintMgr );
	LINK_LIST_HEAD( m_pAutoUpdateAnimObject, CRenderObject2D, AutoUpdateAnimObject );
};
