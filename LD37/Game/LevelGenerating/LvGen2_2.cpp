#include "stdafx.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

void CLevelGenNode2_2_0::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pGreenbeltNode = CreateNode( pXml->FirstChildElement( "greenbelt" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pCargo1Node = CreateNode( pXml->FirstChildElement( "cargo1" )->FirstChildElement(), context );
	m_pBrokenNode = CreateNode( pXml->FirstChildElement( "broken" )->FirstChildElement(), context );
	m_pBoxNode = CreateNode( pXml->FirstChildElement( "box" )->FirstChildElement(), context );
	m_pBarNode[0] = CreateNode( pXml->FirstChildElement( "bar_h" )->FirstChildElement(), context );
	m_pBarNode[1] = CreateNode( pXml->FirstChildElement( "bar_v" )->FirstChildElement(), context );
	m_pBarrelNode[0] = CreateNode( pXml->FirstChildElement( "barrel_s" )->FirstChildElement(), context );
	m_pBarrelNode[1] = CreateNode( pXml->FirstChildElement( "barrel_s1_r" )->FirstChildElement(), context );
	m_pBarrelNode[2] = CreateNode( pXml->FirstChildElement( "barrel_s1_l" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_2_0::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );

	GenRoads();
	GenRooms();
	ConnRoads();
	FillEmpty();
	GenObjs();
	GenHouses();

	for( auto& rect : m_vecRoad )
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
			else if( genData == eType_Obj )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["1"] = eType_Greenbelt_1;
	context.mapTags["1_1"] = eType_Greenbelt_1_1;
	context.mapTags["1_2"] = eType_Greenbelt_1_2;
	for( auto& rect : m_vecGreenbelt )
	{
		m_pGreenbeltNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
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
	sort( m_vecRoom.begin(), m_vecRoom.end(), [] ( const TRectangle<int32>& a, const TRectangle<int32>& b ) {
		return a.y < b.y;
	} );
	int nRoom = SRand::Inst().Rand( 5, 7 );
	for( auto& rect : m_vecRoom )
	{
		nRoom--;
		context.mapTags["car_type"] = nRoom ? 0 : 1;
		if( !nRoom )
			nRoom = SRand::Inst().Rand( 5, 7 );
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
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
	for( auto& p : m_vecBroken )
	{
		m_pBrokenNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& p : m_vecBox )
	{
		m_pBoxNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& rect : m_vecCargo )
	{
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecCargo1 )
	{
		m_pCargo1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_vecRoad.clear();
	m_vecGreenbelt.clear();
	m_vecHouse.clear();
	m_vecRoom.clear();
	m_vecCargo.clear();
	m_vecCargo1.clear();
	m_vecBroken.clear();
	m_vecBox.clear();
	m_vecBar[0].clear();
	m_vecBar[1].clear();
	m_vecBarrel[0].clear();
	m_vecBarrel[1].clear();
	m_vecBarrel[2].clear();
}

void CLevelGenNode2_2_0::GenRoads()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TRectangle<int32> > vecRect;
	vecRect.push_back( TRectangle<int32>( 0, 0, nWidth, nHeight ) );
	const int32 nMinHeight = 8;
	for( int i = 0; i < vecRect.size(); )
	{
		int32 a = SRand::Inst().Rand<int32>( i, vecRect.size() );
		if( a != i )
			swap( vecRect[a], vecRect[i] );
		auto rect = vecRect[i];

		int8 nSplitType = -1;
		do
		{
			if( rect.width == nWidth )
			{
				if( rect.height == nHeight && SRand::Inst().Rand( 0, 2 ) )
				{
					nSplitType = 0;
					break;
				}
				if( rect.height >= SRand::Inst().Rand( 25, 35 ) )
				{
					nSplitType = 2;
					break;
				}
				if( rect.width <= SRand::Inst().Rand( 11, 14 ) )
				{
					nSplitType = 1;
					break;
				}
			}
			if( rect.width >= SRand::Inst().Rand( 20, 30 ) && rect.height >= SRand::Inst().Rand( 20, 30 ) && SRand::Inst().Rand( 0, 2 ) )
			{
				nSplitType = 4;
				break;
			}
			if( rect.width >= SRand::Inst().Rand( 14, 19 ) && rect.height >= SRand::Inst().Rand( 14, 19 ) && SRand::Inst().Rand( 0, 3 ) )
			{
				nSplitType = 3;
				break;
			}
			if( rect.width >= SRand::Inst().Rand( 20, 23 ) || rect.height >= SRand::Inst().Rand( 13, 16 )
				|| rect.width >= ( rect.x > 0 && rect.GetRight() < nWidth ? 10 : SRand::Inst().Rand( 11, 13 ) ) && rect.height >= 8 )
			{
				nSplitType = 0;
				break;
			}
		} while( 0 );

		if( nSplitType == 0 )
		{
			TRectangle<int32> rect1, rect2;
			if( rect.width * 3 >= rect.height * 2 )
			{
				int32 n = Max<int32>( 5, rect.width * 0.4f );
				int32 w1 = SRand::Inst().Rand( n, rect.width - n + 1 );
				rect1 = TRectangle<int32>( rect.x, rect.y, w1, rect.height );
				rect2 = TRectangle<int32>( rect.x + w1, rect.y, rect.width - w1, rect.height );
			}
			else
			{
				int32 n = Max<int32>( 5, rect.height * 0.4f );
				int32 h1 = SRand::Inst().Rand( n, rect.height - n + 1 );
				rect1 = TRectangle<int32>( rect.x, rect.y, rect.width, h1 );
				rect2 = TRectangle<int32>( rect.x, rect.y + h1, rect.width, rect.height - h1 );
			}
			vecRect[i] = rect1;
			vecRect.push_back( rect2 );
		}
		else if( nSplitType == 1 )
		{
			int32 w = SRand::Inst().Rand( 6, 8 );
			int32 w1 = ( rect.width - w + SRand::Inst().Rand( 0, 2 ) ) / 2 + SRand::Inst().Rand( -2, 3 );
			TRectangle<int32> rect1( rect.x, rect.y, w1, rect.height );
			TRectangle<int32> rect2( rect.x + w1, rect.y, w, rect.height );
			TRectangle<int32> rect3( rect.x + w1 + w, rect.y, rect.width - w1 - w, rect.height );
			vecRect[i] = rect1;
			vecRect.push_back( rect2 );
			vecRect.push_back( rect3 );
		}
		else if( nSplitType == 2 )
		{
			vector<TRectangle<int32> > vecTemp;
			TRectangle<int32> r;
			{
				int32 w1 = SRand::Inst().Rand( 19, 25 );
				int32 h1 = Min( 18, SRand::Inst().Rand( 160, 200 ) / ( rect.width - w1 ) );
				int32 h2 = Max( h1 - SRand::Inst().Rand( 5, 10 ), 8 );
				vecTemp.push_back( TRectangle<int32>( 0, 0, w1, h2 ) );
				vecTemp.push_back( TRectangle<int32>( w1, 0, rect.width - w1, h1 ) );
				r = TRectangle<int32>( 0, h2, w1, h1 - h2 );
			}
			int8 nDir = 0;

			while( r.GetBottom() < rect.height )
			{
				int8 nTypes[2] = { 0, 1 };
				SRand::Inst().Shuffle( nTypes, 2 );
				bool b = false;
				for( int j = 0; j < 2; j++ )
				{
					int8 nType = nTypes[j];
					if( r.width < 12 || r.height < 6 )
						continue;
					if( nType == 0 && rect.height - r.GetBottom() < 6 )
						continue;
					int32 w1 = nType == 0 ? SRand::Inst().Rand( 6, 9 ) : ( r.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
					TRectangle<int32> rect1( 0, r.y, w1, Min( rect.height - r.y, r.height + ( nType == 0 ? SRand::Inst().Rand( 6, 12 ) : SRand::Inst().Rand( 4, 8 ) ) ) );
					TRectangle<int32> rect2( w1, r.y, r.width - w1, r.height );
					if( nType == 0 )
					{
						if( rect2.width < 10 )
							continue;
						if( rect.height - rect1.GetBottom() <= 5 )
						{
							if( rect.height - r.GetBottom() < 12 )
								rect1.SetBottom( rect.height );
							else
								rect1.height = r.height + 6;
						}
					}
					else
					{
						if( rect.height - rect1.GetBottom() <= 5 )
							rect1.SetBottom( rect.height );
					}
					TRectangle<int32> rect3( w1, r.GetBottom(), rect.width - w1, rect1.GetBottom() - r.GetBottom() );
					if( nDir == 1 )
					{
						rect1.x = rect.width - rect1.x - rect1.width;
						rect2.x = rect.width - rect2.x - rect2.width;
						rect3.x = rect.width - rect3.x - rect3.width;
					}
					vecTemp.push_back( rect1 );
					vecTemp.push_back( rect2 );
					r = rect3;
					b = true;
					break;
				}
				if( !b )
				{
					TRectangle<int32> rect1( 0, r.y, r.width, Min( rect.height - r.y, r.height + 6 ) );
					if( rect.height - rect1.GetBottom() <= 5 )
						rect1.SetBottom( rect.height );
					TRectangle<int32> rect3( r.width, r.GetBottom(), rect.width - r.width, rect1.GetBottom() - r.GetBottom() );
					if( nDir == 1 )
					{
						rect1.x = rect.width - rect1.x - rect1.width;
						rect3.x = rect.width - rect3.x - rect3.width;
					}
					vecTemp.push_back( rect1 );
					r = rect3;
				}

				nDir = !nDir;
			}
			vecTemp.push_back( r );

			int8 bFlipX = SRand::Inst().Rand( 0, 2 ), bFlipY = SRand::Inst().Rand( 0, 2 );
			for( int iRect = 0; iRect < vecTemp.size(); iRect++ )
			{
				auto r = vecTemp[iRect];
				r.x = bFlipX ? rect.x + r.x : rect.GetRight() - r.width - r.x;
				r.y = bFlipY ? rect.y + r.y : rect.GetBottom() - r.height - r.y;
				if( iRect == 0 )
					vecRect[i] = r;
				else
					vecRect.push_back( r );
			}
		}
		else if( nSplitType == 3 )
		{
			int32 w = SRand::Inst().Rand( 6, 9 );
			int32 h = SRand::Inst().Rand( 6, 9 );
			TRectangle<int32> rects[3];
			if( rect.height + SRand::Inst().Rand( 0, 2 ) > rect.width )
			{
				rects[0] = TRectangle<int32>( 0, 0, w, rect.height );
				rects[1] = TRectangle<int32>( w, 0, rect.width - w, h );
			}
			else
			{
				rects[0] = TRectangle<int32>( 0, 0, rect.width, h );
				rects[1] = TRectangle<int32>( 0, h, w, rect.height - h );
			}
			rects[2] = TRectangle<int32>( w, h, rect.width - w, rect.height - h );
			int8 bFlipX = SRand::Inst().Rand( 0, 2 ), bFlipY = SRand::Inst().Rand( 0, 2 );
			for( int iRect = 0; iRect < 3; iRect++ )
			{
				auto r = rects[iRect];
				r.x = bFlipX ? rect.x + r.x : rect.GetRight() - r.width - r.x;
				r.y = bFlipY ? rect.y + r.y : rect.GetBottom() - r.height - r.y;
				if( iRect == 0 )
					vecRect[i] = r;
				else
					vecRect.push_back( r );
			}
		}
		else if( nSplitType == 4 )
		{
			int32 w1 = SRand::Inst().Rand( 6, 9 );
			int32 h1 = SRand::Inst().Rand( 6, 9 );
			int32 w2 = SRand::Inst().Rand( 6, 9 );
			int32 h2 = SRand::Inst().Rand( 6, 9 );
			TRectangle<int32> rects[5];
			rects[0] = TRectangle<int32>( w1, h1, rect.width - w1 - w2, rect.height - h1 - h2 );
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				rects[1] = TRectangle<int32>( 0, 0, rect.width - w2, h1 );
				rects[2] = TRectangle<int32>( rect.width - w2, 0, w2, rect.height - h2 );
				rects[3] = TRectangle<int32>( w1, rect.height - h2, rect.width - w1, h2 );
				rects[4] = TRectangle<int32>( 0, h1, w1, rect.height - h1 );
			}
			else
			{
				rects[1] = TRectangle<int32>( 0, 0, w1, rect.height - h2 );
				rects[2] = TRectangle<int32>( 0, rect.height - h2, rect.width - w2, h2 );
				rects[3] = TRectangle<int32>( rect.width - w2, h1, w2, rect.height - h1 );
				rects[4] = TRectangle<int32>( w1, 0, rect.width - w1, h1 );
			}
			for( int iRect = 0; iRect < 5; iRect++ )
			{
				auto r = rects[iRect];
				r.x += rect.x;
				r.y += rect.y;
				if( iRect == 0 )
					vecRect[i] = r;
				else
					vecRect.push_back( r );
			}
		}
		else
			i++;
	}

	sort( vecRect.begin(), vecRect.end(), [] ( const TRectangle<int32>& a, const TRectangle<int32>& b ) {
		return a.width + a.height < b.width + b.height;
	} );
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	auto AddRoom = [=, &vecTemp] ( const TRectangle<int32>& room, int8 nType )
	{
		if( nType == eType_Room_Car_3 && room.y < 14 )
			return false;
		for( int i = room.x; i < room.GetRight(); i++ )
		{
			for( int j = room.y; j < room.GetBottom(); j++ )
			{
				if( vecTemp[i + j * nWidth] )
					return false;
			}
		}
		TRectangle<int32> rect1 = room;
		if( nType == eType_Room_Car_3 )
		{
			rect1.x -= 7;
			rect1.width += 14;
		}
		else
		{
			rect1.y -= 7;
			rect1.height += 14;
		}
		rect1 = rect1 * TRectangle<int32>( 0, 0, nWidth, nHeight );
		for( int i = rect1.x; i < rect1.GetRight(); i++ )
		{
			for( int j = rect1.y; j < rect1.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == nType )
					return false;
			}
		}

		int32 nCar = -1;
		float lMax = 0;
		int32 nCarMin = nType == eType_Room_Car_3 ? room.x + ( room.width >= 6 ? 3 : 2 ) : room.y + ( room.height >= 6 ? 3 : 2 );
		int32 nCarMax = nType == eType_Room_Car_3 ? room.GetRight() - ( room.width >= 6 ? 3 : 2 ) : room.GetBottom() - ( room.height >= 6 ? 3 : 2 );
		int32 jMax = Min( 12, nType == eType_Room_Car_3 ? room.y : ( nType == eType_Room_Car_0 ? nWidth - room.GetRight() : room.x ) );
		for( int i = nCarMin; i <= nCarMax; i++ )
		{
			int j;
			for( j = 0; j < jMax; j++ )
			{
				int32 j1 = nType == eType_Room_Car_3 ? room.y - j - 1 : ( nType == eType_Room_Car_0 ? room.GetRight() + j : room.x - j - 1 );
				if( m_gendata[nType == eType_Room_Car_3 ? i + j1 * nWidth : j1 + i * nWidth] > eType_Road
					|| m_gendata[nType == eType_Room_Car_3 ? i - 1 + j1 * nWidth : j1 + ( i - 1 ) * nWidth] > eType_Road )
					break;
			}
			if( j < 8 )
				continue;
			float l = j + SRand::Inst().Rand( 0.0f, 1.0f );
			if( l >= lMax )
			{
				lMax = l;
				nCar = i;
			}
		}
		if( nCar < 0 )
			return false;

		for( int i = room.x; i < room.GetRight(); i++ )
		{
			for( int j = room.y; j < room.GetBottom(); j++ )
			{
				vecTemp[i + j * nWidth] = 1;
				m_gendata[i + j * nWidth] = eType_Room_1;
			}
		}
		auto rect = nType == eType_Room_Car_3 ? TRectangle<int32>( nCar - 1, room.y, 2, room.height ) : TRectangle<int32>( room.x, nCar - 1, room.width, 2 );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = nType;
			}
		}
		int32 l = 8;
		if( nType == eType_Room_Car_3 )
			rect = TRectangle<int32>( rect.x, rect.y - l, rect.width, l );
		else if( nType == eType_Room_Car_0 )
			rect = TRectangle<int32>( rect.GetRight(), rect.y, l, rect.height );
		else
			rect = TRectangle<int32>( rect.x - l, rect.y, l, rect.height );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				vecTemp[i + j * nWidth] = 1;
			}
		}

		m_vecRoom.push_back( room );
		return true;
	};
	for( auto& rect : vecRect )
	{
		TRectangle<int32> room( 0, 0, 0, 0 );
		int8 nTypes[] = { eType_Room_Car_0, eType_Room_Car_2, eType_Room_Car_3 };
		SRand::Inst().Shuffle( nTypes, 3 );
		if( rect.width < 4 || rect.height < 4 )
			continue;
		for( int iType = 0; iType < 3; iType++ )
		{
			int8 nType = nTypes[iType];
			if( nType == eType_Room_Car_0 || nType == eType_Room_Car_2 )
			{
				if( nType == eType_Room_Car_0 && rect.x != 0 || nType == eType_Room_Car_2 && rect.GetRight() != nWidth )
					continue;
				if( rect.height < 8 )
				{
					if( rect.width <= 7 )
					{
						if( AddRoom( rect, nType ) )
						{
							room = rect;
							rect = TRectangle<int32>( 0, 0, 0, 0 );
							break;
						}
					}
					else
					{
						auto w1 = Min( rect.width - 3, SRand::Inst().Rand( 4, 7 ) );
						TRectangle<int32> r( nType == eType_Room_Car_0 ? rect.x : rect.GetRight() - w1, rect.y, w1, rect.height );
						if( AddRoom( r, nType ) )
						{
							room = r;
							rect = TRectangle<int32>( nType == eType_Room_Car_0 ? rect.x + w1 : rect.x, rect.y, rect.width - w1, rect.height );
							break;
						}
					}
				}
				else
				{
					int32 h1 = Min( rect.height - 4, SRand::Inst().Rand( 4, 7 ) );
					int32 w1 = rect.width <= 7 ? rect.width : Min( rect.width - 3, SRand::Inst().Rand( 4, 7 ) );
					int32 k1 = SRand::Inst().Rand( 0, 2 );
					bool b = false;
					for( int k = 0; k < 2; k++ )
					{
						TRectangle<int32> r( nType == eType_Room_Car_0 ? rect.x : rect.GetRight() - w1, !!( k ^ k1 ) ? rect.y : rect.GetBottom() - h1, w1, h1 );
						if( AddRoom( r, nType ) )
						{
							room = r;
							rect = TRectangle<int32>( nType == eType_Room_Car_0 ? rect.x + w1 : rect.x, rect.y, rect.width - w1, rect.height );
							b = true;
							break;
						}
					}
					if( b )
						break;
				}
			}
			else
			{
				if( rect.x == 0 || rect.GetRight() == nWidth )
					continue;
				if( rect.width <= 7 )
				{
					if( rect.height <= 6 )
					{
						if( AddRoom( rect, nType ) )
						{
							room = rect;
							rect = TRectangle<int32>( 0, 0, 0, 0 );
							break;
						}
					}
					else
					{
						int32 h1 = Min( rect.height - 3, SRand::Inst().Rand( 4, 7 ) );
						bool b = false;
						for( int k = 0; k < 2; k++ )
						{
							TRectangle<int32> r( rect.x, k ? rect.y : rect.GetBottom() - h1, rect.width, h1 );
							if( AddRoom( r, nType ) )
							{
								room = r;
								rect = TRectangle<int32>( rect.x, k ? rect.y + h1 : rect.y, rect.width, rect.height - h1 );
								b = true;
								break;
							}
						}
						if( b )
							break;
					}
				}
				else if( rect.height <= 6 && rect.width >= 10 )
				{
					int32 w1 = Min( rect.width - 5, SRand::Inst().Rand( 5, 7 ) );
					int32 k1 = SRand::Inst().Rand( 0, 2 );
					bool b = false;
					for( int k = 0; k < 2; k++ )
					{
						TRectangle<int32> r( !!( k ^ k1 ) ? rect.x : rect.GetRight() - w1, rect.y, w1, rect.height );
						if( AddRoom( r, nType ) )
						{
							room = r;
							rect = TRectangle<int32>( !!( k ^ k1 ) ? rect.x + w1 : rect.x, rect.y, rect.width - w1, rect.height );
							b = true;
							break;
						}
					}
					if( b )
						break;
				}
			}
		}

		if( rect.width <= 0 || rect.height <= 0 )
			continue;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Road;
			}
		}
		m_vecRoad.push_back( rect );
	}
	vector<TVector2<int32> > vec;
	for( int iRoad = m_vecRoad.size() - 1; iRoad >= 0; iRoad-- )
	{
		auto& road = m_vecRoad[iRoad];
		auto rect = road;
		if( rect.x == 0 || rect.GetRight() == nWidth )
		{
			if( rect.height * 3 > rect.width * 2 )
			{
				int32 w1 = Min( rect.width - 4, SRand::Inst().Rand( 4, 6 ) );
				if( w1 < 4 )
				{
					for( int i = road.x; i < road.GetRight(); i++ )
					{
						for( int j = road.y; j < road.GetBottom(); j++ )
						{
							m_gendata[i + j * nWidth] = 0;
						}
					}
					m_vecRoad[iRoad] = m_vecRoad.back();
					m_vecRoad.pop_back();
					continue;
				}
				if( rect.x == 0 )
					rect.SetLeft( rect.GetRight() - w1 );
				else
					rect.width = w1;
			}
			if( rect.height >= 8 && rect.GetBottom() < nHeight && m_gendata[( rect.x == 0 ? 0 : nWidth - 1 ) + rect.GetBottom() * nWidth] == eType_Room_1 )
			{
				rect.height -= 2;
				vec.push_back( TVector2<int32>( rect.x == 0 ? 0 : nWidth - 1, rect.GetBottom() + 1 ) );
			}
			if( rect != road )
			{
				for( int i = road.x; i < road.GetRight(); i++ )
				{
					for( int j = road.y; j < road.GetBottom(); j++ )
					{
						m_gendata[i + j * nWidth] = 0;
					}
				}
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						m_gendata[i + j * nWidth] = eType_Road;
					}
				}
				road = rect;
			}
		}
	}

	for( int iRoad = m_vecRoad.size() - 1; iRoad >= 0; iRoad-- )
	{
		auto& rect = m_vecRoad[iRoad];
		auto rect0 = rect;
		auto rect1 = TRectangle<int32>( rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4 ) * TRectangle<int32>( 0, 0, nWidth, nHeight );
		bool b[4] = { false };
		for( int i = rect1.x; i < rect1.GetRight(); i++ )
		{
			for( int j = rect1.y; j < rect1.GetBottom(); j++ )
			{
				if( ( i < rect.x || i >= rect.GetRight() ) && ( j < rect.y || j >= rect.GetBottom() ) )
					continue;
				if( i >= rect.x && j >= rect.y && i < rect.GetRight() && j < rect.GetBottom() )
					m_gendata[i + j * nWidth] = 0;
				else if( m_gendata[i + j * nWidth] == eType_Road )
				{
					TVector2<int32> p( Max( rect.x, Min( rect.GetRight() - 1, i ) ), Max( rect.y, Min( rect.GetBottom() - 1, j ) ) );
					m_gendata[p.x + p.y * nWidth] = eType_Temp;
					vec.push_back( p );
					if( i < rect.x )
						b[0] = true;
					else if( i >= rect.GetRight() )
						b[2] = true;
					if( j < rect.y )
						b[1] = true;
					else if( j >= rect.GetBottom() )
						b[3] = true;
				}
			}
		}
		if( b[0] )
			rect.SetLeft( rect.x + 1 );
		if( b[1] )
			rect.SetTop( rect.y + 1 );
		if( b[2] )
			rect.width--;
		if( b[3] )
			rect.height--;

		if( !( rect.width >= 3 && rect.height >= 5 || rect.width >= 5 && rect.height >= 3 ) )
		{
			m_vecRoad[iRoad] = m_vecRoad.back();
			m_vecRoad.pop_back();
			continue;
		}
		if( rect.width < rect.height )
		{
			int32 w = Min( 7, Max( 4, 9 - rect.height / 3 ) );
			while( rect.width >= w )
			{
				int32 n;
				if( b[0] > b[2] )
					n = 1;
				else if( b[2] > b[0] )
					n = 0;
				else
					n = SRand::Inst().Rand( 0, 2 );
				if( n == 0 )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						m_gendata[rect.x + y * nWidth] = b[0] ? m_gendata[rect0.x + y * nWidth] : 0;
						if( m_gendata[rect.x + y * nWidth] == eType_Temp )
							vec.push_back( TVector2<int32>( rect.x, y ) );
					}
					rect.SetLeft( rect.x + 1 );
				}
				else
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						m_gendata[rect.GetRight() - 1 + y * nWidth] = b[2] ? m_gendata[rect0.GetRight() - 1 + y * nWidth] : 0;
						if( m_gendata[rect.GetRight() - 1 + y * nWidth] == eType_Temp )
							vec.push_back( TVector2<int32>( rect.GetRight() - 1, y ) );
					}
					rect.width--;
				}
			}
		}
		else
		{
			int32 h = Max( 4, 12 - rect.width / 3 );
			while( rect.height >= h )
			{
				int32 n = SRand::Inst().Rand( 0, 2 );
				if( n == 0 )
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						m_gendata[x + rect.y * nWidth] = b[1] ? m_gendata[x + rect0.y * nWidth] : 0;
						if( m_gendata[x + rect.y * nWidth] == eType_Temp )
							vec.push_back( TVector2<int32>( x, rect.y ) );
					}
					rect.SetTop( rect.y + 1 );
				}
				else
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						m_gendata[x + ( rect.GetBottom() - 1 ) * nWidth] = b[3] ? m_gendata[x + ( rect0.GetBottom() - 1 ) * nWidth] : 0;
						if( m_gendata[x + ( rect.GetBottom() - 1 ) * nWidth] == eType_Temp )
							vec.push_back( TVector2<int32>( x, rect.GetBottom() - 1 ) );
					}
					rect.height--;
				}
			}
		}
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
		m_gendata[p.x + p.y * nWidth] = 0;

	for( int k = 0; k < 2; k++ )
	{
		int32 l = 4 - k;
		if( k == 1 )
		{
			for( auto rect : m_vecGreenbelt )
			{
				if( rect.width < rect.height )
					continue;
				rect = TRectangle<int32>( rect.x, rect.y - l, rect.width, rect.height + l * 2 ) * TRectangle<int32>( 0, 0, nWidth, nHeight );
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						if( m_gendata[i + j * nWidth] == 0 )
							m_gendata[i + j * nWidth] = eType_Temp1;
					}
				}
			}
		}
		for( auto& p : vec )
		{
			if( m_gendata[p.x + p.y * nWidth] != 0 )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 2 ), TVector2<int32>( nWidth, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Greenbelt );
			if( rect.width )
			{
				m_vecGreenbelt.push_back( rect );
				rect = TRectangle<int32>( rect.x, rect.y - l, rect.width, rect.height + l * 2 ) * TRectangle<int32>( 0, 0, nWidth, nHeight );
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						if( m_gendata[i + j * nWidth] == 0 )
							m_gendata[i + j * nWidth] = eType_Temp1;
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
		if( k == 1 )
		{
			for( auto rect : m_vecGreenbelt )
			{
				if( rect.width > rect.height )
					continue;
				rect = TRectangle<int32>( rect.x - l, rect.y, rect.width + l * 2, rect.height ) * TRectangle<int32>( 0, 0, nWidth, nHeight );
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						if( m_gendata[i + j * nWidth] == 0 )
							m_gendata[i + j * nWidth] = eType_Temp1;
					}
				}
			}
		}
		for( auto& p : vec )
		{
			if( m_gendata[p.x + p.y * nWidth] != 0 )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 4 ), TVector2<int32>( 2, 16 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Greenbelt );
			if( rect.width )
			{
				m_vecGreenbelt.push_back( rect );
				if( k == 0 )
				{
					rect = TRectangle<int32>( rect.x - l, rect.y, rect.width + l * 2, rect.height ) * TRectangle<int32>( 0, 0, nWidth, nHeight );
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						for( int j = rect.y; j < rect.GetBottom(); j++ )
						{
							if( m_gendata[i + j * nWidth] == 0 )
								m_gendata[i + j * nWidth] = eType_Temp1;
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

void CLevelGenNode2_2_0::GenRooms()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( auto rect : m_vecGreenbelt )
	{
		int8 nType = rect.width > rect.height ? eType_Temp1 : eType_Temp2;
		if( nType == eType_Temp1 )
		{
			rect.x++;
			rect.width -= 2;
		}
		else
		{
			rect.y++;
			rect.height -= 2;
		}
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = nType;
			}
		}
	}

	for( int iRoom = m_vecRoom.size() - 1; iRoom >= 0; iRoom-- )
	{
		auto& room = m_vecRoom[iRoom];
		int8 nType = -1;
		for( int i = room.x; i < room.GetRight(); i++ )
		{
			for( int j = room.y; j < room.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] >= eType_Room_Car_0 && m_gendata[i + j * nWidth] <= eType_Room_Car_3 )
					nType = m_gendata[i + j * nWidth];
				m_gendata[i + j * nWidth] = 0;
			}
		}

		TVector2<int32> maxSize = room.GetSize();
		if( nType == eType_Room_Car_3 )
			maxSize = TVector2<int32>( Max( maxSize.x, Min( maxSize.x + 2, 8 ) ), Max( maxSize.y, Min( maxSize.y + 2, 6 ) ) );
		else
			maxSize = TVector2<int32>( maxSize.x, Max( maxSize.y, Min( maxSize.y + 2, 6 ) ) );
		room = PutRect( m_gendata, nWidth, nHeight, room, room.GetSize(), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Room_1, 0 );

		int8 t1 = nType == eType_Room_Car_3 ? eType_Temp1 : eType_Temp2;
		int32 nCar = -1;
		float lMax = 4;
		int32 nCarMin = nType == eType_Room_Car_3 ? room.x + 2 : room.y + 2;
		int32 nCarMax = nType == eType_Room_Car_3 ? room.GetRight() - 2 : room.GetBottom() - 2;
		int32 jMax = Min( 12, nType == eType_Room_Car_3 ? room.y : ( nType == eType_Room_Car_0 ? nWidth - room.GetRight() : room.x ) );
		for( int i = nCarMin; i <= nCarMax; i++ )
		{
			int j;
			int nRoad = 0;
			for( j = 0; j < jMax; j++ )
			{
				int32 j1 = nType == eType_Room_Car_3 ? room.y - j - 1 : ( nType == eType_Room_Car_0 ? room.GetRight() + j : room.x - j - 1 );
				int8 n[2] = { m_gendata[nType == eType_Room_Car_3 ? i + j1 * nWidth : j1 + i * nWidth], m_gendata[nType == eType_Room_Car_3 ? i - 1 + j1 * nWidth : j1 + ( i - 1 ) * nWidth] };

				if( n[0] > eType_Road && n[0] != t1 && n[0] != eType_Temp || n[1] > eType_Road && n[1] != t1 && n[1] != eType_Temp )
					break;
				if( n[0] == eType_Road && n[1] == eType_Road )
					nRoad++;
			}
			float l = j + ( nRoad + SRand::Inst().Rand( 0.0f, 1.0f ) ) / ( i == nCarMin || i == nCarMax ? 32 : 16 );
			if( l >= lMax )
			{
				lMax = l;
				nCar = i;
			}
		}
		if( nCar >= 0 )
		{
			auto rect = nType == eType_Room_Car_3 ? TRectangle<int32>( nCar - 1, room.y, 2, room.height ) : TRectangle<int32>( room.x, nCar - 1, room.width, 2 );
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = nType;
				}
			}
			int32 l = floor( lMax );
			if( nType == eType_Room_Car_3 )
				rect = TRectangle<int32>( rect.x, rect.y - l, rect.width, l );
			else if( nType == eType_Room_Car_0 )
				rect = TRectangle<int32>( rect.GetRight(), rect.y, l, rect.height );
			else
				rect = TRectangle<int32>( rect.x - l, rect.y, l, rect.height );
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata1[i + j * nWidth] |= ( nType == eType_Room_Car_3 ? 1 : 2 );
					if( !m_gendata[i + j * nWidth] )
						m_gendata[i + j * nWidth] = eType_Temp;
				}
			}

			CLevelGen2::GenRoom( m_gendata, nWidth, nHeight, room, eType_Room );
		}
		else
		{
			for( int i = room.x; i < room.GetRight(); i++ )
			{
				for( int j = room.y; j < room.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_House_0;
				}
			}
			m_vecHouse.push_back( room );
			m_vecRoom[iRoom] = m_vecRoom.back();
			m_vecRoom.pop_back();
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
	for( auto& rect : m_vecGreenbelt )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata1[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp;
			}
		}
		if( rect.height == 2 )
		{
			int8 k1 = SRand::Inst().Rand( 0, 2 );
			for( int k = 0; k < 2; k++ )
			{
				int32 y = !!( k ^ k1 ) ? rect.y - 1 : rect.GetBottom();
				int32 y0 = !!( k ^ k1 ) ? rect.y : rect.GetBottom() - 1;
				if( y < 0 || y >= nHeight )
					continue;
				bool b = true;
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					if( m_gendata[x + y * nWidth] || !!( m_gendata1[x + y * nWidth] & 2 ) )
					{
						b = false;
						break;
					}
				}
				if( !b )
					continue;
				for( int x = rect.x; x < rect.GetRight(); x++ )
					m_gendata[x + y * nWidth] = m_gendata[x + y0 * nWidth];
				rect.height++;
				if( !!( k ^ k1 ) )
					rect.y--;
			}
		}
		else if( rect.width == 2 )
		{
			int8 k1 = SRand::Inst().Rand( 0, 2 );
			for( int k = 0; k < 2; k++ )
			{
				int32 x = !!( k ^ k1 ) ? rect.x - 1 : rect.GetRight();
				int32 x0 = !!( k ^ k1 ) ? rect.x : rect.GetRight() - 1;
				if( x < 0 || x >= nHeight )
					continue;
				bool b = true;
				for( int y = rect.y; y < rect.GetBottom(); y++ )
				{
					if( m_gendata[x + y * nWidth] || !!( m_gendata1[x + y * nWidth] & 1 ) )
					{
						b = false;
						break;
					}
				}
				if( !b )
					continue;
				for( int y = rect.y; y < rect.GetBottom(); y++ )
					m_gendata[x + y * nWidth] = m_gendata[x0 + y * nWidth];
				rect.width++;
				if( !!( k ^ k1 ) )
					rect.x--;
			}
		}
	}
}

