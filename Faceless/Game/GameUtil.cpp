#include "stdafx.h"
#include "GameUtil.h"

void GetRange( ERangeType eRangeType, uint32 nRange, uint32 nRange1, vector<TVector2<int32>>& result, bool bExcludeSelf )
{
	switch( eRangeType )
	{
	case eRangeType_Normal:
		for( int32 i = -(int32)nRange; i <= (int32)nRange; i++ )
		{
			if( !bExcludeSelf || i )
				result.push_back( TVector2<int32>( i, 0 ) );
			int32 l = nRange * nRange - i * i;
			for( int j = 1; j * j <= l; j++ )
			{
				result.push_back( TVector2<int32>( i, -j ) );
				result.push_back( TVector2<int32>( i, j ) );
			}
		}
		break;
	default:
		break;
	}
}

bool IsInRange( ERangeType eRangeType, uint32 nRange, uint32 nRange1, const TVector2<int32>& pos, bool bExcludeSelf )
{
	if( bExcludeSelf && !pos.x && !pos.y )
		return false;
	switch( eRangeType )
	{
	case eRangeType_Normal:
		return pos.x * pos.x + pos.y * pos.y <= nRange * nRange;
	default:
		break;
	}
	return false;
}

const TVector2<int32>& GetDirOfs( uint8 nDir )
{
	static TVector2<int32> ofs[4] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };
	return ofs[nDir];
}

TVector2<int32> RotateDir( const TVector2<int32>& dir, uint8 nCharDir )
{
	TVector2<int32> dirs[4] = { { dir.y, -dir.x },{ dir.x, dir.y },{ -dir.y, dir.x },{ -dir.x, -dir.y } };
	return dirs[nCharDir & 3];
}

TVector2<int32> RotateDirInv( const TVector2<int32>& dir, uint8 nCharDir )
{
	TVector2<int32> dirs[4] = { { -dir.y, dir.x },{ dir.x, dir.y },{ dir.y, -dir.x },{ -dir.x, -dir.y } };
	return dirs[nCharDir & 3];
}