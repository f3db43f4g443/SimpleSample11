#include "stdafx.h"
#include "LvGen1.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>

void CLevelGenNode1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1xNode = CreateNode( pXml->FirstChildElement( "block1x" )->FirstChildElement(), context );
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );
	m_pBonusNode = CreateNode( pXml->FirstChildElement( "bonus" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenRegions();
	GenBars();
	GenChunks();
	GenBlocks();

	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 x = region.x + i;
			int32 y = region.y + j;
			int8 genData = m_gendata[i + j * region.width];
			context.blueprint[x + y * context.nWidth] = genData;

			if( genData < eType_WallChunk )
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
		if( rect.height > 1 )
			m_pBar2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pBarNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_stones )
	{
		m_pStoneNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["mask"] = eType_Web;
	context.mapTags["0"] = eType_WallChunk1;
	for( auto& rect : m_wallChunks )
	{
		m_pWallChunkNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_bars.clear();
	m_stones.clear();
	m_wallChunks.clear();
}

void CLevelGenNode1_1::GenRegions()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	vector<TRectangle<int32> > vecRegions;
	TRectangle<int32> rect( 0, 0, nWidth, Min( nHeight, 4 ) );
	vecRegions.push_back( rect );
	while( rect.GetBottom() < nHeight - 4 )
	{
		auto rect1 = rect;
		bool bLeft = rect1.x == 0;
		bool bRight = rect1.GetRight() == nWidth;
		if( bLeft && bRight )
		{
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				rect1.SetLeft( nWidth / 4 + SRand::Inst().Rand( 0, 4 ) );
				rect1.SetRight( nWidth - ( nWidth / 4 + SRand::Inst().Rand( 0, 4 ) ) );
				rect1.y = rect.GetBottom() - 1;
				rect1.SetBottom( rect.GetBottom() + SRand::Inst().Rand( 9, 13 ) );
			}
			else
			{
				if( SRand::Inst().Rand( 0, 2 ) )
					rect1.SetLeft( nWidth / 3 + SRand::Inst().Rand( -2, 3 ) );
				else
					rect1.SetRight( nWidth - ( nWidth / 3 + SRand::Inst().Rand( -2, 3 ) ) );
				rect1.height = rect1.width + SRand::Inst().Rand( -2, 3 );
			}
		}
		else if( bLeft )
		{
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				rect1.SetLeft( Max( nWidth / 4, Min( nWidth * 2 / 5, rect1.GetRight() - nWidth / 2 + SRand::Inst().Rand( -3, 4 ) ) ) );
				rect1.SetRight( nWidth );
				rect1.y += ( rect1.height + SRand::Inst().Rand( -1, 3 ) ) / 2;
				rect1.height = rect1.width + SRand::Inst().Rand( -2, 3 );
			}
			else
			{
				rect1.SetLeft( rect1.GetRight() - Min( rect1.width / 2, SRand::Inst().Rand( 8, 12 ) ) );
				rect1.y += SRand::Inst().Rand( rect.height / 2, rect.height );
				rect1.SetBottom( rect.GetBottom() + SRand::Inst().Rand( 8, 11 ) );
			}
		}
		else if( bRight )
		{
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				rect1.SetRight( nWidth - Max( nWidth / 4, Min( nWidth * 2 / 5, nWidth / 2 - rect1.x + SRand::Inst().Rand( -3, 4 ) ) ) );
				rect1.SetLeft( 0 );
				rect1.y += ( rect1.height + SRand::Inst().Rand( -1, 3 ) ) / 2;
				rect1.height = rect1.width + SRand::Inst().Rand( -2, 3 );
			}
			else
			{
				rect1.SetRight( rect1.GetLeft() + Min( rect1.width / 2, SRand::Inst().Rand( 8, 12 ) ) );
				rect1.y += SRand::Inst().Rand( rect.height / 2, rect.height );
				rect1.SetBottom( rect.GetBottom() + SRand::Inst().Rand( 8, 11 ) );
			}
		}
		else
		{
			if( SRand::Inst().Rand( 0, 2 ) == 0 )
			{
				rect1.SetLeft( 0 );
				rect1.SetRight( nWidth );
				rect1.y = rect.GetBottom() - 1;
				rect1.SetBottom( rect.GetBottom() + SRand::Inst().Rand( 6, 10 ) );
			}
			else
			{
				if( SRand::Inst().Rand( rect.x, rect.GetRight() ) < nWidth )
					rect1.SetLeft( 0 );
				else
					rect1.SetRight( nWidth );
				rect1.y = Min( rect.GetBottom() - 1, rect.y + SRand::Inst().Rand( 8, 11 ) );
				rect1.height = Min( nHeight - rect1.y, rect1.width + SRand::Inst().Rand( -2, 3 ) );
				auto& r = vecRegions.back();
				r.height += SRand::Inst().Rand( 0, rect1.height / 3 );
			}
		}

		rect1.height = Min( nHeight - rect1.y, rect1.height );
		vecRegions.push_back( rect1 );
		rect1.SetTop( rect.GetBottom() );
		rect = rect1;
	}
	for( auto& r : vecRegions )
	{
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			for( int j = r.y; j < r.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = m_gendata[i + j * nWidth] == 0 ? eType_Temp : eType_Temp1;
			}
		}
	}
	SRand::Inst().Shuffle( vecRegions );
	for( auto rect : vecRegions )
	{
		for( int j = rect.y; j < rect.GetBottom(); j += rect.height - 1 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp )
					m_gendata[i + j * nWidth] = eType_Temp2;
			}
		}
		for( int i = rect.x; i < rect.GetRight(); i += rect.width - 1 )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp )
					m_gendata[i + j * nWidth] = eType_Temp2;
			}
		}
	}
	for( auto rect : vecRegions )
	{
		for( int j = rect.y; j < rect.GetBottom(); j += rect.height - 1 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp1 )
				{
					if( ( i > 0 && m_gendata[i - 1 + j * nWidth] == eType_Temp2 || i < nWidth - 1 && m_gendata[i + 1 + j * nWidth] == eType_Temp2 )
						&& ( j > 0 && m_gendata[i + ( j - 1 ) * nWidth] == eType_Temp2 || j < nHeight - 1 && m_gendata[i + ( j + 1 ) * nWidth] == eType_Temp2 ) )
						m_gendata[i + j * nWidth] = eType_Temp2;
				}
			}
		}
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp1 )
				{
					if( i == 0 || i == nWidth - 1 )
						m_gendata[i + j * nWidth] = eType_Temp2;
					else
						m_gendata[i + j * nWidth] = eType_Temp;
				}
			}
		}
	}

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Temp2, vec );
	SRand::Inst().Shuffle( vec );
	for( auto p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp2 )
			continue;
		if( !( p.y > 0 && m_gendata[p.x + ( p.y - 1 ) * nWidth] == eType_Temp2 || p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Temp2 ) )
			continue;
		auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 1 ), TVector2<int32>( nWidth, 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp2 );
		if( r.width <= 0 )
			continue;
		auto r1 = r;
		for( ; r1.x > 0; r1.SetLeft( r1.GetLeft() - 1 ) )
		{
			auto nType = m_gendata[r1.x - 1 + r1.y * nWidth];
			if( nType != 0 && nType != eType_Temp && nType != eType_Temp1 )
				break;
		}
		for( ; r1.GetRight() < nWidth; r1.width++ )
		{
			auto nType = m_gendata[r1.GetRight() + r1.y * nWidth];
			if( nType != 0 && nType != eType_Temp && nType != eType_Temp1 )
				break;
		}
		if( r.x > r1.x )
			r.SetLeft( r.GetLeft() - 1 );
		if( r.GetRight() < r1.GetRight() )
			r.width++;
		r1.SetLeft( r.x - Min( 8, ( r.x - r1.x ) / 2 ) );
		r1.SetRight( r.GetRight() + Min( 8, ( r1.GetRight() - r.GetRight() ) / 2 ) );
		if( r.width >= SRand::Inst().Rand( 12, 16 ) )
		{
			int32 w1 = Min( SRand::Inst().Rand( 5, r.width ), r.width - 9 );
			int32 x1 = Min( r1.GetRight() - 3 - w1, Max( r1.x + 4, SRand::Inst().Rand( r.x + 3, r.GetRight() - 2 - w1 ) ) );
			r1.SetLeft( Max( r1.x, Min( x1 - SRand::Inst().Rand( 6, 9 ), r.x - SRand::Inst().Rand( 0, 4 ) ) ) );
			r1.SetRight( Min( r1.GetRight(), Max( x1 + w1 + SRand::Inst().Rand( 6, 9 ), r.GetRight() + SRand::Inst().Rand( 0, 4 ) ) ) );
			for( int x = r1.x; x < r1.GetRight(); x++ )
			{
				if( x >= x1 && x < x1 + w1 )
					m_gendata[x + r1.y * nWidth] = eType_Temp;
				else
					m_gendata[x + r1.y * nWidth] = eType_Temp3;
			}
		}
		else
		{
			int32 w1 = SRand::Inst().Rand( r.width, Min( Max( r.width, SRand::Inst().Rand( 8, 13 ) ), r1.width ) + 1 );
			r1.SetLeft( Max( r1.x, r.GetRight() - w1 ) );
			r1.SetRight( Min( r1.GetRight(), r.x + w1 ) );
			r.width = w1;
			r.x = SRand::Inst().Rand( r1.x, r1.GetRight() - w1 + 1 );
			for( int x = r.x; x < r.GetRight(); x++ )
				m_gendata[x + r.y * nWidth] = eType_Temp3;
		}
	}

	vector<TRectangle<int32> > vecRegions1;
	for( auto p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp2 )
			continue;
		auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 1, 4 ), TVector2<int32>( 1, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp3 );
		if( r.width <= 0 )
			continue;
		vecRegions1.push_back( r );
	}
	vecRegions.clear();
	for( int i = 0; i < vecRegions1.size(); i++ )
	{
		int32 i1 = SRand::Inst().Rand<int32>( i, vecRegions1.size() );
		if( i1 > i )
			swap( vecRegions1[i1], vecRegions1[i] );
		auto r = vecRegions1[i];
		if( r.height >= Min( SRand::Inst().Rand( 10, 17 ), SRand::Inst().Rand( 10, 17 ) ) )
		{
			TRectangle<int32> r1[2] = { r, r };
			r1[0].height = SRand::Inst().Rand( 4, r.height - 4 );
			r1[1].SetTop( r1[0].GetBottom() + 1 );
			for( int k = 0; k < 2; k++ )
			{
				auto& rect = r1[k];
				int32 a = SRand::Inst().Rand( Max( -3, -r.x ), Min( 3, nWidth - r.GetRight() ) );
				if( a == 0 )
					continue;
				int32 n = a > 0 ? a : -a;
				a = a > 0 ? 1 : -1;
				for( ; n > 0; n-- )
				{
					bool b = true;
					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						if( m_gendata[rect.x + a + j * nWidth] > eType_Temp )
						{
							b = false;
							break;
						}
					}
					if( !b )
						continue;
					if( k == 0 )
					{
						if( rect.y > 0 && m_gendata[rect.x + a + ( rect.y - 1 ) * nWidth] != eType_Temp3 )
							continue;
					}
					else
					{
						if( rect.y > 0 && m_gendata[rect.x + a + ( rect.y - 1 ) * nWidth] != eType_Temp3 )
							continue;
					}

					for( int j = rect.y; j < rect.GetBottom(); j++ )
					{
						m_gendata[rect.x + j * nWidth] = m_gendata[rect.x + a + j * nWidth] == eType_Temp ? 0 : eType_Temp;
						m_gendata[rect.x + a + j * nWidth] = eType_Temp3;
					}
					rect.x += a;
				}
			}
			int32 x1 = Min( r.x, Min( r1[0].x, r1[1].x ) );
			int32 x2 = Max( r.x, Max( r1[0].x, r1[1].x ) );
			int32 x10 = x1, x20 = x2;
			for( ; x1 > 0; x1-- )
			{
				if( m_gendata[x1 - 1 + r1[0].GetBottom() * nWidth] >= eType_Temp2 )
					break;
			}
			for( ; x2 < nWidth - 1; x2++ )
			{
				if( m_gendata[x2 + 1 + r1[0].GetBottom() * nWidth] >= eType_Temp2 )
					break;
			}
			int32 w = Min( x2 - x1 + 1, SRand::Inst().Rand( 4, 8 ) );
			x1 = Max( x1, x20 - w + 1 );
			x2 = Min( x2, x10 + w - 1 );
			int32 x0 = SRand::Inst().Rand( x1, x2 - w + 2 );
			for( int x = x0; x < x0 + w; x++ )
				m_gendata[x + r1[0].GetBottom() * nWidth] = eType_Temp3;
			vecRegions1.push_back( r1[0] );
			vecRegions1.push_back( r1[1] );
		}
		else
			vecRegions.push_back( r );
	}

	vec.clear();
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp && (
				i > 0 && m_gendata[i - 1 + j * nWidth] == 0 ||
				i < nWidth - 1 && m_gendata[i + 1 + j * nWidth] == 0 ||
				j > 0 && m_gendata[i + ( j - 1 ) * nWidth] == 0 ||
				j < nHeight - 1 && m_gendata[i + ( j + 1 ) * nWidth] == 0 ) )
			{
				m_gendata[i + j * nWidth] = eType_Temp1;
				vec.push_back( TVector2<int32>( i, j ) );
			}
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp1 )
			continue;
		vector<TVector2<int32> > q;
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, eType_Temp2, q );
		FloodFillExpand( m_gendata, nWidth, nHeight, eType_Temp2, 0, q.size() * 2 + 8, q );
		TRectangle<int32> r( q[0].x, q[0].y, 1, 1 );
		for( int i = 1; i < q.size(); i++ )
			r = r + TRectangle<int32>( q[i].x, q[i].y, 1, 1 );
		TVector2<int32> ofs;
		bool bVertical = r.width > r.height;
		if( bVertical )
		{
			if( p.y - r.y > r.GetBottom() - 1 - p.y )
				ofs = TVector2<int32>( 0, -1 );
			else
				ofs = TVector2<int32>( 0, 1 );
		}
		else
		{
			if( p.x - r.x > r.GetRight() - 1 - p.x )
				ofs = TVector2<int32>( -1, 0 );
			else
				ofs = TVector2<int32>( 1, 0 );
		}
		for( auto p1 : q )
		{
			p1 = p1 + ofs;
			int32 l = 0;
			if( p1.x >= 0 && p1.y >= 0 && p1.x < nWidth && p1.y < nHeight && m_gendata[p1.x + p1.y * nWidth] == 0 )
			{
				auto r1 = PutRect( m_gendata, nWidth, nHeight, p1, bVertical? TVector2<int32>( 1, 4 ) : TVector2<int32>( 4, 1 ),
					bVertical ? TVector2<int32>( 1, nHeight ) : TVector2<int32>( nWidth, 1 ),
					TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
				if( r1.width > 0 )
				{
					int32 l = bVertical ? r1.height : r1.width;
					int32 l1 = bVertical ? SRand::Inst().Rand( 5, 8 ) : SRand::Inst().Rand( 7, 11 );
					if( l > l1 + 2 )
						l = l1;
					if( bVertical )
					{
						auto p2 = p1 + ofs * l;
						if( p2.y >= 0 && p2.y < nHeight && !m_gendata[p2.x + p2.y * nWidth] )
						{
							PutRect( m_gendata, nWidth, nHeight, p1 + ofs * l1, TVector2<int32>( 4, 1 ), TVector2<int32>( SRand::Inst().Rand( 4, 7 ), 1 ),
								TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp3 );
						}
					}
					for( int i = 0; i < l; i++ )
					{
						m_gendata[p1.x + p1.y * nWidth] = eType_Temp3;
						p1 = p1 + ofs;
					}
					break;
				}
			}
		}
	}
	
	for( auto& rect : vecRegions )
	{
		for( int i = 0; i < 2; i++ )
		{
			auto r = rect;
			r.x += i * 2 - 1;
			if( r.x < 0 || r.x >= nWidth )
				continue;
			int8 nType = m_gendata[r.x + r.y * nWidth];
			if( nType != 0 && nType != eType_Temp )
				continue;
			auto r1 = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( 6, r.height ),
				TVector2<int32>( Max( 10, r.height + SRand::Inst().Rand( 0, 8 ) ), r.height ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp, nType );
			if( r1.width <= 0 )
				continue;
			FillRegion( r1 );
		}
	}

	vec.clear();
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] < eType_Temp2 && m_gendata[i + j * nWidth] != eType_Temp )
			{
				m_gendata[i + j * nWidth] = 0;
				if( ( i > 0 && m_gendata[i - 1 + j * nWidth] == eType_Temp3 || i < nWidth - 1 && m_gendata[i + 1 + j * nWidth] == eType_Temp3 )
					&& ( j > 0 && m_gendata[i + ( j - 1 ) * nWidth] == eType_Temp3 || j < nHeight - 1 && m_gendata[i + ( j + 1 ) * nWidth] == eType_Temp3 ) )
					vec.push_back( TVector2<int32>( i, j ) );
			}
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 6 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight), -1, eType_Temp );
		if( rect.width <= 0 )
			continue;

		FillRegion( rect );
	}
	FillEmptyArea();
}

