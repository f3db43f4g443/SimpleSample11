#include "stdafx.h"
#include "LvGen1.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

void CLevelGenNode1_3::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_nType = XmlGetAttr<int32>( pXml, "gen_type", 0 );
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );
	m_pWallChunk0Node = CreateNode( pXml->FirstChildElement( "wallchunk0" )->FirstChildElement(), context );
	m_pWallChunk1Node = CreateNode( pXml->FirstChildElement( "wallchunk1" )->FirstChildElement(), context );
	m_pWallChunk2Node = CreateNode( pXml->FirstChildElement( "wallchunk2" )->FirstChildElement(), context );
	m_pBlock1Node = CreateNode( pXml->FirstChildElement( "block1" )->FirstChildElement(), context );
	m_pBlock2Node = CreateNode( pXml->FirstChildElement( "block2" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pCrate1Node = CreateNode( pXml->FirstChildElement( "crate1" )->FirstChildElement(), context );
	m_pCrate2Node = CreateNode( pXml->FirstChildElement( "crate2" )->FirstChildElement(), context );
	m_pScrapNode = CreateNode( pXml->FirstChildElement( "scrap" )->FirstChildElement(), context );
	m_pWindowNode = CreateNode( pXml->FirstChildElement( "window" )->FirstChildElement(), context );
	m_pWindow1Node[0] = CreateNode( pXml->FirstChildElement( "window1_1" )->FirstChildElement(), context );
	m_pWindow1Node[1] = CreateNode( pXml->FirstChildElement( "window1_2" )->FirstChildElement(), context );
	m_pWindow1Node[2] = CreateNode( pXml->FirstChildElement( "window1_3" )->FirstChildElement(), context );
	m_pWindow1Node[3] = CreateNode( pXml->FirstChildElement( "window1_4" )->FirstChildElement(), context );

	auto pSpider = pXml->FirstChildElement( "spider" );
	if( pSpider )
		m_pSpiderNode = CreateNode( pSpider->FirstChildElement(), context );
	auto pShop = pXml->FirstChildElement( "shop" );
	if( pShop )
		m_pShopNode = CreateNode( pShop->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_3::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenBase();
	GenRestWallChunks();
	GenWallChunk();
	GenWallChunk1();
	GenWindows();
	GenShops();

	context.mapTags["1"] = eType_WallChunk1_1;
	context.mapTags["2"] = eType_WallChunk1_2;
	for( auto& chunk : m_wallChunks1 )
	{
		for( int i = chunk.x; i < chunk.GetRight(); i++ )
		{
			for( int j = chunk.y; j < chunk.GetBottom(); j++ )
			{
				int32 x = region.x + i;
				int32 y = region.y + j;
				int8 genData = m_gendata[i + j * region.width];
				context.blueprint[x + y * context.nWidth] = genData;
			}
		}
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunk1Node->Generate( context, rect );
	}

	GenObjs();
	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData == eType_Wall || genData == eType_Crate1 || genData == eType_Crate2 )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["0"] = eType_WallChunk_0;
	context.mapTags["0_1"] = eType_WallChunk_0_1;
	context.mapTags["0_1a"] = eType_WallChunk_0_1a;
	context.mapTags["0_1b"] = eType_WallChunk_0_1b;
	context.mapTags["1"] = eType_WallChunk_1;
	//context.mapTags["2"] = eType_WallChunk_2;
	context.mapTags["3"] = eType_WallChunk_3;
	for( auto& chunk : m_wallChunks )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunkNode->Generate( context, rect );
	}
	for( auto& window : m_windows )
	{
		auto rect = window.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWindowNode->Generate( context, rect );
	}
	for( int i = 0; i < 4; i++ )
	{
		for( auto& window : m_windows1[i] )
		{
			auto rect = window.Offset( TVector2<int32>( region.x, region.y ) );
			m_pWindow1Node[i]->Generate( context, rect );
		}
	}

	context.mapTags["1"] = eType_Crate1;
	context.mapTags["2"] = eType_Crate2;
	context.mapTags["3"] = eType_Crate3;
	for( auto& scrap : m_scraps )
	{
		auto rect = scrap.Offset( TVector2<int32>( region.x, region.y ) );
		m_pScrapNode->Generate( context, rect );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				context.blueprint[i + j * context.nWidth] = eType_Temp;
			}
		}
	}

	context.mapTags["mask"] = eType_BlockRed;
	m_pBlock1Node->Generate( context, region );
	context.mapTags["mask"] = eType_BlockBlue;
	m_pBlock2Node->Generate( context, region );
	context.mapTags["mask"] = eType_Crate1;
	m_pCrate1Node->Generate( context, region );
	context.mapTags["mask"] = eType_Crate2;
	m_pCrate2Node->Generate( context, region );

	context.mapTags["door"] = eType_Door;
	context.mapTags["mask"] = eType_Web;
	context.mapTags["room"] = eType_Room;
	context.mapTags["pipe0"] = eType_Room_Pipe0;
	context.mapTags["pipe1"] = eType_Room_Pipe1;
	context.mapTags["pipe2"] = eType_Room_Pipe2;
	context.mapTags["pipe3"] = eType_Room_Pipe3;
	for( auto& room : m_rooms )
	{
		auto rect = room.rect.Offset( TVector2<int32>( region.x, region.y ) );
		m_pRoomNode->Generate( context, rect );
	}
	for( auto& chunk : m_wallChunks2 )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunk2Node->Generate( context, rect );
	}
	context.mapTags["block"] = eType_WallChunk0_0;
	context.mapTags["block_x"] = eType_WallChunk0_0x;
	context.mapTags["block_y"] = eType_WallChunk0_0y;
	context.mapTags["1"] = eType_Maggot;
	for( auto& chunk : m_wallChunks0 )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunk0Node->Generate( context, rect );
	}
	for( auto& rect : m_spiders )
	{
		m_pSpiderNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& shop : m_shops )
	{
		TRectangle<int32> rect = shop;
		rect.x = SRand::Inst().Rand( 1, rect.width - 6 ) + rect.x;
		rect.y = SRand::Inst().Rand( 1, rect.height - 4 ) + rect.y;
		rect.width = 6;
		rect.height = 4;
		m_pShopNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_rooms.clear();
	m_wallChunks.clear();
	m_wallChunks0.clear();
	m_wallChunks1.clear();
	m_wallChunks1_1.clear();
	m_wallChunks2.clear();
	m_scraps.clear();
	m_windows.clear();
	m_windows1[0].clear();
	m_windows1[1].clear();
	m_windows1[2].clear();
	m_windows1[3].clear();
	m_spiders.clear();
	m_shops.clear();
}

