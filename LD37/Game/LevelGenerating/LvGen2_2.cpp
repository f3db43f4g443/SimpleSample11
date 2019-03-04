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

void CLevelGenNode2_2_1::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pBillboardNode = CreateNode( pXml->FirstChildElement( "billboard" )->FirstChildElement(), context );
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
	m_pBarrelNode[3] = CreateNode( pXml->FirstChildElement( "barrel" )->FirstChildElement(), context );
	m_pBarrelNode[4] = CreateNode( pXml->FirstChildElement( "barrel1" )->FirstChildElement(), context );
	m_pControlRoomNode = CreateNode( pXml->FirstChildElement( "control_room" )->FirstChildElement(), context );
	m_pFuseNode = CreateNode( pXml->FirstChildElement( "fuse" )->FirstChildElement(), context );
	m_pChainNode = CreateNode( pXml->FirstChildElement( "chain" )->FirstChildElement(), context );
	
	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_2_1::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );
	m_gendata2.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenChunks();
	GenObjs();

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
			else if( genData == eType_Obj )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	for( int i = 0; i < 5; i++ )
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

	context.mapTags["1"] = eType_Control_Room_1;
	context.mapTags["2"] = eType_Control_Room_2;
	context.mapTags["3"] = eType_Control_Room_3;
	context.mapTags["4"] = eType_Control_Room_4;
	context.mapTags["5"] = eType_Control_Room_5;
	for( auto& room : m_vecControlRooms )
	{
		auto rect = room.rect;
		context.mapTags["left"] = room.b[0];
		context.mapTags["top"] = room.b[1];
		context.mapTags["right"] = room.b[2];
		context.mapTags["bottom"] = room.b[3];
		m_pControlRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
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
					m_gendata[i + j * context.nWidth] = context.blueprint[i + region.x + ( j + region.y) * context.nWidth] = eType_Room_1;
			}
		}
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Billboard_1;
	for( auto& rect : m_vecBillboards )
	{
		m_pBillboardNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
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

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_gendata2.clear();
	m_vecRoads.clear();
	m_vecRooms.clear();
	m_vecBillboards.clear();
	m_vecCargos.clear();
	m_vecCargos1.clear();
	m_vecControlRooms.clear();
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
	m_vecBarrel[3].clear();
	m_vecBarrel[4].clear();
	m_vecFuse.clear();
	m_vecChain.clear();
}

void CLevelGenNode2_2_1::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	int8 nDir = SRand::Inst().Rand( 0, 2 );
	vector<TRectangle<int32> > vecTemp1, vecTemp2;
	for( int y = 0; y < nHeight; )
	{
		int32 h = Min( nHeight - y, SRand::Inst().Rand( 6, 9 ) );
		if( nHeight - y - h <= 5 )
			break;
		int32 h1 = Min( nHeight - y, h + SRand::Inst().Rand( 8, 16 ) );
		int32 w = SRand::Inst().Rand( 19, 24 );
		GenTruck1( TRectangle<int32>( nDir ? nWidth - w : 0, y + h1 - h, w, h ), vecTemp1, vecTemp2 );
		y += h1;
		nDir = !nDir;
	}
	SRand::Inst().Shuffle( vecTemp1 );
	SRand::Inst().Shuffle( vecTemp2 );
	for( auto& rect : vecTemp2 )
		GenStack( rect );
	for( auto& rect : vecTemp1 )
		GenTruck2( rect );

	function<void( const TRectangle<int32>& )> Func;
	vector<TVector2<int32> > vec;
	Func = [=, &vec, &Func] ( const TRectangle<int32>& rect ) 
	{
		int32 w = SRand::Inst().Rand( 4, 6 );
		if( rect.height >= 4 || rect.width - w < 8 )
		{
			if( rect.height == 2 )
				AddChunk( rect, eType_Chunk, &m_vecBar2 );
			else if( rect.height >= 4 && rect.width >= 5 && rect.width <= SRand::Inst().Rand( 9, 12 ) )
			{
				AddChunk( rect, 0, &m_vecRooms );
				for( int i = rect.x; i < rect.GetRight(); i += rect.width - 1 )
				{
					for( int j = rect.y; j < rect.GetBottom(); j += rect.height - 1 )
						vec.push_back( TVector2<int32>( i, j ) );
				}
				SRand::Inst().Shuffle( vec );
				for( int i = rect.x + 1; i < rect.GetRight() - 1; i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j += rect.height - 1 )
						vec.push_back( TVector2<int32>( i, j ) );
				}
				SRand::Inst().Shuffle( &vec[4], vec.size() - 4 );
				int32 nMaxWidth = Min( 4, rect.width - 3 );
				for( auto& p : vec )
				{
					if( m_gendata[p.x + p.y * nWidth] )
						continue;
					auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, nMaxWidth + 1 ), 2 ),
						rect, -1, 0 );
					if( r.width )
					{
						auto r1 = TRectangle<int32>( r.x - 1, r.y - 1, r.width + 2, r.height + 2 ) * rect;
						for( int i = r1.x; i < r1.GetRight(); i++ )
						{
							for( int j = r1.y; j < r1.GetBottom(); j++ )
							{
								if( !m_gendata[i + j * nWidth] )
									m_gendata[i + j * nWidth] = eType_Room_2;
							}
						}
						GenCargo( r );
						if( r.width >= 4 )
						{
							int32 x = SRand::Inst().Rand( r.x + 2, r.GetRight() - 2 + 1 );
							for( int y = r.y; y < r.GetBottom(); y++ )
							{
								int8 nType = y == r.y ? ( p.y > rect.y ? eType_Cargo1_2 : eType_Cargo1_3 ) :
									( y == r.GetBottom() - 1 ? ( p.y > rect.y ? eType_Cargo1_3 : eType_Cargo1_2 ) : eType_Cargo1_1 );
								m_gendata[x + y * nWidth] = m_gendata[x - 1 + y * nWidth] = nType;
							}
						}
					}
				}
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = eType_Room_2;
					}
				}

				for( int y = rect.y; y < rect.GetBottom(); y += rect.height - 1 )
				{
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						if( m_gendata[i + y * nWidth] == eType_Room_2 )
						{
							int32 x;
							for( x = i; x < rect.GetRight(); x++ )
							{
								if( m_gendata[x + y * nWidth] != eType_Room_2 )
									break;
							}
							int32 l = x - i;
							if( l <= 2 )
								AddChunk( TRectangle<int32>( i, y, l, 1 ), eType_Room_Door, NULL );
							else if( i > rect.x || x < rect.GetRight() )
							{
								int32 l1 = Min( 2, l - 2 );
								if( ( i > rect.x ) + SRand::Inst().Rand( 0, 2 ) > ( x < rect.GetRight() ) )
								{
									AddChunk( TRectangle<int32>( i, y, l1, 1 ), eType_Room_Door, NULL );
									AddChunk( TRectangle<int32>( i + l1, y, l - l1, 1 ), eType_Room_1, &m_vecBar_a[0] );
								}
								else
								{
									AddChunk( TRectangle<int32>( i, y, l - l1, 1 ), eType_Room_1, &m_vecBar_a[0] );
									AddChunk( TRectangle<int32>( x - l1, y, l1, 1 ), eType_Room_Door, NULL );
								}
							}
							else if( l >= 5 )
							{
								int32 l1 = Min( 2, l - 4 );
								int32 x1 = SRand::Inst().Rand( rect.x + 2, rect.GetRight() - l1 - 2 + 1 );
								AddChunk( TRectangle<int32>( i, y, x1 - i, 1 ), eType_Room_1, &m_vecBar_a[0] );
								AddChunk( TRectangle<int32>( x1, y, l1, 1 ), eType_Room_Door, NULL );
								AddChunk( TRectangle<int32>( x1 + l1, y, x - x1 - l1, 1 ), eType_Room_1, &m_vecBar_a[0] );
							}
						}
					}
				}

				vec.resize( 0 );
			}
			else if( rect.width <= 6 )
				GenCargo( rect );
			else
			{
				int32 w1 = rect.height > 3 ? 0 : SRand::Inst().Rand( Min( 2, rect.width - 6 ), Min( 3, rect.width - 6 ) + 1 );
				w = SRand::Inst().Rand( 3, rect.width - 3 + 1 - w1 );
				if( w1 > 0 )
					AddChunk( TRectangle<int32>( rect.x + w, rect.y, w1, rect.height ), eType_Temp, NULL );
				Func( TRectangle<int32>( rect.x, rect.y, w, rect.height ) );
				Func( TRectangle<int32>( rect.x + w + w1, rect.y, rect.width - w - w1, rect.height ) );
			}
			return;
		}
		int32 w1 = SRand::Inst().Rand( 4, rect.width - w - 4 + 1 );
		Func( TRectangle<int32>( rect.x, rect.y, w1, rect.height ) );
		Func( TRectangle<int32>( rect.x + w + w1, rect.y, rect.width - w - w1, rect.height ) );
		auto r = TRectangle<int32>( rect.x + w1, rect.y, w, rect.height );
		AddChunk( r, eType_Billboard, &m_vecBillboards );
		for( int y = r.y; y < r.GetBottom(); y++ )
			m_gendata[r.x + y * nWidth] = m_gendata[r.GetRight() - 1 + y * nWidth] = eType_Billboard_1;
	};
	vector<TRectangle<int32> > vecTempBillboard;
	int32 yControlRoom = SRand::Inst().Rand( 8, 12 );
	for( int y = 0; y < nHeight; y++ )
	{
		int8 b = SRand::Inst().Rand( 0, 2 );
		for( int8 i = 0; i < 2; i++ )
		{
			bool b0 = !!( i ^ b );
			int32 x = b0 ? 0 : nWidth - 1;
			if( m_gendata[x + y * nWidth] )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 10, 3 ), TVector2<int32>( 10, 3 ), TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, 0 );
			if( !rect.width )
				continue;
			{
				auto rect1 = PutRect( m_gendata, nWidth, nHeight, rect, TVector2<int32>( 12, 8 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, 0, 0 );
				if( rect1.width )
					rect = rect1;
				else
					rect = PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, 0, 0 );
			}
			if( rect.width )
			{
				auto rect0 = rect;
				int8 bRoad = rect.y == 0 ? 1 : 0;
				while( rect.height )
				{
					int32 h = Min( rect.height, bRoad ? SRand::Inst().Rand( 3, 5 ) : SRand::Inst().Rand( 4, 6 ) );
					if( rect.height - h <= 1 )
						h = rect.height;
					TRectangle<int32> r( rect.x, rect.y, rect.width, h );
					rect.SetTop( r.GetBottom() );
					r = PutRect( m_gendata, nWidth, nHeight, r, r.GetSize(), TVector2<int32>( nWidth, r.height ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, 0 );
					if( bRoad )
					{
						if( r.height <= SRand::Inst().Rand( 2, 4 ) )
						{
							if( b0 )
								r.width = rect0.width;
							else
								r.SetLeft( r.GetRight() - rect0.width );
							AddChunk( r, eType_Road, &m_vecRoads );
						}
						else
							AddChunk( r, eType_Billboard, &vecTempBillboard );
					}
					else
					{
						int32 w = Max( rect0.width - SRand::Inst().Rand( 1, 3 ), r.width - SRand::Inst().Rand( 2, 5 ) );
						if( b0 )
							r.width = w;
						else
							r.SetLeft( r.GetRight() - w );
						if( r.height <= 4 )
							Func( r );
						else
						{
							if( r.y >= yControlRoom )
							{
								int32 w = Min( SRand::Inst().Rand( 6, 11 ), r.width - SRand::Inst().Rand( 5, 9 ) );
								if( w >= 4 )
								{
									TRectangle<int32> r1 = r;
									if( b0 )
									{
										r1.width = w;
										r.SetLeft( r1.GetRight() );
									}
									else
									{
										r.width -= w;
										r1.SetLeft( r.GetRight() );
									}

									AddChunk( r1, eType_Control_Room, NULL );
									SControlRoom controlRoom;
									controlRoom.rect = r1;
									controlRoom.b[0] = b0;
									controlRoom.b[1] = false;
									controlRoom.b[2] = !b0;
									controlRoom.b[3] = true;
									m_vecControlRooms.push_back( controlRoom );
									auto rect2 = TRectangle<int32>( b0 ? r1.x : r1.GetRight() - 3, r1.GetBottom() - 3, 3, 3 );
									AddChunk( rect2, eType_Control_Room_5, NULL );
									rect2 = controlRoom.rect;
									rect2 = TRectangle<int32>( b0 ? rect2.x + 1 : rect2.GetRight() - 3, rect2.y + 1, 2, rect2.height - 4 );
									GenLimbs( rect2 );
									int32 i0 = Min( r1.width, SRand::Inst().Rand( 5, 7 ) );
									for( int i = 1; i < i0 - 1; i++ )
									{
										int32 x = b0 ? r1.x + i : r1.GetRight() - 1 - i;
										m_gendata[x + r1.y * nWidth] = eType_Control_Room_3;
									}
									rect2 = TRectangle<int32>( b0 ? r1.x + 3 : r1.GetRight() - i0, r1.y + 1, i0 - 3, r1.height - 2 );
									GenLimbs( rect2 );
									for( int i = i0; i < r1.width - 1; i++ )
									{
										int32 x = b0 ? r1.x + i : r1.GetRight() - 1 - i;
										int32 y0 = SRand::Inst().Rand( r1.y + 1, r1.GetBottom() - 1 );
										for( int y = r1.y; y < r1.GetBottom(); y++ )
											m_gendata[x + y * nWidth] = y == y0 ? eType_Control_Room_2 : eType_Control_Room_1;
										int32 i1 = Min( r1.width, i + SRand::Inst().Rand( 2, 4 ) );
										GenLimbs( TRectangle<int32>( b0 ? i + 1 : i1 + 1, r1.y + 1, i1 - i, r1.height - 2 ) );
										i = i1;
									}
									yControlRoom += SRand::Inst().Rand( 12, 15 );
								}
							}
							Func( r );
						}
					}

					bRoad = !bRoad;
				}
			}
		}
	}
	for( int y = 0; y < nHeight; y++ )
	{
		int8 b = SRand::Inst().Rand( 0, 2 );
		for( int8 i = 0; i < 2; i++ )
		{
			int32 x = ( i ^ b ) ? 0 : nWidth - 1;
			if( m_gendata[x + y * nWidth] )
				continue;
			auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 5, 10 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, 0 );
			if( rect.width > 0 )
			{
				if( rect.width >= 7 )
				{
					rect.width = Min( rect.width - 2, SRand::Inst().Rand( 5, 7 ) );
					if( !( i ^ b ) )
						rect.x = nWidth - rect.width;
					AddChunk( rect, eType_Temp, NULL );
					auto road = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( ( i ^ b ) ? rect.GetRight() : rect.x - 1, rect.y ),
						TVector2<int32>( 4, 8 ), TVector2<int32>( 6, Min( rect.height, SRand::Inst().Rand( 10, 13 ) ) ),
						TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Road );
					if( road.width > 0 )
						m_vecRoads.push_back( road );
					AddChunk( rect, 0, NULL );
					GenTruck2( rect );
				}
			}
		}
	}

	vector<TVector2<int32> > vec1;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, 0, vec1 );
	SRand::Inst().Shuffle( vec1 );
	for( auto& p : vec1 )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 8 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
		if( rect.width <= 0 )
			continue;

		for( int i = 0; i < 2; i++ )
		{
			auto r = TRectangle<int32>( rect.x, i ? rect.y : rect.GetBottom() - 3, rect.width, 3 );
			r = PutRect( m_gendata, nWidth, nHeight, r, r.GetSize(), TVector2<int32>( nWidth, 3 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Billboard, 0 );
			vecTempBillboard.push_back( r );
		}
		AddChunk( TRectangle<int32>( rect.x, rect.y + 3, rect.width, 6 ), eType_Temp1, NULL );
	}
	for( auto& i : m_gendata )
	{
		if( i == eType_Temp1 )
			i = 0;
	}

	for( auto& r : vecTempBillboard )
	{
		m_vecBillboards.push_back( r );
		if( r.height >= 4 )
		{
			for( int y = r.y; y < r.GetBottom(); y += r.height - 1 )
			{
				int8 nDir = SRand::Inst().Rand( 0, 2 );
				int32 y1 = y == r.y ? y - 1 : y + 1;
				int32 i0 = 0;
				for( int i = 0; i <= r.width; i++ )
				{
					int32 x = nDir ? r.x + i : r.GetRight() - 1 - i;
					bool b = false;
					if( y1 >= 0 && y1 < nHeight && i < r.width )
					{
						b = true;
						auto nType = m_gendata[x + y1 * nWidth];
						if( nType == eType_Temp || nType == eType_Room_2 || nType == eType_Room_Door || nType == eType_Billboard
							|| nType == eType_Control_Room_1 || nType == eType_Control_Room_3 || nType == eType_Cargo1_3 )
							b = false;
					}
					if( !b )
					{
						int32 l = i - i0;
						if( l >= 3 )
						{
							int32 l1 = Max( 3, l - SRand::Inst().Rand( 0, 4 ) );
							if( l1 < l )
								i0 += SRand::Inst().Rand( 0, l - l1 + 1 );
							for( int i1 = i0; i1 < i0 + l1; i1++ )
							{
								int32 x = nDir ? r.x + i1 : r.GetRight() - 1 - i1;
								m_gendata[x + y * nWidth] = eType_Billboard_1;
							}
						}
						i0 = i + 1;
					}
				}
			}
		}
		else
		{
			int8 nDir = SRand::Inst().Rand( 0, 2 );
			for( int i = 0; i < r.width; i++ )
			{
				int32 i1 = Min( r.width, i + SRand::Inst().Rand( 4, 8 ) );
				for( ; i < i1; i++ )
				{
					int32 x = nDir ? r.x + i : r.GetRight() - 1 - i;
					m_gendata[x + ( r.y + 1 ) * nWidth] = eType_Billboard_1;
				}
			}
		}
	}
}

