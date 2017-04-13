#include "stdafx.h"
#include "LvGen1.h"
#include "Common/Algorithm.h"
#include "Common/Rand.h"
#include <algorithm>

void CLevelGenNode1_1::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pBlock1Node = CreateNode( pXml->FirstChildElement( "block1" )->FirstChildElement(), context );
	m_pBlock2Node = CreateNode( pXml->FirstChildElement( "block2" )->FirstChildElement(), context );
	m_pRoom1Node = CreateNode( pXml->FirstChildElement( "room1" )->FirstChildElement(), context );
	m_pRoom2Node = CreateNode( pXml->FirstChildElement( "room2" )->FirstChildElement(), context );
	m_pBar1Node = CreateNode( pXml->FirstChildElement( "bar1" )->FirstChildElement(), context );
	m_pBar2Node = CreateNode( pXml->FirstChildElement( "bar2" )->FirstChildElement(), context );
	m_pObjNode = CreateNode( pXml->FirstChildElement( "obj" )->FirstChildElement(), context );
	m_pPipeNode = CreateNode( pXml->FirstChildElement( "pipe" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode1_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenRooms();
	PutHBars();
	ConnRooms();
	GenConnAreas();
	GenDoors();
	GenEmptyArea();
	FillBlockArea();
	GenObjects();

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
			else if( genData == eType_Object )
			{
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				m_pObjNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
			}
			else if( genData == eType_Pipe )
			{
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				m_pPipeNode->Generate( context, TRectangle<int32>( x, y + 1, 1, 1 ) );
			}
		}
	}

	context.mapTags["mask"] = eType_BlockRed;
	m_pBlock1Node->Generate( context, region );
	context.mapTags["mask"] = eType_BlockBlue;
	m_pBlock2Node->Generate( context, region );

	for( auto& bar : m_bars )
	{
		auto rect = bar.rect.Offset( TVector2<int32>( region.x, region.y ) );
		if( bar.nType == 1 )
			m_pBar2Node->Generate( context, rect );
		else
			m_pBar1Node->Generate( context, rect );
	}

	context.mapTags["door"] = eType_Door;
	for( auto& room : m_rooms )
	{
		auto rect = room.rect.Offset( TVector2<int32>( region.x, region.y ) );
		if( room.nType == 1 )
			m_pRoom2Node->Generate( context, rect );
		else
			m_pRoom1Node->Generate( context, rect );
	}

	m_gendata.clear();
	m_rooms.clear();
	m_bars.clear();
}

void CLevelGenNode1_1::GenRooms()
{
	const uint32 nMinWidth = 6;
	const uint32 nMinHeight = 6;
	const uint32 nMaxWidth = 12;
	const uint32 nMaxHeight = 12;
	const uint32 nRoom2Size = 15;

	const uint32 nMaxWidthPlusHeight = 20;
	const uint32 nIterationCount = 200;
	const uint32 nMaxSpace = 1;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;

	vector<TVector2<int32> > vecPossibleGenPoints;
	vector<uint8> vecGenPointValid;
	vecGenPointValid.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth - nMinWidth; i++ )
	{
		for( int j = 0; j <  nHeight - nMinHeight; j++ )
		{
			vecPossibleGenPoints.push_back( TVector2<int32>( i, j ) );
			vecGenPointValid[i + j * nWidth] = 1;
		}
	}
	uint32 iGenPoint = 0;

	for( int i = 0; i < nIterationCount; i++ )
	{
		bool bValidGenPoint = false;
		TVector2<int32> genPoint;
		while( iGenPoint < vecPossibleGenPoints.size() )
		{
			uint32 iGenPoint1 = SRand::Inst().Rand( iGenPoint, vecPossibleGenPoints.size() );
			genPoint = vecPossibleGenPoints[iGenPoint1];
			vecPossibleGenPoints[iGenPoint1] = vecPossibleGenPoints[iGenPoint];
			vecPossibleGenPoints[iGenPoint] = genPoint;
			iGenPoint++;

			if( vecGenPointValid[genPoint.x + genPoint.y * nWidth] )
			{
				bValidGenPoint = true;
				break;
			}
		}
		if( !bValidGenPoint )
			break;

		TRectangle<int32> roomRect( genPoint.x, genPoint.y, nMinWidth, nMinHeight );
		uint32 nExtend = SRand::Inst().Rand( 0u, nMaxWidthPlusHeight - nMinWidth - nMinHeight );
		uint32 nExtendDirs[] = { 0, 1, 2, 3 };
		uint32 nExtendDirCount = 4;

		for( int j = 0; j < nExtend && nExtendDirCount; )
		{
			uint32 iExtendDir = SRand::Inst().Rand( 0u, nExtendDirCount );
			uint32 nExtendDir = nExtendDirs[iExtendDir];
			bool bSucceed = true;

			TRectangle<int32> newPointsRect;
			switch( nExtendDir )
			{
			case 0:
				if( roomRect.width >= nMaxWidth || roomRect.x <= 0 )
				{
					bSucceed = false;
					break;
				}
				newPointsRect = TRectangle<int32>( roomRect.x - 1, roomRect.y, 1, roomRect.height );
				break;
			case 1:
				if( roomRect.height >= nMaxHeight || roomRect.y <= 0 )
				{
					bSucceed = false;
					break;
				}
				newPointsRect = TRectangle<int32>( roomRect.x, roomRect.y - 1, roomRect.width, 1 );
				break;
			case 2:
				if( roomRect.width >= nMaxWidth || roomRect.GetRight() >= nWidth )
				{
					bSucceed = false;
					break;
				}
				newPointsRect = TRectangle<int32>( roomRect.GetRight(), roomRect.y, 1, roomRect.height );
				break;
			case 3:
				if( roomRect.height >= nMaxHeight || roomRect.GetBottom() >= nHeight )
				{
					bSucceed = false;
					break;
				}
				newPointsRect = TRectangle<int32>( roomRect.x, roomRect.GetBottom(), roomRect.width, 1 );
				break;
			default:
				break;
			}

			if( bSucceed )
			{
				for( int iX = newPointsRect.x; iX < newPointsRect.GetRight(); iX++ )
				{
					for( int iY = newPointsRect.y; iY < newPointsRect.GetBottom(); iY++ )
					{
						if( !vecGenPointValid[iX + iY * nWidth] )
						{
							bSucceed = false;
							break;
						}
					}
					if( !bSucceed )
						break;
				}
			}

			if( !bSucceed )
				nExtendDirs[iExtendDir] = nExtendDirs[--nExtendDirCount];
			else
			{
				roomRect = newPointsRect + roomRect;
				j++;
			}
		}

		SRoom room;
		room.nType = roomRect.width + roomRect.height <= nRoom2Size ? 0 : 1;
		room.rect = roomRect;
		m_rooms.push_back( room );

		for( int iX = roomRect.x; iX < roomRect.GetRight(); iX++ )
		{
			for( int iY = roomRect.y; iY < roomRect.GetBottom(); iY++ )
			{
				gendata[iX + iY * nWidth] = room.nType + eType_Room1;
			}
		}

		TRectangle<int32> invalidRect = roomRect;
		invalidRect.x -= nMinWidth;
		invalidRect.y -= nMinHeight;
		invalidRect.width += nMinWidth;
		invalidRect.height += nMinHeight;

		uint32 nLeftSpace = SRand::Inst().Rand( 0u, nMaxSpace );
		if( nLeftSpace )
			invalidRect.SetLeft( invalidRect.x - nLeftSpace );
		uint32 nTopSpace = SRand::Inst().Rand( 0u, nMaxSpace );
		if( nTopSpace )
			invalidRect.SetTop( invalidRect.x - nTopSpace );
		uint32 nRightSpace = SRand::Inst().Rand( 0u, nMaxSpace );
		if( nRightSpace )
			invalidRect.width += nRightSpace;
		uint32 nBottomSpace = SRand::Inst().Rand( 0u, nMaxSpace );
		if( nBottomSpace )
			invalidRect.height += nBottomSpace;

		invalidRect = invalidRect * TRectangle<int32>( 0, 0, nWidth, nHeight );

		for( int iX = invalidRect.x; iX < invalidRect.GetRight(); iX++ )
		{
			for( int iY = invalidRect.y; iY < invalidRect.GetBottom(); iY++ )
			{
				vecGenPointValid[iX + iY * nWidth] = false;
			}
		}
	}

	struct SLess
	{
		bool operator () ( const SRoom& left, const SRoom& right )
		{
			return left.rect.y < right.rect.y;
		}
	};
	std::sort( m_rooms.begin(), m_rooms.end(), SLess() );
}

