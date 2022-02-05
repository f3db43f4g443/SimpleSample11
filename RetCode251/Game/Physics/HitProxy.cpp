#include "StdAfx.h"
#include "HitProxy.h"
#include "HitProxyData.h"
#include "Common/FixedSizeAllocator.h"
#include <algorithm>

bool HitTestCircles( SHitProxyCircle* a, SHitProxyCircle* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult )
{
	CVector2 centerA = transA.MulVector2Pos( a->center );
	CVector2 centerB = transB.MulVector2Pos( b->center );
	CVector2 dCenter = centerB - centerA;
	float d = a->fRadius + b->fRadius;
	float l2 = dCenter.Dot( dCenter );
	if( l2 >= d * d )
		return false;
	if( pResult )
	{
		float l = sqrt( l2 );
		if( l > 0.001f )
			dCenter = dCenter * ( 1.0f / l );
		else
			dCenter = CVector2( 0, 1 );
		pResult->hitPoint1 = centerA + dCenter * a->fRadius;
		pResult->hitPoint2 = centerB - dCenter * b->fRadius;
		pResult->normal = pResult->hitPoint1 - pResult->hitPoint2;
	}
	return true;
}

int8 SweepTestCircles( SHitProxyCircle* a, SHitProxyCircle* b, const CMatrix2D& transA, const CMatrix2D& transB, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult* pResult = NULL )
{
	CVector2 centerA = transA.MulVector2Pos( a->center );

	SHitProxyCircle temp;
	temp.center = b->center;
	temp.fRadius = a->fRadius + b->fRadius;
	CMatrix2D mat;
	mat.Identity();
	bool bHit = temp.Raycast( centerA, centerA + sweepOfs, transB, pResult );
	if( bHit && pResult )
	{
		CVector2 sweepDir;
		sweepDir.Normalize();
		pResult->hitPoint = pResult->hitPoint + sweepDir * a->fRadius;
	}
	return bHit;
}

bool HitTestCircleAndPolygon( SHitProxyCircle* a, SHitProxyPolygon* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult )
{
	CVector2 center = transA.MulVector2Pos( a->center );
	center = transB.MulTVector2PosNoScale( center );

	// Find the min separating edge.
	int32 normalIndex = 0;
	float separation = -FLT_MAX;
	float fRadius = a->fRadius;
	uint32 nVertices = b->nVertices;
	CVector2* vertices = b->vertices;
	CVector2* normals = b->normals;

	for( int32 i = 0; i < nVertices; ++i )
	{
		float s = normals[i].Dot( center - vertices[i] );

		if( s > fRadius )
			return false;

		if( s > separation )
		{
			separation = s;
			normalIndex = i;
		}
	}

	// Vertices that subtend the incident face.
	int32 vertIndex1 = normalIndex;
	int32 vertIndex2 = vertIndex1 + 1 < nVertices ? vertIndex1 + 1 : 0;
	CVector2 v1 = vertices[vertIndex1];
	CVector2 v2 = vertices[vertIndex2];

	// If the center is inside the polygon ...
	if( separation <= 0 )
	{
		if( pResult )
		{
			pResult->normal = transB.MulVector2Dir( normals[normalIndex] * ( separation - fRadius ) );
			pResult->hitPoint1 = transB.MulVector2Pos( center - normals[normalIndex] * fRadius );
			pResult->hitPoint2 = pResult->hitPoint1 - pResult->normal;
		}
		return true;
	}

	// Compute barycentric coordinates
	CVector2 dv1 = ( center - v1 );
	CVector2 dv2 = ( center - v2 );
	float u1 = dv1.Dot( v2 - v1 );
	float u2 = dv2.Dot( v1 - v2 );
	if( u1 <= 0.0f )
	{
		float l2 = dv1.Dot( dv1 );
		if( l2 >= fRadius * fRadius )
			return false;
		if( pResult )
		{
			float l = sqrt( l2 );
			if( l > 0.001f )
				pResult->normal = transB.MulVector2Dir( dv1 * ( ( l - fRadius ) / l ) );
			else
				pResult->normal = transB.MulVector2Dir( normals[normalIndex] * ( separation - fRadius ) );
			pResult->hitPoint2 = transB.MulVector2Pos( v1 );
			pResult->hitPoint1 = pResult->hitPoint2 + pResult->normal;
		}
		return true;
	}
	else if( u2 <= 0.0f )
	{
		float l2 = dv2.Dot( dv2 );
		if( l2 >= fRadius * fRadius )
			return false;
		if( pResult )
		{
			float l = sqrt( l2 );
			if( l > 0.001f )
				pResult->normal = transB.MulVector2Dir( dv2 * ( ( l - fRadius ) / l ) );
			else
				pResult->normal = transB.MulVector2Dir( normals[normalIndex] * ( separation - fRadius ) );
			pResult->hitPoint2 = transB.MulVector2Pos( v2 );
			pResult->hitPoint1 = pResult->hitPoint2 + pResult->normal;
		}
		return true;
	}
	else
	{
		if( pResult )
		{
			pResult->normal = transB.MulVector2Dir( normals[normalIndex] * ( separation - fRadius ) );
			pResult->hitPoint1 = transB.MulVector2Pos( center - normals[normalIndex] * fRadius );
			pResult->hitPoint2 = pResult->hitPoint1 - pResult->normal;
		}
		return true;
	}
}

int8 SweepTestCircleAndPolygon( SHitProxyCircle* a, SHitProxyPolygon* b, const CMatrix2D& transA, const CMatrix2D& transB, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult* pResult = NULL )
{
	CVector2 worldCenter = transA.MulVector2Pos( a->center );
	CVector2 center = transB.MulTVector2PosNoScale( worldCenter );

	// Find the min separating edge.
	int32 normalIndex = 0;
	float separation = -FLT_MAX;
	float fRadius = a->fRadius;
	uint32 nVertices = b->nVertices;
	CVector2* vertices = b->vertices;
	CVector2* normals = b->normals;

	for( int32 i = 0; i < nVertices; ++i )
	{
		float s = normals[i].Dot( center - vertices[i] );

		if( s > separation )
		{
			separation = s;
			normalIndex = i;
		}
	}

	// Vertices that subtend the incident face.
	int32 vertIndex1 = normalIndex;
	int32 vertIndex2 = vertIndex1 + 1 < nVertices ? vertIndex1 + 1 : 0;
	CVector2 v1 = vertices[vertIndex1] + normals[normalIndex] * fRadius;
	CVector2 v2 = vertices[vertIndex2] + normals[normalIndex] * fRadius;

	// If the center is inside the polygon ...
	if( separation <= 0 )
	{
		if( pResult )
		{
			pResult->normal = transB.MulVector2Dir( normals[normalIndex] );
			pResult->fDist = 0;
			pResult->hitPoint = transB.MulVector2Pos( center - normals[normalIndex] * fRadius );
		}
		return 1;
	}

	CVector2 sweepDir = sweepOfs;
	float fMaxDist = sweepDir.Normalize();
	CVector2 dir = transB.MulTVector2DirNoScale( sweepDir );
	// Compute barycentric coordinates
	CVector2 dv1 = ( center - v1 );
	CVector2 dv2 = ( center - v2 );
	float u1 = dv1.Dot( v2 - v1 );
	float u2 = dv2.Dot( v1 - v2 );
	if( u1 > 0.0f && u2 > 0.0f )
	{
		if( separation <= fRadius )
		{
			if( pResult )
			{
				pResult->normal = transB.MulVector2Dir( normals[normalIndex] );
				pResult->fDist = 0;
				pResult->hitPoint = transB.MulVector2Pos( center - normals[normalIndex] * fRadius );
			}
			return 1;
		}
	}

	SRaycastResult tempResult;
	tempResult.fDist = fMaxDist;
	bool bHit = false;
	for( int i = 0; i < nVertices; i++ )
	{
		vertIndex1 = i;
		vertIndex2 = vertIndex1 + 1 < nVertices ? vertIndex1 + 1 : 0;
		v1 = vertices[vertIndex1] + normals[i] * fRadius;
		v2 = vertices[vertIndex2] + normals[i] * fRadius;

		{
			CVector2 p0 = transB.MulVector2Pos( v1 );
			CVector2 p1 = transB.MulVector2Pos( v2 );

			CVector2 a = sweepOfs;
			CVector2 b = p1 - p0;
			CVector2 c = worldCenter - p0;
			float k = ( c.y * b.x - c.x * b.y ) / ( a.x * b.y - a.y * b.x );
			float k1 = ( c.y * a.x - c.x * a.y ) / ( a.x * b.y - a.y * b.x );
			if( k >= -fRadius / fMaxDist && k <= 1 && k1 >= 0 && k1 <= 1 )
			{
				k = Max( 0.0f, k );
				auto normal = CVector2( b.y, -b.x );
				if( normal.Dot( a ) <= 0 )
				{
					if( pResult )
					{
						pResult->hitPoint = worldCenter + a * k;
						pResult->normal = normal;
						pResult->normal.Normalize();
						pResult->fDist = k * fMaxDist;
						if( pResult->fDist < tempResult.fDist )
						{
							bHit = true;
							tempResult = *pResult;
						}
					}
					else
						return 1;
				}
			}
		}

		SHitProxyCircle temp;
		temp.center = vertices[vertIndex2];
		temp.fRadius = fRadius;
		if( temp.Raycast( worldCenter, worldCenter + sweepOfs, transB, pResult ) )
		{
			if( !pResult )
				return 1;
			if( pResult->fDist < tempResult.fDist )
			{
				bHit = true;
				tempResult = *pResult;
			}
		}
	}

	if( pResult && bHit )
	{
		*pResult = tempResult;
		pResult->hitPoint = pResult->hitPoint + sweepDir * a->fRadius;
	}
	return bHit;
}

