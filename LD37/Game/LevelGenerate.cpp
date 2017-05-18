#include "stdafx.h"
#include "LevelGenerate.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "GUI/MainUI.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Entities/Decorator.h"

#include "LevelGenerating/LvGenCommon.h"
#include "LevelGenerating/LvGen1.h"

SLevelBuildContext::SLevelBuildContext( CMyLevel* pLevel, SChunk* pParentChunk ) : pLevel( pLevel ), pParentChunk( pParentChunk )
{
	nWidth = pParentChunk ? pParentChunk->nWidth : pLevel->m_nWidth;
	nHeight = pParentChunk ? pParentChunk->nHeight : pLevel->m_nHeight;
	nBlockSize = CMyLevel::GetBlockSize();
	blueprint.resize( nWidth * nHeight );
	blocks.resize( nWidth * nHeight * 2 );
	for( int i = 0; i < ELEM_COUNT( attachedPrefabs ); i++ )
		attachedPrefabs[i].resize( nWidth * nHeight * 2 );
}

SLevelBuildContext::SLevelBuildContext( uint32 nWidth, uint32 nHeight ) : pLevel( NULL ), pParentChunk( NULL ), nWidth( nWidth ), nHeight( nHeight ), nBlockSize( 32 )
{
	blueprint.resize( nWidth * nHeight );
	blocks.resize( nWidth * nHeight * 2 );
	for( int i = 0; i < ELEM_COUNT( attachedPrefabs ); i++ )
		attachedPrefabs[i].resize( nWidth * nHeight * 2 );
}

SChunk* SLevelBuildContext::CreateChunk( SChunkBaseInfo& baseInfo, const TRectangle<int32>& region )
{
	if( region.x < 0 || region.GetRight() > nWidth )
		return NULL;
	if( region.y < 0 || region.GetBottom() > nHeight )
		return NULL;
	for( int iLayer = 0; iLayer < 2; iLayer++ )
	{
		if( !baseInfo.HasLayer( iLayer ) )
			continue;
		if( pParentChunk && !pParentChunk->HasLayer( iLayer ) )
			return NULL;
		for( int j = region.y; j < region.GetBottom(); j++ )
		{
			for( int i = region.x; i < region.GetRight(); i++ )
			{
				if( GetBlock( i, j, iLayer ) )
					return NULL;
			}
		}
	}

	auto pChunk = new SChunk( baseInfo, TVector2<int32>( region.x * nBlockSize, region.y * nBlockSize ), region.GetSize() );
	chunks.push_back( pChunk );
	for( int iLayer = 0; iLayer < 2; iLayer++ )
	{
		if( !baseInfo.HasLayer( iLayer ) )
			continue;
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				GetBlock( i + region.x, j + region.y, iLayer ) = pChunk->blocks[i + j * pChunk->nWidth].layers + iLayer;
			}
		}
	}

	return pChunk;
}

void SLevelBuildContext::AttachPrefab( CPrefab* pPrefab, TRectangle<int32> rect, uint8 nLayer, uint8 nType )
{
	auto r = rect;
	if( nType > 0 )
		r.width = r.height = 1;
	for( int j = r.y; j < r.GetBottom(); j++ )
	{
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			if( attachedPrefabs[nType][nLayer + ( i + j * nWidth ) * 2].first )
				return;
		}
	}

	for( int j = r.y; j < r.GetBottom(); j++ )
	{
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			attachedPrefabs[nType][nLayer + ( i + j * nWidth ) * 2].first = pPrefab;
			attachedPrefabs[nType][nLayer + ( i + j * nWidth ) * 2].second = rect;
		}
	}
}

void SLevelBuildContext::AddSpawnInfo( SChunkSpawnInfo * pInfo, const TVector2<int32> ofs )
{
	auto pBlock = GetBlock( ofs.x, ofs.y, 1 );
	if( !pBlock )
		pBlock = GetBlock( ofs.x, ofs.y, 0 );
	if( !pBlock )
	{
		delete pInfo;
		return;
	}

	pInfo->pos = pInfo->pos + CVector2( pBlock->pParent->nX * nBlockSize, pBlock->pParent->nY * nBlockSize );

	pBlock->pParent->pOwner->Insert_SpawnInfo( pInfo );
}

