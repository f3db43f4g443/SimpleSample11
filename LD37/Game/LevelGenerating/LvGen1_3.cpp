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
	GenWallChunk0();
	GenWindows();
	GenShops();

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

			if( genData == eType_Wall || genData == eType_Crate1 || genData == eType_Crate2 )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["0"] = eType_WallChunk_0;
	context.mapTags["0_1"] = eType_WallChunk_0_1;
	context.mapTags["0_1a"] = eType_WallChunk_0_1a;
	context.mapTags["0_1b"] = eType_WallChunk_0_1b;
	context.mapTags["1"] = eType_WallChunk_1;
	context.mapTags["2"] = eType_WallChunk_2;
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

	m_gendata.clear();
	m_rooms.clear();
	m_wallChunks.clear();
	m_wallChunks0.clear();
	m_wallChunks1.clear();
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
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin + nHeight1, right - lastRect.x, nCurHeight - nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
				}
			}
			else
			{
				left = 0;
				if( nType == 0 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, lastRect.GetRight() - left, nCurHeight - nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( right - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
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
				m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin + nHeight1, lastRect.width, nCurHeight - nHeight1 ) );
			}
			else if( nType == 1 )
			{
				m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w1, nHeight1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( lastRect.GetRight() - w2, nCurBegin, w2, nHeight1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
			}
			else if( nType == 2 )
			{
				m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin, w1, nHeight1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( right - w2, nCurBegin, w2, nHeight1 ) );
				m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
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
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin + nHeight1, right - lastRect.x, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( lastRect.x + w, nCurBegin, nWidth - lastRect.x - w, nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.x, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( lastRect.x + w, nCurBegin, nWidth - lastRect.x - w, nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( left + w, nCurBegin, nWidth - left - w, nHeight1 ) );
				}
			}
			else
			{
				if( nType == 0 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, lastRect.GetRight() - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( 0, nCurBegin, lastRect.GetRight() - w, nHeight1 ) );
				}
				else if( nType == 1 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( lastRect.GetRight() - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
					GenSubArea( TRectangle<int32>( 0, nCurBegin, lastRect.GetRight() - w, nHeight1 ) );
				}
				else if( nType == 2 )
				{
					m_wallChunks1.push_back( TRectangle<int32>( right - w, nCurBegin, w, nHeight1 ) );
					m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nHeight1, right - left, nCurHeight - nHeight1 ) );
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
			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1 + h2, right - left, h3 ) );

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
			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nCurHeight - h3, right - left, h3 ) );
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

			m_rooms.push_back( TRectangle<int32>( left1, nCurBegin + h0, w1, h1 ) );
			m_wallChunks1.push_back( TRectangle<int32>( 0, nCurBegin + h0, left1, h1 ) );
			m_wallChunks1.push_back( TRectangle<int32>( left1 + w1, nCurBegin + h0, nWidth - left1 - w1, h1 ) );

			m_wallChunks0.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1, w, h2 ) );
			GenSubArea( TRectangle<int32>( 0, nCurBegin + h0 + h1, left, h2 ) );
			GenSubArea( TRectangle<int32>( left + w, nCurBegin + h0 + h1, nWidth - left - w, h2 ) );

			left = SRand::Inst().Rand( 4, 6 );
			int32 right = nWidth - SRand::Inst().Rand( 4, 6 );
			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1 + h2, right - left, h3 ) );

			if( nLastRect >= 0 )
				FillLine( m_wallChunks1[nLastRect] );
			FillLine( m_wallChunks1.back() );

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

			m_rooms.push_back( TRectangle<int32>( 0, nCurBegin + h0, w1, h1 ) );
			m_rooms.push_back( TRectangle<int32>( nWidth - w2, nCurBegin + h0, w2, h1 ) );
			m_wallChunks1.push_back( TRectangle<int32>( w1, nCurBegin + h0, nWidth - w2 - w1, h1 ) );

			m_wallChunks0.push_back( TRectangle<int32>( 0, nCurBegin + h0 + h1, w10, h2 ) );
			m_wallChunks0.push_back( TRectangle<int32>( nWidth - w11, nCurBegin + h0 + h1, w11, h2 ) );
			GenSubArea( TRectangle<int32>( w10, nCurBegin + h0 + h1, nWidth - w10 - w11, h2 ) );

			int32 left = SRand::Inst().Rand( 4, 6 );
			int32 right = nWidth - SRand::Inst().Rand( 4, 6 );
			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1 + h2, right - left, h3 ) );

			if( nLastRect >= 0 )
				FillLine( m_wallChunks1[nLastRect] );
			FillLine( m_wallChunks1.back() );

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
			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + nCurHeight - h3, right - left, h3 ) );

			if( nLastRect >= 0 )
				FillLine( m_wallChunks1[nLastRect] );
			break;
		}
		}

		int32 nSpiders = m_spiders.size();
		if( nRoomCount < m_rooms.size() )
		{
			TRectangle<int32> rect( 0, 0, 0, 0 );
			for( int iRoom = nRoomCount; iRoom < m_rooms.size(); iRoom++ )
			{
				auto& room = m_rooms[iRoom];
				rect = rect.width ? rect + room.rect : room.rect;
			}
			int32 n = nRoomCount;
			if( m_rooms.size() - nRoomCount > 2 )
			{
				SRand::Inst().Shuffle( &m_rooms[nRoomCount], m_rooms.size() - nRoomCount );
				struct SLess
				{
					bool operator () ( const SRoom& a, const SRoom& b )
					{
						int32 n1 = a.rect.GetBottom() + a.rect.width + a.rect.height;
						int32 n2 = b.rect.GetBottom() + b.rect.width + b.rect.height;
						return n1 > n2;
					}
				};
				auto pBegin = &m_rooms[nRoomCount];
				std::sort( pBegin, pBegin + ( m_rooms.size() - nRoomCount ), SLess() );
				for( int iRoom = nRoomCount; iRoom < m_rooms.size() - 2; )
				{
					if( iRoom > n )
						swap( m_rooms[iRoom], m_rooms[n] );
					n++;
					iRoom += Max<int32>( 2, n - nRoomCount );
				}
			}

			for( int iRoom = nRoomCount; iRoom < m_rooms.size(); iRoom++ )
			{
				auto& room = m_rooms[iRoom].rect;

				int8 bDoor[4] = { room.x > rect.x, room.GetRight() < rect.GetRight(), room.y > rect.y, room.GetBottom() < rect.GetBottom() };
				int8& l = bDoor[0];
				int8& r = bDoor[1];
				int8& t = bDoor[2];
				int8& b = bDoor[3];
				if( m_rooms.size() - nRoomCount <= 2 )
					l = r = t = b = 1;
				uint8 nDoors = l + r + t + b;
				if( nDoors > 1 )
				{
					uint8 nDoors1 = SRand::Inst().Rand<int8>( 1, nDoors );
					uint8 n1[4] = { 0, 1, 2, 3 };
					SRand::Inst().Shuffle( n1, 4 );
					for( int i1 = 0; i1 < 4 && nDoors > nDoors1; i1++ )
					{
						if( !bDoor[n1[i1]] )
							continue;
						bDoor[n1[i1]] = 0;
						nDoors--;
					}
				}

				bool bRoomType = iRoom < n;
				m_rooms[iRoom].bRoomType = bRoomType;
				if( bRoomType )
				{
					int32 x[] = { room.x + 2, room.x + ( room.width + SRand::Inst().Rand( 0, 2 ) ) / 2, room.GetRight() - 2 };
					int32 y[] = { room.y + 2, room.y + ( room.height + SRand::Inst().Rand( 0, 2 ) ) / 2, room.GetBottom() - 2 };
					TRectangle<int32> rect( x[l - r + 1] - 1, y[t - b + 1] - 1, 2, 2 );
					bool bSpider = true;
					for( int i = nSpiders; i < m_spiders.size(); i++ )
					{
						if( ( m_spiders[i].GetCenter() - rect.GetCenter() ).Length2() < 14 * 14 )
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
				int32 nCracks = bRoomType ? 3 + (int32)( ( room.width + room.height - 4 ) * SRand::Inst().Rand( 0.25f, 0.3f ) ) : SRand::Inst().Rand( 1, 3 );
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
					int8 nDoorType = !bRoomType ? eType_Door : eType_Web;
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
					int8 nDoorType = !bRoomType ? eType_Door : eType_Web;
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
					int8 nDoorType = !bRoomType ? eType_Door : eType_Web;
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
					int8 nDoorType = !bRoomType ? eType_Door : eType_Web;
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
			}
		}

		nCurBegin += nCurHeight;
		nLastRect = m_wallChunks1.size() - 1;
		lastRect = m_wallChunks1.back();
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

void CLevelGenNode1_3::GenWallChunk()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 nTypes[] = { 2, 0, 1 };
	for( auto& rect : m_wallChunks )
	{
		SRand::Inst().Shuffle( nTypes, ELEM_COUNT( nTypes ) );
		int32 nType = -1;
		for( int i = 0; i < ELEM_COUNT( nTypes ); i++ )
		{
			if( nTypes[i] == 0 )
			{
				if( rect.width < 8 || rect.height < 4 )
					continue;
				int32 n1 = 0, n2 = 0;
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					if( rect.y <= 0 || m_gendata[x + ( rect.y - 1 ) * nWidth] != eType_WallChunk )
						n1++;
					if( rect.GetBottom() >= nHeight || m_gendata[x + ( rect.GetBottom() ) * nWidth] != eType_WallChunk )
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
									m_gendata[x + y * nWidth] = eType_WallChunk_0_1b;
							for( int x = r1.x; x < r1.GetRight(); x++ )
								for( int y = r1.y; y < r1.GetBottom(); y++ )
									m_gendata[x + y * nWidth] = eType_WallChunk_0_1;
						}
						else
						{
							for( int x = r.x; x < r.GetRight(); x++ )
								for( int y = r.y; y < r.GetBottom(); y++ )
									m_gendata[x + y * nWidth] = eType_WallChunk_0_1;
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
								m_gendata[x + y * nWidth] = eType_WallChunk_0_1b;
						for( int x = r1.x; x < r1.GetRight(); x++ )
							for( int y = r1.y; y < r1.GetBottom(); y++ )
								m_gendata[x + y * nWidth] = eType_WallChunk_0_1;
					}
					else
					{
						for( int x = r.x; x < r.GetRight(); x++ )
							for( int y = r.y; y < r.GetBottom(); y++ )
								m_gendata[x + y * nWidth] = eType_WallChunk_0_1;
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
						if( m_gendata[x + y * nWidth] != eType_WallChunk )
							f += 1;
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
						m_gendata[x + y * nWidth] = eType_WallChunk_0_1;
				for( int x = r2.x; x < r2.GetRight(); x++ )
					for( int y = r2.y; y < r2.GetBottom(); y++ )
						m_gendata[x + y * nWidth] = eType_WallChunk_0_1a;

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
								m_gendata[x + y * nWidth] = eType_WallChunk_0_1a;
						for( int x = r2.x; x < r2.GetRight(); x++ )
							for( int y = r2.y; y < r2.GetBottom(); y++ )
								m_gendata[x + y * nWidth] = eType_WallChunk_0_1;
					}
				}
			}
			else if( nTypes[i] == 2 )
			{
				if( rect.height < 7 || rect.width < 4 || rect.width >= rect.height * 1.25f + 1 )
					continue;
				int32 n1 = 0, n2 = 0;
				for( int y = rect.y; y < rect.GetBottom(); y++ )
				{
					if( rect.x <= 0 || m_gendata[rect.x - 1 + y * nWidth] != eType_WallChunk )
						n1++;
					if( rect.GetRight() >= nWidth || m_gendata[rect.GetRight() + y * nWidth] != eType_WallChunk )
						n2++;
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
						int8 nType = eType_WallChunk_0_1a;
						if( Min( x - r.x, r.GetRight() - 1 - x ) >= SRand::Inst().Rand( 1, 4 ) && SRand::Inst().Rand( 0, 3 ) )
							nType = eType_WallChunk_0_1;
						for( int y = r.y; y < r.GetBottom(); y++ )
							m_gendata[x + y * nWidth] = nType;
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
								m_gendata[x + y * nWidth] = m_gendata[x + ( y + 1 ) * nWidth];
					}
					else
					{
						for( int x = r2.x; x < r2.GetRight(); x++ )
							for( int y = r2.GetBottom() - 1; y >= r2.y; y-- )
								m_gendata[x + y * nWidth] = m_gendata[x + ( y - 1 ) * nWidth];
					}
				}
			}

			nType = nTypes[i];
			break;
		}

		if( rect.y > 0 && rect.width >= 4 && rect.height >= 4 )
		{
			int32 nLen = 0;
			float fMaxLen = 0;
			int32 nMax = -1;
			for( int i = rect.x; i <= rect.GetRight(); i++ )
			{
				if( i < rect.GetRight() && m_gendata[i + ( rect.y - 1 ) * nWidth] == eType_WallChunk0
					&& m_gendata[i + rect.y * nWidth] == eType_WallChunk
					&& m_gendata[i + ( rect.y + 1 ) * nWidth] == eType_WallChunk )
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
			if( nLen >= 3 + SRand::Inst().Rand( 0, 3 ) && SRand::Inst().Rand( 0, 2 ) )
			{
				for( int i = 0; i < 3; i++ )
				{
					int32 x = nMax + SRand::Inst().Rand( 1, nLen - 1 );
					int32 y = rect.y + SRand::Inst().Rand( 0, rect.height - 3 );
					if( m_gendata[x + y * nWidth] == eType_WallChunk )
					{
						r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 3, 3 ), TVector2<int32>( 3, 3 ),
							rect, -1, eType_WallChunk_3 );
						if( r.width > 0 )
							break;
					}
				}
				if( r.width > 0 )
					r.SetBottom( SRand::Inst().Rand( r.GetBottom(), rect.GetBottom() ) );
			}
			else if( nLen >= 2 + SRand::Inst().Rand( 0, 2 ) )
			{
				int32 x = nMax + SRand::Inst().Rand( 0, nLen - 1 );
				int32 y = rect.y;
				r = TRectangle<int32>( x, y, 2, Min( rect.GetBottom() - 1, SRand::Inst().Rand( 2, 5 ) ) );

				for( int i = x; i < x + 2; i++ )
				{
					for( int j = y; j < y + 2; j++ )
					{
						m_gendata[i + j * nWidth] = eType_WallChunk_2;
					}
				}
			}
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
					if( m_gendata[i + j * nWidth] == eType_WallChunk )
						m_gendata[i + j * nWidth] = eType_WallChunk_1;
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
					if( m_gendata[x + y * nWidth] != eType_WallChunk )
						break;
					m_gendata[x + y * nWidth] = eType_WallChunk_1;
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
					if( m_gendata[x + y * nWidth] != eType_WallChunk )
						break;
					m_gendata[x + y * nWidth] = eType_WallChunk_1;
				}
				n--;
			}
		}
	}
}

