#include "stdafx.h"
#include "LvGen1.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

void CLevelGenNode1_1_0::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1Node = CreateNode( pXml->FirstChildElement( "block1" )->FirstChildElement(), context );
	m_pBlock2Node = CreateNode( pXml->FirstChildElement( "block2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );
	m_pBonusNode = CreateNode( pXml->FirstChildElement( "bonus" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_1_0::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	GenAreas();
	MakeHoles();

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData == eType_Wall || genData == eType_None )
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

	context.mapTags["mask"] = eType_Block1;
	m_pBlock1Node->Generate( context, region );
	context.mapTags["mask"] = eType_Block2;
	m_pBlock2Node->Generate( context, region );
	for( auto& rect : m_vecStones )
	{
		m_pStoneNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_vecStones.clear();
}

void CLevelGenNode1_1_0::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	const int32 nMinHeight = 4;
	const int32 nMaxHeight = 6;
	const int32 nMinFillCount = 16;
	const int32 nMaxFillCount = 40;
	const float fFillMinPercent = 0.4f;
	const float fFillMaxPercent = 0.6f;
	const float fStoneMinPercent = 0.01f;
	const float fStoneMaxPercent = 0.04f;
	const TVector2<int32> stoneMinSize( 2, 2 ), stoneMaxSize( 4, 4 );
	TRectangle<int32> emptyArea( 12, 0, 8, 6 );

	vector<int32> vecLastHeight;
	vector<int32> vecCurHeight;
	vecLastHeight.resize( nWidth );
	vecCurHeight.resize( nWidth );
	for( int i = 0; i < nWidth; i++ )
	{
		vecLastHeight[i] = Max( 0, emptyArea.GetBottom() - nMinHeight - Max( 0, Max( emptyArea.x - i, i - emptyArea.GetRight() + 1 ) ) );
	}

	int h1 = vecLastHeight[0];
	int h2 = vecLastHeight[nWidth - 1];

	for( ;; )
	{
		bool bLeftToRight = SRand::Inst().Rand( 0, 2 );
		int32 h = ( bLeftToRight ? h1 : h2 ) + SRand::Inst().Rand( nMinHeight, nMaxHeight + 1 );
		if( h >= nHeight )
		{
			bLeftToRight = !bLeftToRight;
			h = ( bLeftToRight ? h1 : h2 ) + SRand::Inst().Rand( nMinHeight, nMaxHeight + 1 );
			if( h >= nHeight )
				break;
		}
		int8 nGenData = ( ( h + ( bLeftToRight ? 0 : nWidth ) ) & 1 ) + eType_Block1;
		for( int i = 0; i < nWidth; i++ )
		{
			int32 x = bLeftToRight ? i : nWidth - i - 1;
			int32 x0 = bLeftToRight ? x - 1 : x + 1;
			int32 x1 = bLeftToRight ? x + 1 : x - 1;

			int8 nDir;
			if( i == 0 )
				nDir = 1;
			else
			{
				bool bDir[3] = 
				{
					true,
					h >= vecLastHeight[x] + nMinHeight && ( i == nWidth - 1 || h >= vecLastHeight[x1] + nMinHeight ),
					h - 1 >= vecLastHeight[x0] + nMinHeight && h - 1 >= vecLastHeight[x] + nMinHeight,
				};
				int32 nDirs[3] = { 0, 1, 2 };
				SRand::Inst().Shuffle( nDirs, 3 );
				int32 k;
				for( k = 0; k < 3; k++ )
				{
					nDir = nDirs[k];
					if( bDir[nDir] )
						break;
				}

				if( k == 3 )
					nDir = 1;
			}

			switch( nDir )
			{
			case 0:
				h++;
				if( h < nHeight )
					m_gendata[x0 + h * nWidth] = m_gendata[x + h * nWidth] = nGenData;
				vecCurHeight[x0] = vecCurHeight[x] = h;
				break;
			case 1:
				if( h < nHeight )
					m_gendata[x + h * nWidth] = nGenData;
				vecCurHeight[x] = h;
				if( i < nWidth - 1 )
				{
					if( h < nHeight )
						m_gendata[x1 + h * nWidth] = nGenData;
					vecCurHeight[x1] = h;
				}
				i++;
				break;
			case 2:
				h--;
				if( h < nHeight )
					m_gendata[x0 + h * nWidth] = m_gendata[x + h * nWidth] = nGenData;
				vecCurHeight[x] = h;
				break;
			}
		}
		h1 = vecCurHeight[0];
		h2 = vecCurHeight[nWidth - 1];
		for( int k = 0; k < nWidth; k++ )
			vecLastHeight[k] = vecCurHeight[k];
	}

	int32 nFillCount = nWidth * nHeight * SRand::Inst().Rand( fFillMinPercent, fFillMaxPercent );

	nFillCount -= FloodFill( m_gendata, nWidth, nHeight, emptyArea.x, emptyArea.y, eType_Wall, nFillCount );
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vecEmpty );
	SRand::Inst().Shuffle( vecEmpty );
	int i;
	for( i = 0; i < vecEmpty.size() && nFillCount; i++ )
	{
		TVector2<int32> p = vecEmpty[i];
		if( !m_gendata[p.x + p.y * nWidth] )
			nFillCount -= FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, eType_Wall, Min( nFillCount, SRand::Inst().Rand( nMinFillCount, nMaxFillCount ) ) );
	}

	int32 nStoneCount = nWidth * nHeight * SRand::Inst().Rand( fStoneMinPercent, fStoneMaxPercent );
	for( ; i < vecEmpty.size(); i++ )
	{
		TVector2<int32> p = vecEmpty[i];
		if( !m_gendata[p.x + p.y * nWidth] )
		{
			TVector2<int32> size( SRand::Inst().Rand( stoneMinSize.x, stoneMaxSize.x + 1 ), SRand::Inst().Rand( stoneMinSize.y, stoneMaxSize.y + 1 ) );
			TRectangle<int32> rect = PutRect( m_gendata, nWidth, nHeight, p, stoneMinSize, stoneMaxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
			if( rect.width )
			{
				m_vecStones.push_back( rect );
				nStoneCount -= rect.width * rect.height;
				if( nStoneCount <= 0 )
					break;
			}
		}
	}

	for( ; i < vecEmpty.size(); i++ )
	{
		TVector2<int32> p = vecEmpty[i];
		if( !m_gendata[p.x + p.y * nWidth] )
		{
			int8 nType = ( SRand::Inst().Rand( 0, 2 ) ) + eType_Block1;
			FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, nType );
		}
	}
}

void CLevelGenNode1_1_0::MakeHoles()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const float fMinPercent = 0.1f;
	const float fMaxPercent = 0.2f;
	const float fObjPercent = 0.02f;
	const float fBonusPercent = 0.02f;

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Block1, vec );
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Block2, vec );

	int32 nHoleCount = vec.size() * SRand::Inst().Rand( fMinPercent, fMaxPercent );
	int32 nObjCount = vec.size() * fObjPercent;
	int32 nBonusCount = vec.size() * fBonusPercent;
	SRand::Inst().Shuffle( vec );

	for( int i = 0; i < vec.size() && nHoleCount; i++ )
	{
		TVector2<int32> p = vec[i];
		int8 nType = m_gendata[p.x + p.y * nWidth];
		if( nType == eType_Block1 || nType == eType_Block2 )
		{
			TVector2<int32> p1 = p;
			if( nType == eType_Block1 )
				p1.x = ( p1.x + p1.y ) & 1 ? p1.x - 1 : p1.x + 1;
			else
				p1.x = ( p1.x + p1.y + 1 ) & 1 ? p1.x - 1 : p1.x + 1;
			if( p1.x < 0 || p1.x >= nWidth )
				continue;
			if( m_gendata[p1.x + p1.y * nWidth] != nType )
				continue;

			if( p1.x < p.x )
				swap( p.x, p1.x );
			bool bSucceed = true;
			for( int x = Max( 0, p.x - 1 ); x <= Min( nWidth - 1, p1.x + 1 ); x++ )
			{
				for( int y = Max( 0, p.y - 1 ); y <= Min( nHeight - 1, p.y + 1 ); y++ )
				{
					if( m_gendata[x + y * nWidth] <= eType_Bonus )
					{
						bSucceed = false;
						break;
					}
				}
				if( !bSucceed )
					break;
			}
			if( !bSucceed )
				continue;

			m_gendata[p.x + p.y * nWidth] = m_gendata[p1.x + p1.y * nWidth] = eType_Wall;
			nHoleCount--;

			bool b = SRand::Inst().Rand( 0, 2 );
			if( nObjCount && SRand::Inst().Rand( 0, 2 ) )
			{
				auto obj = b ? p : p1;
				m_gendata[obj.x + obj.y * nWidth] = eType_Obj;
				nObjCount--;
			}
			if( nBonusCount && SRand::Inst().Rand( 0, 2 ) )
			{
				auto obj = !b ? p : p1;
				m_gendata[obj.x + obj.y * nWidth] = eType_Bonus;
				nBonusCount--;
			}
		}
	}
}

