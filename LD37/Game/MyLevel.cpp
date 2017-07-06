#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "Player.h"
#include "GUI/MainUI.h"
#include "GlobalCfg.h"
#include "Rand.h"
#include "GameState.h"
#include "LevelDesign.h"

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

	s_pLevel = this;
	StartUp();
	CheckSpawn();

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

void CMyLevel::StartUp()
{
	if( m_bIsLevelDesignTest )
	{
		auto pLevel = SafeCast<CDesignLevel>( CLevelDesignGameState::Inst().GetDesignLevel() );
		pLevel->GenerateLevel( this );
		m_nCurLevel = CGlobalCfg::Inst().vecLevels.size();
	}
	else
	{
		CreateGrids( true );
		CacheNextLevel();
	}
}

void CMyLevel::OnPlayerKilled( CPlayer * pPlayer )
{
	CMainGameState::Inst().DelayResetStage( 2.0f );
}

void CMyLevel::OnPlayerEntered( CPlayer * pPlayer )
{
	auto pWeapon = SafeCast<CPlayerWeapon>( CResourceManager::Inst()->CreateResource<CPrefab>( "weapon.pf" )->GetRoot()->CreateInstance() );
	pPlayer->AddItem( pWeapon );
}

void CMyLevel::CreateGrids( bool bNeedInit )
{
	auto& cfg = CGlobalCfg::Inst();
	SLevelBuildContext context( this );

	/*if( bNeedInit )
	{
		CLevelGenerateNode* pNode = cfg.pRootGenerateFile->FindNode( "init" );
		pNode->Generate( context, TRectangle<int32>( 0, 0, m_nWidth, m_nHeight ) );
	}*/

	CLevelGenerateNode* pNode = cfg.pRootGenerateFile->FindNode( CGlobalCfg::Inst().vecLevels[m_nCurLevel].c_str() );
	pNode->Generate( context, TRectangle<int32>( 0, 0, m_nWidth, m_nHeight ) );
	context.Build();
	uint32 nCurBarrierHeight = m_vecBarrierHeightTags.size() ? m_vecBarrierHeightTags.back() : 0;
	m_vecBarrierHeightTags.push_back( nCurBarrierHeight  + m_nHeight );
}

void CMyLevel::CacheNextLevel()
{
	if( m_nCurLevel < CGlobalCfg::Inst().vecLevels.size() )
	{
		m_nCurLevel++;
		if( m_nCurLevel < CGlobalCfg::Inst().vecLevels.size() )
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

	m_fLastScrollPos = 0;
	m_fCurLvBarrierHeight = 0;
	m_nCurBarrierHeightTag = 0;
	m_vecBarrierHeightTags.clear();
	for( int i = 0; i < ELEM_COUNT( m_pScrollObjRoot ); i++ )
	{
		auto pEntity = m_pScrollObjRoot[i];
		while( pEntity->Get_ChildEntity() )
			pEntity->Get_ChildEntity()->SetParentEntity( NULL );
	}
}

void CMyLevel::KillChunk( SChunk * pChunk, bool bCrush )
{
	while( pChunk->Get_StopEvent() )
	{
		CReference<CChunkStopEvent> pStopEvent = pChunk->Get_StopEvent();
		pStopEvent->RemoveFrom_StopEvent();
		if( pStopEvent->killedFunc )
			pStopEvent->killedFunc( pChunk );
	}
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
		if( pChunk->nSubChunkType != 2 )
			nChunks++;
	}
	newChunks.resize( nChunks );
	while( pChunk->Get_SubChunk() )
	{
		auto pSubChunk = pChunk->Get_SubChunk();
		if( pSubChunk->nSubChunkType == 2 )
		{
			pSubChunk->RemoveFrom_SubChunk();
			delete pSubChunk;
		}
		else
		{
			auto& item = newChunks[--nChunks];
			item.first = pSubChunk;
			item.second = item.first->pos;
			item.first->RemoveFrom_SubChunk();
			item.first->bIsSubChunk = false;
		}
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
						pMainUI->UpdateMinimap( pChunk->pos.x / GetBlockSize() + i, pChunk->pos.y / GetBlockSize() + j, iLayer, -1 );
					}
				}
			}
		}

		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			auto pBlock = pChunk->GetBlock( i, 0 );
			auto& basement = m_basements[i + pChunk->pos.x / GetBlockSize()];
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
	{
		CacheNextLevel();
		m_nCurBarrierHeightTag++;
	}

	delete pChunk;
}

