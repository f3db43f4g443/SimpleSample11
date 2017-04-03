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
		rect.x = GetBound().GetRight();
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
		rect.y = GetBound().GetBottom();
		pBottom->AddRect( rect );
		pBottom->SetHitType( eEntityHitType_Platform );
		pBottom->SetParentEntity( pBorder );
	}
	{
		CEntity* pLeft = new CEntity;
		CRectangle rect = GetBound();
		rect.SetLeft( rect.GetLeft() - 1024 );
		rect.width = 256;
		pLeft->AddRect( rect );
		pLeft->SetHitType( eEntityHitType_WorldStatic );
		pLeft->SetParentEntity( pBorder );

		CEntity* pRight = new CEntity;
		rect.x = GetBound().GetRight() + 768;
		pRight->AddRect( rect );
		pRight->SetHitType( eEntityHitType_WorldStatic );
		pRight->SetParentEntity( pBorder );

		CEntity* pTop = new CEntity;
		rect = GetBound();
		rect.SetLeft( rect.GetLeft() - 1024 );
		rect.SetRight( rect.GetRight() + 1024 );
		rect.SetTop( rect.GetTop() - 1024 );
		rect.height = 256;
		pTop->AddRect( rect );
		pTop->SetHitType( eEntityHitType_WorldStatic );
		pTop->SetParentEntity( pBorder );

		CEntity* pBottom = new CEntity;
		rect.y = GetBound().GetBottom() + 768;
		pBottom->AddRect( rect );
		pBottom->SetHitType( eEntityHitType_WorldStatic );
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

const char* g_levels[] = { "lv1_2", "lv1_2", "lv1_2", "lv1_2", "lv1_2" };

void CMyLevel::CreateGrids( bool bNeedInit )
{
	auto& cfg = CGlobalCfg::Inst();
	SLevelBuildContext context( this );

	if( bNeedInit )
	{
		CLevelGenerateNode* pNode = cfg.pRootGenerateFile->FindNode( "init" );
		pNode->Generate( context, TRectangle<int32>( 0, 0, m_nWidth, m_nHeight ) );
	}

	CLevelGenerateNode* pNode = cfg.pRootGenerateFile->FindNode( g_levels[m_nCurLevel] );
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

void CMyLevel::Clear()
{
	for( auto basement : m_basements )
	{
		for( int i = 0; i < 2; i++ )
		{
			while( basement.layers[i].Get_BlockLayer() )
				RemoveChunk( basement.layers[i].Get_BlockLayer()->pParent->pOwner );
		}
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
		item.first->bIsSubChunk = false;
	}
	SplitChunks( pChunk, newChunks );
}

void CMyLevel::RemoveChunk( SChunk* pChunk )
{
	if( pChunk->pChunkObject )
	{
		pChunk->pChunkObject->SetParentEntity( NULL );
		return;
	}

	if( !pChunk->bIsSubChunk )
	{
		auto pMainUI = CMainUI::GetInst();
		if( pMainUI )
		{
			for( int iLayer = 0; iLayer < 2; iLayer++ )
			{
				if( !pChunk->HasLayer( iLayer ) )
					continue;
				for( int j = 0; j < pChunk->nHeight; j++ )
				{
					for( int i = 0; i < pChunk->nWidth; i++ )
					{
						pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, pChunk->pos.y / m_nBlockSize + j, iLayer, -1 );
					}
				}
			}
		}

		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			auto pBlock = pChunk->GetBlock( i, 0 );
			auto& basement = m_basements[i + pChunk->pos.x / m_nBlockSize];
			for( int j = 0; j < 2; j++ )
			{
				if( !pChunk->HasLayer( j ) )
					continue;
				auto pBlockLayer = pBlock->layers + j;
				auto& pSpawnedBlock = basement.layers[j].pSpawnedBlock;
				if( pBlockLayer == pSpawnedBlock )
				{
					auto pPrev = pBlockLayer->GetPrev( pSpawnedBlock );
					pSpawnedBlock = pPrev;
				}
				pBlockLayer->RemoveFrom_BlockLayer();
			}
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
	else
		pChunk->RemoveFrom_SubChunk();

	while( pChunk->Get_SubChunk() )
	{
		auto pSubChunk = pChunk->Get_SubChunk();
		pSubChunk->bIsSubChunk = true;
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
	uint8 nLayerType = pOldChunk->nLayerType;

	vector<SBlockLayer*> ppInsertAfter[2];
	vector<SBlockLayer*> ppInsertBefore[2];
	for( int iLayer = 0; iLayer < 2; iLayer++ )
	{
		if( !pOldChunk->HasLayer( iLayer ) )
			continue;
		ppInsertAfter[iLayer].resize( nWidth );
		ppInsertBefore[iLayer].resize( nWidth );

		for( int i = 0; i < nWidth; i++ )
		{
			ppInsertAfter[iLayer][i] = pOldChunk->GetBlock( i, 0 )->layers + iLayer;
			ppInsertBefore[iLayer][i] = ppInsertAfter[iLayer][i]->NextBlockLayer();
		}
	}

	for( auto& item : newChunks )
	{
		int32 minX = item.second.x / m_nBlockSize;
		int32 minY = item.second.y / m_nBlockSize;
		int32 maxX = minX + item.first->nWidth;
		int32 maxY = minY + item.first->nHeight;
		if( minX < 0 || minY < 0 || maxX > nWidth || maxY > nHeight || ( item.first->nLayerType & nLayerType ) != item.first->nLayerType )
		{
			return;
		}
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			if( !item.first->HasLayer( iLayer ) )
				continue;
			for( int i = minX; i < maxX; i++ )
			{
				auto pInsertAfter = ppInsertAfter[iLayer][i];
				if( minY * m_nBlockSize + pOldChunk->pos.y < pInsertAfter->pParent->pOwner->pos.y )
				{
					return;
				}
			}
		}
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			if( !item.first->HasLayer( iLayer ) )
				continue;
			for( int i = minX; i < maxX; i++ )
			{
				auto& pInsertAfter = ppInsertAfter[iLayer][i];
				auto pNext = item.first->GetBlock( i - minX, 0 )->layers + iLayer;
				pInsertAfter->InsertAfter_BlockLayer( pNext );
				pInsertAfter = pNext;
			}
		}
		item.first->pos = pOldChunk->pos + item.second;
	}

	bool bSpawned = pOldChunk->bSpawned;
	for( int iLayer = 0; iLayer < 2; iLayer++ )
	{
		if( !pOldChunk->HasLayer( iLayer ) )
			continue;

		for( int i = 0; i < nWidth; i++ )
		{
			ppInsertAfter[iLayer][i] = pOldChunk->GetBlock( i, 0 )->layers + iLayer;
		}
	}

	auto pMainUI = CMainUI::GetInst();
	for( int iLayer = 0; iLayer < 2; iLayer++ )
	{
		if( !pOldChunk->HasLayer( iLayer ) )
			continue;
		for( int i = 0; i < nWidth; i++ )
		{
			for( auto pNewBlockLayer = ppInsertAfter[iLayer][i]; pNewBlockLayer != ppInsertBefore[iLayer][i]; pNewBlockLayer = pNewBlockLayer->NextBlockLayer() )
			{
				auto pNewBlock = pNewBlockLayer->pParent;
				if( pMainUI )
					pMainUI->UpdateMinimap( pNewBlock->nX + pNewBlock->pOwner->pos.x / m_nBlockSize, pNewBlock->nY + pNewBlock->pOwner->pos.y / m_nBlockSize,
						iLayer, s_nTypes[pNewBlock->eBlockType] );

				if( bSpawned )
				{
					auto basement = m_basements[i + nX];
					auto basementLayer = basement.layers[iLayer];
					pNewBlock->pOwner->CreateChunkObject( this );
					auto pBlockToSpawn = ( basementLayer.pSpawnedBlock ? basementLayer.pSpawnedBlock->NextBlockLayer() : basementLayer.Get_BlockLayer() );
					if( pBlockToSpawn == pNewBlockLayer )
						basementLayer.pSpawnedBlock = pNewBlockLayer;
				}
			}
		}
	}
	RemoveChunk( pOldChunk );
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
	uint32 nPlayerHeight = 0;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( pPlayer->GetHp() <= 0 )
			return;
		nPlayerHeight = pPlayer->GetCurRoom() && pPlayer->GetCurRoom()->GetChunk() ? pPlayer->GetCurRoom()->GetChunk()->pos.y : floor( pPlayer->GetPosition().y - 16 );
	}

	int8 nTileTypes[] = { 0, 1, 1, 2, 3 };
	auto pMainUI = CMainUI::GetInst();
	vector<SBlockLayer*> vecUpdatedBlocks;
	uint32 nUpdatedBlock = 0;
	for( auto& basement : m_basements )
	{
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			auto pBlockLayer = basement.layers[iLayer].Get_BlockLayer();
			if( pBlockLayer )
			{
				vecUpdatedBlocks.push_back( pBlockLayer );
			}
			basement.layers[iLayer].pVisitedBlock = NULL;
			basement.layers[iLayer].nCurShakeStrength = ceil( basement.fShakeStrength );
			basement.layers[iLayer].nShakeHeight = -1;
		}
	}

	SChunk* pLevelBarrier = NULL;
	while( nUpdatedBlock < vecUpdatedBlocks.size() )
	{
		auto pBlockLayer = vecUpdatedBlocks[nUpdatedBlock++];
		auto pBlock = pBlockLayer->pParent;
		auto pChunk = pBlock->pOwner;
		pChunk->nUpdateCount++;

		if( pChunk->IsFullyUpdated() )
		{
			float fBalance = 1;
			bool bHit = false;
			bool b1 = false;
			bool bIsCurRoom = false;
			if( pChunk->pChunkObject )
				bIsCurRoom = pChunk->pChunkObject == GetStage()->GetPlayer()->GetCurRoom();
			int32 nBaseX = pChunk->pos.x / m_nBlockSize;

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
					auto& basement = m_basements[i + nBaseX];
					pBlock = pChunk->GetBlock( i, 0 );
					
					SBlock* pPreBlock[2];
					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						auto p = basement.layers[iLayer].pVisitedBlock;
						pPreBlock[iLayer] = p ? p->pParent : NULL;
					}
					if( pPreBlock[0] || pPreBlock[1] )
					{
						int32 nPreYs[2];
						bool bCanApplyWeight = false;
						for( int iLayer = 0; iLayer < 2; iLayer++ )
						{
							if( !pChunk->HasLayer( iLayer ) )
							{
								nPreYs[iLayer] = 0;
								continue;
							}

							auto& layer = basement.layers[iLayer];
							if( pPreBlock[iLayer] )
							{
								nPreYs[iLayer] = pPreBlock[iLayer]->pOwner->pos.y + pPreBlock[iLayer]->pOwner->nHeight * m_nBlockSize;
								layer.nCurMargin = 0;
								if( pBlock->eBlockType != eBlockType_Wall )
								{
									auto pLowerBlock = pPreBlock[iLayer]->pOwner->GetBlock( pPreBlock[iLayer]->nX, pPreBlock[iLayer]->pOwner->nHeight - 1 );
									if( pLowerBlock->eBlockType != eBlockType_Wall )
									{
										layer.nCurMargin = Max( pLowerBlock->nUpperMargin, pBlock->nLowerMargin );
										nPreYs[iLayer] += layer.nCurMargin;
									}
								}
								bCanApplyWeight |= !pPreBlock[iLayer]->pOwner->bMovedLastFrame;
							}
							else
								nPreYs[iLayer] = layer.nCurMargin = pBlock->nLowerMargin;

							if( nPreYs[iLayer] > nMinY )
							{
								nMinY = nPreYs[iLayer];
								nMaxFallSpeed = pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->nFallSpeed : 0;
							}
							else if( nPreYs[iLayer] == nMinY )
							{
								nMaxFallSpeed = Min( nMaxFallSpeed, pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->nFallSpeed : 0 );
							}
						}
						int32 nPreY = Max( nPreYs[0], nPreYs[1] );

						if( nPreY >= preY && bCanApplyWeight )
						{
							nApplyWeightCount++;
						}
					}
					else
						nMinY = Max<int32>( nMinY, pBlock->nLowerMargin );
				}

				if( pChunk->nFallSpeed < 60 )
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
							for( int iLayer = 0; iLayer < 2; iLayer++ )
							{
								if( !pChunk->HasLayer( iLayer ) )
									continue;
								for( int j = 0; j < pChunk->nHeight; j++ )
								{
									for( int i = 0; i < pChunk->nWidth; i++ )
									{
										pMainUI->UpdateMinimap( nBaseX + i, y0 + j, iLayer, -1 );
										pMainUI->UpdateMinimap( nBaseX + i, y1 + j, iLayer, nTileTypes[pChunk->GetBlock( i, j )->eBlockType] );
									}
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
						auto& basement = m_basements[nBaseX + i];

						SBlock* pPreBlock[2];
						for( int iLayer = 0; iLayer < 2; iLayer++ )
						{
							auto p = basement.layers[iLayer].pVisitedBlock;
							pPreBlock[iLayer] = p ? p->pParent : NULL;
						}
						for( int iLayer = 0; iLayer < 2; iLayer++ )
						{
							if( !pChunk->HasLayer( iLayer ) )
								continue;

							bool bHit = false;
							auto& basementLayer = basement.layers[iLayer];
							int32& shakeStrength = basementLayer.nCurShakeStrength;
							int32 nCurShakeStrength = shakeStrength;
							if( shakeStrength )
							{
								uint32 nAbsorbShake = bIsCurRoom ? 0 : pChunk->nAbsorbShakeStrength;
								shakeStrength -= Min<int32>( nAbsorbShake, nAbsorbShake *
									Max<int32>( 0, pChunk->nHeight * m_nBlockSize - ( nPlayerHeight - pChunk->pos.y ) ) / (int32)( pChunk->nHeight * m_nBlockSize ) );
								if( shakeStrength < 0 )
									shakeStrength = 0;
							}

							if( pPreBlock[iLayer] )
							{
								int32 nPreY = pPreBlock[iLayer]->pOwner->pos.y + pPreBlock[iLayer]->pOwner->nHeight * m_nBlockSize;

								if( nPreY >= preY - basementLayer.nCurMargin && !pPreBlock[iLayer]->pOwner->bMovedLastFrame )
								{
									if( i < nLeftmost )
										nLeftmost = i;
									if( i > nRightmost )
										nRightmost = i;
									if( !( iLayer == 1 && pPreBlock[0] == pPreBlock[1] ) && pChunk->pChunkObject )
										pPreBlock[iLayer]->pOwner->fAppliedWeight += fAppliedWeight;
									bHit = true;
								}
							}

							if( !bHit )
							{
								nCurShakeStrength = shakeStrength = 0;
							}
							pChunk->nCurShakeStrength = Max<uint32>( pChunk->nCurShakeStrength, nCurShakeStrength );

							if( !shakeStrength && basementLayer.nShakeHeight < 0 )
							{
								basementLayer.nShakeHeight = pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->nHeight + pPreBlock[iLayer]->pOwner->pos.y / m_nBlockSize : 0;
							}
						}
					}
					fBalance = Min( pChunk->nWidth - nLeftmost * 1.0f, ( nRightmost + 1 ) * 1.0f ) / pChunk->nWidth;
					b1 = true;
				}
				else
				{
					bHit = pChunk->pos.y == nMinY && pChunk->nFallSpeed == 0;
				}
			}
			if( !b1 )
			{
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					auto& basement = m_basements[nBaseX + i];

					SBlock* pPreBlock[2];
					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						auto p = basement.layers[iLayer].pVisitedBlock;
						pPreBlock[iLayer] = p ? p->pParent : NULL;
					}
					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						if( !pChunk->HasLayer( iLayer ) )
							continue;
						auto& basementLayer = basement.layers[iLayer];
						int32& shakeStrength = basementLayer.nCurShakeStrength;
						if( !bHit )
							shakeStrength = 0;

						if( shakeStrength )
						{
							pChunk->nCurShakeStrength = Max<uint32>( pChunk->nCurShakeStrength, shakeStrength );
							uint32 nAbsorbShake = bIsCurRoom ? 0 : pChunk->nAbsorbShakeStrength;
							shakeStrength -= Min<int32>( nAbsorbShake, nAbsorbShake *
								Max<int32>( 0, pChunk->nHeight * m_nBlockSize - ( nPlayerHeight - pChunk->pos.y ) ) / (int32)( pChunk->nHeight * m_nBlockSize ) );
							if( shakeStrength < 0 )
								shakeStrength = 0;
						}
						else if( basementLayer.nShakeHeight < 0 )
						{
							basementLayer.nShakeHeight = pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->nHeight + pPreBlock[iLayer]->pOwner->pos.y / m_nBlockSize : 0;
						}
					}
				}
			}

			pChunk->fBalance = fBalance;
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				for( int iLayer = 0; iLayer < 2; iLayer++ )
				{
					if( !pChunk->HasLayer( iLayer ) )
						continue;

					auto pLayer = &pChunk->GetBlock( i, 0 )->layers[iLayer];
					auto pNextBlock = pLayer->NextBlockLayer();
					if( pNextBlock )
						vecUpdatedBlocks.push_back( pNextBlock );
					m_basements[nBaseX + i].layers[iLayer].pVisitedBlock = pLayer;
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
			auto pBlockLayer = vecUpdatedBlocks[nUpdatedBlock++];
			auto pBlock = pBlockLayer->pParent;
			auto pChunk = pBlock->pOwner;
			auto& basement = m_basements[pBlock->nX + pChunk->pos.x / m_nBlockSize];
			pChunk->nUpdateCount++;

			if( pChunk->IsFullyUpdated() )
			{
				int32 nBaseX = pChunk->pos.x / m_nBlockSize;
				pChunk->nFallSpeed = pLevelBarrier->nFallSpeed;

				int32 nMinY = 0;
				int32 preY = pChunk->pos.y;
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					auto& basement = m_basements[i + nBaseX];
					SBlock* pPreBlock[2] = { basement.layers[0].pVisitedBlock->pParent, basement.layers[1].pVisitedBlock->pParent };
					for( int iLayer = 0; iLayer < 2; iLayer++ )
					{
						if( !pChunk->HasLayer( iLayer ) )
							continue;
						auto& basementLayer = basement.layers[iLayer];

						if( pPreBlock[iLayer] )
						{
							int32 nPreY = pPreBlock[iLayer]->pOwner->pos.y + pPreBlock[iLayer]->pOwner->nHeight * m_nBlockSize;
							if( nPreY > nMinY )
								nMinY = nPreY;
						}

						if( basementLayer.nShakeHeight < 0 )
						{
							basementLayer.nShakeHeight = pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->pos.y / m_nBlockSize : 0;
						}

						auto pLayer = &pChunk->GetBlock( i, 0 )->layers[iLayer];
						auto pNextBlock = pLayer->NextBlockLayer();
						if( pNextBlock )
							vecUpdatedBlocks.push_back( pNextBlock );
						basementLayer.pVisitedBlock = pLayer;
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
							for( int iLayer = 0; iLayer < 2; iLayer++ )
							{
								if( !pChunk->HasLayer( iLayer ) )
									continue;
								for( int j = 0; j < pChunk->nHeight; j++ )
								{
									for( int i = 0; i < pChunk->nWidth; i++ )
									{
										pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, y0 + j, iLayer, -1 );
										pMainUI->UpdateMinimap( pChunk->pos.x / m_nBlockSize + i, y1 + j, iLayer, nTileTypes[pChunk->GetBlock( i, j )->eBlockType] );
									}
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
			}
		}
	}

	for( auto pBlockLayer : vecUpdatedBlocks )
	{
		auto pBlock = pBlockLayer->pParent;
		auto pChunk = pBlock->pOwner;
		pChunk->nUpdateCount--;

		if( !pChunk->nUpdateCount )
		{
			if( pChunk->fDestroyBalance > 0 && pChunk->fBalance < pChunk->fDestroyBalance )
			{
				if( !pChunk->bMovedLastFrame )
				{
					pChunk->fCurImbalanceTime += GetStage()->GetElapsedTimePerTick();
					if( pChunk->pChunkObject && pChunk->fCurImbalanceTime > pChunk->fImbalanceTime )
					{
						KillChunk( pChunk );
						continue;
					}
				}
			}
			else
			{
				pChunk->fCurImbalanceTime = 0;
			}

			if( pChunk->fDestroyWeight > 0 && pChunk->fAppliedWeight >= pChunk->fDestroyWeight )
			{
				KillChunk( pChunk, true );
				continue;
			}

			int32 nShakeDmg = pChunk->nCurShakeStrength - pChunk->nShakeDmgThreshold;
			if( nShakeDmg > 0 && pChunk->pChunkObject )
			{
				pChunk->fAppliedWeight = 0;
				pChunk->nCurShakeStrength = 0;
				float fDmg = pChunk->fShakeDmg * Min( nShakeDmg, 64 ) * GetStage()->GetElapsedTimePerTick();
				if( pChunk->bIsBeingRepaired )
					fDmg *= 0.2f;
				pChunk->pChunkObject->Damage( fDmg );
			}
			else
			{
				pChunk->fAppliedWeight = 0;
				pChunk->nCurShakeStrength = 0;
			}
		}
	}

	for( int i = 0; i < m_basements.size(); i++ )
	{
		auto& basement = m_basements[i];
		int32 nShakeHeight = 0;
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			auto& basementLayer = basement.layers[iLayer];
			basementLayer.nCurShakeStrength = 0;
			nShakeHeight = Max( nShakeHeight, basementLayer.nShakeHeight );
		}
		basement.fShakeStrength = Max( 0.0f, basement.fShakeStrength * 0.95f - 0.02f );
		pMainUI->UpdateShakeSmallBar( i, nShakeHeight >= 0 ? Min( nShakeHeight, (int32)m_nHeight ) : (int32)m_nHeight );
	}

	CheckSpawn();
}

void CMyLevel::CheckSpawn()
{
	for( auto& basement : m_basements )
	{
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			auto& basementLayer = basement.layers[iLayer];
			auto pBlockToSpawn = ( basement.layers[iLayer].pSpawnedBlock ? basementLayer.pSpawnedBlock->NextBlockLayer() : basementLayer.Get_BlockLayer() );
			while( pBlockToSpawn && pBlockToSpawn->pParent->pOwner->pos.y <= m_nSpawnHeight * m_nBlockSize )
			{
				pBlockToSpawn->pParent->pOwner->CreateChunkObject( this );
				basementLayer.pSpawnedBlock = pBlockToSpawn;
				pBlockToSpawn = pBlockToSpawn->NextBlockLayer();
			}
		}
	}
}
