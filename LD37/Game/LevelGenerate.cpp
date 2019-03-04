#include "stdafx.h"
#include "LevelGenerate.h"
#include "LevelDesign.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "GUI/MainUI.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Entities/Decorator.h"
#include "LevelScrollObj.h"
#include "MainMenu.h"
#include "Common/Algorithm.h"

#include "LevelGenerating/LvGenCommon.h"
#include "LevelGenerating/LvGen1.h"
#include "LevelGenerating/LvGen2.h"
#include "LevelGenerating/LvBonusGen1.h"
#include "LevelGenerating/LvBonusGen2.h"

SLevelBuildContext::SLevelBuildContext() : pLevel( NULL ), pParentChunk( NULL ), nMaxChunkHeight( 0 ), pLastLevelBarrier( NULL )
{
}

SLevelBuildContext::SLevelBuildContext( CMyLevel* pLevel, SChunk* pParentChunk ) : pLevel( pLevel ), pParentChunk( pParentChunk ), nMaxChunkHeight( 0 ), pLastLevelBarrier( NULL ), bTest( false )
{
	nWidth = pParentChunk ? pParentChunk->nWidth : pLevel->m_nWidth;
	nHeight = pParentChunk ? pParentChunk->nHeight : pLevel->m_nHeight;
	nBlockSize = CMyLevel::GetBlockSize();
	blueprint.resize( nWidth * nHeight );
	blocks.resize( nWidth * nHeight * 2 );
	for( int i = 0; i < ELEM_COUNT( attachedPrefabs ); i++ )
		attachedPrefabs[i].resize( nWidth * nHeight * 2 );
	nSeed = SRand::Inst().nSeed;
}

SLevelBuildContext::SLevelBuildContext( const SLevelBuildContext& par, SChunk * pParentChunk ) : pLevel( par.pLevel ), pParentChunk( pParentChunk ), nMaxChunkHeight( 0 ), pLastLevelBarrier( NULL )
{
	nWidth = pParentChunk ? pParentChunk->nWidth : pLevel->m_nWidth;
	nHeight = pParentChunk ? pParentChunk->nHeight : pLevel->m_nHeight;
	nBlockSize = CMyLevel::GetBlockSize();
	blueprint.resize( nWidth * nHeight );
	blocks.resize( nWidth * nHeight * 2 );
	for( int i = 0; i < ELEM_COUNT( attachedPrefabs ); i++ )
		attachedPrefabs[i].resize( nWidth * nHeight * 2 );
	for( auto& item : par.mapTags )
		mapTags[item.first] = item.second;
	nSeed = par.nSeed;
	bTest = par.bTest;
}

SLevelBuildContext::SLevelBuildContext( uint32 nWidth, uint32 nHeight, bool bTest )
	: pLevel( NULL ), pParentChunk( NULL ), nWidth( nWidth ), nHeight( nHeight ), nBlockSize( 32 ), nMaxChunkHeight( 0 ), pLastLevelBarrier( NULL ), bTest( bTest )
{
	blueprint.resize( nWidth * nHeight );
	blocks.resize( nWidth * nHeight * 2 );
	for( int i = 0; i < ELEM_COUNT( attachedPrefabs ); i++ )
		attachedPrefabs[i].resize( nWidth * nHeight * 2 );
	nSeed = SRand::Inst().nSeed;
}

void SLevelBuildContext::Set( const SLevelBuildContext & par, SChunk * pParentChunk )
{
	pLevel = par.pLevel;
	this->pParentChunk = pParentChunk;
	nWidth = pParentChunk ? pParentChunk->nWidth : pLevel->m_nWidth;
	nHeight = pParentChunk ? pParentChunk->nHeight : pLevel->m_nHeight;
	nBlockSize = CMyLevel::GetBlockSize();
	blueprint.resize( nWidth * nHeight );
	blocks.resize( nWidth * nHeight * 2 );
	for( int i = 0; i < ELEM_COUNT( attachedPrefabs ); i++ )
		attachedPrefabs[i].resize( nWidth * nHeight * 2 );
	for( auto& item : par.mapTags )
		mapTags[item.first] = item.second;
	nSeed = par.nSeed;
	bTest = par.bTest;
}

