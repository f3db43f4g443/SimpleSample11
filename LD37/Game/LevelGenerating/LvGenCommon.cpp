#include "stdafx.h"
#include "LvGenCommon.h"

void CBrickTileNode::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	m_pBrick = CreateNode( pXml->FirstChildElement( "brick" )->FirstChildElement(), context );
	m_pBrick1 = CreateNode( pXml->FirstChildElement( "brick1" )->FirstChildElement(), context );
	m_bVertical = XmlGetAttr( pXml, "vertical", 0 );
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
			for( int i = ( j & 1 ) - 1; i < region.width; i += 2 )
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
			for( int j = ( i & 1 ) - 1; j < region.height; j += 2 )
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