void CLevelGenNode1_3::GenBase()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	int32 minSize[] = { 10, 10, 18, 20, 34, 36, 36, 36 };
	int32 maxSize[] = { 12, 12, 22, 26, 38, 42, 42, 42 };

	vector<TVector2<int32> > vec;
	int32 nLen = nHeight;
	int32 nMinType = m_nType ? 0 : 2;
	while( nLen )
	{
		int32 nType = ELEM_COUNT( minSize );
		do
		{
			nType = SRand::Inst().Rand( nMinType, nType );
		} while( nType > nMinType && minSize[nType] > nLen );
		if( minSize[nType] > nLen )
			break;

		nLen -= minSize[nType];
		vec.push_back( TVector2<int32>( nType, minSize[nType] ) );
	}

	int32 iPos = 0;
	int32 i1 = vec.size();
	while( nLen )
	{
		if( vec[iPos].y < maxSize[vec[iPos].x] )
		{
			vec[iPos].y++;
			nLen--;
			i1 = vec.size();
		}
		else
		{
			i1--;
			if( !i1 )
				break;
		}
		iPos++;
		if( iPos >= vec.size() )
			iPos = 0;
	}

	while( nLen )
	{
		vec[iPos].y++;
		nLen--;
		iPos++;
		if( iPos >= vec.size() )
			iPos = 0;
	}
	SRand::Inst().Shuffle( vec );

	vector<int8> temp;
	vector<TVector2<int32> > temp1;
	temp.reserve( 200 );
	temp1.reserve( 200 );
	TRectangle<int32> lastRect( 8, 0, 16, 0 );
	int32 nLastRect = -1;
	int32 nCurBegin = 0;
	for( auto p : vec )
	{
		uint8 nState = p.x;
		int32 nCurHeight = p.y;
		uint32 nRoomCount = m_rooms.size();

		switch( nState )
		{
		case 0:
		{
			//xxxxxxxxxxxxxxxxxxxxxx
			//xxxxxx
			//oooooooooooooooooooooo
			//----------------------
			//xxxxxxxxxxxxxxxxxxxxxx
			//                xxxxxx
			//oooooooooooooooooooooo

			int32 w = SRand::Inst().Rand( 5, 8 );
			int32 left = SRand::Inst().Rand( -2, 3 ) + lastRect.x;
			left = Min( Max( left, 4 ), 8 );
			int32 right = SRand::Inst().Rand( -2, 3 ) + lastRect.GetRight();
			right = Max( Min( right, nWidth - 4 ), nWidth - 8 );
			int8 nType = SRand::Inst().Rand( 0, 3 );
			int32 nHeight1 = nCurHeight - SRand::Inst().Rand( 3, 5 );

			if( SRand::Inst().Rand( 0, 2 ) )
			{
				right = nWidth;
				if( nType == 0 )
				{
					m_wallChunks0.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( lastRect.x, nCurBegin + nHeight1, right - lastRect.x, nCurHeight - nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks0.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks0.push_back( TRectangle<int32>( left, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
				}
			}
			else
			{
				left = 0;
				if( nType == 0 )
				{
					m_wallChunks0.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, lastRect.GetRight() - left, nCurHeight - nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks0.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks0.push_back( TRectangle<int32>( right - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
				}
			}

			break;
		}
		case 1:
		{
			//xxxxxxxxxxxxxxxxxxxxxx
			//xxxx              xxxx
			//oooooooooooooooooooooo

			int32 w1 = SRand::Inst().Rand( 4, 6 );
			int32 w2 = SRand::Inst().Rand( 4, 6 );
			int32 left = SRand::Inst().Rand( -4, 1 ) + lastRect.x;
			left = Min( Max( left, 4 ), 8 );
			int32 right = SRand::Inst().Rand( 0, 5 ) + lastRect.GetRight();
			right = Max( Min( right, nWidth - 4 ), nWidth - 8 );
			int8 nType = SRand::Inst().Rand( 0, 3 );
			int32 nHeight1 = nCurHeight - SRand::Inst().Rand( 3, 5 );

			if( nType == 0 )
			{
				m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w1, nHeight1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( lastRect.GetRight() - w2, nCurBegin, w2, nHeight1 ) );
				m_wallChunks1_1.push_back( TRectangle<int32>( lastRect.x, nCurBegin + nHeight1, lastRect.width, nCurHeight - nHeight1 ) );
			}
			else if( nType == 1 )
			{
				m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w1, nHeight1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( lastRect.GetRight() - w2, nCurBegin, w2, nHeight1 ) );
				m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
			}
			else if( nType == 2 )
			{
				m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin, w1, nHeight1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( right - w2, nCurBegin, w2, nHeight1 ) );
				m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
			}

			break;
		}
		case 2:
		{
			//xxxxxxxxxxxxxxxxxxxxxx
			//xxxx      ..        ..
			//xxxx      ..        ..
			//oooooooooooooooooooooo
			//----------------------
			//xxxxxxxxxxxxxxxxxxxxxx
			//..       ..       xxxx
			//..       ..       xxxx
			//oooooooooooooooooooooo

			int32 w = SRand::Inst().Rand( 4, 6 );
			int32 left = SRand::Inst().Rand( -2, 3 ) + lastRect.x;
			left = Min( Max( left, 4 ), 8 );
			int32 right = SRand::Inst().Rand( -2, 3 ) + lastRect.GetRight();
			right = Max( Min( right, nWidth - 4 ), nWidth - 8 );
			int8 nType = SRand::Inst().Rand( 0, 3 );
			int32 nHeight1 = nCurHeight - SRand::Inst().Rand( 4, 6 );

			if( SRand::Inst().Rand( 0, 2 ) )
			{
				if( nType == 0 )
				{
					m_wallChunks1_1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( lastRect.x, nCurBegin + nHeight1, right - lastRect.x, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( lastRect.x + w, nCurBegin, nWidth - lastRect.x - w, nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks1_1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( lastRect.x + w, nCurBegin, nWidth - lastRect.x - w, nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( left + w, nCurBegin, nWidth - left - w, nHeight1 ) );
				}
			}
			else
			{
				if( nType == 0 )
				{
					m_wallChunks1_1.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, lastRect.GetRight() - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( 0, nCurBegin, lastRect.GetRight() - w, nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks1_1.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( 0, nCurBegin, lastRect.GetRight() - w, nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks1_1.push_back( TRectangle<int32>( right - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( 0, nCurBegin, right - w, nHeight1 ) );
				}
			}

			break;
		}
		case 3:
		{
			//xxxxxxxxxxxxxxxxxxxxxx
			//    xxxx       ..
			//    ..............
			//     ..       xxxx
			//oooooooooooooooooooooo
			//----------------------
			//xxxxxxxxxxxxxxxxxxxxxx
			//     ..       xxxx
			//    ..............
			//    xxxx       ..
			//oooooooooooooooooooooo

			int32 w1 = SRand::Inst().Rand( 4, 6 );
			int32 w2 = SRand::Inst().Rand( 4, 6 );
			int32 left = SRand::Inst().Rand( 1, 5 ) + lastRect.x;
			left = Min( Max( left, 4 ), nWidth / 2 - w1 );
			int32 right = SRand::Inst().Rand( -4, 0 ) + lastRect.GetRight();
			right = Max( Min( right, nWidth - 4 ), nWidth - nWidth / 2 + w2 );
			int32 h3 = SRand::Inst().Rand( 4, 6 );
			int32 h1 = SRand::Inst().Rand( 2, 4 );
			int32 n = nCurHeight - h3 - h1;
			int32 h0 = SRand::Inst().Rand( n / 2 - 1, n - n / 2 + 2 );
			int32 h2 = nCurHeight - h3 - h1 - h0;

			if( SRand::Inst().Rand( 0, 2 ) )
			{
				m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin, w1, h0 ) );
				m_wallChunks0.push_back( TRectangle<int32>( left, nCurBegin + h0, right - left, h1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( right - w2, nCurBegin + h0 + h1, w2, h2 ) );
				GenSubArea( TRectangle<int32>( 0, nCurBegin, left, nCurHeight - h3 ) );
				GenSubArea( TRectangle<int32>( left, nCurBegin + h0 + h1, right - w2 - left, h2 ) );
				GenSubArea( TRectangle<int32>( right, nCurBegin, nWidth - right, nCurHeight - h3 ) );
				GenSubArea( TRectangle<int32>( left + w1, nCurBegin, right - left - w1, h0 ) );
			}
			else
			{
				m_wallChunks1.push_back( TRectangle<int32>( right - w2, nCurBegin, w2, h0 ) );
				m_wallChunks0.push_back( TRectangle<int32>( left, nCurBegin + h0, right - left, h1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1, w1, h2 ) );
				GenSubArea( TRectangle<int32>( 0, nCurBegin, left, nCurHeight - h3 ) );
				GenSubArea( TRectangle<int32>( left, nCurBegin, right - w2 - left, h0 ) );
				GenSubArea( TRectangle<int32>( right, nCurBegin, nWidth - right, nCurHeight - h3 ) );
				GenSubArea( TRectangle<int32>( left + w1, nCurBegin + h0 + h1, right - left - w1, h2 ) );
			}
			left = Max( 0, left - SRand::Inst().Rand( 1, 5 ) );
			right = Min( nWidth, right + SRand::Inst().Rand( 1, 5 ) );
			m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1 + h2, right - left, h3 ) );

			break;
		}
		case 4:
		{
			int32 h3 = SRand::Inst().Rand( 4, 6 );
			TRectangle<int32> r0( 0, nCurBegin, nWidth, nCurHeight - h3 );
			TRectangle<int32> r( 0, 0, SRand::Inst().Rand( 7, 10 ), 0 );
			r.height = Max( 4, r.width + r0.height - r0.width );
			r.x = ( nWidth - r.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
			r.y = r0.y + ( r0.height - r.height + SRand::Inst().Rand( 0, 2 ) ) / 2;
			m_wallChunks0.push_back( r );
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				m_wallChunks1.push_back( TRectangle<int32>( r.x - 3, r0.y, 3, r.GetBottom() - r0.y ) );
				m_wallChunks1.push_back( TRectangle<int32>( r.GetRight(), r.y, 3, r0.GetBottom() - r.y ) );
				m_wallChunks1.push_back( TRectangle<int32>( 0, r.GetBottom(), r.GetRight(), 3 ) );
				m_wallChunks1.push_back( TRectangle<int32>( r.x, r.y - 3, r0.GetRight() - r.x, 3 ) );
				//GenSubArea( TRectangle<int32>( 0, r0.y, r.x - 3, r.GetBottom() - r0.y ) );
				//GenSubArea( TRectangle<int32>( r.GetRight() + 3, r.y, r0.GetRight() - r.GetRight() - 3, r0.GetBottom() - r.y ) );
				//GenSubArea( TRectangle<int32>( 0, r.GetBottom() + 3, r.GetRight(), r0.GetBottom() - r.GetBottom() - 3 ) );
				//GenSubArea( TRectangle<int32>( r.x, r0.y, r0.GetRight() - r.x, r.y - r0.y - 3 ) );
			}
			else
			{
				m_wallChunks1.push_back( TRectangle<int32>( r.x - 3, r.y, 3, r0.GetBottom() - r.y ) );
				m_wallChunks1.push_back( TRectangle<int32>( r.GetRight(), r0.y, 3,  r.GetBottom() - r0.y) );
				m_wallChunks1.push_back( TRectangle<int32>( r.x, r.GetBottom(), r0.GetRight() - r.x, 3 ) );
				m_wallChunks1.push_back( TRectangle<int32>( 0, r.y - 3, r.GetRight(), 3 ) );
				//GenSubArea( TRectangle<int32>( 0, r.y, r.x - 3, r0.GetBottom() - r.y ) );
				//GenSubArea( TRectangle<int32>( r.GetRight() + 3, r0.y, r0.GetRight() - r.GetRight() - 3, r.GetBottom() - r0.y ) );
				//GenSubArea( TRectangle<int32>( r.x, r.GetRight(), r0.GetRight() - r.x, r0.GetBottom() - r.GetBottom() - 3 ) );
				//GenSubArea( TRectangle<int32>( 0, r.GetBottom() + 3, r0.y, r.y - r0.y - 3 ) );
			}

			int32 left = SRand::Inst().Rand( 4, 6 );
			int32 right = nWidth - SRand::Inst().Rand( 4, 6 );
			m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nCurHeight - h3, right - left, h3 ) );
			break;
		}
		case 5:
		{
			//xxxxxxxxxxxxxxxxxxxxxx
			//..      xxxx        ..
			//..      xxxx        ..
			//xxxxxxx******xxxxxxxxx
			//..      xxxx        ..
			//..      xxxx        ..
			//oooooooooooooooooooooo

			int32 w = SRand::Inst().Rand( 4, 6 );
			int32 n = nWidth - w;
			int32 left = SRand::Inst().Rand( n / 2 - 2, n - n / 2 + 3 );
			int32 left1 = left - SRand::Inst().Rand( 1, 3 );
			int32 w1 = left + w + SRand::Inst().Rand( 1, 3 ) - left1;

			int32 h3 = SRand::Inst().Rand( 4, 6 );
			int32 h1 = SRand::Inst().Rand( 6, 7 );
			n = nCurHeight - h3 - h1;
			int32 h0 = SRand::Inst().Rand( n / 2 - 1, n - n / 2 + 2 );
			int32 h2 = nCurHeight - h3 - h1 - h0;

			m_wallChunks0.push_back( TRectangle<int32>( left, nCurBegin, w, h0 ) );
			GenSubArea( TRectangle<int32>( 0, nCurBegin, left, h0 ) );
			GenSubArea( TRectangle<int32>( left + w, nCurBegin, nWidth - left - w, h0 ) );

			SRoom room( TRectangle<int32>( left1, nCurBegin + h0, w1, h1 ) );
			room.bDoor[2] = room.bDoor[3] = true;
			m_rooms.push_back( room );
			m_wallChunks1_1.push_back( TRectangle<int32>( 0, nCurBegin + h0, left1, h1 ) );
			m_wallChunks1_1.push_back( TRectangle<int32>( left1 + w1, nCurBegin + h0, nWidth - left1 - w1, h1 ) );

			m_wallChunks0.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1, w, h2 ) );
			GenSubArea( TRectangle<int32>( 0, nCurBegin + h0 + h1, left, h2 ) );
			GenSubArea( TRectangle<int32>( left + w, nCurBegin + h0 + h1, nWidth - left - w, h2 ) );

			left = SRand::Inst().Rand( 4, 6 );
			int32 right = nWidth - SRand::Inst().Rand( 4, 6 );
			m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1 + h2, right - left, h3 ) );

			if( nLastRect >= 0 )
				FillLine( m_wallChunks1_1[nLastRect] );
			FillLine( m_wallChunks1_1.back() );

			break;
		}
		case 6:
		{
			//xxxxxxxxxxxxxxxxxxxxxx
			//..      ....        ..
			//..      ....        ..
			//****xxxxxxxxxxxxxx****
			//..      ....        ..
			//..      ....        ..
			//oooooooooooooooooooooo
			int32 w1 = SRand::Inst().Rand( 5, 7 );
			int32 w2 = SRand::Inst().Rand( 5, 7 );
			int32 h3 = SRand::Inst().Rand( 4, 6 );
			int32 h1 = SRand::Inst().Rand( 6, 7 );
			int32 n = nCurHeight - h3 - h1;
			int32 h0 = SRand::Inst().Rand( n / 2 - 1, n - n / 2 + 2 );
			int32 h2 = nCurHeight - h3 - h1 - h0;

			int32 w00 = 4;
			int32 w01 = 4;
			int32 w10 = 4;
			int32 w11 = 4;

			m_wallChunks0.push_back( TRectangle<int32>( 0, nCurBegin, w00, h0 ) );
			m_wallChunks0.push_back( TRectangle<int32>( nWidth - w01, nCurBegin, w01, h0 ) );
			GenSubArea( TRectangle<int32>( w00, nCurBegin, nWidth - w00 - w01, h0 ) );

			SRoom room1( TRectangle<int32>( 0, nCurBegin + h0, w1, h1 ) );
			room1.bDoor[1] = true;
			m_rooms.push_back( room1 );
			SRoom room2( TRectangle<int32>( nWidth - w2, nCurBegin + h0, w2, h1 ) );
			room2.bDoor[0] = true;
			m_rooms.push_back( room2 );
			m_wallChunks1_1.push_back( TRectangle<int32>( w1, nCurBegin + h0, nWidth - w2 - w1, h1 ) );

			m_wallChunks0.push_back( TRectangle<int32>( 0, nCurBegin + h0 + h1, w10, h2 ) );
			m_wallChunks0.push_back( TRectangle<int32>( nWidth - w11, nCurBegin + h0 + h1, w11, h2 ) );
			GenSubArea( TRectangle<int32>( w10, nCurBegin + h0 + h1, nWidth - w10 - w11, h2 ) );

			int32 left = SRand::Inst().Rand( 4, 6 );
			int32 right = nWidth - SRand::Inst().Rand( 4, 6 );
			m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1 + h2, right - left, h3 ) );

			if( nLastRect >= 0 )
				FillLine( m_wallChunks1_1[nLastRect] );
			FillLine( m_wallChunks1_1.back() );

			break;
		}
		case 7:
		{
			int32 h3 = SRand::Inst().Rand( 4, 6 );
			TRectangle<int32> r0( 0, nCurBegin, nWidth, nCurHeight - h3 );
			TRectangle<int32> r( 0, 0, SRand::Inst().Rand( 16, 25 ), 0 );
			r.height = r.width - SRand::Inst().Rand( 0, 3 );
			r.x = ( nWidth - r.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
			r.y = r0.y + ( r0.height - r.height + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 w1 = ( r0.width - 4 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 h1 = ( r0.height - 4 + SRand::Inst().Rand( 0, 2 ) ) / 2;

			int32 w2;
			if( r.width <= 19 + SRand::Inst().Rand( 0, 2 ) )
				w2 = 2;
			else if( r.width <= 21 + SRand::Inst().Rand( 0, 2 ) )
				w2 = 3;
			else
				w2 = 4;
			auto& chunk0 = w2 < 4 ? m_wallChunks0 : m_wallChunks1;
			auto& chunk1 = w2 < 4 ? m_wallChunks1 : m_wallChunks0;

			chunk0.push_back( TRectangle<int32>( w1, r0.y, 4, r.y - r0.y ) );
			chunk0.push_back( TRectangle<int32>( w1, r.GetBottom(), 4, r0.GetBottom() - r.GetBottom() ) );
			chunk0.push_back( TRectangle<int32>( 0, r0.y + h1, r.x, 4 ) );
			chunk0.push_back( TRectangle<int32>( r.GetRight(), r0.y + h1, r0.GetRight() - r.GetRight(), 4 ) );
			GenSubArea( TRectangle<int32>( 0, r0.y, r.x, h1 ) );
			GenSubArea( TRectangle<int32>( r.x, r0.y, w1 - r.x, r.y - r0.y ) );
			GenSubArea( TRectangle<int32>( w1 + 4, r0.y, r.GetRight() - w1 - 4, r.y - r0.y ) );
			GenSubArea( TRectangle<int32>( r.GetRight(), r0.y, nWidth - r.GetRight(), h1 ) );
			GenSubArea( TRectangle<int32>( 0, r0.y + h1 + 4, r.x, r0.height - h1 - 4 ) );
			GenSubArea( TRectangle<int32>( r.x, r.GetBottom(), w1 - r.x, r0.GetBottom() - r.GetBottom() ) );
			GenSubArea( TRectangle<int32>( w1 + 4, r.GetBottom(), r.GetRight() - w1 - 4, r0.GetBottom() - r.GetBottom() ) );
			GenSubArea( TRectangle<int32>( r.GetRight(), r0.y + h1 + 4, nWidth - r.GetRight(), r0.height - h1 - 4 ) );

			chunk1.push_back( TRectangle<int32>( r.x, r.y, r.width, w2 ) );
			chunk1.push_back( TRectangle<int32>( r.x, r.GetBottom() - w2, r.width, w2 ) );
			chunk1.push_back( TRectangle<int32>( r.x, r.y + w2, w2, r.height - w2 * 2 ) );
			chunk1.push_back( TRectangle<int32>( r.GetRight() - w2, r.y + w2, w2, r.height - w2 * 2 ) );
			GenSubArea( TRectangle<int32>( r.x + w2, r.y + w2, r.width - w2 * 2, r.height - w2 * 2 ) );

			int32 left = SRand::Inst().Rand( 4, 6 );
			int32 right = nWidth - SRand::Inst().Rand( 4, 6 );
			m_wallChunks1_1.push_back( TRectangle<int32>( left, nCurBegin + nCurHeight - h3, right - left, h3 ) );

			if( nLastRect >= 0 )
				FillLine( m_wallChunks1_1[nLastRect] );
			break;
		}
		}

		nCurBegin += nCurHeight;
		nLastRect = m_wallChunks1_1.size() - 1;
		lastRect = m_wallChunks1_1.back();
	}

	for( int iRoom = 0; iRoom < m_rooms.size(); iRoom++ )
	{
		auto& room = m_rooms[iRoom].rect;

		int8& l = m_rooms[iRoom].bDoor[0];
		int8& r = m_rooms[iRoom].bDoor[1];
		int8& t = m_rooms[iRoom].bDoor[2];
		int8& b = m_rooms[iRoom].bDoor[3];

		{
			int32 x[] = { room.x + 2, room.x + ( room.width + SRand::Inst().Rand( 0, 2 ) ) / 2, room.GetRight() - 2 };
			int32 y[] = { room.y + 2, room.y + ( room.height + SRand::Inst().Rand( 0, 2 ) ) / 2, room.GetBottom() - 2 };
			TRectangle<int32> rect( x[l - r + 1] - 1, y[t - b + 1] - 1, 2, 2 );
			m_spiders.push_back( rect );
			for( int i = room.x; i < room.GetRight(); i++ )
			{
				for( int j = room.y; j < room.GetBottom(); j++ )
				{
					if( i > room.x && i < room.GetRight() - 1 && j > room.y && j < room.GetBottom() - 1 )
					{
						TVector2<int32> dist( Max( rect.x - i, i - ( rect.GetRight() - 1 ) ), Max( rect.y - j, j - ( rect.GetBottom() - 1 ) ) );
						if( !SRand::Inst().Rand( 0, dist.Length2() + 1 ) )
							m_gendata[i + j * nWidth] = eType_Room1;
						else
							m_gendata[i + j * nWidth] = eType_Web;
					}
					else
						m_gendata[i + j * nWidth] = eType_Room1;
				}
			}
		}

		int32 nCracks = 3 + (int32)( ( room.width + room.height - 4 ) * SRand::Inst().Rand( 0.25f, 0.3f ) );
		temp1.resize( 0 );
		for( int i = room.x + 1; i < room.GetRight() - 1; i++ )
		{
			if( m_gendata[i + ( room.y + 1 ) * nWidth] == eType_Web )
				temp1.push_back( TVector2<int32>( i, room.y ) );
			if( m_gendata[i + ( room.GetBottom() - 2 ) * nWidth] == eType_Web )
				temp1.push_back( TVector2<int32>( i, room.GetBottom() - 1 ) );
		}
		for( int j = room.y + 1; j < room.GetBottom() - 1; j++ )
		{
			if( m_gendata[room.x + 1 + j * nWidth] == eType_Web )
				temp1.push_back( TVector2<int32>( room.x, j ) );
			if( m_gendata[room.GetRight() - 2 + j * nWidth] == eType_Web )
				temp1.push_back( TVector2<int32>( room.GetRight() - 1, j ) );
		}
		SRand::Inst().Shuffle( temp1 );
		nCracks = Min<int32>( nCracks, temp1.size() );
		for( int k = 0; k < nCracks; k++ )
		{
			auto& p = temp1[k];
			m_gendata[p.x + p.y * nWidth] = eType_Web;
		}

		int32 n = room.width - 2;
		int32 x, y;
		if( t )
		{
			x = SRand::Inst().Rand( n / 2, n - n / 2 + 1 ) + room.x;
			int8 nDoorType = eType_Web;
			if( nDoorType == eType_Door )
			{
				for( int x1 = x - 1; x1 < x + 3; x1++ )
				{
					if( m_gendata[x1 + room.y * nWidth] == eType_Web )
					{
						nDoorType = eType_Web;
						break;
					}
				}
			}
			m_gendata[x + room.y * nWidth] = m_gendata[x + 1 + room.y * nWidth] = nDoorType;
		}
		if( b )
		{
			x = SRand::Inst().Rand( n / 2, n - n / 2 + 1 ) + room.x;
			int8 nDoorType = eType_Web;
			if( nDoorType == eType_Door )
			{
				for( int x1 = x - 1; x1 < x + 3; x1++ )
				{
					if( m_gendata[x1 + ( room.GetBottom() - 1 ) * nWidth] == eType_Web )
					{
						nDoorType = eType_Web;
						break;
					}
				}
			}
			m_gendata[x + ( room.GetBottom() - 1 ) * nWidth] = m_gendata[x + 1 + ( room.GetBottom() - 1 ) * nWidth] = nDoorType;
		}

		n = room.height - 2;
		if( l )
		{
			y = SRand::Inst().Rand( n / 2, n - n / 2 + 1 ) + room.y;
			int8 nDoorType = eType_Web;
			if( nDoorType == eType_Door )
			{
				for( int y1 = y - 1; y1 < y + 3; y1++ )
				{
					if( m_gendata[room.x + y1 * nWidth] == eType_Web )
					{
						nDoorType = eType_Web;
						break;
					}
				}
			}
			m_gendata[room.x + y * nWidth] = m_gendata[room.x + ( y + 1 ) * nWidth] = nDoorType;
		}
		if( r )
		{
			y = SRand::Inst().Rand( n / 2, n - n / 2 + 1 ) + room.y;
			int8 nDoorType = eType_Web;
			if( nDoorType == eType_Door )
			{
				for( int y1 = y - 1; y1 < y + 3; y1++ )
				{
					if( m_gendata[( room.GetRight() - 1 ) + y1 * nWidth] == eType_Web )
					{
						nDoorType = eType_Web;
						break;
					}
				}
			}
			m_gendata[room.GetRight() - 1 + y * nWidth] = m_gendata[room.GetRight() - 1 + ( y + 1 ) * nWidth] = nDoorType;
		}

		for( int y = room.y + 1; y < room.GetBottom() - 1; y++ )
		{
			int32 nType;
			if( l - r < 0 )
				nType = eType_Room_Pipe0;
			else if( l - r > 0 )
				nType = eType_Room_Pipe2;
			else
				nType = SRand::Inst().Rand( 0, 2 ) ? eType_Room_Pipe0 : eType_Room_Pipe2;
			int32 x = nType == eType_Room_Pipe0 ? room.x + 1 : room.GetRight() - 2;
			int32 x1 = nType == eType_Room_Pipe0 ? room.x : room.GetRight() - 1;
			if( m_gendata[x1 + y * nWidth] == eType_Door || m_gendata[x1 + y * nWidth] == eType_Web )
				continue;
			if( m_gendata[x + y * nWidth] != eType_Web || !SRand::Inst().Rand( 0, 3 ) )
				m_gendata[x + y * nWidth] = nType;
		}
		for( int x = room.x + 1; x < room.GetRight() - 1; x++ )
		{
			int32 nType;
			if( t - b < 0 )
				nType = eType_Room_Pipe1;
			else if( t - b > 0 )
				nType = eType_Room_Pipe3;
			else
				nType = SRand::Inst().Rand( 0, 2 ) ? eType_Room_Pipe1 : eType_Room_Pipe3;
			int32 y = nType == eType_Room_Pipe1 ? room.y + 1 : room.GetBottom() - 2;
			int32 y1 = nType == eType_Room_Pipe1 ? room.y : room.GetBottom() - 1;
			if( m_gendata[x + y1 * nWidth] == eType_Door || m_gendata[x + y1 * nWidth] == eType_Web )
				continue;
			if( m_gendata[x + y * nWidth] != eType_Web || !SRand::Inst().Rand( 0, 2 ) )
				m_gendata[x + y * nWidth] = nType;
		}
	}

	for( int i = m_wallChunks0.size() - 1; i >= 0; i-- )
	{
		auto rect = m_wallChunks0[i];
		if( Min( rect.width, rect.height ) < 4 )
		{
			m_wallChunks1.push_back( rect );
			m_wallChunks0[i] = m_wallChunks0.back();
			m_wallChunks0.pop_back();
		}
		else
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
				for( int j = rect.y; j < rect.GetBottom(); j++ )
					m_gendata[i + j * nWidth] = eType_WallChunk0;
		}
	}
	for( auto& rect : m_wallChunks )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = eType_WallChunk;
	}
	for( auto& rect : m_wallChunks1_1 )
		m_wallChunks1.push_back( rect );
	for( auto& rect : m_wallChunks1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = eType_WallChunk1;
	}
}

void CLevelGenNode1_3::GenSubArea( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 nMinAreaSize = 5;
	vector<TRectangle<int32> > rects;
	rects.push_back( rect );

	for( int i = 0; i < rects.size(); i++ )
	{
		auto curRect = rects[i];
		if( curRect.width < nMinAreaSize * 2 + 2 && curRect.height < nMinAreaSize * 2 + 2 )
		{
			if( m_nType )
				m_wallChunks.push_back( curRect );
			else
				m_rooms.push_back( curRect );
			continue;
		}
		if( m_nType && Max( curRect.height, curRect.width ) < 14 )
		{
			float f = Min( curRect.height, curRect.width ) / 13.0f;
			if( SRand::Inst().Rand( 0.0f, 1.0f ) >= f )
			{
				m_wallChunks.push_back( curRect );
				continue;
			}
		}

		bool bVertical = curRect.height > curRect.width;
		bool bFromLeft = SRand::Inst().Rand( 0, 1 );
		int32 nSplitWidth = Min( Max( curRect.height, curRect.width ) - nMinAreaSize * 2, SRand::Inst().Rand( 1, 5 ) );
		if( nSplitWidth > 1 && nSplitWidth < 4 )
			nSplitWidth = 2;
		int32 nSplitLen = SRand::Inst().Rand( nMinAreaSize, ( Max( curRect.height, curRect.width ) - nSplitWidth ) / 2 );

		TRectangle<int32> rect1, rect2, rectSplit;
		if( !bVertical )
		{
			rect1 = TRectangle<int32>( curRect.x, curRect.y, nSplitLen, curRect.height );
			rect2 = TRectangle<int32>( curRect.x + nSplitLen + nSplitWidth, curRect.y, curRect.width - nSplitLen - nSplitWidth, curRect.height );
			rectSplit = TRectangle<int32>( curRect.x + nSplitLen, curRect.y, nSplitWidth, curRect.height );
			if( !bFromLeft )
			{
				rect1.x = curRect.x + curRect.GetRight() - rect1.GetRight();
				rect2.x = curRect.x + curRect.GetRight() - rect2.GetRight();
				rectSplit.x = curRect.x + curRect.GetRight() - rectSplit.GetRight();
			}
		}
		else
		{
			rect1 = TRectangle<int32>( curRect.x, curRect.y, curRect.width, nSplitLen );
			rect2 = TRectangle<int32>( curRect.x, curRect.y + nSplitLen + nSplitWidth, curRect.width, curRect.height - nSplitLen - nSplitWidth );
			rectSplit = TRectangle<int32>( curRect.x, curRect.y + nSplitLen, curRect.width, nSplitWidth );
			if( !bFromLeft )
			{
				rect1.y = curRect.y + curRect.GetBottom() - rect1.GetBottom();
				rect2.y = curRect.y + curRect.GetBottom() - rect2.GetBottom();
				rectSplit.y = curRect.y + curRect.GetBottom() - rectSplit.GetBottom();
			}
		}

		rects.push_back( rect1 );
		rects.push_back( rect2 );
		m_wallChunks0.push_back( rectSplit );
	}
}

void CLevelGenNode1_3::FillLine( TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	rect.x = 0;
	rect.width = nWidth;
}