SChunk* SLevelBuildContext::CreateChunk( SChunkBaseInfo& baseInfo, const TRectangle<int32>& region, SLevelBuildContext* pSubContext )
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
				auto pBlock = GetBlock( i, j, iLayer );
				if( pBlock )
				{
					if( !baseInfo.bPack )
						return NULL;
					auto pChunk = pBlock->pParent->pOwner;
					int32 x = pChunk->pos.x / nBlockSize;
					int32 y = pChunk->pos.y / nBlockSize;
					if( x < region.x || y < region.y || x + pChunk->nWidth > region.GetRight() || y + pChunk->nHeight > region.GetBottom() )
						return NULL;
					for( auto pChain = pChunk->Get_ChainEf1(); pChain; pChain = pChain->NextChainEf1() )
					{
						auto pChunk1 = pChain->pEf1;
						int32 x1 = pChunk1->pos.x / nBlockSize;
						int32 y1 = pChunk1->pos.y / nBlockSize;
						if( x1 < region.x || y1 < region.y || x1 + pChunk1->nWidth > region.GetRight() || y1 + pChunk1->nHeight > region.GetBottom() )
							return NULL;
					}
					for( auto pChain = pChunk->Get_ChainEf2(); pChain; pChain = pChain->NextChainEf2() )
					{
						auto pChunk1 = pChain->pEf2;
						int32 x1 = pChunk1->pos.x / nBlockSize;
						int32 y1 = pChunk1->pos.y / nBlockSize;
						if( x1 < region.x || y1 < region.y || x1 + pChunk1->nWidth > region.GetRight() || y1 + pChunk1->nHeight > region.GetBottom() )
							return NULL;
					}
				}

				if( baseInfo.bPack )
				{
					for( int iType = 0; iType < ELEM_COUNT( attachedPrefabs ); iType++ )
					{
						auto& item = attachedPrefabs[iType][iLayer + ( i + j * nWidth ) * 2];
						if( item.pPrefab )
						{
							if( item.rect.x < region.x || item.rect.y < region.y || item.rect.GetRight() > region.GetRight() || item.rect.GetBottom() > region.GetBottom() )
								return NULL;
						}
					}
				}
			}
		}
	}

	auto pChunk = new SChunk( baseInfo, TVector2<int32>( region.x * nBlockSize, region.y * nBlockSize ), region.GetSize() );
	chunks.push_back( pChunk );
	if( pSubContext )
		pSubContext->Set( *this, pChunk );

	for( int iLayer = 0; iLayer < 2; iLayer++ )
	{
		if( !baseInfo.HasLayer( iLayer ) )
			continue;
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				if( baseInfo.bPack )
				{
					auto pBlock = GetBlock( i + region.x, j + region.y, iLayer );
					if( pBlock )
					{
						auto pChunk1 = pBlock->pParent->pOwner;
						int32 x = pChunk1->pos.x / nBlockSize;
						int32 y = pChunk1->pos.y / nBlockSize;
						for( int iLayer = 0; iLayer < 2; iLayer++ )
						{
							if( !pChunk1->HasLayer( iLayer ) )
								continue;
							for( int j = 0; j < pChunk1->nHeight; j++ )
							{
								for( int i = 0; i < pChunk1->nWidth; i++ )
								{
									pSubContext->GetBlock( i + x - region.x, j + y - region.y, iLayer ) = pChunk1->blocks[i + j * pChunk1->nWidth].layers + iLayer;
									GetBlock( i + x, j + y, iLayer ) = NULL;
								}
							}
						}
						pSubContext->chunks.push_back( pChunk1 );
						pChunk1->pos = pChunk1->pos - TVector2<int32>( region.x, region.y ) * nBlockSize;
						pChunk1->pParentChunk = pChunk;
					}

					for( int iType = 0; iType < ELEM_COUNT( attachedPrefabs ); iType++ )
					{
						auto& item = attachedPrefabs[iType][iLayer + ( i + region.x + ( j + region.y ) * nWidth ) * 2];
						if( item.pPrefab )
						{
							auto r = item.rect.Offset( TVector2<int32>( -region.x, -region.y ) );
							for( int j = r.y; j < r.GetBottom(); j++ )
							{
								for( int i = r.x; i < r.GetRight(); i++ )
								{
									auto& item1 = pSubContext->attachedPrefabs[iType][iLayer + ( i + j * nWidth ) * 2];
									item1.pPrefab = item.pPrefab;
									item1.rect = r;
									item1.bType = item.bType;
								}
							}
							r = item.rect;
							for( int j = r.y; j < r.GetBottom(); j++ )
							{
								for( int i = r.x; i < r.GetRight(); i++ )
								{
									attachedPrefabs[iType][iLayer + ( i + j * nWidth ) * 2].pPrefab = NULL;
								}
							}
						}
					}
				}
				GetBlock( i + region.x, j + region.y, iLayer ) = pChunk->blocks[i + j * pChunk->nWidth].layers + iLayer;
			}
		}
	}

	if( strChunkName.length() )
	{
		mapChunkNames[strChunkName] = pChunk;
		strChunkName = "";
	}
	nMaxChunkHeight = Max<uint32>( nMaxChunkHeight, region.GetBottom() );
	pChunk->pParentChunk = pParentChunk;
	return pChunk;
}

void SLevelBuildContext::CreateChain( SChainBaseInfo& baseInfo, int32 x, int32 y1, int32 y2 )
{
	if( bTest )
	{
		SChain* pChain = new SChain( baseInfo, x, y1, y2 );
		chains.push_back( pChain );
		return;
	}

	auto nLayer = baseInfo.nLayer;
	SBlockLayer *p[2] = { NULL, NULL };
	int32 y[2];

	for( y[0] = y1; y[0] > 0; y[0]-- )
	{
		p[0] = GetBlock( x, y[0] - 1, nLayer );
		if( p[0] )
			break;
	}
	for( y[1] = y2; y[1] < nHeight; y[1]++ )
	{
		p[1] = GetBlock( x, y[1], nLayer );
		if( p[1] )
			break;
	}
	if( !p[0] || !p[1] )
		return;

	SChunk* pChunk[2];
	TVector2<int32> ofs[2];
	for( int i = 0; i < 2; i++ )
	{
		pChunk[i] = p[i]->pParent->pOwner;
		ofs[i] = pChunk[i]->pos / CMyLevel::GetBlockSize();
		if( baseInfo.nAttachType1 )
		{
			while( 1 )
			{
				SChunk* pSubChunk1 = NULL;
				for( auto pSubChunk = pChunk[i]->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
				{
					if( pSubChunk->nSubChunkType == 0 )
						continue;
					TVector2<int32> ofs1 = ofs[i] + pSubChunk->pos / CMyLevel::GetBlockSize();
					if( x < ofs1.x || x >= ofs1.x + pSubChunk->nWidth || y[i] != ( i == 1 ? ofs1.y : ofs1.y + pSubChunk->nHeight ) )
						continue;
					pSubChunk1 = pSubChunk;
					ofs[i] = ofs1;
					break;
				}
				if( !pSubChunk1 )
					break;
				pChunk[i] = pSubChunk1;
			}
		}
	}
	if( !pChunk[0]->pPrefab || !pChunk[1]->pPrefab )
		return;

	SChain* pChain = new SChain( baseInfo, pChunk[0], pChunk[1], x, y1 - ofs[0].y, y2 - ofs[1].y );
	chains.push_back( pChain );
}

void SLevelBuildContext::AttachPrefab( CPrefab* pPrefab, TRectangle<int32> rect, uint8 nLayer, uint8 nType, bool bType1 )
{
	auto r = rect;
	if( nType > 0 )
		r.width = r.height = 1;
	for( int j = r.y; j < r.GetBottom(); j++ )
	{
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			if( attachedPrefabs[nType][nLayer + ( i + j * nWidth ) * 2].pPrefab )
				return;
		}
	}

	for( int j = r.y; j < r.GetBottom(); j++ )
	{
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			auto& item = attachedPrefabs[nType][nLayer + ( i + j * nWidth ) * 2];
			item.pPrefab = pPrefab;
			item.rect = rect;
			item.bType = bType1;
		}
	}
}

