#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "Player.h"
#include "GUI/MainUI.h"
#include "GlobalCfg.h"
#include "Rand.h"

CMyLevel* CMyLevel::s_pLevel = NULL;
int8 CMyLevel::s_nTypes[] = { 0, 1, 1, 2, 3 };
void CMyLevel::OnAddedToStage()
{
	m_basements.resize( m_nWidth );
	auto pMainUI = CMainUI::GetInst();
	if( pMainUI )
		pMainUI->ClearMinimap();

	CEntity* pBorder = new CEntity;
	pBorder->SetParentEntity( this );
	{
		CEntity* pLeft = new CEntity;
		CRectangle rect = GetBound();
		rect.SetLeft( rect.GetLeft() - 256 );
		rect.width = 256;
		pLeft->AddRect( rect );
		pLeft->SetHitType( eEntityHitType_Platform );
		pLeft->SetParentEntity( pBorder );

		CEntity* pRight = new CEntity;
		rect.SetLeft( GetBound().GetRight() );
		pRight->AddRect( rect );
		pRight->SetHitType( eEntityHitType_Platform );
		pRight->SetParentEntity( pBorder );

		CEntity* pTop = new CEntity;
		rect = GetBound();
		rect.SetLeft( rect.GetLeft() - 256 );
		rect.SetRight( rect.GetRight() + 256 );
		rect.SetTop( rect.GetTop() - 256 );
		rect.height = 256;
		pTop->AddRect( rect );
		pTop->SetHitType( eEntityHitType_Platform );
		pTop->SetParentEntity( pBorder );

		CEntity* pBottom = new CEntity;
		rect.SetTop( GetBound().GetBottom() );
		pBottom->AddRect( rect );
		pBottom->SetHitType( eEntityHitType_Platform );
		pBottom->SetParentEntity( pBorder );
	}

	CreateGrids( true );
	CacheNextLevel();
	CheckSpawn();
	s_pLevel = this;

	pFireSound = CResourceManager::Inst()->CreateResource<CSoundFile>( CGlobalCfg::Inst().mapSoundPath["fire"].c_str() );
	pHitSound = CResourceManager::Inst()->CreateResource<CSoundFile>( CGlobalCfg::Inst().mapSoundPath["hit"].c_str() );
	pExpSound = CResourceManager::Inst()->CreateResource<CSoundFile>( CGlobalCfg::Inst().mapSoundPath["exp"].c_str() );

	pChunkUIPrefeb = CResourceManager::Inst()->CreateResource<CPrefab>( CGlobalCfg::Inst().mapPrefabPath["chunk_ui"].c_str() );

	for( int i = 0; i < ELEM_COUNT( m_pBulletRoot ); i++ )
	{
		m_pBulletRoot[i] = new CEntity;
		m_pBulletRoot[i]->SetParentEntity( this );
	}
}

void CMyLevel::OnRemovedFromStage()
{
	if( s_pLevel == this )
		s_pLevel = NULL;
}

const char* g_levels[] = { "lv1", "lv2", "lv3", "lv4", "lv5" };

void CMyLevel::CreateGrids( bool bNeedInit )
{
	auto& cfg = CGlobalCfg::Inst();
	SLevelBuildContext context( this );

	if( bNeedInit )
	{
		CLevelGenerateNode* pNode = cfg.levelGenerateNodeContext.mapNamedNodes["init"];
		pNode->Generate( context, TRectangle<int32>( 0, 0, m_nWidth, m_nHeight ) );
	}

	CLevelGenerateNode* pNode = cfg.levelGenerateNodeContext.mapNamedNodes[g_levels[m_nCurLevel]];
	pNode->Generate( context, TRectangle<int32>( 0, 0, m_nWidth, m_nHeight ) );
	context.Build();
}

void CMyLevel::CacheNextLevel()
{
	if( m_nCurLevel < ELEM_COUNT( g_levels ) )
	{
		m_nCurLevel++;
		if( m_nCurLevel < ELEM_COUNT( g_levels ) )
			CreateGrids( false );
	}
}