void CLevelGenNode1_1::FillRegion( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 s = 0;
	if( rect.x > 0 )
		s -= rect.height;
	if( rect.y > 0 )
		s -= rect.width;
	if( rect.GetRight() < nWidth )
		s -= rect.height;
	if( rect.GetBottom() < nHeight )
		s -= rect.width;
	s /= 2;
	for( int j = rect.y - 1; j <= rect.GetBottom(); j += rect.height + 1 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			if( j >= 0 && j < nHeight && m_gendata[i + j * nWidth] != eType_Temp3 )
				s++;
		}
	}
	for( int i = rect.x - 1; i <= rect.GetRight(); i += rect.width + 1 )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( i >= 0 && i < nWidth && m_gendata[i + j * nWidth] != eType_Temp3 )
				s++;
		}
	}
	int8 b[8];
	if( s > 0 )
	{
		CalcB( b, rect );

		int8 n[4] = { 0, 1, 2, 3 };
		SRand::Inst().Shuffle( n, 4 );
		for( int i = 0; i < 4 && s > 0; i++ )
		{
			int8 nTypes[2] = { n[i], n[i] + 4 };
			if( rect.width - nTypes[0] < rect.height - nTypes[1] + SRand::Inst().Rand( 0, 2 ) )
				swap( nTypes[0], nTypes[1] );
			for( int j = 0; j < 2 && s > 0; j++ )
			{
				int8 nType = nTypes[j];
				int32 x0 = !( nType & 1 ) ? rect.x - 1 : rect.GetRight();
				if( x0 < 0 || x0 >= nWidth )
					continue;
				int32 y0 = !( nType & 2 ) ? rect.y - 1 : rect.GetBottom();
				if( y0 < 0 || y0 >= nHeight )
					continue;
				if( !!( nType & 4 ) )
				{
					int32 nEnd = !( nType & 2 ) ? rect.GetBottom() : rect.y - 1;
					int32 d = !( nType & 2 ) ? 1 : -1;
					int32 nLen = SRand::Inst().Rand( 4, 7 );
					int32 l = 0;
					for( int y = y0; y != nEnd && ( s > 0 || l <= nLen ); y += d, l++ )
					{
						if( m_gendata[x0 + y * nWidth] != eType_Temp3 )
						{
							m_gendata[x0 + y * nWidth] = eType_Temp3;
							s--;
						}
					}
				}
				else
				{
					int32 nEnd = !( nType & 1 ) ? rect.GetRight() : rect.x - 1;
					int32 d = !( nType & 1 ) ? 1 : -1;
					int32 nLen = SRand::Inst().Rand( 4, 7 );
					int32 l = 0;
					for( int x = x0; x != nEnd && ( s > 0 || l <= nLen ); x += d, l++ )
					{
						if( m_gendata[x + y0 * nWidth] != eType_Temp3 )
						{
							if( l > nLen && m_gendata[x + y0 * nWidth] == eType_Temp2 )
								break;
							m_gendata[x + y0 * nWidth] = eType_Temp3;
							s--;
						}
					}
				}
			}
		}
	}

	if( Max( rect.width, rect.height ) > 12 && rect.width * rect.height >= SRand::Inst().Rand( 85, 110 ) )
	{
		TRectangle<int32> r[2] = { rect, rect };
		if( rect.width > rect.height )
		{
			int32 w = ( rect.width + SRand::Inst().Rand( -1, 1 ) ) / 2;
			r[0].width = w;
			r[1].SetLeft( r[0].GetRight() + 1 );
		}
		else
		{
			int32 h = ( rect.height + SRand::Inst().Rand( -1, 1 ) ) / 2;
			r[0].height = h;
			r[1].SetTop( r[0].GetBottom() + 1 );
		}
		FillRegion( r[0] );
		FillRegion( r[1] );
		return;
	}
	CalcB( b, rect );
	if( rect.height >= 9 && rect.width >= 7 )
	{
		if( b[4] > rect.height || b[5] > rect.height )
		{
			int32 n1 = b[4] * 65536 + Min( b[0], b[2] );
			int32 n2 = b[5] * 65536 + Min( b[1], b[3] );
			int8 bRight = n1 < n2 + SRand::Inst().Rand( 0, 2 );

			int32 w0 = b[0 + bRight];
			int32 w1 = b[2 + bRight];
			int32 n = ( rect.height + 1 ) / 5;
			int32 h1 = ( rect.height + 1 ) / n ;
			int8* h = (int8*)alloca( n );
			for( int i = 0; i < n; i++ )
				h[i] = h1;
			for( int i = rect.height - h1 * n; i >= 0; i-- )
				h[i]++;
			SRand::Inst().Shuffle( h, n );
			int32 y = -1;
			for( int i = 0; i < n - 1; i++ )
			{
				y += h[i];
				int32 w = ( w1 * y + w0 * ( rect.height + 1 - y ) + SRand::Inst().Rand( 0, rect.height + 1 ) ) / ( rect.height + 1 );
				w = Max( 3, Min( w, ( rect.width + SRand::Inst().Rand( 0, 4 ) ) * 2 / 3 ) );
				for( int x = 0; x < w; x++ )
				{
					if( bRight )
						m_gendata[rect.GetRight() - 1 - x + ( y + rect.y ) * nWidth] = eType_Temp3;
					else
						m_gendata[x + rect.x + ( y + rect.y ) * nWidth] = eType_Temp3;
				}
			}
			return;
		}
	}
}