void SLevelBuildContext::AddSpawnInfo( SChunkSpawnInfo * pInfo, const TVector2<int32>& ofs )
{
	auto pBlock = GetBlock( ofs.x, ofs.y, 1 );
	if( !pBlock )
		pBlock = GetBlock( ofs.x, ofs.y, 0 );
	if( !pBlock )
	{
		delete pInfo;
		return;
	}

	pInfo->rect = pInfo->rect.Offset( CVector2( pBlock->pParent->nX * nBlockSize, pBlock->pParent->nY * nBlockSize ) );

	pBlock->pParent->pOwner->Insert_SpawnInfo( pInfo );
}

void SLevelBuildContext::PushScrollObj( CPrefab* pPrefab, uint32 nType )
{
	scrollObjs[nType].push_back( pPrefab );
}

void SLevelBuildContext::Build()
{
	if( pParentChunk )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			for( int i = 0; i < nWidth; i++ )
			{
				for( int iLayer = 0; iLayer < 2; iLayer++ )
				{
					if( !pParentChunk->HasLayer( iLayer ) )
						continue;

					auto pBlock = GetBlock( i, j, iLayer );
					if( pBlock )
					{
						if( pBlock->pParent->pOwner->nSubChunkType == 1 )
						{
							pParentChunk->GetBlock( i, j )->eBlockType = pBlock->pParent->eBlockType;
							pParentChunk->GetBlock( i, j )->fDmgPercent = pBlock->pParent->fDmgPercent;
							pParentChunk->GetBlock( i, j )->bImmuneToBlockBuff = pBlock->pParent->bImmuneToBlockBuff;
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

		SChunk* pLevelBarrier = pLastLevelBarrier;
		if( !pLevelBarrier )
		{
			if( pLevel )
			{
				for( auto pBlock = pLevel->m_basements[0].layers[0].Get_BlockLayer(); pBlock; pBlock = pBlock->NextBlockLayer() )
				{
					if( pBlock->pParent->pOwner->nLevelBarrierType )
						pLevelBarrier = pBlock->pParent->pOwner;
				}
			}
		}
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
		int32 nYOfs = pLevelBarrier ? pLevelBarrier->pos.y + pLevelBarrier->nHeight * nBlockSize : 0;
		int32 nGridOfs = nYOfs / nBlockSize;

		pLastLevelBarrier = NULL;
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
						if( pBlock->pOwner->nLevelBarrierType )
							pLastLevelBarrier = pBlock->pOwner;

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
					SAttach attachedPrefabLayers[2] = { attachedPrefabs[k][0 + ( i + j * nWidth ) * 2], attachedPrefabs[k][1 + ( i + j * nWidth ) * 2] };

					if( bLayerValid[0] && !bLayerValid[1] && !attachedPrefabLayers[0].pPrefab && attachedPrefabLayers[1].pPrefab )
					{
						attachedPrefabLayers[0] = attachedPrefabLayers[1];
						attachedPrefabLayers[1].pPrefab = NULL;
					}

					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						if( !bLayerValid[iLayer] )
							continue;

						if( k == SBlock::eAttachedPrefab_Right )
						{
							if( i >= nWidth - 1 )
								continue;
							auto pRightBlock = GetBlock( i + 1, j, iLayer );
							if( pRightBlock && pRightBlock->pParent->eBlockType != eBlockType_Wall )
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
						if( k == SBlock::eAttachedPrefab_Left )
						{
							if( i <= 0 )
								continue;
							auto pLeftBlock = GetBlock( i - 1, j, iLayer );
							if( pLeftBlock && pLeftBlock->pParent->eBlockType != eBlockType_Wall )
								continue;
						}
						if( k == SBlock::eAttachedPrefab_Lower )
						{
							if( j <= 0 )
								continue;
							auto pLowerBlock = GetBlock( i, j - 1, iLayer );
							if( pLowerBlock && pLowerBlock->pParent->eBlockType != eBlockType_Wall )
								continue;
						}

						auto& attachedPrefab = attachedPrefabLayers[iLayer];
						auto pBlock = pBlockLayers[iLayer]->pParent;
						if( attachedPrefab.rect.x == i && attachedPrefab.rect.y == j )
						{
							if( !pBlock->pOwner->pPrefab )
							{
								if( !pParentChunk )
									continue;
								pBlock = pParentChunk->GetBlock( i, j );
							}

							pBlock->pAttachedPrefab[k] = attachedPrefab.pPrefab;
							if( k == SBlock::eAttachedPrefab_Center )
								pBlock->attachedPrefabSize = attachedPrefab.rect.GetSize();
							else if( k == SBlock::eAttachedPrefab_Lower )
								pBlock->nLowerMargin = attachedPrefab.rect.height;
							else if( k == SBlock::eAttachedPrefab_Upper )
								pBlock->nUpperMargin = attachedPrefab.rect.height;

							if( attachedPrefab.bType )
								pBlock->nAttachType |= 1 << k;
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
					&& iLayer == pBlock->pParent->pOwner->GetMaxLayer() - 1 )
				{
					delete pBlock->pParent->pOwner;
				}
			}
		}
	}

	if( pLevel )
	{
		for( int i = 0; i < ELEM_COUNT( scrollObjs ); i++ )
		{
			uint32 nHeight = pLevel->GetCurHeightTag();
			for( CPrefab* pPrefab : scrollObjs[i] )
			{
				auto pObj = SafeCast<CLevelScrollObj>( pPrefab->GetRoot()->CreateInstance() );
				pObj->SetParentEntity( pLevel->GetScrollObjRoot( i ) );
				pObj->Set( nHeight );
				pObj->PostBuild( *this );
				pObj->Update( pLevel->GetCurScrollPos() );
				nHeight += pObj->GetHeight();
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
		m_metadata.nSeed = XmlGetAttr<uint32>( pMetadata, "seed", m_metadata.nSeed );
		m_metadata.nDefaultChainType[0] = XmlGetAttr<uint32>( pMetadata, "chain_type1", m_metadata.nDefaultChainType[0] );
		m_metadata.nDefaultChainType[1] = XmlGetAttr<uint32>( pMetadata, "chain_type2", m_metadata.nDefaultChainType[1] );

		auto pTypes = pMetadata->FirstChildElement( "types" );
		if( pTypes )
		{
			int32 nMaxType = 0;
			for( auto pItem = pTypes->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
			{
				SMetadata::SType item;
				item.strName = XmlGetAttr( pItem, "name", "" );
				item.nType = XmlGetAttr( pItem, "value", nMaxType + 1 );
				item.nChainType[0] = XmlGetAttr<uint32>( pItem, "chain_type1", m_metadata.nDefaultChainType[0] );
				item.nChainType[1] = XmlGetAttr<uint32>( pItem, "chain_type2", m_metadata.nDefaultChainType[1] );
				nMaxType = Max( nMaxType, item.nType );
				m_metadata.vecTypes.push_back( item );
			}
		}
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

int8 CLevelGenerateNode::SMetadata::GetChainType( int32 nType, uint8 nLayer ) const
{
	for( int i = 0; i < vecTypes.size(); i++ )
	{
		if( vecTypes[i].nType == nType )
		{
			return vecTypes[i].nChainType[nLayer];
		}
	}
	return nDefaultChainType[nLayer];
}

CVector4 CLevelGenerateNode::SMetadata::GetEditColor( int32 nType ) const
{
	static CVector4 colors[] = 
	{
		{ 1, 0, 0, 1 },
		{ 1, 1, 0, 1 },
		{ 0, 1, 0, 1 },
		{ 0, 1, 1, 1 },
		{ 0, 0, 1, 1 },
		{ 1, 0, 1, 1 },
		{ 0, 1, 0.5f, 1 },
		{ 0.5f, 0, 1, 1 },
		{ 1, 0.5f, 0, 1 },
		{ 0, 0.5f, 1, 1 },
		{ 1, 0, 0.5f, 1 },
		{ 0.5f, 1, 0, 1 },
	};
	for( int i = 0; i < vecTypes.size(); i++ )
	{
		if( vecTypes[i].nType == nType )
		{
			CVector4 color = colors[i % ELEM_COUNT( colors )];
			int32 n1 = ( i / ELEM_COUNT( colors ) ) & 3;
			if( n1 == 1 )
				color = ( color + CVector4( 0, 0, 0, 1 ) ) * 0.5f;
			else if( n1 == 2 )
				color = ( color + CVector4( 1, 1, 1, 1 ) ) * 0.5f;
			else if( n1 == 3 )
				color = ( color + CVector4( 0.5f, 0.5f, 0.5f, 0.5f ) ) * 0.5f;

			return color;
		}
	}
	return CVector4( 0.4f, 0.4f, 0.4f, 0.4f );
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
	chunk.fImbalanceTime = XmlGetAttr( pXml, "imbalancetime", 1.0f );
	chunk.fShakeWeightCoef = XmlGetAttr( pXml, "shakeweightcoef", 0.0f );
	chunk.fShakeDmg = XmlGetAttr( pXml, "shakedmg", 1.0f );
	chunk.fShakeDmgPerWidth = XmlGetAttr( pXml, "shakedmgperwidth", 0.0F );
	chunk.nAbsorbShakeStrength = XmlGetAttr( pXml, "absorbshakestrength", 1 );
	chunk.nDestroyShake = XmlGetAttr( pXml, "destroyshake", 1 );
	chunk.nShakeDmgThreshold = XmlGetAttr( pXml, "shakedmgthreshold", 1 );
	chunk.fWeightPerWidth = XmlGetAttr( pXml, "weightperwidth", 0.0f );
	chunk.fDestroyWeightPerWidth = XmlGetAttr( pXml, "destroyweightperwidth", 0.0f );
	chunk.fAbsorbShakeStrengthPerHeight = XmlGetAttr( pXml, "absorbshakestrengthperheight", 1.0f );
	m_nLevelBarrierType = XmlGetAttr( pXml, "islevelbarrier", 0 );
	m_nLevelBarrierHeight = XmlGetAttr( pXml, "levelbarrierheight", 0 );
	chunk.nLayerType = XmlGetAttr( pXml, "layer_type", 3 );
	chunk.nMoveType = XmlGetAttr( pXml, "movetype", 0 );
	chunk.bIsRoom = XmlGetAttr( pXml, "isroom", 0 );
	chunk.nSubChunkType = XmlGetAttr( pXml, "subchunk_type", 0 );
	chunk.bPack = XmlGetAttr( pXml, "ispack", 0 );
	chunk.nChunkTag = 0;
	static char szCTags[8][16] = { "ctag0", "ctag1", "ctag2", "ctag3", "ctag4", "ctag5", "ctag6", "ctag7" };
	for( int i = 0; i < 8; i++ )
	{
		chunk.nChunkTag |= ( !!XmlGetAttr( pXml, szCTags[i], 0 ) << i );
	}

	chunk.nShowLevelType = XmlGetAttr( pXml, "show_layer_type", chunk.nLayerType > 1 ? 1 : 0 );
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

	const char* szTag = XmlGetValue( pXml, "tags", "" );
	if( szTag[0] )
	{
		c = szTag;
		i = 0;
		while( *c && i < chunk.nWidth * chunk.nHeight )
		{
			if( *c >= '0' && *c <= '9' )
			{
				int32 n = 0;
				do
				{
					n = n * 10 + *c - '0';
					c++;
				}
				while( *c >= '0' && *c <= '9' );

				int y = i / chunk.nWidth;
				int x = i % chunk.nWidth;
				auto& blockInfo = chunk.blockInfos[x + ( chunk.nHeight - y - 1 ) * chunk.nWidth];
				blockInfo.nTag = n;
				i++;
			}
			else
				c++;
		}
	}
	else
	{
		for( auto& blockInfo : chunk.blockInfos )
			blockInfo.nTag = 0;
	}

	const char* szImmuneToBlockBuff = XmlGetValue( pXml, "immune_to_block_buff", "" );
	if( szImmuneToBlockBuff[0] )
	{
		c = szImmuneToBlockBuff;
		i = 0;
		while( *c && i < chunk.nWidth * chunk.nHeight )
		{
			if( *c >= '0' && *c <= '9' )
			{
				int y = i / chunk.nWidth;
				int x = i % chunk.nWidth;
				auto& blockInfo = chunk.blockInfos[x + ( chunk.nHeight - y - 1 ) * chunk.nWidth];
				blockInfo.bImmuneToBlockBuff = *c != '0';
				i++;
			}
			c++;
		}
	}
	else
	{
		for( auto& blockInfo : chunk.blockInfos )
			blockInfo.bImmuneToBlockBuff = 0;
	}

	m_bCopyBlueprint = 0;
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
	SLevelBuildContext tempContext;
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region, m_pSubChunk || m_pChunkBaseInfo->bPack ? &tempContext : NULL );
	if( pChunk )
	{
		pChunk->nLevelBarrierType = m_nLevelBarrierType;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;

		if( m_pSubChunk || m_pChunkBaseInfo->bPack )
		{
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
		CLevelGenerateNode::Generate( context, region );
	}
}

class CLevelGenerateDesignedLevelNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_strFileName = XmlGetAttr( pXml, "file", "" );

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		if( !IsFileExist( m_strFileName.c_str() ) )
			return;
		vector<char> content;
		uint32 nSize = GetFileContent( content, m_strFileName.c_str(), false );
		CBufReader buf( &content[0], nSize );
		SLevelDesignContext context1( region.width, region.height );
		context1.Init();
		context1.Load( buf );

		for( int k = 0; k < 4; k++ )
		{
			for( int i = 0; i < context1.nWidth; i++ )
			{
				for( int j = 0; j < context1.nHeight; j++ )
				{
					auto pItem = context1.items[k][i + j * context1.nWidth];
					if( pItem && pItem->region.x == i && pItem->region.y == ( k >= 2 ? j + 1 : j ) && pItem->pGenNode->GetMetadata().nMinLevel == k )
					{
						if( pItem->strChunkName.length() )
							context.strChunkName = pItem->strChunkName;
						auto reg = pItem->region.Offset( TVector2<int32>( region.x, region.y ) );

						for( auto& item : pItem->pGenNode->GetMetadata().vecTypes )
						{
							context.mapTags[item.strName] = item.nType;
						}
						for( int y = 0; y < reg.height; y++ )
						{
							for( int x = 0; x < reg.width; x++ )
							{
								int8 nType = x + y * reg.width < pItem->vecData.size() ? pItem->vecData[x + y * reg.width] : 0;
								context.blueprint[x + reg.x + ( y + reg.y ) * region.width] = nType;
							}
						}
						pItem->pGenNode->Generate( context, reg );
						for( auto& item : pItem->pGenNode->GetMetadata().vecTypes )
						{
							context.mapTags.erase( item.strName );
						}
					}
				}
			}
		}
	}
protected:
	string m_strFileName;
};

class CLevelGenerateFillTagNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_strGenData = XmlGetAttr( pXml, "gen_data_name", "" );
		m_nBlockType = XmlGetAttr( pXml, "block_type", -1 );
		m_nTag = XmlGetAttr( pXml, "tag", -1 );
		m_nImmuneToBlockBuff = XmlGetAttr( pXml, "immune_to_block_buff", -1 );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		auto itr = context.mapTags.find( m_strGenData );
		if( itr == context.mapTags.end() )
			return;
		int8 nType = itr->second;
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			for( int j = region.y; j < region.GetBottom(); j++ )
			{
				if( context.blueprint[i + j * context.nWidth] == nType )
				{
					if( m_nBlockType != INVALID_8BITID )
						context.pParentChunk->blocks[i + j * context.nWidth].eBlockType = m_nBlockType;
					if( m_nTag!= INVALID_8BITID )
						context.pParentChunk->blocks[i + j * context.nWidth].nTag = m_nTag;
					if( m_nImmuneToBlockBuff != INVALID_8BITID )
						context.pParentChunk->blocks[i + j * context.nWidth].bImmuneToBlockBuff = m_nImmuneToBlockBuff;
				}
			}
		}
	}
protected:
	string m_strGenData;
	uint8 m_nBlockType;
	uint8 m_nTag;
	uint8 m_nImmuneToBlockBuff;
};

class CLevelGenerateSpawnNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( XmlGetAttr( pXml, "prefab", "" ) );
		m_bForceAttach = XmlGetAttr( pXml, "force_attach", 0 );
		m_rectSize.x = XmlGetAttr( pXml, "sizex", 0.0f );
		m_rectSize.y = XmlGetAttr( pXml, "sizey", 0.0f );
		m_rectSize.width = XmlGetAttr( pXml, "sizewidth", 0.0f );
		m_rectSize.height = XmlGetAttr( pXml, "sizeheight", 0.0f );
		m_fCountPerGrid = XmlGetAttr( pXml, "countpergrid", 1.0f );
		m_nCheckGenData = XmlGetAttr( pXml, "check_gen_data", -1 );
		m_strCheckGenData = XmlGetAttr( pXml, "check_gen_data_name", "" );

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
			pSpawnInfo->rect = rect;
			pSpawnInfo->pPrefab = m_pPrefab;
			pSpawnInfo->r = 0;
			context.AddSpawnInfo( pSpawnInfo, TVector2<int32>( region.x, region.y ) );
		}
		else
		{
			if( m_nCheckGenData >= 0 || m_strCheckGenData.length() )
			{
				int32 nCheckGenData1 = -1;
				if( m_strCheckGenData.length() )
				{
					auto itr = context.mapTags.find( m_strCheckGenData );
					if( itr != context.mapTags.end() )
						nCheckGenData1 = itr->second;
				}
				vector<TVector2<int32> > result;
				for( int j = 0; j < region.height; j++ )
				{
					for( int i = 0; i < region.width; i++ )
					{
						int32 x = i + region.x;
						int32 y = j + region.y;
						if( m_nCheckGenData >= 0 && context.blueprint[x + y * context.nWidth] != m_nCheckGenData )
							continue;
						else if( nCheckGenData1 >= 0 && context.blueprint[x + y * context.nWidth] != nCheckGenData1 )
							continue;
						result.push_back( TVector2<int32>( x, y ) );
					}
				}
				uint32 nCount = floor( result.size() * m_fCountPerGrid + SRand::Inst().Rand( 0.0f, 1.0f ) );
				CRectangle rect( 0, 0, context.nBlockSize, context.nBlockSize );
				rect.SetLeft( rect.x - m_rectSize.x );
				rect.SetTop( rect.y - m_rectSize.y );
				rect.SetRight( rect.GetRight() - m_rectSize.GetRight() );
				rect.SetBottom( rect.GetBottom() - m_rectSize.GetBottom() );
				for( int i = 0; i < nCount; i++ )
				{
					TVector2<int32> p = result[SRand::Inst().Rand( 0u, nCount )];

					SChunkSpawnInfo* pSpawnInfo = new SChunkSpawnInfo;
					pSpawnInfo->rect = CRectangle( SRand::Inst().Rand( rect.GetLeft(), rect.GetRight() ), SRand::Inst().Rand( rect.GetTop(), rect.GetBottom() ), 0, 0 );
					pSpawnInfo->pPrefab = m_pPrefab;
					pSpawnInfo->r = 0;
					pSpawnInfo->bForceAttach = m_bForceAttach;
					context.AddSpawnInfo( pSpawnInfo, TVector2<int32>( p.x, p.y ) );
				}
			}
			else
			{
				uint32 nCount = floor( region.width * region.height * m_fCountPerGrid + SRand::Inst().Rand( 0.0f, 1.0f ) );

				for( int i = 0; i < nCount; i++ )
				{
					SChunkSpawnInfo* pSpawnInfo = new SChunkSpawnInfo;
					pSpawnInfo->rect = CRectangle( SRand::Inst().Rand( rect.GetLeft(), rect.GetRight() ), SRand::Inst().Rand( rect.GetTop(), rect.GetBottom() ), 0, 0 );
					pSpawnInfo->pPrefab = m_pPrefab;
					pSpawnInfo->r = 0;
					pSpawnInfo->bForceAttach = m_bForceAttach;
					context.AddSpawnInfo( pSpawnInfo, TVector2<int32>( region.x, region.y ) );
				}
			}
		}
	}
