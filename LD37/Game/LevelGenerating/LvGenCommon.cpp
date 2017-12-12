#include "stdafx.h"
#include "LvGenCommon.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"

void CBrickTileNode::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pBrick = CreateNode( pXml->FirstChildElement( "brick" )->FirstChildElement(), context );
	m_pBrick1 = CreateNode( pXml->FirstChildElement( "brick1" )->FirstChildElement(), context );
	m_bVertical = XmlGetAttr( pXml, "vertical", 0 );
	m_bOfs = XmlGetAttr( pXml, "ofs", 0 );
	CLevelGenerateNode::Load( pXml, context );
}

void CBrickTileNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	int8 nMask = context.mapTags["mask"];

	if( !m_bVertical )
	{
		for( int j = 0; j < region.height; j++ )
		{
			int32 y = j + region.y;
			for( int i = ( m_bOfs ? ( j + 1 ) & 1 : j & 1 ) - 1; i < region.width; i += 2 )
			{
				int32 x = i + region.x;
				bool b0 = i >= 0 && context.blueprint[x + y * context.nWidth] == nMask;
				bool b1 = i + 1 < region.width && context.blueprint[x + 1 + y * context.nWidth] == nMask;
				if( b0 && b1 )
					m_pBrick1->Generate( context, TRectangle<int32>( x, y, 2, 1 ) );
				else if( b0 )
					m_pBrick->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				else if( b1 )
					m_pBrick->Generate( context, TRectangle<int32>( x + 1, y, 1, 1 ) );
			}
		}
	}
	else
	{
		for( int i = 0; i < region.width; i++ )
		{
			int32 x = i + region.x;
			for( int j = ( m_bOfs ? ( i + 1 ) & 1 : i & 1 ) - 1; j < region.height; j += 2 )
			{
				int32 y = j + region.y;
				bool b0 = j >= 0 && context.blueprint[x + y * context.nWidth] == nMask;
				bool b1 = j + 1 < region.height && context.blueprint[x + ( y + 1 ) * context.nWidth] == nMask;
				if( b0 && b1 )
					m_pBrick1->Generate( context, TRectangle<int32>( x, y, 1, 2 ) );
				else if( b0 )
					m_pBrick->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
				else if( b1 )
					m_pBrick->Generate( context, TRectangle<int32>( x, y + 1, 1, 1 ) );
			}
		}
	}
}

void CCommonRoomNode::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pWallBroken = CreateNode( pXml->FirstChildElement( "wall_broken" )->FirstChildElement(), context );
	m_pWallBroken1 = CreateNode( pXml->FirstChildElement( "wall_broken1" )->FirstChildElement(), context );
}

void CCommonRoomNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		SLevelBuildContext tempContext( context.pLevel, pChunk );
		m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );

		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				if( pChunk->GetBlock( i, j )->eBlockType == eBlockType_Block )
					m_pWallBroken1->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
				else
					m_pWallBroken->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
			}
		}

		tempContext.Build();
	}
}

void CRoom0Node::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pDoor1[0] = CreateNode( pXml->FirstChildElement( "door1_0" )->FirstChildElement(), context );
	m_pDoor1[1] = CreateNode( pXml->FirstChildElement( "door1_1" )->FirstChildElement(), context );
	m_pDoor1[2] = CreateNode( pXml->FirstChildElement( "door1_2" )->FirstChildElement(), context );
	m_pDoor1[3] = CreateNode( pXml->FirstChildElement( "door1_3" )->FirstChildElement(), context );
	m_pDoor2[0] = CreateNode( pXml->FirstChildElement( "door2_0" )->FirstChildElement(), context );
	m_pDoor2[1] = CreateNode( pXml->FirstChildElement( "door2_1" )->FirstChildElement(), context );
	m_pDoor2[2] = CreateNode( pXml->FirstChildElement( "door2_2" )->FirstChildElement(), context );
	m_pDoor2[3] = CreateNode( pXml->FirstChildElement( "door2_3" )->FirstChildElement(), context );

	for( int i = 0; i < 4; i++ )
	{
		char sz[64];
		sprintf( sz, "obj_%d", i );
		auto pElem = pXml->FirstChildElement( sz );
		if( pElem )
			m_pObj[i] = CreateNode( pElem->FirstChildElement(), context );
	}
}