void CLevelGenNode1_1::CalcB( int8* b, const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	memset( b, 0, 8 );
	if( rect.x > 0 && rect.y > 0 && m_gendata[rect.x - 1 + ( rect.y - 1 ) * nWidth] == eType_Temp3 )
	{
		b[0]++;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			if( m_gendata[x + ( rect.y - 1 ) * nWidth] != eType_Temp3 )
				break;
			b[0]++;
		}
		b[4]++;
		for( int y = rect.y; y < rect.GetBottom(); y++ )
		{
			if( m_gendata[rect.x - 1 + y * nWidth] != eType_Temp3 )
				break;
			b[4]++;
		}
	}
	if( rect.GetRight() < nWidth && rect.y > 0 && m_gendata[rect.GetRight() + ( rect.y - 1 ) * nWidth] == eType_Temp3 )
	{
		b[1]++;
		for( int x = rect.GetRight() - 1; x >= rect.x; x-- )
		{
			if( m_gendata[x + ( rect.y - 1 ) * nWidth] != eType_Temp3 )
				break;
			b[1]++;
		}
		b[5]++;
		for( int y = rect.y; y < rect.GetBottom(); y++ )
		{
			if( m_gendata[rect.GetRight() + y * nWidth] != eType_Temp3 )
				break;
			b[5]++;
		}
	}
	if( rect.x > 0 && rect.GetBottom() < nHeight && m_gendata[rect.x - 1 + rect.GetBottom() * nWidth] == eType_Temp3 )
	{
		b[2]++;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			if( m_gendata[x + rect.GetBottom() * nWidth] != eType_Temp3 )
				break;
			b[2]++;
		}
		b[6]++;
		for( int y = rect.GetBottom() - 1; y >= rect.y; y-- )
		{
			if( m_gendata[rect.x - 1 + y * nWidth] != eType_Temp3 )
				break;
			b[6]++;
		}
	}
	if( rect.GetRight() < nWidth && rect.GetBottom() < nHeight && m_gendata[rect.GetRight() + rect.GetBottom() * nWidth] == eType_Temp3 )
	{
		b[3]++;
		for( int x = rect.GetRight() - 1; x >= rect.x; x-- )
		{
			if( m_gendata[x + rect.GetBottom() * nWidth] != eType_Temp3 )
				break;
			b[3]++;
		}
		b[7]++;
		for( int y = rect.GetBottom() - 1; y >= rect.y; y-- )
		{
			if( m_gendata[rect.GetRight() + y * nWidth] != eType_Temp3 )
				break;
			b[7]++;
		}
	}
}

void CLevelGenNode1_1::FillEmptyArea()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int32> vecDist;
	vecDist.resize( nWidth * nHeight );
	vector<TVector2<int32> > q;
	GenDistField( m_gendata, nWidth, nHeight, eType_Temp, vecDist, q );

	for( int i = q.size() - 1; i >= 0; i-- )
	{
		auto p = q[i];
		int32 nDist = vecDist[p.x + p.y * nWidth];
		if( nDist < 5 )
			continue;
		TRectangle<int32> r( p.x - nDist + 1, p.y - nDist + 1, nDist * 2 - 1, nDist * 2 - 1 );
		r = r * TRectangle<int32>( 0, 0, nWidth, nHeight );
		r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( r.width, r.height ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp, eType_Temp );
		for( int x = r.x; x < r.GetRight(); x++ )
		{
			for( int y = r.y; y < r.GetBottom(); y++ )
				vecDist[x + y * nWidth] = -1;
		}
		if( r.width * r.height < 120 )
			continue;

		vector<int32> vecY;
		for( int y = r.y + 2; y < r.GetBottom() - 2; y++ )
		{
			vecY.push_back( y );
		}
		int32 y1 = SRand::Inst().Rand( r.y + 2, r.GetBottom() - 2 ), y2 = SRand::Inst().Rand( r.y + 2, r.GetBottom() - 2 );
		SRand::Inst().Shuffle( vecY );
		for( auto y : vecY )
		{
			if( r.x > 0 && m_gendata[r.x - 1 + y * nWidth] != eType_Temp )
			{
				y1 = y;
				break;
			}
		}
		SRand::Inst().Shuffle( vecY );
		for( auto y : vecY )
		{
			if( r.GetRight() < nWidth && m_gendata[r.GetRight() + y * nWidth] != eType_Temp )
			{
				y2 = y;
				break;
			}
		}
		if( abs( y1 - y2 ) <= 2 )
		{
			if( SRand::Inst().Rand( 0, 2 ) )
				y1 = y2;
			else
				y2 = y1;
		}
		for( int x = Min( SRand::Inst().Rand( 5, 9 ), SRand::Inst().Rand( r.width / 2, r.width * 3 / 4 ) ); x >= 0; x-- )
			m_gendata[r.x + x + y1 * nWidth] = eType_Temp3;
		for( int x = Min( SRand::Inst().Rand( 5, 9 ), SRand::Inst().Rand( r.width / 2, r.width * 3 / 4 ) ); x >= 0; x-- )
			m_gendata[r.GetRight() - 1 - x + y2 * nWidth] = eType_Temp3;
		if( abs( y1 - y2 ) >= 6 )
		{
			TRectangle<int32> r1( r.x, Min( y1, y2 ) + 1, r.width, Max( y1, y2 ) - Min( y1, y2 ) - 1 );
			r1.SetLeft( r1.x + Min( SRand::Inst().Rand( 2, 5 ), r.width / 2 - 2 ) );
			r1.SetRight( r1.GetRight() - Min( SRand::Inst().Rand( 2, 5 ), r.width / 2 - 2 ) );
			FillRegion( r1 );
		}

		vector<int32> vecX;
		for( int x = r.x + 1; x < r.GetRight() - 1; x++ )
		{
			vecX.push_back( x );
		}
		int32 x1 = r.x + ( r.width - SRand::Inst().Rand( 0, 2 ) ) / 2, x2 = r.x + ( r.width - SRand::Inst().Rand( 0, 2 ) ) / 2;
		SRand::Inst().Shuffle( vecX );
		for( auto x : vecX )
		{
			if( r.y > 0 && m_gendata[x + ( r.y - 1 ) * nWidth] != eType_Temp )
			{
				x1 = x;
				break;
			}
		}
		SRand::Inst().Shuffle( vecX );
		for( auto x : vecX )
		{
			if( r.GetBottom() < nHeight && m_gendata[x + r.GetBottom() * nWidth] != eType_Temp )
			{
				x2 = x;
				break;
			}
		}

		int32 a = Min( y1, y2 );
		for( int y = r.y; y <= a - 4; )
		{
			int32 yy = Min( y + ( y == r.y ? SRand::Inst().Rand( 1, 4 ) : SRand::Inst().Rand( 4, 9 ) ), a );
			if( y == r.y || !SRand::Inst().Rand( 0, 2 ) )
			{
				for( int j = y; j < yy; j++ )
					m_gendata[x1 + j * nWidth] = eType_Temp3;
			}
			y = yy;
			if( y < a - 4 )
			{
				auto r1 = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x1, y ), TVector2<int32>( 1, 4 ),
					TVector2<int32>( 1, SRand::Inst().Rand( 4, 7 ) ), r, -1, eType_Temp3  );
				if( r1.width <= 0 )
					break;
				x1 = SRand::Inst().Rand( r1.x, r1.GetRight() );
				y++;
			}
		}

		a = Max( y1, y2 );
		for( int y = r.GetBottom() - 1; y >= a + 4; )
		{
			int32 yy = Max( y - ( y == r.GetBottom() - 1 ? SRand::Inst().Rand( 1, 4 ) : SRand::Inst().Rand( 4, 9 ) ), a );
			if( y == r.GetBottom() - 1 || !SRand::Inst().Rand( 0, 2 ) )
			{
				for( int j = y; j > yy; j-- )
					m_gendata[x2 + j * nWidth] = eType_Temp3;
			}
			y = yy;
			if( y > a + 4 )
			{
				auto r1 = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x2, y ), TVector2<int32>( 1, 4 ),
					TVector2<int32>( 1, SRand::Inst().Rand( 4, 7 ) ), r, -1, eType_Temp3 );
				if( r1.width <= 0 )
					break;
				x2 = SRand::Inst().Rand( r1.x, r1.GetRight() );
				y--;
			}
		}
	}
}