bool HitTestCircleAndEdge( SHitProxyCircle* a, SHitProxyEdge* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult )
{
	CVector2 center = transA.MulVector2Pos( a->center );
	center = transB.MulTVector2PosNoScale( center );

	float fRadius = a->fRadius;
	CVector2 norm = b->vert1 - b->vert0;
	norm = CVector2( norm.y, -norm.x );
	norm.Normalize();
	float c = b->vert0.Dot( norm );
	float s = center.Dot( norm ) - c;
	if( s >= fRadius || s <= -fRadius )
		return false;

	CVector2 d = center - b->vert0;
	float dot = d.Dot( b->vert1 - b->vert0 );
	if( dot < 0 )
	{
		float l2 = d.Dot( d );
		if( l2 >= fRadius * fRadius )
			return false;
		if( b->pPrev )
		{
			if( d.Dot( b->pPrev->vert0 - b->pPrev->vert1 ) >= 0 )
				return false;
		}
		if( pResult )
		{
			float l = sqrt( l2 );
			if( l > 0.001f )
				pResult->normal = transB.MulVector2Dir( d * ( ( l - fRadius ) / l ) );
			else
				pResult->normal = transB.MulVector2Dir( norm * ( s > 0 ? s - fRadius : s + fRadius ) );
			pResult->hitPoint2 = transB.MulVector2Pos( b->vert0 );
			pResult->hitPoint1 = pResult->hitPoint2 + pResult->normal;
		}
		return true;
	}
	
	d = center - b->vert1;
	dot = d.Dot( b->vert0 - b->vert1 );
	if( dot < 0 )
	{
		float l2 = d.Dot( d );
		if( l2 >= fRadius * fRadius )
			return false;
		if( b->pNext )
		{
			if( d.Dot( b->pNext->vert1 - b->pNext->vert0 ) >= 0 )
				return false;
		}
		if( pResult )
		{
			float l = sqrt( l2 );
			if( l > 0.001f )
				pResult->normal = transB.MulVector2Dir( d * ( ( l - fRadius ) / l ) );
			else
				pResult->normal = transB.MulVector2Dir( norm * ( s > 0 ? s - fRadius : s + fRadius ) );
			pResult->hitPoint2 = transB.MulVector2Pos( b->vert1 );
			pResult->hitPoint1 = pResult->hitPoint2 + pResult->normal;
		}
		return true;
	}

	pResult->normal = transB.MulVector2Dir( norm * ( s > 0 ? s - fRadius : s + fRadius ) );
	pResult->hitPoint2 = transB.MulVector2Pos( center - norm * fRadius );
	pResult->hitPoint1 = pResult->hitPoint2 + pResult->normal;
	return true;
}

bool _HitTestPolygons( CVector2 vertices1[MAX_POLYGON_VERTEX_COUNT], CVector2 vertices2[MAX_POLYGON_VERTEX_COUNT],
	uint32 nVertices1, uint32 nVertices2, SHitTestResult* pResult )
{
	//Find the leftmost points.
	uint32 n1 = 0;
	float xMin = FLT_MAX;
	for( int i = 0; i < nVertices1; i++ )
	{
		if( vertices1[i].x < xMin )
		{
			xMin = vertices1[i].x;
			n1 = i;
		}
	}
	uint32 n2 = 0;
	xMin = FLT_MAX;
	for( int i = 0; i < nVertices2; i++ )
	{
		if( vertices2[i].x < xMin )
		{
			xMin = vertices2[i].x;
			n2 = i;
		}
	}

	//Build and test the combined polygon
	CVector2 p0 = vertices1[n1] + vertices2[n2];
	CVector2 edge0( 0, -1 );
	float sMin = FLT_MAX;
	CVector2 hitPoint;
	CVector2 hitNormal;
	uint32 a1, b1, a2, b2;
	for( int i1 = 0, i2 = 0;; )
	{
		uint32 l1 = n1;
		uint32 l2 = n2;
		uint32 m1 = n1 + 1 < nVertices1 ? n1 + 1 : 0;
		uint32 m2 = n2 + 1 < nVertices2 ? n2 + 1 : 0;
		CVector2 edge;
		if( i1 < nVertices1 && i2 < nVertices2 )
		{
			CVector2 edge1 = vertices1[m1] - vertices1[n1];
			CVector2 edge2 = vertices2[m2] - vertices2[n2];
			float dot1 = edge0.Dot( edge1 ) / edge1.Length();
			float dot2 = edge0.Dot( edge2 ) / edge2.Length();
			if( dot1 >= dot2 )
			{
				edge = edge1;
				n1 = m1;
				i1++;
			}
			else
			{
				edge = edge2;
				n2 = m2;
				i2++;
			}
		}
		else if( i1 < nVertices1 )
		{
			edge = vertices1[m1] - vertices1[n1];
			n1 = m1;
			i1++;
		}
		else if( i2 < nVertices2 )
		{
			edge = vertices2[m2] - vertices2[n2];
			n2 = m2;
			i2++;
		}
		else
			break;

		CVector2 norm( edge.y, -edge.x );
		norm.Normalize();
		float s = p0.Dot( norm );
		if( s <= 0 )
			return false;
		if( pResult )
		{
			if( s < sMin )
			{
				sMin = s;
				hitNormal = norm * s;
				a1 = l1;
				a2 = l2;
				b1 = n1;
				b2 = n2;
			}
		}
		p0 = p0 + edge;
		edge0 = edge;
	}

	if( pResult )
	{
		pResult->normal = hitNormal;

		if( a1 == b1 )
		{
			pResult->hitPoint1 = vertices1[a1];
			pResult->hitPoint2 = vertices1[a1] - hitNormal;
		}
		else
		{
			pResult->hitPoint2 = vertices2[a2] * -1;
			pResult->hitPoint1 = vertices2[a2] * -1 + hitNormal;
		}
	}
	return true;
}

bool HitTestPolygons( SHitProxyPolygon* a, SHitProxyPolygon* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult )
{
	uint32 nVertices1 = a->nVertices;
	uint32 nVertices2 = b->nVertices;
	CVector2 vertices1[MAX_POLYGON_VERTEX_COUNT];
	CVector2 vertices2[MAX_POLYGON_VERTEX_COUNT];
	for( int i = 0; i < nVertices1; i++ )
	{
		vertices1[i] = transA.MulVector2Pos( a->vertices[i] );
	}
	for( int i = 0; i < nVertices2; i++ )
	{
		vertices2[i] = transB.MulVector2Pos( b->vertices[i] ) * -1;
	}
	return _HitTestPolygons( vertices1, vertices2, nVertices1, nVertices2, pResult );
}

int8 SweepTestPolygons( SHitProxyPolygon* a, SHitProxyPolygon* b, const CMatrix2D& transA, const CMatrix2D& transB, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult* pResult = NULL )
{
	uint32 nVertices1 = a->nVertices;
	uint32 nVertices2 = b->nVertices;
	CVector2 vertices1[MAX_POLYGON_VERTEX_COUNT];
	CVector2 vertices2[MAX_POLYGON_VERTEX_COUNT];
	for( int i = 0; i < nVertices1; i++ )
	{
		vertices1[i] = transA.MulVector2Pos( a->vertices[i] );
	}
	for( int i = 0; i < nVertices2; i++ )
	{
		vertices2[i] = transB.MulVector2Pos( b->vertices[i] ) * -1;
	}
	//We are now sweeping B to A, the dir need to be inversed
	CVector2 dir = sweepOfs * -1;
	float fMaxDist = dir.Normalize();
	if( fMaxDist <= 0 )
	{
		fMaxDist = 0;
	}

	//Find the leftmost points.
	uint32 n1 = 0;
	float xMin = FLT_MAX;
	for( int i = 0; i < nVertices1; i++ )
	{
		if( vertices1[i].x < xMin )
		{
			xMin = vertices1[i].x;
			n1 = i;
		}
	}
	uint32 n2 = 0;
	xMin = FLT_MAX;
	for( int i = 0; i < nVertices2; i++ )
	{
		if( vertices2[i].x < xMin )
		{
			xMin = vertices2[i].x;
			n2 = i;
		}
	}

	//Build and test the combined polygon
	CVector2 p0 = vertices1[n1] + vertices2[n2];
	CVector2 edge0( 0, -1 );
	float sMin = FLT_MAX;
	uint32 a1, b1, a2, b2;

	bool bar = false;
	float foo = 0;
	bool bOKAndNeedCheckThreshold = false;
	float fMin = FLT_MAX, fMax = -FLT_MAX;

	for( int i1 = 0, i2 = 0;; )
	{
		uint32 l1 = n1;
		uint32 l2 = n2;
		uint32 m1 = n1 + 1 < nVertices1 ? n1 + 1 : 0;
		uint32 m2 = n2 + 1 < nVertices2 ? n2 + 1 : 0;
		CVector2 edge;
		if( i1 < nVertices1 && i2 < nVertices2 )
		{
			CVector2 edge1 = vertices1[m1] - vertices1[n1];
			CVector2 edge2 = vertices2[m2] - vertices2[n2];
			float dot1 = edge0.Dot( edge1 ) / edge1.Length();
			float dot2 = edge0.Dot( edge2 ) / edge2.Length();
			if( dot1 >= dot2 )
			{
				edge = edge1;
				n1 = m1;
				i1++;
			}
			else
			{
				edge = edge2;
				n2 = m2;
				i2++;
			}
		}
		else if( i1 < nVertices1 )
		{
			edge = vertices1[m1] - vertices1[n1];
			n1 = m1;
			i1++;
		}
		else if( i2 < nVertices2 )
		{
			edge = vertices2[m2] - vertices2[n2];
			n2 = m2;
			i2++;
		}
		else
			break;


		float f0 = dir.x * p0.y - dir.y * p0.x;
		fMin = Min( fMin, f0 );
		fMax = Max( fMax, f0 );
		CVector2 p1 = p0 + edge;
		CVector2 norm( edge.y, -edge.x );
		norm.Normalize();
		float s = p0.Dot( norm );
		if( s < sMin )
		{
			sMin = s;
			if( pResult )
			{
				pResult->normal = norm * -1;
				a1 = l1;
				a2 = l2;
				b1 = n1;
				b2 = n2;
			}
		}

		if( !bOKAndNeedCheckThreshold )
		{
			if( s <= 0 && fMaxDist > 0 )
			{
				float f1 = dir.x * p1.y - dir.y * p1.x;
				bool bIntersect;

				if( f0 == 0 && f1 == 0 )
					bIntersect = false;
				else if( f0 == 0 )
				{
					if( !bar )
					{
						bar = true;
						foo = f1;
						bIntersect = false;
					}
					else
						bIntersect = ( foo < 0 ) != ( f1 < 0 );
				}
				else if( f1 == 0 )
				{
					if( !bar )
					{
						bar = true;
						foo = f0;
						bIntersect = false;
					}
					else
						bIntersect = ( f0 < 0 ) != ( foo < 0 );
				}
				else
					bIntersect = ( f0 < 0 ) != ( f1 < 0 );

				if( bIntersect )
				{
					float s1 = ( p0 + sweepOfs ).Dot( norm );
					if( s1 <= 0 )
						return 0;
					if( pResult )
					{
						CVector2 ofs = sweepOfs * ( -s / ( s1 - s ) );
						pResult->normal = norm * -1;
						pResult->fDist = ofs.Length();
						pResult->hitPoint = vertices1[l1] + ofs;
					}
					if( fSideThreshold )
						bOKAndNeedCheckThreshold = true;
					else
						return 1;
				}
			}
		}

		p0 = p1;
		edge0 = edge;
	}
	if( bOKAndNeedCheckThreshold )
	{
		if( fMin < -fSideThreshold && fMax > fSideThreshold )
			return 1;
		if( pResult )
			pResult->bThresholdFail = true;
		return -1;
	}

	if( sMin > 0 )
	{
		if( pResult )
		{
			if( a1 == b1 )
				pResult->hitPoint = vertices1[a1];
			else
				pResult->hitPoint = vertices2[a2] * -1 - pResult->normal * sMin;
			pResult->fDist = 0;
		}
		return sMin > fSideThreshold ? 1 : -1;
	}
	return 0;
}