protected:
	CReference<CPrefab> m_pPrefab;
	CRectangle m_rectSize;
	bool m_bForceAttach;
	float m_fCountPerGrid;
	int32 m_nCheckGenData;
	string m_strCheckGenData;
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
		m_bType1 = XmlGetAttr( pXml, "type1", 0 );

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		int32 nSizeX = m_nType > 0 ? 1 : m_size.x;
		int32 nSizeY = m_nType > 0 ? 1 : m_size.y;
		for( int j = region.y; j < region.GetBottom(); j += nSizeY )
		{
			for( int i = region.x; i < region.GetRight(); i += nSizeX )
			{
				context.AttachPrefab( m_pPrefab, TRectangle<int32>( i, j, m_size.x, m_size.y ), m_nLayer, m_nType, m_bType1 );
			}
		}
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	CReference<CPrefab> m_pPrefab;
	TVector2<int32> m_size;
	uint8 m_nLayer;
	uint8 m_nType;
	bool m_bType1;
};

class CLevelGenerateChainNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_baseInfo.nAttachType1 = XmlGetAttr( pXml, "attach_type1", 0 );
		m_baseInfo.nAttachType2 = XmlGetAttr( pXml, "attach_type2", 0 );
		m_baseInfo.nLayer = XmlGetAttr( pXml, "layer", 0 );
		m_baseInfo.nLen = XmlGetAttr( pXml, "len", 16 );
		const char* szPrefab = XmlGetAttr( pXml, "prefab", "" );
		if( szPrefab[0] )
			m_baseInfo.pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );

		m_metadata.bIsDesignValid = true;
		m_metadata.minSize.y = m_metadata.maxSize.y = 0;
		CLevelGenerateNode::Load( pXml, context );
		m_metadata.minSize.x = m_metadata.maxSize.x = 1;
		m_metadata.nMinLevel = m_metadata.nMaxLevel = m_baseInfo.nLayer + 2;
		m_metadata.nEditType = eEditType_Chain;
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		context.CreateChain( m_baseInfo, region.x, region.y, region.GetBottom() );
	}
