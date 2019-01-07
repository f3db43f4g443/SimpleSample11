#include "stdafx.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

void CLevelGen2::GenHouse( vector<int8>& genData, int32 nWidth, int32 nHeight, const TRectangle<int32>& rect, int8 nTypeBegin, int32 la, int32 lb )
{
	const int8 eType_House_0 = nTypeBegin;
	const int8 eType_House_1 = nTypeBegin + 1;
	const int8 eType_House_2 = nTypeBegin + 2;
	const int8 eType_House_2_0 = nTypeBegin + 3;
	const int8 eType_House_2_1 = nTypeBegin + 4;
	const int8 eType_House_2_2 = nTypeBegin + 5;
	const int8 eType_House_2_3 = nTypeBegin + 6;
	const int8 eType_House_2_4 = nTypeBegin + 7;
	const int8 eType_Temp = -1;
	if( rect.width > 2 && rect.height > 2 )
	{
		vector<TVector2<int32> > vec;
		vector<int8> vecTemp;
		vector<int8> vecTemp1;
		vector<TVector2<int32> > q;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				genData[i + j * nWidth] = eType_House_1;
			}
		}
		for( int j = rect.y + SRand::Inst().Rand( 3, 6 ); j < rect.GetBottom(); j += SRand::Inst().Rand( 3, 6 ) )
		{
			int32 w = j == rect.y ? SRand::Inst().Rand( rect.width / 2, rect.width + 1 ) : SRand::Inst().Rand( 2, Max( 3, rect.width / 2 ) );
			int32 i0;
			if( j == rect.y )
				i0 = SRand::Inst().Rand( 0, rect.width );
			else
				i0 = SRand::Inst().Rand( 0, 2 ) ? rect.width - w : 0;
			for( int i = 0; i < w; i++ )
			{
				int32 x = rect.x + ( i + i0 ) % rect.width;
				genData[x + j * nWidth] = eType_House_0;
			}
		}

		if( rect.height >= 3 )
		{
			for( int i = 0; i < 2; i++ )
			{
				for( int j = rect.y + SRand::Inst().Rand( 0, 2 ); j < rect.GetBottom() - 1; j++ )
				{
					int32 x = rect.x + ( rect.width - 1 ) * i;
					if( genData[x + j * nWidth] == eType_House_0 )
						continue;
					int32 nDir = i * 2 - 1;
					int32 l;
					int32 l1 = SRand::Inst().Rand( la, lb );
					for( l = 0; l < l1; l++ )
					{
						int32 x1 = x + nDir * ( l + 1 );
						if( x1 < 0 || x1 >= nWidth || genData[x1 + j * nWidth] )
							break;
					}
					if( l >= l1 )
					{
						genData[x + j * nWidth] = eType_House_2;
					}
				}
			}
		}

		for( int i = 0; i < 2; i++ )
		{
			int32 x = rect.x + ( rect.width - 1 ) * i;
			for( int j = rect.y; j < rect.GetBottom() - 1; j++ )
			{
				if( genData[x + j * nWidth] == eType_House_2 )
				{
					if( genData[x + ( j + 1 ) * nWidth] != eType_House_2 )
					{
						genData[x + j * nWidth] = eType_House_1;
						continue;
					}

					int32 l = Min( SRand::Inst().Rand( 2, 4 ), rect.GetBottom() - 1 - j );
					int32 k;
					for( k = 0; k < l; k++ )
					{
						if( genData[x + ( j + k ) * nWidth] == eType_House_0 )
							break;
						genData[x + ( j + k ) * nWidth] = eType_House_2;
					}
					if( genData[x + ( j + k ) * nWidth] == eType_House_2 )
						genData[x + ( j + k ) * nWidth] = eType_House_1;
					l = k;
					int32 l1 = SRand::Inst().Rand( 0, Max( 0, l - 2 ) + 1 );
					for( k = 0; k < l1; k++ )
						genData[x + ( j + k ) * nWidth] = eType_House_1;
					j += l;
					break;
				}
			}
		}

		if( rect.width >= 6 )
		{
			int32 s[2] = { 0, 0 };
			for( int i = 0; i < 2; i++ )
			{
				for( int j = rect.x + 2; j < rect.GetRight() - 2; j++ )
				{
					int32 y = rect.y + ( rect.height - 1 ) * i;
					int32 nDir = i * 2 - 1;
					int32 l;
					int32 l1 = SRand::Inst().Rand( la, lb );
					for( l = 0; l < l1; l++ )
					{
						int32 y1 = y + nDir * ( l + 1 );
						if( y1 < 0 || y1 >= nHeight || genData[j + y1 * nWidth] )
							break;
					}
					if( l >= l1 )
					{
						genData[j + y * nWidth] = eType_Temp;
						s[i]++;
					}
				}
			}

			int32 jBegin, jEnd;
			if( nHeight <= SRand::Inst().Rand( 6, 10 ) )
			{
				if( s[0] >= s[1] )
				{
					jBegin = 0;
					jEnd = 1;
				}
				else
				{
					jBegin = 1;
					jEnd = 2;
				}
			}
			else
			{
				jBegin = 0;
				jEnd = 2;
			}
			for( int j = jBegin; j < jEnd; j++ )
			{
				int32 y = rect.y + ( rect.height - 1 ) * j;
				int32 xMax = -1, l = 0;
				float lMax = 2;
				for( int x = rect.x + 2; x <= rect.GetRight() - 2; x++ )
				{
					if( x < rect.GetRight() - 2 && genData[x + y * nWidth] == eType_Temp )
						l++;
					else
					{
						float l1 = l + SRand::Inst().Rand( 0.0f, 1.0f );
						if( l1 >= lMax )
						{
							lMax = l1;
							xMax = x - l;
						}
						l = 0;
					}
				}

				if( xMax >= 0 )
				{
					l = floor( lMax );
					int32 x = xMax + SRand::Inst().Rand( 0, l - 2 + 1 );
					for( int32 i = x; i < x + 2; i++ )
					{
						genData[i + y * nWidth] = eType_House_2;
						if( j == 0 )
							genData[i + ( y + 1 ) * nWidth] = eType_House_2;
					}
				}
			}

			for( int i = 0; i < 2; i++ )
			{
				int32 y = rect.y + ( rect.height - 1 ) * i;
				for( int j = rect.x + 2; j < rect.GetRight() - 2; j++ )
				{
					if( genData[j + y * nWidth] == eType_Temp )
						genData[j + y * nWidth] = eType_House_1;
				}
			}
		}

		vec.resize( 0 );
		vecTemp.resize( rect.width * rect.height );
		vecTemp1.resize( rect.width * rect.height );
		for( int i = 0; i < rect.width; i++ )
		{
			int32 x = i + rect.x;
			for( int j = 0; j < rect.height; j++ )
			{
				int32 y = j + rect.y;
				vecTemp[i + j * rect.width] = genData[x + y * nWidth];
			}
		}
		memset( &vecTemp1[0], 0, vecTemp1.size() );
		for( int i = 0; i < rect.width; i++ )
		{
			for( int j = 0; j < rect.height; j++ )
			{
				if( vecTemp[i + j * rect.width] == eType_House_2 )
				{
					TRectangle<int32> r1( i - 1, j - 1, 3, 3 );
					r1 = r1 * TRectangle<int32>( 0, 0, rect.width, rect.height );
					for( int x = r1.x; x < r1.GetRight(); x++ )
					{
						for( int y = r1.y; y < r1.GetBottom(); y++ )
						{
							vecTemp1[x + y * rect.width]++;
						}
					}
					vecTemp[i + j * rect.width] = eType_Temp;
					vec.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		SRand::Inst().Shuffle( vec );

		for( auto& p : vec )
		{
			if( vecTemp[p.x + p.y * rect.width] != eType_Temp )
				continue;
			auto rect1 = PutRect( vecTemp, rect.width, rect.height, p, TVector2<int32>( 1, 1 ),
				TVector2<int32>( rect.width, rect.height ), TRectangle<int32>( 0, 0, rect.width, rect.height ), -1, eType_Temp );
			if( rect1.width <= 1 && rect1.height <= 1 )
			{
				vecTemp[p.x + p.y * rect.width] = eType_House_1;
				TRectangle<int32> r1( p.x - 1, p.y - 1, 3, 3 );
				r1 = r1 * TRectangle<int32>( 0, 0, rect.width, rect.height );
				for( int x = r1.x; x < r1.GetRight(); x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
					{
						vecTemp1[x + y * rect.width]--;
					}
				}
				continue;
			}
			FloodFill( vecTemp, rect.width, rect.height, p.x, p.y, eType_House_1, q );
			for( auto& p1 : q )
			{
				TRectangle<int32> r1( p1.x - 1, p1.y - 1, 3, 3 );
				r1 = r1 * TRectangle<int32>( 0, 0, rect.width, rect.height );
				for( int x = r1.x; x < r1.GetRight(); x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
					{
						vecTemp1[x + y * rect.width]--;
					}
				}
			}
			q.resize( 0 );

			if( rect1.x == 0 || rect1.GetRight() == rect.width )
			{
				rect1 = PutRect( vecTemp1, rect.width, rect.height, rect1, rect1.GetSize(), TVector2<int32>( Max( rect1.width, SRand::Inst().Rand( -1, rect.width / 2 + 1 ) ), rect1.height ),
					TRectangle<int32>( 0, 0, rect.width, rect.height ), -1, 0, 0 );
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
				{
					for( int j = rect1.y; j < rect1.GetBottom(); j++ )
						vecTemp[i + j * rect.width] = eType_House_2;
				}
				int32 j0 = rect1.y;
				if( rect1.y >= 3 )
				{
					int32 l0 = Max( 0, SRand::Inst().Rand( -3, rect1.width ) );
					int32 l1 = Max( 0, SRand::Inst().Rand( -l0, l0 ) );
					int32 l2 = Max( 0, Min( rect1.width - l1 - 1, SRand::Inst().Rand( -3, rect1.width ) ) );
					for( int i = 0; i < l0; i++ )
					{
						int32 x = rect1.x == 0 ? i : rect.width - 1 - i;
						vecTemp[x + ( rect1.GetBottom() - 1 ) * rect.width] = eType_House_1;
					}
					for( int i = 0; i < l1; i++ )
					{
						int32 x = rect1.x == 0 ? i : rect.width - 1 - i;
						vecTemp[x + ( rect1.GetBottom() - 2 ) * rect.width] = eType_House_1;
					}
					for( int i = 0; i < l2; i++ )
					{
						int32 x = rect1.x == 0 ? rect1.GetRight() - 1 - i : rect1.x + i;
						vecTemp[x + ( rect1.GetBottom() - 2 ) * rect.width] = eType_House_1;
					}
					j0 = l2 == 0 ? rect1.y : rect1.y + 1;
				}
				for( int j = j0; j < j0 + 2; j++ )
				{
					int32 x = rect1.x == 0 ? rect1.GetRight() - 1 : rect1.x;
					vecTemp[x + j * rect.width] = rect1.x == 0 ? eType_House_2_2 : eType_House_2_0;
				}
			}
			else if( rect1.y == 0 )
			{
				if( rect1.height < 2 )
					continue;
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
				{
					for( int j = rect1.y; j < rect1.y + 2; j++ )
						vecTemp[i + j * rect.width] = eType_House_2_3;
				}
			}
			else if( rect1.GetBottom() == rect.height )
			{
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
					vecTemp[i + ( rect1.GetBottom() - 1 ) * rect.width] = eType_House_2_1;
			}

			for( int i = rect1.x; i < rect1.GetRight(); i++ )
			{
				for( int j = rect1.y; j < rect1.GetBottom(); j++ )
				{
					if( vecTemp[i + j * rect.width] < eType_House_2 )
						continue;

					TRectangle<int32> r1( i - 1, j - 1, 3, 3 );
					r1 = r1 * TRectangle<int32>( 0, 0, rect.width, rect.height );
					for( int x = r1.x; x < r1.GetRight(); x++ )
					{
						for( int y = r1.y; y < r1.GetBottom(); y++ )
						{
							vecTemp1[x + y * rect.width]++;
						}
					}
				}
			}
		}

		vec.resize( 0 );
		for( int i = 1; i < rect.width - 1; i++ )
		{
			for( int j = 1; j < rect.height - 1; j++ )
			{
				if( vecTemp[i + j * rect.width] == eType_House_1 )
				{
					if( vecTemp1[i + j * rect.width] )
						vecTemp[i + j * rect.width] = eType_Temp;
					else
						vec.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		SRand::Inst().Shuffle( vec );
		int32 s = SRand::Inst().Rand( vec.size() / 3, vec.size() / 2 );
		for( auto& p : vec )
		{
			if( vecTemp[p.x + p.y * rect.width] != eType_House_1 )
				continue;
			auto rect1 = PutRect( vecTemp, rect.width, rect.height, p, TVector2<int32>( 2, 2 ),
				TVector2<int32>( SRand::Inst().Rand( 2, 4 ), 2 ), TRectangle<int32>( 1, 1, rect.width - 2, rect.height - 2 ), -1, eType_House_2 );
			if( rect1.width <= 0 )
				continue;
			TRectangle<int32> r1( rect1.x - 1, rect1.y - 1, rect1.width + 2, rect1.height + 2 );
			r1 = r1 * TRectangle<int32>( 0, 0, rect.width, rect.height );
			for( int x = r1.x; x < r1.GetRight(); x++ )
			{
				for( int y = r1.y; y < r1.GetBottom(); y++ )
				{
					if( vecTemp[x + y * rect.width] == eType_House_1 )
						vecTemp[x + y * rect.width] = eType_Temp;
				}
			}
			s -= rect1.width * rect1.height;
			if( rect1.width > 2 )
			{
				rect1.x += SRand::Inst().Rand( 0, 2 );
				rect1.width = 2;
			}
			for( int x = rect1.x; x < rect1.GetRight(); x++ )
			{
				for( int y = rect1.y; y < rect1.GetBottom(); y++ )
					vecTemp[x + y * rect.width] = eType_House_2_4;
			}
			if( s <= 0 )
				break;
		}

		for( int i = 0; i < rect.width; i++ )
		{
			int32 x = i + rect.x;
			for( int j = 0; j < rect.height; j++ )
			{
				int32 y = j + rect.y;
				if( vecTemp[i + j * rect.width] == eType_Temp )
					vecTemp[i + j * rect.width] = eType_House_1;
			}
		}

		for( int32 k = 0; k < 2; k++ )
		{
			vec.resize( 0 );
			int32 nDir = k ? -1 : 1;
			for( int i = 0; i < rect.width; i++ )
			{
				for( int j = k; j < rect.height - 1 + k; j++ )
				{
					if( vecTemp[i + j * rect.width] == eType_House_1 && vecTemp[i + ( j + nDir ) * rect.width] >= eType_House_2 )
						vec.push_back( TVector2<int32>( i, j ) );
				}
			}
			SRand::Inst().Shuffle( vec );
			s = floor( vec.size() * SRand::Inst().Rand( 0.8f, 1.0f ) );
			for( auto& p : vec )
			{
				if( vecTemp[p.x + p.y * rect.width] != eType_House_1 || vecTemp[p.x + ( p.y + nDir ) * rect.width] < eType_House_2 )
					continue;
				TRectangle<int32> r1( p.x - 1, p.y - 1, 3, 3 );
				r1 = r1 * TRectangle<int32>( 0, 0, rect.width, rect.height );
				bool b = true;
				for( int x = r1.x; x < r1.GetRight() && b; x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
					{
						if( vecTemp[x + y * rect.width] == eType_House_0 )
						{
							b = false;
							break;
						}
					}
				}
				if( !b )
					continue;

				auto rect1 = PutRect( vecTemp, rect.width, rect.height, p, TVector2<int32>( SRand::Inst().Rand( 2, 4 ), 1 ),
					TVector2<int32>( SRand::Inst().Rand( 3, 6 ), SRand::Inst().Rand( 1, 3 ) ), TRectangle<int32>( 0, 0, rect.width, rect.height ), -1, eType_House_0 );
				s -= rect1.width * rect1.height;
				if( s <= 0 )
					break;
			}
		}

		for( int i = 0; i < rect.width; i++ )
		{
			int32 x = i + rect.x;
			for( int j = 0; j < rect.height; j++ )
			{
				int32 y = j + rect.y;
				genData[x + y * nWidth] = vecTemp[i + j * rect.width];
			}
		}
	}
}

void CLevelGen2::GenRoom( vector<int8>& genData, int32 nWidth, int32 nHeight, const TRectangle<int32>& rect, int8 nTypeBegin )
{
	const int8 eType_Room = nTypeBegin;
	const int8 eType_Room_1 = nTypeBegin + 1;
	const int8 eType_Room_2 = nTypeBegin + 2;
	const int8 eType_Room_Door = nTypeBegin + 3;
	const int8 eType_Room_Car_0 = nTypeBegin + 4;
	const int8 eType_Room_Car_2 = nTypeBegin + 5;
	const int8 eType_Room_Car_3 = nTypeBegin + 6;
	const int8 eType_Temp = -1;
	vector<int8> vecTemp;
	vector<TVector2<int32> > vec;
	for( int x = rect.x; x < rect.GetRight(); x += rect.width - 1 )
	{
		for( int y = rect.y; y < rect.GetBottom(); y += rect.height - 1 )
		{
			if( genData[x + y * nWidth] == eType_Room_Door )
				genData[x + y * nWidth] = eType_Room_1;
		}
	}
	vecTemp.resize( rect.width * rect.height );
	vec.resize( 0 );
	bool bDoor = false;
	for( int x = 0; x < rect.width; x++ )
	{
		for( int y = 0; y < rect.height; y++ )
		{
			auto nData = genData[x + rect.x + ( y + rect.y ) * nWidth];
			if( nData == eType_Room_2 || nData == eType_Room_Door )
			{
				vecTemp[x + y * rect.width] = 0;
				bDoor = true;
			}
			else if( nData < eType_Room_2 )
			{
				vecTemp[x + y * rect.width] = 1;
				if( x > 0 && x < rect.width - 1 && y > 0 && y < rect.height - 1 )
					vec.push_back( TVector2<int32>( x, y ) );
			}
			else
				vecTemp[x + y * rect.width] = 2;
		}
	}
	SRand::Inst().Shuffle( vec );
	if( bDoor )
	{
		if( rect.width >= 5 )
		{
			for( int x = 0; x < rect.width; x += rect.width - 1 )
			{
				bool b = false;
				for( int y = 1; y < rect.height - 1; y++ )
				{
					if( genData[x + rect.x + ( y + rect.y ) * nWidth] == ( x == 0 ? eType_Room_Car_2 : eType_Room_Car_0 ) )
					{
						b = true;
						break;
					}
				}
				if( b )
				{
					for( int y = 1; y < rect.height - 1; y++ )
						vecTemp[x + y * rect.width] = 0;
					if( rect.width >= 6 )
					{
						int32 x1 = x == 0 ? x + 1 : x - 1;
						for( int y = 1; y < rect.height - 1; y++ )
						{
							if( genData[x1 + rect.x + ( y + rect.y ) * nWidth] == ( x == 0 ? eType_Room_Car_2 : eType_Room_Car_0 ) )
								vecTemp[x1 + y * rect.width] = 0;
						}
					}
				}
			}
		}
		if( rect.height >= 5 )
		{
			bool b = false;
			int32 y = 0;
			for( int x = 1; x < rect.width - 1; x++ )
			{
				if( genData[x + rect.x + ( y + rect.y ) * nWidth] == eType_Room_Car_3 )
				{
					b = true;
					break;
				}
			}
			if( b )
			{
				for( int x = 1; x < rect.width - 1; x++ )
					vecTemp[x + y * rect.width] = 0;
				if( rect.height >= 6 )
				{
					int32 y1 = y + 1;
					for( int x = 1; x < rect.width - 1; x++ )
					{
						if( genData[x + rect.x + ( y1 + rect.y ) * nWidth] == eType_Room_Car_3 )
							vecTemp[x + y1 * rect.width] = 0;
					}
				}
			}
		}

		TVector2<int32> center( ( rect.width - SRand::Inst().Rand( 0, 2 ) ) / 2, ( rect.height - SRand::Inst().Rand( 0, 2 ) ) / 2 );
		int32 nMinDist = 1000000;
		TVector2<int32> pMin;
		for( auto& p : vec )
		{
			int32 n = abs( p.x - center.x ) + abs( p.y - center.y );
			if( n < nMinDist )
			{
				nMinDist = n;
				pMin = p;
			}
		}
		vecTemp[pMin.x + pMin.y * rect.width] = 0;

		for( int i = 1; i < rect.width - 1; i++ )
		{
			for( int j = 1; j < rect.height - 1; j++ )
			{
				if( vecTemp[i + j * rect.width] )
					vecTemp[i + j * rect.width] += 10;
			}
		}
		ConnectAll( vecTemp, rect.width, rect.height, 0, 11 );
		for( int i = 1; i < rect.width - 1; i++ )
		{
			for( int j = 1; j < rect.height - 1; j++ )
			{
				if( vecTemp[i + j * rect.width] )
					vecTemp[i + j * rect.width] -= 10;
			}
		}
	}

	bool bCar = false;
	for( int x = 0; x < rect.width; x++ )
	{
		for( int y = 0; y < rect.height; y++ )
		{
			auto nType = genData[x + rect.x + ( y + rect.y ) * nWidth];
			if( vecTemp[x + y * rect.width] == 2 && nType >= eType_Room_Car_0 && nType <= eType_Room_Car_3 )
			{
				bCar = true;
				auto r1 = PutRect( vecTemp, rect.width, rect.height, TVector2<int32>( x, y ),
					TVector2<int32>( 2, 2 ), TVector2<int32>( rect.width, rect.height ), TRectangle<int32>( 0, 0, rect.width, rect.height ), -1, 4 );
				if( r1.width )
				{
					if( nType == eType_Room_Car_0 || nType == eType_Room_Car_2 )
					{
						int32 x1 = SRand::Inst().Rand( r1.x, r1.GetRight() - 4 + 1 );
						int32 x2 = x1 + 4;
						for( int j = r1.y; j < r1.GetBottom(); j++ )
						{
							for( int i = r1.x; i < x1; i++ )
								vecTemp[i + j * rect.width] = nType == eType_Room_Car_0 ? 1 : 0;
							for( int i = x2; i < r1.GetRight(); i++ )
								vecTemp[i + j * rect.width] = nType == eType_Room_Car_2 ? 1 : 0;
						}
					}
					else
					{
						int32 y1 = SRand::Inst().Rand( r1.y, r1.GetBottom() - 4 + 1 );
						int32 y2 = y1 + 4;
						for( int i = r1.x; i < r1.GetRight(); i++ )
						{
							for( int j = r1.y; j < y1; j++ )
								vecTemp[i + j * rect.width] = 0;
							for( int j = y2; j < r1.GetBottom(); j++ )
								vecTemp[i + j * rect.width] = 1;
						}
					}
				}
			}
		}
	}

	vec.resize( 0 );
	for( int i = 0; i < rect.width; i += rect.width - 1 )
	{
		for( int j = 0; j < rect.height; j += rect.height - 1 )
			vec.push_back( TVector2<int32>( i, j ) );
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( vecTemp[p.x + p.y * rect.width] != 1 )
			continue;
		auto r = PutRect( vecTemp, rect.width, rect.height, p, TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 3, rect.width ), SRand::Inst().Rand( 3, rect.height ) ),
			TRectangle<int32>( 0, 0, rect.width, rect.height ), -1, 2 );
		if( r.width > 0 )
		{
			bool b[4] = { r.x == 0, r.y == 0, r.GetRight() == rect.width, r.GetBottom() == rect.height };
			auto r1 = TRectangle<int32>( r.x - 1, r.y - 1, r.width + 2, r.height + 2 ) * TRectangle<int32>( 0, 0, rect.width, rect.height );
			for( int x = r1.x; x < r1.GetRight(); x++ )
			{
				for( int y = r1.y; y < r1.GetBottom(); y++ )
				{
					if( vecTemp[x + y * rect.width] == 4 )
					{
						if( x == r1.x )
							b[0] = true;
						else if( x == r1.width - 1 )
							b[2] = true;
						if( y == r1.y )
							b[1] = true;
						else if( y == r1.height - 1 )
							b[3] = true;
					}
				}
			}

			if( b[0] + b[1] + b[2] + b[3] < 3 )
			{
				r1 = r;
				if( !b[0] || !b[2] )
				{
					r1.width = Max( 2, Min( rect.width / 2, r.width - SRand::Inst().Rand( 0, 2 ) ) );
					if( !b[0] && !b[2] )
						r1.x += SRand::Inst().Rand( 0, r.width - r1.width + 1 );
					else if( !b[0] )
						r1.x += r.width - r1.width;
				}
				if( !b[1] || !b[3] )
				{
					r1.height = Max( 2, Min( rect.height / 2, r.height - SRand::Inst().Rand( 0, 2 ) ) );
					if( !b[1] && !b[3] )
						r1.y += SRand::Inst().Rand( 0, r.height - r1.height + 1 );
					else if( !b[1] )
						r1.y += r.height - r1.height;
				}
				for( int x = r.x; x < r.GetRight(); x++ )
				{
					for( int y = r.y; y < r.GetBottom(); y++ )
						vecTemp[x + y * rect.width] = 1;
				}
				for( int x = r1.x; x < r1.GetRight(); x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
						vecTemp[x + y * rect.width] = 2;
				}
				r = r1;
			}
			r1 = TRectangle<int32>( r.x - 1, r.y - 1, r.width + 2, r.height + 2 ) * TRectangle<int32>( 0, 0, rect.width, rect.height );
			for( int x = r1.x; x < r1.GetRight(); x++ )
			{
				for( int y = r1.y; y < r1.GetBottom(); y++ )
				{
					if( vecTemp[x + y * rect.width] == 1 )
						vecTemp[x + y * rect.width] = 3;
				}
			}
		}
	}

	for( int x = 0; x < rect.width; x++ )
	{
		for( int y = 0; y < rect.height; y++ )
		{
			if( vecTemp[x + y * rect.width] == 3 )
				vecTemp[x + y * rect.width] = 1;
		}
	}
	if( !bCar )
	{
		for( int k = 0; k < 2; k++ )
		{
			TRectangle<int32> r( 1, 1, SRand::Inst().Rand( rect.width - 4, rect.width - 2 ), SRand::Inst().Rand( rect.height - 4, rect.height - 2 ) );
			r.x += SRand::Inst().Rand( 0, ( rect.width - 2 ) - r.width + 1 );
			r.y += SRand::Inst().Rand( 0, ( rect.height - 2 ) - r.height + 1 );
			for( int x = r.x; x < r.GetRight(); x++ )
			{
				for( int y = r.y; y < r.GetBottom(); y++ )
				{
					if( vecTemp[x + y * rect.width] == 1 )
						vecTemp[x + y * rect.width] = 0;
				}
			}
		}
	}
	for( int y = 0; y < rect.height; y++ )
	{
		int32 xCur = 0;
		bool b = false;
		for( int x = 0; x <= rect.width; x++ )
		{
			if( x == rect.width || vecTemp[x + y * rect.width] != 1 )
			{
				bool b1 = x < rect.width && vecTemp[x + y * rect.width] == 0;
				if( x > xCur && b && b1 )
				{
					for( int x1 = xCur; x1 < x; x1++ )
						vecTemp[x1 + y * rect.width] = 0;
				}
				xCur = x + 1;
				b = b1;
			}
		}
	}
	for( int x = 0; x < rect.width; x++ )
	{
		int32 yCur = 0;
		bool b = false;
		for( int y = 0; y <= rect.height; y++ )
		{
			if( y == rect.height || vecTemp[x + y * rect.width] != 1 )
			{
				bool b1 = y < rect.height && vecTemp[x + y * rect.width] == 0;
				if( y > yCur && b && b1 )
				{
					for( int y1 = yCur; y1 < y; y1++ )
						vecTemp[x + y1 * rect.width] = 0;
				}
				yCur = y + 1;
				b = b1;
			}
		}
	}

	for( int x = 0; x < rect.width; x++ )
	{
		for( int y = 0; y < rect.height; y++ )
		{
			auto& nType = vecTemp[x + y * rect.width];
			auto& nType1 = genData[x + rect.x + ( y + rect.y ) * nWidth];
			if( nType == 0 )
			{
				if( nType1 != eType_Room_Door )
					nType1 = eType_Room_2;
			}
			else if( nType == 1 )
				nType1 = eType_Room_1;
			else if( nType == 2 )
				nType1 = eType_Room;
		}
	}
}

void CLevelGenNode2_1_0::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pWall1Node = CreateNode( pXml->FirstChildElement( "wall1" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pGarbageBinNode = CreateNode( pXml->FirstChildElement( "garbage_bin" )->FirstChildElement(), context );
	m_pGarbageBin2Node = CreateNode( pXml->FirstChildElement( "garbage_bin2" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_1_0::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenChunks();
	GenObj();
	GenHouse();
	GenWall1();

	for( auto& rect : m_vecWall1 )
	{
		m_pWall1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData == eType_None )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			else if( genData == eType_Obj )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["0"] = eType_House_0;
	context.mapTags["1"] = eType_House_1;
	context.mapTags["2"] = eType_House_2;
	context.mapTags["2_0"] = eType_House_2_0;
	context.mapTags["2_1"] = eType_House_2_1;
	context.mapTags["2_2"] = eType_House_2_2;
	context.mapTags["2_3"] = eType_House_2_3;
	context.mapTags["2_4"] = eType_House_2_4;
	for( auto& rect : m_vecHouse )
	{
		m_pHouseNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	for( auto& rect : m_vecCargo )
	{
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecGarbageBin )
	{
		m_pGarbageBinNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecGarbageBin2 )
	{
		m_pGarbageBin2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_vecWall1.clear();
	m_vecHouse.clear();
	m_vecCargo.clear();
	m_vecGarbageBin.clear();
	m_vecGarbageBin2.clear();
}

void CLevelGenNode2_1_0::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( int y = 0; y < nHeight; y += SRand::Inst().Rand( 18, 22 ) )
	{
		int32 w = SRand::Inst().Rand( 16, 25 );
		int32 h = Min( nHeight - y, SRand::Inst().Rand( 40, 60 ) / w + 2 );
		TRectangle<int32> rect( SRand::Inst().Rand( 0, nWidth - w + 1 ), y, w, h );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = eType_Temp1;
		}
	}

	for( int k = 0; k < 2; k++ )
	{
		vector<TVector2<int32> > vec;
		if( k == 0 )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				for( int i = 0; i < nWidth; i += nWidth - 1 )
				{
					vec.push_back( TVector2<int32>( i, j ) );
				}
				for( int i = ( nWidth - 1 ) / 2; i < nWidth - ( nWidth - 1 ) / 2; i++ )
				{
					vec.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		else
		{
			for( int i = 0; i < nWidth; i++ )
			{
				for( int j = 8; j < nHeight; j++ )
				{
					vec.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		SRand::Inst().Shuffle( vec );

		for( auto& p : vec )
		{
			if( m_gendata[p.x + p.y * nWidth] )
				continue;
			auto vMin = k == 0 ? TVector2<int32>( 8, 13 ) : TVector2<int32>( 8, 10 );
			auto bound = k == 0 ? TRectangle<int32>( 0, 0, nWidth, nHeight ) : TRectangle<int32>( 0, 8, nWidth, nHeight - 8 );
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, vMin, TVector2<int32>( SRand::Inst().Rand( 8, 13 ), SRand::Inst().Rand( 10, 20 ) ), bound, -1, 0 );
			if( !rect.width )
				continue;

			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
					m_gendata[i + j * nWidth] = 0;
			}
			TRectangle<int32> r( rect.x, rect.y, rect.width, 0 );

			while( 1 )
			{
				if( rect.GetBottom() - r.GetBottom() < 2 )
					break;
				TRectangle<int32> r1( rect.x, r.GetBottom(), rect.width, 0 );
				uint8 nType;
				if( r.height == 0 || r.height > 2 )
					nType = Max( 0, SRand::Inst().Rand( -1, 2 ) );
				else
					nType = 1;

				if( nType == 0 )
				{
					r1.height = 2;
					if( r.height == 0 )
					{
						r1.width = SRand::Inst().Rand( 8, rect.width + 1 );
						r1.x = rect.x + SRand::Inst().Rand( 0, rect.width - r1.width + 1 );
					}
					else
					{
						r1.x = SRand::Inst().Rand( Max( rect.x, r.x - 4 ), Max( rect.x, r.x - 1 ) + 1 );
						r1.SetRight( SRand::Inst().Rand( Min( rect.GetRight(), r.GetRight() + 1 ), Min( rect.GetRight(), r.GetRight() + 4 ) + 1 ) );
					}
				}
				else
				{
					r1.height = Min( rect.GetBottom() - r1.y, SRand::Inst().Rand( 3, 7 ) );
					if( r1.height <= 2 )
						break;
					if( r.height == 0 )
					{
						r1.width = SRand::Inst().Rand( 5, Max( 8, rect.width - 3 ) );
						r1.x = rect.x + SRand::Inst().Rand( 0, rect.width - r1.width + 1 );
					}
					else if( r.height == 2 )
					{
						r1.width = Min( r.width, SRand::Inst().Rand( Max( 5, r.width - 6 ), Max( 6, r.width - 2 ) ) );
						r1.x = r.x + SRand::Inst().Rand( 0, r.width - r1.width + 1 );
					}
					else
					{
						r1.width = Max( 5, Min( rect.width, r.width + SRand::Inst().Rand( -1, 2 ) ) );
						if( r1.width < r.width )
							r1.x = r.x + SRand::Inst().Rand( 0, r.width - r1.width + 1 );
						else if( r1.width == r.width )
							r1.x = Max( rect.x, Min( rect.GetRight() - r1.width, r.x + SRand::Inst().Rand( -1, 2 ) ) );
						else
							r1.x = r.x + SRand::Inst().Rand( r.width - r1.width, r1.width - r.width + 1 );
					}
				}
				if( r.x == 0 )
					r1.x = 0;
				else if( r.GetRight() == nWidth )
					r1.x = nWidth - r1.width;
				auto r2 = r1;
				if( nType == 0 )
				{
					auto r3 = PutRect( m_gendata, nWidth, nHeight, r1, r1.GetSize(), TVector2<int32>( nWidth, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, 0 );
					if( Max( rect.x - r3.x, r3.GetRight() - rect.GetRight() ) < SRand::Inst().Rand( 10, 12 ) )
					{
						r2 = r3;
						int32 w = Max( r1.width, Min( SRand::Inst().Rand( 12, 15 ), r2.width - SRand::Inst().Rand( 0, 3 ) ) );
						r2.x = SRand::Inst().Rand( Max( r2.x, r1.GetRight() - w ), Min( r2.GetRight(), r1.x + w ) - w + 1 );
						r2.width = w;
						r1 = r2 * rect;
					}
				}

				for( int i = r2.x; i < r2.GetRight(); i++ )
				{
					for( int j = r2.y; j < r2.GetBottom(); j++ )
					{
						m_gendata[i + j * nWidth] = eType_House_0;
					}
				}
				m_vecHouse.push_back( r2 );
				r = r1;
			}

			for( int k = 0; k < 2; k++ )
			{
				TRectangle<int32> r1( rect.x, rect.y + rect.height * k, rect.width, 0 );
				if( k == 0 )
					r1.SetTop( r1.y - SRand::Inst().Rand( 2, 4 ) );
				else
					r1.height += SRand::Inst().Rand( 2, 4 );
				r1 = r1 * TRectangle<int32>( 0, 0, nWidth, nHeight );
				for( int i = r1.x; i < r1.GetRight(); i++ )
				{
					for( int j = r1.y; j < r1.GetBottom(); j++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = eType_Temp1;
					}
				}
			}
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					if( m_gendata[i + j * nWidth] && m_gendata[i + j * nWidth] != eType_Temp )
					{
						int32 x1 = Max( 0, i - 2 );
						int32 x2 = Min( nWidth - 1, i + 2 );
						for( int x = x1; x <= x2; x++ )
						{
							if( !m_gendata[x + j * nWidth] )
								m_gendata[x + j * nWidth] = eType_Temp;
						}
					}
				}
			}
		}

		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp1 )
					m_gendata[i + j * nWidth] = 0;
			}
		}
	}
}

void CLevelGenNode2_1_0::GenObj()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( int i = 0; i < nWidth; i += nWidth - 1 )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i, j ), TVector2<int32>( 2, 2 ), TVector2<int32>( 2, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
			if( rect.width <= 0 )
				continue;
			GenObjRect( rect, i == 0 ? 1 : 0 );
			j = rect.GetBottom();
		}
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
				m_gendata[i + j * nWidth] = 0;
		}
	}
	vector<TVector2<int32> > vec;
	for( int i = 1; i < nWidth - 1; i++ )
	{
		for( int j = 1; j < nHeight; j++ )
		{
			if( !m_gendata[i + j * nWidth] && m_gendata[i + ( j - 1 ) * nWidth] && ( m_gendata[i - 1 + j * nWidth] || m_gendata[i + 1 + j * nWidth] ) )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		if( m_gendata[p.x - 1 + p.y * nWidth] && m_gendata[p.x + 1 + p.y * nWidth] )
			continue;
		int8 nDir = m_gendata[p.x - 1 + p.y * nWidth] ? 1 : -1;
		int32 x1;
		for( x1 = p.x; x1 > 0 && x1 < nWidth - 1; x1 += nDir )
		{
			int32 x2 = x1 + nDir;
			if( m_gendata[x2 + p.y * nWidth] || !m_gendata[x2 + ( p.y - 1 ) * nWidth] )
				break;
		}
		int32 y1;
		for( y1 = p.y; y1 < nHeight - 1; y1++ )
		{
			int32 x2 = p.x - nDir;
			int32 y2 = y1 + 1;
			if( m_gendata[p.x + y2 * nWidth] || !m_gendata[x2 + y2 * nWidth] )
				break;
		}
		TRectangle<int32> bound( Min( p.x, x1 ), p.y, Max( p.x, x1 ) - Min( p.x, x1 ) + 1, y1 - p.y + 1 );
		int32 s = ( bound.width * bound.height + SRand::Inst().Rand( 5, 15 ) ) / 10;
		for( int i = 0; i < bound.width; i++ )
		{
			int32 x = nDir == 1 ? p.x + i : p.x - i;
			if( m_gendata[x + p.y * nWidth] )
				continue;
			if( i >= SRand::Inst().Rand( -2, bound.width + 1 ) )
				continue;
			if( i >= SRand::Inst().Rand( 0, bound.width + 1 ) && s > 0 )
			{
				auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, p.y ), TVector2<int32>( 1, 2 ),
					TVector2<int32>( SRand::Inst().Rand( 1, 3 ), 2 ), bound, -1, eType_Obj );
				if( rect.width )
				{
					if( rect.width == 1 )
						m_vecGarbageBin.push_back( rect );
					else
						m_vecGarbageBin2.push_back( rect );
					s -= rect.width;
				}
				continue;
			}
			auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, p.y ), TVector2<int32>( 2, 2 ),
				TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 4 ) ), bound, -1, eType_Obj );
			if( rect.width )
				m_vecCargo.push_back( rect );
		}
	}
}