void CLevelGenNode2_2_1::GenCargo1Room( const TRectangle<int32>& rect, const TRectangle<int32>& room )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	if( rect.width >= 4 && rect.height >= 4 )
	{
		int8 nType[4] = { 0, 0, 0, 0 };
		for( int i = 0; i < 2; i++ )
		{
			{
				int32 x0 = i == 0 ? rect.x - 1 : rect.GetRight();
				if( x0 >= 0 && x0 < nWidth )
				{
					for( int y = rect.y + 2; y <= rect.GetBottom() - 2; y++ )
					{
						if( m_gendata[x0 + ( y - 1 ) * nWidth] == eType_Room_2 || m_gendata[x0 + y * nWidth] == eType_Room_2 )
							nType[0 + i * 2] |= 1;
						int32 l;
						for( l = 0; l < 2; l++ )
						{
							int32 x = i == 0 ? x0 - l : x0 + l;
							if( x < 0 || x >= nWidth )
								break;
							int8 n = m_gendata[x + ( y - 1 ) * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
							n = m_gendata[x + y * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
						}
						if( l == 2 )
							nType[0 + i * 2] |= 2;
					}
				}
			}
			{
				int32 y0 = i == 0 ? rect.y - 1 : rect.GetBottom();
				if( y0 >= 0 && y0 < nHeight )
				{
					for( int x = rect.x + 2; x <= rect.GetRight() - 2; x++ )
					{
						if( m_gendata[x - 1 + y0 * nWidth] == eType_Room_2 || m_gendata[x + y0 * nWidth] == eType_Room_2 )
							nType[1 + i * 2] |= 1;
						int32 l;
						for( l = 0; l < 2; l++ )
						{
							int32 y = i == 0 ? y0 - l : y0 + l;
							if( y < 0 || y >= nHeight )
								break;
							int8 n = m_gendata[x - 1 + y * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
							n = m_gendata[x + y * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
						}
						if( l == 2 )
							nType[1 + i * 2] |= 2;
					}
				}
			}
		}
		int8 nIndex[4] = { 0, 3, 2, 1 };
		if( rect.x + rect.GetRight() + SRand::Inst().Rand( 0, 2 ) > nWidth )
			swap( nIndex[0], nIndex[2] );
		int8 nBtn = -1;
		for( int i = 0; i < 4; i++ )
		{
			if( !!( nType[nIndex[i]] & 1 ) )
			{
				bool b = false;
				for( int j = 0; j < 4; j++ )
				{
					if( j != i && !!( nType[nIndex[i]] & 2 ) )
					{
						b = true;
						break;
					}
				}
				if( b )
				{
					nBtn = nIndex[i];
					break;
				}
			}
		}
		if( nBtn == -1 )
			return;
		AddChunk( rect, eType_Cargo1_1, NULL );
		for( int i = 0; i < 2; i++ )
		{
			do
			{
				int8 n = -1;
				if( 0 + i * 2 == nBtn )
					n = eType_Cargo1_2;
				else if( !!( nType[0 + i * 2] ^ 2 ) )
					n = eType_Cargo1_3;
				else
					break;
				vector<int32> vec;
				for( int y = rect.y + 2; y <= rect.GetBottom() - 2; y++ )
					vec.push_back( y );
				SRand::Inst().Shuffle( vec );
				int32 x0 = i == 0 ? rect.x - 1 : rect.GetRight();
				int32 x1 = i == 0 ? rect.x : rect.GetRight() - 1;

				for( int j = 0; j <= vec.size(); j++ )
				{
					int32 y = vec[j];
					if( n == eType_Cargo1_2 )
					{
						if( m_gendata[x0 + ( y - 1 ) * nWidth] == eType_Room_2 || m_gendata[x0 + y * nWidth] == eType_Room_2 )
						{
							m_gendata[x1 + ( y - 1 ) * nWidth] = m_gendata[x1 + y * nWidth] = n;
							break;
						}
					}
					else
					{
						int32 l;
						for( l = 0; l < 2; l++ )
						{
							int32 x = i == 0 ? x0 - l : x0 + l;
							if( x < 0 || x >= nWidth )
								break;
							int8 n = m_gendata[x + ( y - 1 ) * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
							n = m_gendata[x + y * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
						}
						if( l == 2 )
						{
							m_gendata[x1 + ( y - 1 ) * nWidth] = m_gendata[x1 + y * nWidth] = n;
							break;
						}
					}
				}
			} while( 0 );
			do
			{
				int8 n = -1;
				if( 1 + i * 2 == nBtn )
					n = eType_Cargo1_2;
				else if( !!( nType[1 + i * 2] ^ 2 ) )
					n = eType_Cargo1_3;
				else
					break;
				vector<int32> vec;
				for( int x = rect.x + 2; x <= rect.GetRight() - 2; x++ )
					vec.push_back( x );
				SRand::Inst().Shuffle( vec );
				int32 y0 = i == 0 ? rect.y - 1 : rect.GetBottom();
				int32 y1 = i == 0 ? rect.y : rect.GetBottom() - 1;

				for( int j = 0; j <= vec.size(); j++ )
				{
					int32 x = vec[j];
					if( n == eType_Cargo1_2 )
					{
						if( m_gendata[x - 1 + y0 * nWidth] == eType_Room_2 || m_gendata[x + y0 * nWidth] == eType_Room_2 )
						{
							m_gendata[x - 1 + y1 * nWidth] = m_gendata[x + y1 * nWidth] = n;
							break;
						}
					}
					else
					{
						int32 l;
						for( l = 0; l < 2; l++ )
						{
							int32 y = i == 0 ? y0 - l : y0 + l;
							if( y < 0 || y >= nWidth )
								break;
							int8 n = m_gendata[x - 1 + y * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
							n = m_gendata[x + y * nWidth];
							if( n > eType_Room_1 && n != eType_Billboard )
								break;
						}
						if( l == 2 )
						{
							m_gendata[x - 1 + y1 * nWidth] = m_gendata[x + y1 * nWidth] = n;
							break;
						}
					}
				}
			} while( 0 );
		}
	}

	bool b = rect.width + SRand::Inst().Rand( 0, 2 ) > rect.height;
	if( room.width )
	{
		if( rect.width == room.width )
			b = true;
		if( rect.height == room.height )
			b = false;
	}
	if( ( b ? rect.width : rect.height ) >= 4 )
	{
		int8 nBtn;
		int32 xBtn, xBtn1, xControl;
#define GET_DATA( x, y ) m_gendata[b ? ( ( y ) + ( x ) * nWidth ) : ( ( x ) + ( y ) * nWidth )]
		if( b )
		{
			nBtn = room.width ? ( rect.GetBottom() < room.GetBottom() ? 0 : 1 ) : SRand::Inst().Rand( 0, 2 );
			xBtn = nBtn ? rect.y : rect.GetBottom() - 1;
			xBtn1 = nBtn ? rect.y - 1 : rect.GetBottom();
			xControl = nBtn ? rect.GetBottom() - 1 : rect.y;
		}
		else
		{
			nBtn = room.width ? ( rect.x + rect.GetRight() + SRand::Inst().Rand( 0, 2 ) > room.x + room.GetRight() ) : SRand::Inst().Rand( 0, 2 );
			xBtn = nBtn ? rect.x : rect.GetRight() - 1;
			xBtn1 = nBtn ? rect.x - 1 : rect.GetRight();
			xControl = nBtn ? rect.GetRight() - 1 : rect.x;
		}
		if( xBtn1 < 0 || xBtn1 >= ( b ? nHeight : nWidth ) )
			return;
		int8 bDir = SRand::Inst().Rand( 0, 2 );
		auto rect1 = rect;
		if( b )
		{
			swap( rect1.x, rect1.y );
			swap( rect1.width, rect1.height );
		}
		if( rect1.width >= 4 )
		{
			vector<int32> v;
			for( int j = rect1.y + 2; j <= rect1.GetBottom() - 2; j++ )
			{
				int32 y = bDir ? j : rect1.GetBottom() + rect1.y - j;
				if( GET_DATA( xBtn1, y - 1 ) != eType_Room_2 && GET_DATA( xBtn1, y ) != eType_Room_2 )
					v.push_back( y );
			}
			if( !v.size() )
				return;
			int32 yBtn = v[SRand::Inst().Rand<int32>( 0, v.size() )];
			v.resize( 0 );

			for( int j = rect1.y + 2; j <= rect1.GetBottom() - 2; j++ )
			{
				int32 y = bDir ? j : rect1.GetBottom() + rect1.y - j;
				bool bOK = true;
				for( int y1 = y - 1; y1 <= y; y1++ )
				{
					int32 l;
					for( l = 0; l < 2; l++ )
					{
						int32 x = nBtn ? rect1.GetRight() + l : rect1.x - 1 - l;
						if( x < 0 || x >= ( b ? nHeight : nWidth ) )
							break;
						auto n = GET_DATA( x, y1 );
						if( n > eType_Room_1 && n != eType_Billboard )
							break;
					}
					if( l < 2 )
					{
						bOK = false;
						break;
					}
				}
				if( bOK )
				{
					GET_DATA( xControl, y - 1 ) = GET_DATA( xControl, y ) = eType_Cargo1_3;
					v.push_back( y );
					j += SRand::Inst().Rand( 1, 3 );
				}
			}
			if( !v.size() )
				return;

			TRectangle<int32> r1( rect1.x, Min( v[0], v.back() ) - 1, rect1.width, Max( v[0], v.back() ) + 2 - Min( v[0], v.back() ) );
			for( int x = r1.x; x < r1.GetRight(); x++ )
			{
				for( int y = r1.y; y < r1.GetBottom(); y++ )
				{
					if( GET_DATA( x, y ) == eType_Cargo1 )
						GET_DATA( x, y ) = eType_Cargo1_1;
				}
			}
			GET_DATA( xBtn, yBtn - 1 ) = GET_DATA( xBtn, yBtn ) = eType_Cargo1_2;
		}
		else
		{
			for( int j = rect1.y + 2; j <= rect1.GetBottom() - 2; j++ )
			{
				int32 y = bDir ? j : rect1.GetBottom() + rect1.y - j;
				if( GET_DATA( xBtn1, y - 1 ) != eType_Room_2 && GET_DATA( xBtn1, y ) != eType_Room_2 )
					continue;
				bool bOK = true;
				for( int y1 = y - 1; y1 <= y; y1++ )
				{
					int32 l;
					for( l = 0; l < 2; l++ )
					{
						int32 x = nBtn ? rect1.GetRight() + l : rect1.x - 1 - l;
						if( x < 0 || x >= ( b ? nHeight : nWidth ) )
							break;
						auto n = GET_DATA( x, y1 );
						if( n > eType_Room_1 && n != eType_Billboard )
							break;
					}
					if( l < 2 )
					{
						bOK = false;
						break;
					}
				}
				if( bOK )
				{
					for( int x = rect1.x; x < rect1.GetRight(); x++ )
					{
						int8 nType = x == xBtn ? eType_Cargo1_2 : ( x == xControl ? eType_Cargo1_3 : eType_Cargo1_1 );
						GET_DATA( x, y - 1 ) = GET_DATA( x, y ) = nType;
					}
					j += SRand::Inst().Rand( 1, 3 );
				}
			}
		}
	}
#undef GET_DATA
}

void CLevelGenNode2_2_1::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( auto& rect : m_vecRoads )
	{
		AddChunk( rect, 0, NULL );
		TVector2<int32> maxSize = rect.GetSize();
		int8 nType = maxSize.x > maxSize.y;
		if( nType )
		{
			maxSize.x = nWidth;
			maxSize.y = Max( 6, maxSize.y );
		}
		else
		{
			maxSize.y = nHeight;
			maxSize.x = Max( 6, maxSize.x );
		}
		rect = PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, 0 );
		AddChunk( rect, eType_Road, NULL );
	}
	for( int i = 0; i < m_gendata.size(); i++ )
	{
		if( m_gendata[i] == eType_Temp )
		{
			m_gendata[i] = 0;
			m_gendata1[i] = 2;
		}
		else
			m_gendata1[i] = m_gendata[i] <= eType_Road ? 0 : 1;
	}

	vector<int32 > vec0;
	for( int i = 0; i < nWidth; i++ )
		vec0.push_back( i );
	for( int j = 0; j < nHeight; j++ )
	{
		SRand::Inst().Shuffle( vec0 );
		for( auto i : vec0 )
		{
			if( m_gendata1[i + j * nWidth] )
				continue;
			if( j == 0 )
			{
				if( m_gendata[i + j * nWidth] == 0 )
				{
					auto rect = PutRectEx( m_gendata, nWidth, nHeight, TVector2<int32>( i, j ), TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 3, 5 ), SRand::Inst().Rand( 3, 5 ) ),
						TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, [this] ( TRectangle<int32> rect, TRectangle<int32> r1 ) { return CheckCargo( rect, r1 ); } );
					if( rect.width <= 0 )
						continue;
					rect.height = Min( rect.height, Max( 3, rect.width - 1 ) );
					GenCargo( rect );
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
							m_gendata1[x + y * nWidth] = 1;
					}
				}
				continue;
			}
			if( m_gendata1[i + ( j - 1 ) * nWidth] != 1 )
				continue;

			TRectangle<int32> rect0;
			int8 nType = m_gendata[i + j * nWidth] ? 0 : SRand::Inst().Rand( 0, 2 );
			for( ; nType >= 0; nType-- )
			{
				TVector2<int32> maxSize;
				if( nType == 0 )
					maxSize = TVector2<int32>( SRand::Inst().Rand( 2, 4 ), SRand::Inst().Rand( 2, 3 ) );
				else
					maxSize = TVector2<int32>( SRand::Inst().Rand( 3, 6 ), SRand::Inst().Rand( 2, 5 ) );
				maxSize.y = Min( maxSize.y, Max( 3, maxSize.x - 1 ) );
				if( nType == 0 )
					rect0 = PutRect( m_gendata1, nWidth, nHeight, TVector2<int32>( i, j ), TVector2<int32>( 2, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), 4, 2 );
				else
					rect0 = PutRectEx( m_gendata, nWidth, nHeight, TVector2<int32>( i, j ), TVector2<int32>( 3, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), 4, 0,
						[this] ( TRectangle<int32> rect, TRectangle<int32> r1 ) { return CheckCargo( rect, r1 ); } );
				if( rect0.width <= 0 )
					continue;
				if( nType == 0 )
					GenCargo0( rect0 );
				else
					GenCargo( rect0 );
				for( int x = rect0.x; x < rect0.GetRight(); x++ )
				{
					for( int y = rect0.y; y < rect0.GetBottom(); y++ )
						m_gendata1[x + y * nWidth] = nType == 0 ? 2 : 1;
				}
				break;
			}
			if( nType < 0 )
				continue;

			while( rect0.GetBottom() < nHeight )
			{
				TRectangle<int32> rect1( rect0.x, rect0.GetBottom(), rect0.width, 1 );
				if( nType == 0 && CheckBar( rect1, rect1 ) )
				{
					auto rect2 = PutRectEx( m_gendata1, nWidth, nHeight, rect1, TVector2<int32>( 4, 1 ), TVector2<int32>( 10, 1 ),
						TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, [this] ( TRectangle<int32> r, TRectangle<int32> r1 ) { return CheckBar( r, r1 ); } );
					if( rect2.width > 0 )
					{
						int32 w = Max( rect1.width, Max( 4, Min( rect2.width - 2, rect1.width * 3 - 1 ) ) );
						rect2.x = SRand::Inst().Rand( Max( rect1.GetRight() - w, rect2.x ), Min( rect1.x, rect2.GetRight() - w ) + 1 );
						rect2.width = w;
						if( GenBar0( rect2 ) )
							break;
					}
				}
				nType = nType == 0 ? SRand::Inst().Rand( 0, 2 ) : 0;
				for( ; nType >= 0; nType-- )
				{
					TVector2<int32> maxSize;
					if( nType == 0 )
						maxSize = TVector2<int32>( SRand::Inst().Rand( 2, 4 ), SRand::Inst().Rand( 2, 3 ) );
					else
						maxSize = TVector2<int32>( SRand::Inst().Rand( 3, 6 ), SRand::Inst().Rand( 2, 5 ) );
					maxSize.y = Min( maxSize.y, Max( 3, maxSize.x - 1 ) );
					TVector2<int32> p( rect1.x + ( rect1.width + SRand::Inst().Rand( 0, 2 ) ) / 2, rect1.y );
					if( nType == 1 && m_gendata[p.x + p.y * nWidth] || m_gendata1[p.x + p.y * nWidth] )
						continue;
					auto rect2 = nType == 0 ?
						PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), 4, 2 ) :
						PutRectEx( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), 4, 0,
							[this] ( TRectangle<int32> rect, TRectangle<int32> r1 ) { return CheckCargo( rect, r1 ); } );
					if( rect2.width <= 0 )
						continue;
					rect0 = rect2;
					if( nType == 0 )
						GenCargo0( rect0 );
					else
						GenCargo( rect0 );
					for( int x = rect0.x; x < rect0.GetRight(); x++ )
					{
						for( int y = rect0.y; y < rect0.GetBottom(); y++ )
							m_gendata1[x + y * nWidth] = nType == 0 ? 2 : 1;
					}
					break;
				}
				if( nType < 0 )
				{
					for( int x = rect0.x; x < rect0.GetRight(); x++ )
					{
						for( int y = rect0.y; y < rect0.GetBottom(); y++ )
							m_gendata1[x + y * nWidth] = 1;
					}
					break;
				}
			}
		}
	}
}

