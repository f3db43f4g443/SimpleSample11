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
