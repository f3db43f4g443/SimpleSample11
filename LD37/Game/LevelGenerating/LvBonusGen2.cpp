#include "stdafx.h"
#include "LvBonusGen2.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include "LvGen2.h"
#include <algorithm>

void CLevelBonusGenNode2_0::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pWall1Node = CreateNode( pXml->FirstChildElement( "wall1" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelBonusGenNode2_0::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenAreas();
	GenHouses();

	context.mapTags["is_bonus"] = 1;
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

	context.mapTags.clear();
	m_gendata.clear();
	m_vecWall1.clear();
	m_vecRoad.clear();
	m_vecHouse.clear();
	m_vecRoom.clear();
	m_vecCargo.clear();
}

void CLevelBonusGenNode2_0::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<int32> vec;
	int32 h0 = SRand::Inst().Rand( 4, 6 );
	vec.push_back( h0 );
	int32 s = nHeight - h0;
	while( s > 0 )
	{
		int32 l = SRand::Inst().Rand( 14, 28 );
		if( s >= 26 )
			l = Min( s - 9, SRand::Inst().Rand( 17, 28 ) );
		else if( s < 17 )
		{
			vec[0] += s;
			break;
		}
		else
			l = s;

		s -= l;
		vec.push_back( l - 3 );
	}
	SRand::Inst().Shuffle( &vec[1], vec.size() - 1 );
	int32 y = 0;
	int32 xBegin1 = 0, xEnd1 = nWidth;
	int32 xBegin2, xEnd2;
	TRectangle<int32> road;
	int8 nPreType;
	for( int i = 0; i < vec.size(); i++ )
	{
		int32 h = vec[i];
		int8 nType = i == 0 ? 1 : ( i == 1 ? 0 : ( h >= 23 ? 0 : SRand::Inst().Rand( 0, 2 ) ) );
		if( nType == 0 )
			GenAreas1( y, y + h, xBegin1, xEnd1, xBegin2, xEnd2 );
		else
			GenAreas2( y, y + h, xBegin2, xEnd2 );
		GenObj( y, y + h );
		if( i > 0 )
			GenSplitRoad( road, xBegin1, xEnd1, nPreType, nType );
		xBegin1 = xBegin2;
		xEnd1 = xEnd2;
		y += h;
		nPreType = nType;
		if( y < nHeight )
		{
			road = TRectangle<int32>( 0, y, nWidth, 3 );
			AddChunk( road, eType_Road_1_1, &m_vecRoad );
			y += 3;
		}
	}

	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	vector<TVector2<int32> > vecP;
	for( int i = 0; i < vecTemp.size(); i++ )
	{
		if( m_gendata[i] != 0 && m_gendata[i] != eType_Obj )
			vecTemp[i] = 1;
		else
			vecP.push_back( TVector2<int32>( i % nWidth, i / nWidth ) );
	}
	SRand::Inst().Shuffle( vecP );
	for( auto& p : vecP )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( vecTemp, nWidth, nHeight, p, TVector2<int32>( 4, 3 ), TVector2<int32>( SRand::Inst().Rand( 8, 13 ), SRand::Inst().Rand( 4, 6 ) ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
		if( rect.width > 0 )
			m_vecWall1.push_back( rect );
	}
}

void CLevelBonusGenNode2_0::GenAreas1( int32 y, int32 y1, int32 xBegin1, int32 xEnd1, int32& xBegin2, int32& xEnd2 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 y0 = y;
	vector<int32> vec;
	{
		int32 h = y1 - y + 2;
		while( h > 0 )
		{
			int32 n;
			if( h <= SRand::Inst().Rand( 14, 17 ) )
				n = h;
			else
				n = SRand::Inst().Rand( 7, 9 );
			h -= n;
			vec.push_back( n - 2 );
		}
		SRand::Inst().Shuffle( vec );
	}
	int8 nDir = xBegin1 + xEnd1 < nWidth ? 1 : 0;
	int8 nPreType = -1;
	int8 b2 = 0;
	vector<pair<TRectangle<int32>, int8> > vecDoors;
	for( int i0 = 0; i0 < vec.size(); i0++ )
	{
		int32 h = vec[i0];
		int8 nType;
		if( h <= 6 )
			nType = 0;
		else if( h <= 11 )
			nType = 1;
		else
			nType = 2;
		int32 w2 = SRand::Inst().Rand( 6, 9 );
		if( nDir == 1 )
		{
			xBegin2 = nWidth - w2;
			xEnd2 = Min( nWidth - 2, xBegin2 + SRand::Inst().Rand( 3, 6 ) );
		}
		else
		{
			xEnd2 = w2;
			xBegin2 = Max( 2, xEnd2 - SRand::Inst().Rand( 3, 6 ) );
		}
		if( nType == 0 )
		{
			int8 b1 = nPreType == nType;
			if( !b1 )
				b2 = SRand::Inst().Rand( 0, 2 );
			int8 b3 = nDir ^ b2;
			int8 b4 = b1 ^ b2;
			TVector2<int32> size;
			size.x = Max( 4, ( b2 ? Min( xEnd2, nWidth - xBegin2 ) : Min( xEnd1, nWidth - xBegin1 ) ) - ( b4 ? 0 : SRand::Inst().Rand( 2, 4 ) ) );
			size.y = SRand::Inst().Rand( h + 1, h + 4 );
			TVector2<int32> p( b3 ? 0 : nWidth - 1, b1 ? y + h - 1 : y );
			TRectangle<int32> bound( 0, 0, nWidth, nHeight );
			if( b1 )
			{
				bound.SetTop( y );
				bound.SetBottom( p.y + 1 );
			}
			else
			{
				bound.SetTop( p.y );
				bound.SetBottom( y + h );
			}

			if( !b4 && b1 )
			{
				auto& last = m_vecHouse.back();
				AddChunk( last, 0, NULL );
			}
			auto rect0 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 5 ), size, bound, -1, eType_Room_1 );
			auto room = rect0;
			int32 yCar = Min( room.GetBottom() - 2, y + h - SRand::Inst().Rand( 1, 3 ) );
			if( !b4 )
			{
				int32 nRoomWidth1 = SRand::Inst().Rand( 4, 8 );
				if( room.width >= nRoomWidth1 + 2 )
				{
					auto rect1 = room;
					if( b3 )
					{
						room.width = nRoomWidth1;
						rect1.SetLeft( room.GetRight() );
					}
					else
					{
						room.SetLeft( room.GetRight() - nRoomWidth1 );
						rect1.SetRight( room.x );
					}
					AddChunk( rect1, 0, NULL );
					rect1.y = y;
					rect1.height = h;
					AddChunk( rect1, eType_Road_1_1, &m_vecRoad );
				}
				if( b1 )
				{
					auto& last = m_vecHouse.back();
					if( b3 )
						last.SetLeft( room.GetRight() );
					else
						last.SetRight( room.x );
					AddChunk( last, eType_House_0, NULL );
				}
			}
			for( int i = rect0.x; i < rect0.GetRight(); i++ )
			{
				for( int j = yCar - 1; j <= yCar; j++ )
				{
					if( m_gendata[i + j * nWidth] == eType_Road_1_1 )
						m_gendata[i + j * nWidth] = eType_Road;
					else
						m_gendata[i + j * nWidth] = b3 ? eType_Room_Car_0 : eType_Room_Car_2;
				}
			}
			m_vecRoom.push_back( room );

			int32 w1 = nWidth - rect0.width - ( b2 ? Min( xEnd1, nWidth - xBegin1 ) : w2 );
			int32 nSplit = Max( 2, w1 / SRand::Inst().Rand( 6, 9 ) );
			vector<int32> vecLen;
			vecLen.resize( nSplit );
			for( int i = 0; i < nSplit; i++ )
				vecLen[i] = ( w1 + i ) / nSplit;
			SRand::Inst().Shuffle( vecLen );
			int32 nLen = 0;
			for( int i = 0; i < vecLen.size(); i++ )
			{
				int32 l = vecLen[i];
				int32 l1 = Max( 2, SRand::Inst().Rand( l / 3, Max( l / 3 + 1, l / 2 ) ) );
				if( i > 0 )
					AddCargos0( TRectangle<int32>( b3 ? rect0.GetRight() + nLen : rect0.x - nLen - l, y, l, h ) );
				else
					AddChunk( TRectangle<int32>( b3 ? rect0.GetRight() + nLen : rect0.x - nLen - l, y, l, h ), eType_Temp, NULL );
				TRectangle<int32> r1( b3 ? rect0.GetRight() + nLen + l - l1 : rect0.x - nLen - l, y, l1, h );
				AddChunk( r1, eType_Road, &m_vecRoad );
				if( ( b2 || i == vecLen.size() ) && SRand::Inst().Rand( 0, 2 ) )
				{
					int32 y = r1.y + SRand::Inst().Rand( 2, h - 2 );
					for( int x = r1.x; x < r1.GetRight(); x++ )
						m_gendata[x + y * nWidth] = eType_Road_1_1;
				}
				else
				{
					for( int x = r1.x; x < r1.GetRight(); x++ )
					{
						for( int y = r1.y; y < r1.GetBottom(); y += r1.height - 1 )
							m_gendata[x + y * nWidth] = eType_Road_1_1;
					}
				}
				nLen += l;
			}

			if( b4 )
			{
				int32 nDoor = b1 ? Max( room.x + 2, Min( room.GetRight() - 2, SRand::Inst().Rand( xBegin2, xEnd2 ) ) ) :
					( b3 ? room.GetRight() - 2 : room.x + 2 );
				int32 yDoor = b1 ? room.y : room.GetBottom() - 1;
				m_gendata[nDoor - 1 + yDoor * nWidth] = m_gendata[nDoor + yDoor * nWidth] = eType_Room_Door;
				vecDoors.push_back( pair<TRectangle<int32>, int8>( TRectangle<int32>( nDoor - 1, yDoor, 2, 1 ), b1 ) );
			}
			vecDoors.push_back( pair<TRectangle<int32>, int8>( TRectangle<int32>( xBegin2, y + h, xEnd2 - xBegin2, 1 ), 1 ) );
			if( y + h < y1 )
			{
				if( b1 )
				{
					TRectangle<int32> r1( 0, y + h, nWidth, 2 );
					AddChunk( r1, eType_Road_1_1, &m_vecRoad );
					for( int x = xBegin2; x < xEnd2; x++ )
					{
						for( int y = r1.y; y < r1.GetBottom(); y++ )
							m_gendata[x + y * nWidth] = eType_Road;
					}
				}
				else
				{
					if( b2 )
					{
						if( b3 )
							AddChunk( TRectangle<int32>( room.GetRight(), y + h, nWidth - room.GetRight(), 2 ), eType_House_0, &m_vecHouse );
						else
							AddChunk( TRectangle<int32>( 0, y + h, room.x, 2 ), eType_House_0, &m_vecHouse );
					}
					else
						AddChunk( TRectangle<int32>( Min( room.GetRight(), xEnd2 ), y + h, Max( room.x, xBegin2 ) - Min( room.GetRight(), xEnd2 ), 2 ), eType_House_0, &m_vecHouse );
				}
			}
		}
		else if( nType == 1 )
		{
			TRectangle<int32> rect( xBegin1, y, xEnd1 - xBegin1, 2 );
			rect = PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), TVector2<int32>( rect.width + 2, 6 ), TRectangle<int32>( 0, y, nWidth, h ), SRand::Inst().Rand( 4, 6 ), eType_Temp, 0 );
			int32 nDir1 = rect.y == y;
			while( nDir ? rect.GetRight() < xEnd2 : rect.x > xBegin2 )
			{
				int32 x1 = Min( rect.width, SRand::Inst().Rand( 1, 3 ) );
				int32 y1 = Min( rect.height, SRand::Inst().Rand( 1, 3 ) );
				TRectangle<int32> rect1 = TRectangle<int32>( nDir ? rect.GetRight() - x1 : rect.x, nDir1 ? rect.GetBottom() - y1 : rect.y, x1, y1 );
				if( nDir1 )
				{
					int32 l = SRand::Inst().Rand( 5, 8 );
					if( l >= y + h - rect1.y - 2 )
						l = y + h - rect1.y;
					rect1.SetBottom( rect1.y + l );
				}
				else
				{
					int32 l = SRand::Inst().Rand( 5, 8 );
					if( l >= rect1.GetBottom() - y - 2 )
						l = rect1.GetBottom() - y;
					rect1.SetTop( rect1.GetBottom() - l );
				}
				int32 wRect = Min( 7, Max( 2, SRand::Inst().Rand( 8, 10 ) - rect1.height ) );
				if( nDir )
					rect1.width = wRect;
				else
					rect1.SetLeft( rect1.GetRight() - wRect );
				rect1 = rect1 * TRectangle<int32>( 0, 0, nWidth, nHeight );
				AddChunk( rect1, eType_Temp, NULL );
				int32 k = SRand::Inst().Rand( 0, 2 );
				for( int k1 = 0; k1 < 2; k1++ )
				{
					if( k1 ^ k )
					{
						if( rect1.width - x1 < 2 )
							continue;
						int32 w1 = Min( rect1.width - 2, x1 + SRand::Inst().Rand( 0, 1 ) );
						TRectangle<int32> rect2 = rect1;
						if( nDir1 )
							rect2.SetTop( rect.GetBottom() );
						else
							rect2.SetBottom( rect.y );
						if( nDir )
						{
							rect2.width = w1;
							rect1.SetLeft( rect1.x + w1 );
						}
						else
						{
							rect1.width -= w1;
							rect2.SetLeft( rect1.GetRight() );
						}
						AddChunk( rect2, 0, NULL );
					}
					else
					{
						if( rect1.height - y1 < 2 )
							continue;
						int32 h1 = Min( rect1.height - 2, y1 + SRand::Inst().Rand( 0, 1 ) );
						TRectangle<int32> rect2 = rect1;
						if( nDir )
							rect2.SetLeft( rect.GetRight() );
						else
							rect2.SetRight( rect.x );
						if( nDir1 )
						{
							rect2.height = h1;
							rect1.SetTop( rect1.y + h1 );
						}
						else
						{
							rect1.height -= h1;
							rect2.SetTop( rect1.GetBottom() );
						}
						AddChunk( rect2, 0, NULL );
					}
					break;
				}
				if( rect.height <= 2 || ( nDir == 1 ? rect1.GetRight() - rect.GetRight() : rect.x - rect1.x ) >= 4 )
				{
					if( nDir1 == 1 )
					{
						if( rect1.GetBottom() == y + h )
							nDir1 = 0;
					}
					else
					{
						if( rect1.y == y )
							nDir1 = 1;
					}
				}
				rect = rect1;
			}
			if( rect.GetBottom() < y + h )
			{
				rect.y = rect.GetBottom();
				rect.SetBottom( y + h );
				if( nDir )
				{
					rect.SetRight( Max( rect.x + 1, xEnd2 ) );
					rect.SetLeft( Min( rect.x, xEnd2 - 1 ) );
				}
				else
				{
					rect.SetLeft( Min( rect.GetRight() - 1, xBegin2 ) );
					rect.SetRight( Max( rect.GetRight(), xBegin2 + 1 ) );
				}
				if( rect.width )
				{
					AddChunk( rect, eType_Temp, NULL );
					TVector2<int32> p( nDir == 1 ? rect.x : rect.GetRight() - 1, rect.GetBottom() - 1 );
					auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 3 ), TVector2<int32>( nWidth, h ),
						TRectangle<int32>( 0, y, nWidth, h ), -1, 0 );
					if( rect.width > 0 )
					{
						m_gendata[p.x + p.y * nWidth] = eType_Temp;
						ConnectAll( m_gendata, nWidth, nHeight, eType_Temp, 0, TRectangle<int32>( 0, y, nWidth, h ) );
					}
				}
			}

			vector<TVector2<int32> > vecP;
			rect = TRectangle<int32>( Min( xBegin1, xBegin2 ), y, Max( xEnd1, xEnd2 ) - Min( xBegin1, xBegin2 ), h );
			for( int i = 0; i < rect.width; i++ )
			{
				int32 x = nDir == 0 ? rect.x + i : rect.GetRight() - 1 - i;
				for( int j = 0; j < h; j += h - 1 )
				{
					int32 j1 = j == 0 ? j + 1 : j - 1;
					if( m_gendata[x + ( j + y ) * nWidth] == eType_Temp )
					{
						if( !( j == 0 && x >= xBegin1 && x < xEnd1 || j > 0 && x >= xBegin2 && x < xEnd2 ) )
							m_gendata[x + ( j + y ) * nWidth] = 0;
						if( !m_gendata[x + ( j1 + y ) * nWidth] )
							m_gendata[x + ( j1 + y ) * nWidth] = eType_Temp;
						vecP.push_back( TVector2<int32>( x, j ) );
					}
				}
			}

			vector<int8> vecTemp;
			vecTemp.resize( nWidth * h );
			vector<TVector2<int32> > vecP1;
			for( int i = 0; i < nWidth; i++ )
			{
				for( int j = y; j < y + h; j++ )
				{
					if( !m_gendata[i + j * nWidth] )
						vecP1.push_back( TVector2<int32>( i, j ) );
				}
			}
			SRand::Inst().Shuffle( vecP1 );
			int32 n = 2;
			for( auto& p : vecP1 )
			{
				if( m_gendata[p.x + p.y * nWidth] )
					continue;
				auto rect1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 4 ), TVector2<int32>( 4, 4 ), TRectangle<int32>( 0, y, nWidth, h ), -1, 0 );
				if( rect1.width > 0 )
				{
					int8 nTypes[] = { 0, rect1.x + rect1.GetRight() + SRand::Inst().Rand( 0, 2 ) > nWidth ? 1 : 2 };
					SRand::Inst().Shuffle( nTypes, 2 );
					int8 nType = -1;
					auto Func = [] ( int8& n1, int8& n2, int32& l, bool b ) -> bool {
						if( n1 != n2 || n1 != 0 && n1 != eType_Temp )
							return false;
						if( l == -1 )
						{
							if( n1 == eType_Temp )
								l = -2;
						}
						else if( l == -2 )
						{
							if( n1 == 0 )
								l = 3;
						}
						else
						{
							if( n1 == 0 )
							{
								l--;
								if( !l )
									return false;
							}
							else
								return false;
						}
						if( b )
							n1 = n2 = eType_Temp1;
						return true;
					};
					for( int iType = 0; iType < 2; iType++ )
					{
						if( nTypes[iType] == 0 )
						{
							int32 j = rect1.y - 1;
							int32 l = -1;
							for( ; j >= y; j-- )
							{
								auto& n1 = m_gendata[rect1.x + 1 + j * nWidth];
								auto& n2 = m_gendata[rect1.x + 2 + j * nWidth];
								if( !Func( n1, n2, l, false ) )
									break;
							}
							if( l != 0 )
								continue;
						}
						else
						{
							int32 i = nTypes[iType] == 1 ? rect1.x - 1 : rect1.GetRight();
							int32 di = nTypes[iType] == 1 ? -1 : 1;
							int32 l = -1;
							for( ; i >= 0 && i < nWidth; i += di )
							{
								auto& n1 = m_gendata[i + ( rect1.y + 1 ) * nWidth];
								auto& n2 = m_gendata[i + ( rect1.y + 2 ) * nWidth];
								if( !Func( n1, n2, l, false ) )
									break;
							}
							if( l != 0 )
								continue;
						}
						nType = nTypes[iType];
						break;
					}

					if( nType >= 0 )
					{
						AddChunk( rect1, eType_Room_1, &m_vecRoom );
						for( int i = rect1.x; i < rect1.GetRight(); i++ )
						{
							for( int j = rect1.y; j < rect1.GetBottom(); j++ )
							{
								vecTemp[i + ( j - y ) * nWidth] = 1;
							}
						}
						if( nType == 0 )
						{
							int32 j = rect1.y - 1;
							int32 l = -1;
							for( ; j >= y; j-- )
							{
								auto& n1 = m_gendata[rect1.x + 1 + j * nWidth];
								auto& n2 = m_gendata[rect1.x + 2 + j * nWidth];
								if( !Func( n1, n2, l, true ) )
									break;
							}
							rect1.x++;
							rect1.width -= 2;
							AddChunk( rect1, eType_Room_Car_3, NULL );
						}
						else
						{
							int32 i = nType == 1 ? rect1.x - 1 : rect1.GetRight();
							int32 di = nType == 1 ? -1 : 1;
							int32 l = -1;
							for( ; i >= 0 && i < nWidth; i += di )
							{
								auto& n1 = m_gendata[i + ( rect1.y + 1 ) * nWidth];
								auto& n2 = m_gendata[i + ( rect1.y + 2 ) * nWidth];
								if( !Func( n1, n2, l, true ) )
									break;
							}
							rect1.y++;
							rect1.height -= 2;
							AddChunk( rect1, nType == 1 ? eType_Room_Car_2 : eType_Room_Car_0, NULL );
						}
						n--;
						if( !n )
							break;
					}
				}
			}

			for( auto& p : vecP )
			{
				if( vecTemp[p.x + p.y * nWidth] )
					continue;
				auto rect = PutRect( vecTemp, nWidth, h, p, TVector2<int32>( 5, 5 ), TVector2<int32>( SRand::Inst().Rand( 6, 9 ), SRand::Inst().Rand( 6, 9 ) ),
					TRectangle<int32>( 0, 0, nWidth, h ), 11, 1 );
				if( rect.width )
				{
					TRectangle<int32> rect1( rect.x - 2, rect.y, rect.width + 4, rect.height );
					rect1 = rect1 * TRectangle<int32>( 0, 0, nWidth, h );
					for( int i = rect1.x; i < rect1.GetRight(); i++ )
					{
						for( int j = rect1.y; j < rect1.GetBottom(); j++ )
						{
							vecTemp[i + j * nWidth] = 1;
						}
					}

					rect.y += y;
					bool b = false;
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						for( int j = rect.y; j < rect.GetBottom(); j++ )
						{
							if( m_gendata[i + j * nWidth] == 0 )
								m_gendata[i + j * nWidth] = eType_Room_1;
							else
								m_gendata[i + j * nWidth] = eType_Room_2;
						}
					}
					m_vecRoom1.push_back( rect );
				}
			}

			vecP.resize( 0 );
			if( y + h < y1 )
			{
				TRectangle<int32> r1( 0, y + h, nWidth, 2 );
				AddChunk( r1, eType_Road_1_1, &m_vecRoad );
				for( int x = xBegin2; x < xEnd2; x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
						m_gendata[x + y * nWidth] = eType_Road;
				}
			}

			for( int y = rect.y; y < rect.GetBottom(); y += rect.height - 1 )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
					vecP.push_back( TVector2<int32>( x, y ) );
			}
			SRand::Inst().Shuffle( vecP );
			int32 nSize0 = vecP.size();
			for( int y = rect.y + 1; y < rect.GetBottom() - 1; y ++ )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
					vecP.push_back( TVector2<int32>( x, y ) );
			}
			SRand::Inst().Shuffle( &vecP[nSize0], vecP.size() - nSize0 );
			TVector2<int32> minSize[3] = { { 3, 3 }, { 2, 4 }, { 4, 2 } };
			TVector2<int32> maxSize[3] = { { 8, 7 }, { 2, 16 }, { 16, 2 } };
			int32 ext[3] = { 9, -1, -1 };
			vector<TRectangle<int32> > vecRects;
			for( auto p : vecP )
			{
				if( m_gendata[p.x + p.y * nWidth] )
					continue;
				for( int i = 0; i < 3; i++ )
				{
					auto r1 = PutRect( m_gendata, nWidth, nHeight, p, minSize[i], maxSize[i], TRectangle<int32>( 0, y, nWidth, h ), ext[i], eType_House_0 );
					if( r1.width > 0 )
					{
						vecRects.push_back( r1 );
						break;
					}
				}
			}
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					if( m_gendata[i + j * nWidth] == 0 )
						m_gendata[i + j * nWidth] = eType_Temp;
				}
			}
			for( int i = 0; i < vecRects.size(); i++ )
			{
				auto rect = vecRects[i];
				if( rect.height >= 6 )
				{
					auto rect1 = rect;
					rect.SetTop( rect1.GetBottom() - SRand::Inst().Rand( 2, 4 ) );
					rect1.SetBottom( rect.GetTop() );
					AddChunk( rect, eType_Temp, NULL );
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						for( int j = y; j < y + h; j++ )
						{
							if( !m_gendata[i + j * nWidth] )
								m_gendata[i + j * nWidth] = eType_Temp;
						}
					}
					auto rect2 = PutRect( m_gendata, nWidth, nHeight, rect, TVector2<int32>( rect.width + 3, rect.height + 1 ), TVector2<int32>( nWidth, rect.height + 1 ),
						TRectangle<int32>( 0, y, nWidth, h ), -1, eType_Temp, eType_Temp );
					if( rect2.width <= 0 )
					{
						AddChunk( rect, eType_House_0, NULL );
						continue;
					}
					rect.SetLeft( rect2.x + 1 );
					rect.SetRight( rect2.GetRight() - 1 );
					AddChunk( rect, eType_House_0, NULL );
					vecRects[i] = rect1;
					vecRects.push_back( rect );
				}
			}
			for( auto& rect : vecRects )
			{
				m_vecHouse.push_back( rect );
			}
			AddCargos( rect, eType_Temp, 0 );
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					if( m_gendata[i + j * nWidth] == eType_Temp1 )
						m_gendata[i + j * nWidth] = eType_Temp;
				}
			}
		}
		else
		{
			if( y + h < y1 )
			{
				TRectangle<int32> r1( 0, y + h, nWidth, 2 );
				AddChunk( r1, eType_Road_1_1, &m_vecRoad );
				for( int x = xBegin2; x < xEnd2; x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
						m_gendata[x + y * nWidth] = eType_Road;
				}
			}

			int8 nRoomPos[2];
			nRoomPos[1] = SRand::Inst().Rand( 0, 2 ) * 2;
			nRoomPos[0] = nRoomPos[1] + 1;
			TRectangle<int32> room[4];
			TRectangle<int32> car[4];
			int8 carDir[4];
			for( int i = 0; i < 4; i++ )
			{
				room[i].width = SRand::Inst().Rand( 4, 7 );
				room[i].height = Min( ( h - 1 ) / 2, SRand::Inst().Rand( 5, 8 ) );
				if( i == 0 )
				{
					room[i].x = nDir == 1 ? 0 : nWidth - room[i].width;
					room[i].y = y + h - room[i].height;
				}
				else if( i == 1 )
				{
					room[i].x = nDir == 1 ? Max( xBegin1 + 1, xEnd1 - SRand::Inst().Rand( 0, 3 ) ) :
						Min( xEnd1 - 1, xBegin1 + SRand::Inst().Rand( 0, 3 ) ) - room[i].width;
					room[i].y = y;
				}
				else if( i == 2 )
				{
					room[i].width = Min( ( nDir == 0 ? xEnd2 : nWidth - xBegin2 ) - 1, room[i].width );
					room[i].x = nDir == 0 ? 0 : nWidth - room[i].width;
					room[i].y = y + h - room[i].height;
				}
				else
				{
					room[i].x = nDir == 0 ? 0 : nWidth - room[i].width;
					room[i].y = y + SRand::Inst().Rand( 2, 5 );
					room[i].y = Min( room[2].y - room[i].height, room[i].y );
				}
				AddChunk( room[i], eType_Room_1, &m_vecRoom );
				carDir[i] = nDir ^ ( i >= 2 );
				car[i] = TRectangle<int32>( room[i].x, room[i].y + ( room[i].height - SRand::Inst().Rand( 0, 2 ) ) / 2, room[i].width, 2 );
				AddChunk( car[i], carDir[i] == 1 ? eType_Room_Car_0 : eType_Room_Car_2, NULL );
			}
			vector<TRectangle<int32> > vecCars;
			for( int i = 0; i < 2; i++ )
			{
				int8 n = nRoomPos[i];
				int32 h1 = Min( 4, car[n].y - y );
				vecCars.push_back( TRectangle<int32>( carDir[n] == 1 ? car[n].GetRight() : car[n].x - 4, car[n].y, 4, car[n].height ) );
				CreateCarPath( car[n], carDir[n], y, h1, false );
			}
			for( int i = xBegin1; i < xEnd1; i++ )
			{
				for( int j = y; j < y + h; j++ )
				{
					if( m_gendata[i + j * nWidth] )
						break;
					m_gendata[i + j * nWidth] = eType_Temp;
				}
			}

			vector<TVector2<int32> > vecP;
			for( int k = 0; k < 2; k++ )
			{
				for( int i = 0; i < nWidth; i++ )
				{
					for( int j = y + 1; j < y + h; j++ )
					{
						if( m_gendata[i + j * nWidth] == 0 && ( k == 1 || m_gendata[i + ( j - 1 ) * nWidth] == eType_Temp ) )
							vecP.push_back( TVector2<int32>( i, j ) );
					}
				}
				SRand::Inst().Shuffle( vecP );
				for( auto& p : vecP )
				{
					if( m_gendata[p.x + p.y * nWidth] )
						continue;
					auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 2 ), TVector2<int32>( SRand::Inst().Rand( 4, 6 ), 2 ),
						TRectangle<int32>( 0, y, nWidth, h ), -1, eType_House_0 );
					if( rect.width )
					{
						m_vecHouse.push_back( rect );
						for( int i = Max( 0, rect.x - 3 ); i < Min( nWidth, rect.GetRight() + 3 ); i++ )
						{
							int32 j1 = Min( i - ( rect.x - 3 ), rect.GetRight() + 2 - i );
							for( int j = Max( y, rect.y - j1 ); j < Min( y + h, rect.GetBottom() + j1 ); j++ )
							{
								if( !m_gendata[i + j * nWidth] )
									m_gendata[i + j * nWidth] = eType_Temp1;
							}
						}
					}
				}
				vecP.resize( 0 );
			}
			for( int i = 0; i < nWidth; i++ )
			{
				for( int j = y; j < y + h; j++ )
				{
					if( m_gendata[i + j * nWidth] == eType_Temp1 )
						m_gendata[i + j * nWidth] = 0;
				}
			}
			for( int i = xBegin2; i < xEnd2; i++ )
			{
				for( int j = y + h - 1; j >= y; j-- )
				{
					if( m_gendata[i + j * nWidth] )
						break;
					m_gendata[i + j * nWidth] = eType_Temp;
				}
			}
			for( int i = 0; i < nWidth; i++ )
			{
				for( int j = y; j < y + h; j++ )
				{
					if( m_gendata[i + j * nWidth] == eType_Temp )
					{
						m_gendata[i + j * nWidth] = 0;
						vecP.push_back( TVector2<int32>( i, j ) );
					}
				}
			}
			for( auto rect : vecCars )
			{
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						if( m_gendata[i + j * nWidth] == 0 )
							m_gendata[i + j * nWidth] = eType_Temp;
					}
				}
			}
			AddCargos( TRectangle<int32>( 0, y, nWidth, h ), 0, 1 );
			for( auto& p : vecP )
			{
				if( m_gendata[p.x + p.y * nWidth] == 0 )
					m_gendata[p.x + p.y * nWidth] = eType_Temp;
			}
		}

		y += h;
		if( y < y1 )
			y += 2;
		xBegin1 = xBegin2;
		xEnd1 = xEnd2;
		nDir = 1 - nDir;
		if( nType == 0 && nPreType == 0 )
			nPreType = -1;
		else
			nPreType = nType;
	}
	for( auto& door : vecDoors )
	{
		auto rect = door.first;
		rect.y += door.second ? -1 : 1;
		if( rect.y < y0 || rect.y >= y1 )
			continue;
		PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), TVector2<int32>( rect.width, SRand::Inst().Rand( 3, 8 ) ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp, 0 );
	}
}