void CLevelGenNode1_1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1xNode = CreateNode( pXml->FirstChildElement( "block1x" )->FirstChildElement(), context );
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );
	m_pBonusNode = CreateNode( pXml->FirstChildElement( "bonus" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	GenMainPath();
	Flatten();
	GenRooms();
	GenObstacles();
	GenObjsBig();
	GenObjsSmall();
	GenBlocks();
	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData == eType_Path || genData == eType_Temp )
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
	context.mapTags["door"] = eType_Door;
	for( auto& rect : m_rooms )
	{
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["mask"] = eType_Web;
	for( auto& rect : m_wallChunks )
	{
		m_pWallChunkNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_bars.clear();
	m_stones.clear();
	m_rooms.clear();
	m_wallChunks.clear();
}

void CLevelGenNode1_1_1::GenMainPath()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nMinSizePerWaypoint = 128;
	const int32 nMaxSizePerWaypoint = 160;
	const float fMinPathPercent = 0.55f;
	const float fMaxPathPercent = 0.6f;
	const float f2WayChance = 0.25f;
	int32 nWaypointRadius = 2;
	int32 nWaypoints = m_region.width * m_region.height / SRand::Inst().Rand( nMinSizePerWaypoint, nMaxSizePerWaypoint );
	nWaypoints = Max( 3, nWaypoints );

	vector<TVector2<int32> > vecWaypoints;
	{
		vecWaypoints.push_back( TVector2<int32>( SRand::Inst().Rand( nWaypointRadius, nWidth - nWaypointRadius ), 0 ) );
		uint32 a = nWaypoints - 2;
		nWaypointRadius = ( nHeight - nWaypoints ) / ( 2 + a * 2 );
		uint32 b = nHeight - 2 - ( 2 + a * 2 ) * nWaypointRadius;
		int8* result = (int8*)alloca( b );
		SRand::Inst().C( a, b, result );
		int32 nCurY = 1 + nWaypointRadius;
		for( int i = 0; i <= b; i++ )
		{
			if( i == b || result[i] )
			{
				int32 y = i == b ? nHeight - 1 : nCurY + nWaypointRadius + 1;
				int32 prevX = vecWaypoints.back().x;
				if( prevX < nWidth * 3 / 8 )
				{
					if( SRand::Inst().Rand( 0.0f, 1.0f ) < f2WayChance )
						vecWaypoints.push_back( TVector2<int32>( SRand::Inst().Rand( nWidth * 3 / 8, nWidth - nWidth * 3 / 8 ), y ) );
					else
						vecWaypoints.push_back( TVector2<int32>( SRand::Inst().Rand( nWidth - nWidth / 8, nWidth ), y ) );
				}
				else if( prevX >= nWidth - nWidth * 3 / 8 )
				{
					if( SRand::Inst().Rand( 0.0f, 1.0f ) < f2WayChance )
						vecWaypoints.push_back( TVector2<int32>( SRand::Inst().Rand( nWidth * 3 / 8, nWidth - nWidth * 3 / 8 ), y ) );
					else
						vecWaypoints.push_back( TVector2<int32>( SRand::Inst().Rand( 0, nWidth / 8 ), y ) );
				}
				else
				{
					int32 x1 = SRand::Inst().Rand( 0, nWidth / 16 );
					int32 x2 = SRand::Inst().Rand( nWidth - nWidth / 16, nWidth );
					if( !!( SRand::Inst().Rand( 0, 2 ) ) )
						swap( x1, x2 );

					vecWaypoints.push_back( TVector2<int32>( x1, Min( nHeight - 1, SRand::Inst().Rand( nCurY, y + 1 ) ) ) );
					vecWaypoints.push_back( TVector2<int32>( x2, Min( nHeight - 1, SRand::Inst().Rand( y, nCurY + nWaypointRadius * 2 + 1 ) ) ) );
				}
				nCurY += nWaypointRadius * 2 + 1;
			}
			else
				nCurY++;
		}
	}

	for( auto p : vecWaypoints )
	{
		m_gendata[p.x + p.y * nWidth] = eType_Temp;
	}
	vector<TVector2<int32> > par;
	par.resize( nWidth * nHeight );
	TVector2<int32> ofs[2][4] = { { { 3, -1 }, { 3, 0 }, { 2, 0 }, { 1, 1 } }, { { -3, -1 }, { -3, 0 }, { -2, 0 }, { -1, 1 } } };
	for( auto p : vecWaypoints )
	{
		m_gendata[p.x + p.y * nWidth] = eType_None;
		FindPath( m_gendata, nWidth, nHeight, p, eType_Path, eType_Temp, par, ofs[0], 4 );
		m_gendata[p.x + p.y * nWidth] = eType_None;
		FindPath( m_gendata, nWidth, nHeight, p, eType_Path, eType_Temp, par, ofs[1], 4 );
		m_gendata[p.x + p.y * nWidth] = eType_Path;
	}
	FloodFillExpand( m_gendata, nWidth, nHeight, eType_Path, eType_None, nWidth * nHeight * SRand::Inst().Rand( fMinPathPercent, fMaxPathPercent ) );
}

void CLevelGenNode1_1_1::Flatten()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( int j = 1; j < nHeight; j++ )
	{
		for( int i = 1; i < nWidth - 1; i++ )
		{
			if( m_gendata[i + j * nWidth] != eType_None )
				continue;
			if(  m_gendata[i - 1 + j * nWidth] == eType_None )
				continue;
			if(  m_gendata[i + 1 + j * nWidth] == eType_None )
				continue;

			m_gendata[i + j * nWidth] = eType_Path;
		}
	}

	for( int j = 1; j < nHeight; j++ )
	{
		for( int i = 1; i < nWidth - 1; i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
				continue;
			if( m_gendata[i - 1 + j * nWidth] != eType_None )
				continue;
			if( m_gendata[i + 1 + j * nWidth] != eType_None )
				continue;

			m_gendata[i + j * nWidth] = eType_None;
		}
	}
}

void CLevelGenNode1_1_1::GenRooms()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	uint8 b = SRand::Inst().Rand( 0, 2 );
	vector<TVector2<int32> > vecWeb;
	for( int y = 0; y < nHeight; y++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			int32 x = b ? i : nWidth - i - 1;
			if( m_gendata[x + y * nWidth] != eType_Path )
				continue;
			TVector2<int32> minSize;
			if( y >= nHeight / 2 && SRand::Inst().Rand( 0, 2 ) )
				minSize = TVector2<int32>( 7, 6 );
			else
				minSize = TVector2<int32>( 8, SRand::Inst().Rand( 4, 6 ) );
			TVector2<int32> maxSize;
			maxSize.x = SRand::Inst().Rand( minSize.x, minSize.x * 2 );
			maxSize.y = 6;
			auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), minSize, maxSize,
				TRectangle<int32>( 0, y, nWidth, nHeight - y ), -1, eType_Path );
			if( rect.width > 0 )
			{
				bool bRoom = y >= nHeight / 2 && rect.height >= 6;
				if( bRoom )
				{
					uint32 w = Min( rect.width, SRand::Inst().Rand( 6, 8 ) );
					if( rect.x + rect.GetRight() <= nWidth * 2 + SRand::Inst().Rand( 0, 2 ) )
						rect.SetLeft( rect.GetRight() - w );
					else
						rect.width = w;
					m_rooms.push_back( rect );
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						for( int i = rect.x; i < rect.GetRight(); i++ )
						{
							m_gendata[i + j * nWidth] = eType_Room;
						}
					}
				}
				else
				{
					m_wallChunks.push_back( rect );
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						for( int i = rect.x; i < rect.GetRight(); i++ )
						{
							m_gendata[i + j * nWidth] = eType_WallChunk;
							if( SRand::Inst().Rand( 0.0f, 1.0f ) < 0.05f )
								vecWeb.push_back( TVector2<int32>( i, j ) );
						}
					}
				}
				y += rect.height / 2;
				b = 1 - b;
			}
		}
	}

	for( auto& room : m_rooms )
	{
		uint8 nType = SRand::Inst().Rand( 0, 3 );
		switch( nType )
		{
		case 0:
			m_gendata[room.x + ( room.y + 1 ) * nWidth] = m_gendata[room.x + ( room.y + 2 ) * nWidth] = eType_Door;
			m_gendata[room.GetRight() - 1 + ( room.y + 1 ) * nWidth] = m_gendata[room.GetRight() - 1 + ( room.y + 2 ) * nWidth] = eType_Door;
			break;
		case 1:
			m_gendata[room.GetRight() - 1 + ( room.y + 1 ) * nWidth] = m_gendata[room.GetRight() - 1 + ( room.y + 2 ) * nWidth] = eType_Door;
			m_gendata[room.x + ( room.GetBottom() - 3 ) * nWidth] = m_gendata[room.x + ( room.GetBottom() - 2 ) * nWidth] = eType_Door;
			if( SRand::Inst().Rand( 0, 2 ) )
				m_gendata[room.x + 1 + room.y * nWidth] = m_gendata[room.x + 2 + room.y * nWidth] = eType_Door;
			else
				m_gendata[room.GetRight() - 3 + ( room.GetBottom() - 1 ) * nWidth] = m_gendata[room.GetRight() - 2 + ( room.GetBottom() - 1 ) * nWidth] = eType_Door;
			break;
		case 2:
			m_gendata[room.x + ( room.y + 1 ) * nWidth] = m_gendata[room.x + ( room.y + 2 ) * nWidth] = eType_Door;
			m_gendata[room.GetRight() - 1 + ( room.GetBottom() - 3 ) * nWidth] = m_gendata[room.GetRight() - 1 + ( room.GetBottom() - 2 ) * nWidth] = eType_Door;
			if( SRand::Inst().Rand( 0, 2 ) )
				m_gendata[room.GetRight() - 3 + room.y * nWidth] = m_gendata[room.GetRight() - 2 + room.y * nWidth] = eType_Door;
			else
				m_gendata[room.x + 1 + ( room.GetBottom() - 1 ) * nWidth] = m_gendata[room.x + 2 + ( room.GetBottom() - 1 ) * nWidth] = eType_Door;
			break;
		default:
			break;
		}
	}
	SRand::Inst().Shuffle( vecWeb );
	for( auto p : vecWeb )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_WallChunk )
			continue;
		m_gendata[p.x + p.y * nWidth] = eType_Web;
		int32 nDir = SRand::Inst().Rand( 0, 2 ) * 2 - 1;
		int32 n = SRand::Inst().Rand( 2, 6 );
		for( int i = 0; i < n; i++ )
		{
			int32 r = SRand::Inst().Rand( 0, 5 );
			if( r == 0 )
			{
				p.y--;
				if( p.y < 0 || m_gendata[p.x + p.y * nWidth] != eType_WallChunk )
					break;
				m_gendata[p.x + p.y * nWidth] = eType_Web;
			}
			else if( r == 1 )
			{
				p.y++;
				if( p.y >= nHeight || m_gendata[p.x + p.y * nWidth] != eType_WallChunk )
					break;
				m_gendata[p.x + p.y * nWidth] = eType_Web;
			}
			p.x += nDir;
			if( p.x < 0 || p.x >= nWidth || m_gendata[p.x + p.y * nWidth] != eType_WallChunk )
				break;
			m_gendata[p.x + p.y * nWidth] = eType_Web;
		}
	}
}

