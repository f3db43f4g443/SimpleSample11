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
	m_par.clear();
	m_vecRoads.clear();
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
		uint32 nType = nLastType >= 0 && nLastType < 3 ? SRand::Inst().Rand( 0, 9 ) : SRand::Inst().Rand( 0, 3 );
		uint32 nPreCount = vecRect.size();
		if( nType < 3 )
		{
			uint32 w1[10];
			uint32 a1, a2, bMin, bMax, hMin, hMax;
			if( nType == 0 )
			{
				a1 = 3;
				a2 = 2;
				bMin = 6;
				bMax = 7;
				hMin = 14;
				hMax = 16;
			}
			else if( nType == 1 )
			{
				a1 = 3;
				a2 = 3;
				bMin = 6;
				bMax = 7;
				hMin = 11;
				hMax = 13;
			}
			else if( nType == 2 )
			{
				a1 = 3;
				a2 = 4;
				bMin = 5;
				bMax = 6;
				hMin = 10;
				hMax = 12;
			}
			else if( nType == 3 )
			{
				a1 = 4;
				a2 = 3;
				bMin = 5;
				bMax = 6;
				hMin = 8;
				hMax = 10;
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
			m_vecRoads.push_back( rect );
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

	for( auto& house : m_vecHouses )
	{
		if( !house.Generate( m_gendata, nWidth, nHeight, eType_House, eType_House_1, eType_House_2, eType_House_Exit1, eType_House_Exit2,
			m_gendata, eType_None, eType_Temp1, m_par ) )
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
		TVector2<int32> sizeMin( 2, 2 ), sizeMax( 3, 5 );
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

	vector<TRectangle<int32> > m_vecObjs;
	for( auto& rect : m_vecRoads )
	{
		int32 s = rect.width * rect.height;
		int32 r = SRand::Inst().Rand( 0, s );
		if( r > 80 && SRand::Inst().Rand( 0, 1 ) )
		{
			TRectangle<int32> rect1( rect.x, rect.y, 4, 2 );
			rect1.x += SRand::Inst().Rand( 0, rect.width - rect1.width + 1 );
			m_vecObjs.push_back( rect1 );
		}
		else if( r < 60 )
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
	p.x = SRand::Inst().Rand( nWidth - nWidth / 3, nWidth );
	vecPath.push_back( p );
	int32 k = SRand::Inst().Rand( 4, 6 );
	while( p.y + p.z < nHeight )
	{
		auto p1 = vecPath[vecPath.size() - 2];
		auto p2 = vecPath[vecPath.size() - 1];

		k--;
		p.y += p.z;
		if( !k && p.y + 7 <= nHeight )
		{
			p.z = Min( nHeight - p.y, SRand::Inst().Rand( 7, 9 ) );
			vecPath.push_back( p );
			if( p.x > 4 && p.x < nWidth - 5 && SRand::Inst().Rand( 0, 2 ) )
			{
				k = SRand::Inst().Rand( 2, 5 );
				p.x = SRand::Inst().Rand( 0, 4 ) * ( p.x >= nWidth / 2 ? -1 : 1 ) + nWidth / 2;
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
				p.z = Min( nHeight - p.y, SRand::Inst().Rand( 4, 6 ) );
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
}