void CLevelBonusGenNode2_0::GenAreas2( int32 y, int32 y1, int32& xBegin2, int32& xEnd2 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 h = y1 - y;
	vector<TRectangle<int32> > vecRoom;
	vector<TRectangle<int32> > vecHouses;
	vector<TRectangle<int32> > vecCars;
	int32 nDir = SRand::Inst().Rand( 0, 2 );
	if( nDir == 0 )
	{
		xBegin2 = nWidth - SRand::Inst().Rand( 6, 9 );
		xEnd2 = Min( nWidth - 2, xBegin2 + SRand::Inst().Rand( 3, 6 ) );
	}
	else
	{
		xEnd2 = SRand::Inst().Rand( 6, 9 );
		xBegin2 = Max( 2, xEnd2 - SRand::Inst().Rand( 3, 6 ) );
	}
	if( h > 10 )
	{
		TRectangle<int32> room( nDir == 1 ? 0 : nWidth - 4, y, 4, h );
		room.SetTop( Max( y + 4, room.GetBottom() - SRand::Inst().Rand( 5, 7 ) ) );
		while( 1 )
		{
			AddChunk( room, eType_Room_1, &m_vecRoom );
			vecRoom.push_back( room );
			TRectangle<int32> car( room.x, room.y + ( room.height - 1 ) / 2, room.width, 2 );
			AddChunk( car, nDir == 1 ? eType_Room_Car_0 : eType_Room_Car_2, NULL );
			vecCars.push_back( TRectangle<int32>( nDir == 1 ? car.GetRight() : car.x - 4, car.y, 4, car.height ) );
			if( room.GetBottom() == y + h )
			{
				TRectangle<int32> rect( nDir == 1 ? car.GetRight() : car.x - 4, car.GetBottom(), 4, y + h - car.GetBottom() );
				AddChunk( rect, eType_Temp, NULL );
			}
			int32 h1 = 5;
			int32 y0 = CreateCarPath( car, nDir, y, h1, true );
			room.height = SRand::Inst().Rand( 5, 7 );
			room.y = y0 + 2 - room.height;
			room.SetTop( Max( y + 4, room.y ) );
			nDir = 1 - nDir;
			room.x = nDir == 1 ? 0 : nWidth - 4;
			if( room.height < 5 )
				break;
			for( int i = 8; i < nWidth; i++ )
			{
				TVector2<int32> p( nDir == 1 ? i : nWidth - i, y0 );
				if( !m_gendata[p.x + p.y * nWidth] )
				{
					auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 3 ), TVector2<int32>( 2, 3 ),
						TRectangle<int32>( 0, y, nWidth, h ), -1, eType_House_0 );
					if( rect.width )
					{
						vecHouses.push_back( rect );
						m_vecHouse.push_back( rect );
					}
					break;
				}
			}
		}
	}
	for( auto& rect : vecHouses )
	{
		for( int j = Max( y, rect.y - 4 ); j < Min( y + h, rect.GetBottom() + 4 ); j++ )
		{
			int32 i1 = Min( j - ( rect.y - 4 ), rect.GetBottom() + 3 - j );
			for( int i = Max( 0, rect.x - i1 ); i < Min( nWidth, rect.GetRight() + i1 ); i++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp1;
			}
		}
	}

	vector<TVector2<int32> > vecP;
	for( int k = 0; k < 3; k++ )
	{
		if( k == 0 )
		{
			for( int i = 0; i < nWidth; i += nWidth - 1 )
			{
				for( int j = y; j < y + h; j++ )
				{
					if( m_gendata[i + j * nWidth] )
						break;
					vecP.push_back( TVector2<int32>( i, j ) );
				}
			}
			for( int i = 1; i < nWidth - 1; i++ )
			{
				if( !m_gendata[i + y * nWidth] )
					vecP.push_back( TVector2<int32>( i, y ) );
			}
		}
		else
		{
			for( int i = 0; i < nWidth; i++ )
			{
				for( int j = y; j < y + h - 1; j++ )
				{
					if( m_gendata[i + j * nWidth] == 0 && ( k == 2 || m_gendata[i + ( j + 1 ) * nWidth] == eType_Temp ) )
						vecP.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		SRand::Inst().Shuffle( vecP );
		for( auto& p : vecP )
		{
			if( m_gendata[p.x + p.y * nWidth] )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 3 ), TVector2<int32>( 2, SRand::Inst().Rand( 4, 6 ) ),
				TRectangle<int32>( 0, y, nWidth, h ), -1, eType_House_0 );
			if( rect.width )
			{
				vecHouses.push_back( rect );
				m_vecHouse.push_back( rect );
				for( int j = Max( y, rect.y - 4 ); j < Min( y + h, rect.GetBottom() + 4 ); j++ )
				{
					int32 i1 = Min( j - ( rect.y - 4 ), rect.GetBottom() + 3 - j );
					for( int i = Max( 0, rect.x - i1 ); i < Min( nWidth, rect.GetRight() + i1 ); i++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = eType_Temp1;
					}
				}
			}
		}
		vecP.resize( 0 );
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = y; j < y + h; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp1 )
				m_gendata[i + j * nWidth] = 0;
		}
	}

	vector<TVector2<int32> > vecP1;
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * h );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < h; j++ )
		{
			auto type = m_gendata[i + ( j + y ) * nWidth];
			vecTemp[i + j * nWidth] = type == 0 ? 0 : 1;
		}
	}
	for( auto& rect : vecHouses )
	{
		for( int i = 0; i < 2; i++ )
		{
			int32 x = i == 0 ? rect.x - 1 : rect.GetRight();
			int32 dx = i == 0 ? -1 : 1;
			if( x < 0 || x >= nWidth )
				continue;
			for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
			{
				for( int i1 = x; i1 >= 0 && i1 < nWidth; i1 += dx )
				{
					if( abs( i1 - x ) >= 7 )
					{
						vecP1.push_back( TVector2<int32>( x, j - y ) );
						break;
					}
					if( m_gendata[i1 + j * nWidth] )
					{
						if( abs( i1 - x ) >= 4 && m_gendata[i1 + j * nWidth] == eType_House_0 )
							vecP1.push_back( TVector2<int32>( x, j - y ) );
						break;
					}
				}
			}
		}
	}
	SRand::Inst().Shuffle( vecP1 );
	for( auto p : vecP )
		vecP1.push_back( p + TVector2<int32>( 0, -y ) );
	for( auto& p : vecP1 )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( vecTemp, nWidth, h, p, TVector2<int32>( 4, 2 ), TVector2<int32>( 20, 2 ), TRectangle<int32>( 0, 0, nWidth, h ), -1, 1 );
		if( !rect.width )
			continue;
		for( int i = Max( 0, rect.x - 2 ); i < Min( nWidth, rect.GetRight() + 2 ); i++ )
		{
			int32 j1 = Min( 2, Min( i - ( rect.x - 2 ), rect.GetRight() - 1 - i ) );
			for( int j = Max( 0, rect.y - j1 ); j < Min( h, rect.GetBottom() + j1 ); j++ )
			{
				if( !vecTemp[i + j * nWidth] )
					vecTemp[i + j * nWidth] = 1;
			}
		}
		rect.y += y;
		if( rect.y < y + 5 )
		{
			AddChunk( rect, eType_Road, &m_vecRoad );
			for( int k = 0; k < 2; k++ )
			{
				int32 n = Min( ( rect.width - 1 ) / 2, SRand::Inst().Rand( 4, 7 ) );
				for( int i = 0; i < n; i++ )
				{
					int32 x = k ? i + rect.x : rect.GetRight() - 1 - i;
					for( int y = rect.y; y < rect.GetBottom(); y++ )
						m_gendata[x + y * nWidth] = eType_Road_1_1;
				}
			}
		}
		else
		{
			AddChunk( rect, eType_Road_1_1, &m_vecRoad );
			int32 s[2] = { 0, 0 };
			for( int k = 0; k < 2; k++ )
			{
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					int32 y0 = k == 0 ? rect.y - 1 : rect.GetBottom();
					int32 d = k == 0 ? -1 : 1;
					if( y0 < y || y0 >= y + h )
						continue;
					for( int j = y0; k == 0 ? j >= Max( y, y0 - 4 ) : j < Min( y + h, y0 + 4 + 1 ); j += d )
					{
						if( m_gendata[i + j * nWidth] && m_gendata[i + j * nWidth] != eType_Temp )
						{
							s[k] += 4 - abs( y0 - j );
							break;
						}
					}
				}
			}
			if( !s[0] && !s[1] )
				continue;
			int j = s[0] > s[1] ? rect.y : rect.GetBottom() - 1;
			int32 x1 = rect.x + Min( ( rect.width - 1 ) / 3, ( rect.width - 1 ) / 2 + 1 );
			int32 x2 = rect.GetRight() - Min( ( rect.width - 1 ) / 3, ( rect.width - 1 ) / 2 + 1 );
			for( int i = x1; i < x2; i++ )
				m_gendata[i + j * nWidth] = eType_Road;
		}
	}
	if( h > 5 )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = y; j < y + h; j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp )
				{
					m_gendata[i + j * nWidth] = 0;
					vecP.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		for( auto rect : vecCars )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					if( m_gendata[i + j * nWidth] == 0 )
						m_gendata[i + j * nWidth] = eType_Temp;
				}
			}
		}
		AddCargos( TRectangle<int32>( 0, y + 5, nWidth, h - 5 ), 0, 0 );
		for( auto& p : vecP )
		{
			if( m_gendata[p.x + p.y * nWidth] == 0 )
				m_gendata[p.x + p.y * nWidth] = eType_Temp;
		}
	}
}

