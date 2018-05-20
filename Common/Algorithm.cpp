#include "Common.h"
#include "Algorithm.h"
#include "Rand.h"

void FindAllOfTypesInMap( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, vector<TVector2<int32>>& result )
{
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( vec[i + j * nWidth] == nType )
				result.push_back( TVector2<int32>( i, j ) );
		}
	}
}

TRectangle<int32> PutRect( vector<int8>& vec, int32 nWidth, int32 nHeight, TVector2<int32> p
	, TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType )
{
	int32 nBackType = vec[p.x + p.y * nWidth];
	TRectangle<int32> rect( p.x, p.y, 1, 1 );

	uint32 nExtendDirs[] = { 0, 1, 2, 3 };
	uint32 nExtendDirCount = 4;

	for( int j = 0; j < nExtend && nExtendDirCount; )
	{
		uint32 iExtendDir;
		if( rect.width >= minSize.x && rect.height < minSize.y )
		{
			int32 iDir[2] = { -1, -1 };
			for( int i = 0; i < nExtendDirCount; i++ )
			{
				if( nExtendDirs[i] == 1 )
					iDir[0] = i;
				else if( nExtendDirs[i] == 3 )
					iDir[1] = i;
			}
			if( iDir[0] >= 0 && iDir[1] >= 0 )
				iExtendDir = iDir[SRand::Inst().Rand( 0, 2 )];
			else if( iDir[0] >= 0 )
				iExtendDir = iDir[0];
			else if( iDir[1] >= 0 )
				iExtendDir = iDir[1];
			else
				break;
		}
		else if( rect.height >= minSize.y && rect.width < minSize.x )
		{
			int32 iDir[2] = { -1, -1 };
			for( int i = 0; i < nExtendDirCount; i++ )
			{
				if( nExtendDirs[i] == 0 )
					iDir[0] = i;
				else if( nExtendDirs[i] == 2 )
					iDir[1] = i;
			}
			if( iDir[0] >= 0 && iDir[1] >= 0 )
				iExtendDir = iDir[SRand::Inst().Rand( 0, 2 )];
			else if( iDir[0] >= 0 )
				iExtendDir = iDir[0];
			else if( iDir[1] >= 0 )
				iExtendDir = iDir[1];
			else
				break;
		}
		else
			iExtendDir = SRand::Inst().Rand( 0u, nExtendDirCount );
		uint32 nExtendDir = nExtendDirs[iExtendDir];
		bool bSucceed = true;

		TRectangle<int32> newPointsRect;
		switch( nExtendDir )
		{
		case 0:
			if( rect.width >= maxSize.x || rect.x <= lim.x )
			{
				bSucceed = false;
				break;
			}
			newPointsRect = TRectangle<int32>( rect.x - 1, rect.y, 1, rect.height );
			break;
		case 1:
			if( rect.height >= maxSize.y || rect.y <= lim.y )
			{
				bSucceed = false;
				break;
			}
			newPointsRect = TRectangle<int32>( rect.x, rect.y - 1, rect.width, 1 );
			break;
		case 2:
			if( rect.width >= maxSize.x || rect.GetRight() >= lim.GetRight() )
			{
				bSucceed = false;
				break;
			}
			newPointsRect = TRectangle<int32>( rect.GetRight(), rect.y, 1, rect.height );
			break;
		case 3:
			if( rect.height >= maxSize.y || rect.GetBottom() >= lim.GetBottom() )
			{
				bSucceed = false;
				break;
			}
			newPointsRect = TRectangle<int32>( rect.x, rect.GetBottom(), rect.width, 1 );
			break;
		default:
			break;
		}

		if( bSucceed )
		{
			for( int iX = newPointsRect.x; iX < newPointsRect.GetRight(); iX++ )
			{
				for( int iY = newPointsRect.y; iY < newPointsRect.GetBottom(); iY++ )
				{
					if( vec[iX + iY * nWidth] != nBackType )
					{
						bSucceed = false;
						break;
					}
				}
				if( !bSucceed )
					break;
			}
		}

		if( !bSucceed )
			nExtendDirs[iExtendDir] = nExtendDirs[--nExtendDirCount];
		else
		{
			rect = newPointsRect + rect;
			j++;
		}
	}

	if( rect.width < minSize.x || rect.height < minSize.y )
		return TRectangle<int32>( 0, 0, 0, 0 );
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			vec[i + j * nWidth] = nType;
		}
	}
	return rect;
}