void CLevelGenNode1_1_1::GenObstacles()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nMinFloodFillSize = 32;
	const int32 nMaxFloodFillSize = 64;
	const float fMinBarPercent = 0.25f;
	const float fMaxBarPercent = 0.4f;
	const int32 nBarMinSize = 6;
	const int32 nBarMaxSize = 12;
	const int32 nBar2MinSize = 6;
	const int32 nBar2MaxSize = 8;
	const float fBar2Chance = 0.2f;

	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vecEmpty );
	SRand::Inst().Shuffle( vecEmpty );

	int i;
	TVector2<int32> ofs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 2, 0 }, { 0, -2 } };
	for( i = 0; i < vecEmpty.size(); i++ )
	{
		TVector2<int32> p = vecEmpty[i];
		if( m_gendata[p.x + p.y * nWidth] )
			continue;

		vector<TVector2<int32> > q;
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, eType_Temp, SRand::Inst().Rand( nMinFloodFillSize, nMaxFloodFillSize ), q, ofs, ELEM_COUNT( ofs ) );

		int32 nBarSize = q.size() * SRand::Inst().Rand( fMinBarPercent, fMaxBarPercent );
		bool bBar2 = SRand::Inst().Rand( 0.0f, 1.0f ) < fBar2Chance;
		if( bBar2 )
		{
			int32 nBarWidth = Min( nBar2MaxSize, Max( nBar2MinSize, nBarSize / 2 ) );
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( nBar2MinSize, 2 ), TVector2<int32>( nBarWidth, 2 ),
				TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar );
			if( rect.width > 0 )
				m_bars.push_back( rect );
			else
				bBar2 = false;
		}

		if( !bBar2 )
		{
			int32 nBarWidth = Min( nBarMaxSize, Max( nBarMinSize, nBarSize ) );
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( nBarMinSize, 1 ), TVector2<int32>( nBarWidth, 1 ),
				TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar );
			if( rect.width > 0 )
				m_bars.push_back( rect );
		}

		for( auto p1 : q )
		{
			if( m_gendata[p1.x + p1.y * nWidth] == eType_Temp )
				m_gendata[p1.x + p1.y * nWidth] = eType_Temp1;
		}
	}

	for( auto& bar : m_bars )
	{
		auto bar1 = bar;
		for( ; bar1.GetBottom() < nHeight; bar1.y++ )
		{
			int32 y = bar1.GetBottom();
			bool bSucceed = true;
			for( int x = bar1.x; x < bar1.GetRight(); x++ )
			{
				if( m_gendata[x + y * nWidth] != eType_Temp1 )
				{
					bSucceed = false;
					break;
				}
			}

			if( !bSucceed )
				break;
		}

		if( bar1.y != bar.y )
		{
			for( int y = 0; y < bar.height; y++ )
			{
				for( int x = bar1.x; x < bar1.GetRight(); x++ )
				{
					m_gendata[x + ( y + bar.y ) * nWidth] = eType_Temp1;
				}
			}
			for( int y = 0; y < bar.height; y++ )
			{
				for( int x = bar1.x; x < bar1.GetRight(); x++ )
				{
					m_gendata[x + ( y + bar1.y ) * nWidth] = eType_Bar;
				}
			}

			bar.y = bar1.y;
		}

		if( bar.GetBottom() < nHeight )
		{
			vector<int32> vecStackHeight;
			vecStackHeight.resize( bar.width );

			float fMaxLen = 0;
			int32 nMaxPos = -1;
			uint32 nCurLen = 0;
			for( int i = bar.x; i < bar.GetRight(); i++ )
			{
				int j;
				bool bBar = false;
				for( j = bar.GetBottom(); j < nHeight; j++ )
				{
					if( m_gendata[i + j * nWidth] == eType_Path )
						break;
					else if( m_gendata[i + j * nWidth] != eType_Temp1 )
					{
						bBar = true;
						break;
					}
				}
				vecStackHeight[i - bar.x] = bBar ? ~( j - bar.GetBottom() ) : j - bar.GetBottom();
				if( vecStackHeight[i - bar.x] == 0 )
				{
					nCurLen++;
					float fCurLen = nCurLen + SRand::Inst().Rand( 0.0f, 1.0f );
					if( fCurLen > fMaxLen )
					{
						fMaxLen = fCurLen;
						nMaxPos = i - nCurLen + 1;
					}
				}
				else
					nCurLen = 0;
			}
			if( nMaxPos < 0 )
				continue;

			int32 nMaxLen = floor( fMaxLen );
			if( nMaxPos > bar.x )
			{
				int i;
				for( i = nMaxPos - 1; i >= bar.x; i-- )
				{
					if( vecStackHeight[i - bar.x] < 0 )
						break;
				}
				int32 h;
				if( i < bar.x )
				{
					if( i >= 0 )
					{
						for( h = bar.GetBottom(); h < nHeight; h++ )
						{
							if( m_gendata[i + h * nWidth] == eType_Path )
								break;
						}
						h = Min( vecStackHeight[0], Max( 0, h - bar.GetBottom() - 1 ) );
					}
					else
						h = vecStackHeight[0];
				}
				else
					h = Min( ~vecStackHeight[i - bar.x], vecStackHeight[i - bar.x + 1] );

				int32 i0 = i + 1;
				for( i++; i < nMaxPos; i++ )
				{
					for( int j = h; j < vecStackHeight[i - bar.x]; j++ )
					{
						m_gendata[i + ( j + bar.GetBottom() ) * nWidth] = eType_Path;
					}
					vecStackHeight[i - bar.x] = Min( vecStackHeight[i - bar.x], h );
					h = Max( h - 1, 0 );
				}

				TRectangle<int32> rect;
				if( vecStackHeight[i0 - bar.x] > 0 )
				{
					rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i0, bar.GetBottom() + vecStackHeight[i0 - bar.x] - 1 ),
						TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 4 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
					if( rect.width > 0 )
						m_stones.push_back( rect );
					rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i0, bar.GetBottom() ),
						TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 4 ), SRand::Inst().Rand( 2, 5 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
					if( rect.width > 0 )
						m_stones.push_back( rect );
				}
				else if( bar.y > 0 && m_gendata[i0 + ( bar.y - 1 ) * nWidth] == eType_Temp1 )
				{
					rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i0, bar.y - 1 ),
						TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
					if( rect.width > 0 )
						m_stones.push_back( rect );
				}
			}
			if( nMaxPos + nMaxLen < bar.GetRight() )
			{
				int i;
				for( i = nMaxPos + nMaxLen; i < bar.GetRight(); i++ )
				{
					if( vecStackHeight[i - bar.x] < 0 )
						break;
				}
				int32 h;
				if( i >= bar.GetRight() )
				{
					if( i < nWidth )
					{
						for( h = bar.GetBottom(); h < nHeight; h++ )
						{
							if( m_gendata[i + h * nWidth] == eType_Path )
								break;
						}
						h = Min( vecStackHeight.back(), Max( 0, h - bar.GetBottom() - 1 ) );
					}
					else
						h = vecStackHeight.back();
				}
				else
					h = Min( ~vecStackHeight[i - bar.x], vecStackHeight[i - bar.x - 1] );

				int32 i0 = i - 1;
				for( i--; i >= nMaxPos + nMaxLen; i-- )
				{
					for( int j = h; j < vecStackHeight[i - bar.x]; j++ )
					{
						m_gendata[i + ( j + bar.GetBottom() ) * nWidth] = eType_Path;
					}
					vecStackHeight[i - bar.x] = Min( vecStackHeight[i - bar.x], h );
					h = Max( h - 1, 0 );
				}
				TRectangle<int32> rect;
				if( vecStackHeight[i0 - bar.x] > 0 )
				{
					rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i0, bar.GetBottom() + vecStackHeight[i0 - bar.x] - 1 ),
						TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
					if( rect.width > 0 )
						m_stones.push_back( rect );
					rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i0, bar.GetBottom() ),
						TVector2<int32>( 2, 2 ), TVector2<int32>( 2, SRand::Inst().Rand( 2, 5 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
					if( rect.width > 0 )
						m_stones.push_back( rect );
				}
				else if( bar.y > 0 && m_gendata[i0 + ( bar.y - 1 ) * nWidth] == eType_Temp1 )
				{
					rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( i0, bar.y - 1 ),
						TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
					if( rect.width > 0 )
						m_stones.push_back( rect );
				}
			}

		}
	}
}

void CLevelGenNode1_1_1::GenObjsBig()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nFillSizeMin = 6;
	const int32 nFillSizeMax = 12;
	int32 nCount = 2;

	vector<int8> vecTemp;
	vecTemp.resize( m_gendata.size() );
	for( int i = 0; i < m_gendata.size(); i++ )
		vecTemp[i] = m_gendata[i] == eType_Path ? 1 : ( m_gendata[i] == eType_Temp1 ? 0 : 2 );
	ExpandDist( vecTemp, nWidth, nHeight, 1, 0, 3 );
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, vecEmpty );
	SRand::Inst().Shuffle( vecEmpty );

	for( auto p : vecEmpty )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;

		vector<TVector2<int32> > q;
		FloodFill( vecTemp, nWidth, nHeight, p.x, p.y, 1, nWidth * nHeight, q );
		int32 n = Min<int32>( nFillSizeMin + ( q.size() * 0.75f - nFillSizeMin ) * 0.25f, nFillSizeMax );
		if( n < nFillSizeMin )
			continue;

		for( int i = 0; i < n; i++ )
		{
			auto p1 = q[i];
			m_gendata[p1.x + p1.y * nWidth] = eType_Path;
		}

		int32 nObjCount = n * 2 / 3;
		for( int i = 0; i < n && nObjCount; i++ )
		{
			auto p1 = q[i];
			if( m_gendata[p1.x + p1.y * nWidth] == eType_Obj || m_gendata[p1.x + p1.y * nWidth] == eType_Bonus )
				continue;

			for( ; p1.y > 0; p1.y-- )
			{
				if( m_gendata[p1.x + ( p1.y - 1 ) * nWidth] != eType_Path )
					break;
			}
			m_gendata[p1.x + p1.y * nWidth] = i % 3 == 0 ? eType_Obj : eType_Bonus;
			nObjCount--;
		}

		nCount--;
		if( !nCount )
			break;
	}
}

