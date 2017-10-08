#include "stdafx.h"
#include "LvGen1.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Algorithm.h"

void CLvBarrierNodeGen1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pLabelNode = CreateNode( pXml->FirstChildElement( "label" )->FirstChildElement(), context );
	m_pFillNode = CreateNode( pXml->FirstChildElement( "fill" )->FirstChildElement(), context );
	m_pBaseNode = CreateNode( pXml->FirstChildElement( "base" )->FirstChildElement(), context );
	m_pCoreNode = CreateNode( pXml->FirstChildElement( "core" )->FirstChildElement(), context );
	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pWallHNode = CreateNode( pXml->FirstChildElement( "wall_h" )->FirstChildElement(), context );
	m_pWallVNode = CreateNode( pXml->FirstChildElement( "wall_v" )->FirstChildElement(), context );
	m_pWindowNode = CreateNode( pXml->FirstChildElement( "window" )->FirstChildElement(), context );
}

void CLvBarrierNodeGen1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		m_pContext = &context;
		m_region = region;
		m_gendata.resize( region.width * region.height );

		GenBase();
		GenRooms();
		GenWalls();
		GenWindows();

		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		for( auto& rect : m_windows )
		{
			m_pWindowNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}

		if( m_pSubChunk )
		{
			SLevelBuildContext tempContext( context.pLevel, pChunk );
			if( m_pSubChunk )
				m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
			tempContext.Build();
		}

		SLevelBuildContext tempContext( context.pLevel, pChunk );

		m_pLabelNode->Generate( tempContext, m_labelRect );
		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				if( m_gendata[i + j * region.width] == eType_None )
					m_pFillNode->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
				else if( m_gendata[i + j * region.width] == eType_Base )
					m_pBaseNode->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
				else if( m_gendata[i + j * region.width] == eType_Core )
					m_pCoreNode->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
			}
		}

		for( auto& room : m_rooms )
		{
			m_pRoomNode->Generate( tempContext, room );
		}
		for( auto& wall : m_walls )
		{
			if( m_gendata[wall.x + wall.y * region.width] == eType_WallV )
				m_pWallVNode->Generate( tempContext, wall );
			else
				m_pWallHNode->Generate( tempContext, wall );
		}

		tempContext.Build();

		m_gendata.clear();
		m_rooms.clear();
		m_walls.clear();
		m_windows.clear();
	}
}

void CLvBarrierNodeGen1::GenBase()
{
	const uint32 nLabelWidth = 6;
	const uint32 nLabelWidth1 = 10;

	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;

	uint32 n1 = SRand::Inst().Rand( nLabelWidth, nLabelWidth1 + 1 );
	uint32 n0 = SRand::Inst().Rand( 0u, nWidth - n1 );
	for( int i = 0; i < nWidth; i++ )
		m_gendata[i + ( nHeight - 1 ) * nWidth] = eType_Base;
	for( int i = n0; i < n0 + n1; i++ )
		m_gendata[i + ( nHeight - 2 ) * nWidth] = eType_Base;
	m_labelRect = TRectangle<int32>( n0 + SRand::Inst().Rand( 0u, n1 - nLabelWidth + 1 ), nHeight - 2, nLabelWidth, 2 );
	m_labelRect.x += 1;
	m_labelRect.width -= 2;

	uint32 nCoreCount = 7;
	uint32 nCoreSegWidth = nWidth / nCoreCount;
	uint32 nMod = nWidth - nCoreSegWidth * nCoreCount;

	int8* pCResult = (int8*)alloca( nCoreCount + nMod );
	SRand::Inst().C( nMod, nCoreCount + nMod, pCResult );

	uint32* pCoreOfs = (uint32*)alloca( nCoreCount * sizeof( int32 ) );
	for( int i = 0; i < nCoreCount; i++ )
	{
		pCoreOfs[i] = SRand::Inst().Rand( 0u, nCoreSegWidth );
	}

	uint32 iCurX = 0;
	int32 iSeg = 0;
	for( int i = 0; i < nCoreCount + nMod; i++ )
	{
		if( pCResult[i] )
			iCurX++;
		else
		{
			int32 nX = iCurX + pCoreOfs[iSeg++];
			iCurX += nCoreSegWidth;

			if( nX >= n0 && nX < n0 + n1 )
				m_gendata[nX + ( nHeight - 3 ) * nWidth] = eType_Core;
			else
				m_gendata[nX + ( nHeight - 2 ) * nWidth] = eType_Core;
		}
	}
}

