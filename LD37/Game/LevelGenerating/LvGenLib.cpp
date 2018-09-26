#include "stdafx.h"
#include "LvGenLib.h"
#include "Common/Algorithm.h"
#include "Common/Rand.h"
#include <algorithm>

void LvGenLib::FillBlocks( vector<int8>& genData, int32 nWidth, int32 nHeight, int32 nFillSizeMin, int32 nFillSizeMax, int8 nTypeBack, int8* nTypes, int8 nTypeCount )
{
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( genData, nWidth, nHeight, nTypeBack, vecEmpty );
	if( !vecEmpty.size() )
		return;

	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );
	for( auto p : vecEmpty )
	{
		if( genData[p.x + p.y * nWidth] != nTypeBack )
			continue;
		FloodFill( genData, nWidth, nHeight, p.x, p.y, nTypes[SRand::Inst().Rand<int32>( 0, nTypeCount )], SRand::Inst().Rand( nFillSizeMin, nFillSizeMax ) );
	}
}

bool LvGenLib::CheckRoomType( vector<int8>& genData, int32 nWidth, int32 nHeight, const TRectangle<int32>& room, uint8 nRoomType )
{
	{
		uint32 nMaxLen = 0;
		int i;
		for( i = room.x; i <= room.GetRight(); i++ )
		{
			if( i == room.GetRight() || genData[i + room.y * nWidth] != nRoomType )
			{
				if( nMaxLen )
				{
					if( nMaxLen <= 1 )
						return false;
					nMaxLen = 0;
				}
			}
			else
				nMaxLen++;
		}
	}
	{
		uint32 nMaxLen = 0;
		int i;
		for( i = room.x; i <= room.GetRight(); i++ )
		{
			if( i == room.GetRight() || genData[i + ( room.GetBottom() - 1 ) * nWidth] != nRoomType )
			{
				if( nMaxLen )
				{
					if( nMaxLen <= 1 )
						return false;
					nMaxLen = 0;
				}
			}
			else
				nMaxLen++;
		}
	}

	{
		uint32 nMaxLen = 0;
		int i;
		for( i = room.y + 1; i <= room.GetBottom() - 1; i++ )
		{
			if( i == room.GetBottom() - 1 || genData[room.x + i * nWidth] != nRoomType )
			{
				if( nMaxLen )
				{
					if( nMaxLen <= 1 )
						return false;
					nMaxLen = 0;
				}
			}
			else
				nMaxLen++;
		}
	}
	{
		uint32 nMaxLen = 0;
		int i;
		for( i = room.y + 1; i <= room.GetBottom() - 1; i++ )
		{
			if( i == room.GetBottom() - 1 || genData[room.GetRight() - 1 + i * nWidth] != nRoomType )
			{
				if( nMaxLen )
				{
					if( nMaxLen <= 1 )
						return false;
					nMaxLen = 0;
				}
			}
			else
				nMaxLen++;
		}
	}
	return true;
}

