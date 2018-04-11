#include "stdafx.h"
#include "LvBonusGen1.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

void CLevelBonusGenNode1_0::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1xNode = CreateNode( pXml->FirstChildElement( "block1x" )->FirstChildElement(), context );
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj1" )->FirstChildElement(), context );
	m_pBonusNode = CreateNode( pXml->FirstChildElement( "bonus" )->FirstChildElement(), context );
	m_pBonusPickUpNode = CreateNode( pXml->FirstChildElement( "bonus_pickup" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelBonusGenNode1_0::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_nSeed = SRand::Inst().nSeed;
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenChunks();
	GenObjs();

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
			{
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				m_pObjNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			}
			else if( genData == eType_Bonus )
			{
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				m_pBonusNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			}
		}
	}

	context.mapTags["mask"] = eType_Block1x;
	m_pBlock1xNode->Generate( context, region );
	context.mapTags["mask"] = eType_Block2x;
	m_pBlock2xNode->Generate( context, region );
	for( auto& rect : m_bars )
	{
		if( rect.height == 2 )
			m_pBar2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pBarNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_stones )
	{
		m_pStoneNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_wallChunks )
	{
		m_pWallChunkNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );

		int32 x = rect.x + rect.GetRight();
		x = x < region.width * 2 ? x / 2 : ( x + 1 ) / 2;
		int32 y = SRand::Inst().Rand( rect.y + ( rect.height + 1 ) / 2, rect.GetBottom() );
		m_pBonusPickUpNode->Generate( context, TRectangle<int32>( x - 1 + region.x, y - 1 + region.y, 2, 2 ) );
	}

	m_gendata.clear();
	m_stones.clear();
	m_bars.clear();
	m_wallChunks.clear();
}

void CLevelBonusGenNode1_0::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	const uint32 nStateCount = 8;
	const uint32 nMaxBeginState = 1;
	uint32 nStateMinHeight[nStateCount] = { 6, 6, 12, 12, 5, 5, 7, 7 };
	uint32 nStateMaxHeight[nStateCount] = { 7, 7, 28, 28, 6, 6, 8, 8 };
	int8 nStateTransit[nStateCount][nStateCount] =
	{
		{ -1, -1, -1, 0, 0, 0, 0, 0 },
		{ -1, -1, 0, -1, 0, 0, 0, 0 },
		{ -1, -1, -1, 0, 0, 0, 0, 0 },
		{ -1, -1, 0, -1, 0, 0, 0, 0 },
		{ -1, -1, 0, 0, -1, 0, 0, 0 },
		{ -1, -1, 0, 0, 0, -1, 0, 0 },
		{ 0, 0, -1, -1, 0, 0, -1, 0 },
		{ 0, 0, -1, -1, 0, 0, 0, -1 },
	};

	int32 h;
	vector<uint8> vecTypes;
	vector<int32> vecBegin;
	vector<int32> vecEnd;
	int8 nLastType = -1;
	for( h = 0; h < nHeight; )
	{
		uint8 nValidStates[nStateCount];
		uint8 nValidStateCount = 0;
		uint32 nMaxState = vecTypes.size() == 0 ? nMaxBeginState + 1 : nStateCount;
		for( int i = 0; i < nMaxState; i++ )
		{
			uint32 nMinLen = nStateMinHeight[i];
			if( nLastType >= 0 )
			{
				int8 nTransit = nStateTransit[nLastType][i];
				if( nTransit < 0 )
					continue;
				nMinLen += nTransit;
			}
			if( h + nMinLen > nHeight )
				continue;

			nValidStates[nValidStateCount++] = i;
		}
		if( !nValidStateCount )
			break;

		uint8 nType = nValidStates[SRand::Inst().Rand<uint8>( 0, nValidStateCount )];
		vecTypes.push_back( nType );
		vecBegin.push_back( h );
		h += SRand::Inst().Rand( nStateMinHeight[nType], Min<uint32>( nHeight - h, nStateMaxHeight[nType] ) + 1 );
		vecEnd.push_back( h );
		nLastType = nType;
	}

	int32 nFillBegin = vecBegin[0];
	int32 nFillEnd = vecEnd.back();
	for( int i = 0; i < vecTypes.size(); i++ )
	{
		uint8 nType = vecTypes[i];
		int32 nBegin = vecBegin[i];
		int32 nEnd = vecEnd[i];
		int8 nPreType = i > 0 ? vecTypes[i - 1] : -1;
		int8 nNxtType = i < vecTypes.size() - 1 ? vecTypes[i + 1] : -1;

		switch( nType )
		{
		case 0:
		case 1:
		{
			int32 x1 = SRand::Inst().Rand( 6, 10 );
			int32 x2 = SRand::Inst().Rand( 8, 12 );
			int32 x3 = SRand::Inst().Rand( 3, x2 / 2 );

			TRectangle<int32> rect( nType == 0 ? x1 : 0, nBegin, nWidth - x1, nEnd - nBegin );
			if( i < vecTypes.size() - 1 )
			{
				AddRect( eType_Bar, m_bars, TRectangle<int32>( nType == 0 ? 0 : x2, nEnd - 1, nWidth - x2, 1 ) );
				rect.height--;
			}

			TRectangle<int32> rect1 = rect;
			if( nType == 0 )
			{
				rect.width -= x3;
				rect1.SetLeft( rect.GetRight() );
			}
			else
			{
				rect1.width = x3;
				rect.SetLeft( rect1.GetRight() );
			}
			uint32 w = Min( SRand::Inst().Rand( 2, 5 ), rect.width / 6 );
			uint32 w1 = ( rect.width - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( rect.x, rect.y, w1, rect.height ) );
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( rect.x + w + w1, rect.y, rect.width - w - w1, rect.height ) );
			FillStone( rect1 );
			break;
		}
		case 2:
		case 3:
		{
			int32 h = 0;
			int32 nStep = SRand::Inst().Rand( 3, 5 );
			int32 overlap[3];
			int32 l[4];
			int32 iStep = 0;
			int32 nCurX = 0;
			vector<TRectangle<int32> > vecBars;
			bool b = false;
			int32 h1 = 0;
			while( h < nEnd - nBegin - 4 )
			{
				if( iStep == 0 )
				{
					nCurX = 0;
					int32 s = nWidth;
					for( int i = 0; i < nStep - 1; i++ )
					{
						overlap[i] = SRand::Inst().Rand( 1, 3 );
						s += overlap[i];
					}
					for( int i = 0; i < nStep; i++ )
						l[i] = ( s + i ) / nStep;
					SRand::Inst().Shuffle( l, nStep );
					if( !b )
					{
						iStep = SRand::Inst().Rand( 1, nStep );
						for( int i = 0; i < iStep; i++ )
							nCurX += l[i] - overlap[i];
					}
				}

				b = true;
				vecBars.push_back( TRectangle<int32>( nCurX, h, l[iStep], 1 ) );
				nCurX += l[iStep];
				if( iStep < nStep - 1 )
					nCurX -= overlap[iStep];

				h1 = h + 1;
				iStep++;
				if( iStep >= nStep )
				{
					h += nStep == 3 ? SRand::Inst().Rand( 2, 5 ) : SRand::Inst().Rand( 0, 2 );
					iStep = 0;
				}
				else
					h += 3;
			}

			h = h1;
			int32 h0 = ( nEnd - nBegin - h + SRand::Inst().Rand( 0, 2 ) ) / 2 + nBegin;
			for( int i = 0; i < vecBars.size(); i++ )
			{
				vecBars[i].y += h0;
				if( nType == 3 )
					vecBars[i].x = nWidth - vecBars[i].x - vecBars[i].width;
				AddRect( eType_Bar, m_bars, vecBars[i] );
			}

			for( int k = 0; k < 2; k++ )
			{
				for( int i = 0; i < vecBars.size(); i++ )
				{
					auto& r = vecBars[i];
					int32 x = ( r.x + r.GetRight() - SRand::Inst().Rand( 0, 2 ) ) / 2;
					TRectangle<int32> rect;
					if( k )
					{
						int32 y = r.y + 1;
						while( y < nEnd - 1 )
						{
							if( m_gendata[x + ( y + 1 ) * nWidth] )
								break;
							y++;
						}
						rect = TRectangle<int32>( x, r.y + 1, 1, y - r.y );
					}
					else
					{
						int32 y = r.y;
						while( y > nBegin )
						{
							if( m_gendata[x + ( y - 1 ) * nWidth] )
								break;
							y--;
						}
						rect = TRectangle<int32>( x, y, 1, vecBars[i].y - y );
					}

					if( rect.height >= 4 )
					{
						x = rect.x;
						while( x > r.x )
						{
							bool b = true;
							for( int j = rect.y; j < rect.GetBottom(); j++ )
							{
								if( m_gendata[( x - 1 ) + j * nWidth] )
								{
									b = false;
									break;
								}
							}
							if( !b )
								break;
							x--;
						}
						rect.SetLeft( x );

						x = rect.GetRight() - 1;
						while( x < r.GetRight() - 1 )
						{
							bool b = true;
							for( int j = rect.y; j < rect.GetBottom(); j++ )
							{
								if( m_gendata[( x + 1 ) + j * nWidth] )
								{
									b = false;
									break;
								}
							}
							if( !b )
								break;
							x++;
						}
						rect.SetRight( x + 1 );
						AddRect( eType_WallChunk, m_wallChunks, rect );
					}
				}
			}
			
			break;
		}
		case 4:
		{
			int32 w = SRand::Inst().Rand( 4, 8 );
			int32 x1 = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 x2 = x1 + w;
			int32 w1 = SRand::Inst().Rand( 3, 6 );
			int32 w2[3] = { ( nWidth - w1 * 2 ) / 3, ( nWidth - w1 * 2 + 1 ) / 3, ( nWidth - w1 * 2 + 2 ) / 3 };
			SRand::Inst().Shuffle( w2, 3 );
			TRectangle<int32> r( 0, nBegin, 0, nEnd - nBegin - 1 );
			for( int i = 0; i < 3; i++ )
			{
				r.width = w2[i];
				AddRect( eType_WallChunk, m_wallChunks, r );
				r.x = r.GetRight() + w1;
			}
			if( i < vecTypes.size() - 1 )
			{
				AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nEnd - 1, x1, 1 ) );
				AddRect( eType_Bar, m_bars, TRectangle<int32>( x2, nEnd - 1, nWidth - x2, 1 ) );
			}
			break;
		}
		case 5:
		{
			int32 w = SRand::Inst().Rand( 3, 6 );
			int32 x1 = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 x2 = x1 + w;
			int32 x3 = SRand::Inst().Rand( 4, 6 );
			int32 x4 = nWidth - SRand::Inst().Rand( 4, 6 );
			int32 w1 = SRand::Inst().Rand( 3, 6 );
			int32 w2[3] = { ( nWidth - w1 * 2 ) / 3, ( nWidth - w1 * 2 + 1 ) / 3, ( nWidth - w1 * 2 + 2 ) / 3 };
			SRand::Inst().Shuffle( w2, 3 );
			if( i < vecTypes.size() - 1 )
			{
				TRectangle<int32> r( 0, nEnd - 1, 0, 1 );
				for( int i = 0; i < 3; i++ )
				{
					r.width = w2[i];
					AddRect( eType_Bar, m_bars, r );
					r.x = r.GetRight() + w1;
				}
			}
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( x3, nBegin, x1 - x3, nEnd - nBegin - 1 ) );
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( x2, nBegin, x4 - x2, nEnd - nBegin - 1 ) );
			break;
		}
		case 6:
		{
			int32 w = SRand::Inst().Rand( 6, 10 );
			int32 x1 = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 x2 = x1 + w;
			int32 x3 = SRand::Inst().Rand( 4, 6 );
			int32 x4 = nWidth - SRand::Inst().Rand( 4, 6 );
			int32 w2 = w - SRand::Inst().Rand( 2, 3 );
			int32 x5 = ( x1 + x2 - w2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 x6 = x5 + w2;

			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( x3, nBegin, x5 - x3, nEnd - nBegin - 1 ) );
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( x6, nBegin, x4 - x6, nEnd - nBegin - 1 ) );
			FillStone( TRectangle<int32>( 0, nBegin, x3, nEnd - nBegin - 1 ) );
			FillStone( TRectangle<int32>( x4, nBegin, nWidth - x4, nEnd - nBegin - 1 ) );
			if( i < vecTypes.size() - 1 )
			{
				AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nEnd - 1, x1, 1 ) );
				AddRect( eType_Bar, m_bars, TRectangle<int32>( x2, nEnd - 1, nWidth - x2, 1 ) );
			}
			FillStone( TRectangle<int32>( x5, nEnd - 3, x6 - x5, 3 ) );

			break;
		}
		case 7:
		{
			int32 w = SRand::Inst().Rand( 4, 7 );
			int32 w1[3] = { ( nWidth - w * 2 ) / 3, ( nWidth - w * 2 + 1 ) / 3, ( nWidth - w * 2 + 2 ) / 3 };
			SRand::Inst().Shuffle( w1, 3 );
			if( i < vecTypes.size() - 1 )
			{
				TRectangle<int32> r( 0, nEnd - 1, 0, 1 );
				for( int i = 0; i < 3; i++ )
				{
					r.width = w1[i];
					AddRect( eType_Bar, m_bars, r );
					r.x = r.GetRight() + w;
				}
			}
			FillStone( TRectangle<int32>( w1[0] + 1, nEnd - 3, w - 2, 3 ) );
			FillStone( TRectangle<int32>( w1[0] + w + w1[1] + 1, nEnd - 3, w - 2, 3 ) );
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( 0, nBegin, w1[0] + 1, nEnd - nBegin - 1 ) );
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( w1[0] + w - 1, nBegin, w1[1] + 2, nEnd - nBegin - 1 ) );
			AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( nWidth - w1[2] - 1, nBegin, w1[2] + 1, nEnd - nBegin - 1 ) );

			break;
		}
		}
	}
}