void SLevelBuildContext::Build()
{
	if( pParentChunk )
	{
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			if( !pParentChunk->HasLayer( iLayer ) )
				continue;

			for( int j = 0; j < nHeight; j++ )
			{
				for( int i = 0; i < nWidth; i++ )
				{
					auto pBlock = GetBlock( i, j, iLayer );
					if( pBlock )
					{
						if( pBlock->pParent->pOwner->nSubChunkType == 1 )
						{
							pParentChunk->GetBlock( i, j )->eBlockType = pBlock->pParent->eBlockType;
							pParentChunk->GetBlock( i, j )->fDmgPercent = pBlock->pParent->fDmgPercent;
						}
						if( pBlock->pParent->nX == 0 && pBlock->pParent->nY == 0 && iLayer == pBlock->pParent->pOwner->GetMinLayer() && pBlock->pParent->pOwner->pPrefab )
						{
							pParentChunk->Insert_SubChunk( pBlock->pParent->pOwner );
						}
					}
				}
			}
		}
	}
	else
	{
		vector<SBlockLayer*> pPreBlocks[2];
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			pPreBlocks[iLayer].resize( nWidth );
		}

		SChunk* pLevelBarrier = NULL;
		if( pLevel )
		{
			for( auto pBlock = pLevel->m_basements[0].layers[0].Get_BlockLayer(); pBlock; pBlock = pBlock->NextBlockLayer() )
			{
				if( pBlock->pParent->pOwner->bIsLevelBarrier )
				{
					pLevelBarrier = pBlock->pParent->pOwner;
					break;
				}
			}
		}
		int32 nYOfs = pLevelBarrier ? pLevelBarrier->pos.y + pLevelBarrier->nHeight * nBlockSize : 0;
		int32 nGridOfs = nYOfs / nBlockSize;
		if( pLevelBarrier )
		{
			for( int iLayer = 0; iLayer < 2; iLayer++ )
			{
				for( int i = 0; i < nWidth; i++ )
				{
					pPreBlocks[iLayer][i] = pLevelBarrier->GetBlock( i, 0 )->layers + iLayer;
				}
			}
		}

		auto pMainUI = CMainUI::GetInst();

		for( int j = 0; j < nHeight; j++ )
		{
			for( int i = 0; i < nWidth; i++ )
			{
				for( int iLayer = 0; iLayer < 2; iLayer++ )
				{
					auto pBlockLayer = GetBlock( i, j, iLayer );
					if( pBlockLayer )
					{
						auto pBlock = pBlockLayer->pParent;
						if( !pBlock->pOwner->pPrefab )
							continue;
						if( pMainUI )
							pMainUI->UpdateMinimap( i, j + nGridOfs, iLayer, CMyLevel::s_nTypes[pBlock->eBlockType] );

						if( pBlock->nY == 0 )
						{
							if( pBlock->nX == 0 && iLayer == pBlock->pOwner->GetMinLayer() )
								pBlock->pOwner->pos.y += nYOfs;
							uint32 nBasement = i;
							auto pPreBlock = pPreBlocks[iLayer][nBasement];
							if( pPreBlock )
							{
								pPreBlock->InsertAfter_BlockLayer( pBlockLayer );
							}
							else if( pLevel )
							{
								pLevel->m_basements[nBasement].layers[iLayer].Insert_BlockLayer( pBlockLayer );
							}
							pPreBlocks[iLayer][nBasement] = pBlockLayer;
						}
					}
				}
			}
		}
	}

	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			SBlockLayer* pBlockLayers[2] = { GetBlock( i, j, 0 ), GetBlock( i, j, 1 ) };
			if( pParentChunk )
			{
				for( int iLayer = 0; iLayer < 2; iLayer++ )
				{
					if( !pParentChunk->HasLayer( iLayer ) )
						pBlockLayers[iLayer] = NULL;
				}
			}

			if( pBlockLayers[0] || pBlockLayers[1] )
			{
				bool bLayerValid[2];
				if( pBlockLayers[1] && pBlockLayers[0] )
				{
					if( pBlockLayers[1]->pParent == pBlockLayers[0]->pParent )
					{
						bLayerValid[0] = false;
						bLayerValid[1] = true;
					}
					else
						bLayerValid[0] = bLayerValid[1] = true;
				}
				else if( pBlockLayers[1] )
				{
					bLayerValid[0] = false;
					bLayerValid[1] = true;
				}
				else
				{
					bLayerValid[0] = true;
					bLayerValid[1] = false;
				}

				for( int k = 0; k < ELEM_COUNT( attachedPrefabs ); k++ )
				{
					pair<CReference<CPrefab>, TRectangle<int32> > attachedPrefabLayers[2] = { attachedPrefabs[k][0 + ( i + j * nWidth ) * 2], attachedPrefabs[k][1 + ( i + j * nWidth ) * 2] };

					if( bLayerValid[0] && !bLayerValid[1] && !attachedPrefabLayers[0].first && attachedPrefabLayers[1].first )
					{
						attachedPrefabLayers[0] = attachedPrefabLayers[1];
						attachedPrefabLayers[1].first = NULL;
					}

					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						if( !bLayerValid[iLayer] )
							continue;

						if( k == SBlock::eAttachedPrefab_Lower )
						{
							if( j <= 0 )
								continue;
							auto pLowerBlock = GetBlock( i, j - 1, iLayer );
							if( pLowerBlock && pLowerBlock->pParent->eBlockType != eBlockType_Wall )
								continue;
						}
						if( k == SBlock::eAttachedPrefab_Upper )
						{
							if( j >= nHeight - 1 )
								continue;
							auto pUpperBlock = GetBlock( i, j + 1, iLayer );
							if( pUpperBlock && pUpperBlock->pParent->eBlockType != eBlockType_Wall )
								continue;
						}

						auto& attachedPrefab = attachedPrefabLayers[iLayer];
						auto pBlock = pBlockLayers[iLayer]->pParent;
						if( attachedPrefab.second.x == i && attachedPrefab.second.y == j )
						{
							if( !pBlock->pOwner->pPrefab )
							{
								if( !pParentChunk )
									continue;
								pBlock = pParentChunk->GetBlock( i, j );
							}

							pBlock->pAttachedPrefab[k] = attachedPrefab.first;
							if( k == SBlock::eAttachedPrefab_Center )
								pBlock->attachedPrefabSize = attachedPrefab.second.GetSize();
							else if( k == SBlock::eAttachedPrefab_Lower )
								pBlock->nLowerMargin = attachedPrefab.second.height;
							else
								pBlock->nUpperMargin = attachedPrefab.second.height;
						}
					}
				}
			}
		}
	}

	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			for( int iLayer = 0; iLayer < 2; iLayer++ )
			{
				auto pBlock = GetBlock( i, j, iLayer );
				if( pBlock && !pBlock->pParent->pOwner->pPrefab && pBlock->pParent->nX == pBlock->pParent->pOwner->nWidth - 1 && pBlock->pParent->nY == pBlock->pParent->pOwner->nHeight - 1
					&& iLayer == pBlock->pParent->pOwner->GetMaxLayer() )
				{
					delete pBlock->pParent->pOwner;
				}
			}
		}
	}
}

SBlockLayer*& SLevelBuildContext::GetBlock( uint32 x, uint32 y, uint32 z )
{
	return blocks[z + ( x + y * nWidth ) * 2];
}

void CLevelGenerateNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	m_nFillBlueprint = XmlGetAttr( pXml, "fill_blueprint", -1 );
	auto pMetadata = pXml->FirstChildElement( "metadata" );
	if( pMetadata )
	{
		m_metadata.bIsDesignValid = XmlGetAttr<int32>( pMetadata, "isdesignvalid", m_metadata.bIsDesignValid );
		m_metadata.minSize.x = XmlGetAttr<int32>( pMetadata, "minx", m_metadata.minSize.x );
		m_metadata.minSize.y = XmlGetAttr<int32>( pMetadata, "miny", m_metadata.minSize.y );
		m_metadata.maxSize.x = XmlGetAttr<int32>( pMetadata, "maxx", m_metadata.maxSize.x );
		m_metadata.maxSize.y = XmlGetAttr<int32>( pMetadata, "maxy", m_metadata.maxSize.y );
		m_metadata.nMinLevel = XmlGetAttr<int32>( pMetadata, "minlevel", m_metadata.nMinLevel );
		m_metadata.nMaxLevel = XmlGetAttr<int32>( pMetadata, "maxlevel", m_metadata.nMaxLevel );
	}
	if( m_metadata.maxSize == m_metadata.minSize )
		m_metadata.nEditType = eEditType_Brush;
	else
		m_metadata.nEditType = eEditType_Fence;
	
	auto pNextLevel = pXml->FirstChildElement( "next_level" );
	if( pNextLevel )
	{
		m_pNextLevel = CreateNode( pNextLevel->FirstChildElement(), context );
		m_fNextLevelChance = XmlGetAttr( pNextLevel, "p", 1.0f );
	}
}

void CLevelGenerateNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	if( m_nFillBlueprint >= 0 )
	{
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			for( int j = region.y; j < region.GetBottom(); j++ )
			{
				context.blueprint[i + j * context.nWidth] = m_nFillBlueprint;
			}
		}
	}
	if( m_pNextLevel )
	{
		if( SRand::Inst().Rand( 0.0f, 1.0f ) < m_fNextLevelChance )
			m_pNextLevel->Generate( context, region );
	}
}

#define GET_GRID( g, i, j ) ( g[(i) + (j) * size.x] )

void CLevelGenerateSimpleNode::Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context )
{
	auto& chunk = context.pCurFileContext->mapChunkBaseInfo[XmlGetAttr( pXml, "name", "" )];
	m_pChunkBaseInfo = &chunk;
	chunk.nWidth = XmlGetAttr( pXml, "width", 1 );
	chunk.nHeight = XmlGetAttr( pXml, "height", 1 );
	chunk.fWeight = XmlGetAttr( pXml, "weight", 0.0f );
	chunk.fDestroyWeight = XmlGetAttr( pXml, "destroyweight", 0.0f );
	chunk.fDestroyBalance = XmlGetAttr( pXml, "destroybalance", 0.0f );
	chunk.fImbalanceTime = XmlGetAttr( pXml, "imbalancetime", 0.0f );
	chunk.fShakeDmg = XmlGetAttr( pXml, "shakedmg", 1 );
	chunk.nAbsorbShakeStrength = XmlGetAttr( pXml, "absorbshakestrength", 1 );
	chunk.nDestroyShake = XmlGetAttr( pXml, "destroyshake", 1 );
	chunk.nShakeDmgThreshold = XmlGetAttr( pXml, "shakedmgthreshold", 1 );
	chunk.fWeightPerWidth = XmlGetAttr( pXml, "weightperwidth", 0.0f );
	chunk.fDestroyWeightPerWidth = XmlGetAttr( pXml, "destroyweightperwidth", 0.0f );
	chunk.fAbsorbShakeStrengthPerHeight = XmlGetAttr( pXml, "absorbshakestrengthperheight", 1 );
	m_bIsLevelBarrier = XmlGetAttr( pXml, "islevelbarrier", 0 );
	m_nLevelBarrierHeight = XmlGetAttr( pXml, "levelbarrierheight", 0 );
	chunk.nLayerType = XmlGetAttr( pXml, "layer_type", 3 );
	chunk.bIsRoom = XmlGetAttr( pXml, "isroom", 0 );
	chunk.nSubChunkType = XmlGetAttr( pXml, "subchunk_type", 0 );
	chunk.pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( XmlGetAttr( pXml, "prefab", "" ) );
	chunk.blockInfos.resize( chunk.nWidth * chunk.nHeight );

	const char* szType = XmlGetValue( pXml, "types", "" );
	const char* c = szType;
	int i = 0;
	while( c && i < chunk.nWidth * chunk.nHeight )
	{
		if( *c >= '0' && *c <= '9' )
		{
			int y = i / chunk.nWidth;
			int x = i % chunk.nWidth;
			auto& blockInfo = chunk.blockInfos[x + ( chunk.nHeight - y - 1 ) * chunk.nWidth];
			blockInfo.eBlockType = (EBlockType)( *c - '0' );
			blockInfo.fDmgPercent = 1;
			i++;
		}
		c++;
	}

	auto pSubItem = pXml->FirstChildElement( "subitem" );
	if( pSubItem && pSubItem->FirstChildElement() )
	{
		m_pSubChunk = CreateNode( pSubItem->FirstChildElement(), context );
		m_bCopyBlueprint = XmlGetAttr( pSubItem, "copy_blueprint", 0 );
	}

	if( chunk.pPrefab )
	{
		m_metadata.bIsDesignValid = true;
		m_metadata.maxSize = m_metadata.minSize = TVector2<int32>( chunk.nWidth, chunk.nHeight );
		switch( chunk.nLayerType )
		{
		case 1:
			m_metadata.nMinLevel = 0;
			m_metadata.nMaxLevel = 0;
			break;
		case 2:
			m_metadata.nMinLevel = 1;
			m_metadata.nMaxLevel = 1;
			break;
		case 3:
			m_metadata.nMinLevel = 0;
			m_metadata.nMaxLevel = 1;
			break;
		}
	}
	CLevelGenerateNode::Load( pXml, context );
}

void CLevelGenerateSimpleNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;

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
		CLevelGenerateNode::Generate( context, region );
	}
}

class CLevelGenerateSpawnNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( XmlGetAttr( pXml, "prefab", "" ) );
		m_rectSize.x = XmlGetAttr( pXml, "sizex", 0.0f );
		m_rectSize.y = XmlGetAttr( pXml, "sizey", 0.0f );
		m_rectSize.width = XmlGetAttr( pXml, "sizewidth", 0.0f );
		m_rectSize.height = XmlGetAttr( pXml, "sizeheight", 0.0f );
		m_fCountPerGrid = XmlGetAttr( pXml, "countpergrid", 1.0f );

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		CRectangle rect( 0, 0, region.width, region.height );
		rect = rect * context.nBlockSize;
		rect.SetLeft( rect.x - m_rectSize.x );
		rect.SetTop( rect.y - m_rectSize.y );
		rect.SetRight( rect.GetRight() - m_rectSize.GetRight() );
		rect.SetBottom( rect.GetBottom() - m_rectSize.GetBottom() );

		if( m_pPrefab->GetRoot()->GetStaticDataSafe<CDecorator>() )
		{
			SChunkSpawnInfo* pSpawnInfo = new SChunkSpawnInfo;
			pSpawnInfo->pos = CVector2( 0, 0 );
			pSpawnInfo->pPrefab = m_pPrefab;
			pSpawnInfo->r = 0;
			context.AddSpawnInfo( pSpawnInfo, TVector2<int32>( region.x, region.y ) );
		}
		else
		{
			uint32 nCount = region.width * region.height * m_fCountPerGrid;

			for( int i = 0; i < nCount; i++ )
			{
				SChunkSpawnInfo* pSpawnInfo = new SChunkSpawnInfo;
				pSpawnInfo->pos = CVector2( SRand::Inst().Rand( rect.GetLeft(), rect.GetRight() ), SRand::Inst().Rand( rect.GetTop(), rect.GetBottom() ) );
				pSpawnInfo->pPrefab = m_pPrefab;
				pSpawnInfo->r = 0;
				context.AddSpawnInfo( pSpawnInfo, TVector2<int32>( region.x, region.y ) );
			}
		}
	}