void CLevelGenNode1_1::GenBars()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			if( m_gendata[x + y * nWidth] < eType_Temp2 )
				m_gendata[x + y * nWidth] = 0;
		}
	}
	for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			if( m_gendata[x + y * nWidth] != eType_Temp3 )
				continue;
			if( y == 0 )
			{
				m_gendata[x + y * nWidth] = m_gendata[x + ( y + 1 ) * nWidth] == eType_Temp3 ? eType_Temp3 : 0;
			}
			else
			{
				auto r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 4, 1 ), TVector2<int32>( nWidth, 1 ),
					TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp );
				if( r.width <= 0 )
					continue;
				m_bars.push_back( r );
			}
		}
	}
	SRand::Inst().Shuffle( m_bars );
	vector<int32> vec;
	vec.resize( m_gendata.size() );
	for( int i = 0; i < m_bars.size(); i++ )
	{
		auto& rect = m_bars[i];
		for( int x = rect.x; x < rect.GetRight(); x++ )
			vec[x + rect.y * nWidth] = i + 1;
	}
	vector<int8> vecVisitFlag;
	vecVisitFlag.resize( m_bars.size() );
	vector<TVector2<int32> > q;
	for( int i = 0; i < m_bars.size(); i++ )
	{
		if( vecVisitFlag[i] )
			continue;
		if( m_bars[i].width <= 0 )
			continue;
		FloodFill( m_gendata, nWidth, nHeight, m_bars[i].x, m_bars[i].y, eType_Temp1, q );
		vector<int32> bars;
		int32 yMin = nHeight, yMax = -1;
		for( auto& p : q )
		{
			int32 n = vec[p.x + p.y * nWidth] - 1;
			if( n < 0 || vecVisitFlag[n] )
				continue;
			vecVisitFlag[n] = 1;
			bars.push_back( n );
			yMin = Min( yMin, m_bars[n].y );
			yMax = Max( yMax, m_bars[n].y );
		}
		SRand::Inst().Shuffle( bars );
		while( yMax > yMin )
		{
			int32 nMinPrice = 0x7fffffff;
			int32 nBar = -1;
			for( auto n : bars )
			{
				auto& rect = m_bars[n];
				if( rect.width <= 0 )
					continue;
				int32 nDir = rect.y == yMin ? 1 : ( rect.y == yMax ? -1 : 0 );
				if( !nDir )
					continue;
				int32 h1 = nHeight, h2 = nHeight;
				int32 x;
				for( x = rect.x; x < rect.GetRight(); x++ )
				{
					int32 y = rect.y + nDir;
					int8 nType = m_gendata[x + y * nWidth];
					if( nType == eType_Temp )
						break;
					if( nType == eType_Temp1 )
						continue;
					while( y >= 0 && y < nHeight )
					{
						if( nType == eType_Temp3 )
						{
							if( m_gendata[x + y * nWidth] != eType_Temp3 )
								break;
						}
						else
						{
							if( m_gendata[x + y * nWidth] != 0 && m_gendata[x + y * nWidth] != eType_Temp2 )
								break;
						}
						y += nDir;
					}
					if( nType == eType_Temp1 )
						h1 = Min( h1, abs( y - rect.y ) - 1 );
					else
						h2 = Min( h2, abs( y - rect.y ) - 1 );
				}
				if( x >= rect.GetRight() )
				{
					int32 nPrice = Max( 0, 5 - h1 ) * 100 + Max( 0, 4 - h2 ) * 10 - h1 * 2 - h2;
					if( nPrice < nMinPrice )
					{
						nMinPrice = nPrice;
						nBar = n;
					}
				}
			}
			if( nBar == -1 )
				break;

			auto& rect = m_bars[nBar];
			int32 nDir = rect.y == yMin ? 1 : ( rect.y == yMax ? -1 : 0 );
			int32 nMaxX = rect.GetRight();
			for( int x = rect.x; x < nMaxX; x++ )
			{
				m_gendata[x + ( rect.y + nDir ) * nWidth] = eType_Temp1;
				int8 nType0 = rect.y - nDir >= 0 && rect.y - nDir < nHeight ? m_gendata[x + ( rect.y - nDir ) * nWidth] : 0;
				if( nType0 == eType_Temp || nType0 == eType_Temp1 )
					nType0 = 0;
				m_gendata[x + rect.y * nWidth] = nType0;
				vec[x + rect.y * nWidth] = 0;
				int32& nBar1 = vec[x + ( rect.y + nDir ) * nWidth];
				if( nBar1 > 0 && nBar1 != nBar + 1 )
				{
					auto& rect1 = m_bars[nBar1 - 1];
					for( int x1 = rect1.x; x1 < rect1.GetRight(); x1++ )
						vec[x1 + rect1.y * nWidth] = nBar + 1;
					rect1.width = 0;
					rect.SetLeft( Min( rect.GetLeft(), rect1.GetLeft() ) );
					rect.SetRight( Max( rect.GetRight(), rect1.GetRight() ) );
				}
				else
					nBar1 = nBar + 1;
			}
			rect.y += nDir;
			yMin = nHeight;
			yMax = -1;
			for( auto n : bars )
			{
				auto& rect = m_bars[n];
				if( rect.width <= 0 )
					continue;
				yMin = Min( yMin, rect.y );
				yMax = Max( yMax, rect.y );
			}
		}
		q.clear();
		for( auto n : bars )
		{
			auto& rect = m_bars[n];
			if( rect.width <= 0 )
				continue;
			for( int32 x = rect.x; x < rect.GetRight(); x++ )
				m_gendata[x + rect.y * nWidth] = eType_Temp;
		}
	}
	m_bars.clear();

	for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 1; x < nWidth - 1; x++ )
		{
			if( m_gendata[x + y * nWidth] != eType_Temp
				&& m_gendata[x - 1 + y * nWidth] == eType_Temp
				&& m_gendata[x + 1 + y * nWidth] == eType_Temp )
				m_gendata[x + y * nWidth] = eType_Temp;
		}
	}

	for( int y = 1; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			if( m_gendata[x + y * nWidth] != eType_Temp )
				continue;
			auto r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 4, 1 ), TVector2<int32>( nWidth, 1 ),
				TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar );
			if( r.width <= 0 )
			{
				m_gendata[x + y * nWidth] = eType_Temp2;
				continue;
			}

			if( r.width >= SRand::Inst().Rand( 14, 17 ) )
			{
				int32 x1 = r.GetRight();
				int32 x2 = r.x;
				for( int i = r.x; i < r.GetRight(); i++ )
				{
					if( r.y > 0 && m_gendata[i + ( r.y - 1 ) * nWidth] == eType_Temp3
						|| r.y < nHeight - 1 && m_gendata[i + ( r.y + 1 ) * nWidth] == eType_Temp3 )
					{
						x1 = Min( x1, i );
						x2 = Max( x2, i + 1 );
					}
				}
				if( x1 >= x2 )
				{
					x1 = r.x + ( r.width - 1 ) / 2;
					x2 = r.GetRight() - ( r.width - 1 ) / 2;
				}
				else if( x2 == x1 + 1 )
				{
					if( r.y > 0 && m_gendata[x1 + ( r.y - 1 ) * nWidth] == eType_Temp3
						&& r.y < nHeight - 1 && m_gendata[x1 + ( r.y + 1 ) * nWidth] == eType_Temp3
						&& x1 - r.x >= 6 && r.GetRight() - x2 >= 6 )
					{
						m_gendata[x1 + r.y * nWidth] = eType_Temp3;
						m_bars.push_back( TRectangle<int32>( r.x, r.y, x1 - r.x, 1 ) );
						m_bars.push_back( TRectangle<int32>( x2, r.y, r.GetRight() - x2, 1 ) );
						continue;
					}
				}
				else
				{
					float lMax = 1;
					int32 xMax = -1;
					int32 lCur = 0;
					for( int i = x1; i < x2; i++ )
					{
						if( r.y > 0 && m_gendata[i + ( r.y - 1 ) * nWidth] == eType_Temp3
							|| r.y < nHeight - 1 && m_gendata[i + ( r.y + 1 ) * nWidth] == eType_Temp3 )
						{
							int32 left = Max( i - lCur, r.x + 4 );
							int32 right = Min( i, r.GetRight() - 4 );
							float l = right - left + SRand::Inst().Rand( 0.0f, 1.0f );
							if( l >= lMax )
							{
								lMax = l;
								xMax = i;
							}
							lCur = 0;
						}
						else
							lCur++;
					}
					if( lMax >= Min( r.width / 2, SRand::Inst().Rand( 5, 11 ) ) || x2 - x1 + 1 >= SRand::Inst().Rand( 14, 17 ) )
					{
						int32 l = floor( lMax );
						int32 left = Max( xMax - l, r.x + 4 );
						int32 right = Min( xMax, r.GetRight() - 4 );
						if( right - left >= 3 )
						{
							int32 w = SRand::Inst().Rand( 3, Min( 6, right - left ) + 1 );
							int32 x0 = SRand::Inst().Rand( left, right - w + 1 );
							m_bars.push_back( TRectangle<int32>( r.x, r.y, x0 - r.x, 1 ) );
							m_bars.push_back( TRectangle<int32>( x0 + w, r.y, r.GetRight() - x0 - w, 1 ) );
							for( int i = 0; i < w; i++ )
								m_gendata[i + x0 + r.y * nWidth] = eType_Temp2;
							continue;
						}
					}
				}

				int32 w1 = Max( Min( r.width - 1, SRand::Inst().Rand( 13, 16 ) ), x2 - x1 );
				int32 xa = Max( r.x, x2 - w1 );
				int32 xb = Min( r.GetRight(), x1 + w1 );
				int32 x0 = SRand::Inst().Rand( xa, xb - w1 + 1 );
				for( int i = r.x; i < x0; i++ )
					m_gendata[i + r.y * nWidth] = eType_Temp2;
				for( int i = x0 + w1; i < r.GetRight(); i++ )
					m_gendata[i + r.y * nWidth] = eType_Temp2;
				r.width = w1;
				r.x = x0;
			}
			m_bars.push_back( r );
		}
	}

	vector<TVector2<int32> > vecTemp;
	for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			if( x > 0 && x < nWidth - 1 && m_gendata[x + y * nWidth] != eType_Temp3
				&& m_gendata[x - 1 + y * nWidth] == eType_Temp3
				&& m_gendata[x + 1 + y * nWidth] == eType_Temp3 )
				m_gendata[x + y * nWidth] = eType_Temp3;
			if( m_gendata[x + y * nWidth] == eType_Temp3 )
				vecTemp.push_back( TVector2<int32>( x, y ) );
		}
	}
	SRand::Inst().Shuffle( vecTemp );
	for( auto& p : vecTemp )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp3 )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 1 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp3 );
		if( rect.width <= 0 )
			continue;
		TRectangle<int32> r0 = PutRect( m_gendata, nWidth, nHeight, TRectangle<int32>( rect.x, rect.y, 1, rect.height ),
			TVector2<int32>( 1, rect.height ), TVector2<int32>( 1, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp3, eType_Temp3 );
		TRectangle<int32> r1 = PutRect( m_gendata, nWidth, nHeight, TRectangle<int32>( rect.GetRight() - 1, rect.y, 1, rect.height ),
			TVector2<int32>( 1, rect.height ), TVector2<int32>( 1, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Temp3, eType_Temp3 );
		if( rect.width == 2 )
		{
			if( r0.height >= rect.height + 4 && r1.height >= rect.height + 4 )
			{
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					for( int y = rect.y; y < rect.GetBottom(); y++ )
						m_gendata[x + y * nWidth] = eType_Temp1;
				}
			}
			else if( r0.height < r1.height + SRand::Inst().Rand( 0, 2 ) )
			{
				for( int y = r0.y; y < r0.GetBottom(); y++ )
					m_gendata[r0.x + y * nWidth] = eType_Temp1;
			}
			else
			{
				for( int y = r1.y; y < r1.GetBottom(); y++ )
					m_gendata[r1.x + y * nWidth] = eType_Temp1;
			}
			continue;
		}
		if( r0.height < r1.height + SRand::Inst().Rand( 0, 2 ) )
		{
			for( int x = rect.x; x < rect.GetRight() - 1; x++ )
			{
				for( int y = rect.y; y < rect.GetBottom(); y++ )
					m_gendata[x + y * nWidth] = eType_Temp1;
			}
		}
		else
		{
			for( int x = rect.x + 1; x < rect.GetRight(); x++ )
			{
				for( int y = rect.y; y < rect.GetBottom(); y++ )
					m_gendata[x + y * nWidth] = eType_Temp1;
			}
		}
	}

	for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			if( m_gendata[x + y * nWidth] != eType_Temp3 )
				continue;
			auto r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 1, 1 ), TVector2<int32>( 1, nHeight ),
				TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar1 );
			if( r.height < 4 )
			{
				for( int i = r.y; i < r.GetBottom(); i++ )
					m_gendata[r.x + i * nWidth] = eType_Temp1;
				r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( 1, 4 ), TVector2<int32>( 1, SRand::Inst().Rand( 4, 7 ) ),
					TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar1, eType_Temp1 );
				if( !r.width )
					continue;
			}
			m_bars.push_back( r );
		}
	}
}