void CLevelGenNode2_1_0::GenHouse()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( auto& rect : m_vecHouse )
	{
		CLevelGen2::GenHouse( m_gendata, nWidth, nHeight, rect, eType_House_0 );
	}
}

void CLevelGenNode2_1_0::GenWall1()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	for( int i = 0; i < vecTemp.size(); i++ )
		vecTemp[i] = m_gendata[i] == 0 || m_gendata[i] == eType_Obj ? 0 : 1;
	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		auto rect = PutRect( vecTemp, nWidth, nHeight, p, TVector2<int32>( SRand::Inst().Rand( 10, 12 ), 3 ),
			TVector2<int32>( SRand::Inst().Rand( 13, 16 ), SRand::Inst().Rand( 4, 6 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
		if( rect.width )
			m_vecWall1.push_back( rect );
	}
}

void CLevelGenNode2_1_0::GenObjRect( const TRectangle<int32>& rect, int8 nDir )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( rect.y == 0 )
	{
		auto rect1 = PutRect( m_gendata, nWidth, nHeight, TRectangle<int32>( rect.x, rect.y, rect.width, 2 ), TVector2<int32>( 2, 2 ),
			TVector2<int32>( 8, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight), -1, eType_House_0, 0 );
		if( rect1.width > 0 )
			m_vecHouse.push_back( rect1 );
		if( rect.height >= 4 )
			GenObjRect( TRectangle<int32>( rect.x, rect.y + 2, rect.width, rect.height - 2 ), nDir );
		return;
	}
	if( rect.x > 0 && rect.GetRight() < nWidth || rect.height <= SRand::Inst().Rand( 4, 7 ) )
	{
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			auto nData = m_gendata[x + ( rect.y - 1 ) * nWidth];
			if( !nData || nData == eType_Temp )
				return;
		}
		for( int y = rect.y; y < rect.GetBottom(); y++ )
		{
			TVector2<int32> p( nDir == 1 ? rect.x : rect.GetRight() - 1, y );
			if( m_gendata[p.x + p.y * nWidth] )
				continue;
			int32 h = rect.GetBottom() - y == 4 ? 2 : ( rect.GetBottom() - y <= 3 ? rect.GetBottom() - y : SRand::Inst().Rand( 2, 3 ) );
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, h ), TVector2<int32>( SRand::Inst().Rand( 2, h + 2 ), h ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Obj );
			if( rect.width > 0 )
				m_vecCargo.push_back( rect );
			else
				break;
		}

		if( Min( rect.x, nWidth - rect.GetRight() ) <= SRand::Inst().Rand( -1, 4 ) )
		{
			auto rect1 = rect;
			rect1.x += nDir == 1 ? 2 : -2;
			GenObjRect( rect1, nDir );
		}
	}
	else if( rect.height <= SRand::Inst().Rand( 7, 11 ) )
	{
		m_vecHouse.push_back( rect );
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				m_gendata[x + y * nWidth] = eType_House_0;
		}
		auto rect1 = rect;
		rect1.x += nDir == 1 ? 2 : -2;
		if( rect1.height >= SRand::Inst().Rand( 3, 6 ) || SRand::Inst().Rand( 0, 2 ) )
			GenObjRect( rect1, nDir );
	}
	else
	{
		TRectangle<int32> rect1 = rect, rect2 = rect;
		if( rect.height > SRand::Inst().Rand( 9, 11 ) )
		{
			rect1.height = SRand::Inst().Rand( 3, rect.height - 5 + 1 );
			rect2.SetTop( rect1.GetBottom() + 2 );
			auto rect3 = PutRect( m_gendata, nWidth, nHeight, TRectangle<int32>( rect1.x, rect1.GetBottom(), rect1.width, 2 ),
				TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 6, 12 ), 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House_0, 0 );
			if( rect3.width > 0 )
				m_vecHouse.push_back( rect3 );
		}
		else
		{
			rect1.height = SRand::Inst().Rand( 3, rect.height - 3 + 1 );
			rect2.SetTop( rect1.GetBottom() );
		}
		GenObjRect( rect1, nDir );
		GenObjRect( rect2, nDir );
	}
}