protected:
	CReference<CPrefab> m_pPrefab;
	CRectangle m_rectSize;
	float m_fCountPerGrid;
};

class CLevelGenerateAttachNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		const char* szPrefab = XmlGetAttr( pXml, "prefab", "" );
		if( szPrefab[0] )
		{
			m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
		}
		m_size.x = XmlGetAttr( pXml, "sizex", 1 );
		m_size.y = XmlGetAttr( pXml, "sizey", 1 );
		m_nLayer = XmlGetAttr( pXml, "layer", 1 );
		m_nType = Min<uint8>( SBlock::eAttachedPrefab_Count, XmlGetAttr( pXml, "attach_type", 0 ) );

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		for( int j = region.y; j < region.GetBottom(); j += m_size.y )
		{
			for( int i = region.x; i < region.GetRight(); i += m_size.x )
			{
				context.AttachPrefab( m_pPrefab, TRectangle<int32>( i, j, m_size.x, m_size.y ), m_nLayer, m_nType );
			}
		}
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	CReference<CPrefab> m_pPrefab;
	TVector2<int32> m_size;
	uint8 m_nLayer;
	uint8 m_nType;
};

class CLevelGenerateTileNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_pNode = CreateNode( pXml->FirstChildElement(), context );
		tileSize.x = XmlGetAttr( pXml, "tilex", 1 );
		tileSize.y = XmlGetAttr( pXml, "tiley", 1 );

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		for( int j = region.y; j + tileSize.y <= region.y + region.height; j += tileSize.y )
		{
			for( int i = region.x; i + tileSize.x <= region.x + region.width; i += tileSize.x )
			{
				m_pNode->Generate( context, TRectangle<int32>( i, j, tileSize.x, tileSize.y ) );
			}
		}
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	CReference<CLevelGenerateNode> m_pNode;
	TVector2<int32> tileSize;
};

class CLevelGenerateSubRegionNode : public CLevelGenerateNode
{
public:
	struct SSubItem
	{
		CReference<CLevelGenerateNode> pNode;
		TRectangle<int32> subRegion;
		TRectangle<int32> subRegion1;
		float fChance;
	};

	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		for( auto pChild = pXml->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
		{
			auto pNode = CreateNode( pChild, context );
			if( !pNode )
				continue;
			m_subItems.resize( m_subItems.size() + 1 );
			auto& item = m_subItems.back();
			item.pNode = pNode;
			item.subRegion.x = XmlGetAttr( pChild, "x", 0 );
			item.subRegion.y = XmlGetAttr( pChild, "y", 0 );
			item.subRegion.width = XmlGetAttr( pChild, "w", 1 );
			item.subRegion.height = XmlGetAttr( pChild, "h", 1 );
			item.subRegion1.x = XmlGetAttr( pChild, "x1", item.subRegion.x );
			item.subRegion1.y = XmlGetAttr( pChild, "y1", item.subRegion.y );
			item.subRegion1.width = XmlGetAttr( pChild, "w1", item.subRegion.width );
			item.subRegion1.height = XmlGetAttr( pChild, "h1", item.subRegion.height );
			item.fChance = XmlGetAttr( pChild, "p", 1.0f );
		}

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		for( auto& item : m_subItems )
		{
			if( SRand::Inst().Rand( 0.0f, 1.0f ) < item.fChance )
			{
				auto subRegion = item.subRegion;
				if( subRegion.x < 0 )
				{
					subRegion.x = region.width + subRegion.x;
				}
				if( subRegion.width <= 0 )
				{
					subRegion.width = Max( region.width + subRegion.width - subRegion.x, 0 );
				}
				if( subRegion.y < 0 )
				{
					subRegion.y = region.height + subRegion.y;
				}
				if( subRegion.height <= 0 )
				{
					subRegion.height = Max( region.height + subRegion.height - subRegion.y, 0 );
				}
				subRegion = subRegion.Offset( TVector2<int32>( region.x, region.y ) );

				auto subRegion1 = item.subRegion1;
				if( subRegion1.x < 0 )
				{
					subRegion1.x = region.width + subRegion1.x;
				}
				if( subRegion1.width <= 0 )
				{
					subRegion1.width = Max( region.width + subRegion1.width - subRegion1.x, 0 );
				}
				if( subRegion1.y < 0 )
				{
					subRegion1.y = region.height + subRegion1.y;
				}
				if( subRegion1.height <= 0 )
				{
					subRegion1.height = Max( region.height + subRegion1.height - subRegion1.y, 0 );
				}
				subRegion1 = subRegion1.Offset( TVector2<int32>( region.x, region.y ) );

				int32 l0 = Min( subRegion.x, subRegion1.x ), l1 = Max( subRegion.x, subRegion1.x );
				int32 t0 = Min( subRegion.y, subRegion1.y ), t1 = Max( subRegion.y, subRegion1.y );
				int32 r0 = Min( subRegion.GetRight(), subRegion1.GetRight() ), r1 = Max( subRegion.GetRight(), subRegion1.GetRight() );
				int32 b0 = Min( subRegion.GetBottom(), subRegion1.GetBottom() ), b1 = Max( subRegion.GetBottom(), subRegion1.GetBottom() );
				int32 l = SRand::Inst().Rand( l0, l1 + 1 );
				int32 t = SRand::Inst().Rand( t0, t1 + 1 );
				int32 r = SRand::Inst().Rand( r0, r1 + 1 );
				int32 b = SRand::Inst().Rand( b0, b1 + 1 );

				item.pNode->Generate( context, TRectangle<int32>( l, t, r - l, b - t ) );
			}
		}
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	vector<SSubItem> m_subItems;
};

class CLevelGenerateRandomPickNode : public CLevelGenerateNode
{
public:
	struct SSubNodeInfo
	{
		uint32 nWeight;
		CReference<CLevelGenerateNode> pNode;
	};

	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_nTotalWeight = 0;
		for( auto pChild = pXml->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
		{
			auto pNode = CreateNode( pChild, context );
			if( !pNode )
				continue;
			m_infos.resize( m_infos.size() + 1 );
			auto& item = m_infos.back();
			item.pNode = pNode;
			item.nWeight = XmlGetAttr( pChild, "p", 1 );
			m_nTotalWeight += item.nWeight;
		}

		m_nTotalWeight = XmlGetAttr( pXml, "total_p", m_nTotalWeight );
		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		uint32 r = SRand::Inst().Rand( 0u, m_nTotalWeight );
		for( auto& info : m_infos )
		{
			if( r < info.nWeight )
			{
				info.pNode->Generate( context, region );
				break;
			}

			r -= info.nWeight;
		}
		CLevelGenerateNode::Generate( context, region );
	}
private:
	vector<SSubNodeInfo> m_infos;
	uint32 m_nTotalWeight;
};

class CLevelGenerateFrameNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		auto pItem = pXml->FirstChildElement( "center" );
		if( pItem )
		{
			m_pCenter = CreateNode( pItem->FirstChildElement(), context );
		}
		pItem = pXml->FirstChildElement( "left" );
		if( pItem )
		{
			m_nLeftSize = XmlGetAttr( pItem, "size", 1 );
			m_pLeft = CreateNode( pItem->FirstChildElement(), context );
		}
		else
			m_nLeftSize = 0;
		pItem = pXml->FirstChildElement( "right" );
		if( pItem )
		{
			m_nRightSize = XmlGetAttr( pItem, "size", 1 );
			m_pRight = CreateNode( pItem->FirstChildElement(), context );
		}
		else
			m_nRightSize = 0;
		pItem = pXml->FirstChildElement( "top" );
		if( pItem )
		{
			m_nTopSize = XmlGetAttr( pItem, "size", 1 );
			m_pTop = CreateNode( pItem->FirstChildElement(), context );
		}
		else
			m_nTopSize = 0;
		pItem = pXml->FirstChildElement( "bottom" );
		if( pItem )
		{
			m_nBottomSize = XmlGetAttr( pItem, "size", 1 );
			m_pBottom = CreateNode( pItem->FirstChildElement(), context );
		}
		else
			m_nBottomSize = 0;
		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		if( region.width < m_nLeftSize + m_nRightSize || region.height < m_nTopSize + m_nBottomSize )
		{
			m_pCenter->Generate( context, region );
			return;
		}