void CLevelGenNode1_1::GenChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec, vec1;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp1 )
			{
				m_gendata[i + j * nWidth] = 0;
				vec.push_back( TVector2<int32>( i, j ) );
			}
			else if( m_gendata[i + j * nWidth] == eType_Temp2 )
			{
				m_gendata[i + j * nWidth] = 0;
				if( j <= SRand::Inst().Rand( 4, 8 ) )
					continue;
				if( !SRand::Inst().Rand( 0, 16 ) )
					vec.push_back( TVector2<int32>( i, j ) );
				else
					vec1.push_back( TVector2<int32>( i, j ) );
			}
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& p : vec )
		{
			if( m_gendata[p.x + p.y * nWidth] != 0 )
				continue;
			if( i == 0 )
			{
				if( !( p.y > 0 && m_gendata[p.x + ( p.y - 1 ) * nWidth] == eType_Bar1 || p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Bar1 ) )
					continue;
			}
			auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 4, 3 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), SRand::Inst().Rand( 2, 5 ), eType_Stone );
			if( rect.width <= 0 )
			{
				m_gendata[p.x + p.y * nWidth] = eType_Temp2;
				continue;
			}
			m_stones.push_back( rect );
		}
	}
	GenWallChunks();
	for( auto& p : vec1 )
	{
		if( m_gendata[p.x + p.y * nWidth] == 0 )
			m_gendata[p.x + p.y * nWidth] = eType_Temp2;
	}

	vec.clear();
	for( auto& rect : m_bars )
	{
		if( rect.width > 0 )
		{
			for( int i = 0; i < 2; i++ )
			{
				int32 x = i == 0 ? rect.x - 1 : rect.GetRight();
				int32 nDir = i == 0 ? -1 : 1;
				for( int j = SRand::Inst().Rand( 4, 7 ); j > 0; j-- )
				{
					if( x < 0 || x >= nWidth )
						break;
					auto& nType = m_gendata[x + rect.y * nWidth];
					if( nType > 0 )
						break;
					nType = eType_Temp;
					vec.push_back( TVector2<int32>( x, rect.y ) );
					x += nDir;
				}
			}
		}
		else
		{
			for( int i = 0; i < 2; i++ )
			{
				int32 y = i == 0 ? rect.y - 1 : rect.GetBottom();
				int32 nDir = i == 0 ? -1 : 1;
				for( int j = SRand::Inst().Rand( 4, 7 ); j > 0; j-- )
				{
					if( y < 0 || y >= nHeight )
						break;
					auto& nType = m_gendata[rect.x + y * nWidth];
					if( nType == eType_Temp )
					{
						FloodFill( m_gendata, nWidth, nHeight, rect.x, y, eType_Temp2 );
						break;
					}
					else if( nType > 0 )
						break;
					nType = eType_Temp;
					vec.push_back( TVector2<int32>( rect.x, y ) );
					y += nDir;
				}
			}
		}
	}
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] == eType_Temp )
			m_gendata[p.x + p.y * nWidth] = 0;
	}
}

void CLevelGenNode1_1::GenWallChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	SRand::Inst().Shuffle( m_bars );
	struct SLess
	{
		bool operator () ( const TRectangle<int32>& left, const TRectangle<int32>& right )
		{
			return left.y < right.y;
		}
	};
	std::sort( m_bars.begin(), m_bars.end(), SLess() );
	for( auto& rect : m_bars )
	{
		if( rect.y < 8 )
			continue;
		if( rect.width > 1 )
		{
			int32 lCur = 0;
			int32 h = 0;
			int32 nType = 0;
			bool bNeed = true;
			for( int i = rect.x; i <= rect.GetRight(); i++ )
			{
				int32 h0 = h;
				int32 nType0 = nType;
				bool b = false;
				if( i == rect.GetRight() )
					b = true;
				else
				{
					int32 j = rect.y - 1;
					for( ; j >= 0; j-- )
					{
						if( m_gendata[i + j * nWidth] == eType_Bar || m_gendata[i + j * nWidth] == eType_WallChunk )
							break;
					}
					int32 h1 = rect.y - 1 - j;
					int8 nType1 = j < 0 ? -1 : m_gendata[i + j * nWidth];
					if( i == rect.x )
					{
						h = h1;
						nType = nType1;
					}
					if( h != h1 || nType != nType1 )
					{
						b = true;
						h = h1;
						nType = nType1;
					}
				}
				if( b )
				{
					int32 l = lCur;
					TRectangle<int32> r( i - lCur, rect.y - h0 - 1, lCur, 1 );
					if( r.y >= 0 )
					{
						r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( r.width, 1 ), TVector2<int32>( nWidth, 1 ),
							TRectangle<int32>( rect.x, r.y, rect.width, 1 ), -1, nType0, nType0 );
						l = r.width;
					}
					if( l * 12 >= rect.width * ( 6 + Max( 0, h0 - 4 ) ) )
					{
						bNeed = false;
						break;
					}
					lCur = 0;
				}
				else
					lCur++;
			}

			if( bNeed )
			{
				vector<int32> v;
				for( int i = rect.x; i < rect.GetRight(); i++ )
					v.push_back( i );
				SRand::Inst().Shuffle( v );
				int32 y = rect.y - 1;
				for( int32 x : v )
				{
					if( m_gendata[x + y * nWidth] != 0 )
						continue;
					auto r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 4, 4 ), TVector2<int32>( Max( 6, rect.width / 2 ), 4 ),
						TRectangle<int32>( rect.x, 0, rect.width, rect.y ), -1, eType_None );
					if( r.width )
					{
						r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( r.width, r.height ),
							TVector2<int32>( Min( 12, r.width + SRand::Inst().Rand( 2, 6 ) ), SRand::Inst().Rand( 5, 8 ) ),
							TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_WallChunk, eType_None );
						m_wallChunks.push_back( r );
						break;
					}
				}
			}
		}
	}
}