void CLevelGenNode1_3::GenRestWallChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	for( int i = 1; i < nWidth - 1; i++ )
	{
		for( int j = 1; j < nHeight; j++ )
		{
			int32 nType = m_gendata[i + j * nWidth];
			int32 nType1 = m_gendata[i - 1 + j * nWidth];
			int32 nType2 = m_gendata[i + 1 + j * nWidth];
			int32 nType3 = m_gendata[i + ( j - 1 ) * nWidth];
			if( nType == eType_None && ( nType1 == eType_WallChunk1 || nType2 == eType_WallChunk1 ) && nType3 == eType_WallChunk1 )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );

	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] == eType_None )
		{
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 4 ), TVector2<int32>( 32, 16 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_WallChunk );
			if( rect.width )
				m_wallChunks.push_back( rect );
		}
	}

	vec.clear();
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] == eType_None )
		{
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 4 ), TVector2<int32>( 32, 16 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_WallChunk );
			if( rect.width )
			{
				bool bChunk = SRand::Inst().Rand( 0, 5 ) < rect.width + rect.height - 10;
				if( bChunk )
					m_wallChunks.push_back( rect );
				else
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							m_gendata[x + y * nWidth] = eType_Wall;
						}
					}
				}
			}
		}
	}

	for( int i = 0; i < m_gendata.size(); i++ )
	{
		if( m_gendata[i] == eType_None )
			m_gendata[i] = eType_Wall;
	}

	for( int i = 0; i < m_wallChunks.size(); i++ )
	{
		if( m_wallChunks[i].width >= 14 )
		{
			int32 w = SRand::Inst().Rand( 7, m_wallChunks[i].width - 6 );
			TRectangle<int32> r = m_wallChunks[i];
			m_wallChunks[i].width = w;
			r.SetLeft( r.x + w );
			m_wallChunks.push_back( r );
		}
		if( m_wallChunks[i].height >= 10 )
		{
			int32 h1 = Max( m_wallChunks[i].height - 2, Min( m_wallChunks[i].height, SRand::Inst().Rand( 12, 14 ) ) );
			int32 h = SRand::Inst().Rand( 4, Min( 8, h1 - 3 ) );
			TRectangle<int32> r = m_wallChunks[i];
			m_wallChunks[i].height = h;
			r.SetTop( r.GetBottom() - h1 + h );
			m_wallChunks.push_back( r );
			TRectangle<int32> r1( r.x, m_wallChunks[i].GetBottom(), r.width, 0 );
			r1.SetBottom( r.y );
			if( r1.height )
			{
				m_wallChunks1.push_back( r1 );
				for( int x = r1.x; x < r1.GetRight(); x++ )
				{
					for( int y = r1.y; y < r1.GetBottom(); y++ )
					{
						m_gendata[x + y * nWidth] = eType_WallChunk1;
					}
				}
			}
		}
	}
}

void CLevelGenNode1_3::GenWallChunk1()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( auto rect : m_wallChunks1_1 )
	{
		if( rect.width < 2 || rect.height < 2 )
			continue;
		int32 s = SRand::Inst().Rand( rect.width * rect.height / 6, rect.width * rect.height / 5 );
		if( s < 4 )
			continue;
		uint8 nType = SRand::Inst().Rand( 0, 6 );
		if( ( nType == 4 || nType == 5 ) && Min( rect.width, rect.height ) < 5 )
			nType = SRand::Inst().Rand( 0, 4 );

		if( nType == 0 )
		{
		}
		else if( nType == 1 )
		{
			vector<TVector2<int32> > vec;
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				if( !SRand::Inst().Rand( 0, 3 ) )
				{
					m_gendata[i + rect.y * nWidth] = eType_WallChunk1_1;
					s--;
				}
				if( !SRand::Inst().Rand( 0, 3 ) )
				{
					m_gendata[i + ( rect.GetBottom() - 1 ) * nWidth] = eType_WallChunk1_1;
					s--;
				}
			}
			for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
			{
				if( !SRand::Inst().Rand( 0, 3 ) )
				{
					m_gendata[rect.x + j * nWidth] = eType_WallChunk1_1;
					s--;
				}
				if( !SRand::Inst().Rand( 0, 3 ) )
				{
					m_gendata[( rect.GetRight() - 1 ) + j * nWidth] = eType_WallChunk1_1;
					s--;
				}
			}
		}
		if( nType == 4 || nType == 5 )
		{
			if( nType == 5 )
			{
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[i + rect.y * nWidth] = eType_WallChunk1_1;
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[i + ( rect.GetBottom() - 1 ) * nWidth] = eType_WallChunk1_1;
				}
				for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
				{
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[rect.x + j * nWidth] = eType_WallChunk1_1;
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[( rect.GetRight() - 1 ) + j * nWidth] = eType_WallChunk1_1;
				}
				rect.x++;
				rect.width -= 2;
				rect.y++;
				rect.height -= 2;
			}
			else if( rect.width < rect.height + SRand::Inst().Rand( 0, 2 ) )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[rect.x + j * nWidth] = eType_WallChunk1_1;
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[( rect.GetRight() - 1 ) + j * nWidth] = eType_WallChunk1_1;
				}
				rect.x++;
				rect.width -= 2;
			}
			else
			{
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[i + rect.y * nWidth] = eType_WallChunk1_1;
					if( SRand::Inst().Rand( 0, 8 ) )
						m_gendata[i + ( rect.GetBottom() - 1 ) * nWidth] = eType_WallChunk1_1;
				}
				rect.y++;
				rect.height -= 2;
			}
			s = SRand::Inst().Rand( rect.width * rect.height / 7, rect.width * rect.height / 6 );
			nType = SRand::Inst().Rand( 0, 4 );
		}
		if( nType == 2 || nType == 3 )
		{
			int32 w = nType - 1;
			uint8 b = SRand::Inst().Rand( 0, 2 );
			if( rect.width < rect.height + SRand::Inst().Rand( 0, 2 ) )
			{
				for( int j = SRand::Inst().Rand( 1, rect.height / 2 ); j < rect.height - w && s > 0;
					j += ( nType == 2 ? SRand::Inst().Rand( 5, 8 ) : SRand::Inst().Rand( 6, 12 ) ) )
				{
					int32 y0 = b ? j + rect.y : rect.GetBottom() - w - j;
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = y0; y < y0 + w; y++ )
						{
							m_gendata[x + y * nWidth] = nType == 2 ? eType_WallChunk1_1 : eType_WallChunk1_2;
							s--;
						}
					}
				}
			}
			else
			{
				for( int i = SRand::Inst().Rand( 1, rect.width / 2 ); i < rect.width - w && s > 0;
					i += ( nType == 2 ? SRand::Inst().Rand( 5, 8 ) : SRand::Inst().Rand( 6, 12 ) ) )
				{
					int32 x0 = b ? i + rect.x : rect.GetRight() - w - i;
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						for( int x = x0; x < x0 + w; x++ )
						{
							m_gendata[x + y * nWidth] = nType == 2 ? eType_WallChunk1_1 : eType_WallChunk1_2;
							s--;
						}
					}
				}
			}
		}

		vector<TVector2<int32> > vec;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_WallChunk1 )
					vec.push_back( TVector2<int32>( i, j ) );
			}
		}
		SRand::Inst().Shuffle( vec );
		for( auto& p : vec )
		{
			if( s < 4 )
				break;
			auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 2, 2 ), rect, -1, eType_WallChunk1_2 );
			if( r.width <= 0 )
				continue;
			s -= 4;
		}
	}
}

namespace __LvGen1_3__
{
	void GenWallChunk( vector<int8>& genData, int32 nWidth, int32 nHeight, vector<TRectangle<int32> >& wallChunks, int32 nTypeWallChunk, int32 nType1, int32 nType0, int32 nType01, int32 nType01a, int32 nType01b, int32 nType2, int32 nType3, int32 nTypeTemp = -1 )
	{
		int32 nTypes[] = { 2, 0, 1 };
		for( auto& rect : wallChunks )
		{
			SRand::Inst().Shuffle( nTypes, ELEM_COUNT( nTypes ) );
			int32 nTypeCount = 3;
			int32 n1 = 0, n2 = 0;
			if( nTypeTemp >= 0 )
			{
				for( int x = rect.x + 1; x < rect.GetRight() - 1; x++ )
				{
					for( int y = rect.y; y < rect.GetBottom(); y += rect.height - 1 )
					{
						if( genData[x + y * nWidth] == nTypeTemp )
							n1++;
					}
				}
				for( int y = rect.y + 1; y < rect.GetBottom() - 1; y++ )
				{
					for( int x = rect.x; x < rect.GetRight(); x += rect.width - 1 )
					{
						if( genData[x + y * nWidth] == nTypeTemp )
							n2++;
					}
				}
				if( n1 && n2 )
				{
					if( n1 > n2 )
					{
						nTypes[0] = 0;
						nTypes[1] = 2;
						nTypes[2] = 1;
					}
					else
					{
						nTypes[0] = 2;
						nTypes[1] = 0;
						nTypes[2] = 1;
					}
				}
				else if( n1 )
				{
					nTypes[0] = 0;
					nTypes[1] = SRand::Inst().Rand( 0, 2 ) ? 1 : 2;
					nTypes[2] = 3 - nTypes[1];
				}
				else if( n2 )
				{
					nTypes[0] = 2;
					nTypes[1] = SRand::Inst().Rand( 0, 2 ) ? 1 : 0;
					nTypes[2] = 1 - nTypes[1];
				}
			}

			for( int i = 0; i < nTypeCount; i++ )
			{
				if( nTypes[i] == 0 )
				{
					if( nTypeTemp >= 0 && n1 > 0 )
					{
						if( rect.height < 4 || rect.width < rect.height + 1 )
							continue;
						int32 h0 = rect.y + SRand::Inst().Rand( 0, rect.height / 2 );
						if( genData[rect.x + rect.y * nWidth] == nTypeTemp && genData[rect.x + 1 + rect.y * nWidth] == nTypeTemp )
							genData[rect.x + 2 + rect.y * nWidth] = nTypeTemp;
						if( genData[rect.x + ( rect.GetBottom() - 1 ) * nWidth] == nTypeTemp && genData[rect.x + 1 + ( rect.GetBottom() - 1 ) * nWidth] == nTypeTemp )
							genData[rect.x + 2 + ( rect.GetBottom() - 1 ) * nWidth] = nTypeTemp;
						if( genData[rect.GetRight() - 1 + rect.y * nWidth] == nTypeTemp && genData[rect.GetRight() - 2 + rect.y * nWidth] == nTypeTemp )
							genData[rect.GetRight() - 3 + rect.y * nWidth] = nTypeTemp;
						if( genData[rect.GetRight() - 1 + ( rect.GetBottom() - 1 ) * nWidth] == nTypeTemp && genData[rect.GetRight() - 2 + ( rect.GetBottom() - 1 ) * nWidth] == nTypeTemp )
							genData[rect.GetRight() - 3 + ( rect.GetBottom() - 1 ) * nWidth] = nTypeTemp;

						for( int x = rect.x + 1; x < rect.GetRight() - 1; x++ )
						{
							if( genData[x + rect.y * nWidth] == nTypeTemp || genData[x + ( rect.GetBottom() - 1 ) * nWidth] == nTypeTemp )
							{
								for( int y = rect.y; y < rect.GetBottom(); y++ )
								{
									genData[x + y * nWidth] = y < h0 ? nType01 : nType01b;
								}
							}
						}
						break;
					}
					if( rect.width < 8 || rect.height < 4 )
						continue;

					n1 = 0;
					n2 = 0;
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						if( rect.y <= 0 || genData[x + ( rect.y - 1 ) * nWidth] != nTypeWallChunk )
							n1++;
						if( rect.GetBottom() >= nHeight || genData[x + ( rect.GetBottom() ) * nWidth] != nTypeWallChunk )
							n2++;
					}
					if( n1 * 2 < rect.width )
						continue;

					int32 w = SRand::Inst().Rand( Min( 2, rect.width / 5 ), rect.width / 3 );
					if( rect.height <= 6 && rect.width < SRand::Inst().Rand( 6, 13 ) )
						w = Max( w, SRand::Inst().Rand( 4, 7 ) );

					TRectangle<int32> r = rect;
					r.width = w;
					r.height = Min( SRand::Inst().Rand( 4, 8 ), n2 * 2 < rect.width || w > 2 ? r.height - 1 : r.height );
					if( SRand::Inst().Rand( 0, 2 ) && w <= 2 && rect.width > 10 )
					{
						int32 w1 = Min( rect.width - 2 - r.width * 2, SRand::Inst().Rand( 4, 6 ) );
						int32 n = ( rect.width - 2 - r.width ) / ( r.width + w1 );
						int32 w0 = ( r.width + w1 ) * n + r.width;
						r.x += SRand::Inst().Rand( 1, rect.width - w0 );
						for( int k = 0; k < n; k++ )
						{
							if( r.width > 1 )
							{
								TRectangle<int32> r1 = r;
								r1.height = SRand::Inst().Rand( 0, r1.height / 2 );
								for( int x = r.x; x < r.GetRight(); x++ )
									for( int y = r.y; y < r.GetBottom(); y++ )
										genData[x + y * nWidth] = nType01b;
								for( int x = r1.x; x < r1.GetRight(); x++ )
									for( int y = r1.y; y < r1.GetBottom(); y++ )
										genData[x + y * nWidth] = nType01;
							}
							else
							{
								for( int x = r.x; x < r.GetRight(); x++ )
									for( int y = r.y; y < r.GetBottom(); y++ )
										genData[x + y * nWidth] = nType01;
							}
							r.x += r.width + w1;
						}
					}
					else
					{
						r.x += SRand::Inst().Rand( 1, rect.width - w );
						if( r.width > 1 )
						{
							TRectangle<int32> r1 = r;
							r1.height = SRand::Inst().Rand( 0, r1.height / 2 );
							for( int x = r.x; x < r.GetRight(); x++ )
								for( int y = r.y; y < r.GetBottom(); y++ )
									genData[x + y * nWidth] = nType01b;
							for( int x = r1.x; x < r1.GetRight(); x++ )
								for( int y = r1.y; y < r1.GetBottom(); y++ )
									genData[x + y * nWidth] = nType01;
						}
						else
						{
							for( int x = r.x; x < r.GetRight(); x++ )
								for( int y = r.y; y < r.GetBottom(); y++ )
									genData[x + y * nWidth] = nType01;
						}
					}
				}
				else if( nTypes[i] == 1 )
				{
					if( rect.width < 4 || rect.height < 6 )
						continue;

					float fMax = -1;
					int32 nDir = -1;
					for( int k = 0; k < 2; k++ )
					{
						float f = SRand::Inst().Rand( 0.0f, 1.0f );
						int32 x = k == 0 ? rect.x - 1 : rect.GetRight();
						if( x < 0 || x >= nWidth )
							continue;
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							if( genData[x + y * nWidth] != nTypeWallChunk )
								f += 1;
						}
						if( nTypeTemp >= 0 )
						{
							x = k == 0 ? rect.x : rect.GetRight() - 1;
							for( int y = rect.y; y < rect.GetBottom(); y++ )
							{
								if( genData[x + y * nWidth] == nTypeTemp )
									f += rect.height;
							}
						}
						if( f > fMax )
						{
							nDir = k;
							fMax = f;
						}
					}
					if( fMax < 0.5f * rect.height )
						continue;

					int32 h = SRand::Inst().Rand( 3, rect.height - 2 );
					int32 w = SRand::Inst().Rand( 2, 5 );
					TRectangle<int32> r1( nDir ? rect.GetRight() - w : rect.x, rect.GetBottom() - h - 1, w, h );
					TRectangle<int32> r2 = r1;
					int32 w1 = Min( w, SRand::Inst().Rand( 2, 5 ) );
					if( nDir == 1 )
						r2.SetLeft( r2.GetRight() - w1 );
					else
						r2.width = w1;
					for( int x = r1.x; x < r1.GetRight(); x++ )
						for( int y = r1.y; y < r1.GetBottom(); y++ )
							genData[x + y * nWidth] = nType01;
					for( int x = r2.x; x < r2.GetRight(); x++ )
						for( int y = r2.y; y < r2.GetBottom(); y++ )
							genData[x + y * nWidth] = nType01a;

					if( SRand::Inst().Rand( 0, 2 ) )
					{
						int32 w2 = SRand::Inst().Rand( 3, 5 );
						int32 w3 = SRand::Inst().Rand( 1, 4 );
						r1.width = w2;
						for( w1 = w + w3; w1 <= rect.width - 2 && SRand::Inst().Rand( 0, 5 ); w1 += w2 + w3 )
						{
							r1.x = rect.x + ( nDir ? rect.width - w1 - w2 : w1 );
							r1 = r1 * rect;
							r2 = r1;
							r2.width = SRand::Inst().Rand( 0, r1.width );
							r2.x += SRand::Inst().Rand( 0, r1.width - r2.width + 1 );
							for( int x = r1.x; x < r1.GetRight(); x++ )
								for( int y = r1.y; y < r1.GetBottom(); y++ )
									genData[x + y * nWidth] = nType01a;
							for( int x = r2.x; x < r2.GetRight(); x++ )
								for( int y = r2.y; y < r2.GetBottom(); y++ )
									genData[x + y * nWidth] = nType01;
						}
					}
				}
				else if( nTypes[i] == 2 )
				{
					if( rect.height < 7 || rect.width < 4 || rect.width >= rect.height * 1.25f + 1 )
						continue;
					n1 = 0;
					n2 = 0;
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						if( rect.x <= 0 || genData[rect.x - 1 + y * nWidth] != nTypeWallChunk )
							n1++;
						if( rect.GetRight() >= nWidth || genData[rect.GetRight() + y * nWidth] != nTypeWallChunk )
							n2++;
						if( nTypeTemp >= 0 )
						{
							if( genData[rect.x + y * nWidth] == nTypeTemp )
								n1 += rect.height;
							if( genData[rect.GetRight() - 1 + y * nWidth] == nTypeTemp )
								n2 += rect.height;
						}
					}
					if( n1 * 2 < rect.height || n2 * 2 < rect.height )
						continue;

					int32 h = SRand::Inst().Rand( 3, 5 );
					TRectangle<int32> r = rect;
					r.height = h;
					uint8 b = SRand::Inst().Rand( 0, 2 );
					r.y = b ? rect.GetBottom() - h - SRand::Inst().Rand( 1, 3 ) : rect.y + SRand::Inst().Rand( 2, Min( rect.height - h, 6 ) );
					int32 h1 = SRand::Inst().Rand( 4, 7 );
					for( ;; )
					{
						for( int x = r.x; x < r.GetRight(); x++ )
						{
							int8 nType = nType01a;
							if( Min( x - r.x, r.GetRight() - 1 - x ) >= SRand::Inst().Rand( 1, 4 ) && SRand::Inst().Rand( 0, 3 ) )
								nType = nType01;
							for( int y = r.y; y < r.GetBottom(); y++ )
								genData[x + y * nWidth] = nType;
						}
						if( SRand::Inst().Rand( 0, 2 ) )
							break;
						if( b )
						{
							r.y += h1 + h;
							if( r.GetBottom() >= rect.GetBottom() - 1 )
								break;
						}
						else
						{
							r.y -= h1 + h;
							if( r.y < rect.y + 2 )
								break;
						}
					}

					if( SRand::Inst().Rand( 0, 2 ) )
					{
						TRectangle<int32> r1( rect.x, rect.y + 1, rect.width, rect.height - 2 );
						TRectangle<int32> r2 = r1;
						r2.width = SRand::Inst().Rand( 2, r2.width );
						if( r2.width * 2 >= r1.width )
							r2.x += SRand::Inst().Rand( 0, r1.width - r2.width + 1 );
						else if( SRand::Inst().Rand( 0, 2 ) )
							r2.x = r1.GetRight() - r2.width;

						if( SRand::Inst().Rand( 0, 2 ) )
						{
							for( int x = r2.x; x < r2.GetRight(); x++ )
								for( int y = r2.y; y < r2.GetBottom(); y++ )
									genData[x + y * nWidth] = genData[x + ( y + 1 ) * nWidth];
						}
						else
						{
							for( int x = r2.x; x < r2.GetRight(); x++ )
								for( int y = r2.GetBottom() - 1; y >= r2.y; y-- )
									genData[x + y * nWidth] = genData[x + ( y - 1 ) * nWidth];
						}
					}
				}