void CLevelBonusGenNode2_0::AddCargos0( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	AddChunk( rect, eType_Temp, NULL );
	vector<int32> vec;
	int32 nObj = m_vecCargo.size();
	int32 s = SRand::Inst().Rand( rect.width * rect.height * 2 / 5, rect.width * rect.height * 3 / 5 );
	for( int y = rect.y; y < rect.GetBottom() - 1 && s > 0; y++ )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			if( m_gendata[i + y * nWidth] == eType_Temp && ( y == rect.y || m_gendata[i + ( y - 1 ) * nWidth] == eType_Obj ) )
			{
				m_gendata[i + y * nWidth] = eType_Temp1;
				vec.push_back( i );
			}
		}
		SRand::Inst().Shuffle( vec );
		for( int i = vec.size() - 1 - ( rect.width + 2 ) / 4; i >= 0 && s > 0; i-- )
		{
			int32 x = vec[i];
			if( m_gendata[x + y * nWidth] != eType_Temp1 )
				continue;
			auto r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 2, 1 ), TVector2<int32>( SRand::Inst().Rand( 2, 4 ), 1 ),
				rect, -1, eType_Temp );
			if( r.width <= 0 )
				continue;
			r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( r.width, 2 ), TVector2<int32>( r.width, SRand::Inst().Rand( 2, r.width + 1 ) ), rect, -1, eType_Obj, eType_Temp );
			if( r.width <= 0 )
				continue;
			m_vecCargo.push_back( r );
			s -= r.width * r.height;
		}

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			if( m_gendata[i + ( y - 1 ) * nWidth] == eType_Temp1 )
				m_gendata[i + ( y - 1 ) * nWidth] = eType_Temp;
		}
		vec.resize( 0 );
	}

	if( s > 0 )
	{
		for( int32 i = nObj; i < m_vecCargo.size() && s > 0; i++ )
		{
			auto& r = m_vecCargo[i];
			TVector2<int32> size( Min( 3, r.width + 1 ), Min( 3, r.height + 1 ) );
			if( size == r.GetSize() )
				continue;
			s += r.width * r.height;
			AddChunk( r, eType_Temp, NULL );
			r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( r.width, 2 ), size, rect, -1, eType_Obj, eType_Temp );
			s -= r.width * r.height;
		}
	}
}