void CLevelGenNode1_1::GenBlocks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	TVector2<int32> ofs[] = { { -1, 0 }, { -1, 1 }, { -1, -1 }, { 1, 0 }, { 1, 1 }, { 1, -1 } };
	ConnectAll( m_gendata, nWidth, nHeight, eType_Temp2, 0, ofs, ELEM_COUNT( ofs ) );
	GenWallChunks1();

	int32 y0 = 4;
	int32 s = 0;
	int32 k = SRand::Inst().Rand( 5, 11 );
	for( int j = y0 + 1; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp2 )
				s++;
		}
		k--;
		if( k <= 0 )
		{
			int32 n = j - y0 - s;
			if( n >= 3 )
			{
				int8 bDir = SRand::Inst().Rand( 0, 2 );
				int32 y = j - ( j - y0 - SRand::Inst().Rand( 0, 2 ) ) / 2;
				for( int i = SRand::Inst().Rand( 0, Max( 1, nWidth / 2 - n ) ); i < nWidth && n > 0; i++ )
				{
					int32 x = bDir ? i : nWidth - 1 - i;
					if( m_gendata[x + y * nWidth] == 0 )
					{
						m_gendata[x + y * nWidth] = eType_Temp2;
						n--;
					}
					y = Min( j, Max( y0 + 1, y + SRand::Inst().Rand( -1, 2 ) ) );
				}
			}
			k = Min( Max( 4, nHeight - 1 - j ), SRand::Inst().Rand( 5, 11 ) );
			s = 0;
			y0 = j;
		}
	}

	ExpandDist( m_gendata, nWidth, nHeight, eType_Temp2, 0, 1 );

	for( auto& rect : m_wallChunks )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = 0;
		}
	}
	LvGenLib::GenObjs2( m_gendata, nWidth, nHeight, 0, eType_Temp2, 0.1f );
	s = 0;
	for( auto& rect : m_wallChunks )
	{
		vector<TVector2<int32> > v;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Temp2 )
					m_gendata[i + j * nWidth] = eType_Web;
				else
				{
					m_gendata[i + j * nWidth] = eType_WallChunk;
					v.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		s += rect.width * rect.height;
		int32 s1 = SRand::Inst().Rand( rect.width * rect.height / 8, rect.width * rect.height / 6 );
		SRand::Inst().Shuffle( v );
		for( auto p : v )
		{
			auto r1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ),
				rect, 5, eType_WallChunk1 );
			if( r1.width > 0 )
			{
				s1 -= r1.width * r1.height;
				if( s1 <= 0 )
					break;
			}
		}
	}
	FloodFillExpand( m_gendata, nWidth, nHeight, eType_Web, eType_WallChunk, s / 6 );

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = SRand::Inst().Rand( 4, 8 ); j >= 0; j-- )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp2 )
				m_gendata[i + j * nWidth] = 0;
		}
	}
	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Temp2, vec );
	SRand::Inst().Shuffle( vec );

	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] == eType_Temp2 )
		{
			int8 nType = ( SRand::Inst().Rand( 0, 2 ) ) + eType_Block1x;
			FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, nType );
		}
	}

	vec.clear();
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Block1x, vec );
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Block2x, vec );
	int32 nHoleCount = vec.size() * 0.12f;
	int32 nObjCount = vec.size() * 0.025f;
	int32 nBonusCount = vec.size() * 0.05f;
	SRand::Inst().Shuffle( vec );
	for( int i = 0; i < vec.size(); i++ )
	{
		TVector2<int32> p = vec[i];
		int8 nType = m_gendata[p.x + p.y * nWidth];
		if( nType == eType_Block1x || nType == eType_Block2x )
		{
			TVector2<int32> p1 = p;
			if( nType == eType_Block1x )
				p1.x = ( p1.x + p1.y ) & 1 ? p1.x - 1 : p1.x + 1;
			else
				p1.x = ( p1.x + p1.y + 1 ) & 1 ? p1.x - 1 : p1.x + 1;
			if( p1.x < 0 || p1.x >= nWidth )
				continue;
			if( !m_gendata[p1.x + p1.y * nWidth] )
				m_gendata[p1.x + p1.y * nWidth] = nType;
			else if( m_gendata[p1.x + p1.y * nWidth] != nType )
			{
				if( nBonusCount && p.y > 0 && m_gendata[p.x + ( p.y - 1 ) * nWidth] > eType_Bonus )
				{
					m_gendata[p.x + p.y * nWidth] = eType_Bonus;
					nBonusCount--;
				}
				continue;
			}

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

			m_gendata[p.x + p.y * nWidth] = m_gendata[p1.x + p1.y * nWidth] = 0;
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

void CLevelGenNode1_1::GenWallChunks1()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	int32 y0 = 8;
	int32 s = 0;
	int32 k = SRand::Inst().Rand( 7, 11 );
	vector<TVector2<int32> > vecTemp;
	for( int j = y0 + 1; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_WallChunk )
				s++;
			else if( m_gendata[i + j * nWidth] == 0 )
				vecTemp.push_back( TVector2<int32>( i, j ) );
		}
		k--;
		if( k <= 0 )
		{
			int32 n = ( j - y0 ) * 8 - s;
			if( n >= 24 )
			{
				SRand::Inst().Shuffle( vecTemp );
				for( auto& p : vecTemp )
				{
					if( m_gendata[p.x + p.y * nWidth] != 0 )
						continue;
					auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 6, 4 ), TVector2<int32>( nWidth, nHeight ),
						TRectangle<int32>( 0, y0 + 1, nWidth, j - y0 ), sqrt( n + 1 ) * 2, eType_WallChunk );
					if( rect.width > 0 )
					{
						m_wallChunks.push_back( rect );
						break;
					}
				}
			}
			k = Min( Max( 4, nHeight - 1 - j ), SRand::Inst().Rand( 7, 11 ) );
			s = 0;
			vecTemp.clear();
			y0 = j;
		}
	}
}

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
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pRoom1Node = CreateNode( pXml->FirstChildElement( "room1" )->FirstChildElement(), context );
	m_pRoom2Node = CreateNode( pXml->FirstChildElement( "room2" )->FirstChildElement(), context );
	m_pWallChunkNode = CreateNode( pXml->FirstChildElement( "wallchunk" )->FirstChildElement(), context );
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
	GenRooms1( 5 );
	AddMoreBars();
	GenRooms1( 3 );
	FixBars();
	GenWallChunks();
	GenObjs();
	GenBlocks();
	GenBonus();
	/*for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			if( m_gendata[i + j * region.width] == eType_None )
				m_gendata[i + j * region.width] = eType_Path;
		}
	}*/

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
	context.mapTags["mask"] = eType_Block2x;
	m_pBlock2xNode->Generate( context, region );
	context.mapTags["mask"] = eType_Web;
	m_pWebNode->Generate( context, region );
	for( auto& rect : m_bars )
	{
		if( rect.height > 1 )
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
		room.nType = LvGenLib::CheckRoomType( m_gendata, region.width, region.height, room.rect, eType_Room ) ? 0 : 1;
		if( room.nType == 0 )
			m_pRoom1Node->Generate( context, room.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		else
			m_pRoom2Node->Generate( context, room.rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["mask"] = eType_Web1;
	context.mapTags["0"] = eType_WallChunk1;
	for( auto& rect : m_wallChunks )
	{
		m_pWallChunkNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	m_gendata.clear();
	m_par.clear();
	m_stones.clear();
	m_bars.clear();
	m_rooms.clear();
	m_wallChunks.clear();
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
	const int32 nEntranceSize = 16;
	const int32 nBarLenMin = 9;
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
	for( int iStep = 0; !bBreak; iStep++ )
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

						nBarHeight = Min( nHeight - 1, iStep );
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
}

void CLevelGenNode1_1_2::GenRooms1( int32 nDist )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<int8> vecTemp;
	vecTemp.resize( m_gendata.size() );
	for( int i = 0; i < m_gendata.size(); i++ )
		vecTemp[i] = m_gendata[i] == eType_Path ? 1 : ( m_gendata[i] == eType_None ? 0 : 2 );
	ExpandDist( vecTemp, nWidth, nHeight, 1, 0, nDist );
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
	
	for( auto p : vecEmpty )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;

		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 5, 5 ), TVector2<int32>( 8, 8 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar );
		if( rect.width > 0 )
		{
			TRectangle<int32> r1( rect.x - 1, rect.y - 1, rect.width + 2, rect.height + 2 );
			r1 = r1 * TRectangle<int32>( 0, 0, nWidth, nHeight );
			rect = PutRect( m_gendata, nWidth, nHeight, rect, rect.GetSize(), TVector2<int32>( nWidth, nHeight ), r1, -1, eType_Room, eType_Bar );
			SRoom room;
			room.nType = 1;
			room.rect = rect;
			m_rooms.push_back( room );

			LinkRoom( 5 );
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

	TVector2<int32> dst = FindPath( m_gendata, nWidth, nHeight, eType_None, eType_Temp2, nRoomPosType == 5 ? eType_Path : eType_Temp1,
		nRoomPosType == 5 ? q : m_pathFindingTarget, m_par );
	if( dst.x >= 0 )
	{
		vector<TVector2<int32> > q1;
		TVector2<int32> p = m_par[dst.x + dst.y * nWidth];
		while( p.x >= 0 && m_gendata[p.x + p.y * nWidth] == eType_Temp2 )
		{
			q1.push_back( p );
			p = m_par[p.x + p.y * nWidth];
		}

		if( nRoomPosType < 5 )
			ExpandDist( m_gendata, nWidth, nHeight, eType_Temp2, eType_None, 1, q1 );
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

	for( int i = 0; i < nWidth; i += nWidth - 1 )
	{
		int32 l = 0;
		for( int j = 0; j <= nHeight; j++ )
		{
			if( j == nHeight || m_gendata[i + j * nWidth] != eType_None )
			{
				AddBar2( TRectangle<int32>( i, j - l, 1, l ) );
				l = 0;
			}
			else
				l++;
		}
	}

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

	//LvGenLib::AddBars( m_gendata, nWidth, nHeight, m_bars, eType_None, eType_Bar );
	vector<TVector4<int8> > vecWeight;
	vecWeight.resize( nWidth * nHeight );
	memset( &vecWeight[0], 0, 4 * vecWeight.size() );
	SRand::Inst().Shuffle( m_rooms );
	SRand::Inst().Shuffle( m_bars );
	for( auto& rect : m_bars )
		UpdateWeight( rect, vecWeight );
	for( auto& room : m_rooms )
	{
		auto& rect = room.rect;
		AddBar( TRectangle<int32>( rect.x, rect.y, rect.width, 1 ), vecWeight );
		AddBar( TRectangle<int32>( rect.x, rect.GetBottom() - 1, rect.width, 1 ), vecWeight );
	}

	for( int i = 0; i < m_bars.size(); i++ )
	{
		/*int32 n = SRand::Inst().Rand<int32>( i, m_bars.size() );
		if( n != i )
			swap( m_bars[i], m_bars[n] );*/
		AddBar( m_bars[i], vecWeight );
	}
	m_bars.clear();
}

#undef GET_LAST_TYPE

void CLevelGenNode1_1_2::AddBar( TRectangle<int32>& r, vector<TVector4<int8> >& vecWeight )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	bool bVertical = r.height > 1;
	int8 nDir, a, b;
	int32 nLen;
	if( bVertical )
	{
		a = r.y > 0 && m_gendata[r.x + ( r.y - 1 ) * nWidth] >= eType_Bar && m_gendata[r.x + ( r.y - 1 ) * nWidth] <= eType_Door;
		b = r.GetBottom() < nHeight && m_gendata[r.x + r.GetBottom() * nWidth] >= eType_Bar && m_gendata[r.x + r.GetBottom() * nWidth] <= eType_Door;
		nLen = r.height;
	}
	else
	{
		a = r.x > 0 && m_gendata[r.x - 1 + r.y * nWidth] >= eType_Bar && m_gendata[r.x - 1 + r.y * nWidth] <= eType_Door;
		b = r.GetRight() < nWidth && m_gendata[r.GetRight() + r.y * nWidth] >= eType_Bar && m_gendata[r.GetRight() + r.y * nWidth] <= eType_Door;
		nLen = r.width;
	}
	if( a && b )
		return;
	nDir = a - b;

	TRectangle<int32> rect( 0, 0, 0, 0 );
	float fMin = SRand::Inst().Rand( 3.0f, 3.6f );
	float f0 = bVertical ? SRand::Inst().Rand( 10.0f, 11.0f ) : SRand::Inst().Rand( 7.0f, 8.0f );
	float f1 = bVertical ? SRand::Inst().Rand( 0.1f, 0.13f ) : SRand::Inst().Rand( 0.15f, 0.2f );
	float l0 = bVertical ? SRand::Inst().Rand( 7.0f, 8.0f ) : SRand::Inst().Rand( 6.0f, 7.0f );
	for( int i = 0; i < nLen; i++ )
	{
		TVector2<int32> p, v;
		if( bVertical )
		{
			p = TVector2<int32>( r.x, r.y + i );
			v = TVector2<int32>( 1, 0 );
		}
		else
		{
			p = TVector2<int32>( r.x + i, r.y );
			v = TVector2<int32>( 0, 1 );
		}
		for( int j = -1; j <= 1; j += 2 )
		{
			TVector4<int8> wt;
			if( bVertical )
				wt = j == -1 ? TVector4<int8>( 0, 0, 1, 0 ): TVector4<int8>( 1, 0, 0, 0 );
			else
				wt = j == -1 ? TVector4<int8>( 0, 0, 0, 1 ) : TVector4<int8>( 0, 1, 0, 0 );
			TVector2<int32> p0 = p, v0 = v * j;
			int32 nWeight = 0;
			for( int l = 1;; l++ )
			{
				p0 = p0 + v0;
				if( p0.x < 0 || p0.y < 0 || p0.x >= nWidth || p0.y >= nHeight || m_gendata[p0.x + p0.y * nWidth] != eType_None )
					break;
				nWeight += vecWeight[p0.x + p0.y * nWidth].Dot( wt );
				if( l < 4 )
					continue;
				float fCost = nWeight * 1.0f / l + Max( 0.0f, l - f0 ) * f1 + SRand::Inst().Rand( 0.0f, 0.1f );
				if( nDir == 1 )
					fCost += Max( ( l0 - i ) / ( l0 - 1 ), 0.0f );
				else if( nDir == -1 )
					fCost += Max( ( l0 - ( nLen - i - 1 ) ) / ( l0 - 1 ), 0.0f );
				if( fCost < fMin )
				{
					rect = bVertical ? TRectangle<int32>( j == -1 ? p0.x : p.x + 1, p0.y, l, 1 )
						: TRectangle<int32>( p0.x, j == -1 ? p0.y : p.y + 1, 1, l );
					fMin = fCost;
				}
			}
		}
	}

	if( rect.width > 0 )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Bar;
			}
		}
		UpdateWeight( rect, vecWeight );
		m_bars.push_back( rect );
	}
	else if( bVertical )
		AddBar1( r, vecWeight );
}