void CRoom0Node::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 nDoor = context.mapTags["door"];
		SLevelBuildContext tempContext( context.pLevel, pChunk );

		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 1; i < region.width - 1; i++ )
			{
				if( context.blueprint[i + region.x + region.y * context.nWidth] == nDoor )
				{
					if( i + 1 < region.width && context.blueprint[i + 1 + region.x + region.y * context.nWidth] == nDoor )
					{
						m_pDoor2[0]->Generate( tempContext, TRectangle<int32>( i, 0, 2, 1 ) );
						i++;
					}
					else
						m_pDoor1[0]->Generate( tempContext, TRectangle<int32>( i, 0, 1, 1 ) );
				}
				else if( m_pObj[0] )
					m_pObj[0]->Generate( context, TRectangle<int32>( i + region.x, region.y, 1, 1 ) );
			}
		}
		{
			int i;
			for( i = 1; i < region.width - 1; i++ )
			{
				if( context.blueprint[i + region.x + ( region.y + region.height - 1 ) * context.nWidth] == nDoor )
				{
					if( i + 1 < region.width && context.blueprint[i + 1 + region.x + ( region.y + region.height - 1 ) * context.nWidth] == nDoor )
					{
						m_pDoor2[2]->Generate( tempContext, TRectangle<int32>( i, region.height - 1, 2, 1 ) );
						i++;
					}
					else
						m_pDoor1[2]->Generate( tempContext, TRectangle<int32>( i, region.height - 1, 1, 1 ) );
				}
				else if( m_pObj[2] )
					m_pObj[2]->Generate( context, TRectangle<int32>( i + region.x, region.y + region.height - 1, 1, 1 ) );
			}
		}

		{
			int i;
			for( i = 1; i < region.height - 1; i++ )
			{
				if( context.blueprint[region.x + ( i + region.y ) * context.nWidth] == nDoor )
				{
					if( i + 1 < region.height - 1 && context.blueprint[region.x + ( i + 1 + region.y ) * context.nWidth] == nDoor )
					{
						m_pDoor2[1]->Generate( tempContext, TRectangle<int32>( 0, i, 1, 2 ) );
						i++;
					}
					else
						m_pDoor1[1]->Generate( tempContext, TRectangle<int32>( 0, i, 1, 1 ) );
				}
				else if( m_pObj[1] )
					m_pObj[1]->Generate( context, TRectangle<int32>( region.x, i + region.y, 1, 1 ) );
			}
		}
		{
			int i;
			for( i = 1; i < region.height - 1; i++ )
			{
				if( context.blueprint[region.x + region.width - 1 + ( i + region.y ) * context.nWidth] == nDoor )
				{
					if( i + 1 < region.height - 1 && context.blueprint[region.x + region.width - 1 + ( i + 1 + region.y ) * context.nWidth] == nDoor )
					{
						m_pDoor2[3]->Generate( tempContext, TRectangle<int32>( region.width - 1, i, 1, 2 ) );
						i++;
					}
					else
						m_pDoor1[3]->Generate( tempContext, TRectangle<int32>( region.width - 1, i, 1, 1 ) );
				}
				else if( m_pObj[3] )
					m_pObj[3]->Generate( context, TRectangle<int32>( region.x + region.width - 1, i + region.y, 1, 1 ) );
			}
		}

		tempContext.Build();
	}
}

void CRoom1Node::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pWallBroken = CreateNode( pXml->FirstChildElement( "wall_broken" )->FirstChildElement(), context );
	m_pHBar = CreateNode( pXml->FirstChildElement( "hbar" )->FirstChildElement(), context );
	m_pVBar = CreateNode( pXml->FirstChildElement( "vbar" )->FirstChildElement(), context );
	m_pDoor1[0] = CreateNode( pXml->FirstChildElement( "door1_0" )->FirstChildElement(), context );
	m_pDoor1[1] = CreateNode( pXml->FirstChildElement( "door1_1" )->FirstChildElement(), context );
	m_pDoor1[2] = CreateNode( pXml->FirstChildElement( "door1_2" )->FirstChildElement(), context );
	m_pDoor1[3] = CreateNode( pXml->FirstChildElement( "door1_3" )->FirstChildElement(), context );
	m_pDoor2[0] = CreateNode( pXml->FirstChildElement( "door2_0" )->FirstChildElement(), context );
	m_pDoor2[1] = CreateNode( pXml->FirstChildElement( "door2_1" )->FirstChildElement(), context );
	m_pDoor2[2] = CreateNode( pXml->FirstChildElement( "door2_2" )->FirstChildElement(), context );
	m_pDoor2[3] = CreateNode( pXml->FirstChildElement( "door2_3" )->FirstChildElement(), context );
}