void LvGenLib::AddBars( vector<int8>& genData, int32 nWidth, int32 nHeight, vector<TRectangle<int32> >& res, int8 nTypeBack, int8 nTypeBar )
{
	vector<int8> vecTemp;
	vecTemp.resize( genData.size() );
	for( int i = 0; i < genData.size(); i++ )
	{
		vecTemp[i] = genData[i] == nTypeBack ? 0 : 1;
	}
	for( auto& bar : res )
	{
		for( int i = bar.x; i < bar.GetRight(); i++ )
			vecTemp[i + bar.y * nWidth] = 1;

		int32 x1 = bar.x + bar.width / 4;
		int32 x2 = bar.GetRight() - bar.width / 4;
		if( bar.y > 0 )
		{
			int32 y = bar.y - 1;
			for( int32 x = x1; x < x2; x++ )
				vecTemp[x + y * nWidth] = 1;
		}
		if( bar.y < nHeight - 1 )
		{
			int32 y = bar.y + 1;
			for( int32 x = x1; x < x2; x++ )
				vecTemp[x + y * nWidth] = 1;
		}

		x1 = bar.x + bar.width * 3 / 8;
		x2 = bar.GetRight() - bar.width * 3 / 8;
		if( bar.y > 1 )
		{
			int32 y = bar.y - 2;
			for( int32 x = x1; x < x2; x++ )
				vecTemp[x + y * nWidth] = 1;
		}
		if( bar.y < nHeight - 2 )
		{
			int32 y = bar.y + 2;
			for( int32 x = x1; x < x2; x++ )
				vecTemp[x + y * nWidth] = 1;
		}
	}

	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, vecEmpty );
	if( !vecEmpty.size() )
		return;
	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );

	for( auto p : vecEmpty )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;

		auto rect = PutRect( vecTemp, nWidth, nHeight, p, TVector2<int32>( 7, 1 ), TVector2<int32>( 12, 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
		if( rect.width > 0 )
		{
			res.push_back( rect );
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					genData[i + j * nWidth] = nTypeBar;
				}
			}

			int32 x1 = rect.x + rect.width / 4;
			int32 x2 = rect.GetRight() - rect.width / 4;
			if( rect.y > 0 )
			{
				int32 y = rect.y - 1;
				for( int32 x = x1; x < x2; x++ )
					vecTemp[x + y * nWidth] = 1;
			}
			if( rect.y < nHeight - 1 )
			{
				int32 y = rect.y + 1;
				for( int32 x = x1; x < x2; x++ )
					vecTemp[x + y * nWidth] = 1;
			}

			x1 = rect.x + rect.width * 3 / 8;
			x2 = rect.GetRight() - rect.width * 3 / 8;
			if( rect.y > 1 )
			{
				int32 y = rect.y - 2;
				for( int32 x = x1; x < x2; x++ )
					vecTemp[x + y * nWidth] = 1;
			}
			if( rect.y < nHeight - 2 )
			{
				int32 y = rect.y + 2;
				for( int32 x = x1; x < x2; x++ )
					vecTemp[x + y * nWidth] = 1;
			}
		}
	}
}

void LvGenLib::GenObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int32 nMaxSize, int8 nTypeBack, int8 nTypeObj, float fCoef, float fCoef1 )
{
	vector<int8> vecTemp;
	vecTemp.resize( genData.size() );
	for( int i = 0; i < genData.size(); i++ )
		vecTemp[i] = genData[i] == nTypeBack ? 0 : 1;
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, vecEmpty );
	SRand::Inst().Shuffle( vecEmpty );

	for( auto p : vecEmpty )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;

		vector<TVector2<int32> > q;
		FloodFill( vecTemp, nWidth, nHeight, p.x, p.y, 1, q );

		if( nMaxSize <= 0 || q.size() <= nMaxSize )
		{
			int32 nObjCount = q.size() < 3 ? q.size() : floor( fCoef + q.size() * fCoef1 );
			for( int i = 0; i < q.size(); i++ )
			{
				auto p1 = q[i];
				while( p1.y > 0 && genData[p1.x + ( p1.y - 1 ) * nWidth] == nTypeBack )
					p1.y--;

				genData[p1.x + p1.y * nWidth] = nTypeObj;
				nObjCount--;
				if( !nObjCount )
					break;
			}
		}
	}
}

