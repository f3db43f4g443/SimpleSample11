#include "stdafx.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

void CLevelGenNode2_1_0::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pRoadNode1 = CreateNode( pXml->FirstChildElement( "road1" )->FirstChildElement(), context );
	m_pBlockNode = CreateNode( pXml->FirstChildElement( "block" )->FirstChildElement(), context );
	m_pBlock1Node = CreateNode( pXml->FirstChildElement( "block1" )->FirstChildElement(), context );
	m_pBlock2Node = CreateNode( pXml->FirstChildElement( "block2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pCargo2Node = CreateNode( pXml->FirstChildElement( "cargo2" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_1_0::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );

	GenRoad();
	GenBase();
	GenChunks();

	for( auto& road : m_vecRoads )
	{
		if( road.nType == 0 )
			m_pRoadNode->Generate( context, road.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pRoadNode1->Generate( context, road.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
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

	context.mapTags["mask"] = eType_Block;
	m_pBlockNode->Generate( context, region );
	context.mapTags["mask"] = eType_Block1;
	m_pBlock1Node->Generate( context, region );
	context.mapTags["mask"] = eType_Block2;
	m_pBlock2Node->Generate( context, region );
	context.mapTags["mask"] = eType_Obj;
	m_pObjNode->Generate( context, region );
	for( auto& rect : m_vecCargoSmall )
	{
		if( !rect.width )
			continue;
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecCargoLarge )
	{
		m_pCargo2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_gendata1.clear();
	m_vecRoads.clear();
	m_vecCargoSmall.clear();
	m_vecCargoLarge.clear();
}

void CLevelGenNode2_1_0::GenRoad()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	int32 w = SRand::Inst().Rand( 9, 12 );
	int32 h = SRand::Inst().Rand( nHeight / 2 - 4, nHeight / 2 + 1 );
	int32 l = SRand::Inst().Rand( ( nWidth - w ) / 2 - 2, nWidth - w - ( nWidth - w ) / 2 + 3 );
	int32 h0 = SRand::Inst().Rand( 2, 4 );
	int32 h1 = SRand::Inst().Rand( 2, 4 );
	int32 h2 = SRand::Inst().Rand( 2, 4 );
	int32 h3 = SRand::Inst().Rand( 2, 4 );
	int32 w1 = SRand::Inst().Rand( 2, 4 );
	int32 w2 = SRand::Inst().Rand( 2, 4 );
	int32 w3 = SRand::Inst().Rand( 2, 6 );
	int32 w4 = SRand::Inst().Rand( 2, 6 );
	int32 w5 = SRand::Inst().Rand( 2, 6 );
	int32 w6 = SRand::Inst().Rand( 2, 6 );

	GenSubArea( TRectangle<int32>( 0, h0, l, h - h0 ) );
	GenSubArea( TRectangle<int32>( l + w, h0, nWidth - l - w, h - h0 ) );
	GenSubArea( TRectangle<int32>( 0, h + h1, l + 1, nHeight - h - h1 - h3 ) );
	GenSubArea( TRectangle<int32>( l + w - 1, h + h2, nWidth - l - w + 1, nHeight - h - h2 - h3 ) );

	m_vecRoads.push_back( SRoad( TRectangle<int32>( 4, 0, nWidth - 8, h0 ), 0 ) );
	m_vecRoads.push_back( SRoad( TRectangle<int32>( l, h0, w1, h - h0 ), 0 ) );
	m_vecRoads.push_back( SRoad( TRectangle<int32>( l + w - w2, h0, w2, h - h0 ), 0 ) );
	m_road1 = TRectangle<int32>( l + 1, h, w - 2, nHeight - h - h3 );
	m_vecRoads.push_back( SRoad( m_road1, 1 ) );
	m_vecRoads.push_back( SRoad( TRectangle<int32>( w3, h, l + 1 - w3, h1 ), 0 ) );
	m_vecRoads.push_back( SRoad( TRectangle<int32>( l + w - 1, h, nWidth - l - w + 1 - w4, h2 ), 0 ) );
	m_vecRoads.push_back( SRoad( TRectangle<int32>( w5, nHeight - h3, nWidth - w5 - w6, h3 ), 1 ) );

	for( auto& road : m_vecRoads )
	{
		for( int i = road.rect.x; i < road.rect.GetRight(); i++ )
		{
			for( int j = road.rect.y; j < road.rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}
}

void CLevelGenNode2_1_0::GenSubArea( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 nMinAreaSize = 4;
	vector<TRectangle<int32> > rects;
	rects.push_back( rect );

	for( int i = 0; i < rects.size(); i++ )
	{
		auto curRect = rects[i];
		if( curRect.width < nMinAreaSize * 2 + 2 && curRect.height < nMinAreaSize * 2 + 2 )
			continue;
		if( Max( curRect.height, curRect.width ) < 14 )
		{
			float f = Min( curRect.height, curRect.width ) / 13.0f;
			if( SRand::Inst().Rand( 0.0f, 1.0f ) >= f )
				continue;
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
		m_vecRoads.push_back( SRoad( rectSplit, 0 ) );
	}
}

void CLevelGenNode2_1_0::GenBase()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector2<int32> > vecBase;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < 4; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
			{
				vecBase.push_back( TVector2<int32>( i, j ) );
				break;
			}
		}
	}
	SRand::Inst().Shuffle( vecBase );

	for( auto p : vecBase )
	{
		uint8 nType = eType_Block1 + SRand::Inst().Rand( 0, 2 );
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, nType, SRand::Inst().Rand( 16, 64 ) );
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] > eType_Road )
				m_gendata1[i + j * nWidth] = eType1_Block;
		}
	}

	for( int i = 0; i < nWidth; i++ )
	{
		int32 nBegin = SRand::Inst().Rand( 5, 10 );
		int32 nEnd = nBegin + SRand::Inst().Rand( 1, 4 );
		for( int j = nBegin; j < nEnd; j++ )
		{
			m_gendata1[i + j * nWidth] = eType1_Obj;
			if( m_gendata[i + j * nWidth] > eType_Road )
				m_gendata[i + j * nWidth] = eType_None;
		}
	}
	LvGenLib::DropObjs( m_gendata1, nWidth, nHeight, eType1_None, eType1_Obj );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata1[i + j * nWidth] == eType1_Obj )
				m_gendata[i + j * nWidth] = eType_Obj;
		}
	}
}

