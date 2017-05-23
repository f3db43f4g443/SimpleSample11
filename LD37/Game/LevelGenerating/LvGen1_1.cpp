#include "stdafx.h"
#include "LvGen1.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"

void CLevelGenNode1_1_0::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1Node = CreateNode( pXml->FirstChildElement( "block1" )->FirstChildElement(), context );
	m_pBlock2Node = CreateNode( pXml->FirstChildElement( "block2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );

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
		bool bLeftToRight = SRand::Inst().Rand() & 1;
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
	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );
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
			int8 nType = ( SRand::Inst().Rand() & 1 ) + eType_Block1;
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

	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Block1, vec );
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Block2, vec );

	int32 nHoleCount = vec.size() * SRand::Inst().Rand( fMinPercent, fMaxPercent );
	int32 nObjCount = vec.size() * fObjPercent;
	SRand::Inst().Shuffle( &vec[0], vec.size() );

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
					if( m_gendata[x + y * nWidth] == eType_Wall || m_gendata[x + y * nWidth] == eType_None )
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
			if( nObjCount )
			{
				auto obj = SRand::Inst().Rand() & 1 ? p : p1;
				m_gendata[obj.x + obj.y * nWidth] = eType_Obj;
				nObjCount--;
			}
		}
	}
}

void CLevelGenNode1_1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pStoneNode = CreateNode( pXml->FirstChildElement( "stone" )->FirstChildElement(), context );
	m_pBlock1xNode = CreateNode( pXml->FirstChildElement( "block1x" )->FirstChildElement(), context );
	m_pBlock1yNode = CreateNode( pXml->FirstChildElement( "block1y" )->FirstChildElement(), context );
	m_pBlock2xNode = CreateNode( pXml->FirstChildElement( "block2x" )->FirstChildElement(), context );
	m_pBlock2yNode = CreateNode( pXml->FirstChildElement( "block2y" )->FirstChildElement(), context );
	m_pBarNode = CreateNode( pXml->FirstChildElement( "bar" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	GenMainPath();
	Flatten();
	GenObstacles();
	GenObjsBig();
	GenObjsSmall();
	GenAreas();
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

	m_gendata.clear();
	m_bars.clear();
	m_stones.clear();
}

void CLevelGenNode1_1_1::GenMainPath()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nMinSizePerWaypoint = 128;
	const int32 nMaxSizePerWaypoint = 160;
	const float fMinPathPercent = 0.5f;
	const float fMaxPathPercent = 0.55f;
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
					if( !!( SRand::Inst().Rand() & 1 ) )
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
	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );

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
	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );

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

		int32 nObjCount = n / 2;
		for( int i = 0; i < n && nObjCount; i++ )
		{
			auto p1 = q[i];
			if( m_gendata[p1.x + p1.y * nWidth] == eType_Obj )
				continue;

			for( ; p1.y > 0; p1.y-- )
			{
				if( m_gendata[p1.x + ( p1.y - 1 ) * nWidth] != eType_Path )
					break;
			}
			m_gendata[p1.x + p1.y * nWidth] = eType_Obj;
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
	const float fObjPercentMin1 = 0.015f;
	const float fObjPercentMax1 = 0.02f;

	vector<int8> vecTemp;
	vecTemp.resize( m_gendata.size() );
	for( int i = 0; i < m_gendata.size(); i++ )
		vecTemp[i] = m_gendata[i] == eType_Path || m_gendata[i] == eType_Obj ? 1 : ( m_gendata[i] == eType_Temp1 ? 0 : 2 );
	ExpandDist( vecTemp, nWidth, nHeight, 1, 0, 2 );
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, vecEmpty );
	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );
	uint32 nObjCount = nWidth * nHeight * SRand::Inst().Rand( fObjPercentMin, fObjPercentMax );

	for( auto p : vecEmpty )
	{
		if( !nObjCount )
			break;
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp1 )
			continue;

		TVector2<int32> p1s[4];
		int32 nTypes[4];
		int32 nTypeCount = 0;
		if( p.x > 0 && m_gendata[p.x - 1 + p.y * nWidth] == eType_Temp1 )
		{
			p1s[nTypeCount] = TVector2<int32>( p.x - 1, p.y );
			nTypes[nTypeCount] = ( p.x + p.y ) & 1 ? eType_Block1x : eType_Block2x;
			nTypeCount++;
		}
		if( p.y > 0 && m_gendata[p.x + ( p.y - 1 ) * nWidth] == eType_Temp1 )
		{
			p1s[nTypeCount] = TVector2<int32>( p.x, p.y - 1 );
			nTypes[nTypeCount] = ( p.x + p.y ) & 1 ? eType_Block1y : eType_Block2y;
			nTypeCount++;
		}
		if( p.x < nWidth - 1 && m_gendata[p.x + 1 + p.y * nWidth] == eType_Temp1 )
		{
			p1s[nTypeCount] = TVector2<int32>( p.x + 1, p.y );
			nTypes[nTypeCount] = ( p.x + p.y ) & 1 ? eType_Block2x : eType_Block1x;
			nTypeCount++;
		}
		if( p.y < nHeight - 1 && m_gendata[p.x + ( p.y + 1 ) * nWidth] == eType_Temp1 )
		{
			p1s[nTypeCount] = TVector2<int32>( p.x, p.y + 1 );
			nTypes[nTypeCount] = ( p.x + p.y ) & 1 ? eType_Block2y : eType_Block1y;
			nTypeCount++;
		}
		if( !nTypeCount )
			continue;
		
		int32 n = SRand::Inst().Rand( 0, nTypeCount );
		int32 nType = nTypes[n];
		TVector2<int32> p1 = p1s[n];
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, nType, SRand::Inst().Rand( nFillSizeMin, nFillSizeMax ) );
		m_gendata[p.x + p.y * nWidth] = m_gendata[p1.x + p1.y * nWidth] = eType_Temp;

		if( nType < eType_Block1y )
			( SRand::Inst().Rand() & 1 ? m_gendata[p.x + p.y * nWidth] : m_gendata[p1.x + p1.y * nWidth] ) = eType_Obj;
		else
			( p.y < p1.y ? m_gendata[p.x + p.y * nWidth] : m_gendata[p1.x + p1.y * nWidth] ) = eType_Obj;
		nObjCount--;
	}

	nObjCount = nWidth * nHeight * SRand::Inst().Rand( fObjPercentMin1, fObjPercentMax1 );
	vecEmpty.clear();
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Path, vecEmpty );
	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );
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

void CLevelGenNode1_1_1::GenAreas()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	const int32 nFillSizeMin = 32;
	const int32 nFillSizeMax = 48;
	vector<TVector2<int32> > vecEmpty;
	FindAllOfTypesInMap( m_gendata, nWidth, nHeight, eType_Temp1, vecEmpty );
	SRand::Inst().Shuffle( &vecEmpty[0], vecEmpty.size() );
	for( auto p : vecEmpty )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Temp1 )
			continue;
		FloodFill( m_gendata, nWidth, nHeight, p.x, p.y, eType_Block1x + SRand::Inst().Rand( 0, 4 ), SRand::Inst().Rand( nFillSizeMin, nFillSizeMax ) );
	}
}