void CLevelGenNode1_1_2::AddBar1( TRectangle<int32>& r, vector<TVector4<int8>>& vecWeight )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int8 a = r.y > 0 && m_gendata[r.x + ( r.y - 1 ) * nWidth] >= eType_Bar && m_gendata[r.x + ( r.y - 1 ) * nWidth] <= eType_Door;
	int8 b = r.GetBottom() < nHeight && m_gendata[r.x + r.GetBottom() * nWidth] >= eType_Bar && m_gendata[r.x + r.GetBottom() * nWidth] <= eType_Door;
	int8 nLen = r.height;
	int8 nDir = a - b;
	if( nDir == 0 )
		return;

	TRectangle<int32> rect( 0, 0, 0, 0 );
	float fBase = SRand::Inst().Rand( 5.8f, 6.2f );
	float fMin = -fBase * 0.33f;
	float f0 = SRand::Inst().Rand( 7.0f, 9.0f );
	float f1 = SRand::Inst().Rand( 0.13f, 0.15f );
	float l0 = SRand::Inst().Rand( 2.0f, 4.0f );
	for( int i = 0; i < nLen; i++ )
	{
		TVector2<int32> p, v;
		p = TVector2<int32>( r.x, r.y + i );
		v = TVector2<int32>( 1, 0 );
		float s = 0;
		TRectangle<int32> r0( p.x, p.y, 1, 1 );
		for( int j = -1; j <= 1; j += 2 )
		{
			TVector4<int8> wt = j == -1 ? TVector4<int8>( 0, 0, 1, 0 ) : TVector4<int8>( 1, 0, 0, 0 );
			TVector2<int32> p0 = p, v0 = v * j;
			int32 nWeight = 0;
			float fMin1 = 0;
			int32 l1 = 0;
			for( int l = 1;; l++ )
			{
				p0 = p0 + v0;
				if( p0.x < 0 || p0.y < 0 || p0.x >= nWidth || p0.y >= nHeight || m_gendata[p0.x + p0.y * nWidth] != eType_None )
					break;
				nWeight += vecWeight[p0.x + p0.y * nWidth].Dot( wt );
				float fCost = nWeight * 1.0f / l + Max( 0.0f, l - f0 ) * f1 + SRand::Inst().Rand( 0.0f, 0.1f ) - fBase;
				if( nDir == 1 )
					fCost += Max( ( l0 - i ) / ( l0 - 1 ), 0.0f );
				else if( nDir == -1 )
					fCost += Max( ( l0 - ( nLen - i - 1 ) ) / ( l0 - 1 ), 0.0f );
				if( fCost < fMin1 )
				{
					l1 = l;
					fMin1 = fCost;
				}
			}
			if( l1 )
			{
				s += fMin1;
				r0 = r0 + TRectangle<int32>( j == -1 ? p.x - l1 : p.x + 1, p0.y, l1, 1 );
			}
		}
		if( s < fMin )
		{
			fMin = s;
			rect = r0;
		}
	}

	if( rect.width > 0 )
	{
		TRectangle<int32> rect0 = r;
		TRectangle<int32> rect1 = r;
		if( nDir == 1 )
		{
			rect0.SetTop( rect.y );
			rect1.SetBottom( rect.y );
		}
		else
		{
			rect0.SetBottom( rect.GetBottom() );
			rect1.SetTop( rect.GetBottom() );
		}
		for( int i = rect0.x; i < rect0.GetRight(); i++ )
		{
			for( int j = rect0.y; j < rect0.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_None;
			}
		}
		UpdateWeight( rect0, vecWeight, -1 );

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				m_gendata[i + j * nWidth] = eType_Bar;
			}
		}
		UpdateWeight( rect, vecWeight );
		m_bars.push_back( rect );
		r = rect1;
	}
}

void CLevelGenNode1_1_2::AddBar2( TRectangle<int32>& r )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( r.height <= 12 )
		return;
	float fMax = 0;
	TRectangle<int32> r1( 0, 0, 0, 0 );
	for( int y = r.y + r.height / 3; y < r.GetBottom() - r.height / 3; y++ )
	{
		auto rect = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( r.x, y ), TVector2<int32>( 4, 1 ), TVector2<int32>( nWidth / 3, 1 ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_None );
		if( rect.width <= 0 )
			continue;
		float f = rect.width + SRand::Inst().Rand( 0.0f, 0.1f );
		if( f >= fMax )
		{
			fMax = f;
			r1 = rect;
		}
	}
	if( r1.width <= 0 )
		return;
	for( int i = r1.x; i < r1.GetRight(); i++ )
		m_gendata[i + r1.y * nWidth] = eType_Bar;
	m_bars.push_back( r1 );
	AddBar2( TRectangle<int32>( r.x, r.y, r.width, r1.y - r.y ) );
	AddBar2( TRectangle<int32>( r.x, r1.y + 1, r.width, r.GetBottom() - r1.y - 1 ) );
}

void CLevelGenNode1_1_2::UpdateWeight( const TRectangle<int32>& rect, vector<TVector4<int8> >& vecWeight, int8 n )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto rect1 = rect;
	if( rect.width > 1 )
	{
		rect1.y -= 6;
		rect1.height += 12;
	}
	else
	{
		rect1.x -= 6;
		rect1.width += 12;
	}
	rect1 = rect1 * TRectangle<int32>( 0, 0, nWidth, nHeight );
	for( int i = rect1.x; i < rect1.GetRight(); i++ )
	{
		for( int j = rect1.y; j < rect1.GetBottom(); j++ )
		{
			if( rect.width > 1 )
			{
				int8 a = Max( 0, 7 - ( rect.y - j ) );
				int8 b = Max( 0, 7 - ( j - ( rect.GetBottom() - 1 ) ) );
				if( j < rect.y )
					vecWeight[i + j * nWidth] = vecWeight[i + j * nWidth] + TVector4<int8>( 1, -1, 1, 1 ) * a * n;
				if( j > rect.GetBottom() - 1 )
					vecWeight[i + j * nWidth] = vecWeight[i + j * nWidth] + TVector4<int8>( 1, 1, 1, -1 ) * b * n;
			}
			else
			{
				int8 a = Max( 0, 7 - ( rect.x - i ) );
				int8 b = Max( 0, 7 - ( i - ( rect.GetRight() - 1 ) ) );
				vecWeight[i + j * nWidth].y += Min( a, b ) * n;
				vecWeight[i + j * nWidth].w += Min( a, b ) * n;
				if( i < rect.x )
					vecWeight[i + j * nWidth] = vecWeight[i + j * nWidth] + TVector4<int8>( -1, 1, 1, 1 ) * a * n;
				if( i > rect.GetRight() - 1 )
					vecWeight[i + j * nWidth] = vecWeight[i + j * nWidth] + TVector4<int8>( 1, 1, -1, 1 ) * b * n;
			}
		}
	}
}

