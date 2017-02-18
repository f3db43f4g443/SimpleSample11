#include "stdafx.h"
#include "LevelGenerate.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "GUI/MainUI.h"
#include "Common/ResourceManager.h"

SLevelBuildContext::SLevelBuildContext( CMyLevel* pLevel ) : pLevel( pLevel )
{
	blocks.resize( pLevel->m_nWidth * pLevel->m_nHeight );
	chunks.resize( pLevel->m_nWidth * pLevel->m_nHeight );
	for( int i = 0; i < ELEM_COUNT( attachedPrefabs ); i++ )
		attachedPrefabs[i].resize( pLevel->m_nWidth * pLevel->m_nHeight );
}

SChunk* SLevelBuildContext::CreateChunk( SChunkBaseInfo& baseInfo, const TRectangle<int32>& region )
{
	if( region.x < 0 || region.GetRight() > pLevel->m_nWidth )
		return NULL;
	if( region.y < 0 || region.GetBottom() > pLevel->m_nHeight )
		return NULL;
	for( int j = region.y; j < region.GetBottom(); j++ )
	{
		for( int i = region.x; i < region.GetRight(); i++ )
		{
			if( blocks[i + j * pLevel->m_nWidth] )
				return NULL;
		}
	}

	auto pChunk = new SChunk( baseInfo, TVector2<int32>( region.x * pLevel->m_nBlockSize, region.y * pLevel->m_nBlockSize ), region.GetSize() );
	chunks.push_back( pChunk );
	for( int j = 0; j < pChunk->nHeight; j++ )
	{
		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			blocks[i + region.x + ( j + region.y ) * pLevel->m_nWidth] = &pChunk->blocks[i + j * pChunk->nWidth];
		}
	}

	return pChunk;
}

void SLevelBuildContext::AttachPrefab( CPrefab* pPrefab, TRectangle<int32> rect, uint8 nType )
{
	auto r = rect;
	if( nType > 0 )
		r.width = r.height = 1;
	for( int j = r.y; j < r.GetBottom(); j++ )
	{
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			if( attachedPrefabs[nType][i + j * pLevel->m_nWidth].first )
				return;
		}
	}

	for( int j = r.y; j < r.GetBottom(); j++ )
	{
		for( int i = r.x; i < r.GetRight(); i++ )
		{
			attachedPrefabs[nType][i + j * pLevel->m_nWidth].first = pPrefab;
			attachedPrefabs[nType][i + j * pLevel->m_nWidth].second = rect;
		}
	}
}

void SLevelBuildContext::AddSpawnInfo( SChunkSpawnInfo * pInfo, const TVector2<int32> ofs )
{
	auto pBlock = blocks[ofs.x + ofs.y * pLevel->m_nWidth];
	if( !pBlock )
	{
		delete pInfo;
		return;
	}

	pBlock->pOwner->Insert_SpawnInfo( pInfo );
}