bool HitTestPolygonAndEdge( SHitProxyPolygon* a, SHitProxyEdge* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult )
{
	uint32 nVertices1 = a->nVertices;
	uint32 nVertices2 = 2;
	CVector2 vertices1[MAX_POLYGON_VERTEX_COUNT];
	CVector2 vertices2[MAX_POLYGON_VERTEX_COUNT];
	for( int i = 0; i < nVertices1; i++ )
	{
		vertices1[i] = transA.MulVector2Pos( a->vertices[i] );
	}
	vertices2[0] = transB.MulVector2Pos( b->vert0 ) * -1;
	vertices2[1] = transB.MulVector2Pos( b->vert1 ) * -1;

	SHitTestResult tempResult;
	SHitTestResult* pRes = pResult ? pResult : &tempResult;
	if( !_HitTestPolygons( vertices1, vertices2, nVertices1, nVertices2, pRes ) )
		return false;
	if( b->pPrev && pRes->normal.Dot( b->pPrev->vert0 - b->pPrev->vert1 ) > 0 )
		return false;
	if( b->pNext && pRes->normal.Dot( b->pNext->vert1 - b->pNext->vert0 ) > 0 )
		return false;
	return true;
}


bool CircleContainCircle( SHitProxyCircle* a, SHitProxyCircle* b, const CMatrix2D& transA, const CMatrix2D& transB )
{
	float k = a->fRadius - b->fRadius;
	if( k < 0 )
		return false;
	auto d = transA.MulVector2Pos( a->center ) - transB.MulVector2Pos( b->center );
	return d.Length2() <= k * k;
}

bool CircleContainPolygon( SHitProxyCircle* a, SHitProxyPolygon* b, const CMatrix2D& transA, const CMatrix2D& transB )
{
	CVector2 center = transA.MulVector2Pos( a->center );
	center = transB.MulTVector2PosNoScale( center );
	for( int i = 0; i < b->nVertices; i++ )
	{
		auto d = b->vertices[i] - center;
		if( d.Length2() > a->fRadius * a->fRadius )
			return false;
	}
	return true;
}

bool PolygonContainCircle( SHitProxyPolygon* a, SHitProxyCircle* b, const CMatrix2D& transA, const CMatrix2D& transB )
{
	CVector2 center = transB.MulVector2Pos( b->center );
	center = transA.MulTVector2PosNoScale( center );
	for( int32 i = 0; i < a->nVertices; ++i )
	{
		float s = a->normals[i].Dot( center - a->vertices[i] );
		if( s > b->fRadius )
			return false;
	}
	return true;
}

bool PolygonContainPolygon( SHitProxyPolygon* a, SHitProxyPolygon* b, const CMatrix2D& transA, const CMatrix2D& transB )
{
	CVector2 vertices1[MAX_POLYGON_VERTEX_COUNT], vertices2[MAX_POLYGON_VERTEX_COUNT];
	for( int i = 0; i < a->nVertices; i++ )
		vertices1[i] = transA.MulVector2Pos( a->vertices[i] );
	for( int i = 0; i < b->nVertices; i++ )
		vertices2[i] = transB.MulVector2Pos( b->vertices[i] );

	for( int i = 0; i < a->nVertices; i++ )
	{
		int32 i1 = i;
		int32 i2 = i + 1 < a->nVertices ? i + 1 : 0;
		CVector2 edge = vertices1[i2] - vertices1[i1];
		CVector2 norm( edge.y, -edge.x );
		for( int j = 0; j < b->nVertices; j++ )
		{
			if( ( vertices2[j] - vertices1[i1] ).Dot( norm ) > 0 )
				return false;
		}
	}
	return true;
}


bool SHitProxyCircle::operator==( const SHitProxyCircle & rhs ) const
{
	return fRadius == rhs.fRadius && center == rhs.center;
}

bool SHitProxyCircle::Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult )
{
	CVector2 c = trans.MulVector2Pos( center );
	CVector2 dCenter = c - begin;
	if( dCenter.Dot( dCenter ) <= fRadius * fRadius )
	{
		if( pResult )
		{
			pResult->hitPoint = begin;
			pResult->normal = pResult->hitPoint - c;
			if( pResult->normal.Normalize() <= 0 )
				pResult->normal = CVector2( 1, 0 );
			pResult->fDist = 0;
		}
		return true;
	}

	CVector2 dir = end - begin;
	float fMaxDist = dir.Normalize();
	if( fMaxDist <= 0 )
		return false;

	float l1 = dCenter.Dot( dir );
	float r12 = dCenter.Dot( dCenter ) - l1 * l1;
	float l22 = fRadius * fRadius - r12;
	if( l22 < 0 )
		return false;
	float l = l1 - sqrt( l22 );
	if( l < 0 || l > fMaxDist )
		return false;
	if( pResult )
	{
		pResult->fDist = l;
		pResult->hitPoint = dir * l + begin;
		pResult->normal = pResult->hitPoint - c;
		pResult->normal.Normalize();
	}
	return true;
}

SHitProxyPolygon::SHitProxyPolygon( const CRectangle & rect )
{
	nType = eHitProxyType_Polygon;
	vertices[0] = CVector2( rect.x, rect.y );
	vertices[1] = CVector2( rect.x + rect.width, rect.y );
	vertices[2] = CVector2( rect.x + rect.width, rect.y + rect.height );
	vertices[3] = CVector2( rect.x, rect.y + rect.height );
	nVertices = 4;
	CalcNormals();
}

bool SHitProxyPolygon::operator==( const SHitProxyPolygon & rhs ) const
{
	if( nVertices != rhs.nVertices )
		return false;
	for( int i = 0; i < nVertices; i++ )
	{
		if( vertices[i] != rhs.vertices[i] )
			return false;
	}
	return true;
}

bool SHitProxyPolygon::Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult )
{
	CVector2 p0 = trans.MulTVector2PosNoScale( begin );
	CVector2 p1 = trans.MulTVector2PosNoScale( end );
	CVector2 dir = p1 - p0;
	float fMaxDist = dir.Normalize();
	if( fMaxDist <= 0 )
	{
		fMaxDist = 0;
	}

	int32 normalIndex = 0;
	float separation = FLT_MAX;
	bool bar = false;
	float foo = 0;
	for( int i = 0; i < nVertices; i++ )
	{
		CVector2 vert1 = vertices[i] - p0;
		CVector2 vert2 = vertices[i < nVertices - 1 ? i + 1 : 0] - p0;
		float s = normals[i].Dot( vert1 );
		
		if( s < separation )
		{
			separation = s;
			normalIndex = i;
		}

		if( s < 0 && fMaxDist > 0 )
		{
			float f0 = dir.x * vert1.y - dir.y * vert1.x;
			float f1 = dir.x * vert2.y - dir.y * vert2.x;
			bool bIntersect;

			if( f0 == 0 && f1 == 0 )
				bIntersect = false;
			else if( f0 == 0 )
			{
				if( !bar )
				{
					bar = true;
					foo = f1;
					bIntersect = false;
				}
				else
					bIntersect = foo * f1 < 0;
			}
			else if( f1 == 0 )
			{
				if( !bar )
				{
					bar = true;
					foo = f0;
					bIntersect = false;
				}
				else
					bIntersect = f0 * foo < 0;
			}
			else
				bIntersect = f0 * f1 < 0;

			if( !bIntersect )
				continue;

			float s1 = normals[i].Dot( vertices[i] - p1 );
			if( s1 < 0 )
				return false;
			if( pResult )
			{
				CVector2 ofs = ( p1 - p0 ) * ( -s / ( s1 - s ) );
				pResult->normal = trans.MulVector2Dir( normals[i] );
				pResult->fDist = ofs.Length();
				pResult->hitPoint = trans.MulVector2Pos( p0 + ofs );
			}
			/*float k = ( vert1.y * vert2.x - vert1.x * vert2.y ) / ( dir.x * ( vert1.y - vert2.y ) - dir.y * ( vert1.x - vert2.x ) );
			if( k < 0 || k > fMaxDist )
				return false;

			if( pResult )
			{
				pResult->normal = trans.MulVector2Dir( normals[i] );
				pResult->hitPoint = trans.MulVector2Pos( p0 + dir * k );
				pResult->fDist = k;
			}*/
			return true;
		}
	}

	if( separation >= 0 )
	{
		if( pResult )
		{
			pResult->normal = trans.MulVector2Dir( normals[normalIndex] );
			pResult->hitPoint = begin;
			pResult->fDist = 0;
		}
		return true;
	}
	return false;
}