void CLevelBonusGenNode1_0::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;

	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			if( m_gendata[i + j * nWidth] )
				continue;
			if( j > 0 && m_gendata[i + ( j - 1 ) * nWidth] )
				continue;
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );

	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 1, 1 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp );
		if( rect.width <= 0 )
			continue;
		if( rect.width * rect.height < 4 )
			continue;
		int32 nMin = Min( 2, rect.height );
		int32 nMax = Max( Min( rect.height, 4 + ( rect.height - 4 ) / 2 ), nMin );
		int8 nType1 = eType_Block1x + SRand::Inst().Rand( 0, 2 );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			if( rect.y > 0 && !m_gendata[i + ( rect.y - 1 ) * nWidth] )
			{
				m_gendata[i + ( rect.y - 1 ) * nWidth] = nType1;
			}
			int32 h = rect.y + SRand::Inst().Rand( nMin, nMax + 1 );
			int32 h1 = rect.y - 1;
			if( ( rect.height == 1 || rect.height >= 3 ) && SRand::Inst().Rand( 0, 2 ) )
				h1 = SRand::Inst().Rand( rect.y, h );
			for( int j = rect.y; j < h; j++ )
			{
				m_gendata[i + j * nWidth] = j == h1 ? eType_Obj : eType_Bonus;
			}
		}
	}

	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
				m_gendata[i + j * nWidth] = eType_None;
		}
	}
}

void CLevelBonusGenNode1_0::AddRect( uint8 nType, vector<TRectangle<int32>>& vec, const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vec.push_back( rect );
	for( int j = rect.y; j < rect.GetBottom(); j++ )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			m_gendata[i + j * nWidth] = nType;
		}
	}
}

void CLevelBonusGenNode1_0::FillStone( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ),
			rect, -1, eType_Stone );
		if( r.width > 0 )
			m_stones.push_back( r );
		else
		{
			m_gendata[p.x + p.y * nWidth] = SRand::Inst().Rand( 0, 5 ) ? eType_Bonus : eType_Obj;
		}
	}
}