void CLevelGenNode2_2_0::ConnRoads()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int32> vecRoadIndex;
	vecRoadIndex.resize( nWidth * nHeight );
	memset( &vecRoadIndex[0], -1, sizeof( int32 ) * vecRoadIndex.size() );
	for( int iRoad = 0; iRoad < m_vecRoad.size(); iRoad++ )
	{
		auto& road = m_vecRoad[iRoad];
		for( int i = road.x; i < road.GetRight(); i++ )
		{
			for( int j = road.y; j < road.GetBottom(); j++ )
			{
				vecRoadIndex[i + j * nWidth] = iRoad;
			}
		}
	}
	vector<int32> vec;
	for( int iRoad = 0; iRoad < m_vecRoad.size(); iRoad++ )
	{
		auto& road = m_vecRoad[iRoad];
		map<int32, int8> mapConn;
		for( int x = road.x + 1; x < road.GetRight() - 1; x++ )
		{
			for( int j = 0; j < 2; j++ )
			{
				bool b = false;
				int32 l = Min( 4, j == 0 ? road.y : nHeight - road.GetBottom() );
				for( int j1 = 0; j1 < l; j1++ )
				{
					int32 y = j == 0 ? road.y - 1 - j1 : road.GetBottom() + j1;
					int32 nIndex = vecRoadIndex[x + y * nWidth];
					if( nIndex >= 0 )
					{
						mapConn[nIndex] = b ? Max<int8>( mapConn[nIndex], 0 ) : 1;
						break;
					}
					else if( m_gendata[x + y * nWidth] == eType_Temp1 )
						b = true;
					else if( m_gendata[x + y * nWidth] != 0 && m_gendata[x + y * nWidth] != eType_Temp )
						break;
				}
			}
		}
		for( int y = road.y + 1; y < road.GetBottom() - 1; y++ )
		{
			for( int i = 0; i < 2; i++ )
			{
				bool b = false;
				int32 l = Min( 4, i == 0 ? road.x : nWidth - road.GetRight() );
				for( int i1 = 0; i1 < l; i1++ )
				{
					int32 x = i == 0 ? road.x - 1 - i1 : road.GetRight() + i1;
					int32 nIndex = vecRoadIndex[x + y * nWidth];
					if( nIndex > iRoad )
					{
						mapConn[nIndex] = b ? Max<int8>( mapConn[nIndex], 0 ) : 1;
						break;
					}
					else if( m_gendata[x + y * nWidth] == eType_Temp2 )
						b = true;
					else if( m_gendata[x + y * nWidth] != 0 && m_gendata[x + y * nWidth] != eType_Temp )
						break;
				}
			}
		}

		for( auto& item : mapConn )
		{
			if( item.second )
				continue;
			auto road1 = m_vecRoad[item.first];
			TRectangle<int32> r( Max( road.x, road1.x ), Max( road.y, road1.y ),
				Min( road.GetRight(), road1.GetRight() ) - Max( road.x, road1.x ), Min( road.GetBottom(), road1.GetBottom() ) - Max( road.y, road1.y ) );
			if( r.height < 0 )
			{
				if( r.width <= 2 )
					continue;
				r.height = -r.height;
				r.y -= r.height;
				for( int i = r.x; i < r.GetRight(); i++ )
					vec.push_back( i );
				SRand::Inst().Shuffle( vec );
				for( auto& x : vec )
				{
					bool b = false;
					for( int y = r.y; y < r.GetBottom(); y++ )
					{
						auto n = m_gendata[x + y * nWidth];
						if( n == eType_Temp1 )
							b = true;
						else if( n != 0 && n != eType_Road && n != eType_Temp )
						{
							b = false;
							break;
						}
					}
					if( b )
					{
						for( int y = r.y; y < r.GetBottom(); y++ )
						{
							auto& n = m_gendata[x + y * nWidth];
							if( n == eType_Temp1 )
								n = eType_Temp;
						}
						break;
					}
				}
				vec.resize( 0 );
			}
			else
			{
				if( r.height <= 2 )
					continue;
				r.width = -r.width;
				r.x -= r.width;
				for( int j = r.y; j < r.GetBottom(); j++ )
					vec.push_back( j );
				SRand::Inst().Shuffle( vec );
				for( auto& y : vec )
				{
					bool b = false;
					for( int x = r.x; x < r.GetRight(); x++ )
					{
						auto n = m_gendata[x + y * nWidth];
						if( n == eType_Temp2 )
							b = true;
						else if( n != 0 && n != eType_Road && n != eType_Temp )
						{
							b = false;
							break;
						}
					}
					if( b )
					{
						for( int x = r.x; x < r.GetRight(); x++ )
						{
							auto& n = m_gendata[x + y * nWidth];
							if( n == eType_Temp2 )
								n = eType_Temp;
						}
						break;
					}
				}
				vec.resize( 0 );
			}
		}
	}

	for( int iRect = m_vecGreenbelt.size() - 1; iRect >= 0; iRect-- )
	{
		auto& rect = m_vecGreenbelt[iRect];
		int i1, i2, j1, j2;
		bool b = rect.width > rect.height;
		if( rect.width > rect.height )
		{
			i1 = rect.x; j1 = rect.y; i2 = rect.GetRight(); j2 = rect.GetBottom();
		}
		else
		{
			i1 = rect.y; j1 = rect.x; i2 = rect.GetBottom(); j2 = rect.GetRight();
		}
		uint8 nDir = SRand::Inst().Rand( 0, 2 );
		bool b0 = false;
		int32 k = SRand::Inst().Rand( 3, 7 );
		int32 s = 0;
		for( int i = i1; i < i2; i++ )
		{
			int32 x = nDir == 0 ? i : i2 + i1 - 1 - i;
			bool b1 = true;
			for( int y = j1; y < j2; y++ )
			{
				if( m_gendata[b ? x + y * nWidth : y + x * nWidth] != eType_Temp )
				{
					b1 = false;
					break;
				}
			}
			if( b1 && !b0 )
			{
				b0 = true;
				k = SRand::Inst().Rand( 2, 4 );
			}
			else if( i == i2 - 1 )
				b0 = false;
			else
			{
				k--;
				if( !k )
				{
					if( b1 )
						k = 1;
					else
					{
						b0 = !b0;
						k = b0 ? SRand::Inst().Rand( 1, 3 ) : SRand::Inst().Rand( 3, 7 );
					}
				}
			}

			for( int y = j1; y < j2; y++ )
			{
				m_gendata[b ? x + y * nWidth : y + x * nWidth] = b0 ? eType_Greenbelt : eType_Greenbelt_1_1;
			}
			if( b0 )
				s++;
		}

		if( j2 - j1 >= 3 && s >= ( i2 - i1 ) / 5 && s <= ( i2 - i1 ) / 2 && i2 - i1 >= 6 )
		{
			m_vecHouse.push_back( rect );

			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
					m_gendata[i + j * nWidth] = m_gendata[i + j * nWidth] == eType_Greenbelt ? eType_House_2 : eType_House_0;
			}

			m_vecGreenbelt[iRect] = m_vecGreenbelt.back();
			m_vecGreenbelt.pop_back();
		}
		else if( j2 - j1 >= 3 && i2 - i1 <= ( j2 - j1 ) * 2 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
					m_gendata[i + j * nWidth] = 0;
			}

			m_vecGreenbelt[iRect] = m_vecGreenbelt.back();
			m_vecGreenbelt.pop_back();
		}
	}
}