protected:
	SChainBaseInfo m_baseInfo;
};

class CLevelGenerateScrollObjNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		const char* szPrefab = XmlGetAttr( pXml, "prefab", "" );
		if( szPrefab[0] )
		{
			m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
		}
		m_nType = Min<uint8>( SBlock::eAttachedPrefab_Count, XmlGetAttr( pXml, "scrollobj_type", 0 ) );

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		context.PushScrollObj( m_pPrefab, m_nType );
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	CReference<CPrefab> m_pPrefab;
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
		auto pParams = pXml->FirstChildElement( "params" );
		if( pParams )
		{
			for( auto pItem = pParams->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
			{
				m_vecParams.resize( m_vecParams.size() + 1 );
				auto& item = m_vecParams.back();
				item.first = XmlGetAttr( pItem, "name", "" );
				item.second = XmlGetAttr( pItem, "value", 0 );
			}
		}

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
		for( auto& item : m_vecParams )
			context.mapTags[item.first] = item.second;
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
				if( l < region.x || t < region.y || r > region.GetRight() || b > region.GetBottom() )
					continue;
				item.pNode->Generate( context, TRectangle<int32>( l, t, r - l, b - t ) );
			}
		}
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	vector<SSubItem> m_subItems;
	vector<pair<string, int8> > m_vecParams;
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