bool SHitProxyEdge::operator==( const SHitProxyEdge & rhs ) const
{
	return vert0 == rhs.vert0 && vert1 == rhs.vert1;
	return false;
}

bool SHitProxyEdge::Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult )
{
	CVector2 p0 = trans.MulVector2Pos( vert0 );
	CVector2 p1 = trans.MulVector2Pos( vert1 );

	CVector2 a = end - begin;
	CVector2 b = p1 - p0;
	CVector2 c = begin - p0;
	float k = ( c.y * b.x - c.x * b.y ) / ( a.x * b.y - a.y * b.x );
	float k1 = ( c.y * a.x - c.x * a.y ) / ( a.x * b.y - a.y * b.x );
	if( k < 0 || k > 1 || k1 < 0 || k1 > 1 )
		return false;
	auto normal = CVector2( b.y, -b.x );
	if( normal.Dot( a ) > 0 )
		return false;
	if( pResult )
	{
		pResult->hitPoint = begin + a * k;
		pResult->normal = normal;
		pResult->normal.Normalize();
		pResult->fDist = k * a.Length();
	}
	return true;
}

bool SHitProxy::operator==( const SHitProxy& rhs ) const
{
	if( nType != rhs.nType )
		return false;

	if( nType == eHitProxyType_Circle )
		return *static_cast<const SHitProxyCircle*>( this ) == *static_cast<const SHitProxyCircle*>( &rhs );
	else if( nType == eHitProxyType_Polygon )
		return *static_cast<const SHitProxyPolygon*>( this ) == *static_cast<const SHitProxyPolygon*>( &rhs );
	else
		return *static_cast<const SHitProxyEdge*>( this ) == *static_cast<const SHitProxyEdge*>( &rhs );
	return true;
}

bool SHitProxy::HitTest( SHitProxy* a, SHitProxy* b, const CMatrix2D& transA, const CMatrix2D& transB, SHitTestResult* pResult )
{
	if( a->nType == eHitProxyType_Circle )
	{
		if( b->nType == eHitProxyType_Circle )
			return HitTestCircles( static_cast<SHitProxyCircle*>( a ), static_cast<SHitProxyCircle*>( b ), transA, transB, pResult );
		else if( b->nType == eHitProxyType_Polygon )
			return HitTestCircleAndPolygon( static_cast<SHitProxyCircle*>( a ), static_cast<SHitProxyPolygon*>( b ), transA, transB, pResult );
		else
			return HitTestCircleAndEdge( static_cast<SHitProxyCircle*>( a ), static_cast<SHitProxyEdge*>( b ), transA, transB, pResult );
	}
	else if( a->nType == eHitProxyType_Polygon )
	{
		if( b->nType == eHitProxyType_Circle )
		{
			bool bResult = HitTestCircleAndPolygon( static_cast<SHitProxyCircle*>( b ), static_cast<SHitProxyPolygon*>( a ), transB, transA, pResult );
			if( bResult && pResult )
			{
				CVector2 temp = pResult->hitPoint1;
				pResult->hitPoint1 = pResult->hitPoint2;
				pResult->hitPoint2 = temp;
				pResult->normal = pResult->normal * -1;
			}
			return bResult;
		}
		else if( b->nType == eHitProxyType_Polygon )
			return HitTestPolygons( static_cast<SHitProxyPolygon*>( a ), static_cast<SHitProxyPolygon*>( b ), transA, transB, pResult );
		else
			return HitTestPolygonAndEdge( static_cast<SHitProxyPolygon*>( a ), static_cast<SHitProxyEdge*>( b ), transA, transB, pResult );
	}
	else
	{
		if( b->nType == eHitProxyType_Circle )
		{
			bool bResult = HitTestCircleAndEdge( static_cast<SHitProxyCircle*>( b ), static_cast<SHitProxyEdge*>( a ), transB, transA, pResult );
			if( bResult && pResult )
			{
				CVector2 temp = pResult->hitPoint1;
				pResult->hitPoint1 = pResult->hitPoint2;
				pResult->hitPoint2 = temp;
				pResult->normal = pResult->normal * -1;
			}
			return bResult;
		}
		else if( b->nType == eHitProxyType_Polygon )
		{
			bool bResult = HitTestPolygonAndEdge( static_cast<SHitProxyPolygon*>( b ), static_cast<SHitProxyEdge*>( a ), transB, transA, pResult );
			if( bResult && pResult )
			{
				CVector2 temp = pResult->hitPoint1;
				pResult->hitPoint1 = pResult->hitPoint2;
				pResult->hitPoint2 = temp;
				pResult->normal = pResult->normal * -1;
			}
			return bResult;
		}
		else
			return false;
	}
}

bool SHitProxy::Raycast( const CVector2& begin, const CVector2& end, const CMatrix2D& trans, SRaycastResult* pResult )
{
	if( nType == eHitProxyType_Circle )
		return static_cast<SHitProxyCircle*>( this )->Raycast( begin, end, trans, pResult );
	else if( nType == eHitProxyType_Polygon )
		return static_cast<SHitProxyPolygon*>( this )->Raycast( begin, end, trans, pResult );
	else
		return static_cast<SHitProxyEdge*>( this )->Raycast( begin, end, trans, pResult );
}

int8 SHitProxy::SweepTest( SHitProxy * a, SHitProxy * b, const CMatrix2D & transA, const CMatrix2D & transB, const CVector2 & sweepOfs, float fSideThreshold, SRaycastResult* pResult )
{
	if( a->nType == eHitProxyType_Circle )
	{
		if( b->nType == eHitProxyType_Circle )
			return SweepTestCircles( static_cast<SHitProxyCircle*>( a ), static_cast<SHitProxyCircle*>( b ), transA, transB, sweepOfs, fSideThreshold, pResult );
		else if( b->nType == eHitProxyType_Polygon )
			return SweepTestCircleAndPolygon( static_cast<SHitProxyCircle*>( a ), static_cast<SHitProxyPolygon*>( b ), transA, transB, sweepOfs, fSideThreshold, pResult );
	}
	else if( a->nType == eHitProxyType_Polygon )
	{
		if( b->nType == eHitProxyType_Circle )
		{
			bool bResult = SweepTestCircleAndPolygon( static_cast<SHitProxyCircle*>( b ), static_cast<SHitProxyPolygon*>( a ), transB, transA, sweepOfs * -1, fSideThreshold, pResult );
			if( bResult && pResult )
			{
				pResult->normal = pResult->normal * -1;
				CVector2 sweepDir = sweepOfs;
				pResult->hitPoint = pResult->hitPoint + sweepDir * pResult->fDist;
			}
			return bResult;
		}
		else if( b->nType == eHitProxyType_Polygon )
			return SweepTestPolygons( static_cast<SHitProxyPolygon*>( a ), static_cast<SHitProxyPolygon*>( b ), transA, transB, sweepOfs, fSideThreshold, pResult );
	}
	return 0;
}

bool SHitProxy::Contain( SHitProxy* a, SHitProxy* b, const CMatrix2D & transA, const CMatrix2D & transB )
{
	if( a->nType == eHitProxyType_Circle )
	{
		if( b->nType == eHitProxyType_Circle )
			return CircleContainCircle( static_cast<SHitProxyCircle*>( a ), static_cast<SHitProxyCircle*>( b ), transA, transB );
		else if( b->nType == eHitProxyType_Polygon )
			return CircleContainPolygon( static_cast<SHitProxyCircle*>( a ), static_cast<SHitProxyPolygon*>( b ), transA, transB );
	}
	else if( a->nType == eHitProxyType_Polygon )
	{
		if( b->nType == eHitProxyType_Circle )
			return PolygonContainCircle( static_cast<SHitProxyPolygon*>( a ), static_cast<SHitProxyCircle*>( b ), transA, transB );
		else if( b->nType == eHitProxyType_Polygon )
			return PolygonContainPolygon( static_cast<SHitProxyPolygon*>( a ), static_cast<SHitProxyPolygon*>( b ), transA, transB );
	}
	return false;
}

