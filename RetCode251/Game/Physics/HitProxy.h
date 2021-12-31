#pragma once
#include "Common/Math3D.h"
#include "Common/LinkList.h"
#include "Common/BufFile.h"
#include <vector>
using namespace std;

#define MAX_POLYGON_VERTEX_COUNT 8

enum
{
	eHitProxyType_Circle,
	eHitProxyType_Polygon,
	eHitProxyType_Edge,

	eHitProxyType_Count
};

struct SHitProxy;
struct SHitProxyGrid
{
	SHitProxy* pHitProxy;
	LINK_LIST( SHitProxyGrid, InProxy );
	LINK_LIST( SHitProxyGrid, InGrid );
};

struct SHitTestResult
{
	CVector2 hitPoint1, hitPoint2;
	CVector2 normal;
	int32 nUser;
};

class CHitProxy;
struct SRaycastResult
{
	SRaycastResult() : pHitProxy( NULL ), nUser( 0 ), bThresholdFail( false ) {}
	CHitProxy* pHitProxy;
	CVector2 hitPoint;
	CVector2 normal;
	float fDist;
	int32 nUser;
	bool bThresholdFail;
};

struct SHitProxy
{
	SHitProxy() : m_pProxyGrid( NULL ), pOwner( NULL ), bound( 0, 0, 0, 0 ) {}
	bool operator == ( const SHitProxy& rhs ) const;

	static bool HitTest( SHitProxy* a, SHitProxy* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult = NULL );
	bool Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult = NULL );
	static int8 SweepTest( SHitProxy* a, SHitProxy* b, const CMatrix2D& transA, const CMatrix2D& transB, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult* pResult = NULL );
	static bool Contain( SHitProxy* a, SHitProxy* b, const CMatrix2D& transA, const CMatrix2D& transB );

	void CalcBound( const CMatrix2D& trans, CRectangle& newBound );
	void CalcBoundGrid( const CMatrix2D& trans );

	uint8 nType;
	TRectangle<int32> bound;

	CHitProxy* pOwner;

	LINK_LIST( SHitProxy, HitProxy );
	LINK_LIST_HEAD( m_pProxyGrid, SHitProxyGrid, InProxy );
};

struct SHitProxyCircle : public SHitProxy
{
	SHitProxyCircle() { nType = eHitProxyType_Circle; }
	SHitProxyCircle( const struct SClassCreateContext& context ) { nType = eHitProxyType_Circle; }
	bool operator == ( const SHitProxyCircle& rhs ) const;
	bool Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult = NULL );
	float fRadius;
	CVector2 center;
};

struct SHitProxyPolygon : public SHitProxy
{
	SHitProxyPolygon() { nType = eHitProxyType_Polygon; }
	SHitProxyPolygon( const CRectangle& rect );
	SHitProxyPolygon( const struct SClassCreateContext& context ) { nType = eHitProxyType_Polygon; }
	bool operator == ( const SHitProxyPolygon& rhs ) const;
	bool Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult = NULL );
	uint32 nVertices;
	CVector2 vertices[MAX_POLYGON_VERTEX_COUNT];
	CVector2 normals[MAX_POLYGON_VERTEX_COUNT];

	void CalcNormals();
};

struct SHitProxyEdge : public SHitProxy
{
	SHitProxyEdge() { nType = eHitProxyType_Edge; }
	bool operator == ( const SHitProxyEdge& rhs ) const;
	bool Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult = NULL );

	CVector2 vert0, vert1;
	SHitProxyEdge* pPrev;
	SHitProxyEdge* pNext;
};

class CHitProxy;
struct SHitProxyManifold
{
	CHitProxy* pOtherHitProxy;
	SHitProxyManifold* pOther;
	CVector2 hitPoint;
	CVector2 normal;
	LINK_LIST( SHitProxyManifold, Manifold );
};

class CHitProxy
{
	friend class CHitTestMgr;
	friend void RegisterGameClasses();
	enum
	{
		eVersion_Cur = 0,
	};
public:
	CHitProxy() : bIgnoreFlag( false ), m_pMgr( NULL ), m_pHitProxies( NULL ), m_pManifolds( NULL ), m_bLastPosValid( false ), m_velocity( 0, 0 )
		, m_bDirty( false ), m_bBulletMode( false ), m_bTransparent( false ) {}
	CHitProxy( const struct SClassCreateContext& context ) : m_pMgr( NULL ), m_pManifolds( NULL ), m_bLastPosValid( false ), m_bDirty( false ) {}
	virtual ~CHitProxy();

	bool operator == ( const CHitProxy& rhs ) const;

	void SetDirty() { m_bDirty = true; }
	void SetBulletMode( bool bBulletMode );
	bool IsTransparent() { return m_bTransparent; }
	void SetTransparent( bool bTransparent );