void CLevelGenNode2_2_0::FillEmpty()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( int iRoad = m_vecRoad.size() - 1; iRoad >= 0; iRoad-- )
	{
		auto& road = m_vecRoad[iRoad];
		auto rect = road;
		if( road.GetRight() < nWidth / 2 - 3 || road.x == 0 )
		{
			rect.SetLeft( 0 );
			rect.SetRight( road.x );
		}
		else if( road.x >= nWidth - nWidth / 2 + 3 || road.GetRight() == nWidth )
		{
			rect.SetRight( nWidth );
			rect.SetLeft( road.GetRight() );
		}
		else
			continue;
		bool b = true;
		for( int x = rect.x; x < rect.GetRight() && b; x++ )
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
		if( !b )
			continue;
		for( int x = road.x; x < road.GetRight(); x++ )
		{
			for( int y = road.y; y < road.GetBottom(); y++ )
				m_gendata[x + y * nWidth] = 0;
		}
		if( road.height <= 6 && road.width > road.height * 2 )
		{
			int32 w = Max( 3, Min( road.width / 4, SRand::Inst().Rand( 4, 7 ) ) );
			road.width -= w;
			if( rect.x == 0 )
				road.x += w;
		}
		else
		{
			auto r = rect + road;
			if( r.height < 5 || r.width <= SRand::Inst().Rand( 10, 13 ) )
			{
				m_vecRoad[iRoad] = m_vecRoad.back();
				m_vecRoad.pop_back();
				continue;
			}
			int32 w = Max( 3, Min( rect.height / 2, Min( rect.width, SRand::Inst().Rand( 4, 6 ) ) ) );
			if( rect.x == 0 )
				road.SetLeft( road.GetRight() - w );
			else
				road.width = w;
		}

		for( int x = road.x; x < road.GetRight(); x++ )
		{
			for( int y = road.y; y < road.GetBottom(); y++ )
				m_gendata[x + y * nWidth] = eType_Road;
		}
	}
	for( int i = 1; i < nWidth - 1; i++ )
	{
		for( int j = 0; j < nHeight - 1; j++ )
		{
			if( m_gendata[i + j * nWidth] )
				continue;
			TVector2<int32> ofs[3] = { { -1, 0 }, { 1, 0 }, { 0, 1 } };
			for( int k = 0; k < 3; k++ )
			{
				auto nType = m_gendata[i + ofs[k].x + ( j + ofs[k].y ) * nWidth];
				if( nType == eType_Greenbelt || nType == eType_House_2 )
				{
					m_gendata1[i + j * nWidth] = 3;
					break;
				}
			}
		}
	}

	vector<TVector2<int32> > vec, vec1, vec2;
	for( int k = nWidth / 2 - 1; k >= 0; k-- )
	{
		for( int i = 0; i < 2; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
				vec.push_back( TVector2<int32>( i ? k : nWidth - k - 1, j ) );
		}
		SRand::Inst().Shuffle( vec );
		for( auto& p : vec )
		{
			if( m_gendata[p.x + p.y * nWidth] )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 3 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp );
			if( rect.width )
			{
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						if( m_gendata1[i + j * nWidth] )
							m_gendata[i + j * nWidth] = eType_Temp1;
					}
				}
				int8 nDir = 0;
				if( rect.x == 0 )
					nDir = -1;
				else if( rect.GetRight() == nWidth )
					nDir = 1;
				else
				{
					int32 s[2] = { 0 };
					for( int i = 0; i < 2; i++ )
					{
						int32 x = i == 0 ? rect.x - 1 : rect.GetRight();
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							if( !m_gendata[x + y * nWidth] )
								s[i]++;
						}
					}
					if( Max( s[0], s[1] ) > rect.height / 3 )
						nDir = s[0] + SRand::Inst().Rand( 0, 2 ) > s[1] ? -1 : 1;
				}
				if( nDir == 0 )
				{
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						for( int j = rect.y; j < rect.GetBottom(); j++ )
							vec1.push_back( TVector2<int32>( i, j ) );
					}
					SRand::Inst().Shuffle( vec1 );
				}
				else
				{
					int8 nDir1;
					int32 x = nDir == -1 ? rect.x : rect.GetRight() - 1;
					int32 x1 = nDir == -1 ? rect.x - 1 : rect.GetRight();
					int8 b1 = ( rect.y == 0 || m_gendata[x + ( rect.y - 1 ) * nWidth] ) * 2 + ( x1 < 0 || x1 >= nWidth || m_gendata[x1 + rect.y * nWidth] );
					int8 b2 = ( rect.GetBottom() == nHeight || m_gendata[x + rect.GetBottom() * nWidth] ) * 2 + ( x1 < 0 || x1 >= nWidth || m_gendata[x1 + ( rect.GetBottom() - 1 ) * nWidth] );
					nDir1 = b1 >= b2;
					for( int i = 0; i < rect.width; i++ )
					{
						int32 x = nDir == -1 ? rect.x + i : rect.GetRight() - 1 - i;
						for( int j = 0; j < rect.height; j++ )
						{
							int32 y = nDir1 ? rect.y + j : rect.GetBottom() - 1 - j;
							vec1.push_back( TVector2<int32>( x, y ) );
						}
					}
					for( int j = 0; j < rect.height; j++ )
					{
						int32 y = nDir1 ? rect.y + j : rect.GetBottom() - 1 - j;
						for( int i = 0; i < rect.width; i++ )
						{
							int32 x = nDir == -1 ? rect.x + i : rect.GetRight() - 1 - i;
							vec2.push_back( TVector2<int32>( x, y ) );
						}
					}
				}
				int32 s = rect.width * rect.height * SRand::Inst().Rand( 0.8f, 0.9f );
				int32 i1 = vec1.size() - 1;
				for( auto& p : vec1 )
				{
					if( m_gendata[p.x + p.y * nWidth] != eType_Temp )
						continue;
					TVector2<int32> maxSize;
					maxSize.x = SRand::Inst().Rand( 4, 7 );
					maxSize.y = Min( maxSize.x, SRand::Inst().Rand( 3, 5 ) );
					auto r1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 3 ), maxSize, rect, -1, eType_Temp );
					if( r1.width > 0 )
					{
						if( r1.x == 0 || r1.GetRight() == nWidth )
						{
							if( r1.GetBottom() == rect.GetBottom() - 1 )
								r1.SetBottom( rect.GetBottom() );
							else if( r1.y == rect.y + 1 )
								r1.SetTop( rect.y );
						}
					}
					else if( p.x == 0 || p.x == nWidth - 1 )
					{
						r1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 5, 2 ), TVector2<int32>( rect.width, 2 ), rect, -1, eType_Temp );
						if( r1.width <= 0 )
							continue;
					}
					else
						continue;

					int8 nType = eType_Cargo1;
					if( r1.x == 0 || r1.GetRight() == nWidth )
					{
						if( r1.height <= 2 )
							nType = eType_House_0;
						else if( r1.height >= 4 )
						{
							int32 s1 = 0;
							for( int y = r1.y; y < r1.GetBottom(); y++ )
							{
								for( int i = 0; i < 2; i++ )
								{
									int32 x = r1.x == 0 ? r1.GetRight() + i : r1.x - 1 - i;
									if( x < 0 || x >= nWidth )
										break;
									if( m_gendata[x + y * nWidth] == eType_Road )
									{
										s1++;
										break;
									}
									else if( m_gendata[x + y * nWidth] != 0 && m_gendata[x + y * nWidth] != eType_Temp )
										break;
								}
							}
							if( s1 > r1.height / 2 )
								nType = eType_House_0;
						}
					}
					if( nType == eType_Cargo1 )
						m_vecCargo1.push_back( r1 );
					else
						m_vecHouse.push_back( r1 );
					for( int x = r1.x; x < r1.GetRight(); x++ )
					{
						for( int y = r1.y; y < r1.GetBottom(); y++ )
							m_gendata[x + y * nWidth] = nType;
					}
					if( nDir )
					{
						int32 n1 = Max( 0, i1 - Max<int32>( 1, r1.width * r1.height * SRand::Inst().Rand( 0.05f, 0.1f ) ) );
						for( ; i1 >= n1; i1-- )
						{
							auto& p1 = vec2[i1];
							if( m_gendata[p1.x + p1.y * nWidth] != eType_Temp )
							{
								if( !n1 )
									break;
								n1--;
							}
							else
								m_gendata[p1.x + p1.y * nWidth] = eType_Temp1;
						}
					}
					s -= r1.width * r1.height;
					if( s <= 0 )
						break;
				}

				if( nDir )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						for( int i = 0; i < rect.width; i++ )
						{
							int32 x = nDir == -1 ? rect.x + i : rect.GetRight() - 1 - i;
							if( m_gendata[x + y * nWidth] < eType_Temp )
							{
								for( i--; i >= 0; i-- )
								{
									int32 x = nDir == -1 ? rect.x + i : rect.GetRight() - 1 - i;
									m_gendata[x + y * nWidth] = 0;
								}
								break;
							}
						}
					}
				}

				vec1.resize( 0 );
				vec2.resize( 0 );
			}
		}

		vec.resize( 0 );
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] >= eType_Temp )
				m_gendata[i + j * nWidth] = 0;
		}
	}
}

