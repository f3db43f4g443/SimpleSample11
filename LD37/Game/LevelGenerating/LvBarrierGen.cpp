#include "stdafx.h"
#include "LvGen1.h"
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

	const uint32 nCoreCountMin = 6;
	const uint32 nCoreCountMax = 8;

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

	uint32 nCoreCount = SRand::Inst().Rand( nCoreCountMin, nCoreCountMax + 1 );
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