void CMyLevel::KillChunk( SChunk * pChunk, bool bCrush )
{
	if( pChunk->pChunkObject )
	{
		if( bCrush )
			pChunk->pChunkObject->Crush();
		else
			pChunk->pChunkObject->Kill();
		return;
	}
	if( !pChunk->Get_SubChunk() )
	{
		RemoveChunk( pChunk );
		return;
	}

	vector< pair<SChunk*, TVector2<int32> > > newChunks;
	uint32 nChunks = 0;
	for( auto pSubChunk = pChunk->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
	{
		nChunks++;
	}
	newChunks.resize( nChunks );
	while( pChunk->Get_SubChunk() )
	{
		auto& item = newChunks[--nChunks];
		item.first = pChunk->Get_SubChunk();
		item.second = item.first->pos;
		item.first->RemoveFrom_SubChunk();
	}
	SplitChunks( pChunk, newChunks );
}

void CMyLevel::RemoveChunk( SChunk* pChunk )
{
	if( !pChunk->bIsSubChunk )
	{
		if( pChunk->pChunkObject )
		{
			pChunk->pChunkObject->SetParentEntity( NULL );
			return;
		}

		auto pMainUI = CMainUI::GetInst();
		if( pMainUI )
		{
			for( int j = 0; j < pChunk->nHeight; j++ )
			{
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, pChunk->pos.y / m_nBlockSize + j, -1 );
				}
			}
		}

		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			auto pBlock = pChunk->GetBlock( i, 0 );
			auto& basement = m_basements[i + pChunk->pos.x / m_nBlockSize];
			if( pBlock == basement.pSpawnedBlock )
			{
				auto pPrev = pBlock->GetPrev( basement.pSpawnedBlock );
				basement.pSpawnedBlock = pPrev;
			}
			pBlock->RemoveFrom_Block();
		}
		for( auto& block : pChunk->blocks )
		{
			if( block.pEntity )
			{
				block.pEntity->SetParentEntity( NULL );
				block.pEntity = NULL;
			}
		}

		for( auto& basement : m_basements )
		{
			basement.fShakeStrength += pChunk->nDestroyShake;
		}
	}

	while( pChunk->Get_SubChunk() )
	{
		auto pSubChunk = pChunk->Get_SubChunk();
		pSubChunk->bIsSubChunk = true;
		pSubChunk->RemoveFrom_SubChunk();
		RemoveChunk( pSubChunk );
	}

	if( pChunk->bIsLevelBarrier )
		CacheNextLevel();

	delete pChunk;
}

void CMyLevel::SplitChunks( SChunk* pOldChunk, vector< pair<SChunk*, TVector2<int32> > > newChunks )
{
	uint32 nX = pOldChunk->pos.x / m_nBlockSize;
	uint32 nY = pOldChunk->pos.y / m_nBlockSize;
	uint32 nWidth = pOldChunk->nWidth;
	uint32 nHeight = pOldChunk->nHeight;

	vector<SBlock*> ppInsertAfter;
	vector<SBlock*> ppInsertBefore;
	ppInsertAfter.resize( nWidth );
	ppInsertBefore.resize( nWidth );
	for( int i = 0; i < nWidth; i++ )
	{
		ppInsertAfter[i] = pOldChunk->GetBlock( i, 0 );
		ppInsertBefore[i] = ppInsertAfter[i]->NextBlock();
	}

	for( auto& item : newChunks )
	{
		int32 minX = item.second.x;
		int32 minY = item.second.y;
		int32 maxX = minX + item.first->nWidth;
		int32 maxY = minY + item.first->nHeight;
		if( minX < 0 || minY < 0 || maxX > nWidth || maxY > nHeight )
		{
			return;
		}
		for( int i = minX; i < maxX; i++ )
		{
			auto pInsertAfter = ppInsertAfter[i];
			if( minY * m_nBlockSize + pOldChunk->pos.y < pInsertAfter->pOwner->pos.y )
			{
				return;
			}
		}

		for( int i = minX; i < maxX; i++ )
		{
			auto& pInsertAfter = ppInsertAfter[i];
			pInsertAfter->InsertAfter_Block( item.first->GetBlock( i - minX, 0 ) );
			pInsertAfter = item.first->GetBlock( i - minX, 0 );
		}
		item.first->pos = pOldChunk->pos + item.second * m_nBlockSize;
	}

	bool bSpawned = pOldChunk->bSpawned;
	for( int i = 0; i < nWidth; i++ )
	{
		ppInsertAfter[i] = pOldChunk->GetBlock( i, 0 )->NextBlock();
	}
	RemoveChunk( pOldChunk );

	auto pMainUI = CMainUI::GetInst();
	for( int i = 0; i < nWidth; i++ )
	{
		for( auto pNewBlock = ppInsertAfter[i]; pNewBlock != ppInsertBefore[i]; pNewBlock = pNewBlock->NextBlock() )
		{
			if( pMainUI )
				pMainUI->UpdateMinimap( pNewBlock->nX + pNewBlock->pOwner->pos.x / m_nBlockSize, pNewBlock->nY + pNewBlock->pOwner->pos.y / m_nBlockSize,
					s_nTypes[pNewBlock->pBaseInfo->eBlockType] );

			if( bSpawned )
			{
				auto basement = m_basements[i + nX];
				pNewBlock->pOwner->CreateChunkObject( this );
				auto pBlockToSpawn = ( basement.pSpawnedBlock ? basement.pSpawnedBlock->NextBlock() : basement.Get_Block() );
				if( pBlockToSpawn == pNewBlock )
					basement.pSpawnedBlock = pNewBlock;
			}
		}
	}
}