void CLevelBonusGenNode2_0::AddCargos( const TRectangle<int32>& rect, int8 nType, int8 nType1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vecP;
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( j > 0 && m_gendata[i + j * nWidth] == nType &&
				m_gendata[i + ( j - 1 ) * nWidth] > 0 && m_gendata[i + ( j - 1 ) * nWidth] < eType_Temp )
				vecP.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vecP );
	vector<TVector2<int32> > vecP1;
	for( auto& p : vecP )
	{
		if( m_gendata[p.x + p.y * nWidth] != nType )
			continue;
		auto r1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, nType1 ? 2 : 3 ), TVector2<int32>( 8, 8 ), rect, -1, nType );
		if( r1.width <= 0 )
			continue;
		r1.x += 1;
		r1.width -= 2;
		if( !nType1 )
			r1.height--;
		for( int i = r1.x; i < r1.GetRight(); i++ )
		{
			if( m_gendata[i + ( r1.y - 1 ) * nWidth] > 0 && m_gendata[i + ( r1.y - 1 ) * nWidth] < eType_Temp )
				vecP1.push_back( TVector2<int32>( i, r1.y ) );
		}
		SRand::Inst().Shuffle( vecP1 );
		for( auto& p1 : vecP1 )
		{
			if( m_gendata[p1.x + p1.y * nWidth] != nType )
				continue;
			auto r2 = PutRect( m_gendata, nWidth, nHeight, p1, TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 4 ), SRand::Inst().Rand( 2, 4 ) ), r1, -1, eType_Obj );
			if( r2.width > 0 )
				m_vecCargo.push_back( r2 );
		}
		vecP1.resize( 0 );
	}
}

void CLevelBonusGenNode2_0::AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32>>* pVec )
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

int32 CLevelBonusGenNode2_0::CreateCarPath( const TRectangle<int32>& car, int8 nDir, int32 y, int32 h1, bool bType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	TRectangle<int32> rect( nDir == 1 ? car.GetRight() : car.x - 4, car.y, 4, 2 );
	AddChunk( rect, eType_Temp, NULL );
	while( rect.y > y )
	{
		if( bType )
		{
			if( nDir == 1 && rect.GetRight() >= nWidth - 4 || nDir == 0 && rect.x <= 4 )
				break;
		}
		else
		{
			if( rect.x <= 0 || rect.GetRight() >= nWidth )
				break;
		}
		int32 d1 = car.y - rect.y;
		int32 d3 = nDir == 1 ? rect.GetRight() - car.GetRight() : car.x - rect.x;
		int8 k = d3 * h1 - ( d1 * nWidth + SRand::Inst().Rand( 0, nWidth ) ) > 0;
		int8 k1;
		for( k1 = 0; k1 < 2; k1++ )
		{
			if( ( k ^ k1 ) == 1 )
			{
				bool b = false;
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					auto nType = m_gendata[x + ( rect.y - 1 ) * nWidth];
					if( nType && nType != eType_Temp )
					{
						b = true;
						break;
					}
				}
				if( b )
					continue;
				rect.y--;
				AddChunk( rect, eType_Temp, NULL );
				break;
			}
			else
			{
				int32 x = nDir == 1 ? rect.GetRight() : rect.x - 1;
				bool b = false;
				for( int y = rect.y; y < rect.GetBottom(); y++ )
				{
					auto nType = m_gendata[x + y * nWidth];
					if( nType && nType != eType_Temp )
					{
						b = true;
						break;
					}
				}
				if( b )
					continue;
				rect.x += nDir == 1 ? 1 : -1;
				AddChunk( rect, eType_Temp, NULL );
				break;
			}
		}
		if( k1 >= 2 )
			break;
	}
	return rect.y;
}

void CLevelBonusGenNode2_0::GenObj( int32 y, int32 y1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( int i = 0; i < nWidth; i += nWidth - 1 )
	{
		for( int j = y; j < y1; j++ )
		{
			if( m_gendata[i + j * nWidth] )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i, j ), TVector2<int32>( 2, 2 ), TVector2<int32>( 2, nHeight ), TRectangle<int32>( 0, y, nWidth, y1 - y ), -1, 0 );
			if( rect.width <= 0 )
				continue;
			GenObjRect( rect, i == 0 ? 1 : 0, y, y1 );
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
}

void CLevelBonusGenNode2_0::GenObjRect( const TRectangle<int32>& rect, int8 nDir, int32 y, int32 y1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( rect.y == 0 )
	{
		auto rect1 = PutRect( m_gendata, nWidth, nHeight, TRectangle<int32>( rect.x, rect.y, rect.width, 2 ), TVector2<int32>( 2, 2 ),
			TVector2<int32>( 8, 2 ), TRectangle<int32>( 0, y, nWidth, y1 - y ), -1, eType_House_0, 0 );
		if( rect1.width > 0 )
			m_vecHouse.push_back( rect1 );
		if( rect.height >= 4 )
			GenObjRect( TRectangle<int32>( rect.x, rect.y + 2, rect.width, rect.height - 2 ), nDir, y, y1 );
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
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, h ), TVector2<int32>( SRand::Inst().Rand( 2, h + 2 ), h ), TRectangle<int32>( 0, y, nWidth, y1 - y ), -1, eType_Obj );
			if( rect.width > 0 )
				m_vecCargo.push_back( rect );
			else
				break;
		}

		if( Min( rect.x, nWidth - rect.GetRight() ) <= SRand::Inst().Rand( -1, 4 ) )
		{
			auto rect1 = rect;
			rect1.x += nDir == 1 ? 2 : -2;
			GenObjRect( rect1, nDir, y, y1 );
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
			GenObjRect( rect1, nDir, y, y1 );
	}
	else
	{
		TRectangle<int32> rect1 = rect, rect2 = rect;
		if( rect.height > SRand::Inst().Rand( 9, 11 ) )
		{
			rect1.height = SRand::Inst().Rand( 3, rect.height - 5 + 1 );
			rect2.SetTop( rect1.GetBottom() + 2 );
			auto rect3 = PutRect( m_gendata, nWidth, nHeight, TRectangle<int32>( rect1.x, rect1.GetBottom(), rect1.width, 2 ),
				TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 6, 12 ), 2 ), TRectangle<int32>( 0, y, nWidth, y1 - y ), -1, eType_House_0, 0 );
			if( rect3.width > 0 )
				m_vecHouse.push_back( rect3 );
		}
		else
		{
			rect1.height = SRand::Inst().Rand( 3, rect.height - 3 + 1 );
			rect2.SetTop( rect1.GetBottom() );
		}
		GenObjRect( rect1, nDir, y, y1 );
		GenObjRect( rect2, nDir, y, y1 );
	}
}

