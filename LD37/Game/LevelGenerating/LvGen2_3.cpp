#include "stdafx.h"
#include "LvGen2.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "LvGenLib.h"
#include <algorithm>


void CLv2WallNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_strMask = XmlGetAttr( pXml, "mask", "" );
	m_pNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pNode0 = CreateNode( pXml->FirstChildElement( "wall0" )->FirstChildElement(), context );
	m_pBar0 = CreateNode( pXml->FirstChildElement( "bar0" )->FirstChildElement(), context );
	CLevelGenerateNode::Load( pXml, context );
}

void CLv2WallNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	int8 nMask = -1;
	auto itr = context.mapTags.find( m_strMask );
	if( itr != context.mapTags.end() )
		nMask = itr->second;
	vector<int8> vecTemp;
	vecTemp.resize( region.width * region.height );
	for( int i = 0; i < region.width; i++ )
	{
		for( int j = 0; j < region.height; j++ )
		{
			vecTemp[i + j * region.width] = context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] == nMask ? 0 : 1;
		}
	}
	vector<int8> vecTemp1;
	vecTemp1.resize( region.height );
	int32 y0 = 0;

	function<void( const TRectangle<int32>& rect )> Func;
	Func = [=, &Func, &context, &region, &vecTemp, &vecTemp1] ( const TRectangle<int32>& rect ) {
		if( rect.width >= 6 )
		{
			int32 h = rect.height;
			int8 n1 = rect.y > 0 && rect.y <= 2 ? 0 : 1;
			int8 n2 = rect.GetBottom() < region.height && rect.GetBottom() >= region.height - 2 ? 0 : 1;
			h = Min( h - n1 - n2, SRand::Inst().Rand( 3, 5 ) );
			if( h >= 2 )
			{
				int32 y0 = SRand::Inst().Rand( rect.y + n1, rect.GetBottom() - h - n2 + 1 );
				int32 h1 = SRand::Inst().Rand( 0, 2 );
				if( rect.GetBottom() - y0 - h > 0 )
					Func( TRectangle<int32>( rect.x, y0 + h, rect.width, rect.GetBottom() - y0 - h ) );
				if( y0 - rect.y > 0 )
					Func( TRectangle<int32>( rect.x, rect.y, rect.width, y0 - rect.y ) );

				int8 nDir = SRand::Inst().Rand( 0, 2 );
				int8 b = rect.width >= SRand::Inst().Rand( 8, 12 );
				for( int i = 0; i < rect.width; )
				{
					int32 w = b ? SRand::Inst().Rand( 3, 5 ) : SRand::Inst().Rand( 1, 3 );
					w = Min( rect.width - i, w );
					if( b )
					{
						for( ; w > 0; w--, i++ )
						{
							int32 x = nDir ? rect.x + i : rect.GetRight() - 1 - i;
							for( int32 y = y0; y < y0 + h; y++ )
							{
								if( vecTemp[x + y * region.width] == 0 )
								{
									vecTemp[x + y * region.width] = 1;
									m_pNode0->Generate( context, TRectangle<int32>( x + region.x, y + region.y, 1, 1 ) );
								}
							}
						}
						for( ; i < rect.width; i++ )
						{
							int32 x = nDir ? rect.x + i : rect.GetRight() - 1 - i;
							bool b = false;
							for( int32 y = y0; y < y0 + h; y++ )
							{
								if( vecTemp[x + y * region.width] != 0 )
								{
									b = true;
									break;
								}
							}
							if( !b )
								break;
						}
					}
					else
					{
						Func( TRectangle<int32>( nDir ? rect.x + i : rect.GetRight() - i - w, y0, w, h ) );
						i += w;
					}
					b = !b;
				}
				return;
			}
		}
		if( rect.height >= 3 )
		{
			int32 h = SRand::Inst().Rand( 1, rect.height - 1 + 1 );
			Func( TRectangle<int32>( rect.x, rect.y, rect.width, h ) );
			Func( TRectangle<int32>( rect.x, rect.y + h, rect.width, rect.height - h ) );
			return;
		}

		int8 bDir = SRand::Inst().Rand( 0, 2 );
		{
			int32 y0 = SRand::Inst().Rand( rect.y, rect.GetBottom() );
			if( vecTemp1[y0] && rect.height > 1 )
				y0 += y0 == rect.y ? 1 : -1;
			if( !vecTemp1[y0] )
			{
				int32 i0 = 0;
				for( int i = 0; i <= rect.width; i++ )
				{
					int32 x = rect.x + ( bDir ? i : rect.width - i - 1 );
					bool b = false;
					if( i < rect.width )
					{
						b = true;
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							if( vecTemp[x + y * region.width] )
							{
								b = false;
								break;
							}
						}
					}
					if( !b )
					{
						if( i - i0 >= region.width * 3 / 4 )
						{
							TRectangle<int32> r( rect.x + ( bDir ? i0 : rect.width - i ), y0, i - i0, 1 );
							m_pBar0->Generate( context, r.Offset( TVector2<int32>( region.x, region.y ) ) );
							int32 y1 = Max( 0, y0 - 2 );
							int32 y2 = Min( region.height - 1, y0 + 2 );
							for( int y = y1; y <= y2; y++ )
								vecTemp1[y] = 1;
							for( int x = r.x; x < r.GetRight(); x++ )
							{
								vecTemp[x + r.y * region.width] = 1;
							}

							if( rect.height > 1 )
							{
								r.y += r.y == rect.y ? 1 : -1;
								Func( r );
							}
							break;
						}
						i0 = i + 1;
					}
				}
			}
		}

		for( int i = 0; i < rect.width; )
		{
			int32 w = Min( rect.width - i, ( SRand::Inst().Rand( 4, 9 ) + 1 ) / 3 );

			for( int i1 = 0; i1 < w; i1++ )
			{
				int32 x = rect.x + ( bDir ? i + i1 : rect.width - i - i1 - 1 );
				bool b = true;
				for( int y = rect.y; y < rect.GetBottom(); y++ )
				{
					if( vecTemp[x + y * region.width] )
					{
						b = false;
						break;
					}
				}
				if( !b )
				{
					w = i1;
					break;
				}
			}
			if( !w )
			{
				i++;
				continue;
			}

			TRectangle<int32> rect( rect.x + ( bDir ? i : rect.width - i - w ), rect.y, w, rect.height );
			m_pNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
			i += w;
		}
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( !vecTemp[i + j * region.width] )
					vecTemp[i + j * region.width] = 1;
			}
		}
	};

	for( int y = 0; y < region.height; y++ )
	{
		bool b = false;
		if( y < region.height - 1 )
		{
			b = true;
			for( int x = 0; x < region.width; x++ )
			{
				if( vecTemp[x + y * region.width] != vecTemp[x + ( y + 1 ) * region.width] )
				{
					b = false;
					break;
				}
			}
		}
		if( !b )
		{
			int32 h = y - y0 + 1;
			if( h >= 2 )
				Func( TRectangle<int32>( 0, y0, region.width, h ) );
			y0 = y + 1;
		}
	}
	vector<TVector2<int32> > vec;
	FindAllOfTypesInMap( vecTemp, region.width, region.height, 0, vec );
	SRand::Inst().Shuffle( vec );
	for( int k = 0; k < 2; k++ )
	{
		for( auto& p : vec )
		{
			if( vecTemp[p.x + p.y * region.width] )
				continue;
			auto rect = PutRect( vecTemp, region.width, region.height, p, k == 0 ? TVector2<int32>( 2, 2 ) : TVector2<int32>( 1, 1 ),
				TVector2<int32>( region.width, region.height ), TRectangle<int32>( 0, 0, region.width, region.height ), -1, 0 );
			if( rect.width > 0 )
				Func( rect );
		}
	}
}