void CLevelGenNode1_1_1::GenObjsSmall()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nFillSizeMin = 16;
	const int32 nFillSizeMax = 32;
	const float fObjPercentMin = 0.008f;
	const float fObjPercentMax = 0.01f;
	const float fBonusPercentMin = 0.008f;
	const float fBonusPercentMax = 0.01f;
	const float fObjPercentMin1 = 0.015f;
	const float fObjPercentMax1 = 0.02f;

	vector<int8> vecTemp;
	vecTemp.resize( m_gendata.size() );
	for( int i = 0; i < m_gendata.size(); i++ )
		vecTemp[i] = m_gendata[i] == eType_Path || m_gendata[i] == eType_Obj || m_gendata[i] == eType_Bonus ? 1 : ( m_gendata[i] == eType_Temp1 ? 0 : 2 );
	ExpandDist( vecTemp, nWidth, nHeight, 1, 0, 2 );
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, vecEmpty );
	SRand::Inst().Shuffle( vecEmpty );
	uint32 nObjCount = nWidth * nHeight * SRand::Inst().Rand( fObjPercentMin, fObjPercentMax );
	uint32 nBonusCount = nWidth * nHeight * SRand::Inst().Rand( fBonusPercentMin, fBonusPercentMax );

	for( auto p : vecEmpty )
	{
		if( !nObjCount && !nBonusCount )
			break;
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp1 )
			continue;
		bool bObj = nObjCount && SRand::Inst().Rand( 0, 2 );
		bool bBonus = nBonusCount && SRand::Inst().Rand( 0, 2 );
		if( !bObj && !bBonus )
			continue;

		TVector2<int32> p1s[2];
		int32 nTypes[2];
		int32 nTypeCount = 0;
		if( p.x > 0 && m_gendata[p.x - 1 + p.y * nWidth] == eType_Temp1 )
		{
			p1s[nTypeCount] = TVector2<int32>( p.x - 1, p.y );
			nTypes[nTypeCount] = ( p.x + p.y ) & 1 ? eType_Block1x : eType_Block2x;
			nTypeCount++;
		}
		if( p.x < nWidth - 1 && m_gendata[p.x + 1 + p.y * nWidth] == eType_Temp1 )
		{
			p1s[nTypeCount] = TVector2<int32>( p.x + 1, p.y );
			nTypes[nTypeCount] = ( p.x + p.y ) & 1 ? eType_Block2x : eType_Block1x;
			nTypeCount++;
		}
		if( !nTypeCount )
			continue;
		
		int32 n = SRand::Inst().Rand( 0, nTypeCount );
		int32 nType = nTypes[n];
		TVector2<int32> p1 = p1s[n];
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, nType, SRand::Inst().Rand( nFillSizeMin, nFillSizeMax ) );
		m_gendata[p.x + p.y * nWidth] = m_gendata[p1.x + p1.y * nWidth] = eType_Temp;

		if( bObj && bBonus )
		{
			m_gendata[p.x + p.y * nWidth] = eType_Obj;
			m_gendata[p1.x + p1.y * nWidth] = eType_Bonus;
		}
		else
		{
			int8 nType = bObj ? eType_Obj : eType_Bonus;
			( p.y < p1.y ? m_gendata[p.x + p.y * nWidth] : m_gendata[p1.x + p1.y * nWidth] ) = nType;
		}
		if( bObj )
			nObjCount--;
		if( bBonus )
			nBonusCount--;
	}

	nObjCount = nWidth * nHeight * SRand::Inst().Rand( fObjPercentMin1, fObjPercentMax1 );
	vecEmpty.clear();
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Path, vecEmpty );
	SRand::Inst().Shuffle( vecEmpty );
	for( auto p : vecEmpty )
	{
		if( !nObjCount )
			break;
		if( m_gendata[p.x + p.y * nWidth] != eType_Path )
			continue;

		for( p.y--; p.y >= 0; p.y-- )
		{
			if( m_gendata[p.x + p.y * nWidth] == eType_Path )
				continue;
			if( m_gendata[p.x + p.y * nWidth] == eType_Obj )
				break;

			m_gendata[p.x + ( p.y + 1 ) * nWidth] = eType_Obj;
			nObjCount--;
			break;
		}
	}
}

void CLevelGenNode1_1_1::GenBlocks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int8 nTypes[2] = { eType_Block1x, eType_Block2x };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_Temp1, nTypes, 2 );
}

