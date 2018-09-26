#include "stdafx.h"
#include "LvGen1.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Algorithm.h"


void CLvBarrierNodeGen2::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pLabelNode = CreateNode( pXml->FirstChildElement( "label" )->FirstChildElement(), context );
	m_pChunkNode = CreateNode( pXml->FirstChildElement( "chunk" )->FirstChildElement(), context );
}

void CLvBarrierNodeGen2::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		m_pContext = &context;
		m_region = region;
		m_gendata.resize( region.width * region.height );

		GenBlocks();

		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );
		m_pLabelNode->Generate( context, m_labelRect.Offset( TVector2<int32>( region.x, region.y ) ) );

		if( m_pSubChunk )
		{
			SLevelBuildContext tempContext( context, pChunk );
			if( m_pSubChunk )
				m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
			tempContext.Build();
		}

		SLevelBuildContext tempContext( context, pChunk );
		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				if( m_gendata[i + j * region.width] == eType_Blocked )
					pChunk->GetBlock( i, j )->eBlockType = eBlockType_Block;
				else if( m_gendata[i + j * region.width] == eType_Track )
				{
					pChunk->GetBlock( i, j )->nTag = 1;
					m_gendata[i + j * region.width] = eType_None;
				}
				tempContext.blueprint[i + j * region.width] = m_gendata[i + j * region.width];
			}
		}
		GenTracks( pChunk );

		tempContext.mapTags["mask"] = eType_None;
		m_pChunkNode->Generate( tempContext, TRectangle<int32>( 0, 0, region.width, region.height ) );

		for( auto pChunk : tempContext.chunks )
		{
			pChunk->nSubChunkType = 2;
		}
		tempContext.Build();

		m_gendata.clear();
	}
}