void CLevelGenNode1_1::PutHBars()
{
	float fHasBarChance = 0.5f;
	uint32 nMinBarLen = 6;
	uint32 nMaxBarLen = 16;
	uint32 nMaxBarDist = 2;
	float fWideBarPercent = 0.35f;
	float fExtendDownPercent = 0.1f;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;

	uint32 nBars = 0;
	for( int i = 0; i < m_rooms.size(); i++ )
	{
		auto& room = m_rooms[i];
		int iY = room.rect.y - 1;
		if( iY < 0 )
			continue;
		float fChance = Max( Min( i * fHasBarChance - nBars, 1.0f ), 0.0f );
		if( SRand::Inst().Rand( 0.0f, 1.0f ) >= fChance )
			continue;

		int32 nMaxBaseLen = 0;
		int32 nMaxBaseLenBegin = 0;
		int32 nBaseLen = 0;
		for( int iX = room.rect.x; iX < room.rect.GetRight(); iX++ )
		{
			if( gendata[iX + iY * nWidth] )
				nBaseLen = 0;
			else
			{
				nBaseLen++;
				if( nBaseLen > nMaxBaseLen )
				{
					nMaxBaseLen = nBaseLen;
					nMaxBaseLenBegin = iX - nBaseLen + 1;
				}
			}
		}

		if( nMaxBaseLen >= room.rect.width * 0.75f )
		{
			TRectangle<int32> rect( nMaxBaseLenBegin, iY, nMaxBaseLen, 1 );
			bool bCanExtendLeft = true;
			bool bCanExtendRight = true;
			bool bCanExtendDown = true;

			while( bCanExtendRight || bCanExtendLeft )
			{
				uint8 nOp;
				if( rect.width >= nMinBarLen && bCanExtendDown )
				{
					if( ( !bCanExtendLeft && !bCanExtendRight ) || SRand::Inst().Rand( 0.0f, 1.0f ) < fExtendDownPercent )
						nOp = 2;
					else
					{
						if( !bCanExtendLeft )
							nOp = 1;
						else if( !bCanExtendRight )
							nOp = 0;
						else
							nOp = SRand::Inst().Rand( 0, 2 );
					}
				}
				else
				{
					if( !bCanExtendLeft && !bCanExtendRight )
						break;

					if( !bCanExtendLeft )
						nOp = 1;
					else if( !bCanExtendRight )
						nOp = 0;
					else
						nOp = SRand::Inst().Rand( 0, 2 );
				}

				bool bSucceed = true;
				TRectangle<int32> newPointsRect;
				switch( nOp )
				{
				case 0:
					if( rect.width >= nMaxBarLen || rect.x <= 0 )
					{
						bSucceed = false;
						break;
					}
					newPointsRect = TRectangle<int32>( rect.x - 1, rect.y, 1, rect.height );
					break;
				case 1:
					if( rect.width >= nMaxBarLen || rect.GetRight() >= nWidth )
					{
						bSucceed = false;
						break;
					}
					newPointsRect = TRectangle<int32>( rect.GetRight(), rect.y, 1, rect.height );
					break;
				case 2:
					if( rect.height >= nMaxBarDist + 1 || rect.y <= 0 )
					{
						bSucceed = false;
						break;
					}
					newPointsRect = TRectangle<int32>( rect.x, rect.y - 1, rect.width, 1 );
					break;
				}

				if( bSucceed )
				{
					for( int iX = newPointsRect.x; iX < newPointsRect.GetRight(); iX++ )
					{
						for( int iY = newPointsRect.y; iY < newPointsRect.GetBottom(); iY++ )
						{
							if( gendata[iX + iY * nWidth] )
							{
								bSucceed = false;
								break;
							}
						}
						if( !bSucceed )
							break;
					}
				}

				if( bSucceed )
					rect = rect + newPointsRect;
				else
				{
					if( nOp == 0 )
						bCanExtendLeft = false;
					else if( nOp == 1 )
						bCanExtendRight = false;
					else
						bCanExtendDown = false;
				}
			}

			if( rect.width < nMinBarLen )
				continue;

			auto barRect = rect;
			barRect.height = 1;
			if( rect.height >= 2 && SRand::Inst().Rand( 0.0f, 1.0f ) < fWideBarPercent )
				barRect.height = 2;

			SBar bar;
			bar.rect = barRect;
			bar.nType = barRect.height > 1 ? 1 : 0;
			m_bars.push_back( bar );

			for( int iX = barRect.x; iX < barRect.GetRight(); iX++ )
			{
				for( int iY = barRect.y; iY < barRect.GetBottom(); iY++ )
				{
					gendata[iX + iY * nWidth] = eType_Bar;
				}
			}
			nBars++;
		}
	}
}