void CLevelGenNode2_3_0::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pWall1Node = CreateNode( pXml->FirstChildElement( "wall1" )->FirstChildElement(), context );
	m_pBlockNode[0] = CreateNode( pXml->FirstChildElement( "block_0" )->FirstChildElement(), context );
	m_pBlockNode[1] = CreateNode( pXml->FirstChildElement( "block_1" )->FirstChildElement(), context );
	m_pBlockNode[2] = CreateNode( pXml->FirstChildElement( "block_2" )->FirstChildElement(), context );
	m_pBlockNode[3] = CreateNode( pXml->FirstChildElement( "block_3" )->FirstChildElement(), context );
	m_pChunkNode1[0] = CreateNode( pXml->FirstChildElement( "chunk_1_a" )->FirstChildElement(), context );
	m_pChunkNode1[1] = CreateNode( pXml->FirstChildElement( "chunk_1_b" )->FirstChildElement(), context );
	m_pChunkNode2[0] = CreateNode( pXml->FirstChildElement( "chunk_2_a" )->FirstChildElement(), context );
	m_pChunkNode2[1] = CreateNode( pXml->FirstChildElement( "chunk_2_b" )->FirstChildElement(), context );
	m_pChunkNode3[0] = CreateNode( pXml->FirstChildElement( "chunk_3_a" )->FirstChildElement(), context );
	m_pChunkNode3[1] = CreateNode( pXml->FirstChildElement( "chunk_3_b" )->FirstChildElement(), context );
	m_pChunkNode4[0] = CreateNode( pXml->FirstChildElement( "chunk_4_a" )->FirstChildElement(), context );
	m_pChunkNode4[1] = CreateNode( pXml->FirstChildElement( "chunk_4_b" )->FirstChildElement(), context );
	m_pChunkNode5 = CreateNode( pXml->FirstChildElement( "chunk_5" )->FirstChildElement(), context );
	m_pBuildingNode = CreateNode( pXml->FirstChildElement( "building" )->FirstChildElement(), context );

	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pBillboardNode = CreateNode( pXml->FirstChildElement( "billboard" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pCargo1Node = CreateNode( pXml->FirstChildElement( "cargo1" )->FirstChildElement(), context );
	m_pCargo2Node = CreateNode( pXml->FirstChildElement( "cargo2" )->FirstChildElement(), context );
	m_pBrokenNode = CreateNode( pXml->FirstChildElement( "broken" )->FirstChildElement(), context );
	m_pBoxNode = CreateNode( pXml->FirstChildElement( "box" )->FirstChildElement(), context );
	m_pBar0Node[0] = CreateNode( pXml->FirstChildElement( "bar0" )->FirstChildElement(), context );
	m_pBar0Node[1] = CreateNode( pXml->FirstChildElement( "bar0_r" )->FirstChildElement(), context );
	m_pBar0Node[2] = CreateNode( pXml->FirstChildElement( "bar0_l" )->FirstChildElement(), context );
	m_pBar0Node[3] = CreateNode( pXml->FirstChildElement( "bar0_2" )->FirstChildElement(), context );
	m_pBarNode[0] = CreateNode( pXml->FirstChildElement( "bar_h" )->FirstChildElement(), context );
	m_pBarNode[1] = CreateNode( pXml->FirstChildElement( "bar_v" )->FirstChildElement(), context );
	m_pBarNode_a[0] = CreateNode( pXml->FirstChildElement( "bar_h_a" )->FirstChildElement(), context );
	m_pBarNode_a[1] = CreateNode( pXml->FirstChildElement( "bar_v_a" )->FirstChildElement(), context );
	m_pControlRoomNode = CreateNode( pXml->FirstChildElement( "control_room" )->FirstChildElement(), context );

	m_pSawBladeNode[0] = CreateNode( pXml->FirstChildElement( "sawblade_0" )->FirstChildElement(), context );
	m_pSawBladeNode[1] = CreateNode( pXml->FirstChildElement( "sawblade_1" )->FirstChildElement(), context );
	m_pSawBladeNode[2] = CreateNode( pXml->FirstChildElement( "sawblade_2" )->FirstChildElement(), context );
	m_pSawBladeNode[3] = CreateNode( pXml->FirstChildElement( "sawblade_3" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_3_0::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );
	m_gendata1.resize( region.width * region.height );

	GenBase();

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
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}
	context.mapTags["1"] = eType_Wall1;
	for( auto& rect : m_vecWall1 )
	{
		m_pWall1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	for( int i = 0; i < 4; i++ )
	{
		for( auto& p : m_vecBlocks[i] )
		{
			m_pBlockNode[i]->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecChunk1[i] )
		{
			m_pChunkNode1[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
		for( auto& rect : m_vecChunk2[i] )
		{
			m_pChunkNode2[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
		for( auto& rect : m_vecChunk3[i] )
		{
			m_pChunkNode3[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
		for( auto& rect : m_vecChunk4[i] )
		{
			m_pChunkNode4[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( auto& rect : m_vecChunk5 )
	{
		m_pChunkNode5->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Building_1;
	context.mapTags["2"] = eType_Building_2;
	context.mapTags["3"] = eType_Building_3;
	for( auto& rect : m_vecBuildings )
	{
		m_pBuildingNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	for( int i = 0; i < 4; i++ )
	{
		for( auto& rect : m_vecBar0[i] )
		{
			m_pBar0Node[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecBar[i] )
		{
			m_pBarNode[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecBar_a[i] )
		{
			m_pBarNode_a[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}

	for( auto& p : m_vecBroken )
	{
		m_pBrokenNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& p : m_vecBox )
	{
		m_pBoxNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& rect : m_vecCargos )
	{
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Cargo1_1;
	context.mapTags["2"] = eType_Cargo1_2;
	context.mapTags["3"] = eType_Cargo1_3;
	context.mapTags["4"] = eType_Cargo1_4;
	context.mapTags["5"] = eType_Cargo1_5;
	context.mapTags["6"] = eType_Cargo1_6;
	for( auto& rect : m_vecCargos1 )
	{
		m_pCargo1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( auto& rect : m_vecCargos2 )
	{
		context.mapTags["1"] = eType_Cargo2;
		m_pWall1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata1[i + j * region.width] )
					m_gendata[i + j * region.width] = context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] = eType_Cargo2_1;
				else
					m_gendata[i + j * region.width] = context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] = eType_Cargo2;
			}
		}
		context.mapTags["1"] = eType_Cargo2_1;
		m_pCargo2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags["1"] = eType_Control_Room_1;
	context.mapTags["2"] = eType_Control_Room_2;
	context.mapTags["3"] = eType_Control_Room_3;
	context.mapTags["4"] = eType_Control_Room_4;
	context.mapTags["5"] = eType_Control_Room_5;
	for( auto& room : m_vecControlRooms )
	{
		auto rect = room.rect;
		context.mapTags["left"] = room.b[0];
		context.mapTags["top"] = room.b[1];
		context.mapTags["right"] = room.b[2];
		context.mapTags["bottom"] = room.b[3];
		m_pControlRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags["1"] = eType_Room_1;
	context.mapTags["2"] = eType_Room_2;
	context.mapTags["door"] = eType_Room_Door;
	for( auto& rect : m_vecRooms )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * context.nWidth] != eType_Room_2 && m_gendata[i + j * context.nWidth] != eType_Room_Door )
					m_gendata[i + j * context.nWidth] = context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] = eType_Room_1;
			}
		}
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["0_0"] = eType_Billboard_0_0;
	context.mapTags["0_1"] = eType_Billboard_0_1;
	context.mapTags["1"] = eType_Billboard_1;
	context.mapTags["2"] = eType_Billboard_2;
	for( auto& rect : m_vecBillboards )
	{
		m_pBillboardNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( int i = 0; i < 4; i++ )
	{
		for( auto& rect : m_vecSawBlade[i] )
		{
			m_pSawBladeNode[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}

	context.mapTags.clear();
	m_gendata.clear();
	m_gendata1.clear();

	m_vecWall1.clear();
	for( int i = 0; i < 4; i++ )
		m_vecBlocks[i].clear();
	for( int i = 0; i < 2; i++ )
	{
		m_vecChunk1[i].clear();
		m_vecChunk2[i].clear();
		m_vecChunk3[i].clear();
		m_vecChunk4[i].clear();
	}
	m_vecChunk5.clear();
	m_vecBuildings.clear();
	m_vecRooms.clear();
	m_vecBillboards.clear();
	m_vecCargos.clear();
	m_vecCargos1.clear();
	m_vecCargos2.clear();
	m_vecControlRooms.clear();
	m_vecBroken.clear();
	m_vecBox.clear();
	m_vecBar0[0].clear();
	m_vecBar0[1].clear();
	m_vecBar0[2].clear();
	m_vecBar0[3].clear();
	m_vecBar[0].clear();
	m_vecBar[1].clear();
	m_vecBar_a[0].clear();
	m_vecBar_a[1].clear();
	for( int i = 0; i < 4; i++ )
		m_vecSawBlade[i].clear();
}

void CLevelGenNode2_3_0::GenBase()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	struct SNode
	{
		SNode() { memset( nEdges, -1, sizeof( nEdges ) ); b1[0] = b1[1] = false; }
		TRectangle<int32> rect;
		int32 nEdges[4];
		bool b1[2];
	};
	struct SEdge
	{
		SEdge() { memset( nNodes, -1, sizeof( nNodes ) ); }
		TRectangle<int32> rect;
		int8 bVert;
		int32 nNodes[2];
	};
	vector<SNode> vecNodes;
	vector<SEdge> vecEdges;
	struct STemp
	{
		int32 nLastVert;
		int32 x;
	};
	vector<STemp> vec;
	int32 y0 = 0;
	vector<SNode> vecTempNodes;
	uint8 bDir;
	auto FuncSort = [=, &bDir] ( const SNode& a, const SNode& b ) {
		return bDir ? a.rect.x > b.rect.x : a.rect.x < b.rect.x;
	};
	while( y0 < nHeight )
	{
		int32 h1 = nHeight - y0;
		if( h1 >= 20 )
			h1 = SRand::Inst().Rand( 10, Min( h1 - 10, 14 ) + 1 );
		else if( h1 >= 14 )
			h1 = Max( 10, h1 - SRand::Inst().Rand( 6, 9 ) );
		int32 h2 = h1 >= 10 ? SRand::Inst().Rand( 4, ( h1 / 2 - 1 ) + 1 ) : 0;
		h1 -= h2;
		if( !vec.size() )
		{
			bDir = SRand::Inst().Rand( 0, 2 );
			int32 nCurX = 2;
			for( ; nCurX <= nWidth - 2; nCurX += SRand::Inst().Rand( 8, 16 ) )
			{
				STemp temp;
				temp.x = bDir ? nWidth - nCurX : nCurX;
				temp.nLastVert = -1;
				vec.push_back( temp );
			}
			SRand::Inst().Shuffle( vec );
		}
		for( int i = 0; i < vec.size(); i++ )
		{
			vecEdges.resize( vecEdges.size() + 1 );
			auto& edge = vecEdges.back();
			edge.bVert = true;
			edge.rect = TRectangle<int32>( vec[i].x - 2, y0, 4, h1 );
			edge.nNodes[0] = vec[i].nLastVert;
			if( vec[i].nLastVert >= 0 )
				vecNodes[vec[i].nLastVert].nEdges[1] = vecEdges.size() - 1;
			if( h2 > 0 )
			{
				vecTempNodes.resize( vecTempNodes.size() + 1 );
				auto& node = vecTempNodes.back();
				node.rect = TRectangle<int32>( edge.rect.x, y0 + h1, edge.rect.width, h2 );
				node.nEdges[3] = vecEdges.size() - 1;
			}
		}
		vec.resize( 0 );
		if( vecTempNodes.size() )
		{
			bDir = SRand::Inst().Rand( 0, 2 );
			sort( vecTempNodes.begin(), vecTempNodes.end(), FuncSort );
			if( bDir == 0 && vecTempNodes[0].rect.x >= nWidth / 2 || bDir == 1 && vecTempNodes[0].rect.GetRight() <= nWidth / 2 )
			{
				bDir = !bDir;
				for( int i = 0; i < vecTempNodes.size() / 2; i++ )
					swap( vecTempNodes[i], vecTempNodes[vecTempNodes.size() - 1 - i] );
			}

			int32 nNode0 = vecNodes.size();
			vecNodes.push_back( vecTempNodes[0] );
			vecEdges[vecTempNodes[0].nEdges[3]].nNodes[1] = vecNodes.size() - 1;
			int32 nCurX = bDir ? nWidth - vecTempNodes[0].rect.x : vecTempNodes[0].rect.GetRight();
			STemp temp;
			temp.x = vecTempNodes[0].rect.x + 2;
			temp.nLastVert = vecNodes.size() - 1;
			vec.push_back( temp );
			int32 l0 = SRand::Inst().Rand( 8, 16 );
			for( int i = 1;; )
			{
				int32 l = Max( 8, l0 + SRand::Inst().Rand( -1, 2 ) );
				l = Min( nWidth - nCurX, l );
				if( l < 8 )
					break;
				bool bUseNxt = false;
				if( i < vecTempNodes.size() )
				{
					for( ; i < vecTempNodes.size(); i++ )
					{
						int32 iNode = bDir ? vecTempNodes.size() - 1 - i : i;
						auto& nxt = vecTempNodes[i];
						auto d = ( bDir ? nWidth - nxt.rect.x : nxt.rect.GetRight() ) - nCurX;
						if( l <= d - 8 )
							break;
						else
						{
							vecEdges[nxt.nEdges[3]].nNodes[1] = vecNodes.size();
							if( l <= d + 8 )
							{
								bool b = true;
								if( l > d && i + 1 < vecTempNodes.size() )
								{
									auto& nxtnxt = vecTempNodes[i + 1];
									auto d1 = ( bDir ? nWidth - nxtnxt.rect.x : nxtnxt.rect.GetRight() ) - nCurX;
									if( l > d1 - 8 )
									{
										if( l >= d + 4 )
											b = false;
										else
											l = Min( l, d1 - 8 );
									}
								}
								if( b )
								{
									if( abs( l - d ) >= SRand::Inst().Rand( 2, 5 ) )
									{
										temp.x = bDir ? nWidth - ( nCurX + l ) + 2 : ( nCurX + l ) - 2;
										auto d1 = l - d;
										l = Max( l, d );
										if( bDir )
											d1 = -d1;
										if( d1 > 0 )
											nxt.rect.width += d1;
										else
											nxt.rect.SetLeft( nxt.rect.x + d1 );
									}
									else
									{
										l = d;
										temp.x = bDir ? nWidth - ( nCurX + l ) + 2 : ( nCurX + l ) - 2;
									}
									vecNodes.push_back( nxt );
									i++;
									bUseNxt = true;
									break;
								}
							}

							vecNodes.push_back( nxt );
						}
					}
				}
				nCurX += l;
				if( !bUseNxt )
				{
					if( nCurX >= nWidth - 4 )
						nCurX = nWidth;
					vecNodes.resize( vecNodes.size() + 1 );
					auto& node = vecNodes.back();
					node.rect = TRectangle<int32>( bDir ? nWidth - nCurX : nCurX - 4, y0 + h1, 4, h2 );
					temp.x = bDir ? nWidth - nCurX + 2 : nCurX - 2;
				}
				temp.nLastVert = vecNodes.size() - 1;
				vec.push_back( temp );
			}

			int32 y1 = ( h2 - 4 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			for( int i = nNode0; i <= vecNodes.size(); i++ )
			{
				int32 x1, x2;
				if( bDir )
				{
					x2 = i == nNode0 ? nWidth : vecNodes[i - 1].rect.x;
					x1 = i == vecNodes.size() ? 0 : vecNodes[i].rect.GetRight();
				}
				else
				{
					x1 = i == nNode0 ? 0 : vecNodes[i - 1].rect.GetRight();
					x2 = i == vecNodes.size() ? nWidth : vecNodes[i].rect.x;
				}
				if( x2 - x1 < 4 )
					continue;

				vecEdges.resize( vecEdges.size() + 1 );
				auto& edge = vecEdges.back();
				edge.bVert = false;
				edge.nNodes[0] = i == nNode0 ? -1 : i - 1;
				edge.nNodes[1] = i == vecNodes.size() ? -1 : i;
				if( bDir )
					swap( edge.nNodes[0], edge.nNodes[1] );
				edge.rect = TRectangle<int32>( x1, y0 + h1 + y1, x2 - x1, 4 );
				if( edge.nNodes[0] >= 0 )
					vecNodes[edge.nNodes[0]].nEdges[0] = vecEdges.size() - 1;
				if( edge.nNodes[1] >= 0 )
					vecNodes[edge.nNodes[1]].nEdges[2] = vecEdges.size() - 1;
			}
			SRand::Inst().Shuffle( vec );
			vecTempNodes.resize( 0 );
		}

		y0 += h1 + h2;
	}

	vector<int8> vecTemp1;
	vecTemp1.resize( nWidth * nHeight );
	for( auto& node : vecNodes )
	{
		for( int i = node.rect.x; i < node.rect.GetRight(); i++ )
		{
			for( int j = node.rect.y; j < node.rect.GetBottom(); j++ )
			{
				vecTemp1[i + j * nWidth] = 1;
			}
		}
	}
	for( auto& edge : vecEdges )
	{
		for( int i = edge.rect.x; i < edge.rect.GetRight(); i++ )
		{
			for( int j = edge.rect.y; j < edge.rect.GetBottom(); j++ )
			{
				vecTemp1[i + j * nWidth] = 1;
			}
		}
	}

	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	for( auto& edge : vecEdges )
	{
		if( edge.bVert )
		{
			for( int i = edge.rect.x; i < edge.rect.GetRight(); i++ )
			{
				for( int j = edge.rect.y; j < edge.rect.GetBottom(); j++ )
				{
					vecTemp[i + j * nWidth] = -1;
				}
			}
		}
	}
	for( int i = 0; i < vecNodes.size(); i++ )
	{
		auto& rect = vecNodes[i].rect;
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				vecTemp[x + y * nWidth] = i + 1;
			}
		}
	}

	auto Func1 = [=, &vecTemp] ( const TRectangle<int32>& rect, int32 k, int32 lMax0 = 5 )
	{
		int32 lMax = lMax0;
		int32 iMax = -1;
		int8 nTypeMax = -1;
		for( int i = 0; i < rect.width; i++ )
		{
			int8 nType = -2;
			int32 y = rect.y;
			int32 x0 = k == 0 ? rect.x + i : rect.GetRight() - 1 - i;
			if( vecTemp[x0 + ( y - 1 ) * nWidth] )
				break;
			int32 x = x0;
			for( ; y > 0; y-- )
			{
				auto n = vecTemp[x + ( y - 1 ) * nWidth];
				if( n && n != -2 )
					break;
				if( n )
					nType = -3;
				x += k == 0 ? -1 : 1;
				if( x < 0 || x >= nWidth || vecTemp[x + ( y - 1 ) * nWidth] != n )
				{
					y = rect.y;
					break;
				}
			}
			int32 l = rect.y - y;
			if( l >= lMax )
			{
				lMax = l;
				iMax = i;
				nTypeMax = nType;
			}
		}

		if( iMax >= 0 )
		{
			int32 y = rect.y;
			int32 x = k == 0 ? rect.x + iMax : rect.GetRight() - 1 - iMax;
			for( ; lMax > 0; y--, lMax-- )
			{
				vecTemp[x + ( y - 1 ) * nWidth] = nTypeMax;
				m_gendata1[x + ( y - 1 ) * nWidth] = 1 - k;
				x += k == 0 ? -1 : 1;
				vecTemp[x + ( y - 1 ) * nWidth] = nTypeMax;
				m_gendata1[x + ( y - 1 ) * nWidth] = 1 - k;
			}
			return true;
		}
		return false;
	};
	for( int k0 = 0; k0 < 2; k0++ )
	{
		for( int i = vecNodes.size() - 1; i >= 0; i-- )
		{
			auto& rect1 = vecNodes[i].rect;
			int32 w = k0 == 1 ? Max( 6, SRand::Inst().Rand( 11, 13 ) - rect1.height ) : 6;
			if( w > rect1.width )
			{
				for( int x = rect1.x; x < rect1.GetRight(); x++ )
				{
					for( int y = rect1.y; y < rect1.GetBottom(); y++ )
					{
						vecTemp[x + y * nWidth] = 0;
					}
				}
				auto rect = PutRect( vecTemp, nWidth, nHeight, rect1, rect1.GetSize(), TVector2<int32>( w, rect1.height ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0, 0 );
				int32 x1 = rect.x;
				int32 x2 = rect.GetRight();
				w = Min( x2 - x1, w );
				x1 = Max( x1, rect1.GetRight() - w );
				x2 = Min( x2, rect1.x + w );
				rect1.width = w;
				rect1.x = ( x1 + x2 - w + SRand::Inst().Rand( 0, 2 ) ) / 2;
				for( int x = rect1.x; x < rect1.GetRight(); x++ )
				{
					for( int y = rect1.y; y < rect1.GetBottom(); y++ )
					{
						vecTemp[x + y * nWidth] = i + 1;
					}
				}
			}

			if( k0 == 1 )
			{
				int32 k1 = SRand::Inst().Rand( 0, 2 );
				for( int k2 = 0; k2 < 2; k2++ )
				{
					int32 k = k1 ^ k2;
					vecNodes[i].b1[k] = Func1( rect1, k );
				}
			}
		}
	}

	for( int i = vecEdges.size() - 1; i >= 0; i-- )
	{
		auto& edge = vecEdges[i];
		if( edge.bVert == -1 )
			continue;
		if( !edge.bVert )
		{
			bool b = true;
			if( edge.nNodes[0] >= 0 )
				edge.rect.SetLeft( vecNodes[edge.nNodes[0]].rect.GetRight() );
			if( edge.nNodes[1] >= 0 )
				edge.rect.SetRight( vecNodes[edge.nNodes[1]].rect.x );
			if( edge.rect.width < 4 )
				continue;
			for( int i = edge.rect.x; i < edge.rect.GetRight() && b; i++ )
			{
				for( int j = edge.rect.y; j < edge.rect.GetBottom(); j++ )
				{
					if( vecTemp[i + j * nWidth] )
					{
						b = false;
						break;
					}
				}
			}
			if( !b )
			{
				edge.bVert = -1;
				continue;
			}

			int32 k1 = SRand::Inst().Rand( 0, 2 );
			for( int k2 = 0; k2 < 2; k2++ )
			{
				int32 k = k1 ^ k2;
				if( edge.nNodes[1 - k] >= 0 && vecNodes[edge.nNodes[1 - k]].b1[k] )
					continue;
				if( Func1( edge.rect, k ) )
					break;
			}
		}
		else
		{
			if( edge.nNodes[1] >= 0 )
			{
				auto rect1 = edge.rect;
				if( edge.nNodes[0] >= 0 || edge.nNodes[1] >= 0 )
				{
					if( edge.nNodes[0] >= 0 && edge.nNodes[1] >= 0 )
					{
						rect1.SetLeft( Min( rect1.x, Max( vecNodes[edge.nNodes[0]].rect.x, vecNodes[edge.nNodes[1]].rect.x ) ) );
						rect1.SetRight( Max( rect1.GetRight(), Min( vecNodes[edge.nNodes[0]].rect.GetRight(), vecNodes[edge.nNodes[1]].rect.GetRight() ) ) );
					}
					else if( edge.nNodes[0] >= 0 )
					{
						rect1.SetLeft( Min( rect1.x, vecNodes[edge.nNodes[0]].rect.x ) );
						rect1.SetRight( Max( rect1.GetRight(), vecNodes[edge.nNodes[0]].rect.GetRight() ) );
					}
					else
					{
						rect1.SetLeft( Min( rect1.x, vecNodes[edge.nNodes[1]].rect.x ) );
						rect1.SetRight( Max( rect1.GetRight(), vecNodes[edge.nNodes[1]].rect.GetRight() ) );
					}
					for( int x = edge.rect.x; x < edge.rect.GetRight(); x++ )
					{
						for( int y = edge.rect.y; y < edge.rect.GetBottom(); y++ )
							vecTemp[x + y * nWidth] = 0;
					}
					rect1 = PutRect( vecTemp, nWidth, nHeight, edge.rect, edge.rect.GetSize(), rect1.GetSize(), rect1, -1, 0, 0 );
					if( rect1.width >= 4 )
					{
						int32 w = Min( rect1.width, Min( Max( 4, ( rect1.height + 1 ) / 2 ), SRand::Inst().Rand( 5, 7 ) ) );
						edge.rect.x = SRand::Inst().Rand( Max( rect1.x, edge.rect.GetRight() - w ), Min( rect1.GetRight(), edge.rect.x + w ) - w + 1 );
						edge.rect.width = w;
					}
					for( int x = edge.rect.x; x < edge.rect.GetRight(); x++ )
					{
						for( int y = edge.rect.y; y < edge.rect.GetBottom(); y++ )
							vecTemp[x + y * nWidth] = -1;
					}
				}

				auto& node = vecNodes[edge.nNodes[1]];
				if( node.rect.width >= edge.rect.width * 5 / 2 && !node.b1[0] && !node.b1[1] )
				{
					for( int x = edge.rect.x; x < edge.rect.GetRight(); x++ )
					{
						for( int y = edge.rect.y; y < edge.rect.GetBottom(); y++ )
						{
							vecTemp[x + y * nWidth] = 0;
						}
					}
					auto rect = PutRect( vecTemp, nWidth, nHeight, edge.rect, edge.rect.GetSize(), TVector2<int32>( nWidth, edge.rect.height ),
						TRectangle<int32>( node.rect.x, 0, node.rect.width, nHeight ), -1, 0, 0 );
					int32 h = Max( edge.rect.x - rect.x, rect.GetRight() - edge.rect.GetRight() );
					bool bLeft = edge.rect.x - rect.x + SRand::Inst().Rand( 0, 2 ) > rect.GetRight() - edge.rect.GetRight();
					h = Min( h, rect.height - 4 );
					if( h < 2 )
					{
						h = 2;
						auto rect1 = edge.rect;
						rect1.height -= h;
						AddConn1( rect1 );
						int32 x = bLeft ? rect1.x : rect1.GetRight() - 1;
						rect1 = TRectangle<int32>( rect1.x, rect1.GetBottom(), rect1.width, h );
						rect1 = PutRect( vecTemp, nWidth, nHeight, rect1, rect1.GetSize(), TVector2<int32>( node.rect.width / 2, rect1.height ),
							TRectangle<int32>( node.rect.x, 0, node.rect.width, nHeight ), -1, 0, 0 );
						AddChunk( rect1, eType_Cargo1, &m_vecCargos1 );
					}
					else
					{
						h = SRand::Inst().Rand( Max( 2, h / 2 ), h + 1 );
						auto rect1 = edge.rect;
						rect1.height -= h;
						AddConn1( rect1 );
						int32 x = bLeft ? rect1.x : rect1.GetRight() - 1;
						rect1 = TRectangle<int32>( bLeft ? rect1.x + 1 : rect1.x, rect1.GetBottom(), rect1.width - 1, h );
						AddChunk( rect1, eType_Cargo1, &m_vecCargos1 );
						for( int y = rect1.y; y < rect1.GetBottom(); y++ )
						{
							int8 k = bLeft ? -1 : 1;
							vecTemp[x + y * nWidth] = vecTemp[x + k + y * nWidth] = -2;
							m_gendata1[x + y * nWidth] = m_gendata1[x + k + y * nWidth] = bLeft ? 0 : 1;
							x += k;
						}
					}
					continue;
				}
			}
		}
		if( edge.bVert )
			AddConn1( edge.rect );
		else
			AddConn2( edge.rect );
	}

	for( auto& node : vecNodes )
		AddChunk( node.rect, eType_Chunk, NULL );
	vector<TVector2<int32> > v;
	FindAllOfTypesInMap( vecTemp, nWidth, nHeight, 0, v );
	SRand::Inst().Shuffle( v );
	vector<TRectangle<int32> > vecTempWall1;
	for( auto& p : v )
	{
		if( vecTemp1[p.x + p.y * nWidth] )
			continue;
		auto r = PutRect( vecTemp1, nWidth, nHeight, p, TVector2<int32>( 6, 4 ), TVector2<int32>( nWidth, SRand::Inst().Rand( 6, 9 ) ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
		if( r.width > 0 )
		{
			int32 h = Min( Max( 4, r.height - 2 ), SRand::Inst().Rand( 4, 7 ) );
			r.y += ( r.height - h + SRand::Inst().Rand( 0, 2 ) ) / 2;
			r.height = h;
			vecTempWall1.push_back( r );
			bool b1 = false;
			for( int x = r.x; x < r.GetRight() && !b1; x++ )
			{
				for( int y = r.y; y < r.GetBottom(); y++ )
				{
					if( vecTemp[x + y * nWidth] == -3 )
					{
						b1 = true;
						break;
					}
				}
			}
			if( !b1 )
			{
				int32 y1 = ( r.y + r.GetBottom() + SRand::Inst().Rand( 0, 2 ) ) / 2;
				for( int x = r.x + 2; x <= r.GetRight() - 2; x++ )
				{
					if( vecTemp[x - 2 + y1 * nWidth] == 0 && vecTemp[x - 1 + y1 * nWidth] == -2
						&& vecTemp[x + y1 * nWidth] == -2 && vecTemp[x + 1 + y1 * nWidth] == 0 )
					{
						auto k = !m_gendata1[x + y1 * nWidth];
						vecTemp[x - 1 + y1 * nWidth] = vecTemp[x + y1 * nWidth] = -3;
						m_gendata1[x - 1 + y1 * nWidth] = m_gendata1[x + y1 * nWidth] = k;
						int32 x1 = x;
						for( int y = y1 + 1; y < r.GetBottom() - 1; y++ )
						{
							x1 += k ? 1 : -1;
							if( vecTemp[x1 - 1 + y * nWidth] || vecTemp[x1 + y * nWidth] )
								break;
							vecTemp[x1 - 1 + y * nWidth] = vecTemp[x1 + y * nWidth] = -3;
							m_gendata1[x1 - 1 + y * nWidth] = m_gendata1[x1 + y * nWidth] = k;
						}
						x1 = x;
						for( int y = y1 - 1; y > r.y; y-- )
						{
							x1 -= k ? 1 : -1;
							if( vecTemp[x1 - 1 + y * nWidth] || vecTemp[x1 + y * nWidth] )
								break;
							vecTemp[x1 - 1 + y * nWidth] = vecTemp[x1 + y * nWidth] = -3;
							m_gendata1[x1 - 1 + y * nWidth] = m_gendata1[x1 + y * nWidth] = k;
						}
					}
				}
			}
		}
	}

	for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 1; x < nWidth; x++ )
		{
			auto n = vecTemp[x + y * nWidth];
			if( n == -2 || n == -3 )
			{
				if( vecTemp[x - 1 + y * nWidth] == n )
				{
					AddChunk( TRectangle<int32>( x - 1, y, 2, 1 ), eType_Chunk, &( n == -2 ? m_vecChunk1 : m_vecChunk2 )[m_gendata1[x + y * nWidth]] );
					x++;
				}
			}
		}
	}

	for( auto& r : vecTempWall1 )
	{
		AddWall1( r );
		for( int x = r.x; x < r.GetRight(); x++ )
		{
			bool b = true;
			for( int y = r.y + 1; y < r.GetBottom() - 1; y++ )
			{
				if( m_gendata[x + y * nWidth] != eType_Wall1 )
				{
					b = false;
					break;
				}
			}
			if( !b )
			{
				for( int y = r.y + 1; y < r.GetBottom() - 1; y++ )
				{
					if( m_gendata[x + y * nWidth] == eType_Wall1 )
						m_gendata[x + y * nWidth] = 0;
				}
			}
		}
	}

	for( auto& node : vecNodes )
	{
		TVector4<int8> doors( 0, 0, 0, 0 );
		if( node.nEdges[0] >= 0 )
			doors.x = 2;
		if( node.nEdges[1] >= 0 )
			doors.y = 2;
		if( node.nEdges[2] >= 0 )
			doors.z = 2;
		if( node.nEdges[3] >= 0 )
			doors.w = 2;
		AddRooms( node.rect, doors, TVector2<int32>( 0, 0 ), 1, false );
	}

	/*int32 y0 = 0;
	for( int y = 0; y < nHeight - 16; )
	{
		int32 w = SRand::Inst().Rand( 0, 2 ) ? SRand::Inst().Rand( 9, 12 ) * 2 : nWidth;
		int32 h = Min( nHeight - y, SRand::Inst().Rand( 16, 33 ) ) & ~1;
		TRectangle<int32> rect( SRand::Inst().Rand( 0, 2 ) ? 0 : nWidth - w, y, w, h );
		GenArea( rect );
		if( rect.width < nWidth )
		{
			auto rect1 = rect.x == 0 ? TRectangle<int32>( rect.GetRight(), rect.y, nWidth - rect.GetRight(), rect.height ) :
				TRectangle<int32>( 0, rect.y, rect.x, rect.height );
			function<void( const TRectangle<int32>& rect )> Func;
			Func = [=, &Func] ( const TRectangle<int32>& rect )
			{
				if( rect.height >= 9 )
				{
					int32 h = SRand::Inst().Rand( 4, rect.height - 4 );
					Func( TRectangle<int32>( rect.x, rect.y, rect.width, h ) );
					Func( TRectangle<int32>( rect.x, rect.y + h + 1, rect.width, rect.height - h - 1 ) );
					return;
				}
				AddWall1( rect );
			};
			Func( rect1 );
		}
		if( y > y0 )
		{
			TRectangle<int32> rect1( rect.x, y0, rect.width, y - y0 );
			AddChunk( rect1, eType_Billboard, &m_vecBillboards );
		}
		y += h;
		y0 = y;
		y += Max( 0, Min( nHeight - 16 - y, SRand::Inst().Rand( 4, 9 ) ) );
	}*/
}

void CLevelGenNode2_3_0::GenArea( const TRectangle<int32>& rect )
{
	/*int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int8 nType = rect.width >= 32 || rect.height >= 26 ? 1 : 0;

	if( nType == 0 )
	{
		int32 w = Max( Max( 3, rect.width / 5 ), rect.width / 2 - SRand::Inst().Rand( 6, 8 ) ) * 2;
		int32 w1 = ( rect.width - w ) / 2;
		int32 h = Max( Max( 6, rect.height * 2 / 5 ), rect.height - SRand::Inst().Rand( 12, 15 ) );
		int32 h1 = ( rect.height - h + SRand::Inst().Rand( 0, 2 ) ) / 2;
		int32 h2 = rect.height - h - h1;
		AddRooms( TRectangle<int32>( rect.x, rect.y + h1, 4, h ), TVector4<int8>( 1, 1, 0, 1 ), TVector2<int32>( rect.width - 4, 0 ), 2, true, true );
		if( w <= 12 )
		{
			AddRooms( TRectangle<int32>( rect.x + w1, rect.y, w, 4 ), TVector4<int8>( 1, 1, 1, 1 ), TVector2<int32>( 0, 0 ), 1, false, true );
			AddRooms( TRectangle<int32>( rect.x + w1, rect.GetBottom() - 4, w, 4 ), TVector4<int8>( 1, 1, 1, 1 ), TVector2<int32>( 0, 0 ), 1, false, true );
		}
		else
		{
			AddRooms( TRectangle<int32>( rect.x + w1, rect.y, w / 2, 4 ), TVector4<int8>( 1, 1, 1, 1 ), TVector2<int32>( w / 2, 0 ), 2, true, true );
			AddRooms( TRectangle<int32>( rect.x + w1, rect.GetBottom() - 4, w / 2, 4 ), TVector4<int8>( 1, 1, 1, 1 ), TVector2<int32>( w / 2, 0 ), 2, true, true );
		}
		AddRooms( TRectangle<int32>( rect.x, rect.y, w1, h1 ), TVector4<int8>( 2, 2, 0, 1 ), TVector2<int32>( rect.width - w1, 0 ), 2, true, false );
		AddRooms( TRectangle<int32>( rect.x, rect.GetBottom() - h2, w1, h2 ), TVector4<int8>( 2, 1, 0, 2 ), TVector2<int32>( rect.width - w1, 0 ), 2, true, false );
		GenArea1( TRectangle<int32>( rect.x + 4, rect.y + h1, rect.width - 8, h ), TRectangle<int32>( rect.x + w1, rect.y + 4, w, rect.height - 8 ), TVector2<int32>( 0, 0 ), 1 );
	}
	else if( nType == 1 )
	{
		int32 nSplitX = ( rect.width - 6 ) / 12;
		int32 nSplitY = ( rect.height - 4 ) / 10;
		int32 w = Min( 4, rect.width / ( nSplitX * 4 + 2 ) ) * 2;
		int32 h = Min( 4, rect.height / ( nSplitY * 4 + 2 ) ) * 2;
		int32 w1 = ( rect.width - w * ( nSplitX + 1 ) ) / nSplitX;
		int32 h1 = ( rect.height - h * ( nSplitY + 1 ) ) / nSplitY;
		for( int j = 0; j <= nSplitY; j++ )
		{
			AddRooms( TRectangle<int32>( rect.x, rect.y + ( h + h1 ) * j, w, h ), TVector4<int8>( 1, j < nSplitY, 0, j > 0 ),
				TVector2<int32>( ( w + w1 ) * nSplitX, 0 ), 2, true, false );
			if( nSplitX > 1 )
			{
				AddRooms( TRectangle<int32>( rect.x + w + w1, rect.y + ( h + h1 ) * j, w, h ), TVector4<int8>( 1, j < nSplitX, 1, j > 0 ),
					TVector2<int32>( w + w1, 0 ), nSplitX - 1, false, false );
			}
			AddRooms( TRectangle<int32>( rect.x + w, rect.y + ( h + h1 ) * j + ( h - 4 ) / 2, w1, 4 ), TVector4<int8>( 1, 1, 1, 1 ),
				TVector2<int32>( w + w1, 0 ), nSplitX, true, true );
			if( j < nSplitY )
			{
				AddRooms( TRectangle<int32>( rect.x + ( w - 4 ) / 2, rect.y + h + ( h + h1 ) * j, 4, h1 ), TVector4<int8>( 1, 1, 1, 1 ),
					TVector2<int32>( w + w1, 0 ), nSplitX + 1, true, true );
			}
		}
		for( int j = 0; j < nSplitY; j++ )
		{
			GenArea1( TRectangle<int32>( rect.x + ( w - 4 ) / 2 + 4, rect.y + h + ( h + h1 ) * j, w + w1 - 4, h1 ),
				TRectangle<int32>( rect.x + w, rect.y + ( h + h1 ) * j + ( h - 4 ) / 2 + 4, w1, h + h1 - 4 ),
				TVector2<int32>( w + w1, 0 ), nSplitX );
		}
	}*/
}

void CLevelGenNode2_3_0::AddRooms( const TRectangle<int32>& r, TVector4<int8> doors, const TVector2<int32>& ofs, int32 nCount, bool bFlip )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int8 nBackType = eType_Room_2;
	TVector2<int32> nDoors[4];
	for( int k = 0; k < 2; k++ )
	{
		int8 nDoor = k == 0 ? doors.x : doors.z;
		int32& nBegin = nDoors[k == 0 ? 0 : 2].x;
		int32& nEnd = nDoors[k == 0 ? 0 : 2].y;
		if( nDoor == 0 )
			nBegin = nEnd = -1;
		else if( nDoor == 1 )
		{
			nBegin = ( r.height - 1 ) / 2;
			nEnd = r.height - ( r.height - 1 ) / 2;
		}
		else
		{
			nBegin = nEnd = -1;
			int32 x = k == 0 ? r.GetRight() : r.x - 1;
			if( x >= 0 && x < nWidth )
			{
				for( int y = r.y + 1; y < r.GetBottom() - 1; y++ )
				{
					if( m_gendata[x + y * nWidth] == eType_Room_2 || m_gendata[x + y * nWidth] == eType_Room_Door
						|| m_gendata[x + y * nWidth] >= eType_Billboard && m_gendata[x + y * nWidth] <= eType_Billboard_0_1 )
					{
						if( nBegin == -1 )
							nBegin = y - r.y;
						nEnd = y - r.y + 1;
					}
				}
			}
		}
	}
	for( int k = 0; k < 2; k++ )
	{
		int8 nDoor = k == 0 ? doors.y : doors.w;
		int32& nBegin = nDoors[k == 0 ? 1 : 3].x;
		int32& nEnd = nDoors[k == 0 ? 1 : 3].y;
		if( nDoor == 0 )
			nBegin = nEnd = -1;
		else if( nDoor == 1 )
		{
			nBegin = ( r.width - 1 ) / 2;
			nEnd = r.width - ( r.width - 1 ) / 2;
		}
		else
		{
			nBegin = nEnd = -1;
			int32 y = k == 0 ? r.GetBottom() : r.y - 1;
			if( y >= 0 && y < nHeight )
			{
				for( int x = r.x + 1; x < r.GetRight() - 1; x++ )
				{
					if( m_gendata[x + y * nWidth] == eType_Room_2 || m_gendata[x + y * nWidth] == eType_Room_Door
						|| m_gendata[x + y * nWidth] >= eType_Billboard && m_gendata[x + y * nWidth] <= eType_Billboard_0_1 )
					{
						if( nBegin == -1 )
							nBegin = x - r.x;
						nEnd = x - r.x + 1;
					}
				}
			}
		}
	}
	auto r0 = r;
	bool bFlipXY = false;
	if( r0.height + SRand::Inst().Rand( 0, 2 ) > r0.width )
	{
		swap( r0.width, r0.height );
		swap( nDoors[0], nDoors[1] );
		swap( nDoors[2], nDoors[3] );
		bFlipXY = true;
	}
	bool bFlipX = SRand::Inst().Rand( 0, 2 );
	if( bFlipX )
	{
		swap( nDoors[0], nDoors[2] );
		if( nDoors[1].x >= 0 )
			nDoors[1] = TVector2<int32>( r0.width - nDoors[1].y, r0.width - nDoors[1].x );
		if( nDoors[3].x >= 0 )
			nDoors[3] = TVector2<int32>( r0.width - nDoors[3].y, r0.width - nDoors[3].x );
	}
	bool bFlipY = SRand::Inst().Rand( 0, 2 );
	if( bFlipY )
	{
		swap( nDoors[1], nDoors[3] );
		if( nDoors[0].x >= 0 )
			nDoors[0] = TVector2<int32>( r0.height - nDoors[0].y, r0.height - nDoors[0].x );
		if( nDoors[2].x >= 0 )
			nDoors[2] = TVector2<int32>( r0.height - nDoors[2].y, r0.height - nDoors[2].x );
	}
	vector<int8> vecTemp;
	vecTemp.resize( r0.width * r0.height );
	auto Func = [=, &r, &ofs, &vecTemp] ( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				vecTemp[i - r0.x + ( j - r0.y ) * r0.width] = nType;
			}
		}
		TRectangle<int32> r1 = rect.Offset( TVector2<int32>( -r.x, -r.y ) );
		if( bFlipX )
			r1.x = r0.width - r1.x - r1.width;
		if( bFlipY )
			r1.y = r0.height - r1.y - r1.height;
		if( bFlipXY )
		{
			swap( r1.x, r1.y );
			swap( r1.width, r1.height );
		}
		r1 = r1.Offset( TVector2<int32>( r.x, r.y ) );
		auto r2 = r1;
		r2.x = r.x + r.GetRight() - r2.x - r2.width;
		for( int i = 0; i < nCount; i++ )
		{
			auto r3 = bFlip && !!( i & 1 ) ? r2 : r1;
			AddChunk( r3.Offset( ofs * i ), nType, pVec );
		}
	};
	Func( r0, nBackType, &m_vecRooms );
	TRectangle<int32> doorRects[4] = {
		{ r0.GetRight() - 1, r0.y + nDoors[0].x, 1, nDoors[0].y - nDoors[0].x },
		{ r0.x + nDoors[1].x, r0.GetBottom() - 1, nDoors[1].y - nDoors[1].x, 1 },
		{ r0.x, r0.y + nDoors[2].x, 1, nDoors[2].y - nDoors[2].x },
		{ r0.x + nDoors[3].x, r0.y, nDoors[3].y - nDoors[3].x, 1 },
	};
	for( int i = 0; i < 4; i++ )
	{
		if( doorRects[i].width && doorRects[i].height )
			Func( doorRects[i], eType_Room_Door, NULL );
	}

	if( r0.width >= r0.height + SRand::Inst().Rand( 2, 4 ) )
	{
		int32 j = SRand::Inst().Rand( 0, 2 ) * ( r0.height - 1 );
		for( int i = 0; i < r0.width; i += r0.width - 1 )
		{
			for( int j = 0; j < r0.height; j += r0.height - 1 )
			{
				if( vecTemp[i + j * r0.width] != nBackType )
					continue;
				auto rect = PutRect( vecTemp, r0.width, r0.height, TVector2<int32>( i, j ), TVector2<int32>( 1, 2 ), TVector2<int32>( r0.width, r0.height ),
					TRectangle<int32>( 0, 0, r0.width, r0.height ), -1, nBackType );
				if( rect.width <= 0 )
					continue;
				if( rect.width >= 2 )
				{
					int32 w = SRand::Inst().Rand( 2, Min( rect.width, Max( 6 - rect.height, 2 ) ) + 1 );
					if( rect.x == 0 )
						rect.width = w;
					else
						rect.SetLeft( rect.GetRight() - w );
					Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Cargo1, &m_vecCargos1 );
					auto rect1 = TRectangle<int32>( rect.x - 1, rect.y - 1, rect.width + 2, rect.height + 2 ) * TRectangle<int32>( 0, 0, r0.width, r0.height );
					for( int x = rect1.x; x < rect1.GetRight(); x++ )
					{
						for( int y = rect1.y; y < rect1.GetBottom(); y++ )
						{
							if( vecTemp[x + y * r0.width] == nBackType )
								vecTemp[x + y * r0.width] = eType_Temp0;
						}
					}
				}
				else
				{
					Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Room_1, &m_vecBar_a[bFlipXY ? 0 : 1] );
				}
			}
		}
		for( int32 i = 1; i < r0.width - 1; i++ )
		{
			if( vecTemp[i + j * r0.width] != nBackType )
				continue;
			auto rect = PutRect( vecTemp, r0.width, r0.height, TVector2<int32>( i, j ), TVector2<int32>( 2, 2 ), TVector2<int32>( r0.width, r0.height ),
				TRectangle<int32>( i, 0, r0.width - i, r0.height ), -1, nBackType );
			if( rect.width <= 0 )
				continue;
			if( rect.x > 1 && rect.x <= r0.width - 4 && SRand::Inst().Rand( 0, 2 ) )
			{
				rect.width = 2;
				int32 h = Max( 2, Min( rect.height, r0.height - 1 ) );
				if( rect.y > 0 )
					rect.SetTop( rect.GetBottom() - h );
				else
					rect.height = h;
			}
			else
			{
				if( rect.y > 0 )
					rect.SetTop( rect.GetBottom() - 2 );
				else
					rect.height = 2;
				if( rect.width >= 4 )
				{
					int32 w = Min( rect.width, SRand::Inst().Rand( 2, 6 ) );
					if( w == rect.width - 1 )
						w = rect.width - 2;
					rect.width = w;
				}
			}
			Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Cargo1, &m_vecCargos1 );
			auto rect1 = TRectangle<int32>( rect.x - 1, rect.y - 1, rect.width + 2, rect.height + 2 ) * TRectangle<int32>( 0, 0, r0.width, r0.height );
			for( int x = rect1.x; x < rect1.GetRight(); x++ )
			{
				for( int y = rect1.y; y < rect1.GetBottom(); y++ )
				{
					if( vecTemp[x + y * r0.width] == nBackType )
						vecTemp[x + y * r0.width] = eType_Temp0;
				}
			}

			i = rect.GetRight();
			j = r0.height - 1 - j;
		}
		for( int i = 1; i < r0.width - 1; i++ )
		{
			for( int j = 1; j < r0.height - 1; j++ )
			{
				if( vecTemp[i + j * r0.width] == nBackType )
					vecTemp[i + j * r0.width] = eType_Temp0;
			}
		}
		ConnectAll( vecTemp, r0.width, r0.height, eType_Room_Door, eType_Temp0 );
		for( int i = 0; i < r0.width; i++ )
		{
			for( int j = 0; j < r0.height; j++ )
			{
				if( vecTemp[i + j * r0.width] == eType_Temp0 )
					vecTemp[i + j * r0.width] = nBackType;
			}
		}
		for( int j = 0; j < r0.height; j += r0.height - 1 )
		{
			int32 i0 = 0;
			for( int i = 0; i <= r0.width; i++ )
			{
				if( i == r0.width || vecTemp[i + j * r0.width] != nBackType )
				{
					if( i >= i0 + 1 )
					{
						Func( TRectangle<int32>( i0 + r0.x, j + r0.y, i - i0, 1 ), eType_Room_1, &m_vecBar_a[bFlipXY ? 1 : 0] );
					}
					i0 = i + 1;
				}
			}
			i0 = 0;
			for( int i = 0; i <= r0.width; i++ )
			{
				if( i == r0.width || vecTemp[i + j * r0.width] != nBackType && vecTemp[i + j * r0.width] != eType_Room_Door )
				{
					if( i >= i0 )
						Func( TRectangle<int32>( i0 + r0.x, j + r0.y, i - i0, 1 ), i - i0 <= 2 ? eType_Room_Door : nBackType, NULL );
					i0 = i + 1;
				}
			}
		}
	}
	else
	{
		TRectangle<int32> rectCenter;
		for( int k = 0; k < 2; k++ )
		{
			int32& x = k == 0 ? rectCenter.y : rectCenter.x;
			int32& w = k == 0 ? rectCenter.height : rectCenter.width;
			if( nDoors[0 + k].x > 0 && nDoors[2 + k].x > 0 )
			{
				x = Min( nDoors[0 + k].x, nDoors[2 + k].x );
				w = Max( nDoors[0 + k].y, nDoors[2 + k].y ) - x;
			}
			else if( nDoors[0 + k].x > 0 )
			{
				x = nDoors[0 + k].x;
				w = nDoors[0 + k].y - x;
			}
			else if( nDoors[2 + k].x > 0 )
			{
				x = nDoors[2 + k].x;
				w = nDoors[2 + k].y - x;
			}
			else
			{
				int32 w1 = k == 0 ? r0.height : r0.width;
				x = ( w1 - 1 ) / 2;
				w = w1 - ( w1 - 1 ) / 2 * 2;
			}
		}
		for( int i = rectCenter.x; i < rectCenter.GetRight(); i++ )
		{
			for( int j = rectCenter.y; j < rectCenter.GetBottom(); j++ )
			{
				vecTemp[i + j * r0.width] = eType_Temp0;
			}
		}
		for( int i = 0; i < 4; i++ )
		{
			if( doorRects[i].width && doorRects[i].height )
			{
				auto rect1 = doorRects[i].Offset( TVector2<int32>( -r0.x, -r0.y ) );
				rect1.SetRight( Max( rect1.GetRight(), rectCenter.x ) );
				rect1.SetLeft( Min( rect1.x, rectCenter.GetRight() ) );
				rect1.SetBottom( Max( rect1.GetBottom(), rectCenter.y ) );
				rect1.SetTop( Min( rect1.y, rectCenter.GetBottom() ) );
				for( int i = rect1.x; i < rect1.GetRight(); i++ )
				{
					for( int j = rect1.y; j < rect1.GetBottom(); j++ )
					{
						if( vecTemp[i + j * r0.width] == nBackType )
							vecTemp[i + j * r0.width] = eType_Temp0;
					}
				}
			}
		}
		ConnectAll( vecTemp, r0.width, r0.height, eType_Room_Door, eType_Temp0 );
		for( int i = 0; i < r0.width; i++ )
		{
			for( int j = 0; j < r0.height; j++ )
			{
				if( vecTemp[i + j * r0.width] == eType_Temp0 )
					vecTemp[i + j * r0.width] = nBackType;
			}
		}

		for( int x = 0; x < r0.width; x += r0.width - 1 )
		{
			for( int y = 1; y < r0.height - 1; y++ )
			{
				if( vecTemp[x + y * r0.width] != nBackType )
					continue;
				auto rect = PutRect( vecTemp, r0.width, r0.height, TVector2<int32>( x, y ), TVector2<int32>( 2, 2 ),
					r0.GetSize(), TRectangle<int32>( 0, 0, r0.width, r0.height ), -1, nBackType );
				if( rect.width > 0 )
					Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Cargo1, &m_vecCargos1 );
			}
		}
		for( int y = 0; y < r0.height; y += r0.height - 1 )
		{
			for( int x = 1; x < r0.width - 1; x++ )
			{
				if( vecTemp[x + y * r0.width] != nBackType )
					continue;
				auto rect = PutRect( vecTemp, r0.width, r0.height, TVector2<int32>( x, y ), TVector2<int32>( 2, 2 ),
					r0.GetSize(), TRectangle<int32>( 0, 0, r0.width, r0.height ), -1, nBackType );
				if( rect.width > 0 )
					Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Cargo1, &m_vecCargos1 );
			}
		}
		for( int i = 0; i < 2; i++ )
		{
			for( int j = 0; j < 2; j++ )
			{
				TVector2<int32> p( i * ( r0.width - 1 ), j * ( r0.height - 1 ) );
				if( vecTemp[p.x + p.y * r0.width] != nBackType )
					continue;
				auto rect = PutRect( vecTemp, r0.width, r0.height, p, TVector2<int32>( 1, 1 ), r0.GetSize(), TRectangle<int32>( 0, 0, r0.width, r0.height ), -1, eType_Room_2 );
				if( rect.height == 1 )
					Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Room_1, &m_vecBar_a[bFlipXY ? 1 : 0] );
				else if( rect.width == 1 )
					Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Room_1, &m_vecBar_a[bFlipXY ? 0 : 1] );
				else
					Func( rect.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Cargo1, &m_vecCargos1 );
			}
		}
		for( int i = 0; i < 4; i++ )
		{
			if( doorRects[i].width && doorRects[i].height )
			{
				Func( doorRects[i], nBackType, NULL );
				auto rect1 = doorRects[i].Offset( TVector2<int32>( -r0.x, -r0.y ) );
				rect1 = PutRect( vecTemp, r0.width, r0.height, rect1, rect1.GetSize(), !!( i & 1 ) ? TVector2<int32>( r0.width, 1 ) : TVector2<int32>( 1, r0.height ),
					TRectangle<int32>( 0, 0, r0.width, r0.height ), -1, nBackType, nBackType );
				if( rect1.width <= 2 && rect1.height <= 2 )
					Func( rect1.Offset( TVector2<int32>( r0.x, r0.y ) ), eType_Room_Door, NULL );
			}
		}
	}
}

void CLevelGenNode2_3_0::AddConn1( const TRectangle<int32>& r )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	AddChunk( r, eType_Billboard, &m_vecBillboards );
	bool b = r.x <= 4 || r.GetRight() >= nWidth - 4;
	if( b )
	{
		int8 nType = SRand::Inst().Rand( 0, 2 );
		if( nType == 1 && ( r.width - 1 ) * 2 > r.height )
			nType = 0;
		if( nType == 0 )
		{
			int8 b0 = SRand::Inst().Rand( 0, 2 );
			int8 nDir = r.x + r.GetRight() > nWidth;
			int8 k = b0 ? SRand::Inst().Rand( 1, 3 ) : SRand::Inst().Rand( 2, Max( 2, Min( 4, r.height - 5 ) ) + 1 );
			for( int y = r.y; y < r.GetBottom(); y++, k-- )
			{
				if( !k )
				{
					if( y <= r.GetBottom() - 5 )
					{
						AddChunk( TRectangle<int32>( nDir ? r.GetRight() - 2 : r.x, y, 2, 2 ), eType_Chunk, &m_vecChunk5 );
						for( int y1 = y; y1 < y + 2; y1++ )
						{
							for( int i1 = 0; i1 < y1 - y + 1; i1++ )
							AddChunk( TRectangle<int32>( nDir ? r.GetRight() - 3 - i1 : r.x + 2 + i1, y1, 1, 1 ), eType_Chunk, &m_vecChunk4[( y1 + i1 - r.y + nDir + 1 ) % 2] );
						}
						y += 2;
						AddChunk( TRectangle<int32>( r.x, y, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );
						if( b0 )
						{
							k = 1;
							continue;
						}
						else
						{
							y++;
							k = SRand::Inst().Rand( 2, 5 );
						}
					}
				}
				AddChunk( TRectangle<int32>( nDir ? r.GetRight() - 2 : r.x, y, 2, 1 ), eType_Chunk, &m_vecChunk3[( y - r.y + nDir ) % 2] );
				if( r.width > 4 )
					AddChunk( TRectangle<int32>( nDir ? r.GetRight() - 3 : r.x + 2, y, 1, 1 ), eType_Chunk, &m_vecChunk4[( y - r.y + nDir + 1 ) % 2] );
			}
		}
		else
		{
			int32 h0 = ( r.width - 1 ) * 2;
			h0 += SRand::Inst().Rand( 0, ( r.height - h0 ) / 2 + 1 ) * 2;
			int32 y0 = SRand::Inst().Rand( 0, r.height - h0 + 1 ) + r.y;
			int32 y1 = y0 + h0 - r.width + 1;
			int8 b = r.x + r.GetRight() < nWidth;
			for( int y = r.y; y < r.GetBottom(); y++ )
			{
				TRectangle<int32> rect;
				bool b1;
				if( y < y0 )
				{
					rect = TRectangle<int32>( b ? r.x : r.GetRight() - 2, y, 2, 1 );
					b1 = ( b + y0 - y ) % 2;
				}
				else if( y < y0 + r.width - 1 )
				{
					rect = TRectangle<int32>( b ? r.x + y - y0 : r.GetRight() - 2 - y + y0, y, 2, 1 );
					b1 = b;
				}
				else if( y < y1 )
				{
					rect = TRectangle<int32>( b ? r.GetRight() - 2 : r.x, y, 2, 1 );
					b1 = ( b + y - ( y0 + r.width - 2 ) ) % 2;
				}
				else if( y < y1 + r.width - 1 )
				{
					rect = TRectangle<int32>( b ? r.GetRight() - 2 - y + y1 : r.x + y - y1, y, 2, 1 );
					b1 = !b;
				}
				else
				{
					rect = TRectangle<int32>( b ? r.x : r.GetRight() - 2, y, 2, 1 );
					b1 = ( b + 1 + y - ( y1 + r.width - 2 ) ) % 2;
				}
				AddChunk( rect, eType_Chunk, &m_vecChunk3[b1] );
				if( y > y0 && y < y1 + r.width - 2 )
					AddChunk( TRectangle<int32>( b ? rect.x - 1 : rect.GetRight(), y, 1, 1 ), eType_Chunk, &m_vecChunk4[!b1] );
				else if( r.width > 4 )
					AddChunk( TRectangle<int32>( !b ? rect.x - 1 : rect.GetRight(), y, 1, 1 ), eType_Chunk, &m_vecChunk4[!b1] );
			}
		}
	}
	else
	{
		int8 nType = SRand::Inst().Rand( 0, 2 );
		if( nType == 0 )
		{
			int32 y = r.y;
			int8 bDir = SRand::Inst().Rand( 0, 2 );
			int32 i = SRand::Inst().Rand( 0, 4 );
			int32 h0 = SRand::Inst().Rand( 2, 4 );
			int32 n1 = 0;
			if( i > r.width - 4 )
			{
				i = Min( r.width - 3, i - ( r.width - 5 ) );
				y++;
				n1 = SRand::Inst().Rand( 0, r.width - i - 2 + 1 );
			}
			while( y < r.GetBottom() - 1 )
			{
				int32 h = Min( r.width - 1 - i, Min( r.GetBottom() - y, h0 + SRand::Inst().Rand( 0, 2 ) ) );
				int32 h1 = 0;
				if( SRand::Inst().Rand( 0, 2 ) )
				{
					h1 = Max( 2, SRand::Inst().Rand( -4, 8 ) );
					if( r.GetBottom() - y - h >= h1 )
					{
						if( h1 >= r.GetBottom() - y - h - 2 )
							h1 = r.GetBottom() - y - h;
						i = 0;
					}
					else
						h1 = 0;
				}

				for( int x = 0; x < i; x++ )
					AddChunk( TRectangle<int32>( bDir ? r.x + x : r.GetRight() - x - 1, y, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + i + 1 - x ) % 2] );
				for( int j = 0; j < h; j++ )
				{
					AddChunk( TRectangle<int32>( bDir ? r.x + i + j : r.GetRight() - ( i + j ) - 2, y + j, 2, 1 ), eType_Chunk, &m_vecChunk3[bDir] );
					if( j < h - 1 )
					{
						for( int i1 = i + 2; i1 < Min( r.width - j, i + 2 + n1 ); i1++ )
							AddChunk( TRectangle<int32>( bDir ? r.x + i1 + j : r.GetRight() - ( i1 + j ) - 1, y + j, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + 1 + i1 - i ) % 2] );
					}
				}
				for( int x = i + 2 + h - 1; x < r.width; x++ )
					AddChunk( TRectangle<int32>( bDir ? r.x + x : r.GetRight() - x - 1, y + h - 1, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + x - ( i + h ) ) % 2] );
				y += h;

				int32 i0 = i + h - 3;
				if( h1 )
				{
					if( h1 <= SRand::Inst().Rand( 3, 6 ) )
					{
						AddChunk( TRectangle<int32>( r.x, y + h1 - 2, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );
						for( int j = y - h + 1; j < y + h1 - 2; j++ )
							AddChunk( TRectangle<int32>( bDir ? r.x : r.GetRight() - 1, j, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + j - ( y - h + 1 ) ) % 2] );
						int32 w1 = SRand::Inst().Rand( 2, r.width - 1 + 1 );
						for( int j = y; j < y + h1 - 2; j++ )
							AddChunk( TRectangle<int32>( bDir ? r.GetRight() - 1 : r.x, j, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + j - y ) % 2] );
						if( h1 - 2 >= 2 )
							AddChunk( TRectangle<int32>( r.x + 1, y, r.width - 2, h1 - 2 ), eType_Chunk, &m_vecChunk5 );
						i0 = r.width;
					}
					else
					{
						AddChunk( TRectangle<int32>( r.x, y + h1 - 3, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );
						for( int j = y - h + 1; j < y + h1 - 3; j++ )
							AddChunk( TRectangle<int32>( bDir ? r.x : r.GetRight() - 1, j, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + j - ( y - h + 1 ) ) % 2] );
						int32 w1 = SRand::Inst().Rand( 2, r.width - 2 + 1 );
						AddChunk( TRectangle<int32>( bDir ? r.GetRight() - w1 : r.x, y + h1 - 2, w1, 2 ), eType_Cargo1_1, &m_vecCargos1 );
						for( int j = y; j < y + h1 - 3; j++ )
							AddChunk( TRectangle<int32>( bDir ? r.GetRight() - 1 : r.x, j, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + j - y ) % 2] );
						if( h1 - 3 >= 2 )
							AddChunk( TRectangle<int32>( r.x + 1, y, r.width - 2, h1 - 3 ), eType_Chunk, &m_vecChunk5 );
						i0 = r.width - w1 - 2;
					}
					y += h1;
				}

				int32 i1 = Max( y + 3 - r.GetBottom(), SRand::Inst().Rand( 0, r.width - 2 ) );
				if( i1 >= i0 )
				{
					y++;
					i1 = Max( y + 3 - r.GetBottom(), i1 );
					n1 = SRand::Inst().Rand( 0, r.width - i1 - 2 + 1 );
				}
				else
					n1 = SRand::Inst().Rand( 0, i0 - i1 );
				i = i1;
			}
		}
		else if( nType == 1 )
		{
			int32 y = r.y;
			int8 bDir = SRand::Inst().Rand( 0, 2 );
			int32 nType0 = -1;
			while( y < r.GetBottom() - 2 )
			{
				int32 h = r.GetBottom() - y;
				int8 nType1[] = { 0, 1 };
				SRand::Inst().Shuffle( nType1, ELEM_COUNT( nType1 ) );
				for( int i = 0; i < ELEM_COUNT( nType1 ); i++ )
				{
					if( nType1[i] == 0 )
					{
						if( h < 3 || y == r.y )
							continue;
						if( nType0 == 0 )
						{
							y++;
							h--;
						}
						h = Min( h, SRand::Inst().Rand( 4, 9 ) );
						if( y + h >= r.GetBottom() - 2 )
						{
							if( r.GetBottom() - y >= 7 )
								h = r.GetBottom() - y - 3;
							else
								h = r.GetBottom() - y;
						}
						int32 h1 = h - 2;
						int32 w1 = SRand::Inst().Rand( 2, r.width - 2 + 1 );
						bDir = SRand::Inst().Rand( 0, 2 );
						AddChunk( TRectangle<int32>( bDir ? r.GetRight() - w1 : r.x, y + h1, w1, 2 ), eType_Cargo1_1, &m_vecCargos1 );
						h1--;
						AddChunk( TRectangle<int32>( r.x, y + h1, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );
						for( int i = 0; i < 2; i++ )
						{
							for( int j = 0; j < h1; j++ )
							{
								AddChunk( TRectangle<int32>( r.x + i * ( r.width - 1 ), y + j, 1, 1 ), eType_Chunk, &m_vecChunk4[( i + h1 - j ) % 2] );
							}
						}
						if( h1 > 1 )
						{
							TRectangle<int32> r1( r.x + 1, y, r.width - 2, h1 );
							r1.SetTop( r1.GetBottom() - Max( 2, r1.height * 2 / 3 ) );
							AddChunk( r1, eType_Chunk, &m_vecChunk5 );
						}
					}
					else if( nType1[i] == 1 )
					{
						h = Min( h, SRand::Inst().Rand( 3, 8 ) );
						if( y + h >= r.GetBottom() - 2 )
							h = r.GetBottom() - y;
						auto h1 = h - 2;
						AddChunk( TRectangle<int32>( r.x, y + h1, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );

						bDir = !bDir;
						int32 i = 0;
						for( int j = 0; j < h1; j++ )
						{
							AddChunk( TRectangle<int32>( bDir ? r.x + i : r.GetRight() - i - 2, y + j, 2, 1 ), eType_Chunk, &m_vecChunk3[bDir] );
							i++;
							if( i >= r.width - 1 )
							{
								i = 0;
								bDir = !bDir;
							}
						}
						for( int i = 0; i < 2; i++ )
						{
							int32 x = r.x + i * ( r.width - 1 );
							int j0 = 0;
							for( ; j0 < h1; j0++ )
							{
								if( m_gendata[x + ( j0 + y ) * nWidth] != eType_Billboard )
									break;
							}
							for( int j = h1 - 1; j > j0; j-- )
							{
								auto y1 = j + y;
								if( m_gendata[x + y1 * nWidth] == eType_Billboard )
									AddChunk( TRectangle<int32>( x, y1, 1, 1 ), eType_Chunk, &m_vecChunk4[( i + h1 - j ) % 2] );
							}
						}
						int8 bDir = SRand::Inst().Rand( 0, 2 );
					}
					nType0 = nType1[i];
					break;
				}
				y += h;
			}
		}
	}
}