void SLevelBuildContext::Build()
{
	vector<SBlock*> pPreBlocks;
	pPreBlocks.resize( pLevel->m_nWidth );

	SChunk* pLevelBarrier = NULL;
	for( auto pBlock = pLevel->m_basements[0].Get_Block(); pBlock; pBlock = pBlock->NextBlock() )
	{
		if( pBlock->pOwner->bIsLevelBarrier )
		{
			pLevelBarrier = pBlock->pOwner;
			break;
		}
	}
	int32 nYOfs = pLevelBarrier ? pLevelBarrier->pos.y + pLevelBarrier->nHeight * pLevel->GetBlockSize() : 0;
	int32 nGridOfs = nYOfs / pLevel->GetBlockSize();
	if( pLevelBarrier )
	{
		for( int i = 0; i < pLevel->m_nWidth; i++ )
		{
			pPreBlocks[i] = pLevelBarrier->GetBlock( i, 0 );
		}
	}

	auto pMainUI = CMainUI::GetInst();

	for( int j = 0; j < pLevel->m_nHeight; j++ )
	{
		for( int i = 0; i < pLevel->m_nWidth; i++ )
		{
			auto pBlock = blocks[i + j * pLevel->m_nWidth];
			if( pBlock )
			{
				if( pMainUI )
					pMainUI->UpdateMinimap( i, j + nGridOfs, CMyLevel::s_nTypes[pBlock->pBaseInfo->eBlockType] );
			}
			if( pBlock && pBlock->nY == 0 )
			{
				pBlock->pOwner->pos.y += nYOfs;
				uint32 nBasement = i;
				auto pPreBlock = pPreBlocks[nBasement];
				if( pPreBlock )
				{
					pPreBlock->InsertAfter_Block( pBlock );
				}
				else
				{
					pLevel->m_basements[nBasement].Insert_Block( pBlock );
				}
				pPreBlocks[nBasement] = pBlock;
			}
		}
	}

	for( int j = 0; j < pLevel->m_nHeight; j++ )
	{
		for( int i = 0; i < pLevel->m_nWidth; i++ )
		{
			auto pBlock = blocks[i + j * pLevel->m_nWidth];
			if( pBlock )
			{
				for( int k = 0; k < ELEM_COUNT( attachedPrefabs ); k++ )
				{
					if( k == SBlock::eAttachedPrefab_Lower )
					{
						if( j <= 0 )
							continue;
						auto pLowerBlock = blocks[i + ( j - 1 ) * pLevel->m_nWidth];
						if( pLowerBlock && pLowerBlock->pBaseInfo->eBlockType != eBlockType_Wall )
							continue;
					}
					if( k == SBlock::eAttachedPrefab_Upper )
					{
						if( j >= pLevel->m_nHeight - 1 )
							continue;
						auto pUpperBlock = blocks[i + ( j + 1 ) * pLevel->m_nWidth];
						if( pUpperBlock && pUpperBlock->pBaseInfo->eBlockType != eBlockType_Wall )
							continue;
					}

					auto attachedPrefab = attachedPrefabs[k][i + j * pLevel->m_nWidth];
					if( attachedPrefab.second.x == i && attachedPrefab.second.y == j )
					{
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

SBlock* SLevelBuildContext::GetBlock( uint32 x, uint32 y )
{
	return blocks[x + y * pLevel->m_nWidth];
}

void CLevelGenerateNode::Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context )
{
	auto pNextLevel = pXml->FirstChildElement( "next_level" );
	if( pNextLevel )
	{
		m_pNextLevel = CreateNode( pNextLevel->FirstChildElement(), context );
		m_fNextLevelChance = XmlGetAttr( pNextLevel, "p", 1.0f );
	}
}

void CLevelGenerateNode::Generate( SLevelBuildContext& context, const TRectangle<int32>& region )
{
	if( m_pNextLevel )
	{
		if( SRand::Inst().Rand( 0.0f, 1.0f ) < m_fNextLevelChance )
			m_pNextLevel->Generate( context, region );
	}
}

#define GET_GRID( g, i, j ) ( g[(i) + (j) * size.x] )

class CLevelGenerateSimpleNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override
	{
		auto& chunk = context.mapChunkBaseInfo[XmlGetAttr( pXml, "name", "" )];
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
		chunk.bIsRoom = XmlGetAttr( pXml, "isroom", 0 );
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

		for( auto pSubItem = pXml->FirstChildElement( "subitem" ); pSubItem; pSubItem = pSubItem->NextSiblingElement( "subitem" ) )
		{
			const char* szName = XmlGetAttr( pSubItem, "name", "" );
			auto itr = context.mapChunkBaseInfo.find( szName );
			if( itr == context.mapChunkBaseInfo.end() )
				continue;
			int32 x = XmlGetAttr( pSubItem, "x", 0 );
			int32 y = XmlGetAttr( pSubItem, "y", 0 );
			int32 arrayx = XmlGetAttr( pSubItem, "arrayx", 1 );
			int32 arrayy = XmlGetAttr( pSubItem, "arrayy", 1 );
			for( int i = 0; i < arrayx; i += itr->second.nWidth )
			{
				for( int j = 0; j < arrayy; j += itr->second.nHeight )
				{
					chunk.subInfos.push_back( pair<SChunkBaseInfo*, TVector2<int32> >( &itr->second, TVector2<int32>( x + i, y + j ) ) );
				}
			}
		}

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
		if( pChunk )
		{
			pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
			CLevelGenerateNode::Generate( context, region );
		}
	}
protected:
	SChunkBaseInfo* m_pChunkBaseInfo;
	bool m_bIsLevelBarrier;
};

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
		rect = rect * context.pLevel->GetBlockSize();
		rect.SetLeft( rect.x - m_rectSize.x );
		rect.SetTop( rect.y - m_rectSize.y );
		rect.SetRight( rect.GetRight() - m_rectSize.GetRight() );
		rect.SetBottom( rect.GetBottom() - m_rectSize.GetBottom() );

		uint32 nCount = region.width * region.height * m_fCountPerGrid;

		for( int i = 0; i < nCount; i++ )
		{
			SChunkSpawnInfo* pSpawnInfo = new SChunkSpawnInfo;
			pSpawnInfo->pos = CVector2( SRand::Inst().Rand( rect.GetLeft(), rect.GetRight() ), SRand::Inst().Rand( rect.GetRight(), rect.GetBottom() ) );
			pSpawnInfo->pPrefab = m_pPrefab;
			pSpawnInfo->r = 0;
			context.AddSpawnInfo( pSpawnInfo, TVector2<int32>( region.x, region.y ) );
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
		m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( XmlGetAttr( pXml, "prefab", "" ) );
		m_size.x = XmlGetAttr( pXml, "sizex", 1 );
		m_size.y = XmlGetAttr( pXml, "sizey", 1 );
		m_nType = Min<uint8>( SBlock::eAttachedPrefab_Count, XmlGetAttr( pXml, "attach_type", 0 ) );

		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
		for( int j = region.y; j < region.GetBottom(); j += m_size.y )
		{
			for( int i = region.x; i < region.GetRight(); i += m_size.x )
			{
				context.AttachPrefab( m_pPrefab, TRectangle<int32>( i, j, m_size.x, m_size.y ), m_nType );
			}
		}
		CLevelGenerateNode::Generate( context, region );
	}
protected:
	CReference<CPrefab> m_pPrefab;
	TVector2<int32> m_size;
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
				if( subRegion.width == -1 )
				{
					subRegion.width = Max( region.width - subRegion.x, 0 );
				}
				if( subRegion.y < 0 )
				{
					subRegion.y = region.height + subRegion.y;
				}
				if( subRegion.height == -1 )
				{
					subRegion.height = Max( region.height - subRegion.y, 0 );
				}
				subRegion = subRegion.Offset( TVector2<int32>( region.x, region.y ) );
				item.pNode->Generate( context, subRegion );
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
				return;
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
			item.fWeight = XmlGetAttr( pChild, "p", 1.0f );
		}
		m_bCheckBlock = XmlGetAttr( pXml, "check_block", 0 );

		float fTotalWeight = 0;
		for( int i = m_infos.size() - 1; i >= 0; i-- )
		{
			auto& item = m_infos[i];
			fTotalWeight += item.fWeight;
			item.fWeightRecalculated = item.fWeight / fTotalWeight;
		}
		CLevelGenerateNode::Load( pXml, context );
	}
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override
	{
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
		if( m_bCheckBlock )
		{
			for( int j = 0; j < size.y; j++ )
			{
				for( int i = 0; i < size.x; i++ )
				{
					int32 x = ( bFlipX ? size.x - 1 - i : i ) + region.x;
					int32 y = ( bFlipY ? size.y - 1 - j : j ) + region.y;
					GET_GRID( genResult, i, j ).first = context.GetBlock( x, y ) ? &dummy : NULL;
				}
			}
		}

		for( auto& subNodeInfo : m_infos )
		{
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
	bool m_bCheckBlock;
	vector<SSubNodeInfo> m_infos;
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
		auto itr = context.mapNamedNodes.find( XmlGetAttr( pXml, "name", "" ) );
		if( itr != context.mapNamedNodes.end() )
			return itr->second;
		return NULL;
	}
	else if( !strcmp( pXml->Value(), "node" ) )
	{
		auto pNode = CLevelGenerateFactory::Inst().LoadNode( pXml, context );
		if( !pNode )
			return NULL;
		const char* szName = XmlGetAttr( pXml, "name", "" );
		if( szName[0] )
			context.mapNamedNodes[szName] = pNode;
		return pNode;
	}
	return NULL;
}