void SHitProxy::CalcBound( const CMatrix2D& trans, CRectangle& newBound )
{
	if( nType == eHitProxyType_Circle )
	{
		CVector2 c = trans.MulVector2Pos( static_cast<SHitProxyCircle*>( this )->center );
		float fRadius = static_cast<SHitProxyCircle*>( this )->fRadius;
		newBound = CRectangle( c.x - fRadius, c.y - fRadius, fRadius * 2, fRadius * 2 );
	}
	else if( nType == eHitProxyType_Polygon )
	{
		CVector2* vertices = static_cast<SHitProxyPolygon*>( this )->vertices;
		uint32 nVertices = static_cast<SHitProxyPolygon*>( this )->nVertices;
		CVector2 vMin = trans.MulVector2Pos( vertices[0] );
		CVector2 vMax = vMin;
		for( int i = 1; i < nVertices; i++ )
		{
			CVector2 transformedPos = trans.MulVector2Pos( vertices[i] );
			vMin.x = Min( vMin.x, transformedPos.x );
			vMin.y = Min( vMin.y, transformedPos.y );
			vMax.x = Max( vMax.x, transformedPos.x );
			vMax.y = Max( vMax.y, transformedPos.y );
		}
		newBound = CRectangle( vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y );
	}
	else
	{
		auto pEdge = static_cast<SHitProxyEdge*>( this );
		CVector2 v0 = trans.MulVector2Pos( pEdge->vert0 );
		CVector2 v1 = trans.MulVector2Pos( pEdge->vert1 );
		CVector2 vMin;
		CVector2 vMax;
		vMin.x = Min( v0.x, v1.x );
		vMin.y = Min( v0.y, v1.y );
		vMax.x = Max( v0.x, v1.x );
		vMax.y = Max( v0.y, v1.y );
		if( vMin.x == vMax.x )
		{
			float d = v1.y - v0.y;
			if( d > 0 )
				vMin.x -= d;
			else
				vMax.x -= d;
		}
		if( vMin.y == vMax.y )
		{
			float d = v0.x - v1.x;
			if( d > 0 )
				vMin.y -= d;
			else
				vMax.y -= d;
		}
		newBound = CRectangle( vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y );
	}
}

void SHitProxy::CalcBoundGrid( const CMatrix2D& trans )
{
	CRectangle rect;
	CalcBound( trans, rect );
	TRectangle<int32> newRect( floor( rect.x / CHitTestMgr::nGridSize ), floor( rect.y / CHitTestMgr::nGridSize ), ceil( ( rect.x + rect.width ) / CHitTestMgr::nGridSize ), ceil( ( rect.y + rect.height ) / CHitTestMgr::nGridSize ) );
	newRect.width -= newRect.x;
	newRect.height -= newRect.y;
	bound = newRect;
}

void SHitProxyPolygon::CalcNormals()
{
	for( int i = 0; i < nVertices; i++ )
	{
		int32 i1 = i;
		int32 i2 = i + 1 < nVertices ? i + 1 : 0;
		CVector2 edge = vertices[i2] - vertices[i1];
		normals[i] = CVector2( edge.y, -edge.x );
		normals[i].Normalize();
	}
}

CHitProxy::~CHitProxy()
{
	while( m_pHitProxies )
	{
		SHitProxy* pHitProxy = m_pHitProxies;
		RemoveProxy( pHitProxy );
	}
}

bool CHitProxy::operator==( const CHitProxy& rhs ) const
{
	if( m_bBulletMode != rhs.m_bBulletMode )
		return false;

	auto pHitProxy = m_pHitProxies;
	auto pHitProxy1 = rhs.m_pHitProxies;
	for( ; pHitProxy && pHitProxy1; pHitProxy = pHitProxy->NextHitProxy(), pHitProxy1 = pHitProxy1->NextHitProxy() )
	{
		if( *pHitProxy == *pHitProxy1 )
			continue;
		return false;
	}
	if( pHitProxy && !pHitProxy1 || !pHitProxy && pHitProxy1 )
		return false;
	return true;
}

void CHitProxy::SetBulletMode( bool bBulletMode )
{
	m_bBulletMode = bBulletMode;
	if( m_pMgr )
		m_pMgr->ReAdd( this );
}

void CHitProxy::SetTransparent( bool bTransparent )
{
	m_bTransparent = bTransparent;
	if( m_pMgr && bTransparent )
		m_pMgr->Remove( this );
}

SHitProxyCircle* CHitProxy::AddCircle( float fRadius, const CVector2 &center )
{
	SHitProxyCircle* pHitProxy = new ( TObjectAllocator<SHitProxyCircle>::Inst().Alloc() ) SHitProxyCircle;
	pHitProxy->pOwner = this;
	Insert_HitProxy( pHitProxy );
	pHitProxy->fRadius = fRadius;
	pHitProxy->center = center;
	return pHitProxy;
}

SHitProxyPolygon* CHitProxy::AddRect( const CRectangle& rect )
{
	CVector2 vertices[4] = {
		{ rect.x, rect.y },
		{ rect.x + rect.width, rect.y },
		{ rect.x + rect.width, rect.y + rect.height },
		{ rect.x, rect.y + rect.height },
	};
	return AddPolygon( 4, vertices );
}

SHitProxyPolygon* CHitProxy::AddPolygon( uint32 nVertices, const CVector2* vertices )
{
	SHitProxyPolygon* pHitProxy = new ( TObjectAllocator<SHitProxyPolygon>::Inst().Alloc() ) SHitProxyPolygon;
	pHitProxy->pOwner = this;
	Insert_HitProxy( pHitProxy );
	if( nVertices > MAX_POLYGON_VERTEX_COUNT )
		nVertices = MAX_POLYGON_VERTEX_COUNT;
	pHitProxy->nVertices = nVertices;
	memcpy( pHitProxy->vertices, vertices, sizeof( CVector2 ) * nVertices );
	pHitProxy->CalcNormals();
	return pHitProxy;
}

SHitProxyEdge* CHitProxy::AddEdge( const CVector2& begin, const CVector2& end )
{
	SHitProxyEdge* pHitProxy = new ( TObjectAllocator<SHitProxyEdge>::Inst().Alloc() ) SHitProxyEdge;
	pHitProxy->pOwner = this;
	Insert_HitProxy( pHitProxy );
	pHitProxy->vert0 = begin;
	pHitProxy->vert1 = end;
	pHitProxy->pPrev = pHitProxy->pNext = NULL;
	return pHitProxy;
}

void CHitProxy::AddProxy( const SHitProxyData& data )
{
	for( int i = 0; i < data.nItems; i++ )
	{
		auto& item = data.pItems[i];
		if( item.nType == eHitProxyType_Circle )
			AddCircle( item.fRadius, item.center );
		else
			AddPolygon( item.nVertices, item.vertices );
	}
}

void CHitProxy::RemoveProxy( SHitProxy* pHitProxy )
{
	pHitProxy->RemoveFrom_HitProxy();
	if( pHitProxy->nType == eHitProxyType_Circle )
		TObjectAllocator<SHitProxyCircle>::Inst().Free( pHitProxy );
	else if( pHitProxy->nType == eHitProxyType_Polygon )
		TObjectAllocator<SHitProxyPolygon>::Inst().Free( pHitProxy );
	else
		TObjectAllocator<SHitProxyEdge>::Inst().Free( pHitProxy );
}

void CHitProxy::PackData( CBufFile& buf, bool bWithMetaData ) const
{
	if( bWithMetaData )
		buf.Write<uint16>( eVersion_Cur );

	buf.Write<uint8>( m_bBulletMode );

	vector<SHitProxy*> hitProxies;
	for( auto pHitProxy = m_pHitProxies; pHitProxy; pHitProxy = pHitProxy->NextHitProxy() )
		hitProxies.push_back( pHitProxy );
	buf.Write( hitProxies.size() );
	for( int i = hitProxies.size() - 1; i >= 0; i-- )
	{
		auto pHitProxy = hitProxies[i];
		buf.Write( pHitProxy->nType );
		if( pHitProxy->nType == eHitProxyType_Circle )
		{
			SHitProxyCircle* pCircle = static_cast<SHitProxyCircle*>( pHitProxy );
			buf.Write( pCircle->fRadius );
			buf.Write( pCircle->center );
		}
		else
		{
			SHitProxyPolygon* pPolygon = static_cast<SHitProxyPolygon*>( pHitProxy );
			buf.Write( pPolygon->nVertices );
			buf.Write( pPolygon->vertices, sizeof( pPolygon->vertices[0] ) * pPolygon->nVertices );
		}
	}
}

void CHitProxy::UnpackData( IBufReader& buf, bool bWithMetaData, void* pContext )
{
	uint16 nVersion;
	if( bWithMetaData )
		buf.Read( nVersion );

	m_bBulletMode = buf.Read<uint8>();
	uint32 nSize = buf.Read<uint32>();
	for( int i = 0; i < nSize; i++ )
	{
		uint8 nType = buf.Read<uint8>();
		
		if( nType == eHitProxyType_Circle )
		{
			float fRadius;
			CVector2 center;
			buf.Read( fRadius );
			buf.Read( center );
			AddCircle( fRadius, center );
		}
		else
		{
			uint32 nVertices = buf.Read<uint32>();
			CVector2 vertices[MAX_POLYGON_VERTEX_COUNT];
			buf.Read( vertices, sizeof( vertices[0] ) * nVertices );
			AddPolygon( nVertices, vertices );
		}
	}
}

bool CHitProxy::DiffData( const CHitProxy & obj0, CBufFile & buf ) const
{
	if( *this == obj0 )
		return false;
	PackData( buf, true );
	return true;
}

void CHitProxy::PatchData( IBufReader & buf, void * pContext )
{
	UnpackData( buf, true, pContext );
}

void CHitProxy::CalcBounds()
{
	for( SHitProxy* pProxy = m_pHitProxies; pProxy; pProxy = pProxy->NextHitProxy() )
	{
		pProxy->CalcBoundGrid( GetGlobalTransform() );
	}
}

bool CHitProxy::HitTest( const CVector2& pos )
{
	SHitProxyCircle circle;
	circle.center = pos;
	circle.fRadius = 0;
	CMatrix2D transform;
	transform.Identity();
	const CMatrix2D& mat = GetGlobalTransform();
	for( SHitProxy* pProxy = m_pHitProxies; pProxy; pProxy = pProxy->NextHitProxy() )
	{
		if( SHitProxy::HitTest( pProxy, &circle, mat, transform, NULL ) )
			return true;
	}
	return false;
}