void CLvBarrierNodeGen1::GenRooms()
{
	const uint32 nMinSize = 2;
	const uint32 nMaxSize = 5;
	const uint32 nMaxWidthPlusHeight = 10;

	vector<TVector2<int32> > vecCorners;

	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;

	for( int i = 1; i < nWidth - 1; i++ )
	{
		for( int j = 1; j < nHeight - 1; j++ )
		{
			if( m_gendata[i + j * nWidth] )
				continue;
			int8 nType0 = !!m_gendata[i - 1 + j * nWidth];
			int8 nType1 = !!m_gendata[i + ( j - 1 ) * nWidth];
			int8 nType2 = !!m_gendata[i + 1 + j * nWidth];
			int8 nType3 = !!m_gendata[i + ( j + 1 ) * nWidth];
			if( nType0 + nType1 + nType2 + nType3 != 2 )
				continue;
			if( nType0 && nType2 || nType1 && nType3 )
				continue;
			
			vecCorners.push_back( TVector2<int32>( i, j ) );
		}
	}
	
	uint32 iCorner = 0;
	while( iCorner < vecCorners.size() )
	{
		uint32 n = SRand::Inst().Rand( iCorner, vecCorners.size() );
		auto p = vecCorners[n];
		vecCorners[n] = vecCorners[iCorner];
		vecCorners[iCorner] = p;
		iCorner++;

		{
			if( m_gendata[p.x + p.y * nWidth] )
				continue;
			int8 nType0 = !!m_gendata[p.x - 1 + p.y * nWidth];
			int8 nType1 = !!m_gendata[p.x + ( p.y - 1 ) * nWidth];
			int8 nType2 = !!m_gendata[p.x + 1 + p.y * nWidth];
			int8 nType3 = !!m_gendata[p.x + ( p.y + 1 ) * nWidth];
			if( nType0 + nType1 + nType2 + nType3 != 2 )
				continue;
			if( nType0 && nType2 || nType1 && nType3 )
				continue;
		}

		uint32 nExtend = SRand::Inst().Rand( nMinSize * 2 - 2, nMaxWidthPlusHeight - 1 );
		TRectangle<int32> roomRect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( nMinSize, nMinSize ), TVector2<int32>( nMaxSize, nMaxSize ),
			TRectangle<int32>( 1, 1, nWidth - 2, nHeight - 2 ), nExtend, eType_Room );

		if( roomRect.width < nMinSize || roomRect.height < nMinSize )
			continue;

		m_rooms.push_back( roomRect );

		for( int i = roomRect.x - 1; i < roomRect.GetRight() + 1; i++ )
		{
			for( int j = roomRect.y - 1; j < roomRect.GetBottom() + 1; j++ )
			{
				bool bRoomX = i >= roomRect.x && i < roomRect.GetRight();
				bool bRoomY = j >= roomRect.y && j < roomRect.GetBottom();
				if( bRoomX && bRoomY )
					m_gendata[i + j * nWidth] = eType_Room;
				else if( bRoomX )
				{
					if( !m_gendata[i + j * nWidth] || m_gendata[i + j * nWidth] == eType_Wall )
						m_gendata[i + j * nWidth] = eType_WallH;
				}
				else if( bRoomY )
				{
					if( !m_gendata[i + j * nWidth] || m_gendata[i + j * nWidth] == eType_Wall )
						m_gendata[i + j * nWidth] = eType_WallV;
				}
				else
				{
					if( !m_gendata[i + j * nWidth] )
						m_gendata[i + j * nWidth] = eType_Wall;
				}
			}
		}

		for( int i = Max( 1, roomRect.x - 1 ); i < Min( (int32)nWidth - 1, roomRect.GetRight() + 1 ); i++ )
		{
			int j = roomRect.y - 2;
			if( j > 0 )
			{
				if( !m_gendata[i + j * nWidth] )
				{
					int8 nType0 = !!m_gendata[i - 1 + j * nWidth];
					int8 nType1 = !!m_gendata[i + ( j - 1 ) * nWidth];
					int8 nType2 = !!m_gendata[i + 1 + j * nWidth];
					int8 nType3 = !!m_gendata[i + ( j + 1 ) * nWidth];
					if( nType0 + nType1 + nType2 + nType3 == 2 && !( nType0 && nType2 || nType1 && nType3 ) )
					{
						vecCorners.push_back( TVector2<int32>( i, j ) );
					}
				}
			}
			j = roomRect.GetBottom() + 1;
			if( j < nHeight - 1 )
			{
				if( !m_gendata[i + j * nWidth] )
				{
					int8 nType0 = !!m_gendata[i - 1 + j * nWidth];
					int8 nType1 = !!m_gendata[i + ( j - 1 ) * nWidth];
					int8 nType2 = !!m_gendata[i + 1 + j * nWidth];
					int8 nType3 = !!m_gendata[i + ( j + 1 ) * nWidth];
					if( nType0 + nType1 + nType2 + nType3 == 2 && !( nType0 && nType2 || nType1 && nType3 ) )
					{
						vecCorners.push_back( TVector2<int32>( i, j ) );
					}
				}
			}
		}

		for( int j = Max( 1, roomRect.y - 1 ); j < Min( (int32)nHeight - 1, roomRect.GetBottom() + 1 ); j++ )
		{
			int i = roomRect.x - 2;
			if( i > 0 )
			{
				if( !m_gendata[i + j * nWidth] )
				{
					int8 nType0 = !!m_gendata[i - 1 + j * nWidth];
					int8 nType1 = !!m_gendata[i + ( j - 1 ) * nWidth];
					int8 nType2 = !!m_gendata[i + 1 + j * nWidth];
					int8 nType3 = !!m_gendata[i + ( j + 1 ) * nWidth];
					if( nType0 + nType1 + nType2 + nType3 == 2 && !( nType0 && nType2 || nType1 && nType3 ) )
					{
						vecCorners.push_back( TVector2<int32>( i, j ) );
					}
				}
			}
			i = roomRect.GetRight() + 1;
			if( i < nWidth - 1 )
			{
				if( !m_gendata[i + j * nWidth] )
				{
					int8 nType0 = !!m_gendata[i - 1 + j * nWidth];
					int8 nType1 = !!m_gendata[i + ( j - 1 ) * nWidth];
					int8 nType2 = !!m_gendata[i + 1 + j * nWidth];
					int8 nType3 = !!m_gendata[i + ( j + 1 ) * nWidth];
					if( nType0 + nType1 + nType2 + nType3 == 2 && !( nType0 && nType2 || nType1 && nType3 ) )
					{
						vecCorners.push_back( TVector2<int32>( i, j ) );
					}
				}
			}
		}
	}
}