void CLevelGenNode1_3::GenWallChunk0()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( auto rect : m_wallChunks1 )
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

void CLevelGenNode1_3::GenWindows()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 nMinFillSize = 60;
	int32 nMaxFillSize = 70;

	vector<int8> tempData;
	tempData.resize( m_gendata.size() );
	vector<TVector2<int32> > vec;
	for( auto rect : m_wallChunks1 )
	{
		rect.x -= 2;
		rect.y -= 2;
		rect.width += 4;
		rect.height += 4;
		rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				auto nType = m_gendata[i + j * nWidth];
				if( nType < eType_WallChunk1 || nType > eType_WallChunk1_2 )
				{
					if( !tempData[i + j * nWidth] )
					{
						tempData[i + j * nWidth] = 1;
						vec.push_back( TVector2<int32>( i, j ) );
					}
				}
			}
		}
	}

	SRand::Inst().Shuffle( vec );

	vector<TVector2<int32> > vecPoints;
	for( auto p : vec )
	{
		if( tempData[p.x + p.y * nWidth] != 1 )
			continue;
		if( m_gendata[p.x + p.y * nWidth] != eType_WallChunk )
			continue;

		int32 n = FloodFill( tempData, nWidth, nHeight, p.x, p.y, 2, SRand::Inst().Rand( nMinFillSize, nMaxFillSize ) );
		if( n >= nMinFillSize )
			vecPoints.push_back( p );
	}

	memset( &tempData[0], 0, tempData.size() );
	vector<TVector2<int32> > vecPoints1;
	for( int i = 0; i < m_wallChunks.size(); i++ )
	{
		auto& rect = m_wallChunks[i];
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( m_gendata[x + y * nWidth] == eType_WallChunk )
				{
					tempData[x + y * nWidth] = i + 1;
					if( !SRand::Inst().Rand( 0, 3 ) )
						vecPoints1.push_back( TVector2<int32>( x, y ) );
				}
			}
		}
	}

	for( auto p : vecPoints )
	{
		auto rect = PutRect( tempData, nWidth, nHeight, p, TVector2<int32>( 4, 4 ), TVector2<int32>( 4, 4 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
		if( rect.width )
		{
			m_windows.push_back( rect );
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_WallChunk_0;
				}
			}
		}
	}
	SRand::Inst().Shuffle( vecPoints1 );
	for( auto p : vecPoints1 )
	{
		if( !tempData[p.x + p.y * nWidth] )
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

		auto rect = PutRect( tempData, nWidth, nHeight, p, sizeMin1, sizeMax1, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
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
			m_windows1[nType].push_back( rect );

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
					m_gendata[i + j * nWidth] = eType_WallChunk_0;
				}
			}
		}
	}

	for( auto& rect : m_wallChunks )
	{
		vecPoints1.clear();
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				auto nData = m_gendata[i + j * nWidth];
				tempData[i + j * nWidth] = nData >= eType_WallChunk_0_1 && nData <= eType_WallChunk_0_1b ? 1 : 0;
				if( tempData[i + j * nWidth] && !SRand::Inst().Rand( 0, 3 ) )
					vecPoints1.push_back( TVector2<int32>( i, j ) );
			}
		}
		SRand::Inst().Shuffle( vecPoints1 );

		TVector2<int32> sizeMin = TVector2<int32>( 3, 3 );
		TVector2<int32> sizeMax = TVector2<int32>( 7, 5 );
		for( auto& p : vecPoints1 )
		{
			if( !tempData[p.x + p.y * nWidth] )
				continue;
			auto r = PutRect( tempData, nWidth, nHeight, p, sizeMin, sizeMax, rect, -1, 1 );
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
				m_windows1[3].push_back( r );
				for( int x = r.x; x < r.GetRight(); x++ )
					for( int y = r.y; y < r.GetBottom(); y++ )
						tempData[x + y * nWidth] = 0;
			}
		}
	}
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
	for( auto rect : m_wallChunks0 )
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

	vector<int8> vecData;
	vecData.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			auto nType = m_gendata[i + j * nWidth];
			vecData[i + j * nWidth] = nType == eType_Wall || nType >= eType_WallChunk1 && nType <= eType_WallChunk1_2 || nType == eType_Temp1? 0 : 1;
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
			if( rect.y == wallChunk.y && ( rect.y == 0 || !vecData[rect.x + ( rect.y - 1 ) * nWidth] ) )
				continue;
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

		if( m_pShopNode )
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
		}
	}
}