void CLevelGenNode2_2_0::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] <= eType_Road )
				m_gendata1[i + j * nWidth] = 0;
			else
				m_gendata1[i + j * nWidth] = 1;
		}
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			auto nType = m_gendata[i + j * nWidth];
			if( nType >= eType_Room_Car_0 && nType <= eType_Room_Car_3 )
			{
				TRectangle<int32> rect;
				if( nType == eType_Room_Car_0 )
					rect = TRectangle<int32>( i + 1, j, 2, 1 );
				else if( nType == eType_Room_Car_2 )
					rect = TRectangle<int32>( i - 2, j, 2, 1 );
				else
					rect = TRectangle<int32>( i, j - 2, 1, 2 );
				rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						if( m_gendata1[x + y * nWidth] == 0 )
							m_gendata1[x + y * nWidth] = 2;
					}
				}
			}
		}
	}

	vector<TVector2<int32> > vec, vec1;
	for( int32 x = 0; x < nWidth; x++ )
	{
		for( int32 y = 1; y < nHeight - 1; y++ )
		{
			if( !m_gendata1[x + y * nWidth] && !m_gendata1[x + ( y + 1 ) * nWidth] && m_gendata1[x + ( y - 1 ) * nWidth] == 1 )
			{
				m_gendata1[x + y * nWidth] = 3;
				vec.push_back( TVector2<int32>( x, y ) );
			}
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata1[p.x + p.y * nWidth] != 3 )
			continue;
		auto rect = PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 1, 1 ), TVector2<int32>( nWidth, 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
		if( rect.width <= 1 )
			continue;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < nHeight; y++ )
			{
				if( m_gendata1[x + y * nWidth] == 1 )
					break;
				rect.height = Max( rect.height, y - rect.y + 1 );
			}
		}
		int32 s = Max( rect.width, rect.height ) * 2 * SRand::Inst().Rand( 0.7f, 0.9f );
		int32 s1 = ( s + SRand::Inst().Rand( 0, 15 ) ) / 15;
		vec1.push_back( TVector2<int32>( rect.x, rect.y ) );
		vec1.push_back( TVector2<int32>( rect.GetRight() - 1, rect.y ) );
		SRand::Inst().Shuffle( vec1 );
		int i;
		for( i = 0; i < vec1.size() && ( s > 0 || s1 > 0 ); i++ )
		{
			int32 a = SRand::Inst().Rand<int32>( i, vec1.size() );
			if( a != i )
				swap( vec1[a], vec1[i] );
			auto p1 = vec1[i];
			if( m_gendata1[p1.x + p1.y * nWidth] )
				continue;
			if( s1 > 0 )
			{
				int a = p1.y == rect.y ? 0 : SRand::Inst().Rand( 1, 3 );
				if( a == 0 )
				{
					if( !SRand::Inst().Rand( 0, 3 ) )
					{
						auto nType = m_gendata[p1.x + ( p1.y - 1 ) * nWidth];
						if( nType == eType_Greenbelt || nType == eType_House_2 )
						{
							auto r = PutRect( m_gendata1, nWidth, nHeight, p1, TVector2<int32>( 1, 2 ), TVector2<int32>( 1, 2 ), rect, -1, 1 );
							if( r.width > 0 )
							{
								m_vecBarrel[0].push_back( r );
								s -= 2;
								s1--;
								if( s > 0 )
								{
									if( r.x > rect.x && m_gendata1[r.x - 1 + r.y * nWidth] == 0 && m_gendata1[r.x - 1 + ( r.y - 1 ) * nWidth] == 1 )
										vec1.push_back( TVector2<int32>( r.x - 1, r.y ) );
									if( r.GetRight() < rect.GetRight() && m_gendata1[r.GetRight() + r.y * nWidth] == 0 && m_gendata1[r.GetRight() + ( r.y - 1 ) * nWidth] == 1 )
										vec1.push_back( TVector2<int32>( r.GetRight(), r.y ) );
								}
								continue;
							}
						}
					}
				}
				else if( a <= 2 )
				{
					bool b = true;
					for( int i1 = 0; i1 < 5; i1++ )
					{
						int32 x = a == 1 ? p1.x + i1 : p1.x - i1 - 1;
						if( x < 0 || x >= nWidth )
						{
							b = false;
							break;
						}
						if( m_gendata[x + p1.y * nWidth] == eType_Greenbelt || m_gendata[x + p1.y * nWidth] == eType_House_2 )
							break;
						if( m_gendata1[x + p1.y * nWidth] || m_gendata[x + p1.y * nWidth] > eType_Road )
						{
							b = false;
							break;
						}
					}
					if( b )
					{
						auto r = PutRect( m_gendata1, nWidth, nHeight, p1, TVector2<int32>( 2, 1 ), TVector2<int32>( 2, 1 ), rect, -1, 1 );
						if( r.width > 0 )
						{
							m_vecBarrel[a].push_back( r );
							s -= 2;
							s1--;
							if( s > 0 && r.GetBottom() < rect.GetBottom() )
							{
								for( int x = r.x; x < r.GetRight(); x++ )
								{
									if( m_gendata1[x + r.GetBottom() * nWidth] == 0 )
										vec1.push_back( TVector2<int32>( x, r.GetBottom() ) );
								}
							}
							int32 x0 = a == 1 ? r.GetRight() : rect.x;
							int32 x1 = a == 1 ? rect.GetRight() : r.x;
							for( int x = x0; x < x1; x++ )
								m_gendata1[x + r.y * nWidth] = 2;
							continue;
						}
					}
				}
			}
			if( s <= 0 )
			{
				if( p1.y + 1 >= rect.GetBottom() || m_gendata1[p1.x + ( p1.y + 1 ) * nWidth] )
					continue;
				if( rect.GetBottom() - p1.y <= SRand::Inst().Rand( 3, 6 )  )
				{
					if( rect.GetBottom() >= nHeight )
						continue;
					auto nType = m_gendata[p1.x + rect.GetBottom() * nWidth];
					if( nType != eType_Greenbelt && nType != eType_House_2 )
						continue;
				}
				m_gendata1[p1.x + ( p1.y + 1 ) * nWidth] = m_gendata1[p1.x + ( p1.y + 1 ) * nWidth] = 1;
				m_vecBarrel[0].push_back( TRectangle<int32>( p1.x, p1.y, 1, 2 ) );
				s1--;
			}

			TVector2<int32> maxSize( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 4 ) );
			maxSize.y = Max( 2, Min( maxSize.x - 1, maxSize.y ) );
			auto r = PutRect( m_gendata1, nWidth, nHeight, p1, TVector2<int32>( 2, 2 ), maxSize, rect, -1, 0 );
			if( r.width <= 0 )
				continue;
			if( r.width > 2 )
			{
				int32 s = 0;
				for( int x = r.x; x < r.GetRight(); x++ )
				{
					if( m_gendata1[x + ( rect.y - 1 ) * nWidth] != 1 )
						s++;
				}
				if( s * 2 >= r.width )
					r = PutRect( m_gendata1, nWidth, nHeight, p1, TVector2<int32>( 2, 2 ), TVector2<int32>( 2, 2 ), r, -1, 0 );
			}
			for( int x = r.x; x < r.GetRight(); x++ )
			{
				for( int y = r.y; y < r.GetBottom(); y++ )
				{
					m_gendata1[x + y * nWidth] = 1;
				}
			}
			m_vecCargo.push_back( r );
			s -= r.width * r.height;

			if( r.x > rect.x && m_gendata1[r.x - 1 + r.y * nWidth] == 0 && m_gendata1[r.x - 1 + ( r.y - 1 ) * nWidth] == 1 )
				vec1.push_back( TVector2<int32>( r.x - 1, r.y ) );
			if( r.GetRight() < rect.GetRight() && m_gendata1[r.GetRight() + r.y * nWidth] == 0 && m_gendata1[r.GetRight() + ( r.y - 1 ) * nWidth] == 1 )
				vec1.push_back( TVector2<int32>( r.GetRight(), r.y ) );
			if( r.GetBottom() < rect.GetBottom() )
			{
				for( int x = r.x; x < r.GetRight(); x++ )
				{
					if( m_gendata1[x + r.GetBottom() * nWidth] == 0 )
						vec1.push_back( TVector2<int32>( x, r.GetBottom() ) );
				}
			}
		}
		vec1.resize( 0 );
	}

	vector<TRectangle<int32> > vecRect;
	for( auto rect : m_vecCargo1 )
	{
		bool b[4] = { false };
		if( rect.x + rect.GetRight() + SRand::Inst().Rand( 0, 2 ) > nWidth )
			b[0] = rect.width >= SRand::Inst().Rand( 4, 6 );
		else
			b[2] = rect.width >= SRand::Inst().Rand( 4, 6 );
		if( rect.height >= SRand::Inst().Rand( 5, 10 ) )
			b[1] = b[3] = true;
		else if( rect.height >= SRand::Inst().Rand( 3, 6 ) )
		{
			if( SRand::Inst().Rand( 0, 2 ) )
				b[1] = true;
			else
				b[3] = true;
		}
		if( b[1] )
		{
			m_vecBar[0].push_back( TRectangle<int32>( rect.x, rect.GetBottom() - 1, rect.width, 1 ) );
			rect.height--;
		}
		if( b[3] )
		{
			m_vecBar[0].push_back( TRectangle<int32>( rect.x, rect.y, rect.width, 1 ) );
			rect.SetTop( rect.y + 1 );
		}
		if( b[0] )
		{
			m_vecBar[1].push_back( TRectangle<int32>( rect.GetRight() - 1, rect.y, 1, rect.height ) );
			rect.width--;
		}
		if( b[2] )
		{
			m_vecBar[1].push_back( TRectangle<int32>( rect.x, rect.y, 1, rect.height ) );
			rect.SetLeft( rect.x + 1 );
		}
		vecRect.push_back( rect );
	}

	for( int iRect = 0; iRect < vecRect.size(); iRect++ )
	{
		auto& rect = vecRect[iRect];
		if( rect.width > rect.height )
		{
			if( rect.width >= SRand::Inst().Rand( 7, 9 ) && rect.height >= 3 )
			{
				int32 w = SRand::Inst().Rand( 2, rect.width - 2 );
				vecRect.push_back( TRectangle<int32>( rect.x, rect.y, w, rect.height ) );
				m_vecBar[1].push_back( TRectangle<int32>( rect.x + w, rect.y, 1, SRand::Inst().Rand( Max( 3, rect.height / 2 ), rect.height + 1 ) ) );
				vecRect.push_back( TRectangle<int32>( rect.x + w + 1, rect.y, rect.width - w - 1, rect.height ) );
				continue;
			}
		}
		else
		{
			if( rect.height >= SRand::Inst().Rand( 7, 9 ) && rect.width >= 3 )
			{
				int32 h = SRand::Inst().Rand( 2, rect.height - 2 );
				vecRect.push_back( TRectangle<int32>( rect.x, rect.y, rect.width, h ) );
				int32 w1 = SRand::Inst().Rand( Max( 3, rect.width / 2 ), rect.width + 1 );
				m_vecBar[0].push_back( TRectangle<int32>( rect.x + SRand::Inst().Rand( 0, rect.width - w1 + 1 ), rect.y + h, w1, 1 ) );
				vecRect.push_back( TRectangle<int32>( rect.x, rect.y + h + 1, rect.width, rect.height - h - 1 ) );
				continue;
			}
		}

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = 0;
				vec1.push_back( TVector2<int32>( i, j ) );
			}
		}
		SRand::Inst().Shuffle( vec1 );
		int32 s = rect.width * rect.height * SRand::Inst().Rand( 0.85f, 0.9f );
		int32 s1 = ( s + SRand::Inst().Rand( 0, 6 ) ) / 6;
		for( auto& p : vec1 )
		{
			if( s > 0 )
			{
				auto r1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 4 ) ),
					rect, -1, eType_Obj1 );
				if( r1.width > 0 )
				{
					m_vecCargo.push_back( r1 );
					s -= r1.width * r1.height;
				}
			}
			else if( s1 > 0 )
			{
				int32 a = SRand::Inst().Rand( 0, 5 );
				if( a > 0 )
				{
					if( rect.x + rect.GetRight() + SRand::Inst().Rand( 0, 2 ) >= nWidth )
						a = 2;
					else
						a = 1;
				}
				TVector2<int32> size[3] = { { 1, 2 }, { 2, 1 }, { 2, 1 } };
				auto r1 = PutRect( m_gendata, nWidth, nHeight, p, size[a], size[a], rect, -1, eType_Obj1 );
				if( r1.width > 0 )
				{
					m_vecBarrel[a].push_back( r1 );
					s1--;
				}
			}
			else
				break;
		}
		vec1.resize( 0 );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Cargo1;
			}
		}
	}

	for( auto& rect : m_vecCargo )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				int32 a = SRand::Inst().Rand( 0, 4 );
				if( a == 0 )
					m_vecBroken.push_back( TVector2<int32>( i, j ) );
				else if( a == 1 )
					m_vecBox.push_back( TVector2<int32>( i, j ) );
			}
		}
	}

	for( int i = 0; i < m_gendata1.size(); i++ )
	{
		if( m_gendata1[i] == 1 && m_gendata[i] <= eType_Road )
			m_gendata[i] = m_gendata[i] == eType_Road ? eType_Obj1 : eType_Obj;
		else if( m_gendata[i] == eType_Road || m_gendata[i] == eType_Greenbelt || m_gendata[i] == eType_House_2 )
			m_gendata[i] = 0;
	}
}

void CLevelGenNode2_2_0::GenHouses()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TRectangle<int32> > vecRect;
	vector<TVector2<int32> > vec;
	auto Func = [=] ( const TRectangle<int32>& rect, const TVector2<int32>& p )
	{
		TRectangle<int32> r( p.x - 1, p.y - 1, 3, 3 );
		r = r * rect;
		for( int x = r.x; x < r.GetRight(); x++ )
		{
			for( int y = r.y; y < r.GetBottom(); y++ )
			{
				if( m_gendata[x + y * nWidth] > eType_House_2 )
					return false;
			}
		}
		return true;
	};
	for( auto& rect : m_vecHouse )
	{
		if( rect.width <= 2 || rect.height <= 2 )
			continue;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] != eType_House_0 )
					continue;
				auto r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i, j ), TVector2<int32>( 1, 1 ), rect.GetSize(), rect, -1, eType_Temp );
				vecRect.push_back( r );
			}
		}

		SRand::Inst().Shuffle( vecRect );
		for( auto& r : vecRect )
		{
			if( r.width <= 2 || r.height <= 2 )
			{
				if( r.height > r.width )
				{
					for( int i = r.x; i < r.GetRight(); i++ )
					{
						int8 nType = eType_House_0 + SRand::Inst().Rand( 0, 2 );
						for( int j = r.y; j < r.GetBottom(); j++ )
							m_gendata[i + j * nWidth] = nType;
					}
					if( r.width >= 2 )
					{
						int32 x = Max( rect.x + 1, Min( rect.GetRight() - 2, SRand::Inst().Rand( r.x, r.GetRight() ) ) );
						int32 y = SRand::Inst().Rand( 0, r.height / 2 + 1 );
						int32 y1 = SRand::Inst().Rand( 0, r.height );
						for( int j = 0; j < y; j++ )
							m_gendata[x + ( r.y + ( j + y1 ) % r.height ) * nWidth] = eType_House_2;
					}
				}
				else
				{
					for( int j = r.y; j < r.GetBottom(); j++ )
					{
						int8 nType = eType_House_0 + SRand::Inst().Rand( 0, 2 );
						for( int i = r.x; i < r.GetRight(); i++ )
							m_gendata[i + j * nWidth] = nType;
					}
					if( r.height >= 2 )
					{
						int32 y = Max( rect.y + 1, Min( rect.GetRight() - 2, SRand::Inst().Rand( r.y, r.GetBottom() ) ) );
						int32 x = SRand::Inst().Rand( 0, r.width / 2 + 1 );
						int32 x1 = SRand::Inst().Rand( 0, r.width );
						for( int i = 0; i < x; i++ )
							m_gendata[( r.x + ( i + x1 ) % r.width ) + y * nWidth] = eType_House_2;
					}
				}
			}
			else
			{
				CLevelGen2::GenHouse( m_gendata, nWidth, nHeight, r, eType_House_0, 2, 3 );
				for( int i = 0; i < 2; i++ )
				{
					if( i == 0 ? r.x > rect.x : r.GetRight() < rect.GetRight() )
					{
						int32 x = i == 0 ? r.x : r.GetRight() - 1;
						int32 y0 = SRand::Inst().Rand( 2, r.height );
						int32 y1 = SRand::Inst().Rand( 0, r.height );
						for( int j = 0; j < y0; j++ )
						{
							int32 y = r.y + ( j + y1 ) % r.height;
							if( !Func( r, TVector2<int32>( x, y ) ) )
								break;
							m_gendata[x + y * nWidth] = eType_House_2;
						}
					}
				}
				for( int j = 0; j < 2; j++ )
				{
					if( j == 0 ? r.y > rect.y : r.GetBottom() < rect.GetBottom() )
					{
						int32 y = j == 0 ? r.y : r.GetBottom() - 1;
						int32 x0 = SRand::Inst().Rand( 2, r.width );
						int32 x1 = SRand::Inst().Rand( 0, r.width );
						for( int i = 0; i < x0; i++ )
						{
							int32 x = r.x + ( i + x1 ) % r.width;
							if( !Func( r, TVector2<int32>( x, y ) ) )
								break;
							m_gendata[x + y * nWidth] = eType_House_2;
						}
					}
				}
			}
		}

		vecRect.resize( 0 );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_House_2 )
					m_gendata[i + j * nWidth] = 0;
				if( m_gendata[i + j * nWidth] == 0 )
					vec.push_back( TVector2<int32>( i, j ) );
			}
		}

		SRand::Inst().Shuffle( vec );
		for( auto& p : vec )
		{
			if( m_gendata[p.x + p.y * nWidth] )
				continue;
			auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 4, 4 ), rect, -1, eType_House_2 );
			if( r.width <= 0 )
			{
				m_gendata[p.x + p.y * nWidth] = eType_House_2;
				continue;
			}
			r = TRectangle<int32>( r.x + ( r.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2, r.y + ( r.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2, 2, 2 );
			for( int i = r.x; i < r.GetRight(); i++ )
			{
				for( int j = r.y; j < r.GetBottom(); j++ )
					m_gendata[i + j * nWidth] = eType_House_2_4;
			}
		}

		vec.resize( 0 );
	}
	for( auto& rect : m_vecRoad )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}
	for( auto& rect : m_vecGreenbelt )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Greenbelt;
			}
		}
	}

	vector<int32> vec1;
	vec1.resize( nWidth );
	for( int i = 0; i < nWidth; i++ )
		vec1[i] = i;
	int32 n = SRand::Inst().Rand( 20, 25 );
	for( int j = 0; j < nHeight; j++ )
	{
		SRand::Inst().Shuffle( vec1 );
		for( int i = 0; i < nWidth; i++ )
		{
			int32 x = vec1[i];
			if( m_gendata[x + j * nWidth] != eType_Greenbelt_1_1 )
				continue;
			n--;
			if( n <= 0 )
			{
				n = SRand::Inst().Rand( 20, 25 );
				m_gendata[x + j * nWidth] = eType_Greenbelt_1_2;
			}
		}
	}
}

void CTruckNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	CLevelGenerateNode::Load( pXml, context );
	m_pBaseNode[0] = CreateNode( pXml->FirstChildElement( "base0" )->FirstChildElement(), context );
	m_pBaseNode[1] = CreateNode( pXml->FirstChildElement( "base1" )->FirstChildElement(), context );
	m_pControlRoomNode[0] = CreateNode( pXml->FirstChildElement( "room0" )->FirstChildElement(), context );
	m_pControlRoomNode[1] = CreateNode( pXml->FirstChildElement( "room1" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pCargo2Node = CreateNode( pXml->FirstChildElement( "cargo2" )->FirstChildElement(), context );
}

void CTruckNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	bool b = region.x * 2 + region.GetRight() > context.nWidth + SRand::Inst().Rand( 0, 2 ) - 0.5f;
	int32 nBaseHeight = region.height > 4 ? 2 : 0;
	TRectangle<int32> rect1;
	TRectangle<int32> controlRoomRect;
	if( region.width < 10 )
	{
		nBaseHeight = 0;
		rect1 = TRectangle<int32>( 0, 0, region.width, region.height );
		controlRoomRect = TRectangle<int32>( 0, 0, 0, 0 );
	}
	else if( b )
	{
		rect1 = TRectangle<int32>( 4, nBaseHeight, region.width - 4, region.height - nBaseHeight );
		controlRoomRect = TRectangle<int32>( 0, nBaseHeight, 4, region.height - nBaseHeight );
	}
	else
	{
		rect1 = TRectangle<int32>( 0, nBaseHeight, region.width - 4, region.height - nBaseHeight );
		controlRoomRect = TRectangle<int32>( region.width - 4, nBaseHeight, 4, region.height - nBaseHeight );
	}
	if( nBaseHeight )
	{
		TRectangle<int32> rect0( region.x, region.y, region.width, 2 );
		m_pBaseNode[b ? 1 : 0]->Generate( context, rect0 );
	}
	if( controlRoomRect.width > 0 )
		m_pControlRoomNode[b ? 1 : 0]->Generate( context, controlRoomRect.Offset( TVector2<int32>( region.x, region.y ) ) );
	uint32 h1 = rect1.height - nBaseHeight;
	uint32 l = rect1.width;
	uint32 n = Max( 1u, l / Max( h1 + 2, 6u ) );
	vector<int32> vec;
	for( int i = 0; i < n; i++ )
		vec.push_back( ( l + i ) / n );
	SRand::Inst().Shuffle( vec );
	int32 x = 0;
	int32 k = SRand::Inst().Rand( 0, Max<int32>( 4, n + 1 ) );
	for( int i = 0; i < n; i++ )
	{
		( i == k ? m_pCargoNode : m_pCargo2Node )->Generate( context,
			TRectangle<int32>( rect1.x + x, rect1.y, vec[i], rect1.height ).Offset( TVector2<int32>( region.x, region.y ) ) );
		x += vec[i];
	}
}

void CLevelGenNode2_2_1::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pWalkableNodes[0] = CreateNode( pXml->FirstChildElement( "walkable_a" )->FirstChildElement(), context );
	m_pWalkableNodes[1] = CreateNode( pXml->FirstChildElement( "walkable_b" )->FirstChildElement(), context );
	m_pWalkableNodes[2] = CreateNode( pXml->FirstChildElement( "walkable_c" )->FirstChildElement(), context );
	m_pWalkableNodes[3] = CreateNode( pXml->FirstChildElement( "walkable_d" )->FirstChildElement(), context );
	m_pBlockNode = CreateNode( pXml->FirstChildElement( "block" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	m_pFenceNode = CreateNode( pXml->FirstChildElement( "fence" )->FirstChildElement(), context );
	m_pPlantNode = CreateNode( pXml->FirstChildElement( "plant" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pCargo2Node = CreateNode( pXml->FirstChildElement( "cargo2" )->FirstChildElement(), context );
	m_pBarrelNode = CreateNode( pXml->FirstChildElement( "barrel" )->FirstChildElement(), context );
	m_pBarrel1Node = CreateNode( pXml->FirstChildElement( "barrel1" )->FirstChildElement(), context );
	m_pControlRoomNode = CreateNode( pXml->FirstChildElement( "control_room" )->FirstChildElement(), context );
	m_pTruckNode = CreateNode( pXml->FirstChildElement( "truck" )->FirstChildElement(), context );
	
	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_2_1::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenChunks();
	GenHouses();
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
	for( auto& rect : m_vecRoads1 )
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

	context.mapTags["mask"] = eType_Temp;
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
	context.mapTags["s"] = 1;
	/*for( auto& house : m_vecHouses )
	{
		if( house.rect.width )
			m_pHouseNode->Generate( context, house.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}*/

	for( auto& rect : m_vecObjs )
	{
		if( rect.width == 2 && rect.height == 2 )
			m_pCargo2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else if( rect.width == 2 && rect.height == 3 )
			m_pBarrelNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pBarrel1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecCargos )
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecControlRooms )
		m_pControlRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecTrucks )
		m_pTruckNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_par.clear();
	m_vecRoads.clear();
	m_vecRoads1.clear();
	m_vecFences.clear();
	m_vecFenceBlock.clear();
	m_vecRooms.clear();
	//m_vecHouses.clear();
	m_vecCargos.clear();
	m_vecControlRooms.clear();
	m_vecTrucks.clear();
	m_vecObjs.clear();
}

void CLevelGenNode2_2_1::GenChunks()
{
	struct SChunkRect
	{
		SChunkRect() {}
		SChunkRect( const TRectangle<int32>& rect, int32 nType ) : rect( rect ), nType( nType ) {}
		TRectangle<int32> rect;
		int32 nType;
	};
	vector<SChunkRect> vecRect;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int32> vecCurHeight;
	vecCurHeight.resize( nWidth );
	for( int i = 0; i < nWidth; i++ )
		vecCurHeight[i] = 0;
	uint32 nCurHeight = 0;
	uint32 nMaxHeight = nHeight - 16;
	uint32 nMaxHeight1 = nHeight - 12;

	int32 nLastType = -1;
	while( nCurHeight < nMaxHeight )
	{
		uint32 nType = nLastType < 4 ? 4 : SRand::Inst().Rand( 0, 4 );
		uint32 nPreCount = vecRect.size();
		if( nType < 4 )
		{
			uint32 w1[10];
			uint32 a1, a2, bMin, bMax, hMin, hMax;
			if( nType == 0 )
			{
				a1 = 3;
				a2 = 2;
				bMin = 6;
				bMax = 7;
				hMin = 11;
				hMax = 16;
			}
			else if( nType == 1 )
			{
				a1 = 3;
				a2 = 3;
				bMin = 6;
				bMax = 7;
				hMin = 12;
				hMax = 16;
			}
			else if( nType == 2 )
			{
				a1 = 3;
				a2 = 4;
				bMin = 5;
				bMax = 6;
				hMin = 13;
				hMax = 16;
			}
			else if( nType == 3 )
			{
				a1 = 4;
				a2 = 3;
				bMin = 5;
				bMax = 6;
				hMin = 14;
				hMax = 16;
			}

			uint32 w = nWidth;
			for( int i = 0; i < a1; i++ )
			{
				w1[i] = SRand::Inst().Rand( bMin, bMax + 1 );
				w -= w1[i];
			}
			for( int i = a1; i < a1 + a2; i++ )
				w1[i] = ( w + i - a1 ) / a2;
			SRand::Inst().Shuffle( w1 + a1, a2 );

			uint32 h = Min( nMaxHeight1 - nCurHeight, SRand::Inst().Rand( hMin, hMax + 1 ) );
			uint32 s = 0;
			uint8 b = SRand::Inst().Rand( 0, 2 );
			for( int i = 0; i < a1 + a2; i++ )
			{
				int32 n;
				if( a1 < a2 )
					n = ( i >> 1 ) + ( i & 1 ? 0 : a1 );
				else if( a1 > a2 )
					n = ( i >> 1 ) + ( i & 1 ? a1 : 0 );
				else
					n = ( i >> 1 ) + ( ( i + b ) & 1 ? 0 : a1 );
				vecRect.push_back( SChunkRect( TRectangle<int32>( s, nCurHeight, w1[n], h ), n < a1 ? 1 : 0 ) );
				s += w1[n];
			}
		}
		else
		{
			uint32 nType1;
			if( nMaxHeight1 - nCurHeight <= 5 )
				nType1 = 3;
			else
				nType1 = SRand::Inst().Rand( 0, 4 );

			if( nType1 == 0 )
			{
				uint32 w = SRand::Inst().Rand( nWidth * 3 / 5, nWidth * 2 / 3 );
				vecRect.push_back( SChunkRect( TRectangle<int32>( 0, nCurHeight, w, SRand::Inst().Rand( 7, 9 ) ), 2 ) );
				vecRect.push_back( SChunkRect( TRectangle<int32>( w, nCurHeight, nWidth - w, SRand::Inst().Rand( 7, 9 ) ), 0 ) );
			}
			else if( nType1 == 1 )
			{
				uint32 w = SRand::Inst().Rand( nWidth * 3 / 5, nWidth * 2 / 3 );
				vecRect.push_back( SChunkRect( TRectangle<int32>( 0, nCurHeight, nWidth - w, SRand::Inst().Rand( 7, 9 ) ), 0 ) );
				vecRect.push_back( SChunkRect( TRectangle<int32>( nWidth - w, nCurHeight, w, SRand::Inst().Rand( 7, 9 ) ), 2 ) );
			}
			else if( nType1 == 2 )
			{
				uint32 w = SRand::Inst().Rand( nWidth * 3 / 5, nWidth * 2 / 3 );
				uint32 w1 = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
				vecRect.push_back( SChunkRect( TRectangle<int32>( 0, nCurHeight, w1, SRand::Inst().Rand( 6, 8 ) ), 3 ) );
				vecRect.push_back( SChunkRect( TRectangle<int32>( w1, nCurHeight, w, SRand::Inst().Rand( 6, 8 ) ), 2 ) );
				vecRect.push_back( SChunkRect( TRectangle<int32>( w + w1, nCurHeight, nWidth - w - w1, SRand::Inst().Rand( 6, 8 ) ), 3 ) );
			}
			else if( nType1 == 3 )
			{
				uint32 w = SRand::Inst().Rand( nWidth / 3, nWidth * 2 / 5 );
				uint32 w1 = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
				vecRect.push_back( SChunkRect( TRectangle<int32>( 0, nCurHeight, w1, SRand::Inst().Rand( 6, 8 ) ), 2 ) );
				vecRect.push_back( SChunkRect( TRectangle<int32>( w1, nCurHeight, w, SRand::Inst().Rand( 6, 8 ) ), 3 ) );
				vecRect.push_back( SChunkRect( TRectangle<int32>( w + w1, nCurHeight, nWidth - w - w1, SRand::Inst().Rand( 6, 8 ) ), 2 ) );
			}
		}

		uint32 nCurCount = vecRect.size();
		for( int i = nPreCount; i < nCurCount; i++ )
		{
			auto& rect = vecRect[i];
			while( rect.rect.y > 0 )
			{
				bool bSuccess = true;
				for( int32 x = rect.rect.x; x < rect.rect.GetRight(); x++ )
				{
					if( m_gendata[x + ( rect.rect.y - 1 ) * nWidth] )
					{
						bSuccess = false;
						break;
					}
				}
				if( !bSuccess )
					break;

				rect.rect.y--;
			}

			if( rect.rect.GetBottom() > nMaxHeight1 )
				rect.rect.SetBottom( nMaxHeight1 );
			nCurHeight = Max<uint32>( nCurHeight, rect.rect.GetBottom() );

			for( int x = rect.rect.x; x < rect.rect.GetRight(); x++ )
			{
				for( int y = rect.rect.y; y < rect.rect.GetBottom(); y++ )
				{
					m_gendata[x + y * nWidth] = 1;
				}
			}
		}

		nLastType = nType;
	}

	memset( &m_gendata[0], 0, nWidth * nHeight );
	for( auto& rect : vecRect )
	{
		rect.rect.y = nHeight - rect.rect.GetBottom();
		for( int x = rect.rect.x; x < rect.rect.GetRight(); x++ )
		{
			for( int y = rect.rect.y; y < rect.rect.GetBottom(); y++ )
			{
				m_gendata[x + y * nWidth] = eType_Temp;
			}
		}
	}
	
	vector<TVector2<int32> > vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] )
				break;
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	bool b = false;
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		TRectangle<int32> rect;
		if( SRand::Inst().Rand( 0, 2 ) )
			rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 12, 2 ), TVector2<int32>( SRand::Inst().Rand( 12, 17 ), 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Road );
		else
			rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 4 ), TVector2<int32>( 2, SRand::Inst().Rand( 4, 9 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Road );
		if( rect.width <= 0 )
			continue;
		if( b )
		{
			m_vecFences.push_back( rect );
			if( rect.width == 2 )
				m_vecFenceBlock.push_back( rect );
			else
			{
				int32 w1 = SRand::Inst().Rand( 2, 4 );
				int32 w2 = SRand::Inst().Rand( 2, 4 );
				m_vecFenceBlock.push_back( TRectangle<int32>( rect.x, rect.y, w1, rect.height ) );
				m_vecFenceBlock.push_back( TRectangle<int32>( rect.GetRight() - w2, rect.y, w2, rect.height ) );
			}

		}
		else
			m_vecRoads1.push_back( rect );
		b = !b;
	}
	for( auto& rect : m_vecFenceBlock )
	{
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				m_gendata[x + y * nWidth] = eType_Chunk_Plant;
			}
		}
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
				m_gendata[i + j * nWidth] = eType_None;
		}
	}

	for( auto& chunkRect : vecRect )
	{
		switch( chunkRect.nType )
		{
		case 0:
			m_vecRoads.push_back( chunkRect.rect );
			for( int x = chunkRect.rect.x; x < chunkRect.rect.GetRight(); x++ )
			{
				for( int y = chunkRect.rect.y; y < chunkRect.rect.GetBottom(); y++ )
				{
					m_gendata[x + y * nWidth] = eType_Road;
				}
			}
			break;
		case 1:
		{
			auto& rect = chunkRect.rect;
			if( rect.height < 10 )
			{
				for( int x = chunkRect.rect.x; x < chunkRect.rect.GetRight(); x++ )
				{
					for( int y = chunkRect.rect.y; y < chunkRect.rect.GetBottom(); y++ )
					{
						m_gendata[x + y * nWidth] = eType_Temp;
					}
				}
				continue;
			}

			uint32 h0 = Min( rect.width / 2 + 2, rect.height - 6 );
			if( rect.width < 6 || rect.height - h0 + SRand::Inst().Rand( 0, 3 ) <= 14 )
			{
				TRectangle<int32> roomRect( rect.x, rect.y, rect.width, rect.height - h0 );
				TRectangle<int32> controlRoomRect( rect.x, rect.GetBottom() - h0, rect.width, h0 );
				m_vecRooms.push_back( roomRect );
				m_vecControlRooms.push_back( controlRoomRect );
				for( int x = roomRect.x; x < roomRect.GetRight(); x++ )
				{
					for( int y = roomRect.y; y < roomRect.GetBottom(); y++ )
					{
						m_gendata[x + y * nWidth] = eType_Room;
					}
				}
				for( int x = controlRoomRect.x; x < controlRoomRect.GetRight(); x++ )
				{
					for( int y = controlRoomRect.y; y < controlRoomRect.GetBottom(); y++ )
					{
						m_gendata[x + y * nWidth] = eType_Chunk;
					}
				}
			}
			else
			{
				uint32 h = SRand::Inst().Rand( 2, 5 );
				if( rect.height + h < 12 + h0 )
				{
					TRectangle<int32> roomRect( rect.x, rect.y, rect.width, rect.height - h0 - h );
					TRectangle<int32> cargoRect( rect.x, rect.GetBottom() - h0 - h, rect.width, h );
					TRectangle<int32> controlRoomRect( rect.x, rect.GetBottom() - h0, rect.width, h0 );
					if( SRand::Inst().Rand( 0, 2 ) )
					{
						cargoRect.y = rect.y;
						roomRect.y = cargoRect.GetBottom();
					}
					m_vecRooms.push_back( roomRect );
					m_vecCargos.push_back( cargoRect );
					m_vecControlRooms.push_back( controlRoomRect );
					for( int x = roomRect.x; x < roomRect.GetRight(); x++ )
					{
						for( int y = roomRect.y; y < roomRect.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Room;
						}
					}
					for( int x = cargoRect.x; x < cargoRect.GetRight(); x++ )
					{
						for( int y = cargoRect.y; y < cargoRect.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Room;
						}
					}
					for( int x = controlRoomRect.x; x < controlRoomRect.GetRight(); x++ )
					{
						for( int y = controlRoomRect.y; y < controlRoomRect.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Chunk;
						}
					}
				}
				else
				{
					TRectangle<int32> roomRect( rect.x, rect.y, rect.width, ( rect.height - h0 + SRand::Inst().Rand( 0, 1 ) ) / 2 );
					TRectangle<int32> room1Rect( rect.x, roomRect.GetBottom(), rect.width, rect.GetBottom() - h0 - roomRect.GetBottom() );
					TRectangle<int32> controlRoomRect( rect.x, rect.GetBottom() - h0, rect.width, h0 );
					m_vecRooms.push_back( roomRect );
					m_vecRooms.push_back( room1Rect );
					m_vecControlRooms.push_back( controlRoomRect );
					for( int x = roomRect.x; x < roomRect.GetRight(); x++ )
					{
						for( int y = roomRect.y; y < roomRect.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Room;
						}
					}
					for( int x = room1Rect.x; x < room1Rect.GetRight(); x++ )
					{
						for( int y = room1Rect.y; y < room1Rect.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Room;
						}
					}
					for( int x = controlRoomRect.x; x < controlRoomRect.GetRight(); x++ )
					{
						for( int y = controlRoomRect.y; y < controlRoomRect.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Chunk;
						}
					}
				}
			}
			break;
		}
		case 2:
		{
			auto& rect = chunkRect.rect;
			m_vecTrucks.push_back( rect );
			for( int x = rect.x; x < rect.GetRight(); x++ )
			{
				for( int y = rect.y; y < rect.GetBottom(); y++ )
				{
					m_gendata[x + y * nWidth] = eType_Chunk;
				}
			}

			break;
		}
		case 3:
			for( int x = chunkRect.rect.x; x < chunkRect.rect.GetRight(); x++ )
			{
				for( int y = chunkRect.rect.y; y < chunkRect.rect.GetBottom(); y++ )
				{
					m_gendata[x + y * nWidth] = eType_Temp;
				}
			}
			break;
		}
	}

	for( auto& room : m_vecRooms )
	{
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

void CLevelGenNode2_2_1::GenHouses()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector2<int32> > vecTemp;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Temp, vecTemp );
	for( auto& p : vecTemp )
		m_gendata[p.x + p.y * nWidth] = eType_None;

	for( auto& p : vecTemp )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		TVector2<int32> sizeMin, sizeMax;
		if( SRand::Inst().Rand( 0, 2 ) )
			sizeMin = TVector2<int32>( 4, 6 );
		else
			sizeMin = TVector2<int32>( 6, 4 );
		sizeMax = sizeMin + TVector2<int32>( 2, 2 );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin, sizeMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House );
		if( rect.width > 0 )
		{
			//SHouse house( rect );
			//m_vecHouses.push_back( house );
		}
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
				m_gendata1[i + j * nWidth] = 1;
			else if( m_gendata[i + j * nWidth] == eType_Road )
				m_gendata1[i + j * nWidth] = 2;
			else
				m_gendata1[i + j * nWidth] = 0;
		}
	}

	/*for( auto& house : m_vecHouses )
	{
		if( !house.Generate( m_gendata, nWidth, nHeight, eType_House, eType_House_1, eType_House_2, eType_House_Exit1, eType_House_Exit2,
			m_gendata1, eType_None, eType_Temp1, m_par ) )
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
	}*/

	for( auto& p : vecTemp )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		TVector2<int32> sizeMin( 2, 2 ), sizeMax( 5, 3 );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin, sizeMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Chunk );
		if( rect.width > 0 )
			m_vecCargos.push_back( rect );
	}
	ExpandDist( m_gendata, nWidth, nHeight, eType_Temp1, eType_None, 1 );
	for( auto& p : vecTemp )
	{
		if( !m_gendata[p.x + p.y * nWidth] )
			m_gendata[p.x + p.y * nWidth] = eType_Temp;
	}
	ExpandDist( m_gendata, nWidth, nHeight, eType_Temp, eType_None, 3 );
	
	for( auto& nType : m_gendata )
	{
		if( nType == eType_None )
			nType = eType_Temp1;
	}
	int8 nTypes[4] = { eType_Walkable_a, eType_Walkable_b, eType_Walkable_c, eType_Walkable_d };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 64, 128, eType_Temp1, nTypes, 4 );
}