void CLevelGenNode1_1_2::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1xNode = CreateNode( pXml->FirstChildElement( "block1x" )->FirstChildElement(), context );
	m_pBlock1yNode = CreateNode( pXml->FirstChildElement( "block1y" )->FirstChildElement(), context );
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBlock2yNode = CreateNode( pXml->FirstChildElement( "block2y" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pRoom1Node = CreateNode( pXml->FirstChildElement( "room1" )->FirstChildElement(), context );
	m_pRoom2Node = CreateNode( pXml->FirstChildElement( "room2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );
	m_pBonusNode = CreateNode( pXml->FirstChildElement( "bonus" )->FirstChildElement(), context );
	m_pWebNode = CreateNode( pXml->FirstChildElement( "web" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_1_2::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenRooms();
	GenRooms1();
	AddMoreBars();
	GenObjs();
	GenBlocks();
	GenBonus();

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData == eType_Path )
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
	context.mapTags["mask"] = eType_Block1y;
	m_pBlock1yNode->Generate( context, region );
	context.mapTags["mask"] = eType_Block2x;
	m_pBlock2xNode->Generate( context, region );
	context.mapTags["mask"] = eType_Block2y;
	m_pBlock2yNode->Generate( context, region );
	context.mapTags["mask"] = eType_Web;
	m_pWebNode->Generate( context, region );
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
	context.mapTags["door"] = eType_Door;
	for( auto& room : m_rooms )
	{
		if( room.nType == 0 )
			m_pRoom1Node->Generate( context, room.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pRoom2Node->Generate( context, room.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_par.clear();
	m_stones.clear();
	m_bars.clear();
	m_rooms.clear();
	m_path.clear();
	m_pathFindingTarget.clear();
	m_vecHeight.clear();
}

void CLevelGenNode1_1_2::GenRooms()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nBlockCountMin = 3;
	const int32 nBlockCountMax = 6;
	const int32 nBlockGroupSizeMin = 1;
	const int32 nBlockGroupSizeMax = 4;
	const int32 nEntranceSize = 8;
	const int32 nBarLenMin = 6;
	const int32 nBarLenMax = 12;
	const int32 fBarChance = 0.75f;
	enum
	{
		eOp_Blocks,
		eOp_StoneOrBar,
		eOp_Room,
	};

	uint8 nOp = eOp_StoneOrBar;
	m_vecHeight.resize( nWidth );
	memset( &m_vecHeight[0], -1, m_vecHeight.size() * sizeof( int32 ) );
	int32 nEntranceBegin = SRand::Inst().Rand( 0, nWidth - nEntranceSize + 1 );
	for( int i = 0; i < nEntranceSize; i++ )
	{
		m_gendata[i + nEntranceBegin] = eType_Path;
		m_vecHeight[i + nEntranceBegin] = 0;
		m_pathFindingTarget.push_back( TVector2<int32>( i + nEntranceBegin, 0 ) );
	}
	int8 nRoomPosType;
	if( nEntranceBegin <= nWidth / 8 )
		nRoomPosType = 0;
	else if( nEntranceBegin + nEntranceSize >= nWidth - nWidth / 8 )
		nRoomPosType = 3;
	else if( SRand::Inst().Rand<float>( nEntranceBegin, nEntranceBegin + nEntranceSize ) < nWidth * 0.5f )
		nRoomPosType = 0;
	else
		nRoomPosType = 3;

	vector<int32> indexPool;
	indexPool.resize( nWidth );
	for( int i = 0; i < nWidth; i++ )
		indexPool[i] = i;

#define GET_LAST_TYPE( x ) ( m_vecHeight[x] < 0 ? eType_None : m_gendata[(x) + m_vecHeight[x] * nWidth] )

	bool bBreak = false;
	bool bFirstTime = true;
	while( !bBreak )
	{
		switch( nOp )
		{
		case eOp_Blocks:
		{
			nOp = eOp_Room;
			SRand::Inst().Shuffle( indexPool );
			int iPos = 0;
			int32 nBlockCount = SRand::Inst().Rand( nBlockCountMin, nBlockCountMax + 1 );
			while( nBlockCount )
			{
				int32 nGroupSize = Min( nBlockCount, SRand::Inst().Rand( nBlockGroupSizeMin, nBlockGroupSizeMax + 1 ) );
				nBlockCount -= nGroupSize;
				int32 nGroupWidth = SRand::Inst().Rand( nGroupSize - nGroupSize / 2, nGroupSize + 1 );

				int32 iBegin = -1;
				for( ; iPos < nWidth; iPos++ )
				{
					int32 i1 = indexPool[iPos];
					if( i1 + nGroupWidth > nWidth )
						continue;

					bool bSucceed = true;
					for( int i2 = 0; i2 < nGroupWidth; i2++ )
					{
						if( GET_LAST_TYPE( i2 + i1 ) == eType_Path )
						{
							bSucceed = false;
							break;
						}
					}
					if( bSucceed )
					{
						iBegin = i1;
						iPos++;
						break;
					}
				}
				if( iBegin < 0 )
					continue;

				int32 nGroupWidth1 = nGroupSize - nGroupWidth;
				int32 iBegin1 = iBegin + SRand::Inst().Rand( 0, nGroupWidth - nGroupWidth1 + 1 );
				for( int i = 0; i < nGroupWidth; i++ )
				{
					int32 x = i + iBegin;
					if( m_vecHeight[x] < nHeight - 1 )
					{
						m_gendata[x + nWidth * ++m_vecHeight[x]] = eType_Temp;
					}
				}
				for( int i = 0; i < nGroupWidth1; i++ )
				{
					int32 x = i + iBegin1;
					if( m_vecHeight[x] < nHeight - 1 )
					{
						m_gendata[x + nWidth * ++m_vecHeight[x]] = eType_Temp;
					}
				}
			}
			break;
		}
		case eOp_StoneOrBar:
		{
			nOp = eOp_Blocks;
			bool bStone = false;
			for( int k = 0; k < 2; k++ )
			{
				SRand::Inst().Shuffle( indexPool );
				bool bBar = bFirstTime || bStone ? true : SRand::Inst().Rand( 0.0f, 1.0f ) < fBarChance;
				if( bBar )
				{
					int32 nBarLen = SRand::Inst().Rand( nBarLenMin, nBarLenMax + 1 );
					int32 iBegin = -1;
					int32 nBarHeight = 0;
					for( int i = 0; i < nWidth; i++ )
					{
						int32 i1 = indexPool[i];
						if( i1 + nBarLen > nWidth )
							continue;

						nBarHeight = 0;
						bool b1 = false;
						bool b2 = false;
						for( int i2 = 0; i2 < nBarLen; i2++ )
						{
							int32 x = i2 + i1;
							int8 lastType = GET_LAST_TYPE( x );
							if( lastType == eType_Room )
								b1 = true;
							else if( lastType == eType_Path )
								b2 = true;
							nBarHeight = Max( nBarHeight, m_vecHeight[x] + 1 );
						}

						if( nBarHeight < nHeight && ( bFirstTime ? !b2 : b1 ) )
						{
							iBegin = i1;
							break;
						}
					}
					if( iBegin < 0 )
						bBar = false;
					else
					{
						for( int i = 0; i < nBarLen; i++ )
						{
							int32 x = i + iBegin;
							m_gendata[x + nBarHeight * nWidth] = eType_Bar;
							m_vecHeight[x] = nBarHeight;
						}
						m_bars.push_back( TRectangle<int32>( iBegin, nBarHeight, nBarLen, 1 ) );
					}
				}

				if( !bBar )
				{
					bStone = true;
					for( int i = 0; i < nWidth; i++ )
					{
						int32 x = indexPool[i];
						int32 y = m_vecHeight[x] + 1;
						if( y >= nHeight )
							continue;
						auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 2, 2 ),
							TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
						if( rect.width > 0 )
						{
							int32 y = rect.GetBottom() - 1;
							for( int i = 0; i < rect.width; i++ )
							{
								int32 x = i + rect.x;
								for( int j = 0; j < rect.height; j++ )
								{
									m_gendata[x + ( j + rect.y ) * nWidth] = eType_Stone;
								}
								m_vecHeight[x] = Max( m_vecHeight[x], y );
							}
							m_stones.push_back( rect );
							break;
						}
					}
				}
			}

			break;
		}
		case eOp_Room:
		{
			nOp = nRoomPosType == 4 ? eOp_Room : eOp_StoneOrBar;
			
			switch( nRoomPosType )
			{
			case 0:
				nRoomPosType = SRand::Inst().Rand( 0.0f, 1.0f ) < 0.25f ? 1 : 3;
				break;
			case 3:
				nRoomPosType = SRand::Inst().Rand( 0.0f, 1.0f ) < 0.25f ? 2 : 0;
				break;
			case 1:
				nRoomPosType = 3;
				break;
			case 2:
				nRoomPosType = 0;
				break;
			}

			int32 nGenBegin, nGenEnd;
			TVector2<int32> sizeMin, sizeMax;
			switch( nRoomPosType )
			{
			case 0:
				nGenBegin = 0;
				nGenEnd = nWidth * 3 / 8;
				sizeMin = TVector2<int32>( 6, 6 );
				sizeMax = TVector2<int32>( 8, 9 );
				break;
			case 3:
				nGenBegin = nWidth - nWidth * 3 / 8;
				nGenEnd = nWidth;
				sizeMin = TVector2<int32>( 6, 6 );
				sizeMax = TVector2<int32>( 8, 9 );
				break;
			case 1:
			case 2:
				nGenBegin = nWidth * 3 / 8;
				nGenEnd = nWidth - nWidth * 3 / 8;
				sizeMin = TVector2<int32>( 6, 6 );
				sizeMax = TVector2<int32>( 7, 7 );
				break;
			case 4:
				nGenBegin = 0;
				nGenEnd = nWidth;
				sizeMin = TVector2<int32>( 6, 6 );
				sizeMax = TVector2<int32>( 8, 8 );
				break;
			}

			SRand::Inst().Shuffle( indexPool );
			bool bCreated = false;
			for( int i = 0; i < nWidth; i++ )
			{
				int32 x = indexPool[i];
				int32 y = m_vecHeight[x] + 1;
				if( x < nGenBegin || x >= nGenEnd || y + 6 > nHeight )
					continue;
				if( m_rooms.size() && nRoomPosType < 4 )
					y = Min( nHeight - 6, Max( y, m_rooms.back().rect.GetBottom() - 6 ) );
				auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), sizeMin, sizeMax, TRectangle<int32>( nGenBegin, y, nGenEnd - nGenBegin, nHeight - y ), -1, eType_Room );
				if( rect.width > 0 )
				{
					SRoom room;
					room.nType = rect.width + rect.height >= 15 ? 1 : 0;
					room.rect = rect;
					m_rooms.push_back( room );

					if( !bFirstTime )
					{
						for( int i = 0; i < nWidth; i++ )
						{
							if( i >= rect.x && i < rect.GetRight() )
								continue;

							int8 nLastType = GET_LAST_TYPE( i );
							if( nLastType == eType_None || nLastType == eType_Path )
							{
								int32 h = SRand::Inst().Rand( 1, 4 );
								for( int k = 0; k < h && m_vecHeight[i] < nHeight - 1; k++ )
									m_gendata[i + nWidth * ++m_vecHeight[i]] = eType_Temp;
							}
						}

						for( auto p : m_pathFindingTarget )
						{
							m_gendata[p.x + p.y * nWidth] = eType_Path;
						}
					}

					LinkRoom( nRoomPosType );
					bCreated = true;
					break;
				}
			}

			bFirstTime = false;
			if( !bCreated )
			{
				if( nRoomPosType < 4 )
					nRoomPosType = 4;
				else
					bBreak = true;
			}
			break;
		}
		default:
			break;
		}
	}

	for( auto p : m_pathFindingTarget )
	{
		m_gendata[p.x + p.y * nWidth] = eType_Path;
	}
	for( int i = 0; i < m_gendata.size(); i++ )
	{
		if( m_gendata[i] == eType_Temp )
			m_gendata[i] = eType_None;
	}

	vector<TVector2<int32> > q;
	for( int i = 0; i < nWidth; i++ )
	{
		if( m_gendata[i + ( nHeight - 1 ) * nWidth] == eType_None )
		{
			m_gendata[i + ( nHeight - 1 ) * nWidth] = eType_Path;
			q.push_back( TVector2<int32>( i, nHeight - 1 ) );
		}
	}
	SRand::Inst().Shuffle( q );
	FloodFillExpand( m_gendata, nWidth, nHeight, eType_Path, eType_None, nWidth * nHeight * 0.1f, q );
}

void CLevelGenNode1_1_2::GenRooms1()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int8> vecTemp;
	vecTemp.resize( m_gendata.size() );
	for( int i = 0; i < m_gendata.size(); i++ )
		vecTemp[i] = m_gendata[i] == eType_Path ? 1 : ( m_gendata[i] == eType_None ? 0 : 2 );
	ExpandDist( vecTemp, nWidth, nHeight, 1, 0, 3 );
	vector<TVector2<int32> > vecEmpty;
	for( int i = 0; i < m_gendata.size(); i++ )
	{
		if( m_gendata[i] == eType_None )
		{
			if( vecTemp[i] == 1 )
				m_gendata[i] = eType_Temp;
			else
				vecEmpty.push_back( TVector2<int32>( i % nWidth, i / nWidth ) );
		}
	}
	SRand::Inst().Shuffle( vecEmpty );
	
	int32 nCount = 2;
	for( auto p : vecEmpty )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;

		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 5, 5 ), TVector2<int32>( 8, 8 ), TRectangle<int32>( 0, 3, nWidth, nHeight - 6 ), -1, eType_Room );
		if( rect.width > 0 )
		{
			SRoom room;
			room.nType = 1;
			room.rect = rect;
			m_rooms.push_back( room );

			LinkRoom( 5 );

			nCount--;
			if( !nCount )
				break;
		}
	}

	for( int i = 0; i < m_gendata.size(); i++ )
	{
		if( m_gendata[i] == eType_Temp )
			m_gendata[i] = eType_None;
	}
}