bool CHitProxy::HitTest( CHitProxy* pOther, const CMatrix2D& transform, const CMatrix2D& transform1, SHitTestResult* pResult )
{
	for( SHitProxy* pProxy = m_pHitProxies; pProxy; pProxy = pProxy->NextHitProxy() )
	{
		for( SHitProxy* pProxy1 = pOther->m_pHitProxies; pProxy1; pProxy1 = pProxy1->NextHitProxy() )
		{
			auto rect = pProxy->bound * pProxy1->bound;
			if( rect.width > 0 && rect.height > 0 && SHitProxy::HitTest( pProxy, pProxy1, transform, transform1, pResult ) )
				return true;
		}
	}
	return false;
}

bool CHitProxy::HitTest( CHitProxy* pOther, SHitTestResult* pResult )
{
	if( m_bDirty && !m_pMgr )
	{
		m_bDirty = false;
		CalcBounds();
	}
	if( pOther->m_bDirty && !pOther->m_pMgr )
	{
		pOther->m_bDirty = false;
		pOther->CalcBounds();
	}

	const CMatrix2D& mat = GetGlobalTransform();
	const CMatrix2D& mat1 = pOther->GetGlobalTransform();
	for( SHitProxy* pProxy = m_pHitProxies; pProxy; pProxy = pProxy->NextHitProxy() )
	{
		for( SHitProxy* pProxy1 = pOther->m_pHitProxies; pProxy1; pProxy1 = pProxy1->NextHitProxy() )
		{
			auto rect = pProxy->bound * pProxy1->bound;
			if( rect.width > 0 && rect.height > 0 && SHitProxy::HitTest( pProxy, pProxy1, mat, mat1, pResult ) )
				return true;
		}
	}
	return false;
}

bool CHitProxy::HitTest( SHitProxy* pProxy1, const CMatrix2D& transform, SHitTestResult* pResult )
{
	const CMatrix2D& mat = GetGlobalTransform();
	for( SHitProxy* pProxy = m_pHitProxies; pProxy; pProxy = pProxy->NextHitProxy() )
	{
		if( SHitProxy::HitTest( pProxy, pProxy1, mat, transform, pResult ) )
			return true;
	}
	return false;
}

bool CHitProxy::Raycast( const CVector2& begin, const CVector2& end, SRaycastResult* pResult )
{
	float fDist = -1;
	SRaycastResult result;
	result.pHitProxy = this;
	for( SHitProxy* pProxy = m_pHitProxies; pProxy; pProxy = pProxy->NextHitProxy() )
	{
		if( pProxy->Raycast( begin, end, GetGlobalTransform(), pResult? &result: NULL ) )
		{
			if( !pResult )
				return true;
			if( fDist == -1 || result.fDist < fDist )
			{
				fDist = result.fDist;
				*pResult = result;
			}
		}
	}
	return fDist >= 0;
}

bool CHitProxy::SweepTest( SHitProxy* pProxy1, const CMatrix2D& transform, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult * pResult )
{
	float fDist = -1;
	SRaycastResult result;
	result.pHitProxy = this;
	for( SHitProxy* pProxy = m_pHitProxies; pProxy; pProxy = pProxy->NextHitProxy() )
	{
		if( SHitProxy::SweepTest( pProxy1, pProxy, transform, GetGlobalTransform(), sweepOfs, fSideThreshold, pResult ? &result : NULL ) )
		{
			if( !pResult )
				return true;
			if( fDist == -1 || result.fDist < fDist )
			{
				fDist = result.fDist;
				*pResult = result;
			}
		}
	}
	return fDist >= 0;
}

CHitTestMgr::SGrid * CHitTestMgr::GetGrid( const TVector2<int32>& grid )
{
	if( grid.x < m_size.x || grid.y < m_size.y || grid.x >= m_size.GetRight() || grid.y >= m_size.GetBottom() )
		return NULL;
	uint32 i = grid.x - m_size.x;
	uint32 j = grid.y - m_size.y;
	return &m_grids[i + j * m_size.width];
}

void CHitTestMgr::Add( CHitProxy* pProxy )
{
	pProxy->m_bDirty = true;
	if( pProxy->m_bTransparent )
		return;
	if( pProxy->m_bBulletMode )
		pProxy->InsertTo_HitProxy( m_pHitProxyBulletMode );
	else
		Insert_HitProxy( pProxy );
	pProxy->m_bLastPosValid = false;
	pProxy->m_pMgr = this;
}

void CHitTestMgr::Remove( CHitProxy* pProxy )
{
	if( pProxy->m_pMgr != this )
		return;
	pProxy->m_pMgr = NULL;
	pProxy->m_bDirty = false;
	ClearManifolds( pProxy );
	for( SHitProxy* pHitProxy = pProxy->Get_HitProxy(); pHitProxy; pHitProxy = pHitProxy->NextHitProxy() )
	{
		UpdateBound( pHitProxy, TRectangle<int32>( 0, 0, 0, 0 ) );
	}
	pProxy->RemoveFrom_HitProxy();
}

void CHitTestMgr::ReAdd( CHitProxy* pProxy )
{
	pProxy->m_bDirty = true;
	if( pProxy->m_pMgr )
		pProxy->RemoveFrom_HitProxy();
	if( pProxy->m_bBulletMode )
		pProxy->InsertTo_HitProxy( m_pHitProxyBulletMode );
	else
		Insert_HitProxy( pProxy );
	pProxy->m_pMgr = this;
}

void CHitTestMgr::ClearManifolds( CHitProxy* pProxy )
{
	SHitProxyManifold* pManifold;
	while( ( pManifold = pProxy->m_pManifolds ) != NULL )
	{
		pManifold->pOther->RemoveFrom_Manifold();
		FreeManifold( pManifold->pOther );
		pManifold->RemoveFrom_Manifold();
		FreeManifold( pManifold );
	}
}

void CHitTestMgr::ClearNoBulletModeManifolds( CHitProxy * pProxy )
{
	SHitProxyManifold* pManifold;
	for( pManifold = pProxy->m_pManifolds; pManifold; )
	{
		auto pNext = pManifold->NextManifold();

		if( !pManifold->pOtherHitProxy->m_bBulletMode )
		{
			pManifold->pOther->RemoveFrom_Manifold();
			FreeManifold( pManifold->pOther );
			pManifold->RemoveFrom_Manifold();
			FreeManifold( pManifold );
		}

		pManifold = pNext;
	}
}

void CHitTestMgr::UpdateBound( SHitProxy* pProxy, const TRectangle<int32>& newRect, vector<CHitProxy*>* pOverlaps, bool bInsertGrids )
{
	TRectangle<int32>& oldRect = pProxy->bound;

	SHitProxyGrid* pProxyGrid;
	while( pProxyGrid = pProxy->Get_InProxy(), pProxyGrid != NULL )
	{
		pProxyGrid->RemoveFrom_InProxy();
		pProxyGrid->RemoveFrom_InGrid();
		FreeProxyGrid( pProxyGrid );
	}

	Reserve( newRect );

	uint32 nBaseX = newRect.x - m_size.x;
	uint32 nBaseY = newRect.y - m_size.y;
	uint32 nMaxX = nBaseX + newRect.width;
	uint32 nMaxY = nBaseY + newRect.height;
	for( uint32 i = nBaseX; i < nMaxX; i++ )
	{
		for( uint32 j = nBaseY; j < nMaxY; j++ )
		{
			SGrid& grid = m_grids[i + j * m_size.width];

			if( pOverlaps )
			{
				for( SHitProxyGrid* pProxyGrid1 = grid.m_pProxyGrid; pProxyGrid1; pProxyGrid1 = pProxyGrid1->NextInGrid() )
				{
					CHitProxy* pOwner = pProxyGrid1->pHitProxy->pOwner;
					if( !pOwner->m_bDirty )
						pOverlaps->push_back( pOwner );
				}
			}

			if( bInsertGrids )
			{
				pProxyGrid = AllocProxyGrid();
				pProxyGrid->pHitProxy = pProxy;
				grid.Insert_InGrid( pProxyGrid );
				pProxy->Insert_InProxy( pProxyGrid );
			}
		}
	}

	oldRect = newRect;
}

void CHitTestMgr::Reserve( const TRectangle<int32>& newRect )
{
	TRectangle<int32> newSize = newRect + m_size;
	if( newSize == m_size )
		return;

	uint32 nSize = sizeof(SGrid)* newSize.width * newSize.height;
	SGrid* newGrids = (SGrid*)malloc( nSize );
	memset( newGrids, 0, nSize );
	SGrid* pOld = m_grids;
	SGrid* pNew = newGrids + m_size.x - newSize.x + ( m_size.y - newSize.y ) * newSize.width;
	for( int i = 0; i < m_size.height; i++ )
	{
		SGrid* pOld1 = pOld;
		SGrid* pNew1 = pNew;
		for( int j = 0; j < m_size.width; j++ )
		{
			if( pOld1->m_pProxyGrid )
				pOld1->m_pProxyGrid->Transplant_InGrid( pNew1->m_pProxyGrid );
			pOld1++;
			pNew1++;
		}
		pOld += m_size.width;
		pNew += newSize.width;
	}

	if( m_grids )
		free( m_grids );
	m_grids = newGrids;
	m_size = newSize;
}

SHitProxyGrid* CHitTestMgr::AllocProxyGrid()
{
	return new ( TObjectAllocator<SHitProxyGrid>::Inst().Alloc() ) SHitProxyGrid;
}

void CHitTestMgr::FreeProxyGrid( SHitProxyGrid* pProxyGrid )
{
	TObjectAllocator<SHitProxyGrid>::Inst().Free( pProxyGrid );
}

SHitProxyManifold* CHitTestMgr::AllocManifold()
{
	return new ( TObjectAllocator<SHitProxyManifold>::Inst().Alloc() ) SHitProxyManifold;
}