void LvGenLib::GenObjs1( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nBlockType, int8 nSpaceType, int8 nObjType )
{
	vector<int8> vecTemp;
	vecTemp.resize( genData.size() );
	for( int i = 0; i < genData.size(); i++ )
		vecTemp[i] = genData[i] == nBlockType ? 0 : 1;
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, vecEmpty );
	SRand::Inst().Shuffle( vecEmpty );

	for( auto p : vecEmpty )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;

		vector<TVector2<int32> > q;

		bool bFound = false;
		int i = 0;
		int32 h[] = { 1, 2, 3 };
		int32 n[] = { 4, 8, 15 };

		int32 minX = nWidth, minY = nHeight, maxX = -1, maxY = -1;
		int k;
		for( k = 0; k < ELEM_COUNT( h ); k++ )
		{
			if( k == 0 )
				FloodFill( vecTemp, nWidth, nHeight, p.x, p.y, 1, n[k], q );
			else
				FloodFillExpand( vecTemp, nWidth, nHeight, 1, 0, n[k], q );
			if( q.size() < n[k] )
				break;
			for( ; i < n[k]; i++ )
			{
				auto p1 = q[i];
				if( p1.x < minX )
					minX = p1.x;
				if( p1.y < minY )
					minY = p1.y;
				if( p1.x > maxX )
					maxX = p1.x;
				if( p1.y > maxY )
					maxY = p1.y;
			}
			if( maxY - minY <= h[k] )
			{
				bFound = true;
				break;
			}
		}

		if( bFound )
		{
			int32 nHoleSizesMin[] = { 4, 6, 10 };
			int32 nHoleSizesMax[] = { 4, 8, 13 };
			int32 nHoleSize = nHoleSizesMin[k];
			minX = nWidth;
			minY = nHeight;
			maxX = -1;
			maxY = -1;
			for( i = 0; i < nHoleSize; i++ )
			{
				auto p1 = q[i];
				if( p1.x < minX )
					minX = p1.x;
				if( p1.y < minY )
					minY = p1.y;
				if( p1.x > maxX )
					maxX = p1.x;
				if( p1.y > maxY )
					maxY = p1.y;
			}

			if( minY <= 0 )
				continue;
			else
			{
				bool bPath = false;
				for( i = minX; i <= maxX; i++ )
				{
					if( genData[i + ( minY - 1 ) * nWidth] == nSpaceType || genData[i + ( minY - 1 ) * nWidth] == nObjType )
					{
						bPath = true;
						break;
					}
				}
				if( bPath )
					continue;
			}
			if( maxY >= nHeight - 1 )
				continue;
			else
			{
				bool bPath = false;
				for( i = minX; i <= maxX; i++ )
				{
					if( genData[i + ( maxY + 1 ) * nWidth] == nSpaceType || genData[i + ( maxY + 1 ) * nWidth] == nObjType )
					{
						bPath = true;
						break;
					}
				}
				if( bPath )
					continue;
			}

			nHoleSize = SRand::Inst().Rand( nHoleSizesMin[k], nHoleSizesMax[k] + 1 );
			int32 nObjCount = nHoleSize * 0.3f;
			SRand::Inst().Shuffle( &q[0], nHoleSize );
			for( i = 0; i < nHoleSize; i++ )
			{
				auto p1 = q[i];
				genData[p1.x + p1.y * nWidth] = nSpaceType;
				if( nObjCount )
				{
					if( genData[p1.x + p1.y * nWidth] == nSpaceType )
					{
						while( p1.y > 0 && genData[p1.x + ( p1.y - 1 ) * nWidth] == nSpaceType )
							p1.y--;

						genData[p1.x + p1.y * nWidth] = nObjType;
						nObjCount--;
					}
				}
			}
		}
	}
}

void LvGenLib::GenObjs2( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nTypeBack, int8 nTypeObj, float fPercent )
{
	vector<TVector2<int32> > q1, q2;
	int32 n = 0;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( genData[i + j * nWidth] != nTypeBack )
				continue;
			n++;
			uint8 l = i == 0 || genData[( i - 1 ) + j * nWidth] != nTypeBack;
			uint8 r = i == nWidth - 1 || genData[( i + 1 ) + j * nWidth] != nTypeBack;
			uint8 t = j == 0 || genData[i + ( j - 1 ) * nWidth] != nTypeBack;
			uint8 b = j == nHeight - 1 || genData[i + ( j + 1 ) * nWidth] != nTypeBack;
			if( l + r + t + b >= 3 )
				q1.push_back( TVector2<int32>( i, j ) );
			else if( ( l || r ) && ( t || b ) )
				q2.push_back( TVector2<int32>( i, j ) );
		}
	}

	n *= fPercent;
	int32 n1 = 0, n2 = 0;
	TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
	while( n > 0 && ( n1 < q1.size() || n2 < q2.size() ) )
	{
		TVector2<int32> p;
		if( n1 < q1.size() )
			p = q1[n1++];
		else
			p = q2[n2++];
		if( genData[p.x + p.y * nWidth] == nTypeObj )
			continue;
		genData[p.x + p.y * nWidth] = nTypeObj;
		n--;

		SRand::Inst().Shuffle( ofs, 4 );
		for( int j = 0; j < 4; j++ )
		{
			TVector2<int32> p1 = p + ofs[j];
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight
				&& genData[p1.x + p1.y * nWidth] == nTypeBack )
			{
				uint8 l = p1.x == 0 || genData[( p1.x - 1 ) + p1.y * nWidth] != nTypeBack;
				uint8 r = p1.x == nWidth - 1 || genData[( p1.x + 1 ) + p1.y * nWidth] != nTypeBack;
				uint8 t = p1.y == 0 || genData[p1.x + ( p1.y - 1 ) * nWidth] != nTypeBack;
				uint8 b = p1.y == nHeight - 1 || genData[p1.x + ( p1.y + 1 ) * nWidth] != nTypeBack;
				if( l + r + t + b >= 3 )
					q1.push_back( p1 );
				else if( ( l || r ) && ( t || b ) )
					q2.push_back( p1 );
			}
		}
	}
}

