#include "stdafx.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

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
		int32 s = bound.width * bound.height + SRand::Inst().Rand( 5, 15 ) / 20;
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

	vector<TVector2<int32> > vec;
	vector<int8> vecTemp;
	vector<int8> vecTemp1;
	vector<TVector2<int32> > q;
	for( auto& rect : m_vecHouse )
	{
		if( rect.width > 2 && rect.height > 2 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_House_1;
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
					m_gendata[x + j * nWidth] = eType_House_0;
				}
			}

			if( rect.height >= 3 )
			{
				for( int i = 0; i < 2; i++ )
				{
					for( int j = rect.y + SRand::Inst().Rand( 0, 2 ); j < rect.GetBottom() - 1; j++ )
					{
						int32 x = rect.x + ( rect.width - 1 ) * i;
						if( m_gendata[x + j * nWidth] == eType_House_0 )
							continue;
						int32 nDir = i * 2 - 1;
						int32 l;
						int32 l1 = SRand::Inst().Rand( 3, 5 );
						for( l = 0; l < l1; l++ )
						{
							int32 x1 = x + nDir * ( l + 1 );
							if( x1 < 0 || x1 >= nWidth || m_gendata[x1 + j * nWidth] )
								break;
						}
						if( l >= l1 )
						{
							m_gendata[x + j * nWidth] = eType_House_2;
						}
					}
				}
			}

			for( int i = 0; i < 2; i++ )
			{
				int32 x = rect.x + ( rect.width - 1 ) * i;
				for( int j = rect.y; j < rect.GetBottom() - 1; j++ )
				{
					if( m_gendata[x + j * nWidth] == eType_House_2 )
					{
						if( m_gendata[x + ( j + 1 ) * nWidth] != eType_House_2 )
						{
							m_gendata[x + j * nWidth] = eType_House_1;
							continue;
						}

						int32 l = Min( SRand::Inst().Rand( 2, 4 ), rect.GetBottom() - 1 - j );
						int32 k;
						for( k = 0; k < l; k++ )
						{
							if( m_gendata[x + ( j + k ) * nWidth] == eType_House_0 )
								break;
							m_gendata[x + ( j + k ) * nWidth] = eType_House_2;
						}
						if( m_gendata[x + ( j + k ) * nWidth] == eType_House_2 )
							m_gendata[x + ( j + k ) * nWidth] = eType_House_1;
						l = k;
						int32 l1 = SRand::Inst().Rand( 0, Max( 0, l - 2 ) + 1 );
						for( k = 0; k < l1; k++ )
							m_gendata[x + ( j + k ) * nWidth] = eType_House_1;
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
						int32 l1 = SRand::Inst().Rand( 3, 5 );
						for( l = 0; l < l1; l++ )
						{
							int32 y1 = y + nDir * ( l + 1 );
							if( y1 < 0 || y1 >= nHeight || m_gendata[j + y1 * nWidth] )
								break;
						}
						if( l >= l1 )
						{
							m_gendata[j + y * nWidth] = eType_Temp;
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
						if( x < rect.GetRight() - 2 && m_gendata[x + y * nWidth] == eType_Temp )
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
							m_gendata[i + y * nWidth] = eType_House_2;
							if( j == 0 )
								m_gendata[i + ( y + 1 ) * nWidth] = eType_House_2;
						}
					}
				}

				for( int i = 0; i < 2; i++ )
				{
					int32 y = rect.y + ( rect.height - 1 ) * i;
					for( int j = rect.x + 2; j < rect.GetRight() - 2; j++ )
					{
						if( m_gendata[j + y * nWidth] == eType_Temp )
							m_gendata[j + y * nWidth] = eType_House_1;
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
					vecTemp[i + j * rect.width] = m_gendata[x + y * nWidth];
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
					m_gendata[x + y * nWidth] = vecTemp[i + j * rect.width];
				}
			}
		}
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