void CHitTestMgr::FreeManifold( SHitProxyManifold* pManifold )
{
	TObjectAllocator<SHitProxyManifold>::Inst().Free( pManifold );
}

void CHitTestMgr::Update( CHitProxy* pHitProxy, vector<CHitProxy*>* pVecOverlaps, bool bUpdateLastPos )
{
	vector<CHitProxy*> vecOverlaps = *pVecOverlaps;
	ClearManifolds( pHitProxy );
	const CMatrix2D& transform = pHitProxy->GetGlobalTransform();
	for( SHitProxy* pProxy = pHitProxy->Get_HitProxy(); pProxy; pProxy = pProxy->NextHitProxy() )
	{
		CRectangle rect;
		pProxy->CalcBound( transform, rect );
		TRectangle<int32> newRect( floor( rect.x / nGridSize ), floor( rect.y / nGridSize ), ceil( ( rect.x + rect.width ) / nGridSize ), ceil( ( rect.y + rect.height ) / nGridSize ) );
		newRect.width -= newRect.x;
		newRect.height -= newRect.y;
		UpdateBound( pProxy, newRect, &vecOverlaps, !pHitProxy->m_bBulletMode );
	}

	if( vecOverlaps.size() )
	{
		std::sort( vecOverlaps.begin(), vecOverlaps.end() );
		int i, j;
		for( i = 1, j = 1; i < vecOverlaps.size(); i++ )
		{
			if( vecOverlaps[i] != vecOverlaps[i - 1] )
				vecOverlaps[j++] = vecOverlaps[i];
		}

		for( i = 0; i < j; i++ )
		{
			CHitProxy* pHitProxy1 = vecOverlaps[i];
			SHitTestResult hitTestResult;
			if( !pHitProxy->HitTest( pHitProxy1, &hitTestResult ) )
				continue;
			SHitProxyManifold* pManifold = AllocManifold();
			SHitProxyManifold* pManifold1 = AllocManifold();
			pManifold->pOtherHitProxy = pHitProxy1;
			pManifold1->pOtherHitProxy = pHitProxy;
			pManifold->pOther = pManifold1;
			pManifold1->pOther = pManifold;
			pManifold->hitPoint = hitTestResult.hitPoint1;
			pManifold->normal = hitTestResult.normal;
			pManifold1->hitPoint = hitTestResult.hitPoint2;
			pManifold1->normal = hitTestResult.normal * -1;
			pHitProxy->Insert_Manifold( pManifold );
			pHitProxy1->Insert_Manifold( pManifold1 );
		}

		vecOverlaps.resize( 0 );
	}
	pHitProxy->m_bDirty = false;
	if( bUpdateLastPos )
	{
		if( !pHitProxy->m_bLastPosValid )
		{
			pHitProxy->m_bLastPosValid = true;
			pHitProxy->m_lastPos = pHitProxy->m_curPos = transform.GetPosition();
			pHitProxy->m_velocity = CVector2( 0, 0 );
		}
		else
		{
			pHitProxy->m_lastPos = pHitProxy->m_curPos;
			pHitProxy->m_curPos = transform.GetPosition();
			pHitProxy->m_velocity = ( pHitProxy->m_curPos - pHitProxy->m_lastPos ) * GetElapsedTimePerTickInv();
		}
	}
}

void CHitTestMgr::Update()
{
	vector<CHitProxy*> vecOverlaps;
	vecOverlaps.reserve( 10 );
	for( CHitProxy* pHitProxy = m_pHitProxy; pHitProxy; pHitProxy = pHitProxy->NextHitProxy() )
	{
		if( pHitProxy->m_bDirty )
			Update( pHitProxy, &vecOverlaps, true );
		else
		{
			if( !pHitProxy->m_bLastPosValid )
			{
				pHitProxy->m_bLastPosValid = true;
				pHitProxy->m_lastPos = pHitProxy->m_curPos = pHitProxy->GetGlobalTransform().GetPosition();
				pHitProxy->m_velocity = CVector2( 0, 0 );
			}
			else
			{
				pHitProxy->m_lastPos = pHitProxy->m_curPos;
				pHitProxy->m_curPos = pHitProxy->GetGlobalTransform().GetPosition();
				pHitProxy->m_velocity = ( pHitProxy->m_curPos - pHitProxy->m_lastPos ) * GetElapsedTimePerTickInv();
			}
		}
	}
	for( CHitProxy* pHitProxy = m_pHitProxyBulletMode; pHitProxy; pHitProxy = pHitProxy->NextHitProxy() )
	{
		Update( pHitProxy, &vecOverlaps, true );
	}
}

void CHitTestMgr::Update( CHitProxy* pHitProxy )
{
	const CMatrix2D& transform = pHitProxy->GetGlobalTransform();
	if( !pHitProxy->m_bDirty )
		return;
	vector<CHitProxy*> vecOverlaps;
	ClearNoBulletModeManifolds( pHitProxy );
	for( SHitProxy* pProxy = pHitProxy->Get_HitProxy(); pProxy; pProxy = pProxy->NextHitProxy() )
	{
		CRectangle rect;
		pProxy->CalcBound( transform, rect );
		TRectangle<int32> newRect( floor( rect.x / nGridSize ), floor( rect.y / nGridSize ), ceil( ( rect.x + rect.width ) / nGridSize ), ceil( ( rect.y + rect.height ) / nGridSize ) );
		newRect.width -= newRect.x;
		newRect.height -= newRect.y;
		UpdateBound( pProxy, newRect, &vecOverlaps, !pHitProxy->m_bBulletMode );
	}

	if( vecOverlaps.size() )
	{
		std::sort( vecOverlaps.begin(), vecOverlaps.end() );
		int i, j;
		for( i = 1, j = 1; i < vecOverlaps.size(); i++ )
		{
			if( vecOverlaps[i] != vecOverlaps[i - 1] )
				vecOverlaps[j++] = vecOverlaps[i];
		}

		for( i = 0; i < j; i++ )
		{
			CHitProxy* pHitProxy1 = vecOverlaps[i];
			SHitTestResult hitTestResult;
			if( !pHitProxy->HitTest( pHitProxy1, &hitTestResult ) )
				continue;
			SHitProxyManifold* pManifold = AllocManifold();
			SHitProxyManifold* pManifold1 = AllocManifold();
			pManifold->pOtherHitProxy = pHitProxy1;
			pManifold1->pOtherHitProxy = pHitProxy;
			pManifold->pOther = pManifold1;
			pManifold1->pOther = pManifold;
			pManifold->hitPoint = hitTestResult.hitPoint1;
			pManifold->normal = hitTestResult.normal;
			pManifold1->hitPoint = hitTestResult.hitPoint2;
			pManifold1->normal = hitTestResult.normal * -1;
			pHitProxy->Insert_Manifold( pManifold );
			pHitProxy1->Insert_Manifold( pManifold1 );
		}

		vecOverlaps.resize( 0 );
	}
	pHitProxy->m_bDirty = false;
}

void CHitTestMgr::CalcBound( SHitProxy* pProxy, const CMatrix2D& transform )
{
	CRectangle rect;
	pProxy->CalcBound( transform, rect );
	TRectangle<int32> newRect( floor( rect.x / nGridSize ), floor( rect.y / nGridSize ), ceil( ( rect.x + rect.width ) / nGridSize ), ceil( ( rect.y + rect.height ) / nGridSize ) );
	newRect.width -= newRect.x;
	newRect.height -= newRect.y;
	pProxy->bound = newRect;
}

void CHitTestMgr::HitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CHitProxy*>& vecResult, vector<SHitTestResult>* pResult )
{
	CRectangle rect;
	pProxy->CalcBound( transform, rect );
	TRectangle<int32> newRect( floor( rect.x / nGridSize ), floor( rect.y / nGridSize ), ceil( ( rect.x + rect.width ) / nGridSize ), ceil( ( rect.y + rect.height ) / nGridSize ) );
	newRect.width -= newRect.x;
	newRect.height -= newRect.y;
	pProxy->bound = newRect;
	newRect = newRect * m_size;

	uint32 nBaseX = newRect.x - m_size.x;
	uint32 nBaseY = newRect.y - m_size.y;
	uint32 nMaxX = nBaseX + newRect.width;
	uint32 nMaxY = nBaseY + newRect.height;
	vector<CHitProxy*> vecOverlaps;
	for( uint32 i = nBaseX; i < nMaxX; i++ )
	{
		for( uint32 j = nBaseY; j < nMaxY; j++ )
		{
			SGrid& grid = m_grids[i + j * m_size.width];
			for( SHitProxyGrid* pProxyGrid = grid.m_pProxyGrid; pProxyGrid; pProxyGrid = pProxyGrid->NextInGrid() )
			{
				CHitProxy* pHitProxy = pProxyGrid->pHitProxy->pOwner;
				if( !pHitProxy->bIgnoreFlag )
					vecOverlaps.push_back( pHitProxy );
			}
		}
	}

	if( !vecOverlaps.size() )
		return;
	std::sort( vecOverlaps.begin(), vecOverlaps.end() );
	int i, j;
	for( i = 1, j = 1; i < vecOverlaps.size(); i++ )
	{
		if( vecOverlaps[i] != vecOverlaps[i - 1] )
			vecOverlaps[j++] = vecOverlaps[i];
	}
	for( i = 0; i < j; i++ )
	{
		CHitProxy* pHitProxy1 = vecOverlaps[i];
		SHitTestResult result;
		if( pHitProxy1->HitTest( pProxy, transform, pResult? &result: NULL ) )
		{
			vecResult.push_back( pHitProxy1 );
			if( pResult )
				pResult->push_back( result );
		}
	}
}