void CLevelBonusGenNode2_0::GenSplitRoad( const TRectangle<int32>& rect, int32 xBegin1, int32 xEnd1, int8 nType, int8 nType1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( nType == 0 || nType1 == 0 )
	{
		AddChunk( TRectangle<int32>( xBegin1, rect.y, xEnd1 - xBegin1, rect.height ), eType_Road, NULL );
	}
	else
	{
		vector<TVector2<int32> > vec;
		int32 xBegin = SRand::Inst().Rand( 2, 5 );
		int32 xEnd = nWidth - SRand::Inst().Rand( 2, 5 );
		AddChunk( TRectangle<int32>( xBegin, rect.y, xEnd - xBegin, rect.height ), eType_Road, NULL );
		vec.push_back( TVector2<int32>( xBegin, xEnd ) );
		for( int i = 0; i < vec.size(); i++ )
		{
			auto nBegin = vec[i].x;
			auto nEnd = vec[i].y;
			if( nEnd - nBegin < SRand::Inst().Rand( 6, 10 ) )
				continue;
			int32 w = Min( ( nEnd - nBegin ) / 3, SRand::Inst().Rand( 2, 4 ) );
			int32 x1 = nBegin + ( nEnd - nBegin - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
			AddChunk( TRectangle<int32>( x1, rect.y, w, rect.height ), eType_Road_1_1, NULL );
			vec.push_back( TVector2<int32>( nBegin, x1 ) );
			vec.push_back( TVector2<int32>( x1 + w, nEnd ) );
		}
	}
}

void CLevelBonusGenNode2_0::GenHouses()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( auto& rect : m_vecHouse )
	{
		if( rect.width <= 2 || rect.height <= 2 )
			continue;
		AddChunk( rect, eType_House_1, NULL );
		int32 nMax = rect.width * rect.height;
		int32 nDir = SRand::Inst().Rand( 0, 2 );
		for( int i = SRand::Inst().Rand( 0, rect.width * 2 ); i < nMax; )
		{
			int32 l = rect.width + SRand::Inst().Rand( 0, 3 );
			for( int i1 = 0; i1 < l && i < nMax; i++, i1++ )
			{
				int32 x = rect.x + ( nDir == 0 ? i % rect.width : rect.width - 1 - i % rect.width );
				int32 y = rect.y + ( i / rect.width );
				m_gendata[x + y * nWidth] = eType_House_0;
			}
			i += rect.width + SRand::Inst().Rand( 0, 3 );
		}
	}
	for( auto& rect : m_vecRoom )
	{
		CLevelGen2::GenRoom( m_gendata, nWidth, nHeight, rect, eType_Room );
	}
	vector<int8> vecTemp;
	vector<TVector2<int32> > vec;
	for( auto& rect : m_vecRoom1 )
	{
		vecTemp.resize( rect.width * rect.height );
		for( int i = 0; i < rect.width; i += rect.width - 1 )
		{
			for( int j = 0; j < rect.height; j += rect.height - 1 )
				vec.push_back( TVector2<int32>( i, j ) );
		}
		for( int i = 0; i < rect.width; i++ )
		{
			for( int j = 0; j < rect.height; j++ )
			{
				if( m_gendata[i + rect.x + ( j + rect.y ) * nWidth] == eType_Room_1 )
				{
					vecTemp[i + j * rect.width] = 1;
					vec.push_back( TVector2<int32>( i, j ) );
				}
				else
					vecTemp[i + j * rect.width] = 0;
			}
		}
		ConnectAll( vecTemp, rect.width, rect.height, 0, 1 );
		SRand::Inst().Shuffle( vec );
		for( auto& p : vec )
		{
			if( vecTemp[p.x + p.y * rect.width] != 1 )
				continue;
			auto r1 = PutRect( vecTemp, rect.width, rect.height, p, TVector2<int32>( 2, 2 ), TVector2<int32>( rect.width, rect.height ),
				TRectangle<int32>( 0, 0, rect.width, rect.height ), ( rect.width + rect.height ) / 2, 2 );
			if( r1.width > 0 )
			{
				r1 = TRectangle<int32>( r1.x - 1, r1.y - 1, r1.width + 2, r1.height + 2 ) * TRectangle<int32>( 0, 0, rect.width, rect.height );
				for( int i = r1.x; i < r1.GetRight(); i++ )
				{
					for( int j = r1.y; j < r1.GetBottom(); j++ )
					{
						if( vecTemp[i + j * rect.width] == 1 )
							vecTemp[i + j * rect.width] = 3;
					}
				}
			}
		}

		vec.resize( 0 );
		for( int i = 0; i < rect.width; i++ )
		{
			for( int j = 0; j < rect.height; j++ )
			{
				if( vecTemp[i + j * rect.width] == 3 )
					vecTemp[i + j * rect.width] = 1;
			}
		}
		for( int i = 1; i < rect.width - 1; i++ )
		{
			for( int j = 1; j < rect.height - 1; j++ )
			{
				if( vecTemp[i + j * rect.width] == 1 )
				{
					int32 n = ( vecTemp[i - 1 + j * rect.width] == 1 ) + ( vecTemp[i + 1 + j * rect.width] == 1 )
						+ ( vecTemp[i + ( j - 1 ) * rect.width] == 1 ) + ( vecTemp[i + ( j + 1 ) * rect.width] == 1 );
					if( n < 1 )
						vec.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
		SRand::Inst().Shuffle( vec );
		for( int i = 0; i < vec.size(); i++ )
		{
			auto p = vec[i];
			vecTemp[p.x + p.y * rect.width] = 0;
			SRand::Inst().Shuffle( ofs, 4 );
			for( int i1 = 0; i1 < 4; i1++ )
			{
				auto p1 = p + ofs[i1];
				if( vecTemp[p1.x + p1.y * rect.width] == 1 && p1.x > 0 && p1.y > 0 && p1.x < rect.width - 1 && p1.y < rect.height - 1 )
				{
					int32 n = ( vecTemp[p1.x - 1 + p1.y * rect.width] == 1 ) + ( vecTemp[p1.x + 1 + p1.y * rect.width] == 1 )
						+ ( vecTemp[p1.x + ( p1.y - 1 ) * rect.width] == 1 ) + ( vecTemp[p1.x + ( p1.y + 1 ) * rect.width] == 1 );
					if( n < 1 )
						vec.push_back( p1 );
				}
			}
		}
		vec.resize( 0 );

		for( int i = 0; i < rect.width; i++ )
		{
			for( int j = 0; j < rect.height; j++ )
			{
				auto nType = vecTemp[i + j * rect.width];
				if( nType == 1 )
					m_gendata[i + rect.x + ( j + rect.y ) * nWidth] = eType_Room_1;
				else if( nType == 2 )
					m_gendata[i + rect.x + ( j + rect.y ) * nWidth] = eType_Room;
				else
					m_gendata[i + rect.x + ( j + rect.y ) * nWidth] = eType_Room_2;
			}
		}
		m_vecRoom.push_back( rect );
	}
	m_vecRoom1.clear();

	vector<int32> vec1;
	vec1.resize( nWidth );
	for( int i = 0; i < nWidth; i++ )
		vec1.push_back( i );
	int32 n = SRand::Inst().Rand( 15, 20 );
	for( int j = 0; j < nHeight; j++ )
	{
		SRand::Inst().Shuffle( vec1 );
		for( int i = 0; i < nWidth; i++ )
		{
			int32 x = vec1[i];
			if( m_gendata[x + j * nWidth] != eType_Road_1_1 )
				continue;
			n--;
			if( n <= 0 )
			{
				n = SRand::Inst().Rand( 15, 20 );
				m_gendata[x + j * nWidth] = eType_Road_1_2;
			}
		}
	}
}

void CLevelBonusGenNode2_1::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pCargo1Node = CreateNode( pXml->FirstChildElement( "cargo1" )->FirstChildElement(), context );
	m_pBrokenNode = CreateNode( pXml->FirstChildElement( "broken" )->FirstChildElement(), context );
	m_pBoxNode = CreateNode( pXml->FirstChildElement( "box" )->FirstChildElement(), context );
	m_pBar0Node = CreateNode( pXml->FirstChildElement( "bar0" )->FirstChildElement(), context );
	m_pBarNode[0] = CreateNode( pXml->FirstChildElement( "bar_h" )->FirstChildElement(), context );
	m_pBarNode[1] = CreateNode( pXml->FirstChildElement( "bar_v" )->FirstChildElement(), context );
	m_pBarNode_a[0] = CreateNode( pXml->FirstChildElement( "bar_h_a" )->FirstChildElement(), context );
	m_pBarNode_a[1] = CreateNode( pXml->FirstChildElement( "bar_v_a" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pBarrelNode[0] = CreateNode( pXml->FirstChildElement( "barrel_s" )->FirstChildElement(), context );
	m_pBarrelNode[1] = CreateNode( pXml->FirstChildElement( "barrel_s1_r" )->FirstChildElement(), context );
	m_pBarrelNode[2] = CreateNode( pXml->FirstChildElement( "barrel_s1_l" )->FirstChildElement(), context );
	m_pFuseNode = CreateNode( pXml->FirstChildElement( "fuse" )->FirstChildElement(), context );
	m_pChainNode = CreateNode( pXml->FirstChildElement( "chain" )->FirstChildElement(), context );
	m_pPtNode = CreateNode( pXml->FirstChildElement( "pt1" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelBonusGenNode2_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );
	m_gendata2.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenChunks();

	for( auto& rect : m_vecRoads )
	{
		m_pRoadNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
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
			else if( genData == eType_Obj || genData == eType_Obj2 )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	for( int i = 0; i < 3; i++ )
	{
		for( auto& rect : m_vecBarrel[i] )
		{
			m_pBarrelNode[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecBar[i] )
		{
			m_pBarNode[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecBar_a[i] )
		{
			m_pBarNode_a[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( auto& rect : m_vecBar0 )
	{
		m_pBar0Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& p : m_vecBroken )
	{
		m_pBrokenNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& p : m_vecBox )
	{
		m_pBoxNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& rect : m_vecCargos )
	{
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Cargo1_1;
	context.mapTags["2"] = eType_Cargo1_2;
	context.mapTags["3"] = eType_Cargo1_3;
	for( auto& rect : m_vecCargos1 )
	{
		m_pCargo1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags["1"] = eType_Room_1;
	context.mapTags["2"] = eType_Room_2;
	context.mapTags["door"] = eType_Room_Door;
	for( auto& rect : m_vecRooms )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * context.nWidth] != eType_Room_2 && m_gendata[i + j * context.nWidth] != eType_Room_Door )
					m_gendata[i + j * context.nWidth] = context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] = eType_Room_1;
			}
		}
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecBar2 )
	{
		m_pBar2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& p : m_vecFuse )
	{
		m_pFuseNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& rect : m_vecChain )
	{
		m_pChainNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& p : m_vecPt )
	{
		m_pPtNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_gendata2.clear();
	m_vecRoads.clear();
	m_vecRooms.clear();
	m_vecCargos.clear();
	m_vecCargos1.clear();
	m_vecBroken.clear();
	m_vecBox.clear();
	m_vecBar0.clear();
	m_vecBar[0].clear();
	m_vecBar[1].clear();
	m_vecBar_a[0].clear();
	m_vecBar_a[1].clear();
	m_vecBar2.clear();
	m_vecBarrel[0].clear();
	m_vecBarrel[1].clear();
	m_vecBarrel[2].clear();
	m_vecFuse.clear();
	m_vecChain.clear();
	m_vecPt.clear();
}

void CLevelBonusGenNode2_1::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
			m_gendata[i + j * nWidth] = eType_Temp;
	}

	vector<int32> vecH;
	vector<int8> vecType;
	int32 y;
	for( int k = 0; k < 2; k++ )
	{
		y = k == 0 ? nHeight : nHeight - 12;
		for( ; y > 0; )
		{
			int32 h;
			if( y >= 28 )
				h = SRand::Inst().Rand( 14, Min( 28, y - 14 ) + 1 );
			else if( y >= 14 )
				h = y;
			else
				break;
			vecH.push_back( h );
			y -= h;
		}
		SRand::Inst().Shuffle( vecH );
		for( int i = 0; i < vecH.size(); i++ )
			vecType.push_back( vecH[i] < 21 ? 1 : 0 );

		if( k == 1 )
		{
			vecH.push_back( 12 );
			vecType.push_back( 0 );
		}
		else
		{
			int i;
			if( vecType[vecType.size() - 1] == 0 )
			{
				for( i = vecH.size() - 2; i >= 1; i-- )
				{
					if( vecType[i] == 1 )
					{
						swap( vecH[i], vecH[vecH.size() - 1] );
						swap( vecType[i], vecType[vecType.size() - 1] );
						break;
					}
				}
				if( vecType[vecType.size() - 1] == 0 )
				{
					vecH.resize( 0 );
					vecType.resize( 0 );
					continue;
				}
			}
		}
		for( int i = 1; i < vecType.size() - 1; i++ )
		{
			if( vecType[i] == 0 )
				vecType[i] = SRand::Inst().Rand( 0, 2 );
		}
		vecType[0] = 0;
		vecType.push_back( 0 );
		break;
	}

	y = 0;
	int8 nFlag = vecType[0] ? 1 : 15;
	for( int i = 0; i < vecH.size(); i++ )
	{
		auto h = vecH[i];
		auto nType = vecType[i];
		auto nType1 = vecType[i + 1];
		if( nType == 0 )
			GenChunks1( y, y + h, nFlag, nType1, nFlag );
		else
			GenChunks2( y, y + h, nFlag, nType1, nFlag );
		y += h;
	}

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, 0, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 2, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Obj );
		if( rect.width )
		{
			AddChunk( rect, eType_Obj, &m_vecCargos );
		}
		else
		{
			m_gendata[p.x + p.y * nWidth] = eType_Obj;
			m_vecBox.push_back( p );
		}
	}

	for( int x = 0; x < nWidth; x++ )
	{
		for( int y = 0; y < nHeight; y++ )
		{
			if( m_gendata[x + y * nWidth] == eType_Temp )
			{
				m_gendata[x + y * nWidth] = eType_Obj;
				m_vecBox.push_back( TVector2<int32>( x, y ) );
				m_vecFuse.push_back( TVector2<int32>( x, y ) );
			}
		}
	}

	vec.resize( 0 );
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Temp1, vec );
	SRand::Inst().Shuffle( vec );
	vector<TVector2<int32> > q;
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp1 )
			continue;
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, 0, q );
		for( auto& p1 : q )
		{
			if( m_gendata[p1.x + p1.y * nWidth] )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, p1, TVector2<int32>( 2, 2 ), TVector2<int32>( 2, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Obj );
			if( rect.width )
			{
				m_vecCargos.push_back( rect );
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						m_vecFuse.push_back( TVector2<int32>( x, y ) );
					}
				}
			}
			else
			{
				m_gendata[p1.x + p1.y * nWidth] = eType_Obj;
				m_vecBox.push_back( p1 );
				m_vecFuse.push_back( p1 );
			}
		}
		q.resize( 0 );
	}
}

