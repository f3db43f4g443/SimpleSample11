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


TRectangle<int32> PutRect( vector<int8>& vec, int32 nWidth, int32 nHeight, TVector2<int32> p,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType )
{
	int32 nBackType = vec[p.x + p.y * nWidth];
	TRectangle<int32> rect( p.x, p.y, 1, 1 );
	return PutRect( vec, nWidth, nHeight, rect, minSize, maxSize, lim, nExtend, nType, nBackType );
}

TRectangle<int32> PutRect( vector<int8>& vec, int32 nWidth, int32 nHeight, TRectangle<int32> init,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType, int32 nBackType )
{
	TRectangle<int32> rect = init;
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( vec[i + j * nWidth] != nBackType )
				return TRectangle<int32>( 0, 0, 0, 0 );
		}
	}

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

TRectangle<int32> PutRectEx( vector<int8>& vec, int32 nWidth, int32 nHeight, TVector2<int32> p,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType, function<bool( TRectangle<int32>, TRectangle<int32> )> func )
{
	TRectangle<int32> rect( p.x, p.y, 1, 1 );
	return PutRectEx( vec, nWidth, nHeight, rect, minSize, maxSize, lim, nExtend, nType, func );
}

TRectangle<int32> PutRectEx( vector<int8>& vec, int32 nWidth, int32 nHeight, TRectangle<int32> init,
	TVector2<int32> minSize, TVector2<int32> maxSize, TRectangle<int32> lim, uint32 nExtend, int32 nType, function<bool( TRectangle<int32>, TRectangle<int32> )> func )
{
	TRectangle<int32> rect = init;
	for( int j = 0; j < nExtend; )
	{
		uint32 nExtendDirs[] = { 0, 1, 2, 3 };
		uint32 nExtendDirCount = 4;
		if( rect.width >= minSize.x && rect.height < minSize.y )
		{
			nExtendDirs[0] = 1;
			nExtendDirs[1] = 3;
			nExtendDirCount = 2;
		}
		else if( rect.height >= minSize.y && rect.width < minSize.x )
		{
			nExtendDirs[0] = 0;
			nExtendDirs[1] = 2;
			nExtendDirCount = 2;
		}
		SRand::Inst().Shuffle( nExtendDirs, nExtendDirCount );

		bool bSucceed = false;
		for( int iExtendDir = 0; iExtendDir < nExtendDirCount; iExtendDir++ )
		{
			uint32 nExtendDir = nExtendDirs[iExtendDir];
			bSucceed = true;

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

			if( bSucceed && !func( rect, newPointsRect ) )
				bSucceed = false;

			if( bSucceed )
			{
				rect = newPointsRect + rect;
				j++;
				break;
			}
		}
		if( !bSucceed )
			break;
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

int32 FloodFill( vector<int32>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType )
{
	int32 nBackType = vec[x + y * nWidth];
	if( nBackType == nType )
		return 0;

	vec[x + y * nWidth] = nType;
	vector<TVector2<int32> > q;
	q.push_back( TVector2<int32>( x, y ) );

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

void FloodFill( vector<int32>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, vector<TVector2<int32> >& q )
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

int32 FloodFillExpand( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, uint32 nTargetCount )
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

int32 FloodFillExpand( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, uint32 nTargetCount, vector<TVector2<int32>>& q )
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

int32 FloodFillExpand1( vector<int8>& vec, int32 nWidth, int32 nHeight, int32* nTypes, int32 nTypeCount, int32 nBackType, uint32 nTargetCount, TVector2<int32>* pOfs, int32 nOfs )
{
	vector<TVector2<int32> > q;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			for( int k = 0; k < nTypeCount; k++ )
			{
				if( vec[i + j * nWidth] == nTypes[k] )
				{
					q.push_back( TVector2<int32>( i, j ) );
					break;
				}
			}
		}
	}
	if( !q.size() )
		return 0;
	SRand::Inst().Shuffle( &q[0], q.size() );

	return FloodFillExpand1( vec, nWidth, nHeight, nBackType, nTargetCount, q, pOfs, nOfs );
}

int32 FloodFillExpand1( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nBackType, uint32 nTargetCount, vector<TVector2<int32> >& q, TVector2<int32>* pOfs, int32 nOfs )
{
	TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
	if( !pOfs )
	{
		pOfs = ofs;
		nOfs = 4;
	}
	for( int i = 0; i < q.size() && q.size() < nTargetCount; i++ )
	{
		TVector2<int32> p = q[i];
		SRand::Inst().Shuffle( pOfs, nOfs );
		int32 nType = vec[p.x + p.y * nWidth];
		for( int j = 0; j < nOfs; j++ )
		{
			TVector2<int32> p1 = p + pOfs[j];
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

int32 ExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist, TVector2<int32>* pOfs, int32 nOfs )
{
	vector<TVector2<int32> > q;
	if( nDist > 0 )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				if( vec[i + j * nWidth] == nType )
					q.push_back( TVector2<int32>( i, j ) );
			}
		}
	}
	else if( nDist < 0 )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				if( vec[i + j * nWidth] != nType )
					q.push_back( TVector2<int32>( i, j ) );
			}
		}
	}
	else
		return 0;
	return ExpandDist( vec, nWidth, nHeight, nType, nBackType, nDist, q, pOfs, nOfs );
}