void CLvBarrierNodeGen2::GenBlocks()
{
	uint32 nLabelWidth = 6;
	uint32 nLabelWidth1 = 10;
	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;

	uint32 n1 = SRand::Inst().Rand( nLabelWidth, nLabelWidth1 + 1 );
	uint32 n0 = ( nWidth - n1 + SRand::Inst().Rand( 0, 2 ) ) / 2;
	for( int i = 0; i < nWidth; i++ )
		m_gendata[i + ( nHeight - 1 ) * nWidth] = eType_Blocked;
	for( int i = n0; i < n0 + n1; i++ )
		m_gendata[i + ( nHeight - 2 ) * nWidth] = eType_Blocked;
	for( int i = 0; i < nHeight; i++ )
	{
		m_gendata[0 + i * nWidth] = eType_Blocked;
		m_gendata[( nWidth - 1 ) + i * nWidth] = eType_Blocked;
	}
	m_gendata[1 + ( nHeight - 2 ) * nWidth] = m_gendata[( nWidth - 2 ) + ( nHeight - 2 ) * nWidth] = eType_Blocked;
	m_labelRect = TRectangle<int32>( n0 + SRand::Inst().Rand( 0u, n1 - nLabelWidth + 1 ), nHeight - 2, nLabelWidth, 2 );
	m_labelRect.x += 1;
	m_labelRect.width -= 2;

	uint32 nOut0 = SRand::Inst().Rand( 4, 7 );
	uint32 nOut1 = SRand::Inst().Rand( 6, 8 );

	{
		uint32 w1 = nWidth - n1 - 4;
		uint32 nSegWidth = w1 / nOut1;
		uint32 nMod = w1 - nSegWidth * nOut1;

		int8* pCResult = (int8*)alloca( nOut1 + nMod );
		SRand::Inst().C( nMod, nOut1 + nMod, pCResult );

		uint32* pOfs = (uint32*)alloca( nOut1 * sizeof( int32 ) );
		for( int i = 0; i < nOut1; i++ )
		{
			pOfs[i] = SRand::Inst().Rand( 0u, nSegWidth );
		}

		uint32 iCurX = 0;
		int32 iSeg = 0;
		for( int i = 0; i < nOut1 + nMod; i++ )
		{
			if( pCResult[i] )
				iCurX++;
			else
			{
				int32 nX = iCurX + pOfs[iSeg++];
				iCurX += nSegWidth;
				nX = nX >= n0 - 2 ? nX + 2 + n1 : nX + 2;
				
				m_gendata[nX + ( nHeight - 2 ) * nWidth] = eType_Track;
			}
		}

		int32 nCurLen = 0;
		int32 iX;
		for( iX = 2; iX < nWidth - 2; iX++ )
		{
			if( m_gendata[iX + ( nHeight - 2 ) * nWidth] == eType_None )
				nCurLen++;
			else
			{
				if( nCurLen )
				{
					int32 nLen = SRand::Inst().Rand( Max( nCurLen - 3, 1 ), nCurLen + 1 );
					int32 nBeg = iX - nCurLen + SRand::Inst().Rand( 0, nCurLen - nLen + 1 );
					for( int32 iX1 = nBeg; iX1 < nBeg + nLen; iX1++ )
						m_gendata[iX1 + ( nHeight - 2 ) * nWidth] = eType_Blocked;
				}
				nCurLen = 0;
			}
		}
		if( nCurLen )
		{
			int32 nLen = SRand::Inst().Rand( Max( nCurLen - 3, 1 ), nCurLen + 1 );
			int32 nBeg = iX - nCurLen + SRand::Inst().Rand( 0, nCurLen - nLen + 1 );
			for( int32 iX1 = nBeg; iX1 < nBeg + nLen; iX1++ )
				m_gendata[iX1 + ( nHeight - 2 ) * nWidth] = eType_Blocked;
		}
	}

	uint32 nOut00[2];
	nOut00[0] = ( nOut0 + SRand::Inst().Rand( 0, 2 ) ) / 2;
	nOut00[1] = nOut0 - nOut00[0];

	for( int32 i = 0; i < 2; i++ )
	{
		int32 nX0 = i * ( nWidth - 1 );
		int32 nX1 = nX0 + 1 - i * 2;
		
		uint32 w1 = nHeight - 2;
		uint32 nSegWidth = w1 / nOut00[i];
		uint32 nMod = w1 - nSegWidth * nOut00[i];

		int8* pCResult = (int8*)alloca( nOut00[i] + nMod );
		SRand::Inst().C( nMod, nOut00[i] + nMod, pCResult );

		uint32* pOfs = (uint32*)alloca( nOut00[i] * sizeof( int32 ) );
		for( int j = 0; j < nOut00[i]; j++ )
		{
			pOfs[j] = SRand::Inst().Rand( 0u, nSegWidth );
		}

		uint32 iCurY = 0;
		int32 iSeg = 0;
		for( int j = 0; j < nOut00[i] + nMod; j++ )
		{
			if( pCResult[j] )
				iCurY++;
			else
			{
				int32 nY = iCurY + pOfs[iSeg++];
				iCurY += nSegWidth;
				
				m_gendata[nX1 + nY * nWidth] = eType_Track;
			}
		}

		int32 nCurLen = 0;
		int32 iY;
		for( iY = 0; iY < nHeight - 2; iY++ )
		{
			if( m_gendata[nX1 + iY * nWidth] == eType_None )
				nCurLen++;
			else
			{
				if( nCurLen )
				{
					int32 nLen = SRand::Inst().Rand( Max( nCurLen - 3, 1 ), nCurLen + 1 );
					int32 nBeg = iY - nCurLen + SRand::Inst().Rand( 0, nCurLen - nLen + 1 );
					for( int32 iY1 = nBeg; iY1 < nBeg + nLen; iY1++ )
						m_gendata[nX1 + iY1 * nWidth] = eType_Blocked;
				}
				nCurLen = 0;
			}
		}
		if( nCurLen )
		{
			int32 nLen = SRand::Inst().Rand( Max( nCurLen - 3, 1 ), nCurLen + 1 );
			int32 nBeg = iY - nCurLen + SRand::Inst().Rand( 0, nCurLen - nLen + 1 );
			for( int32 iY1 = nBeg; iY1 < nBeg + nLen; iY1++ )
				m_gendata[nX1 + iY1 * nWidth] = eType_Blocked;
		}
	}
}