bool SHouse::Generate( vector<int8>& genData, int32 nWidth, int32 nHeight, uint8 nType, uint8 nType0, uint8 nType0a, uint8 nType1, uint8 nType2,
	vector<int8>& genData1, uint8 nWalkableType, uint8 nPathType, vector<TVector2<int32> >& par )
{
	int32 l[4] = { rect.height, rect.width, rect.height, rect.width };
	int32 nLeft[4] = { 1, 0, 1, 0 };
	int32 nRight[4] = { 3, 2, 3, 2 };
	for( int i = 0; i < 4; i++ )
	{
		if( nExitType[i] == 1 )
			continue;
		vector<TVector2<int32> > q;
		if( nExitType[i] == 2 )
		{
			for( int j = exit[i].x; j < exit[i].x + exit[i].y; j++ )
			{
				TVector2<int32> p[4] = { { 0, j }, { j, 0 }, { rect.width - 1, j }, { j, rect.height - 1 } };
				TVector2<int32> pt = p[i] + TVector2<int32>( rect.x, rect.y );
				TVector2<int32> dir[4] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
				TVector2<int32> d = dir[i];
				for( pt = pt + d; pt.x >= 0 && pt.y >= 0 && pt.x < nWidth && pt.y < nHeight; pt = pt + d )
				{
					if( genData1[pt.x + pt.y * nWidth] != 1 )
						break;
					genData1[pt.x + pt.y * nWidth] = 3;
					q.push_back( pt );
				}
			}
			ExpandDist( genData1, nWidth, nHeight, 3, 1, 2, q );
		}
		else if( nExitType[i ^ 2] != 2 || l[i ^ 1] >= 5 )
		{
			int32 iLeft = nLeft[i];
			int32 iRight = nRight[i];
			int32 nLeft = nExitType[iLeft] == 2 ? 3 : 2;
			int32 nRight = nExitType[iRight] == 2 ? 3 : 2;
			if( 2 + nLeft + nRight > l[i] )
				continue;

			vector<TVector2<int32> > q1;
			for( int j = nLeft; j < l[i] - nRight; j++ )
			{
				TVector2<int32> p[4] = { { 0, j }, { j, 0 }, { rect.width - 1, j }, { j, rect.height - 1 } };
				TVector2<int32> pt = p[i] + TVector2<int32>( rect.x, rect.y );
				q1.push_back( pt );
			}

			if( !q1.size() )
				continue;
			SRand::Inst().Shuffle( q1 );
			TVector2<int32> p = FindPath( genData1, nWidth, nHeight, 1, 3, 2, q1, par );
			if( p.x < 0 )
				continue;

			exit[i] = TVector2<int32>( nLeft, l[i] - nRight - nLeft );
			nExitType[i] = 1;
			p = par[p.x + p.y * nWidth];
			while( p.x >= 0 && genData1[p.x + p.y * nWidth] == 3 )
			{
				q.push_back( p );
				p = par[p.x + p.y * nWidth];
			}
			genData[p.x + p.y * nWidth] = nType1;

			ExpandDist( genData1, nWidth, nHeight, 3, 1, 2, q );
		}

		for( auto& p : q )
		{
			genData1[p.x + p.y * nWidth] = 2;
			genData[p.x + p.y * nWidth] = nPathType;
		}
	}
	bool b = false;
	for( int i = 0; i < 4; i++ )
	{
		if( !nExitType[i] )
		{
			int32 iLeft = nLeft[i];
			int32 iRight = nRight[i];

			int32 nLeft = !nExitType[iLeft] ? 0 : ( nExitType[iLeft] == 2 ? 3 : 2 );
			int32 nRight = !nExitType[iRight] ? 0 : ( nExitType[iRight] == 2 ? 3 : 2 );

			exit[i] = TVector2<int32>( nLeft, l[i] - nRight - nLeft );
		}
		else
			b = true;
	}
	if( !b )
		return false;

	int32 cornerSize[8];
	for( int i = 0; i < 4; i++ )
	{
		cornerSize[i * 2] = exit[i].x;
		cornerSize[i * 2 + 1] = l[i] - exit[i].x - exit[i].y;
	}

	uint8 extends[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	uint8 nExtends = 8;

	while( nExtends )
	{
		uint32 s = cornerSize[0] * cornerSize[2] + cornerSize[1] * cornerSize[6] + cornerSize[4] * cornerSize[3] + cornerSize[5] * cornerSize[7];
		if( s >= rect.width * rect.height / 2 )
			break;

		uint8 r = SRand::Inst().Rand<uint8>( 0, nExtends );
		uint8 n = extends[r];

		uint8 bMinOrMax = n & 1;
		uint8 bXOrY = ( n & 2 ) >> 1;
		uint8 bLOrR = ( n & 4 ) >> 2;
		int32 nEdgeLen = bXOrY ? rect.width : rect.height;
		int32 nRandMin = 2;
		int32 nRandMax = nEdgeLen / 2 + 2;

		bool bSucceed = false;
		do
		{
			if( !cornerSize[n] )
				break;

			if( cornerSize[n ^ 1] && cornerSize[n] + cornerSize[n ^ 1] >= nEdgeLen - 2 )
				break;
			if( cornerSize[n] + cornerSize[n ^ 5] >= nEdgeLen - 1 )
				break;
			TVector2<int32> p( 0, cornerSize[n] );
			if( bMinOrMax )
				p.y = nEdgeLen - p.y - 1;
			if( bLOrR )
				p.x = ( bXOrY ? rect.height : rect.width ) - 1;
			if( bXOrY )
				swap( p.x, p.y );

			p = p + TVector2<int32>( rect.x, rect.y );
			if( genData[p.x + p.y * nWidth] == nType1 || genData[p.x + p.y * nWidth] == nType2 )
				break;

			cornerSize[n]++;
			bSucceed = true;
		} while( 0 );

		if( !bSucceed )
		{
			extends[r] = extends[nExtends - 1];
			nExtends--;
		}
	}

	for( int i = 0; i < 8; i++ )
	{
		int32 nEdgeLen = !!( i & 2 ) ? rect.width : rect.height;
		if( !cornerSize[i ^ 1] && cornerSize[i] == nEdgeLen - 1 )
			cornerSize[i] = nEdgeLen;
	}

	for( int i = 0; i < 4; i++ )
	{
		if( !nExitType[i] )
			continue;
		exit[i].x = cornerSize[i * 2];
		exit[i].y = l[i] - cornerSize[i * 2 + 1] - exit[i].x;
		uint8 t = nExitType[i] == 2 ? nType2 : nType1;
		for( int j = exit[i].x; j < exit[i].x + exit[i].y; j++ )
		{
			TVector2<int32> p[4] = { { 0, j }, { j, 0 }, { rect.width - 1, j }, { j, rect.height - 1 } };
			TVector2<int32> pt = p[i] + TVector2<int32>( rect.x, rect.y );
			genData[pt.x + pt.y * nWidth] = t;
			if( nExitType[i] == 2 )
			{
				TVector2<int32> p1[4] = { { 1, j }, { j, 1 }, { rect.width - 2, j }, { j, rect.height - 2 } };
				pt = p1[i] + TVector2<int32>( rect.x, rect.y );
					genData[pt.x + pt.y * nWidth] = t;
			}
		}
	}

	uint8 arr[4][2] = { { 2, 0 }, { 6, 1 }, { 3, 4 }, { 7, 5 } };
	for( int i = 0; i < 4; i++ )
	{
		int32 w = cornerSize[arr[i][0]];
		int32 h = cornerSize[arr[i][1]];
		int32 x = !!( arr[i][0] & 1 ) ? rect.width - w : 0;
		int32 y = !!( arr[i][1] & 1 ) ? rect.height - h : 0;

		for( int iX = 0; iX < w; iX++ )
		{
			for( int iY = 0; iY < h; iY++ )
			{
				genData[iX + x + rect.x + ( iY + y + rect.y ) * nWidth] = nType0 + i;
			}
		}

		if( w >= 4 && h >= 4 )
		{
			int32 nX = x + ( w - SRand::Inst().Rand( 0, 2 ) ) / 2 + rect.x;
			int32 nY = y + ( h - SRand::Inst().Rand( 0, 2 ) ) / 2 + rect.y;
			genData[nX + nY * nWidth] = nType0a;
		}
	}
	return true;
}

void CLevelGenNode2_1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pWalkableNodes[0] = CreateNode( pXml->FirstChildElement( "walkable_a" )->FirstChildElement(), context );
	m_pWalkableNodes[1] = CreateNode( pXml->FirstChildElement( "walkable_b" )->FirstChildElement(), context );
	m_pWalkableNodes[2] = CreateNode( pXml->FirstChildElement( "walkable_c" )->FirstChildElement(), context );
	m_pWalkableNodes[3] = CreateNode( pXml->FirstChildElement( "walkable_d" )->FirstChildElement(), context );
	m_pBlockNode = CreateNode( pXml->FirstChildElement( "block" )->FirstChildElement(), context );
	m_pBlockNodes[0] = CreateNode( pXml->FirstChildElement( "block_a" )->FirstChildElement(), context );
	m_pBlockNodes[1] = CreateNode( pXml->FirstChildElement( "block_b" )->FirstChildElement(), context );
	m_pBlockNodes[2] = CreateNode( pXml->FirstChildElement( "block_c" )->FirstChildElement(), context );
	m_pBlockNodes[3] = CreateNode( pXml->FirstChildElement( "block_d" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "fence" ) )
		m_pFenceNode = CreateNode( pXml->FirstChildElement( "fence" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "plant" ) )
		m_pPlantNode = CreateNode( pXml->FirstChildElement( "plant" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "room" ) )
		m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "cargo" ) )
		m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "cargo2" ) )
		m_pCargo2Node = CreateNode( pXml->FirstChildElement( "cargo2" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "barrel" ) )
		m_pBarrelNode = CreateNode( pXml->FirstChildElement( "barrel" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "barrel1" ) )
		m_pBarrel1Node = CreateNode( pXml->FirstChildElement( "barrel1" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenAreas();
	GenObjs();

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;
		}
	}
	for( auto& rect : m_vecRoads )
	{
		m_pRoadNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Chunk_Plant;
	if( m_pFenceNode )
	{
		for( auto& rect : m_vecFences )
		{
			m_pFenceNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	if( m_pPlantNode )
		m_pPlantNode->Generate( context, region );

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];

			if( genData == eType_Unwalkable )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			else if( genData >= eType_Walkable_a && genData <= eType_Walkable_d )
				m_pWalkableNodes[genData - eType_Walkable_a]->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			else if( genData >= eType_Block_a && genData <= eType_Block_d )
				m_pBlockNodes[genData - eType_Block_a]->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["mask"] = eType_Block;
	m_pBlockNode->Generate( context, region );

	context.mapTags["door"] = eType_Door;
	for( auto& room : m_vecRooms )
	{
		m_pRoomNode->Generate( context, room.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_House_1;
	context.mapTags["2"] = eType_House_2;
	context.mapTags["exit1"] = eType_House_Exit1;
	context.mapTags["exit2"] = eType_House_Exit2;
	context.mapTags["s"] = m_nType >= 4 ? 1 : 0;
	for( auto& house : m_vecHouses )
	{
		if( house.rect.width )
			m_pHouseNode->Generate( context, house.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	for( auto& rect : m_vecCargoSmall )
	{
		if( !rect.width )
			continue;
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecCargoLarge )
	{
		m_pCargo2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecBarrels )
	{
		m_pBarrelNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecBarrels1 )
	{
		m_pBarrel1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_par.clear();
	m_vecRoads.clear();
	m_vecFences.clear();
	m_vecFenceBlock.clear();
	m_vecArea1.clear();
	m_vecArea2.clear();
	m_vecRooms.clear();
	m_vecHouses.clear();
	m_vecCargoSmall.clear();
	m_vecCargoLarge.clear();
	m_vecBarrels.clear();
	m_vecBarrels1.clear();
}

void CLevelGenNode2_1_1::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	uint8 nType;
	if( !m_pFenceNode )
		nType = SRand::Inst().Rand( 0, 3 );
	else
		nType = SRand::Inst().Rand( 3, 5 );
	m_nType = nType;

	vector<TRectangle<int32> > tempRect;
	if( nType == 0 )
	{
		uint32 w0 = 8;
		uint32 l0 = SRand::Inst().Rand( ( nWidth - w0 ) / 2 - 1, nWidth - w0 - ( nWidth - w0 ) / 2 + 2 );
		uint32 h0 = SRand::Inst().Rand( nHeight / 2 - 1, nHeight / 2 + 1 );
		TRectangle<int32> r0( l0, 0, w0, h0 );

		uint32 lDist = SRand::Inst().Rand( 4, 6 );
		uint32 rDist = SRand::Inst().Rand( 4, 6 );
		TRectangle<int32> r1( lDist, h0 - 2, r0.x - lDist, 2 );
		TRectangle<int32> r2( r0.GetRight(), h0 - 2, nWidth - rDist - r0.GetRight(), 2 );

		TRectangle<int32> r3( r1.x, h0, 2, nHeight - h0 );
		TRectangle<int32> r4( r2.GetRight() - 2, h0, 2, nHeight - h0 );

		{
			uint32 w = SRand::Inst().Rand( 3, 5 );
			uint32 h = SRand::Inst().Rand( h0 / 2 - 1, h0 / 2 + 2 );
			m_vecArea1.push_back( TRectangle<int32>( r0.x - w, h0 - 2 - h, w, h ) );
			w = SRand::Inst().Rand( 3, 5 );
			h = SRand::Inst().Rand( h0 / 2 - 1, h0 / 2 + 2 );
			m_vecArea1.push_back( TRectangle<int32>( r0.GetRight(), h0 - 2 - h, w, h ) );
		}
		{
			TRectangle<int32> r( r1.x, r1.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
			r1.SetLeft( r.GetRight() );
			r3.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( r2.GetRight() - 2, r2.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
			r2.SetRight( r.GetLeft() );
			r4.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			uint32 h = SRand::Inst().Rand( 4, 6 );
			uint32 l = SRand::Inst().Rand( 4, 6 );
			uint32 r = SRand::Inst().Rand( 4, 6 );
			m_vecArea1.push_back( TRectangle<int32>( nWidth / 2 - l, r0.GetBottom(), l + r, h ) );
			SHouse house( nWidth / 2 - l, r0.GetBottom() + h, l + r, nHeight - r0.GetBottom() - h  );
			house.exit[1] = TVector2<int32>( ( house.rect.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}

		m_vecRoads.push_back( r0 );
		m_vecRoads.push_back( r1 );
		m_vecRoads.push_back( r2 );
		m_vecRoads.push_back( r3 );
		m_vecRoads.push_back( r4 );
	}
	else if( nType == 1 )
	{
		uint32 w0 = 8;
		uint32 l0 = SRand::Inst().Rand( ( nWidth - w0 ) / 2 - 1, nWidth - w0 - ( nWidth - w0 ) / 2 + 2 );
		uint32 h0 = SRand::Inst().Rand( 6, 8 );
		TRectangle<int32> r0( l0, 0, w0, h0 );

		uint32 h00 = h0 / 2 + SRand::Inst().Rand( 0, 2 );
		uint32 lDist = SRand::Inst().Rand( 3, 5 );
		uint32 rDist = SRand::Inst().Rand( 3, 5 );
		TRectangle<int32> r1( lDist, h00, r0.x - lDist, h0 - h00 );
		TRectangle<int32> r2( r0.GetRight(), h00, nWidth - rDist - r0.GetRight(), h0 - h00 );

		uint32 h1 = SRand::Inst().Rand( 6, 8 );
		TRectangle<int32> r3( r1.x, h0, 2, nHeight - h1 - h0 );
		TRectangle<int32> r4( r2.GetRight() - 2, h0, 2, nHeight - h1 - h0 );

		uint32 l = nWidth - rDist - lDist - 4;
		uint32 w2a = SRand::Inst().Rand( 3, 5 );
		uint32 w2b = SRand::Inst().Rand( 3, 5 );
		uint32 l2 = ( l - w2a - w2b ) / 2 - SRand::Inst().Rand( 0, 3 );
		uint32 l2a = lDist + 2 + SRand::Inst().Rand( ( l - l2 - w2a - w2b ) / 2 - 1, ( l - l2 - w2a - w2b ) - ( l - l2 - w2a - w2b ) / 2 + 2 );
		uint32 l2b = l2a + w2a + l2;
		uint32 h2 = Min( nHeight - r0.GetBottom() - 4, SRand::Inst().Rand( nHeight / 2 + 2, nHeight / 2 + 5 ) );
		TRectangle<int32> r5( l2a, nHeight - h2, w2a, h2 );
		TRectangle<int32> r6( l2b, nHeight - h2, w2b, h2 );

		{
			TRectangle<int32> r( r1.x, r1.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 1, 3 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 3, 4 ) );
			r1.SetLeft( r.GetRight() );
			r3.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( r2.GetRight() - 2, r2.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 1, 3 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 3, 4 ) );
			r2.SetRight( r.GetLeft() );
			r4.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( r1.x, 0, r1.width, r0.y );
			r.SetLeft( r.GetLeft() + SRand::Inst().Rand( 2, 6 ) );
			m_vecArea1.push_back( r );
		}
		{
			TRectangle<int32> r( r2.x, 0, r2.width, r0.y );
			r.SetRight( r.GetRight() - SRand::Inst().Rand( 2, 6 ) );
			m_vecArea1.push_back( r );
		}
		{
			TRectangle<int32> r( r5.x, r0.GetBottom(), r5.width, 0 );
			r.SetLeft( Max( r3.GetRight(), r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r5.GetRight() + ( r6.x - r5.GetRight() ) / 2, r.GetRight() + 2 ) );
			r.SetBottom( Min( r5.y, r.y + SRand::Inst().Rand( 4, 8 ) ) );
			SHouse house( r );
			house.exit[3] = TVector2<int32>( SRand::Inst().Rand( 0, r5.width - 1 ) + r5.x - r.x, 2 );
			house.nExitType[3] = 1;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r6.x, r0.GetBottom(), r6.width, 0 );
			r.SetLeft( Max( r6.x - ( r6.x - r5.GetRight() ) / 2, r.GetLeft() - 2 ) );
			r.SetRight( Min( r4.x, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r6.y, r.y + SRand::Inst().Rand( 4, 8 ) ) );
			SHouse house( r );
			house.exit[3] = TVector2<int32>( SRand::Inst().Rand( 0, r6.width - 1 ) + r6.x - r.x, 2 );
			house.nExitType[3] = 1;
			m_vecHouses.push_back( house );
		}

		{
			TRectangle<int32> r( r3.x, r3.GetBottom(), r3.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( 0, r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r5.x, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r3.width - 1 ) + r3.x - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r4.x, r4.GetBottom(), r4.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( r6.GetRight(), r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( nWidth, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r4.width - 1 ) + r4.x - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( 0, 0, 0, 0 );
			r.x = r3.GetRight();
			r.SetRight( r5.GetLeft() );
			r.y = Max( m_vecArea2[0].GetBottom(), m_vecHouses[0].rect.GetBottom() );
			r.SetBottom( m_vecHouses[2].rect.GetTop() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( 0, 0, 0, 0 );
			r.x = r6.GetRight();
			r.SetRight( r4.GetLeft() );
			r.y = Max( m_vecArea2[1].GetBottom(), m_vecHouses[1].rect.GetBottom() );
			r.SetBottom( m_vecHouses[3].rect.GetTop() );
			m_vecArea2.push_back( r );
		}

		m_vecRoads.push_back( r0 );
		m_vecRoads.push_back( r1 );
		m_vecRoads.push_back( r2 );
		m_vecRoads.push_back( r3 );
		m_vecRoads.push_back( r4 );
		m_vecRoads.push_back( r5 );
		m_vecRoads.push_back( r6 );
	}
	else if( nType == 2 )
	{
		uint32 w0 = SRand::Inst().Rand( 12, 15 );
		uint32 l0 = SRand::Inst().Rand( ( nWidth - w0 ) / 2 - 1, nWidth - w0 - ( nWidth - w0 ) / 2 + 2 );
		uint32 h0 = SRand::Inst().Rand( 3, 5 );
		TRectangle<int32> r0( l0, 0, w0, h0 );

		uint32 h1 = SRand::Inst().Rand( nHeight / 2 + 2, nHeight / 2 + 5 ) - h0;
		uint32 w1 = SRand::Inst().Rand( 3, 5 );
		uint32 w2 = SRand::Inst().Rand( 3, 5 );
		uint32 ofs1 = SRand::Inst().Rand( 1, 3 );
		uint32 ofs2 = SRand::Inst().Rand( 1, 3 );
		TRectangle<int32> r1( l0 - ofs1, h0, w1, h1 );
		TRectangle<int32> r2( l0 + w0 + ofs2 - w2, h0, w2, h1 );

		uint32 w3 = SRand::Inst().Rand( 2, 3 );
		uint32 l3 = SRand::Inst().Rand( ( nWidth - w3 ) / 2 - 1, nWidth - w3 - ( nWidth - w3 ) / 2 + 2 );
		uint32 h2 = SRand::Inst().Rand( 5, 7 );
		TRectangle<int32> r3( l3, nHeight - h2, w3, h2 );

		uint32 h3 = SRand::Inst().Rand( ( h1 + h0 ) / 2 - 2, ( h1 + h0 ) / 2 + 2 );
		uint32 lDist = SRand::Inst().Rand( 4, 6 );
		uint32 rDist = SRand::Inst().Rand( 4, 6 );
		TRectangle<int32> rc1( lDist, h3, r1.x - lDist, 2 );
		TRectangle<int32> rc2( r2.GetRight(), h3, nWidth - rDist - r2.GetRight(), 2 );

		{
			TRectangle<int32> r( r1.x, r1.GetBottom(), r1.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( 0, r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r3.x, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r1.width - 1 ) + r1.x - r.x, 2 );
			house.nExitType[1] = 1;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r2.x, r2.GetBottom(), r2.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( r3.GetRight(), r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( nWidth, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r2.width - 1 ) + r2.x - r.x, 2 );
			house.nExitType[1] = 1;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( 0, rc1.y, rc1.x, rc1.height );
			r.SetTop( Max( 0, r.GetTop() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r1.GetBottom(), r.GetBottom() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[2] = TVector2<int32>( SRand::Inst().Rand( 0, rc1.height - 1 ) + rc1.y - r.y, 2 );
			house.nExitType[2] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( rc2.GetRight(), rc2.y, nWidth - rc2.GetRight(), rc2.height );
			r.SetTop( Max( 0, r.GetTop() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r2.GetBottom(), r.GetBottom() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[0] = TVector2<int32>( SRand::Inst().Rand( 0, rc2.height - 1 ) + rc2.y - r.y, 2 );
			house.nExitType[0] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( rc1.x, rc1.GetBottom(), rc1.width, m_vecHouses[0].rect.y - rc1.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( rc2.x, rc2.GetBottom(), rc2.width, m_vecHouses[1].rect.y - rc2.GetBottom() );
			m_vecArea2.push_back( r );
		}

		m_vecRoads.push_back( r0 );
		m_vecRoads.push_back( r1 );
		m_vecRoads.push_back( r2 );
		m_vecRoads.push_back( r3 );
		m_vecRoads.push_back( rc1 );
		m_vecRoads.push_back( rc2 );
	}
	else if( nType == 3 )
	{
		uint32 w = SRand::Inst().Rand( 15, 18 );
		uint32 l = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
		uint32 r = nWidth - l - w;
		uint32 w1 = ( w - 4 + SRand::Inst().Rand( 0, 3 ) ) / 3;
		uint32 w2 = ( w - 4 + SRand::Inst().Rand( 0, 3 ) ) / 3;
		uint32 w3 = w - 4 - w1 - w2;
		if( w1 < w3 )
			swap( w1, w3 );
		if( w2 < w3 )
			swap( w2, w3 );
		if( SRand::Inst().Rand( 0, 2 ) )
			swap( w1, w2 );

		uint32 h0 = 4;
		uint32 h1 = SRand::Inst().Rand( 7, 10 );
		if( w3 < 4 )
			h1 = Max( h1, h0 + 4 );
		uint32 h2 = Min<uint32>( nHeight / 2 + SRand::Inst().Rand( -1, 2 ), h1 + SRand::Inst().Rand( 2, 5 ) );

		m_vecRoads.push_back( TRectangle<int32>( l, 0, w1, nHeight - h0 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + 2, 0, w3, nHeight - h1 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + w3 + 4, 0, w2, nHeight - h0 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1, 0, 2, nHeight - h0 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1 + w3 + 2, 0, 2, nHeight - h0 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1, 0, 2, nHeight - h2 - 2 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1, nHeight - h2, 2, h2 - h0 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1 + w3 + 2, 0, 2, nHeight - h2 - 2 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1 + w3 + 2, nHeight - h2, 2, h2 - h0 ) );

		tempRect.push_back( TRectangle<int32>( l + w1, nHeight - h1, w - w1 - w2, h1 ) );
		if( w3 >= 4 )
			m_vecBarrels1.push_back( TRectangle<int32>( SRand::Inst().Rand( m_vecRoads[1].x, m_vecRoads[1].GetRight() - 3 ), nHeight - h1 + 1, 4, 2 ) );
		else
			m_vecBarrels.push_back( TRectangle<int32>( SRand::Inst().Rand( m_vecRoads[1].x, m_vecRoads[1].GetRight() - 1 ), nHeight - h1 + 1, 2, 3 ) );

		{
			TRectangle<int32> r( l, nHeight - h0, w1, h0 );
			r.SetLeft( r.GetLeft() - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( ( w1 - 1 ) / 2 + l - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( l + w - w2, nHeight - h0, w2, h0 );
			r.SetLeft( r.GetLeft() - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( ( w2 - 2 ) / 2 + l + w - w2 - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
	}
	else if( nType == 4 )
	{
		uint32 w = SRand::Inst().Rand( 15, 18 );
		uint32 l = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
		uint32 r = nWidth - l - w;
		uint32 w1 = 3;
		uint32 w2 = 3;
		uint32 w3 = w - 4 - w1 - w2;
		uint32 h0 = SRand::Inst().Rand( nHeight / 2 - 3, nHeight / 2 + 1 );
		uint32 h00 = h0 - 2;
		uint32 h1 = SRand::Inst().Rand( 6, 8 );
		uint32 h2 = SRand::Inst().Rand( 6, 8 );
		uint32 h3 = 4;

		m_vecRoads.push_back( TRectangle<int32>( l, h0, w1, nHeight - h0 - h1 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + 2, h00, w3, nHeight - h00 - h3 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + w3 + 4, h0, w2, nHeight - h0 - h2 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1, h0, 2, nHeight - h0 - h3 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1 + w3 + 2, h0, 2, nHeight - h0 - h3 ) );
		m_vecFenceBlock.push_back( m_vecFences[0] );
		m_vecFenceBlock.push_back( m_vecFences[1] );
		m_vecFenceBlock[0].SetTop( m_vecFenceBlock[0].y + SRand::Inst().Rand( 2, 4 ) );
		m_vecFenceBlock[1].SetTop( m_vecFenceBlock[1].y + SRand::Inst().Rand( 2, 4 ) );
		tempRect.push_back( TRectangle<int32>( l, nHeight - h1, w1, h1 ) );
		tempRect.push_back( TRectangle<int32>( l + w - w2, nHeight - h2, w2, h2 ) );

		m_vecBarrels.push_back( TRectangle<int32>( SRand::Inst().Rand( l, l + w1 - 1 ), nHeight - h1 + 1, 2, 3 ) );
		m_vecBarrels.push_back( TRectangle<int32>( SRand::Inst().Rand( l + w - w2, l + w - 1 ), nHeight - h2 + 1, 2, 3 ) );

		{
			TRectangle<int32> r( l + w1, nHeight - h3, w3 + 4, h3 );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( ( w3 - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2 + l + w1 + 2 - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}

		m_vecRoads.push_back( TRectangle<int32>( l, 0, w1 + 2, h0 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w - w2 - 2, 0, w2 + 2, h0 ) );
		TRectangle<int32> rect( l + w1 + 2, 0, w - w1 - w2 - 4, h00 );
		m_vecFences.push_back( rect );
		rect.SetLeft( rect.x + 1 );
		rect.SetRight( rect.GetRight() - 1 );
		rect.height -= SRand::Inst().Rand( 1, 3 );
		m_vecFenceBlock.push_back( rect );
	}

	for( auto& road : m_vecRoads )
	{
		for( int i = road.x; i < road.GetRight(); i++ )
		{
			for( int j = road.y; j < road.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}
	for( auto& fence : m_vecFences )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk1;
			}
		}
	}
	for( auto& fence : m_vecFenceBlock )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk_Plant;
			}
		}
	}
	for( auto& house : m_vecHouses )
	{
		for( int i = house.rect.x; i < house.rect.GetRight(); i++ )
		{
			for( int j = house.rect.y; j < house.rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_House;
			}
		}
	}
	for( auto& rect : m_vecArea1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp0;
			}
		}
	}
	for( auto& rect : m_vecArea2 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp1;
			}
		}
	}
	for( auto& rect : m_vecBarrels )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}
	for( auto& rect : m_vecBarrels1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}

	if( m_pRoomNode )
	{
		if( m_nType < 3 )
		{
			int32 nRoomCount = 2;
			for( int i = 0; i < 2 && nRoomCount; i++ )
			{
				vector<TVector2<int32> > vec;
				uint8 nType = i == 0 ? eType_Temp1 : eType_None;
				FindAllOfTypesInMap( m_gendata, nWidth, nHeight, nType, vec );
				SRand::Inst().Shuffle( vec );
				for( auto& p : vec )
				{
					if( m_gendata[p.x + p.y * nWidth] != nType )
						continue;

					auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 6, 6 ), TVector2<int32>( 8, 8 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Room );
					if( rect.width > 0 )
					{
						m_vecRooms.push_back( rect );
						nRoomCount--;
						if( !nRoomCount )
							break;
					}
				}
			}
		}

		for( auto& room : m_vecRooms )
		{
			for( int i = room.x; i < room.GetRight(); i++ )
			{
				for( int j = room.y; j < room.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_Room;
				}
			}

			if( room.width >= 6 )
			{
				int32 x = room.x + ( room.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				int32 y = room.y;
				m_gendata[x + y * nWidth] = m_gendata[x + 1 + y * nWidth] = eType_Door;
				x = room.x + ( room.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				y = room.GetBottom() - 1;
				m_gendata[x + y * nWidth] = m_gendata[x + 1 + y * nWidth] = eType_Door;
			}

			if( room.height >= 6 )
			{
				int32 x = room.x;
				int32 y = room.y + ( room.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				m_gendata[x + y * nWidth] = m_gendata[x + ( y + 1 ) * nWidth] = eType_Door;
				x = room.GetRight() - 1;
				y = room.y + ( room.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				m_gendata[x + y * nWidth] = m_gendata[x + ( y + 1 ) * nWidth] = eType_Door;
			}
		}
	}

	for( auto& rect : tempRect )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp2;
			}
		}
	}
	if( m_nType >= 3 )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			uint32 h1 = SRand::Inst().Rand( 6, 9 );
			uint32 h2 = SRand::Inst().Rand( 4, 6 );

			for( int j = h1; j < nHeight - h2; j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					m_gendata[i + j * nWidth] = eType_Temp0;
			}
		}
	}

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	if( nType >= 3 )
		FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Temp0, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( nType < 3 && m_vecHouses.size() >= 6 )
			break;
		if( m_gendata[p.x + p.y * nWidth] != eType_None && ( nType < 3 || m_gendata[p.x + p.y * nWidth] != eType_Temp0 ) )
			continue;

		int i = 0;
		if( nType < 3 || m_gendata[p.x + p.y * nWidth] == eType_None )
		{
			TVector2<int32> ofs[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
			for( i = 0; i < 4; i++ )
			{
				auto p1 = p + ofs[i];
				if( p1.x < 0 || p1.y < 0 || p1.x >= nWidth || p1.y >= nHeight )
					continue;
				if( m_gendata[p1.x + p1.y * nWidth] == eType_Road || m_gendata[p1.x + p1.y * nWidth] == eType_Temp0 )
					break;
			}
			if( i >= 4 )
				continue;
		}

		TVector2<int32> sizeMin = i == 0 || i == 2 ? TVector2<int32>( 4, 6 ) : TVector2<int32>( 6, 4 );
		TVector2<int32> sizeMax;
		if( nType < 3 )
			sizeMax = TVector2<int32>( SRand::Inst().Rand( 8, 11 ), SRand::Inst().Rand( 8, 11 ) );
		else
			sizeMax = sizeMin + TVector2<int32>( SRand::Inst().Rand( 0, 3 ), SRand::Inst().Rand( 0, 3 ) );

		auto rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin, sizeMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House );
		if( rect.width > 0 )
			m_vecHouses.push_back( rect );
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp0 )
				m_gendata1[i + j * nWidth] = 1;
			else if( m_gendata[i + j * nWidth] == eType_Road )
				m_gendata1[i + j * nWidth] = 2;
			else
				m_gendata1[i + j * nWidth] = 0;
		}
	}

	for( auto& house : m_vecHouses )
	{
		if( !house.Generate( m_gendata, nWidth, nHeight, eType_House, eType_House_1, eType_House_2, eType_House_Exit1, eType_House_Exit2,
			m_gendata1, eType_Temp0, eType_Temp0_0, m_par ) )
		{
			for( int i = 0; i < house.rect.width; i++ )
			{
				for( int j = 0; j < house.rect.height; j++ )
				{
					m_gendata[i + house.rect.x + ( j + house.rect.y ) * nWidth] = eType_None;
				}
			}
			house.rect.width = house.rect.height = 0;
		}
	}

	for( auto& rect : tempRect )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp2 )
					m_gendata[i + j * nWidth] = eType_None;
			}
		}
	}
}

void CLevelGenNode2_1_1::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector2<int32> > vecRoad;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Road, vecRoad );

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& rect : m_vecArea2 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp1 )
					m_gendata[i + j * nWidth] = eType_None;
			}
		}
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] > eType_Road )
				m_gendata1[i + j * nWidth] = eType1_Block;
			else
				m_gendata1[i + j * nWidth] = eType1_None;
		}
	}

	for( auto p : vec )
	{
		if( m_gendata1[p.x + p.y * nWidth] != eType1_None )
			continue;
		uint8 nType = 0;
		if( p.x > 0 && m_gendata[p.x - 1 + p.y * nWidth] == eType_Road
			|| p.x < nWidth - 1 && m_gendata[p.x + 1 + p.y * nWidth] == eType_Road
			|| p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Road )
			nType = 1;

		PlaceChunk( p, nType );
	}

	for( auto& rect : m_vecArea2 )
	{
		vec.clear();
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					vec.push_back( TVector2<int32>( i, j ) );
			}
		}
		SRand::Inst().Shuffle( vec );
		int32 s = vec.size() / 3;

		for( auto p : vec )
		{
			if( m_gendata1[p.x + p.y * nWidth] != eType1_None )
				continue;
			uint8 nType = 0;
			if( p.x > 0 && m_gendata[p.x - 1 + p.y * nWidth] == eType_Road
				|| p.x < nWidth - 1 && m_gendata[p.x + 1 + p.y * nWidth] == eType_Road
				|| p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Road )
				nType = 1;
			else
				nType = SRand::Inst().Rand( 0, 2 );

			auto chunk = PlaceChunk( p, nType );
			s -= chunk.width * chunk.height;
			if( s <= 0 )
				break;
		}
	}

	for( auto& rect : m_vecCargoSmall )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_None;
			}
		}
	}
	LvGenLib::DropObj1( m_gendata1, nWidth, nHeight, m_vecCargoSmall, eType1_None, eType1_Obj );

	for( auto& rect : m_vecArea2 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}

	for( auto& p : vecRoad )
	{
		m_gendata[p.x + p.y * nWidth] = eType_Road;
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
			{
				if( m_gendata1[i + j * nWidth] == eType1_None )
					m_gendata[i + j * nWidth] = eType_Block;
				else if( m_gendata1[i + j * nWidth] == eType1_Obj )
					m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}

	int8 nTypes[4] = { eType_Walkable_a, eType_Walkable_b, eType_Walkable_c, eType_Walkable_d };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 64, 128, eType_Temp0_0, nTypes, 4 );
	int8 nTypes1[4] = { eType_Block_a, eType_Block_b, eType_Block_c, eType_Block_d };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 64, 128, eType_Temp0, nTypes1, 4 );
}

