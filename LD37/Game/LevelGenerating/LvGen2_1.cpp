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

void CLevelGenNode2_2_0::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );


	m_gendata.clear();
	m_gendata1.clear();
	m_vecRoads.clear();
}

void CLevelGenNode2_2_0::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	uint8 nType = SRand::Inst().Rand( 0, 3 );
	m_nType = nType;

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
			SHouse house( nWidth / 2 - l, r0.GetBottom() + h, l + r, nHeight - r0.GetBottom() + h  );
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
		uint32 l2a = SRand::Inst().Rand( ( l - l2 - w2a - w2b ) / 2 - 1, ( l - l2 - w2a - w2b ) - ( l - l2 - w2a - w2b ) / 2 + 2 );
		uint32 l2b = l2a + w2a + l2;
		uint32 h2 = SRand::Inst().Rand( nHeight / 2 + 2, nHeight / 2 + 5 );
		TRectangle<int32> r5( l2a, nHeight - h2, w2a, h2 );
		TRectangle<int32> r6( l2b, nHeight - h2, w2b, h2 );

		{
			TRectangle<int32> r( r1.x, r1.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 1, 3 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
			r1.SetLeft( r.GetRight() );
			r3.SetTop( r.GetBottom() );
			m_vecArea2.push_back( r );
		}
		{
			TRectangle<int32> r( r2.GetRight() - 2, r2.y, 2, 2 );
			r.SetLeft( r.x - SRand::Inst().Rand( 2, 4 ) );
			r.SetTop( r.y - SRand::Inst().Rand( 1, 3 ) );
			r.SetRight( r.GetRight() + SRand::Inst().Rand( 2, 4 ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
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
			r.SetLeft( Max( r1.x, r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( nWidth / 2, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r5.y, r.y + SRand::Inst().Rand( 4, 8 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r5.width - 1 ) + r5.x - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r6.x, r0.GetBottom(), r6.width, 0 );
			r.SetLeft( Max( nWidth - nWidth / 2, r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r2.GetRight(), r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( Min( r6.y, r.y + SRand::Inst().Rand( 4, 8 ) ) );
			SHouse house( r );
			house.exit[1] = TVector2<int32>( SRand::Inst().Rand( 0, r6.width - 1 ) + r6.x - r.x, 2 );
			house.nExitType[1] = 2;
			m_vecHouses.push_back( house );
		}

		{
			TRectangle<int32> r( r3.x, r3.GetBottom(), r3.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( 0, r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( r5.x, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[3] = TVector2<int32>( SRand::Inst().Rand( 0, r3.width - 1 ) + r3.x - r.x, 2 );
			house.nExitType[3] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( r4.x, r4.GetBottom(), r4.width, Min<int32>( h1, SRand::Inst().Rand( 4, 7 ) ) );
			r.SetLeft( Max( r6.GetRight(), r.GetLeft() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetRight( Min( nWidth, r.GetRight() + SRand::Inst().Rand( 2, 4 ) ) );
			SHouse house( r );
			house.exit[3] = TVector2<int32>( SRand::Inst().Rand( 0, r4.width - 1 ) + r4.x - r.x, 2 );
			house.nExitType[3] = 2;
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
	else
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
			TRectangle<int32> r( 0, rc1.y, rc1.x, rc1.GetBottom() );
			r.SetTop( Max( 0, r.GetTop() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
			SHouse house( r );
			house.exit[2] = TVector2<int32>( SRand::Inst().Rand( 0, rc1.height - 1 ) + rc1.y - r.y, 2 );
			house.nExitType[2] = 2;
			m_vecHouses.push_back( house );
		}
		{
			TRectangle<int32> r( rc2.x, rc2.y, nWidth - rc2.x, rc2.GetBottom() );
			r.SetTop( Max( 0, r.GetTop() - SRand::Inst().Rand( 2, 4 ) ) );
			r.SetBottom( r.GetBottom() + SRand::Inst().Rand( 2, 4 ) );
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
				m_gendata[i + j * nWidth] = eType_Temp1;
			}
		}
	}

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

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_None, vec );
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_vecHouses.size() >= 8 )
			break;
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;
		TVector2<int32> ofs[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
		int i;
		for( i = 0; i < 4; i++ )
		{
			auto p1 = p + ofs[i];
			if( p1.x < 0 || p1.y < 0 || p1.x >= nWidth || p1.y >= nHeight )
				continue;
			if( m_gendata[p1.x + p1.y * nWidth] == eType_Road )
				break;
		}
		if( i >= 4 )
			continue;

		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 6, 6 ), TVector2<int32>( SRand::Inst().Rand( 8, 11 ), SRand::Inst().Rand( 8, 11 ) ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_House );
		if( rect.width > 0 )
			m_vecHouses.push_back( rect );
	}

	for( auto& house : m_vecHouses )
	{
		int32 l[4] = { house.rect.width, house.rect.height, house.rect.width, house.rect.height };
		int32 nLeft[4] = { 1, 0, 1, 0 };
		int32 nRight[4] = { 3, 2, 3, 2 };
		for( int i = 0; i < 4; i++ )
		{
			if( house.nExitType[i] )
				continue;
			int32 iLeft = nLeft[i];
			int32 iRight = nRight[i];
			int32 nLeft = house.nExitType[iLeft] == 2 ? 3 : 2;
			int32 nRight = house.nExitType[iRight] == 2 ? 3 : 2;
			if( 2 + nLeft + nRight < l[i] )
				continue;

			vector<TVector2<int32> > q;
			for( int j = nLeft; j < house.rect.width - nRight; j++ )
			{
				TVector2<int32> p[4] = { { 0, j }, { j, 0 }, { nWidth - 1, j }, { j, nHeight - 1 } };
				TVector2<int32> pt = p[i] + TVector2<int32>( house.rect.x, house.rect.y );
				q.push_back( pt );
			}

			if( !q.size() )
				continue;
			SRand::Inst().Shuffle( q );
			TVector2<int32> p = FindPath( m_gendata, nWidth, nHeight, eType_Temp0, eType_Temp0_0, eType_Road, q, m_par );
			if( p.x < 0 )
				continue;

			house.exit[i] = TVector2<int32>( nLeft, house.rect.width - nRight - nLeft );
			house.nExitType[i] = 1;
			vector<TVector2<int32> > q1;
			p = m_par[p.x + p.y * nWidth];
			while( p.x >= 0 && m_gendata[p.x + p.y * nWidth] == eType_Temp0_0 )
			{
				q1.push_back( p );
				p = m_par[p.x + p.y * nWidth];
			}
			m_gendata[p.x + p.y * nWidth] = eType_House_Exit1;

			ExpandDist( m_gendata, nWidth, nHeight, eType_Temp0_0, eType_Temp0, 2, q1 );
		}
	}
}