		auto centerRect = region;
		centerRect.SetLeft( centerRect.GetLeft() + m_nLeftSize );
		centerRect.SetRight( centerRect.GetRight() - m_nRightSize );
		centerRect.SetTop( centerRect.GetTop() + m_nTopSize );
		centerRect.SetBottom( centerRect.GetBottom() - m_nBottomSize );
		if( m_pCenter )
			m_pCenter->Generate( context, centerRect );
		if( m_pLeft )
		{
			auto rect = region;
			rect.SetRight( centerRect.GetLeft() );
			rect.SetTop( centerRect.GetTop() );
			rect.SetBottom( centerRect.GetBottom() );
			m_pLeft->Generate( context, rect );
		}
		if( m_pRight )
		{
			auto rect = region;
			rect.SetLeft( centerRect.GetRight() );
			rect.SetTop( centerRect.GetTop() );
			rect.SetBottom( centerRect.GetBottom() );
			m_pRight->Generate( context, rect );
		}
		if( m_pTop )
		{
			auto rect = region;
			rect.SetBottom( centerRect.GetTop() );
			m_pTop->Generate( context, rect );
		}
		if( m_pBottom )
		{
			auto rect = region;
			rect.SetTop( centerRect.GetBottom() );
			m_pBottom->Generate( context, rect );
		}
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	CReference<CLevelGenerateNode> m_pLeft;
	CReference<CLevelGenerateNode> m_pRight;
	CReference<CLevelGenerateNode> m_pTop;
	CReference<CLevelGenerateNode> m_pBottom;
	CReference<CLevelGenerateNode> m_pCenter;
	uint32 m_nLeftSize, m_nRightSize, m_nTopSize, m_nBottomSize;
};