void CLevelGenNode2_3_0::AddConn2( const TRectangle<int32>& r )
{
	AddChunk( r, eType_Billboard, &m_vecBillboards );
	int8 nTypes[] = { 0, 1 };
	SRand::Inst().Shuffle( nTypes, ELEM_COUNT( nTypes ) );
	for( int i = 0; i < ELEM_COUNT( nTypes ); i++ )
	{
		if( nTypes[i] == 0 )
		{
			TRectangle<int32> rect( r.x + 1, r.y + 1, r.width - 2, r.height - 2 );
			int8 bDir = SRand::Inst().Rand( 0, 2 );
			int8 b = rect.width <= 2 ? 1 : SRand::Inst().Rand( 0, 2 );
			for( int i = 0; i < rect.width; b = !b )
			{
				int32 w = rect.width - i;
				if( b )
				{
					if( i >= rect.width - 1 )
						break;
					w = Min( w, SRand::Inst().Rand( 2, 4 ) );
					AddChunk( TRectangle<int32>( bDir ? rect.x + i : rect.GetRight() - w - i, rect.y, w, rect.height ), eType_Cargo1_1, &m_vecCargos1 );
				}
				else
				{
					w = 1;
					for( int y = rect.y; y < rect.GetBottom(); y++ )
					{
						AddChunk( TRectangle<int32>( bDir ? rect.x + i : rect.GetRight() - i - 1, y, 1, 1 ), eType_Chunk, &m_vecChunk4[( bDir + y + i ) % 2] );
					}
					AddChunk( TRectangle<int32>( bDir ? rect.x + i : rect.GetRight() - 2 - i, rect.y, 2, rect.height ), eType_Chunk, &m_vecChunk5 );
				}
				i += w;
			}
			AddChunk( TRectangle<int32>( r.x, r.y, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );
			AddChunk( TRectangle<int32>( r.x, r.GetBottom() - 1, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );
		}
		else if( nTypes[i] == 1 )
		{
			int32 h1 = Min( r.height - 1, r.width / 2 - 1 );
			for( int j = 0; j < h1; j++ )
			{
				for( int i = 0; i < 2; i++ )
					AddChunk( TRectangle<int32>( i == 1 ? r.x + j : r.GetRight() - 2 - j, r.y + j, 2, 1 ), eType_Chunk, &m_vecChunk3[i] );
			}
			if( h1 <= r.height - 2 )
			{
				auto w = 2 + ( r.width & 1 );
				AddChunk( TRectangle<int32>( r.x + ( r.width - w ) / 2, r.y + h1, w, r.height - h1 ), eType_Cargo1_1, &m_vecCargos1 );
				for( int i = 0; i < ( r.width - w ) / 2; i++ )
				{
					AddChunk( TRectangle<int32>( r.x + i, r.GetBottom() - 1, 1, 1 ), eType_Chunk, &m_vecChunk4[i % 2] );
					AddChunk( TRectangle<int32>( r.GetRight() - 1 - i, r.GetBottom() - 1, 1, 1 ), eType_Chunk, &m_vecChunk4[( i + 1 ) % 2] );
				}
			}
			else
			{
				AddChunk( TRectangle<int32>( r.x, r.GetBottom() - 1, r.width, 1 ), eType_Chunk, &m_vecBar0[3] );
				if( r.width > ( h1 + 1 ) * 2 )
				{
					int8 n = SRand::Inst().Rand( 0, 2 );
					for( int i = r.x + h1 + 1; i < r.GetRight() - h1 - 1; i++ )
						AddChunk( TRectangle<int32>( i, r.GetBottom() - 2, 1, 1 ), eType_Chunk, &m_vecChunk4[( i + n ) % 2] );
				}
			}
		}
		break;
	}

}

void CLevelGenNode2_3_0::AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32>>* pVec, bool b )
{
	if( pVec )
		pVec->push_back( rect );
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			( b ? m_gendata1 : m_gendata )[i + j * m_region.width] = nType;
		}
	}
}