void CLevelGenNode2_1_0::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	for( int i = m_road1.x; i < m_road1.GetRight(); i++ )
	{
		for( int j = m_road1.y; j < m_road1.GetBottom(); j++ )
		{
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );

	for( auto p : vec )
	{
		if( m_gendata1[p.x + p.y * nWidth] != eType1_None )
			continue;
		uint8 nType;
		if( p.x >= m_road1.x && p.x < m_road1.GetRight() && p.y >= m_road1.y && p.y < m_road1.GetBottom() )
			nType = 1;
		else if( p.x > 0 && m_gendata[p.x - 1 + p.y * nWidth] == eType_Road
			|| p.x < nWidth - 1 && m_gendata[p.x + 1 + p.y * nWidth] == eType_Road
			|| p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Road )
			nType = 1;
		else
			nType = SRand::Inst().Rand( 0, 2 );

		if( nType == 1 )
		{
			auto rect = PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 2, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType1_Obj );
			if( rect.width > 0 )
			{
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						m_gendata[i + j * nWidth] = eType_Temp;
						m_gendata1[i + j * nWidth] = eType1_Obj;
					}
				}
				m_vecCargoSmall.push_back( rect );
			}
		}
		else
		{
			TVector2<int32> maxSize;
			maxSize.x = SRand::Inst().Rand( 4, 7 );
			maxSize.y = Min( maxSize.x, SRand::Inst().Rand( 3, 5 ) );
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Chunk );
			if( rect.width > 0 )
			{
				for( int i = rect.x; i < rect.GetRight(); i++ )
				{
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						m_gendata[i + j * nWidth] = eType_Chunk;
						m_gendata1[i + j * nWidth] = eType1_Obj;
					}
				}
				m_vecCargoLarge.push_back( rect );
			}
		}
	}

	for( auto& rect : m_vecCargoSmall )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_None;
			}
		}
	}
	LvGenLib::DropObj1( m_gendata1, nWidth, nHeight, m_vecCargoSmall, eType1_None, eType1_Obj );

	for( auto& road : m_vecRoads )
	{
		for( int i = road.rect.x; i < road.rect.GetRight(); i++ )
		{
			for( int j = road.rect.y; j < road.rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] != eType_Obj )
					m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}

	for( int i = 0; i < nWidth; i++ )
	{
		int32 j = nHeight - 1;
		for( ; j >= 0; j-- )
		{
			if( m_gendata[i + j * nWidth] > eType_Road || m_gendata1[i + j * nWidth] != eType_None )
				break;
		}
		for( ; j >= 0; j-- )
		{
			if( m_gendata[i + j * nWidth] == eType_None && m_gendata1[i + j * nWidth] == eType_None )
				m_gendata[i + j * nWidth] = eType_Block;
		}
	}
}