class CLevelGenerateSwitchNode : public CLevelGenerateNode
{
public:
	struct SSubNodeInfo
	{
		int32 nValue;
		CReference<CLevelGenerateNode> pNode;
	};

	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		m_strSwitchValue = XmlGetAttr( pXml, "str", "" );
		for( auto pChild = pXml->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
		{
			auto pNode = CreateNode( pChild, context );
			if( !pNode )
				continue;
			m_infos.resize( m_infos.size() + 1 );
			auto& item = m_infos.back();
			item.pNode = pNode;
			item.nValue = XmlGetAttr( pChild, "case", -1 );
		}

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		uint8 nValue = context.mapTags[m_strSwitchValue];
		for( auto& info : m_infos )
		{
			if( info.nValue == nValue || info.nValue == -1 )
			{
				info.pNode->Generate( context, region );
				break;
			}
		}
		CLevelGenerateNode::Generate( context, region );
	}
private:
	vector<SSubNodeInfo> m_infos;
	string m_strSwitchValue;
};

class CLevelGenerateSwitchTileNode : public CLevelGenerateNode
{
public:
	struct SSubNodeInfo
	{
		string str;
		CReference<CLevelGenerateNode> pNode;
		int8 nType;
		int32 nOfsX, nOfsY;
	};
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		for( auto pChild = pXml->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
		{
			auto pNode = CreateNode( pChild, context );
			if( !pNode )
				continue;
			m_infos.resize( m_infos.size() + 1 );
			auto& item = m_infos.back();
			item.pNode = pNode;
			item.str = XmlGetAttr( pChild, "case", "" );
			item.nOfsX = XmlGetAttr( pChild, "ofsx", 0 );
			item.nOfsY = XmlGetAttr( pChild, "ofsy", 0 );
		}

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		for( auto& info : m_infos )
		{
			info.nType = -1;
			if( info.str.length() )
			{
				auto itr = context.mapTags.find( info.str );
				if( itr != context.mapTags.end() )
					info.nType = itr->second;
			}
		}
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			for( int j = region.y; j < region.GetBottom(); j++ )
			{
				for( auto& info : m_infos )
				{
					int32 x = i + info.nOfsX;
					int32 y = j + info.nOfsY;
					if( x >= region.x && y >= region.y && x < region.GetRight() && y < region.GetBottom()
						&& info.nType == context.blueprint[x + y * context.nWidth] || !info.str.length() )
					{
						info.pNode->Generate( context, TRectangle<int32>( i, j, 1, 1 ) );
						break;
					}
				}
			}
		}
	}