void CLvBarrierNodeGen1::GenWalls()
{
	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Wall )
			{
				bool bType0 = i > 0 ? m_gendata[i - 1 + j * nWidth] == eType_WallH : false;
				bool bType1 = j > 0 ? m_gendata[i + ( j - 1 ) * nWidth] == eType_WallV : false;
				bool bType2 = i < nWidth - 1 ? m_gendata[i + 1 + j * nWidth] == eType_WallH : false;
				bool bType3 = j < nHeight - 1 ? m_gendata[i + ( j + 1 ) * nWidth] == eType_WallV : false;

				int8 n = (int8)bType0 + (int8)bType2 - (int8)bType1 - (int8)bType3;
				if( n == 0 )
					m_gendata[i + j * nWidth] = SRand::Inst().Rand( 0, 2 ) + eType_WallH;
				else if( n > 0 )
					m_gendata[i + j * nWidth] = eType_WallH;
				else
					m_gendata[i + j * nWidth] = eType_WallV;
			}
		}
	}

	for( int j = 0; j < nHeight; j++ )
	{
		uint32 nCurLength = 0;
		for( int i = 0; i < nWidth; i++ )
		{
			if( m_gendata[i + j * nWidth] == eType_WallH )
				nCurLength++;
			else if( nCurLength )
			{
				m_walls.push_back( TRectangle<int32>( i - nCurLength, j, nCurLength, 1 ) );
				nCurLength = 0;
			}
		}
		if( nCurLength )
		{
			m_walls.push_back( TRectangle<int32>( nWidth - nCurLength, j, nCurLength, 1 ) );
			nCurLength = 0;
		}
	}

	for( int i = 0; i < nWidth; i++ )
	{
		uint32 nCurLength = 0;
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_WallV )
				nCurLength++;
			else if( nCurLength )
			{
				m_walls.push_back( TRectangle<int32>( i, j - nCurLength, 1, nCurLength ) );
				nCurLength = 0;
			}
		}
		if( nCurLength )
		{
			m_walls.push_back( TRectangle<int32>( i, nHeight - nCurLength, 1, nCurLength ) );
			nCurLength = 0;
		}
	}
}

void CLvBarrierNodeGen1::GenWindows()
{
	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;

	uint32 nWindowCount = 6;
	uint32 nWindowSegWidth = nWidth / nWindowCount;
	uint32 nMod = nWidth - nWindowSegWidth * nWindowCount;

	int8* pCResult = (int8*)alloca( nWindowCount + nMod );
	SRand::Inst().C( nMod, nWindowCount + nMod, pCResult );

	uint32* pWindowOfs = (uint32*)alloca( nWindowCount * sizeof( int32 ) );
	for( int i = 0; i < nWindowCount; i++ )
	{
		pWindowOfs[i] = SRand::Inst().Rand( 1u, nWindowSegWidth - 2 );
	}

	uint32 iCurX = 0;
	int32 iSeg = 0;
	for( int i = 0; i < nWindowCount + nMod; i++ )
	{
		if( pCResult[i] )
			iCurX++;
		else
		{
			int32 nX = iCurX + pWindowOfs[iSeg++];
			iCurX += nWindowSegWidth;

			for( int j = 5; j >= 1; j-- )
			{
				if( m_gendata[nX + j * nWidth] != eType_Room || m_gendata[nX + 1 + j * nWidth] != eType_Room )
				{
					m_windows.push_back( TRectangle<int32>( nX, j - 1, 2, 2 ) );
					break;
				}
			}
		}
	}
}

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

		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );
		m_pLabelNode->Generate( context, m_labelRect.Offset( TVector2<int32>( region.x, region.y ) ) );

		if( m_pSubChunk )
		{
			SLevelBuildContext tempContext( context.pLevel, pChunk );
			if( m_pSubChunk )
				m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
			tempContext.Build();
		}

		SLevelBuildContext tempContext( context.pLevel, pChunk );
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
	uint32 n0 = ( nWidth - 1 + SRand::Inst().Rand( 0, 2 ) ) / 2;
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