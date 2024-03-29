#include "stdafx.h"
#include "Block.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Common/ResourceManager.h"
#include "Entities/Barrage.h"
#include "Entities/EffectObject.h"
#include "Stage.h"
#include "BlockItem.h"
#include "Entities/Decorator.h"
#include "BlockBuff.h"
#include "Render/Rope2D.h"

CBlockObject::CBlockObject( SBlock* pBlock, CEntity* pParent, CMyLevel* pLevel )
	: m_pBlock( pBlock ), m_nBlockRTIndex( -1 ), m_bBlockRTActive( false )
{
	SET_BASEOBJECT_ID( CBlockObject );

	SetPosition( CVector2( pBlock->nX, pBlock->nY ) * CMyLevel::GetBlockSize() );
	if( pBlock->eBlockType == eBlockType_Block || pBlock->eBlockType == eBlockType_LowBlock )
	{
		AddRect( CRectangle( 0, 0, CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize() ) );
		SetHitType( eEntityHitType_WorldStatic );
	}
	else
	{
		AddRect( CRectangle( 0, 0, CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize() ) );
		SetHitType( eEntityHitType_Sensor );
	}
	SetParentEntity( pParent );
}

CBlockObject::CBlockObject( SBlock* pBlock, CEntity* pParent, uint32 nSize )
	: m_pBlock( pBlock ), m_nBlockRTIndex( -1 ), m_bBlockRTActive( false )
{
	SET_BASEOBJECT_ID( CBlockObject );

	SetPosition( CVector2( pBlock->nX, pBlock->nY ) * nSize );
	SetParentEntity( pParent );
}

void CBlockObject::OnRemovedFromStage()
{
	CMyLevel::GetInst()->OnBlockObjectRemoved( this );
}

void CBlockObject::ClearBuffs()
{
	LINK_LIST_FOR_EACH_BEGIN( pChild, m_pChildrenEntity, CEntity, ChildEntity )
		auto pBuff = SafeCast<CBlockBuff>( pChild );
		if( pBuff )
			pBuff->SetParentEntity( NULL );
	LINK_LIST_FOR_EACH_END( pChild, m_pChildrenEntity, CEntity, ChildEntity )
}

void CBlockObject::ClearEfts()
{
	CMyLevel::GetInst()->OnBlockObjectRemoved( this );
	if( m_pDetectUI )
		m_pDetectUI->RemoveThis();
}


SChain::SChain( SChainBaseInfo & baseInfo, int32 x, int32 y1, int32 y2 )
	: p1( NULL ), p2( NULL ), nX( x ), nY1( y1 ), nY2( y2 ), nMaxLen( 0 )
	, nLayer( baseInfo.nLayer ), pPrefab( baseInfo.pPrefab )
	, pChainObject( NULL ), pEf2( NULL ), nYLim( 0 )
{
}

SChain::SChain( SChainBaseInfo& baseInfo, SChunk* a, SChunk* b, int32 x, int32 y1, int32 y2 )
	: p1( a ), p2( b ), nX( x ), nY1( y1 ), nY2( y2 )
	, nLayer( baseInfo.nLayer ), pPrefab( baseInfo.pPrefab )
	, pChainObject( NULL ), pEf1( NULL ), pEf2( NULL )
{
	y1 = nY1 * CMyLevel::GetBlockSize() + p1->pos.y;
	y2 = nY2 * CMyLevel::GetBlockSize() + p2->pos.y;
	p1->Insert_Chain2( this );
	p2->Insert_Chain1( this );
	SChunk* pRoot1 = p1;
	while( pRoot1->pParentChunk && pRoot1->nSubChunkType >= 1 )
	{
		pRoot1 = pRoot1->pParentChunk;
		y1 += pRoot1->pos.y;
	}
	SChunk* pRoot2 = p2;
	while( pRoot2->pParentChunk && pRoot2->nSubChunkType >= 1 )
	{
		pRoot2 = pRoot2->pParentChunk;
		y2 += pRoot2->pos.y;
	}
	nMaxLen = y2 - y1 + baseInfo.nLen;
	Init();
}

void SChain::Init()
{
	if( pEf1 )
	{
		pEf1 = NULL;
		RemoveFrom_ChainEf2();
	}
	if( pEf2 )
	{
		pEf2 = NULL;
		RemoveFrom_ChainEf1();
	}

	int32 y1 = nY1 * CMyLevel::GetBlockSize(), y2 = nY2 * CMyLevel::GetBlockSize();
	SChunk* pRoot1 = p1;
	while( pRoot1->pParentChunk && pRoot1->nSubChunkType >= 1 )
	{
		y1 += pRoot1->pos.y;
		pRoot1 = pRoot1->pParentChunk;
	}
	SChunk* pRoot2 = p2;
	while( pRoot2->pParentChunk && pRoot2->nSubChunkType >= 1 )
	{
		y2 += pRoot2->pos.y;
		pRoot2 = pRoot2->pParentChunk;
	}
	pEf1 = pRoot1;
	pEf2 = pRoot2;
	pRoot1->Insert_ChainEf2( this );
	pRoot2->Insert_ChainEf1( this );

	nYLim = nMaxLen + y1 - y2;
	if( pChainObject )
		pChainObject->Update();
}