void CRoom1Node::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 nDoor = context.mapTags["door"];
		SLevelBuildContext tempContext( context.pLevel, pChunk );

		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 0; i < region.width; i++ )
			{
				if( context.blueprint[i + region.x + region.y * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						m_pHBar->Generate( tempContext, TRectangle<int32>( i - nMaxLen, 0, nMaxLen, 1 ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.width && context.blueprint[i + 1 + region.x + region.y * context.nWidth] == nDoor )
					{
						m_pDoor2[0]->Generate( tempContext, TRectangle<int32>( i, 0, 2, 1 ) );
						i++;
					}
					else
						m_pDoor1[0]->Generate( tempContext, TRectangle<int32>( i, 0, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				m_pHBar->Generate( tempContext, TRectangle<int32>( i - nMaxLen, 0, nMaxLen, 1 ) );
				nMaxLen = 0;
			}
		}
		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 0; i < region.width; i++ )
			{
				if( context.blueprint[i + region.x + ( region.y + region.height - 1 ) * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						m_pHBar->Generate( tempContext, TRectangle<int32>( i - nMaxLen, region.height - 1, nMaxLen, 1 ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.width && context.blueprint[i + 1 + region.x + ( region.y + region.height - 1 ) * context.nWidth] == nDoor )
					{
						m_pDoor2[2]->Generate( tempContext, TRectangle<int32>( i, region.height - 1, 2, 1 ) );
						i++;
					}
					else
						m_pDoor1[2]->Generate( tempContext, TRectangle<int32>( i, region.height - 1, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				m_pHBar->Generate( tempContext, TRectangle<int32>( i - nMaxLen, region.height - 1, nMaxLen, 1 ) );
				nMaxLen = 0;
			}
		}

		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 1; i < region.height - 1; i++ )
			{
				if( context.blueprint[region.x + ( i + region.y ) * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						m_pVBar->Generate( tempContext, TRectangle<int32>( 0, i - nMaxLen, 1, nMaxLen ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.height - 1 && context.blueprint[region.x + ( i + 1 + region.y ) * context.nWidth] == nDoor )
					{
						m_pDoor2[1]->Generate( tempContext, TRectangle<int32>( 0, i, 1, 2 ) );
						i++;
					}
					else
						m_pDoor1[1]->Generate( tempContext, TRectangle<int32>( 0, i, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				m_pVBar->Generate( tempContext, TRectangle<int32>( 0, i - nMaxLen, 1, nMaxLen ) );
				nMaxLen = 0;
			}
		}
		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 1; i < region.height - 1; i++ )
			{
				if( context.blueprint[region.x + region.width - 1 + ( i + region.y ) * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						m_pVBar->Generate( tempContext, TRectangle<int32>( region.width - 1, i - nMaxLen, 1, nMaxLen ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.height - 1 && context.blueprint[region.x + region.width - 1 + ( i + 1 + region.y ) * context.nWidth] == nDoor )
					{
						m_pDoor2[3]->Generate( tempContext, TRectangle<int32>( region.width - 1, i, 1, 2 ) );
						i++;
					}
					else
						m_pDoor1[3]->Generate( tempContext, TRectangle<int32>( region.width - 1, i, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				m_pVBar->Generate( tempContext, TRectangle<int32>( region.width - 1, i - nMaxLen, 1, nMaxLen ) );
				nMaxLen = 0;
			}
		}

		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				m_pWallBroken->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
			}
		}

		tempContext.Build();
	}

}

void CRoom2Node::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pWallBroken = CreateNode( pXml->FirstChildElement( "wall_broken" )->FirstChildElement(), context );
	m_pHBar = CreateNode( pXml->FirstChildElement( "hbar" )->FirstChildElement(), context );
	m_pVBar = CreateNode( pXml->FirstChildElement( "vbar" )->FirstChildElement(), context );
	m_pCorner = CreateNode( pXml->FirstChildElement( "corner" )->FirstChildElement(), context );
	m_pDoor1[0] = CreateNode( pXml->FirstChildElement( "door1_0" )->FirstChildElement(), context );
	m_pDoor1[1] = CreateNode( pXml->FirstChildElement( "door1_1" )->FirstChildElement(), context );
	m_pDoor1[2] = CreateNode( pXml->FirstChildElement( "door1_2" )->FirstChildElement(), context );
	m_pDoor1[3] = CreateNode( pXml->FirstChildElement( "door1_3" )->FirstChildElement(), context );
	m_pDoor2[0] = CreateNode( pXml->FirstChildElement( "door2_0" )->FirstChildElement(), context );
	m_pDoor2[1] = CreateNode( pXml->FirstChildElement( "door2_1" )->FirstChildElement(), context );
	m_pDoor2[2] = CreateNode( pXml->FirstChildElement( "door2_2" )->FirstChildElement(), context );
	m_pDoor2[3] = CreateNode( pXml->FirstChildElement( "door2_3" )->FirstChildElement(), context );
}

void CRoom2Node::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 nDoor = context.mapTags["door"];
		SLevelBuildContext tempContext( context.pLevel, pChunk );

		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 1; i < region.width - 1; i++ )
			{
				if( context.blueprint[i + region.x + region.y * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pHBar )->Generate( tempContext, TRectangle<int32>( i - nMaxLen, 0, nMaxLen, 1 ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.width && context.blueprint[i + 1 + region.x + region.y * context.nWidth] == nDoor )
					{
						m_pDoor2[0]->Generate( tempContext, TRectangle<int32>( i, 0, 2, 1 ) );
						i++;
					}
					else
						m_pDoor1[0]->Generate( tempContext, TRectangle<int32>( i, 0, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				( nMaxLen == 1 ? m_pCorner : m_pHBar )->Generate( tempContext, TRectangle<int32>( i - nMaxLen, 0, nMaxLen, 1 ) );
				nMaxLen = 0;
			}
		}
		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 1; i < region.width - 1; i++ )
			{
				if( context.blueprint[i + region.x + ( region.y + region.height - 1 ) * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pHBar )->Generate( tempContext, TRectangle<int32>( i - nMaxLen, region.height - 1, nMaxLen, 1 ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.width && context.blueprint[i + 1 + region.x + ( region.y + region.height - 1 ) * context.nWidth] == nDoor )
					{
						m_pDoor2[2]->Generate( tempContext, TRectangle<int32>( i, region.height - 1, 2, 1 ) );
						i++;
					}
					else
						m_pDoor1[2]->Generate( tempContext, TRectangle<int32>( i, region.height - 1, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				( nMaxLen == 1 ? m_pCorner : m_pHBar )->Generate( tempContext, TRectangle<int32>( i - nMaxLen, region.height - 1, nMaxLen, 1 ) );
				nMaxLen = 0;
			}
		}

		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 1; i < region.height - 1; i++ )
			{
				if( context.blueprint[region.x + ( i + region.y ) * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pVBar )->Generate( tempContext, TRectangle<int32>( 0, i - nMaxLen, 1, nMaxLen ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.height - 1 && context.blueprint[region.x + ( i + 1 + region.y ) * context.nWidth] == nDoor )
					{
						m_pDoor2[1]->Generate( tempContext, TRectangle<int32>( 0, i, 1, 2 ) );
						i++;
					}
					else
						m_pDoor1[1]->Generate( tempContext, TRectangle<int32>( 0, i, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				( nMaxLen == 1 ? m_pCorner : m_pVBar )->Generate( tempContext, TRectangle<int32>( 0, i - nMaxLen, 1, nMaxLen ) );
				nMaxLen = 0;
			}
		}
		{
			uint32 nMaxLen = 0;
			int i;
			for( i = 1; i < region.height - 1; i++ )
			{
				if( context.blueprint[region.x + region.width - 1 + ( i + region.y ) * context.nWidth] == nDoor )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pVBar )->Generate( tempContext, TRectangle<int32>( region.width - 1, i - nMaxLen, 1, nMaxLen ) );
						nMaxLen = 0;
					}

					if( i + 1 < region.height - 1 && context.blueprint[region.x + region.width - 1 + ( i + 1 + region.y ) * context.nWidth] == nDoor )
					{
						m_pDoor2[3]->Generate( tempContext, TRectangle<int32>( region.width - 1, i, 1, 2 ) );
						i++;
					}
					else
						m_pDoor1[3]->Generate( tempContext, TRectangle<int32>( region.width - 1, i, 1, 1 ) );
				}
				else
					nMaxLen++;
			}
			if( nMaxLen )
			{
				( nMaxLen == 1 ? m_pCorner : m_pVBar )->Generate( tempContext, TRectangle<int32>( region.width - 1, i - nMaxLen, 1, nMaxLen ) );
				nMaxLen = 0;
			}
		}

		m_pCorner->Generate( tempContext, TRectangle<int32>( 0, 0, 1, 1 ) );
		m_pCorner->Generate( tempContext, TRectangle<int32>( region.width - 1, 0, 1, 1 ) );
		m_pCorner->Generate( tempContext, TRectangle<int32>( 0, region.height - 1, 1, 1 ) );
		m_pCorner->Generate( tempContext, TRectangle<int32>( region.width - 1, region.height - 1, 1, 1 ) );

		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				m_pWallBroken->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
			}
		}

		tempContext.Build();
	}

}

void CPipeNode::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_fBeginPointCountPercent = XmlGetAttr( pXml, "begin_percent", 0.2f );
	m_fBeginPointHeightPercent = XmlGetAttr( pXml, "begin_height_percent", 0.8f );
	m_fEndPointHeightPercent = XmlGetAttr( pXml, "end_height_percent", 0.2f );
	m_nBeginClipLen = XmlGetAttr( pXml, "begin_clip_len", 1 );
	m_nMinHLength = XmlGetAttr( pXml, "min_h_len", 1 );
	m_nMaxHLength = XmlGetAttr( pXml, "max_h_len", 8 );
	m_nMinVLength = XmlGetAttr( pXml, "min_v_len", 5 );
	m_nMaxVLength = XmlGetAttr( pXml, "max_v_len", 10 );
	m_fIntersectStopChance = XmlGetAttr( pXml, "intersect_stop", 0.5f );

	for( int i = 0; i < ELEM_COUNT( m_pPipes ); i++ )
	{
		char buf[32];
		sprintf( buf, "pipe%d", i );
		m_pPipes[i] = CreateNode( pXml->FirstChildElement( buf )->FirstChildElement(), context );
	}
	CLevelGenerateNode::Load( pXml, context );
}

void CPipeNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	uint32 nWidth = region.width;
	uint32 nHeight = region.height;
	m_gendata.resize( nWidth * nHeight );

	float fBeginPointCount = nWidth * m_fBeginPointCountPercent;
	uint32 nBeginPointCount = floor( fBeginPointCount );
	float r = fBeginPointCount - nBeginPointCount;
	if( SRand::Inst().Rand( 0.0f, 1.0f ) < r )
		nBeginPointCount++;

	uint32* beginPoints = (uint32*)alloca( nBeginPointCount * sizeof( uint32 ) );
	SRand::Inst().A( nBeginPointCount, nWidth, beginPoints );
	for( int i = 0; i < nBeginPointCount; i++ )
	{
		int32 x = beginPoints[i];
		int32 y = SRand::Inst().Rand( Min( (uint32)floor( m_fBeginPointHeightPercent * nHeight ), nHeight - 1 ), nHeight );
		for( int32 y1 = y; y1 >= y - m_nBeginClipLen && y1 > 0; y1-- )
		{
			if( context.GetBlock( region.x + x, region.y + y1, 1 ) );
			{
				y = y1;
				break;
			}
		}

		GenPipe( TVector2<int32>( x, y ) );
	}

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_gendata[i + j * nWidth] & 1 )
			{
				uint8 n0 = !( m_gendata[i + j * nWidth] & 2 );
				uint8 n1 = !( m_gendata[i + j * nWidth] & 4 );
				uint8 n2 = i < nWidth - 1 ? !( m_gendata[i + 1 + j * nWidth] & 2 ) : 1;
				uint8 n3 = j < nHeight - 1 ? !( m_gendata[i + ( j + 1 ) * nWidth] & 4 ) : 1;
				uint32 n = n0 + ( n1 << 1 ) + ( n2 << 2 ) + ( n3 << 3 );
				if( n < ELEM_COUNT( m_pPipes ) )
					m_pPipes[n]->Generate( context, TRectangle<int32>( i + region.x, j + region.y, 1, 1 ) );
			}
		}
	}

	m_gendata.clear();
	m_pContext = NULL;
}

void CPipeNode::GenPipe( TVector2<int32> beginPoint )
{
	TVector2<int32> p = beginPoint;
	uint32 nWidth = m_region.width;
	uint32 nHeight = m_region.height;
	float fEndHeight = SRand::Inst().Rand( 0.0f, m_fEndPointHeightPercent ) * nHeight;
	int32 nEndHeight = floor( fEndHeight );
	float rHeight = fEndHeight - nEndHeight;
	if( SRand::Inst().Rand( 0.0f, 1.0f ) < rHeight )
		nEndHeight++;

	uint8 nPrevDir = 0;	//0 = down, 1 = left, 2 = right
	uint32 nLen = SRand::Inst().Rand( m_nMinVLength, m_nMaxVLength - 1 );
	for( ;; )
	{
		bool bIntersect = m_gendata[p.x + p.y * nWidth] & 1;
		bool bCanMoveLeft = nPrevDir != 2 && p.x > 0;
		bool bCanMoveRight = nPrevDir != 1 && p.x < nWidth - 1;
		if( bIntersect )
		{
			if( SRand::Inst().Rand( 0.0f, 1.0f ) < m_fIntersectStopChance )
				break;
			bCanMoveLeft = bCanMoveLeft && ( m_gendata[p.x + p.y * nWidth] & 2 );
			bCanMoveRight = bCanMoveRight && ( m_gendata[( p.x + 1 ) + p.y * nWidth] & 2 );
		}
		else
		{
			m_gendata[p.x + p.y * nWidth] |= 1;
		}

		if( nPrevDir == 0 )
		{
			if( ( bCanMoveLeft || bCanMoveRight ) && !nLen )
			{
				if( bCanMoveLeft && bCanMoveRight )
					nPrevDir = SRand::Inst().Rand( 1, 3 );
				else if( bCanMoveLeft )
					nPrevDir = 1;
				else
					nPrevDir = 2;
				nLen = SRand::Inst().Rand( m_nMinHLength, m_nMaxHLength - 1 );
			}
			else if( nLen )
				nLen--;
		}
		else
		{
			if( nPrevDir == 1 && !bCanMoveLeft || nPrevDir == 2 && !bCanMoveRight || !nLen )
			{
				nPrevDir = 0;
				nLen = SRand::Inst().Rand( m_nMinVLength, m_nMaxVLength - 1 );
			}
			else if( nLen )
				nLen--;
		}

		switch( nPrevDir )
		{
		case 0:
			p.y--;
			break;
		case 1:
			p.x--;
			break;
		case 2:
			p.x++;
			break;
		}
		if( p.y < nEndHeight )
			break;
		switch( nPrevDir )
		{
		case 0:
			m_gendata[p.x + ( p.y + 1 ) * nWidth] |= 4;
			break;
		case 1:
			m_gendata[p.x + 1 + p.y * nWidth] |= 2;
			break;
		case 2:
			m_gendata[p.x + p.y * nWidth] |= 2;
			break;
		}
	}
}

void CSplitNode::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	auto pSplit = pXml->FirstChildElement( "split" );
	if( pSplit )
		m_pSplitNode = CreateNode( pSplit->FirstChildElement(), context );
	auto pSpace = pXml->FirstChildElement( "space" );
	if( pSpace )
		m_pSpaceNode = CreateNode( pSpace->FirstChildElement(), context );
	m_bVertical = XmlGetAttr( pXml, "vertical", 0 );
	m_nMinWidth = XmlGetAttr( pXml, "min_width", 1 );
	m_nMaxWidth = XmlGetAttr( pXml, "max_width", 2 );
	m_nSpaceWidth = XmlGetAttr( pXml, "space_width", 1 );
	CLevelGenerateNode::Load( pXml, context );
}

void CSplitNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	int32 nWidth = m_bVertical ? region.height : region.width;
	if( nWidth <= 0 )
		return;

	int32 nMaxCount = ( nWidth + m_nSpaceWidth ) / ( m_nMinWidth + m_nSpaceWidth );
	int32 nMinCount = Max<int32>( nWidth + m_nSpaceWidth - 1, 0 ) / ( m_nMaxWidth + m_nSpaceWidth ) + 1;
	nMinCount = Min( nMinCount, nMaxCount );
	int32 nCount = SRand::Inst().Rand( nMinCount, nMaxCount + 1 );
	if( nCount <= 1 )
	{
		m_pSplitNode->Generate( context, region );
		CLevelGenerateNode::Generate( context, region );
		return;
	}

	int32 a = nCount - 1;
	int32 b = nWidth - nCount * ( m_nMinWidth + m_nSpaceWidth ) + m_nSpaceWidth + a;
	assert( b >= a );
	int8* result = (int8*)alloca( b );
	SRand::Inst().C( a, b, result );

	int32 nCurPos = 0;
	int32 nPrePos = 0;
	int32 nSplit = 0;
	for( int i = 0; i < b; i++ )
	{
		if( result[i] )
		{
			nCurPos += m_nMinWidth + m_nSpaceWidth;
			nSplit++;

			TRectangle<int32> region1 = region;
			if( m_bVertical )
			{
				region1.y += nPrePos;
				region1.height = nCurPos - nPrePos - m_nSpaceWidth;
			}
			else
			{
				region1.x += nPrePos;
				region1.width = nCurPos - nPrePos - m_nSpaceWidth;
			}
			if( m_pSplitNode )
				m_pSplitNode->Generate( context, region1 );
			if( m_bVertical )
			{
				region1.y += region1.height;
				region1.height = m_nSpaceWidth;
			}
			else
			{
				region1.x += region1.width;
				region1.width = m_nSpaceWidth;
			}
			if( m_pSpaceNode )
				m_pSpaceNode->Generate( context, region1 );

			nPrePos = nCurPos;
		}
		else
			nCurPos++;
	}

	TRectangle<int32> region1 = region;
	if( m_bVertical )
	{
		region1.y += nPrePos;
		region1.height = nCurPos - nPrePos - m_nSpaceWidth;
	}
	else
	{
		region1.x += nPrePos;
		region1.width = nCurPos - nPrePos - m_nSpaceWidth;
	}
	if( m_pSplitNode )
		m_pSplitNode->Generate( context, region1 );

	CLevelGenerateNode::Generate( context, region );
}


void CHouseNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pBrokenNode = CreateNode( pXml->FirstChildElement( "broken" )->FirstChildElement(), context );
	m_pSubChunkNode = CreateNode( pXml->FirstChildElement( "subchunk" )->FirstChildElement(), context );
	m_pCarSpawnerNode[0] = CreateNode( pXml->FirstChildElement( "car_spawner_0" )->FirstChildElement(), context );
	m_pCarSpawnerNode[1] = CreateNode( pXml->FirstChildElement( "car_spawner_1" )->FirstChildElement(), context );
	m_pCarSpawnerNode[2] = CreateNode( pXml->FirstChildElement( "car_spawner_2" )->FirstChildElement(), context );
	m_pCarSpawnerNode[3] = CreateNode( pXml->FirstChildElement( "car_spawner_3" )->FirstChildElement(), context );
	m_pEntranceNode[0] = CreateNode( pXml->FirstChildElement( "entrance_0" )->FirstChildElement(), context );
	m_pEntranceNode[1] = CreateNode( pXml->FirstChildElement( "entrance_1" )->FirstChildElement(), context );
	m_pEntranceNode[2] = CreateNode( pXml->FirstChildElement( "entrance_2" )->FirstChildElement(), context );
	m_pEntranceNode[3] = CreateNode( pXml->FirstChildElement( "entrance_3" )->FirstChildElement(), context );
}

void CHouseNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 n1 = context.mapTags["1"];
		int8 n2 = context.mapTags["2"];
		int8 nExit1 = context.mapTags["exit1"];
		int8 nExit2 = context.mapTags["exit2"];
		SLevelBuildContext tempContext( context.pLevel, pChunk );

		for( int i = 0; i < region.width; i++ )
		{
			uint32 x = i + region.x;
			uint32 y = region.y;
			uint8 nType = context.blueprint[x + y * context.nWidth];
			if( nType == nExit1 )
			{
				pChunk->GetBlock( i, 0 )->nTag = 1;
				m_pEntranceNode[3]->Generate( context, TRectangle<int32>( i + region.x, 0 + region.y, 1, 1 ) );
			}
			else if( nType == nExit2 )
			{
				pChunk->GetBlock( i, 0 )->nTag = 1;
				pChunk->GetBlock( i, 1 )->nTag = 1;
				pChunk->GetBlock( i + 1, 0 )->nTag = 1;
				pChunk->GetBlock( i + 1, 1 )->nTag = 1;
				m_pCarSpawnerNode[3]->Generate( context, TRectangle<int32>( i + region.x, 0 + region.y, 2, 4 ) );
				i++;
			}
		}
		for( int i = 0; i < region.width; i++ )
		{
			uint32 x = i + region.x;
			uint32 y = region.y + region.height - 1;
			uint8 nType = context.blueprint[x + y * context.nWidth];
			if( nType == nExit1 )
			{
				pChunk->GetBlock( i, region.height - 1 )->nTag = 1;
				m_pEntranceNode[1]->Generate( context, TRectangle<int32>( i + region.x, region.height - 1 + region.y, 1, 1 ) );
			}
			else if( nType == nExit2 )
			{
				pChunk->GetBlock( i, region.height - 2 )->nTag = 1;
				pChunk->GetBlock( i, region.height - 1 )->nTag = 1;
				pChunk->GetBlock( i + 1, region.height - 2 )->nTag = 1;
				pChunk->GetBlock( i + 1, region.height - 1 )->nTag = 1;
				m_pCarSpawnerNode[1]->Generate( context, TRectangle<int32>( i + region.x, region.height - 4 + region.y, 2, 4 ) );
				i++;
			}
		}
		for( int j = 0; j < region.height; j++ )
		{
			uint32 x = region.x;
			uint32 y = j + region.y;
			uint8 nType = context.blueprint[x + y * context.nWidth];
			if( nType == nExit1 )
			{
				pChunk->GetBlock( 0, j )->nTag = 1;
				m_pEntranceNode[2]->Generate( context, TRectangle<int32>( 0 + region.x, j + region.y, 1, 1 ) );
			}
			else if( nType == nExit2 )
			{
				pChunk->GetBlock( 0, j )->nTag = 1;
				pChunk->GetBlock( 1, j )->nTag = 1;
				pChunk->GetBlock( 0, j + 1 )->nTag = 1;
				pChunk->GetBlock( 1, j + 1 )->nTag = 1;
				m_pCarSpawnerNode[2]->Generate( context, TRectangle<int32>( 0 + region.x, j + region.y, 4, 2 ) );
				j++;
			}
		}
		for( int j = 0; j < region.height; j++ )
		{
			uint32 x = region.x + region.width - 1;
			uint32 y = j + region.y;
			uint8 nType = context.blueprint[x + y * context.nWidth];
			if( nType == nExit1 )
			{
				pChunk->GetBlock( region.width - 1, j )->nTag = 1;
				m_pEntranceNode[0]->Generate( context, TRectangle<int32>( region.width - 1 + region.x, j + region.y, 1, 1 ) );
			}
			else if( nType == nExit2 )
			{
				pChunk->GetBlock( region.width - 2, j )->nTag = 1;
				pChunk->GetBlock( region.width - 1, j )->nTag = 1;
				pChunk->GetBlock( region.width - 2, j + 1 )->nTag = 1;
				pChunk->GetBlock( region.width - 1, j + 1 )->nTag = 1;
				m_pCarSpawnerNode[0]->Generate( context, TRectangle<int32>( region.width - 4 + region.x, j + region.y, 4, 2 ) );
				j++;
			}
		}

		for( int ix = 0; ix < 2; ix++ )
		{
			for( int iy = 0; iy < 2; iy++ )
			{
				TVector2<int32> p( ix * ( region.width - 1 ), iy * ( region.height - 1 ) );
				TVector2<int32> p1( 1 - ix * 2, 1 - iy * 2 );

				int32 t = context.blueprint[region.x + p.x + ( region.y + p.y ) * context.nWidth];
				if( t >= n1 && t < n1 + 4 && !pChunk->GetBlock( p.x, p.y )->nTag )
				{
					int32 w = 0;
					int32 h = 0;
					for( ; w < region.width; w++ )
					{
						if( context.blueprint[p.x + w * p1.x + region.x + ( p.y + region.y ) * context.nWidth] != t )
							break;
					}
					for( ; h < region.height; h++ )
					{
						if( context.blueprint[p.x + region.x + ( p.y + h * p1.y + region.y ) * context.nWidth] != t )
							break;
					}

					int32 x = p.x - ( w - 1 ) * ix;
					int32 y = p.y - ( h - 1 ) * iy;
					for( int i = 0; i < w; i++ )
					{
						for( int j = 0; j < h; j++ )
						{
							if( context.blueprint[i + x + region.x + ( j + y + region.y ) * context.nWidth] == n2 )
								pChunk->GetBlock( i + x, j + y )->nTag = 3 + ( i * 2 < w ? 0 : 2 ) + ( j * 2 < h ? 0 : 1 );
							else
								pChunk->GetBlock( i + x, j + y )->nTag = 2;
						}
					}
					m_pSubChunkNode->Generate( tempContext, TRectangle<int32>( x, y, w, h ) );
				}

			}
		}

		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				m_pBrokenNode->Generate( tempContext, TRectangle<int32>( i, j, 1, 1 ) );
			}
		}

		tempContext.Build();
	}
}

void CFenceNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_nType0 = XmlGetAttr( pXml, "type0", 0 );
	m_nType1 = XmlGetAttr( pXml, "type1", 2 );
	if( pXml->FirstChildElement( "fence" ) )
		m_pFenceNode = CreateNode( pXml->FirstChildElement( "fence" )->FirstChildElement(), context );
	if( pXml->FirstChildElement( "tile" ) )
		m_pTileNode = CreateNode( pXml->FirstChildElement( "tile" )->FirstChildElement(), context );
}

void CFenceNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	vector<int8> vecType;
	vecType.resize( region.width * region.height );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );
		if( m_pSubChunk )
		{
			SLevelBuildContext tempContext( context.pLevel, pChunk );
			m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
			tempContext.Build();
			if( m_bCopyBlueprint )
			{
				for( int i = 0; i < tempContext.nWidth; i++ )
				{
					for( int j = 0; j < tempContext.nHeight; j++ )
					{
						context.blueprint[i + region.x + ( j + region.y ) * context.nWidth]
							= tempContext.blueprint[i + j * tempContext.nWidth];
					}
				}
			}
		}

		int8 n1 = context.mapTags["1"];

		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				if( context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] == n1 )
				{
					pChunk->GetBlock( i, j )->eBlockType = m_nType1;
					if( m_pTileNode )
						m_pTileNode->Generate( context, TRectangle<int32>( i + region.x, j + region.y, 1, 1 ) );
					vecType[i + j * region.width] = 1;
				}
				else
					pChunk->GetBlock( i, j )->eBlockType = m_nType0;
			}
		}

		vector<TVector2<int32> > vec;
		FindAllOfTypesInMap( vecType, region.width, region.height, 1, vec );
		for( auto& p : vec )
		{
			if( vecType[p.x + p.y * region.width] != 1 )
				continue;
			auto rect = PutRect( vecType, region.width, region.height, p, TVector2<int32>( 1, 1 ), TVector2<int32>( region.width, region.height ),
				TRectangle<int32>( 0, 0, region.width, region.height ), -1, 0 );
			if( rect.width && rect.height && m_pFenceNode )
				m_pFenceNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
}