protected:
	vector<SSubNodeInfo> m_infos;
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

class CLevelAutoFillGenerateNode : public CLevelGenerateNode
{
public:
	struct SSubNodeInfo
	{
		TVector2<int32> sizeMin;
		TVector2<int32> sizeMax;
		CReference<CLevelGenerateNode> pNode;
	};

	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
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
		}
		m_nCheckBlockType = XmlGetAttr( pXml, "check_block", 0 );
		m_nCheckGenData = XmlGetAttr( pXml, "check_gen_data", -1 );
		m_strCheckGenData = XmlGetAttr( pXml, "check_gen_data_name", "" );
		m_strCheckGenData1 = XmlGetAttr( pXml, "check_gen_data_name1", "" );
		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		auto size = region.GetSize();
		vector<int8> vecTemp;
		vecTemp.resize( size.x * size.y );

		if( m_nCheckBlockType || m_nCheckGenData >= 0 || m_strCheckGenData.length() )
		{
			int32 nCheckGenData1 = -1;
			if( m_strCheckGenData.length() )
			{
				auto itr = context.mapTags.find( m_strCheckGenData );
				if( itr != context.mapTags.end() )
					nCheckGenData1 = itr->second;
				else
				{
					for( int j = 0; j < size.y; j++ )
					{
						for( int i = 0; i < size.x; i++ )
							GET_GRID( vecTemp, i, j ) = 1;
					}
				}
			}

			int32 nCheckGenData2 = nCheckGenData1;
			if( m_strCheckGenData1.length() )
			{
				auto itr = context.mapTags.find( m_strCheckGenData1 );
				if( itr != context.mapTags.end() )
					nCheckGenData2 = itr->second;
				else
				{
					for( int j = 0; j < size.y; j++ )
					{
						for( int i = 0; i < size.x; i++ )
							GET_GRID( vecTemp, i, j ) = 1;
					}
				}
			}

			for( int j = 0; j < size.y; j++ )
			{
				for( int i = 0; i < size.x; i++ )
				{
					int32 x = i + region.x;
					int32 y = j + region.y;
					auto& grid = GET_GRID( vecTemp, i, j );
					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						if( !!( m_nCheckBlockType & ( 1 << iLayer ) ) && context.GetBlock( x, y, iLayer ) )
							grid = 1;
						else if( m_nCheckGenData >= 0 && context.blueprint[x + y * context.nWidth] != m_nCheckGenData )
							grid = 1;
						else if( nCheckGenData2 > nCheckGenData1 )
						{
							if( context.blueprint[x + y * context.nWidth] < nCheckGenData1 ||
								context.blueprint[x + y * context.nWidth] > nCheckGenData2 )
								grid = 1;
						}
						else if( nCheckGenData1 >= 0 && context.blueprint[x + y * context.nWidth] != nCheckGenData1 )
							grid = 1;
					}
				}
			}
		}

		for( auto& item : m_infos )
		{
			for( int j = 0; j < size.y; j++ )
			{
				for( int i = 0; i < size.x; i++ )
				{
					if( GET_GRID( vecTemp, i, j ) == 1 )
						continue;
					auto rect = PutRect( vecTemp, size.x, size.y, TVector2<int32>( i, j ), item.sizeMin, item.sizeMax, TRectangle<int32>( 0, 0, size.x, size.y ), -1, 1 );
					if( rect.width > 0 )
					{
						item.pNode->Generate( context, rect.Offset( TVector2<int32>( region.x, region.y ) ) );
					}
				}
			}

		}
	}