				break;
			}

			if( nTypeTemp >= 0 )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						if( genData[x + y * nWidth] == nTypeTemp )
							genData[x + y * nWidth] = nTypeWallChunk;
					}
				}
			}

			if( rect.y > 0 && rect.width >= 4 && rect.height >= 4 )
			{
				int32 nLen = 0;
				float fMaxLen = 0;
				int32 nMax = -1;
				for( int i = rect.x; i <= rect.GetRight(); i++ )
				{
					if( i < rect.GetRight() && genData[i + ( rect.y - 1 ) * nWidth] == nType0
						&& genData[i + rect.y * nWidth] == nTypeWallChunk
						&& genData[i + ( rect.y + 1 ) * nWidth] == nTypeWallChunk )
						nLen++;
					else if( nLen )
					{
						float fLen = nLen + SRand::Inst().Rand( 0.0f, 1.0f );
						if( fLen > fMaxLen )
						{
							fMaxLen = fLen;
							nMax = i - nLen;
						}
						nLen = 0;
					}
				}

				nLen = floor( fMaxLen );
				TRectangle<int32> r( 0, 0, 0, 0 );
				if( nType3 >= 0 && nLen >= 3 + SRand::Inst().Rand( 0, 3 ) && SRand::Inst().Rand( 0, 2 ) )
				{
					for( int i = 0; i < 3; i++ )
					{
						int32 x = nMax + SRand::Inst().Rand( 1, nLen - 1 );
						int32 y = rect.y + SRand::Inst().Rand( 0, rect.height - 3 );
						if( genData[x + y * nWidth] == nTypeWallChunk )
						{
							r = PutRect( genData, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 3, 3 ), TVector2<int32>( 3, 3 ),
								rect, -1, nType3 );
							if( r.width > 0 )
								break;
						}
					}
					if( r.width > 0 )
						r.SetBottom( SRand::Inst().Rand( r.GetBottom(), rect.GetBottom() ) );
				}
				/*else if( nType2 >= 0 && nLen >= 2 + SRand::Inst().Rand( 0, 2 ) )
				{
					int32 x = nMax + SRand::Inst().Rand( 0, nLen - 1 );
					int32 y = rect.y;
					r = TRectangle<int32>( x, y, 2, Min( rect.GetBottom() - 1, SRand::Inst().Rand( 2, 5 ) ) );

					for( int i = x; i < x + 2; i++ )
					{
						for( int j = y; j < y + 2; j++ )
						{
							genData[i + j * nWidth] = nType2;
						}
					}
				}*/
				else if( SRand::Inst().Rand( 0, 2 ) )
				{
					r.width = SRand::Inst().Rand( 2, rect.width );
					r.height = 1;
					r.x = rect.x + SRand::Inst().Rand( 0, rect.width - r.width + 1 );
					r.y = rect.y;
				}
				else
					continue;

				for( int i = r.x; i < r.GetRight(); i++ )
				{
					for( int j = rect.y; j < r.GetBottom(); j++ )
					{
						if( genData[i + j * nWidth] == nTypeWallChunk )
							genData[i + j * nWidth] = nType1;
					}
				}
				int32 h = r.GetBottom();
				int32 n = 0;
				for( int x = r.x - 1; x >= rect.x; x-- )
				{
					if( !n )
					{
						h = Min( rect.GetBottom() - 1, Max( rect.y, h + ( h > 2 ? SRand::Inst().Rand( -2, 1 ) : SRand::Inst().Rand( -1, 2 ) ) ) );
						n = SRand::Inst().Rand( 2, 5 );
					}
					for( int y = rect.y; y < h; y++ )
					{
						if( genData[x + y * nWidth] != nTypeWallChunk )
							break;
						genData[x + y * nWidth] = nType1;
					}
					n--;
				}
				h = r.GetBottom();
				n = 0;
				for( int x = r.GetRight(); x < rect.GetRight(); x++ )
				{
					if( !n )
					{
						h = Min( rect.GetBottom() - 1, Max( rect.y, h + ( h > 2 ? SRand::Inst().Rand( -2, 1 ) : SRand::Inst().Rand( -1, 2 ) ) ) );
						n = SRand::Inst().Rand( 2, 5 );
					}
					for( int y = rect.y; y < h; y++ )
					{
						if( genData[x + y * nWidth] != nTypeWallChunk )
							break;
						genData[x + y * nWidth] = nType1;
					}
					n--;
				}
			}
		}
	}
	void GenWindows( vector<int8>& genData, int32 nWidth, int32 nHeight, vector<TRectangle<int32> >& wallChunks, vector<TRectangle<int32> >& wallChunks1, vector<TRectangle<int32> >& windows, vector<TRectangle<int32> > windows1[4],
		int32 nType, int32 nType0, int32 nType1a, int32 nType1b, int32 nType0a, int32 nType0b, int32 nType1, int32 nType3 )
	{
		int32 nMinFillSize = 130;
		int32 nMaxFillSize = 150;

		vector<int8> tempData1;
		tempData1.resize( genData.size() );
		memset( &tempData1[0], 0, tempData1.size() );
		vector<TVector2<int32> > vecPoints1;
		for( int i = 0; i < wallChunks.size(); i++ )
		{
			auto& rect = wallChunks[i];
			for( int x = rect.x; x < rect.GetRight(); x++ )
			{
				for( int y = rect.y; y < rect.GetBottom(); y++ )
				{
					if( genData[x + y * nWidth] == nType )
					{
						tempData1[x + y * nWidth] = i + 1;
						if( !SRand::Inst().Rand( 0, 3 ) )
							vecPoints1.push_back( TVector2<int32>( x, y ) );
					}
				}
			}
		}

		if( nType3 >= 0 )
		{
			vector<int8> tempData;
			tempData.resize( genData.size() );
			vector<TVector2<int32> > vec;
			for( auto rect : wallChunks1 )
			{
				rect.x -= 3;
				rect.y -= 3;
				rect.width += 6;
				rect.height += 6;
				rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );

				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						auto nType = genData[i + j * nWidth];
						if( nType < nType1a || nType > nType1b )
						{
							if( !tempData[i + j * nWidth] )
							{
								tempData[i + j * nWidth] = 1;
								vec.push_back( TVector2<int32>( i, j ) );
							}
						}
						else
							tempData[i + j * nWidth] = 1;
					}
				}
			}

			SRand::Inst().Shuffle( vec );

			for( auto p : vec )
			{
				if( tempData[p.x + p.y * nWidth] != 1 )
					continue;
				if( genData[p.x + p.y * nWidth] != nType )
					continue;

				vector<TVector2<int32> > q;
				FloodFill( tempData, nWidth, nHeight, p.x, p.y, 2, SRand::Inst().Rand( nMinFillSize, nMaxFillSize ), q );
				if( q.size() >= nMinFillSize )
				{
					CVector2 s;
					for( auto& p1 : q )
					{
						s.x += p1.x;
						s.y += p1.y;
					}
					s = s / q.size() + CVector2( SRand::Inst().Rand( -0.1f, 0.1f ), SRand::Inst().Rand( -0.1f, 0.1f ) );
					struct SLess
					{
						CVector2 s;
						bool operator () ( const TVector2<int32>& left, const TVector2<int32>& right )
						{
							CVector2 a( left.x - s.x, left.y - s.y );
							CVector2 b( right.x - s.x, right.y - s.y );
							return a.Length2() < b.Length2();
						}
					};
					SLess less;
					less.s = s;
					sort( q.begin(), q.end(), less );
					for( auto& p1 : q )
					{
						if( genData[p1.x + p1.y * nWidth] == nType )
						{
							auto rect = PutRect( tempData1, nWidth, nHeight, p, TVector2<int32>( 4, 4 ), TVector2<int32>( 4, 4 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
							if( rect.width )
							{
								for( int i = rect.x; i < rect.GetRight(); i++ )
								{
									for( int j = rect.y; j < rect.GetBottom(); j++ )
									{
										genData[i + j * nWidth] = nType0;
										tempData[i + j * nWidth] = 0;
									}
								}
								bool b = true;
								for( auto& rect1 : windows )
								{
									int32 x = Max( 0, Max( rect.x - rect1.GetRight(), rect1.x - rect.GetRight() ) );
									int32 y = Max( 0, Max( rect.y - rect1.GetBottom(), rect1.y - rect.GetBottom() ) );
									if( x * x + y * y < 20 * 20 )
									{
										b = false;
										break;
									}
								}
								if( b )
									windows.push_back( rect );
								else
								{
									for( int i = rect.x; i < rect.GetRight(); i++ )
									{
										for( int j = rect.y; j < rect.GetBottom(); j++ )
										{
											genData[i + j * nWidth] = nType1;
										}
									}
									rect.x += SRand::Inst().Rand( 0, 2 );
									rect.y += SRand::Inst().Rand( 0, 2 );
									rect.width--;
									rect.height--;
									for( int i = rect.x; i < rect.GetRight(); i++ )
									{
										for( int j = rect.y; j < rect.GetBottom(); j++ )
										{
											genData[i + j * nWidth] = nType3;
										}
									}
								}
							}
							break;
						}
					}
				}
			}
		}

		SRand::Inst().Shuffle( vecPoints1 );
		for( auto p : vecPoints1 )
		{
			if( !tempData1[p.x + p.y * nWidth] )
				continue;
			uint8 nType = Min( 2, SRand::Inst().Rand( 0, 4 ) );
			TVector2<int32> sizeMin, sizeMax, sizeMin1, sizeMax1;
			if( nType == 0 )
			{
				sizeMin = TVector2<int32>( 3, 2 );
				sizeMax = TVector2<int32>( 6, 2 );
				sizeMin1 = TVector2<int32>( 4, 3 );
				sizeMax1 = TVector2<int32>( 6, 4 );
			}
			else if( nType == 1 )
			{
				sizeMin = TVector2<int32>( 3, 3 );
				sizeMax = TVector2<int32>( 4, 4 );
				sizeMin1 = TVector2<int32>( 4, 3 );
				sizeMax1 = TVector2<int32>( 5, 4 );
			}
			else
			{
				sizeMin = TVector2<int32>( 4, 4 );
				sizeMax = TVector2<int32>( 8, 5 );
				sizeMin1 = TVector2<int32>( 4, 4 );
				sizeMax1 = TVector2<int32>( 8, 5 );
			}

			auto rect = PutRect( tempData1, nWidth, nHeight, p, sizeMin1, sizeMax1, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
			if( rect.width > 0 )
			{
				TVector2<int32> size( sizeMin.x + ( sizeMax.x - sizeMin.x ) * ( rect.width - sizeMin1.x ) / ( sizeMax1.x / sizeMin1.x ),
					sizeMin.y + ( sizeMax.y - sizeMin.y ) * ( rect.height - sizeMin1.y ) / ( sizeMax1.y / sizeMin1.y ) );
				size.x = Min( size.x, rect.width );
				size.y = Min( size.y, rect.height );
				rect.x += SRand::Inst().Rand( 0, rect.width - size.x + 1 );
				rect.y += SRand::Inst().Rand( 0, rect.height - size.y + 1 );
				rect.width = size.x;
				rect.height = size.y;
				windows1[nType].push_back( rect );

				TRectangle<int32> rect1 = rect;
				rect1.x++;
				rect1.width -= 2;
				if( nType > 0 )
				{
					rect1.y++;
					rect1.height -= 2;
				}
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
				{
					for( int j = rect1.y; j < rect1.GetBottom(); j++ )
					{
						genData[i + j * nWidth] = nType0;
					}
				}
			}
		}

		for( auto& rect : wallChunks )
		{
			vecPoints1.clear();
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					auto nData = genData[i + j * nWidth];
					tempData1[i + j * nWidth] = nData >= nType0a && nData <= nType0b ? 1 : 0;
					if( tempData1[i + j * nWidth] && !SRand::Inst().Rand( 0, 3 ) )
						vecPoints1.push_back( TVector2<int32>( i, j ) );
				}
			}
			SRand::Inst().Shuffle( vecPoints1 );

			TVector2<int32> sizeMin = TVector2<int32>( 3, 3 );
			TVector2<int32> sizeMax = TVector2<int32>( 7, 5 );
			for( auto& p : vecPoints1 )
			{
				if( !tempData1[p.x + p.y * nWidth] )
					continue;
				auto r = PutRect( tempData1, nWidth, nHeight, p, sizeMin, sizeMax, rect, -1, 1 );
				if( r.width > 0 )
				{
					int32 n = SRand::Inst().Rand( 1, 4 );
					TVector2<int32> size = r.GetSize();
					for( ; n > 0; n-- )
					{
						if( r.width > r.height )
						{
							if( size.x > sizeMin.x )
								size.x--;
							else if( size.y > sizeMin.y )
								size.y--;
							else
								break;
						}
						else
						{
							if( size.y > sizeMin.y )
								size.y--;
							else if( size.x > sizeMin.x )
								size.x--;
							else
								break;
						}
					}
					if( n )
						continue;
					r.x += SRand::Inst().Rand( 0, r.width - size.x + 1 );
					r.y += SRand::Inst().Rand( 0, r.height - size.y + 1 );
					r.width = size.x;
					r.height = size.y;
					windows1[3].push_back( r );
					for( int x = r.x; x < r.GetRight(); x++ )
						for( int y = r.y; y < r.GetBottom(); y++ )
							tempData1[x + y * nWidth] = 0;
				}
			}
		}
	}
}

void CLevelGenNode1_3::GenWallChunk()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( auto& rect : m_wallChunks0 )
	{
		if( rect.height < 6 )
			continue;
		for( int x = rect.x - 1; x <= rect.GetRight(); x += rect.width + 1 )
		{
			if( x < 0 || x >= nWidth )
				continue;

			int8 b = SRand::Inst().Rand( 0, 2 );
			int8 b1 = SRand::Inst().Rand( 0, 2 );
			int32 k = SRand::Inst().Rand( 0, 7 );
			for( int j = 0; j < rect.height; j++ )
			{
				int32 y = b ? j + rect.y : rect.GetBottom() - 1 - j;
				if( !k )
				{
					k = SRand::Inst().Rand( 6, 11 );
					b1 = !b1;
				}
				if( m_gendata[x + y * nWidth] == eType_WallChunk && b1 )
					m_gendata[x + y * nWidth] = eType_Temp2;
			}
		}
	}

	__LvGen1_3__::GenWallChunk( m_gendata, nWidth, nHeight, m_wallChunks, eType_WallChunk, eType_WallChunk_1, eType_WallChunk0, eType_WallChunk_0_1, eType_WallChunk_0_1a, eType_WallChunk_0_1b, eType_WallChunk_2, eType_WallChunk_3, eType_Temp2 );
}

void CLevelGenNode1_3::GenWallChunk0()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	for( auto& rect : m_wallChunks0 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_WallChunk0;
			}
		}
		bool bVertical = rect.height > rect.width;
		GenWallChunk0Rect( rect, bVertical );

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp )
				{
					int8 l = i > rect.x ? m_gendata[i - 1 + j * nWidth] : 0;
					int8 r = i < rect.GetRight() - 1 ? m_gendata[i + 1 + j * nWidth] : 0;
					int8 t = j > rect.y ? m_gendata[i + ( j - 1 ) * nWidth] : 0;
					int8 b = j < rect.GetBottom() - 1 ? m_gendata[i + ( j + 1 ) * nWidth] : 0;
					if( ( l == eType_Temp || l == eType_WallChunk0_0 || r == eType_Temp || r == eType_WallChunk0_0 )
						&& ( t == eType_Temp || t == eType_WallChunk0_0 || b == eType_Temp || b == eType_WallChunk0_0 ) )
					{
						m_gendata[i + j * nWidth] = eType_WallChunk0_0;
					}
				}
			}
		}

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp )
				{
					auto r1 = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i, j ), TVector2<int32>( 1, 1 ), TVector2<int32>( nWidth, nHeight ), rect, -1, eType_WallChunk0_0 );
					if( r1.width > 1 || r1.height > 1 )
					{
						int8 nType = r1.width > 1 ? eType_WallChunk0_0x : eType_WallChunk0_0y;
						for( int x = r1.x; x < r1.GetRight(); x++ )
						{
							for( int y = r1.y; y < r1.GetBottom(); y++ )
							{
								m_gendata[x + y * nWidth] = nType;
							}
						}
					}
				}
				else if( m_gendata[i + j * nWidth] == eType_WallChunk0 )
					vec.push_back( TVector2<int32>( i, j ) );
			}
		}
	}

	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_WallChunk0 )
			continue;
		TVector2<int32> maxSize( SRand::Inst().Rand( 3, 7 ), SRand::Inst().Rand( 3, 7 ) );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), 6, eType_WallChunk2 );
		if( rect.width > 0 )
		{
			m_wallChunks2.push_back( rect );
			bool bSpider = false;
			if( rect.width > 2 || rect.height > 2 )
			{
				bSpider = true;
				TVector2<int32> p1( rect.x + ( rect.width + SRand::Inst().Rand( 0, 2 ) ) / 2, rect.y + ( rect.height + SRand::Inst().Rand( 0, 2 ) ) / 2 );
				for( auto& spider : m_spiders )
				{
					if( ( spider.GetCenter() - p1 ).Length2() < 12 * 12 )
					{
						bSpider = false;
						break;
					}
				}
				if( bSpider )
					m_spiders.push_back( TRectangle<int32>( p1.x - 1, p1.y - 1, 2, 2 ) );
			}

			if( !bSpider )
			{
				for( int x = rect.x - 1; x <= rect.GetRight(); x += rect.width + 1 )
				{
					if( x < 0 || x >= nWidth )
						continue;
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						if( m_gendata[x + y * nWidth] == eType_WallChunk0 && !SRand::Inst().Rand( 0, 2 ) )
							m_gendata[x + y * nWidth] = eType_Maggot;
					}
				}
				for( int y = rect.y - 1; y <= rect.GetBottom(); y += rect.height + 1 )
				{
					if( y < 0 || y >= nHeight )
						continue;
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						if( m_gendata[x + y * nWidth] == eType_WallChunk0 && !SRand::Inst().Rand( 0, 2 ) )
							m_gendata[x + y * nWidth] = eType_Maggot;
					}
				}
			}
		}
		else if( !SRand::Inst().Rand( 0, 4 ) )
			m_gendata[p.x + p.y * nWidth] = eType_Maggot;
	}
}