void CHitTestMgr::Raycast( const CVector2& begin, const CVector2& end, vector<SRaycastResult>& vecResult )
{
	CVector2 gridBegin = begin * ( 1.0f / nGridSize );
	CVector2 gridEnd = end * ( 1.0f / nGridSize );
	CVector2 dGrid = gridEnd - gridBegin;
	TVector2<int32> curGrid( floor( gridBegin.x ),  floor( gridBegin.y ) );
	TVector2<int32> end1( floor( gridEnd.x ), floor( gridEnd.y ) );
	TVector2<int32> dir1, dir2, pt;
	if( dGrid.x > 0 )
	{
		if( dGrid.y > 0 )
		{
			pt = TVector2<int32>( 1, 1 );
			dir1 = TVector2<int32>( 1, 0 );
			dir2 = TVector2<int32>( 0, 1 );
		}
		else
		{
			pt = TVector2<int32>( 1, 0 );
			dir1 = TVector2<int32>( 0, -1 );
			dir2 = TVector2<int32>( 1, 0 );
		}
	}
	else
	{
		if( dGrid.y > 0 )
		{
			pt = TVector2<int32>( 0, 1 );
			dir1 = TVector2<int32>( 0, 1 );
			dir2 = TVector2<int32>( -1, 0 );
		}
		else
		{
			pt = TVector2<int32>( 0, 0 );
			dir1 = TVector2<int32>( -1, 0 );
			dir2 = TVector2<int32>( 0, -1 );
		}
	}
	TVector2<int32> dir0 = dGrid.Dot( CVector2( dir1.x, dir1.y ) ) > dGrid.Dot( CVector2( dir2.x, dir2.y ) ) ? dir1 : dir2;
	
	vector<CHitProxy*> vecOverlaps;
	float s = dGrid.x * ( pt.y + curGrid.y - gridBegin.y ) - dGrid.y * ( pt.x + curGrid.x - gridBegin.x );
	while( 1 )
	{
		int32 i = curGrid.x - m_size.x;
		int32 j = curGrid.y - m_size.y;
		if( i >= 0 && i < m_size.width && j >= 0 && j < m_size.height )
		{
			SGrid& grid = m_grids[i + j * m_size.width];
			for( SHitProxyGrid* pProxyGrid = grid.m_pProxyGrid; pProxyGrid; pProxyGrid = pProxyGrid->NextInGrid() )
			{
				CHitProxy* pHitProxy = pProxyGrid->pHitProxy->pOwner;
				if( !pHitProxy->bIgnoreFlag )
					vecOverlaps.push_back( pHitProxy );
			}
		}
		if( curGrid.x == end1.x && curGrid.y == end1.y )
			break;

		if( s > 0 )
		{
			curGrid = curGrid + dir1;
			s += dGrid.x * dir1.y - dGrid.y * dir1.x;
		}
		else if( s < 0 )
		{
			curGrid = curGrid + dir2;
			s += dGrid.x * dir2.y - dGrid.y * dir2.x;
		}
		else
		{
			curGrid = curGrid + dir0;
			s += dGrid.x * dir0.y - dGrid.y * dir0.x;
		}
	}

	if( !vecOverlaps.size() )
		return;
	std::sort( vecOverlaps.begin(), vecOverlaps.end() );
	int i, j;
	for( i = 1, j = 1; i < vecOverlaps.size(); i++ )
	{
		if( vecOverlaps[i] != vecOverlaps[i - 1] )
			vecOverlaps[j++] = vecOverlaps[i];
	}
	for( i = 0; i < j; i++ )
	{
		CHitProxy* pHitProxy1 = vecOverlaps[i];
		SRaycastResult result;
		if( pHitProxy1->Raycast( begin, end, &result ) )
		{
			vecResult.push_back( result );
		}
	}

	struct SLess
	{
		bool operator () ( const SRaycastResult& lhs, const SRaycastResult& rhs )
		{
			if( lhs.fDist < rhs.fDist )
				return true;
			if( lhs.fDist > rhs.fDist )
				return false;
			return lhs.pHitProxy < rhs.pHitProxy;
		}
	};
	std::sort( vecResult.begin(), vecResult.end(), SLess() );
}

void CHitTestMgr::SweepTest( SHitProxy* pProxy, const CMatrix2D & transform, const CVector2 & sweepOfs, float fSideThreshold, vector<SRaycastResult>& vecResult, bool bSort )
{
	CRectangle rect;
	pProxy->CalcBound( transform, rect );
	CRectangle rect1 = rect.Offset( sweepOfs );
	CRectangle rectGrid = rect * ( 1.0f / nGridSize );
	CRectangle rectGrid1 = rect1 * ( 1.0f / nGridSize );
	CRectangle fullBound = rectGrid + rectGrid1;
	CVector2 dGrid = sweepOfs * ( 1.0f / nGridSize );
	TRectangle<int32> newRect( floor( fullBound.x ), floor( fullBound.y ), ceil( fullBound.x + fullBound.width ), ceil( fullBound.y + fullBound.height ) );
	newRect.width -= newRect.x;
	newRect.height -= newRect.y;
	newRect = newRect * m_size;

	vector<TVector2<int32> > grids;

	CVector2 dir1, dir2, pt;
	if( dGrid.x > 0 )
	{
		if( dGrid.y > 0 )
		{
			pt = CVector2( 1, 1 );
			dir1 = CVector2( 1, 0 );
			dir2 = CVector2( 0, 1 );
		}
		else
		{
			pt = CVector2( 1, 0 );
			dir1 = CVector2( 0, -1 );
			dir2 = CVector2( 1, 0 );
		}
	}
	else
	{
		if( dGrid.y > 0 )
		{
			pt = CVector2( 0, 1 );
			dir1 = CVector2( 0, 1 );
			dir2 = CVector2( -1, 0 );
		}
		else
		{
			pt = CVector2( 0, 0 );
			dir1 = CVector2( -1, 0 );
			dir2 = CVector2( 0, -1 );
		}
	}
	CVector2 dir0 = dGrid.Dot( CVector2( dir1.x, dir1.y ) ) > dGrid.Dot( CVector2( dir2.x, dir2.y ) ) ? dir1 : dir2;

	CVector2 a1 = CVector2( rectGrid.x, rectGrid.y ) + CVector2( rectGrid.width, rectGrid.height ) * ( pt - dir1 );
	CVector2 a2 = CVector2( rectGrid.x, rectGrid.y ) + CVector2( rectGrid.width, rectGrid.height ) * ( pt - dir2 );
	CVector2 b1 = a1 + dGrid;
	CVector2 b2 = a2 + dGrid;

	if( rectGrid.width <= 1 && rectGrid.height <= 1  || abs( dGrid.y ) * newRect.width <= abs( dGrid.x ) || abs( dGrid.x ) * newRect.height <= abs( dGrid.y ) )
	{
		for( int i = 0; i < newRect.width; i++ )
		{
			for( int j = 0; j < newRect.height; j++ )
			{
				grids.push_back( TVector2<int32>( i + newRect.x, j + newRect.y ) );
			}
		}
	}
	else
	{
		for( int j = 0; j < newRect.height; j++ )
		{
			int32 y1 = newRect.y + j;
			int32 y2 = y1 + 1;
			float xs[4] =
			{
				( b1.x - a1.x ) * ( y1 - a1.y ) / ( b1.y - a1.y ) + a1.x,
				( b2.x - a2.x ) * ( y1 - a2.y ) / ( b2.y - a2.y ) + a2.x,
				( b1.x - a1.x ) * ( y2 - a1.y ) / ( b1.y - a1.y ) + a1.x,
				( b2.x - a2.x ) * ( y2 - a2.y ) / ( b2.y - a2.y ) + a2.x,
			};
			float x0 = xs[0], x1 = xs[0];
			for( int i = 1; i < 4; i++ )
			{
				x0 = Min( x0, xs[i] );
				x1 = Max( x1, xs[i] );
			}
			int32 left = Max( (int32)floor( x0 ), newRect.x );
			int32 right = Min( (int32)ceil( x1 ), newRect.GetRight() );
			for( int x = left; x < right; x++ )
			{
				grids.push_back( TVector2<int32>( x, y1 ) );
			}
		}
	}

	vector<CHitProxy*> vecOverlaps;
	for( auto item : grids )
	{
		SGrid& grid = m_grids[item.x - m_size.x + ( item.y - m_size.y ) * m_size.width];
		for( SHitProxyGrid* pProxyGrid = grid.m_pProxyGrid; pProxyGrid; pProxyGrid = pProxyGrid->NextInGrid() )
		{
			CHitProxy* pHitProxy = pProxyGrid->pHitProxy->pOwner;
			if( !pHitProxy->bIgnoreFlag )
				vecOverlaps.push_back( pHitProxy );
		}
	}

	if( !vecOverlaps.size() )
		return;
	std::sort( vecOverlaps.begin(), vecOverlaps.end() );
	int i, j;
	for( i = 1, j = 1; i < vecOverlaps.size(); i++ )
	{
		if( vecOverlaps[i] != vecOverlaps[i - 1] )
			vecOverlaps[j++] = vecOverlaps[i];
	}
	for( i = 0; i < j; i++ )
	{
		CHitProxy* pHitProxy1 = vecOverlaps[i];
		SRaycastResult result;
		if( pHitProxy1->SweepTest( pProxy, transform, sweepOfs, fSideThreshold, &result ) )
		{
			vecResult.push_back( result );
		}
	}

	if( bSort )
	{
		struct SLess
		{
			bool operator () ( const SRaycastResult& lhs, const SRaycastResult& rhs )
			{
				if( lhs.fDist < rhs.fDist )
					return true;
				if( lhs.fDist > rhs.fDist )
					return false;
				return lhs.pHitProxy < rhs.pHitProxy;
			}
		};
		std::sort( vecResult.begin(), vecResult.end(), SLess() );
	}
}