void SChain::CreateChainObject( CMyLevel* pLevel )
{
	if( pChainObject )
		return;
	auto pChainObject = SafeCast<CChainObject>( pPrefab->GetRoot()->CreateInstance() );
	pChainObject->SetChain( this, pLevel );
}

void SChain::CreateChainObjectPreview( CEntity* pRootEntity )
{
	if( pChainObject )
		return;
	auto pChainObject = SafeCast<CChainObject>( pPrefab->GetRoot()->CreateInstance() );
	pChainObject->Preview( this, pRootEntity );
}

void SChain::ForceDestroy()
{
	if( p1 )
	{
		RemoveFrom_Chain1();
		p1 = NULL;
	}
	if( p2 )
	{
		RemoveFrom_Chain2();
		p2 = NULL;
	}
	delete this;
}

void CChainObject::SetChain( SChain* pChain, CMyLevel* pLevel )
{
	m_pChain = pChain;
	pChain->pChainObject = this;
	SetParentBeforeEntity( pChain->nLayer == 0 ? CMyLevel::GetInst()->GetChunkRoot() : CMyLevel::GetInst()->GetChunkRoot1() );
	if( m_pChainTilePrefab )
	{
		int32 nChain = pChain->nMaxLen < m_nChainTileBegin ? 0 : ( pChain->nMaxLen - m_nChainTileBegin ) / m_nChainTileLen + 1;
		m_vecChainTileObject.resize( nChain );
	}
	Update();
}

void CChainObject::Preview( SChain* pChain, CEntity * pParent )
{
	m_pChain = pChain;
	pChain->pChainObject = this;
	SetParentEntity( pParent );

	auto pRope = static_cast<CRopeObject2D*>( GetRenderObject() );
	auto& data = pRope->GetData();
	auto& begin = data.data[0];
	auto& end = data.data[1];
	begin.center.x = end.center.x = ( m_pChain->nX + 0.5f ) * CMyLevel::GetBlockSize();
	begin.center.y = m_pChain->nY1 * CMyLevel::GetBlockSize();
	end.center.y = m_pChain->nY2 * CMyLevel::GetBlockSize();
	if( m_fTexYTileLen > 0 )
		end.tex0.y = end.tex1.y = ( m_pChain->nY2 - m_pChain->nY1 ) * CMyLevel::GetBlockSize() / m_fTexYTileLen;
	if( m_nEftType == 1 )
		begin.tex1.x = end.tex1.x = end.tex0.y;
	pRope->SetTransformDirty();
}

void CChainObject::RemoveChain()
{
	if( m_pChain && CMyLevel::GetInst() )
	{
		auto pChain = m_pChain;
		m_pChain = NULL;
		pChain->pChainObject = NULL;
		CMyLevel::GetInst()->RemoveChain( pChain );
	}
}

void CChainObject::Update()
{
	auto pRope = static_cast<CRopeObject2D*>( GetRenderObject() );
	auto& data = pRope->GetData();
	auto& begin = data.data[0];
	auto& end = data.data[1];
	begin.center.x = end.center.x = ( m_pChain->nX + 0.5f ) * CMyLevel::GetBlockSize();

	int32 y1 = m_pChain->nY1 * CMyLevel::GetBlockSize() + m_pChain->p1->pos.y,
		y2 = m_pChain->nY2 * CMyLevel::GetBlockSize() + m_pChain->p2->pos.y;
	SChunk* pRoot1 = m_pChain->p1;
	while( pRoot1->pParentChunk && pRoot1->nSubChunkType >= 1 )
	{
		pRoot1 = pRoot1->pParentChunk;
		y1 += pRoot1->pos.y;
	}
	SChunk* pRoot2 = m_pChain->p2;
	while( pRoot2->pParentChunk && pRoot2->nSubChunkType >= 1 )
	{
		pRoot2 = pRoot2->pParentChunk;
		y2 += pRoot2->pos.y;
	}
	begin.center.y = y1;
	end.center.y = y2;

	if( m_fTexYTileLen > 0 )
		end.tex0.y = end.tex1.y = ( y2 - y1 ) / m_fTexYTileLen;
	if( m_nEftType == 1 )
		begin.tex1.x = end.tex1.x = end.tex0.y;
	pRope->SetTransformDirty();

	for( int i = 0; i < m_vecChainTileObject.size(); i++ )
	{
		auto& item = m_vecChainTileObject[i];
		if( item.nState == 1 && !item.pEntity->GetStage() )
		{
			item.nState = 2;
			item.pEntity = NULL;
		}
		if( item.nState == 2 )
			continue;

		int32 y = y1 + m_nChainTileBegin + m_nChainTileLen * i;
		if( y > y2 )
		{
			if( item.nState == 1 )
			{
				item.nState = 0;
				item.pEntity->SetParentEntity( NULL );
			}
		}
		else
		{
			if( item.nState == 0 )
			{
				if( !item.pEntity )
					item.pEntity = SafeCast<CEntity>( m_pChainTilePrefab->GetRoot()->CreateInstance() );
				item.nState = 1;
				item.pEntity->SetParentEntity( this );
			}
			item.pEntity->SetPosition( CVector2( begin.center.x, y ) );
		}
	}
}