void CLevelGenNode2_3_0::GenArea1( const TRectangle<int32>& rect1, const TRectangle<int32>& rect2, const TVector2<int32>& ofs, int32 nCount )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;

	auto Func = [=] ( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec, vector<TRectangle<int32> >* pVec1 = NULL, bool b = false )
	{
		TRectangle<int32> r1 = rect.Offset( TVector2<int32>( -rect1.x, -rect1.y ) );
		r1 = r1.Offset( TVector2<int32>( rect1.x, rect1.y ) );
		auto r2 = r1;
		r2.x = rect1.x + rect1.GetRight() - r2.x - r2.width;
		bool b1 = false;
		for( int i = 0; i < nCount; i++ )
		{
			auto r3 = !!( i & 1 ) ? r2 : r1;
			AddChunk( r3.Offset( ofs * i ), nType, pVec1 && b1 ? pVec1 : pVec, b );
			b1 = !b1;
		}
	};
	auto Func0 = [=, &Func] ( const TRectangle<int32>& rect, const TRectangle<int32>& rect0 )
	{
		if( rect.width < 1 || rect.height < 2 || rect.width + rect.height <= 3 )
			return;
		for( int32 i = SRand::Inst().Rand( ( rect.width + rect.height - 2 ) / 4, ( rect.width + rect.height - 2 ) / 2 + 1 ); i > 0; i-- )
		{
			int32 n = SRand::Inst().Rand( 0, rect.width + rect.height - 3 );
			if( n < rect.width - 1 )
			{
				int32 x = n + 1;
				int32 w = Min( rect.width - x, SRand::Inst().Rand( 1, 3 ) );
				if( w > 1 )
					i--;
				TRectangle<int32> r( rect.x + x, rect.y, w, rect.height );
				Func( r, eType_Building_3, NULL );
				Func( TRectangle<int32>( rect0.GetRight() + rect0.x - r.x - r.width, r.y, r.width, r.height ), eType_Building_3, NULL );
			}
			else
			{
				int32 y = n - ( rect.width - 1 ) + 1;
				int32 h = Min( rect.height - 1 - y, SRand::Inst().Rand( 1, 3 ) );
				if( h > 1 )
					i--;
				TRectangle<int32> r( rect.x, rect.y + y, rect.width, h );
				Func( r, eType_Building_3, NULL );
				Func( TRectangle<int32>( rect0.GetRight() + rect0.x - r.x - r.width, r.y, r.width, r.height ), eType_Building_3, NULL );
			}
		}

		bool b0 = false;
		for( int32 x = 2; x < rect.width; x += 2 )
		{
			for( int k = 0; k < 2; k++ )
			{
				if( x == rect.width - 2 && !( rect.width & 2 ) )
					continue;
				if( x == rect.width - 1 && ( rect.width & 2 ) )
					continue;
				int32 x1 = Min( x + 2, rect.width );
				int32 l;
				for( l = 0; l < rect.height; l++ )
				{
					bool b = true;
					for( int i = x; i < x1; i++ )
					{
						auto n = m_gendata[i + rect.x + ( k == 0 ? rect.y + l : rect.GetBottom() - 1 - l ) * nWidth];
						if( n == eType_Building_1 || n == eType_Building_2 || n == eType_Building_3 && b0 && ( l == 0 || SRand::Inst().Rand( 0, 2 ) ) )
						{
							b = false;
							break;
						}
					}
					if( !b )
						break;
				}
				if( l > 0 )
				{
					b0 = true;
					TRectangle<int32> r( rect.x + x, k == 0 ? rect.y : rect.GetBottom() - l, x1 - x, l );
					Func( r, eType_Building_1, NULL );
					Func( TRectangle<int32>( rect0.GetRight() + rect0.x - r.x - r.width, r.y, r.width, r.height ), eType_Building_1, NULL );
				}

				x += SRand::Inst().Rand( 1, 3 ) * 2;
			}
		}
		b0 = false;
		for( int32 y = SRand::Inst().Rand( 1, 4 ); y < rect.height - 2; y++ )
		{
			int32 y1 = y + 2;
			int32 l;
			for( l = 0; l < rect.width; l++ )
			{
				bool b = true;
				for( int i = y; i < y1; i++ )
				{
					auto n = m_gendata[rect.x + l + ( i + rect.y ) * nWidth];
					if( n == eType_Building_1 || n == eType_Building_2 || n == eType_Building_3 && b0 && ( l == 0 || SRand::Inst().Rand( 0, 2 ) ) )
					{
						b = false;
						break;
					}
				}
				if( !b )
					break;
			}
			if( l > 0 )
			{
				b0 = true;
				TRectangle<int32> r( rect.x, rect.y + y, l, y1 - y );
				Func( r, eType_Building_2, NULL );
				Func( TRectangle<int32>( rect0.GetRight() + rect0.x - r.x - r.width, r.y, r.width, r.height ), eType_Building_2, NULL );
			}

			y += SRand::Inst().Rand( 2, 5 );
		}

		int32 n = 1 + ( rect.width * rect.height ) / 20;
		int32* xs = (int32*)alloca( sizeof( int32 ) * n );
		int32* ys = (int32*)alloca( sizeof( int32 ) * n );
		for( int i = 0; i < n; i++ )
		{
			xs[i] = i;
			ys[i] = i;
		}
		SRand::Inst().Shuffle( xs, n );
		SRand::Inst().Shuffle( ys, n );
		int32 s = 0;
		int32 s1 = 0;
		for( int i = 0; i < n; i++ )
		{
			s1 += ( rect.width + xs[i] ) / n;
			xs[i] = s1 - s >= 3 ? SRand::Inst().Rand( s + 1, s1 - 1 ) : SRand::Inst().Rand( s, s1 );
			s = s1;
		}
		s = 0;
		s1 = 0;
		for( int i = 0; i < n; i++ )
		{
			s1 += ( rect.height + ys[i] ) / n;
			ys[i] = s1 - s >= 3 ? SRand::Inst().Rand( s + 1, s1 - 1 ) : SRand::Inst().Rand( s, s1 );
			s = s1;
		}
		SRand::Inst().Shuffle( xs, n );
		SRand::Inst().Shuffle( ys, n );
		for( int i = 0; i < n; i++ )
		{
			TRectangle<int32> r( xs[i] + rect.x, ys[i] + rect.y, 1, 1 );
			Func( r, eType_Cargo2_1, NULL, NULL, true );
			Func( TRectangle<int32>( rect0.GetRight() + rect0.x - r.x - r.width, r.y, r.width, r.height ), eType_Cargo2_1, NULL, NULL, true );
		}
	};
	auto Func1 = [=, &Func, &Func0] ( const TRectangle<int32>& rect )
	{
		Func( rect, eType_Cargo2, &m_vecCargos2 );
		if( rect.height > 8 )
		{
			int32 h = ( rect.height - 1 ) / 2;
			if( rect.width > 10 )
			{
				int32 w = ( rect.width - 1 ) / 2;
				TRectangle<int32> rects[4] = { { rect.x, rect.y, w, h }, { rect.GetRight() - w, rect.y, w, h },
				{ rect.x, rect.GetBottom() - h, w, h }, { rect.GetRight() - w, rect.GetBottom() - h, w, h } };
				for( int i = 0; i < 4; i++ )
					Func( rects[i], eType_Building, &m_vecBuildings );
				Func0( rects[0], rect );
				Func0( rects[2], rect );
			}
			else
			{
				TRectangle<int32> rects[2] = { { rect.x, rect.y, rect.width, h }, { rect.x, rect.GetBottom() - h, rect.width, h } };
				Func( rects[0], eType_Building, &m_vecBuildings );
				Func( rects[1], eType_Building, &m_vecBuildings );
				int32 w = ( rect.width + 1 ) / 2;
				Func0( TRectangle<int32>( rects[0].x, rects[0].y, w, rects[0].height ), rect );
				Func0( TRectangle<int32>( rects[1].x, rects[1].y, w, rects[1].height ), rect );
			}
		}
		else
		{
			if( rect.width > 6 )
			{
				int32 w = ( rect.width - 1 ) / 2;
				TRectangle<int32> rects[2] = { { rect.x, rect.y, w, rect.height }, { rect.GetRight() - w, rect.y, w, rect.height } };
				for( int i = 0; i < 2; i++ )
					Func( rects[i], eType_Building, &m_vecBuildings );
				Func0( rects[0], rect );
			}
			else
			{
				auto r = rect;
				int32 h1 = Max( 1, ( r.height - 3 ) / 2 );
				r.y += h1;
				r.height -= h1 * 2;
				int32 w = ( r.width + 1 ) / 2;
				Func( r, eType_Building, &m_vecBuildings );
				Func0( TRectangle<int32>( r.x, r.y, w, r.height ), rect );
			}
		}
	};

	for( int k = 0; k < 2; k++ )
	{
		if( k == 1 )
		{
			int32 h = ( ( Min( rect1.width, rect1.height ) - 4 ) / 2 + 1 + SRand::Inst().Rand( 0, 2 ) ) / 2;
			int32 w = Max( 1, Min( h, h + ( rect1.width - rect1.height ) / 2 ) );
			TRectangle<int32> r0( rect1.x + w, rect1.y + h, rect1.width - w * 2, rect1.height - h * 2 );
			Func1( r0 );
			for( int i = 0; i < nCount; i++ )
			{
				//AddWall1( TRectangle<int32>( rect1.x, rect2.y, rect1.width, r0.y - rect2.y ).Offset( ofs * i ) );
				//AddWall1( TRectangle<int32>( rect1.x, r0.y, r0.x - rect1.x, r0.height ).Offset( ofs * i ) );
				//AddWall1( TRectangle<int32>( r0.GetRight(), r0.y, rect1.GetRight() - r0.GetRight(), r0.height ).Offset( ofs * i ) );
				//AddWall1( TRectangle<int32>( rect1.x, r0.GetBottom(), rect1.width, rect2.GetBottom() - r0.GetBottom() ).Offset( ofs * i ) );
			}

			for( int i = 0; i < h; i++ )
			{
				Func( TRectangle<int32>( rect1.x + i, rect1.y + i, 2, 1 ), eType_Chunk, &m_vecChunk1[1], &m_vecChunk1[0] );
				Func( TRectangle<int32>( rect1.GetRight() - 2 - i, rect1.y + i, 2, 1 ), eType_Chunk, &m_vecChunk1[0], &m_vecChunk1[1] );
				Func( TRectangle<int32>( rect1.x + i, rect1.GetBottom() - 1 - i, 2, 1 ), eType_Chunk, &m_vecChunk1[0], &m_vecChunk1[1] );
				Func( TRectangle<int32>( rect1.GetRight() - 2 - i, rect1.GetBottom() - 1 - i, 2, 1 ), eType_Chunk, &m_vecChunk1[1], &m_vecChunk1[0] );
			}
			break;
		}
		else
		{
			if( rect1.width >= rect1.height || rect1.height < 9 )
				continue;

			int32 hMax = Min( ( rect1.width - 4 ) / 2, ( rect1.height - 7 ) / 2 );
			bool b = false;
			for( int32 h = 1; h <= hMax; h++ )
			{
				int32 h1 = ( rect1.height - h * 2 - 1 ) / 2;
				int32 w1 = h1 * 2 - 1;
				int32 w = Max( 1, ( rect1.width - w1 ) / 2 );
				if( w > h )
					continue;
				Func1( TRectangle<int32>( rect1.x + w, rect1.y + h, rect1.width - w * 2, h1 ) );
				Func1( TRectangle<int32>( rect1.x + w, rect1.GetBottom() - h - h1, rect1.width - w * 2, h1 ) );
				for( int i = 0; i < nCount; i++ )
				{
					//AddWall1( TRectangle<int32>( rect1.x, rect2.y, rect1.width, rect1.y + h - rect2.y ).Offset( ofs * i ) );
					//AddWall1( TRectangle<int32>( rect1.x, rect1.y + h, w, rect1.height - h1 - h ).Offset( ofs * i ) );
					//AddWall1( TRectangle<int32>( rect1.GetRight() - w, rect1.y + h, w, rect1.height - h1 - h ).Offset( ofs * i ) );
					//AddWall1( TRectangle<int32>( rect1.x, rect1.GetBottom() - h1, rect1.width, rect2.GetBottom() - rect1.GetBottom() + h1 ).Offset( ofs * i ) );
				}

				for( int i = 0; i < h; i++ )
				{
					Func( TRectangle<int32>( rect1.x + i, rect1.y + i, 2, 1 ), eType_Chunk, &m_vecChunk1[1], &m_vecChunk1[0] );
					Func( TRectangle<int32>( rect1.GetRight() - 2 - i, rect1.y + i, 2, 1 ), eType_Chunk, &m_vecChunk1[0], &m_vecChunk1[1] );
					Func( TRectangle<int32>( rect1.x + i, rect1.GetBottom() - 1 - i, 2, 1 ), eType_Chunk, &m_vecChunk1[0], &m_vecChunk1[1] );
					Func( TRectangle<int32>( rect1.GetRight() - 2 - i, rect1.GetBottom() - 1 - i, 2, 1 ), eType_Chunk, &m_vecChunk1[1], &m_vecChunk1[0] );
				}
				b = true;
				break;
			}
			if( b )
				break;
		}
	}
}