void CLevelGenNode2_2_1::GenTruck1( const TRectangle<int32>& rect, vector<TRectangle<int32> >& vec1, vector<TRectangle<int32> >& vec2 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	AddChunk( TRectangle<int32>( rect.x, rect.y, rect.width, 2 ), eType_Chunk, &m_vecBar2 );
	int8 nDir = rect.x > 0 ? 1 : 0;
	int32 w1 = SRand::Inst().Rand( 0, 3 );
	TRectangle<int32> rect1( rect.x + ( nDir ? w1 : 0 ), rect.y + 2, rect.width - w1, rect.height - 2 );
	TRectangle<int32> rect2 = rect1;
	if( nDir )
	{
		rect2.width = 4;
		rect1.SetLeft( rect2.GetRight() );
	}
	else
	{
		rect1.width -= 4;
		rect2.SetLeft( rect1.GetRight() );
	}
	AddChunk( rect2, eType_Control_Room, NULL );
	SControlRoom controlRoom;
	controlRoom.rect = rect2;
	controlRoom.b[0] = nDir;
	controlRoom.b[1] = false;
	controlRoom.b[2] = !nDir;
	controlRoom.b[3] = true;
	m_vecControlRooms.push_back( controlRoom );
	for( int y = rect2.y; y < rect2.GetBottom() - 1; y++ )
	{
		m_gendata[( nDir ? rect2.GetRight() - 1 : rect2.x ) + y * nWidth] = eType_Control_Room_3;
	}
	rect2 = TRectangle<int32>( nDir ? rect2.x : rect2.GetRight() - 3, rect2.GetBottom() - 3, 3, 3 );
	AddChunk( rect2, eType_Control_Room_5, NULL );
	rect2 = controlRoom.rect;
	rect2 = TRectangle<int32>( rect2.x + 1, rect2.y + 1, rect2.width - 2, rect2.height - 4 );
	GenLimbs( rect2 );

	int32 nRoadWidth = SRand::Inst().Rand( 5, 7 );
	int32 nRoadBegin = SRand::Inst().Rand( 5, Max( 5, rect1.width - 5 - nRoadWidth ) + 1 );
	if( nRoadBegin >= 8 && nRoadBegin <= SRand::Inst().Rand( 12, 14 ) )
		nRoadBegin = SRand::Inst().Rand( 5, 8 );
	TRectangle<int32> road( nDir ? nWidth - nRoadBegin - nRoadWidth : nRoadBegin, rect.y - 1, nRoadWidth, 1 );
	road = PutRect( m_gendata, nWidth, nHeight, road, TVector2<int32>( road.width, 8 ), TVector2<int32>( road.width, SRand::Inst().Rand( 8, 12 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Road, 0 );

	int32 yMax = rect1.GetBottom();
	bool bRoom = true;
	auto Func = [=, &yMax, &bRoom] ( int32 i1, int32 i2, int32 y, int8 nType )
	{
		while( i1 < i2 )
		{
			int32 w = bRoom ? SRand::Inst().Rand( 6, 12 ) : SRand::Inst().Rand( 4, 7 );
			if( nType == 0 )
			{
				if( i2 - i1 - w < 3 )
				{
					w = i2 - i1;
					if( w >= ( bRoom ? 11 : 8 ) )
						w -= 3;
				}
			}
			else
			{
				if( i2 - i1 < 7 )
				{
					bRoom = bRoom || SRand::Inst().Rand( 0, 2 );
					return TRectangle<int32>( nDir ? rect1.x + i1 : rect1.GetRight() - i2, y, i2 - i1, yMax - y );
				}
				w = Min( w, i2 - i1 - 4 );
			}
			TRectangle<int32> rect( nDir ? rect1.x + i1 : rect1.GetRight() - w - i1, y, w, yMax - y );
			if( rect.width <= 3 && nHeight - rect.y >= 4 )
			{
				rect.height = Max( 4, rect.height );
				AddChunk( rect, eType_Control_Room, NULL );
				SControlRoom controlRoom;
				controlRoom.rect = rect;
				controlRoom.b[0] = nDir == 0 && rect.x == rect1.x;
				controlRoom.b[2] = nDir == 1 && rect.GetRight() == rect1.GetRight();
				controlRoom.b[1] = false;
				controlRoom.b[3] = true;
				m_vecControlRooms.push_back( controlRoom );
				if( !controlRoom.b[0] && !controlRoom.b[2] )
				{
					for( int j = SRand::Inst().Rand( 1, 3 ); j <= rect.height - 2; )
					{
						int32 j1 = Min( rect.height - 1, j + SRand::Inst().Rand( 1, 3 ) );
						for( int x = rect.x; x < rect.GetRight(); x++ )
						{
							for( int y = rect.y + j; y < rect.y + j1; y++ )
								m_gendata[x + y * nWidth] = eType_Control_Room_1;
						}
						j = j1 + 1;
					}
				}
				TRectangle<int32> limbs( rect.x, rect.y + 1, rect.width, rect.height - 2 );
				if( controlRoom.b[0] )
					limbs.SetLeft( limbs.x + 1 );
				if( controlRoom.b[2] )
					limbs.width--;
				GenLimbs( limbs );
			}
			else if( bRoom && nHeight - rect.y >= 4 )
			{
				rect.height = Max( 4, rect.height );
				bRoom = SRand::Inst().Rand( 0, 2 );
				AddChunk( rect, eType_Room_2, &m_vecRooms );
				bool bType = y > rect1.y;

				int8 k = bType ? 1 : SRand::Inst().Rand( 0, 2 );
				for( int i = i1; i < i1 + rect.width; )
				{
					int32 h1 = i == 0 ? 2 : SRand::Inst().Rand( 2, rect.height - 2 + 1 );
					if( h1 == 3 && rect.height <= 6 )
						h1 = 2;
					int32 w1;
					if( h1 == 2 )
						w1 = SRand::Inst().Rand( 4, 8 );
					else if( h1 == 3 )
						w1 = SRand::Inst().Rand( 4, 6 );
					else
						w1 = SRand::Inst().Rand( 2, 4 );
					w1 = Min( w1, i1 + rect.width - i );
					if( w1 > i1 + rect.width - i - 2 )
					{
						w1 = i1 + rect.width - i;
						if( h1 == 2 )
						{
							if( w1 >= 8 )
								w1 -= SRand::Inst().Rand( 2, 5 );
						}
					}
					if( w1 < 2 )
						break;

					auto cargo = TRectangle<int32>( nDir ? rect1.x + i : rect1.GetRight() - w1 - i, k ? rect.GetBottom() - h1 : rect.y, w1, h1 );
					GenCargo( cargo );
					if( cargo.width >= 4 )
					{
						bool b = SRand::Inst().Rand( 0, 2 );
						for( int i = SRand::Inst().Rand( 2, Max( 2, Min( cargo.width - 2, 5 ) ) + 1 ); i <= cargo.width - 2; i += SRand::Inst().Rand( 2, 6 ) )
						{
							int32 x = b ? cargo.x + i : cargo.GetRight() - i;
							for( int j = cargo.y; j < cargo.GetBottom(); j++ )
							{
								int8 nType = j == cargo.y ? ( k ? eType_Cargo1_2 : eType_Cargo1_3 ) :
									( j == cargo.GetBottom() - 1 ? ( k ? eType_Cargo1_3 : eType_Cargo1_2 ) : eType_Cargo1_1 );
								m_gendata[x + j * nWidth] = m_gendata[x - 1 + j * nWidth] = nType;
							}
						}
					}

					if( rect.height >= 5 && h1 == 2 && SRand::Inst().Rand( 0, 2 ) )
						i += w1;
					else
					{
						i += w1 + SRand::Inst().Rand( 1, 3 );
						if( !bType && SRand::Inst().Rand( 0, 2 ) )
							k = !k;
						else
							i++;
					}
				}

				for( k = 0; k < 2; k++ )
				{
					if( k == 0 && bType )
						continue;
					int32 j = k ? rect.GetBottom() - 1 : rect.y;
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						if( m_gendata[i + j * nWidth] == eType_Room_2 )
						{
							int32 x;
							for( x = i; x < rect.GetRight(); x++ )
							{
								if( m_gendata[x + j * nWidth] != eType_Room_2 )
									break;
							}
							AddChunk( TRectangle<int32>( i, j, x - i, 1 ), eType_Room_1, &m_vecBar_a[0] );
						}
					}
				}
			}
			else
			{
				if( y > rect1.y )
					rect.height = Max( 4, rect.height );
				else if( rect.GetBottom() < SRand::Inst().Rand( y + 2, rect1.GetBottom() + 4 ) )
					rect.height++;
				rect.SetBottom( Min( rect.GetBottom(), nHeight ) );
				AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 1, rect.width, 1 ), eType_Obj1, &m_vecBar[0] );
				if( rect.width >= SRand::Inst().Rand( 4, 6 ) )
				{
					int32 x = SRand::Inst().Rand( rect.x, rect.GetRight() );
					AddChunk( TRectangle<int32>( x, rect.y, 1, rect.height - 1 ), eType_Obj1, &m_vecBar[1] );
				}
				GenCargo( rect );
			}

			yMax = rect.GetBottom();
			i1 += w;
		}
		return TRectangle<int32>( 0, 0, 0, 0 );
	};

	if( road.width >= 0 )
	{
		TRectangle<int32> cargo1( Max( rect1.x, road.x ), rect1.y, Min( road.GetRight(), rect1.GetRight() ) - Max( rect1.x, road.x ), 1 );
		if( cargo1.width >= 4 )
		{
			int8 k = SRand::Inst().Rand( 0, 2 );
			for( int k1 = 0; k1 < 2; k1++ )
			{
				if( k ^ k1 )
				{
					int8 nDir1 = SRand::Inst().Rand( 0, 2 );
					if( nDir ^ nDir1 )
					{
						cargo1.SetLeft( rect1.x );
						cargo1.SetRight( Max( road.x + 4, Min( SRand::Inst().Rand( 7, 10 ), cargo1.width ) + cargo1.x ) );
					}
					else
					{
						cargo1.SetRight( rect1.GetRight() );
						cargo1.SetLeft( Min( road.GetRight() - 4, cargo1.GetRight() - Min( SRand::Inst().Rand( 7, 10 ), cargo1.width ) ) );
					}
					int32 nLeft = Max( cargo1.x, road.x );
					int32 nRight = Min( cargo1.GetRight(), road.GetRight() );
					if( rect1.height > 4 && SRand::Inst().Rand( 0, 2 ) )
					{
						TRectangle<int32> obj( SRand::Inst().Rand( nLeft, nRight - 2 + 1 ), cargo1.y, 2, 3 );
						cargo1.height = 3;
						AddChunk( obj, eType_Obj1, &m_vecBarrel[3] );
					}
					else
					{
						TRectangle<int32> obj( SRand::Inst().Rand( nLeft, nRight - 4 + 1 ), cargo1.y, 4, 2 );
						cargo1.height = 2;
						AddChunk( obj, eType_Obj1, &m_vecBarrel[4] );
					}
					GenCargo( cargo1 );
					if( nDir1 )
					{
						Func( 0, rect1.width - cargo1.width, rect1.y, 0 );
						Func( rect1.width - cargo1.width, rect1.width, cargo1.GetBottom(), 0 );
					}
					else
					{
						Func( 0, cargo1.width, cargo1.GetBottom(), 0 );
						Func( cargo1.width, rect1.width, rect1.y, 0 );
					}
				}
				else
				{
					int32 n = nDir ? cargo1.GetRight() - rect1.x : rect1.GetRight() - cargo1.x;
					int32 n1 = n - road.width;
					n = Min( rect.width - 4, Max( n1 + 4, n + SRand::Inst().Rand( -2, 3 ) ) );
					cargo1 = Func( 0, n, rect1.y, 1 );
					int32 nLeft = Max( cargo1.x, road.x );
					int32 nRight = Min( cargo1.GetRight(), road.GetRight() );
					if( nRight - nLeft < 4 || SRand::Inst().Rand( 0, 2 ) )
					{
						TRectangle<int32> obj( SRand::Inst().Rand( nLeft, nRight - 2 + 1 ), cargo1.y, 2, 3 );
						AddChunk( obj, eType_Obj1, &m_vecBarrel[3] );
					}
					else
					{
						TRectangle<int32> obj( SRand::Inst().Rand( nLeft, nRight - 4 + 1 ), cargo1.y, 4, 2 );
						AddChunk( obj, eType_Obj1, &m_vecBarrel[4] );
					}
					GenCargo( cargo1 );
					Func( n, rect1.width, rect1.y, 0 );
				}
				break;
			}

			vector<int32> vecTemp;
			for( int i = cargo1.x; i < cargo1.GetRight(); i++ )
			{
				if( m_gendata[i + cargo1.y * nWidth] == eType_Cargo1 )
					vecTemp.push_back( i );
			}
			if( vecTemp.size() )
			{
				int32 a = vecTemp[SRand::Inst().Rand<int32>( 0, vecTemp.size() )];
				m_vecFuse.push_back( TVector2<int32>( a, cargo1.y ) );
				m_vecFuse.push_back( TVector2<int32>( a, cargo1.y - 1 ) );
			}
		}
		else
			Func( 0, rect1.width, rect1.y, 0 );
	}
	else
		Func( 0, rect1.width, rect1.y, 0 );

	if( road.width > 0 )
	{
		int32 nMaxHeight = Max( road.height, SRand::Inst().Rand( 12, 16 ) );
		TRectangle<int32> r[2] = { { rect.x, road.GetBottom() - 1, road.x - rect.x, 1 }, { road.GetRight(), road.GetBottom() - 1, rect.GetRight() - road.GetRight(), 1 } };
		if( nDir )
			swap( r[0], r[1] );
		int32 w = SRand::Inst().Rand( 5, 8 );
		if( r[1].width > w )
		{
			int32 w0 = SRand::Inst().Rand( 13, 15 );
			if( r[1].width < w0 && r[1].width < SRand::Inst().Rand( w0, w ) )
			{
				auto r1 = r[1];
				if( nDir )
					r1.SetRight( nWidth );
				else
					r1.SetLeft( 0 );
			}
			if( r[1].width < w0 )
			{
				if( nDir )
					r[1].x += r[1].width - w;
				r[1].width = w;
			}
		}

		for( int k = 0; k < 2; k++ )
		{
			auto& r1 = r[k];
			if( r1.width >= 8 )
			{
				vec2.push_back( TRectangle<int32>( r1.x, r1.y, r1.width, 1 ) );
				continue;
			}
			r1 = PutRect( m_gendata, nWidth, nHeight, r1, TVector2<int32>( r1.width, 8 ), TVector2<int32>( r1.width, 24 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, 0 );
			if( r1.width )
			{
				if( r1.height >= nMaxHeight + 5 )
					r1.SetTop( r1.GetBottom() - nMaxHeight );
				else if( r1.height >= 17 )
					r1.SetTop( r1.GetBottom() - SRand::Inst().Rand( 10, r1.height - 5 ) );
				vec1.push_back( r1 );
			}
		}
		m_vecRoads.push_back( road );
	}
}