void CLevelGenNode1_1_2::FixBars()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Bar )
			{
				m_gendata[i + j * nWidth] = eType_Temp;
				vec.push_back( TVector2<int32>( i, j ) );
			}
		}
	}

	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 1 ), TVector2<int32>( nWidth, 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar );
		if( rect.width > 0 )
			m_bars.push_back( rect );
	}
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 1, 4 ), TVector2<int32>( 1, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar );
		if( rect.width > 0 )
			m_bars.push_back( rect );
		else
			m_gendata[p.x + p.y * nWidth] = eType_None;
	}
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_None )
			continue;
		auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( 4, 3 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), 4, eType_Stone );
		if( rect.width > 0 )
			m_stones.push_back( rect );
	}
}

void CLevelGenNode1_1_2::GenWallChunks()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	ExpandDist( m_gendata, nWidth, nHeight, eType_Path, eType_None, 1 );

	vector<int8> vecData;
	vecData.resize( m_gendata.size() );
	for( int i = 0; i < m_gendata.size(); i++ )
	{
		vecData[i] = m_gendata[i] == eType_None ? 0 : ( m_gendata[i] == eType_Path ? 1 : 2 );
	}
	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( vecData, nWidth, nHeight, 1, vec );
	SRand::Inst().Shuffle( vec );
	for( auto p : vec )
	{
		if( vecData[p.x + p.y * nWidth] != 1 )
			continue;
		int32 s = 0;
		auto rect = PutRectEx( vecData, nWidth, nHeight, p, TVector2<int32>( 6, 4 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ),
			-1, 3, [this, &s, &vecData, nWidth, nHeight] ( TRectangle<int32> rect, TRectangle<int32> rect1 ) -> bool {
			int32 s1 = 0;
			for( int x = rect1.x; x < rect1.GetRight(); x++ )
			{
				for( int y = rect1.y; y < rect1.GetBottom(); y++ )
				{
					if( vecData[x + y * nWidth] == 0 )
						s1++;
					else if( vecData[x + y * nWidth] != 1 )
						return false;
				}
			}

			if( s1 * 2 > rect1.x * rect1.y )
				return false;
			if( ( s + s1 ) * 4 > rect.x * rect.y + rect1.x * rect1.y + 8 )
				return false;
			s += s1;
			return true;
		} );
		if( rect.width > 0 )
			GenWallChunk( rect, vecData );
	}

	SRand::Inst().Shuffle( m_bars );
	struct SLess
	{
		bool operator () ( const TRectangle<int32>& left, const TRectangle<int32>& right )
		{
			return left.y < right.y;
		}
	};
	std::sort( m_bars.begin(), m_bars.end(), SLess() );
	for( auto& rect : m_bars )
	{
		if( rect.y < 8 )
			continue;
		if( rect.width > 1 )
		{
			int32 lCur = 0;
			int32 h = 0;
			int32 nType = 0;
			bool bNeed = true;
			for( int i = rect.x; i <= rect.GetRight(); i++ )
			{
				int32 h0 = h;
				int32 nType0 = nType;
				bool b = false;
				if( i == rect.GetRight() )
					b = true;
				else
				{
					int32 j = rect.y - 1;
					for( ; j >= 0; j-- )
					{
						if( m_gendata[i + j * nWidth] >= eType_WallChunk )
							break;
					}
					int32 h1 = rect.y - 1 - j;
					int8 nType1 = j < 0 ? -1 : m_gendata[i + j * nWidth];
					if( i == rect.x )
					{
						h = h1;
						nType = nType1;
					}
					if( h != h1 || nType != nType1 )
					{
						b = true;
						h = h1;
						nType = nType1;
					}
				}
				if( b )
				{
					int32 l = lCur;
					TRectangle<int32> r( i - lCur, rect.y - h0 - 1, lCur, 1 );
					if( r.y >= 0 )
					{
						r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( r.width, 1 ), TVector2<int32>( nWidth, 1 ),
							TRectangle<int32>( rect.x, r.y, rect.width, 1 ), -1, nType0, nType0 );
						l = r.width;
					}
					if( l * 12 >= rect.width * ( 6 + Max( 0, h0 - 4 ) ) )
					{
						bNeed = false;
						break;
					}
					lCur = 0;
				}
				else
					lCur++;
			}

			if( bNeed )
			{
				vector<int32> v;
				for( int i = rect.x; i < rect.GetRight(); i++ )
					v.push_back( i );
				SRand::Inst().Shuffle( v );
				int32 y = rect.y - 1;
				for( int32 x : v )
				{
					if( m_gendata[x + y * nWidth] != eType_None )
						continue;
					auto r = PutRect( m_gendata, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 4, 4 ), TVector2<int32>( Max( 6, rect.width / 2 ), 4 ),
						TRectangle<int32>( rect.x, 0, rect.width, rect.y ), -1, eType_None );
					if( r.width )
					{
						r = PutRect( m_gendata, nWidth, nHeight, r, TVector2<int32>( r.width, r.height ),
							TVector2<int32>( Min( 12, r.width + SRand::Inst().Rand( 2, 6 ) ), SRand::Inst().Rand( 5, 8 ) ),
							TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_WallChunk, eType_None );
						m_wallChunks.push_back( r );
						break;
					}
				}
			}
		}
	}
}

void CLevelGenNode1_1_2::GenWallChunk( const TRectangle<int32>& rect, vector<int8>& vecData )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( rect.height >= 9 )
	{
		int32 h = ( rect.height - SRand::Inst().Rand( 0, 2 ) ) / 2;
		TRectangle<int32> bar( rect.x, rect.y + h, rect.width, 1 );
		for( int i = bar.x; i < bar.GetRight(); i++ )
			m_gendata[i + bar.y * nWidth] = eType_Path;
		bar = PutRect( m_gendata, nWidth, nHeight, bar, bar.GetSize(), TVector2<int32>( 14, 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Bar, eType_Path );
		for( int i = bar.x; i < bar.GetRight(); i++ )
			vecData[i + bar.y * nWidth] = 2;
		m_bars.push_back( bar );
		GenWallChunk( TRectangle<int32>( rect.x, rect.y, rect.width, h ), vecData );
		GenWallChunk( TRectangle<int32>( rect.x, rect.y + h + 1, rect.width, rect.height - 1 - h ), vecData );
		return;
	}

	auto r = rect;
	r.width = Min( r.width, SRand::Inst().Rand( 8, 11 ) );
	r.x += SRand::Inst().Rand( 0, rect.width - r.width + 1 );
	r.height = Min( r.height, Max( 6, r.width - SRand::Inst().Rand( 1, 3 ) ) );
	r.y += SRand::Inst().Rand( 0, rect.height - r.height + 1 );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_gendata[i + j * nWidth] = eType_WallChunk;
		}
	}
	m_wallChunks.push_back( r );
}

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
	int8 nTypes[] = { eType_Block1x, eType_Block2x };
	LvGenLib::FillBlocks( m_gendata, nWidth, nHeight, 32, 48, eType_None, nTypes, ELEM_COUNT( nTypes ) );
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
			if( nType == eType_Path || nType == eType_WallChunk || nType == eType_Obj || nType == eType_Door )
				opacity = 0;
			else if( nType >= eType_Block1x && nType <= eType_Block2x )
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
		if( bar.height > 1 )
			continue;
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
		vecLightMap[i] = m_gendata[i] == eType_Path || m_gendata[i] == eType_WallChunk ? 0 : 1;
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
				if( nType >= eType_Block1x && nType <= eType_Block2x )
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
			if( bar.height > 1 )
				continue;
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
		vecLightMap[i] = m_gendata[i] == eType_Path || m_gendata[i] == eType_WallChunk ? 0 : 1;
	}
	FindAllOfTypesInMap( vecLightMap, nWidth, nHeight, 0, vecEmpty );
	std::sort( vecEmpty.begin(), vecEmpty.end(), [&vecRisk, nWidth] ( const TVector2<int32> & left, const TVector2<int32> & right ) {
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
	for( auto& rect : m_wallChunks )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
				m_gendata[i + j * nWidth] = eType_Path;
		}
	}
	LvGenLib::GenObjs2( m_gendata, nWidth, nHeight, eType_Path, eType_Web, 0.25f );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = Min( nHeight, SRand::Inst().Rand( 4, 8 ) ); j >= 0; j-- )
		{
			if( m_gendata[i + j * nWidth] == eType_Web )
				m_gendata[i + j * nWidth] = eType_Path;
		}
	}

	int32 s = 0;
	for( auto& rect : m_wallChunks )
	{
		vector<TVector2<int32> > v;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] == eType_Web )
					m_gendata[i + j * nWidth] = eType_Web1;
				else
				{
					m_gendata[i + j * nWidth] = eType_WallChunk;
					v.push_back( TVector2<int32>( i, j ) );
				}
			}
		}
		s += rect.width * rect.height;
		int32 s1 = SRand::Inst().Rand( rect.width * rect.height / 8, rect.width * rect.height / 6 );
		SRand::Inst().Shuffle( v );
		for( auto p : v )
		{
			auto r1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), TVector2<int32>( SRand::Inst().Rand( 2, 5 ), SRand::Inst().Rand( 2, 5 ) ),
				rect, 5, eType_WallChunk1 );
			if( r1.width > 0 )
			{
				s1 -= r1.width * r1.height;
				if( s1 <= 0 )
					break;
			}
		}
	}
	FloodFillExpand( m_gendata, nWidth, nHeight, eType_Web, eType_WallChunk, s / 8 );

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
