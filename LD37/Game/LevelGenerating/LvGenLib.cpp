#include "stdafx.h"
#include "LvGenLib.h"
#include "Common/Algorithm.h"
#include "Common/Rand.h"

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
		FloodFill( genData, nWidth, nHeight, p.x, p.y, nTypes[SRand::Inst().Rand( 0, 4 )], SRand::Inst().Rand( nFillSizeMin, nFillSizeMax ) );
	}
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

void LvGenLib::GenObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int32 nMaxSize, int8 nTypeBack, int8 nTypeObj )
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
			int32 nObjCount = q.size() < 3 ? q.size() : 1 + q.size() / 2;
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
			int32 nObjCount = nHoleSize / 2;
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

void LvGenLib::DropObjs( vector<int8>& genData, int32 nWidth, int32 nHeight, int8 nSpaceType, int8 nObjType )
{
	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			if( genData[i + j * nWidth] == nObjType )
			{
				int y;
				for( y = j - 1; y >= 0; y-- )
				{
					if( genData[i + j * nWidth] != nSpaceType )
						break;
				}

				if( y < j - 1 )
				{
					genData[i + j * nWidth] = nSpaceType;
					if( y > 0 )
						genData[i + y * nWidth] = nObjType;
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