void CLevelGenNode2_3_0::AddWall1( const TRectangle<int32>& rect )
{
	m_vecWall1.push_back( rect );
	for( int x = rect.x; x < rect.GetRight(); x++ )
	{
		for( int y = rect.y; y < rect.GetBottom(); y++ )
		{
			if( m_gendata[x + y * m_region.width] == 0 )
				m_gendata[x + y * m_region.width] = eType_Wall1;
		}
	}
}


void CLevelGenNode2_3_1::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_pWallNode = CreateNode( pXml->FirstChildElement( "wall" )->FirstChildElement(), context );
	m_pBlockNode[0] = CreateNode( pXml->FirstChildElement( "block_0" )->FirstChildElement(), context );
	m_pBlockNode[1] = CreateNode( pXml->FirstChildElement( "block_1" )->FirstChildElement(), context );
	m_pBlockNode[2] = CreateNode( pXml->FirstChildElement( "block_2" )->FirstChildElement(), context );
	m_pBlockNode[3] = CreateNode( pXml->FirstChildElement( "block_3" )->FirstChildElement(), context );
	m_pChunkNode1[0] = CreateNode( pXml->FirstChildElement( "chunk_1_a" )->FirstChildElement(), context );
	m_pChunkNode1[1] = CreateNode( pXml->FirstChildElement( "chunk_1_b" )->FirstChildElement(), context );
	m_pChunkNode2[0] = CreateNode( pXml->FirstChildElement( "chunk_2_a" )->FirstChildElement(), context );
	m_pChunkNode2[1] = CreateNode( pXml->FirstChildElement( "chunk_2_b" )->FirstChildElement(), context );
	m_pChunkNode3[0] = CreateNode( pXml->FirstChildElement( "chunk_3_a" )->FirstChildElement(), context );
	m_pChunkNode3[1] = CreateNode( pXml->FirstChildElement( "chunk_3_b" )->FirstChildElement(), context );
	m_pChunkNode4[0] = CreateNode( pXml->FirstChildElement( "chunk_4_a" )->FirstChildElement(), context );
	m_pChunkNode4[1] = CreateNode( pXml->FirstChildElement( "chunk_4_b" )->FirstChildElement(), context );
	m_pBuildingNode = CreateNode( pXml->FirstChildElement( "building" )->FirstChildElement(), context );

	m_pRoomNode = CreateNode( pXml->FirstChildElement( "room" )->FirstChildElement(), context );
	m_pBillboardNode = CreateNode( pXml->FirstChildElement( "billboard" )->FirstChildElement(), context );
	m_pCargoNode = CreateNode( pXml->FirstChildElement( "cargo" )->FirstChildElement(), context );
	m_pCargo1Node = CreateNode( pXml->FirstChildElement( "cargo1" )->FirstChildElement(), context );
	m_pCargo2Node = CreateNode( pXml->FirstChildElement( "cargo2" )->FirstChildElement(), context );
	m_pBrokenNode = CreateNode( pXml->FirstChildElement( "broken" )->FirstChildElement(), context );
	m_pBoxNode = CreateNode( pXml->FirstChildElement( "box" )->FirstChildElement(), context );
	m_pBar0Node[0] = CreateNode( pXml->FirstChildElement( "bar0" )->FirstChildElement(), context );
	m_pBar0Node[1] = CreateNode( pXml->FirstChildElement( "bar0_r" )->FirstChildElement(), context );
	m_pBar0Node[2] = CreateNode( pXml->FirstChildElement( "bar0_l" )->FirstChildElement(), context );
	m_pBarNode[0] = CreateNode( pXml->FirstChildElement( "bar_h" )->FirstChildElement(), context );
	m_pBarNode[1] = CreateNode( pXml->FirstChildElement( "bar_v" )->FirstChildElement(), context );
	m_pBarNode_a[0] = CreateNode( pXml->FirstChildElement( "bar_h_a" )->FirstChildElement(), context );
	m_pBarNode_a[1] = CreateNode( pXml->FirstChildElement( "bar_v_a" )->FirstChildElement(), context );
	m_pControlRoomNode = CreateNode( pXml->FirstChildElement( "control_room" )->FirstChildElement(), context );

	m_pSawBladeNode[0] = CreateNode( pXml->FirstChildElement( "sawblade_0" )->FirstChildElement(), context );
	m_pSawBladeNode[1] = CreateNode( pXml->FirstChildElement( "sawblade_1" )->FirstChildElement(), context );
	m_pSawBladeNode[2] = CreateNode( pXml->FirstChildElement( "sawblade_2" )->FirstChildElement(), context );
	m_pSawBladeNode[3] = CreateNode( pXml->FirstChildElement( "sawblade_3" )->FirstChildElement(), context );

	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenNode2_3_1::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	m_pContext = &context;
	m_region = region;
	m_gendata.resize( region.width * region.height );

	GenBase();

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
				m_pWallNode->Generate( context, TRectangle<int32>( x, y, 1, 1 ) );
		}
	}

	for( int i = 0; i < 4; i++ )
	{
		for( auto& p : m_vecBlocks[i] )
		{
			m_pBlockNode[i]->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecChunk1[i] )
		{
			m_pChunkNode1[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
		for( auto& rect : m_vecChunk2[i] )
		{
			m_pChunkNode2[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
		for( auto& rect : m_vecChunk3[i] )
		{
			m_pChunkNode3[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
		for( auto& rect : m_vecChunk4[i] )
		{
			m_pChunkNode4[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( auto& rect : m_vecBuildings )
	{
		m_pBuildingNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	for( int i = 0; i < 3; i++ )
	{
		for( auto& rect : m_vecBar0[i] )
		{
			m_pBar0Node[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecBar[i] )
		{
			m_pBarNode[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		for( auto& rect : m_vecBar_a[i] )
		{
			m_pBarNode_a[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}

	for( auto& p : m_vecBroken )
	{
		m_pBrokenNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& p : m_vecBox )
	{
		m_pBoxNode->Generate( context, TRectangle<int32>( p.x + region.x, p.y + region.y, 1, 1 ) );
	}
	for( auto& rect : m_vecCargos )
	{
		m_pCargoNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Cargo1_1;
	context.mapTags["2"] = eType_Cargo1_2;
	context.mapTags["3"] = eType_Cargo1_3;
	context.mapTags["4"] = eType_Cargo1_4;
	context.mapTags["5"] = eType_Cargo1_5;
	context.mapTags["6"] = eType_Cargo1_6;
	for( auto& rect : m_vecCargos1 )
	{
		m_pCargo1Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Cargo2_1;
	context.mapTags["2"] = eType_Cargo2_2;
	context.mapTags["3"] = eType_Cargo2_3;
	context.mapTags["4"] = eType_Cargo2_4;
	for( auto& rect : m_vecCargos2 )
	{
		m_pCargo2Node->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags["1"] = eType_Control_Room_1;
	context.mapTags["2"] = eType_Control_Room_2;
	context.mapTags["3"] = eType_Control_Room_3;
	context.mapTags["4"] = eType_Control_Room_4;
	context.mapTags["5"] = eType_Control_Room_5;
	for( auto& room : m_vecControlRooms )
	{
		auto rect = room.rect;
		context.mapTags["left"] = room.b[0];
		context.mapTags["top"] = room.b[1];
		context.mapTags["right"] = room.b[2];
		context.mapTags["bottom"] = room.b[3];
		m_pControlRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}

	context.mapTags["1"] = eType_Room_1;
	context.mapTags["2"] = eType_Room_2;
	context.mapTags["door"] = eType_Room_Door;
	for( auto& rect : m_vecRooms )
	{
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_gendata[i + j * context.nWidth] != eType_Room_2 && m_gendata[i + j * context.nWidth] != eType_Room_Door )
					m_gendata[i + j * context.nWidth] = context.blueprint[i + region.x + ( j + region.y ) * context.nWidth] = eType_Room_1;
			}
		}
		m_pRoomNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	context.mapTags["1"] = eType_Billboard_1;
	context.mapTags["2"] = eType_Billboard_2;
	for( auto& rect : m_vecBillboards )
	{
		m_pBillboardNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
	}
	for( int i = 0; i < 4; i++ )
	{
		for( auto& rect : m_vecSawBlade[i] )
		{
			m_pSawBladeNode[i]->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
		}
	}

	context.mapTags.clear();
	m_gendata.clear();

	for( int i = 0; i < 4; i++ )
		m_vecBlocks[i].clear();
	for( int i = 0; i < 2; i++ )
	{
		m_vecChunk1[i].clear();
		m_vecChunk2[i].clear();
		m_vecChunk3[i].clear();
		m_vecChunk4[i].clear();
	}
	m_vecBuildings.clear();
	m_vecRooms.clear();
	m_vecBillboards.clear();
	m_vecCargos.clear();
	m_vecCargos1.clear();
	m_vecCargos2.clear();
	m_vecControlRooms.clear();
	m_vecBroken.clear();
	m_vecBox.clear();
	m_vecBar0[0].clear();
	m_vecBar0[1].clear();
	m_vecBar0[2].clear();
	m_vecBar[0].clear();
	m_vecBar[1].clear();
	m_vecBar_a[0].clear();
	m_vecBar_a[1].clear();
	for( int i = 0; i < 4; i++ )
		m_vecSawBlade[i].clear();
}

void CLevelGenNode2_3_1::GenBase()
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector3<int32> > vec;
	vector<TVector3<int32> > vec1;
	vector<TVector3<int32> > vec2;

	TRectangle<int32> lastRect( 0, 0, nWidth, 0 );
	int32 n = SRand::Inst().Rand( 0, 3 );
	if( n == 0 )
	{
		lastRect.width = SRand::Inst().Rand( 13, 21 );
	}
	else if( n == 1 )
	{
		lastRect.width = SRand::Inst().Rand( 13, 21 );
		lastRect.x = nWidth - lastRect.width;
	}
	else
	{
		int32 x1 = SRand::Inst().Rand( 7, 10 );
		lastRect.x = x1;
		lastRect.SetRight( nWidth - x1 );
	}
	auto Func1 = [=, &vec1] ( const TVector3<int32>& v )
	{
		vec1.push_back( v );
		TVector2<int32> p0( v.x, v.y );
		int8 nDir = v.z > 0 ? 1 : -1;
		int32 h = v.z > 0 ? v.z : -v.z;
		int32 k = ( p0.x + p0.y ) & 1 ? 0 : 1;
		int32 X1 = k == 0 ? p0.x - 1 : p0.x, X2 = k == 0 ? p0.x : p0.x + 1;
		for( int i = 0; i < h; i++ )
		{
			TVector2<int32> p( X2 + i * nDir, p0.y + i );
			m_gendata[p.x - 1 + p.y * nWidth] = m_gendata[p.x + p.y * nWidth] = eType_Temp0;
		}
	};
	while( lastRect.GetBottom() < nHeight - 4 )
	{
		TRectangle<int32> rect1( 0, 0, 0, 0 );

		if( lastRect.x == 0 )
		{
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				bool b = lastRect.width <= SRand::Inst().Rand( 15, 19 );
				rect1.y = Min( nHeight, lastRect.GetBottom() + SRand::Inst().Rand( 5, 8 ) );
				if( nHeight - rect1.y <= 2 )
					rect1.y = nHeight;
				else if( nHeight - rect1.y <= 3 )
				{
					rect1.y = nHeight - 2;
					rect1.height = 2;
				}
				else
					rect1.height = 2;
				bool b1 = false;
				if( m_vecBuildings.size() )
				{
					AddChunk( lastRect, 0, NULL );
					m_vecBuildings.pop_back();
					lastRect.height--;
					b1 = true;
				}
				rect1.width = b ? lastRect.width + ( rect1.y - lastRect.GetBottom() ) : lastRect.width - ( rect1.y - lastRect.GetBottom() );
				rect1.x = 0;
				if( rect1.height > 0 )
					AddChunk( rect1, eType_Chunk, NULL );
				int32 n1 = SRand::Inst().Rand( 1, 3 );
				if( b1 )
					n1 += GenTrap( lastRect, rect1, 0, b );
				if( rect1.height > 0 )
					m_vecBuildings.push_back( rect1 );
				int32 x = lastRect.GetRight() - 1 - n1;
				Func1( TVector3<int32>( x, lastRect.GetBottom(), b ? rect1.y - lastRect.GetBottom() : -rect1.y + lastRect.GetBottom() ) );
			}
			else
			{
				int32 x1 = SRand::Inst().Rand( 7, 10 );
				rect1.x = x1;
				rect1.SetRight( nWidth - x1 );
				rect1.y = Min( nHeight, lastRect.GetBottom() + SRand::Inst().Rand( 7, 11 ) );
				if( nHeight - rect1.y <= 2 )
					rect1.y = nHeight;
				else if( nHeight - rect1.y <= 3 )
				{
					rect1.y = nHeight - 2;
					rect1.height = 2;
				}
				else
					rect1.height = 2;
				if( rect1.height > 0 )
					AddChunk( rect1, eType_Chunk, &m_vecBuildings );
				int32 d = rect1.y - lastRect.GetBottom();
				int32 xMin = Max( lastRect.x, rect1.x - d );
				int32 xMax = Min( lastRect.GetRight(), rect1.GetRight() - d ) - 1;
				Func1( TVector3<int32>( ( xMin + xMax + SRand::Inst().Rand( 0, 2 ) ) / 2, lastRect.GetBottom(), d ) );
			}
		}
		else if( lastRect.GetRight() == nWidth )
		{
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				bool b = lastRect.width <= SRand::Inst().Rand( 15, 19 );
				rect1.y = Min( nHeight, lastRect.GetBottom() + SRand::Inst().Rand( 5, 8 ) );
				if( nHeight - rect1.y <= 2 )
					rect1.y = nHeight;
				else if( nHeight - rect1.y <= 3 )
				{
					rect1.y = nHeight - 2;
					rect1.height = 2;
				}
				else
					rect1.height = 2;
				bool b1 = false;
				if( m_vecBuildings.size() )
				{
					AddChunk( lastRect, 0, NULL );
					m_vecBuildings.pop_back();
					lastRect.height--;
					b1 = true;
				}
				rect1.width = b ? lastRect.width + ( rect1.y - lastRect.GetBottom() ) : lastRect.width - ( rect1.y - lastRect.GetBottom() );
				rect1.x = nWidth - rect1.width;
				if( rect1.height > 0 )
					AddChunk( rect1, eType_Chunk, NULL );
				int32 n1 = SRand::Inst().Rand( 1, 3 );
				if( b1 )
					n1 += GenTrap( lastRect, rect1, 1, b );
				if( rect1.height > 0 )
					m_vecBuildings.push_back( rect1 );
				int32 x = lastRect.x + n1;
				Func1( TVector3<int32>( x, lastRect.GetBottom(), b ? -rect1.y + lastRect.GetBottom() : rect1.y - lastRect.GetBottom() ) );
			}
			else
			{
				int32 x1 = SRand::Inst().Rand( 7, 10 );
				rect1.x = x1;
				rect1.SetRight( nWidth - x1 );
				rect1.y = Min( nHeight, lastRect.GetBottom() + SRand::Inst().Rand( 7, 11 ) );
				if( nHeight - rect1.y <= 2 )
					rect1.y = nHeight;
				else if( nHeight - rect1.y <= 3 )
				{
					rect1.y = nHeight - 2;
					rect1.height = 2;
				}
				else
					rect1.height = 2;
				if( rect1.height > 0 )
					AddChunk( rect1, eType_Chunk, &m_vecBuildings );
				int32 d = rect1.y - lastRect.GetBottom();
				int32 xMin = Max( lastRect.x, rect1.x + d );
				int32 xMax = Min( lastRect.GetRight(), rect1.GetRight() + d ) - 1;
				Func1( TVector3<int32>( ( xMin + xMax + SRand::Inst().Rand( 0, 2 ) ) / 2, lastRect.GetBottom(), -d ) );
			}
		}
		else
		{
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				int32 h = nHeight - lastRect.GetBottom();
				if( h < 12 )
				{
					rect1.y = h >= 8 ? Min( lastRect.GetBottom() + 8, nHeight - 2 ) : nHeight;
					rect1.SetBottom( nHeight );
					int32 w1 = SRand::Inst().Rand( 4, 7 );
					TRectangle<int32> rects[2] = { { 0, rect1.y, w1, rect1.height }, { nWidth - w1, rect1.y, w1, rect1.height } };
					int32 d = rect1.y - lastRect.GetBottom();
					for( int k = 0; k < 2; k++ )
					{
						if( rects[k].height > 0 )
							AddChunk( rects[k], eType_Chunk, NULL );
						int32 d1 = k == 0 ? d : -d;
						int32 xMin = Max( lastRect.x, rects[k].x + d1 );
						int32 xMax = Min( lastRect.GetRight(), rects[k].GetRight() + d1 ) - 1;
						Func1( TVector3<int32>( ( xMin + xMax + SRand::Inst().Rand( 0, 2 ) ) / 2, lastRect.GetBottom(), -d1 ) );
					}
				}
				else
				{
					rect1.y = lastRect.GetBottom() + Min( h, SRand::Inst().Rand( 12, 18 ) );
					if( nHeight - rect1.y <= 2 )
						rect1.y = nHeight;
					else if( nHeight - rect1.y <= 3 )
					{
						rect1.y = nHeight - 2;
						rect1.height = 2;
					}
					else
						rect1.height = 2;
					int32 x1 = SRand::Inst().Rand( 7, 10 );
					rect1.x = x1;
					rect1.SetRight( nWidth - x1 );

					h = rect1.y - lastRect.GetBottom();
					int32 h1 = Max( 5, ( h - SRand::Inst().Rand( 2, 5 ) ) / 2 );
					TRectangle<int32> rect2( 0, lastRect.GetBottom() + h1, nWidth, h - h1 * 2 );
					int32 w1 = SRand::Inst().Rand( 5, 7 );
					TRectangle<int32> rects[2] = { { 0, rect2.y, w1, rect2.height }, { nWidth - w1, rect2.y, w1, rect2.height } };
					for( int k = 0; k < 2; k++ )
					{
						if( rects[k].height > 0 )
							AddChunk( rects[k], eType_Chunk, &m_vecBuildings );
						int32 d1 = k == 0 ? h1 : -h1;
						int32 xMin = Max( lastRect.x, rects[k].x + d1 );
						int32 xMax = Min( lastRect.GetRight(), rects[k].GetRight() + d1 ) - 1;
						Func1( TVector3<int32>( ( xMin + xMax + SRand::Inst().Rand( 0, 2 ) ) / 2, lastRect.GetBottom(), -d1 ) );
						xMin = Max( rect1.x - d1, rects[k].x );
						xMax = Min( rect1.GetRight() - d1, rects[k].GetRight() ) - 1;
						Func1( TVector3<int32>( ( xMin + xMax + SRand::Inst().Rand( 0, 2 ) ) / 2, rect2.GetBottom(), d1 ) );
					}
				}
			}
			else
			{
				int8 nType = SRand::Inst().Rand( 0, 2 );
				rect1.y = Min( nHeight, lastRect.GetBottom() + SRand::Inst().Rand( 7, 11 ) );
				if( nHeight - rect1.y <= 2 )
					rect1.y = nHeight;
				else if( nHeight - rect1.y <= 3 )
				{
					rect1.y = nHeight - 2;
					rect1.height = 2;
				}
				else
					rect1.height = 2;
				int32 d = rect1.y - lastRect.GetBottom();
				if( nType == 0 )
				{
					rect1.x = 0;
					rect1.width = lastRect.GetRight() - d;
				}
				else
				{
					rect1.x = lastRect.x + d;
					rect1.width = nWidth - rect1.x;
				}
				d = nType == 0 ? -d : d;
				int32 xMin = Max( lastRect.x, rect1.x - d );
				int32 xMax = Min( lastRect.GetRight(), rect1.GetRight() - d ) - 1;
				Func1( TVector3<int32>( ( xMin + xMax + SRand::Inst().Rand( 0, 2 ) ) / 2, lastRect.GetBottom(), d ) );
			}
			if( rect1.height > 0 )
				AddChunk( rect1, eType_Chunk, &m_vecBuildings );
		}
		lastRect = rect1;
	}

	for( int i = 1; i <= 2; i++ )
	{
		for( auto& rect : m_vecBar0[i] )
		{
			if( i == 1 && rect.x == 0 || i == 2 && rect.GetRight() == nWidth )
			{
				TVector2<int32> p0( i == 1 ? rect.x : rect.GetRight() - 1, rect.y + 1 );
				if( m_gendata[p0.x + p0.y * nWidth] )
					continue;
				auto r = PutRect( m_gendata, nWidth, nHeight, p0, TVector2<int32>( 5, 4 ), TVector2<int32>( rect.width - 2, SRand::Inst().Rand( 4, 8 ) ),
					TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, eType_Billboard );
				if( r.width > 0 )
				{
					m_vecBillboards.push_back( r );
					int32 x0 = Min( r.width - 4, SRand::Inst().Rand( 1, 4 ) );
					AddChunk( TRectangle<int32>( i == 1 ? r.x : r.GetRight() - x0, r.y, x0, 1 ), eType_Billboard_1, NULL );
					AddChunk( TRectangle<int32>( i == 1 ? r.x + x0 : r.GetRight() - x0 - 3, r.y, 3, 3 ), eType_Billboard_2, NULL );
					GenBillboard( TRectangle<int32>( r.x, r.y + 1, r.width, r.height - 1 ) );
				}
			}
			else if( i == 2 && rect.x == 0 || i == 1 && rect.GetRight() == nWidth )
			{
				TVector2<int32> p0( i == 2 ? rect.x : rect.GetRight() - 1, rect.y + 1 );
				if( m_gendata[p0.x + p0.y * nWidth] )
					continue;
				auto r = PutRect( m_gendata, nWidth, nHeight, p0, TVector2<int32>( 5, 4 ), TVector2<int32>( rect.width - 2, 8 ),
					TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
				if( r.width > 0 )
				{
					m_vecSawBlade[1].push_back( TRectangle<int32>( i == 2 ? r.GetRight() : r.x - 2, rect.y, 2, 1 ) );
					GenWorkshop( r, TVector2<int32>( i == 2 ? r.GetRight() - 1 : r.x, r.y ) );
				}
			}
		}
	}

	for( auto& item : m_vecTempWorkshop1 )
	{
		GenWorkshop1( item.first, item.second );
	}
	m_vecTempWorkshop1.clear();

	for( auto& i : m_gendata )
	{
		if( i == eType_Temp0 )
			i = 0;
	}

	SRand::Inst().Shuffle( vec1 );
	for( int iItem = 0; iItem < vec1.size(); iItem++ )
	{
		auto& item = vec1[iItem];
		TVector2<int32> p0( item.x, item.y );
		int8 nDir = item.z > 0 ? 1 : -1;
		int32 h = item.z > 0 ? item.z : -item.z;
		float fMin = 100000.0f;
		int32 nMin = -1;
		int32 lMin[2];
		int32 k = ( p0.x + p0.y ) & 1 ? 0 : 1;
		int32 X1 = k == 0 ? p0.x - 1 : p0.x, X2 = k == 0 ? p0.x : p0.x + 1;

		for( int i = 0; i < h; i++ )
		{
			TVector2<int32> p( X2 + i * nDir, p0.y + i );
			if( m_gendata[p.x - 1 + p.y * nWidth] || m_gendata[p.x + p.y * nWidth] )
			{
				lMin[0] = lMin[1] = 0;
				nMin = h;
				break;
			}
		}
		for( int iStep = 0; iStep < 2 && nMin < 0; iStep++ )
		{
			for( int i = 1; i < h - 1; i++ )
			{
				float f = i - ( h - 1 ) * 0.5f;
				f = f * f * 3 + SRand::Inst().Rand( 0.0f, 0.1f );
				int32 l[2];
				for( int j = 0; j < 2; j++ )
				{
					for( l[j] = 0; ; l[j]++ )
					{
						int32 y = j == 0 ? l[j] + 1 + p0.y + i : p0.y + i - l[j] - 1;
						int32 x1 = X1 + nDir * ( i - ( j == 0 ? 1 : -1 ) * ( l[j] + 1 ) );
						int32 x2 = X2 + nDir * ( i - ( j == 0 ? 1 : -1 ) * ( l[j] + 1 ) );
						if( y >= nHeight || y < 0 )
							break;
						if( x1 < 0 || x2 >= nWidth )
						{
							if( iStep == 0 )
								f += 100000.0f;
							else
							{
								f += l[j] + 20;
								l[j] = 0;
							}
							break;
						}
						int32 n1 = m_gendata[x1 + y * nWidth], n2 = m_gendata[x2 + y * nWidth];
						bool b1 = n1 == 0 || n1 == eType_Temp0;
						bool b2 = n2 == 0 || n2 == eType_Temp1;
						if( nDir < 0 )
							swap( b1, b2 );
						if( j )
							swap( b1, b2 );
						if( !b1 && b2 )
						{
							if( iStep == 0 )
								f += 100000.0f;
							else
							{
								f += l[j] + 20;
								l[j] = 0;
							}
						}
						if( !b1 || !b2 )
							break;
					}
				}
				if( iStep > 0 && !l[0] && !l[1] )
					continue;
				f += l[0] + l[1];
				if( f < fMin )
				{
					fMin = f;
					nMin = i;
					lMin[0] = l[0];
					lMin[1] = l[1];
				}
			}
		}

		if( nMin >= 0 )
		{
			if( nMin < h && h < lMin[0] + lMin[1] + 3 )
			{
				for( int i = 0; i < h; i++ )
				{
					TVector2<int32> p( X2 + i * nDir, p0.y + i );
					vec.push_back( TVector3<int32>( p.x, p.y, ( nDir > 0 ? 1 : 0 ) | 2 ) );
					m_gendata[p.x - 1 + p.y * nWidth] = eType_Temp2;
					m_gendata[p.x + p.y * nWidth] = eType_Temp3;
				}
				TVector2<int32> p1( X2 + nMin * nDir, p0.y + nMin );
				for( int k = 0; k < 2; k++ )
				{
					if( lMin[k] > 0 )
					{
						TVector3<int32> p;
						if( k == 0 )
							p = TVector3<int32>( p1.x - nDir, p1.y + 1, -nDir * lMin[0] );
						else
							p = TVector3<int32>( p1.x + nDir * lMin[1], p1.y - lMin[1], -nDir * lMin[1] );
						if( lMin[k] <= 6 )
						{
							for( int i = 0; i < lMin[k]; i++ )
							{
								TVector2<int32> p( p.x - i * nDir, p.y + i );
								if( !m_gendata[p.x - 1 + p.y * nWidth] && !m_gendata[p.x + p.y * nWidth] )
								{
									vec.push_back( TVector3<int32>( p.x, p.y, nDir < 0 ? 1 : 0 ) );
									m_gendata[p.x - 1 + p.y * nWidth] = eType_Temp0;
									m_gendata[p.x + p.y * nWidth] = eType_Temp1;
								}
								else if( m_gendata[p.x - 1 + p.y * nWidth] == eType_Temp0 && m_gendata[p.x + p.y * nWidth] == eType_Temp1 )
								{
									vec.push_back( TVector3<int32>( p.x, p.y, ( nDir < 0 ? 1 : 0 ) | 2 ) );
									m_gendata[p.x - 1 + p.y * nWidth] = eType_Temp2;
									m_gendata[p.x + p.y * nWidth] = eType_Temp3;
								}
							}
						}
						else
							vec1.push_back( p );
					}
				}
			}
			else
			{
				for( int i = 0; i < h; i++ )
				{
					TVector2<int32> p( X2 + i * nDir, p0.y + i );
					if( !m_gendata[p.x - 1 + p.y * nWidth] && !m_gendata[p.x + p.y * nWidth] )
					{
						vec.push_back( TVector3<int32>( p.x, p.y, nDir > 0 ? 1 : 0 ) );
						m_gendata[p.x - 1 + p.y * nWidth] = eType_Temp0;
						m_gendata[p.x + p.y * nWidth] = eType_Temp1;
					}
					else if( m_gendata[p.x - 1 + p.y * nWidth] == eType_Temp0 && m_gendata[p.x + p.y * nWidth] == eType_Temp1 )
					{
						vec.push_back( TVector3<int32>( p.x, p.y, ( nDir > 0 ? 1 : 0 ) | 2 ) );
						m_gendata[p.x - 1 + p.y * nWidth] = eType_Temp2;
						m_gendata[p.x + p.y * nWidth] = eType_Temp3;
					}
				}
				if( nMin < h )
				{
					for( int i = -lMin[1]; i <= lMin[0]; i++ )
					{
						TVector2<int32> p( X2 + ( nMin - i ) * nDir, p0.y + nMin + i );
						vec.push_back( TVector3<int32>( p.x, p.y, ( nDir < 0 ? 1 : 0 ) | 2 ) );
						m_gendata[p.x - 1 + p.y * nWidth] = eType_Temp2;
						m_gendata[p.x + p.y * nWidth] = eType_Temp3;
					}
				}
			}
			if( nMin < h )
			{
				if( !lMin[0] )
				{
					TVector2<int32> p( nDir > 0 ? X1 : X2, p0.y + 1 );
					if( p.x >= 0 && p.x < nWidth && p.y >= 0 && p.y < nHeight && !m_gendata[p.x + p.y * nWidth] )
						vec2.push_back( TVector3<int32>( p.x, p.y, 1 ) );
				}
				if( !lMin[1] )
				{
					TVector2<int32> p( nDir > 0 ? X2 : X1, p0.y - 1 );
					if( p.x >= 0 && p.x < nWidth && p.y >= 0 && p.y < nHeight && !m_gendata[p.x + p.y * nWidth] )
						vec2.push_back( TVector3<int32>( p.x, p.y, -1 ) );
				}
			}
		}
	}

	for( int i = 0; i <= 2; i++ )
	{
		for( auto& rect : m_vecBar0[i] )
		{
			for( int j = 0; j < ( i > 0 ? 2 : 1 ); j++ )
			{
				int32 y = rect.y - 1 + j * 2;
				if( y < 0 || y >= nHeight )
					continue;
				for( int x = rect.x; x < rect.GetRight(); x++ )
				{
					if( !m_gendata[x + y * nWidth] || j == 1 && i > 0 && ( m_gendata[x + y * nWidth] == eType_Temp0 || m_gendata[x + y * nWidth] == eType_Temp1 ) )
					{
						m_gendata[x + y * nWidth] = eType_Wall1;
						m_vecBlocks[1 + j * 2].push_back( TVector2<int32>( x, y ) );
					}
				}
			}
		}
	}

	for( auto& p : vec )
	{
		if( !( p.z & 2 ) && m_gendata[p.x - 1 + p.y * nWidth] == eType_Temp0 && m_gendata[p.x + p.y * nWidth] == eType_Temp1 )
		{
			m_vecChunk1[p.z & 1].push_back( TRectangle<int32>( p.x - 1, p.y, 2, 1 ) );
			m_gendata[p.x - 1 + p.y * nWidth] = m_gendata[p.x + p.y * nWidth] = eType_Chunk;
		}
		else if( !!( p.z & 2 ) && m_gendata[p.x - 1 + p.y * nWidth] == eType_Temp2 && m_gendata[p.x + p.y * nWidth] == eType_Temp3 )
		{
			m_vecChunk2[p.z & 1].push_back( TRectangle<int32>( p.x - 1, p.y, 2, 1 ) );
			m_gendata[p.x - 1 + p.y * nWidth] = m_gendata[p.x + p.y * nWidth] = eType_Wall;
		}
	}

	for( auto& p : vec2 )
	{
		
	}
}

void CLevelGenNode2_3_1::GenBillboard( const TRectangle<int32>& rect )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	vector<TVector2<int32> > vec;
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( m_gendata[p.x + p.y * nWidth] != eType_Billboard )
			continue;
		auto r = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 4, 1 ), TVector2<int32>( Max( 4, SRand::Inst().Rand( rect.width / 2, rect.width ) ), 1 ),
			rect, -1, eType_Billboard_1 );
		if( r.width > 0 )
		{
			r.x--;
			r.width += 2;
			r.SetTop ( r.y - SRand::Inst().Rand( 1, 3 ) );
			r.height += SRand::Inst().Rand( 1, 3 );
			r = r * rect;
			for( int i = r.x; i < r.GetRight(); i++ )
			{
				for( int j = r.y; j < r.GetBottom(); j++ )
				{
					if( m_gendata[i + j * nWidth] == eType_Billboard )
						m_gendata[i + j * nWidth] = eType_Temp0;
				}
			}
		}
	}
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			if( m_gendata[i + j * nWidth] == eType_Temp0 )
				m_gendata[i + j * nWidth] = eType_Billboard;
		}
	}
}

int32 CLevelGenNode2_3_1::GenTrap( const TRectangle<int32>& bar, const TRectangle<int32>& r1, int8 nDir, int8 nDir1 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	if( !nDir1 )
	{
		int8 k0 = SRand::Inst().Rand( 0, 2 );
		for( int k = 0; k < 2; k++ )
		{
			if( ( k ^ k0 ) == 1 )
			{
				int32 nMaxOfs = bar.width - 12;
				if( nMaxOfs <= 0 )
					continue;
				int32 nOfs = SRand::Inst().Rand( 1, nMaxOfs + 1 );
				int32 w1 = SRand::Inst().Rand( 3, 5 );
				int32 h1 = SRand::Inst().Rand( 2, 4 );
				TRectangle<int32> rect( nDir ? bar.x + nOfs - w1 : bar.GetRight() - nOfs, bar.GetBottom(), w1, h1 );
				AddChunk( bar, eType_Obj, &m_vecBar0[nDir + 1] );
				AddChunk( rect, eType_Chunk, NULL );
				m_vecTempWorkshop1.push_back( pair<TRectangle<int32>, TVector2<int32> >( rect, TVector2<int32>( nDir ? rect.GetRight() - 1 : rect.x, rect.y ) ) );
				return nOfs;
			}
			else
			{
				if( r1.y - bar.GetBottom() < 7 )
					continue;
				int32 nOfs = Max( 0, SRand::Inst().Rand( 18, 22 ) - bar.width );
				int32 h1 = Min( r1.y - bar.GetBottom() - 5, SRand::Inst().Rand( 2, 5 ) );
				TRectangle<int32> r( nDir ? bar.x - 1 + nOfs : bar.GetRight() - 1 - nOfs, bar.GetBottom(), 2, 1 );
				for( int i = 0; i < h1; i++ )
				{
					AddChunk( r, eType_Chunk, &m_vecChunk1[1 - nDir] );
					r.y++;
					r.x += nDir ? -1 : 1;
				}
				int32 l = SRand::Inst().Rand( 2, h1 * 2 - 2 + 1 );
				if( nDir )
				{
					r.width += l;
					r.SetLeft( r.x - SRand::Inst().Rand( 2, l + 1 ) );
				}
				else
				{
					r.SetLeft( r.x - l );
					r.width += SRand::Inst().Rand( 2, l + 1 );
				}
				AddChunk( r, eType_Obj, &m_vecBar0[2 - nDir] );
				r = TRectangle<int32>( r.x, r.GetBottom(), r.width, r1.y - r.GetBottom() );
				r.height = SRand::Inst().Rand( 4, r.height + 1 );
				if( nDir )
					r.width = SRand::Inst().Rand( 5, r.width + 1 );
				else
					r.SetLeft( r.GetRight() - SRand::Inst().Rand( 5, r.width + 1 ) );
				AddChunk( r, eType_Billboard, &m_vecBillboards );
				AddChunk( TRectangle<int32>( nDir ? r.x + 1 : r.GetRight() - 4, r.y, 3, 3 ), eType_Billboard_2, NULL );
				AddChunk( TRectangle<int32>( r.x, r.GetBottom() - 1, r.width, 1 ), eType_Billboard_1, NULL );

				AddChunk( bar, eType_Obj, &m_vecBar0[2 - nDir] );
				return nOfs + 1;
			}
		}
	}
	else
	{
		int8 k0 = SRand::Inst().Rand( 0, 2 );
		for( int k = 0; k < 2; k++ )
		{
			if( ( k ^ k0 ) == 1 )
			{
				TVector2<int32> p( nDir ? bar.x - 1 : bar.GetRight(), bar.y - 1 );
				if( p.y < 0 || m_gendata[p.x + p.y * nWidth] )
					continue;
				auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 6, 1 ), TVector2<int32>( 6, 1 ),
					nDir ? TRectangle<int32>( 4, 0, bar.x - 4, nHeight ) : TRectangle<int32>( bar.GetRight(), 0, nWidth - 4 - bar.GetRight(), nHeight ), -1, 0 );
				if( rect.width <= 0 )
					continue;
				rect = PutRect( m_gendata, nWidth, nHeight, rect, TVector2<int32>( 6, 1 ), TVector2<int32>( SRand::Inst().Rand( 8, 13 ), 1 ),
					TRectangle<int32>( 4, 0, nWidth - 8, nHeight ), -1, 0, 0 );
				for( int i = SRand::Inst().Rand( 0, 4 ); i >= 0; i-- )
				{
					p.y--;
					if( p.y < 0 || m_gendata[p.x + p.y * nWidth] )
						continue;
					auto rect1 = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 6, 1 ), TVector2<int32>( 6, 1 ),
						nDir ? TRectangle<int32>( 4, 0, bar.x - 4, nHeight ) : TRectangle<int32>( bar.GetRight(), 0, nWidth - 4 - bar.GetRight(), nHeight ), -1, 0 );
					if( rect1.width <= 0 )
						continue;
					rect1 = PutRect( m_gendata, nWidth, nHeight, rect1, TVector2<int32>( 6, 1 ), TVector2<int32>( SRand::Inst().Rand( 8, 13 ), 1 ),
						TRectangle<int32>( 4, 0, nWidth - 8, nHeight ), -1, 0, 0 );
					if( nDir )
					{
						auto rect2 = PutRect( m_gendata, nWidth, nHeight, rect1, rect1.GetSize(), TVector2<int32>( nWidth, 1 ),
							TRectangle<int32>( rect1.x, 0, nWidth - rect1.x, nHeight ), -1, 0, 0 );
						rect1.width = Min( rect1.width, rect2.width - 2 );
					}
					else
					{
						auto rect2 = PutRect( m_gendata, nWidth, nHeight, rect1, rect1.GetSize(), TVector2<int32>( nWidth, 1 ),
							TRectangle<int32>( 0, 0, rect1.GetRight(), nHeight ), -1, 0, 0 );
						rect1.SetLeft( rect1.GetRight() - Min( rect1.width, rect2.width - 2 ) );
					}
					rect = rect1;
				}
				AddChunk( rect, eType_Obj, &m_vecBar0[0] );
				if( rect.width >= 8 )
				{
					int32 w = rect.width / 2;
					m_vecSawBlade[1].push_back( TRectangle<int32>( rect.x, rect.y, w, 1 ) );
					m_vecSawBlade[1].push_back( TRectangle<int32>( rect.GetRight() - w, rect.y, w, 1 ) );
				}
				else
					m_vecSawBlade[1].push_back( TRectangle<int32>( rect.x, rect.y, rect.width, 1 ) );
				AddChunk( bar, eType_Obj, &m_vecBar0[nDir + 1] );
				return 0;
			}
			else
			{
				TVector2<int32> p( nDir ? bar.x - 2 : bar.GetRight() + 1, bar.y );
				if( m_gendata[p.x + p.y * nWidth] )
					continue;
				auto rect = PutRect( m_gendata, nWidth, nHeight, p, TVector2<int32>( 3, 3 ), TVector2<int32>( SRand::Inst().Rand( 4, 6 ), SRand::Inst().Rand( 3, 5 ) ),
					nDir ? TRectangle<int32>( 4, 0, p.x + 1 - 4, p.y + 2 ) : TRectangle<int32>( p.x, 0, nWidth - 4 - p.x, p.y + 2 ), -1, 0 );
				if( rect.width <= 0 )
					continue;
				int32 nMaxOfs = Min( r1.width - ( bar.width + 1 ), bar.width - 10 );
				if( nMaxOfs <= 1 )
					continue;
				AddChunk( rect, eType_Chunk, &m_vecBuildings );
				int32 nMinOfs = r1.width - ( bar.width + rect.width - 1 );
				int32 nOfs = SRand::Inst().Rand( nMinOfs, nMaxOfs + 1 );
				if( nDir )
					m_vecSawBlade[0].push_back( TRectangle<int32>( rect.GetRight() - 1, rect.y, 1, rect.height ) );
				else
					m_vecSawBlade[2].push_back( TRectangle<int32>( rect.x, rect.y, 1, rect.height ) );
				AddChunk( bar, eType_Obj, &m_vecBar0[nDir + 1] );

				int32 x = nDir ? r1.x + nOfs - 1 : r1.GetRight() - nOfs + 1;
				TRectangle<int32> rect1( x - 1, rect.GetBottom(), 2, r1.y - rect.GetBottom() );
				auto bound = rect1;
				if( nDir )
					bound.SetLeft( 0 );
				else
					bound.SetRight( nWidth );
				rect1 = PutRect( m_gendata, nWidth, nHeight, rect1, TVector2<int32>( 4, rect1.height ), TVector2<int32>( 16, rect1.height ), bound, -1, 0, 0 );
				if( rect1.width > 0 )
					GenWorkshop( rect1, TVector2<int32>( nDir ? rect1.GetRight() - 1 : rect1.x, rect1.y ) );

				return nOfs;
			}
		}
	}
	AddChunk( bar, eType_Obj, &m_vecBar0[nDir + 1] );
	return 0;
}

void CLevelGenNode2_3_1::GenWorkshop( const TRectangle<int32>& r, const TVector2<int32>& p0 )
{
	int8 bVertical;
	int8 n;
	float fValue[2];
	for( int k = 0; k < 2; k++ )
	{
		int32 l = ( k == 0 ? r.width : r.height ) - 3;
		if( k == 0 )
		{
			if( p0.x >= r.x + 4 && p0.x < r.GetRight() - 4 )
				l /= 2;
		}
		else
		{
			if( p0.y >= r.y + 4 && p0.y < r.GetBottom() - 4 )
				l /= 2;
		}
		fValue[k] = l + SRand::Inst().Rand( 0.0f, 0.1f );
	}
	bVertical = fValue[0] < fValue[1] ? 1 : 0;
	int32 l = bVertical == 0 ? r.width : r.height;
	int8 nSplit = r.width / 5;
	if( bVertical == 0 )
	{
		if( p0.x >= r.x + 4 && p0.x < r.GetRight() - 4 )
			n = 0;
		else if( p0.x < ( r.x + r.GetRight() + SRand::Inst().Rand( 0, 2 ) ) / 2 )
			n = -1;
		else
			n = 1;
		float f = FLT_MAX;
		int32 nLen = -1;
		for( int iLen = 3; iLen <= 5; iLen++ )
		{
			float f1 = iLen + r.height % iLen + Min( 2, abs( r.height / iLen - 3 ) );
			if( f1 < f )
			{
				f = f1;
				nLen = iLen;
			}
		}
		int32 nSplit = r.height / nLen;
		int32 nOfs0 = p0.y == r.y ? 0 : ( p0.y == r.GetBottom() - 1 ? r.height % nLen : SRand::Inst().Rand( 0, r.height % nLen + 1 ) );
		int32 nOfs1 = Min( r.height % nLen, Max( 0, nOfs0 + SRand::Inst().Rand( 0, 2 ) ) );
		int32 nOfs2 = Min( r.height % nLen, Max( 0, nOfs0 + SRand::Inst().Rand( 0, 2 ) ) );

		int32 w0 = Max( 3, Min( nLen, Min( r.width - ( n == 0 ? 12 : 6 ), SRand::Inst().Rand( 3, 5 ) ) ) );
		TRectangle<int32> rect0( r.x, r.y + nOfs0, w0, nLen );
		if( n == 0 )
			rect0.x += ( r.width - rect0.width + SRand::Inst().Rand( 0, 2 ) ) / 2;
		else if( n == 1 )
			rect0.x = r.GetRight() - rect0.width;
		int32 rand0 = SRand::Inst().Rand( 0, 2 );
		for( int i = 0; i < nSplit; i++ )
		{
			auto rect = rect0;
			rect.y += nLen * i;
			if( rect.height >= 4 )
			{
				rect.height--;
				if( rand0 )
					rect.y++;
			}
			AddChunk( rect, eType_Cargo2, &m_vecCargos2 );
			if( p0.x == r.x )
			{
				AddChunk( TRectangle<int32>( rect.x, rect.y, 1, rect.height ), eType_Cargo2_4, NULL );
				AddChunk( TRectangle<int32>( rect.x + 1, rect.GetBottom() - 2, rect.width - 1, 2 ), eType_Cargo2_1, NULL );
			}
			else if( p0.x == r.GetRight() - 1 )
			{
				AddChunk( TRectangle<int32>( rect.GetRight() - 1, rect.y, 1, rect.height ), eType_Cargo2_4, NULL );
				AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 2, rect.width - 1, 2 ), eType_Cargo2_1, NULL );
			}
			else if( p0.y == r.y )
			{
				AddChunk( TRectangle<int32>( rect.x, rect.y, rect.width, 1 ), eType_Cargo2_4, NULL );
				AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 2, rect.width - 1, 2 ), eType_Cargo2_1, NULL );
			}
		}
		for( int k = -1; k < 2; k += 2 )
		{
			if( k + n != 0 )
				continue;
			TRectangle<int32> rect1;
			if( k == -1 )
				rect1 = TRectangle<int32>( r.x, r.y + ( k == -1 ? nOfs1 : nOfs2 ), rect0.x - r.x, nLen );
			else
				rect1 = TRectangle<int32>( rect0.GetRight(), r.y + ( k == -1 ? nOfs1 : nOfs2 ), r.GetRight() - rect0.GetRight(), nLen );
			if( rect1.width >= SRand::Inst().Rand( 6, 10 ) )
			{
				rect1.width--;
				if( k == 1 )
					rect1.x++;
			}
			GenWorkshopRooms( rect1, TVector2<int32>( 0, nLen ), k == 1 ? 0 : 2, nSplit );
		}
	}
	else
	{
		if( p0.y >= r.y + 4 && p0.y < r.GetBottom() - 4 )
			n = 0;
		else if( p0.y < ( r.y + r.GetBottom() + SRand::Inst().Rand( 0, 2 ) ) / 2 )
			n = -1;
		else
			n = 1;

		float f = FLT_MAX;
		int32 nLen = -1;
		for( int iLen = 3; iLen <= 5; iLen++ )
		{
			float f1 = iLen + r.width % iLen + Min( 2, abs( r.width / iLen - 3 ) );
			if( f1 < f )
			{
				f = f1;
				nLen = iLen;
			}
		}
		int32 nSplit = r.width / nLen;
		int32 nOfs0 = p0.x == r.x ? 0 : ( p0.x == r.GetRight() - 1 ? r.width % nLen : SRand::Inst().Rand( 0, r.width % nLen + 1 ) );
		int32 nOfs1 = Min( r.width % nLen, Max( 0, nOfs0 + SRand::Inst().Rand( 0, 2 ) ) );
		int32 nOfs2 = Min( r.width % nLen, Max( 0, nOfs0 + SRand::Inst().Rand( 0, 2 ) ) );

		int32 h0 = Max( 3, Min( nLen, Min( r.height - ( n == 0 ? 12 : 6 ), SRand::Inst().Rand( 3, 5 ) ) ) );
		TRectangle<int32> rect0( r.x + nOfs0, r.y, nLen, h0 );
		if( n == 0 )
			rect0.y += ( r.height - rect0.height + SRand::Inst().Rand( 0, 2 ) ) / 2;
		else if( n == 1 )
			rect0.y = r.GetBottom() - rect0.height;
		int32 rand0 = SRand::Inst().Rand( 0, 2 );
		for( int i = 0; i < nSplit; i++ )
		{
			auto rect = rect0;
			rect.x += nLen * i;
			if( rect.width >= 4 )
			{
				rect.width--;
				if( rand0 )
					rect.x++;
			}
			AddChunk( rect, eType_Cargo2, &m_vecCargos2 );
			if( p0.x == r.x )
			{
				AddChunk( TRectangle<int32>( rect.x, rect.y, 1, rect.height ), eType_Cargo2_4, NULL );
				AddChunk( TRectangle<int32>( rect.x + 1, rect.GetBottom() - 2, rect.width - 1, 2 ), eType_Cargo2_1, NULL );
			}
			else if( p0.x == r.GetRight() - 1 )
			{
				AddChunk( TRectangle<int32>( rect.GetRight() - 1, rect.y, 1, rect.height ), eType_Cargo2_4, NULL );
				AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 2, rect.width - 1, 2 ), eType_Cargo2_1, NULL );
			}
			else if( p0.y == r.y )
			{
				AddChunk( TRectangle<int32>( rect.x, rect.y, rect.width, 1 ), eType_Cargo2_4, NULL );
				AddChunk( TRectangle<int32>( rect.x, rect.GetBottom() - 2, rect.width - 1, 2 ), eType_Cargo2_1, NULL );
			}
		}
		for( int k = -1; k < 2; k += 2 )
		{
			if( k + n != 0 )
				continue;
			TRectangle<int32> rect1;
			if( k == -1 )
				rect1 = TRectangle<int32>( r.x + ( k == -1 ? nOfs1 : nOfs2 ), r.y, nLen, rect0.y - r.y );
			else
				rect1 = TRectangle<int32>( r.x + ( k == -1 ? nOfs1 : nOfs2 ), rect0.GetBottom(), nLen, r.GetBottom() - rect0.GetBottom() );
			if( rect1.height >= SRand::Inst().Rand( 6, 10 ) )
			{
				rect1.height--;
				if( k == 1 )
					rect1.y++;
			}
			GenWorkshopRooms( rect1, TVector2<int32>( nLen, 0 ), k == 1 ? 1 : 3, nSplit );
		}
	}
}