class CLevelRandomFillGenerateNode : public CLevelGenerateNode
{
public:
	struct SSubNodeInfo
	{
		TVector2<int32> sizeMin;
		TVector2<int32> sizeMax;
		float fWeight;
		float fWeightRecalculated;
		int32 nWeightGroup;
		int32 nWeightGroupIndex;
		float fWeightInWeightGroup;
		CReference<CLevelGenerateNode> pNode;
	};

	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_nWeightGroupCount = 0;
		for( auto pChild = pXml->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
		{
			auto pNode = CreateNode( pChild, context );
			if( !pNode )
				continue;
			m_infos.resize( m_infos.size() + 1 );
			auto& item = m_infos.back();
			item.pNode = pNode;
			item.sizeMin.x = XmlGetAttr( pChild, "sizeminx", 1 );
			item.sizeMin.y = XmlGetAttr( pChild, "sizeminy", 1 );
			item.sizeMax.x = XmlGetAttr( pChild, "sizemaxx", item.sizeMin.x );
			item.sizeMax.y = XmlGetAttr( pChild, "sizemaxy", item.sizeMin.y );
			item.fWeight = XmlGetAttr( pChild, "p", 1.0f );
			item.nWeightGroup = XmlGetAttr( pChild, "weight_group", -1 );
			m_nWeightGroupCount = Max( m_nWeightGroupCount, item.nWeightGroup + 1 );
		}
		m_nCheckBlockType = XmlGetAttr( pXml, "check_block", 0 );
		m_nCheckGenData = XmlGetAttr( pXml, "check_gen_data", -1 );
		m_strCheckGenData = XmlGetAttr( pXml, "check_gen_data_name", "" );
		CLevelGenerateNode::Load( pXml, context );

		if( m_nWeightGroupCount )
		{
			m_weightGroups.resize( m_nWeightGroupCount );
			vector<float> vecWeightGroupsWeight;
			vecWeightGroupsWeight.resize( m_nWeightGroupCount );

			for( int i = 0; i < m_infos.size(); i++ )
			{
				auto& info = m_infos[i];
				if( info.nWeightGroup >= 0 )
				{
					info.nWeightGroupIndex = m_weightGroups[info.nWeightGroup].size();
					m_weightGroups[info.nWeightGroup].push_back( i );
					vecWeightGroupsWeight[info.nWeightGroup] += info.fWeight;
				}
			}
			for( auto& info : m_infos )
			{
				if( info.nWeightGroup >= 0 )
				{
					info.fWeightInWeightGroup = info.fWeight / vecWeightGroupsWeight[info.nWeightGroup];
				}
			}
		}
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		if( m_nWeightGroupCount )
		{
			vector<float> vecWeightGroupsWeight;
			vecWeightGroupsWeight.resize( m_nWeightGroupCount );
			for( int i = 0; i < m_nWeightGroupCount; i++ )
				vecWeightGroupsWeight[i] = 1;

			for( auto& info : m_infos )
			{
				info.fWeightRecalculated = 0;
			}

			for( auto& info : m_infos )
			{
				if( info.nWeightGroup >= 0 )
				{
					float fAddWeight[2];
					fAddWeight[0] = SRand::Inst().Rand( 0.0f, info.fWeight );
					fAddWeight[1] = info.fWeight - fAddWeight[0];

					for( int k = 0; k < 2; k++ )
					{
						float r = SRand::Inst().Rand( 0.0f, 1.0f );
						auto& vecGroup = m_weightGroups[info.nWeightGroup];
						int i = 0;
						for( ; i < vecGroup.size() - 1; i++ )
						{
							float fWeightInWeightGroup = m_infos[vecGroup[i]].fWeightInWeightGroup;
							if( r < fWeightInWeightGroup )
								break;
							r -= fWeightInWeightGroup;
						}

						m_infos[vecGroup[i]].fWeightRecalculated += fAddWeight[k];
					}
				}
			}
		}

		float fTotalWeight = 0;
		for( int i = m_infos.size() - 1; i >= 0; i-- )
		{
			auto& item = m_infos[i];
			float fWeight = item.nWeightGroup >= 0 ? item.fWeightRecalculated : item.fWeight;
			fTotalWeight += fWeight;
			item.fWeightRecalculated = fWeight / fTotalWeight;
		}

		auto size = region.GetSize();

		SSubNodeInfo dummy;
		vector<pair<SSubNodeInfo*, TRectangle<int32> > > genResult;
		genResult.resize( size.x * size.y );

		vector<int32> gridExt;
		gridExt.resize( size.x * size.y );
		vector<int32> gridOk;
		gridOk.resize( size.x * size.y );

		//Randomly flip
		bool bFlipX = SRand::Inst().Rand() & 1;
		bool bFlipY = SRand::Inst().Rand() & 1;
		if( m_nCheckBlockType || m_nCheckGenData >= 0 || m_strCheckGenData.length() )
		{
			int32 nCheckGenData1 = -1;
			if( m_strCheckGenData.length() )
			{
				auto itr = context.mapTags.find( m_strCheckGenData );
				if( itr != context.mapTags.end() )
					nCheckGenData1 = itr->second;
			}
			for( int j = 0; j < size.y; j++ )
			{
				for( int i = 0; i < size.x; i++ )
				{
					int32 x = ( bFlipX ? size.x - 1 - i : i ) + region.x;
					int32 y = ( bFlipY ? size.y - 1 - j : j ) + region.y;
					auto& grid = GET_GRID( genResult, i, j ).first;
					grid = NULL;
					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						if( !!( m_nCheckBlockType & ( 1 << iLayer ) ) && context.GetBlock( x, y, iLayer ) )
							grid = &dummy;
						else if( m_nCheckGenData >= 0 && context.blueprint[x + y * context.nWidth] != m_nCheckGenData )
							grid = &dummy;
						else if( nCheckGenData1 >= 0 && context.blueprint[x + y * context.nWidth] != nCheckGenData1 )
							grid = &dummy;
					}
				}
			}
		}