void CLevelGenNode1_1::ConnRooms()
{
	const float fLoopPercent = 0.1f;
	const float fLoopChance = 0.3f;
	const float fBackDoorChance = 0.25f;
	vector<uint8> vecFlags;
	vector<int32> vecPars;
	vector<int32> vecGroups;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;

	vector<TVector2<int32> > q;
	vector<bool> vecIsConn;

	vecFlags.resize( nWidth * nHeight );
	vecPars.resize( nWidth * nHeight );
	vecGroups.resize( nWidth * nHeight );

	memset( &vecPars[0], -1, sizeof( int32 ) * nWidth * nHeight );
	memset( &vecGroups[0], -1, sizeof( int32 ) * nWidth * nHeight );
	CUnionFind g;
	g.Init( m_rooms.size() );
	uint32 nLoopCount = fLoopPercent * m_rooms.size();
	vecIsConn.resize( m_rooms.size() * m_rooms.size() );

	for( int i = 0; i < nWidth * nHeight; i++ )
	{
		if( gendata[i] )
			vecFlags[i] = -1;
	}
	for( int i = 0; i < m_rooms.size(); i++ )
	{
		auto& room = m_rooms[i];
		for( int iX = room.rect.x; iX < room.rect.GetRight(); iX++ )
		{
			for( int iY = room.rect.y; iY < room.rect.GetBottom(); iY++ )
			{
				vecGroups[iX + iY * nWidth] = i;
			}
		}

		if( room.nType == 0 )
		{
			for( int i1 = 2; i1 < room.rect.width - 2; i1++ )
			{
				auto pos = TVector2<int32>( i1, 0 ) + TVector2<int32>( room.rect.x, room.rect.y );
				vecFlags[pos.x + pos.y * nWidth] = 2;
				q.push_back( pos );
				pos = TVector2<int32>( i1, room.rect.height - 1 ) + TVector2<int32>( room.rect.x, room.rect.y );
				vecFlags[pos.x + pos.y * nWidth] = 2;
				q.push_back( pos );
			}
		}
		else
		{
			for( int i1 = 1; i1 < room.rect.width - 1; i1++ )
			{
				auto pos = TVector2<int32>( i1, 0 ) + TVector2<int32>( room.rect.x, room.rect.y );
				vecFlags[pos.x + pos.y * nWidth] = 2;
				q.push_back( pos );
				pos = TVector2<int32>( i1, room.rect.height - 1 ) + TVector2<int32>( room.rect.x, room.rect.y );
				vecFlags[pos.x + pos.y * nWidth] = 2;
				q.push_back( pos );
			}
		}
		for( int i1 = 1; i1 < room.rect.height - 1; i1++ )
		{
			auto pos = TVector2<int32>( 0, i1 ) + TVector2<int32>( room.rect.x, room.rect.y );
			vecFlags[pos.x + pos.y * nWidth] = 2;
			q.push_back( pos );
			pos = TVector2<int32>( room.rect.width - 1, i1 ) + TVector2<int32>( room.rect.x, room.rect.y );
			vecFlags[pos.x + pos.y * nWidth] = 2;
			q.push_back( pos );
		}
	}

	if( q.size() )
		SRand::Inst().Shuffle( &q[0], q.size() );

	TVector2<int32> ofs[4] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };
	for( int i = 0; i < q.size(); i++ )
	{
		TVector2<int32> pos = q[i];
		int x = q[i].x;
		int y = q[i].y;
		vecFlags[pos.x + pos.y * nWidth] = 1;
		int32 nGroup = vecGroups[pos.x + pos.y * nWidth];

		SRand::Inst().Shuffle( ofs, 4 );
		for( int k = 0; k < 4; k++ )
		{
			auto pos1 = q[i] + ofs[k];
			if( pos1.x < 0 || pos1.y < 0 || pos1.x >= nWidth || pos1.y >= nHeight )
				continue;

			int8 nFlag = vecFlags[pos1.x + pos1.y * nWidth];
			int32& nGroup1 = vecGroups[pos1.x + pos1.y * nWidth];
			if( nFlag )
			{
				if( nFlag < 0 )
					continue;

				if( nGroup1 >= 0 && nGroup1 != nGroup )
				{
					if( vecIsConn[nGroup1 + nGroup * m_rooms.size()] )
						continue;
					vecIsConn[nGroup1 + nGroup * m_rooms.size()] = vecIsConn[nGroup + nGroup1 * m_rooms.size()] = true;
					if( !g.Union( nGroup1, nGroup ) )
					{
						if( !nLoopCount )
							continue;
						if( SRand::Inst().Rand( 0.0f, 1.0f ) >= fLoopChance )
							continue;

						if( vecPars[pos.x + pos.y * nWidth] < 0 && vecPars[pos1.x + pos1.y * nWidth] < 0 )
							continue;

						nLoopCount--;
					}

					TVector2<int32> p = pos;
					for( ;; )
					{
						int32 par = vecPars[p.x + p.y * nWidth];
						if( par < 0 )
						{
							gendata[p.x + p.y * nWidth] = eType_Door;
							break;
						}
						if( gendata[p.x + p.y * nWidth] == eType_Path )
							break;
						gendata[p.x + p.y * nWidth] = eType_Path;
						p = q[par];
					}
					p = pos1;
					for( ;; )
					{
						int32 par = vecPars[p.x + p.y * nWidth];
						if( par < 0 )
						{
							gendata[p.x + p.y * nWidth] = eType_Door;
							break;
						}
						if( gendata[p.x + p.y * nWidth] == eType_Path )
							break;
						gendata[p.x + p.y * nWidth] = eType_Path;
						p = q[par];
					}
				}
				continue;
			}
			if( vecPars[pos1.x + pos1.y * nWidth] >= 0 )
				continue;

			q.push_back( pos1 );
			vecPars[pos1.x + pos1.y * nWidth] = i;
			nGroup1 = nGroup;
		}
	}

	for( auto& room : m_rooms )
	{
		auto& rect = room.rect;

		{
			int32 xBegin = rect.x;
			int32 xEnd = rect.x + rect.width;
			if( rect.y >= 3 && SRand::Inst().Rand( 0.0f, 1.0f ) < fBackDoorChance )
			{
				int32 yBegin = rect.y - 3;
				int32 yEnd = rect.y;
				int32 yBase = rect.y;
				int32 yBase1 = rect.y - 1;
				int32 nMaxLen = 0;
				int32 nMaxLenBegin = 0;
				int32 nCurLen = 0;
				for( int i = xBegin; i < xEnd; i++ )
				{
					bool bSuccess = true;
					if( gendata[i + yBase * nWidth] == eType_Door )
						bSuccess = false;
					else
					{
						for( int j = yBegin; j < yEnd; j++ )
						{
							if( gendata[i + j * nWidth] )
							{
								bSuccess = false;
								break;
							}
						}
					}

					if( !bSuccess )
						nCurLen = 0;
					else
					{
						nCurLen++;
						if( nCurLen > nMaxLen )
						{
							nMaxLen = nCurLen;
							nMaxLenBegin = i - nCurLen + 1;
						}
					}
				}

				if( nMaxLen >= 5 )
				{
					for( int iX = nMaxLenBegin + 2; iX < nMaxLenBegin + nMaxLen - 2; iX++ )
						gendata[iX + yBase1 * nWidth] = eType_Path;
					int32 nDoorPos = SRand::Inst().Rand( nMaxLenBegin + 2, nMaxLenBegin + nMaxLen - 2 );
					gendata[nDoorPos + yBase * nWidth] = eType_Door;
				}
			}

			if( rect.y + rect.height <= nHeight - 3 && SRand::Inst().Rand( 0.0f, 1.0f ) < fBackDoorChance )
			{
				int32 yBegin = rect.y + rect.height;
				int32 yEnd = rect.y + rect.height + 3;
				int32 yBase = rect.y + rect.height - 1;
				int32 yBase1 = rect.y + rect.height;
				int32 nMaxLen = 0;
				int32 nMaxLenBegin = 0;
				int32 nCurLen = 0;
				for( int i = xBegin; i < xEnd; i++ )
				{
					bool bSuccess = true;
					if( gendata[i + yBase * nWidth] == eType_Door )
						bSuccess = false;
					else
					{
						for( int j = yBegin; j < yEnd; j++ )
						{
							if( gendata[i + j * nWidth] )
							{
								bSuccess = false;
								break;
							}
						}
					}

					if( !bSuccess )
						nCurLen = 0;
					else
					{
						nCurLen++;
						if( nCurLen > nMaxLen )
						{
							nMaxLen = nCurLen;
							nMaxLenBegin = i - nCurLen + 1;
						}
					}
				}

				if( nMaxLen >= 5 )
				{
					for( int iX = nMaxLenBegin + 2; iX < nMaxLenBegin + nMaxLen - 2; iX++ )
						gendata[iX + yBase1 * nWidth] = eType_Path;
					int32 nDoorPos = SRand::Inst().Rand( nMaxLenBegin + 2, nMaxLenBegin + nMaxLen - 2 );
					gendata[nDoorPos + yBase * nWidth] = eType_Door;
				}
			}
		}


		{
			int32 yBegin = rect.y;
			int32 yEnd = rect.y + rect.height;
			if( rect.x >= 3 && SRand::Inst().Rand( 0.0f, 1.0f ) < fBackDoorChance )
			{
				int32 xBegin = rect.x - 3;
				int32 xEnd = rect.x;
				int32 xBase = rect.x;
				int32 xBase1 = rect.x - 1;
				int32 nMaxLen = 0;
				int32 nMaxLenBegin = 0;
				int32 nCurLen = 0;
				for( int i = yBegin; i < yEnd; i++ )
				{
					bool bSuccess = true;
					if( gendata[xBase + i * nWidth] == eType_Door )
						bSuccess = false;
					else
					{
						for( int j = xBegin; j < xEnd; j++ )
						{
							if( gendata[j + i * nWidth] )
							{
								bSuccess = false;
								break;
							}
						}
					}

					if( !bSuccess )
						nCurLen = 0;
					else
					{
						nCurLen++;
						if( nCurLen > nMaxLen )
						{
							nMaxLen = nCurLen;
							nMaxLenBegin = i - nCurLen + 1;
						}
					}
				}

				if( nMaxLen >= 5 )
				{
					for( int iY = nMaxLenBegin + 2; iY < nMaxLenBegin + nMaxLen - 2; iY++ )
						gendata[xBase1 + iY * nWidth] = eType_Path;
					int32 nDoorPos = SRand::Inst().Rand( nMaxLenBegin + 2, nMaxLenBegin + nMaxLen - 2 );
					gendata[xBase + nDoorPos * nWidth] = eType_Door;
				}
			}

			if( rect.x + rect.width <= nWidth - 3 && SRand::Inst().Rand( 0.0f, 1.0f ) < fBackDoorChance )
			{
				int32 xBegin = rect.x + rect.width;
				int32 xEnd = rect.x + rect.width + 3;
				int32 xBase = rect.x + rect.width - 1;
				int32 xBase1 = rect.x + rect.width;
				int32 nMaxLen = 0;
				int32 nMaxLenBegin = 0;
				int32 nCurLen = 0;
				for( int i = yBegin; i < yEnd; i++ )
				{
					bool bSuccess = true;
					if( gendata[xBase + i * nWidth] == eType_Door )
						bSuccess = false;
					else
					{
						for( int j = xBegin; j < xEnd; j++ )
						{
							if( gendata[j + i * nWidth] )
							{
								bSuccess = false;
								break;
							}
						}
					}

					if( !bSuccess )
						nCurLen = 0;
					else
					{
						nCurLen++;
						if( nCurLen > nMaxLen )
						{
							nMaxLen = nCurLen;
							nMaxLenBegin = i - nCurLen + 1;
						}
					}
				}

				if( nMaxLen >= 5 )
				{
					for( int iY = nMaxLenBegin + 2; iY < nMaxLenBegin + nMaxLen - 2; iY++ )
						gendata[xBase1 + iY * nWidth] = eType_Path;
					int32 nDoorPos = SRand::Inst().Rand( nMaxLenBegin + 2, nMaxLenBegin + nMaxLen - 2 );
					gendata[xBase + nDoorPos * nWidth] = eType_Door;
				}
			}
		}
	}
}