void CLevelGenNode1_3::GenWallChunk0Rect( const TRectangle<int32>& rect, bool bVertical, bool bSplit )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 l = bVertical ? rect.height : rect.width;
	int32 w = bVertical ? rect.width : rect.height;
	if( l > SRand::Inst().Rand( 9, 12 ) )
	{
		int32 l0 = SRand::Inst().Rand( 4, l - 4 );
		TRectangle<int32> r1 = rect, r2 = rect;
		if( bVertical )
		{
			r1.SetBottom( rect.y + l0 );
			r2.SetTop( rect.y + l0 + 1 );
		}
		else
		{
			r1.SetRight( rect.x + l0 );
			r2.SetLeft( rect.x + l0 + 1 );
		}
		int32 w1 = SRand::Inst().Rand( w / 2, Max( w / 2 + 1, w - 3 ) );
		int32 i0 = SRand::Inst().Rand( 0, w );
		for( int i = 0; i < w1; i++ )
		{
			int32 i1 = ( i + i0 ) % w;
			if( bVertical )
				m_gendata[rect.x + i1 + ( rect.y + l0 ) * nWidth] = eType_Temp;
			else
				m_gendata[rect.x + l0 + ( rect.y + i1 ) * nWidth] = eType_Temp;
		}
		GenWallChunk0Rect( r1, bVertical, true );
		GenWallChunk0Rect( r2, bVertical, true );

		return;
	}

	int8 nType = bSplit && l < w ? SRand::Inst().Rand( 0, 3 ) : SRand::Inst().Rand( 1, 3 );
	if( nType == 0 )
	{

	}
	else if( nType == 1 )
	{
		for( int j = 0; j < w; j += w - 1 )
		{
			uint8 b = SRand::Inst().Rand( 0, 2 );
			uint8 b1 = SRand::Inst().Rand( 0, 2 );
			uint8 n = SRand::Inst().Rand( 1, 4 );
			for( int i = 0; i < l; i++ )
			{
				int32 x = b ? i : l - 1 - i;
				int32 y = j;
				if( bVertical )
					swap( x, y );
				if( b1 )
					m_gendata[x + rect.x + ( y + rect.y ) * nWidth] = eType_Temp;

				n--;
				if( !n )
				{
					b1 = !b1;
					n = SRand::Inst().Rand( 1, 4 );
				}
			}
		}
	}
	else
	{
		int32 l0 = SRand::Inst().Rand( Max( l / 2, l - 3 ), l );
		int32 i0 = SRand::Inst().Rand( 0, l );
		int32 j0 = ( w - SRand::Inst().Rand( 0, 2 ) ) / 2;
		for( int i = 0; i < l0; i++ )
		{
			int32 x = ( i + i0 ) % l;
			int32 y = j0;
			if( bVertical )
				swap( x, y );
			m_gendata[x + rect.x + ( y + rect.y ) * nWidth] = eType_Temp;
		}
	}
}

void CLevelGenNode1_3::GenWindows()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	__LvGen1_3__::GenWindows( m_gendata, nWidth, nHeight, m_wallChunks, m_wallChunks1, m_windows, m_windows1, eType_WallChunk, eType_WallChunk_0, eType_WallChunk1, eType_WallChunk1_2, eType_WallChunk_0_1, eType_WallChunk_0_1b, eType_WallChunk_1, eType_WallChunk_3 );
}

void CLevelGenNode1_3::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			int32 nType = m_gendata[i + j * nWidth];
			int32 nType1 = j > 0 ? m_gendata[i + ( j - 1 ) * nWidth] : eType_WallChunk1;
			if( nType == eType_Wall && nType1 >= eType_WallChunk1 && nType1 <= eType_WallChunk1_2 )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}

	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		int32 nType = m_gendata[p.x + p.y * nWidth];
		if( nType != eType_Wall )
			continue;
		TVector2<int32> ofs[] = { { -1, 0 }, { 1, 0 }, { -2, 0 }, { 2, 0 }, { 0, -1 }, { 0, 1 } };
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, eType_Temp, 4, ofs, ELEM_COUNT( ofs ) );
	}

	vec.clear();
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Wall, vec );
	SRand::Inst().Shuffle( vec );
	int32 nCount = vec.size() / 4;
	for( int i = 0; i < vec.size(); i++ )
	{
		auto p = vec[i];
		int32 nType = m_gendata[p.x + p.y * nWidth];
		if( nType != eType_Wall )
			continue;
		nCount -= FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, eType_Temp, 8 );
		if( nCount <= 0 )
			break;
	}

	LvGenLib::GenObjs( m_gendata, nWidth, nHeight, 0, eType_Wall, eType_Temp1 );
	for( auto rect : m_wallChunks1 )
	{
		if( rect.height > 2 )
			continue;
		if( rect.y == 0 )
			continue;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			auto type = m_gendata[x + ( rect.y - 1 ) * nWidth];
			if( type == eType_Wall || type == eType_WallChunk0 || type >= eType_WallChunk1 && type <= eType_WallChunk1_2 )
				continue;

			int32 nHeight = Min( rect.height, SRand::Inst().Rand( 0, rect.height + 2 ) );
			for( int y = rect.y; y < rect.y + nHeight; y++ )
				m_gendata[x + y * nWidth] = eType_Temp1;
		}
	}
	GenWallChunk0();

	vector<int8> vecData;
	vecData.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			auto nType = m_gendata[i + j * nWidth];
			vecData[i + j * nWidth] = nType == eType_Wall || nType == eType_Maggot || nType == eType_WallChunk0
				|| nType >= eType_WallChunk1 && nType <= eType_WallChunk1_2 || nType == eType_Temp1? 0 : 1;
		}
	}
	SRand::Inst().Shuffle( m_wallChunks );
	for( auto rect : m_wallChunks )
	{
		if( rect.GetBottom() >= nHeight )
			continue;
		rect.y = rect.GetBottom();
		rect.height = 1;
		while( rect.x > 0 )
		{
			if( vecData[rect.x - 1 + rect.y * nWidth] )
				break;
			if( m_gendata[rect.x - 1 + ( rect.y - 1 ) * nWidth] != eType_WallChunk )
				break;
			rect.x--;
		}
		while( rect.GetRight() < nWidth )
		{
			if( vecData[rect.GetRight() + rect.y * nWidth] )
				break;
			if( m_gendata[rect.GetRight() + ( rect.y - 1 ) * nWidth] != eType_WallChunk )
				break;
			rect.width++;
		}
		auto r1 = rect;
		if( r1.GetBottom() < nHeight )
			r1.height++;
		rect = PutRect( vecData, nWidth, nHeight, rect, rect.GetSize(), r1.GetSize(), r1, -1, 2, 0 );
		if( rect.width > 0 )
		{
			if( rect.width >= 16 )
			{
				int32 w = SRand::Inst().Rand( 8, rect.width - 7 );
				TRectangle<int32> r = rect;
				rect.width = w;
				r.SetLeft( r.x + w );
				m_scraps.push_back( r );
				m_scraps.push_back( rect );
			}
			else
				m_scraps.push_back( rect );
		}
	}
	for( auto& rect : m_scraps )
	{
		for( int n = SRand::Inst().Rand( 1, 4 ); n >= 0 && rect.x > 0; n-- )
		{
			bool b = true;
			int32 x = rect.x - 1;
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( vecData[x + y * nWidth] )
				{
					b = false;
					break;
				}
			}
			if( !b )
				break;
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				vecData[x + y * nWidth] = 2;
			rect.SetLeft( rect.x - 1 );
		}
		for( int n = SRand::Inst().Rand( 1, 4 ); n >= 0 && rect.GetRight() < nWidth; n-- )
		{
			bool b = true;
			int32 x = rect.GetRight();
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( vecData[x + y * nWidth] )
				{
					b = false;
					break;
				}
			}
			if( !b )
				break;
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				vecData[x + y * nWidth] = 2;
			rect.width++;
		}
	}
	for( auto wallChunk : m_wallChunks )
	{
		for( int n = 0; n < 2; n++ )
		{
			TRectangle<int32> rect( n == 0 ? wallChunk.x - 1 : wallChunk.GetRight(), wallChunk.y, 1, wallChunk.height );
			if( rect.x < 0 || rect.x >= nWidth )
				continue;
			while( rect.height > 1 )
			{
				if( !vecData[rect.x + rect.y * nWidth] )
					break;
				rect.y++;
				rect.height--;
			}
			if( rect.y == wallChunk.y )
			{
				if( rect.y == 0 )
					continue;
				if( !vecData[rect.x + ( rect.y - 1 ) * nWidth] )
				{
					int y0;
					for( y0 = rect.y; y0 > Max( 0, rect.y - 5 ); y0-- )
					{
						if( vecData[rect.x + ( y0 - 1 ) * nWidth] )
							break;
					}
					if( y0 <= Max( 0, rect.y - 5 ) )
						continue;
					rect.SetTop( y0 );
				}
			}
			while( rect.GetBottom() < nHeight )
			{
				if( vecData[rect.x + rect.GetBottom() * nWidth] )
					break;
				if( n == 0 && rect.x > 0 && vecData[rect.x - 1 + rect.GetBottom() * nWidth] != 1 )
					break;
				if( n == 1 && rect.x < nWidth - 1 && vecData[rect.x + 1 + rect.GetBottom() * nWidth] != 1 )
					break;
				rect.height++;
			}
			if( rect.height < 4 )
				continue;
			auto r1 = rect;
			if( n == 0 && r1.x > 0 )
				r1.SetLeft( r1.x - 1 );
			if( n == 1 && r1.x < nWidth - 1 )
				r1.width++;;
			rect = PutRect( vecData, nWidth, nHeight, rect, rect.GetSize(), r1.GetSize(), r1, -1, 2, 0 );
			if( rect.width > 0 )
			{
				if( rect.height >= 16 )
				{
					int32 h = SRand::Inst().Rand( 8, rect.height - 7 );
					TRectangle<int32> r = rect;
					rect.height = h;
					r.SetBottom( r.y + h );
					m_scraps.push_back( r );
					m_scraps.push_back( rect );
				}
				else
					m_scraps.push_back( rect );
			}
		}
	}
	for( auto& rect : m_scraps )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] != eType_Temp1 && !SRand::Inst().Rand( 0, 3 ) )
					m_gendata[i + j * nWidth] = eType_Temp1;
				else if( m_gendata[i + j * nWidth] == eType_Maggot )
					m_gendata[i + j * nWidth] = eType_WallChunk0;
			}
		}
		if( rect.width > rect.height )
		{
			if( rect.height >= 2 && rect.width >= SRand::Inst().Rand( 6, 13 ) )
			{
				TRectangle<int32> rect1( SRand::Inst().Rand( 0, rect.width - 3 ) + rect.x, rect.y, 3, rect.height );
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
				{
					for( int j = rect1.y; j < rect1.GetBottom(); j++ )
					{
						m_gendata[i + j * nWidth] = eType_Crate3;
					}
				}
			}
		}
	}

	int8 nTypes[] = { eType_BlockRed, eType_BlockBlue };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_Temp, nTypes, ELEM_COUNT( nTypes ) );
	int8 nTypes1[] = { eType_Crate1, eType_Crate2 };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_Temp1, nTypes1, ELEM_COUNT( nTypes1 ) );
}

void CLevelGenNode1_3::GenShops()
{
	if( m_pShopNode )
	{
		int32 nHeight = m_region.height;
		struct SLess
		{
			bool operator () ( const SRoom& left, const SRoom& right )
			{
				return left.rect.y < right.rect.y;
			}
		};
		std::sort( m_rooms.begin(), m_rooms.end(), SLess() );

		/*if( m_pShopNode )
		{
			int32 nShop = floor( nHeight / 32 + 0.5f );
			vector<int32> vecPossibleShop;
			for( int i = 0; i < m_rooms.size(); i++ )
			{
				if( m_rooms[i].rect.width >= 8 && m_rooms[i].rect.height >= 6 && !m_rooms[i].bRoomType )
					vecPossibleShop.push_back( i );
			}

			int32 n = 0;
			for( int i = 0; i < nShop; i++ )
			{
				int32 h = nHeight / nShop * ( i + SRand::Inst().Rand( 0.5f, 0.75f ) );
				for( ; n < vecPossibleShop.size(); n++ )
				{
					auto& room = m_rooms[vecPossibleShop[n]];
					if( room.rect.y >= h )
					{
						m_shops.push_back( room.rect );
						n++;
						break;
					}
				}
			}
		}*/
	}
}


void CLevelGenNode1_3_0::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );
	m_pWallChunk0Node = CreateNode( pXml->FirstChildElement( "wallchunk0" )->FirstChildElement(), context );
	m_pWallChunk1Node = CreateNode( pXml->FirstChildElement( "wallchunk1" )->FirstChildElement(), context );
	m_pBlock1Node = CreateNode( pXml->FirstChildElement( "block1" )->FirstChildElement(), context );
	m_pBlock2Node = CreateNode( pXml->FirstChildElement( "block2" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pCrate1Node = CreateNode( pXml->FirstChildElement( "crate1" )->FirstChildElement(), context );
	m_pCrate2Node = CreateNode( pXml->FirstChildElement( "crate2" )->FirstChildElement(), context );
	m_pScrapNode = CreateNode( pXml->FirstChildElement( "scrap" )->FirstChildElement(), context );
	m_pAcNode = CreateNode( pXml->FirstChildElement( "air_conditioner" )->FirstChildElement(), context );
	m_pWindowNode = CreateNode( pXml->FirstChildElement( "window" )->FirstChildElement(), context );
	m_pWindow1Node[0] = CreateNode( pXml->FirstChildElement( "window1_1" )->FirstChildElement(), context );
	m_pWindow1Node[1] = CreateNode( pXml->FirstChildElement( "window1_2" )->FirstChildElement(), context );
	m_pWindow1Node[2] = CreateNode( pXml->FirstChildElement( "window1_3" )->FirstChildElement(), context );
	m_pWindow1Node[3] = CreateNode( pXml->FirstChildElement( "window1_4" )->FirstChildElement(), context );

	auto pSpider = pXml->FirstChildElement( "spider" );
	if( pSpider )
		m_pSpiderNode = CreateNode( pSpider->FirstChildElement(), context );
	auto pShop = pXml->FirstChildElement( "shop" );
	if( pShop )
		m_pShopNode = CreateNode( pShop->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_3_0::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenChunks();
	GenRooms();

	for( auto& chunk : m_wallChunks0 )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunk0Node->Generate( context, rect );
	}
	context.mapTags["1"] = eType_WallChunk1_1;
	context.mapTags["2"] = eType_WallChunk1_2;
	for( auto& chunk : m_wallChunks1 )
	{
		for( int i = chunk.x; i < chunk.GetRight(); i++ )
		{
			for( int j = chunk.y; j < chunk.GetBottom(); j++ )
			{
				int32 x = region.x + i;
				int32 y = region.y + j;
				int8 genData = m_gendata[i + j * region.width];
				context.blueprint[x + y * context.nWidth] = genData;
			}
		}
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunk1Node->Generate( context, rect );
	}
	GenObjs();

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData == eType_None || genData == eType_Crate1 || genData == eType_Crate2 )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["0"] = eType_WallChunk_0;
	context.mapTags["0_1"] = eType_WallChunk_0_1;
	context.mapTags["0_1a"] = eType_WallChunk_0_1a;
	context.mapTags["0_1b"] = eType_WallChunk_0_1b;
	context.mapTags["1"] = eType_WallChunk_1;
	//context.mapTags["2"] = eType_WallChunk_2;
	context.mapTags["3"] = eType_WallChunk_3;
	for( auto& chunk : m_wallChunks )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunkNode->Generate( context, rect );
	}
	for( auto& window : m_windows )
	{
		auto rect = window.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWindowNode->Generate( context, rect );
	}
	for( int i = 0; i < 4; i++ )
	{
		for( auto& window : m_windows1[i] )
		{
			auto rect = window.Offset( TVector2<int32>( region.x, region.y ) );
			m_pWindow1Node[i]->Generate( context, rect );
		}
	}

	context.mapTags["1"] = eType_Crate1;
	context.mapTags["2"] = eType_Crate2;
	context.mapTags["3"] = eType_Crate3;
	for( auto& scrap : m_scraps )
	{
		auto rect = scrap.Offset( TVector2<int32>( region.x, region.y ) );
		m_pScrapNode->Generate( context, rect );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				context.blueprint[i + j * context.nWidth] = eType_Temp;
			}
		}
	}
	for( auto& r : m_acs )
	{
		auto rect = r.Offset( TVector2<int32>( region.x, region.y ) );
		m_pAcNode->Generate( context, rect );
	}

	context.mapTags["mask"] = eType_BlockRed;
	m_pBlock1Node->Generate( context, region );
	context.mapTags["mask"] = eType_BlockBlue;
	m_pBlock2Node->Generate( context, region );
	context.mapTags["mask"] = eType_Crate1;
	m_pCrate1Node->Generate( context, region );
	context.mapTags["mask"] = eType_Crate2;
	m_pCrate2Node->Generate( context, region );

	context.mapTags["door"] = eType_Door;
	context.mapTags["mask"] = eType_Web;
	context.mapTags["room"] = eType_Room;
	context.mapTags["pipe0"] = eType_Room_Pipe0;
	context.mapTags["pipe1"] = eType_Room_Pipe1;
	context.mapTags["pipe2"] = eType_Room_Pipe2;
	context.mapTags["pipe3"] = eType_Room_Pipe3;
	for( auto& room : m_rooms )
	{
		auto rect = room.rect.Offset( TVector2<int32>( region.x, region.y ) );
		m_pRoomNode->Generate( context, rect );
	}
	for( auto& rect : m_spiders )
	{
		m_pSpiderNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& shop : m_shops )
	{
		TRectangle<int32> rect = shop;
		rect.x = SRand::Inst().Rand( 1, rect.width - 6 ) + rect.x;
		rect.y = SRand::Inst().Rand( 1, rect.height - 4 ) + rect.y;
		rect.width = 6;
		rect.height = 4;
		m_pShopNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_vecTemp1.clear();
	m_gendata.clear();
	m_rooms.clear();
	m_wallChunks.clear();
	m_wallChunks0.clear();
	m_wallChunks1.clear();
	m_scraps.clear();
	m_acs.clear();
	m_windows.clear();
	m_windows1[0].clear();
	m_windows1[1].clear();
	m_windows1[2].clear();
	m_windows1[3].clear();
	m_spiders.clear();
	m_shops.clear();
}