void CLevelGenNode2_1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pWall1Node = CreateNode( pXml->FirstChildElement( "wall1" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pGarbageBinNode = CreateNode( pXml->FirstChildElement( "garbage_bin" )->FirstChildElement(), context );
	m_pGarbageBin2Node = CreateNode( pXml->FirstChildElement( "garbage_bin2" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenChunks();
	GenChunks1();
	GenObjs();
	GenRoads();
	GenHouse();

	for( auto& rect : m_vecWall1 )
	{
		m_pWall1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData == eType_None )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			else if( genData == eType_Obj )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["1"] = eType_Road_1;
	context.mapTags["1_1"] = eType_Road_1_1;
	context.mapTags["1_2"] = eType_Road_1_2;
	for( auto& rect : m_vecRoad )
	{
		m_pRoadNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags["0"] = eType_House_0;
	context.mapTags["1"] = eType_House_1;
	context.mapTags["2"] = eType_House_2;
	context.mapTags["2_0"] = eType_House_2_0;
	context.mapTags["2_1"] = eType_House_2_1;
	context.mapTags["2_2"] = eType_House_2_2;
	context.mapTags["2_3"] = eType_House_2_3;
	context.mapTags["2_4"] = eType_House_2_4;
	for( auto& rect : m_vecHouse )
	{
		m_pHouseNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags["0"] = eType_Room;
	context.mapTags["1"] = eType_Room_1;
	context.mapTags["2"] = eType_Room_2;
	context.mapTags["door"] = eType_Room_Door;
	context.mapTags["car_0"] = eType_Room_Car_0;
	context.mapTags["car_2"] = eType_Room_Car_2;
	context.mapTags["car_3"] = eType_Room_Car_3;
	for( auto& rect : m_vecRoom )
	{
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	for( auto& rect : m_vecCargo )
	{
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecGarbageBin )
	{
		m_pGarbageBinNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecGarbageBin2 )
	{
		m_pGarbageBin2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags.clear();
	m_gendata.clear();
	m_vecWall1.clear();
	m_vecRoad.clear();
	m_vecHouse.clear();
	m_vecRoom.clear();
	m_vecCargo.clear();
	m_vecGarbageBin.clear();
	m_vecGarbageBin2.clear();
}

void CLevelGenNode2_1_1::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<int32> vecHeight;
	int32 h = 0;
	for( ; h < nHeight - 5; )
	{
		int32 h1 = Min( nHeight - 5 - h, SRand::Inst().Rand( 11, 16 ) );
		if( h1 < 11 )
			break;
		vecHeight.push_back( h1 );
		h += h1;
	}
	int32 h1 = nHeight - h - SRand::Inst().Rand( 8, 11 );
	if( h1 > 0 )
	{
		sort( vecHeight.begin(), vecHeight.end() );
		for( int i = 0; i < h1; i++ )
		{
			int32 n = vecHeight[0];
			if( n >= 15 )
				break;
			int32 j;
			for( j = 0; j < vecHeight.size() - 1; j++ )
			{
				if( vecHeight[j + 1] > n )
					break;
			}
			vecHeight[j]++;
			h++;
		}
	}
	SRand::Inst().Shuffle( vecHeight );

	h = nHeight - h;
	if( h >= 8 )
		h = SRand::Inst().Rand( 5, Min( 7, h - 3 ) + 1 );
	{
		int32 hRoad = Min( ( h + 1 ) / 2, SRand::Inst().Rand( 2, 5 ) );
		TRectangle<int32> r( 0, 0, SRand::Inst().Rand( 12, 18 ), h - hRoad );
		r.x = ( nWidth - r.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		AddChunk( r, eType_Wall1, &m_vecWall1 );
		TRectangle<int32> r1( 0, r.height, Max( r.width + 4, SRand::Inst().Rand( 18, 24 ) ), hRoad );
		r1.x = ( nWidth - r1.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		AddChunk( r1, eType_Road, &m_vecRoad );
	}
	uint8 nDir = SRand::Inst().Rand( 0, 2 );

	for( int i = 0; i < vecHeight.size(); i++ )
	{
		int32 h1 = vecHeight[i];
		auto preRect = m_vecRoad.back();
		auto r1 = nDir == 1 ? TRectangle<int32>( preRect.GetRight(), preRect.y, nWidth - preRect.GetRight(), preRect.height ) :
			TRectangle<int32>( 0, preRect.y, preRect.x, preRect.height );
		int32 nType = i == 0 || r1.width < 6 ? 0 : SRand::Inst().Rand( 0, 3 );
		int32 hRoad = ( nType == 1 ? 1 : 0 ) + Min( ( h1 + SRand::Inst().Rand( 1, 5 ) ) / 4, SRand::Inst().Rand( 3, 5 ) );
		TRectangle<int32> road( 0, h + h1 - hRoad, nWidth, hRoad );
		auto bound1 = TRectangle<int32>( 0, h, nWidth, h1 - hRoad );
		auto bound0 = r1;
		bound0.SetBottom( bound1.GetBottom() );
		if( nType == 0 || nType == 1 )
		{
			bound0.SetTop( Max( 0, bound0.y - 2 ) );
			if( nDir == 1 )
				r1.width = Min( 4, r1.width );
			else
				r1.SetLeft( r1.GetRight() - Min( 4, r1.width ) );
			TRectangle<int32> r( nDir == 1 ? preRect.GetRight() - 1 : preRect.x, h, 1, SRand::Inst().Rand( 3, 5 ) );
			if( i > 0 && r1.width > 0 )
			{
				auto rect = PutRect( m_gendata, nWidth, nHeight, r1, TVector2<int32>( 4, 6 ),
					TVector2<int32>( SRand::Inst().Rand( 4, 9 ), SRand::Inst().Rand( 6, 8 ) ), bound0, -1, eType_Room_1, 0 );
				if( rect.width > 0 )
				{
					m_vecRoom.push_back( rect );
					auto carY = preRect.y + SRand::Inst().Rand( 0, preRect.height - 2 );
					for( int y = carY; y < carY + 2; y++ )
					{
						for( int x = rect.x; x < rect.GetRight(); x++ )
						{
							m_gendata[x + y * nWidth] = nDir == 1 ? eType_Room_Car_2 : eType_Room_Car_0;
						}
					}
					r.height = Max( 3, rect.GetBottom() - r.y );

					TRectangle<int32> rect2;
					int8 b = SRand::Inst().Rand( 0, 2 );
					for( int k = 0; k < 2; k++ )
					{
						TRectangle<int32> rect1( ( nDir == 1 ? rect.x : rect.GetRight() ) - 1, rect.y - 2, 2, 2 );
						TVector2<int32> vMin, vMax;
						if( k ^ b )
						{
							vMin = TVector2<int32>( 6, 2 );
							vMax = TVector2<int32>( SRand::Inst().Rand( 6, 11 ), 2 );
						}
						else
						{
							vMin = TVector2<int32>( 4, 3 );
							vMax = TVector2<int32>( SRand::Inst().Rand( 5, 9 ), SRand::Inst().Rand( 3, 5 ) );
						}
						if( vMin.y == 2 && ( nDir == 1 ? rect.x <= 1 : rect.GetRight() >= nWidth - 1 ) )
						{
							if( nDir == 1 )
								rect1.SetRight( nWidth );
							else
								rect1.SetLeft( 0 );
						}
						auto rect2 = PutRect( m_gendata, nWidth, nHeight, rect1, vMin, vMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House_0, 0 );
						if( rect2.width )
							m_vecHouse.push_back( rect2 );
					}
				}
			}

			bool bRoom = false;
			do
			{
				int32 x0 = r.x;
				r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( 4, r.height ),
					TVector2<int32>( SRand::Inst().Rand( 5, 8 ), r.height ), bound1, -1, eType_Wall1, 0 );
				if( r.width <= 0 )
					break;

				if( nType == 0 && r.y > h + SRand::Inst().Rand( 0, h1 - hRoad ) && bound1.GetBottom() - r.y >= 5
					&& bound1.GetBottom() - r.y <= 6 && ( nDir == 1 ? r.GetRight() >= nWidth - 1 : r.x <= 1 ) )
				{
					if( nDir == 1 )
					{
						r.SetRight( nWidth );
						r.SetLeft( Min( x0 - 1, r.x ) );
					}
					else
					{
						r.SetLeft( 0 );
						r.SetRight( Max( x0 + 2, r.GetRight() ) );
					}
					r.SetBottom( bound1.GetBottom() );
					AddChunk( r, eType_Room, &m_vecRoom );
					int32 nDoor = nDir == 1 ? Min( r.GetRight() - 3, SRand::Inst().Rand( r.x + 1, x0 + 1 ) ) : 
						Max( r.x + 2, SRand::Inst().Rand( x0, r.GetRight() - 2 + 1 ) );
					m_gendata[nDoor + r.y * nWidth] = m_gendata[nDoor + ( nDir == 1 ? 1 : -1 ) + r.y * nWidth] = eType_Room_Door;
					nDoor = SRand::Inst().Rand( r.x + 2, r.GetRight() - 2 + 1 );
					m_gendata[nDoor - 1 + ( r.GetBottom() - 1 ) * nWidth] = m_gendata[nDoor + ( r.GetBottom() - 1 ) * nWidth] = eType_Room_Door;

					bRoom = true;
					break;
				}

				m_vecWall1.push_back( r );
				if( nDir == 1 )
					bound1.SetLeft( Min( nWidth - 8, r.x + SRand::Inst().Rand( 2, r.width / 2 + 1 ) ) );
				else
					bound1.SetRight( Max( 8, r.GetRight() - SRand::Inst().Rand( 2, r.width / 2 + 1 ) ) );

				r = TRectangle<int32>( nDir == 1 ? r.GetRight() - 1 : r.x, r.GetBottom(), 1,
					Max( 3, Min( bound1.GetBottom() - r.GetBottom(), SRand::Inst().Rand( 3, 6 ) ) ) );
			} while( r.GetBottom() <= bound1.GetBottom() );

			if( !bRoom )
			{
				int32 x0 = r.x;
				r.SetBottom( h + h1 );
				r.height = Min( nHeight - r.y, Max( r.height, SRand::Inst().Rand( 6, 9 ) ) );
				int32 w1 = nType == 1 ? SRand::Inst().Rand( 8, 10 ) : SRand::Inst().Rand( 6, 9 );
				if( nDir == 1 )
				{
					r.SetRight( nWidth );
					r.SetLeft( Min( x0 - 1, Min( r.x, r.GetRight() - w1 ) ) );
					if( nType == 0 && r.width > 8 )
						r.SetRight( Min( nWidth - 2, r.x + SRand::Inst().Rand( 7, 9 ) ) );
				}
				else
				{
					r.SetLeft( 0 );
					r.SetRight( Max( x0 + 2, Max( r.GetRight(), r.x + w1 ) ) );
					if( nType == 0 && r.width > 8 )
						r.SetLeft( Max( 2, r.GetRight() - SRand::Inst().Rand( 7, 9 ) ) );
				}
				AddChunk( r, eType_Room_1, &m_vecRoom );
				int32 nDoor = nDir == 1 ? Min( r.GetRight() - 3, SRand::Inst().Rand( r.x + 1, x0 + 1 ) ) :
					Max( r.x + 2, SRand::Inst().Rand( x0, r.GetRight() - 2 + 1 ) );
				m_gendata[nDoor + r.y * nWidth] = m_gendata[nDoor + ( nDir == 1 ? 1 : -1 ) + r.y * nWidth] = eType_Room_Door;
				int32 xDoor = nDir == 1 ? r.x : r.GetRight() - 1;
				if( nType == 1 )
				{
					road.SetTop( road.y + 1 );
					for( int y = road.y; y < road.GetBottom() - 2; y++ )
						m_gendata[xDoor + y * nWidth] = eType_Room_2;
					for( int y = road.GetBottom() - 2; y < road.GetBottom(); y++ )
					{
						for( int x = r.x; x < r.GetRight(); x++ )
							m_gendata[x + y * nWidth] = nDir == 1 ? eType_Room_Car_2 : eType_Room_Car_0;
					}
				}
				else
				{
					nDoor = Max( r.y + 2, SRand::Inst().Rand( road.y + 1, road.GetBottom() - 1 + 1 ) );
					m_gendata[xDoor + ( nDoor - 1 ) * nWidth] = m_gendata[xDoor + nDoor * nWidth] = eType_Room_Door;
				}
				if( nDir == 1 )
					road.SetRight( r.x );
				else
					road.SetLeft( r.GetRight() );
			}
			if( nType == 0 )
			{
				if( nDir == 1 )
					road.SetLeft( road.x + SRand::Inst().Rand( 4, 9 ) );
				else
					road.width -= SRand::Inst().Rand( 4, 9 );
			}
			AddChunk( road, eType_Road, &m_vecRoad );
		}
		else
		{
			bound0.SetTop( Max( 0, bound0.y - 3 ) );
			if( nDir == 1 )
				r1.width = Min( 6, r1.width );
			else
				r1.SetLeft( r1.GetRight() - Min( 6, r1.width ) );
			auto rect = PutRect( m_gendata, nWidth, nHeight, r1, r1.GetSize(),
				TVector2<int32>( nWidth, SRand::Inst().Rand( 7, 9 ) ), bound0, -1, eType_Room_1, 0 );
			m_vecRoom.push_back( rect );
			int32 xDoor = nDir == 1 ? rect.x : rect.GetRight() - 1;
			int32 nDoor = SRand::Inst().Rand( preRect.y + 1, preRect.GetBottom() - 1 + 1 );
			m_gendata[xDoor + ( nDoor - 1 ) * nWidth] = m_gendata[xDoor + nDoor * nWidth] = eType_Room_Door;
			nDoor = SRand::Inst().Rand( rect.x + 2, rect.GetRight() - 4 + 1 ) + ( nDir == 1 ? 2 : 0 );
			m_gendata[nDoor - 1 + ( rect.GetBottom() - 1 ) * nWidth] = m_gendata[nDoor + ( rect.GetBottom() - 1 ) * nWidth] = eType_Room_Door;

			auto rect2 = rect;
			rect2.y = road.y - SRand::Inst().Rand( 1, 3 );
			rect2.SetBottom( road.GetBottom() + SRand::Inst().Rand( 1, 3 ) );
			rect2.y = Min( nHeight - rect2.height, rect2.y );
			rect2.SetTop( Max( rect.GetBottom() + 3, rect2.y ) );
			AddChunk( rect2, eType_Room_1, &m_vecRoom );
			for( int x = nDoor - 1; x <= nDoor; x++ )
			{
				for( int y = rect2.y; y < rect2.GetBottom(); y++ )
					m_gendata[x + y * nWidth] = eType_Room_Car_3;
			}
			TRectangle<int32> rect1( nDoor - 2, rect.GetBottom(), 4, rect2.y - rect.GetBottom() );
			if( rect1.y <= 4 )
			{
				rect1 = PutRect( m_gendata, nWidth, nHeight, rect1, rect1.GetSize(), TVector2<int32>( 7, rect1.height ),
					TRectangle<int32>( rect.x, 0, rect.width, nHeight ), -1, eType_Wall1, 0 );
				m_vecWall1.push_back( rect1 );
			}
			else
				AddChunk( rect1, eType_Road, &m_vecRoad );
			nDoor += nDir == 1 ? -2 : 2;
			m_gendata[nDoor - 1 + rect2.y * nWidth] = m_gendata[nDoor + rect2.y * nWidth] = eType_Room_2;
			xDoor = nDir == 1 ? rect2.x : rect2.GetRight() - 1;
			nDoor = SRand::Inst().Rand( road.y + 1, road.GetBottom() - 1 + 1 );
			m_gendata[xDoor + ( nDoor - 1 ) * nWidth] = m_gendata[xDoor + nDoor * nWidth] = eType_Room_Door;

			if( nDir == 1 )
			{
				road.SetRight( rect2.x );
				road.SetLeft( road.x + SRand::Inst().Rand( 4, 9 ) );
			}
			else
			{
				road.SetLeft( rect2.GetRight() );
				road.width -= SRand::Inst().Rand( 4, 9 );
			}
			AddChunk( road, eType_Road, &m_vecRoad );
		}

		TRectangle<int32> rect( Max( preRect.x, road.x ), preRect.GetBottom(),
			Min( preRect.GetRight(), road.GetRight() ) - Max( preRect.x, road.x ), road.y - preRect.GetBottom() );
		if( i > 0 && rect.height >= SRand::Inst().Rand( 9, 14 ) )
		{
			int32 xCur = rect.x, xMax = -1;
			float lMax = 1;
			for( int x = rect.x; x <= rect.GetRight(); x++ )
			{
				bool b = true;
				if( x < rect.GetRight() )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						if( m_gendata[x + y * nWidth] )
						{
							b = false;
							break;
						}
					}
				}
				else
					b = false;

				if( !b )
				{
					float l = x - xCur + SRand::Inst().Rand( 0.0f, 1.0f );
					if( l >= lMax )
					{
						lMax = l;
						xMax = xCur;
					}
					xCur = x + 1;
				}
			}
			int32 l = floor( lMax );
			if( l > 6 )
			{
				TRectangle<int32> car( xMax + SRand::Inst().Rand( 0, l - 1 ), rect.y + SRand::Inst().Rand( 4, rect.height - 4 + 1 ), 2, 4 );
				auto room = PutRect( m_gendata, nWidth, nHeight, car, TVector2<int32>( 6, 6 ), TVector2<int32>( SRand::Inst().Rand( 6, 9 ), SRand::Inst().Rand( 6, 9 ) ),
					rect, -1, eType_Room_1, 0 );
				if( room.width > 0 )
				{
					m_vecRoom.push_back( room );
					for( int x = car.x; x < car.GetRight(); x++ )
					{
						for( int y = room.y; y < room.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Room_Car_3;
						}
						for( int y = rect.y; y < room.y; y++ )
						{
							if( !m_gendata[x + y * nWidth] )
								m_gendata[x + y * nWidth] = eType_Temp;
						}
					}
				}
			}
		}

		nDir = !nDir;
		h += h1;
	}

	for( int k = 0; k < 2; k++ )
	{
		uint8 nType = 0;
		uint8 nLastType = 0;
		TRectangle<int32> preRect( 0, 0, 0, 0 );
		for( int y = 0; y < nHeight - 1; )
		{
			TRectangle<int32> rect0( k == 0 ? 0 : nWidth - 2, y, 2, 2 );
			bool b = true;
			for( int i = rect0.x; i < rect0.GetRight() && b; i++ )
			{
				for( int j = rect0.y; j < rect0.GetBottom(); j++ )
				{
					if( m_gendata[i + j * nWidth] )
					{
						b = false;
						break;
					}
				}
			}
			if( !b )
			{
				y++;
				nType = nLastType = 0;
				preRect = TRectangle<int32>( 0, 0, 0, 0 );
				continue;
			}

			if( nType == 0 )
			{
				auto rect = PutRect( m_gendata, nWidth, nHeight, rect0, TVector2<int32>( 5, 2 ), TVector2<int32>( nWidth / 2, 2 ),
					TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, 0, 0 );
				if( rect.width <= 0 )
				{
					if( nLastType == 2 )
						nType = 1;
					else
						nType = 2;
					continue;
				}
				int32 w1 = SRand::Inst().Rand( 7, 10 );
				if( w1 < rect.width - 3 )
					rect.width = w1;
				rect.x = k == 0 ? 0 : nWidth - rect.width;
				AddChunk( rect, eType_House_0, &m_vecHouse );
				nLastType = 0;
				preRect = rect;
				y += 2;
			}
			else if( nType == 1 )
			{
				TVector2<int32> vMax( SRand::Inst().Rand( 4, 8 ), SRand::Inst().Rand( 3, 7 ) );
				if( preRect.width )
				{
					if( nLastType == 0 )
						vMax.x = Max( 4, Min( vMax.x, preRect.width - SRand::Inst().Rand( 0, 3 ) ) );
					else if( nLastType == 1 )
						vMax.x = Max( 4, Min( 7, preRect.width + SRand::Inst().Rand( -1, 2 ) ) );
				}

				auto rect = PutRect( m_gendata, nWidth, nHeight, rect0, TVector2<int32>( 4, 3 ), vMax, TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, eType_House_0, 0 );
				if( rect.width <= 0 )
				{
					if( nLastType == 2 )
					{
						y++;
						nType = 0;
						preRect = TRectangle<int32>( 0, 0, 0, 0 );
					}
					else
						nType = 2;
					continue;
				}
				m_vecHouse.push_back( rect );
				nLastType = 1;
				preRect = rect;
				y += rect.height;
			}
			else
			{
				auto rect = PutRect( m_gendata, nWidth, nHeight, rect0, TVector2<int32>( 2, 4 ), TVector2<int32>( 2, 16 ),
					TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, 0, 0 );
				if( rect.width <= 0 )
				{
					y += 3;
					nType = nLastType = 0;
					preRect = TRectangle<int32>( 0, 0, 0, 0 );
					continue;
				}
				int32 h1 = SRand::Inst().Rand( 4, 7 );
				if( h1 < rect.height - 5 )
					rect.height = h1;
				AddChunk( rect, eType_House_0, &m_vecHouse );
				nLastType = 2;
				preRect = TRectangle<int32>( 0, 0, 0, 0 );
				y += rect.height;
			}

			if( nType == 0 )
				nType = 1;
			else if( nType == 1 )
				nType = Max( 0, SRand::Inst().Rand( -1, 2 ) );
			else
				nType = 0;
		}
	}
}

void CLevelGenNode2_1_1::GenChunks1()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == 0 && ( j > 0 && m_gendata[i + ( j - 1 ) * nWidth] == eType_Road
				|| j < nHeight - 1 && m_gendata[i + ( j + 1 ) * nWidth] == eType_Road ) )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	int32 i = 0;

	SRand::Inst().Shuffle( m_vecHouse );
	for( int k = 0; k < vec.size(); k++ )
	{
		for( ; i < m_vecHouse.size(); i++ )
		{
			auto rect = m_vecHouse[i];
			if( rect.height > 2 )
				continue;

			bool b1 = false;
			int8 k0 = SRand::Inst().Rand( 0, 2 );
			for( int8 k1 = 0; k1 < 2; k1++ )
			{
				int8 nDir = k1 & k0;
				int32 y = nDir == 0 ? rect.y - 1 : rect.GetBottom();
				if( y < 1 || y >= nHeight - 1 )
					continue;
				int32 l, r;
				for( l = 0; l < rect.width; l++ )
				{
					bool b = false;
					for( int j = 0; j < 2; j++ )
					{
						int32 y1 = y + ( nDir == 0 ? -j : j );
						if( m_gendata[rect.x + l + y1 * nWidth] )
						{
							b = true;
							break;
						}
					}
					if( b )
						break;
				}
				for( r = 0; r < rect.width; r++ )
				{
					bool b = false;
					for( int j = 0; j < 2; j++ )
					{
						int32 y1 = y + ( nDir == 0 ? -j : j );
						if( m_gendata[rect.GetRight() - 1 - r + y1 * nWidth] )
						{
							b = true;
							break;
						}
					}
					if( b )
						break;
				}

				TRectangle<int32> r0( rect.x, y, rect.width, 1 );
				if( l >= rect.width )
					b1 = GenChunks2( r0, r0, nDir, b1 ) || b1;
				else
				{
					TRectangle<int32> rect1[2] = { { rect.x, y, l, 1 }, { rect.GetRight() - r, y, r, 1 } };
					SRand::Inst().Shuffle( rect1, 2 );
					for( int j = 0; j < 2; j++ )
					{
						if( rect1[j].width >= 2 )
							b1 = GenChunks2( rect1[j], r0, nDir, b1 ) || b1;
					}
				}
			}
		}

		auto p = vec[k];
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		int32 nDir = ( p.y > 0 && m_gendata[p.x + ( p.y - 1 ) * nWidth] == eType_Road ) ? 1 : 0;
		GenChunks3( p, nDir, -1, -1 );
	}
}

bool CLevelGenNode2_1_1::GenChunks2( const TRectangle<int32>& r, const TRectangle<int32>& r0, int8 nDir, bool b1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	bool b[2] = { r.x == r0.x, r.GetRight() == r0.GetRight() };
	int8 nDir1;
	if( b[0] && b[1] )
		nDir1 = SRand::Inst().Rand( 0, 2 );
	else if( b[0] )
		nDir1 = 0;
	else if( b[1] )
		nDir1 = 1;
	else
		nDir1 = -1;

	if( r.width >= ( b[0] || b[1] ? 5 : SRand::Inst().Rand( 6, 8 ) ) )
	{
		TVector2<int32> p( r.x + ( r.width - SRand::Inst().Rand( 0, 2 ) ) / 2, r.y );
		int32 x1 = r.x + ( nDir1 == 1 ? 0 : 1 );
		int32 x2 = r.GetRight() - ( nDir1 == 0 ? 0 : 1 );
		auto r1 = GenChunks3( p, nDir, x1, x2 );
		if( r1.width > 0 )
		{
			if( !b1 )
			{
				TRectangle<int32> rect1[2] = { { r.x, r.y, r1.x - r.x, 1 }, { r1.GetRight(), r1.y, r.GetRight() - r1.GetRight(), 1 } };
				SRand::Inst().Shuffle( rect1, 2 );
				for( int j = 0; j < 2; j++ )
				{
					if( rect1[j].width >= 2 )
						GenChunks2( rect1[j], r0, nDir, b1 );
				}
			}
			return true;
		}
	}
	if( !b[0] && !b[1] || b[0] && b[1] || b1 )
		return true;
	TRectangle<int32> bound( 0, 0, nWidth, nHeight );
	if( nDir == 0 )
		bound.SetBottom( r.y + 1 );
	else
		bound.SetTop( r.y );
	if( nDir1 == 0 )
		bound.SetRight( r0.x + r0.width / 2 );
	else
		bound.SetLeft( r0.GetRight() - r0.width / 2 );

	auto rect1 = PutRect( m_gendata, nWidth, nHeight, TRectangle<int32>( nDir1 == 1 ? r.GetRight() - 2 : 0, r.y, 2, 1 ),
		TVector2<int32>( 6, 2 ), TVector2<int32>( r.width + SRand::Inst().Rand( 6, 9 ), 2 ),
		bound, -1, eType_House_0, 0 );
	if( rect1.width > 0 )
	{
		m_vecHouse.push_back( rect1 );
		return true;
	}
	return false;
}

TRectangle<int32> CLevelGenNode2_1_1::GenChunks3( const TVector2<int32>& p, int8 nDir, int32 xBegin, int32 xEnd )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( m_gendata[p.x + p.y * nWidth] )
		return TRectangle<int32>( 0, 0, 0, 0 );
	TRectangle<int32> bound( xBegin, 0, xEnd - xBegin, nHeight );
	if( xBegin < 0 )
		bound = TRectangle<int32>( 0, 0, nWidth, nHeight );
	auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 3 ), TVector2<int32>( SRand::Inst().Rand( 4, 8 ), SRand::Inst().Rand( 3, 7 ) ),
		bound, -1, eType_House_1 );
	if( rect.width <= 0 )
		return TRectangle<int32>( 0, 0, 0, 0 );
	m_vecHouse.push_back( rect );

	TRectangle<int32> rect1( rect.x, nDir == 0 ? rect.y - 1 : rect.GetBottom(), rect.width, 1 );
	if( rect1.y < 0 || rect1.GetBottom() > nHeight )
		return rect;
	auto r1 = PutRect( m_gendata, nWidth, nHeight, rect1, TVector2<int32>( rect1.width + 2, 2 ), TVector2<int32>( Max( rect1.width + 4, SRand::Inst().Rand( 7, 12 ) ), 2 ),
		TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House_0, 0 );
	if( r1.width <= 0 )
	{
		TRectangle<int32> rect2[2] = { rect1, rect1 };
		rect2[0].width = rect1.width / 2;
		rect2[1].SetLeft( rect1.GetRight() - rect1.width / 2 );
		SRand::Inst().Shuffle( rect2, 2 );
		for( int k = 0; k < 2; k++ )
		{
			r1 = PutRect( m_gendata, nWidth, nHeight, rect2[k], TVector2<int32>( rect1.width + 2, 2 ), TVector2<int32>( Max( rect1.width + 4, SRand::Inst().Rand( 7, 12 ) ), 2 ),
				TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House_0, 0 );
			if( r1.width > 0 )
				break;
		}
	}
	if( r1.width > 0 )
		m_vecHouse.push_back( r1 );

	return rect;
}