void CLevelGenNode2_2_1::GenTruck2( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	TRectangle<int32> rect1 = rect;
	rect1.height -= rect.height >= SRand::Inst().Rand( 11, 15 ) ? 4 : 3;
	TRectangle<int32> rect2( rect.x, rect1.GetBottom(), rect.width, rect.GetBottom() - rect1.GetBottom() );
	AddChunk( rect2, eType_Control_Room, NULL );
	SControlRoom controlRoom;
	controlRoom.rect = rect2;
	controlRoom.b[0] = controlRoom.b[2] = controlRoom.b[3] = true;
	controlRoom.b[1] = false;
	m_vecControlRooms.push_back( controlRoom );
	for( int y = rect2.y; y < rect2.GetBottom(); y++ )
	{
		for( int x = rect2.x; x < rect2.GetRight(); x += rect2.width - 1 )
			m_gendata[x + y * nWidth] = eType_Control_Room_4;
	}
	for( int x = rect2.x; x < rect2.GetRight(); x++ )
		m_gendata[x + ( rect2.GetBottom() - 1 ) * nWidth] = eType_Control_Room_4;
	for( int x = rect2.x + 1; x < rect2.GetRight() - 1; x++ )
		m_gendata[x + rect2.y * nWidth] = eType_Control_Room_3;
	GenLimbs( TRectangle<int32>( rect2.x + 1, rect2.y + 1, rect2.width - 2, rect2.height - 2 ) );

	auto rect0 = rect1;
	bool bPass[2] = { false, false };
	if( rect1.height >= SRand::Inst().Rand( 9, 12 ) && SRand::Inst().Rand( 0, 3 ) )
	{
		if( rect1.y > 0 )
			bPass[0] = true;
		rect2 = TRectangle<int32>( rect1.x, rect1.y, rect1.width, rect1.height >= SRand::Inst().Rand( 10, 12 ) ? SRand::Inst().Rand( 3, 5 ) : 3 );
		AddChunk( rect2, eType_Control_Room, NULL );
		controlRoom.rect = rect2;
		controlRoom.b[0] = controlRoom.b[1] = controlRoom.b[2] = true;
		controlRoom.b[3] = false;
		m_vecControlRooms.push_back( controlRoom );
		int32 x1 = rect2.x + ( rect2.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		int32 y1 = rect2.y + 1;
		int32 y2 = y1 + ( rect2.height == 4 ? SRand::Inst().Rand( 1, 3 ) : 1 );
		for( int y = y1; y < y2; y++ )
		{
			for( int x = rect.x; x < rect.GetRight(); x++ )
				m_gendata[x + y * nWidth] = x == x1 - 1 || x == x1 ? eType_Control_Room_2 : eType_Control_Room_1;
		}
		for( int y = y2; y < rect2.GetBottom(); y++ )
			m_gendata[x1 - 1 + y * nWidth] = m_gendata[x1 + y * nWidth] = eType_Control_Room_1;
		GenLimbs( TRectangle<int32>( rect2.x + 1, rect2.y + 1, rect2.width - 2, rect2.height - 1 ) );
		rect1.SetTop( rect2.GetBottom() );
	}
	if( rect1.height >= SRand::Inst().Rand( 9, 12 ) && !SRand::Inst().Rand( 0, 3 ) )
	{
		bPass[1] = !bPass[0] || rect1.height >= 11 && SRand::Inst().Rand( 0, 2 );
		rect2 = TRectangle<int32>( rect1.x, rect1.y, rect1.width, ( rect1.height >= SRand::Inst().Rand( 10, 12 ) ? SRand::Inst().Rand( 2, 4 ) : 2 ) );
		if( bPass[1] )
			rect2.height++;
		rect2.y = rect1.GetBottom() - rect2.height;

		AddChunk( rect2, eType_Control_Room, NULL );
		controlRoom.rect = rect2;
		controlRoom.b[0] = controlRoom.b[2] = true;
		controlRoom.b[1] = controlRoom.b[3] = false;
		m_vecControlRooms.push_back( controlRoom );
		int32 x1 = rect2.x + ( rect2.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		for( int y = rect2.y; y < rect2.GetBottom(); y++ )
			m_gendata[x1 - 1 + y * nWidth] = m_gendata[x1 + y * nWidth] = eType_Control_Room_1;
		if( bPass[1] )
		{
			int32 y1 = rect2.y + ( rect2.height == 4 ? SRand::Inst().Rand( 1, 3 ) : 1 );
			int32 y2 = Min( rect2.GetBottom() - 1, y1 + SRand::Inst().Rand( 1, 3 ) );
			for( int y = y1; y < y2; y++ )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
					m_gendata[x + y * nWidth] = x == x1 - 1 || x == x1 ? eType_Control_Room_2 : eType_Control_Room_1;
			}
		}
		GenLimbs( TRectangle<int32>( rect2.x + 1, rect2.y, rect2.width - 2, rect2.height ) );
		rect1.SetBottom( rect2.y );
	}

	vector<TRectangle<int32> > vec;
	auto Func = [=, &vec] ( const TRectangle<int32>& rect, bool bConn )
	{
		AddChunk( rect, eType_Room_2, &m_vecRooms );
		bool b = false;
		for( int y = 0; y < rect.height; )
		{
			TRectangle<int32> r;
			int32 h = y == 0 ? Min( SRand::Inst().Rand( 2, 6 ), rect.height - y - 3 ) : Min( SRand::Inst().Rand( 2, 5 ), rect.height - y );
			if( y + rect.y == rect0.y )
			{
				if( y > 0 )
					h = rect.height - y;
				int32 w = h >= 3 ? 2 : Min( SRand::Inst().Rand( 7 - Max( h, 3 ), Max( 7 - Max( h, 3 ), rect.width - 2 ) + 1 ), rect.width );
				if( w == rect.width - 1 )
					w = rect.width - 2;
				int32 h1 = SRand::Inst().Rand( 2, Max( 3, h / 2 ) );
				if( h1 >= h )
					w = rect.width;
				vec.push_back( TRectangle<int32>( b ? rect.width - w : 0, y, w, h ) );
				if( w < rect.width )
					vec.push_back( TRectangle<int32>( b ? 0 : w, y == 0 ? 0 : rect.height - h1, rect.width - w, h1 ) );
				if( y > 0 )
					break;
			}
			else
			{
				if( y + h >= rect.height - 2 )
					h = rect.height - y;
				int32 w = SRand::Inst().Rand( ( rect.width + 1 ) / 2, rect.width );
				if( h <= 3 )
					w = Max( w, 4 );
				if( y == 0 || y + h == rect.height )
					w = 2;
				else if( bConn )
					w = Min( w, rect.width - 2 );
				vec.push_back( TRectangle<int32>( b ? rect.width - w : 0, y, w, h ) );
			}
			y = Max( y + h, Min( y + h + SRand::Inst().Rand( bConn ? 1 : 2, 4 ), rect.height - 2 ) );
			b = !b;
		}
		int8 bFlipX = SRand::Inst().Rand( 0, 2 );
		for( auto& r : vec )
		{
			r.x = bFlipX ? rect.GetRight() - r.x - r.width : r.x + rect.x;
			r.y = r.y + rect.y;
			GenCargo( r );
			for( int x = r.x; x < r.GetRight(); x++ )
			{
				for( int y = r.y; y < r.GetBottom(); y++ )
				{
					m_gendata[x + y * nWidth] = eType_Room_1;
				}
			}
		}

		for( int k = 0; k < 2; k++ )
		{
			int32 x = k == 0 ? rect.x : rect.GetRight() - 1;
			int32 x1 = k == 0 ? rect.x + 1 : rect.GetRight() - 2;
			int32 yBegin = rect.y;
			bool bBar = true;
			for( int y = rect.y; y <= rect.GetBottom(); y++ )
			{
				if( y == rect.GetBottom() || m_gendata[x + y * nWidth] == eType_Room_1 )
				{
					if( y > yBegin && bBar )
					{
						if( bConn )
						{
							int32 nDoorBegin, nDoorEnd;
							if( yBegin == rect.y && y == rect.GetBottom() )
							{
								if( y - yBegin < 5 )
									nDoorBegin = nDoorEnd = y;
								else
								{
									int32 lDoor = y - yBegin == 5 ? 1 : 2;
									nDoorBegin = ( y + yBegin - lDoor + SRand::Inst().Rand( 0, 2 ) ) / 2;
									nDoorEnd = nDoorBegin + lDoor;
								}
							}
							else
							{
								if( y - yBegin <= 2 )
								{
									nDoorBegin = yBegin;
									nDoorEnd = y;
								}
								else if( y - yBegin <= 5 )
								{
									int32 lDoor = y - yBegin == 3 ? 1 : 2;
									if( SRand::Inst().Rand( 0, 2 ) )
									{
										nDoorBegin = yBegin;
										nDoorEnd = yBegin + lDoor;
									}
									else
									{
										nDoorBegin = y - lDoor;
										nDoorEnd = y;
									}
								}
								else
								{
									int32 nDoor = SRand::Inst().Rand( yBegin + 3, y - 3 + 1 );
									nDoorBegin = nDoor - 1;
									nDoorEnd = nDoor + 1;
								}

								if( nDoorEnd > nDoorBegin )
								{
									if( nDoorBegin == rect.y )
									{
										if( nDoorEnd == y )
											nDoorBegin = nDoorEnd = y;
										else
										{
											nDoorBegin = y - ( nDoorEnd - nDoorBegin );
											nDoorEnd = y;
										}
									}
									if( nDoorEnd == rect.GetBottom() )
									{
										if( nDoorBegin == yBegin )
											nDoorBegin = nDoorEnd = y;
										else
										{
											nDoorEnd = yBegin + ( nDoorEnd - nDoorBegin );
											nDoorBegin = yBegin;
										}
									}
								}
							}

							for( int y1 = yBegin; y1 < y; y1++ )
								m_gendata[x + y1 * nWidth] = y1 >= nDoorBegin && y1 < nDoorEnd ? eType_Room_Door : eType_Room_1;
							if( nDoorBegin > yBegin )
								m_vecBar_a[1].push_back( TRectangle<int32>( x, yBegin, 1, nDoorBegin - yBegin ) );
							if( nDoorEnd < y )
								m_vecBar_a[1].push_back( TRectangle<int32>( x, nDoorEnd, 1, y - nDoorEnd ) );
						}
						else if( y >= yBegin + 2 )
							AddChunk( TRectangle<int32>( x, yBegin, 1, y - yBegin ), eType_Room_1, &m_vecBar_a[1] );
					}
					yBegin = y + 1;
					bBar = true;
				}
				else if( m_gendata[x1 + y * nWidth] == eType_Room_1 )
					bBar = false;
			}
		}
		for( auto& r : vec )
		{
			for( int x = r.x; x < r.GetRight(); x++ )
			{
				for( int y = r.y; y < r.GetBottom(); y++ )
				{
					m_gendata[x + y * nWidth] = eType_Cargo1;
				}
			}
			GenCargo1Room( r, rect );
		}
		vec.resize( 0 );
	};

	if( rect1.height >= SRand::Inst().Rand( 12, 14 ) )
	{
		int32 h1 = rect1.height >= SRand::Inst().Rand( 13, 15 ) ? SRand::Inst().Rand( 2, 4 ) : 2;
		bool bPass0 = !bPass[0] && !bPass[1] && SRand::Inst().Rand( 0, 2 );
		if( bPass0 )
			h1++;
		int32 y1 = rect1.y + ( rect1.height - h1 + SRand::Inst().Rand( 0, 2 ) ) / 2;
		rect2 = TRectangle<int32>( rect1.x, y1, rect1.width, h1 );
		AddChunk( rect2, eType_Control_Room, NULL );
		controlRoom.rect = rect2;
		controlRoom.b[0] = controlRoom.b[2] = true;
		controlRoom.b[1] = controlRoom.b[3] = false;
		m_vecControlRooms.push_back( controlRoom );
		int32 x1 = rect2.x + ( rect2.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		for( int y = rect2.y; y < rect2.GetBottom(); y++ )
			m_gendata[x1 - 1 + y * nWidth] = m_gendata[x1 + y * nWidth] = eType_Control_Room_1;
		if( bPass0 )
		{
			int32 y1 = rect2.y + ( rect2.height == 4 ? SRand::Inst().Rand( 1, 3 ) : 1 );
			int32 y2 = Min( rect2.GetBottom() - 1, y1 + SRand::Inst().Rand( 1, 3 ) );
			for( int y = y1; y < y2; y++ )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
					m_gendata[x + y * nWidth] = x == x1 - 1 || x == x1 ? eType_Control_Room_2 : eType_Control_Room_1;
			}
		}
		GenLimbs( TRectangle<int32>( rect2.x + 1, rect2.y, rect2.width - 2, rect2.height ) );
		Func( TRectangle<int32>( rect1.x, rect1.y, rect1.width, y1 - rect1.y ), !bPass[0] );
		Func( TRectangle<int32>( rect1.x, y1 + h1, rect1.width, rect1.GetBottom() - y1 - h1 ), !bPass[1] );
	}
	else
	{
		Func( rect1, !bPass[0] && !bPass[1] );
	}
}

void CLevelGenNode2_2_1::GenStack( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto maxRect = PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), TVector2<int32>( rect.width, 20 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, 0 );
	if( maxRect.height >= 4 )
	{
		int32 h = SRand::Inst().Rand( 16, 19 );
		if( h > maxRect.height - 1 )
			h = maxRect.height;
		maxRect.SetTop( maxRect.GetBottom() - h );
		auto rect1 = maxRect;

		int8 nControlRoom = 0;
		int8 nBillboard = 0;

		vector<int8> vecTemp;
		vecTemp.resize( maxRect.width * maxRect.height );
		vector<TRectangle<int32> > vecTempRoom;
		vector<TRectangle<int32> > vecTempBillboard;
		int32 nLastCargo = m_vecCargos1.size();
		while( rect1.height )
		{
			TRectangle<int32> r = rect1;
			if( rect1.height <= 6 )
				rect1.height = 0;
			else if( rect1.height <= 8 )
			{
				TRectangle<int32> bar = r;
				if( SRand::Inst().Rand( 0, 2 ) )
				{
					bar.SetTop( bar.GetBottom() - 2 );
					r.SetBottom( bar.y );
				}
				else
				{
					bar.height = 2;
					r.SetTop( bar.GetBottom() );
				}
				AddChunk( bar, eType_Chunk, &m_vecBar2 );
				rect1.height = 0;
			}
			else if( rect1.height <= SRand::Inst().Rand( 11, 13 ) )
			{
				r.SetTop( r.GetBottom() - ( r.height + SRand::Inst().Rand( 0, 2 ) ) / 2 );
				rect1.SetBottom( r.y );
			}
			else
			{
				r.SetTop( rect1.GetBottom() - SRand::Inst().Rand( 5, 7 ) );
				rect1.SetBottom( r.y );
			}

			if( !nControlRoom )
				nControlRoom = SRand::Inst().Rand( -1, 2 );
			else
				nControlRoom = 0;

			bool bDir;
			if( nControlRoom )
			{
				TRectangle<int32> r1 = r;
				if( nControlRoom == -1 )
				{
					bDir = 1;
					r1.width = 4;
					r.SetLeft( r1.GetRight() );
				}
				else
				{
					bDir = 0;
					r.width -= 4;
					r1.SetLeft( r.GetRight() );
				}

				AddChunk( r1, eType_Control_Room, NULL );
				SControlRoom controlRoom;
				controlRoom.rect = r1;
				controlRoom.b[0] = bDir;
				controlRoom.b[1] = false;
				controlRoom.b[2] = !bDir;
				controlRoom.b[3] = true;
				m_vecControlRooms.push_back( controlRoom );
				for( int y = r1.y; y < r1.GetBottom() - 1; y++ )
				{
					m_gendata[( bDir ? r1.GetRight() - 1 : r1.x ) + y * nWidth] = eType_Control_Room_3;
				}
				r1 = TRectangle<int32>( bDir ? r1.x : r1.GetRight() - 3, r1.GetBottom() - 3, 3, 3 );
				AddChunk( r1, eType_Control_Room_5, NULL );
				r1 = controlRoom.rect;
				r1 = TRectangle<int32>( r1.x + 1, r1.y + 1, r1.width - 2, r1.height - 4 );
				GenLimbs( r1 );
				r1 = controlRoom.rect.Offset( TVector2<int32>( -maxRect.x, -maxRect.y ) );
				for( int i = r1.x; i < r1.GetRight(); i++ )
				{
					for( int j = r1.y; j < r1.GetBottom(); j++ )
						vecTemp[i + j * maxRect.width] = 2;
				}
				for( int y = r1.y + 1; y < r1.GetBottom() - 1; y++ )
					vecTemp[( bDir ? r1.GetRight() - 1 : r1.x ) + y * maxRect.width] = 0;
				vecTemp[( bDir ? r1.GetRight() - 1 : r1.x ) + SRand::Inst().Rand( r1.y + 1, r1.GetBottom() - 1 ) * maxRect.width] = 1;
			}
			else
				bDir = SRand::Inst().Rand( 0, 2 );
			while( r.width )
			{
				int32 w = nBillboard ? SRand::Inst().Rand( 9, 15 ) : SRand::Inst().Rand( 6, 9 );
				if( w >= r.width - ( nBillboard ? 7 : 5 ) )
					w = r.width;
				TRectangle<int32> r1 = r;
				if( bDir )
				{
					r1.width = w;
					r.SetLeft( r1.GetRight() );
				}
				else
				{
					r.width -= w;
					r1.SetLeft( r.GetRight() );
				}
				if( r1.width >= ( nBillboard ? SRand::Inst().Rand( 13, 16 ) : SRand::Inst().Rand( 9, 11 ) ) )
				{
					bool bBillboard = nBillboard || nControlRoom;
					w = bBillboard ? 3 : Min( SRand::Inst().Rand( 4, 7 ), r1.width - 6 );
					auto r2 = r1;
					if( bDir )
					{
						r2.width = w;
						r1.SetLeft( r2.GetRight() );
					}
					else
					{
						r1.width -= w;
						r2.SetLeft( r1.GetRight() );
					}
					if( bBillboard )
					{
						AddChunk( r2, eType_Control_Room, NULL );
						SControlRoom controlRoom;
						controlRoom.rect = r2;
						controlRoom.b[0] = r2.x == rect1.x;
						controlRoom.b[1] = !nControlRoom;
						controlRoom.b[2] = r2.GetRight() == rect1.GetRight();
						controlRoom.b[3] = true;
						m_vecControlRooms.push_back( controlRoom );
						int32 y = r2.y + ( r2.height + SRand::Inst().Rand( 0, 2 ) ) / 2;
						int32 x1 = r2.x + ( r2.width - 1 + SRand::Inst().Rand( 0, 2 ) ) / 2;
						for( int x = r2.x; x < r2.GetRight(); x++ )
						{
							m_gendata[x + ( y - 1 ) * nWidth] = m_gendata[x + y * nWidth] = x == x1 ? eType_Control_Room_2 : eType_Control_Room_1;
						}
						TRectangle<int32> r3( r2.x, r2.y + 1, r2.width, r2.GetBottom() - 2 );
						if( controlRoom.b[1] )
							r3.SetLeft( r3.x + 1 );
						if( controlRoom.b[3] )
							r3.width--;
						GenLimbs( r3 );
						r2 = controlRoom.rect.Offset( TVector2<int32>( -maxRect.x, -maxRect.y ) );
						for( int i = r2.x; i < r2.GetRight(); i++ )
						{
							for( int j = r2.y; j < r2.GetBottom(); j++ )
								vecTemp[i + j * maxRect.width] = 2;
						}
						y -= maxRect.y;
						for( int x = r2.x; x < r2.GetRight(); x++ )
						{
							vecTemp[x + ( y - 1 ) * maxRect.width] = vecTemp[x + y * maxRect.width] = 0;
						}
					}
					else
					{
						GenCargo( r2 );
						r2 = r2.Offset( TVector2<int32>( -maxRect.x, -maxRect.y ) );
						for( int i = r2.x; i < r2.GetRight(); i++ )
						{
							for( int j = r2.y; j < r2.GetBottom(); j++ )
								vecTemp[i + j * maxRect.width] = 2;
						}
					}
				}
				if( nBillboard )
					AddChunk( r1, eType_Billboard, &vecTempBillboard );
				else
					AddChunk( r1, eType_Room_2, &vecTempRoom );
			}

			nBillboard = !nBillboard;
		}
		for( auto& r : vecTempBillboard )
		{
			m_vecBillboards.push_back( r );
			auto r1 = r.Offset( TVector2<int32>( -maxRect.x, -maxRect.y ) );
			int32 n = Max( 1, SRand::Inst().Rand( r.width * r.height / 12, r.width * r.height / 10 + 1 ) );
			for( int i = 0; i < n; i++ )
			{
				vecTemp[SRand::Inst().Rand( r1.x, r1.GetRight() ) + SRand::Inst().Rand( r1.y, r1.GetBottom() ) * maxRect.width] = 1;
			}
			if( r.y == maxRect.y )
				vecTemp[SRand::Inst().Rand( r1.x, r1.GetRight() ) + r1.y * maxRect.width] = 1;
			if( r.GetBottom() == maxRect.GetBottom() )
				vecTemp[SRand::Inst().Rand( r1.x, r1.GetRight() ) + ( r1.GetBottom() - 1 ) * maxRect.width] = 1;
		}
		for( auto& r : vecTempRoom )
		{
			m_vecRooms.push_back( r );
			auto r1 = r.Offset( TVector2<int32>( -maxRect.x, -maxRect.y ) );
			vecTemp[r1.x + r1.y * maxRect.width] = vecTemp[r1.x + 1 + r1.y * maxRect.width]
				= vecTemp[r1.GetRight() - 1 + r1.y * maxRect.width] = vecTemp[r1.GetRight() - 2 + r1.y * maxRect.width]
				= vecTemp[r1.x + ( r1.GetBottom() - 1 ) * maxRect.width] = vecTemp[r1.x + 1 + ( r1.GetBottom() - 1 ) * maxRect.width]
				= vecTemp[r1.GetRight() - 1 + ( r1.GetBottom() - 1 ) * maxRect.width] = vecTemp[r1.GetRight() - 2 + ( r1.GetBottom() - 1 ) * maxRect.width] = 2;
			if( r.y == maxRect.y )
				vecTemp[SRand::Inst().Rand( r1.x + 2, r1.GetRight() - 2 ) + r1.y * maxRect.width] = 1;
			if( r.GetBottom() == maxRect.GetBottom() )
				vecTemp[SRand::Inst().Rand( r1.x + 2, r1.GetRight() - 2 ) + ( r1.GetBottom() - 1 ) * maxRect.width] = 1;
			int32 l1 = ( Min( r1.width, r1.height ) - 1 ) / 2;
			r1 = TRectangle<int32>( r1.x + l1, r1.y + l1, r1.width - l1 * 2, r1.height - l1 * 2 );
			for( int i = 0; i < 2; i++ )
			{
				vecTemp[SRand::Inst().Rand( r1.x, r1.GetRight() ) + SRand::Inst().Rand( r1.y, r1.GetBottom() ) * maxRect.width] = 1;
			}
		}
		ConnectAll( vecTemp, maxRect.width, maxRect.height, 1, 0 );

		for( auto r : vecTempBillboard )
		{
			bool bVertical = SRand::Inst().Rand( 0, 2 );
			while( 1 )
			{
				auto r0 = r;
				auto r1 = r.Offset( TVector2<int32>( -maxRect.x, -maxRect.y ) );
				if( !bVertical )
				{
					for( int j = 0; j < r.height; j += Max( 2, r.height - 1 ) )
					{
						int32 y = j + r.y;
						int32 i0 = 0;
						bool b = false;
						for( int i = 0; i <= r.width; i++ )
						{
							if( i == r.width || vecTemp[r1.x + i + ( r1.y + j ) * maxRect.width] )
							{
								if( i - i0 >= SRand::Inst().Rand( 2, 4 ) )
								{
									int32 l = Min( Min( i - i0, SRand::Inst().Rand( 6, 8 ) ), Max( 3, SRand::Inst().Rand( ( i - i0 ) / 2, i - i0 + 1 ) ) );
									int32 xBegin = r.x + i0 + SRand::Inst().Rand( 0, i - i0 - l + 1 );
									int32 xEnd = xBegin + l;
									for( int x = xBegin; x < xEnd; x++ )
										m_gendata[x + y * nWidth] = eType_Billboard_1;
									b = true;
								}
								i0 = i + 1;
							}
						}
						if( b )
						{
							if( j == 0 )
								r0.SetTop( r0.y + 2 );
							else
								r0.height -= 2;
						}
					}
					if( r0.y > r.y && r0.GetBottom() < r.GetBottom() )
					{
						r = r0;
						if( r.height <= 3 )
							break;
						int32 l = SRand::Inst().Rand( 3, Max( 3, r.width / 2 ) + 1 );
						r.y += SRand::Inst().Rand( 0, r.height - l + 1 );
						r.height = l;
					}
					else
					{
						if( r0.y == r.y )
							r0.SetTop( r0.y + 1 );
						if( r0.GetBottom() == r.GetBottom() )
							r0.height--;
						r = r0;
						if( r.height < 1 )
							break;
					}
				}
				else
				{
					for( int i = 0; i < r.width; i += Max( 2, r.width - 1 ) )
					{
						int32 x = i + r.x;
						int32 j0 = 0;
						bool b = false;
						for( int j = 0; j <= r.height; j++ )
						{
							if( j == r.height || vecTemp[r1.x + i + ( r1.y + j ) * maxRect.width] )
							{
								if( j - j0 >= SRand::Inst().Rand( 2, 4 ) )
								{
									int32 l = Min( Min( j - j0, SRand::Inst().Rand( 6, 8 ) ), Max( 3, SRand::Inst().Rand( ( j - j0 ) / 2, j - j0 + 1 ) ) );
									int32 yBegin = r.y + j0 + SRand::Inst().Rand( 0, j - j0 - l + 1 );
									int32 yEnd = yBegin + l;
									for( int y = yBegin; y < yEnd; y++ )
										m_gendata[x + y * nWidth] = eType_Billboard_1;
									b = true;
								}
								j0 = j + 1;
							}
						}
						if( b )
						{
							if( i == 0 )
								r0.SetLeft( r0.x + 2 );
							else
								r0.width -= 2;
						}
					}
					r = r0;
					if( r.width <= 3 )
						break;
				}

				bVertical = !bVertical;
			}
		}
		for( auto& r : vecTempRoom )
		{
			auto r1 = r.Offset( TVector2<int32>( -maxRect.x, -maxRect.y ) );
			vecTemp[r1.x + r1.y * maxRect.width] = vecTemp[r1.x + 1 + r1.y * maxRect.width]
				= vecTemp[r1.GetRight() - 1 + r1.y * maxRect.width] = vecTemp[r1.GetRight() - 2 + r1.y * maxRect.width]
				= vecTemp[r1.x + ( r1.GetBottom() - 1 ) * maxRect.width] = vecTemp[r1.x + 1 + ( r1.GetBottom() - 1 ) * maxRect.width]
				= vecTemp[r1.GetRight() - 1 + ( r1.GetBottom() - 1 ) * maxRect.width] = vecTemp[r1.GetRight() - 2 + ( r1.GetBottom() - 1 ) * maxRect.width] = 0;
			for( int i = r1.x + 1; i < r1.GetRight() - 1; i++ )
			{
				for( int j = r1.y + 1; j < r1.GetBottom() - 1; j++ )
				{
					vecTemp[i + j * maxRect.width] = 0;
				}
			}
			
			int8 k = SRand::Inst().Rand( 0, 2 );
			int8 nDir = SRand::Inst().Rand( 0, 2 );
			for( int i = 0; i < r1.width - 1; i++ )
			{
				TVector2<int32> p( nDir ? r1.x + i : r1.GetRight() - 1 - i, k ? r1.y : r1.GetBottom() - 1);
				if( vecTemp[p.x + p.y * maxRect.width] )
					continue;
				auto lim = r1;
				if( nDir )
					lim.SetLeft( p.x );
				else
					lim.SetRight( p.x + 1 );
				auto cargo = PutRect( vecTemp, maxRect.width, maxRect.height, p, TVector2<int32>( 2, 2 ), TVector2<int32>( r1.width, r1.height - 2 ), lim, -1, 0 );
				if( cargo.width <= 0 )
					continue;

				int8 k2 = SRand::Inst().Rand( 0, 2 );
				bool bOK = false;
				for( int k1 = 0; k1 < 2; k1++ )
				{
					if( k1 ^ k2 )
					{
						if( cargo.height < 4 )
							continue;
						int32 w = 2;
						if( w == r1.width - i - 1 )
						{
							if( cargo.width <= 2 )
								continue;
							w = 3;
						}
						if( nDir )
							cargo.width = w;
						else
							cargo.SetLeft( cargo.GetRight() - w );
					}
					else
					{
						if( k )
							cargo.height = 2;
						else
							cargo.SetTop( cargo.GetBottom() - 2 );
						int32 w = Min( cargo.width, SRand::Inst().Rand( 4, 8 ) );
						if( w == r1.width - i - 1 )
						{
							if( cargo.width <= 2 )
								continue;
							if( w >= SRand::Inst().Rand( 5, 8 ) )
								w--;
							else
								w = r1.width - i;
						}
						if( nDir )
							cargo.width = w;
						else
							cargo.SetLeft( cargo.GetRight() - w );
					}

					bOK = true;
					break;
				}

				if( bOK )
				{
					GenCargo( cargo.Offset( TVector2<int32>( maxRect.x, maxRect.y ) ) );
					for( int i = cargo.x; i < cargo.GetRight(); i++ )
					{
						for( int j = cargo.y; j < cargo.GetBottom(); j++ )
							vecTemp[i + j * maxRect.width] = 2;
					}
					cargo.SetLeft( cargo.x - SRand::Inst().Rand( 1, 3 ) );
					cargo.SetTop( cargo.y - SRand::Inst().Rand( 1, 3 ) );
					cargo.width += SRand::Inst().Rand( 1, 3 );
					cargo.height += SRand::Inst().Rand( 1, 3 );
					cargo = cargo * r1;
					for( int i = cargo.x; i < cargo.GetRight(); i++ )
					{
						for( int j = cargo.y; j < cargo.GetBottom(); j++ )
						{
							if( !vecTemp[i + j * maxRect.width] )
								vecTemp[i + j * maxRect.width] = 3;
						}
					}
					i--;
					k = !k;
				}
			}

			for( int i = r1.x; i < r1.GetRight(); i++ )
			{
				for( int j = r1.y; j < r1.GetBottom(); j++ )
				{
					if( vecTemp[i + j * maxRect.width] == 3 )
						vecTemp[i + j * maxRect.width] = 0;
				}
			}
			for( k = 0; k < 2; k++ )
			{
				int32 j = k ? r1.GetBottom() - 1 : r1.y;
				for( int i = r1.x; i < r1.GetRight(); i++ )
				{
					if( !vecTemp[i + j * maxRect.width] )
					{
						int32 x;
						for( x = i; x < r1.GetRight(); x++ )
						{
							if( vecTemp[x + j * maxRect.width] )
								break;
						}
						if( x - i >= 2 )
						{
							auto r2 = TRectangle<int32>( i, j, x - i, 1 ).Offset( TVector2<int32>( maxRect.x, maxRect.y ) );
							AddChunk( r2, eType_Room_1, &m_vecBar_a[0] );
						}
						i = x;
					}
				}
			}
			for( k = 0; k < 2; k++ )
			{
				int32 j = k ? r.GetBottom() - 1 : r.y;
				for( int i = r.x; i < r.GetRight(); i++ )
				{
					if( m_gendata[i + j * nWidth] == eType_Room_2 )
					{
						int32 x;
						for( x = i; x < r.GetRight(); x++ )
						{
							if( m_gendata[x + j * nWidth] != eType_Room_2 )
								break;
						}
						if( x - i >= 1 && x - i <= 2 )
						{
							auto r2 = TRectangle<int32>( i, j, x - i, 1 );
							AddChunk( r2, eType_Room_Door, NULL );
						}
						i = x;
					}
				}
			}
		}

		for( int iCargo = nLastCargo; iCargo < m_vecCargos1.size(); iCargo++ )
		{
			auto rect = m_vecCargos1[iCargo];
			GenCargo1Room( rect, TRectangle<int32>( 0, 0, 0, 0 ) );
		}
	}
	else if( maxRect.height == 2 )
		AddChunk( maxRect, eType_Chunk, &m_vecBar2 );
	else
		maxRect = TRectangle<int32>( rect.x, rect.GetBottom(), rect.width, 0 );
}