	const CVector2& GetLastPos() { return m_lastPos; }
	void SetLastPos( const CVector2& pos ) { m_lastPos = pos; }
	void SetCurPos( const CVector2& pos ) { m_curPos = pos; }
	const CVector2& GetVelocity() { return m_velocity; }
	void SetVelocity( const CVector2& velocity ) { m_velocity = velocity; }

	SHitProxyCircle* AddCircle( float fRadius, const CVector2 &center );
	SHitProxyPolygon* AddRect( const CRectangle& rect );
	SHitProxyPolygon* AddPolygon( uint32 nVertices, const CVector2* vertices );
	SHitProxyEdge* AddEdge( const CVector2& begin, const CVector2& end );
	void AddProxy( const struct SHitProxyData& data );
	void RemoveProxy( SHitProxy* pHitProxy );

	void PackData( class CBufFile& buf, bool bWithMetaData ) const;
	void UnpackData( class IBufReader& buf, bool bWithMetaData, void* pContext );
	bool DiffData( const CHitProxy& obj0, class CBufFile& buf ) const;
	void PatchData( class IBufReader& buf, void* pContext );
	
	CHitTestMgr* HitTestMgr() { return m_pMgr; }
	void CalcBounds();
	bool HitTest( const CVector2& pos );
	bool HitTest( CHitProxy* pOther, const CMatrix2D& transform, const CMatrix2D& transform1, SHitTestResult* pResult = NULL );
	bool HitTest( CHitProxy* pOther, SHitTestResult* pResult = NULL );
	bool HitTest( SHitProxy* pProxy1, const CMatrix2D& transform, SHitTestResult* pResult = NULL );
	bool Raycast( const CVector2& begin, const CVector2& end, SRaycastResult* pResult = NULL );
	bool SweepTest( SHitProxy* pProxy1, const CMatrix2D& transform, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult* pResult = NULL );
	virtual CMatrix2D GetGlobalTransform() = 0;

	bool bIgnoreFlag;
private:
	CHitTestMgr* m_pMgr;

	CVector2 m_lastPos;
	CVector2 m_curPos;
	CVector2 m_velocity;
	bool m_bLastPosValid;
protected:
	bool m_bDirty;
	bool m_bBulletMode;
	bool m_bTransparent;
	LINK_LIST( CHitProxy, HitProxy );
	LINK_LIST_HEAD( m_pHitProxies, SHitProxy, HitProxy );
	LINK_LIST_HEAD( m_pManifolds, SHitProxyManifold, Manifold )
};

class CHitTestMgr
{
public:
	CHitTestMgr() : m_pHitProxy( NULL ), m_pHitProxyBulletMode( NULL ), m_grids( NULL ), m_size( 0, 0, 0, 0 ) {}
	struct SGrid
	{
		friend class CHitTestMgr;
		LINK_LIST_HEAD( m_pProxyGrid, SHitProxyGrid, InGrid );
	};
	SGrid* GetGrid( const TVector2<int32>& grid );
	static const uint32 nGridSize = 32;
	float GetElapsedTimePerTick() { return 1.0f / 60; }
	float GetElapsedTimePerTickInv() { return 60; }
	
	void Add( CHitProxy* pProxy );
	void Remove( CHitProxy* pProxy );
	void ReAdd( CHitProxy* pProxy );
	void ClearManifolds( CHitProxy* pProxy );
	void ClearNoBulletModeManifolds( CHitProxy* pProxy );

	void Update();
	void Update( CHitProxy* pHitProxy );
	void CalcBound( SHitProxy* pProxy, const CMatrix2D& transform );
	void HitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CHitProxy*>& vecResult, vector<SHitTestResult>* pResult = NULL );
	void Raycast( const CVector2& begin, const CVector2& end, vector<SRaycastResult>& vecResult );
	void SweepTest( SHitProxy* pProxy, const CMatrix2D& transform, const CVector2& sweepOfs, float fSideThreshold, vector<SRaycastResult>& vecResult, bool bSort = true );
private:
	void Update( CHitProxy* pHitProxy, vector<CHitProxy*>* pVecOverlaps, bool bUpdateLastPos = false );
	void UpdateBound( SHitProxy* pProxy, const TRectangle<int32>& newRect, vector<CHitProxy*>* pOverlaps = NULL, bool bInsertGrids = true );
	void Reserve( const TRectangle<int32>& newRect );
	SHitProxyGrid* AllocProxyGrid();
	void FreeProxyGrid( SHitProxyGrid* pProxyGrid );
	SHitProxyManifold* AllocManifold();
	void FreeManifold( SHitProxyManifold* pManifold );

	SGrid* m_grids;
	TRectangle<int32> m_size;

	CHitProxy* m_pHitProxyBulletMode;
	LINK_LIST_HEAD( m_pHitProxy, CHitProxy, HitProxy );
};