int32 FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType )
{
	int32 nBackType = vec[x + y * nWidth];
	if( nBackType == nType )
		return 0;

	vec[x + y * nWidth] = nType;
	vector<TVector2<int32> > q;
	q.push_back(TVector2<int32>( x, y ) );

	for( int i = 0; i < q.size(); i++ )
	{
		TVector2<int32> p = q[i];
		TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
		for( int j = 0; j < 4; j++ )
		{
			TVector2<int32> p1 = p + ofs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& vec[p1.x + p1.y * nWidth] == nBackType )
			{
				vec[p1.x + p1.y * nWidth] = nType;
				q.push_back( p1 );
			}
		}
	}
	return q.size();
}

int32 FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, int32 nMaxCount, TVector2<int32>* pOfs, int32 nOfs )
{
	int32 nBackType = vec[x + y * nWidth];
	if( nBackType == nType )
		return 0;
	TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
	if( !pOfs )
	{
		pOfs = ofs;
		nOfs = 4;
	}

	vec[x + y * nWidth] = nType;
	vector<TVector2<int32> > q;
	q.push_back( TVector2<int32>( x, y ) );

	for( int i = 0; i < q.size() && q.size() < nMaxCount; i++ )
	{
		TVector2<int32> p = q[i];
		SRand::Inst().Shuffle( pOfs, nOfs );
		for( int j = 0; j < nOfs; j++ )
		{
			TVector2<int32> p1 = p + pOfs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& vec[p1.x + p1.y * nWidth] == nBackType )
			{
				vec[p1.x + p1.y * nWidth] = nType;
				q.push_back( p1 );
				if( q.size() >= nMaxCount )
					break;
			}
		}
	}
	return q.size();
}

void FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, vector<TVector2<int32> >& q )
{
	int32 nBackType = vec[x + y * nWidth];
	if( nBackType == nType )
		return;

	vec[x + y * nWidth] = nType;
	int i = q.size();
	q.push_back( TVector2<int32>( x, y ) );

	for( ; i < q.size(); i++ )
	{
		TVector2<int32> p = q[i];
		TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
		for( int j = 0; j < 4; j++ )
		{
			TVector2<int32> p1 = p + ofs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& vec[p1.x + p1.y * nWidth] == nBackType )
			{
				vec[p1.x + p1.y * nWidth] = nType;
				q.push_back( p1 );
			}
		}
	}
}

void FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, int32 nMaxCount, vector<TVector2<int32> >& q, TVector2<int32>* pOfs, int32 nOfs )
{
	int32 nBackType = vec[x + y * nWidth];
	if( nBackType == nType )
		return;
	TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
	if( !pOfs )
	{
		pOfs = ofs;
		nOfs = 4;
	}

	vec[x + y * nWidth] = nType;
	int i = q.size();
	nMaxCount += i;
	q.push_back( TVector2<int32>( x, y ) );

	for( i; i < q.size() && q.size() < nMaxCount; i++ )
	{
		TVector2<int32> p = q[i];
		SRand::Inst().Shuffle( pOfs, nOfs );
		for( int j = 0; j < nOfs; j++ )
		{
			TVector2<int32> p1 = p + pOfs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& vec[p1.x + p1.y * nWidth] == nBackType )
			{
				vec[p1.x + p1.y * nWidth] = nType;
				q.push_back( p1 );
				if( q.size() >= nMaxCount )
					break;
			}
		}
	}
}

int32 FloodFillExpand( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nTargetCount )
{
	vector<TVector2<int32> > q;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( vec[i + j * nWidth] == nType )
				q.push_back( TVector2<int32>( i, j ) );
		}
	}
	if( !q.size() )
		return 0;
	SRand::Inst().Shuffle( &q[0], q.size() );

	return FloodFillExpand( vec, nWidth, nHeight, nType, nBackType, nTargetCount, q );
}

int32 FloodFillExpand( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nTargetCount, vector<TVector2<int32>>& q )
{
	for( int i = 0; i < q.size() && q.size() < nTargetCount; i++ )
	{
		TVector2<int32> p = q[i];
		TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
		SRand::Inst().Shuffle( ofs, 4 );
		for( int j = 0; j < 4; j++ )
		{
			TVector2<int32> p1 = p + ofs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& vec[p1.x + p1.y * nWidth] == nBackType )
			{
				vec[p1.x + p1.y * nWidth] = nType;
				q.push_back( p1 );
				if( q.size() >= nTargetCount )
					break;
			}
		}
	}
	return q.size();
}