void LvGenLib::DropObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nSpaceType, int8 nObjType )
{
	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			if( genData[i + j * nWidth] == nObjType )
			{
				if( j == 0 )
				{
					genData[i + j * nWidth] = nSpaceType;
					continue;
				}
				int y;
				for( y = j - 1; y >= 0; y-- )
				{
					if( genData[i + y * nWidth] != nSpaceType )
						break;
				}

				if( y + 1 < j )
				{
					genData[i + j * nWidth] = nSpaceType;
					if( y >= 0 )
						genData[i + ( y + 1 ) * nWidth] = nObjType;
				}
			}
		}
	}
}

void LvGenLib::DropObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nSpaceType, int8 * nObjTypes, uint8 nObjTypeCount )
{
	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			bool b = false;
			int8 nType = genData[i + j * nWidth];
			for( int k = 0; k < nObjTypeCount; k++ )
			{
				if( nType == nObjTypes[k] )
				{
					b = true;
					break;
				}
			}
			if( b )
			{
				if( j == 0 )
				{
					genData[i + j * nWidth] = nSpaceType;
					continue;
				}
				int y;
				for( y = j - 1; y >= 0; y-- )
				{
					if( genData[i + y * nWidth] != nSpaceType )
						break;
				}

				if( y + 1 < j || j == 0 )
				{
					genData[i + j * nWidth] = nSpaceType;
					if( y >= 0 )
						genData[i + ( y + 1 ) * nWidth] = nType;
				}
			}
		}
	}
}

void LvGenLib::Flatten( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nSpaceType, int8 nFlattenType, int8 nFillType )
{
	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 1; i < nWidth - 1; i++ )
		{
			if( genData[i + j * nWidth] == nSpaceType && genData[i - 1 + j * nWidth] == nFlattenType && genData[i + 1 + j * nWidth] == nFlattenType )
			{
				genData[i + j * nWidth] = nFillType;
			}
		}
	}
}

void LvGenLib::DropObj1( vector<int8>& gendata, int32 nWidth, int32 nHeight, vector<TRectangle<int32> >& objs,
	int8 nTypeNone, int8 nTypeObj, bool bDropOut )
{
	vector<int32> vec;
	vec.resize( objs.size() );
	for( int i = 0; i < vec.size(); i++ )
		vec[i] = i;
	auto func = [&objs] ( int32 l, int32 r ) -> bool
	{
		return objs[l].y < objs[r].y;
	};
	std::sort( vec.begin(), vec.end(), func );

	for( auto i : vec )
	{
		auto& rect = objs[i];
		auto rect1 = rect;
		while( rect1.y > 0 )
		{
			bool bBreak = false;
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				if( gendata[i + ( rect1.y - 1 ) * nWidth] != nTypeNone )
				{
					bBreak = true;
					break;
				}
			}
			if( bBreak )
				break;

			rect1.y--;
		}

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				gendata[i + j * nWidth] = nTypeNone;
			}
		}
		if( !bDropOut || rect1.y > 0 )
		{
			rect = rect1;
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					gendata[i + j * nWidth] = nTypeObj;
				}
			}
		}
		else
			rect = TRectangle<int32>( 0, 0, 0, 0 );
	}
}