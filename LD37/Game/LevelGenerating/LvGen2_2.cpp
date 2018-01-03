#include "stdafx.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

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
	TRectangle<int32> rect1;
	TRectangle<int32> controlRoomRect;
	if( b )
	{
		rect1 = TRectangle<int32>( 4, 2, region.width - 4, region.height - 2 );
		controlRoomRect = TRectangle<int32>( 0, 2, 4, region.height - 2 );
	}
	else
	{
		rect1 = TRectangle<int32>( 0, 2, region.width - 4, region.height - 2 );
		controlRoomRect = TRectangle<int32>( region.width - 4, 2, 4, region.height - 2 );
	}
	TRectangle<int32> rect0( region.x, region.y, region.width, 2 );
	
	m_pBaseNode[b ? 1 : 0]->Generate( context, rect0 );
	m_pControlRoomNode[b ? 1 : 0]->Generate( context, controlRoomRect.Offset( TVector2<int32>( region.x, region.y ) ) );
	uint32 h1 = rect1.height - 2;
	uint32 l = rect1.width;
	uint32 n = l / Max( h1 + 2, 6u );
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
	context.mapTags["1"] = eType_Chunk;
	for( auto& rect : m_vecFences )
	{
		m_pFenceNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	
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
	for( auto& house : m_vecHouses )
	{
		if( house.rect.width )
			m_pHouseNode->Generate( context, house.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

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
	m_vecHouses.clear();
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
				m_gendata[x + y * nWidth] = eType_Chunk;
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
			SHouse house( rect );
			m_vecHouses.push_back( house );
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

	for( auto& house : m_vecHouses )
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
	}

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
}

void CLevelGenNode2_2_2::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
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
				p.z = Min( nHeight - 4 - p.y, SRand::Inst().Rand( 4, 6 ) );
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
	int32 k1 = SRand::Inst().Rand( 2, 4 ), k2 = SRand::Inst().Rand( 4, 6 );
	for( int i = 0; i < vecPath.size(); i++ )
	{
		auto& cur = vecPath[i];

		if( i >= vecPath.size() - 1 )
		{
			TRectangle<int32> r1 = r;
			r1.SetBottom( cur.y + cur.z );
			r1.SetTop( Max( r.GetTop(), cur.y ) );
			if( r1.height >= 5 )
				AddRoom( r1 );
			if( r.y < r1.y )
				AddRoad( TRectangle<int32>( r.x, r.y, r.width, r1.y - r.y ) );
			break;
		}
		auto& nxt = vecPath[i + 1];

		if( i == 0 )
		{
			r = TRectangle<int32>( nxt.x, nxt.y + 1, 1, nxt.z - 1 );
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
				r1.SetLeft( Max( cur.x + SRand::Inst().Rand( 3, 7 ), 4 ) );
			}
			r1 = r1 + r;
			vecTempChunks.push_back( TRectangle<int32>( 0, r1.y, r1.x, r1.GetBottom() ) );
			vecTempChunks.push_back( TRectangle<int32>( r1.GetRight(), r1.y, nWidth - r1.GetRight(), r1.GetBottom() ) );

			TRectangle<int32> r2( r.x, 0, r.width, 1 );
			r2.SetLeft( Max( 0, r2.x - SRand::Inst().Rand( 0, 3 ) ) );
			r2.SetRight( Min( nWidth, r2.GetRight() + SRand::Inst().Rand( 0, 3 ) ) );
			AddBar( r2 );
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
							r1.SetRight( r1.GetRight() + SRand::Inst().Rand( 5, 8 ) );
						else
							r1.SetLeft( r1.GetLeft() - SRand::Inst().Rand( 5, 8 ) );
						AddRoom( r1 );

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

					r = TRectangle<int32>( nxt.x, nxt.y, 0, nxt.z );
					if( nxt.x < cur.x )
					{
						r.SetRight( Min( r.GetRight() + SRand::Inst().Rand( 5, 8 ), nWidth + 2 ) );
						if( r.x > 0 )
							vecTempChunks.push_back( TRectangle<int32>( 0, r.y, r.x, 1 ) );
					}
					else
					{
						r.SetLeft( Max( r.GetLeft() - SRand::Inst().Rand( 5, 8 ), nWidth - 2 ) );
						if( r.GetRight < nWidth )
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
				else if( nLen >= 17 )
				{
					bool b = SRand::Inst().Rand( 0, 2 ) > 0;
					bRoomBegin = !b;
					bRoomEnd = b;
				}
				else
				{
					bRoomBegin = false;
					bRoomEnd = false;
				}

				TRectangle<int32> r1, r2;
				r2 = TRectangle<int32>( Min( cur.x, nxt.x ), cur.y, nLen, cur.z );

				r1 = TRectangle<int32>( r.x, cur.y, r.width, cur.z );
				if( r1.GetBottom() - r.y <= SRand::Inst().Rand( 8, 12 ) )
				{
					r1.SetTop( r.y );
					if( bRoomBegin )
						AddRoom( r1 );
					else
						AddBigChunk( r1, 1 );
				}
				else if( bRoomBegin )
				{
					if( r.height > 0 )
					{
						r1.SetTop( Min( r1.GetBottom() - 5, r1.y + SRand::Inst().Rand( -2, 2 ) ) );
						r.SetBottom( r1.y );
						AddRoad( r );
					}
					AddRoom( r1 );
				}
				else
				{
					r1.SetTop( r.y );
					AddRoad( r1 );
				}
				vecTempChunks.push_back( TRectangle<int32>( r1.x, r1.GetBottom(), r1.width, 0 ) );

				if( nxt.x < cur.x )
					r2.SetRight( r.x );
				else
					r2.SetLeft( r.GetRight() );

				r = TRectangle<int32>( nxt.x, nxt.y, 0, nxt.z );
				if( nxt.x < cur.x )
				{
					r.SetRight( Min( r.GetRight() + SRand::Inst().Rand( 5, 8 ), nWidth + 2 ) );
					if( r.x > 0 )
						vecTempChunks.push_back( TRectangle<int32>( 0, r.y, r.x, 1 ) );
				}
				else
				{
					r.SetLeft( Max( r.GetLeft() - SRand::Inst().Rand( 5, 8 ), nWidth - 2 ) );
					if( r.GetRight < nWidth )
						vecTempChunks.push_back( TRectangle<int32>( r.GetRight(), r.y, nWidth - r.GetRight(), 1 ) );
				}
				if( nxt.x < cur.x )
					r2.SetLeft( r.GetRight() );
				else
					r2.SetRight( r.GetLeft() );
				if( bRoomEnd )
				{
					AddRoom( r );
					r.SetBottom( r.GetTop() );
				}
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

				r1.SetLeft( Max( 0, r1.GetLeft() - SRand::Inst().Rand( 0, 2 ) ) );
				r1.SetRight( Min( nWidth, r1.GetRight() - SRand::Inst().Rand( 0, 2 ) ) );
				AddRoom( r1 );
				if( r1.GetLeft() < 4 )
				{
					TRectangle<int32> r2( r1.GetRight(), cur.y, 0, cur.z );
					r2.SetRight( Min( nWidth, r2.GetRight() + SRand::Inst().Rand( 15, 18 ) ) );
					AddBigChunk( r2, SRand::Inst().Rand( 0, 2 ) ? 0 : 2 );
					if( r2.GetRight() < nWidth )
						vecTempChunks.push_back( TRectangle<int32>( r2.GetRight(), r2.y, nWidth - r2.GetRight(), r2.height ) );
				}
				else if( r1.GetRight() >= nWidth - 4 )
				{
					TRectangle<int32> r2( r1.GetLeft(), cur.y, 0, cur.z );
					r2.SetRight( Max( 0, r2.GetLeft() - SRand::Inst().Rand( 15, 18 ) ) );
					AddBigChunk( r2, SRand::Inst().Rand( 0, 2 ) ? 0 : 2 );
					if( r2.GetLeft() > 0 )
						vecTempChunks.push_back( TRectangle<int32>( 0, r2.y, nWidth - r2.GetLeft(), r2.height ) );
				}
				else
				{
					bool b = SRand::Inst().Rand( 0, 2 );;
					AddBigChunk( TRectangle<int32>( 0, cur.y, r1.x, cur.z ), b ? 0 : 2 );
					AddBigChunk( TRectangle<int32>( r1.GetRight(), cur.y, nWidth - r1.GetRight(), cur.z ), b ? 2 : 0 );
				}

				r = TRectangle<int32>( r.x, r1.GetBottom(), r.width, r1.GetBottom() );
			}

			k2 = SRand::Inst().Rand( 3, 5 );
			continue;
		}
		if( k1 <= 0 )
		{
			if( cur.y + cur.z < nHeight )
			{
				auto& nxt1 = vecPath[i + 2];

				if( cur.y == nxt.y )
				{
					if( r.height > 0 )
						AddRoad( r );
					TRectangle<int32> r1 = r;
					r1.y = cur.y;
					r1.height = cur.z;
					r1.SetLeft( Min( r1.GetLeft(), nxt.x ) );
					r1.SetRight( Max( r1.GetRight(), nxt.x + 1 ) );
					AddRoad( r1 );
					r.y = r1.y;
					r.height = 0;

					nxt.x = cur.x;
					nxt.y = nxt1.y;
					nxt.z = nxt1.z;
					if( k2 > 1 )
					{
						r.height = nxt.z;
						r.SetLeft( Max( 0, r.GetLeft() + SRand::Inst().Rand( 0, 3 ) ) );
						r.SetRight( Min( nWidth, r.GetRight() - SRand::Inst().Rand( 0, 3 ) ) );
						AddRoom( r );

						if( nxt.x < nxt1.x )
						{
							TRectangle<int32> r1( 0, r.y, r.x, 1 );
							r1.SetRight( Min( r1.GetRight(), SRand::Inst().Rand( 4, 7 ) ) );
							if( r1.width > 0 )
								vecTempChunks.push_back( r1 );

							r = TRectangle<int32>( r.GetRight(), nxt.y, nWidth - r.GetRight(), 1 );
							AddBar( r );
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
							AddBar( r );
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
					if( r.height > 0 )
						AddRoad( r );
					if( nxt.y == nxt1.y || k2 <= 1 )
					{
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
						if( r.x > 0 )
						{
							int32 l = Min( r.x, SRand::Inst().Rand( 9, 13 ) );
							vecTempBars.push_back( TRectangle<int32>( r.x - l, cur.y - 1, l, 1 ) );
							TRectangle<int32> r2( r.x - l + SRand::Inst().Rand( 1, 4 ), cur.y, 0, cur.z );
							r2.SetRight( Min( r2.GetRight() + SRand::Inst().Rand( 4, 5 ), r1.GetLeft() ) );
							r2.height += SRand::Inst().Rand( 0, 3 );
							if( r2.x > 0 )
								vecTempChunks.push_back( TRectangle<int32>( 0, cur.y, r2.x, 1 ) );

							if( r2.x >= 7 )
							{
								l = Min( r2.x, SRand::Inst().Rand( 7, 11 ) );
								int32 h1 = cur.y + cur.z - Min( cur.z - 4, SRand::Inst().Rand( 0, 3 ) );
								vecTempBars.push_back( TRectangle<int32>( r2.x - l, h1 - 1, l, 1 ) );
								TRectangle<int32> r3( r2.x - l + SRand::Inst().Rand( 0, 3 ), h1, 0, nxt.y + nxt.z - h1 );
								r3.SetRight( Min( r3.GetRight() + SRand::Inst().Rand( 4, 5 ), r2.GetLeft() ) );
								if( r3.x > 0 )
									vecTempChunks.push_back( TRectangle<int32>( 0, r3.y, r3.x, 1 ) );
							}
						}
						if( r.GetRight() < nWidth )
						{
							int32 l = Min( nWidth - r.GetRight(), SRand::Inst().Rand( 9, 13 ) );
							vecTempBars.push_back( TRectangle<int32>( r.GetRight(), cur.y - 1, l, 1 ) );
							TRectangle<int32> r2( r.GetRight() + l - SRand::Inst().Rand( 1, 4 ), cur.y, 0, cur.z );
							r2.SetLeft( Min( r2.GetLeft() - SRand::Inst().Rand( 4, 5 ), r1.GetRight() ) );
							r2.height += SRand::Inst().Rand( 0, 3 );
							if( r2.GetRight() < nWidth )
								vecTempChunks.push_back( TRectangle<int32>( r2.GetRight(), cur.y, nWidth - r2.GetRight(), 1 ) );

							if( r2.GetRight() <= nWidth - 7 )
							{
								l = Min( nWidth - r2.GetRight(), SRand::Inst().Rand( 7, 11 ) );
								int32 h1 = cur.y + cur.z - Min( cur.z - 4, SRand::Inst().Rand( 0, 3 ) );
								vecTempBars.push_back( TRectangle<int32>( r2.GetRight(), h1 - 1, l, 1 ) );
								TRectangle<int32> r3( r2.GetRight() + l + SRand::Inst().Rand( 0, 3 ), h1, 0, nxt.y + nxt.z - h1 );
								r3.SetLeft( Max( r3.GetLeft() - SRand::Inst().Rand( 4, 5 ), r2.GetRight() ) );
								if( r3.GetRight() < nWidth )
									vecTempChunks.push_back( TRectangle<int32>( r3.GetRight(), r3.y, nWidth - r3.GetRight(), 1 ) );
							}
						}

						r.SetTop( nxt.y + nxt.z );
						r.SetBottom( nxt.y + nxt.z );
						i++;
						k2--;
						k1 = SRand::Inst().Rand( 2, 4 );
						continue;
					}
				}
				k1 = SRand::Inst().Rand( 3, 5 );
				continue;
			}
		}

		if( cur.y == nxt.y )
		{
			if( r.height > 0 )
				AddRoad( r );
			TRectangle<int32> r1 = r;
			r1.y = cur.y;
			r1.height = cur.z;
			r1.SetLeft( Min( r1.GetLeft(), nxt.x ) );
			r1.SetRight( Max( r1.GetRight(), nxt.x + 1 ) );
			AddRoad( r1 );

			r.y = r1.y;
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

void CLevelGenNode2_2_2::AddRoad( const TRectangle<int32>& r )
{
	m_vecRoads.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetRight(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Road;
		}
	}
}

void CLevelGenNode2_2_2::AddRoom( const TRectangle<int32>& r )
{
	m_vecRooms.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetRight(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Room;
		}
	}
}

void CLevelGenNode2_2_2::AddBar( const TRectangle<int32>& r )
{
	m_vecBars.push_back( r );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetRight(); j++ )
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
		for( int j = r.y; j < r.GetRight(); j++ )
		{
			m_gendata[i + j * m_region.width] = eType_Chunk;
		}
	}
}

void CLevelGenNode2_2_2::ProcessTempBar( const vector<TRectangle<int32> > v )
{
	for( auto rect : v )
	{
		int32 nWidth = m_region.width;
		int32 nHeight = m_region.height;
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
			int32 nBegin = rect.x;
			int32 nEnd = rect.GetRight() - 1;
			while( nBegin > 0 && !m_gendata[nBegin - 1] )
				nBegin--;
			while( nEnd < nWidth + 1 && !m_gendata[nEnd + 1] )
				nEnd++;
			int32 nLen = Min<int32>( 2 + floor( ( nEnd - nBegin - 1 ) * SRand::Inst().Rand( 0.75f, 0.99f ) ), SRand::Inst().Rand( 10, 16 ) );
			rect.x -= ( nLen - rect.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
			rect.width = nLen;
			rect.x = Max( rect.x, nBegin );
			rect.x = Min( rect.x, nEnd - nLen + 1 );
		}
		AddBar( rect );
	}
}

void CLevelGenNode2_2_2::ProcessTempChunks( const vector<TRectangle<int32>> v )
{
}