bool SHouse::Generate( vector<int8>& genData, int32 nWidth, int32 nHeight, uint8 nType, uint8 nType0, uint8 nType0a, uint8 nType1, uint8 nType2,
	vector<int8>& genData1, uint8 nWalkableType, uint8 nPathType, vector<TVector2<int32> >& par )
{
	int32 l[4] = { rect.height, rect.width, rect.height, rect.width };
	int32 nLeft[4] = { 1, 0, 1, 0 };
	int32 nRight[4] = { 3, 2, 3, 2 };
	for( int i = 0; i < 4; i++ )
	{
		if( nExitType[i] == 1 )
			continue;
		vector<TVector2<int32> > q;
		if( nExitType[i] == 2 )
		{
			for( int j = exit[i].x; j < exit[i].x + exit[i].y; j++ )
			{
				TVector2<int32> p[4] = { { 0, j }, { j, 0 }, { rect.width - 1, j }, { j, rect.height - 1 } };
				TVector2<int32> pt = p[i] + TVector2<int32>( rect.x, rect.y );
				TVector2<int32> dir[4] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
				TVector2<int32> d = dir[i];
				for( pt = pt + d; pt.x >= 0 && pt.y >= 0 && pt.x < nWidth && pt.y < nHeight; pt = pt + d )
				{
					if( genData1[pt.x + pt.y * nWidth] != 1 )
						break;
					genData1[pt.x + pt.y * nWidth] = 3;
					q.push_back( pt );
				}
			}
			ExpandDist( genData1, nWidth, nHeight, 3, 1, 2, q );
		}
		else if( nExitType[i ^ 2] != 2 || l[i ^ 1] >= 5 )
		{
			int32 iLeft = nLeft[i];
			int32 iRight = nRight[i];
			int32 nLeft = nExitType[iLeft] == 2 ? 3 : 2;
			int32 nRight = nExitType[iRight] == 2 ? 3 : 2;
			if( 2 + nLeft + nRight > l[i] )
				continue;

			vector<TVector2<int32> > q1;
			for( int j = nLeft; j < l[i] - nRight; j++ )
			{
				TVector2<int32> p[4] = { { 0, j }, { j, 0 }, { rect.width - 1, j }, { j, rect.height - 1 } };
				TVector2<int32> pt = p[i] + TVector2<int32>( rect.x, rect.y );
				q1.push_back( pt );
			}

			if( !q1.size() )
				continue;
			SRand::Inst().Shuffle( q1 );
			TVector2<int32> p = FindPath( genData1, nWidth, nHeight, 1, 3, 2, q1, par );
			if( p.x < 0 )
				continue;

			exit[i] = TVector2<int32>( nLeft, l[i] - nRight - nLeft );
			nExitType[i] = 1;
			p = par[p.x + p.y * nWidth];
			while( p.x >= 0 && genData1[p.x + p.y * nWidth] == 3 )
			{
				q.push_back( p );
				p = par[p.x + p.y * nWidth];
			}
			genData[p.x + p.y * nWidth] = nType1;

			ExpandDist( genData1, nWidth, nHeight, 3, 1, 2, q );
		}

		for( auto& p : q )
		{
			genData1[p.x + p.y * nWidth] = 2;
			genData[p.x + p.y * nWidth] = nPathType;
		}
	}
	bool b = false;
	for( int i = 0; i < 4; i++ )
	{
		if( !nExitType[i] )
		{
			int32 iLeft = nLeft[i];
			int32 iRight = nRight[i];

			int32 nLeft = !nExitType[iLeft] ? 0 : ( nExitType[iLeft] == 2 ? 3 : 2 );
			int32 nRight = !nExitType[iRight] ? 0 : ( nExitType[iRight] == 2 ? 3 : 2 );

			exit[i] = TVector2<int32>( nLeft, l[i] - nRight - nLeft );
		}
		else
			b = true;
	}
	if( !b )
		return false;

	int32 cornerSize[8];
	for( int i = 0; i < 4; i++ )
	{
		cornerSize[i * 2] = exit[i].x;
		cornerSize[i * 2 + 1] = l[i] - exit[i].x - exit[i].y;
	}

	uint8 extends[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	uint8 nExtends = 8;

	while( nExtends )
	{
		uint32 s = cornerSize[0] * cornerSize[2] + cornerSize[1] * cornerSize[6] + cornerSize[4] * cornerSize[3] + cornerSize[5] * cornerSize[7];
		if( s >= rect.width * rect.height / 2 )
			break;

		uint8 r = SRand::Inst().Rand<uint8>( 0, nExtends );
		uint8 n = extends[r];

		uint8 bMinOrMax = n & 1;
		uint8 bXOrY = ( n & 2 ) >> 1;
		uint8 bLOrR = ( n & 4 ) >> 2;
		int32 nEdgeLen = bXOrY ? rect.width : rect.height;
		int32 nRandMin = 2;
		int32 nRandMax = nEdgeLen / 2 + 2;

		bool bSucceed = false;
		do
		{
			if( !cornerSize[n] )
				break;

			if( cornerSize[n ^ 1] && cornerSize[n] + cornerSize[n ^ 1] >= nEdgeLen - 2 )
				break;
			if( cornerSize[n] + cornerSize[n ^ 5] >= nEdgeLen - 1 )
				break;
			TVector2<int32> p( 0, cornerSize[n] );
			if( bMinOrMax )
				p.y = nEdgeLen - p.y - 1;
			if( bLOrR )
				p.x = ( bXOrY ? rect.height : rect.width ) - 1;
			if( bXOrY )
				swap( p.x, p.y );

			p = p + TVector2<int32>( rect.x, rect.y );
			if( genData[p.x + p.y * nWidth] == nType1 || genData[p.x + p.y * nWidth] == nType2 )
				break;

			cornerSize[n]++;
			bSucceed = true;
		} while( 0 );

		if( !bSucceed )
		{
			extends[r] = extends[nExtends - 1];
			nExtends--;
		}
	}

	for( int i = 0; i < 8; i++ )
	{
		int32 nEdgeLen = !!( i & 2 ) ? rect.width : rect.height;
		if( !cornerSize[i ^ 1] && cornerSize[i] == nEdgeLen - 1 )
			cornerSize[i] = nEdgeLen;
	}

	for( int i = 0; i < 4; i++ )
	{
		if( !nExitType[i] )
			continue;
		exit[i].x = cornerSize[i * 2];
		exit[i].y = l[i] - cornerSize[i * 2 + 1] - exit[i].x;
		uint8 t = nExitType[i] == 2 ? nType2 : nType1;
		for( int j = exit[i].x; j < exit[i].x + exit[i].y; j++ )
		{
			TVector2<int32> p[4] = { { 0, j }, { j, 0 }, { rect.width - 1, j }, { j, rect.height - 1 } };
			TVector2<int32> pt = p[i] + TVector2<int32>( rect.x, rect.y );
			genData[pt.x + pt.y * nWidth] = t;
			if( nExitType[i] == 2 )
			{
				TVector2<int32> p1[4] = { { 1, j }, { j, 1 }, { rect.width - 2, j }, { j, rect.height - 2 } };
				pt = p1[i] + TVector2<int32>( rect.x, rect.y );
					genData[pt.x + pt.y * nWidth] = t;
			}
		}
	}

	uint8 arr[4][2] = { { 2, 0 }, { 6, 1 }, { 3, 4 }, { 7, 5 } };
	for( int i = 0; i < 4; i++ )
	{
		int32 w = cornerSize[arr[i][0]];
		int32 h = cornerSize[arr[i][1]];
		int32 x = !!( arr[i][0] & 1 ) ? rect.width - w : 0;
		int32 y = !!( arr[i][1] & 1 ) ? rect.height - h : 0;

		for( int iX = 0; iX < w; iX++ )
		{
			for( int iY = 0; iY < h; iY++ )
			{
				genData[iX + x + rect.x + ( iY + y + rect.y ) * nWidth] = nType0 + i;
			}
		}

		if( w >= 4 && h >= 4 )
		{
			int32 nX = x + ( w - SRand::Inst().Rand( 0, 2 ) ) / 2 + rect.x;
			int32 nY = y + ( h - SRand::Inst().Rand( 0, 2 ) ) / 2 + rect.y;
			genData[nX + nY * nWidth] = nType0a;
		}
	}
	return true;
}

void CLevelGenNode2_1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pWalkableNodes[0] = CreateNode( pXml->FirstChildElement( "walkable_a" )->FirstChildElement(), context );
	m_pWalkableNodes[1] = CreateNode( pXml->FirstChildElement( "walkable_b" )->FirstChildElement(), context );
	m_pWalkableNodes[2] = CreateNode( pXml->FirstChildElement( "walkable_c" )->FirstChildElement(), context );
	m_pWalkableNodes[3] = CreateNode( pXml->FirstChildElement( "walkable_d" )->FirstChildElement(), context );
	m_pBlockNode = CreateNode( pXml->FirstChildElement( "block" )->FirstChildElement(), context );
	m_pBlockNodes[0] = CreateNode( pXml->FirstChildElement( "block_a" )->FirstChildElement(), context );
	m_pBlockNodes[1] = CreateNode( pXml->FirstChildElement( "block_b" )->FirstChildElement(), context );
	m_pBlockNodes[2] = CreateNode( pXml->FirstChildElement( "block_c" )->FirstChildElement(), context );
	m_pBlockNodes[3] = CreateNode( pXml->FirstChildElement( "block_d" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "fence" ) )
		m_pFenceNode = CreateNode( pXml->FirstChildElement( "fence" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "room" ) )
		m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "cargo" ) )
		m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "cargo2" ) )
		m_pCargo2Node = CreateNode( pXml->FirstChildElement( "cargo2" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "barrel" ) )
		m_pBarrelNode = CreateNode( pXml->FirstChildElement( "barrel" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "barrel1" ) )
		m_pBarrel1Node = CreateNode( pXml->FirstChildElement( "barrel1" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenAreas();
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

			if( genData == eType_Unwalkable )
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			else if( genData >= eType_Walkable_a && genData <= eType_Walkable_d )
				m_pWalkableNodes[genData - eType_Walkable_a]->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			else if( genData >= eType_Block_a && genData <= eType_Block_d )
				m_pBlockNodes[genData - eType_Block_a]->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	context.mapTags["mask"] = eType_Block;
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
	context.mapTags["s"] = m_nType >= 4 ? 1 : 0;
	for( auto& house : m_vecHouses )
	{
		if( house.rect.width )
			m_pHouseNode->Generate( context, house.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	for( auto& rect : m_vecCargoSmall )
	{
		if( !rect.width )
			continue;
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecCargoLarge )
	{
		m_pCargo2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecBarrels )
	{
		m_pBarrelNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecBarrels1 )
	{
		m_pBarrel1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_par.clear();
	m_vecRoads.clear();
	m_vecFences.clear();
	m_vecFenceBlock.clear();
	m_vecArea1.clear();
	m_vecArea2.clear();
	m_vecRooms.clear();
	m_vecHouses.clear();
	m_vecCargoSmall.clear();
	m_vecCargoLarge.clear();
	m_vecBarrels.clear();
	m_vecBarrels1.clear();
}

void CLevelGenNode2_1_1::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	uint8 nType;
	if( !m_pFenceNode )
		nType = SRand::Inst().Rand( 0, 3 );
	else
		nType = SRand::Inst().Rand( 3, 5 );
	m_nType = nType;

	vector<TRectangle<int32> > tempRect;
	if( nType == 0 )
	{
		uint32 w0 = 8;
		uint32 l0 = SRand::Inst().Rand( ( nWidth - w0 ) / 2 - 1, nWidth - w0 - ( nWidth - w0 ) / 2 + 2 );
		uint32 h0 = SRand::Inst().Rand( nHeight / 2 - 1, nHeight / 2 + 1 );
		TRectangle<int32> r0( l0, 0, w0, h0 );

		uint32 lDist = SRand::Inst().Rand( 4, 6 );
		uint32 rDist = SRand::Inst().Rand( 4, 6 );
		TRectangle<int32> r1( lDist, h0 - 2, r0.x - lDist, 2 );
		TRectangle<int32> r2( r0.GetRight(), h0 - 2, nWidth - rDist - r0.GetRight(), 2 );

		TRectangle<int32> r3( r1.x, h0, 2, nHeight - h0 );
		TRectangle<int32> r4( r2.GetRight() - 2, h0, 2, nHeight - h0 );

		{
			uint32 w = SRand::Inst().Rand( 3, 5 );
			uint32 h = SRand::Inst().Rand( h0 / 2 - 1, h0 / 2 + 2 );
			m_vecArea1.push_back( TRectangle<int32>( r0.x - w, h0 - 2 - h, w, h ) );
			w = SRand::Inst().Rand( 3, 5 );
			h = SRand::Inst().Rand( h0 / 2 - 1, h0 / 2 + 2 );
			m_vecArea1.push_back( TRectangle<int32>( r0.GetRight(), h0 - 2 - h, w, h ) );
		}
		{
			TRectangle<int32> r( r1.x, r1.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
			r1.SetLeft( r.GetRight() );
			r3.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( r2.GetRight() - 2, r2.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
			r2.SetRight( r.GetLeft() );
			r4.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			uint32 h = SRand::Inst().Rand( 4, 6 );
			uint32 l = SRand::Inst().Rand( 4, 6 );
			uint32 r = SRand::Inst().Rand( 4, 6 );
			m_vecArea1.push_back( TRectangle<int32>( nWidth / 2 - l, r0.GetBottom(), l + r, h ) );
			SHouse house( nWidth / 2 - l, r0.GetBottom() + h, l + r, nHeight - r0.GetBottom() - h  );
			house.exit[1] = TVector2<int32>( ( house.rect.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}

		m_vecRoads.push_back( r0 );
		m_vecRoads.push_back( r1 );
		m_vecRoads.push_back( r2 );
		m_vecRoads.push_back( r3 );
		m_vecRoads.push_back( r4 );
	}
	else if( nType == 1 )
	{
		uint32 w0 = 8;
		uint32 l0 = SRand::Inst().Rand( ( nWidth - w0 ) / 2 - 1, nWidth - w0 - ( nWidth - w0 ) / 2 + 2 );
		uint32 h0 = SRand::Inst().Rand( 6, 8 );
		TRectangle<int32> r0( l0, 0, w0, h0 );

		uint32 h00 = h0 / 2 + SRand::Inst().Rand( 0, 2 );
		uint32 lDist = SRand::Inst().Rand( 3, 5 );
		uint32 rDist = SRand::Inst().Rand( 3, 5 );
		TRectangle<int32> r1( lDist, h00, r0.x - lDist, h0 - h00 );
		TRectangle<int32> r2( r0.GetRight(), h00, nWidth - rDist - r0.GetRight(), h0 - h00 );

		uint32 h1 = SRand::Inst().Rand( 6, 8 );
		TRectangle<int32> r3( r1.x, h0, 2, nHeight - h1 - h0 );
		TRectangle<int32> r4( r2.GetRight() - 2, h0, 2, nHeight - h1 - h0 );

		uint32 l = nWidth - rDist - lDist - 4;
		uint32 w2a = SRand::Inst().Rand( 3, 5 );
		uint32 w2b = SRand::Inst().Rand( 3, 5 );
		uint32 l2 = ( l - w2a - w2b ) / 2 - SRand::Inst().Rand( 0, 3 );
		uint32 l2a = lDist + 2 + SRand::Inst().Rand( ( l - l2 - w2a - w2b ) / 2 - 1, ( l - l2 - w2a - w2b ) - ( l - l2 - w2a - w2b ) / 2 + 2 );
		uint32 l2b = l2a + w2a + l2;
		uint32 h2 = Min( nHeight - r0.GetBottom() - 4, SRand::Inst().Rand( nHeight / 2 + 2, nHeight / 2 + 5 ) );
		TRectangle<int32> r5( l2a, nHeight - h2, w2a, h2 );
		TRectangle<int32> r6( l2b, nHeight - h2, w2b, h2 );

		{
			TRectangle<int32> r( r1.x, r1.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 1, 3 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 3, 4 ) );
			r1.SetLeft( r.GetRight() );
			r3.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( r2.GetRight() - 2, r2.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 1, 3 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 3, 4 ) );
			r2.SetRight( r.GetLeft() );
			r4.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( r1.x, 0, r1.width, r0.y );
			r.SetLeft( r.GetLeft() + SRand::Inst().Rand( 2, 6 ) );
			m_vecArea1.push_back( r );
		}
		{
			TRectangle<int32> r( r2.x, 0, r2.width, r0.y );
			r.SetRight( r.GetRight() - SRand::Inst().Rand( 2, 6 ) );
			m_vecArea1.push_back( r );
		}
		{
			TRectangle<int32> r( r5.x, r0.GetBottom(), r5.width, 0 );
			r.SetLeft( Max( r3.GetRight(), r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r5.GetRight() + ( r6.x - r5.GetRight() ) / 2, r.GetRight() + 2 ) );
			r.SetBottom( Min( r5.y, r.y + SRand::Inst().Rand( 4, 8 ) ) );
			SHouse house( r );
			house.exit[3] = TVector2<int32>( SRand::Inst().Rand( 0, r5.width - 1 ) + r5.x - r.x, 2 );
			house.nExitType[3] = 1;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r6.x, r0.GetBottom(), r6.width, 0 );
			r.SetLeft( Max( r6.x - ( r6.x - r5.GetRight() ) / 2, r.GetLeft() - 2 ) );
			r.SetRight( Min( r4.x, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r6.y, r.y + SRand::Inst().Rand( 4, 8 ) ) );
			SHouse house( r );
			house.exit[3] = TVector2<int32>( SRand::Inst().Rand( 0, r6.width - 1 ) + r6.x - r.x, 2 );
			house.nExitType[3] = 1;
			m_vecHouses.push_back( house );
		}

		{
			TRectangle<int32> r( r3.x, r3.GetBottom(), r3.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( 0, r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r5.x, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r3.width - 1 ) + r3.x - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r4.x, r4.GetBottom(), r4.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( r6.GetRight(), r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( nWidth, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r4.width - 1 ) + r4.x - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( 0, 0, 0, 0 );
			r.x = r3.GetRight();
			r.SetRight( r5.GetLeft() );
			r.y = Max( m_vecArea2[0].GetBottom(), m_vecHouses[0].rect.GetBottom() );
			r.SetBottom( m_vecHouses[2].rect.GetTop() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( 0, 0, 0, 0 );
			r.x = r6.GetRight();
			r.SetRight( r4.GetLeft() );
			r.y = Max( m_vecArea2[1].GetBottom(), m_vecHouses[1].rect.GetBottom() );
			r.SetBottom( m_vecHouses[3].rect.GetTop() );
			m_vecArea2.push_back( r );
		}

		m_vecRoads.push_back( r0 );
		m_vecRoads.push_back( r1 );
		m_vecRoads.push_back( r2 );
		m_vecRoads.push_back( r3 );
		m_vecRoads.push_back( r4 );
		m_vecRoads.push_back( r5 );
		m_vecRoads.push_back( r6 );
	}
	else if( nType == 2 )
	{
		uint32 w0 = SRand::Inst().Rand( 12, 15 );
		uint32 l0 = SRand::Inst().Rand( ( nWidth - w0 ) / 2 - 1, nWidth - w0 - ( nWidth - w0 ) / 2 + 2 );
		uint32 h0 = SRand::Inst().Rand( 3, 5 );
		TRectangle<int32> r0( l0, 0, w0, h0 );

		uint32 h1 = SRand::Inst().Rand( nHeight / 2 + 2, nHeight / 2 + 5 ) - h0;
		uint32 w1 = SRand::Inst().Rand( 3, 5 );
		uint32 w2 = SRand::Inst().Rand( 3, 5 );
		uint32 ofs1 = SRand::Inst().Rand( 1, 3 );
		uint32 ofs2 = SRand::Inst().Rand( 1, 3 );
		TRectangle<int32> r1( l0 - ofs1, h0, w1, h1 );
		TRectangle<int32> r2( l0 + w0 + ofs2 - w2, h0, w2, h1 );

		uint32 w3 = SRand::Inst().Rand( 2, 3 );
		uint32 l3 = SRand::Inst().Rand( ( nWidth - w3 ) / 2 - 1, nWidth - w3 - ( nWidth - w3 ) / 2 + 2 );
		uint32 h2 = SRand::Inst().Rand( 5, 7 );
		TRectangle<int32> r3( l3, nHeight - h2, w3, h2 );

		uint32 h3 = SRand::Inst().Rand( ( h1 + h0 ) / 2 - 2, ( h1 + h0 ) / 2 + 2 );
		uint32 lDist = SRand::Inst().Rand( 4, 6 );
		uint32 rDist = SRand::Inst().Rand( 4, 6 );
		TRectangle<int32> rc1( lDist, h3, r1.x - lDist, 2 );
		TRectangle<int32> rc2( r2.GetRight(), h3, nWidth - rDist - r2.GetRight(), 2 );

		{
			TRectangle<int32> r( r1.x, r1.GetBottom(), r1.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( 0, r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r3.x, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r1.width - 1 ) + r1.x - r.x, 2 );
			house.nExitType[1] = 1;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r2.x, r2.GetBottom(), r2.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( r3.GetRight(), r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( nWidth, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r2.width - 1 ) + r2.x - r.x, 2 );
			house.nExitType[1] = 1;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( 0, rc1.y, rc1.x, rc1.height );
			r.SetTop( Max( 0, r.GetTop() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r1.GetBottom(), r.GetBottom() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[2] = TVector2<int32>( SRand::Inst().Rand( 0, rc1.height - 1 ) + rc1.y - r.y, 2 );
			house.nExitType[2] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( rc2.GetRight(), rc2.y, nWidth - rc2.GetRight(), rc2.height );
			r.SetTop( Max( 0, r.GetTop() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r2.GetBottom(), r.GetBottom() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[0] = TVector2<int32>( SRand::Inst().Rand( 0, rc2.height - 1 ) + rc2.y - r.y, 2 );
			house.nExitType[0] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( rc1.x, rc1.GetBottom(), rc1.width, m_vecHouses[0].rect.y - rc1.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( rc2.x, rc2.GetBottom(), rc2.width, m_vecHouses[1].rect.y - rc2.GetBottom() );
			m_vecArea2.push_back( r );
		}

		m_vecRoads.push_back( r0 );
		m_vecRoads.push_back( r1 );
		m_vecRoads.push_back( r2 );
		m_vecRoads.push_back( r3 );
		m_vecRoads.push_back( rc1 );
		m_vecRoads.push_back( rc2 );
	}
	else if( nType == 3 )
	{
		uint32 w = SRand::Inst().Rand( 15, 18 );
		uint32 l = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
		uint32 r = nWidth - l - w;
		uint32 w1 = ( w - 4 + SRand::Inst().Rand( 0, 3 ) ) / 3;
		uint32 w2 = ( w - 4 + SRand::Inst().Rand( 0, 3 ) ) / 3;
		uint32 w3 = w - 4 - w1 - w2;
		if( w1 < w3 )
			swap( w1, w3 );
		if( w2 < w3 )
			swap( w2, w3 );
		if( SRand::Inst().Rand( 0, 2 ) )
			swap( w1, w2 );

		uint32 h0 = 4;
		uint32 h1 = SRand::Inst().Rand( 7, 10 );
		if( w3 < 4 )
			h1 = Max( h1, h0 + 4 );
		uint32 h2 = Min<uint32>( nHeight / 2 + SRand::Inst().Rand( -1, 2 ), h1 + SRand::Inst().Rand( 2, 5 ) );

		m_vecRoads.push_back( TRectangle<int32>( l, 0, w1, nHeight - h0 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + 2, 0, w3, nHeight - h1 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + w3 + 4, 0, w2, nHeight - h0 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1, 0, 2, nHeight - h0 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1 + w3 + 2, 0, 2, nHeight - h0 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1, 0, 2, nHeight - h2 - 2 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1, nHeight - h2, 2, h2 - h0 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1 + w3 + 2, 0, 2, nHeight - h2 - 2 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + w1 + w3 + 2, nHeight - h2, 2, h2 - h0 ) );

		tempRect.push_back( TRectangle<int32>( l + w1, nHeight - h1, w - w1 - w2, h1 ) );
		if( w3 >= 4 )
			m_vecBarrels1.push_back( TRectangle<int32>( SRand::Inst().Rand( m_vecRoads[1].x, m_vecRoads[1].GetRight() - 3 ), nHeight - h1 + 1, 4, 2 ) );
		else
			m_vecBarrels.push_back( TRectangle<int32>( SRand::Inst().Rand( m_vecRoads[1].x, m_vecRoads[1].GetRight() - 1 ), nHeight - h1 + 1, 2, 3 ) );

		{
			TRectangle<int32> r( l, nHeight - h0, w1, h0 );
			r.SetLeft( r.GetLeft() - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( ( w1 - 1 ) / 2 + l - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( l + w - w2, nHeight - h0, w2, h0 );
			r.SetLeft( r.GetLeft() - SRand::Inst().Rand( 2, 4 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( ( w2 - 2 ) / 2 + l + w - w2 - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
	}
	else if( nType == 4 )
	{
		uint32 w = SRand::Inst().Rand( 15, 18 );
		uint32 l = ( nWidth - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
		uint32 r = nWidth - l - w;
		uint32 w1 = 3;
		uint32 w2 = 3;
		uint32 w3 = w - 4 - w1 - w2;
		uint32 h0 = SRand::Inst().Rand( nHeight / 2 - 3, nHeight / 2 + 1 );
		uint32 h00 = h0 - 2;
		uint32 h1 = SRand::Inst().Rand( 6, 8 );
		uint32 h2 = SRand::Inst().Rand( 6, 8 );
		uint32 h3 = 4;

		m_vecRoads.push_back( TRectangle<int32>( l, h0, w1, nHeight - h0 - h1 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + 2, h00, w3, nHeight - h00 - h3 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w1 + w3 + 4, h0, w2, nHeight - h0 - h2 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1, h0, 2, nHeight - h0 - h3 ) );
		m_vecFences.push_back( TRectangle<int32>( l + w1 + w3 + 2, h0, 2, nHeight - h0 - h3 ) );
		m_vecFenceBlock.push_back( m_vecFences[0] );
		m_vecFenceBlock.push_back( m_vecFences[1] );
		m_vecFenceBlock[0].SetTop( m_vecFenceBlock[0].y + SRand::Inst().Rand( 2, 4 ) );
		m_vecFenceBlock[1].SetTop( m_vecFenceBlock[1].y + SRand::Inst().Rand( 2, 4 ) );
		tempRect.push_back( TRectangle<int32>( l, nHeight - h1, w1, h1 ) );
		tempRect.push_back( TRectangle<int32>( l + w - w2, nHeight - h2, w2, h2 ) );

		m_vecBarrels.push_back( TRectangle<int32>( SRand::Inst().Rand( l, l + w1 - 1 ), nHeight - h1 + 1, 2, 3 ) );
		m_vecBarrels.push_back( TRectangle<int32>( SRand::Inst().Rand( l + w - w2, l + w - 1 ), nHeight - h2 + 1, 2, 3 ) );

		{
			TRectangle<int32> r( l + w1, nHeight - h3, w3 + 4, h3 );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( ( w3 - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2 + l + w1 + 2 - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}

		m_vecRoads.push_back( TRectangle<int32>( l, 0, w1 + 2, h0 ) );
		m_vecRoads.push_back( TRectangle<int32>( l + w - w2 - 2, 0, w2 + 2, h0 ) );
		TRectangle<int32> rect( l + w1 + 2, 0, w - w1 - w2 - 4, h00 );
		m_vecFences.push_back( rect );
		rect.SetLeft( rect.x + 1 );
		rect.SetRight( rect.GetRight() - 1 );
		rect.height -= SRand::Inst().Rand( 1, 3 );
		m_vecFenceBlock.push_back( rect );
	}

	for( auto& road : m_vecRoads )
	{
		for( int i = road.x; i < road.GetRight(); i++ )
		{
			for( int j = road.y; j < road.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}
	for( auto& fence : m_vecFences )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk1;
			}
		}
	}
	for( auto& fence : m_vecFenceBlock )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk;
			}
		}
	}
	for( auto& house : m_vecHouses )
	{
		for( int i = house.rect.x; i < house.rect.GetRight(); i++ )
		{
			for( int j = house.rect.y; j < house.rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_House;
			}
		}
	}
	for( auto& rect : m_vecArea1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp0;
			}
		}
	}
	for( auto& rect : m_vecArea2 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp1;
			}
		}
	}
	for( auto& rect : m_vecBarrels )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}
	for( auto& rect : m_vecBarrels1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}

	if( m_pRoomNode )
	{
		if( m_nType < 3 )
		{
			int32 nRoomCount = 2;
			for( int i = 0; i < 2 && nRoomCount; i++ )
			{
				vector<TVector2<int32> > vec;
				uint8 nType = i == 0 ? eType_Temp1 : eType_None;
				FindAllOfTypesInMap( m_gendata, nWidth, nHeight, nType, vec );
				SRand::Inst().Shuffle( vec );
				for( auto& p : vec )
				{
					if( m_gendata[p.x + p.y * nWidth] != nType )
						continue;

					auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 6, 6 ), TVector2<int32>( 8, 8 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Room );
					if( rect.width > 0 )
					{
						m_vecRooms.push_back( rect );
						nRoomCount--;
						if( !nRoomCount )
							break;
					}
				}
			}
		}

		for( auto& room : m_vecRooms )
		{
			for( int i = room.x; i < room.GetRight(); i++ )
			{
				for( int j = room.y; j < room.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_Room;
				}
			}

			if( room.width >= 6 )
			{
				int32 x = room.x + ( room.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				int32 y = room.y;
				m_gendata[x + y * nWidth] = m_gendata[x + 1 + y * nWidth] = eType_Door;
				x = room.x + ( room.width - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
				y = room.GetBottom() - 1;
				m_gendata[x + y * nWidth] = m_gendata[x + 1 + y * nWidth] = eType_Door;
			}

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

	for( auto& rect : tempRect )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !m_gendata[i + j * nWidth] )
					m_gendata[i + j * nWidth] = eType_Temp2;
			}
		}
	}
	if( m_nType >= 3 )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			uint32 h1 = SRand::Inst().Rand( 6, 9 );
			uint32 h2 = SRand::Inst().Rand( 4, 6 );

			for( int j = h1; j < nHeight - h2; j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					m_gendata[i + j * nWidth] = eType_Temp0;
			}
		}
	}

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	if( nType >= 3 )
		FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Temp0, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( nType < 3 && m_vecHouses.size() >= 6 )
			break;
		if( m_gendata[p.x + p.y * nWidth] != eType_None && ( nType < 3 || m_gendata[p.x + p.y * nWidth] != eType_Temp0 ) )
			continue;

		int i = 0;
		if( nType < 3 || m_gendata[p.x + p.y * nWidth] == eType_None )
		{
			TVector2<int32> ofs[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
			for( i = 0; i < 4; i++ )
			{
				auto p1 = p + ofs[i];
				if( p1.x < 0 || p1.y < 0 || p1.x >= nWidth || p1.y >= nHeight )
					continue;
				if( m_gendata[p1.x + p1.y * nWidth] == eType_Road || m_gendata[p1.x + p1.y * nWidth] == eType_Temp0 )
					break;
			}
			if( i >= 4 )
				continue;
		}

		TVector2<int32> sizeMin = i == 0 || i == 2 ? TVector2<int32>( 4, 6 ) : TVector2<int32>( 6, 4 );
		TVector2<int32> sizeMax;
		if( nType < 3 )
			sizeMax = TVector2<int32>( SRand::Inst().Rand( 8, 11 ), SRand::Inst().Rand( 8, 11 ) );
		else
			sizeMax = sizeMin + TVector2<int32>( SRand::Inst().Rand( 0, 3 ), SRand::Inst().Rand( 0, 3 ) );

		auto rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin, sizeMax, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House );
		if( rect.width > 0 )
			m_vecHouses.push_back( rect );
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp0 )
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
			m_gendata1, eType_Temp0, eType_Temp0_0, m_par ) )
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

	for( auto& rect : tempRect )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp2 )
					m_gendata[i + j * nWidth] = eType_None;
			}
		}
	}
}

void CLevelGenNode2_1_1::GenObjs()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TVector2<int32> > vecRoad;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Road, vecRoad );

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& rect : m_vecArea2 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp1 )
					m_gendata[i + j * nWidth] = eType_None;
			}
		}
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] > eType_Road )
				m_gendata1[i + j * nWidth] = eType1_Block;
			else
				m_gendata1[i + j * nWidth] = eType1_None;
		}
	}

	for( auto p : vec )
	{
		if( m_gendata1[p.x + p.y * nWidth] != eType1_None )
			continue;
		uint8 nType = 0;
		if( p.x > 0 && m_gendata[p.x - 1 + p.y * nWidth] == eType_Road
			|| p.x < nWidth - 1 && m_gendata[p.x + 1 + p.y * nWidth] == eType_Road
			|| p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Road )
			nType = 1;

		PlaceChunk( p, nType );
	}

	for( auto& rect : m_vecArea2 )
	{
		vec.clear();
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					vec.push_back( TVector2<int32>( i, j ) );
			}
		}
		SRand::Inst().Shuffle( vec );
		int32 s = vec.size() / 3;

		for( auto p : vec )
		{
			if( m_gendata1[p.x + p.y * nWidth] != eType1_None )
				continue;
			uint8 nType = 0;
			if( p.x > 0 && m_gendata[p.x - 1 + p.y * nWidth] == eType_Road
				|| p.x < nWidth - 1 && m_gendata[p.x + 1 + p.y * nWidth] == eType_Road
				|| p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Road )
				nType = 1;
			else
				nType = SRand::Inst().Rand( 0, 2 );

			auto chunk = PlaceChunk( p, nType );
			s -= chunk.width * chunk.height;
			if( s <= 0 )
				break;
		}
	}

	for( auto& rect : m_vecCargoSmall )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_None;
			}
		}
	}
	LvGenLib::DropObj1( m_gendata1, nWidth, nHeight, m_vecCargoSmall, eType1_None, eType1_Obj );

	for( auto& rect : m_vecArea2 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}

	for( auto& p : vecRoad )
	{
		m_gendata[p.x + p.y * nWidth] = eType_Road;
	}
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
			{
				if( m_gendata1[i + j * nWidth] == eType1_None )
					m_gendata[i + j * nWidth] = eType_Block;
				else if( m_gendata1[i + j * nWidth] == eType1_Obj )
					m_gendata[i + j * nWidth] = eType_Unwalkable;
			}
		}
	}

	int8 nTypes[4] = { eType_Walkable_a, eType_Walkable_b, eType_Walkable_c, eType_Walkable_d };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 64, 128, eType_Temp0_0, nTypes, 4 );
	int8 nTypes1[4] = { eType_Block_a, eType_Block_b, eType_Block_c, eType_Block_d };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 64, 128, eType_Temp0, nTypes1, 4 );
}

