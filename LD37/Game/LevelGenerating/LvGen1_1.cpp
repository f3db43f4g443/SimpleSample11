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
	float fMinPercent = 0.1f;
	float fMaxPercent = 0.2f;
	float fObjPercent = 0.02f;

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