void CLevelGenNode1_1::GenConnAreas()
{
	const uint32 nMaxDist = 2;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;
	vector<int8> gendataTemp;
	gendataTemp.resize( nWidth * nHeight );

	for( int i = 0; i < nWidth * nHeight; i++ )
		gendataTemp[i] = gendata[i] == eType_Path ? 1 : 0;

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			auto& dst = gendata[i + j * nWidth];
			if( dst )
				continue;

			const TVector2<int32> ofs[4] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };
			bool bConn[4] = { false };
			for( int k = 0; k < 4; k++ )
			{
				int32 x = i + ofs[k].x;
				int32 y = j + ofs[k].y;
				bConn[k] = x >= 0 && y >= 0 && x < nWidth && y < nHeight && gendataTemp[x + y * nWidth];
			}

			if( bConn[0] && bConn[2] || bConn[1] && bConn[3] )
				dst = eType_Path;
		}
	}

	vector<int32> vecGroups;
	vector<TVector2<int32> > q;
	vecGroups.resize( nWidth * nHeight );
	vector<int32> vecDists;
	vecDists.resize( nWidth * nHeight );
	memset( &vecGroups[0], -1, sizeof( int32 ) * nWidth * nHeight );
	uint32 nGroup = 0;
	int iq = 0;
	for( int i = 0; i < nWidth * nHeight; i++ )
	{
		if( gendata[i] == eType_Path && vecGroups[i] < 0 )
		{
			TVector2<int32> pos0( i % nWidth, i / nWidth );
			q.push_back( pos0 );
			vecGroups[i] = nGroup;

			for( ; iq < q.size(); iq++ )
			{
				TVector2<int32> pos = q[iq];

				const TVector2<int32> ofs[4] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };
				for( int iOfs = 0; iOfs < 4; iOfs++ )
				{
					TVector2<int32> pos1 = pos + ofs[iOfs];
					if( pos1.x < 0 || pos1.y < 0 || pos1.x >= nWidth || pos1.y >= nHeight )
						continue;
					if( gendata[pos1.x + pos1.y * nWidth] == eType_Path && vecGroups[pos1.x + pos1.y * nWidth] < 0 )
					{
						vecGroups[pos1.x + pos1.y * nWidth] = nGroup;
						q.push_back( pos1 );
					}
				}
			}
			nGroup++;
		}
	}

	if( q.size() )
		SRand::Inst().Shuffle( &q[0], q.size() );
	for( iq = 0; iq < q.size(); iq++ )
	{
		TVector2<int32> pos = q[iq];
		int32 nDist = vecDists[pos.x + pos.y * nWidth];
		if( nDist >= nMaxDist )
			continue;
		int32 nGroup = vecGroups[pos.x + pos.y * nWidth];

		TVector2<int32> ofs[4] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };
		SRand::Inst().Shuffle( ofs, 4 );
		for( int iOfs = 0; iOfs < 4; iOfs++ )
		{
			TVector2<int32> pos1 = pos + ofs[iOfs];
			if( pos1.x < 0 || pos1.y < 0 || pos1.x >= nWidth || pos1.y >= nHeight )
				continue;
			if( gendata[pos1.x + pos1.y * nWidth] )
				continue;
			if( vecGroups[pos1.x + pos1.y * nWidth] < 0 )
			{
				gendata[pos1.x + pos1.y * nWidth] = eType_Path;
				vecGroups[pos1.x + pos1.y * nWidth] = nGroup;
				vecDists[pos1.x + pos1.y * nWidth] = nDist + SRand::Inst().Rand( 1, 3 );
				q.push_back( pos1 );
			}
		}
	}

	for( int iX = 0; iX < nWidth - 1; iX++ )
	{
		for( int iY = 0; iY < nHeight - 1; iY++ )
		{
			int i0 = iX + iY * nWidth;
			int i1 = iX + 1 + iY * nWidth;
			int i2 = iX + ( iY + 1 ) * nWidth;
			int i3 = iX + 1 + ( iY + 1 ) * nWidth;
			if( vecGroups[i0] >= 0 && vecGroups[i1] >= 0 && vecGroups[i0] != vecGroups[i1] )
				gendata[i0] = gendata[i1] = 0;
			if( vecGroups[i0] >= 0 && vecGroups[i2] >= 0 && vecGroups[i0] != vecGroups[i2] )
				gendata[i0] = gendata[i2] = 0;
			if( vecGroups[i0] >= 0 && vecGroups[i3] >= 0 && vecGroups[i0] != vecGroups[i3] )
				gendata[i0] = gendata[i3] = 0;
			if( vecGroups[i1] >= 0 && vecGroups[i2] >= 0 && vecGroups[i1] != vecGroups[i2] )
				gendata[i1] = gendata[i2] = 0;
		}
	}
}

