#pragma once
#include "Character.h"
#include "Common/PriorityQueue.h"

class INavigationProvider
{
public:
	virtual uint8 GetNavigationData( const TVector2<int32>& pos ) = 0;
	virtual TRectangle<int32> GetMapRect() = 0;
	virtual CVector2 GetGridSize() = 0;
};

class CNavigationUnit
{
public:
	CNavigationUnit() : m_mapRect( 0, 0, 0, 0 ), m_gridSize( 0, 0 ), m_bCanFly( false ) {}
	void Set( bool bCanFly, float fMaxScanDist, uint32 nGridsPerStep );
	void Reset();
	void Clear();
	CEntity* GetTarget() { return m_pTarget; }
	void SetTarget( CEntity* pEntity );
	void Step( CCharacter* pChar );

	struct SGridData : public TPriorityQueueNode<SGridData, float>
	{
		SGridData() : nType( -1 ), bClosed( false ), bInserted( false ), par( -1, -1 ), fDist( FLT_MAX ), fAStarDist( FLT_MAX ) {}
		virtual float GetPriority() { return fDist + fAStarDist; }
		uint8 nType;
		bool bClosed;
		bool bInserted;
		TVector2<int32> pos;
		TVector2<int32> par;
		float fDist;
		float fAStarDist;
	};
	inline SGridData& GetGrid( const TVector2<int32>& pos ) { return m_vecGrid[pos.x + pos.y * m_mapRect.width]; }
	inline TVector2<int32> GetGridByPos( const CVector2& pos );
	inline CRectangle GridToRect( const TVector2<int32>& grid );
	inline SGridData& CacheGridData( const TVector2<int32>& pos, INavigationProvider* pProvider )
	{
		auto& grid = GetGrid( pos );
		if( grid.nType == INVALID_8BITID )
		{
			grid.nType = pProvider->GetNavigationData( pos );
			grid.pos = pos;
			grid.par = TVector2<int32>( -1, -1 );
			grid.fDist = FLT_MAX;
			grid.fAStarDist = m_pTarget ? sqrt( ( grid.pos - m_curTargetGrid ).Length2() ) : 0;
			m_visited.push_back( grid.pos );
		}
		return grid;
	}

	void BuildPath( SGridData* pGridData, CCharacter* pCharacter );
	void ClearPath();
	CVector2 FollowPath( CCharacter* pCharacter );
	bool HasPath() { return m_curPath.size() > 0; }

	void RegisterVisitGridEvent( CTrigger* pTrigger ) { m_trigger.Register( 0, pTrigger ); }
	void RegisterFindTargetEvent( CTrigger* pTrigger ) { m_trigger.Register( 1, pTrigger ); }

	static CNavigationUnit* Alloc();
	static void Free( CNavigationUnit* pUnit );
private:
	vector<SGridData> m_vecGrid;
	TPriorityQueue<SGridData, float> m_q;
	vector<TVector2<int32> > m_visited;
	vector<TVector2<int32> > m_curPath;

	TRectangle<int32> m_mapRect;
	CVector2 m_gridSize;

	CReference<CEntity> m_pTarget;
	TVector2<int32> m_curSrcGrid;
	TVector2<int32> m_curTargetGrid;

	bool m_bCanFly;
	float m_fMaxScanDist;
	uint32 m_nGridsPerStep;

	CEventTrigger<2> m_trigger;
};