void CLevelGenNode2_1_1::GenRoads()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	for( auto& rect : m_vecHouse )
	{
		bool b = false;
		for( int j = rect.y; j < rect.GetBottom() && !b; j += rect.height - 1 )
		{
			for( int i = rect.x; i < rect.GetRight() && !b; i++ )
			{
				int32 nDir = j == rect.y ? -1 : 1;
				for( int y = j + nDir; y >= 0 && y < nHeight; y += nDir )
				{
					if( m_gendata[i + y * nWidth] >= eType_House_0 )
						break;
					if( m_gendata[i + y * nWidth] >= eType_Road )
					{
						b = true;
						break;
					}
				}
			}
		}
		if( b )
			continue;

		for( int j = rect.y; j < rect.GetBottom() && !b; j += rect.height - 1 )
		{
			for( int i = rect.x; i < rect.GetRight() && !b; i++ )
			{
				int32 nDir = j == rect.y ? -1 : 1;
				for( int y = j + nDir; y >= 0 && y < nHeight; y += nDir )
				{
					if( m_gendata[i + y * nWidth] == eType_House_0 )
					{
						vecTemp[i + y * nWidth] = 1;
						break;
					}
				}
			}
		}
	}

	for( int i = m_vecHouse.size() - 1; i >= 0; i-- )
	{
		auto rect = m_vecHouse[i];
		if( rect.height > 2 )
			continue;

		int32 s = 0;
		for( int i = rect.x + 2; i < rect.GetRight() - 2; i++ )
		{
			if( rect.y > 0 && m_gendata[i + ( rect.y - 1 ) * nWidth] == eType_Road && vecTemp[i + ( rect.GetBottom() - 1 ) * nWidth] == 1
				|| rect.GetBottom() < nHeight && m_gendata[i + rect.GetBottom() * nWidth] == eType_Road && vecTemp[i + rect.y * nWidth] == 1 )
				s++;
		}
		if( s < rect.width / 4 )
			continue;

		m_vecHouse[i] = m_vecHouse.back();
		m_vecHouse.pop_back();
		AddChunk( rect, eType_Road_1_1, &m_vecRoad );
		uint8 nDir = SRand::Inst().Rand( 0, 2 );
		int32 k = 2;
		int32 k1 = SRand::Inst().Rand( 1, 3 );
		int32 k2 = SRand::Inst().Rand( 3, 5 );
		for( int i = 0; i < rect.width - 2; i++ )
		{
			int32 x = nDir == 0 ? i + rect.x : rect.GetRight() - 1 - i;
			bool b = rect.y > 0 && m_gendata[x + ( rect.y - 1 ) * nWidth] == eType_Road && vecTemp[x + ( rect.GetBottom() - 1 ) * nWidth] == 1
				|| rect.GetBottom() < nHeight && m_gendata[x + rect.GetBottom() * nWidth] == eType_Road && vecTemp[x + rect.y * nWidth] == 1;
			if( b )
			{
				if( !k && !k1 && k2 )
				{
					m_gendata[x + rect.y * nWidth] = m_gendata[x + ( rect.y + 1 ) * nWidth] = eType_Road;
					k2--;
					if( !k2 )
					{
						k = SRand::Inst().Rand( 2, 4 );
						k1 = SRand::Inst().Rand( 1, 3 );
						k2 = SRand::Inst().Rand( 3, 5 );
					}
				}
				if( k1 )
					k1--;
			}
			else
			{
				k1 = SRand::Inst().Rand( 1, 3 );
				k2 = SRand::Inst().Rand( 3, 5 );
			}
			if( k )
				k--;
		}
	}
}