void CLevelGenNode1_1_2::LinkRoom( int8 nRoomPosType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nPathSizeMin = 4;
	const int32 nPathSizeMax = 8;
	auto& room = m_rooms.back();
	auto& rect = room.rect;

	vector<TVector2<int32> > q;
	//left
	if( rect.x > 0 )
	{
		int32 nCurLen = 0;
		float fMaxLen = 0;
		int32 nMaxPos = -1;
		int32 x = rect.x - 1;
		for( int32 y = rect.y + 1; y < rect.GetBottom() - 1; y++ )
		{
			if( m_gendata[x + y * nWidth] == eType_None )
			{
				nCurLen = Min( nCurLen + 1, 3 );
				float fCurLen = nCurLen + SRand::Inst().Rand( 0.0f, 1.0f );
				if( fCurLen > fMaxLen )
				{
					fMaxLen = fCurLen;
					nMaxPos = y - nCurLen + 1;
				}
			}
			else
				nCurLen = 0;
		}

		if( nMaxPos >= 0 )
		{
			int32 nMaxLen = floor( fMaxLen );
			int32 y = SRand::Inst().Rand( nMaxPos, nMaxPos + nMaxLen );
			FloodFill( m_gendata, nWidth, nHeight, x, y, eType_Temp1, SRand::Inst().Rand( nPathSizeMin, nPathSizeMax + 1 ), q );

			bool bLeft = y > rect.y + 1;
			bool bRight = y < rect.GetBottom() - 2;
			if( room.nType == 0 )
			{
				if( y == rect.y + 2 )
				{
					bLeft = true;
					bRight = false;
				}
				if( y == rect.GetBottom() - 3 )
				{
					bLeft = false;
					bRight = true;
				}
			}
			if( bLeft && bRight )
				( SRand::Inst().Rand( 0, 2 ) ? bLeft : bRight ) = false;
			m_gendata[x + 1 + y * nWidth] = eType_Door;
			if( bLeft )
				m_gendata[x + 1 + ( y - 1 ) * nWidth] = eType_Door;
			else
				m_gendata[x + 1 + ( y + 1 ) * nWidth] = eType_Door;
		}
	}
	//right
	if( rect.GetRight() < nWidth )
	{
		int32 nCurLen = 0;
		float fMaxLen = 0;
		int32 nMaxPos = -1;
		int32 x = rect.GetRight();
		for( int32 y = rect.y + 1; y < rect.GetBottom() - 1; y++ )
		{
			if( m_gendata[x + y * nWidth] == eType_None )
			{
				nCurLen = Min( nCurLen + 1, 3 );
				float fCurLen = nCurLen + SRand::Inst().Rand( 0.0f, 1.0f );
				if( fCurLen > fMaxLen )
				{
					fMaxLen = fCurLen;
					nMaxPos = y - nCurLen + 1;
				}
			}
			else
				nCurLen = 0;
		}

		if( nMaxPos >= 0 )
		{
			int32 nMaxLen = floor( fMaxLen );
			int32 y = SRand::Inst().Rand( nMaxPos, nMaxPos + nMaxLen );
			FloodFill( m_gendata, nWidth, nHeight, x, y, eType_Temp1, SRand::Inst().Rand( nPathSizeMin, nPathSizeMax + 1 ), q );

			bool bLeft = y > rect.y + 1;
			bool bRight = y < rect.GetBottom() - 2;
			if( room.nType == 0 )
			{
				if( y == rect.y + 2 )
				{
					bLeft = true;
					bRight = false;
				}
				if( y == rect.GetBottom() - 3 )
				{
					bLeft = false;
					bRight = true;
				}
			}
			if( bLeft && bRight )
				( SRand::Inst().Rand( 0, 2 ) ? bLeft : bRight ) = false;
			m_gendata[x - 1 + y * nWidth] = eType_Door;
			if( bLeft )
				m_gendata[x - 1 + ( y - 1 ) * nWidth] = eType_Door;
			else
				m_gendata[x - 1 + ( y + 1 ) * nWidth] = eType_Door;
		}
	}

	//top
	bool bTop = true;
	if( rect.y > 0 )
	{
		if( nRoomPosType <= 4 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				if( GET_LAST_TYPE( i ) == eType_Path )
				{
					bTop = false;
					break;
				}
			}
		}

		if( bTop )
		{
			int32 nCurLen = 0;
			float fMaxLen = 0;
			int32 nMaxPos = -1;
			int32 y = rect.y - 1;
			for( int32 x = rect.x + 1; x < rect.GetRight() - 1; x++ )
			{
				if( m_gendata[x + y * nWidth] == eType_None )
				{
					nCurLen = Min( nCurLen + 1, 3 );
					float fCurLen = nCurLen + SRand::Inst().Rand( 0.0f, 1.0f );
					if( fCurLen > fMaxLen )
					{
						fMaxLen = fCurLen;
						nMaxPos = x - nCurLen + 1;
					}
				}
				else
					nCurLen = 0;
			}

			if( nMaxPos >= 0 )
			{
				int32 nMaxLen = floor( fMaxLen );
				int32 x = SRand::Inst().Rand( nMaxPos, nMaxPos + nMaxLen );
				FloodFill( m_gendata, nWidth, nHeight, x, y, eType_Temp1, SRand::Inst().Rand( nPathSizeMin, nPathSizeMax + 1 ), q );

				bool bLeft = x > rect.x + 1;
				bool bRight = x < rect.GetRight() - 2;
				if( bLeft && bRight )
					( SRand::Inst().Rand( 0, 2 ) ? bLeft : bRight ) = false;
				m_gendata[x + ( y + 1 ) * nWidth] = eType_Door;
				if( bLeft )
					m_gendata[x - 1 + ( y + 1 ) * nWidth] = eType_Door;
				else
					m_gendata[x + 1 + ( y + 1 ) * nWidth] = eType_Door;
			}
		}
	}
	//bottom
	if( rect.GetBottom() < nHeight - 1 && ( nRoomPosType != 1 && nRoomPosType != 2 || !bTop ) )
	{
		int32 nCurLen = 0;
		float fMaxLen = 0;
		int32 nMaxPos = -1;
		int32 y = rect.GetBottom();
		for( int32 x = rect.x + 1; x < rect.GetRight() - 1; x++ )
		{
			if( m_gendata[x + y * nWidth] == eType_None )
			{
				nCurLen = Min( nCurLen + 1, 3 );
				float fCurLen = nCurLen + SRand::Inst().Rand( 0.0f, 1.0f );
				if( fCurLen > fMaxLen )
				{
					fMaxLen = fCurLen;
					nMaxPos = x - nCurLen + 1;
				}
			}
			else
				nCurLen = 0;
		}

		if( nMaxPos >= 0 )
		{
			int32 nMaxLen = floor( fMaxLen );
			int32 x = SRand::Inst().Rand( nMaxPos, nMaxPos + nMaxLen );
			FloodFill( m_gendata, nWidth, nHeight, x, y, eType_Temp1, SRand::Inst().Rand( nPathSizeMin, nPathSizeMax + 1 ), q );

			bool bLeft = x > rect.x + 1;
			bool bRight = x < rect.GetRight() - 2;
			if( bLeft && bRight )
				( SRand::Inst().Rand( 0, 2 ) ? bLeft : bRight ) = false;
			m_gendata[x + ( y - 1 ) * nWidth] = eType_Door;
			if( bLeft )
				m_gendata[x - 1 + ( y - 1 ) * nWidth] = eType_Door;
			else
				m_gendata[x + 1 + ( y - 1 ) * nWidth] = eType_Door;
		}
	}

	TVector2<int32> dst = FindPath( m_gendata, nWidth, nHeight, eType_None, eType_Temp2, nRoomPosType == 5 ? eType_Path : eType_Temp1, m_pathFindingTarget, m_par );
	if( dst.x >= 0 )
	{
		vector<TVector2<int32> > q1;
		TVector2<int32> p = m_par[dst.x + dst.y * nWidth];
		while( p.x >= 0 && m_gendata[p.x + p.y * nWidth] == eType_Temp2 )
		{
			q1.push_back( p );
			p = m_par[p.x + p.y * nWidth];
		}

		ExpandDist( m_gendata, nWidth, nHeight, eType_Temp2, eType_None, nRoomPosType == 5 ? 1 : 2, q1 );
		for( auto p : q1 )
		{
			m_gendata[p.x + p.y * nWidth] = eType_Path;
			m_vecHeight[p.x] = Max( m_vecHeight[p.x], p.y );
			m_path.push_back( p );
		}
	}

	if( nRoomPosType <= 4 )
	{
		m_pathFindingTarget.clear();
		m_pathFindingTarget.resize( q.size() );
		for( int i = 0; i < q.size(); i++ )
		{
			auto p = q[i];
			m_pathFindingTarget[i] = p;
			m_vecHeight[p.x] = Max( m_vecHeight[p.x], p.y );
		}
		for( int x = rect.x; x < rect.GetRight(); x++ )
			m_vecHeight[x] = Max( m_vecHeight[x], rect.GetBottom() - 1 );
	}
	else
	{
		for( int i = 0; i < q.size(); i++ )
		{
			auto p = q[i];
			m_gendata[p.x + p.y * nWidth] = eType_Path;
		}
	}
}

void CLevelGenNode1_1_2::AddMoreBars()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 1; i < nWidth - 1; i++ )
		{
			auto& data = m_gendata[i + j * nWidth];
			if( data == eType_None && m_gendata[i - 1 + j * nWidth] == eType_Path && m_gendata[i + 1 + j * nWidth] == eType_Path )
				data = eType_Path;
		}
	}
	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 1; i < nWidth - 1; i++ )
		{
			auto& data = m_gendata[i + j * nWidth];
			if( data == eType_Path && m_gendata[i - 1 + j * nWidth] == eType_None && m_gendata[i + 1 + j * nWidth] == eType_None )
				data = eType_None;
		}
	}

	LvGenLib::AddBars( m_gendata, nWidth, nHeight, m_bars, eType_None, eType_Bar );
}

#undef GET_LAST_TYPE

void CLevelGenNode1_1_2::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	LvGenLib::GenObjs( m_gendata, nWidth, nHeight, 15, eType_Path, eType_Obj );
	LvGenLib::GenObjs1( m_gendata, nWidth, nHeight, eType_None, eType_Path, eType_Obj );
}

void CLevelGenNode1_1_2::GenBlocks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int8 nTypes[4] = { eType_Block1x, eType_Block1y, eType_Block2x, eType_Block2y };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_None, nTypes, 4 );
}

