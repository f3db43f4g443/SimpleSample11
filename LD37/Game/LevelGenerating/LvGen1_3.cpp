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
	m_pWindowNode = CreateNode( pXml->FirstChildElement( "window" )->FirstChildElement(), context );

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
	GenWindows();
	GenObjs();
	GenShops();

	for( auto& chunk : m_wallChunks )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunkNode->Generate( context, rect );
	}
	for( auto& chunk : m_wallChunks0 )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		( Min( rect.width, rect.height ) >= 4 ? m_pWallChunk0Node : m_pWallChunk1Node )->Generate( context, rect );
	}
	for( auto& chunk : m_wallChunks1 )
	{
		auto rect = chunk.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWallChunk1Node->Generate( context, rect );
	}
	for( auto& window : m_windows )
	{
		auto rect = window.Offset( TVector2<int32>( region.x, region.y ) );
		m_pWindowNode->Generate( context, rect );
	}

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
	m_windows.clear();
	m_spiders.clear();
	m_shops.clear();
}

void CLevelGenNode1_3::GenBase()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	int32 minSize[] = { 10, 10, 18, 20, 36, 36 };
	int32 maxSize[] = { 12, 12, 22, 26, 42, 42 };

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
			int32 nHeight1 = nCurHeight / 2 + SRand::Inst().Rand( 0, 2 );

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
			int32 nHeight1 = nCurHeight / 2 + SRand::Inst().Rand( 0, 2 );

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

			int32 w00 = SRand::Inst().Rand( 4, 6 );
			int32 w01 = SRand::Inst().Rand( 4, 6 );
			int32 w10 = SRand::Inst().Rand( 4, 6 );
			int32 w11 = SRand::Inst().Rand( 4, 6 );

			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin, w, h0 ) );
			m_wallChunks0.push_back( TRectangle<int32>( 0, nCurBegin, w00, h0 ) );
			m_wallChunks0.push_back( TRectangle<int32>( nWidth - w01, nCurBegin, w01, h0 ) );
			GenSubArea( TRectangle<int32>( w00, nCurBegin, left - w00, h0 ) );
			GenSubArea( TRectangle<int32>( left + w, nCurBegin, nWidth - w01 - left - w, h0 ) );

			m_rooms.push_back( TRectangle<int32>( left1, nCurBegin + h0, w1, h1 ) );
			m_wallChunks1.push_back( TRectangle<int32>( 0, nCurBegin + h0, left1, h1 ) );
			m_wallChunks1.push_back( TRectangle<int32>( left1 + w1, nCurBegin + h0, nWidth - left1 - w1, h1 ) );

			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1, w, h2 ) );
			m_wallChunks0.push_back( TRectangle<int32>( 0, nCurBegin + h0 + h1, w10, h2 ) );
			m_wallChunks0.push_back( TRectangle<int32>( nWidth - w11, nCurBegin + h0 + h1, w11, h2 ) );
			GenSubArea( TRectangle<int32>( w10, nCurBegin + h0 + h1, left - w10, h2 ) );
			GenSubArea( TRectangle<int32>( left + w, nCurBegin + h0 + h1, nWidth - w11 - left - w, h2 ) );

			left = SRand::Inst().Rand( 4, 6 );
			int32 right = nWidth - SRand::Inst().Rand( 4, 6 );
			m_wallChunks1.push_back( TRectangle<int32>( left, nCurBegin + h0 + h1 + h2, right - left, h3 ) );

			if( nLastRect >= 0 )
				FillLine( m_wallChunks1[nLastRect] );
			FillLine( m_wallChunks1.back() );

			break;
		}
		case 5:
		{
			//xxxxxxxxxxxxxxxxxxxxxx
			//..      ....        ..
			//..      ....        ..
			//****xxxxxxxxxxxxxx****
			//..      ....        ..
			//..      ....        ..
			//oooooooooooooooooooooo
			int32 w1 = SRand::Inst().Rand( 4, 6 );
			int32 w2 = SRand::Inst().Rand( 4, 6 );
			int32 h3 = SRand::Inst().Rand( 4, 6 );
			int32 h1 = SRand::Inst().Rand( 6, 7 );
			int32 n = nCurHeight - h3 - h1;
			int32 h0 = SRand::Inst().Rand( n / 2 - 1, n - n / 2 + 2 );
			int32 h2 = nCurHeight - h3 - h1 - h0;

			int32 w00 = SRand::Inst().Rand( 4, 6 );
			int32 w01 = SRand::Inst().Rand( 4, 6 );
			int32 w10 = SRand::Inst().Rand( 4, 6 );
			int32 w11 = SRand::Inst().Rand( 4, 6 );

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

	for( auto& rect : m_wallChunks0 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = eType_WallChunk0;
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
		if( m_wallChunks[i].height >= 8 )
		{
			int32 h = SRand::Inst().Rand( 4, m_wallChunks[i].height - 3 );
			TRectangle<int32> r = m_wallChunks[i];
			m_wallChunks[i].height = h;
			r.SetTop( r.y + h );
			m_wallChunks.push_back( r );
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
				if( m_gendata[i + j * nWidth] != eType_WallChunk1 )
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
	for( int i = 0; i < m_wallChunks.size(); i++ )
	{
		auto& rect = m_wallChunks[i];
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				tempData[x + y * nWidth] = i + 1;
			}
		}
	}

	for( auto p : vecPoints )
	{
		auto rect = PutRect( tempData, nWidth, nHeight, p, TVector2<int32>( 4, 4 ), TVector2<int32>( 4, 4 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
		if( rect.width )
			m_windows.push_back( rect );
	}

	for( auto& rect : m_wallChunks0 )
	{
		if( Min( rect.width, rect.height ) >= 4 )
			continue;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				m_gendata[x + y * nWidth] = eType_WallChunk1;
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
			if( nType == eType_Wall && nType1 == eType_WallChunk1 )
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
			if( type == eType_Wall || type == eType_WallChunk0 || type == eType_WallChunk1 )
				continue;

			int32 nHeight = SRand::Inst().Rand( 0, rect.height + 1 );
			for( int y = rect.y; y < rect.y + nHeight; y++ )
				m_gendata[x + y * nWidth] = eType_Temp1;
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
