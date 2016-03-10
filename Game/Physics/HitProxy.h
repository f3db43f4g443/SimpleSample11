#pragma once
#include "Common/Math3D.h"
#include "Common/LinkList.h"
#include <vector>
using namespace std;

#define MAX_POLYGON_VERTEX_COUNT 8

enum
{
	eHitProxyType_Circle,
	eHitProxyType_Polygon,
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
};

class CHitProxy;
struct SRaycastResult
{
	CHitProxy* pHitProxy;
	CVector2 hitPoint;
	CVector2 normal;
	float fDist;
};

struct SHitProxy
{
	SHitProxy() : m_pProxyGrid( NULL ), pOwner( NULL ), bound( 0, 0, 0, 0 ) {}
	static bool HitTest( SHitProxy* a, SHitProxy* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult = NULL );
	bool Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult = NULL );

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
	bool Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult = NULL );
	float fRadius;
	CVector2 center;
};

struct SHitProxyPolygon : public SHitProxy
{
	SHitProxyPolygon() { nType = eHitProxyType_Polygon; }
	bool Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult = NULL );
	uint32 nVertices;
	CVector2 vertices[MAX_POLYGON_VERTEX_COUNT];
	CVector2 normals[MAX_POLYGON_VERTEX_COUNT];

	void CalcNormals();
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
public:
	CHitProxy() : m_pMgr( NULL ), m_pHitProxies( NULL ), m_pManifolds( NULL ), m_bDirty( false ), m_bBulletMode( false ) {}
	virtual ~CHitProxy();
	void SetDirty() { m_bDirty = true; }
	void SetBulletMode( bool bBulletMode );

	void AddCircle( float fRadius, const CVector2 &center );
	void AddRect( const CRectangle& rect );
	void AddPolygon( uint32 nVertices, const CVector2* vertices );
	void AddProxy( const struct SHitProxyData& data );

	bool HitTest( CHitProxy* pOther, SHitTestResult* pResult = NULL );
	bool HitTest( SHitProxy* pProxy1, const CMatrix2D& transform, SHitTestResult* pResult = NULL );
	bool Raycast( const CVector2& begin, const CVector2& end, SRaycastResult* pResult = NULL );
	virtual const CMatrix2D& GetGlobalTransform() = 0;
protected:
	CHitTestMgr* m_pMgr;
	bool m_bDirty;
	bool m_bBulletMode;
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
	static const uint32 nGridSize = 32;
	
	void Add( CHitProxy* pProxy );
	void Remove( CHitProxy* pProxy );
	void ReAdd( CHitProxy* pProxy );
	void ClearManifolds( CHitProxy* pProxy );

	void Update();
	void HitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CHitProxy*>& vecResult, vector<SHitTestResult>* pResult = NULL );
	void Raycast( const CVector2& begin, const CVector2& end, vector<SRaycastResult>& vecResult );
private:
	void Update( CHitProxy* pHitProxy, vector<CHitProxy*>& vecOverlaps );
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