TRectangle<int32> CLevelGenNode2_1_1::PlaceChunk( TVector2<int32>& p, uint8 nType )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( nType == 1 )
	{
		auto rect = PutRect( m_gendata1, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 2, 2 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType1_Obj );
		if( rect.width > 0 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_Temp1;
					m_gendata1[i + j * nWidth] = eType1_Obj;
				}
			}
			m_vecCargoSmall.push_back( rect );
		}
		return rect;
	}
	else
	{
		TVector2<int32> maxSize;
		maxSize.x = SRand::Inst().Rand( 3, 7 );
		maxSize.y = Min( maxSize.x, SRand::Inst().Rand( 2, 5 ) );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 2 ), maxSize, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Chunk );
		if( rect.width > 0 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata[i + j * nWidth] = eType_Chunk;
					m_gendata1[i + j * nWidth] = eType1_Obj;
				}
			}
			m_vecCargoLarge.push_back( rect );
		}
		return rect;
	}
}

void CLevelGenNode2_1_2::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pRoadNode = CreateNode( pXml->FirstChildElement( "road" )->FirstChildElement(), context );
	m_pWalkableNodes[0] = CreateNode( pXml->FirstChildElement( "walkable_a" )->FirstChildElement(), context );
	m_pWalkableNodes[1] = CreateNode( pXml->FirstChildElement( "walkable_b" )->FirstChildElement(), context );
	m_pWalkableNodes[2] = CreateNode( pXml->FirstChildElement( "walkable_c" )->FirstChildElement(), context );
	m_pWalkableNodes[3] = CreateNode( pXml->FirstChildElement( "walkable_d" )->FirstChildElement(), context );
	m_pHouseNode = CreateNode( pXml->FirstChildElement( "house" )->FirstChildElement(), context );
	m_pFenceNode = CreateNode( pXml->FirstChildElement( "fence" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_1_2::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );
	m_par.resize( region.width * region.height );

	GenAreas();
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

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();
	m_par.clear();
	m_vecRoads.clear();
	m_vecFences.clear();
	m_vecFenceBlock.clear();
	m_vecHouses.clear();
}