void CLevelBonusGenNode2_1::GenChunks1( int32 yBegin, int32 yEnd, int8 nFlag, int8 nNxtType, int8& nFlag1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 yBegin1;
	TRectangle<int32> rects[2] = { { 0, yBegin, 13, 4 }, { 19, yBegin, 13, 4 } };
	TRectangle<int32> rects1[2] = { { 0, yBegin + 8, 13, 4 }, { 19, yBegin + 8, 13, 4 } };
	if( yBegin == 0 && yEnd - yBegin <= 20 )
	{
		for( int i = 0; i < 2; i++ )
			rects1[i] = rects[i];
		for( int i = 0; i < 2; i++ )
		{
			auto& rect = rects[i];
			AddChunk( rect, eType_Room_2, &m_vecRooms );
			for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
				m_gendata[( i == 0 ? rect.GetRight() - 1 : rect.x ) + j * nWidth] = eType_Temp;
			for( int j = 0; j < 2; j++ )
			{
				int32 w = SRand::Inst().Rand( 2, 4 );
				int32 x = SRand::Inst().Rand( 0, 3 - w + 1 );
				x = ( j == 0 ? rect.x + 1 : rect.GetRight() - 4 ) + x;
				int32 x1 = x + w;
				x = Max( rect.x + 2, x );
				x1 = Min( rect.GetRight() - 2, x1 );
				for( int i1 = x; i1 < x1; i1++ )
					m_gendata[i1 + rect.y * nWidth] = eType_Temp;
			}
		}
		GenCargo( TRectangle<int32>( rects[0].GetRight(), yBegin, rects[1].x - rects[0].GetRight(), 4 ) );
		yBegin1 = 4 + yBegin;
	}
	else
	{
		bool b[4] = { !!( nFlag & 1 ), !!( nFlag & 2 ), !!( nFlag & 4 ), !!( nFlag & 8 ) };
		bool b1[2] = { SRand::Inst().Rand( 0, 2 ), SRand::Inst().Rand( 0, 2 ) };
		for( int i = 0; i < 2; i++ )
		{
			auto& rect = rects[i];
			AddChunk( rect, eType_Room_2, &m_vecRooms );
			for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
				m_gendata[( i == 0 ? rect.GetRight() - 1 : rect.x ) + j * nWidth] = eType_Temp;
			for( int j = 0; j < 2; j++ )
			{
				if( b[i * 2 + j] )
				{
					int32 w = SRand::Inst().Rand( 2, 4 );
					int32 x = SRand::Inst().Rand( 0, 3 - w + 1 );
					x = ( j == 0 ? rect.x + 1 : rect.GetRight() - 4 ) + x;
					int32 x1 = x + w;
					x = Max( rect.x + 2, x );
					x1 = Min( rect.GetRight() - 2, x1 );
					for( int i1 = x; i1 < x1; i1++ )
						m_gendata[i1 + rect.y * nWidth] = eType_Temp;
				}
			}
			if( b1[i] )
				AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 2, rect.width, 1 ), eType_Temp, NULL );
			GenRoom( rect, 1 | ( 1 << 1 ) );
		}
		GenCargo( TRectangle<int32>( 13, yBegin, 6, 3 ), 1 | ( 2 << 1 ) );
		AddChunk( TRectangle<int32>( 13, yBegin + 3, 6, 1 ), eType_Obj, &m_vecBar0 );
		TRectangle<int32> room1( 13, yBegin + 4, 6, 5 );
		AddChunk( room1, eType_Room_2, &m_vecRooms );
		for( int x = room1.x; x < room1.GetRight(); x += room1.width - 1 )
		{
			for( int y = room1.y + 1; y < room1.GetBottom() - 2; y++ )
				m_gendata[x + y * nWidth] = eType_Temp;
		}
		GenRoom( room1 );
		TRectangle<int32> cargos( 13, yBegin + 9, 6, 3 );
		for( int i = 0; i < 2; i++ )
		{
			auto& rect = rects1[i];
			if( b1[i] )
				rect.SetTop( rect.y - 1 );
			TRectangle<int32> rect1( rect.x, rects[i].GetBottom(), rect.width, rect.y - rects[i].GetBottom() );
			int32 w1 = SRand::Inst().Rand( b1[i] ? 11 : 12, rect.width + ( b1[i] ? 0 : 1 ) );
			if( w1 < rect.width )
			{
				AddChunk( TRectangle<int32>( i ? room1.GetRight() : room1.x - rect.width + w1, rect.y,
					rect.width - w1, cargos.y - rect.y ), eType_Temp1, NULL );
				if( i )
					rect.x += rect.width - w1;
				rect.width = w1;
			}
			AddChunk( rect, eType_Room_2, &m_vecRooms );
			if( b1[i] )
			{
				for( int y = rect.y + 1; y < rect.GetBottom() - 2; y++ )
					m_gendata[( i ? rect.x : rect.GetRight() - 1 ) + y * nWidth] = eType_Temp;
				GenCargos( rect1, 1 | ( 1 << 1 ) );
			}
			else
			{
				TRectangle<int32> rect2 = rect1;
				int32 l = SRand::Inst().Rand( 4, 6 );
				int32 w2 = SRand::Inst().Rand( 2, l - 2 + 1 );
				int32 x2 = l + SRand::Inst().Rand( -1, 2 );
				for( int32 i1 = 0; i1 < w2; i1++ )
					m_gendata[( i ? rect.x + x2 - 1 - i1 : rect.GetRight() - x2 + i1 ) + rect.y * nWidth] = eType_Temp;
				if( i )
				{
					rect2.width = l + rect1.width - rect.width;
					rect1.SetLeft( rect2.GetRight() );
				}
				else
				{
					rect1.width -= l + rect1.width - rect.width;
					rect2.SetLeft( rect1.GetRight() );
				}
				GenCargos( rect1, 1 | ( 1 << 1 ) );
				GenCargos( TRectangle<int32>( rect2.x, rect2.y, rect2.width, 2 ), 1 );
				AddChunk( TRectangle<int32>( rect2.x, rect2.y + 2, rect2.width, rect2.height - 2 ), eType_Temp1, NULL );
			}
		}

		cargos.SetLeft( rects1[0].GetRight() );
		cargos.SetRight( rects1[1].x );
		if( cargos.width >= SRand::Inst().Rand( 8, 11 ) )
		{
			int32 x = Max( room1.x + 1, Min( room1.GetRight() - 1, cargos.x + ( cargos.width + SRand::Inst().Rand( 0, 2 ) ) / 2 ) );
			GenCargo( TRectangle<int32>( cargos.x, cargos.y, x - cargos.x, cargos.height ) );
			GenCargo( TRectangle<int32>( x, cargos.y, cargos.GetRight() - x, cargos.height ) );
		}
		else
			GenCargo( cargos );
		yBegin1 = cargos.GetBottom();
	}

	int32 h = yEnd - yBegin1;
	uint8 nType = h >= 9;
	if( nType )
	{
		TRectangle<int32> r[2] = { { 4, yBegin1, 5, h }, { nWidth - 9, yBegin1, 5, h } };
		int32 x1 = nWidth / 2;
		for( int i = 0; i < 2; i++ )
		{
			int32 w = h >= 14 ? 5 : SRand::Inst().Rand( 4, 6 );
			r[i].x += SRand::Inst().Rand( 0, r[i].width - w + 1 );
			r[i].width = w;
			GenBars0( r[i], 1 );
			if( i == 0 )
			{
				TRectangle<int32> r1( 0, r[i].y, x1, r[i].height );
				GenBars0Wall( TRectangle<int32>( 0, r[i].y, r[i].x, r[i].height ), r1, 1 );
				GenBars0Wall( TRectangle<int32>( r[i].GetRight(), r[i].y, x1 - r[i].GetRight(), r[i].height - 4 ), r1, 1 );
			}
			else
			{
				TRectangle<int32> r1( x1, r[i].y, nWidth - x1, r[i].height );
				GenBars0Wall( TRectangle<int32>( r[i].GetRight(), r[i].y, nWidth - r[i].GetRight(), r[i].height ), r1, 1 );
				GenBars0Wall( TRectangle<int32>( x1, r[i].y, r[i].x - x1, r[i].height - 4 ), r1, 1 );
			}

			auto& rect = rects1[i];
			for( int x = rect.x; x < rect.GetRight(); x++ )
			{
				if( m_gendata[x + rect.GetBottom() * nWidth] == eType_Obj2 )
					m_gendata[x + ( rect.GetBottom() - 1 ) * nWidth] = eType_Temp;
			}
		}
		TRectangle<int32> c1( r[0].GetRight(), yEnd - 4, r[1].x - r[0].GetRight(), 4 );
		c1 = PutRect( m_gendata, nWidth, nHeight, c1, c1.GetSize(), TVector2<int32>( nWidth, c1.height ),
			TRectangle<int32>( 0, c1.y, nWidth, c1.height ), -1, eType_Temp, eType_Temp );
		if( nNxtType == 0 )
			GenCargo( c1, 8 );
		else
			AddChunk( c1, 0, &m_vecRoads );
	}
	else
	{
		for( int i = 0; i < 2; i++ )
		{
			auto& rect = rects1[i];
			int32 x = rect.x + ( rect.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
			m_gendata[x - 1 + ( rect.GetBottom() - 1 ) * nWidth] = m_gendata[x + ( rect.GetBottom() - 1 ) * nWidth] = eType_Temp;
		}
	}

	for( int i = 0; i < 2; i++ )
		GenRoom( rects1[i] );

	if( nNxtType == 1 )
		nFlag1 = 0;
	else
		nFlag1 = 15;
}

void CLevelBonusGenNode2_1::GenChunks2( int32 yBegin, int32 yEnd, int8 nFlag, int8 nNxtType, int8 & nFlag1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	int8 nType = yEnd - yBegin >= SRand::Inst().Rand( 18, 22 );
	int32 h = nType == 0 ? yEnd - 4 - yBegin : ( yEnd - yBegin + SRand::Inst().Rand( 0, 2 ) ) / 2;
	int32 w = h >= 14 ? 5 : SRand::Inst().Rand( 4, 6 );
	TRectangle<int32> r0( ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2, yBegin, w, h );
	GenBars0( r0, !!( nFlag & 1 ) );
	TRectangle<int32> rects[2] = { { 8, r0.y, r0.x - 8, r0.height }, { r0.GetRight(), r0.y, nWidth - 8 - r0.GetRight(), r0.height } };

	if( nType == 0 )
	{
		TRectangle<int32> room1( 7, r0.GetBottom(), 18, 4 );
		int32 nRoomHeight = 5;
		for( int i = 0; i < 2; i++ )
		{
			auto& rect = rects[i];
			auto bound = rect;
			if( i == 0 )
				bound.SetRight( nWidth / 2 - 1 );
			else
				bound.SetLeft( nWidth - nWidth / 2 + 1 );
			GenBars0Wall( TRectangle<int32>( rect.x, rect.y, rect.width, rect.height - nRoomHeight ), bound, 0 | ( 2 << 1 ) );
			TRectangle<int32> room( rect.x, rect.GetBottom() - nRoomHeight, rect.width, nRoomHeight );
			room = PutRect( m_gendata, nWidth, nHeight, room, room.GetSize(), TVector2<int32>( nWidth, room.y ), bound, -1, eType_Room_2, eType_Temp );
			m_vecRooms.push_back( room );
			for( int x = room.x; x < room.GetRight(); x += room.width - 1 )
			{
				for( int y = room.GetBottom() - 3; y < room.GetBottom() - 1; y++ )
					m_gendata[x + y * nWidth] = eType_Temp;
			}
			GenRoom( room, 1 | ( 2 << 1 ) );
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				auto rect1 = i == 0 ? TRectangle<int32>( 0, room1.y, room1.x, room1.height ) : TRectangle<int32>( room1.GetRight(), room1.y, nWidth - room1.GetRight(), room1.height );
				auto rect2 = i == 0 ? TRectangle<int32>( 0, rect.y, rect.x, rect.height ) : TRectangle<int32>( rect.GetRight(), rect.y, nWidth - rect.GetRight(), rect.height );
				AddChunk( rect1, 0, NULL );
				AddChunk( rect2, 0, NULL );
				AddChunk( TRectangle<int32>( rect1.x, rect1.GetBottom() - 1, rect1.width, 1 ), eType_Obj, &m_vecBar0 );
				int32 h = rect2.height - 1;
				GenCargos( TRectangle<int32>( rect2.x, rect2.y, rect2.width, h ) );
				AddChunk( TRectangle<int32>( rect2.x, rect2.y + h, rect2.width, 1 ), eType_Obj, &m_vecBar0 );
				AddChunk( TRectangle<int32>( i == 0 ? room1.x : room1.GetRight() - 1, rect2.y + h + 1, 1, room1.y - ( rect2.y + h + 1 ) ), eType_Obj2, &m_vecChain );
				AddChunk( TRectangle<int32>( i == 0 ? 0 : nWidth - 1, rect2.y + h + 1, 1, rect1.y + 1 - ( rect2.y + h + 1 ) ), eType_Obj2, &m_vecChain );
			}
			else
			{
				if( i == 0 )
					room1.SetLeft( room1.x - 1 );
				else
					room1.width++;
				auto rect1 = i == 0 ? TRectangle<int32>( 0, room1.y, room1.x, room1.height ) : TRectangle<int32>( room1.GetRight(), room1.y, nWidth - room1.GetRight(), room1.height );
				auto rect2 = i == 0 ? TRectangle<int32>( 0, rect.y, rect.x, rect.height ) : TRectangle<int32>( rect.GetRight(), rect.y, nWidth - rect.GetRight(), rect.height );
				AddChunk( rect1, 0, NULL );
				AddChunk( rect2, 0, NULL );
				int32 h1 = Max( 2, rect2.height - 8 );
				int32 h2 = rect2.height;
				GenCargos( TRectangle<int32>( rect2.x, rect2.y, rect2.width, h1 ) );
				room = TRectangle<int32>( rect2.x + 1, rect2.y + h1, rect2.width - 2, h2 - h1 );
				AddChunk( room, eType_Room_2, &m_vecRooms );
				AddChunk( TRectangle<int32>( room.x, room.GetBottom() - 1, room.width, 1 ), eType_Room_1, &m_vecBar_a[0] );

				auto r = room;
				TRectangle<int32> r1( r.x + 1, r.y + 3, r.width - 2, r.height - 4 );
				AddChunk( r1, eType_Temp, NULL );
				GenRoom( r, 0 | ( 2 << 1 ) );
				AddChunk( TRectangle<int32>( i == 0 ? room1.x : room1.GetRight() - 1, room.GetBottom(), 1, room1.y - room.GetBottom() ), eType_Obj2, &m_vecChain );

				room = TRectangle<int32>( i == 0 ? rect1.x + 1 : rect1.x, room.GetBottom(), rect1.width - 1, rect1.GetBottom() - room.GetBottom() );
				if( nNxtType == 1 )
				{
					AddChunk( room, eType_Room_2, &m_vecRooms );
					for( int y = room.y + 1; y < room.y + 3; y++ )
						m_gendata[( i == 0 ? room.x : room.GetRight() - 1 ) + y * nWidth] = eType_Temp;
					m_gendata[( i == 1 ? room.x : room.GetRight() - 1 ) + ( room.GetBottom() - 2 ) * nWidth] = eType_Temp;
					GenRoom( room, 1 );
				}
				else
				{
					GenCargo( TRectangle<int32>( room.x, room.y, room.width, room.height - 1 ) );
					AddChunk( TRectangle<int32>( room.x, room.GetBottom() - 1, room.width, 1 ), eType_Obj, &m_vecBar0 );
				}
			}
		}
		if( nNxtType == 1 )
		{
			AddChunk( room1, eType_Room_2, &m_vecRooms );
			for( int x = room1.x; x < room1.GetRight(); x += room1.width - 1 )
			{
				for( int y = room1.y + 1; y < room1.GetBottom() - 1; y++ )
					m_gendata[x + y * nWidth] = eType_Temp;
			}
			AddChunk( TRectangle<int32>( 13, room1.GetBottom() - 1, 6, 1 ), eType_Temp, NULL );
			GenRoom( room1 );
			nFlag1 = 1;
		}
		else
		{
			GenCargo( room1, 8 );
			nFlag1 = 1 + 8;
		}
	}
	else
	{
		int32 nRoomHeight = 4;
		TRectangle<int32> r[2] = { { 4, yBegin + h, 5, yEnd - yBegin - h }, { nWidth - 9, yBegin + h, 5, yEnd - yBegin - h } };
		h = yEnd - yBegin - h;
		int32 x1 = nWidth / 2;
		for( int i = 0; i < 2; i++ )
		{
			auto& rect = rects[i];
			if( i == 0 )
				rect.SetLeft( 0 );
			else
				rect.SetRight( nWidth );
			auto bound = rect;
			if( i == 0 )
				bound.SetRight( nWidth / 2 - 1 );
			else
				bound.SetLeft( nWidth - nWidth / 2 + 1 );
			GenBars0Wall( TRectangle<int32>( rect.x, rect.y, rect.width, rect.height - nRoomHeight ), bound, 0 | ( 2 << 1 ) );
			TRectangle<int32> road( rect.x, rect.GetBottom() - nRoomHeight, rect.width, nRoomHeight );
			road = PutRect( m_gendata, nWidth, nHeight, road, road.GetSize(), TVector2<int32>( nWidth, road.y ), bound, -1, eType_Room_2, eType_Temp );
			if( i == 0 )
				road.SetLeft( 0 );
			else
				road.SetRight( nWidth );
			AddChunk( road, 0, &m_vecRoads );
			AddChunk( TRectangle<int32>( road.x, road.GetBottom() - 1, road.width, 1 ), eType_Obj, &m_vecBar0 );

			int32 w = h >= 14 ? 5 : SRand::Inst().Rand( 4, 6 );
			r[i].x += SRand::Inst().Rand( 0, r[i].width - w + 1 );
			r[i].width = w;
			GenBars0( r[i], 0 );
			if( i == 0 )
			{
				TRectangle<int32> r1( 0, r[i].y, x1, r[i].height );
				GenBars0Wall( TRectangle<int32>( 0, r[i].y, r[i].x, r[i].height ), r1, 0 );
				GenBars0Wall( TRectangle<int32>( r[i].GetRight(), r[i].y, x1 - r[i].GetRight(), r[i].height - 4 ), r1, 0 );
			}
			else
			{
				TRectangle<int32> r1( x1, r[i].y, nWidth - x1, r[i].height );
				GenBars0Wall( TRectangle<int32>( r[i].GetRight(), r[i].y, nWidth - r[i].GetRight(), r[i].height ), r1, 0 );
				GenBars0Wall( TRectangle<int32>( x1, r[i].y, r[i].x - x1, r[i].height - 4 ), r1, 0 );
			}
		}
		TRectangle<int32> c1( r[0].GetRight(), yEnd - 4, r[1].x - r[0].GetRight(), 4 );
		c1 = PutRect( m_gendata, nWidth, nHeight, c1, c1.GetSize(), TVector2<int32>( nWidth, c1.height ),
			TRectangle<int32>( 0, c1.y, nWidth, c1.height ), -1, eType_Temp, eType_Temp );
		if( nNxtType == 0 )
			GenCargo( c1, 8 );
		else
			AddChunk( c1, 0, &m_vecRoads );

		if( nNxtType == 1 )
			nFlag1 = 0;
		else
			nFlag1 = 15;
	}
}

