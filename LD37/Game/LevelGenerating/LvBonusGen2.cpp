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