void CLevelGenNode2_2_1::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int8> gendata1;
	gendata1.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth * nHeight; i++ )
	{
		gendata1[i] = m_gendata[i] < eType_Temp ? 0 : 1;
	}

	for( auto& rect : m_vecRoads )
	{
		int32 s = rect.width * rect.height;
		int32 r = SRand::Inst().Rand( 0, s );
		if( r > 60 && !SRand::Inst().Rand( 0, 3 ) )
		{
			TRectangle<int32> rect1( rect.x, rect.y, 4, 2 );
			rect1.x += SRand::Inst().Rand( 0, rect.width - rect1.width + 1 );
			m_vecObjs.push_back( rect1 );
		}
		else if( r > 40 && !SRand::Inst().Rand( 0, 2 ) )
		{
			TRectangle<int32> rect1( rect.x, rect.y, 2, 3 );
			rect1.x += SRand::Inst().Rand( 0, rect.width - rect1.width + 1 );
			m_vecObjs.push_back( rect1 );
		}
		else
		{
			uint32 n = ( r + SRand::Inst().Rand( 0, 30 ) ) / 30;
			for( int i = 0; i < n; i++ )
			{
				TRectangle<int32> rect1( rect.x, rect.y + 2 * i, 2, 2 );
				rect1.x += SRand::Inst().Rand( 0, rect.width - rect1.width + 1 );
				m_vecObjs.push_back( rect1 );
			}
		}
	}

	for( auto& obj : m_vecObjs )
	{
		for( int i = obj.x; i < obj.GetRight(); i++ )
		{
			for( int j = obj.y; j < obj.GetBottom(); j++ )
			{
				gendata1[i + j * nWidth] = 2;
			}
		}
	}
	LvGenLib::DropObj1( gendata1, nWidth, nHeight, m_vecObjs, 0, 2 );
}

void CLevelGenNode2_2_2::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pChunkNode = CreateNode( pXml->FirstChildElement( "chunk" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pSupportANode = CreateNode( pXml->FirstChildElement( "support_a" )->FirstChildElement(), context );
	m_pSupportBNode = CreateNode( pXml->FirstChildElement( "support_b" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pBigChunk1Node = CreateNode( pXml->FirstChildElement( "big_chunk_1" )->FirstChildElement(), context );
	m_pBigChunk2Node = CreateNode( pXml->FirstChildElement( "big_chunk_2" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pThrusterNode = CreateNode( pXml->FirstChildElement( "thruster" )->FirstChildElement(), context );
	m_pFill1Node = CreateNode( pXml->FirstChildElement( "fill1" )->FirstChildElement(), context );
	m_pFill2Node = CreateNode( pXml->FirstChildElement( "fill2" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_2_2::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenPath();
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
	for( auto& rect : m_vecChunks )
		m_pChunkNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecBars )
		m_pBarNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecSupports )
	{
		if( !!( rect.y & 1 ) )
			m_pSupportANode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pSupportBNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecRoads )
		m_pRoadNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecCargos )
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecThrusters )
		m_pThrusterNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecBigChunk2 )
		m_pBigChunk2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );

	context.mapTags["door"] = eType_Door;
	for( auto& rect : m_vecRooms )
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	context.mapTags["1"] = eType_Chunk1;
	for( auto& rect : m_vecBigChunk1 )
		m_pBigChunk1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	for( auto& rect : m_vecBigChunk1_1 )
		m_pBigChunk1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );

	context.mapTags["mask"] = eType_None;
	m_pFill1Node->Generate( context, region );
	context.mapTags["mask"] = eType_Blocked;
	m_pFill2Node->Generate( context, region );

	context.mapTags.clear();
	m_gendata.clear();
	m_vecChunks.clear();
	m_vecBars.clear();
	m_vecSupports.clear();
	m_vecRoads.clear();
	m_vecCargos.clear();
	m_vecBigChunk1.clear();
	m_vecBigChunk1_1.clear();
	m_vecBigChunk2.clear();
	m_vecRooms.clear();
	m_vecThrusters.clear();
}