SChunk::SChunk( const SChunkBaseInfo& baseInfo, const TVector2<int32>& pos, const TVector2<int32>& size )
	: nWidth( size.x )
	, nHeight( size.y )
	, fWeight( baseInfo.fWeight + baseInfo.fWeightPerWidth * (int32)( size.x - baseInfo.nWidth ) )
	, fDestroyWeight( baseInfo.fDestroyWeight + baseInfo.fDestroyWeightPerWidth * (int32)( size.x - baseInfo.nWidth ) )
	, fDestroyBalance( baseInfo.fDestroyBalance )
	, fImbalanceTime( baseInfo.fImbalanceTime )
	, fShakeDmg( baseInfo.fShakeDmg + baseInfo.fShakeDmgPerWidth * (int32)( size.x - baseInfo.nWidth ) )
	, fShakeWeightCoef( baseInfo.fShakeWeightCoef )
	, nShakeDmgThreshold( baseInfo.nShakeDmgThreshold )
	, nAbsorbShakeStrength( baseInfo.nAbsorbShakeStrength + baseInfo.fAbsorbShakeStrengthPerHeight * (int32)( size.y - baseInfo.nHeight ) )
	, nDestroyShake( baseInfo.nDestroyShake )
	, pos( pos )
	, nFallSpeed( 0 )
	, pPrefab( baseInfo.pPrefab )
	, pChunkObject( NULL )
	, nUpdateCount( 0 )
	, fCurImbalanceTime( 0 )
	, nCurShakeStrength( 0 )
	, nLayerType( baseInfo.nLayerType )
	, bInvulnerable( 0 )
	, bIsRoom( baseInfo.bIsRoom )
	, nMoveType( baseInfo.nMoveType )
	, nShowLevelType( baseInfo.nShowLevelType )
	, nLevelBarrierType( false )
	, bStopMove( false )
	, bForceStop( false )
	, bSpawned( false )
	, bIsSubChunk( false )
	, bMovedLastFrame( false )
	, bIsBeingRepaired( false )
	, nSubChunkType( baseInfo.nSubChunkType )
	, nVisitFlag( 0 )
	, nChunkTag( baseInfo.nChunkTag )
	, pParentChunk( NULL )
	, m_pSubChunks( NULL )
	, m_pSpawnInfos( NULL )
	, m_pChunkStopEvents( NULL )
	, m_pChain1( NULL )
	, m_pChain2( NULL )
	, m_pChainEf1( NULL )
	, m_pChainEf2( NULL )
{
	blocks.resize( size.x * size.y );
	for( int j = 0; j < size.y; j++ )
	{
		for( int i = 0; i < size.x; i++ )
		{
			auto& block = blocks[i + j * nWidth];
			block.nX = i;
			block.nY = j;
			block.pOwner = this;
			int x = i == size.x - 1 ? baseInfo.nWidth - 1 : Max<int32>( 0, Min<int32>( i, baseInfo.nWidth - 2 ) );
			int y = j == size.y - 1 ? baseInfo.nHeight - 1 : Max<int32>( 0, Min<int32>( j, baseInfo.nHeight - 2 ) );
			auto& blockInfo = baseInfo.blockInfos[x + y * baseInfo.nWidth];
			block.eBlockType = blockInfo.eBlockType;
			block.nTag = blockInfo.nTag;
			block.fDmgPercent = blockInfo.fDmgPercent;
			block.nRail = blockInfo.nRail;
			block.bImmuneToBlockBuff = blockInfo.bImmuneToBlockBuff;
		}
	}
}

SChunk::~SChunk()
{
	while( Get_StopEvent() )
	{
		CReference<CChunkStopEvent> pStopEvent = Get_StopEvent();
		pStopEvent->RemoveFrom_StopEvent();
	}
	while( Get_SpawnInfo() )
	{
		auto pSpawnInfo = Get_SpawnInfo();
		pSpawnInfo->RemoveFrom_SpawnInfo();
		delete pSpawnInfo;
	}
}