void CLevelGenNode1_3_0::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int32> v1;
	v1.resize( nWidth * nHeight );
	memset( &v1[0], -1, sizeof( int32 ) * v1.size() );
	vector<TVector2<int32> > vecTemp;
	vector<TRectangle<int32> > vecRects;

	vector<int32> vecFlag;
	vector<int32> q;
	vector<int32> q1;

	vector<int32> vecPars;
	vector<int32> vecGroups;
	auto CreateChunk = [this, nWidth, nHeight, &v1, &vecRects, &vecFlag, &vecPars, &vecGroups] ( const TVector2<int32>& p, const TRectangle<int32>& lim ) -> TRectangle<int32>
	{
		if( m_gendata[p.x + p.y * nWidth] )
			return TRectangle<int32>( 0, 0, 0, 0 );
		TVector2<int32> vMin, vMax;
		if( SRand::Inst().Rand( 0, 4 ) )
		{
			vMin = TVector2<int32>( 6, 4 );
			vMax = TVector2<int32>( SRand::Inst().Rand( 6, 12 ), SRand::Inst().Rand( 4, 7 ) );
		}
		else
		{
			vMin = TVector2<int32>( 4, 4 );
			vMax = TVector2<int32>( SRand::Inst().Rand( 4, 8 ), SRand::Inst().Rand( 6, 10 ) );
		}
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, vMin, vMax, lim, 14, eType_WallChunk );
		if( rect.width <= 0 )
			return TRectangle<int32>( 0, 0, 0, 0 );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				v1[i + j * nWidth] = vecRects.size();
		}
		vecRects.push_back( rect );
		vecFlag.push_back( 0 );
		vecPars.push_back( -1 );
		vecGroups.push_back( -1 );
		return rect;
	};

	{
		CreateChunk( TVector2<int32>( 0, 0 ), TRectangle<int32>( 0, 0, nWidth, nHeight ) );
		CreateChunk( TVector2<int32>( nWidth - 1, 0 ), TRectangle<int32>( 0, 0, nWidth, nHeight ) );

		TVector2<int32> p[2] = { { 0, 0 }, { nWidth - 1, 0 } };
		int32 s = 0;
		for( ; p[1].y < nHeight - 6; )
		{
			if( p[0].x == p[1].x )
			{
				if( p[0].y == p[1].y && SRand::Inst().Rand( 0, 2 ) )
				{
					TVector2<int32> p1( p[0].x, Min( nHeight - 6, p[0].y + SRand::Inst().Rand( 12, 18 ) ) );
					int32 nMin = Min( nWidth - nWidth / 4 - 1, Max( nWidth / 4, p1.x + s - 4 ) );
					int32 nMax = Max( nWidth / 4 + 1, Min( nWidth - nWidth / 4, p1.x + s + 5 ) );
					p1.x = SRand::Inst().Rand( nMin, nMax );
					auto rect = CreateChunk( p1, TRectangle<int32>( 0, p1.y, nWidth, nHeight - p1.y ) );
					if( rect.width > 0 )
						s += ( rect.x + rect.GetRight() - nWidth + SRand::Inst().Rand( 0, 2 ) ) / 2;
					p[1] = p1;
					p[0].x = p1.x;
				}
				else
				{
					TVector2<int32> p1;
					p1.x = s + SRand::Inst().Rand( -2, 4 ) > 0 ? 0 : nWidth - 1;
					int32 d = abs( p1.x - p[1].x );
					p1.y = Min( nHeight - 6, p[1].y + 7 + SRand::Inst().Rand( 0, d * 7 / nWidth ) );
					auto rect = CreateChunk( p1, TRectangle<int32>( 0, p1.y, nWidth, nHeight - p1.y ) );
					if( rect.width > 0 )
						s += ( rect.x + rect.GetRight() - nWidth + SRand::Inst().Rand( 0, 2 ) ) / 2;
					p[0] = p[1];
					p[1] = p1;
				}
			}
			else
			{
				TVector2<int32> a = p[0] + p[1];
				a.x = ( a.x + SRand::Inst().Rand( 0, 2 ) ) / 2;
				a.y = ( a.y + SRand::Inst().Rand( 0, 2 ) ) / 2;
				TVector2<int32> b( p[0].y - p[1].y, p[1].x - p[0].x );
				if( b.y < 0 )
					b = b * -1;
				TVector2<int32> p1 = a + b;
				int32 x1 = Min( nWidth - 1, Max( 0, p1.x - s + SRand::Inst().Rand( -3, 4 ) ) );
				if( b.x )
					p1.y = Min( p1.y + Min( abs( b.x ), Max( 0, ( x1 - p1.x ) * b.y / b.x ) ), nHeight - 6 );
				p1.x = x1;
				if( p1.x < nWidth / 4 )
				{
					p1.x = 0;
					p[0] = p[0].y > p[1].y ? p[0] : p[1];
					p[1] = p1;
				}
				else if( p1.x >= nWidth - nWidth / 4 )
				{
					p1.x = nWidth - 1;
					p[0] = p[0].y > p[1].y ? p[0] : p[1];
					p[1] = p1;
				}
				else
				{
					p1.y = Min( p1.y, Max( p[0].y, p[1].y ) + SRand::Inst().Rand( 8, 13 ) );
					p[0] = p[1] = p1;
				}
				auto rect = CreateChunk( p1, TRectangle<int32>( 0, p1.y, nWidth, nHeight - p1.y ) );
				if( rect.width > 0 )
					s += ( rect.x + rect.GetRight() - nWidth + SRand::Inst().Rand( 0, 2 ) ) / 2;
			}
		}
	}

	SRand::Inst().Shuffle( vecRects );
	for( int i = 0; i < vecRects.size(); i++ )
	{
		auto& rect = vecRects[i];
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				v1[x + y * nWidth] = i;
		}
		q1.push_back( i );
		vecGroups[i] = i;
	}
	int32 nGroupCount = q1.size();
	CUnionFind g;
	g.Init( nGroupCount );
	vector<bool> vecIsConn;
	vecIsConn.resize( nGroupCount * nGroupCount );
	int32 nConn = vecRects.size() - 1;
	for( int iq = 0; iq < q1.size() && nConn; iq++ )
	{
		auto n = q1[iq];
		auto rect = vecRects[n];
		vecFlag[n] = 1;
		int32 nGroup = vecGroups[n];
		q.resize( 0 );
		bool b = false;
		for( int y = rect.GetBottom(); y < Min( nHeight, rect.GetBottom() + 3 ) && !b; y++ )
		{
			for( int x = rect.x; x < rect.GetRight(); x++ )
			{
				int32 n1 = v1[x + y * nWidth];
				if( n1 >= 0 )
				{
					q.push_back( n1 );
					b = true;
				}
			}
		}
		if( rect.GetBottom() < nHeight )
		{
			bool b1 = SRand::Inst().Rand( 0, 2 );
			int32 y = rect.GetBottom();
			for( int i = 1; i < rect.width - 1; i++ )
			{
				int32 x = b1 ? i + rect.x : rect.GetRight() - 1 - i;
				auto rect1 = CreateChunk( TVector2<int32>( x, y ), TRectangle<int32>( 0, 0, nWidth, nHeight ) );
				if( rect1.width > 0 )
				{
					q.push_back( vecRects.size() - 1 );
					i = b1 ? rect1.GetRight() - 1 + SRand::Inst().Rand( 0, 3 ) : rect1.x - SRand::Inst().Rand( 0, 3 );
				}
			}
		}
		if( !b )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
					m_gendata[i + j * nWidth] = 0;
			}
			rect = PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), TVector2<int32>( rect.width, rect.height + 2 ),
				TRectangle<int32>( 0, rect.y, nWidth, nHeight - rect.y ), -1, eType_WallChunk, 0 );
			vecRects[n] = rect;
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
					v1[i + j * nWidth] = n;
			}
			for( int y = rect.GetBottom(); y < Min( nHeight, rect.GetBottom() + 3 ) && !b; y++ )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					int32 n1 = v1[x + y * nWidth];
					if( n1 >= 0 )
					{
						q.push_back( n1 );
						b = true;
					}
				}
			}
		}

		bool b1 = false;
		for( int x = rect.x - 1; x >= Max( 0, rect.x - 2 ) && !b1; x-- )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				int32 n1 = v1[x + y * nWidth];
				if( n1 >= 0 && vecRects[n1].y >= rect.y )
				{
					q.push_back( n1 );
					b1 = true;
					break;
				}
			}
		}
		if( !b1 && rect.x > 0 && rect.y > 0 )
		{
			int32 x = rect.x - 1;
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( v1[x + y * nWidth] < 0 )
				{
					int32 y1 = SRand::Inst().Rand( y, rect.GetBottom() );
					auto rect1 = CreateChunk( TVector2<int32>( x, y1 ), TRectangle<int32>( 0, y, nWidth, nHeight - y ) );
					if( rect1.width > 0 )
						q.push_back( vecRects.size() - 1 );
					break;
				}
			}
		}

		bool b2 = false;
		for( int x = rect.GetRight(); x < Min( nWidth, rect.GetRight() + 2 ) && !b2; x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				int32 n1 = v1[x + y * nWidth];
				if( n1 >= 0 && vecRects[n1].y >= rect.y )
				{
					q.push_back( n1 );
					b2 = true;
					break;
				}
			}
		}
		if( !b2 && rect.GetRight() < nWidth && rect.y > 0 )
		{
			int32 x = rect.GetRight();
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( v1[x + y * nWidth] < 0 )
				{
					int32 y1 = SRand::Inst().Rand( y, rect.GetBottom() );
					auto rect1 = CreateChunk( TVector2<int32>( x, y1 ), TRectangle<int32>( 0, y, nWidth, nHeight - y ) );
					if( rect1.width > 0 )
						q.push_back( vecRects.size() - 1 );
					break;
				}
			}
		}

		SRand::Inst().Shuffle( q );
		for( auto n1 : q )
		{
			int32& nGroup1 = vecGroups[n1];
			if( vecFlag[n1] )
			{
				if( nGroup1 >= 0 && nGroup1 != nGroup )
				{
					if( vecIsConn[nGroup1 + nGroup * nGroupCount] )
						continue;
					vecIsConn[nGroup1 + nGroup * nGroupCount] = vecIsConn[nGroup + nGroup1 * nGroupCount] = true;

					int32 m = n;
					for( ; m >= 0; )
					{
						if( vecFlag[m] != 1 )
							break;
						vecFlag[m] = 2;
						m = vecPars[m];
					}
					m = n1;
					for( ; m >= 0; )
					{
						if( vecFlag[m] != 1 )
							break;
						vecFlag[m] = 2;
						m = vecPars[m];
					}

					if( g.Union( nGroup1, nGroup ) )
					{
						nConn--;
						if( !nConn )
							break;
					}
				}
				continue;
			}
			if( vecPars[n1] >= 0 )
				continue;
			q1.push_back( n1 );
			vecPars[n1] = n;
			nGroup1 = nGroup;
		}
	}

	for( int i = 0; i < vecRects.size(); i++ )
	{
		auto& rect = vecRects[i];
		if( vecFlag[i] != 2 )
		{
			for( int x = rect.x; x < rect.GetRight(); x++ )
			{
				for( int y = rect.y; y < rect.GetBottom(); y++ )
					m_gendata[x + y * nWidth] = 0;
			}
		}
	}
	vecTemp.clear();
	for( int i = vecRects.size() - 1; i >= 0; i-- )
	{
		auto& rect = vecRects[i];
		if( vecFlag[i] == 2 )
		{
			int32 i1 = i;
			auto rect1 = vecRects[i1];
			int32 h1 = rect1.GetBottom();
			for( ;; )
			{
				int32 i2 = vecPars[i1];
				if( i2 < 0 || vecFlag[i2] != 2 )
					break;
				auto& rect2 = vecRects[i2];
				if( Max( rect1.x, rect2.x ) < Min( rect1.GetRight(), rect2.GetRight() ) )
					break;
				for( int x = rect2.x; x < rect2.GetRight(); x++ )
				{
					for( int y = rect2.y; y < rect2.GetBottom(); y++ )
					{
						vecTemp.push_back( TVector2<int32>( x, y ) );
						m_gendata[x + y * nWidth] = y == rect.y ? eType_Temp : 0;
					}
				}
				rect1 = rect1 + rect2;
				h1 = Min( h1, rect2.GetBottom() );
				i1 = i2;
				vecFlag[i1] = 3;
			}
			if( i1 != i )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						vecTemp.push_back( TVector2<int32>( x, y ) );
						m_gendata[x + y * nWidth] = y == rect.y ? eType_Temp : 0;
					}
				}
				vecFlag[i] = 3;
			}
		}
	}
	SRand::Inst().Shuffle( vecTemp );
	for( int i = vecRects.size() - 1; i >= 0; i-- )
	{
		if( vecFlag[i] == 2 )
			m_wallChunks.push_back( vecRects[i] );
	}
	vecRects.clear();

	for( int j = 0; j < nHeight; j++ )
	{
		int32 iBegin = 0;
		for( int32 iEnd = 0; iEnd <= nWidth; iEnd++ )
		{
			if( iEnd != nWidth && m_gendata[iEnd + j * nWidth] != eType_WallChunk )
				continue;
			int32 xBegin = iBegin;
			int32 xEnd = iEnd;
			iBegin = iEnd + 1;
			if( xEnd > xBegin )
			{
				while( xBegin < nWidth && !m_gendata[xBegin + j * nWidth] )
					xBegin++;
				while( xEnd > 0 && !m_gendata[( xEnd - 1 ) + j * nWidth] )
					xEnd--;
				if( xEnd <= xBegin )
					continue;
				int32 y0 = 0;
				for( int32 x = xBegin; x < xEnd; x++ )
				{
					m_gendata[x + j * nWidth] = eType_Temp;
					for( int32 y = j; y > y0; y-- )
					{
						if( y < j && m_gendata[x + y * nWidth] >= eType_Temp || m_gendata[x + ( y - 1 ) * nWidth] == eType_WallChunk )
						{
							y0 = y;
							break;
						}
					}
				}

				int32 y = j;
				if( y0 != j )
				{
					bool b = j >= y0 + SRand::Inst().Rand( 3, 6 );
					for( int32 x = xBegin; x < xEnd; x++ )
					{
						m_gendata[x + y0 * nWidth] = eType_Temp;
						if( !b )
							m_gendata[x + j * nWidth] = 0;
					}
					y = b ? j : y0;
				}
				while( xEnd < nWidth && !m_gendata[xEnd + y * nWidth] )
				{
					m_gendata[xEnd + y * nWidth] = eType_Temp1;
					xEnd++;
				}
				while( xBegin > 0 && !m_gendata[( xBegin - 1 ) + y * nWidth] )
				{
					m_gendata[xBegin + y * nWidth] = eType_Temp1;
					xBegin--;
				}
			}
		}
	}
	vector<TVector2<int32> > vecTemp1;
	for( int i = 0; i < nWidth; i++ )
	{
		int32 n0 = vecTemp1.size();
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp1 )
				m_gendata[i + j * nWidth] = 0;
			else if( m_gendata[i + j * nWidth] == eType_Temp )
				vecTemp1.push_back( TVector2<int32>( i, j ) );
		}
		if( vecTemp1.size() > n0 )
			SRand::Inst().Shuffle( &vecTemp1[n0], vecTemp1.size() - n0 );
	}
	for( auto& p : vecTemp1 )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 1 ), TVector2<int32>( nWidth, 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
		if( rect.width <= 0 )
			continue;
		rect = PutRectEx( m_gendata, nWidth, nHeight, rect, TVector2<int32>( rect.width, 2 ), TVector2<int32>( rect.width + SRand::Inst().Rand( 0, 6 ), 2 ),
			TRectangle<int32>( 0, rect.y, nWidth, nHeight - rect.y ),
			-1, eType_Temp1, [this, nWidth, nHeight] ( TRectangle<int32> rect, TRectangle<int32> rect1 ) -> bool {
			for( int x = rect1.x; x < rect1.GetRight(); x++ )
			{
				for( int y = rect1.y; y < rect1.GetBottom(); y++ )
				{
					if( m_gendata[x + y * nWidth] != 0 && m_gendata[x + y * nWidth] != eType_Temp )
						return false;
				}
			}
			return true;
		} );
		if( rect.width <= 0 )
			continue;
		vecRects.push_back( rect );
	}
	for( auto& i : m_gendata )
	{
		if( i == eType_Temp )
			i = 0;
	}
	for( auto& rect : vecRects )
	{
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				m_gendata[x + y * nWidth] = 0;
		}
		int32 nMaxHeight = Min( 4, 24 / rect.width ) + 2;
		rect = PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), TVector2<int32>( rect.width, nMaxHeight ), TRectangle<int32>( 0, 0, nWidth, rect.GetBottom() ), -1, eType_Temp1, 0 );
	}
	int32 n0 = vecRects.size();
	for( auto& p : vecTemp )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		TVector2<int32> vMin, vMax;
		vMin = TVector2<int32>( 4, 4 );
		vMax = TVector2<int32>( SRand::Inst().Rand( 6, 10 ), SRand::Inst().Rand( 4, 7 ) );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, vMin, vMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), 12, eType_Temp1 );
		if( rect.width <= 0 )
			continue;
		vecRects.push_back( rect );
		rect.y -= 4;
		rect.height += 8;
		rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( m_gendata[x + y * nWidth] == 0 )
					m_gendata[x + y * nWidth] = eType_Temp;
			}
		}
	}
	for( auto& i : m_gendata )
	{
		if( i == eType_Temp )
			i = 0;
	}

	for( auto& rect : m_wallChunks )
	{
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				v1[x + y * nWidth] = eType_Temp1;
		}
		vecRects.push_back( rect );
	}
	m_wallChunks.clear();
	LvGenLib::DropObj1( m_gendata, nWidth, nHeight, vecRects, 0, eType_Temp1, false );
	memset( &v1[0], -1, sizeof( int32 ) * v1.size() );
	for( int i = 0; i < vecRects.size(); i++ )
	{
		auto rect = vecRects[i];
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				v1[x + y * nWidth] = i;
		}
	}
	for( int i = 0; i < n0; i++ )
	{
		auto r = vecRects[i];
		int32 w = SRand::Inst().Rand( 14, 17 );
		if( r.width <= w )
			continue;
		int32 x1 = r.x, x2 = r.GetRight() - 1;
		int32 n10 = r.y > 0 ? v1[x1 + ( r.y - 1 ) * nWidth] : -1;
		int32 n11 = r.GetBottom() < nHeight ? v1[x1 + r.GetBottom() * nWidth] : -1;
		int32 n20 = r.y > 0 ? v1[x2 + ( r.y - 1 ) * nWidth] : -1;
		int32 n21 = r.GetBottom() < nHeight ? v1[x2 + r.GetBottom() * nWidth] : -1;
		for( x1++; x1 < r.GetRight(); x1++ )
		{
			int32 n0 = r.y > 0 ? v1[x1 + ( r.y - 1 ) * nWidth] : -1;
			int32 n1 = r.GetBottom() < nHeight ? v1[x1 + r.GetBottom() * nWidth] : -1;
			if( n10 == -1 )
				n10 = n0;
			else if( n10 != n0 )
				break;
			if( n11 == -1 )
				n11 = n1;
			else if( n11 != n1 )
				break;
		}
		for( x2--; x2 >= r.x; x2-- )
		{
			int32 n0 = r.y > 0 ? v1[x2 + ( r.y - 1 ) * nWidth] : -1;
			int32 n1 = r.GetBottom() < nHeight ? v1[x2 + r.GetBottom() * nWidth] : -1;
			if( n20 == -1 )
				n20 = n0;
			else if( n20 != n0 )
				break;
			if( n21 == -1 )
				n21 = n1;
			else if( n21 != n1 )
				break;
		}
		if( x1 >= r.GetRight() )
			x1 = r.x;
		else
			x1 = SRand::Inst().Rand( ( r.x + x1 ) / 2, x1 );
		if( x2 < r.x )
			x2 = r.GetRight() - 1;
		else
			x2 = SRand::Inst().Rand( ( x2 + 1 + r.GetRight() ) / 2, r.GetRight() );
		if( x2 - x1 + 1 < w )
		{
			int32 left = Max( x2 + 1 - w, r.x );
			int32 right = Min( x1 + w, r.GetRight() );
			x1 = SRand::Inst().Rand( left, right - w + 1 );
			x2 = x1 + w - 1;
		}
		TRectangle<int32> rect1( x1, r.y, x2 - x1 + 1, r.height );
		for( int x = r.x; x < r.GetRight(); x++ )
		{
			if( x >= x1 && x <= x2 )
				continue;
			for( int y = r.y; y < r.GetBottom(); y++ )
			{
				v1[x + y * nWidth] = -1;
				m_gendata[x + y * nWidth] = 0;
			}
		}
		vecRects[i] = rect1;
	}

	for( int32 i = 0; i < vecRects.size(); i++ )
	{
		auto& rect = vecRects[i];
		int8 nType;
		if( rect.width <= 0 )
			continue;
		if( rect.height <= 3 && i < n0 )
		{
			nType = eType_WallChunk1;
			m_wallChunks1.push_back( rect );
		}
		else if( rect.height <= SRand::Inst().Rand( 4, 6 ) && i < n0 )
		{
			nType = eType_WallChunk0;
			m_wallChunks0.push_back( rect );
		}
		else
		{
			nType = eType_WallChunk;
			m_wallChunks.push_back( rect );
		}
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = nType;
		}
	}
}

