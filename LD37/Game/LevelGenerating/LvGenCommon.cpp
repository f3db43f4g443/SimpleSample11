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

void CRandomTileNode1::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	int i = 0;
	for( auto pNode = pXml->FirstChildElement(); pNode && i < 16; pNode = pNode->NextSiblingElement(), i++ )
	{
		m_pNodes[i] = CreateNode( pNode, context );
	}
	CLevelGenerateNode::Load( pXml, context );
}

void CRandomTileNode1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	int8 nMask = context.mapTags["mask"];
	vector<pair<uint8, uint8> > vecType;
	vecType.resize( ( region.width + 1 ) * ( region.height + 1 ) );
	for( int i = 0; i < vecType.size(); i++ )
	{
		vecType[i].first = SRand::Inst().Rand( 0, 2 );
		vecType[i].second = SRand::Inst().Rand( 0, 2 );
	}
	vector<TVector2<int32> > vec;
	for( int j = 0; j < region.height; j++ )
	{
		for( int i = 0; i < region.width; i++ )
		{
			int32 x = i + region.x;
			int32 y = j + region.y;
			if( context.blueprint[x + y * context.nWidth] == nMask )
			{
				uint8 x1 = vecType[i + j * ( region.width + 1 )].first;
				uint8 y1 = vecType[i + j * ( region.width + 1 )].second;
				uint8 x2 = vecType[( i + 1 ) + j * ( region.width + 1 )].first;
				uint8 y2 = vecType[i + ( j + 1 ) * ( region.width + 1 )].second;

				static uint32 indexX[] = { 3, 2, 0, 1 };
				static uint32 indexY[] = { 0, 1, 3, 2 };
				uint32 tX = indexX[x1 + x2 * 2];
				uint32 tY = indexY[y1 + y2 * 2];
				m_pNodes[tX + tY * 4]->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );

				vec.push_back( TVector2<int32>( x, y ) );
			}
		}
	}

	if( m_pNextLevel )
	{
		int32 n = Min<int32>( vec.size(), (int32)floor( vec.size() * m_fNextLevelChance + SRand::Inst().Rand( 0.0f, 1.0f ) ) );
		SRand::Inst().Shuffle( vec );
		for( int i = 0; i < n; i++ )
			m_pNextLevel->Generate( context, TRectangle<int32>( vec[i].x, vec[i].y, 1, 1 ) );
	}
}

void CBarFillNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pNode = CreateNode( pXml->FirstChildElement(), context );
	m_bVertical = XmlGetAttr( pXml, "vertical", 0 );
	m_strName = XmlGetAttr( pXml, "check_gen_data_name", "" );
	CLevelGenerateNode::Load( pXml, context );
}

void CBarFillNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	int8 nMask = context.mapTags[m_strName.c_str()];

	if( !m_bVertical )
	{
		for( int j = region.y; j < region.GetBottom(); j++ )
		{
			int32 nLen = 0;
			for( int i = region.x; i <= region.GetRight(); i++ )
			{
				if( i < region.GetRight() && context.blueprint[i + j * context.nWidth] == nMask )
					nLen++;
				else if( nLen )
				{
					m_pNode->Generate( context, TRectangle<int32>( i - nLen, j, nLen, 1 ) );
					nLen = 0;
				}
			}
		}
	}
	else
	{
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			int32 nLen = 0;
			for( int j = region.y; j <= region.GetBottom(); j++ )
			{
				if( j < region.GetBottom() && context.blueprint[i + j * context.nWidth] == nMask )
					nLen++;
				else if( nLen )
				{
					m_pNode->Generate( context, TRectangle<int32>( i, j - nLen, 1, nLen ) );
					nLen = 0;
				}
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
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, &tempContext );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		if( m_pSubChunk )
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
	m_pWallBroken = CreateNode( pXml->FirstChildElement( "wall_broken" )->FirstChildElement(), context );
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
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, &tempContext );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 nDoor = context.mapTags["door"];
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
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, &tempContext );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 nDoor = context.mapTags["door"];

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
	m_strMask = XmlGetAttr( pXml, "wall_mask", "" );
}