bool SChunk::CreateChunkObject( CMyLevel* pLevel, SChunk* pParent )
{
	if( pParent && pChunkObject )
		return false;

	if( !pChunkObject )
	{
		auto pPreParent = pParentChunk ? pParentChunk : pParent;
		pParentChunk = pParent;
		pChunkObject = SafeCast<CChunkObject>( pPrefab->GetRoot()->CreateInstance() );
		pChunkObject->SetChunk( this, pLevel );

		for( auto pChain = Get_Chain2(); pChain; pChain = pChain->NextChain2() )
		{
			pChain->CreateChainObject( pLevel );
		}

		if( Get_SpawnInfo() )
		{
			CVector2 spawnPos = pChunkObject->GetPosition();
			CChunkObject* pChunkObject1 = pChunkObject;
			while( true )
			{
				CChunkObject* pChunkObject2 = SafeCast<CChunkObject>( pChunkObject1->GetParentEntity() );
				if( !pChunkObject2 )
					break;
				spawnPos = spawnPos + pChunkObject2->GetPosition();
				pChunkObject1 = pChunkObject2;
			}

			while( Get_SpawnInfo() )
			{
				auto pSpawnInfo = Get_SpawnInfo();
				CReference<CEntity> pEntity = SafeCast<CEntity>( pSpawnInfo->pPrefab->GetRoot()->CreateInstance() );

				auto pDecorator = SafeCast<CDecorator>( pEntity.GetPtr() );
				if( pDecorator )
				{
					if( pChunkObject->m_p1 )
						pDecorator->SetParentEntity( pChunkObject->m_p1 );
					else
						pDecorator->SetParentBeforeEntity( pChunkObject->GetRenderObject() );
					pDecorator->SetPosition( CVector2( pSpawnInfo->rect.x, pSpawnInfo->rect.y ) );
					pDecorator->Init( CVector2( pSpawnInfo->rect.width, pSpawnInfo->rect.height ), pPreParent );
				}
				else if( pSpawnInfo->bForceAttach )
				{
					pEntity->SetPosition( CVector2( pSpawnInfo->rect.x, pSpawnInfo->rect.y ) );
					pEntity->SetRotation( pSpawnInfo->r );
					pEntity->SetParentBeforeEntity( pChunkObject->GetRenderObject() );
				}
				else
				{
					pEntity->SetPosition( CVector2( pSpawnInfo->rect.x, pSpawnInfo->rect.y ) + spawnPos );
					pEntity->SetRotation( pSpawnInfo->r );
					pEntity->SetParentBeforeEntity( pChunkObject1->GetParentEntity() );
				}

				pSpawnInfo->RemoveFrom_SpawnInfo();
				delete pSpawnInfo;
			}
		}
	}
	else
	{
		if( !pParentChunk )
			return false;
		pParentChunk = NULL;
		pChunkObject->SetChunk( this, pLevel );
		pChunkObject->ClearHitShake();
	}

	for( auto pSubChunk = Get_SubChunk(); pSubChunk; )
	{
		if( pSubChunk->nSubChunkType >= 1 )
		{
			pSubChunk->bIsSubChunk = true;
			pSubChunk->CreateChunkObject( pLevel, this );
		}
		auto pNext = pSubChunk->NextSubChunk();
		pSubChunk = pNext;
	}
	pChunkObject->OnCreateComplete( pLevel );
	return true;
}

void SChunk::CreateChunkObjectPreview( CEntity* pRootEntity, SChunk* pParent )
{
	pParentChunk = pParent;
	pChunkObject = SafeCast<CChunkObject>( pPrefab->GetRoot()->CreateInstance() );
	pChunkObject->Preview( this, pRootEntity );

	for( auto pChain = Get_Chain2(); pChain; pChain = pChain->NextChain2() )
	{
		pChain->CreateChainObjectPreview( pRootEntity );
	}

	for( auto pSpawnInfo = m_pSpawnInfos; pSpawnInfo; pSpawnInfo = pSpawnInfo->NextSpawnInfo() )
	{
		if( pSpawnInfo->pPrefab->GetRoot()->GetStaticDataSafe<CDecorator>() )
		{
			CReference<CEntity> pEntity = SafeCast<CEntity>( pSpawnInfo->pPrefab->GetRoot()->CreateInstance() );

			auto pDecorator = SafeCast<CDecorator>( pEntity.GetPtr() );
			if( pDecorator )
			{
				if( pChunkObject->m_p1 )
					pDecorator->SetParentEntity( pChunkObject->m_p1 );
				else
					pDecorator->SetParentBeforeEntity( pChunkObject->GetRenderObject() );
				pDecorator->SetPosition( CVector2( pSpawnInfo->rect.x, pSpawnInfo->rect.y ) );
				pDecorator->Init( CVector2( pSpawnInfo->rect.width, pSpawnInfo->rect.height ), pParent );
			}
		}
	}

	for( auto pSubChunk = Get_SubChunk(); pSubChunk; )
	{
		if( pSubChunk->nSubChunkType >= 1 )
		{
			pSubChunk->bIsSubChunk = true;
			pSubChunk->CreateChunkObjectPreview( pRootEntity, this );
		}
		pSubChunk = pSubChunk->NextSubChunk();
	}
	pChunkObject->OnCreateComplete( NULL );
}

float SChunk::GetFallSpeed()
{
	return nFallSpeed * CMyLevel::GetInst()->GetFallDistPerSpeedFrame() * 60;
}

void SChunk::ForceDestroy()
{
	while( m_pChain1 )
		m_pChain1->ForceDestroy();
	while( m_pChain2 )
		m_pChain2->ForceDestroy();
	while( m_pSubChunks )
	{
		auto pChunk = m_pSubChunks;
		pChunk->RemoveFrom_SubChunk();
		pChunk->ForceDestroy();
	}
	delete this;
}

