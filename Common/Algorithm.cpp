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
	if( nBackType == nType )
		return TRectangle<int32>( 0, 0, 0, 0 );
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
		else if( rect.height >= minSize.x && rect.width < minSize.y )
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

int32 FloodFill( vector<int8>& vec, int32 nWidth, int32 nHeight, int32 x, int32 y, int32 nType, int32 nMaxCount )
{
	int32 nBackType = vec[x + y * nWidth];
	if( nBackType == nType )
		return 0;

	vec[x + y * nWidth] = nType;
	vector<TVector2<int32> > q;
	q.push_back( TVector2<int32>( x, y ) );

	for( int i = 0; i < q.size() && q.size() < nMaxCount; i++ )
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
				if( q.size() >= nMaxCount )
					break;
			}
		}
	}
	return q.size();
}