void CLevelGenNode1_3_0::GenRooms()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int32> vecDist;
	vecDist.resize( nWidth * nHeight );
	vector<TVector2<int32> > q;
	GenDistField( m_gendata, nWidth, nHeight, 0, vecDist, q );

	TVector2<int32> ofs[8] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } };
	for( int k = 0; k < 1; k++ )
	{
		vector<TVector2<int32> > q1;
		for( int i = q.size() - 1; i >= 0; i-- )
		{
			auto& p = q[i];
			int32 nDist = vecDist[p.x + p.y * nWidth];
			if( nDist <= 0 )
				break;
			bool b = true;
			int32 pp[8];
			for( int j = 0; j < ELEM_COUNT( ofs ); j++ )
			{
				auto p1 = p + ofs[j];
				p1.x = Min( nWidth - 1, Max( 0, p1.x ) );
				p1.y = Min( nHeight - 1, Max( 0, p1.y ) );
				if( p1.x <= 0 || p1.y <= 0 || p1.x >= nWidth - 1 || p1.y >= nHeight - 1 )
					pp[j] = nDist - 1;
				else
					pp[j] = vecDist[p1.x + p1.y * nWidth];
				if( pp[j] > nDist )
				{
					b = false;
					break;
				}
			}
			if( !b )
				continue;

			int32 n1 = 0;
			for( int j = 0; j < 4; j++ )
			{
				if( pp[j] != pp[j + 4] )
					n1++;
			}
			int32 n2 = 0;
			for( int j = 0; j < 8; j++ )
			{
				if( pp[j] != pp[j == 7 ? 0 : j + 1] )
					n2++;
			}
			if( n1 >= 2 && n2 > 2 )
			{
				q1.push_back( p );
			}
		}
		if( !q1.size() )
			break;
		for( auto& p : q1 )
			vecDist[p.x + p.y * nWidth]--;
		std::stable_sort( q.begin(), q.end(), [&vecDist, nWidth, nHeight] ( const TVector2<int32>& left, const TVector2<int32>& right ) {
			return vecDist[left.x + left.y * nWidth] < vecDist[right.x + right.y * nWidth];
		} );
	}
	SplitDistField( m_gendata, nWidth, nHeight, eType_Temp, vecDist, q );
	q.clear();
	GenDistField( m_gendata, nWidth, nHeight, 0, vecDist, q );

	for( int i = q.size() - 1; i >= 0; i-- )
	{
		auto& p = q[i];
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		if( vecDist[p.x + p.y * nWidth] <= 2 )
			continue;
		TVector2<int32> vMin, vMax;
		vMin = TVector2<int32>( 6, 6 );
		vMax = vMin + TVector2<int32>( SRand::Inst().Rand( 1, 4 ), SRand::Inst().Rand( 1, 4 ) );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, vMin, vMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Room );
		if( rect.width )
		{
			m_rooms.push_back( rect );
			rect.SetLeft( rect.x - SRand::Inst().Rand( 0, 3 ) );
			rect.SetTop( rect.y - SRand::Inst().Rand( 0, 3 ) );
			rect.SetRight( rect.GetRight() + SRand::Inst().Rand( 0, 3 ) );
			rect.SetBottom( rect.GetBottom() + SRand::Inst().Rand( 0, 3 ) );
			rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					if( !m_gendata[i + j * nWidth] && !SRand::Inst().Rand( 0, 4 ) )
						m_gendata[i + j * nWidth] = eType_Temp1;
				}
			}
		}
	}

	AddRooms();

	vector<int32> v1;
	v1.resize( nWidth * nHeight );
	memset( &v1[0], -1, sizeof( int32 ) * v1.size() );
	for( int i = 0; i < m_rooms.size(); i++ )
	{
		auto& rect = m_rooms[i].rect;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
				v1[x + y * nWidth] = i;
		}
	}
	vector<int8> vecFlag;
	vecFlag.resize( m_rooms.size() );
	vector<int8> temp;
	vector<TVector2<int32> > temp1;
	for( int iRoom = 0; iRoom < m_rooms.size(); iRoom++ )
	{
		if( vecFlag[iRoom] )
			continue;
		vector<int32> q;
		q.push_back( iRoom );
		vecFlag[iRoom] = true;

		for( int i = 0; i < q.size(); i++ )
		{
			auto n = q[i];
			auto& room = m_rooms[n].rect;
			auto bDoor = m_rooms[n].bDoor;
			int8& l = bDoor[0];
			int8& r = bDoor[1];
			int8& t = bDoor[2];
			int8& b = bDoor[3];
			l = r = t = b = 1;
			int32 qSize = q.size();
			for( int x = room.x + 1; x < room.GetRight() - 1; x++ )
			{
				for( int y = room.y - 1; y >= Max( 0, room.y - 3 ); y-- )
				{
					if( m_gendata[x + y * nWidth] == eType_Temp )
						break;
					if( v1[x + y * nWidth] >= 0 )
					{
						if( !vecFlag[v1[x + y * nWidth]] )
						{
							vecFlag[v1[x + y * nWidth]] = true;
							q.push_back( v1[x + y * nWidth] );
						}
						t = 0;
						break;
					}
				}
				for( int y = room.GetBottom(); y < Min( nHeight, room.GetBottom() + 3 ); y++ )
				{
					if( m_gendata[x + y * nWidth] == eType_Temp )
						break;
					if( v1[x + y * nWidth] >= 0 )
					{
						if( !vecFlag[v1[x + y * nWidth]] )
						{
							vecFlag[v1[x + y * nWidth]] = true;
							q.push_back( v1[x + y * nWidth] );
						}
						b = 0;
						break;
					}
				}
			}
			for( int y = room.y + 1; y < room.GetBottom() - 1; y++ )
			{
				for( int x = room.x - 1; x >= Max( 0, room.x - 3 ); x-- )
				{
					if( m_gendata[x + y * nWidth] == eType_Temp )
						break;
					if( v1[x + y * nWidth] >= 0 )
					{
						if( !vecFlag[v1[x + y * nWidth]] )
						{
							vecFlag[v1[x + y * nWidth]] = true;
							q.push_back( v1[x + y * nWidth] );
						}
						l = 0;
						break;
					}
				}
				for( int x = room.GetRight(); x < Min( nWidth, room.GetRight() + 3 ); x++ )
				{
					if( m_gendata[x + y * nWidth] == eType_Temp )
						break;
					if( v1[x + y * nWidth] >= 0 )
					{
						if( !vecFlag[v1[x + y * nWidth]] )
						{
							vecFlag[v1[x + y * nWidth]] = true;
							q.push_back( v1[x + y * nWidth] );
						}
						r = 0;
						break;
					}
				}
			}
			if( q.size() > qSize )
				SRand::Inst().Shuffle( &q[qSize], q.size() - qSize );

			uint8 nDoors = 4 - ( l + r + t + b );
			if( nDoors > 1 )
			{
				uint8 nDoors1 = SRand::Inst().Rand<int8>( 1, nDoors );
				uint8 n1[4] = { 0, 1, 2, 3 };
				SRand::Inst().Shuffle( n1, 4 );
				for( int i1 = 0; i1 < 4 && nDoors > nDoors1; i1++ )
				{
					if( bDoor[n1[i1]] )
						continue;
					bDoor[n1[i1]] = 0;
					nDoors--;
				}
			}
		}

		if( q.size() >= SRand::Inst().Rand( 4, 6 ) )
		{
			int32 n = q[0];
			auto& room = m_rooms[n].rect;
			m_wallChunks.push_back( room );
			for( int i = room.x; i < room.GetRight(); i++ )
			{
				for( int j = room.y; j < room.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_WallChunk;
				}
			}
			room = TRectangle<int32>( 0, 0, 0, 0 );
			for( int i = 0; i < q.size() - 1; i++ )
			{
				q[i] = q[i + 1];
			}
			q.pop_back();
		}

		int32 s = 0;
		int32 yMin = nHeight;
		for( int i = 0; i < q.size(); i++ )
		{
			auto& room = m_rooms[q[i]].rect;
			s += room.width * room.height;
			yMin = Min( yMin, room.y );
		}
		int32 nRoomType = Max<int32>( q.size() / SRand::Inst().Rand( 3, 5 ), s / SRand::Inst().Rand( 100, 125 ) );
		for( int i = 0; i < q.size(); i++ )
		{
			bool bRoomType = 0;
			auto& room = m_rooms[q[i]].rect;
			if( nRoomType && room.width + room.height >= 12 && room.y > Max( 6, yMin ) )
			{
				bRoomType = 1;
				nRoomType--;
			}
			auto bDoor = m_rooms[q[i]].bDoor;
			m_rooms[q[i]].bRoomType = bRoomType;
			int8& l = bDoor[0];
			int8& r = bDoor[1];
			int8& t = bDoor[2];
			int8& b = bDoor[3];

			bool bSpider = false;
			if( bRoomType )
			{
				int32 x[] = { room.x + 2, room.x + ( room.width + SRand::Inst().Rand( 0, 2 ) ) / 2, room.GetRight() - 2 };
				int32 y[] = { room.y + 2, room.y + ( room.height + SRand::Inst().Rand( 0, 2 ) ) / 2, room.GetBottom() - 2 };
				TRectangle<int32> rect( x[l - r + 1] - 1, y[t - b + 1] - 1, 2, 2 );
				bSpider = true;
				for( int i = 0; i < m_spiders.size(); i++ )
				{
					if( ( m_spiders[i].GetCenter() - rect.GetCenter() ).Length2() < 10 * 10 )
					{
						bSpider = false;
						break;
					}
				}
				if( bSpider )
					m_spiders.push_back( rect );
				else
				{
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						for( int j = rect.y; j < rect.GetBottom(); j++ )
							m_gendata[i + j * nWidth] = eType_Web;
					}
				}
				for( int i = room.x; i < room.GetRight(); i++ )
				{
					for( int j = room.y; j < room.GetBottom(); j++ )
					{
						if( i > room.x && i < room.GetRight() - 1 && j > room.y && j < room.GetBottom() - 1 )
						{
							TVector2<int32> dist( Max( rect.x - i, i - ( rect.GetRight() - 1 ) ), Max( rect.y - j, j - ( rect.GetBottom() - 1 ) ) );
							if( !SRand::Inst().Rand( 0, dist.Length2() + 1 ) )
								m_gendata[i + j * nWidth] = eType_Room1;
							else
								m_gendata[i + j * nWidth] = eType_Web;
						}
						else
							m_gendata[i + j * nWidth] = eType_Room1;
					}
				}
			}
			else
			{
				for( int i = room.x; i < room.GetRight(); i++ )
					for( int j = room.y; j < room.GetBottom(); j++ )
						m_gendata[i + j * nWidth] = eType_Room;

				bool bCrack[4] = { false, false, false, false };
				for( int i = room.x + 1; i < room.GetRight() - 1; i++ )
				{
					if( room.y > 0 && m_gendata[i + ( room.y - 1 ) * nWidth] == eType_Web )
						m_gendata[i + room.y * nWidth] = m_gendata[i + ( room.y + 1 ) * nWidth] = eType_Web;
					if( room.GetBottom() < nHeight - 1 && m_gendata[i + room.GetBottom() * nWidth] == eType_Web )
						m_gendata[i + ( room.GetBottom() - 1 ) * nWidth] = m_gendata[i + ( room.GetBottom() - 2 ) * nWidth] = eType_Web;
				}
				for( int j = room.y + 1; j < room.GetBottom() - 1; j++ )
				{
					if( room.x > 0 && m_gendata[room.x - 1 + j * nWidth] == eType_Web )
						m_gendata[room.x + j * nWidth] = m_gendata[room.x + 1 + j * nWidth] = eType_Web;
					if( room.GetRight() < nWidth - 1 && m_gendata[room.GetRight() + j * nWidth] == eType_Web )
						m_gendata[room.GetRight() - 1 + j * nWidth] = m_gendata[room.GetRight() - 2 + j * nWidth] = eType_Web;
				}

				if( SRand::Inst().Rand( 0, 2 ) )
					m_gendata[room.x + 1 + ( room.y + 1 ) * nWidth] = eType_Web;
				if( SRand::Inst().Rand( 0, 2 ) )
					m_gendata[room.GetRight() - 2 + ( room.y + 1 ) * nWidth] = eType_Web;
				if( SRand::Inst().Rand( 0, 2 ) )
					m_gendata[room.x + 1 + ( room.GetBottom() - 2 ) * nWidth] = eType_Web;
				if( SRand::Inst().Rand( 0, 2 ) )
					m_gendata[room.GetRight() - 2 + ( room.GetBottom() - 2 ) * nWidth] = eType_Web;
				int32 k = SRand::Inst().Rand( 0, 3 );
				for( ; k > 0; k-- )
				{
					TVector2<int32> p( SRand::Inst().Rand( room.x + 1, room.GetRight() - 1 ),
						SRand::Inst().Rand( room.y + 1, room.GetBottom() - 1 ) );
					m_gendata[p.x + p.y * nWidth] = eType_Web;
					if( SRand::Inst().Rand( 0, 2 ) )
					{
						TVector2<int32> p1 = p;
						for( ;; )
						{
							p1.x += SRand::Inst().Rand( 2, 5 );
							p1.y += SRand::Inst().Rand( -1, 2 );
							p1.y = Min( Max( p1.y, room.y + 1 ), room.GetBottom() - 2 );
							if( p1.x >= room.GetRight() - 1 )
								break;
							m_gendata[p1.x + p1.y * nWidth] = eType_Web;
						}
						p1 = p;
						for( ;; )
						{
							p1.x -= SRand::Inst().Rand( 2, 5 );
							p1.y += SRand::Inst().Rand( -1, 2 );
							p1.y = Min( Max( p1.y, room.y + 1 ), room.GetBottom() - 2 );
							if( p1.x <= room.x )
								break;
							m_gendata[p1.x + p1.y * nWidth] = eType_Web;
						}
					}
					else
					{
						TVector2<int32> p1 = p;
						for( ;; )
						{
							p1.y += SRand::Inst().Rand( 2, 5 );
							p1.x += SRand::Inst().Rand( -1, 2 );
							p1.x = Min( Max( p1.x, room.x + 1 ), room.GetRight() - 2 );
							if( p1.y >= room.GetBottom() - 1 )
								break;
							m_gendata[p1.x + p1.y * nWidth] = eType_Web;
						}
						p1 = p;
						for( ;; )
						{
							p1.y -= SRand::Inst().Rand( 2, 5 );
							p1.x += SRand::Inst().Rand( -1, 2 );
							p1.x = Min( Max( p1.x, room.x + 1 ), room.GetRight() - 2 );
							if( p1.y <= room.y )
								break;
							m_gendata[p1.x + p1.y * nWidth] = eType_Web;
						}
					}
				}

				temp.resize( ( room.width - 2 ) * ( room.height - 2 ) );
				for( int i = 0; i < room.width - 2; i++ )
					for( int j = 0; j < room.height - 2; j++ )
						temp[i + j * ( room.width - 2 )] = m_gendata[i + room.x + 1 + ( j + room.y + 1 ) * nWidth] == eType_Room ? 0 : 1;
				FloodFillExpand( temp, room.width - 2, room.height - 2, 1, 0, temp.size() * SRand::Inst().Rand( 0.2f, 0.3f ) );

				for( int i = 0; i < room.width - 2; i++ )
					for( int j = 0; j < room.height - 2; j++ )
						m_gendata[i + room.x + 1 + ( j + room.y + 1 ) * nWidth] = temp[i + j * ( room.width - 2 )] == 1 ? eType_Web : eType_Room;
			}
			int32 nCracks = bRoomType ? 3 + (int32)( ( room.width + room.height - 4 ) * SRand::Inst().Rand( 0.25f, 0.3f ) ) : SRand::Inst().Rand( 2, 5 );
			temp1.resize( 0 );
			for( int i = room.x + 1; i < room.GetRight() - 1; i++ )
			{
				if( m_gendata[i + ( room.y + 1 ) * nWidth] == eType_Web )
					temp1.push_back( TVector2<int32>( i, room.y ) );
				if( m_gendata[i + ( room.GetBottom() - 2 ) * nWidth] == eType_Web )
					temp1.push_back( TVector2<int32>( i, room.GetBottom() - 1 ) );
			}
			for( int j = room.y + 1; j < room.GetBottom() - 1; j++ )
			{
				if( m_gendata[room.x + 1 + j * nWidth] == eType_Web )
					temp1.push_back( TVector2<int32>( room.x, j ) );
				if( m_gendata[room.GetRight() - 2 + j * nWidth] == eType_Web )
					temp1.push_back( TVector2<int32>( room.GetRight() - 1, j ) );
			}
			SRand::Inst().Shuffle( temp1 );
			nCracks = Min<int32>( nCracks, temp1.size() );
			for( int k = 0; k < nCracks; k++ )
			{
				auto& p = temp1[k];
				m_gendata[p.x + p.y * nWidth] = eType_Web;
			}

			int32 n = room.width - 2;
			int32 x, y;
			if( !t )
			{
				bool bCrack = false;
				for( int i = room.x + 1; i < room.GetRight() - 1; i++ )
				{
					if( m_gendata[i + room.y * nWidth] == eType_Web )
					{
						bCrack = true;
						break;
					}
				}
				if( !bCrack )
				{
					int32 x = SRand::Inst().Rand( room.x + 1, room.GetRight() - 1 );
					m_gendata[x + room.y * nWidth] = eType_Web;
				}
			}
			if( !b )
			{
				bool bCrack = false;
				for( int i = room.x + 1; i < room.GetRight() - 1; i++ )
				{
					if( m_gendata[i + ( room.GetBottom() - 1 ) * nWidth] == eType_Web )
					{
						bCrack = true;
						break;
					}
				}
				if( !bCrack )
				{
					int32 x = SRand::Inst().Rand( room.x + 1, room.GetRight() - 1 );
					m_gendata[x + ( room.GetBottom() - 1 ) * nWidth] = eType_Web;
				}
			}

			n = room.height - 2;
			if( !l )
			{
				bool bCrack = false;
				for( int i = room.y + 1; i < room.GetBottom() - 1; i++ )
				{
					if( m_gendata[room.x + i * nWidth] == eType_Web )
					{
						bCrack = true;
						break;
					}
				}
				if( !bCrack )
				{
					int32 y = SRand::Inst().Rand( room.y + 1, room.GetBottom() - 1 );
					m_gendata[room.x + y * nWidth] = eType_Web;
				}
			}
			if( !r )
			{
				bool bCrack = false;
				for( int i = room.y + 1; i < room.GetBottom() - 1; i++ )
				{
					if( m_gendata[room.GetRight() - 1 + i * nWidth] == eType_Web )
					{
						bCrack = true;
						break;
					}
				}
				if( !bCrack )
				{
					int32 y = SRand::Inst().Rand( room.y + 1, room.GetBottom() - 1 );
					m_gendata[room.GetRight() - 1 + y * nWidth] = eType_Web;
				}
			}
			m_rooms[q[i]].bSpider = bSpider;
		}
	}

	for( int i = m_rooms.size() - 1; i >= 0; i-- )
	{
		if( m_rooms[i].rect.width <= 0 )
		{
			m_rooms[i] = m_rooms.back();
			m_rooms.pop_back();
		}
	}
	GenDoors();

	for( int iRoom = 0; iRoom < m_rooms.size(); iRoom++ )
	{
		if( m_rooms[iRoom].bSpider )
		{
			auto& room = m_rooms[iRoom].rect;
			int8& l = m_rooms[iRoom].bDoor[0];
			int8& r = m_rooms[iRoom].bDoor[1];
			int8& t = m_rooms[iRoom].bDoor[2];
			int8& b = m_rooms[iRoom].bDoor[3];
			for( int y = room.y + 1; y < room.GetBottom() - 1; y++ )
			{
				int32 nType;
				if( l - r < 0 )
					nType = eType_Room_Pipe0;
				else if( l - r > 0 )
					nType = eType_Room_Pipe2;
				else
					nType = SRand::Inst().Rand( 0, 2 ) ? eType_Room_Pipe0 : eType_Room_Pipe2;
				int32 x = nType == eType_Room_Pipe0 ? room.x + 1 : room.GetRight() - 2;
				int32 x1 = nType == eType_Room_Pipe0 ? room.x : room.GetRight() - 1;
				if( m_gendata[x1 + y * nWidth] == eType_Door || m_gendata[x1 + y * nWidth] == eType_Web )
					continue;
				if( m_gendata[x + y * nWidth] != eType_Web || !SRand::Inst().Rand( 0, 3 ) )
					m_gendata[x + y * nWidth] = nType;
			}
			for( int x = room.x + 1; x < room.GetRight() - 1; x++ )
			{
				int32 nType;
				if( t - b < 0 )
					nType = eType_Room_Pipe1;
				else if( t - b > 0 )
					nType = eType_Room_Pipe3;
				else
					nType = SRand::Inst().Rand( 0, 2 ) ? eType_Room_Pipe1 : eType_Room_Pipe3;
				int32 y = nType == eType_Room_Pipe1 ? room.y + 1 : room.GetBottom() - 2;
				int32 y1 = nType == eType_Room_Pipe1 ? room.y : room.GetBottom() - 1;
				if( m_gendata[x + y1 * nWidth] == eType_Door || m_gendata[x + y1 * nWidth] == eType_Web )
					continue;
				if( m_gendata[x + y * nWidth] != eType_Web || !SRand::Inst().Rand( 0, 3 ) )
					m_gendata[x + y * nWidth] = nType;
			}
		}
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp1 )
			{
				m_vecTemp1.push_back( TVector2<int32>( i, j ) );
				m_gendata[i + j * nWidth] = 0;
			}
		}
	}

	for( auto& rect : m_wallChunks1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = 0;
		}
	}
	LvGenLib::GenObjs2( m_gendata, nWidth, nHeight, 0, eType_Temp, 0.16f );
	for( auto& rect : m_wallChunks1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = eType_WallChunk1;
		}
	}
}