void CChunkObject::SetChunk( SChunk* pChunk, CMyLevel* pLevel )
{
	if( !m_pChunk )
	{
		m_pChunk = pChunk;
		pChunk->bSpawned = true;
		pChunk->pChunkObject = this;
		SetPosition( CVector2( pChunk->pos.x, pChunk->pos.y ) );
		if( pChunk->nMoveType )
		{
			AddRect( CRectangle( 0, 0, pChunk->nWidth * CMyLevel::GetBlockSize(), pChunk->nHeight * CMyLevel::GetBlockSize() ) );
			SetHitType( eEntityHitType_Sensor );
		}
		if( pChunk->pParentChunk )
		{
			SetParentEntity( pChunk->pParentChunk->pChunkObject );
			SetRenderParent( pChunk->nShowLevelType ? pLevel->GetChunkRoot1() : pLevel->GetChunkRoot() );
			if( pChunk->nShowLevelType == pChunk->pParentChunk->nShowLevelType )
				SetZOrder( 1 );
			if( pChunk->nSubChunkType <= 1 )
			{
				int32 x0 = pChunk->pos.x / CMyLevel::GetBlockSize();
				int32 y0 = pChunk->pos.y / CMyLevel::GetBlockSize();
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					for( int j = 0; j < pChunk->nHeight; j++ )
					{
						pChunk->pParentChunk->pChunkObject->GetChunk()->GetBlock( i + x0, j + y0 )->pEntity->SetRenderParentBefore( this );
					}
				}
			}
		}
		else
			SetParentEntity( pChunk->nShowLevelType ? pLevel->GetChunkRoot1() : pLevel->GetChunkRoot() );

		if( m_pDamagedEffectsRoot )
		{
			for( auto pChild = m_pDamagedEffectsRoot->Get_TransformChild(); pChild && m_nDamagedEffectsCount < 4; pChild = pChild->NextTransformChild() )
			{
				pChild->bVisible = false;
				m_pDamagedEffects[m_nDamagedEffectsCount++] = pChild;
			}
		}

		OnSetChunk( pChunk, pLevel );
	}
	else if( GetParentEntity() != pLevel->GetChunkRoot() && GetParentEntity() != pLevel->GetChunkRoot1() )
	{
		SetPosition( GetPosition() + GetParentEntity()->GetPosition() );
		SetParentEntity( pChunk->nLayerType > 1 ? pLevel->GetChunkRoot1() : pLevel->GetChunkRoot() );
	}

	bool b = pChunk->nSubChunkType == 1 && pChunk->pParentChunk;
	if( b )
	{
		if( !m_pTempBlock )
		{
			m_pTempBlock = new CEntity();
			m_pTempBlock->SetParentEntity( this );
		}
	}
	if( !GetChunk()->blocks[0].pEntity )
	{
		for( auto& block : pChunk->blocks )
		{
			if( !b )
			{
				if( m_pTempBlock )
				{
					block.pEntity = new CBlockObject( &block, NULL, pLevel );
					block.pEntity->SetParentBeforeEntity( m_pTempBlock );
					continue;
				}
				block.pEntity = new CBlockObject( &block, this, pLevel );
			}
			if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Center] )
			{
				auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Center]->GetRoot()->CreateInstance() );
				pEntity->SetPosition( CVector2( block.nX + block.attachedPrefabSize.x * 0.5f, block.nY + block.attachedPrefabSize.y * 0.5f ) * CMyLevel::GetBlockSize() );
				if( !!( block.nAttachType & 1 ) )
					pEntity->SetParentAfterEntity( b ? m_pTempBlock : GetChunk()->blocks[0].pEntity );
				else
					pEntity->SetParentEntity( this );
				block.pAttachedPrefab[SBlock::eAttachedPrefab_Center] = NULL;
			}

			if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Right] )
			{
				auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Right]->GetRoot()->CreateInstance() );
				pEntity->SetPosition( CVector2( block.nX + 1, block.nY + 0.5f ) * CMyLevel::GetBlockSize() );
				if( !!( block.nAttachType & 2 ) )
					pEntity->SetParentAfterEntity( b ? m_pTempBlock : GetChunk()->blocks[0].pEntity );
				else
					pEntity->SetParentEntity( this );
				block.pAttachedPrefab[SBlock::eAttachedPrefab_Right] = NULL;
			}

			if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Upper] )
			{
				auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Upper]->GetRoot()->CreateInstance() );
				pEntity->SetPosition( CVector2( block.nX + 0.5f, block.nY + 1 ) * CMyLevel::GetBlockSize() );
				if( !!( block.nAttachType & 4 ) )
					pEntity->SetParentAfterEntity( b ? m_pTempBlock : GetChunk()->blocks[0].pEntity );
				else
					pEntity->SetParentEntity( this );
				block.pAttachedPrefab[SBlock::eAttachedPrefab_Upper] = NULL;
			}

			if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Left] )
			{
				auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Left]->GetRoot()->CreateInstance() );
				pEntity->SetPosition( CVector2( block.nX, block.nY + 0.5f ) * CMyLevel::GetBlockSize() );
				if( !!( block.nAttachType & 8 ) )
					pEntity->SetParentAfterEntity( b ? m_pTempBlock : GetChunk()->blocks[0].pEntity );
				else
					pEntity->SetParentEntity( this );
				block.pAttachedPrefab[SBlock::eAttachedPrefab_Left] = NULL;
			}

			if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Lower] )
			{
				auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Lower]->GetRoot()->CreateInstance() );
				pEntity->SetPosition( CVector2( block.nX + 0.5f, block.nY ) * CMyLevel::GetBlockSize() );
				if( !!( block.nAttachType & 16 ) )
					pEntity->SetParentAfterEntity( b ? m_pTempBlock : GetChunk()->blocks[0].pEntity );
				else
					pEntity->SetParentEntity( this );
				block.pAttachedPrefab[SBlock::eAttachedPrefab_Lower] = NULL;
			}
		}
	}
	if( !b )
	{
		if( m_pTempBlock )
		{
			m_pTempBlock->SetParentEntity( NULL );
			m_pTempBlock = NULL;
		}
	}
}