TRectangle<int32> CLevelGenNode2_1_1::PlaceChunk( TVector2<int32>& p, uint8 nType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( nType == 1 )
	{
		auto rect = PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 2, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType1_Obj );
		if( rect.width > 0 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_Temp1;
					m_gendata1[i + j * nWidth] = eType1_Obj;
				}
			}
			m_vecCargoSmall.push_back( rect );
		}
		return rect;
	}
	else
	{
		TVector2<int32> maxSize;
		maxSize.x = SRand::Inst().Rand( 3, 7 );
		maxSize.y = Min( maxSize.x, SRand::Inst().Rand( 2, 5 ) );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Chunk );
		if( rect.width > 0 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_Chunk;
					m_gendata1[i + j * nWidth] = eType1_Obj;
				}
			}
			m_vecCargoLarge.push_back( rect );
		}
		return rect;
	}
}

void CLevelGenNode2_1_2::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pWalkableNodes[0] = CreateNode( pXml->FirstChildElement( "walkable_a" )->FirstChildElement(), context );
	m_pWalkableNodes[1] = CreateNode( pXml->FirstChildElement( "walkable_b" )->FirstChildElement(), context );
	m_pWalkableNodes[2] = CreateNode( pXml->FirstChildElement( "walkable_c" )->FirstChildElement(), context );
	m_pWalkableNodes[3] = CreateNode( pXml->FirstChildElement( "walkable_d" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	m_pFenceNode = CreateNode( pXml->FirstChildElement( "fence" )->FirstChildElement(), context );
	m_pPlantNode = CreateNode( pXml->FirstChildElement( "plant" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_1_2::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenAreas();
	GenObjs();

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;
		}
	}
	for( auto& rect : m_vecRoads )
	{
		m_pRoadNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Chunk_Plant;
	for( auto& rect : m_vecFences )
	{
		m_pFenceNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	m_pPlantNode->Generate( context, region );

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];

			if( genData >= eType_Walkable_a && genData <= eType_Walkable_d )
				m_pWalkableNodes[genData - eType_Walkable_a]->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["door"] = eType_Door;
	for( auto& room : m_vecRooms )
	{
		m_pRoomNode->Generate( context, room.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_House_1;
	context.mapTags["2"] = eType_House_2;
	context.mapTags["exit1"] = eType_House_Exit1;
	context.mapTags["exit2"] = eType_House_Exit2;
	context.mapTags["s"] = 1;
	for( auto& house : m_vecHouses )
	{
		if( house.rect.width )
			m_pHouseNode->Generate( context, house.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_par.clear();
	m_vecRoads.clear();
	m_vecFences.clear();
	m_vecFenceBlock.clear();
	m_vecRooms.clear();
	m_vecHouses.clear();
}

void CLevelGenNode2_1_2::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	
	vector<TRectangle<int32> > vecTempRect;
	uint32 l = SRand::Inst().Rand( 4, 6 );
	uint32 r = SRand::Inst().Rand( 4, 6 );
	uint32 w = nWidth - l - r;
	uint32 h0 = 4;
	m_vecRoads.push_back( TRectangle<int32>( 3, 0, nWidth - 6, h0 ) );
	m_vecFences.push_back( TRectangle<int32>( 0, 0, 3, h0 ) );
	m_vecFences.push_back( TRectangle<int32>( nWidth - 3, 0, 3, h0 ) );
	m_vecFenceBlock.push_back( m_vecFences[0] );
	m_vecFenceBlock.push_back( m_vecFences[1] );

	{
		TRectangle<int32> rect( l, h0, nWidth - l - r, 2 );
		m_vecFences.push_back( rect );
		vecTempRect.push_back( m_vecFences.back().Offset( TVector2<int32>( 0, 2 ) ) );
		uint32 nSplit = ( rect.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		m_vecFenceBlock.push_back( TRectangle<int32>( l + 2, h0, nSplit - 3, 2 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + nSplit + 1, h0, rect.width - nSplit - 3, 2 ) );
	}
	h0 += 2;

	uint32 h1 = 4;
	TRectangle<int32> centerRect( l, h0, nWidth - r - l, nHeight - h1 - h0 );

	uint32 w1[3] = { ( centerRect.width - 4 ) / 3, ( centerRect.width - 3 ) / 3, ( centerRect.width - 2 ) / 3 };
	SRand::Inst().Shuffle( w1, ELEM_COUNT( w1 ) );
	m_vecRoads.push_back( TRectangle<int32>( centerRect.x + w1[0], centerRect.y, 2, centerRect.height ) );
	vecTempRect.push_back( TRectangle<int32>( centerRect.x + w1[0] - 2, centerRect.y, 6, 4 ) );
	m_vecRoads.push_back( TRectangle<int32>( centerRect.x + w1[0] + w1[1] + 2, centerRect.y, 2, centerRect.height ) );
	vecTempRect.push_back( TRectangle<int32>( centerRect.x + w1[0] + w1[1], centerRect.y, 6, 4 ) );

	TRectangle<int32> houseRect1( centerRect.x + w1[0], centerRect.GetBottom(), w1[1] + 4, 4 );
	uint32 nSplit = ( houseRect1.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
	{
		TRectangle<int32> rect = houseRect1;
		rect.width = nSplit;
		rect.SetLeft( rect.x - SRand::Inst().Rand( 2, 4 ) );
		SHouse house( rect );
		house.exit[1] = TVector2<int32>( w1[0] + centerRect.x - rect.x, 2 );
		house.nExitType[1] = 2;
		m_vecHouses.push_back( house );
	}
	{
		TRectangle<int32> rect = houseRect1;
		rect.SetLeft( rect.x + nSplit );
		rect.SetRight( rect.GetRight() + SRand::Inst().Rand( 2, 4 ) );
		SHouse house( rect );
		house.exit[1] = TVector2<int32>( w1[0] + w1[1] + 2 + centerRect.x - rect.x, 2 );
		house.nExitType[1] = 2;
		m_vecHouses.push_back( house );
	}

	for( int i = 0; i < 2; i++ )
	{
		uint32 hSplit = SRand::Inst().Rand( 1, 3 );
		uint32 nSplit = centerRect.y - 2 + ( centerRect.height + SRand::Inst().Rand( 0, 2 ) - hSplit ) / 2;

		TRectangle<int32> rect( i * ( centerRect.GetRight() - w1[2] ), nSplit,
			i == 0 ? centerRect.x + w1[0] : nWidth - centerRect.GetRight() + w1[2], hSplit );
		m_vecFences.push_back( rect );
		m_vecFenceBlock.push_back( rect );

		{
			TRectangle<int32> houseRect( i * centerRect.GetRight(), centerRect.y - 2,
				i == 0 ? centerRect.x : nWidth - centerRect.GetRight(), nSplit - ( centerRect.y - 2 ) );
			SHouse house( houseRect );
			/*uint32 nExitLen = SRand::Inst().Rand( 2, houseRect.height - 3 );
			house.exit[2 - i * 2] = TVector2<int32>( 2 + SRand::Inst().Rand( 0u, houseRect.height - 3 - nExitLen ), nExitLen );
			house.nExitType[2 - i * 2] = 2;*/
			m_vecHouses.push_back( house );
		}

		{
			TRectangle<int32> houseRect( i * centerRect.GetRight(), nSplit + hSplit,
				i == 0 ? centerRect.x : nWidth - centerRect.GetRight(), centerRect.GetBottom() - 2 - nSplit - hSplit );
			if( houseRect.width > 4 && SRand::Inst().Rand( 0, 2 ) )
			{
				houseRect.width--;
				if( i )
					houseRect.x++;
			}
			SHouse house( houseRect );
			uint32 nExitLen = SRand::Inst().Rand( 2, houseRect.height - 3 );
			house.exit[2 - i * 2] = TVector2<int32>( 2 + SRand::Inst().Rand( 0u, houseRect.height - 3 - nExitLen ), nExitLen );
			house.nExitType[2 - i * 2] = 2;
			m_vecHouses.push_back( house );
		}

		{
			TRectangle<int32> r1 = m_vecHouses.back().rect;
			TRectangle<int32> r2 = m_vecHouses[i].rect;
			TRectangle<int32> houseRect( i * r2.GetRight(), r1.GetBottom(), i == 0 ? r2.x : nWidth - r2.GetRight(), nHeight - r1.GetBottom() );
			uint32 w = Min( houseRect.width - 4, SRand::Inst().Rand( 2, 4 ) );
			uint32 h = SRand::Inst().Rand( 1, 3 );
			TRectangle<int32> fenceRect( i * ( centerRect.GetRight() - w ), houseRect.y, w, h );
			m_vecFences.push_back( fenceRect );
			m_vecFenceBlock.push_back( fenceRect );
			houseRect.SetTop( houseRect.y + h );
			
			SHouse house( houseRect );
			/*uint32 nExitLen = SRand::Inst().Rand( 2, houseRect.width - 4 );
			house.exit[1] = TVector2<int32>( 3 - i + SRand::Inst().Rand( 0u, houseRect.width - 4 - nExitLen ), nExitLen );
			house.nExitType[1] = 2;*/
			m_vecHouses.push_back( house );
		}
	}

	TRectangle<int32> centerRect1( centerRect.x + w1[0] + 2, centerRect.y, w1[1], centerRect.height );
	{
		uint32 nHouseWidth = Min( centerRect1.width, SRand::Inst().Rand( 6, 9 ) );
		uint32 nHouseHeight = SRand::Inst().Rand( 6, 9 );
		TRectangle<int32> rect( centerRect1.x + ( centerRect1.width + SRand::Inst().Rand( 0, 2 ) - nHouseWidth ) / 2,
			centerRect1.y + ( centerRect1.height + SRand::Inst().Rand( 0, 2 ) - nHouseHeight ) / 2, nHouseWidth, nHouseHeight );
		SHouse house( rect );
		m_vecHouses.push_back( house );
	}

	for( auto& road : m_vecRoads )
	{
		for( int i = road.x; i < road.GetRight(); i++ )
		{
			for( int j = road.y; j < road.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}
	for( auto& fence : m_vecFences )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk1;
			}
		}
	}
	for( auto& fence : m_vecFenceBlock )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk_Plant;
			}
		}
	}

	for( auto& room : m_vecRooms )
	{
		for( int i = room.x; i < room.GetRight(); i++ )
		{
			for( int j = room.y; j < room.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Room;
			}
		}

		if( room.width >= 6 )
		{
			int32 x = room.x + ( room.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 y = room.y;
			m_gendata[x + y * nWidth] = m_gendata[x + 1 + y * nWidth] = eType_Door;
			x = room.x + ( room.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			y = room.GetBottom() - 1;
			m_gendata[x + y * nWidth] = m_gendata[x + 1 + y * nWidth] = eType_Door;
		}

		if( room.height >= 6 )
		{
			int32 x = room.x;
			int32 y = room.y + ( room.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			m_gendata[x + y * nWidth] = m_gendata[x + ( y + 1 ) * nWidth] = eType_Door;
			x = room.GetRight() - 1;
			y = room.y + ( room.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			m_gendata[x + y * nWidth] = m_gendata[x + ( y + 1 ) * nWidth] = eType_Door;
		}
	}

	for( auto& house : m_vecHouses )
	{
		for( int i = house.rect.x; i < house.rect.GetRight(); i++ )
		{
			for( int j = house.rect.y; j < house.rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_House;
			}
		}
	}

	for( auto& rect : vecTempRect )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					m_gendata[i + j * nWidth] = eType_Temp0;
			}
		}
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
				m_gendata1[i + j * nWidth] = 1;
			else if( m_gendata[i + j * nWidth] == eType_Road || m_gendata[i + j * nWidth] == eType_Chunk1 || m_gendata[i + j * nWidth] == eType_Temp0 )
				m_gendata1[i + j * nWidth] = 2;
			else
				m_gendata1[i + j * nWidth] = 0;
		}
	}

	for( auto& house : m_vecHouses )
	{
		if( !house.Generate( m_gendata, nWidth, nHeight, eType_House, eType_House_1, eType_House_2, eType_House_Exit1, eType_House_Exit2,
			m_gendata1, eType_None, eType_Temp0, m_par ) )
		{
			for( int i = 0; i < house.rect.width; i++ )
			{
				for( int j = 0; j < house.rect.height; j++ )
				{
					m_gendata[i + house.rect.x + ( j + house.rect.y ) * nWidth] = eType_None;
				}
			}
			house.rect.width = house.rect.height = 0;
		}
	}
}

void CLevelGenNode2_1_2::GenObjs()
{
	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;

	vector<TVector2<int32> > vecTemp;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vecTemp );
	SRand::Inst().Shuffle( vecTemp );
	TVector2<int32> sizeMin[2] = { { 3, 1 }, { 1, 3 } };
	TVector2<int32> sizeMax[2] = { { 5, 1 }, { 1, 5 } };
	for( auto& p : vecTemp )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;

		int32 i = SRand::Inst().Rand( 0, 2 );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin[i], sizeMax[i], TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Chunk );
		if( !rect.width )
		{
			rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin[1 - i], sizeMax[1 - i], TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Chunk );
			if( !rect.width )
				continue;
		}

		m_vecFences.push_back( rect );
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
				m_gendata[i + j * nWidth] = eType_Temp0;
		}
	}

	int8 nTypes[4] = { eType_Walkable_a, eType_Walkable_b, eType_Walkable_c, eType_Walkable_d };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 64, 128, eType_Temp0, nTypes, 4 );
}