int32 ExpandDist( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, int32 nDist, vector<TVector2<int32>>& q, TVector2<int32>* pOfs, int32 nOfs )
{
	TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
	if( !pOfs )
	{
		pOfs = ofs;
		nOfs = 4;
	}
	vector<int32> vecDist;
	vecDist.resize( nWidth * nHeight );
	bool bInv = nDist < 0;
	if( bInv )
		nDist = -nDist;

	for( int i = 0; i < q.size(); i++ )
	{
		TVector2<int32> p = q[i];
		int32 nDist1 = vecDist[p.x + p.y * nWidth] + 1;
		if( nDist1 <= nDist )
		{
			for( int j = 0; j < nOfs; j++ )
			{
				TVector2<int32> p1 = p + pOfs[j];
				if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
					&& vec[p1.x + p1.y * nWidth] == ( bInv ? nType : nBackType ) )
				{
					vec[p1.x + p1.y * nWidth] = bInv ? nBackType : nType;
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
	auto result = FindPath( vec, nWidth, nHeight, nBackType, nPathType, nDstType, q, par, pOfs, nOfs );
	if( result.x < 0 )
		vec[src.x + src.y * nWidth] = nBackType;
	return result;
}

void ConnectAll( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, TVector2<int32>* pOfs, int32 nOfs )
{
	vector<TVector2<int32> > q;
	FindAllOfTypesInMap( vec, nWidth, nHeight, nType, q );
	vector<int32> vec1;
	vec1.resize( nWidth * nHeight );
	vector<int32> vecPars;
	vecPars.resize( nWidth * nHeight );
	memset( &vecPars[0], -1, sizeof( int32 ) * nWidth * nHeight );
	for( int i = 0; i < vec1.size(); i++ )
		vec1[i] = vec[i] == nType ? -2 : ( vec[i] == nBackType ? 0 : -1 );
	int32 nGroups = 0;
	for( auto& p : q )
	{
		if( vec1[p.x + p.y * nWidth] != -2 )
			continue;
		vector<TVector2<int32> > q1;
		FloodFill( vec, nWidth, nHeight, p.x, p.y, nBackType, q1 );
		nGroups++;
		for( auto& p1 : q1 )
		{
			vec1[p1.x + p1.y * nWidth] = nGroups;
			vec[p1.x + p1.y * nWidth] = nType;
		}
	}

	CUnionFind g;
	g.Init( nGroups );
	SRand::Inst().Shuffle( q );
	TVector2<int32> ofs[4] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	if( !pOfs )
	{
		pOfs = ofs;
		nOfs = 4;
	}
	for( int i = 0; i < q.size(); i++ )
	{
		TVector2<int32> pos = q[i];
		int32 nGroup = vec1[pos.x + pos.y * nWidth];

		SRand::Inst().Shuffle( pOfs, nOfs );
		for( int k = 0; k < nOfs; k++ )
		{
			auto pos1 = q[i] + pOfs[k];
			if( pos1.x < 0 || pos1.y < 0 || pos1.x >= nWidth || pos1.y >= nHeight )
				continue;

			int8 nFlag = vec[pos1.x + pos1.y * nWidth];
			int32& nGroup1 = vec1[pos1.x + pos1.y * nWidth];
			if( nGroup1 < 0 )
				continue;
			if( nGroup1 > 0 )
			{
				if( nGroup1 != nGroup )
				{
					if( !g.Union( nGroup1 - 1, nGroup - 1 ) )
						continue;

					TVector2<int32> p = pos;
					for( ;; )
					{
						int32 par = vecPars[p.x + p.y * nWidth];
						if( par < 0 )
							break;
						if( vec[p.x + p.y * nWidth] == nType )
							break;
						vec[p.x + p.y * nWidth] = nType;
						p = q[par];
					}
					p = pos1;
					for( ;; )
					{
						int32 par = vecPars[p.x + p.y * nWidth];
						if( par < 0 )
							break;
						if( vec[p.x + p.y * nWidth] == nType )
							break;
						vec[p.x + p.y * nWidth] = nType;
						p = q[par];
					}
				}
				continue;
			}
			if( vecPars[pos1.x + pos1.y * nWidth] >= 0 )
				continue;

			q.push_back( pos1 );
			vecPars[pos1.x + pos1.y * nWidth] = i;
			nGroup1 = nGroup;
		}
	}
}

void ConnectAll( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, int32 nBackType, const TRectangle<int32>& bound, TVector2<int32>* pOfs, int32 nOfs )
{
	vector<int8> vecTemp;
	vecTemp.resize( bound.width * bound.height );
	for( int i = 0; i < bound.width; i++ )
	{
		for( int j = 0; j < bound.height; j++ )
		{
			vecTemp[i + j * bound.width] = vec[i + bound.x + ( j + bound.y ) * nWidth];
		}
	}
	ConnectAll( vecTemp, bound.width, bound.height, nType, nBackType, pOfs, nOfs );
	for( int i = 0; i < bound.width; i++ )
	{
		for( int j = 0; j < bound.height; j++ )
		{
			vec[i + bound.x + ( j + bound.y ) * nWidth] = vecTemp[i + j * bound.width];
		}
	}
}

void GenDistField( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, vector<int32>& vecDist, vector<TVector2<int32> >& q, bool bEdge )
{
	vector<TVector2<int32> > q1;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			int32 nDist = vec[i + j * nWidth] == nType ? -1 : 0;
			if( nDist == 0 )
			{
				q.push_back( TVector2<int32>( i, j ) );
			}
			else if( bEdge )
			{
				if( i == 0 || j == 0 || i == nWidth - 1 || j == nHeight - 1 )
				{
					nDist = 1;
					q1.push_back( TVector2<int32>( i, j ) );
				}
			}
			vecDist[i + j * nWidth] = nDist;
		}
	}
	SRand::Inst().Shuffle( q );
	SRand::Inst().Shuffle( q1 );
	for( auto p : q1 )
		q.push_back( p );
	q1.clear();

	TVector2<int32> ofs[8] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 } };
	for( int i = 0; i < q.size(); i++ )
	{
		TVector2<int32> p = q[i];
		int32 nDist1 = vecDist[p.x + p.y * nWidth] + 1;
		SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );
		for( int j = 0; j < ELEM_COUNT( ofs ); j++ )
		{
			TVector2<int32> p1 = p + ofs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& vecDist[p1.x + p1.y * nWidth] == -1 )
			{
				vecDist[p1.x + p1.y * nWidth] = nDist1;
				q.push_back( p1 );
			}
		}
	}
}

