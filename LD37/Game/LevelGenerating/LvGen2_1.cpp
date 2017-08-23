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

	struct SLess
	{
		bool operator () ( const TRectangle<int32>& l, const TRectangle<int32>& r )
		{
			return l.y < r.y;
		}
	};
	std::sort( m_vecCargoSmall.begin(), m_vecCargoSmall.end(), SLess() );

	for( auto& rect : m_vecCargoSmall )
	{
		auto rect1 = rect;
		while( rect1.y > 0 )
		{
			bool bBreak = false;
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				if( m_gendata1[i + ( rect1.y - 1 ) * nWidth] != eType1_None )
				{
					bBreak = true;
					break;
				}
			}
			if( bBreak )
				break;

			rect1.y--;
		}

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = m_gendata1[i + j * nWidth] = eType1_None;
			}
		}
		if( rect1.y > 0 )
		{
			rect = rect1;
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					m_gendata1[i + j * nWidth] = eType1_Obj;
				}
			}
		}
		else
			rect = TRectangle<int32>( 0, 0, 0, 0 );
	}

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