void CLevelGenNode2_1_1::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector2<int32> > vec, vec1;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
			{
				vec1.push_back( TVector2<int32>( i, j ) );
				m_gendata[i + j * nWidth] = 0;
			}
			if( !m_gendata[i + j * nWidth] )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 5, 3 ), TVector2<int32>( SRand::Inst().Rand( 8, 12 ), SRand::Inst().Rand( 4, 6 ) ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Wall1 );
		if( rect.width > 0 )
			m_vecWall1.push_back( rect );
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Wall1 )
				m_gendata[i + j * nWidth] = 0;
		}
	}
	for( auto& p : vec1 )
	{
		if( !m_gendata[p.x + p.y * nWidth] )
			m_gendata[p.x + p.y * nWidth] = eType_Temp;
	}

	vec.clear();
	for( int i = 1; i < nWidth - 1; i++ )
	{
		for( int j = 1; j < nHeight; j++ )
		{
			if( !m_gendata[i + j * nWidth] && m_gendata[i + ( j - 1 ) * nWidth] && ( m_gendata[i - 1 + j * nWidth] || m_gendata[i + 1 + j * nWidth] ) )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		if( m_gendata[p.x - 1 + p.y * nWidth] && m_gendata[p.x + 1 + p.y * nWidth] )
			continue;
		int8 nDir = m_gendata[p.x - 1 + p.y * nWidth] ? 1 : -1;
		int32 x1;
		for( x1 = p.x; x1 > 0 && x1 < nWidth - 1; x1 += nDir )
		{
			int32 x2 = x1 + nDir;
			if( m_gendata[x2 + p.y * nWidth] || !m_gendata[x2 + ( p.y - 1 ) * nWidth] )
				break;
		}
		int32 y1;
		for( y1 = p.y; y1 < nHeight - 1; y1++ )
		{
			int32 x2 = p.x - nDir;
			int32 y2 = y1 + 1;
			if( m_gendata[p.x + y2 * nWidth] || !m_gendata[x2 + y2 * nWidth] )
				break;
		}
		TRectangle<int32> bound( Min( p.x, x1 ), p.y, Max( p.x, x1 ) - Min( p.x, x1 ) + 1, y1 - p.y + 1 );
		int32 s = ( bound.width * bound.height + SRand::Inst().Rand( 5, 15 ) ) / 10;
		for( int i = 0; i < bound.width; i++ )
		{
			int32 x = nDir == 1 ? p.x + i : p.x - i;
			if( m_gendata[x + p.y * nWidth] )
				continue;
			if( i >= SRand::Inst().Rand( -2, bound.width + 1 ) )
				continue;
			if( i >= SRand::Inst().Rand( 0, bound.width + 1 ) && s > 0 )
			{
				auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, p.y ), TVector2<int32>( 1, 2 ),
					TVector2<int32>( SRand::Inst().Rand( 1, 3 ), 2 ), bound, -1, eType_Obj );
				if( rect.width )
				{
					if( rect.width == 1 )
						m_vecGarbageBin.push_back( rect );
					else
						m_vecGarbageBin2.push_back( rect );
					s -= rect.width;
				}
				continue;
			}
			auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, p.y ), TVector2<int32>( 2, 2 ),
				TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 4 ) ), bound, -1, eType_Obj );
			if( rect.width )
				m_vecCargo.push_back( rect );
		}
	}

	for( auto& p : vec1 )
	{
		if( m_gendata[p.x + p.y * nWidth] == eType_Temp )
			m_gendata[p.x + p.y * nWidth] = 0;
	}
}

void CLevelGenNode2_1_1::GenHouse()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( auto& rect : m_vecRoad )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Road )
					m_gendata[i + j * nWidth] = 0;
			}
		}
	}
	for( auto& rect : m_vecHouse )
	{
		CLevelGen2::GenHouse( m_gendata, nWidth, nHeight, rect, eType_House_0 );
	}
	for( auto& rect : m_vecRoad )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == 0 )
					m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}

	for( auto& rect : m_vecRoom )
	{
		CLevelGen2::GenRoom( m_gendata, nWidth, nHeight, rect, eType_Room );
	}
}

void CLevelGenNode2_1_1::AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec )
{
	if( pVec )
		pVec->push_back( rect );
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			m_gendata[i + j * m_region.width] = nType;
		}
	}
}