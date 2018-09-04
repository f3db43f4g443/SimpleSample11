#pragma once
#include <vector>
#include <functional>
#include "Math3D.h"
using namespace std;

class CUnionFind
{
public:
	void Init( uint32 nCount )
	{
		vecGroups.resize( nCount );
		for( int i = 0; i < nCount; i++ )
			vecGroups[i] = i;
	}

	bool Union( uint32 a, uint32 b )
	{
		a = FindParent( a );
		b = FindParent( b );
		if( a != b )
		{
			vecGroups[b] = a;
			return true;
		}
		return false;
	}

	uint32 FindParent( uint32 a )
	{
		uint32 a0 = a;
		uint32 a1 = vecGroups[a];
		while( a1 != a )
		{
			a = a1;
			a1 = vecGroups[a];
		}

		a = a0;
		a0 = vecGroups[a];
		while( a0 != a )
		{
			a = a0;
			a0 = vecGroups[a];
			vecGroups[a] = a1;
		}
		return a1;
	}
private:
	vector<uint32> vecGroups;
};

void FindAllOfTypesInMap( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, vector<TVector2<int32> >& result );
TRectangle<int32> PutRect( vector<int8>& vec, int32 nWidth, int32 nHeight, TVector2<int32> p,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType );
TRectangle<int32> PutRect( vector<int8>& vec, int32 nWidth, int32 nHeight, TRectangle<int32> init,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType, int32 nBackType );
TRectangle<int32> PutRectEx( vector<int8>& vec, int32 nWidth, int32 nHeight, TVector2<int32> p,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType, function<bool( TRectangle<int32>, TRectangle<int32> )> func );
TRectangle<int32> PutRectEx( vector<int8>& vec, int32 nWidth, int32 nHeight, TRectangle<int32> init,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType, function<bool( TRectangle<int32>, TRectangle<int32> )> func );
int32 FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType );
int32 FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, int32 nMaxCount, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
void FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, vector<TVector2<int32> >& q );
void FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, int32 nMaxCount, vector<TVector2<int32> >& q, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
int32 FloodFillExpand( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nTargetCount );
int32 FloodFillExpand( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nTargetCount, vector<TVector2<int32> >& q );
int32 ExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist );
int32 ExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist, vector<TVector2<int32> >& q );
int32 StepExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist,
	vector<TVector2<int32> >& q, vector<int32> vecDist, int32& i );
TVector2<int32> FindPath( vector<int8>& vec, int32 nWidth, int32 nHeight, int8 nBackType, int8 nPathType, int8 nDstType,
	vector<TVector2<int32> >& q, vector<TVector2<int32> >& par, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
TVector2<int32> FindPath( vector<int8>& vec, int32 nWidth, int32 nHeight, TVector2<int32> src, int8 nPathType, int8 nDstType,
	vector<TVector2<int32> >& par, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
void ConnectAll( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, TVector2<int32>* pOfs = NULL, int32 nOfs = 0 );
void GenDistField( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, vector<int32>& vecDist, vector<TVector2<int32> >& q );