void CLevelGenNode1_1_2::GenBonus()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const float fBonusPercentMin = 0.018f;
	const float fBonusPercentMax = 0.02f;

	vector<float> vecRisk, vecRisk1;
	vecRisk.resize( nWidth * nHeight );
	vecRisk1.resize( nWidth * nHeight );
	vector<float> vecRiskOpacity;
	vecRiskOpacity.resize( nWidth * nHeight );

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			float& opacity = vecRiskOpacity[i + j * nWidth];
			int8 nType = m_gendata[i + j * nWidth];
			if( nType == eType_Path || nType == eType_Obj || nType == eType_Door )
				opacity = 0;
			else if( nType >= eType_Block1x && nType <= eType_Block1y )
				opacity = 0.5f;
			else
				opacity = 0.85f;
		}
	}

	for( auto& room : m_rooms )
	{
		auto rect = room.rect;
		for( int i = rect.x + 1; i < rect.GetRight() - 1; i++ )
		{
			for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
			{
				vecRiskOpacity[i + j * nWidth] = 0.15f;
			}
		}

		rect.y -= 2;
		rect.height = 2;
		rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );
		for( int i = rect.x + 1; i < rect.GetRight() - 1; i++ )
		{
			for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
			{
				vecRiskOpacity[i + j * nWidth] = Max( vecRiskOpacity[i + j * nWidth], 0.25f );
			}
		}
	}

	for( auto bar : m_bars )
	{
		bar.y -= bar.height;
		bar = bar * TRectangle<int32>( 0, 0, nWidth, nHeight );
		for( int i = bar.x; i < bar.GetRight(); i++ )
		{
			for( int j = bar.y; j < bar.GetBottom(); j++ )
			{
				vecRiskOpacity[i + j * nWidth] = Max( vecRiskOpacity[i + j * nWidth], 0.25f );
			}
		}
	}
	for( auto stone : m_stones )
	{
		stone.height = ( stone.height + 1 ) / 2;
		stone.y -= stone.height;
		stone = stone * TRectangle<int32>( 0, 0, nWidth, nHeight );
		for( int i = stone.x; i < stone.GetRight(); i++ )
		{
			for( int j = stone.y; j < stone.GetBottom(); j++ )
			{
				vecRiskOpacity[i + j * nWidth] = Max( vecRiskOpacity[i + j * nWidth], 0.25f );
			}
		}
	}

	vector<int8> vecLightMap;
	vecLightMap.resize( nWidth * nHeight );
	for( int i = 0; i < m_gendata.size(); i++ )
	{
		vecLightMap[i] = m_gendata[i] == eType_Path ? 0 : 1;
	}

	vector<TVector2<int32> > vecLightArea;
	vector<int32> vecDist;
	vecDist.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth; i++ )
	{
		if( m_gendata[i + ( nHeight - 1 ) * nWidth] )
			vecLightArea.push_back( TVector2<int32>( i, nHeight - 1 ) );
	}
	for( auto p : m_path )
	{
		if( p.y < nHeight - 1 )
		vecLightArea.push_back( p );
	}
	int32 iq = 0;

	vector<TVector2<int32> > vecCoords;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			vecCoords.push_back( TVector2<int32>( i, j ) );
		}
	}
	TVector2<int32> ofs[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
	
	const int32 nIteration = 8;
	for( int iIteration = 0; iIteration < nIteration; iIteration++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				int8 nType = m_gendata[i + j * nWidth];
				if( nType >= eType_Block1x && nType <= eType_Block1y )
					vecRisk[i + j * nWidth] += 0.25f;
				else if( nType == eType_Obj )
				{
					vecRisk[i + j * nWidth] += 5.0f;
				}
			}
		}
		for( auto& room : m_rooms )
		{
			auto rect = room.rect;
			rect.y -= 2;
			rect.height = 2;
			rect = rect * TRectangle<int32>( 0, 0, nWidth, nHeight );
			for( int i = rect.x + 1; i < rect.GetRight() - 1; i++ )
			{
				for( int j = rect.y + 1; j < rect.GetBottom() - 1; j++ )
				{
					vecRisk[i + j * nWidth] += 2.0f;
				}
			}
		}
		for( auto bar : m_bars )
		{
			bar.y -= bar.height;
			bar = bar * TRectangle<int32>( 0, 0, nWidth, nHeight );
			for( int i = bar.x; i < bar.GetRight(); i++ )
			{
				for( int j = bar.y; j < bar.GetBottom(); j++ )
				{
					vecRisk[i + j * nWidth] += 1.0f;
				}
			}
		}
		for( auto stone : m_stones )
		{
			stone.height = ( stone.height + 1 ) / 2;
			stone.y -= stone.height;
			stone = stone * TRectangle<int32>( 0, 0, nWidth, nHeight );
			for( int i = stone.x; i < stone.GetRight(); i++ )
			{
				for( int j = stone.y; j < stone.GetBottom(); j++ )
				{
					vecRisk[i + j * nWidth] += 1.0f;
				}
			}
		}

		SRand::Inst().Shuffle( vecCoords );
		for( auto& p : vecCoords )
		{
			float s = 0;
			int32 n = 1;
			for( int i = 0; i < 4; i++ )
			{
				auto& p1 = p + ofs[i];
				if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight )
				{
					s += 1.0f - vecRiskOpacity[p1.x + p1.y * nWidth];
					n++;
				}
			}

			float s0 = 1.0f - vecRiskOpacity[p.x + p.y * nWidth];
			float f = ( 1.0f - vecRiskOpacity[p.x + p.y * nWidth] ) / n * vecRisk[p.x + p.y * nWidth];
			vecRisk1[p.x + p.y * nWidth] += vecRisk[p.x + p.y * nWidth] - f * s;
			for( int i = 0; i < 4; i++ )
			{
				auto& p1 = p + ofs[i];
				if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight )
				{
					float s1 = 1.0f - vecRiskOpacity[p1.x + p1.y * nWidth];
					vecRisk1[p1.x + p1.y * nWidth] += f * s1;
				}
			}
		}

		for( int i = 0; i < vecRisk.size(); i++ )
		{
			vecRisk[i] = vecRisk1[i] * 0.5f;
			vecRisk1[i] = 0;
		}

		int32 nMaxDist = iIteration + 1;
		StepExpandDist( vecLightMap, nWidth, nHeight, 0, 1, nMaxDist, vecLightArea, vecDist, iq );
		for( int i = 0; i < iq; i++ )
		{
			auto p = vecLightArea[i];
			int32 dist = vecDist[p.x + p.y * nWidth];
			float f = ( nMaxDist - dist ) * 5.0f / nMaxDist;
			vecRisk[p.x + p.y * nWidth] = Max( vecRisk[p.x + p.y * nWidth] - f, 0.0f );
		}
	}

	vector<TVector2<int32> > vecEmpty;
	for( int i = 0; i < m_gendata.size(); i++ )
	{
		vecLightMap[i] = m_gendata[i] == eType_Path ? 0 : 1;
	}
	FindAllOfTypesInMap( vecLightMap, nWidth, nHeight, 0, vecEmpty );
	sort( vecEmpty.begin(), vecEmpty.end(), [&vecRisk, nWidth] ( const TVector2<int32> & left, const TVector2<int32> & right ) {
		return vecRisk[left.x + left.y * nWidth] > vecRisk[right.x + right.y * nWidth];
	} );

	int32 nCount = nWidth * nHeight * SRand::Inst().Rand( fBonusPercentMin, fBonusPercentMax );
	for( auto& p : vecEmpty )
	{
		if( !nCount )
			break;

		if( SRand::Inst().Rand( 0, 2 ) )
		{
			m_gendata[p.x + p.y * nWidth] = eType_Bonus;
			nCount--;
		}
	}
	LvGenLib::GenObjs2( m_gendata, nWidth, nHeight, eType_Path, eType_Web, 0.25f );
	int8 nObjTypes[] = { eType_Bonus, eType_Obj };
	LvGenLib::DropObjs( m_gendata, nWidth, nHeight, eType_Path, nObjTypes, 2 );
}