void CLevelBonusGenNode1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1xNode = CreateNode( pXml->FirstChildElement( "block1x" )->FirstChildElement(), context );
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pRoom1Node = CreateNode( pXml->FirstChildElement( "room1" )->FirstChildElement(), context );
	m_pRoom2Node = CreateNode( pXml->FirstChildElement( "room2" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );
	m_pObjNode[0] = CreateNode( pXml->FirstChildElement( "obj1" )->FirstChildElement(), context );
	m_pObjNode[1] = CreateNode( pXml->FirstChildElement( "obj2" )->FirstChildElement(), context );
	m_pObjNode[2] = CreateNode( pXml->FirstChildElement( "obj3" )->FirstChildElement(), context );
	m_pBonusNode = CreateNode( pXml->FirstChildElement( "bonus" )->FirstChildElement(), context );
	m_pBonusPickUpNode = CreateNode( pXml->FirstChildElement( "bonus_pickup" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelBonusGenNode1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_nSeed = SRand::Inst().nSeed;
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenRooms();
	FixBlocks();

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
			else if( genData >= eType_Obj && genData < eType_Obj_Max )
			{
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				m_pObjNode[genData - eType_Obj]->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			}
			else if( genData == eType_Bonus )
			{
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				m_pBonusNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			}
		}
	}

	context.mapTags["mask"] = eType_Block1x;
	m_pBlock1xNode->Generate( context, region );
	context.mapTags["mask"] = eType_Block2x;
	m_pBlock2xNode->Generate( context, region );
	for( auto& rect : m_bars )
	{
		if( rect.height == 2 )
			m_pBar2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pBarNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_stones )
	{
		m_pStoneNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_wallChunks )
	{
		m_pWallChunkNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["door"] = eType_Door;
	for( auto& room : m_rooms )
	{
		if( room.nType == 0 )
			m_pRoom1Node->Generate( context, room.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pRoom2Node->Generate( context, room.rect.Offset( TVector2<int32>( region.x, region.y ) ) );

		int32 x = room.rect.x + room.rect.GetRight();
		x = x < region.width * 2 ? x / 2 : ( x + 1 ) / 2;
		int32 y = room.rect.y + room.rect.height / 2;
		m_pBonusPickUpNode->Generate( context, TRectangle<int32>( x - 1 + region.x, y - 1 + region.y, 2, 2 ) );
	}

	m_gendata.clear();
	m_stones.clear();
	m_bars.clear();
	m_rooms.clear();
	m_wallChunks.clear();
}

void CLevelBonusGenNode1_1::GenRooms()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const uint32 nStateCount = 6;
	const uint32 nMaxBeginState = 1;
	uint32 nStateMinHeight[nStateCount] = { 7, 7, 6, 7, 8, 8 };
	uint32 nStateMaxHeight[nStateCount] = { 8, 6, 7, 8, 12, 12 };
	int8 nStateTransit[nStateCount][nStateCount] =
	{
		{ -1, -1, 0, 0, -1, -1 },
		{ 0, -1, 2, 0, -1, -1 },
		{ 1, 1, -1, 1, 1, 1 },
		{ 0, 0, 1, -1, 0, 0 },
		{ 0, 1, -1, 0, -1, 0 },
		{ 0, 1, -1, 0, 0, -1 },
	};
	int8 nStateTransitMax[nStateCount][nStateCount] =
	{
		{ -1, -1, 3, 0, -1, -1 },
		{ 0, -1, 3, 0, -1, -1 },
		{ 3, 3, -1, 3, 3, 3 },
		{ 0, 2, 3, -1, 2, 2 },
		{ 0, 2, -1, 2, -1, 0 },
		{ 0, 2, -1, 2, 0, -1 },
	};

	int32 h;
	vector<uint8> vecTypes;
	vector<int32> vecBegin;
	vector<int32> vecEnd;
	int8 nLastType = -1;
	for( h = 0; h < nHeight; )
	{
		uint8 nValidStates[nStateCount];
		uint8 nValidStateCount = 0;
		uint32 nMaxState = vecTypes.size() == 0 ? nMaxBeginState + 1 : nStateCount;
		for( int i = 0; i < nMaxState; i++ )
		{
			uint32 nMinLen = nStateMinHeight[i];
			if( nLastType >= 0 )
			{
				int8 nTransit = nStateTransit[nLastType][i];
				if( nTransit < 0 )
					continue;
				nMinLen += nTransit;
			}
			if( h + nMinLen > nHeight )
				continue;

			nValidStates[nValidStateCount++] = i;
		}
		if( !nValidStateCount )
			break;

		uint8 nType = nValidStates[SRand::Inst().Rand<uint8>( 0, nValidStateCount )];
		vecTypes.push_back( nType );
		if( nLastType >= 0 )
		{
			h += Min<int32>( nHeight - nStateMinHeight[nType] - h, SRand::Inst().Rand<int32>( nStateTransit[nLastType][nType], nStateTransitMax[nLastType][nType] + 1 ) );
		}
		vecBegin.push_back( h );
		h += SRand::Inst().Rand( nStateMinHeight[nType], Min<uint32>( nHeight - h, nStateMaxHeight[nType] ) + 1 );
		vecEnd.push_back( h );
		nLastType = nType;
	}

	h = ( nHeight - h + SRand::Inst().Rand( 0, 2 ) ) / 2;
	for( int i = 0; i < vecTypes.size(); i++ )
	{
		vecBegin[i] += h;
		vecEnd[i] += h;
	}
	int32 nFillBegin = vecBegin[0];
	int32 nFillEnd = vecEnd.back();
	for( int i = 0; i < vecTypes.size(); i++ )
	{
		uint8 nType = vecTypes[i];
		int32 nBegin = vecBegin[i];
		int32 nEnd = vecEnd[i];
		int8 nPreType = i > 0 ? vecTypes[i - 1] : -1;
		int32 nPreEnd = i > 0 ? vecEnd[i - 1] : 0;
		int8 nNxtType = i < vecTypes.size() - 1 ? vecTypes[i + 1] : -1;
		int32 nNxtBegin = i < vecTypes.size() - 1 ? vecBegin[i + 1] : nHeight;

		switch( nType )
		{
		case 0:
		{
			int32 r = nWidth / 2 - SRand::Inst().Rand( 2, 4 );
			int32 w = SRand::Inst().Rand( 8, 11 );
			TRectangle<int32> r1( r - w, nBegin, w, nEnd - nBegin );
			TRectangle<int32> r2( nWidth - r, nBegin, w, nEnd - nBegin );
			AddRoom( 1, r1 );
			AddRoom( 1, r2 );

			uint32 nDoor;
			if( nPreType == 1 || nPreType == 3 )
			{
				nDoor = r - 2;
				while( nDoor > r - w + 2 )
				{
					uint8 genData = m_gendata[nDoor + ( nBegin - 1 ) * nWidth];
					if( genData <= eType_Temp )
						break;
					nDoor--;
				}
				m_gendata[nDoor + nBegin * nWidth] = m_gendata[nDoor - 1 + nBegin * nWidth]
					= m_gendata[nWidth - nDoor - 1 + nBegin * nWidth] = m_gendata[nWidth - nDoor + nBegin * nWidth] = eType_Door;
			}
			else if( nPreType == 2 )
			{
				int32 nDoorMin = Max( 7, r1.x + 1 );
				int32 nDoorMax = Min( 9, r1.GetRight() - 1 );
				for( int x = nDoorMin; x < nDoorMax; x++ )
					m_gendata[x + nBegin * nWidth] = m_gendata[nWidth - x - 1 + nBegin * nWidth] = eType_Door;
			}

			nDoor = ( nEnd + nBegin - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			m_gendata[r1.x + nDoor * nWidth] = m_gendata[r1.x + ( nDoor + 1 ) * nWidth]
				= m_gendata[r2.GetRight() - 1 + nDoor * nWidth] = m_gendata[r2.GetRight() - 1 + ( nDoor + 1 ) * nWidth] = eType_Door;
			if( nPreType >= 4 )
			{
				m_gendata[r2.x + nDoor * nWidth] = m_gendata[r2.x + ( nDoor + 1 ) * nWidth]
					= m_gendata[r1.GetRight() - 1 + nDoor * nWidth] = m_gendata[r1.GetRight() - 1 + ( nDoor + 1 ) * nWidth] = eType_Door;
			}

			if( nNxtType == 2 )
			{
				int32 nDoorMin = Max( 7, r1.x + 1 );
				int32 nDoorMax = Min( 9, r1.GetRight() - 1 );
				for( int x = nDoorMin; x < nDoorMax; x++ )
					m_gendata[x + ( nEnd - 1 ) * nWidth] = m_gendata[nWidth - x - 1 + ( nEnd - 1 ) * nWidth] = eType_Door;
			}
			else if( nNxtType == 3 )
			{
				int32 nDoorMin = Max( 5, r1.x + 1 );
				int32 nDoorMax = Min( 7, r1.GetRight() - 1 );
				int32 nDoorMin2 = Max( 10, r1.x + 1 );
				int32 nDoorMax2 = Min( 12, r1.GetRight() - 1 );
				if( nDoorMax > nDoorMin && nDoorMax2 > nDoorMin2 )
				{
					if( SRand::Inst().Rand( 0, 2 ) )
					{
						for( int x = nDoorMin; x < nDoorMax; x++ )
							m_gendata[x + ( nEnd - 1 ) * nWidth] = m_gendata[nWidth - x - 1 + ( nEnd - 1 ) * nWidth] = eType_Door;
					}
					else
					{
						for( int x = nDoorMin2; x < nDoorMax2; x++ )
							m_gendata[x + ( nEnd - 1 ) * nWidth] = m_gendata[nWidth - x - 1 + ( nEnd - 1 ) * nWidth] = eType_Door;
					}
				}
				else
				{
					for( int x = nDoorMin; x < nDoorMax; x++ )
						m_gendata[x + ( nEnd - 1 ) * nWidth] = m_gendata[nWidth - x - 1 + ( nEnd - 1 ) * nWidth] = eType_Door;
					for( int x = nDoorMin2; x < nDoorMax2; x++ )
						m_gendata[x + ( nEnd - 1 ) * nWidth] = m_gendata[nWidth - x - 1 + ( nEnd - 1 ) * nWidth] = eType_Door;
				}
			}
			FillRoomGap( TRectangle<int32>( r1.GetRight(), nBegin, r2.x - r1.GetRight(), nEnd - nBegin ), false, false );
			if( nPreType != 5 && nPreType != 4 )
			{
				FillRoomGap( TRectangle<int32>( 0, nBegin, r1.x, nEnd - nBegin ), true, false );
				CopyData( TRectangle<int32>( r2.GetRight(), nBegin, nWidth - r2.GetRight(), nEnd - nBegin ), TVector2<int32>( 0, nBegin ), true );
			}
			else if( nPreType != 5 )
			{
				FillRoomGap( TRectangle<int32>( 0, nBegin, r1.x, nEnd - nBegin ), true, false );
				FillRoomGap( TRectangle<int32>( r2.GetRight(), nBegin, nWidth - r2.GetRight(), nEnd - nBegin ), false, true );
			}
			else
			{
				FillRoomGap( TRectangle<int32>( 0, nBegin, r1.x, nEnd - nBegin ), false, true );
				FillRoomGap( TRectangle<int32>( r2.GetRight(), nBegin, nWidth - r2.GetRight(), nEnd - nBegin ), true, false );
			}
			break;
		}
		case 1:
		{
			int32 l = nWidth / 2 - SRand::Inst().Rand( 4, 6 );
			int32 r = nWidth - l;
			int32 w1 = SRand::Inst().Rand( 7, 10 );
			AddRoom( 1, TRectangle<int32>( l, nBegin, r - l, nEnd - nBegin ) );

			m_gendata[nWidth / 2 - 1 + nBegin * nWidth] = m_gendata[nWidth / 2 + nBegin * nWidth] = eType_Door;
			m_gendata[nWidth / 2 - 1 + ( nEnd - 1 ) * nWidth] = m_gendata[nWidth / 2 + ( nEnd - 1 ) * nWidth] = eType_Door;
			int32 nDoor = ( nEnd + nBegin - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			m_gendata[l + nDoor * nWidth] = m_gendata[r - 1 + nDoor * nWidth]
				= m_gendata[l + ( nDoor + 1 ) * nWidth] = m_gendata[r - 1 + ( nDoor + 1 ) * nWidth] = eType_Door;

			int32 l1 = l - SRand::Inst().Rand( w1 / 2, w1 );
			int32 r1 = nWidth - l1;
			if( nPreType != 5 )
				AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( l - w1, nBegin + 1, w1, nEnd - nBegin - 1 ) );
			else
			{
				AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( l - w1, nBegin, w1, nEnd - nBegin - 1 ) );
				AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nEnd - 1, l, 1 ) );
			}
			if( nPreType != 4 )
				AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( r, nBegin + 1, w1, nEnd - nBegin - 1 ) );
			else
			{
				AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( l - w1, nBegin, w1, nEnd - nBegin - 1 ) );
				AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nEnd - 1, l, 1 ) );
			}

			nPreEnd = Max( nBegin - 3, nPreEnd );
			if( i == 0 )
				nFillBegin = nPreEnd;
			int32 nFillWidth;
			if( nPreEnd == nBegin )
			{
				if( nPreType != 5 )
					AddRect( eType_Bar, m_bars, TRectangle<int32>( l1, nBegin, l - l1, 1 ) );
				if( nPreType != 4 )
					AddRect( eType_Bar, m_bars, TRectangle<int32>( r, nBegin, r1 - r, 1 ) );
				nFillWidth = l1;
			}
			else if( nPreEnd >= nBegin - 2 )
			{
				int32 l2 = SRand::Inst().Rand( l + 1, nWidth / 2 - 1 );
				int32 r2 = nWidth - l2;
				AddRect( eType_Bar, m_bars, TRectangle<int32>( l1, nPreEnd, l2 - l1, nBegin - nPreEnd ) );
				AddRect( eType_Bar, m_bars, TRectangle<int32>( r2, nPreEnd, r1 - r2, nBegin - nPreEnd ) );
				nFillWidth = l;
				if( nPreType == 5 || nPreType == 4 )
					FillRoomGap( TRectangle<int32>( l2, nPreEnd, r2 - l2, nBegin - nPreEnd ), false, false );
			}
			else
			{
				int32 l2 = SRand::Inst().Rand( l + 1, nWidth / 2 - 1 );
				int32 r2 = nWidth - l2;
				int32 w3 = SRand::Inst().Rand( 3, 5 );
				AddRect( eType_Stone, m_stones, TRectangle<int32>( l2 - w3, nPreEnd + 1, w3, 2 ) );
				AddRect( eType_Stone, m_stones, TRectangle<int32>( r2, nPreEnd + 1, w3, 2 ) );
				l2 -= SRand::Inst().Rand( 1, w3 - 1 );
				r2 = nWidth - l2;
				AddRect( eType_Bar, m_bars, TRectangle<int32>( l1, nPreEnd, l2 - l1, 1 ) );
				AddRect( eType_Bar, m_bars, TRectangle<int32>( r2, nPreEnd, r1 - r2, 1 ) );
				nFillWidth = l;
				if( nPreType == 5 || nPreType == 4 )
					FillRoomGap( TRectangle<int32>( l2, nPreEnd, r2 - l2, nBegin - nPreEnd ), false, false );
			}

			if( nPreType != 5 && nPreType != 4 )
			{
				FillRoomGap( TRectangle<int32>( 0, nPreEnd, nFillWidth, nEnd - nPreEnd ), false, false );
				CopyData( TRectangle<int32>( nWidth - nFillWidth, nPreEnd, nFillWidth, nEnd - nPreEnd ), TVector2<int32>( 0, nPreEnd ), true );
			}
			else if( nPreType != 5 )
			{
				FillRoomGap( TRectangle<int32>( 0, nPreEnd, nFillWidth, nEnd - nPreEnd ), false, false );
				FillRoomGap( TRectangle<int32>( nWidth - nFillWidth, nPreEnd, nFillWidth, nEnd - nPreEnd ), false, true );
			}
			else
			{
				FillRoomGap( TRectangle<int32>( nWidth - nFillWidth, nPreEnd, nFillWidth, nEnd - nPreEnd ), false, false );
				FillRoomGap( TRectangle<int32>( 0, nPreEnd, nFillWidth, nEnd - nPreEnd ), false, true );
			}
			break;
		}
		case 2:
		{
			TRectangle<int32> rect( 1, nBegin, 6, nEnd - nBegin );
			int32 nDoor = nBegin + 1;
			nPreEnd = Max( nBegin - 3, nPreEnd );
			if( i == 0 )
				nFillBegin = nPreEnd;
			nNxtBegin = Min( nEnd + 3, nNxtBegin );
			if( i == vecTypes.size() - 1 )
				nFillEnd = nNxtBegin;
			int8 nFillType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
			int8 nStoneHalfWidth = Max( nBegin - nPreEnd - 1, SRand::Inst().Rand( 1, 3 ) );
			int8 nStoneHalfWidth1 = Max( nNxtBegin - nEnd - 1, SRand::Inst().Rand( 1, 3 ) );
			for( int i = 0; i < 4; i++ )
			{
				auto r = rect;
				rect.x += 8;
				if( i == 0 && ( nPreType == 5 || nNxtType == 4 ) )
				{
					r.SetLeft( 0 );
					if( nNxtType == 4 )
					{
						r.SetBottom( nNxtBegin );
						m_gendata[r.x + 1 + ( r.GetBottom() - 1 ) * nWidth] = m_gendata[r.x + 2 + ( r.GetBottom() - 1 ) * nWidth] = eType_Door;
					}
				}
				else
					m_gendata[r.x + nDoor * nWidth] = m_gendata[r.x + ( nDoor + 1 ) * nWidth] = eType_Door;
				if( i == 3 && ( nPreType == 4 || nNxtType == 5 ) )
				{
					r.SetRight( nWidth );
					if( nNxtType == 5 )
					{
						r.SetBottom( nNxtBegin );
						m_gendata[r.GetRight() - 2 + ( r.GetBottom() - 1 ) * nWidth] = m_gendata[r.GetRight() - 3 + ( r.GetBottom() - 1 ) * nWidth] = eType_Door;
					}
				}
				else
					m_gendata[r.x + nDoor * nWidth] = m_gendata[r.x + ( nDoor + 1 ) * nWidth] = eType_Door;
				AddRoom( 0, r );
				m_gendata[r.x + nDoor * nWidth] = m_gendata[r.GetRight() - 1 + nDoor * nWidth]
					= m_gendata[r.x + ( nDoor + 1 ) * nWidth] = m_gendata[r.GetRight() - 1 + ( nDoor + 1 ) * nWidth] = eType_Door;
				for( int j = nPreEnd; j < nBegin; j++ )
				{
					for( int i = r.x; i < r.GetRight(); i++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = nFillType;
					}
				}
				if( nBegin - nPreEnd >= 2 )
					AddRect( eType_Stone, m_stones, TRectangle<int32>( r.x + 3 - nStoneHalfWidth, nPreEnd, nStoneHalfWidth * 2, nBegin - nPreEnd ) );
				else if( nBegin - nPreEnd == 1 )
				{
					for( int i = r.x; i < r.GetRight(); i++ )
					{
						if( !m_gendata[i + nPreEnd * nWidth] )
							m_gendata[i + nPreEnd * nWidth] = nFillType;
					}
				}
				for( int j = r.GetBottom(); j < nNxtBegin; j++ )
				{
					for( int i = r.x; i < r.GetRight(); i++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = nFillType;
					}
				}
				if( nNxtType != 1 )
				{
					if( nNxtBegin - r.GetBottom() >= 2 )
						AddRect( eType_Stone, m_stones, TRectangle<int32>( r.x + 3 - nStoneHalfWidth1, r.GetBottom(), nStoneHalfWidth1 * 2, nNxtBegin - r.GetBottom() ) );
					else if( nNxtBegin - r.GetBottom() == 1 )
					{
						for( int i = r.x; i < r.GetRight(); i++ )
						{
							if( !m_gendata[i + r.GetBottom() * nWidth] )
								m_gendata[i + r.GetBottom() * nWidth] = nFillType;
						}
					}
				}
			}

			for( int j = nPreEnd; j < nBegin; j++ )
			{
				for( int i = 0; i < nWidth; i++ )
				{
					if( !m_gendata[i + j * nWidth] )
						m_gendata[i + j * nWidth] = nFillType;
				}
			}
			if( nBegin - nPreEnd >= 2 )
			{
				TRectangle<int32> rect( 12 + nStoneHalfWidth, nPreEnd, 8 - nStoneHalfWidth * 2, nBegin - nPreEnd );
				FillRoomGap( rect, false, false, false );
				rect.SetBottom( rect.GetBottom() + SRand::Inst().Rand( 0, 3 ) );
				for( int j = rect.y + 1; j < rect.GetBottom(); j++ )
				{
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = eType_Bonus;
					}
				}
				CopyData( rect.Offset( TVector2<int32>( -8, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				CopyData( rect.Offset( TVector2<int32>( 8, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				rect.width /= 2;
				CopyData( rect.Offset( TVector2<int32>( 16, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				rect.x += rect.width;
				CopyData( rect.Offset( TVector2<int32>( -16, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
			}
			if( nNxtBegin - nEnd >= 2 && nNxtType != 1 )
			{
				TRectangle<int32> rect( 12 + nStoneHalfWidth, nEnd, 8 - nStoneHalfWidth * 2, nNxtBegin - nEnd );
				FillRoomGap( rect, false, false, false );
				for( int j = rect.y + 1; j < rect.GetBottom(); j++ )
				{
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = eType_Bonus;
					}
				}
				CopyData( rect.Offset( TVector2<int32>( -8, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				CopyData( rect.Offset( TVector2<int32>( 8, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				rect.width /= 2;
				CopyData( rect.Offset( TVector2<int32>( 16, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				rect.x += rect.width;
				CopyData( rect.Offset( TVector2<int32>( -16, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
			}
			break;
		}
		case 3:
		{
			TRectangle<int32> rect( 2, nBegin, 8, nEnd - nBegin );
			nPreEnd = Max( nBegin - 2, nPreEnd );
			if( i == 0 )
				nFillBegin = nPreEnd;
			nNxtBegin = Min( nEnd + 2, nNxtBegin );
			if( i == vecTypes.size() - 1 )
				nFillEnd = nNxtBegin;
			int32 nDoor = ( nEnd + nBegin - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			for( int i = 0; i < 3; i++ )
			{
				AddRoom( 1, rect );
				if( nPreType < 4 )
					m_gendata[rect.x + 3 + nBegin * nWidth] = m_gendata[rect.x + 4 + nBegin * nWidth] = eType_Door;
				m_gendata[rect.x + nDoor * nWidth] = m_gendata[rect.GetRight() - 1 + nDoor * nWidth]
					= m_gendata[rect.x + ( nDoor + 1 ) * nWidth] = m_gendata[rect.GetRight() - 1 + ( nDoor + 1 ) * nWidth] = eType_Door;
				if( nNxtType < 4 )
					m_gendata[rect.x + 3 + ( nEnd - 1 ) * nWidth] = m_gendata[rect.x + 4 + ( nEnd - 1 ) * nWidth] = eType_Door;
				rect.x += 10;
			}

			if( nPreType != 3 )
			{
				TRectangle<int32> rect( 11, nPreEnd, 10, nBegin - nPreEnd );
				FillRoomGap( rect, false, false, false );
				rect.SetBottom( rect.GetBottom() + SRand::Inst().Rand( 0, 3 ) );
				for( int j = rect.y + 1; j < rect.GetBottom(); j++ )
				{
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						if( !m_gendata[i + j * nWidth] )
							m_gendata[i + j * nWidth] = eType_Bonus;
					}
				}
				CopyData( rect.Offset( TVector2<int32>( -10, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				CopyData( rect.Offset( TVector2<int32>( 10, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
			}
			if( nNxtType != 1 && nNxtType != 3 )
			{
				TRectangle<int32> rect( 11, nEnd, 10, nNxtBegin - nEnd );
				if( rect.height > 0 )
				{
					FillRoomGap( rect, false, false, false );
					rect.SetTop( rect.GetTop() - SRand::Inst().Rand( 0, 3 ) );
					int8 nFillType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						for( int j = rect.y; j < rect.GetBottom() - 1; j++ )
						{
							if( !m_gendata[i + j * nWidth] )
								m_gendata[i + j * nWidth] = nFillType;
						}
						if( !m_gendata[i + ( rect.GetBottom() - 1 ) * nWidth] )
							m_gendata[i + ( rect.GetBottom() - 1 ) * nWidth] = eType_Bonus;
					}
					CopyData( rect.Offset( TVector2<int32>( -10, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
					CopyData( rect.Offset( TVector2<int32>( 10, 0 ) ), TVector2<int32>( rect.x, rect.y ), false );
				}
			}
			break;
		}
		case 4:
		case 5:
		{
			uint32 w1 = SRand::Inst().Rand( 6, 10 );
			uint32 w2 = SRand::Inst().Rand( 3, 5 );
			uint32 h1 = nEnd - nBegin >= 10 ? 2 : 1;
			TRectangle<int32> rect[3];
			if( nType == 4 )
			{
				rect[0] = TRectangle<int32>( nWidth - w1 - ( w2 + 6 ) * 2, nBegin, 6, nEnd - nBegin - h1 * 2 );
				rect[1] = TRectangle<int32>( nWidth - w1 - w2 - 6, nBegin + h1, 6, nEnd - nBegin - h1 * 2 );
				rect[2] = TRectangle<int32>( nWidth - w1, nBegin + h1, w1, nEnd - nBegin - h1 );
				AddRoom( 0, rect[0] );
				AddRoom( 0, rect[1] );
				AddRoom( 1, rect[2] );
				for( int i = 0; i < 3; i++ )
				{
					m_gendata[rect[i].x + ( rect[i].y + 1 ) * nWidth] = m_gendata[rect[i].x + ( rect[i].y + 2 ) * nWidth] = eType_Door;
					if( i < 2 )
					{
						if( h1 == 1 )
							m_gendata[rect[i].GetRight() - 1 + ( rect[i].y + 1 ) * nWidth] = m_gendata[rect[i].GetRight() - 1 + ( rect[i].y + 2 ) * nWidth] = eType_Door;
						else
							m_gendata[rect[i].GetRight() - 1 + ( rect[i].GetBottom() - 2 ) * nWidth] = m_gendata[rect[i].GetRight() - 1 + ( rect[i].GetBottom() - 3 ) * nWidth] = eType_Door;
					}
					else
						m_gendata[rect[i].GetRight() - 2 + ( rect[i].GetBottom() - 1 ) * nWidth] = m_gendata[rect[i].GetRight() - 3 + ( rect[i].GetBottom() - 1 ) * nWidth] = eType_Door;
				}
				if( ( w2 + 6 ) * 2 + w1 <= nWidth - 5 )
					AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( 0, nBegin, nWidth - w1 - ( w2 + 6 ) * 2, nEnd - nBegin - h1 * 2 ) );
				else
					FillRoomGap( TRectangle<int32>( 0, nBegin, nWidth - w1 - ( w2 + 6 ) * 2, nEnd - nBegin - h1 * 2 ), false, true );

				if( i < vecTypes.size() - 1 )
				{
					if( h1 == 2 )
					{
						AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nEnd - 4, rect[0].GetRight(), 1 ) );
						int32 l = SRand::Inst().Rand( 1, 3 );
						int32 r = SRand::Inst().Rand( 1, 3 );
						AddRect( eType_Stone, m_stones, TRectangle<int32>( rect[0].GetRight() - l, nEnd - 3, l + r, 3 ) );
						AddRect( eType_Bar, m_bars, TRectangle<int32>( rect[0].GetRight() + r, nEnd - 2, rect[2].x - rect[0].GetRight() - r, 2 ) );
						int8 nFillType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
						for( int j = nEnd - 3; j < nEnd; j++ )
						{
							for( int i = 0; i < rect[0].GetRight() - l; i++ )
								m_gendata[i + j * nWidth] = nFillType;
						}
					}
					else
					{
						AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nEnd - 2, rect[0].GetRight(), 1 ) );
						AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nEnd - 1, rect[2].x, 1 ) );
					}
				}
				AddRect( eType_Bar, m_bars, TRectangle<int32>( rect[0].GetRight(), nBegin, nWidth - rect[0].GetRight(), h1 ) );

				FillRoomGap( TRectangle<int32>( rect[0].GetRight(), rect[0].y, rect[1].x - rect[0].GetRight(), rect[1].GetBottom() - rect[0].y ), false, false );
				FillRoomGap( TRectangle<int32>( rect[1].GetRight(), rect[1].y, rect[2].x - rect[1].GetRight(), rect[2].GetBottom() - rect[1].y ), false, false );
			}
			else
			{
				rect[0] = TRectangle<int32>( w1 + w2 * 2 + 6, nBegin, 6, nEnd - nBegin - h1 * 2 );
				rect[1] = TRectangle<int32>( w1 + w2, nBegin + h1, 6, nEnd - nBegin - h1 * 2 );
				rect[2] = TRectangle<int32>( 0, nBegin + h1, w1, nEnd - nBegin - h1 );
				AddRoom( 0, rect[0] );
				AddRoom( 0, rect[1] );
				AddRoom( 1, rect[2] );
				for( int i = 0; i < 3; i++ )
				{
					m_gendata[rect[i].GetRight() - 1 + ( rect[i].y + 1 ) * nWidth] = m_gendata[rect[i].GetRight() - 1 + ( rect[i].y + 2 ) * nWidth] = eType_Door;
					if( i < 2 )
					{
						if( h1 == 1 )
							m_gendata[rect[i].x + ( rect[i].y + 1 ) * nWidth] = m_gendata[rect[i].x + ( rect[i].y + 2 ) * nWidth] = eType_Door;
						else
							m_gendata[rect[i].x + ( rect[i].GetBottom() - 2 ) * nWidth] = m_gendata[rect[i].x + ( rect[i].GetBottom() - 3 ) * nWidth] = eType_Door;
					}
					else
						m_gendata[rect[i].x + 1 + ( rect[i].GetBottom() - 1 ) * nWidth] = m_gendata[rect[i].x + 2 + ( rect[i].GetBottom() - 1 ) * nWidth] = eType_Door;
				}
				if( ( w2 + 6 ) * 2 + w1 <= nWidth - 5 )
					AddRect( eType_WallChunk, m_wallChunks, TRectangle<int32>( w1 + ( w2 + 6 ) * 2, nBegin, nWidth - w1 - ( w2 + 6 ) * 2, nEnd - nBegin - h1 * 2 ) );
				else
					FillRoomGap( TRectangle<int32>( w1 + ( w2 + 6 ) * 2, nBegin, nWidth - w1 - ( w2 + 6 ) * 2, nEnd - nBegin - h1 * 2 ), false, true );

				if( i < vecTypes.size() - 1 )
				{
					if( h1 == 2 )
					{
						AddRect( eType_Bar, m_bars, TRectangle<int32>( rect[0].x, nEnd - 4, nWidth - rect[0].x, 1 ) );
						int32 l = SRand::Inst().Rand( 1, 3 );
						int32 r = SRand::Inst().Rand( 1, 3 );
						AddRect( eType_Stone, m_stones, TRectangle<int32>( rect[0].x - l, nEnd - 3, l + r, 3 ) );
						AddRect( eType_Bar, m_bars, TRectangle<int32>( rect[2].GetRight(), nEnd - 2, rect[0].x - rect[2].GetRight() - l, 2 ) );
						int8 nFillType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
						for( int j = nEnd - 3; j < nEnd; j++ )
						{
							for( int i = rect[0].x + r; i < nWidth; i++ )
								m_gendata[i + j * nWidth] = nFillType;
						}
					}
					else
					{
						AddRect( eType_Bar, m_bars, TRectangle<int32>( rect[0].x, nEnd - 2, nWidth - rect[0].x, 1 ) );
						AddRect( eType_Bar, m_bars, TRectangle<int32>( rect[2].GetRight(), nEnd - 1, nWidth - rect[2].GetRight(), 1 ) );
					}
				}
				AddRect( eType_Bar, m_bars, TRectangle<int32>( 0, nBegin, rect[0].x, h1 ) );

				FillRoomGap( TRectangle<int32>( rect[1].GetRight(), rect[1].y, rect[0].x - rect[1].GetRight(), rect[0].GetBottom() - rect[0].y ), false, false );
				FillRoomGap( TRectangle<int32>( rect[2].GetRight(), rect[2].y, rect[1].x - rect[2].GetRight(), rect[1].GetBottom() - rect[1].y ), false, false );
			}

			if( i == vecTypes.size() - 1 )
			{
				nFillEnd = nHeight;
				vector<TVector2<int32> > vec;
				for( int i = 0; i < nWidth; i++ )
				{
					for( int j = nHeight - 1; j >= nBegin; j-- )
					{
						if( m_gendata[i + j * nWidth] )
							break;
						vec.push_back( TVector2<int32>( i, j ) );
					}
				}
				SRand::Inst().Shuffle( vec );
				
				int32 s = 0;
				int8 nType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
				for( auto& p : vec )
				{
					if( !SRand::Inst().Rand( 0, 3 ) )
					{
						TVector2<int32> p1 = p;
						if( nType == eType_Block1x )
							p1.x = ( p1.x + p1.y ) & 1 ? p1.x - 1 : p1.x + 1;
						else
							p1.x = ( p1.x + p1.y + 1 ) & 1 ? p1.x - 1 : p1.x + 1;
						if( !m_gendata[p.x + p.y * nWidth] && !m_gendata[p1.x + p1.y * nWidth] )
							m_gendata[p.x + p.y * nWidth] = m_gendata[p1.x + p1.y * nWidth] = nType;
					}
					if( s * 4 < vec.size() )
					{
						auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ),
							TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
						if( rect.width > 0 )
						{
							s += rect.width * rect.height;
							m_stones.push_back( rect );
						}
					}
					else if( s * 5 / 2 < vec.size() )
					{
						m_gendata[p.x + p.y * nWidth] = eType_Bonus;
						s++;
					}
					else
						break;
				}
			}
			break;
		}
		}
	}

	if( nFillBegin > 0 )
	{
		int32 l = 0;
		for( int i = 0; i <= nWidth; i++ )
		{
			if( i < nWidth && m_gendata[i + nFillBegin * nWidth] >= eType_Bar && m_gendata[i + nFillBegin * nWidth] <= eType_Room )
				l++;
			else
			{
				if( l > 2 )
				{
					int32 nType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
					int32 x0 = i - l, x1 = i;
					for( int y = nFillBegin - 1; y >= 0; y-- )
					{
						int32 r = SRand::Inst().Rand( 0, 3 );
						if( r == 1 )
							x0++;
						else if( r == 2 )
							x1--;
						if( x1 <= x0 )
							break;
						for( int x = x0; x < x1; x++ )
							m_gendata[x + y * nWidth] = nType;
					}
				}
				l = 0;
			}
		}
	}

	if( nFillEnd < nHeight )
	{
		int32 l = 0;
		for( int i = 0; i <= nWidth; i++ )
		{
			if( i < nWidth && m_gendata[i + ( nFillEnd - 1 ) * nWidth] > eType_Temp )
				l++;
			else
			{
				if( l > 2 )
				{
					int32 nType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
					int32 x0 = i - l, x1 = i;
					for( int y = nFillEnd; y < nHeight; y++ )
					{
						int32 r = SRand::Inst().Rand( 0, 3 );
						if( r == 1 )
							x0++;
						else if( r == 2 )
							x1--;
						if( x1 <= x0 )
							break;
						for( int x = x0; x < x1; x++ )
							m_gendata[x + y * nWidth] = nType;
					}
				}
				l = 0;
			}
		}
	}
}

void CLevelBonusGenNode1_1::FixBlocks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			auto& data = m_gendata[i + j * nWidth];
			if( data == eType_Block1x || data == eType_Block2x )
			{
				int32 i1;
				if( data == eType_Block1x )
					i1 = ( i + j ) & 1 ? i - 1 : i + 1;
				else
					i1 = ( i + j + 1 ) & 1 ? i - 1 : i + 1;
				if( i1 < 0 || i1 >= nWidth )
					continue;
				int8 data1 = m_gendata[i1 + j * nWidth];
				if( data1 >= eType_Obj )
					data = eType_Bonus;
			}
		}

		int8 nTypes[] = { eType_Obj, eType_Obj + 1, eType_Obj + 2, eType_Bonus };
		LvGenLib::DropObjs( m_gendata, nWidth, nHeight, eType_None, nTypes, ELEM_COUNT( nTypes ) );
	}
}

void CLevelBonusGenNode1_1::AddRect( uint8 nType, vector<TRectangle<int32>>& vec, const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vec.push_back( rect );
	for( int j = rect.y; j < rect.GetBottom(); j++ )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			m_gendata[i + j * nWidth] = nType;
		}
	}
}

void CLevelBonusGenNode1_1::AddRoom( uint8 nType, const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	SRoom room;
	room.nType = nType;
	room.rect = rect;
	m_rooms.push_back( room );
	for( int j = rect.y; j < rect.GetBottom(); j++ )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
				m_gendata[i + j * nWidth] = eType_Room;
		}
	}
}

void CLevelBonusGenNode1_1::FillRoomGap( TRectangle<int32> rect, bool bUp, bool bDown, bool bCheckDoor )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	bool bSymmetric = 0 == ( rect.width & 1 );
	int32 nLeftDoorBegin = -1;
	int32 nRightDoorBegin = -1;
	int32 nLeftDoorEnd = -1;
	int32 nRightDoorEnd = -1;
	for( int y = 0; y < rect.height; y++ )
	{
		int8 l = -1, r = -1;
		if( rect.x > 0 )
		{
			l = m_gendata[rect.x - 1 + ( y + rect.y ) * nWidth];
			if( bCheckDoor && l <= eType_Door )
			{
				if( nLeftDoorBegin == -1 )
					nLeftDoorBegin = y;
				nLeftDoorEnd = y;
			}
		}
		if( rect.GetRight() < nWidth )
		{
			r = m_gendata[rect.GetRight() + ( y + rect.y ) * nWidth];
			if( bCheckDoor && r <= eType_Door )
			{
				if( nRightDoorBegin == -1 )
					nRightDoorBegin = y;
				nRightDoorEnd = y;
			}
		}
		bSymmetric = bSymmetric && l == r;
	}

	int8 nFillType = eType_Block1x + SRand::Inst().Rand( 0, 2 );
	if( nLeftDoorBegin < 0 && nRightDoorBegin < 0 )
	{
		nLeftDoorBegin = ( rect.height - 1 ) * SRand::Inst().Rand( 0.7f, 1.0f );
		nRightDoorBegin = ( rect.height - 1 ) * SRand::Inst().Rand( 0.7f, 1.0f );
		nLeftDoorEnd = nRightDoorEnd = rect.height - 1;
	}
	else if( nLeftDoorBegin < 0 )
	{
		int32 h = Min( SRand::Inst().Rand( 1, 4 ), rect.height );
		if( bDown )
		{
			nLeftDoorBegin = 0;
			nLeftDoorEnd = nLeftDoorBegin + h - 1;
			nRightDoorBegin = 0;
			nRightDoorEnd = Min( nRightDoorEnd + SRand::Inst().Rand( 1, 4 ), rect.height - 1 );
		}
		else if( bUp )
		{
			nLeftDoorBegin = rect.height - h;
			nLeftDoorEnd = rect.height - 1;
			nRightDoorBegin = Max( nRightDoorBegin - SRand::Inst().Rand( 1, 4 ), 0 );
			nRightDoorEnd = rect.height - 1;
		}
		else
		{
			nLeftDoorBegin = SRand::Inst().Rand( 0, rect.height + 1 - h );
			nLeftDoorEnd = nLeftDoorBegin + h - 1;
			nRightDoorBegin = Max( nRightDoorBegin - SRand::Inst().Rand( 1, 4 ), 0 );
			nRightDoorEnd = Min( nRightDoorEnd + SRand::Inst().Rand( 1, 4 ), rect.height - 1 );
			int32 w = SRand::Inst().Rand( 1, (int32)ceil( rect.width * 0.4f ) );
			w = Min( 2, Min( w, Max( 0, rect.width - 2 ) ) );
			for( int j = 0; j < rect.height; j++ )
			{
				for( int i = 0; i < w; i++ )
				{
					auto& data = m_gendata[i + rect.x + ( j + rect.y ) * nWidth];
					if( !data )
						data = nFillType;
				}
			}
			rect.SetLeft( rect.x + w );
		}
	}
	else if( nRightDoorBegin < 0 )
	{
		int32 h = Min( SRand::Inst().Rand( 1, 4 ), rect.height );
		if( bDown )
		{
			nRightDoorBegin = 0;
			nRightDoorEnd = nRightDoorBegin + h - 1;
			nLeftDoorBegin = 0;
			nLeftDoorEnd = Min( nLeftDoorEnd + SRand::Inst().Rand( 1, 4 ), rect.height - 1 );
		}
		else if( bUp )
		{
			nRightDoorBegin = rect.height - h;
			nRightDoorEnd = rect.height - 1;
			nLeftDoorBegin = Max( nLeftDoorBegin - SRand::Inst().Rand( 1, 4 ), 0 );
			nLeftDoorEnd = rect.height - 1;
		}
		else
		{
			nRightDoorBegin = SRand::Inst().Rand( 0, rect.height + 1 - h );
			nRightDoorEnd = nRightDoorBegin + h - 1;
			nLeftDoorBegin = Max( nLeftDoorBegin - SRand::Inst().Rand( 1, 4 ), 0 );
			nLeftDoorEnd = Min( nLeftDoorEnd + SRand::Inst().Rand( 1, 4 ), rect.height - 1 );
		}
		int32 w = SRand::Inst().Rand( 1, (int32)ceil( rect.width * 0.4f ) );
		w = Min( 2, Min( w, Max( 0, rect.width - 2 ) ) );
		for( int j = 0; j < rect.height; j++ )
		{
			for( int i = 0; i < w; i++ )
			{
				auto& data = m_gendata[rect.GetRight() - 1 - i + ( j + rect.y ) * nWidth];
				if( !data )
					data = nFillType;
			}
		}
		rect.SetRight( rect.GetRight() - w );
	}

	if( bSymmetric )
	{
		rect.width /= 2;
		if( nRightDoorBegin >= 0 )
		{
			if( nRightDoorBegin > 2 )
				nRightDoorBegin = SRand::Inst().Rand( 2, nRightDoorBegin );
			if( nRightDoorEnd < rect.height - 2 )
				nRightDoorEnd = SRand::Inst().Rand( nRightDoorEnd + 1, rect.height - 1 );
		}
	}

	vector<int32> vec;
	vec.resize( rect.width );
	vector<int32> vec1;
	vec1.resize( rect.width );
	for( int i = 0; i < 2; i++ )
	{
		vec[0] = Max( 0, i == 0 ? nLeftDoorBegin : rect.height - nLeftDoorEnd - 1 );
		vec[rect.width - 1] = Max( 0, i == 0 ? nRightDoorBegin : rect.height - nRightDoorEnd - 1 );
		vector<pair<int32, int32> > vecSegs;
		vecSegs.push_back( pair<int32, int32>( 0, rect.width - 1 ) );
		for( int n = 0; n < vecSegs.size(); n++ )
		{
			int32 nBegin = vecSegs[n].first;
			int32 nEnd = vecSegs[n].second;
			if( nEnd - nBegin < 2 )
				continue;
			int32 nMid = ( nBegin + nEnd + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 nValue = ( vec[nBegin] + vec[nEnd] + SRand::Inst().Rand( 0, 2 ) ) / 2;
			nValue = Max( Min( nValue, Max( vec[nBegin], vec[nEnd] ) ), Min( vec[nBegin], vec[nEnd] ) );
			for( ; nValue > 0; nValue-- )
			{
				if( !m_gendata[nMid + rect.x + ( i == 0 ? nValue - 1 + rect.y : rect.GetBottom() - nValue ) * nWidth] )
					break;
			}
			vec[nMid] = nValue;
			vecSegs.push_back( pair<int32, int32>( nBegin, nMid ) );
			vecSegs.push_back( pair<int32, int32>( nMid, nEnd ) );
		}
		vec1[0] = Max( 0, ( vec[0] + SRand::Inst().Rand( 0, 2 ) ) / 2 );
		vec1[rect.width - 1] = Max( 0, ( vec[rect.width - 1] + SRand::Inst().Rand( 0, 2 ) ) / 2 );
		vecSegs.clear();
		vecSegs.push_back( pair<int32, int32>( 0, rect.width - 1 ) );
		for( int n = 0; n < vecSegs.size(); n++ )
		{
			int32 nBegin = vecSegs[n].first;
			int32 nEnd = vecSegs[n].second;
			if( nEnd - nBegin < 2 )
				continue;
			int32 nMid = ( nBegin + nEnd + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 nValue = ( vec1[nBegin] + vec1[nEnd] - vec[nBegin] - vec[nEnd] + vec[nMid] * 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			nValue = Max( Min( nValue, Min( nBegin, nEnd ) ), Max( nBegin, nEnd ) );
			nValue = Min( nValue, vec[nMid] );
			vec1[nMid] = nValue;
			vecSegs.push_back( pair<int32, int32>( nBegin, nMid ) );
			vecSegs.push_back( pair<int32, int32>( nMid, nEnd ) );
		}
		for( int x = 0; x < rect.width; x++ )
		{
			int32 h = i == 1 ? rect.height - vec[x] : 0;
			for( int j = 0; j < vec[x]; j++ )
			{
				auto& data = m_gendata[x + rect.x + ( j + rect.y + h ) * nWidth];
				if( data )
					continue;
				if( vec1[x] < vec[x] )
				{
					vec1[x]++;
					data = SRand::Inst().Rand( 0, 4 ) ? eType_Bonus : SRand::Inst().Rand<int8>( eType_Obj, eType_Obj_Max );
				}
				else
					data = nFillType;
			}
		}
	}

	if( bSymmetric )
		CopyData( rect.Offset( TVector2<int32>( rect.width, 0 ) ), TVector2<int32>( rect.x, rect.y ), true );
}

void CLevelBonusGenNode1_1::CopyData( const TRectangle<int32>& dst, const TVector2<int32>& src, bool bFlipX )
{
	for( int j = 0; j < dst.height; j++ )
	{
		int32 srcY = src.y + j;
		int32 dstY = dst.y + j;
		for( int i = 0; i < dst.width; i++ )
		{
			int32 srcX = bFlipX ? src.x + dst.width - i - 1 : src.x + i;
			int32 dstX = dst.x + i;
			auto n = m_gendata[srcX + srcY * m_region.width];
			auto& d = m_gendata[dstX + dstY * m_region.width];
			if( n >= eType_Block1x && !d )
				d = n;
		}
	}
}