void CLevelGenNode2_3_1::GenWorkshop1( const TRectangle<int32>& r, const TVector2<int32>& p0 )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	int32 nDir = p0.x == r.x ? -1 : 1;
	int32 nDir1 = p0.y == r.y ? 1 : -1;
	AddChunk( r, 0, NULL );

	auto Check = [=] ( const TRectangle<int32>& r )
	{
		if( r.x < 0 || r.y < 0 || r.GetRight() > nWidth || r.GetBottom() > nHeight )
			return false;
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			for( int j = r.y; j < r.GetBottom(); j++ )
			{
				if( m_gendata[i + j * nWidth] )
					return false;
			}
		}
		return true;
	};
	int32 nMaxValue = 0;
	TVector3<int32> vMax;
	for( int nOfsX = 1; nOfsX < r.width; nOfsX++ )
	{
		TVector2<int32> ofs( nOfsX * nDir, Max( 3, r.height ) * nDir1 );
		int32 nMaxCount;
		for( nMaxCount = 0; ; nMaxCount++ )
		{
			auto r1 = r.Offset( ofs * nMaxCount );
			if( !Check( r1 ) )
				break;
		}
		int32 l = 0;
		for( int nCount = nMaxCount; nCount >= 1; nCount-- )
		{
			for( ; l < 12; l++ )
			{
				TRectangle<int32> r0( nDir == 1 ? r.x + nOfsX - 1 - l : r.GetRight() - nOfsX + l, r.y + nDir1 * r.height, 1, r.height );
				if( nDir1 == 1 )
					r0.height = Max( 3, r0.height );
				else
					r0.SetTop( r0.GetBottom() - Max( 3, r0.height ) );
				bool b = false;
				for( int i = 0; i < nCount; i++ )
				{
					auto r1 = r0.Offset( ofs * i );
					if( !Check( r1 ) )
					{
						b = true;
						break;
					}
				}
				if( b )
					break;
			}

			int32 nValue = nCount * ( l - 4 ) * ( r.width + abs( nOfsX - r.width / 2 ) );
			if( nValue > nMaxValue )
			{
				nMaxValue = nValue;
				vMax = TVector3<int32>( nCount, l, nOfsX );
			}
		}
	}
	if( !nMaxValue )
		return;

	int32 nCount = vMax.x;
	int32 l = vMax.y;
	int32 nOfsX = vMax.z;
	TVector2<int32> ofs( nOfsX * nDir, Max( 3, r.height ) * nDir1 );
	TRectangle<int32> r0( nDir == 1 ? r.x + nOfsX - l : r.GetRight() - nOfsX, r.y + nDir1 * r.height, l, r.height );
	auto rr = r;
	rr.width--;
	if( nDir != 1 )
		rr.x++;
	int32 w1 = Min( rr.width, rr.height );
	rr = TRectangle<int32>( rr.x + SRand::Inst().Rand( 0, rr.width - w1 + 1 ), rr.y, w1, rr.height );
	for( int i = 0; i < nCount; i++ )
	{
		auto r1 = r.Offset( ofs * i );
		AddChunk( r1, eType_Cargo2, &m_vecCargos2 );
		auto rectSawblade = nDir == 1 ? TRectangle<int32>( r1.GetRight() - 1, r1.y, 1, r1.height ) : TRectangle<int32>( r1.x, r1.y, 1, r1.height );
		m_vecSawBlade[nDir == 1 ? 0 : 2].push_back( rectSawblade );
		AddChunk( rectSawblade, eType_Cargo2_4, NULL );
		AddChunk( rr, eType_Cargo2_2, NULL );
	}
	GenWorkshopRooms( r0, ofs, nDir != 1 ? 0 : 2, nCount );
}