void CChunkObject::Preview( SChunk* pChunk, CEntity* pParent )
{
	m_pChunk = pChunk;
	pChunk->bSpawned = true;
	pChunk->pChunkObject = this;
	SetPosition( CVector2( pChunk->pos.x, pChunk->pos.y ) );
	if( pChunk->pParentChunk )
	{
		if( pChunk->nSubChunkType <= 1 )
			SetParentAfterEntity( pChunk->pParentChunk->pChunkObject->GetChunk()->blocks[0].pEntity );
		else
			SetParentEntity( pChunk->pParentChunk->pChunkObject );
	}
	else
		SetParentEntity( pParent );

	if( m_pDamagedEffectsRoot )
	{
		for( auto pChild = m_pDamagedEffectsRoot->Get_TransformChild(); pChild && m_nDamagedEffectsCount < 4; pChild = pChild->NextTransformChild() )
		{
			pChild->bVisible = false;
			m_pDamagedEffects[m_nDamagedEffectsCount++] = pChild;
		}
	}

	OnSetChunk( pChunk, NULL );

	if( pChunk->nSubChunkType == 1 && pChunk->pParentChunk )
		return;

	for( auto& block : pChunk->blocks )
	{
		block.pEntity = new CBlockObject( &block, this, 32 );

		if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Center] )
		{
			auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Center]->GetRoot()->CreateInstance() );
			pEntity->SetPosition( CVector2( block.nX + block.attachedPrefabSize.x * 0.5f, block.nY + block.attachedPrefabSize.y * 0.5f ) * 32 );
			pEntity->SetParentEntity( this );
		}

		if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Right] )
		{
			auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Right]->GetRoot()->CreateInstance() );
			pEntity->SetPosition( CVector2( block.nX + 1, block.nY + 0.5f ) * 32 );
			pEntity->SetParentEntity( this );
		}

		if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Upper] )
		{
			auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Upper]->GetRoot()->CreateInstance() );
			pEntity->SetPosition( CVector2( block.nX + 0.5f, block.nY + 1 ) * 32 );
			pEntity->SetParentEntity( this );
		}

		if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Left] )
		{
			auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Left]->GetRoot()->CreateInstance() );
			pEntity->SetPosition( CVector2( block.nX, block.nY + 0.5f ) * 32 );
			pEntity->SetParentEntity( this );
		}

		if( block.pAttachedPrefab[SBlock::eAttachedPrefab_Lower] )
		{
			auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab[SBlock::eAttachedPrefab_Lower]->GetRoot()->CreateInstance() );
			pEntity->SetPosition( CVector2( block.nX + 0.5f, block.nY ) * 32 );
			pEntity->SetParentEntity( this );
		}
	}
}

void CChunkObject::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	if( GetRenderObject() )
	{
		auto pImage2D = SafeCast<CImage2D>( GetRenderObject() );
		if( pImage2D )
		{
			auto& texRect = pImage2D->GetElem().texRect;
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				for( int j = 0; j < pChunk->nHeight; j++ )
				{
					auto pBlock = GetBlock( i, j );
					auto& rtRect = pBlock->rtTexRect;
					rtRect = texRect;
					rtRect.width /= pChunk->nWidth;
					rtRect.height /= pChunk->nHeight;
					rtRect.x += rtRect.width * i;
					rtRect.y += rtRect.height * ( pChunk->nHeight - 1 - j );
				}
			}
		}
	}
}

CRectangle CChunkObject::GetRect()
{
	return CRectangle( globalTransform.GetPosition().x, globalTransform.GetPosition().y,
		m_pChunk->nWidth * CMyLevel::GetBlockSize(), m_pChunk->nHeight * CMyLevel::GetBlockSize() );
}