void CLevelGenNode1_1_3::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1xNode = CreateNode( pXml->FirstChildElement( "block1x" )->FirstChildElement(), context );
	m_pBlock1yNode = CreateNode( pXml->FirstChildElement( "block1y" )->FirstChildElement(), context );
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBlock2yNode = CreateNode( pXml->FirstChildElement( "block2y" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pRoom1Node = CreateNode( pXml->FirstChildElement( "room1" )->FirstChildElement(), context );
	m_pRoom2Node = CreateNode( pXml->FirstChildElement( "room2" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );
	m_pBonusNode = CreateNode( pXml->FirstChildElement( "bonus" )->FirstChildElement(), context );
	m_pWebNode = CreateNode( pXml->FirstChildElement( "web" )->FirstChildElement(), context );

	auto pShop = pXml->FirstChildElement( "shop" );
	if( pShop )
		m_pShopNode = CreateNode( pShop->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_1_3::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );

	GenAreas();
	GenObstacles();
	GenShops();

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
	context.mapTags["mask"] = eType_Block1y;
	m_pBlock1yNode->Generate( context, region );
	context.mapTags["mask"] = eType_Block2x;
	m_pBlock2xNode->Generate( context, region );
	context.mapTags["mask"] = eType_Block2y;
	m_pBlock2yNode->Generate( context, region );
	context.mapTags["mask"] = eType_Web;
	m_pWebNode->Generate( context, region );
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
	}
	if( m_nShop >= 0 )
	{
		auto& room = m_rooms[m_nShop];
		TRectangle<int32> rect = room.rect;
		rect.x = SRand::Inst().Rand( 1, rect.width - 6 ) + rect.x;
		rect.y = SRand::Inst().Rand( 1, rect.height - 4 ) + rect.y;
		rect.width = 6;
		rect.height = 4;
		m_pShopNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_gendata1.clear();
	m_areas.clear();
	m_stones.clear();
	m_bars.clear();
	m_rooms.clear();
	m_wallChunks.clear();
}

void CLevelGenNode1_1_3::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const TVector2<int32> sizeMin = TVector2<int32>( 10, 7 );
	const float fChanceXCoefMin = 0.8f;
	const float fChanceXCoefMax = 0.85f;
	const float fChanceYCoefMin = 1.35f;
	const float fChanceYCoefMax = 1.4f;

	vector<int32> vecLeft, vecRight;
	vecLeft.resize( nHeight );
	vecRight.resize( nHeight );

	for( int i = 0; i < m_gendata.size(); i++ )
		m_gendata[i] = eType_Temp;
	for( ;; )
	{
		bool bLeft = SRand::Inst().Rand( 0, 2 );
		auto& vec = bLeft ? vecLeft : vecRight;
		auto& vec1 = !bLeft ? vecLeft : vecRight;

		int32 nMin = nWidth;
		int32 nMax = -1;
		for( int i = 0; i < nHeight; i++ )
		{
			nMin = Min( nMin, vec[i] );
			nMax = Max( nMax, vec[i] );
		}

		bool bSucceed = false;
		for( int k = 0; k < 2; k++ )
		{
			for( int32 y = 0; y < nHeight; y++ )
			{
				if( k == 0 ? vec[y] <= nMin + 2 : vec[y] > nMin + 2 )
				{
					int32 x = vec[y];
					if( nWidth - x < sizeMin.x )
						continue;
					if( m_gendata[( bLeft ? x : nWidth - x - 1 ) + y * nWidth] == eType_None )
						continue;
					float fScale = SRand::Inst().Rand( 1.0f, 1.2f );
					TVector2<int32> sizeMax( sizeMin.x * fScale, sizeMin.y * fScale );
					auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( bLeft ? x : nWidth - x - 1, y ), sizeMin,
						sizeMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp );
					if( rect.width == 0 )
						continue;

					int32 xMin = bLeft ? rect.x : nWidth - rect.GetRight();
					int32 xMax = bLeft ? rect.GetRight() : nWidth - rect.x;
					int32 xMax1 = -1;
					for( int y1 = rect.y; y1 < rect.GetBottom(); y1++ )
						xMax1 = Max( xMax1, vec1[y1] );
					int32 spaceLeft = nWidth - xMax - xMax1;
					if( spaceLeft && spaceLeft < sizeMin.x )
					{
						int32 spaceLeft1 = nWidth - xMax1 - xMin;
						if( spaceLeft1 < sizeMin.x * 2 )
							xMax = nWidth - xMax1;
						else
							xMax = spaceLeft1 / 2;

						if( bLeft )
							rect.SetRight( xMax );
						else
							rect.SetLeft( nWidth - xMax );
					}

					for( int i = rect.x; i < rect.GetRight(); i++ )
					{
						for( int j = rect.y; j < rect.GetBottom(); j++ )
						{
							m_gendata[i + j * nWidth] = eType_None;
						}
					}

					int32 nMaxDif = 0, nMaxDif1 = 0;
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						nMaxDif = Max( nMaxDif, xMax - vec[j] );
						nMaxDif1 = Max( nMaxDif1, nWidth - xMin - vec1[j] );
					}
					if( nMaxDif <= nMaxDif1 )
					{
						for( int j = rect.y; j < rect.GetBottom(); j++ )
						{
							vec[j] = Max( vec[j], xMax );
						}
					}
					else
					{
						for( int j = rect.y; j < rect.GetBottom(); j++ )
						{
							vec1[j] = Max( vec1[j], nWidth - xMin );
						}
					}

					SArea area;
					area.rect = rect;
					area.nType = 0;
					m_areas.push_back( area );
					bSucceed = true;
					break;
				}
			}

			if( bSucceed )
				break;
		}

		if( !bSucceed )
			break;
	}

	vector<TVector2<int32> > temp;
	m_gendata1.resize( nWidth * nHeight );
	int32 nEntranceWidth = 12;
	int32 nEntranceBegin = SRand::Inst().Rand( 0, nWidth + 1 - nEntranceWidth );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp )
				temp.push_back( TVector2<int32>( i, j ) );
			if( j == 0 )
				m_gendata1[i + j * nWidth] = i >= nEntranceBegin && i < nEntranceBegin + nEntranceWidth ? -100 : 1;
			else
				m_gendata1[i + j * nWidth] = i == 0 || i == nWidth - 1 || j == nHeight - 1 ? 1 : 0;
		}
	}
	if( temp.size() )
		SRand::Inst().Shuffle( temp );
	for( auto p : temp )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 6, 6 ), TVector2<int32>( 12, 12 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_None );
		if( rect.width > 0 )
		{
			SArea area;
			area.rect = rect;
			area.nType = 0;
			m_areas.push_back( area );
		}
	}

	float fChanceXCoef1 = SRand::Inst().Rand( fChanceXCoefMin, fChanceXCoefMax );
	float fChanceXCoef2 = SRand::Inst().Rand( fChanceXCoefMin, fChanceXCoefMax );
	float fChanceYCoef1 = SRand::Inst().Rand( fChanceYCoefMin, fChanceYCoefMax );
	float fChanceYCoef2 = SRand::Inst().Rand( fChanceYCoefMin, fChanceYCoefMax );
	SRand::Inst().Shuffle( m_areas );
	int32 nWallChunksLim = SRand::Inst().Rand( 2, 4 );

	for( auto& area : m_areas )
	{
		float xCenter = area.rect.x + area.rect.width * 0.5f;
		float yCenter = area.rect.y + area.rect.height * 0.5f;
		float fXCoef = xCenter * 2 < nWidth ? fChanceXCoef1 : fChanceXCoef2;
		float fYCoef = xCenter * 2 < nWidth ? fChanceYCoef1 : fChanceYCoef2;
		float x = Min( xCenter, nWidth - xCenter ) * 2 / nWidth * fXCoef;
		float y = yCenter / nHeight * fYCoef;

		if( x * x + y * y <= 1 )
			area.nType = 0;
		else if( ( nHeight - yCenter ) * 2 >= nHeight )
			area.nType = 1;
		else
		{
			if( nWallChunksLim )
			{
				area.nType = 2;
				nWallChunksLim--;
			}
			else
				area.nType = 1;
		}

		if( area.nType == 0 )
		{
			const auto& rect = area.rect;

			int8 nRoomType = SRand::Inst().Rand( 0.0f, 1.0f ) < ( area.rect.height - 6 ) * 0.5f;
			TRectangle<int32> rect0;
			bool bLowerDoor = false, bUpperDoor = false;
			if( nRoomType == 0 )
			{
				rect0.width = Max( 6, Min( rect.width - 3, SRand::Inst().Rand( 6, 9 ) ) );
				rect0.height = Max( 6, Min( rect.height - 2, SRand::Inst().Rand( 6, 8 ) ) );
				rect0.x = rect.x + ( rect.width - rect0.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
				rect0.y = rect.y + ( rect.height - rect0.height + SRand::Inst().Rand( 0, 2 ) ) / 2;
			}
			else
			{
				bLowerDoor = rect.y < sizeMin.y || SRand::Inst().Rand( 0.0f, 1.0f ) < 0.5f;
				bUpperDoor = SRand::Inst().Rand( 0.0f, 1.0f ) < 0.5f;
				rect0.width = Max( 8, Min( rect.width - 2, SRand::Inst().Rand( 8, 10 ) ) );
				int32 n = bLowerDoor ? 0 : 1;
				n += bUpperDoor ? 0 : 1;
				n = Min( rect.height - 7, n );
				rect0.height = Min( rect.height - n, SRand::Inst().Rand( 7, 10 ) );
				rect0.x = rect.x + SRand::Inst().Rand( 0, rect.width - rect0.width + 1 );
				rect0.y = rect.y + SRand::Inst().Rand( 0, rect.height - rect0.height + 1 );
			}

			auto rect1 = rect;
			rect1.x = rect.x * 2 - rect0.x;
			rect1.y = rect.y * 2 - rect0.y;
			rect1.width = rect.width * 2 - rect0.width;
			rect1.height = rect.height * 2 - rect0.height;
			rect1 = rect1 * TRectangle<int32>( 0, 0, nWidth, nHeight );

			SRoom room;
			room.nType = nRoomType;
			room.rect = rect0;
			m_rooms.push_back( room );
			for( int i = rect0.x; i < rect0.GetRight(); i++ )
			{
				for( int j = rect0.y; j < rect0.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_Room;
				}
			}

			{
				int32 nDoor = room.nType == 0 ? rect0.y + 1 : rect0.y + 1 + ( rect0.height - 3 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				m_gendata[rect0.x + nDoor * nWidth] = m_gendata[rect0.x + ( nDoor + 1 ) * nWidth] = eType_Door;
				nDoor = room.nType == 0 ? rect0.y + 1 : rect0.y + 1 + ( rect0.height - 3 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				m_gendata[rect0.GetRight() - 1 + nDoor * nWidth] = m_gendata[rect0.GetRight() - 1 + ( nDoor + 1 ) * nWidth] = eType_Door;
			}
			if( bLowerDoor )
			{
				int32 nDoor = rect0.x + 1 + ( rect0.width - 3 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				m_gendata[nDoor + rect0.y * nWidth] = m_gendata[nDoor + 1 + rect0.y * nWidth] = eType_Door;

				for( int i = rect1.x; i < rect0.x; i++ )
				{
					for( int j = rect1.y; j < rect0.y; j++ )
					{
						m_gendata1[i + j * nWidth]++;
					}
				}
				for( int i = rect0.GetRight(); i < rect1.GetRight(); i++ )
				{
					for( int j = rect1.y; j < rect0.y; j++ )
					{
						m_gendata1[i + j * nWidth]++;
					}
				}
			}
			else
			{
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
				{
					for( int j = rect1.y; j < rect0.y; j++ )
					{
						m_gendata1[i + j * nWidth]++;
					}
				}
			}
			if( bUpperDoor )
			{
				int32 nDoor = rect0.x + 1 + ( rect0.width - 3 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				m_gendata[nDoor + ( rect0.GetBottom() - 1 ) * nWidth] = m_gendata[nDoor + 1 + ( rect0.GetBottom() - 1 ) * nWidth] = eType_Door;

				for( int i = rect1.x; i < rect0.x; i++ )
				{
					for( int j = rect0.GetBottom(); j < rect1.GetBottom(); j++ )
					{
						m_gendata1[i + j * nWidth]++;
					}
				}
				for( int i = rect0.GetRight(); i < rect1.GetRight(); i++ )
				{
					for( int j = rect0.GetBottom(); j < rect1.GetBottom(); j++ )
					{
						m_gendata1[i + j * nWidth]++;
					}
				}
			}
			else
			{
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
				{
					for( int j = rect0.GetBottom(); j < rect1.GetBottom(); j++ )
					{
						m_gendata1[i + j * nWidth]++;
					}
				}
			}
		}
		else if( area.nType == 2 )
		{
			m_wallChunks.push_back( area.rect );
			for( int i = area.rect.x; i < area.rect.GetRight(); i++ )
			{
				for( int j = area.rect.y; j < area.rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_WallChunk;
				}
			}
		}
		else
		{
			for( int i = area.rect.x; i < area.rect.GetRight(); i++ )
			{
				int n = Min( i - area.rect.x, area.rect.GetRight() - 1 - i );
				float p = n * 2.0f / area.rect.width;
				float p1 = n * 1.5f / area.rect.width;
				int32 yMin = area.rect.y < sizeMin.y ? 0 : Max( 0, area.rect.height / 2 - n );
				int32 yMax = area.rect.y < sizeMin.y ? Min( area.rect.height / 2, n ) : area.rect.height - 1 - yMin;
				int32 y = area.rect.y + SRand::Inst().Rand( yMin, yMax + 1 );
				if( SRand::Inst().Rand( 0.0f, 1.0f ) < p )
				{
					if( SRand::Inst().Rand( 0.0f, 1.0f ) < p1 )
						FloodFill( m_gendata, nWidth, nHeight, i, y, eType_Obj, SRand::Inst().Rand( 4, 8 ) );
					else
						m_gendata1[i + y * nWidth] = 100;
				}
			}
		}
	}
}

void CLevelGenNode1_1_3::GenObstacles()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 n2Min = 16;
	const int32 n2Max = 24;

	for( int i = 0; i < m_gendata1.size(); i++ )
	{
		if( m_gendata1[i] >= 3 )
			m_gendata1[i] = 2;
		else if( m_gendata1[i] >= 1 )
			m_gendata1[i] = 1;
		else
			m_gendata1[i] = 0;
	}

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	SRand::Inst().Shuffle( vec );
	for( auto p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;
		int8 nType1 = m_gendata1[p.x + p.y * nWidth];
		if( !nType1 )
			continue;

		if( nType1 == 2 )
		{
			FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, eType_Temp, SRand::Inst().Rand( n2Min, n2Max + 1 ) );
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ),
				TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Stone );
			if( rect.width )
				m_stones.push_back( rect );
		}
		else
			m_gendata[p.x + p.y * nWidth] = eType_Temp;
	}
	//ExpandDist( m_gendata, nWidth, nHeight, eType_None, eType_Temp, 1 );
	LvGenLib::AddBars( m_gendata, nWidth, nHeight, m_bars, eType_Temp, eType_Bar );

	LvGenLib::GenObjs1( m_gendata, nWidth, nHeight, eType_Temp, eType_None, eType_Obj );
	LvGenLib::GenObjs2( m_gendata, nWidth, nHeight, eType_None, eType_Web, 0.5f );
	LvGenLib::DropObjs( m_gendata, nWidth, nHeight, eType_None, eType_Obj );
	LvGenLib::Flatten( m_gendata, nWidth, nHeight, eType_None, eType_Obj, eType_Bonus );

	int8 nTypes[4] = { eType_Block1x, eType_Block1y, eType_Block2x, eType_Block2y };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_Temp, nTypes, 4 );
}

void CLevelGenNode1_1_3::GenShops()
{
	m_nShop = -1;
	if( m_pShopNode && m_region.height >= 12 )
	{
		SRand::Inst().Shuffle( m_rooms );
		for( int i = 0; i < m_rooms.size(); i++ )
		{
			auto& room = m_rooms[i];
			if( room.rect.width >= 8 && room.rect.height >= 6 )
			{
				m_nShop = i;
				break;
			}
		}
	}
}