void CLvBarrierNodeGen2::GenTracks( SChunk* pChunk )
{
	vector<int8> gendata;
	int32 nWidth = pChunk->nWidth;
	int32 nHeight = pChunk->nHeight;
	gendata.resize( nWidth * nHeight );

	vector<TVector2<int32> > p[3];
	int32 n0 = SRand::Inst().Rand( 0, 2 );
	for( int32 i = 0; i < nWidth; i++ )
	{
		for( int32 j = 0; j < nHeight; j++ )
		{
			if( pChunk->GetBlock( i, j )->eBlockType == eBlockType_Block || pChunk->GetBlock( i, j )->nTag == 2 )
				gendata[i + j * nWidth] = 2;
			else if( pChunk->GetBlock( i, j )->nTag == 1 )
			{
				if( j >= nHeight - 2 )
					p[2].push_back( TVector2<int32>( i, j ) );
				else if( i < nWidth / 2 )
					p[n0].push_back( TVector2<int32>( i, j ) );
				else
					p[1 - n0].push_back( TVector2<int32>( i, j ) );
			}
			pChunk->GetBlock( i, j )->nTag = 0;
		}
	}
	SRand::Inst().Shuffle( p[0] );
	SRand::Inst().Shuffle( p[1] );
	SRand::Inst().Shuffle( p[2] );

	vector<pair<TVector2<int32>, TVector2<int32> > > vecPairs;
	for( int i = Min( p[0].size(), p[1].size() ) - 1; i >= 0; i-- )
	{
		auto p0 = p[0][i];
		auto p1 = p[1][i];
		vecPairs.push_back( pair<TVector2<int32>, TVector2<int32> >( p0, p1 ) );
	}
	for( int k = 0; k < 2; k++ )
	{
		for( int i = Min( p[0].size(), p[1].size() ); i < p[k].size(); i++ )
		{
			auto p0 = p[k][i];
			auto p1 = p[2][SRand::Inst().Rand( 0u, p[2].size() )];
			vecPairs.push_back( pair<TVector2<int32>, TVector2<int32> >( p0, p1 ) );
		}
	}
	for( int i = 0; i < p[2].size(); i++ )
	{
		auto p0 = p[2][i];
		auto p1 = p[0][SRand::Inst().Rand( 0u, p[0].size() )];
		auto p2 = p[1][SRand::Inst().Rand( 0u, p[1].size() )];
		if( abs( p2.x - p0.x ) + abs( p2.y - p0.y ) > abs( p1.x - p0.x ) + abs( p1.y - p0.y ) )
			p1 = p2;
		vecPairs.push_back( pair<TVector2<int32>, TVector2<int32> >( p0, p1 ) );
	}

	vector<TVector2<int32> > par;
	par.resize( nWidth * nHeight );
	uint32 n = 0;
	uint32 nMax = nWidth * nHeight / 2;
	int32 i1 = -1;
	vector<int32> vecTemp;
	for( int i = nWidth / 4; i < nWidth - nWidth / 4; i++ )
		vecTemp.push_back( i );
	for( int i = 0; i < vecPairs.size(); i++ )
	{
		auto p0 = vecPairs[i].first;
		auto p1 = vecPairs[i].second;

		gendata[p1.x + p1.y * nWidth] = 3;
		FindPath( gendata, nWidth, nHeight, p0, 1, 3, par );

		auto pt = p1;
		while( 1 )
		{
			gendata[pt.x + pt.y * nWidth] = 0;

			if( pt == p0 )
				break;
			auto pp = par[pt.x + pt.y * nWidth];
			if( !pChunk->GetBlock( pt.x, pt.y )->nTag )
				n++;
			if( !pChunk->GetBlock( pp.x, pp.y )->nTag )
				n++;
			if( pp.x == pt.x - 1 )
			{
				pChunk->GetBlock( pt.x, pt.y )->nTag |= 1;
				pChunk->GetBlock( pp.x, pp.y )->nTag |= 2;
			}
			else if( pp.x == pt.x + 1 )
			{
				pChunk->GetBlock( pt.x, pt.y )->nTag |= 2;
				pChunk->GetBlock( pp.x, pp.y )->nTag |= 1;
			}
			else if( pp.y == pt.y - 1 )
			{
				pChunk->GetBlock( pt.x, pt.y )->nTag |= 4;
				pChunk->GetBlock( pp.x, pp.y )->nTag |= 8;
			}
			else if( pp.y == pt.y + 1 )
			{
				pChunk->GetBlock( pt.x, pt.y )->nTag |= 8;
				pChunk->GetBlock( pp.x, pp.y )->nTag |= 4;
			}

			pt = pp;
		}
		gendata[p0.x + p0.y * nWidth] = 0;

		if( n >= nMax )
		{
			if( i1 >= 0 && !!( i & 1 ) )
				break;
		}
		else if( i == vecPairs.size() - 1 && i1 < 6 )
		{
			i1++;
			vecPairs.clear();
			SRand::Inst().Shuffle( vecTemp );
			for( int j = 0; j < vecTemp.size(); j++ )
			{
				TVector2<int32> p0( vecTemp[j], i1 );
				if( !pChunk->GetBlock( p0.x, p0.y )->nTag )
					continue;
				auto p1 = p[0][SRand::Inst().Rand( 0u, p[0].size() )];
				auto p2 = p[1][SRand::Inst().Rand( 0u, p[1].size() )];
				vecPairs.push_back( pair<TVector2<int32>, TVector2<int32> >( p0, p1 ) );
				vecPairs.push_back( pair<TVector2<int32>, TVector2<int32> >( p0, p2 ) );
			}
			i = -1;
		}
	}
}