protected:
	uint8 m_nCheckBlockType;
	int32 m_nCheckGenData;
	string m_strCheckGenData;
	string m_strCheckGenData1;
	vector<SSubNodeInfo> m_infos;
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
		bool bFlipX = SRand::Inst().Rand( 0, 2 );
		bool bFlipY = SRand::Inst().Rand( 0, 2 );
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
	REGISTER_GENERATE_NODE( "designed_level", CLevelGenerateDesignedLevelNode );
	REGISTER_GENERATE_NODE( "simple_attach", CLevelGenerateAttachNode );
	REGISTER_GENERATE_NODE( "simple_spawn", CLevelGenerateSpawnNode );
	REGISTER_GENERATE_NODE( "chain", CLevelGenerateChainNode );
	REGISTER_GENERATE_NODE( "filltag", CLevelGenerateFillTagNode );
	REGISTER_GENERATE_NODE( "scrollobj", CLevelGenerateScrollObjNode );
	REGISTER_GENERATE_NODE( "tile", CLevelGenerateTileNode );
	REGISTER_GENERATE_NODE( "subregion", CLevelGenerateSubRegionNode );
	REGISTER_GENERATE_NODE( "randompick", CLevelGenerateRandomPickNode );
	REGISTER_GENERATE_NODE( "switch", CLevelGenerateSwitchNode );
	REGISTER_GENERATE_NODE( "switch_tile", CLevelGenerateSwitchTileNode );
	REGISTER_GENERATE_NODE( "frame", CLevelGenerateFrameNode );
	REGISTER_GENERATE_NODE( "autofill", CLevelAutoFillGenerateNode );
	REGISTER_GENERATE_NODE( "randomfill", CLevelRandomFillGenerateNode );

	REGISTER_GENERATE_NODE( "mainmenu", CMainMenuGenerateNode );

	REGISTER_GENERATE_NODE( "bricktile", CBrickTileNode );
	REGISTER_GENERATE_NODE( "ramdomtile1", CRandomTileNode1 );
	REGISTER_GENERATE_NODE( "bar_fill", CBarFillNode );
	REGISTER_GENERATE_NODE( "commonroom", CCommonRoomNode );
	REGISTER_GENERATE_NODE( "room0", CRoom0Node );
	REGISTER_GENERATE_NODE( "room1", CRoom1Node );
	REGISTER_GENERATE_NODE( "room2", CRoom2Node );
	REGISTER_GENERATE_NODE( "billboard", CBillboardNode );
	REGISTER_GENERATE_NODE( "pipes", CPipeNode );
	REGISTER_GENERATE_NODE( "split", CSplitNode );
	REGISTER_GENERATE_NODE( "house", CHouseNode );
	REGISTER_GENERATE_NODE( "fence", CFenceNode );
	REGISTER_GENERATE_NODE( "fiber", CFiberNode );
	REGISTER_GENERATE_NODE( "control_room", CControlRoomNode );

	REGISTER_GENERATE_NODE( "lv1type1", CLevelGenNode1_1 );
	REGISTER_GENERATE_NODE( "lv1type1_0", CLevelGenNode1_1_0 );
	REGISTER_GENERATE_NODE( "lv1type1_1", CLevelGenNode1_1_1 );
	REGISTER_GENERATE_NODE( "lv1type1_2", CLevelGenNode1_1_2 );
	REGISTER_GENERATE_NODE( "lv1type1_3", CLevelGenNode1_1_3 );
	REGISTER_GENERATE_NODE( "lv1type2", CLevelGenNode1_2 );
	REGISTER_GENERATE_NODE( "lv1type3", CLevelGenNode1_3 );
	REGISTER_GENERATE_NODE( "lv1type3_0", CLevelGenNode1_3_0 );
	REGISTER_GENERATE_NODE( "lv1bonus0", CLevelBonusGenNode1_0 );
	REGISTER_GENERATE_NODE( "lv1bonus1", CLevelBonusGenNode1_1 );
	REGISTER_GENERATE_NODE( "lv1bonus2", CLevelBonusGenNode1_2 );

	REGISTER_GENERATE_NODE( "lv2type1_0", CLevelGenNode2_1_0 );
	REGISTER_GENERATE_NODE( "lv2type1_1", CLevelGenNode2_1_1 );
	REGISTER_GENERATE_NODE( "lv2type2_0", CLevelGenNode2_2_0 );
	REGISTER_GENERATE_NODE( "lv2type2_1", CLevelGenNode2_2_1 );
	REGISTER_GENERATE_NODE( "lv2type2_2", CLevelGenNode2_2_2 );
	REGISTER_GENERATE_NODE( "lv2bonus0", CLevelBonusGenNode2_0 );
	REGISTER_GENERATE_NODE( "lv2bonus1", CLevelBonusGenNode2_1 );
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

SChunk* CLevelGenerateNode::AddSubChunk( CChunkObject* pChunkObject, const TRectangle<int32>& region )
{
	SLevelBuildContext context( region.width, region.height );
	Generate( context, TRectangle<int32>( 0, 0, region.width, region.height ) );
	context.Build();

	assert( context.chunks.size() == 1 );
	auto pChunk = context.chunks[0];
	assert( pChunk->nWidth == region.width && pChunk->nHeight == region.height );

	pChunk->pos.x = region.x * 32;
	pChunk->pos.y = region.y * 32;
	pChunk->bIsSubChunk = true;
	pChunk->nSubChunkType = 2;
	pChunkObject->GetChunk()->Insert_SubChunk( pChunk );
	pChunk->CreateChunkObject( CMyLevel::GetInst(), pChunkObject->GetChunk() );
	return pChunk;
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
		szDefaultType = "scrollobj";
		auto pScrollObjRoot = pLevelGenRoot->FirstChildElement( "scrollobjs" );
		if( pScrollObjRoot )
		{
			for( auto pItem = pScrollObjRoot->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
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