void CLevelGenNode2_3_1::GenWorkshopRooms( const TRectangle<int32>& r, const TVector2<int32>& ofs, int8 nDir, int32 nCount )
{
	int32 nWidth = m_region.width;
	int32 nHeight = m_region.height;
	auto r0 = r;
	if( !!( nDir & 1 ) )
		swap( r0.width, r0.height );

	auto Func = [=, &r, &ofs] ( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec )
	{
		TRectangle<int32> r1 = rect.Offset( TVector2<int32>( -r.x, -r.y ) );
		if( !!( nDir & 1 ) )
		{
			swap( r1.x, r1.y );
			swap( r1.width, r1.height );
		}
		if( nDir == 2 )
			r1.x = r.width - r1.x - r1.width;
		if( nDir == 3 )
			r1.y = r.height - r1.y - r1.height;
		r1 = r1.Offset( TVector2<int32>( r.x, r.y ) );
		for( int i = 0; i < nCount; i++ )
		{
			AddChunk( r1, nType, pVec );
			r1 = r1.Offset( ofs );
		}
	};

	Func( r0, eType_Room_2, &m_vecRooms );
	if( r0.width >= 4 && SRand::Inst().Rand( 0, 2 ) )
	{
		int8 k1 = SRand::Inst().Rand( 0, 2 );
		auto rect = TRectangle<int32>( r0.x, k1 == 1 ? r0.y : r0.GetBottom() - 3, 4, 3 );
		int32 x1 = rect.x + 1;
		auto rect1 = TRectangle<int32>( r0.x + 4, k1 == 1 ? r0.y : r0.GetBottom() - 1, r0.width - 4, 1 );
		Func( rect, eType_Cargo1, &m_vecCargos1 );
		Func( TRectangle<int32>( x1, k1 == 1 ? rect.GetBottom() - 1 : rect.y, 2, 1 ), eType_Cargo1_2, NULL );
		Func( TRectangle<int32>( x1, k1 != 1 ? rect.GetBottom() - 2 : rect.y, 2, 2 ), eType_Cargo1_6, NULL );
		if( rect1.width > 1 )
			Func( rect1, eType_Room_1, &m_vecBar_a[0] );
	}
	else
	{
		auto rect = TRectangle<int32>( r0.x, r0.y, 3, r0.height );
		int32 y1 = rect.y + ( rect.height - 2 + SRand::Inst().Rand( 0, 2 ) ) / 2;
		auto rect1 = TRectangle<int32>( r0.x + 3, r0.y, r0.width - 3, 1 );
		if( SRand::Inst().Rand( 0, 2 ) )
			rect1.y = r0.GetBottom() - 1;
		Func( rect, eType_Cargo1, &m_vecCargos1 );
		Func( TRectangle<int32>( rect.GetRight() - 1, y1, 1, 2 ), eType_Cargo1_2, NULL );
		Func( TRectangle<int32>( rect.x, y1, 2, 2 ), eType_Cargo1_6, NULL );
		if( rect1.width > 1 )
			Func( rect1, eType_Room_1, &m_vecBar_a[0] );
	}
}

void CLevelGenNode2_3_1::AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32>>* pVec )
{
	if( pVec )
		pVec->push_back( rect );
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			m_gendata[i + j * m_region.width] = nType;
		}
	}
}