		for( auto& subNodeInfo : m_infos )
		{
			if( subNodeInfo.fWeightRecalculated * region.width * region.height < 0.5f )
				continue;

			memset( &gridExt[0], 0, sizeof( int32 ) * size.x * size.y );
			memset( &gridOk[0], 0, sizeof( int32 ) * size.x * size.y );
			auto minSize = subNodeInfo.sizeMin;
			auto maxSize = subNodeInfo.sizeMax;
			
			for( int j = 0; j < size.y; j++ )
			{
				int i0 = 0;
				for( int i = 0; ; i++ )
				{
					if( i == size.x || GET_GRID( genResult, i, j ).first )
					{
						for( ; i0 < i; i0++ )
						{
							GET_GRID( gridExt, i0, j ) = i - i0;
						}
						i0++;
						if( i == size.x )
							break;
					}
				}
			}
			for( int i = 0; i < size.x; i++ )
			{
				int j0 = 0;
				for( int j = 0; ; j++ )
				{
					if( j == size.y || GET_GRID( gridExt, i, j ) < minSize.x )
					{
						for( ; j0 < j; j0++ )
						{
							GET_GRID( gridOk, i, j0 ) = j - j0;
						}
						j0++;
						if( j == size.y )
							break;
					}
				}
			}
			for( int j = 0; j < size.y; j++ )
			{
				for( int i = 0; i < size.x; i++ )
				{
					GET_GRID( gridOk, i, j ) = GET_GRID( gridOk, i, j ) >= minSize.y ? 1 : 0;
				}
			}

			float fAverageSkipGrids = ( minSize.x + maxSize.x ) * ( minSize.y + maxSize.y ) / 4 * ( 1 - subNodeInfo.fWeightRecalculated ) / subNodeInfo.fWeightRecalculated;
			uint32 nSkipGrids = SRand::Inst().Rand( 0.0f, fAverageSkipGrids );
			uint32 nVisitedGrids = 0, nUsedGrids = 0, nGeneratedCount = 0;

			for( int j = 0; j < size.y; j++ )
			{
				for( int i = 0; i < size.x; i++ )
				{
					if( GET_GRID( genResult, i, j ).first )
						continue;

					if( nSkipGrids )
					{
						nVisitedGrids++;
						nSkipGrids--;
					}
					else if( GET_GRID( gridOk, i, j ) )
					{
						nGeneratedCount++;
						TRectangle<int32> rect( i, j, minSize.x, minSize.y );

						//Try to extend the rect
						TVector2<int32> desiredSize( SRand::Inst().Rand( minSize.x, maxSize.x + 1 ), SRand::Inst().Rand( minSize.y, maxSize.y + 1 ) );
						uint8 ValidDir[4] = { 0, 1, 2, 3 };
						uint8 nValidDir = 4;
						while( rect.width < desiredSize.x || rect.height < desiredSize.y )
						{
							uint8 nDir = ValidDir[SRand::Inst().Rand<uint8>( 0, nValidDir )];
							bool bCheckInvalid = false;

							switch( nDir )
							{
							case 0: //left
							{
								int32 x = rect.x - 1;
								if( x < 0 || rect.width == desiredSize.x )
								{
									bCheckInvalid = true;
									break;
								}
								bool bCanExtend = true;
								for( int y = rect.y; y < rect.GetBottom(); y++ )
								{
									if( GET_GRID( genResult, x, y ).first )
									{
										bCanExtend = false;
										break;
									}
								}
								if( !bCanExtend )
									bCheckInvalid = true;
								else
									rect.SetLeft( rect.x - 1 );
								break;
							}
							case 2: //right
							{
								int32 x = rect.GetRight();
								if( x >= size.x || rect.width == desiredSize.x )
								{
									bCheckInvalid = true;
									break;
								}
								bool bCanExtend = true;
								for( int y = rect.y; y < rect.GetBottom(); y++ )
								{
									if( GET_GRID( genResult, x, y ).first )
									{
										bCanExtend = false;
										break;
									}
								}
								if( !bCanExtend )
									bCheckInvalid = true;
								else
									rect.width++;
								break;
							}
							case 1: //top
							{
								int32 y = rect.y - 1;
								if( y < 0 || rect.height == desiredSize.y )
								{
									bCheckInvalid = true;
									break;
								}
								bool bCanExtend = true;
								for( int x = rect.x; x < rect.GetRight(); x++ )
								{
									if( GET_GRID( genResult, x, y ).first )
									{
										bCanExtend = false;
										break;
									}
								}
								if( !bCanExtend )
									bCheckInvalid = true;
								else
									rect.SetTop( rect.y - 1 );
								break;
							}
							case 3: //bottom
							{
								int32 y = rect.GetBottom();
								if( y >= size.y || rect.height == desiredSize.y )
								{
									bCheckInvalid = true;
									break;
								}
								bool bCanExtend = true;
								for( int x = rect.x; x < rect.GetRight(); x++ )
								{
									if( GET_GRID( genResult, x, y ).first )
									{
										bCanExtend = false;
										break;
									}
								}
								if( !bCanExtend )
									bCheckInvalid = true;
								else
									rect.height++;
								break;
							}
							default:
								break;
							}

							if( bCheckInvalid )
							{
								for( int k = nValidDir - 1; k >= 0; k-- )
								{
									bool bInvalid = false;
									if( ValidDir[k] == nDir )
										bInvalid = true;
									if( bInvalid )
										ValidDir[k] = ValidDir[--nValidDir];
								}
								if( !nValidDir )
									break;
							}
						}

						//Fill
						for( int x = rect.x; x < rect.GetRight(); x++ )
						{
							for( int y = rect.y; y < rect.GetBottom(); y++ )
							{
								GET_GRID( genResult, x, y ).first = &subNodeInfo;
								GET_GRID( genResult, x, y ).second = rect;
							}
						}
						nVisitedGrids += rect.width * rect.height;
						nUsedGrids += rect.width * rect.height;
						for( int x = Max( 0, rect.x - minSize.x + 1 ); x < rect.GetRight(); x++ )
						{
							for( int y = Max( 0, rect.y - minSize.y + 1 ); y < rect.GetBottom(); y++ )
							{
								GET_GRID( gridOk, x, y ) = 0;
							}
						}

						float fPercent = nUsedGrids / ( nVisitedGrids + fAverageSkipGrids * 0.5f );
						nSkipGrids = ( fAverageSkipGrids * 0.5f + SRand::Inst().Rand( 0.0f, fAverageSkipGrids ) ) * fPercent / subNodeInfo.fWeightRecalculated;
					}
				}
			}
		}

		//Gen children
		for( int j = 0; j < size.y; j++ )
		{
			for( int i = 0; i < size.x; i++ )
			{
				auto& result = GET_GRID( genResult, i, j );
				if( result.first && result.first != &dummy && i == result.second.x && j == result.second.y )
				{
					auto rect = result.second;
					if( bFlipX )
						rect.x = size.x - rect.x - rect.width;
					if( bFlipY )
						rect.y = size.y - rect.y - rect.height;
					rect.x += region.x;
					rect.y += region.y;
					result.first->pNode->Generate( context, rect );
				}
			}
		}

		CLevelGenerateNode::Generate( context, region );
	}
protected:
	uint8 m_nCheckBlockType;
	int32 m_nCheckGenData;
	string m_strCheckGenData;
	int32 m_nWeightGroupCount;
	vector<SSubNodeInfo> m_infos;
	vector<vector<uint32> > m_weightGroups;
};