void CLevelBonusGenNode2_1::GenRoom( const TRectangle<int32>& rect, uint8 nType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( int i = rect.x; i < rect.GetRight(); i += rect.width - 1 )
	{
		for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
				m_gendata[( i == rect.x ? i + 1 : i - 1 ) + j * nWidth] = eType_Temp;
		}
	}
	for( int j = rect.y; j < rect.GetBottom(); j += rect.height - 1 )
	{
		for( int i = rect.x + 1; i < rect.GetRight() - 1; i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
				m_gendata[ i + ( j == rect.y ? j + 1 : j - 1 ) * nWidth] = eType_Temp;
		}
	}
	ConnectAll( m_gendata, nWidth, nHeight, eType_Temp, eType_Room_2, TRectangle<int32>( rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2 ) );
	vector<TVector2<int32> > vec;
	for( int i = rect.x; i < rect.GetRight(); i += rect.width - 1 )
	{
		for( int j = rect.y; j < rect.GetBottom(); j += rect.height - 1 )
		{
			if( m_gendata[i + j * nWidth] == eType_Room_2 )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	int32 nSize0 = vec.size();
	for( int i = rect.x; i < rect.GetRight(); i += rect.width - 1 )
	{
		for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Room_2 )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	for( int j = rect.y; j < rect.GetBottom(); j += rect.height - 1 )
	{
		for( int i = rect.x + 1; i < rect.GetRight() - 1; i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Room_2 )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	if( vec.size() - nSize0 > 0 )
		SRand::Inst().Shuffle( &vec[nSize0], vec.size() - nSize0 );
	for( int i = 0; i < vec.size(); i++ )
	{
		auto& p = vec[i];
		if( m_gendata[p.x + p.y * nWidth] != eType_Room_2 )
			continue;
		TVector2<int32> maxSize;
		if( i >= nSize0 )
			maxSize = TVector2<int32>( rect.width - 2, rect.height - 2 );
		else if( nWidth >= nHeight )
			maxSize = TVector2<int32>( rect.width - 2, rect.height );
		else
			maxSize = TVector2<int32>( rect.width, rect.height - 2 );
		auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), maxSize, rect, -1, eType_Cargo1 );
		if( r.width > 0 )
		{
			if( i >= nSize0 )
			{
				auto r1 = TRectangle<int32>( r.x - 2, r.y - 2, r.width + 4, r.height + 4 ) * rect;
				for( int x = r1.x; x < r1.GetRight(); x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
					{
						if( m_gendata[x + y * nWidth] == eType_Room_2 )
							m_gendata[x + y * nWidth] = eType_Temp1;
					}
				}
			}
			GenCargo( r, nType );
		}
		else if( i < nSize0 )
			PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 1, 1 ), TVector2<int32>( 2, 2 ), rect, -1, eType_Temp1 );
	}
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp1 )
				m_gendata[i + j * nWidth] = eType_Room_2;
		}
	}
	for( int i = nSize0; i < vec.size(); i++ )
	{
		auto& p = vec[i];
		if( m_gendata[p.x + p.y * nWidth] == eType_Room_2 )
		{
			int8 n = p.x == rect.x || p.x == rect.GetRight() - 1;
			TVector2<int32> maxSize = n ? TVector2<int32>( 1, nHeight ) : TVector2<int32>( nWidth, 1 );
			auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 1, 1 ), maxSize, rect, -1, eType_Room_2 );
			AddChunk( r, eType_Room_1, &m_vecBar_a[n] );
		}
	}
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
				m_gendata[i + j * nWidth] = eType_Room_2;
		}
	}
}

void CLevelBonusGenNode2_1::GenCargos( const TRectangle<int32>& rect, uint8 nType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( rect.height >= 6 )
	{
		int32 n = SRand::Inst().Rand( 3, rect.height - 3 + 1 );
		GenCargos( TRectangle<int32>( rect.x, rect.y, rect.width, n ), nType );
		GenCargos( TRectangle<int32>( rect.x, rect.y + n, rect.width, rect.height - n ), nType );
		return;
	}
	int32 w = Max( 5, SRand::Inst().Rand( rect.height * 2, rect.height * 2 + 2 ) );
	if( rect.width >= w * 2 )
	{
		int32 n = SRand::Inst().Rand( w, rect.width - w + 1 );
		GenCargos( TRectangle<int32>( rect.x, rect.y, n, rect.height ), nType );
		GenCargos( TRectangle<int32>( rect.x + n, rect.y, rect.width - n, rect.height ), nType );
		return;
	}
	GenCargo( rect, nType );
}

void CLevelBonusGenNode2_1::GenBars0( TRectangle<int32> rect, int8 nBaseType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 nChainLenx2 = 5;

	int32 nBarCount = Min( 3, ( rect.height + 1 ) / 5 - 1 );
	vector<int32> vec;
	for( int i = 0; i < nBarCount + 1; i++ )
		vec.push_back( ( rect.height - nBarCount + i ) / ( nBarCount + 1 ) );
	SRand::Inst().Shuffle( vec );
	int32 y = rect.y;
	bool b1 = nBaseType && nBarCount >= SRand::Inst().Rand( 2, 4 );
	bool b2 = b1 || nBaseType && nBarCount >= SRand::Inst().Rand( 2, 4 );
	int8 nDir = SRand::Inst().Rand( 0, 2 );
	for( int i = 0; i < nBarCount; i++ )
	{
		int32 y0 = y;
		y += vec[i];
		int32 nOfs = ( 1 + ( i == 0 && b1 ? nChainLenx2 : nChainLenx2 * ( nBarCount - i ) ) ) / 2;
		int32 nOfs1 = ( i == 0 && b1 ? nChainLenx2 : nChainLenx2 * ( nBarCount - i ) ) / 2;
		TRectangle<int32> bar;
		TVector2<int32> p( nDir ? rect.x : rect.GetRight() - 1, y0 );
		TRectangle<int32> lim0( rect.x, y0, rect.width, y - y0 );
		if( i == 0 || i == 1 && b1 )
		{
			AddChunk( TRectangle<int32>( rect.x, y0, rect.width, y - y0 ), 0, NULL );
			bar = TRectangle<int32>( rect.x, y, rect.width, 1 );
			AddChunk( bar, eType_Obj, &m_vecBar0 );
			int32 yBegin = SRand::Inst().Rand( Max( Max( y0, rect.y + 2 ), y - nOfs ), y - 1 );
			int32 yEnd = Min( y + 1, Max( yBegin + SRand::Inst().Rand( 2, 4 ), y - nOfs1 + 2 ) );
			int32 x1 = nDir ? rect.x - 1 : rect.GetRight();
			for( int j = yBegin; j < yEnd; j++ )
			{
				m_gendata1[x1 + j * nWidth] = 1;
			}
		}
		else
		{
			int32 yBegin = SRand::Inst().Rand( Max( y0, y - nOfs1 + 2 ), y + 2 );
			if( yBegin > y0 )
				AddChunk( TRectangle<int32>( rect.x, y0, rect.width, yBegin - y0 ), 0, NULL );
			if( nDir )
				rect.SetLeft( rect.x + 1 );
			else
				rect.width--;
			if( y + 1 > yBegin )
				AddChunk( TRectangle<int32>( rect.x, yBegin, rect.width, y + 1 - yBegin ), 0, NULL );
			TRectangle<int32> bar( rect.x, y, rect.width, 1 );
			AddChunk( bar, eType_Obj, &m_vecBar0 );
		}
		if( i == 1 && b2 )
		{
			TRectangle<int32> lim( rect.x, y0, rect.width - 1, y - y0 );
			if( nDir )
				lim.x++;
			auto cargo = lim;
			int32 w = Min( cargo.width, SRand::Inst().Rand( 2, 4 ) );
			int32 h = Min( cargo.height, SRand::Inst().Rand( 2, 4 ) );
			cargo = TRectangle<int32>( cargo.x + SRand::Inst().Rand( 0, cargo.width - w + 1 ), cargo.y + SRand::Inst().Rand( 0, cargo.height - h + 1 ), w, h );
			GenCargo( cargo );
			int32 nChain = nDir ? SRand::Inst().Rand( rect.x, cargo.x ) : SRand::Inst().Rand( cargo.GetRight(), rect.GetRight() );
			AddChunk( TRectangle<int32>( nChain, y0, 1, y - y0 ), eType_Obj2, &m_vecChain );
			if( nDir )
				lim.SetLeft( nChain + 1 );
			else
				lim.SetRight( nChain );

			TRectangle<int32> road( cargo.x, rect.y, cargo.width, Min( cargo.y - rect.y, 6 ) );
			w = SRand::Inst().Rand( 2, road.width + 1 );
			road = TRectangle<int32>( road.x + SRand::Inst().Rand( 0, road.width - w + 1 ), road.y, w, road.height );
			m_vecRoads.push_back( road );
			while( cargo.GetBottom() < y - 1 )
			{
				int8 nCargoType = SRand::Inst().Rand( 0, 2 );
				cargo.y = cargo.GetBottom();
				lim.SetTop( cargo.y );
				cargo.height = lim.height <= 3 ? lim.height : ( lim.height == 4 ? 2 : SRand::Inst().Rand( 2, 4 ) );
				int32 w = SRand::Inst().Rand( 2, lim.width );
				if( !nCargoType )
					w = Min( w, 3 );
				if( w < cargo.width )
					cargo.x = SRand::Inst().Rand( cargo.x, cargo.GetRight() - w + 1 );
				else
					cargo.x = SRand::Inst().Rand( Max( lim.x, cargo.x - 1 ),
						Min( lim.GetRight(), cargo.GetRight() + 1 ) - w + 1 );
				cargo.width = w;
				if( nCargoType )
					GenCargo( cargo );
				else
					GenCargo0( cargo );
			}
		}
		else
		{
			TRectangle<int32> lim( rect.x, y0, rect.width - 1, y - y0 );
			if( nDir )
				lim.x++;
			if( i > 0 )
			{
				int32 nChain = nDir ? SRand::Inst().Rand( rect.x, rect.GetRight() - 2 ) : SRand::Inst().Rand( rect.x + 2, rect.GetRight() );
				AddChunk( TRectangle<int32>( nChain, y0, 1, y - y0 ), eType_Obj2, &m_vecChain );
				if( nDir )
					lim.SetLeft( nChain + 1 );
				else
					lim.SetRight( nChain );
			}
			while( lim.height )
			{
				int8 nCargoType = SRand::Inst().Rand( 0, 2 );
				if( i == 0 )
				{
					if( b2 )
						nCargoType = 0;
					else if( !nBaseType )
						nCargoType = 1;
				}
				auto cargo = lim;
				cargo.height = lim.height <= 3 ? lim.height : ( lim.height == 4 ? 2 : SRand::Inst().Rand( 2, 4 ) );
				int32 w = SRand::Inst().Rand( 2, lim.width + 1 );
				if( !nCargoType )
					w = Min( w, 3 );
				if( nDir )
					cargo.SetLeft( cargo.GetRight() - w );
				else
					cargo.width = w;
				if( nCargoType )
					GenCargo( cargo );
				else
					GenCargo0( cargo );
				lim.SetTop( cargo.GetBottom() );
			}
		}

		TRectangle<int32> preRect( 0, 0, 0, 0 );
		int32 w0 = Min( 4, rect.width );
		int32 w = w0;
		bool bCargoType = nBaseType || i > 0;
		while( p.y < y )
		{
			if( m_gendata[p.x + p.y * nWidth] )
				break;
			if( bCargoType )
				w = SRand::Inst().Rand( 1, w0 + 1 );
			if( w > 1 )
			{
				auto cargo = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( w, 2 ), lim0, -1, eType_Obj2 );
				if( cargo.width > 0 )
				{
					AddChunk( cargo, eType_Obj2, &m_vecCargos );
					if( preRect.width )
					{
						int32 xFuse = SRand::Inst().Rand( Max( preRect.x, cargo.x ), Min( preRect.GetRight(), cargo.GetRight() ) );
						if( preRect.height > 1 )
							m_vecFuse.push_back( TVector2<int32>( xFuse, p.y - 1 ) );
						m_vecFuse.push_back( TVector2<int32>( xFuse, p.y ) );
					}
					preRect = cargo;
					if( !bCargoType )
						w = cargo.width;
					p.y = cargo.GetBottom();
					continue;
				}
			}
			m_vecBox.push_back( p );
			m_vecFuse.push_back( p );
			if( preRect.height > 1 )
				m_vecFuse.push_back( TVector2<int32>( p.x, p.y - 1 ) );
			preRect = TRectangle<int32>( p.x, p.y, 1, 1 );
			m_gendata[p.x + p.y * nWidth] = eType_Obj2;
			p.y++;
			if( !bCargoType )
				w = 1;
		}

		y++;
		nDir = !nDir;
	}
	AddChunk( TRectangle<int32>( rect.x, y, rect.width, rect.GetBottom() - y ), eType_Temp1, NULL );
	vec.resize( 0 );
	for( int x = rect.x; x < rect.GetRight(); x++ )
	{
		vec.push_back( x );
	}
	SRand::Inst().Shuffle( vec );
	for( auto x : vec )
	{
		auto nType = m_gendata[x + rect.GetBottom() * nWidth];
		if( nType <= eType_Road || nType == eType_Room_2 || nType == eType_Room_Door )
			continue;
		AddChunk( TRectangle<int32>( x, y, 1, rect.GetBottom() - y ), eType_Obj2, &m_vecChain );
		break;
	}
}