void CLevelGenNode2_2_1::AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32>>* pVec )
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

void CLevelGenNode2_2_1::GenLimbs( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto r = rect;
	if( r.width >= r.height )
	{
		if( r.height > 2 )
		{
			r.x += SRand::Inst().Rand( 0, r.height - 2 + 1 );
			r.height = 2;
		}
		for( int i = 0; i < r.width; i++ )
		{
			int32 x = r.x + SRand::Inst().Rand( 0, r.width );
			for( int y = r.y; y < r.GetBottom(); y++ )
			{
				if( m_gendata[x + y * nWidth] != eType_Control_Room )
					break;
				m_gendata[x + y * nWidth] = eType_Control_Room_4;
			}
		}
	}
	else
	{
		if( r.width > 2 )
		{
			r.y += SRand::Inst().Rand( 0, r.width - 2 + 1 );
			r.width = 2;
		}
		for( int i = 0; i < r.height; i++ )
		{
			int32 y = r.y + SRand::Inst().Rand( 0, r.height );
			for( int x = r.x; x < r.GetRight(); x++ )
			{
				if( m_gendata[x + y * nWidth] != eType_Control_Room )
					break;
				m_gendata[x + y * nWidth] = eType_Control_Room_4;
			}
		}
	}
}

void CLevelGenNode2_2_1::GenCargo( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector2<int32> > vec;
	int32 s0 = 0;
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( m_gendata[i + j * nWidth] >= eType_Obj1 )
				continue;
			s0++;
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	int32 s = s0 * SRand::Inst().Rand( 0.7f, 0.8f );
	int32 s1 = rect.width >= 3 && rect.height >= 3 ? s0 * SRand::Inst().Rand( 0.5f, 0.6f ) : 0;
	int32 s2 = s * SRand::Inst().Rand( 0.3f, 0.35f );
	for( auto& p : vec )
	{
		if( s <= 0 )
			break;
		if( m_gendata[p.x + p.y * nWidth] >= eType_Obj1 )
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
			int8 nType = SRand::Inst().Rand( 0, 3 );
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

void CLevelGenNode2_2_1::GenCargo0( const TRectangle<int32>& rect )
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

bool CLevelGenNode2_2_1::GenBar0( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TRectangle<int32> > vecBar;
	vector<TRectangle<int32> > vecCargo;
	vector<TRectangle<int32> > vecChain;
	vecBar.push_back( rect );
	auto r = rect;
	vector<int32> vecX;
	vector<int32> vecLen;
	while( r.GetBottom() < nHeight )
	{
		vecX.resize( 0 );
		for( int i = 0; i < ( r.width + 1 ) / 2; i++ )
		{
			int a = r.x + i;
			int b = r.GetRight() - 1 - i;
			if( a == b )
				vecX.push_back( a );
			else
			{
				if( SRand::Inst().Rand( 0, 2 ) )
					swap( a, b );
				vecX.push_back( a );
				vecX.push_back( b );
			}
		}
		vecLen.resize( vecX.size() );
		int32 i1 = vecX.size() / 4 * 2;
		TVector2<int32> chain( -1, -1 );
		int8 nOK = 1;
		for( int k = 0; k < 2; k++ )
		{
			for( int i = ( k == 0 ? vecX.size() - 1 : i1 - 1 ); i >= ( k == 0 ? i1 : 0 ); i-- )
			{
				int32 x = vecX[i];
				int32 l;
				for( l = 0; l <= 7; l++ )
				{
					vecLen[i] = l;
					if( l + r.GetBottom() >= nHeight )
					{
						l = -1;
						break;
					}
					auto nType = m_gendata[x + ( l + r.GetBottom() ) * nWidth];
					if( nType == eType_Room_1 || nType == eType_Control_Room || nType == eType_Billboard_1
						|| nType >= eType_Cargo1 && nType <= eType_Chunk )
						break;
					if( nType <= eType_Road || nType == eType_Billboard )
						continue;
					l = -1;
					break;
				}
				if( l >= 3 && l <= 7 )
				{
					chain = TVector2<int32>( x, r.GetBottom() + l );
					break;
				}
			}
			if( chain.x >= 0 )
			{
				nOK = -1;
				vecChain.push_back( TRectangle<int32>( chain.x, r.GetBottom(), 1, chain.y - r.GetBottom() ) );
				break;
			}

			if( k == 0 )
			{
				TRectangle<int32> r1( 0, 0, 0, 0 );
				if( r.GetBottom() < nHeight )
				{
					do
					{
						TRectangle<int32> r2( r.x, r.GetBottom(), r.width, 1 );
						TVector2<int32> maxSize( r.width, SRand::Inst().Rand( 5, 7 ) + 2 );
						r2 = PutRect( m_gendata1, nWidth, nHeight, r2, TVector2<int32>( 3, 4 ), maxSize, TRectangle<int32>( 0, r2.y, nWidth, nHeight - r2.y ), -1, 0, 0 );
						if( r2.width <= 0 )
							break;
						r2.height = Max( r2.height - 2, 4 );
						r2.SetTop( r2.GetBottom() - 1 );
						if( !CheckBar( r2, r2 ) )
							break;
						auto r3 = PutRectEx( m_gendata1, nWidth, nHeight, r2, TVector2<int32>( 4, 1 ), TVector2<int32>( SRand::Inst().Rand( 6, 9 ), 1 ),
							TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, [this] ( TRectangle<int32> r, TRectangle<int32> r1 ) { return CheckBar( r, r1 ); } );
						if( r3.width <= 0 )
							break;
						int32 w = Max( r2.width, Max( 4, Min( r3.width - 2, ( r2.width - 1 ) * 3 - 1 ) ) );
						r3.x = SRand::Inst().Rand( Max( r2.GetRight() - w, r3.x ), Min( r2.x, r3.GetRight() - w ) + 1 );
						r3.width = w;
						int32 nCargoWidth = r3.width / 3 + 1;
						int32 xChain1;
						if( r3.x + r3.GetRight() + SRand::Inst().Rand( 0, 2 ) > r2.x + r2.GetRight() )
						{
							xChain1 = r2.GetRight() - 1 - SRand::Inst().Rand( 0, r2.width - nCargoWidth );
							r2 = TRectangle<int32>( r2.x, r.GetBottom(), xChain1 - r2.x, r2.y - r.GetBottom() );
						}
						else
						{
							xChain1 = r2.x + SRand::Inst().Rand( 0, r2.width - nCargoWidth );
							r2 = TRectangle<int32>( xChain1 + 1, r.GetBottom(), r2.GetRight() - xChain1 - 1, r2.y - r.GetBottom() );
						}
						vecBar.push_back( r3 );
						vecChain.push_back( TRectangle<int32>( xChain1, r.GetBottom(), 1, r3.y - r.GetBottom() ) );
						r1 = r3;
						TRectangle<int32> lastCargo( 0, 0, 0, 0 );
						while( r2.height )
						{
							auto cargo = r2;
							int32 h;
							if( r2.height <= 3 )
								h = r2.height;
							else if( r2.height == 4 )
								h = 2;
							else
								h = SRand::Inst().Rand( 2, 4 );
							r2.height -= h;
							cargo.SetTop( r2.GetBottom() );
							if( lastCargo.width )
							{
								cargo.width = SRand::Inst().Rand( 2, lastCargo.width );
								if( cargo.width < lastCargo.width )
									cargo.x = SRand::Inst().Rand( lastCargo.x, lastCargo.GetRight() - cargo.width + 1 );
								else
									cargo.x = SRand::Inst().Rand( Max( r2.x, lastCargo.x - 1 ),
										Min( r2.GetRight(), lastCargo.GetRight() + 1 ) - cargo.width + 1 );
							}
							else
							{
								cargo.width = nCargoWidth;
								cargo.x = SRand::Inst().Rand( r2.x, r2.GetRight() - cargo.width + 1 );
							}
							lastCargo = cargo;
							vecCargo.push_back( cargo );
						}
					} while( 0 );
				}
				if( r1.width )
				{
					r = r1;
					break;
				}

				for( int i = vecX.size() - 1; i >= i1; i-- )
				{
					int32 x = vecX[i];
					for( int32 l = 3; l < vecLen[i]; l++ )
					{
						int32 y = r.GetBottom() + l;
						if( m_gendata[x + y * nWidth] == 0 )
						{
							TVector2<int32> maxSize( SRand::Inst().Rand( 3, 6 ), SRand::Inst().Rand( 2, 5 ) );
							maxSize.y = Min( maxSize.y, Max( 3, maxSize.x - 1 ) );
							auto r2 = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 2, 2 ), maxSize,
								TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, 0 );
							if( r2.width > 0 )
							{
								GenCargo( r2 );
								for( int x = r2.x; x < r2.GetRight(); x++ )
								{
									for( int y = r2.y; y < r2.GetBottom(); y++ )
										m_gendata1[x + y * nWidth] = 1;
								}
								vecChain.push_back( TRectangle<int32>( x, r.GetBottom(), 1, y - r.GetBottom() ) );
								nOK = -1;
								break;
							}
						}
					}
					if( nOK == -1 )
						break;
				}
			}
			else
				nOK = 0;
		}
		if( nOK == 0 )
			return false;
		else if( nOK == -1 )
			break;
	}

	for( auto& rect : vecChain )
	{
		m_vecChain.push_back( rect );
		for( int y = rect.y; y < rect.GetBottom(); y++ )
			m_gendata1[rect.x + y * nWidth] = 2;
	}
	for( auto& rect : vecCargo )
	{
		GenCargo0( rect );
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				m_gendata1[x + y * nWidth] = 2;
		}
	}
	for( int i = 0; i < vecBar.size(); i++ )
	{
		auto& rect = vecBar[i];
		AddChunk( rect, eType_Obj, &m_vecBar0 );
		for( int x = rect.x; x < rect.GetRight(); x++ )
			m_gendata1[x + rect.y * nWidth] = i < vecBar.size() - 1 ? 2 : 1;
		auto rect1 = TRectangle<int32>( rect.x - 4, rect.y - 4, rect.width + 8, rect.height + 8 ) * TRectangle<int32>( 0, 0, nWidth, nHeight );
		for( int x = rect1.x; x < rect1.GetRight(); x++ )
		{
			for( int y = rect1.y; y < rect1.GetBottom(); y++ )
			{
				if( x - rect.x + y - rect.y < -4 || x - rect.x + rect.GetBottom() - 1 - y < -4
					|| rect.GetRight() - 1 - x + y - rect.y < -4 || rect.GetRight() - 1 - x + rect.GetBottom() - 1 - y < -4 )
					continue;
				m_gendata2[x + y * nWidth] = 1;
			}
		}
	}
	return true;
}

bool CLevelGenNode2_2_1::CheckBar( TRectangle<int32> rect, TRectangle<int32> r1 )
{
	for( int x = r1.x; x < r1.GetRight(); x++ )
	{
		for( int y = r1.y; y < r1.GetBottom(); y++ )
		{
			if( m_gendata1[x + y * m_region.width] || m_gendata2[x + y * m_region.width] )
				return false;
		}
	}
	return true;
}

bool CLevelGenNode2_2_1::CheckCargo( TRectangle<int32> rect, TRectangle<int32> r1 )
{
	for( int x = r1.x; x < r1.GetRight(); x++ )
	{
		for( int y = r1.y; y < r1.GetBottom(); y++ )
		{
			if( m_gendata[x + y * m_region.width] || m_gendata1[x + y * m_region.width] )
				return false;
		}
	}
	return true;
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