void CMyLevel::AddShakeStrength( float fShakeStrength )
{
	for( auto& basement : m_basements )
	{
		basement.fShakeStrength += fShakeStrength;
	}
}

void CMyLevel::UpdateBlocksMovement()
{
	int8 nTileTypes[] = { 0, 1, 1, 2, 3 };
	auto pMainUI = CMainUI::GetInst();
	vector<SBlock*> vecUpdatedBlocks;
	uint32 nUpdatedBlock = 0;
	for( auto& basement : m_basements )
	{
		basement.nCurShakeStrength = ceil( basement.fShakeStrength );
		basement.nShakeHeight = -1;
		auto pBlock = basement.Get_Block();
		if( pBlock )
		{
			pBlock->pPreBlock = NULL;
			vecUpdatedBlocks.push_back( pBlock );
		}
	}

	SChunk* pLevelBarrier = NULL;
	while( nUpdatedBlock < vecUpdatedBlocks.size() )
	{
		auto pBlock = vecUpdatedBlocks[nUpdatedBlock++];
		auto pChunk = pBlock->pOwner;
		auto& basement = m_basements[pBlock->nX + pChunk->pos.x / m_nBlockSize];
		pChunk->nUpdateCount++;

		int32& shakeStrength = basement.nCurShakeStrength;
		if( shakeStrength || !pChunk->nAbsorbShakeStrength )
		{
			shakeStrength -= pChunk->nAbsorbShakeStrength;
			if( shakeStrength < 0 )
				shakeStrength = 0;
			pChunk->bAbsorbedShake = true;
		}
		else if( basement.nShakeHeight < 0 )
		{
			basement.nShakeHeight = pBlock->pPreBlock ? pBlock->pPreBlock->pOwner->nHeight + pBlock->pPreBlock->pOwner->pos.y / m_nBlockSize : 0;
		}

		if( pChunk->nUpdateCount == pChunk->nWidth )
		{
			float fBalance = 1;
			if( pChunk->bForceStop )
			{
				pChunk->nFallSpeed = 0;
				pChunk->bForceStop = false;
			}
			else if( !pChunk->bStopMove )
			{
				int32 nMinY = 0;
				uint32 nMaxFallSpeed = 0;
				uint32 nApplyWeightCount = 0;
				int32 preY = pChunk->pos.y;
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					auto pPreBlock = pChunk->GetBlock( i, 0 )->pPreBlock;
					if( pPreBlock )
					{
						int32 nPreY = pPreBlock->pOwner->pos.y + pPreBlock->pOwner->nHeight * m_nBlockSize;

						if( nPreY >= preY && !pPreBlock->pOwner->bMovedLastFrame )
						{
							nApplyWeightCount++;
						}

						if( nPreY > nMinY )
						{
							nMinY = nPreY;
							nMaxFallSpeed = pPreBlock->pOwner->nFallSpeed;
						}
						else if( nPreY == nMinY )
						{
							nMaxFallSpeed = Min( nMaxFallSpeed, pPreBlock->pOwner->nFallSpeed );
						}
					}
				}

				pChunk->nFallSpeed++;
				int32 nFallDist = floor( pChunk->nFallSpeed * m_fFallDistPerSpeedFrame );
				pChunk->pos.y -= nFallDist;
				if( pChunk->pos.y < nMinY )
				{
					pChunk->pos.y = nMinY;
					uint32 nPreSpeed = pChunk->nFallSpeed;
					pChunk->nFallSpeed = nMaxFallSpeed;
					if( pChunk->pChunkObject )
					{
						pChunk->pChunkObject->OnLandImpact( nPreSpeed, nMaxFallSpeed );
					}
				}

				pChunk->bMovedLastFrame = preY != pChunk->pos.y;
				if( pChunk->bMovedLastFrame )
				{
					if( pMainUI )
					{
						uint32 y0 = preY / m_nBlockSize;
						uint32 y1 = pChunk->pos.y / m_nBlockSize;
						if( y0 != y1 )
						{
							for( int j = 0; j < pChunk->nHeight; j++ )
							{
								for( int i = 0; i < pChunk->nWidth; i++ )
								{
									pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, y0 + j, -1 );
									pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, y1 + j, nTileTypes[pChunk->GetBlock( i, j )->pBaseInfo->eBlockType] );
								}
							}
						}
					}
					if( pChunk->pChunkObject )
					{
						pChunk->pChunkObject->SetPosition( CVector2( pChunk->pos.x, pChunk->pos.y ) );
					}

					//pChunk->fCurImbalanceTime = 0;
				}

				if( nApplyWeightCount )
				{
					float fAppliedWeight = pChunk->fWeight / nApplyWeightCount;
					int32 nLeftmost = pChunk->nWidth;
					int32 nRightmost = -1;
					for( int i = 0; i < pChunk->nWidth; i++ )
					{
						auto pPreBlock = pChunk->GetBlock( i, 0 )->pPreBlock;
						if( pPreBlock )
						{
							int32 nPreY = pPreBlock->pOwner->pos.y + pPreBlock->pOwner->nHeight * m_nBlockSize;

							if( nPreY >= preY && !pPreBlock->pOwner->bMovedLastFrame )
							{
								if( i < nLeftmost )
									nLeftmost = i;
								if( i > nRightmost )
									nRightmost = i;
								if( pChunk->bAbsorbedShake )
									pPreBlock->pOwner->fAppliedWeight += fAppliedWeight;
							}
						}
					}
					fBalance = Min( pChunk->nWidth - nLeftmost * 1.0f, ( nRightmost + 1 ) * 1.0f ) / pChunk->nWidth;
				}
			}

			pChunk->fBalance = fBalance;
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				auto pNextBlock = pChunk->GetBlock( i, 0 )->NextBlock();
				if( pNextBlock )
				{
					vecUpdatedBlocks.push_back( pNextBlock );
					pNextBlock->pPreBlock = pBlock;
				}
			}
			if( pChunk->bIsLevelBarrier )
			{
				pLevelBarrier = pChunk;
				break;
			}
		}
	}

	if( pLevelBarrier )
	{
		while( nUpdatedBlock < vecUpdatedBlocks.size() )
		{
			auto pBlock = vecUpdatedBlocks[nUpdatedBlock++];
			auto pChunk = pBlock->pOwner;
			auto& basement = m_basements[pBlock->nX + pChunk->pos.x / m_nBlockSize];
			pChunk->nUpdateCount++;
			if( basement.nShakeHeight < 0 )
			{
				basement.nShakeHeight = pBlock->pPreBlock ? pBlock->pPreBlock->pOwner->nHeight + pBlock->pPreBlock->pOwner->pos.y / m_nBlockSize : 0;
			}

			if( pChunk->nUpdateCount == pChunk->nWidth )
			{
				pChunk->nFallSpeed = pLevelBarrier->nFallSpeed;

				int32 nMinY = 0;
				int32 preY = pChunk->pos.y;
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					auto pPreBlock = pChunk->GetBlock( i, 0 )->pPreBlock;
					if( pPreBlock )
					{
						int32 nPreY = pPreBlock->pOwner->pos.y + pPreBlock->pOwner->nHeight * m_nBlockSize;
						if( nPreY > nMinY )
							nMinY = nPreY;
					}
				}
				pChunk->pos.y = nMinY;
				if( preY != pChunk->pos.y )
				{
					if( pMainUI )
					{
						uint32 y0 = preY / m_nBlockSize;
						uint32 y1 = pChunk->pos.y / m_nBlockSize;
						if( y0 != y1 )
						{
							for( int j = 0; j < pChunk->nHeight; j++ )
							{
								for( int i = 0; i < pChunk->nWidth; i++ )
								{
									pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, y0 + j, -1 );
									pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, y1 + j, nTileTypes[pChunk->GetBlock( i, j )->pBaseInfo->eBlockType] );
								}
							}
						}
					}
					if( pChunk->pChunkObject )
					{
						pChunk->pChunkObject->SetPosition( CVector2( pChunk->pos.x, pChunk->pos.y ) );
					}
				}

				pChunk->fBalance = 1;
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					auto pNextBlock = pChunk->GetBlock( i, 0 )->NextBlock();
					if( pNextBlock )
					{
						vecUpdatedBlocks.push_back( pNextBlock );
						pNextBlock->pPreBlock = pBlock;
					}
				}
			}
		}
	}

	for( auto pBlock : vecUpdatedBlocks )
	{
		pBlock->pPreBlock = NULL;
		pBlock->pOwner->nUpdateCount--;

		if( !pBlock->pOwner->nUpdateCount )
		{
			if( pBlock->pOwner->fDestroyBalance > 0 && pBlock->pOwner->fBalance < pBlock->pOwner->fDestroyBalance )
			{
				if( !pBlock->pOwner->bMovedLastFrame )
				{
					pBlock->pOwner->fCurImbalanceTime += GetStage()->GetElapsedTimePerTick();
					if( pBlock->pOwner->bAbsorbedShake && pBlock->pOwner->fCurImbalanceTime > pBlock->pOwner->fImbalanceTime )
					{
						KillChunk( pBlock->pOwner );
						continue;
					}
				}
			}
			else
			{
				pBlock->pOwner->fCurImbalanceTime = 0;
			}
			if( pBlock->pOwner->fDestroyWeight > 0 && pBlock->pOwner->fAppliedWeight >= pBlock->pOwner->fDestroyWeight )
			{
				KillChunk( pBlock->pOwner, true );
				continue;
			}

			pBlock->pOwner->fAppliedWeight = 0;
			pBlock->pOwner->bAbsorbedShake = false;
		}
	}

	for( int i = 0; i < m_basements.size(); i++ )
	{
		auto& basement = m_basements[i];
		basement.fShakeStrength = Max( 0.0f, basement.fShakeStrength * 0.95f - 0.02f );
		basement.nCurShakeStrength = 0;
		pMainUI->UpdateShakeSmallBar( i, basement.nShakeHeight >= 0 ? Min( basement.nShakeHeight, (int32)m_nHeight ) : (int32)m_nHeight );
	}

	CheckSpawn();
}

void CMyLevel::CheckSpawn()
{
	for( auto& basement : m_basements )
	{
		auto pBlockToSpawn = ( basement.pSpawnedBlock ? basement.pSpawnedBlock->NextBlock() : basement.Get_Block() );
		while( pBlockToSpawn && pBlockToSpawn->pOwner->pos.y <= m_nSpawnHeight * m_nBlockSize )
		{
			pBlockToSpawn->pOwner->CreateChunkObject( this );
			basement.pSpawnedBlock = pBlockToSpawn;
			pBlockToSpawn = pBlockToSpawn->NextBlock();
		}
	}
}