int32 ExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist )
{
	vector<TVector2<int32> > q;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( vec[i + j * nWidth] == nType )
				q.push_back( TVector2<int32>( i, j ) );
		}
	}
	return ExpandDist( vec, nWidth, nHeight, nType, nBackType, nDist, q );
}

int32 ExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist, vector<TVector2<int32>>& q )
{
	vector<int32> vecDist;
	vecDist.resize( nWidth * nHeight );

	for( int i = 0; i < q.size(); i++ )
	{
		TVector2<int32> p = q[i];
		int32 nDist1 = vecDist[p.x + p.y * nWidth] + 1;
		if( nDist1 <= nDist )
		{
			TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
			for( int j = 0; j < 4; j++ )
			{
				TVector2<int32> p1 = p + ofs[j];
				if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
					&& vec[p1.x + p1.y * nWidth] == nBackType )
				{
					vec[p1.x + p1.y * nWidth] = nType;
					vecDist[p1.x + p1.y * nWidth] = nDist1;
					q.push_back( p1 );
				}
			}
		}
	}
	return q.size();
}

int32 StepExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist,
	vector<TVector2<int32> >& q, vector<int32> vecDist, int32& i )
{
	int32 i0 = i;
	for( ; i < q.size(); i++ )
	{
		TVector2<int32> p = q[i];
		int32 nDist1 = vecDist[p.x + p.y * nWidth];
		if( nDist1 > nDist )
			break;
		nDist1++;

		TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
		for( int j = 0; j < 4; j++ )
		{
			TVector2<int32> p1 = p + ofs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& vec[p1.x + p1.y * nWidth] == nBackType )
			{
				vec[p1.x + p1.y * nWidth] = nType;
				vecDist[p1.x + p1.y * nWidth] = nDist1;
				q.push_back( p1 );
			}
		}
	}
	return i - i0;
}

TVector2<int32> FindPath( vector<int8>& vec, int32 nWidth, int32 nHeight, int8 nBackType, int8 nPathType, int8 nDstType,
	vector<TVector2<int32> >& q, vector<TVector2<int32> >& par, TVector2<int32>* pOfs, int32 nOfs )
{
	if( !q.size() )
		return TVector2<int32>( -1, -1 );
	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	if( !pOfs )
	{
		pOfs = ofs;
		nOfs = 4;
	}
	int32 nSrcCount = q.size();
	for( int i = 0; i < nSrcCount; i++ )
	{
		TVector2<int32> p = q[i];
		par[p.x + p.y * nWidth] = TVector2<int32>( -1, -1 );
	}

	bool bFound = false;
	for( int i = 0; i < q.size(); i++ )
	{
		TVector2<int32> p = q[i];

		SRand::Inst().Shuffle( pOfs, nOfs );
		for( int j = 0; j < nOfs; j++ )
		{
			TVector2<int32> p1 = p + pOfs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight )
			{
				auto& type = vec[p1.x + p1.y * nWidth];
				if( type == nBackType )
				{
					type = nPathType;
					par[p1.x + p1.y * nWidth] = p;
					q.push_back( p1 );
				}
				else if( type == nDstType )
				{
					par[p1.x + p1.y * nWidth] = p;
					q.push_back( p1 );
					bFound = true;
					break;
				}
			}
		}
		if( bFound )
			break;
	}

	for( int i = nSrcCount; i < ( bFound ? q.size() - 1 : q.size() ); i++ )
	{
		TVector2<int32> p = q[i];
		vec[p.x + p.y * nWidth] = nBackType;
	}

	if( !bFound )
		return TVector2<int32>( -1, -1 );

	TVector2<int32> p = q.back();
	TVector2<int32> res = p;
	for( ; p.x >= 0; )
	{
		auto p1 = par[p.x + p.y * nWidth];
		if( p1.x < 0 )
			break;
		if( vec[p.x + p.y * nWidth] == nBackType )
			vec[p.x + p.y * nWidth] = nPathType;
		p = p1;
	}
	q.clear();
	return res;
}

TVector2<int32> FindPath( vector<int8>& vec, int32 nWidth, int32 nHeight, TVector2<int32> src, int8 nPathType, int8 nDstType,
	vector<TVector2<int32> >& par, TVector2<int32>* pOfs, int32 nOfs )
{
	int8 nBackType = vec[src.x + src.y * nWidth];
	vector<TVector2<int32> > q;
	q.push_back( src );
	vec[src.x + src.y * nWidth] = nPathType;
	return FindPath( vec, nWidth, nHeight, nBackType, nPathType, nDstType, q, par, pOfs, nOfs );
}