void CChunkObject::CreateBlockRTLayer( CBlockObject* pBlockObject )
{
	CDrawableGroup* pDrawable = m_strBlockRTDrawable;
	if( !pDrawable )
		return;
	auto& texRect = pBlockObject->GetBlock()->rtTexRect;
	if( texRect.width <= 0 || texRect.height <= 0 )
		return;

	auto pImage2D = SafeCast<CImage2D>( pDrawable->CreateInstance() );
	pImage2D->SetRect( CRectangle( pBlockObject->m_pBlock->nX * CMyLevel::GetBlockSize(), pBlockObject->m_pBlock->nY * CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize() ) );
	pImage2D->SetTexRect( pBlockObject->GetBlock()->rtTexRect );
	uint16 nParamCount;
	CVector4* pParams = pImage2D->GetParam( nParamCount );
	if( nParamCount )
	{
		int32 x = pBlockObject->m_nBlockRTIndex & 63;
		int32 y = ( pBlockObject->m_nBlockRTIndex >> 6 ) & 31;
		pParams[nParamCount - 1] = CVector4( x / 64.0f, ( 32 - 1 - y ) / 32.0f, 1 / 64.0f, 1 / 32.0f );
	}
	pBlockObject->m_pBlockRTObject = pImage2D;

	CRenderObject2D* pParent;
	if( m_p1 )
		pParent = m_p1;
	else
		pParent = GetRenderObject();
	pParent->AddChild( pImage2D );
}

void CChunkObject::RemoveChunk()
{
	if( m_pChunk && CMyLevel::GetInst() )
	{
		auto pChunk = m_pChunk;
		m_pChunk = NULL;
		pChunk->pChunkObject = NULL;
		CMyLevel::GetInst()->RemoveChunk( pChunk );
	}
}

CVector2 CChunkObject::GetShake()
{
	CVector2 pos = m_hitShakeVector * cos( m_nHitShakeFrame * 1.275235 ) + CVector2( m_hitShakeVector.y, -m_hitShakeVector.x ) * sin( m_nHitShakeFrame * 1.58792 );
	pos = CVector2( floor( pos.x + 0.5f ), floor( pos.y + 0.5f ) );
	return pos;
}

CVector2 CChunkObject::GetSurfaceVel( const CVector2& pos )
{
	return m_surfaceVel;
}