#define REGISTER_GENERATE_NODE( Name, Class ) m_mapCreateFuncs[Name] = [] ( TiXmlElement* pXml ) { return new Class; };
CLevelGenerateFactory::CLevelGenerateFactory()
{
	REGISTER_GENERATE_NODE( "empty", CLevelGenerateNode );
	REGISTER_GENERATE_NODE( "simple", CLevelGenerateSimpleNode );
	REGISTER_GENERATE_NODE( "simple_attach", CLevelGenerateAttachNode );
	REGISTER_GENERATE_NODE( "simple_spawn", CLevelGenerateSpawnNode );
	REGISTER_GENERATE_NODE( "tile", CLevelGenerateTileNode );
	REGISTER_GENERATE_NODE( "subregion", CLevelGenerateSubRegionNode );
	REGISTER_GENERATE_NODE( "randompick", CLevelGenerateRandomPickNode );
	REGISTER_GENERATE_NODE( "frame", CLevelGenerateFrameNode );
	REGISTER_GENERATE_NODE( "randomfill", CLevelRandomFillGenerateNode );

	REGISTER_GENERATE_NODE( "bricktile", CBrickTileNode );
	REGISTER_GENERATE_NODE( "room1", CRoom1Node );
	REGISTER_GENERATE_NODE( "room2", CRoom2Node );
	REGISTER_GENERATE_NODE( "pipes", CPipeNode );
	REGISTER_GENERATE_NODE( "split", CSplitNode );
	REGISTER_GENERATE_NODE( "lv1type1", CLevelGenNode1_1 );
	REGISTER_GENERATE_NODE( "barrier1", CLvBarrierNodeGen1 );
}

CLevelGenerateNode* CLevelGenerateFactory::LoadNode( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context )
{
	auto itr = m_mapCreateFuncs.find( XmlGetAttr( pXml, "type", context.szDefaultType ) );
	if( itr != m_mapCreateFuncs.end() )
	{
		auto pNode = itr->second( pXml );
		pNode->Load( pXml, context );
		return pNode;
	}
	return NULL;
}

CLevelGenerateNode* CLevelGenerateNode::CreateNode( TiXmlElement* pXml, SLevelGenerateNodeLoadContext & context )
{
	if( !strcmp( pXml->Value(), "ref" ) )
	{
		const char* szName = XmlGetAttr( pXml, "name", "" );
		auto pNode = context.pCurFileContext->FindNode( szName );
		return pNode;
	}
	else if( !strcmp( pXml->Value(), "node" ) )
	{
		auto pNode = CLevelGenerateFactory::Inst().LoadNode( pXml, context );
		if( !pNode )
			return NULL;
		const char* szName = XmlGetAttr( pXml, "name", "" );
		if( szName[0] )
			context.pCurFileContext->mapNamedNodes[szName] = pNode;
		return pNode;
	}
	return NULL;
}

void SLevelGenerateFileContext::AddInclude( SLevelGenerateFileContext* pContext, bool bPublic )
{
	auto& pFile = mapIncludeFiles[pContext->strFullPath];
	if( pFile )
		return;
	pFile = pContext;
	vecIncludeFiles.push_back( pair<SLevelGenerateFileContext*, bool>( pContext, bPublic ) );
	for( auto& item : pContext->vecIncludeFiles )
	{
		if( item.second )
			AddInclude( item.first, bPublic );
	}
}

CLevelGenerateNode* SLevelGenerateFileContext::FindNode( const char* szNode )
{
	auto itr = mapNamedNodes.find( szNode );
	if( itr != mapNamedNodes.end() )
		return itr->second;

	for( auto item : vecIncludeFiles )
	{
		itr = item.first->mapNamedNodes.find( szNode );
		if( itr != item.first->mapNamedNodes.end() )
			return itr->second;
	}
	return NULL;
}

SLevelGenerateFileContext* SLevelGenerateNodeLoadContext::FindFile( const char* szFileName )
{
	string strPath = "";
	strPath = strPath + szFileName + ".xml";
	auto itr = mapFiles.find( strPath );
	if( itr != mapFiles.end() )
		return &itr->second;

	for( auto& path : vecPaths )
	{
		strPath = path;
		strPath = strPath + szFileName + ".xml";
		auto itr = mapFiles.find( strPath );
		if( itr != mapFiles.end() )
			return &itr->second;
	}
	
	return NULL;
}

SLevelGenerateFileContext* SLevelGenerateNodeLoadContext::LoadFile( const char* szFileName, const char* szPath )
{
	string strPath = szPath;
	strPath = strPath + szFileName + ".xml";

	if( mapFiles.find( strPath ) == mapFiles.end() && !IsFileExist( strPath.c_str() ) )
	{
		bool bFound = false;
		for( auto& path : vecPaths )
		{
			strPath = path;
			strPath = strPath + szFileName + ".xml";
			if( mapFiles.find( strPath ) != mapFiles.end() || IsFileExist( strPath.c_str() ) )
			{
				bFound = true;
				break;
			}
		}
		if( !bFound )
			return NULL;
	}

	auto& fileContext = mapFiles[strPath];
	if( fileContext.bValid )
		return &fileContext;
	fileContext.bValid = true;
	fileContext.strFileName = szFileName;
	fileContext.strFullPath = strPath;

	vector<char> content;
	GetFileContent( content, strPath.c_str(), true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );

	strPath = strPath.substr( 0, strPath.rfind( '/' ) + 1 );
	auto pIncludeRoot = doc.RootElement()->FirstChildElement( "includes" );
	if( pIncludeRoot )
	{
		for( auto pItem = pIncludeRoot->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			const char* szFileName = XmlGetAttr( pItem, "name", "" );
			bool bPublic = XmlGetAttr( pItem, "public", 0 );
			auto pInc = LoadFile( szFileName, strPath.c_str() );
			if( pInc )
				fileContext.AddInclude( pInc, bPublic );
		}
	}

	pCurFileContext = &fileContext;
	auto pLevelGenRoot = doc.RootElement()->FirstChildElement( "level_gen" );
	if( pLevelGenRoot )
	{
		szDefaultType = "simple_spawn";
		auto pSpawnRoot = pLevelGenRoot->FirstChildElement( "spawns" );
		if( pSpawnRoot )
		{
			for( auto pItem = pSpawnRoot->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
			{
				CLevelGenerateNode::CreateNode( pItem, *this );
			}
		}
		szDefaultType = "simple_attach";
		auto pAttachRoot = pLevelGenRoot->FirstChildElement( "attachs" );
		if( pAttachRoot )
		{
			for( auto pItem = pAttachRoot->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
			{
				CLevelGenerateNode::CreateNode( pItem, *this );
			}
		}
		szDefaultType = "simple";
		auto pNodeRoot = pLevelGenRoot->FirstChildElement( "nodes" );
		if( pNodeRoot )
		{
			for( auto pItem = pNodeRoot->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
			{
				CLevelGenerateNode::CreateNode( pItem, *this );
			}
		}
	}
	return &fileContext;
}