void CLevelGenNode2_2_2::GenPath()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector3<int32> > vecPath;
	TVector3<int32> p( SRand::Inst().Rand( nWidth / 4, nWidth / 3 ), 0, SRand::Inst().Rand( 4, 6 ) );
	vecPath.push_back( p );
	p.x = nWidth - nWidth / 3 + SRand::Inst().Rand( 0, 5 );
	vecPath.push_back( p );
	int32 k = SRand::Inst().Rand( 4, 6 );
	while( p.y + p.z < nHeight )
	{
		auto p1 = vecPath[vecPath.size() - 2];
		auto p2 = vecPath[vecPath.size() - 1];

		k--;
		p.y += p.z;
		if( !k && p.y + 6 <= nHeight )
		{
			p.z = Min( nHeight - p.y, SRand::Inst().Rand( 6, 9 ) );
			if( nHeight - p.y < 8 )
				p.z = nHeight - p.y;
			vecPath.push_back( p );
			if( ( p.x < 4 || p.x >= nWidth - 4 ) && SRand::Inst().Rand( 0, 2 ) )
			{
				k = SRand::Inst().Rand( 2, 5 );
				p.x = SRand::Inst().Rand( 4, 6 ) * ( p.x >= nWidth / 2 ? -1 : 1 ) + nWidth / 2;
			}
			else
			{
				k = 3;
				p.x = p.x >= nWidth / 2 ? 0 : nWidth - 1;
			}
			vecPath.push_back( p );
		}
		else
		{
			if( nHeight - p.y <= 7 )
				p.z = nHeight - p.y;
			else
				p.z = Min( nHeight - 4 - p.y, 5 );
			vecPath.push_back( p );
		}
	}
	if( SRand::Inst().Rand( 0, 2 ) )
	{
		for( auto& p : vecPath )
		{
			p.x = nWidth - 1 - p.x;
		}
	}

	vector<TRectangle<int32> > vecTempChunks;
	vector<TRectangle<int32> > vecTempBars;
	TRectangle<int32> r;
	int32 k1 = SRand::Inst().Rand( 1, 3 ), k2 = SRand::Inst().Rand( 2, 5 );
	for( int i = 0; i < vecPath.size(); i++ )
	{
		auto& cur = vecPath[i];

		if( i >= vecPath.size() - 1 )
		{
			TRectangle<int32> r1 = r;
			r1.SetBottom( cur.y + cur.z );
			r1.SetTop( Max( r.GetTop(), cur.y ) );
			if( r1.height >= 5 )
				AddRoom( r1, true, true, true, true );
			if( r.y < r1.y )
				AddRoad( TRectangle<int32>( r.x, r.y, r.width, r1.y - r.y ) );
			break;
		}
		auto& nxt = vecPath[i + 1];

		if( i == 0 )
		{
			r = TRectangle<int32>( nxt.x, nxt.y, 1, nxt.z );
			if( nxt.x < cur.x )
				r.SetRight( Min( r.GetRight() + SRand::Inst().Rand( 8, 10 ), cur.x - 3 ) );
			else
				r.SetLeft( Max( r.GetLeft() - SRand::Inst().Rand( 8, 10 ), cur.x + 4 ) );

			TRectangle<int32> r1;
			if( nxt.x < cur.x )
			{
				r1 = TRectangle<int32>( r.GetRight(), 0, 0, cur.z );
				r1.SetRight( Min( cur.x + 1 + SRand::Inst().Rand( 3, 7 ), nWidth - 4 ) );
			}
			else
			{
				r1 = TRectangle<int32>( r.x, 0, 0, cur.z );
				r1.SetLeft( Max( cur.x - SRand::Inst().Rand( 3, 7 ), 4 ) );
			}
			r1 = r1 + r;
			vecTempChunks.push_back( TRectangle<int32>( 0, r1.y, r1.x, r1.GetBottom() ) );
			vecTempChunks.push_back( TRectangle<int32>( r1.GetRight(), r1.y, nWidth - r1.GetRight(), r1.GetBottom() ) );

			continue;
		}

		auto& pre = vecPath[i - 1];
		if( pre.y == cur.y )
			continue;
		k1--;
		k2--;

		if( k1 <= 0 && k2 <= 0 )
		{
			if( cur.y == nxt.y )
			{
				if( nxt.y + nxt.z < nHeight )
				{
					int32 nLen = abs( cur.x - nxt.x ) + 1;
					bool bRoomRoadEnd, bRoomBegin, bRoadAtEnd;
					if( nLen >= nWidth - 3 )
					{
						bRoomRoadEnd = false;
						bRoomBegin = true;
						bRoadAtEnd = true;
					}
					else if( nLen >= 17 )
					{
						bool b = SRand::Inst().Rand( 0, 2 ) > 0;
						bRoomRoadEnd = b;
						bRoomBegin = !b;
						bRoadAtEnd = b;
					}
					else
					{
						bRoomRoadEnd = false;
						bRoomBegin = false;
						bRoadAtEnd = false;
					}

					TRectangle<int32> r1, r2;
					r2 = TRectangle<int32>( Min( cur.x, nxt.x ), cur.y, nLen, cur.z );
					if( bRoomBegin )
					{
						int32 nBottom = r.GetBottom();
						r.SetBottom( Max( cur.y + 3, cur.y + cur.z - SRand::Inst().Rand( 0, 3 ) ) );
						vecTempChunks.push_back( TRectangle<int32>( r.x, r.GetBottom(), r.width, nBottom - r.GetBottom() ) );
						AddRoad( r );

						r1 = TRectangle<int32>( nxt.x < cur.x ? r.GetLeft(): r.GetRight(), cur.y, 0, Max( cur.z, SRand::Inst().Rand( 5, 7 ) ) );
						if( nxt.x < cur.x )
						{
							r1.SetLeft( r1.GetLeft() - SRand::Inst().Rand( 5, 8 ) );
							AddRoom( r1, false, true, false, true );
						}
						else
						{
							r1.SetRight( r1.GetRight() + SRand::Inst().Rand( 5, 8 ) );
							AddRoom( r1, true, false, false, true );
						}

						if( nxt.x < cur.x )
						{
							r2.SetRight( r1.x );
							vecTempBars.push_back( TRectangle<int32>( r1.x - 1, r1.y - 1, 2, 1 ) );
						}
						else
						{
							r2.SetLeft( r1.GetRight() );
							vecTempBars.push_back( TRectangle<int32>( r2.x - 1, r2.y - 1, 2, 1 ) );
						}
					}
					else if( bRoomRoadEnd )
					{
						auto& nxt1 = vecPath[i + 2];
						uint32 z1 = nxt.z + nxt1.z;
						r1 = TRectangle<int32>( r.x, cur.y, r.width, z1 );
						if( r.height > 0 )
						{
							r1.SetTop( r1.y + SRand::Inst().Rand( 0, 2 ) );
							r.SetBottom( r1.y );
							AddRoad( r );
						}
						r1.SetBottom( r1.GetBottom() - SRand::Inst().Rand( 0, 3 ) );
						AddBigChunk( r1, 1 );
						vecTempChunks.push_back( TRectangle<int32>( r1.x, r1.GetBottom(), r1.width, cur.y + z1 - r1.GetBottom() ) );

						if( nxt.x < cur.x )
							r2.SetRight( r.x );
						else
							r2.SetLeft( r.GetRight() );
					}
					else
					{
						int32 nBottom = r.GetBottom();
						r.SetBottom( Max( cur.y + 3, cur.y + cur.z - SRand::Inst().Rand( 0, 3 ) ) );
						vecTempChunks.push_back( TRectangle<int32>( r.x, r.GetBottom(), r.width, nBottom - r.GetBottom() ) );
						AddRoad( r );
						if( nxt.x < cur.x )
							r2.SetRight( r.x );
						else
							r2.SetLeft( r.GetRight() );
					}

					r = TRectangle<int32>( nxt.x + ( nxt.x > cur.x ? 1 : 0 ), nxt.y, 0, nxt.z );
					if( nxt.x < cur.x )
					{
						r.SetRight( Min( r.GetRight() + SRand::Inst().Rand( 5, 8 ), nWidth - 2 ) );
						if( r.x > 0 )
							vecTempChunks.push_back( TRectangle<int32>( 0, r.y, r.x, 1 ) );
					}
					else
					{
						r.SetLeft( Max( r.GetLeft() - SRand::Inst().Rand( 5, 8 ), 2 ) );
						if( r.GetRight() < nWidth )
							vecTempChunks.push_back( TRectangle<int32>( r.GetRight(), r.y, nWidth - r.GetRight(), 1 ) );
					}
					if( bRoadAtEnd )
					{
						if( nxt.x < cur.x )
						{
							r2.SetLeft( r.GetRight() );
							vecTempBars.push_back( TRectangle<int32>( r.x, r.y - 1, r.width + 1, 1 ) );
						}
						else
						{
							r2.SetRight( r.GetLeft() );
							vecTempBars.push_back( TRectangle<int32>( r.x - 1, r.y - 1, r.width + 1, 1 ) );
						}
					}
					else
					{
						r.SetTop( r.GetBottom() );
					}
					AddBigChunk( r2, 2 );

					k1 = SRand::Inst().Rand( 3, 5 );
					k2 = SRand::Inst().Rand( 3, 5 );
					continue;
				}
			}
		}
		if( k2 <= 0 )
		{
			if( cur.y == nxt.y )
			{
				int32 nLen = abs( cur.x - nxt.x ) + 1;
				bool bRoomBegin, bRoomEnd;
				if( nLen >= nWidth - 3 )
				{
					bRoomBegin = true;
					bRoomEnd = true;
				}
				else
				{
					bRoomBegin = SRand::Inst().Rand() > 0;
					bRoomEnd = false;
				}

				TRectangle<int32> r1, r2;
				r2 = TRectangle<int32>( Min( cur.x, nxt.x ), cur.y, nLen, cur.z );

				r1 = TRectangle<int32>( r.x, cur.y, r.width, cur.z );
				if( r1.GetBottom() - r.y <= SRand::Inst().Rand( 8, 12 ) )
				{
					r1.SetTop( r.y );
					if( nxt.x > cur.x )
						r1.SetRight( Min( nWidth, Max( r1.GetLeft() + 5, r1.GetRight() - SRand::Inst().Rand( 2, 4 ) ) ) );
					else
						r1.SetLeft( Max( 0, Min( r1.GetRight() - 5, r1.GetLeft() + SRand::Inst().Rand( 2, 4 ) ) ) );
					if( bRoomBegin )
						AddRoom( r1, nxt.x < cur.x, nxt.x > cur.x, true, false );
					else
						AddBigChunk( r1, 1 );
				}
				else if( bRoomBegin )
				{
					if( r.height > 0 )
					{
						r.SetBottom( r1.y );
						AddRoad( r );
					}
					if( nxt.x > cur.x )
						r1.SetRight( Min( nWidth, Max( r1.GetLeft() + 5, r1.GetRight() - SRand::Inst().Rand( 2, 4 ) ) ) );
					else
						r1.SetLeft( Max( 0, Min( r1.GetRight() - 5, r1.GetLeft() + SRand::Inst().Rand( 2, 4 ) ) ) );
					AddRoom( r1, nxt.x < cur.x, nxt.x > cur.x, true, false );
				}
				else
				{
					r1.SetTop( r.y );
					AddRoad( r1 );
				}
				vecTempChunks.push_back( TRectangle<int32>( r1.x, r1.GetBottom(), r1.width, 0 ) );

				if( nxt.x < cur.x )
					r2.SetRight( r1.x );
				else
					r2.SetLeft( r1.GetRight() );

				r = TRectangle<int32>( nxt.x + ( nxt.x > cur.x ? 1 : 0 ), nxt.y, 0, nxt.z );
				if( nxt.x < cur.x )
				{
					r.SetRight( Min( r.GetRight() + SRand::Inst().Rand( 5, 8 ), nWidth - 2 ) );
					if( r.x > 0 )
						vecTempChunks.push_back( TRectangle<int32>( 0, r.y, r.x, 1 ) );
				}
				else
				{
					r.SetLeft( Max( r.GetLeft() - SRand::Inst().Rand( 5, 8 ), 2 ) );
					if( r.GetRight() < nWidth )
						vecTempChunks.push_back( TRectangle<int32>( r.GetRight(), r.y, nWidth - r.GetRight(), 1 ) );
				}
				if( bRoomEnd )
				{
					AddRoom( r, nxt.x > cur.x, nxt.x < cur.x, false, true );
					if( nxt.x < cur.x )
						r2.SetLeft( r.GetRight() );
					else
						r2.SetRight( r.GetLeft() );
				}
				r.SetTop( r.GetBottom() );
				AddBigChunk( r2, 2 );
			}
			else
			{
				TRectangle<int32> r1( r.x, cur.y, r.width, cur.z );
				if( r.height > 0 )
				{
					r1.SetTop( Min( r1.y + SRand::Inst().Rand( -1, 3 ), r1.GetBottom() - 6 ) );
					r.SetBottom( r1.y );
					AddRoad( r );
				}

				if( r1.GetLeft() < 4 )
				{
					r1.SetRight( Min( nWidth, Max( r1.x + 5, r1.GetRight() - SRand::Inst().Rand( 1, 3 ) ) ) );
					AddRoom( r1, true, true, true, true );
					TRectangle<int32> r2( r1.GetRight(), cur.y, 0, cur.z );
					r2.SetRight( Min( nWidth, r2.GetRight() + SRand::Inst().Rand( 15, 18 ) ) );
					AddBigChunk( r2, SRand::Inst().Rand( 0, 2 ) ? 0 : 2 );
					if( r2.GetRight() < nWidth )
						vecTempChunks.push_back( TRectangle<int32>( r2.GetRight(), r2.y, nWidth - r2.GetRight(), r2.height ) );
				}
				else if( r1.GetRight() >= nWidth - 4 )
				{
					r1.SetLeft( Max( 0, Min( r1.GetRight() - 5, r1.GetLeft() + 1 ) ) );
					AddRoom( r1, true, true, true, true );
					TRectangle<int32> r2( r1.GetLeft(), cur.y, 0, cur.z );
					r2.SetLeft( Max( 0, r2.GetRight() - SRand::Inst().Rand( 15, 18 ) ) );
					AddBigChunk( r2, SRand::Inst().Rand( 0, 2 ) ? 0 : 2 );
					if( r2.GetLeft() > 0 )
						vecTempChunks.push_back( TRectangle<int32>( 0, r2.y, nWidth - r2.GetLeft(), r2.height ) );
				}
				else
				{
					r1.SetRight( Min( nWidth, r1.GetRight() - 1 ) );
					r1.SetLeft( Max( 0, r1.GetLeft() + 1 ) );
					if( r1.width > 4 )
						AddRoom( r1, true, true, true, true );
					else
						AddRoad( r1 );
					bool b = SRand::Inst().Rand( 0, 2 );;
					AddBigChunk( TRectangle<int32>( 0, cur.y, r1.x, cur.z ), b ? 0 : 2 );
					AddBigChunk( TRectangle<int32>( r1.GetRight(), cur.y, nWidth - r1.GetRight(), cur.z ), b ? 2 : 0 );
				}

				r = TRectangle<int32>( r.x, r1.GetBottom(), r.width, 0 );
			}

			k2 = SRand::Inst().Rand( 3, 5 );
			continue;
		}
		if( k1 <= 0 )
		{
			if( cur.y + cur.z < nHeight )
			{
				if( cur.y == nxt.y )
				{
					if( cur.z > 4 && i + 2 < vecPath.size() )
					{
						auto& nxt1 = vecPath[i + 2];
						int32 h1 = cur.z - 4;
						cur.z = 4;
						nxt1.y -= h1;
						nxt1.z += h1;
						for( int j = i + 3; j < vecPath.size() && vecPath[j].y == nxt1.y; j++ )
						{
							auto& nxt2 = vecPath[j];
							nxt2.y -= h1;
							nxt2.z += h1;
						}
					}

					auto& nxt1 = vecPath[i + 2];
					if( r.height > 0 )
						AddRoad( r );
					TRectangle<int32> r1 = r;
					r1.y = cur.y;
					r1.height = cur.z;
					r1.SetLeft( Min( r1.GetLeft(), nxt.x ) );
					r1.SetRight( Max( r1.GetRight(), nxt.x + 1 ) );
					AddRoad( r1 );
					r.y = r1.GetBottom();
					r.height = 0;

					nxt.x = cur.x;
					nxt.y = nxt1.y;
					nxt.z = nxt1.z;
					if( k2 > 1 )
					{
						r.height = nxt.z;
						r.SetLeft( Max( 0, r.GetLeft() - SRand::Inst().Rand( 0, 3 ) ) );
						r.SetRight( Min( nWidth, r.GetRight() + SRand::Inst().Rand( 0, 3 ) ) );
						AddRoom( r, nxt1.x < cur.x, nxt1.x > cur.x, true, false );

						if( nxt.x < nxt1.x )
						{
							TRectangle<int32> r1( 0, r.y, r.x, 1 );
							r1.SetRight( Min( r1.GetRight(), SRand::Inst().Rand( 4, 7 ) ) );
							if( r1.width > 0 )
								vecTempChunks.push_back( r1 );

							r = TRectangle<int32>( r.GetRight(), nxt.y, nWidth - r.GetRight(), 1 );
							AddChunk( r, m_vecBars );
							r.y++;
							r.height = nxt.z - 1;
							r.SetLeft( Max( r.GetLeft(), r.GetRight() - SRand::Inst().Rand( 5, 8 ) ) );
						}
						else
						{
							TRectangle<int32> r1( r.GetRight(), r.y, nWidth - r.GetRight(), 1 );
							r1.SetLeft( Max( r1.GetLeft(), nWidth - SRand::Inst().Rand( 4, 7 ) ) );
							if( r1.width > 0 )
								vecTempChunks.push_back( r1 );

							r = TRectangle<int32>( 0, nxt.y, r.x, 1 );
							AddChunk( r, m_vecBars );
							r.y++;
							r.height = nxt.z - 1;
							r.SetRight( Min( r.GetRight(), r.GetLeft() + SRand::Inst().Rand( 5, 8 ) ) );
						}

						i++;
						k2--;
						k1 = SRand::Inst().Rand( 2, 4 );
						continue;
					}
				}
				else
				{
					TRectangle<int32> r0 = r;
					if( i + 2 < vecPath.size() && nxt.y == vecPath[i + 2].y || k2 <= 1 )
					{
						if( r.x + r.GetRight() + SRand::Inst().Rand( 0, 2 ) > nWidth )
							r.x = Min( nWidth - r.width, r.x + SRand::Inst().Rand( 2, 4 ) );
						else
							r.x = Max( 0, r.x - SRand::Inst().Rand( 2, 4 ) );
						r.SetTop( cur.y );
						r.SetBottom( nxt.y );
						AddBigChunk( r, 1 );
						r.SetTop( nxt.y );
					}
					else
					{
						int32 h1 = cur.z + nxt.z;

						TRectangle<int32> r1 = r;
						r1.SetTop( cur.y );
						r1.SetBottom( nxt.y );
						int32 w = SRand::Inst().Rand( 2, 5 );
						r1.width += w;
						r1.x = Max( 0, Min( nWidth - r1.width, r1.x - SRand::Inst().Rand( 1, w ) ) );
						AddBigChunk( r1, 1 );
						if( r.x >= 7 )
						{
							int32 l = Min( r.x, SRand::Inst().Rand( 9, 13 ) );
							int32 l1 = Min( r.width / 2 - 1, SRand::Inst().Rand( 1, 3 ) );
							vecTempBars.push_back( TRectangle<int32>( r.x - l, cur.y - 1, l + l1, 1 ) );
							TRectangle<int32> r2( r.x - l + SRand::Inst().Rand( 1, 3 ), cur.y, 0, cur.z );
							r2.SetRight( Min( r2.GetRight() + SRand::Inst().Rand( 4, 5 ), r1.GetLeft() ) );
							r2.height += SRand::Inst().Rand( 0, 3 );
							AddBigChunk( r2, 1 );
							if( r2.x > 0 )
								vecTempChunks.push_back( TRectangle<int32>( 0, cur.y, r2.x, 1 ) );

							if( r2.x >= 7 )
							{
								l = Min( r2.x, SRand::Inst().Rand( 7, 11 ) );
								int32 h1 = cur.y + cur.z - Min( cur.z - 4, SRand::Inst().Rand( 0, 3 ) );
								vecTempBars.push_back( TRectangle<int32>( r2.x - l, h1 - 1, l, 1 ) );
								TRectangle<int32> r3( r2.x - l + SRand::Inst().Rand( 0, 3 ), h1, 0, nxt.y + nxt.z - h1 );
								r3.SetRight( Min( r3.GetRight() + SRand::Inst().Rand( 4, 5 ), r2.GetLeft() ) );
								AddBigChunk( r3, 1 );
								if( r3.x > 0 )
									vecTempChunks.push_back( TRectangle<int32>( 0, r3.y, r3.x, 1 ) );
							}
						}
						if( r.GetRight() <= nWidth - 7 )
						{
							int32 l = Min( nWidth - r.GetRight(), SRand::Inst().Rand( 9, 13 ) );
							int32 l1 = Min( r.width / 2 - 1, SRand::Inst().Rand( 1, 3 ) );
							vecTempBars.push_back( TRectangle<int32>( r.GetRight() - l1, cur.y - 1, l + l1, 1 ) );
							TRectangle<int32> r2( r.GetRight() + l - SRand::Inst().Rand( 1, 3 ), cur.y, 0, cur.z );
							r2.SetLeft( Max( r2.GetLeft() - SRand::Inst().Rand( 4, 5 ), r1.GetRight() ) );
							r2.height += SRand::Inst().Rand( 0, 3 );
							AddBigChunk( r2, 1 );
							if( r2.GetRight() < nWidth )
								vecTempChunks.push_back( TRectangle<int32>( r2.GetRight(), cur.y, nWidth - r2.GetRight(), 1 ) );

							if( r2.GetRight() <= nWidth - 7 )
							{
								l = Min( nWidth - r2.GetRight(), SRand::Inst().Rand( 7, 11 ) );
								int32 h1 = cur.y + cur.z - Min( cur.z - 4, SRand::Inst().Rand( 0, 3 ) );
								vecTempBars.push_back( TRectangle<int32>( r2.GetRight(), h1 - 1, l, 1 ) );
								TRectangle<int32> r3( r2.GetRight() + l - SRand::Inst().Rand( 0, 3 ), h1, 0, nxt.y + nxt.z - h1 );
								r3.SetLeft( Max( r3.GetLeft() - SRand::Inst().Rand( 4, 5 ), r2.GetRight() ) );
								AddBigChunk( r3, 1 );
								if( r3.GetRight() < nWidth )
									vecTempChunks.push_back( TRectangle<int32>( r3.GetRight(), r3.y, nWidth - r3.GetRight(), 1 ) );
							}
						}

						r.SetTop( nxt.y + nxt.z );
						r.SetBottom( nxt.y + nxt.z );
						i++;
						k2--;
						k1 = SRand::Inst().Rand( 2, 4 );

						if( r0.height > 1 )
						{
							r0.height--;
							AddRoad( r0 );
						}
						continue;
					}
				}
				k1 = SRand::Inst().Rand( 3, 5 );
				continue;
			}
		}

		if( cur.y == nxt.y )
		{
			if( cur.z > 4 && i + 2 < vecPath.size() )
			{
				auto& nxt1 = vecPath[i + 2];
				int32 h1 = cur.z - 4;
				cur.z = 4;
				nxt1.y -= h1;
				nxt1.z += h1;
				for( int j = i + 3; j < vecPath.size() && vecPath[j].y == nxt1.y; j++ )
				{
					auto& nxt2 = vecPath[j];
					nxt2.y -= h1;
					nxt2.z += h1;
				}
			}

			if( r.height > 0 )
				AddRoad( r );
			TRectangle<int32> r1 = r;
			r1.y = cur.y;
			r1.height = cur.z;
			r1.SetLeft( Min( r1.GetLeft(), nxt.x ) );
			r1.SetRight( Max( r1.GetRight(), nxt.x + 1 ) );
			AddRoad( r1 );

			r.y = r1.GetBottom();
			r.width = SRand::Inst().Rand( 5, 8 );
			if( nxt.x > cur.x )
				r.x = r1.GetRight() - r.width;
			else
				r.x = r1.x;
			r.height = 0;
		}
		else
		{
			r.SetBottom( cur.y + cur.z );
			continue;
		}
	}

	SRand::Inst().Shuffle( vecTempBars );
	ProcessTempBar( vecTempBars );
	ProcessTempChunks( vecTempChunks );
}