void CChunkObject::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		if( m_bEftTiled )
		{
			for( int i = 0; i < m_pChunk->nWidth; i++ )
			{
				for( int j = 0; j < m_pChunk->nHeight; j++ )
				{
					auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
					pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
					pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
					pEffect->SetState( 2 );
				}
			}
		}
		else
		{
			auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
			pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			pEffect->SetPosition( globalTransform.GetPosition() );
			pEffect->SetState( 2 );
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

bool CChunkObject::Damage( float fDmg, uint8 nType )
{
	SDamageContext context = { fDmg, nType, eDamageSourceType_None, CVector2( 0, 0 ) };
	return Damage( context );
}

bool CChunkObject::Damage( SDamageContext& context )
{
	auto pLevel = CMyLevel::GetInst();
	if( pLevel && y >= pLevel->GetLvBarrierHeight() )
		return false;
	if( m_pChunk )
	{
		if( m_pChunk->bInvulnerable )
			return false;
		if( !m_nMaxHp )
		{
			auto pParentChunk = m_pChunk->pParentChunk;
			if( pParentChunk && m_pChunk->nSubChunkType == 2 )
				return pParentChunk->pChunkObject->Damage( context );
			return false;
		}
	}
	if( m_fHp <= 0 )
		return false;

	if( context.nSourceType > eDamageSourceType_None )
	{
		AddHitShake( context.hitDir );
	}

	uint32 nLastDamageFrame = Min<float>( m_nDamagedEffectsCount, Max<float>( ( m_fHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 ) );
	m_fHp = Max<float>( m_fHp - (float)context.fDamage, 0 );
	uint32 nDamageFrame = Min<float>( m_nDamagedEffectsCount, Max<float>( ( m_fHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 ) );
	if( nLastDamageFrame != nDamageFrame )
	{
		if( nLastDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nLastDamageFrame]->bVisible = false;
		if( nDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nDamageFrame]->bVisible = true;
	}
	
	m_triggerDamaged.Trigger( 0, &context );
	if( m_fHp <= 0 )
	{
		m_fHp = 0;
		if( context.nType == 1 )
			Crush();
		else
			Kill();
	}
	return true;
}

float CChunkObject::Repair( float fAmount )
{
	uint32 nLastDamageFrame = Min<float>( m_nDamagedEffectsCount, Max<float>( ( m_fHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 ) );
	float fLastHp = m_fHp;
	m_fHp = fAmount >= 0 ? Min<float>( m_fHp + fAmount, m_nMaxHp ) : m_nMaxHp;
	uint32 nDamageFrame = Min<float>( m_nDamagedEffectsCount, Max<float>( ( m_fHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 ) );
	if( nLastDamageFrame != nDamageFrame )
	{
		if( nLastDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nLastDamageFrame]->bVisible = false;
		if( nDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nDamageFrame]->bVisible = true;
	}
	return m_fHp - fLastHp;
}

void CChunkObject::Kill()
{
	DEFINE_TEMP_REF_THIS()
	while( m_pChunk->Get_StopEvent() )
	{
		CReference<CChunkStopEvent> pStopEvent = m_pChunk->Get_StopEvent();
		pStopEvent->RemoveFrom_StopEvent();
		if( pStopEvent->killedFunc )
			pStopEvent->killedFunc( m_pChunk );
	}
	m_triggerKilled.Trigger( 0, this );
	OnKilled();

	auto pChunk = m_pChunk;
	if( pChunk )
	{
		m_pChunk = NULL;
		pChunk->pChunkObject = NULL;
		CMyLevel::GetInst()->KillChunk( pChunk, false, this );
	}
	m_triggerPostKilled.Trigger( 0, this );
	SetParentEntity( NULL );
}

void CChunkObject::AddHitShake( CVector2 shakeVector )
{
	if( m_hitShakeVector.Dot( m_hitShakeVector ) < 1 )
		m_nHitShakeFrame = 0;
	if( m_hitShakeVector.Dot( shakeVector ) < 0 )
		shakeVector = shakeVector * -1;
	m_hitShakeVector = m_hitShakeVector + shakeVector;
	float l = m_hitShakeVector.Length();
	if( l > 8 )
		m_hitShakeVector = m_hitShakeVector * ( 8 / l );
	//MoveToTopmost( true );

	if( !m_onHitShakeTick.IsRegistered() )
		GetStage()->RegisterBeforeHitTest( 1, &m_onHitShakeTick );
}

void CChunkObject::ClearHitShake()
{
	if( m_onHitShakeTick.IsRegistered() )
		m_onHitShakeTick.Unregister();
	m_hitShakeVector = CVector2( 0, 0 );
	HandleHitShake( CVector2( 0, 0 ) );
	if( m_pChunk )
	{
		for( auto pSubChunk = m_pChunk->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
		{
			if( pSubChunk->pChunkObject )
				pSubChunk->pChunkObject->HandleHitShake( CVector2( 0, 0 ) );
		}
	}
}

void CChunkObject::HitShakeTick()
{
	m_hitShakeVector = m_hitShakeVector * 0.95;
	float l = m_hitShakeVector.Length();
	if( l <= 1 )
	{
		ClearHitShake();
		return;
	}
	m_hitShakeVector = m_hitShakeVector * ( ( l - 1 ) / l );

	CVector2 pos = GetShake();
	HandleHitShake( pos );

	for( auto pSubChunk = m_pChunk->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
	{
		if( pSubChunk->pChunkObject )
			pSubChunk->pChunkObject->HandleHitShake( pos );
	}

	m_nHitShakeFrame++;
	GetStage()->RegisterBeforeHitTest( 1, &m_onHitShakeTick );
}

void CChunkObject::HandleHitShake( const CVector2& ofs )
{
	/*if( GetRenderObject() )
		GetRenderObject()->SetPosition( ofs );
	if( m_p1 )
		m_p1->SetPosition( ofs );
	if( m_pDamagedEffectsRoot )
		m_pDamagedEffectsRoot->SetPosition( ofs );*/
}

bool CExplosiveChunk::Damage( SDamageContext& context )
{
	if( m_bKilled )
	{
		m_fHp -= context.fDamage;
		if( -m_fHp >= m_nMaxKillHp )
			Explode();
		return true;
	}
	else
		return CChunkObject::Damage( context );
}

void CExplosiveChunk::Kill()
{
	if( m_bKilled )
		return;
	m_triggerKilled.Trigger( 0, this );
	m_bKilled = true;
	m_fHp = 0;
	m_nDeathDamageCDLeft = 0;
	AddHitShake( CVector2( 8, 0 ) );
	Tick();
	OnKilled();
}

void CExplosiveChunk::OnAddedToStage()
{
	CChunkObject::OnAddedToStage();
	m_pKillEffect = CResourceManager::Inst()->CreateResource<CPrefab>( m_strKillEffect.c_str() );
}

void CExplosiveChunk::Tick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_deathTick );

	AddHitShake( CVector2( 2, 0 ) );
	
	if( m_pKillEffect )
	{
		if( m_nKillEffectCDLeft )
			m_nKillEffectCDLeft--;
		if( !m_nKillEffectCDLeft )
		{
			CVector2 center = CVector2( SRand::Inst().Rand( 0u, m_pChunk->nWidth * CMyLevel::GetBlockSize() ), SRand::Inst().Rand( 0u, m_pChunk->nHeight * CMyLevel::GetBlockSize() ) );
			auto pEffect = SafeCast<CEffectObject>( m_pKillEffect->GetRoot()->CreateInstance() );
			pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			pEffect->SetPosition( GetPosition() + center );
			pEffect->SetState( 2 );
			m_nKillEffectCDLeft = m_nKillEffectInterval;
		}
	}

	if( m_nDeathDamageCDLeft )
		m_nDeathDamageCDLeft--;
	if( !m_nDeathDamageCDLeft )
	{
		m_nDeathDamageCDLeft = m_nDeathDamageInterval;
		CChunkObject::Damage( m_nDeathDamage );
	}
}