void CLevelGenNode1_3_0::AddRooms()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	int32 s = 0;
	vector<TVector2<int32> > q;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Room )
				s++;
			if( m_gendata[i + j * nWidth] != eType_WallChunk && m_gendata[i + j * nWidth] != eType_Temp )
				vecTemp[i + j * nWidth] = 0;
			else
				vecTemp[i + j * nWidth] = 1;
		}
	}
	ExpandDist( vecTemp, nWidth, nHeight, 1, 0, 1 );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( !vecTemp[i + j * nWidth] )
			{
				FloodFill( vecTemp, nWidth, nHeight, i, j, 2, q );
				bool b = q.size() < 35;
				if( b && q.size() >= 24 )
				{
					for( auto& p : q )
					{
						if( m_gendata[p.x + p.y * nWidth] != 0 )
						{
							b = false;
							break;
						}
					}
				}
				q.clear();
				if( b )
					FloodFill( vecTemp, nWidth, nHeight, i, j, 1 );
			}
		}
	}

	vector<int32> vecDist;
	vecDist.resize( nWidth * nHeight );
	GenDistField( vecTemp, nWidth, nHeight, 1, vecDist, q, false );

	struct STemp
	{
		int32 nWallChunk;
		float f;
	};
	vector<STemp> vec;
	vec.resize( m_wallChunks.size() );
	for( int i = 0; i < m_wallChunks.size(); i++ )
	{
		auto& rect = m_wallChunks[i];
		auto& item = vec[i];
		item.nWallChunk = i;
		int32 s = 0, nMin = 0x7fffffff;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				s += vecDist[x + y * nWidth];
				nMin = Min( nMin, vecDist[x + y * nWidth] );
			}
		}
		item.f = Max<float>( s * 1.0f / ( rect.width * rect.height ) - Min( rect.width, rect.height ) / 4.0f, nMin ) + SRand::Inst().Rand( 0.0f, 0.1f );
		item.f += Max( 0.0f, nWidth / 4 - CVector2( ( rect.x + rect.GetRight() - nWidth ) * 0.5f, rect.y * 2.0f ).Length() );
	}
	std::sort( vec.begin(), vec.end(), [] ( const STemp& left, const STemp& right ) -> bool {
		return left.f > right.f;
	} );

	int32 targetS = nWidth * nHeight / 5;
	for( int i = 0; i < vec.size() && ( s < targetS || vec[i].f > 3.5f ); i++ )
	{
		int32 n = vec[i].nWallChunk;
		s += m_wallChunks[n].width * m_wallChunks[n].height;
		m_rooms.push_back( m_wallChunks[n] );
		m_wallChunks[n] = TRectangle<int32>( 0, 0, 0, 0 );
	}

	for( int i = m_wallChunks.size() - 1; i >= 0; i-- )
	{
		if( m_wallChunks[i].width <= 0 )
		{
			m_wallChunks[i] = m_wallChunks.back();
			m_wallChunks.pop_back();
		}
	}
}

void CLevelGenNode1_3_0::GenDoors()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int8> vec;
	vec.resize( nWidth * nHeight );
	vector<int32> vec1;
	vec1.resize( nWidth * nHeight );
	memset( &vec1[0], -1, sizeof( int32 ) * vec1.size() );
	for( int i = 0; i < m_wallChunks.size(); i++ )
	{
		auto& rect = m_wallChunks[i];
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				vec1[x + y * nWidth] = i;
			}
		}
	}

	for( auto& room : m_rooms )
	{
		auto& rect = room.rect;
		int32 iBegins[] = { rect.y + 1, rect.y + 1, rect.x + 1, rect.x + 1 };
		int32 iEnds[] = { rect.GetBottom() - 1, rect.GetBottom() - 1, rect.GetRight() - 1, rect.GetRight() - 1 };
		int32 js[] = { rect.x, rect.GetRight() - 1, rect.y, rect.GetBottom() - 1 };
		TVector2<int32> ofss[] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
		for( int iDoor = 0; iDoor < 4; iDoor++ )
		{
			bool bDoor = room.bDoor[iDoor];
			if( !bDoor )
				continue;

			int32 iBegin = iBegins[iDoor];
			int32 iEnd = iEnds[iDoor];
			TVector2<int32> ofs = ofss[iDoor];

			float fMax = 0;
			int32 iMax = -1;
			int32 l0Max = 0;
			int32 lMax = 0;
			int8 nMax = -1;
			int32 n1Max = -1;
			int32 i0 = iBegin;
			int32 lLast = 0;
			int8 nLast = -1;
			int32 n1Last = -1;
			
			for( int i = iBegin; i <= iEnd; i++ )
			{
				int8 n = -1;
				int32 n1 = -1;
				int32 l = 0;
				if( i < iEnd )
				{
					int32 x = iDoor < 2 ? js[iDoor] : i;
					int32 y = iDoor < 2 ? i : js[iDoor];
					TVector2<int32> p( x, y );
					for( ; l < 3; l++ )
					{
						if( vec1[p.x + p.y * nWidth] >= 0 )
							break;
						auto p1 = p + ofs;
						if( p1.x < 0 || p1.y < 0 || p1.x >= nWidth || p1.y >= nHeight )
							break;
						p = p1;
					}

					n = vec[p.x + p.y * nWidth];
					n1 = vec1[p.x + p.y * nWidth];
				}

				if( n != nLast || n1 != n1Last )
				{
					if( i - i0 > 1 )
					{
						float f = i - i0 + SRand::Inst().Rand( 0.0f, 1.0f ) + nLast * 1000000.0f;
						if( f >= fMax )
						{
							fMax = f;
							iMax = i0;
							l0Max = i - i0;
							lMax = lLast;
							nMax = nLast;
							n1Max = n1Last;
						}
					}

					i0 = i;
					lLast = l;
					nLast = n;
					n1Last = n1;
				}
			}

			int32 nDoorBegin, nDoorEnd;
			if( iMax >= 0 )
			{
				nDoorBegin = iMax + ( l0Max - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				nDoorEnd = nDoorBegin + 2;

				if( n1Max >= 0 )
				{
					for( int i = nDoorBegin; i < nDoorEnd; i++ )
					{
						int32 x = iDoor < 2 ? js[iDoor] : i;
						int32 y = iDoor < 2 ? i : js[iDoor];
						TVector2<int32> p( x, y );
						p = p + ofs * lMax;
						m_gendata[p.x + p.y * nWidth] = eType_Temp2;
						auto& r1 = m_wallChunks[n1Max];
						for( ; p.x >= r1.x && p.y >= r1.y && p.x < r1.GetRight() && p.y < r1.GetBottom(); p = p + ofs )
							vec[p.x + p.y * nWidth] = 1;
					}
				}
			}
			else
			{
				nDoorBegin = ( iBegin + iEnd + SRand::Inst().Rand( 0, 2 ) ) / 2 - 1;
				nDoorEnd = nDoorBegin + 2;
			}

			int8 nDoorType = !room.bRoomType ? eType_Door : eType_Web;
			if( nDoorType == eType_Door )
			{
				for( int i = nDoorBegin - 1; i < nDoorEnd + 1; i++ )
				{
					int32 x = iDoor < 2 ? js[iDoor] : i;
					int32 y = iDoor < 2 ? i : js[iDoor];
					if( m_gendata[x + y * nWidth] == eType_Web )
					{
						nDoorType = eType_Web;
						break;
					}
				}
			}
			for( int i = nDoorBegin; i < nDoorEnd; i++ )
			{
				int32 x = iDoor < 2 ? js[iDoor] : i;
				int32 y = iDoor < 2 ? i : js[iDoor];
				m_gendata[x + y * nWidth] = nDoorType;
			}
		}
	}
}

void CLevelGenNode1_3_0::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<int8> vec;
	vec.resize( nWidth * nHeight );
	vector<int8> vec1;
	vec1.resize( nWidth * nHeight );
	vector<TVector2<int32> > vecTemp;
	for( int i = 0; i < vec.size(); i++ )
	{
		vec[i] = m_gendata[i] == eType_None || m_gendata[i] == eType_WallChunk1 ? 3 : 1;
	}
	FindAllOfTypesInMap( vec, nWidth, nHeight, 3, vecTemp );
	SRand::Inst().Shuffle( vecTemp );
	vector<TVector2<int32> > q;
	vector<TRectangle<int32> > vecObjs;
	for( auto& p : vecTemp )
	{
		if( vec[p.x + p.y * nWidth] != 3 )
			continue;
		FloodFill( vec, nWidth, nHeight, p.x, p.y, 4, SRand::Inst().Rand( 60, 70 ), q );
		if( q.size() >= SRand::Inst().Rand( 8, 15 ) )
		{
			for( auto& p1 : q )
			{
				if( vec[p1.x + p1.y * nWidth] != 4 )
					continue;
				if( p1.y < 6 )
					continue;
				auto rect = PutRect( vec, nWidth, nHeight, p1, TVector2<int32>( 3, 2 ), TVector2<int32>( 3, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 2 );
				if( rect.width > 0 )
				{
					vecObjs.push_back( rect );
					for( int x = rect.x; x < rect.GetRight(); x++ )
						m_vecTemp1.push_back( TVector2<int32>( x, rect.y - 1 ) );
					break;
				}
			}
		}
		for( auto& p1 : q )
		{
			if( vec[p1.x + p1.y * nWidth] == 4 )
				vec[p1.x + p1.y * nWidth] = 0;
		}
		q.clear();
	}

	for( auto& rect : m_wallChunks1 )
	{
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( y > 0 && vec[x + ( y - 1 )] * nWidth )
					m_vecTemp1.push_back( TVector2<int32>( x, y ) );
			}
		}
	}
	SRand::Inst().Shuffle( m_vecTemp1 );

	for( auto& p : m_vecTemp1 )
	{
		if( vec[p.x + p.y * nWidth] || vec1[p.x + p.y * nWidth] )
			continue;
		if( p.y <= 6 )
			continue;

		auto rect = PutRectEx( vec, nWidth, nHeight, p, TVector2<int32>( 6, 1 ), TVector2<int32>( SRand::Inst().Rand( 8, 12 ), 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 2,
			[&vec, &vec1, nWidth, nHeight] ( TRectangle<int32> rect, TRectangle<int32> rect1 ) -> bool {
			for( int i = rect1.x; i < rect1.GetRight(); i++ )
			{
				for( int j = rect1.y; j < rect1.GetBottom(); j++ )
				{
					if( vec[i + j * nWidth] || vec1[i + j * nWidth] )
						return false;
				}
			}
			return true;
		} );
		if( rect.width > 0 )
		{
			vecObjs.push_back( rect );
			rect.x -= 1;
			rect.width += 2;
			rect.y -= 4;
			rect.height += 8;
			rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );
			for( int x = rect.x; x < rect.GetRight(); x++ )
			{
				for( int y = rect.y; y < rect.GetBottom(); y++ )
					vec1[x + y * nWidth] = 1;
			}
		}
	}
	memset( &vec1[0], 0, vec1.size() );

	for( auto& p : m_vecTemp1 )
	{
		if( vec[p.x + p.y * nWidth] || vec1[p.x + p.y * nWidth] )
			continue;
		if( p.y <= 6 )
			continue;

		auto rect = PutRectEx( vec, nWidth, nHeight, p, TVector2<int32>( 1, 5 ), TVector2<int32>( 2, SRand::Inst().Rand( 7, 10 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 2,
			[&vec, &vec1, nWidth, nHeight] ( TRectangle<int32> rect, TRectangle<int32> rect1 ) -> bool {
			for( int i = rect1.x; i < rect1.GetRight(); i++ )
			{
				for( int j = rect1.y; j < rect1.GetBottom(); j++ )
				{
					if( vec[i + j * nWidth] || vec1[i + j * nWidth] )
						return false;
				}
			}
			return true;
		} );
		if( rect.width > 0 )
		{
			while( rect.y > 0 )
			{
				int32 y = rect.y - 1;
				bool b = false;
				bool b1 = false;
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					if( vec[x + y * nWidth] )
					{
						b = true;
						break;
					}
					if( m_gendata[x + y * nWidth] == eType_WallChunk1 )
						b1 = true;
				}
				if( b )
					break;
				if( b1 )
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
						vec[x + y * nWidth] = 2;
					rect.y--;
					rect.height++;
				}
				else
				{
					int32 x = SRand::Inst().Rand( rect.x, rect.GetRight() );
					auto r = PutRectEx( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 4, 4 ), TVector2<int32>( SRand::Inst().Rand( 4, 7 ), 4 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_WallChunk,
						[this, &vec, nWidth, nHeight ] ( TRectangle<int32> rect, TRectangle<int32> rect1 ) -> bool {
						for( int i = rect1.x; i < rect1.GetRight(); i++ )
						{
							for( int j = rect1.y; j < rect1.GetBottom(); j++ )
							{
								if( vec[i + j * nWidth] || m_gendata[i + j * nWidth] )
									return false;
							}
						}
						return true;
					} );
					if( r.width > 0 )
					{
						for( int i = r.x; i < r.GetRight(); i++ )
						{
							for( int j = r.y; j < r.GetBottom(); j++ )
							{
								vec[i + j * nWidth] = 1;
							}
						}
						m_wallChunks.push_back( r );
					}
					else
					{
						for( int x = rect.x; x < rect.GetRight(); x++ )
						{
							vec[x + y * nWidth] = 1;
							m_gendata[x + y * nWidth] = eType_Temp;
						}
					}

					break;
				}
			}
			
			vecObjs.push_back( rect );
			rect.x -= 4;
			rect.width += 8;
			rect.y -= 1;
			rect.height += 2;
			rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );
			for( int x = rect.x; x < rect.GetRight(); x++ )
			{
				for( int y = rect.y; y < rect.GetBottom(); y++ )
					vec1[x + y * nWidth] = 1;
			}
		}
	}
	LvGenLib::DropObj1( vec, nWidth, nHeight, vecObjs, 0, 2 );

	for( auto& rect : vecObjs )
	{
		if( rect.y <= 0 )
			continue;
		int32 y = rect.y - 1;
		int32 s = 0;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			if( vec[x + y * nWidth] )
				s++;
		}
		int32 n = rect.width * 2 / 3 - s;
		int8 b = SRand::Inst().Rand( 0, 2 );
		for( int i = 0; i < rect.width && n > 0; i++ )
		{
			int32 x = b ? rect.x + i : rect.GetRight() - 1 - i;
			if( m_gendata[x + y * nWidth] == 0 )
			{
				if( n > 6 )
				{
					auto r = PutRectEx( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 4, 4 ), TVector2<int32>( SRand::Inst().Rand( 4, 7 ), 4 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_WallChunk,
						[this, &vec, nWidth, nHeight]( TRectangle<int32> rect, TRectangle<int32> rect1 ) -> bool {
						for( int i = rect1.x; i < rect1.GetRight(); i++ )
						{
							for( int j = rect1.y; j < rect1.GetBottom(); j++ )
							{
								if( vec[i + j * nWidth] || m_gendata[i + j * nWidth] )
									return false;
							}
						}
						return true;
					} );
					if( r.width > 0 )
					{
						for( int i = r.x; i < r.GetRight(); i++ )
						{
							for( int j = r.y; j < r.GetBottom(); j++ )
							{
								vec[i + j * nWidth] = 1;
							}
						}
						m_wallChunks.push_back( r );
						n--;
						continue;
					}
				}
				m_gendata[x + y * nWidth] = eType_Temp;
				vec[x + y * nWidth] = 1;
				n--;
			}
		}
	}
	for( auto& rect : vecObjs )
	{
		if( rect.width == 3 && rect.height == 2 )
			m_acs.push_back( rect );
		else
			m_scraps.push_back( rect );
	}

	m_vecTemp1.clear();

	__LvGen1_3__::GenWallChunk( m_gendata, nWidth, nHeight, m_wallChunks, eType_WallChunk, eType_WallChunk_1, eType_WallChunk0, eType_WallChunk_0_1, eType_WallChunk_0_1a, eType_WallChunk_0_1b, -1, -1, eType_Temp2 );
	__LvGen1_3__::GenWindows( m_gendata, nWidth, nHeight, m_wallChunks, m_wallChunks1, m_windows, m_windows1, eType_WallChunk, eType_WallChunk_0, eType_WallChunk1, eType_WallChunk1_2, eType_WallChunk_0_1, eType_WallChunk_0_1b, eType_WallChunk_1, -1 );

	LvGenLib::GenObjs( vec, nWidth, nHeight, 0, 0, 3, 3, 0.15f );
	LvGenLib::DropObjs( vec, nWidth, nHeight, 0, 3 );
	for( int i = 0; i < vec.size(); i++ )
	{
		if( vec[i] == 3 )
			m_gendata[i] = eType_Temp1;
	}
	for( auto& rect : m_scraps )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] != eType_Temp1 && !SRand::Inst().Rand( 0, 3 ) )
					m_gendata[i + j * nWidth] = eType_Temp1;
			}
		}
	}

	int8 nTypes[] = { eType_BlockRed, eType_BlockBlue };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_Temp, nTypes, ELEM_COUNT( nTypes ) );
	int8 nTypes1[] = { eType_Crate1, eType_Crate2 };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_Temp1, nTypes1, ELEM_COUNT( nTypes1 ) );
}