void CMyLevel::SplitChunks( SChunk* pOldChunk, vector< pair<SChunk*, TVector2<int32> > > newChunks )
{
	uint32 nX = pOldChunk->pos.x / GetBlockSize();
	uint32 nY = pOldChunk->pos.y / GetBlockSize();
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
		int32 minX = item.second.x / GetBlockSize();
		int32 minY = item.second.y / GetBlockSize();
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
				if( minY * GetBlockSize() + pOldChunk->pos.y < pInsertAfter->pParent->pOwner->pos.y )
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
	CVector2 oldChunkOfs;
	if( pOldChunk->pChunkObject )
		oldChunkOfs = pOldChunk->pChunkObject->GetLastPos() - CVector2( pOldChunk->pos.x, pOldChunk->pos.y );
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
					pMainUI->UpdateMinimap( pNewBlock->nX + pNewBlock->pOwner->pos.x / GetBlockSize(), pNewBlock->nY + pNewBlock->pOwner->pos.y / GetBlockSize(),
						iLayer, s_nTypes[pNewBlock->eBlockType] );

				if( bSpawned )
				{
					auto basement = m_basements[i + nX];
					auto basementLayer = basement.layers[iLayer];
					if( pNewBlock->pOwner->CreateChunkObject( this ) )
					{
						for( auto& block : pNewBlock->pOwner->blocks )
						{
							block.pEntity->SetLastPos( oldChunkOfs + CVector2( block.nX, block.nY ) * CMyLevel::GetBlockSize()
								+ CVector2( pNewBlock->pOwner->pos.x, pNewBlock->pOwner->pos.y ) );
						}
					}

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

bool CMyLevel::IsReachEnd()
{
	for( auto& basement : m_basements )
	{
		for( int i = 0; i < ELEM_COUNT( basement.layers ); i++ )
		{
			auto pBlockLayer = basement.layers[i].Get_BlockLayer();
			if( pBlockLayer && !pBlockLayer->pParent->pOwner->bIsLevelBarrier )
				return false;
		}
	}
	return true;
}

void CMyLevel::UpdateBack0Position( const CVector2 & pos )
{
	float center = GetBound().GetCenterX();
	m_pBack0->SetPosition( CVector2( floor( pos.x - ( pos.x - center ) * 0.25f + 0.5f ), 0 ) );
}

CVector2 CMyLevel::GetCamPos()
{
	auto pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return CVector2( 0, 0 );
	return GetStage()->GetPlayer()->GetCam();
}

void CMyLevel::UpdateBlocksMovement()
{
	uint32 nPlayerHeight = 0;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( pPlayer->GetHp() <= 0 )
		{
			UpdateShake();
			return;
		}
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
			int32 nBaseX = pChunk->pos.x / GetBlockSize();

			if( pChunk->bForceStop )
			{
				pChunk->nFallSpeed = 0;
				pChunk->bForceStop = false;
			}
			else if( !pChunk->bStopMove )
			{
				int32 nMinY = 0;
				auto pStopEvent = pChunk->Get_StopEvent();
				if( pChunk->Get_StopEvent() )
					nMinY = pStopEvent->nHeight;
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
								nPreYs[iLayer] = pPreBlock[iLayer]->pOwner->pos.y + pPreBlock[iLayer]->pOwner->nHeight * GetBlockSize();
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
					if( pStopEvent && pStopEvent->nHeight == nMinY )
					{
						DEFINE_TEMP_REF( pStopEvent );
						pChunk->bStopMove = true;
						pChunk->bMovedLastFrame = false;
						pStopEvent->RemoveFrom_StopEvent();
						pStopEvent->func( pChunk );
					}
				}

				pChunk->bMovedLastFrame = preY != pChunk->pos.y;
				if( pChunk->bMovedLastFrame )
				{
					if( pMainUI )
					{
						uint32 y0 = preY / GetBlockSize();
						uint32 y1 = pChunk->pos.y / GetBlockSize();
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
								uint32 nAbsorbShake = pChunk->nAbsorbShakeStrength;
								shakeStrength -= nAbsorbShake;
								//uint32 nAbsorbShake = bIsCurRoom ? 0 : pChunk->nAbsorbShakeStrength;
								//shakeStrength -= Min<int32>( nAbsorbShake, nAbsorbShake *
								//	Max<int32>( 0, pChunk->nHeight * GetBlockSize() - ( nPlayerHeight - pChunk->pos.y ) ) / (int32)( pChunk->nHeight * GetBlockSize() ) );
								if( shakeStrength < 0 )
									shakeStrength = 0;
							}

							if( pPreBlock[iLayer] )
							{
								int32 nPreY = pPreBlock[iLayer]->pOwner->pos.y + pPreBlock[iLayer]->pOwner->nHeight * GetBlockSize();

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
								basementLayer.nShakeHeight = pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->nHeight + pPreBlock[iLayer]->pOwner->pos.y / GetBlockSize() : 0;
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
			else
				pChunk->bMovedLastFrame = false;
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
							uint32 nAbsorbShake = pChunk->nAbsorbShakeStrength;
							shakeStrength -= nAbsorbShake;
							//uint32 nAbsorbShake = bIsCurRoom ? 0 : pChunk->nAbsorbShakeStrength;
							//shakeStrength -= Min<int32>( nAbsorbShake, nAbsorbShake *
							//	Max<int32>( 0, pChunk->nHeight * GetBlockSize() - ( nPlayerHeight - pChunk->pos.y ) ) / (int32)( pChunk->nHeight * GetBlockSize() ) );
							if( shakeStrength < 0 )
								shakeStrength = 0;
						}
						else if( basementLayer.nShakeHeight < 0 )
						{
							basementLayer.nShakeHeight = pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->nHeight + pPreBlock[iLayer]->pOwner->pos.y / GetBlockSize() : 0;
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

	float fTargetScrollPos = 0;
	if( pLevelBarrier )
	{
		m_fCurLvBarrierHeight = pLevelBarrier->pos.y + pLevelBarrier->nBarrierHeight * GetBlockSize();
		if( m_nCurBarrierHeightTag < m_vecBarrierHeightTags.size() )
			fTargetScrollPos += m_vecBarrierHeightTags[m_nCurBarrierHeightTag] * GetBlockSize() -
			( pLevelBarrier->pos.y + pLevelBarrier->nHeight * GetBlockSize() );

		while( nUpdatedBlock < vecUpdatedBlocks.size() )
		{
			auto pBlockLayer = vecUpdatedBlocks[nUpdatedBlock++];
			auto pBlock = pBlockLayer->pParent;
			auto pChunk = pBlock->pOwner;
			auto& basement = m_basements[pBlock->nX + pChunk->pos.x / GetBlockSize()];
			pChunk->nUpdateCount++;

			if( pChunk->IsFullyUpdated() )
			{
				int32 nBaseX = pChunk->pos.x / GetBlockSize();
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
							int32 nPreY = pPreBlock[iLayer]->pOwner->pos.y + pPreBlock[iLayer]->pOwner->nHeight * GetBlockSize();
							if( nPreY > nMinY )
								nMinY = nPreY;
						}

						if( basementLayer.nShakeHeight < 0 )
						{
							basementLayer.nShakeHeight = pPreBlock[iLayer] ? pPreBlock[iLayer]->pOwner->pos.y / GetBlockSize() : 0;
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
						uint32 y0 = preY / GetBlockSize();
						uint32 y1 = pChunk->pos.y / GetBlockSize();
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
										pMainUI->UpdateMinimap( pChunk->pos.x / GetBlockSize() + i, y0 + j, iLayer, -1 );
										pMainUI->UpdateMinimap( pChunk->pos.x / GetBlockSize() + i, y1 + j, iLayer, nTileTypes[pChunk->GetBlock( i, j )->eBlockType] );
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

	UpdateShake();
	CheckSpawn();
	m_fLastScrollPos = m_fCurScrollPos;
	if( m_fCurScrollPos < fTargetScrollPos )
		m_fCurScrollPos = Min( m_fCurScrollPos + 2.0f, fTargetScrollPos );
}

void CMyLevel::UpdateShake()
{
	auto pMainUI = CMainUI::GetInst();
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
}

void CMyLevel::CheckSpawn()
{
	for( auto& basement : m_basements )
	{
		for( int iLayer = 0; iLayer < 2; iLayer++ )
		{
			auto& basementLayer = basement.layers[iLayer];
			auto pBlockToSpawn = ( basement.layers[iLayer].pSpawnedBlock ? basementLayer.pSpawnedBlock->NextBlockLayer() : basementLayer.Get_BlockLayer() );
			while( pBlockToSpawn && pBlockToSpawn->pParent->pOwner->pos.y < m_nSpawnHeight * GetBlockSize() )
			{
				pBlockToSpawn->pParent->pOwner->CreateChunkObject( this );
				basementLayer.pSpawnedBlock = pBlockToSpawn;
				pBlockToSpawn = pBlockToSpawn->NextBlockLayer();
			}
		}
	}
}