void CLevelBonusGenNode2_1::GenBars0Wall( TRectangle<int32> rect, const TRectangle<int32>& bound, uint8 nType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	auto Func = [=] ( TRectangle<int32>& rect, bool bRoom ) {
		while( rect.x > bound.x )
		{
			if( m_gendata[rect.x - 1 + rect.y * nWidth] != eType_Temp || !bRoom && m_gendata1[rect.x - 1 + rect.y * nWidth] )
				break;
			rect.SetLeft( rect.x - 1 );
		}
		while( rect.GetRight() < bound.GetRight() )
		{
			if( m_gendata[rect.GetRight() + rect.y * nWidth] != eType_Temp || !bRoom && m_gendata1[rect.GetRight() + rect.y * nWidth] )
				break;
			rect.width++;
		}
	};
	bool bRoom = true;
	while( rect.height )
	{
		TRectangle<int32> rect1( rect.x + 1, rect.y, rect.width - 2, 1 );
		Func( rect1, bRoom );
		while( rect1.GetBottom() < rect.GetBottom() )
		{
			TRectangle<int32> rect2( rect.x + 1, rect1.GetBottom(), rect.width - 2, 1 );
			Func( rect2, bRoom );
			if( rect2.x != rect1.x || rect2.width != rect1.width )
				break;
			rect1.height++;
		}
		if( bRoom )
		{
			if( rect1.height > 5 )
			{
				float h0 = SRand::Inst().Rand( 8.0f, 9.0f );
				float wt0 = 10000;
				int32 jMin = rect1.height;
				for( int j = rect1.height; j >= 5; j-- )
				{
					if( j == rect1.height - 1 )
						continue;
					float wt = abs( j - h0 );
					if( m_gendata1[rect1.x + ( rect1.y + j - 1 ) * nWidth]
						|| m_gendata1[rect1.GetRight() - 1 + ( rect1.y + j - 1 ) * nWidth] )
						wt += 3;
					if( wt < wt0 )
					{
						wt0 = wt;
						jMin = j;
					}
				}
				rect1.height = jMin;
			}
			AddChunk( rect1, eType_Room_2, &m_vecRooms );

			for( int j = rect1.y; j < rect1.GetBottom(); j++ )
			{
				int32 x1 = rect1.x;
				int32 x2 = rect1.GetRight() - 1;
				if( m_gendata1[x1 + j * nWidth] )
				{
					x1++;
					m_gendata1[x1 + j * nWidth] = 2;
					vec.push_back( TVector2<int32>( x1, j ) );
				}
				if( m_gendata1[x2 + j * nWidth] )
				{
					x2--;
					m_gendata1[x2 + j * nWidth] = 2;
					vec.push_back( TVector2<int32>( x2, j ) );
				}
				if( ( x1 > rect1.x || x2 < rect1.GetRight() - 1 ) && j > rect1.y && j < rect1.GetBottom() )
				{
					for( int x = rect1.x + 2; x < rect1.GetRight() - 2; x++ )
						m_gendata1[x + j * nWidth] = 3;
				}
			}
			if( !vec.size() )
			{
				int32 ax = rect1.width > 4 ? 2 : 1;
				int32 ay = rect1.height > 4 ? 2 : 1;
				TRectangle<int32> r1( rect1.x + ax, rect1.y + ay, rect1.width - ax * 2, rect1.height - ay * 2 );
				int32 s = Max( 1, ( rect1.width + rect1.height ) / 8 );
				for( int i = 0; i < s; i++ )
					m_gendata[SRand::Inst().Rand( r1.x, r1.GetRight() ) + SRand::Inst().Rand( r1.y, r1.GetBottom() ) * nWidth] = eType_Temp;
				GenRoom( rect1, nType );
			}
			else
			{
				SRand::Inst().Shuffle( vec );
				for( auto& p : vec )
				{
					if( m_gendata1[p.x + p.y * nWidth] != 2 )
						continue;
					auto bar = PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 1, 2 ), TVector2<int32>( 1, nHeight ), rect1, -1, 1 );
					if( bar.width > 0 )
						AddChunk( bar, eType_Room_1, &m_vecBar_a[1] );
					else
					{
						m_gendata1[p.x + p.y * nWidth] = 3;
						bar = PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 2, 1 ), TVector2<int32>( nWidth, 1 ), rect1, -1, 1 );
						if( bar.width > 0 )
							AddChunk( bar, eType_Room_1, &m_vecBar_a[0] );
					}
				}

				vec.resize( 0 );
				for( int x = rect1.x + 1; x < rect1.GetRight() - 1; x += Max( 1, rect1.width - 3 ) )
				{
					for( int j = rect1.y + 1; j < rect1.GetBottom() - 1; j++ )
					{
						int32 x1 = x == rect1.x + 1 ? x + 1 : x - 1;
						if( m_gendata1[x1 + j * nWidth] && ( m_gendata1[x + ( j + 1 ) * nWidth] || m_gendata1[x + ( j - 1 ) * nWidth] ) )
							vec.push_back( TVector2<int32>( x, j ) );
					}
				}
				int32 nSize1 = vec.size();
				for( int x = rect1.x; x < rect1.GetRight(); x += rect1.width - 1 )
				{
					for( int j = rect1.y; j < rect1.GetBottom(); j++ )
						vec.push_back( TVector2<int32>( x, j ) );
				}
				if( vec.size() > nSize1 )
					SRand::Inst().Shuffle( &vec[nSize1], vec.size() - nSize1 );
				for( auto& p : vec )
				{
					if( m_gendata1[p.x + p.y * nWidth] )
						continue;
					auto cargo = PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 2, 1 ), TVector2<int32>( rect1.width, rect1.height ), rect1, -1, 1 );
					if( cargo.width > 0 )
					{
						if( cargo.height > 1 )
							GenCargo( cargo, nType );
						else
							AddChunk( cargo, eType_Room_1, &m_vecBar_a[0] );
					}
				}
				vec.resize( 0 );
				for( int j = rect1.y; j < rect1.GetBottom(); j += rect1.height - 1 )
				{
					for( int x = rect1.x + 2; x < rect1.GetRight() - 2; x++ )
					{
						if( m_gendata1[x + j * nWidth] )
							continue;
						auto bar = PutRect( m_gendata1, nWidth, nHeight, TVector2<int32>( x, j ), TVector2<int32>( 2, 1 ), TVector2<int32>( nWidth, 1 ), rect1, -1, 1 );
						AddChunk( bar, eType_Room_1, &m_vecBar_a[0] );
					}
				}

				for( int x = rect1.x + 1; x < rect1.GetRight() - 1; x++ )
				{
					for( int y = rect1.y + 1; y < rect1.GetBottom() - 1; y++ )
					{
						if( m_gendata[x + y * nWidth] == eType_Room_2 )
							vec.push_back( TVector2<int32>( x, y ) );
					}
				}
				SRand::Inst().Shuffle( vec );
				if( vec.size() )
					m_vecPt.push_back( vec[0] );
				vec.resize( 0 );
			}

			bRoom = false;
		}
		else
		{
			if( rect1.height <= 1 )
				AddChunk( rect1, eType_Obj, &m_vecBar_a[0] );
			else
			{
				if( rect1.height >= 5 )
					rect1.height = Min( rect1.height - 2, SRand::Inst().Rand( 2, 5 ) );
				while( rect1.width > 0 )
				{
					int32 w;
					if( rect1.width <= rect1.height + 2 )
						w = rect1.width;
					else if( rect1.width <= rect1.height * 2 + 2 )
						w = ( rect1.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
					else
						w = rect1.height + SRand::Inst().Rand( 1, 3 );
					if( SRand::Inst().Rand( 0, 2 ) )
					{
						GenCargo( TRectangle<int32>( rect1.GetRight() - w, rect1.y, w, rect1.height ), nType );
						rect1.width -= w;
					}
					else
					{
						GenCargo( TRectangle<int32>( rect1.x, rect1.y, w, rect1.height ), nType );
						rect1.SetLeft( rect1.x + w );
					}
				}
			}
		}

		rect.SetTop( rect1.GetBottom() );
	}
}

void CLevelBonusGenNode2_1::AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32>>* pVec )
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

void CLevelBonusGenNode2_1::GenCargo( const TRectangle<int32>& rect, uint8 nType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	if( !!( nType & 8 ) )
	{
		AddChunk( TRectangle<int32>( rect.x, rect.y, rect.width, rect.height ), 0, NULL );
		TRectangle<int32> r1( rect.x, rect.y, rect.width, rect.height - 1 );
		if( rect.height >= 4 )
		{
			AddChunk( TRectangle<int32>( rect.x, rect.y, rect.width, 1 ), eType_Obj1, &m_vecBar[0] );
			r1.SetTop( r1.y + 1 );
		}
		AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 1, rect.width, 1 ), eType_Obj1, &m_vecBar[0] );
		int8 nDir = SRand::Inst().Rand( 0, 2 );
		for( int i = Min( r1.width / 2, SRand::Inst().Rand( 2, 6 ) ); i < r1.width; i++ )
		{
			int32 x = nDir ? r1.x + i : r1.GetRight() - 1 - i;
			AddChunk( TRectangle<int32>( x, r1.y, 1, r1.height ), eType_Obj1, &m_vecBar[1] );
			i += SRand::Inst().Rand( 4, 7 );
		}
	}

	vector<TVector2<int32> > vec;
	int32 s0 = 0;
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Obj1 )
				continue;
			m_gendata[i + j * nWidth] = eType_Cargo1;
			s0++;
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	int32 s = s0 * SRand::Inst().Rand( 0.5f, 0.55f );
	int32 s1 = !!( nType & 1 ) ? 0 : rect.width >= 3 && rect.height >= 3 ? s0 * SRand::Inst().Rand( 0.5f, 0.6f ) : 0;
	int32 s2 = s * SRand::Inst().Rand( 0.25f, 0.3f );
	int8 nBarrelType = ( nType >> 1 ) & 3;
	for( auto& p : vec )
	{
		if( s <= 0 )
			break;
		if( m_gendata[p.x + p.y * nWidth] == eType_Obj1 )
			continue;
		bool b = s2 > 0 && SRand::Inst().Rand( 0, 3 );
		TRectangle<int32> r0 = rect;
		if( m_gendata[p.x + p.y * nWidth] < eType_Obj )
		{
			if( s1 )
			{
				TVector2<int32> maxSize( Min( rect.width - 1, SRand::Inst().Rand( 2, 5 ) ), Min( rect.height - 1, SRand::Inst().Rand( 2, 4 ) ) );
				maxSize.y = Min( maxSize.y, Min( 6 - maxSize.x, maxSize.x ) );
				auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), maxSize, rect, -1, eType_Obj );
				if( r.width > 0 )
				{
					m_vecCargos.push_back( r );
					s1 -= r.width * r.height;
					r0 = r;
				}
			}
		}
		else
			b = false;

		if( b )
		{
			int8 nType;
			if( !nBarrelType )
				nType = SRand::Inst().Rand( 0, 3 );
			else if( nBarrelType == 1 )
				nType = 0;
			else
				nType = SRand::Inst().Rand( 1, 3 );
			auto size = nType == 0 ? TVector2<int32>( 1, 2 ) : TVector2<int32>( 2, 1 );
			auto r = PutRect( m_gendata, nWidth, nHeight, p, size, size, r0, -1, eType_Obj1 );
			if( r.width > 0 )
			{
				m_vecBarrel[nType].push_back( r );
				s -= r.width * r.height;
				s2 -= r.width * r.height;
				continue;
			}
		}
		m_gendata[p.x + p.y * nWidth] = eType_Obj1;
		m_vecBox.push_back( p );
		s--;
	}

	AddChunk( rect, eType_Cargo1, &m_vecCargos1 );
}

void CLevelBonusGenNode2_1::GenCargo0( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( SRand::Inst().Rand( 0, 15 ) < rect.width * rect.height )
	{
		int8 nType = SRand::Inst().Rand( 0, 3 );
		auto size = nType == 0 ? TVector2<int32>( 1, 2 ) : TVector2<int32>( 2, 1 );
		TRectangle<int32> r( rect.x + SRand::Inst().Rand( 0, rect.width - size.x + 1 ), rect.y + SRand::Inst().Rand( 0, rect.height - size.y + 1 ), size.x, size.y );
		AddChunk( rect, eType_Obj1, &m_vecBarrel[nType] );
	}
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Obj1 )
				continue;
			int a = SRand::Inst().Rand( 0, 5 );
			if( a <= 1 )
				m_vecBox.push_back( TVector2<int32>( i, j ) );
			else if( a == 2 )
				m_vecBroken.push_back( TVector2<int32>( i, j ) );
		}
	}
	AddChunk( rect, eType_Obj, &m_vecCargos );
}