void CLevelGenNode2_1_2::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	
	vector<TRectangle<int32> > vecTempRect;
	uint32 l = SRand::Inst().Rand( 4, 6 );
	uint32 r = SRand::Inst().Rand( 4, 6 );
	uint32 w = nWidth - l - r;
	uint32 h0 = 4;
	m_vecRoads.push_back( TRectangle<int32>( 3, 0, nWidth - 6, h0 ) );
	m_vecFences.push_back( TRectangle<int32>( 0, 0, 3, h0 ) );
	m_vecFences.push_back( TRectangle<int32>( nWidth - 3, 0, 3, h0 ) );
	m_vecFenceBlock.push_back( m_vecFences[0] );
	m_vecFenceBlock.push_back( m_vecFences[1] );

	{
		TRectangle<int32> rect( l, h0, nWidth - l - r, 2 );
		m_vecFences.push_back( rect );
		vecTempRect.push_back( m_vecFences.back().Offset( TVector2<int32>( 0, 2 ) ) );
		uint32 nSplit = ( rect.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		m_vecFenceBlock.push_back( TRectangle<int32>( l + 2, h0, nSplit - 3, 2 ) );
		m_vecFenceBlock.push_back( TRectangle<int32>( l + nSplit + 1, h0, rect.width - nSplit - 3, 2 ) );
	}
	h0 += 2;

	uint32 h1 = 4;
	TRectangle<int32> centerRect( l, h0, r - l, h1 - h0 );

	uint32 w1[3] = { ( centerRect.width - 4 ) / 3, ( centerRect.width - 3 ) / 3, ( centerRect.width - 2 ) / 3 };
	SRand::Inst().Shuffle( w1, ELEM_COUNT( w1 ) );
	m_vecRoads.push_back( TRectangle<int32>( centerRect.x + w1[0], centerRect.y, 2, centerRect.height ) );
	vecTempRect.push_back( TRectangle<int32>( centerRect.x + w1[0] - 2, centerRect.y, 6, 4 ) );
	m_vecRoads.push_back( TRectangle<int32>( centerRect.x + w1[0] + w1[1] + 2, centerRect.y, 2, centerRect.height ) );
	vecTempRect.push_back( TRectangle<int32>( centerRect.x + w1[0] + w1[1], centerRect.y, 6, 4 ) );

	TRectangle<int32> houseRect1( centerRect.x + w1[0], centerRect.GetBottom(), w1[1] + 4, 4 );
	uint32 nSplit = ( houseRect1.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
	{
		TRectangle<int32> rect = houseRect1;
		rect.width = nSplit;
		rect.SetLeft( rect.x - SRand::Inst().Rand( 2, 4 ) );
		SHouse house( rect );
		house.exit[1] = TVector2<int32>( w1[0] + centerRect.x - rect.x, 2 );
		house.nExitType[1] = 2;
		m_vecHouses.push_back( house );
	}
	{
		TRectangle<int32> rect = houseRect1;
		rect.SetLeft( rect.x + nSplit );
		rect.SetRight( rect.GetRight() + SRand::Inst().Rand( 2, 4 ) );
		SHouse house( rect );
		house.exit[1] = TVector2<int32>( w1[0] + w1[1] + 2 + centerRect.x - rect.x, 2 );
		house.nExitType[1] = 2;
		m_vecHouses.push_back( house );
	}

	for( int i = 0; i < 2; i++ )
	{
		uint32 hSplit = SRand::Inst().Rand( 1, 3 );
		uint32 nSplit = centerRect.y - 2 + ( centerRect.height + SRand::Inst().Rand( 0, 2 ) - hSplit ) / 2;

		TRectangle<int32> rect( i * ( centerRect.GetRight() - w1[2] ), nSplit,
			i == 0 ? centerRect.x + w1[0] : nWidth - centerRect.GetRight() + w1[2], hSplit );
		m_vecFences.push_back( rect );
		m_vecFenceBlock.push_back( rect );

		{
			TRectangle<int32> houseRect( i * centerRect.GetRight(), centerRect.y - 2,
				i == 0 ? centerRect.x : nWidth - centerRect.GetRight(), nSplit );
			SHouse house( houseRect );
			/*uint32 nExitLen = SRand::Inst().Rand( 2, houseRect.height - 3 );
			house.exit[2 - i * 2] = TVector2<int32>( 2 + SRand::Inst().Rand( 0u, houseRect.height - 3 - nExitLen ), nExitLen );
			house.nExitType[2 - i * 2] = 1;*/
			m_vecHouses.push_back( house );
		}

		{
			TRectangle<int32> houseRect( i * centerRect.GetRight(), nSplit + 2,
				i == 0 ? centerRect.x : nWidth - centerRect.GetRight(), centerRect.GetBottom() - 2 - nSplit - 2 );
			if( houseRect.width > 4 && SRand::Inst().Rand( 0, 2 ) )
			{
				houseRect.width--;
				if( i )
					houseRect.x++;
			}
			SHouse house( houseRect );
			/*uint32 nExitLen = SRand::Inst().Rand( 2, houseRect.height - 3 );
			house.exit[2 - i * 2] = TVector2<int32>( 2 + SRand::Inst().Rand( 0u, houseRect.height - 3 - nExitLen ), nExitLen );
			house.nExitType[2 - i * 2] = 1;*/
			m_vecHouses.push_back( house );
		}

		{
			TRectangle<int32> r1 = m_vecHouses.back().rect;
			TRectangle<int32> r2 = m_vecHouses[i].rect;
			TRectangle<int32> houseRect( i * r2.GetRight(), r1.GetBottom(), i == 0 ? r2.x : nWidth - r2.GetRight(), nHeight - r1.GetBottom() );
			uint32 w = Min( houseRect.width - 4, SRand::Inst().Rand( 2, 4 ) );
			uint32 h = SRand::Inst().Rand( 1, 3 );
			TRectangle<int32> fenceRect( i * ( centerRect.GetRight() - w ), houseRect.y, w, h );
			m_vecFences.push_back( fenceRect );
			m_vecFenceBlock.push_back( fenceRect );
			houseRect.SetTop( houseRect.y + h );
			
			SHouse house( houseRect );
			/*uint32 nExitLen = SRand::Inst().Rand( 2, houseRect.width - 4 );
			house.exit[1] = TVector2<int32>( 3 - i + SRand::Inst().Rand( 0u, houseRect.width - 4 - nExitLen ), nExitLen );
			house.nExitType[1] = 1;*/
			m_vecHouses.push_back( house );
		}
	}

	TRectangle<int32> centerRect1( centerRect.x + w1[0] + 2, centerRect.y, w1[1], centerRect.height );
	{
		uint32 nHouseWidth = Min( centerRect.width, SRand::Inst().Rand( 4, 7 ) );
		uint32 nHouseHeight = SRand::Inst().Rand( 6, 8 );
		TRectangle<int32> houseRect( centerRect1.x + ( centerRect1.width + SRand::Inst().Rand( 0, 2 ) - nHouseWidth ) / 2,
			centerRect1.y + ( centerRect1.height + SRand::Inst().Rand( 0, 2 ) - nHouseHeight ) / 2, nHouseWidth, nHouseHeight );
		SHouse house( houseRect );
		m_vecHouses.push_back( house );
	}

	for( auto& road : m_vecRoads )
	{
		for( int i = road.x; i < road.GetRight(); i++ )
		{
			for( int j = road.y; j < road.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Road;
			}
		}
	}
	for( auto& fence : m_vecFences )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk1;
			}
		}
	}
	for( auto& fence : m_vecFenceBlock )
	{
		for( int i = fence.x; i < fence.GetRight(); i++ )
		{
			for( int j = fence.y; j < fence.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Chunk;
			}
		}
	}
	for( auto& house : m_vecHouses )
	{
		for( int i = house.rect.x; i < house.rect.GetRight(); i++ )
		{
			for( int j = house.rect.y; j < house.rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_House;
			}
		}
	}

	for( auto& rect : vecTempRect )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_None )
					m_gendata[i + j * nWidth] = eType_Temp0;
			}
		}
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
				m_gendata1[i + j * nWidth] = 1;
			else if( m_gendata[i + j * nWidth] == eType_Road || m_gendata[i + j * nWidth] == eType_Chunk1 || m_gendata[i + j * nWidth] == eType_Temp0 )
				m_gendata1[i + j * nWidth] = 2;
			else
				m_gendata1[i + j * nWidth] = 0;
		}
	}

	for( auto& house : m_vecHouses )
	{
		if( !house.Generate( m_gendata, nWidth, nHeight, eType_House, eType_House_1, eType_House_2, eType_House_Exit1, eType_House_Exit2,
			m_gendata1, eType_None, eType_Temp0, m_par ) )
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
}

void CLevelGenNode2_1_2::GenObjs()
{
	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;

	vector<TVector2<int32> > vecTemp;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vecTemp );
	SRand::Inst().Shuffle( vecTemp );
	TVector2<int32> sizeMin[2] = { { 3, 1 }, { 1, 3 } };
	TVector2<int32> sizeMax[2] = { { 100, 2 }, { 2, 100 } };
	for( auto& p : vecTemp )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;

		int32 i = SRand::Inst().Rand( 0, 2 );
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin[i], sizeMax[i], TRectangle<int32>(), -1, eType_Chunk );
		if( !rect.width )
		{
			rect = PutRect( m_gendata, nWidth, nHeight, p, sizeMin[1 - i], sizeMax[1 - i], TRectangle<int32>(), -1, eType_Chunk );
			if( !rect.width )
				continue;
		}

		m_vecFences.push_back( rect );
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_None )
				m_gendata1[i + j * nWidth] = eType_Temp0;
		}
	}

	int8 nTypes[4] = { eType_Walkable_a, eType_Walkable_b, eType_Walkable_c, eType_Walkable_d };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 64, 128, eType_Temp0, nTypes, 4 );
}