void CRoom2Node::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, &tempContext );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 nDoor = context.mapTags["door"];
		int8 nMask = -1;
		if( m_strMask.length() )
			nMask = context.mapTags[m_strMask];
		if( !!( m_bCopyBlueprint & 2 ) )
		{
			for( int i = 0; i < tempContext.nWidth; i++ )
			{
				for( int j = 0; j < tempContext.nHeight; j++ )
				{
					tempContext.blueprint[i + j * tempContext.nWidth]
						= context.blueprint[i + region.x + ( j + region.y ) * context.nWidth];
				}
			}
		}
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
				else if( context.blueprint[i + region.x + region.y * context.nWidth] == nMask )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pHBar )->Generate( tempContext, TRectangle<int32>( i - nMaxLen, 0, nMaxLen, 1 ) );
						nMaxLen = 0;
					}
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
				else if( context.blueprint[i + region.x + ( region.y + region.height - 1 ) * context.nWidth] == nMask )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pHBar )->Generate( tempContext, TRectangle<int32>( i - nMaxLen, region.height - 1, nMaxLen, 1 ) );
						nMaxLen = 0;
					}
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
				else if( context.blueprint[region.x + ( i + region.y ) * context.nWidth] == nMask )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pVBar )->Generate( tempContext, TRectangle<int32>( 0, i - nMaxLen, 1, nMaxLen ) );
						nMaxLen = 0;
					}
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
				else if( context.blueprint[region.x + region.width - 1 + ( i + region.y ) * context.nWidth] == nMask )
				{
					if( nMaxLen )
					{
						( nMaxLen == 1 ? m_pCorner : m_pVBar )->Generate( tempContext, TRectangle<int32>( region.width - 1, i - nMaxLen, 1, nMaxLen ) );
						nMaxLen = 0;
					}
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

		if( m_pSubChunk )
			m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
		tempContext.Build();
		if( !!( m_bCopyBlueprint & 1 ) )
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

}

void CBillboardNode::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );

	auto pNode = pXml->FirstChildElement( "nodes" );
	for( auto pItem = pNode->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
	{
		m_vecSubNodes.push_back( CreateNode( pItem, context ) );
	}
}

void CBillboardNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, &tempContext );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		vector<int8> vecTypes;
		vecTypes.resize( m_vecSubNodes.size() );
		for( int i = 0; i < vecTypes.size(); i++ )
		{
			char buf[32];
			sprintf( buf, "%d", i + 1 );
			vecTypes[i] = context.mapTags[buf];
		}

		for( int i = region.x; i < region.GetRight(); i++ )
		{
			for( int j = region.y; j < region.GetBottom(); j++ )
			{
				int8 nType = context.blueprint[i + j * context.nWidth];
				for( int k = 0; k < vecTypes.size(); k++ )
				{
					if( vecTypes[k] == nType )
					{
						m_vecSubNodes[k]->Generate( tempContext, TRectangle<int32>( i - region.x, j - region.y, 1, 1 ) );
						break;
					}
				}
			}
		}

		if( m_pSubChunk )
			m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
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
	if( pXml->FirstChildElement( "node1" ) )
		m_pNode1 = CreateNode( pXml->FirstChildElement( "node1" )->FirstChildElement(), context );
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
			if( context.GetBlock( region.x + i, region.y + j, 1 ) )
				continue;
			if( m_gendata[i + j * nWidth] & 1 )
			{
				uint8 n0 = !( m_gendata[i + j * nWidth] & 2 );
				uint8 n1 = !( m_gendata[i + j * nWidth] & 4 );
				uint8 n2 = i < nWidth - 1 ? !( m_gendata[i + 1 + j * nWidth] & 2 ) : 1;
				uint8 n3 = j < nHeight - 1 ? !( m_gendata[i + ( j + 1 ) * nWidth] & 4 ) : 1;
				uint32 n = n0 + ( n1 << 1 ) + ( n2 << 2 ) + ( n3 << 3 );
				if( n < ELEM_COUNT( m_pPipes ) )
				{
					m_pPipes[n]->Generate( context, TRectangle<int32>( i + region.x, j + region.y, 1, 1 ) );
					if( n != 7 && n != 11 && n != 14 && ( m_gendata[i + j * nWidth] & 8 ) )
						m_pNode1->Generate( context, TRectangle<int32>( i + region.x, j + region.y, 1, 1 ) );
				}
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
	int32 n1 = SRand::Inst().Rand( 0, 7 );
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
		if( !( m_gendata[p.x + p.y * nWidth] & 8 ) )
		{
			if( !n1 )
			{
				m_gendata[p.x + p.y * nWidth] |= 8;
				n1 = SRand::Inst().Rand( 6, 9 );
			}
			n1--;
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
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, &tempContext );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		int8 n1 = context.mapTags["1"];
		int8 n2 = context.mapTags["2"];
		int8 nExit1 = context.mapTags["exit1"];
		int8 nExit2 = context.mapTags["exit2"];

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
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, m_pSubChunk || m_pChunkBaseInfo->bPack ? &tempContext : NULL );
	vector<int8> vecType;
	vecType.resize( region.width * region.height );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );
		if( m_pSubChunk || m_pChunkBaseInfo->bPack )
		{
			if( m_pSubChunk )
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

void CFiberNode::Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context )
{
	m_nType = XmlGetAttr( pXml, "fiber_type", 0 );
	for( auto pChild = pXml->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
	{
		if( strcmp( pChild->Value(), "block_type" ) == 0 )
			m_vecBlockTypes.push_back( XmlGetAttr( pChild, "name", "" ) );
		else if( strcmp( pChild->Value(), "node" ) == 0 )
			m_pNode = CreateNode( pChild->FirstChildElement(), context );
	}
	CLevelGenerateNode::Load( pXml, context );
}

void CFiberNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	vector<int8> vecTypes;
	for( auto str : m_vecBlockTypes )
		vecTypes.push_back( context.mapTags[str] );
	for( int x = region.x; x < region.GetRight(); x++ )
	{
		int32 j;
		for( j = 0; j < region.height; j++ )
		{
			int32 y = m_nType == 1 ? j + region.y : region.GetBottom() - 1 - j;
			int8 nType = context.blueprint[x + y * context.nWidth];
			bool bBreak = false;
			for( int i = 0; i < vecTypes.size(); i++ )
			{
				if( vecTypes[i] == nType )
				{
					bBreak = true;
					break;
				}
			}
			if( bBreak )
				break;
		}

		int32 y = m_nType == 1 ? region.y : region.GetBottom() - j;
		m_pNode->Generate( context, TRectangle<int32>( x, y, 1, j ) );
	}
}

void CControlRoomNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	const char* szNames0[] = { "sub_0_1", "sub_0_2x", "sub_0_2y", "sub_0_4" };
	const char* szNames1[] = { "sub_1_l", "sub_1_l0", "sub_1_t", "sub_1_t0", "sub_1_r", "sub_1_r0", "sub_1_b", "sub_1_b0" };
	const char* szNames2[] = { "sub_2_l", "sub_2_l0", "sub_2_t", "sub_2_t0", "sub_2_r", "sub_2_r0", "sub_2_b", "sub_2_b0" };
	const char* szNames3[] = { "sub_3_l", "sub_3_t", "sub_3_r", "sub_3_b" };
	for( int i = 0; i < ELEM_COUNT( szNames0 ); i++ )
		m_pNode0[i] = CreateNode( pXml->FirstChildElement( szNames0[i] )->FirstChildElement(), context );
	for( int i = 0; i < ELEM_COUNT( szNames1 ); i++ )
		m_pNode1[i] = CreateNode( pXml->FirstChildElement( szNames1[i] )->FirstChildElement(), context );
	for( int i = 0; i < ELEM_COUNT( szNames2 ); i++ )
		m_pNode2[i] = CreateNode( pXml->FirstChildElement( szNames2[i] )->FirstChildElement(), context );
	for( int i = 0; i < ELEM_COUNT( szNames3 ); i++ )
		m_pNode3[i] = CreateNode( pXml->FirstChildElement( szNames3[i] )->FirstChildElement(), context );
}

void CControlRoomNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, &tempContext );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;

		if( !!( m_bCopyBlueprint & 2 ) )
		{
			for( int i = 0; i < tempContext.nWidth; i++ )
			{
				for( int j = 0; j < tempContext.nHeight; j++ )
				{
					tempContext.blueprint[i + j * tempContext.nWidth]
						= context.blueprint[i + region.x + ( j + region.y ) * context.nWidth];
				}
			}
		}

		bool b[4] = { true, true, true, true };
		const char* szTag[] = { "left", "top", "right", "bottom" };
		for( int i = 0; i < 4; i++ )
		{
			auto itr = context.mapTags.find( szTag[i] );
			if( itr != context.mapTags.end() )
				b[i] = itr->second;
		}
		int8 nTag1 = -1;
		auto itr = context.mapTags.find( "1" );
		if( itr != context.mapTags.end() )
			nTag1 = itr->second;
		int8 nTag2 = -1;
		itr = context.mapTags.find( "2" );
		if( itr != context.mapTags.end() )
			nTag2 = itr->second;
		int8 nTag3 = -1;
		itr = context.mapTags.find( "3" );
		if( itr != context.mapTags.end() )
			nTag3 = itr->second;
		int8 nTag4 = -1;
		itr = context.mapTags.find( "4" );
		if( itr != context.mapTags.end() )
			nTag4 = itr->second;
		int8 nTag5 = -1;
		itr = context.mapTags.find( "5" );
		if( itr != context.mapTags.end() )
			nTag5 = itr->second;
		int8 nTag6 = -1;
		itr = context.mapTags.find( "6" );
		if( itr != context.mapTags.end() )
			nTag6 = itr->second;
		vector<int8> vecTemp;
		vecTemp.resize( region.width * region.height );
		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				auto pBlock = pChunk->GetBlock( i, j );
				if( i == 0 && b[0] )
					pBlock->nTag = 1;
				if( j == 0 && b[1] )
					pBlock->nTag = 1;
				if( i == region.width - 1 && b[2] )
					pBlock->nTag = 1;
				if( j == region.height - 1 && b[3] )
					pBlock->nTag = 1;
				auto nTag = context.blueprint[i + region.x + ( j + region.y ) * context.nWidth];
				if( nTag == nTag1 )
				{
					pBlock->eBlockType = eBlockType_Wall;
					vecTemp[i + j * region.width] = 1;
				}
				else if( nTag == nTag2 )
				{
					pBlock->eBlockType = eBlockType_Wall;
					vecTemp[i + j * region.width] = 2;
				}
				else if( nTag == nTag3 )
				{
					pBlock->eBlockType = eBlockType_Wall;
					vecTemp[i + j * region.width] = 3;
				}
				else if( nTag == nTag4 || nTag == nTag6 )
				{
					if( pBlock->nTag == 1 )
					{
						pBlock->nTag = 2;
						if( nTag5 >= 0 )
							context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] = nTag5;
					}
					else
						pBlock->nTag = 4;
				}
				else if( nTag == nTag5 )
					pBlock->nTag = 3;
				else if( nTag == nTag6 )
				{
					if( pBlock->nTag == 1 )
					{
						pBlock->nTag = 2;
						if( nTag5 >= 0 )
							context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] = nTag5;
					}
					else
						pBlock->nTag = 4;
				}
			}
		}

		for( int k = 0; k < 2; k++ )
		{
			for( int x = 0; x < region.width - 2; x++ )
			{
				int32 y = k == 0 ? 0 : region.height - 1;
				if( x < region.width - 3 && !vecTemp[x + y * region.width] && vecTemp[x + 1 + y * region.width] == 1
					&& vecTemp[x + 2 + y * region.width] == 1 && !vecTemp[x + 3 + y * region.width] )
				{
					for( int j = 0; j < region.height; j++ )
					{
						y = k == 0 ? j : region.height - 1 - j;
						if( !vecTemp[x + y * region.width] && vecTemp[x + 1 + y * region.width] == 1
							&& vecTemp[x + 2 + y * region.width] == 1 && !vecTemp[x + 3 + y * region.width] )
						{
							m_pNode2[y == 0 && b[1] ? 3 : ( y == region.height - 1 && b[3] ? 7 : ( k == 0 ? 2 : 6 ) )]->Generate( tempContext, TRectangle<int32>( x + 1, y, 2, 1 ) );
							vecTemp[x + 1 + y * region.width] = vecTemp[x + 2 + y * region.width] = 0;
						}
						else
							break;
					}
				}
				else if( !vecTemp[x + y * region.width] && vecTemp[x + 1 + y * region.width] == 1
					&& !vecTemp[x + 2 + y * region.width] )
				{
					for( int j = 0; j < region.height; j++ )
					{
						y = k == 0 ? j : region.height - 1 - j;
						if( !vecTemp[x + y * region.width] && vecTemp[x + 1 + y * region.width] == 1
							&& !vecTemp[x + 2 + y * region.width] )
						{
							m_pNode1[y == 0 && b[1] ? 3 : ( y == region.height - 1 && b[3] ? 7 : ( k == 0 ? 2 : 6 ) )]->Generate( tempContext, TRectangle<int32>( x + 1, y, 1, 1 ) );
							vecTemp[x + 1 + y * region.width] = 0;
						}
						else
							break;
					}
				}
			}

			for( int y = 0; y < region.height - 2; y++ )
			{
				int32 x = k == 0 ? 0 : region.width - 1;
				if( y < region.height - 3 && !vecTemp[x + y * region.width] && vecTemp[x + ( y + 1 ) * region.width] == 1
					&& vecTemp[x + ( y + 2 ) * region.width] == 1 && !vecTemp[x + ( y + 3 ) * region.width] )
				{
					for( int i = 0; i < region.width; i++ )
					{
						x = k == 0 ? i : region.width - 1 - i;
						if( !vecTemp[x + y * region.width] && vecTemp[x + ( y + 1 ) * region.width] == 1
							&& vecTemp[x + ( y + 2 ) * region.width] == 1 && !vecTemp[x + ( y + 3 ) * region.width] )
						{
							m_pNode2[x == 0 && b[0] ? 1 : ( x == region.width - 1 && b[2] ? 5 : ( k == 0 ? 0 : 4 ) )]->Generate( tempContext, TRectangle<int32>( x, y + 1, 1, 2 ) );
							vecTemp[x + ( y + 1 ) * region.width] = vecTemp[x + ( y + 2 ) * region.width] = 0;
						}
						else
							break;
					}
				}
				else if( !vecTemp[x + y * region.width] && vecTemp[x + ( y + 1 ) * region.width] == 1
					&& !vecTemp[x + ( y + 2 ) * region.width] )
				{
					for( int i = 0; i < region.width; i++ )
					{
						x = k == 0 ? i : region.width - 1 - i;
						if( !vecTemp[x + y * region.width] && vecTemp[x + ( y + 1 ) * region.width] == 1
							&& !vecTemp[x + ( y + 2 ) * region.width] )
						{
							m_pNode1[x == 0 && b[0] ? 1 : ( x == region.width - 1 && b[2] ? 5 : ( k == 0 ? 0 : 4 ) )]->Generate( tempContext, TRectangle<int32>( x, y + 1, 1, 1 ) );
							vecTemp[x + ( y + 1 ) * region.width] = 0;
						}
						else
							break;
					}
				}
			}
		}
		for( int i = 0; i < region.width; i++ )
		{
			for( int j = 0; j < region.height; j++ )
			{
				if( vecTemp[i + j * region.width] == 2 )
				{
					auto rect = PutRect( vecTemp, region.width, region.height, TVector2<int32>( i, j ), TVector2<int32>( 1, 1 ), TVector2<int32>( 2, 2 ),
						TRectangle<int32>( 0, 0, region.width, region.height ), -1, 0 );
					if( rect.width > 0 )
					{
						if( rect.width == 2 && rect.height == 2 )
							m_pNode0[3]->Generate( tempContext, rect );
						else if( rect.width == 2 )
							m_pNode0[1]->Generate( tempContext, rect );
						else if( rect.height == 2 )
							m_pNode0[2]->Generate( tempContext, rect );
						else
							m_pNode0[0]->Generate( tempContext, rect );
					}
				}
			}
		}

		for( int k = 0; k < 2; k++ )
		{
			int32 y = k == 0 ? 0 : region.height - 1;
			for( int x = 1; x < region.width - 1; x++ )
			{
				if( vecTemp[x + y * region.width] == 3 )
				{
					auto rect = PutRect( vecTemp, region.width, region.height, TVector2<int32>( x, y ), TVector2<int32>( 2, 1 ), TVector2<int32>( region.width, 1 ),
						TRectangle<int32>( 0, 0, region.width, region.height ), -1, 0 );
					if( rect.width > 0 )
						( k == 0 ? m_pNode3[1] : m_pNode3[3] )->Generate( tempContext, rect );
				}
			}
		}
		for( int k = 0; k < 2; k++ )
		{
			int32 x = k == 0 ? 0 : region.width - 1;
			for( int y = 1; y < region.height - 1; y++ )
			{
				if( vecTemp[x + y * region.width] == 3 )
				{
					auto rect = PutRect( vecTemp, region.width, region.height, TVector2<int32>( x, y ), TVector2<int32>( 1, 2 ), TVector2<int32>( 1, region.height ),
						TRectangle<int32>( 0, 0, region.width, region.height ), -1, 0 );
					if( rect.width > 0 )
						( k == 0 ? m_pNode3[0] : m_pNode3[2] )->Generate( tempContext, rect );
				}
			}
		}

		if( m_pSubChunk )
			m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
		tempContext.Build();
		if( !!( m_bCopyBlueprint & 1 ) )
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
		CLevelGenerateNode::Generate( context, region );
	}
}