int32 SplitDistField( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 nType, vector<int32>& vecDist, vector<TVector2<int32> >& q )
{
	vector<int8> vecGroup;
	vecGroup.resize( nWidth * nHeight );
	memset( &vecGroup[0], -1, sizeof( int8 ) * vecGroup.size() );
	int32 nGroupCount = 0;
	vector<TVector2<int32> > q1, q2;
	TVector2<int32> ofs[8] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 } };
	int32 i = q.size() - 1, i1 = 0;
	for( ; i >= 0 || i1 < q1.size(); )
	{
		auto p = q[i];
		bool b = false;
		if( i1 >= q1.size() && q2.size() )
		{
			auto p1 = q2[0];
			if( vecDist[p1.x + p1.y * nWidth] >= vecDist[p.x + p.y * nWidth] )
			{
				for( auto p2 : q2 )
					q1.push_back( p2 );
				q2.clear();
			}
		}
		if( i1 < q1.size() )
		{
			auto p1 = q1[i1];
			if( vecDist[p1.x + p1.y * nWidth] >= vecDist[p.x + p.y * nWidth] )
			{
				p = p1;
				i1++;
				b = true;
			}
		}
		if( !b )
			i--;

		int32 n = vecDist[p.x + p.y * nWidth];
		if( n <= 0 )
			break;
		auto& nGroup = vecGroup[p.x + p.y * nWidth];
		if( nGroup < 0 )
			nGroup = nGroupCount++;
		SRand::Inst().Shuffle( ofs, ELEM_COUNT( ofs ) );
		for( int j = 0; j < ELEM_COUNT( ofs ); j++ )
		{
			auto p1 = p + ofs[j];
			if( p1.x < 0 || p1.y < 0 || p1.x >= nWidth || p1.y >= nHeight || vecDist[p1.x + p1.y * nWidth] <= 0 )
				continue;
			auto& nGroup1 = vecGroup[p1.x + p1.y * nWidth];
			if( nGroup1 >= 0 )
			{
				if( nGroup != nGroup1 && vec[p.x + p.y * nWidth] != nType )
					vec[p1.x + p1.y * nWidth] = nType;
			}
			else
			{
				nGroup1 = nGroup;
				if( vecDist[p1.x + p1.y * nWidth] < n )
					q2.push_back( p1 );
				else
					q1.push_back( p1 );
			}
		}
	}

	return nGroupCount;
}