void CLevelGenNode1_1::GenDoors()
{
	const float fDoorWidth2Chance = 0.8f;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;

	for( auto& room : m_rooms )
	{
		auto& rect = room.rect;

		{
			int32 xBegin = room.nType == 1 ? rect.x + 1 : rect.x + 2;
			int32 xEnd = room.nType == 1 ? rect.x + rect.width - 1 : rect.x + rect.width - 2;

			if( rect.y > 0 )
			{
				int32 nCurLen = 0;
				int32 nDoorCount = 0;

				for( int i = xBegin; i < xEnd; i++ )
				{
					TVector2<int32> pos( i, rect.y );
					TVector2<int32> pos1( i, rect.y - 1 );
					if( gendata[pos1.x + pos1.y * nWidth] == eType_Path )
					{
						nCurLen++;
						if( gendata[pos.x + pos.y * nWidth] == eType_Door )
							nDoorCount++;
					}
					else
					{
						if( nCurLen && nDoorCount )
						{
							int32 nDoorBegin = i - nCurLen + SRand::Inst().Rand( 0, nCurLen );
							for( int iDoor = i - nCurLen; iDoor < i; iDoor++ )
							{
								gendata[iDoor + pos.y * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
							}

						}
						nCurLen = 0;
						nDoorCount = 0;

						if( gendata[pos1.x + pos1.y * nWidth] != eType_Door )
							gendata[pos.x + pos.y * nWidth] = room.nType + eType_Room1;
					}
				}
				if( nCurLen && nDoorCount )
				{
					int32 nDoorBegin = xEnd - nCurLen + SRand::Inst().Rand( 0, nCurLen );
					for( int iDoor = xEnd - nCurLen; iDoor < xEnd; iDoor++ )
					{
						gendata[iDoor + rect.y * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
					}

					nCurLen = 0;
				}

				for( int i = xBegin; i < xEnd; i++ )
				{
					TVector2<int32> pos( i, rect.y );
					if( gendata[pos.x + pos.y * nWidth] == eType_Door )
					{
						TVector2<int32> posLeft( i - 1, pos.y );
						TVector2<int32> posRight( i + 1, pos.y );
						bool bLeft = i - 1 >= ( room.nType == 1 ? rect.x + 1 : rect.x + 2 ) && gendata[i - 2 + pos.y * nWidth] != eType_Door;
						bool bRight = i + 1 < ( room.nType == 1 ? rect.x + rect.width - 1 : rect.x + rect.width - 2 ) && gendata[i + 2 + pos.y * nWidth] != eType_Door;
						if( bLeft && bRight )
						{
							if( SRand::Inst().Rand( 0, 2 ) )
								gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
							else
								gendata[posRight.x + posRight.y * nWidth] = eType_Door;
						}
						else if( bLeft )
							gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
						else if( bRight )
							gendata[posRight.x + posRight.y * nWidth] = eType_Door;

						i++;
					}
				}
			}

			if( rect.y + rect.height <= nHeight - 1 )
			{
				int32 nCurLen = 0;
				int32 nDoorCount = 0;

				for( int i = xBegin; i < xEnd; i++ )
				{
					TVector2<int32> pos( i, rect.y + rect.height - 1 );
					TVector2<int32> pos1( i, rect.y + rect.height );
					if( gendata[pos1.x + pos1.y * nWidth] == eType_Path )
					{
						nCurLen++;
						if( gendata[pos.x + pos.y * nWidth] == eType_Door )
							nDoorCount++;
					}
					else
					{
						if( nCurLen && nDoorCount )
						{
							int32 nDoorBegin = i - nCurLen + SRand::Inst().Rand( 0, nCurLen );
							for( int iDoor = i - nCurLen; iDoor < i; iDoor++ )
							{
								gendata[iDoor + pos.y * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
							}

						}
						nCurLen = 0;
						nDoorCount = 0;

						if( gendata[pos1.x + pos1.y * nWidth] != eType_Door )
							gendata[pos.x + pos.y * nWidth] = room.nType + eType_Room1;
					}
				}
				if( nCurLen && nDoorCount )
				{
					int32 nDoorBegin = xEnd - nCurLen + SRand::Inst().Rand( 0, nCurLen );
					for( int iDoor = xEnd - nCurLen; iDoor < xEnd; iDoor++ )
					{
						gendata[iDoor + ( rect.y + rect.height - 1 ) * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
					}

					nCurLen = 0;
				}

				for( int i = xBegin; i < xEnd; i++ )
				{
					TVector2<int32> pos( i, rect.y + rect.height - 1 );
					if( gendata[pos.x + pos.y * nWidth] == eType_Door )
					{
						TVector2<int32> posLeft( i - 1, pos.y );
						TVector2<int32> posRight( i + 1, pos.y );
						bool bLeft = i - 1 >= ( room.nType == 1 ? rect.x + 1 : rect.x + 2 ) && gendata[i - 2 + pos.y * nWidth] != eType_Door;
						bool bRight = i + 1 < ( room.nType == 1 ? rect.x + rect.width - 1 : rect.x + rect.width - 2 ) && gendata[i + 2 + pos.y * nWidth] != eType_Door;
						if( bLeft && bRight )
						{
							if( SRand::Inst().Rand( 0, 2 ) )
								gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
							else
								gendata[posRight.x + posRight.y * nWidth] = eType_Door;
						}
						else if( bLeft )
							gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
						else if( bRight )
							gendata[posRight.x + posRight.y * nWidth] = eType_Door;

						i++;
					}
				}
			}
		}

		{
			int32 yBegin = rect.y + 1;
			int32 yEnd = rect.y + rect.height - 1;

			if( rect.x > 0 )
			{
				int32 nCurLen = 0;
				int32 nDoorCount = 0;

				for( int i = yBegin; i < yEnd; i++ )
				{
					TVector2<int32> pos( rect.x, i );
					TVector2<int32> pos1( rect.x - 1, i );
					if( gendata[pos1.x + pos1.y * nWidth] == eType_Path )
					{
						nCurLen++;
						if( gendata[pos.x + pos.y * nWidth] == eType_Door )
							nDoorCount++;
					}
					else
					{
						if( nCurLen && nDoorCount )
						{
							int32 nDoorBegin = i - nCurLen + SRand::Inst().Rand( 0, nCurLen );
							for( int iDoor = i - nCurLen; iDoor < i; iDoor++ )
							{
								gendata[pos.x + iDoor * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
							}

						}
						nCurLen = 0;
						nDoorCount = 0;

						if( gendata[pos1.x + pos1.y * nWidth] != eType_Door )
							gendata[pos.x + pos.y * nWidth] = room.nType + eType_Room1;
					}
				}
				if( nCurLen && nDoorCount )
				{
					int32 nDoorBegin = yEnd - nCurLen + SRand::Inst().Rand( 0, nCurLen );
					for( int iDoor = yEnd - nCurLen; iDoor < yEnd; iDoor++ )
					{
						gendata[rect.x + iDoor * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
					}

					nCurLen = 0;
				}

				for( int i = yBegin; i < yEnd; i++ )
				{
					TVector2<int32> pos( rect.x, i );
					if( gendata[pos.x + pos.y * nWidth] == eType_Door && ( i == rect.y + 2 || i == rect.y + rect.height - 3 || SRand::Inst().Rand( 0.0f, 1.0f ) < fDoorWidth2Chance ) )
					{
						TVector2<int32> posLeft( pos.x, i - 1 );
						TVector2<int32> posRight( pos.x, i + 1 );
						bool bLeft = i - 1 >= rect.y + 1 && ( room.nType == 1 || i - 1 != rect.y + 2 && i - 1 != rect.y + rect.height - 4 ) && gendata[pos.x + ( i - 2 ) * nWidth] != eType_Door;
						bool bRight = i + 1 < rect.y + rect.height - 1 && ( room.nType == 1 || i != rect.y + 2 && i != rect.y + rect.height - 4 ) && gendata[pos.x + ( i + 2 ) * nWidth] != eType_Door;
						if( bLeft && bRight )
						{
							if( SRand::Inst().Rand( 0, 2 ) )
								gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
							else
								gendata[posRight.x + posRight.y * nWidth] = eType_Door;
						}
						else if( bLeft )
							gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
						else if( bRight )
							gendata[posRight.x + posRight.y * nWidth] = eType_Door;

						i++;
					}
				}
			}

			if( rect.x + rect.width <= nWidth - 1 )
			{
				int32 nCurLen = 0;
				int32 nDoorCount = 0;

				for( int i = yBegin; i < yEnd; i++ )
				{
					TVector2<int32> pos( rect.x + rect.width - 1, i );
					TVector2<int32> pos1( rect.x + rect.width, i );
					if( gendata[pos1.x + pos1.y * nWidth] == eType_Path )
					{
						nCurLen++;
						if( gendata[pos.x + pos.y * nWidth] == eType_Door )
							nDoorCount++;
					}
					else
					{
						if( nCurLen && nDoorCount )
						{
							int32 nDoorBegin = i - nCurLen + SRand::Inst().Rand( 0, nCurLen );
							for( int iDoor = i - nCurLen; iDoor < i; iDoor++ )
							{
								gendata[pos.x + iDoor * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
							}

						}
						nCurLen = 0;
						nDoorCount = 0;

						if( gendata[pos1.x + pos1.y * nWidth] != eType_Door )
							gendata[pos.x + pos.y * nWidth] = room.nType + eType_Room1;
					}
				}
				if( nCurLen && nDoorCount )
				{
					int32 nDoorBegin = yEnd - nCurLen + SRand::Inst().Rand( 0, nCurLen );
					for( int iDoor = yEnd - nCurLen; iDoor < yEnd; iDoor++ )
					{
						gendata[rect.x + rect.width - 1 + iDoor * nWidth] = iDoor == nDoorBegin ? eType_Door : room.nType + eType_Room1;
					}

					nCurLen = 0;
				}

				for( int i = yBegin; i < yEnd; i++ )
				{
					TVector2<int32> pos( rect.x + rect.width - 1, i );
					if( gendata[pos.x + pos.y * nWidth] == eType_Door && ( i == rect.y + 2 || i == rect.y + rect.height - 3 || SRand::Inst().Rand( 0.0f, 1.0f ) < fDoorWidth2Chance ) )
					{
						TVector2<int32> posLeft( pos.x, i - 1 );
						TVector2<int32> posRight( pos.x, i + 1 );
						bool bLeft = i - 1 >= rect.y + 1 && ( room.nType == 1 || i - 1 != rect.y + 2 && i - 1 != rect.y + rect.height - 4 ) && gendata[pos.x + ( i - 2 ) * nWidth] != eType_Door;
						bool bRight = i + 1 < rect.y + rect.height - 1 && ( room.nType == 1 || i != rect.y + 2 && i != rect.y + rect.height - 4 ) && gendata[pos.x + ( i + 2 ) * nWidth] != eType_Door;
						if( bLeft && bRight )
						{
							if( SRand::Inst().Rand( 0, 2 ) )
								gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
							else
								gendata[posRight.x + posRight.y * nWidth] = eType_Door;
						}
						else if( bLeft )
							gendata[posLeft.x + posLeft.y * nWidth] = eType_Door;
						else if( bRight )
							gendata[posRight.x + posRight.y * nWidth] = eType_Door;

						i++;
					}
				}
			}
		}
	}
}

void CLevelGenNode1_1::GenEmptyArea()
{
	const int32 nMaxDist = 9;
	const float fChance = 0.5f;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;

	vector<int32> vecDist;
	vecDist.resize( nWidth * nHeight );
	vector<TVector2<int32> > q;

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( gendata[i + j * nWidth] == eType_Path )
			{
				vecDist[i + j * nWidth] = 0;
				q.push_back( TVector2<int32>( i, j ) );
			}
			else if( gendata[i + j * nWidth] == eType_None )
				vecDist[i + j * nWidth] = -1;
			else
				vecDist[i + j * nWidth] = -2;
		}
	}

	for( int i = 0; i < q.size(); i++ )
	{
		auto pos = q[i];
		int32 nDist = vecDist[pos.x + pos.y * nWidth];
		if( nDist >= nMaxDist )
			continue;

		TVector2<int32> ofs[8] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 },{ -1, -1 },{ 1, -1 },{ 1, 1 },{ -1, 1 } };
		SRand::Inst().Shuffle( ofs, 4 );
		SRand::Inst().Shuffle( ofs + 4, 4 );
		for( int i = 0; i < 8; i++ )
		{
			TVector2<int32> pos1 = pos + ofs[i];
			if( pos1.x >= 0 && pos1.y >= 0 && pos1.x < nWidth && pos1.y < nHeight
				&& vecDist[pos1.x + pos1.y * nWidth] == -1 )
			{
				vecDist[pos1.x + pos1.y * nWidth] = nDist + ( SRand::Inst().Rand( 0.0f, 1.0f ) < fChance ? 2 : 1 );
				q.push_back( pos1 );
			}
		}
	}

	for( int i = 0; i < nWidth * nHeight; i++ )
	{
		if( vecDist[i] == -1 )
			gendata[i] = eType_Path;
	}
}

void CLevelGenNode1_1::FillBlockArea()
{
	const int32 nMin = 60;
	const int32 nMax = 150;

	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;

	vector<TVector2<int32> > black;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( gendata[i + j * nWidth] == eType_None )
				black.push_back( TVector2<int32>( i, j ) );
			else if( j < nHeight - 1 && gendata[i + j * nWidth] == eType_Path )
			{
				if( gendata[i + ( j + 1 ) * nWidth] == eType_Room2 && SRand::Inst().Rand( 0.0f, 1.0f ) < 0.3f )
				{
					gendata[i + j * nWidth] = eType_Pipe;
				}
			}
		}
	}
	if( black.size() )
		SRand::Inst().Shuffle( &black[0], black.size() );

	for( int32 i = 0; i < black.size(); i++ )
	{
		TVector2<int32> p = black[i];
		if( gendata[p.x + p.y * nWidth] )
			continue;

		bool bCreatedPipe = false;
		int32 n = SRand::Inst().Rand( nMin, nMax );
		vector<TVector2<int32> > q;
		q.push_back( p );
		int8 nType = SRand::Inst().Rand( 0, 2 ) + eType_BlockRed;

		for( int32 iq = 0; iq < q.size(); iq++ )
		{
			TVector2<int32> pos = q[iq];
			gendata[pos.x + pos.y * nWidth] = nType;

			if( !bCreatedPipe && pos.y > 0 && gendata[pos.x + ( pos.y - 1 ) * nWidth] == eType_Path )
			{
				gendata[pos.x + ( pos.y - 1 ) * nWidth] = eType_Pipe;
				bCreatedPipe = true;
			}

			TVector2<int32> ofs[4] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };
			SRand::Inst().Shuffle( ofs, 4 );

			for( int k = 0; k < 4 && q.size() < n; k++ )
			{
				TVector2<int32> pos1 = pos + ofs[k];
				if( pos1.x >= 0 && pos1.y >= 0 && pos1.x < nWidth && pos1.y < nHeight && gendata[pos1.x + pos1.y * nWidth] == eType_None )
					q.push_back( pos1 );
			}
		}
	}
}