void CLevelGenNode2_2_2::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( auto& r : m_vecBigChunk1 )
	{
		GenBigChunk( r, 0 );
		AddThrusters( r );
	}
	for( auto& r : m_vecBigChunk1_1 )
		GenBigChunk( r, 1 );
	for( auto& r : m_vecBigChunk2 )
		AddThrusters( r );
	for( auto& r : m_vecBars )
		AddThrusters( r );
}

void CLevelGenNode2_2_2::AddRoad( const TRectangle<int32>& r )
{
	m_vecRoads.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Road;
		}
	}
}

void CLevelGenNode2_2_2::AddRoom( const TRectangle<int32>& r, bool bLeft, bool bRight, bool bTop, bool bBottom )
{
	m_vecRooms.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Room;
		}
	}

	if( bLeft && r.height >= 5 )
	{
		int32 i = r.height > 5 ? ( r.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2 : 2;
		m_gendata[r.x + ( i + r.y ) * m_region.width] = eType_Door;
		if( r.height > 5 )
			m_gendata[r.x + ( i + r.y + 1 ) * m_region.width] = eType_Door;
	}
	if( bRight && r.height >= 5 )
	{
		int32 i = r.height > 5 ? ( r.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2 : 2;
		m_gendata[r.GetRight() - 1 + ( i + r.y ) * m_region.width] = eType_Door;
		if( r.height > 5 )
			m_gendata[r.GetRight() - 1 + ( i + r.y + 1 ) * m_region.width] = eType_Door;
	}
	if( bTop && r.width >= 5 )
	{
		int32 i = r.width > 5 ? ( r.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2 : 2;
		m_gendata[i + r.x + r.y * m_region.width] = eType_Door;
		if( r.width > 5 )
			m_gendata[i + r.x + 1 + r.y * m_region.width] = eType_Door;
	}
	if( bBottom && r.width >= 5 )
	{
		int32 i = r.width > 5 ? ( r.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2 : 2;
		m_gendata[i + r.x + ( r.GetBottom() - 1 ) * m_region.width] = eType_Door;
		if( r.width > 5 )
			m_gendata[i + r.x + 1 + ( r.GetBottom() - 1 ) * m_region.width] = eType_Door;
	}
}

void CLevelGenNode2_2_2::AddChunk( const TRectangle<int32>& r, vector<TRectangle<int32> >& chunks )
{
	chunks.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Chunk;
		}
	}
}

void CLevelGenNode2_2_2::AddBigChunk( const TRectangle<int32>& r, int8 nType )
{
	if( nType == 0 )
		m_vecBigChunk1.push_back( r );
	else if( nType == 1 )
		m_vecBigChunk1_1.push_back( r );
	else
		m_vecBigChunk2.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Chunk;
		}
	}
}

void CLevelGenNode2_2_2::AddThruster( const TRectangle<int32>& r )
{
	m_vecThrusters.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Thruster;
		}
	}
}

void CLevelGenNode2_2_2::AddSupport( const TRectangle<int32>& r )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( r.x < 0 || r.x >= nWidth || r.y < 0 || r.y >= nHeight )
		return;

	auto rect = r;
	while( rect.y > 0 )
	{
		if( m_gendata[rect.x + ( rect.y - 1 ) * nWidth] || m_gendata[rect.x + 1 + ( rect.y - 1 ) * nWidth] )
			break;
		rect.y--;
		rect.height++;
	}
	if( rect.height <= 0 )
		return;
	AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 1, rect.width, 1 ), m_vecChunks );
	rect.height--;

	int32 nBaseHeight = Max( 0, rect.height - SRand::Inst().Rand( 0, 5 ) ) / 4;
	for( int i = nBaseHeight; i < rect.height; i++ )
	{
		AddChunk( TRectangle<int32>( rect.x, rect.y + i, 2, 1 ), m_vecSupports );
	}

	vector<TVector2<int32> > vecTemp;
	int32 x0 = rect.x, x1 = rect.GetRight() - 1, y = rect.y + nBaseHeight - 1;
	int32 y0 = y;
	while( y0 >= 0 )
	{
		int32 nMin = nWidth, nMax = -1;
		for( int x = x0; x <= x1; x++ )
		{
			if( ( y == y0 || m_gendata[x + ( y0 + 1 ) * nWidth] == eType_Chunk1 ) && !m_gendata[x + y0 * nWidth] )
			{
				m_gendata[x + y0 * nWidth] = eType_Chunk1;
				vecTemp.push_back( TVector2<int32>( x, y0 ) );
				nMin = Min( x, nMin );
				nMax = Max( x, nMax );
			}
		}
		if( nMin > nMax )
			break;

		if( nMin > 0 && nMin == x0 && !SRand::Inst().Rand( 0, 2 ) && !m_gendata[nMin - 1 + y0 * nWidth] )
		{
			m_gendata[nMin - 1 + y0 * nWidth] = eType_Chunk1;
			vecTemp.push_back( TVector2<int32>( nMin - 1, y0 ) );
			nMin--;
		}
		if( nMax < nWidth - 1 && nMax == x1 && !SRand::Inst().Rand( 0, 2 ) && !m_gendata[nMax + 1 + y0 * nWidth] )
		{
			m_gendata[nMax + 1 + y0 * nWidth] = eType_Chunk1;
			vecTemp.push_back( TVector2<int32>( nMax + 1, y0 ) );
			nMax++;
		}
		x0 = nMin;
		x1 = nMax;
		y0--;
	}
	for( auto& p : vecTemp )
		m_gendata[p.x + p.y * nWidth] = eType_Blocked;
}

void CLevelGenNode2_2_2::ProcessTempBar( const vector<TRectangle<int32> >& v )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( auto rect : v )
	{
		bool bContinue = false;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			if( m_gendata[i + rect.y * nWidth] )
			{
				bContinue = true;
				break;
			}
		}
		if( bContinue )
			continue;

		if( rect.width <= 2 )
		{
			int32 nBegin = rect.x + 1;
			int32 nEnd = rect.GetRight() - 2;
			while( nBegin > 0 && !m_gendata[nBegin - 1 + rect.y * nWidth] )
				nBegin--;
			while( nEnd < nWidth - 1 && !m_gendata[nEnd + 1 + rect.y * nWidth] )
				nEnd++;
			if( nEnd - nBegin < 1 )
				continue;
			int32 nLen = Min<int32>( 2 + floor( ( nEnd - nBegin - 1 ) * SRand::Inst().Rand( 0.75f, 0.99f ) ), SRand::Inst().Rand( 10, 16 ) );
			rect.x -= ( nLen - rect.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
			rect.width = nLen;
			rect.x = Max( rect.x, nBegin );
			rect.x = Min( rect.x, nEnd - nLen + 1 );
		}
		AddChunk( rect, m_vecBars );
	}
}

void CLevelGenNode2_2_2::ProcessTempChunks( const vector<TRectangle<int32> >& v )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( auto& r : v )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			for( int i = r.x; i < r.GetRight(); i++ )
			{
				if( !m_gendata[i + j * m_region.width] )
					m_gendata[i + j * m_region.width] = eType_Blocked;
			}
		}
	}

	float f = 0.5f;
	for( int i = 0; i < nHeight; i++ )
	{
		if( m_gendata[i * nWidth] == eType_Blocked )
		{
			f = 0.5f;
			for( int x = 0; x < nWidth / 2; x++ )
			{
				if( m_gendata[x + i * nWidth] != eType_Blocked )
					break;
				f += 1;
			}
		}
		else
		{
			f = Max( 0.0f, Min( nWidth / 2.0f - 0.5f, f + SRand::Inst().Rand( -0.3f, 0.3f ) ) );
			int32 n = floor( f );
			int x;
			for( x = 0; x < n; x++ )
			{
				if( m_gendata[x + i * nWidth] )
					break;
				m_gendata[x + i * nWidth] = eType_Blocked;
			}
			f = Min( x + 0.5f, f );
		}
	}

	f = 0.5f;
	for( int i = 0; i < nHeight; i++ )
	{
		if( m_gendata[nWidth - 1 + i * nWidth] == eType_Blocked )
		{
			f = 0.5f;
			for( int x = 0; x < nWidth / 2; x++ )
			{
				if( m_gendata[nWidth - 1 - x + i * nWidth] != eType_Blocked )
					break;
				f += 1;
			}
		}
		else
		{
			f = Max( 0.0f, Min( nWidth / 2.0f - 0.5f, f + SRand::Inst().Rand( -0.3f, 0.3f ) ) );
			int32 n = floor( f );
			int x;
			for( x = 0; x < n; x++ )
			{
				if( m_gendata[nWidth - 1 - x + i * nWidth] )
					break;
				m_gendata[nWidth - 1 - x + i * nWidth] = eType_Blocked;
			}
			f = Min( x + 0.5f, f );
		}
	}
}

void CLevelGenNode2_2_2::AddThrusters( const TRectangle<int32>& r )
{
	if( r.y < 12 )
		return;
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 maxl = 0, maxn = 0;
	int32 nCur = 0;
	uint8 bDir = SRand::Inst().Rand( 0, 2 );
	for( int i = 0; i < r.width; i++ )
	{
		int32 x = bDir ? i + r.x : r.GetRight() - i - 1;
		bool b = true;
		for( int j = r.y - 1; j >= r.y - 4; j-- )
		{
			if( m_gendata[x + j * nWidth] )
			{
				b = false;
				break;
			}
		}
		if( b )
		{
			nCur++;
			if( nCur > maxl )
			{
				maxl = nCur;
				maxn = i;
			}
		}
		else
			nCur = 0;
	}

	if( maxl < 10 )
		return;
	maxn = maxn + 1 - maxl;

	int32 nType = SRand::Inst().Rand( 0, 3 );
	switch( nType )
	{
	case 0:
	{
		int32 l = ( maxl + 2 ) / 4 * 4 - 2;
		int32 ofs = SRand::Inst().Rand( 0, maxl - l + 1 );

		for( int i = 0; i < l; i += 4 )
		{
			int32 x = i + ofs + maxn;
			x = bDir ? x + r.x : r.GetRight() - 2 - x;
			AddThruster( TRectangle<int32>( x, r.y - 2, 2, 2 ) );
			AddSupport( TRectangle<int32>( x, r.y - 2, 2, 0 ) );
		}

		break;
	}
	case 1:
	{
		int32 l = ( maxl + 3 ) / 7 * 7 - 3;
		int32 ofs = SRand::Inst().Rand( 0, maxl - l + 1 );

		for( int i = 0; i < l; i += 7 )
		{
			int32 x = i + ofs + maxn;
			x = bDir ? x + r.x : r.GetRight() - 4 - x;
			AddThruster( TRectangle<int32>( x, r.y - 2, 2, 2 ) );
			AddThruster( TRectangle<int32>( x + 2, r.y - 2, 2, 2 ) );
			AddSupport( TRectangle<int32>( x + 1, r.y - 2, 2, 0 ) );
		}

		break;
	}
	case 2:
	{
		int32 l = ( maxl + 4 ) / 8 * 8 - 4;
		int32 ofs = SRand::Inst().Rand( 0, maxl - l + 1 );

		for( int i = 0; i < l; i += 8 )
		{
			int32 x = i + ofs + maxn;
			x = bDir ? x + r.x : r.GetRight() - 4 - x;
			AddThruster( TRectangle<int32>( x, r.y - 2, 2, 2 ) );
			AddThruster( TRectangle<int32>( x + 2, r.y - 2, 2, 2 ) );
			AddThruster( TRectangle<int32>( x + 1, r.y - 4, 2, 2 ) );
			AddSupport( TRectangle<int32>( x + 1, r.y - 4, 2, 0 ) );
		}
		break;
	}
	}
}

void CLevelGenNode2_2_2::GenBigChunk( const TRectangle<int32>& r, int8 nType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( nType == 0 )
	{
		int32 nSubType = SRand::Inst().Rand( 0, 2 );
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			m_gendata[i + r.y * nWidth] = eType_Chunk1;
			m_gendata[i + ( r.GetBottom() - 1 ) * nWidth] = eType_Chunk1;
		}

		uint8 bDir = SRand::Inst().Rand( 0, 2 );
		if( nSubType == 0 )
		{
			int n = SRand::Inst().Rand( 1, 4 );
			int32 h0 = r.height - 2;
			int32 hMin = Max( 1, h0 / 2 );
			int32 hMax = Max( hMin, h0 * 3 / 4 );
			for( ; n < r.width - 1; n += SRand::Inst().Rand( 4, 6 ) )
			{
				int32 x = bDir ? r.x + n : r.GetRight() - 1 - n;
				int32 h = SRand::Inst().Rand( hMin, hMax + 1 );
				int32 y1 = SRand::Inst().Rand( 0, h + 1 );
				for( int y = r.y + 1; y < r.y + y1 + 1; y++ )
					m_gendata[x + y * nWidth] = eType_Chunk1;
				for( int y = r.GetBottom() - 2; y > r.GetBottom() - ( h - y1 ) - 2; y-- )
					m_gendata[x + y * nWidth] = eType_Chunk1;
			}
		}
		else
		{
			uint8 bDir1 = SRand::Inst().Rand( 0, 2 );
			int n = SRand::Inst().Rand( 1, 3 );
			int32 h0 = r.height - 2;
			int32 hMin = Max( 1, h0 * 2 / 3 );
			int32 hMax = Max( hMin, h0 * 3 / 4 );
			for( ; n < r.width - 1; n += SRand::Inst().Rand( 3, 5 ) )
			{
				int32 x = bDir ? r.x + n : r.GetRight() - 1 - n;
				int32 h = SRand::Inst().Rand( hMin, hMax + 1 );
				if( bDir1 )
				{
					for( int y = r.y + 1; y < r.y + h + 1; y++ )
						m_gendata[x + y * nWidth] = eType_Chunk1;
				}
				else
				{
					for( int y = r.GetBottom() - 2; y > r.GetBottom() - h - 2; y-- )
						m_gendata[x + y * nWidth] = eType_Chunk1;
				}
			}

		}
	}
	else
	{
		uint8 bDir = SRand::Inst().Rand( 0, 2 );
		int n = SRand::Inst().Rand( 0, 3 );
		int32 x0 = SRand::Inst().Rand( 0, r.width );
		int32 wMin = Max( 2, r.width / 3 );
		int32 wMax = Max( wMin, r.width / 2 );
		for( ; n < r.height; n += SRand::Inst().Rand( 2, 4 ) )
		{
			int32 y = n + r.y;
			int32 w = SRand::Inst().Rand( wMin, wMax + 1 );
			if( n == 0 || n == r.height - 1 )
				x0 = r.width - ( w + SRand::Inst().Rand( 0, 2 ) ) / 2;
			else
			{
				x0 += SRand::Inst().Rand( wMin, wMax + 1 );
				if( !SRand::Inst().Rand( 0, 3 ) )
					x0 += SRand::Inst().Rand( wMin, wMax + 1 );
			}
			x0 = x0 % r.width;
			for( int i = 0; i < w; i++ )
			{
				int32 x = i + x0;
				if( x >= r.width )
					x -= r.width;
				x = bDir ? r.x + x : r.GetRight() - 1 - x;
				m_gendata[x + y * nWidth] = eType_Chunk1;
			}
		}
	}
}