void CLevelGenNode1_1::GenObjects()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto& gendata = m_gendata;

	vector<TVector2<int32> > vecResults;

	int8 patterns[][9] =
	{
		{
			2, 2, 2,
			0, 4, 0,
			1, 1, 1
		},
		{
			2, 0, 0,
			1, 3, 0,
			1, 1, 2
		},
		{
			0, 0, 2,
			0, 3, 1,
			2, 1, 1
		},
		{
			1, 1, 0,
			1, 0, 0,
			1, 1, 2
		},
		{
			0, 1, 1,
			0, 0, 1,
			2, 1, 1
		},
	};
	bool bPatternOk[5][eType_Count] =
	{
		{ false, false, false, false, false, false, false, true, false },
		{ true, true, true, true, true, true, false, false, false },
		{ true, true, true, true, true, true, false, true, false },
		{ true, true, true, false, false, false, false, false, false },
		{ true, true, true, false, false, false, false, true, false },
	};
	float fPatternPercent[] = { 0.4f, 0.8f, 0.8f, 1.0f, 1.0f };

	TVector2<int32> ofs[9] = { { -1, 1 },{ 0, 1 },{ 1, 1 },{ -1, 0 },{ 0, 0 },{ 1, 0 },{ -1, -1 },{ 0, -1 },{ 1, -1 } };

	for( int i = 1; i < nWidth - 1; i++ )
	{
		for( int j = 1; j < nHeight - 1; j++ )
		{
			TVector2<int32> pos( i, j );
			for( int iPattern = 0; iPattern < 5; iPattern++ )
			{
				if( SRand::Inst().Rand( 0.0f, 1.0f ) >= fPatternPercent[iPattern] )
					continue;

				bool bSucceed = true;
				for( int k = 0; k < 9; k++ )
				{
					TVector2<int32> pos1 = pos + ofs[k];
					int8 nType = gendata[pos1.x + pos1.y * nWidth];
					int8 nPatternType = patterns[iPattern][k];

					if( !bPatternOk[nPatternType][nType] )
					{
						bSucceed = false;
						break;
					}
				}

				if( bSucceed )
				{
					vecResults.push_back( pos );
					break;
				}
			}
		}
	}

	for( int i = 0; i < vecResults.size(); i++ )
	{
		gendata[vecResults[i].x + vecResults[i].y * nWidth] = eType_Object;
	}
}
