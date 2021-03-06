#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "Entities/Neutral.h"
#include "GUI/MainUI.h"
#include "GlobalCfg.h"
#include "Rand.h"
#include "GameState.h"
#include "LevelDesign.h"
#include "PlayerData.h"
#include "Render/Scene2DManager.h"
#include "Render/GlobalRenderResources.h"
#include "Render/CommonShader.h"

CMyLevel* CMyLevel::s_pLevel = NULL;
int8 CMyLevel::s_nTypes[] = { 0, 1, 1, 2, 3 };

void CMyLevel::OnAddedToStage()
{
	m_pBlockElemRoot = new CRenderObject2D;
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( m_pBlockElemRoot );

	m_freedBlockRTRects.reserve( 32 * 32 * 2 );
	for( int i = 0; i < 32 * 32 * 2; i++ )
	{
		m_freedBlockRTRects.push_back( i );
	}

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
		pBottom->SetHitType( eEntityHitType_System );
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

	for( int i = 0; i < ELEM_COUNT( m_pBulletRoot ); i++ )
	{
		m_pBulletRoot[i] = new CEntity;
		m_pBulletRoot[i]->SetParentEntity( this );
	}

	s_pLevel = this;
	StartUp();
	CheckSpawn();
	GetStage()->SetNavigationProvider( this );
}

void CMyLevel::OnRemovedFromStage()
{
	m_pBlockElemRoot->RemoveThis();
	m_pBlockElemRoot = NULL;
	GetStage()->SetNavigationProvider( NULL );
	if( s_pLevel == this )
		s_pLevel = NULL;
}

void CMyLevel::StartUp()
{
	if( m_bIsLevelDesignTest )
	{
		const char* szTestLevel = CLevelDesignGameState::Inst().GetTestLevel();
		if( szTestLevel[0] )
			CreateGrids( szTestLevel );
		else
		{
			auto pLevel = SafeCast<CDesignLevel>( CLevelDesignGameState::Inst().GetDesignLevel() );
			pLevel->GenerateLevel( this );
		}
	}
	else
	{
		CreateGrids( CGlobalCfg::Inst().strMainMenuLevel.c_str() );
	}
	m_nCurLevel = m_nGenLevel = CGlobalCfg::Inst().vecLevels.size();
}

void CMyLevel::BeginGenLevel( int32 nLevel )
{
	m_bBeginGenLevel = true;
	m_nCurLevel = m_nGenLevel = nLevel;
	CreateGrids( CGlobalCfg::Inst().vecLevels[m_nGenLevel].c_str() );
}

void CMyLevel::OnPlayerKilled( CPlayer * pPlayer )
{
	CMainGameState::Inst().DelayResetStage( 2.0f );
}

void CMyLevel::OnPlayerEntered( CPlayer * pPlayer )
{
	auto pWeapon = SafeCast<CPlayerWeapon>( CResourceManager::Inst()->CreateResource<CPrefab>( "weapon.pf" )->GetRoot()->CreateInstance() );
	pPlayer->AddItem( pWeapon );
	pPlayer->GetBlockDetectUI()->SetRenderParentBefore( m_pBulletRoot[eBulletLevel_Player] );
	pPlayer->GetBlockDetectUI()->SetShown( true );
	if( m_bIsBonusStage )
		pPlayer->OnBonusStageBegin();
}

CChunkObject* CMyLevel::TrySpawnAt( CVector2& pos, SHitProxy* pHitProxy )
{
	CMatrix2D transform;
	CRectangle bound;
	transform.Identity();
	pHitProxy->CalcBound( transform, bound );
	transform.Translate( pos.x, pos.y );
	vector<CReference<CEntity> > vecHit;
	vector<SHitTestResult> result;
	GetStage()->MultiHitTest( pHitProxy, transform, vecHit, &result );

	bool bOK = false;
	CRectangle r;
	CChunkObject* pChunkObject;
	for( int i = 0; i < vecHit.size(); i++ )
	{
		auto pBlockObject = SafeCast<CBlockObject>( vecHit[i].GetPtr() );
		if( pBlockObject && ( pBlockObject->GetHitType() > eEntityHitType_Platform && pBlockObject->GetHitType() < eEntityHitType_System ) )
		{
			auto pChunk = pBlockObject->GetBlock()->pOwner;
			CRectangle rect( pChunk->pChunkObject->globalTransform.GetPosition().x, pChunk->pChunkObject->globalTransform.GetPosition().y,
				pChunk->nWidth * CMyLevel::GetBlockSize(), pChunk->nHeight * CMyLevel::GetBlockSize() );
			if( rect.Contains( pos ) )
			{
				pChunkObject = pChunk->pChunkObject;
				r = rect;
				bOK = true;
				break;
			}
		}
	}
	if( !bOK )
		return NULL;

	CVector2 ofs( 0, 0 );
	for( int i = 0; i < vecHit.size(); i++ )
	{
		auto pEntity = vecHit[i].GetPtr();
		if( pEntity && pEntity->GetHitType() <= eEntityHitType_WorldStatic )
		{
			auto hitResult = result[i];
			float fDot = hitResult.normal.Dot( ofs );
			if( fDot * fDot >= hitResult.normal.Length2() )
				continue;
			else if( fDot * fDot >= ofs.Length2() )
				ofs = hitResult.normal;
			else
			{
				float a1 = ofs.x, b1 = ofs.y, c1 = 2;
				float a2 = hitResult.normal.x, b2 = hitResult.normal.y, c2 = 2;
				float d = a1 * b2 - a2 * b1;
				if( d == 0 )
				{
					bOK = false;
					break;
				}
				float x = ( c1 * b1 - c2 * b2 ) * a1 * a2 / ( b1 * a2 - b2 * a1 );
				float y = ( c1 * a1 - c2 * a2 ) * b1 * b2 / ( a1 * b2 - a2 * b1 );
				if( abs( x ) > bound.width || abs( y ) > bound.height )
				{
					bOK = false;
					break;
				}
				ofs = CVector2( x, y );
			}
		}
	}
	if( !bOK )
		return NULL;

	bool bCheck = false;
	if( ofs.Length2() > 0 )
	{
		bCheck = true;
		ofs = ofs + ofs * ( 0.125f / ofs.Length() );
	}
	CVector2 p1 = pos + ofs;
	CRectangle bound1 = bound.Offset( p1 );
	if( bound1.x < r.x )
	{
		bound1.x = r.x;
		bCheck = true;
	}
	if( bound1.y < r.y )
	{
		bound1.y = r.y;
		bCheck = true;
	}
	if( bound1.GetRight() > r.GetRight() )
	{
		bound1.x = r.GetRight() - bound1.width;
		bCheck = true;
	}
	if( bound1.GetBottom() > r.GetBottom() )
	{
		bound1.y = r.GetBottom() - bound1.height;
		bCheck = true;
	}
	p1 = CVector2( bound1.x - bound.x, bound1.y - bound.y );
	if( bCheck )
	{
		transform.Translate( p1.x, p1.y );
		vecHit.clear();
		result.clear();
		GetStage()->MultiHitTest( pHitProxy, transform, vecHit );

		for( int i = 0; i < vecHit.size(); i++ )
		{
			auto pEntity = vecHit[i].GetPtr();
			if( pEntity && pEntity->GetHitType() <= eEntityHitType_WorldStatic )
				return NULL;
		}
	}

	pos = p1;
	return pChunkObject;
}

SChunk* CMyLevel::CreateGrids( const char* szNode, SChunk* pLevelBarrier )
{
	auto& cfg = CGlobalCfg::Inst();
	SLevelBuildContext context( this );
	context.pLastLevelBarrier = pLevelBarrier;
	CLevelGenerateNode* pNode = cfg.pRootGenerateFile->FindNode( szNode );
	if( pNode->GetMetadata().nSeed )
		SRand::Inst().nSeed = pNode->GetMetadata().nSeed;
	static int32 nSeed;
	nSeed = SRand::Inst().nSeed;
	pNode->Generate( context, TRectangle<int32>( 0, 0, m_nWidth, m_nHeight ) );
	context.Build();
	uint32 nCurBarrierHeight = m_vecBarrierHeightTags.size() ? m_vecBarrierHeightTags.back() : 0;
	m_vecBarrierHeightTags.push_back( nCurBarrierHeight + context.nMaxChunkHeight );
	return context.pLastLevelBarrier;
}

SChunk* CMyLevel::CacheNextLevel( SChunk* pLevelBarrier )
{
	if( m_nGenLevel < CGlobalCfg::Inst().vecLevels.size() )
	{
		m_nGenLevel++;
		if( m_nGenLevel < CGlobalCfg::Inst().vecLevels.size() )
			return CreateGrids( CGlobalCfg::Inst().vecLevels[m_nGenLevel].c_str() );
	}
	return NULL;
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

void CMyLevel::KillChunk( SChunk * pChunk, bool bCrush, CChunkObject* pPreObject )
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
	if( !pChunk->Get_SubChunk() || pChunk->pParentChunk )
	{
		RemoveChunk( pChunk );
		return;
	}

	while( pChunk->Get_Chain1() )
		RemoveChain( pChunk->Get_Chain1() );
	while( pChunk->Get_Chain2() )
		RemoveChain( pChunk->Get_Chain2() );
	vector< pair<SChunk*, TVector2<int32> > > newChunks;
	uint32 nChunks = 0;
	for( auto pSubChunk = pChunk->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
	{
		if( pSubChunk->nSubChunkType != 2 )
			nChunks++;
	}
	newChunks.resize( nChunks );
	while( pChunk->Get_SubChunk() )
	{
		auto pSubChunk = pChunk->Get_SubChunk();
		if( pSubChunk->nSubChunkType == 2 )
		{
			if( pSubChunk->pChunkObject )
				pSubChunk->pChunkObject->Kill();
			else
			{
				pSubChunk->RemoveFrom_SubChunk();
				delete pSubChunk;
			}
		}
		else
		{
			auto& item = newChunks[--nChunks];
			item.first = pSubChunk;
			item.second = item.first->pos;
			item.first->RemoveFrom_SubChunk();
			item.first->bIsSubChunk = false;

			if( pSubChunk->nLevelBarrierType )
				pChunk->nLevelBarrierType = 0;
		}
	}

	SplitChunks( pChunk, newChunks, pPreObject );
}

void CMyLevel::RemoveChunk( SChunk* pChunk )
{
	if( pChunk->pChunkObject )
	{
		pChunk->pChunkObject->SetParentEntity( NULL );
		return;
	}

	while( pChunk->Get_Chain1() )
		RemoveChain( pChunk->Get_Chain1() );
	while( pChunk->Get_Chain2() )
		RemoveChain( pChunk->Get_Chain2() );

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
					auto pPrev = pBlockLayer->GetPrev( basement.layers[j].Get_BlockLayer() );
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

		m_fShakeStrength += pChunk->nDestroyShake;
	}
	else
		pChunk->RemoveFrom_SubChunk();

	while( pChunk->Get_SubChunk() )
	{
		auto pSubChunk = pChunk->Get_SubChunk();
		pSubChunk->bIsSubChunk = true;
		RemoveChunk( pSubChunk );
	}

	if( pChunk->nLevelBarrierType )
	{
		CPlayerData::Inst().nPassedLevels = Max<int32>( CPlayerData::Inst().nPassedLevels, m_nCurLevel );
		CPlayerData::Inst().Save();
		m_nCurLevel++;
		m_nCurBarrierHeightTag++;

		if( pChunk->nLevelBarrierType == 2 )
			BeginBonusStage();
		else if( pChunk->nLevelBarrierType == 3 )
			EndBonusStage();
	}

	delete pChunk;
}

void CMyLevel::SplitChunks( SChunk* pOldChunk, vector< pair<SChunk*, TVector2<int32> > >& newChunks, CChunkObject* pPreObject )
{
	pOldChunk->pChunkObject = pPreObject;
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
			for( auto pNewBlockLayer = ppInsertAfter[iLayer][i]->NextBlockLayer(); pNewBlockLayer != ppInsertBefore[iLayer][i]; pNewBlockLayer = pNewBlockLayer->NextBlockLayer() )
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

	if( bSpawned )
	{
		for( auto& item : newChunks )
		{
			if( item.first->nSubChunkType == 1 )
				continue;
			int32 minX = item.second.x / GetBlockSize();
			int32 minY = item.second.y / GetBlockSize();
			for( auto& block : item.first->blocks )
			{
				auto pOldBlock = pOldChunk->GetBlock( block.nX + minX, block.nY + minY );
				if( pOldBlock->pEntity )
				{
					auto pBlockObject = static_cast<CBlockObject*>( pOldBlock->pEntity.GetPtr() );
					if( pBlockObject->m_pBlockRTObject )
					{
						auto pBlockObject1 = static_cast<CBlockObject*>( block.pEntity.GetPtr() );
						pBlockObject1->m_nBlockRTIndex = pBlockObject->m_nBlockRTIndex;
						item.first->pChunkObject->CreateBlockRTLayer( pBlockObject1 );

						pBlockObject->m_nBlockRTIndex = -1;
						pBlockObject->m_pBlockRTObject->RemoveThis();
						pBlockObject->m_pBlockRTObject = NULL;
					}
				}
			}
		}
	}
	for( auto& item : newChunks )
	{
		item.first->pParentChunk = NULL;
	}

	while( pOldChunk->Get_ChainEf1() )
		pOldChunk->Get_ChainEf1()->Init();
	while( pOldChunk->Get_ChainEf2() )
		pOldChunk->Get_ChainEf2()->Init();
	pOldChunk->pChunkObject = NULL;
	RemoveChunk( pOldChunk );
}

void CMyLevel::RemoveChain( SChain* pChain )
{
	if( pChain->pChainObject )
	{
		pChain->pChainObject->SetParentEntity( NULL );
		return;
	}
	if( pChain->pEf1 )
	{
		pChain->pEf1 = NULL;
		pChain->RemoveFrom_ChainEf2();
	}
	if( pChain->pEf2 )
	{
		pChain->pEf2 = NULL;
		pChain->RemoveFrom_ChainEf1();
	}
	if( pChain->p1 )
	{
		pChain->p1 = NULL;
		pChain->RemoveFrom_Chain1();
	}
	if( pChain->p2 )
	{
		pChain->p2 = NULL;
		pChain->RemoveFrom_Chain2();
	}
	delete pChain;
}

bool CMyLevel::IsReachEnd()
{
	for( auto& basement : m_basements )
	{
		for( int i = 0; i < ELEM_COUNT( basement.layers ); i++ )
		{
			auto pBlockLayer = basement.layers[i].Get_BlockLayer();
			if( pBlockLayer && !pBlockLayer->pParent->pOwner->nLevelBarrierType )
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

void CMyLevel::BeginBonusStage()
{
	if( m_bIsBonusStage )
		return;
	m_bIsBonusStage = true;
	auto pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
		pPlayer->OnBonusStageBegin();
	IRenderSystem::Inst()->SetTimeScale( 0.0f, 0.25f );
}

void CMyLevel::EndBonusStage()
{
	if( !m_bIsBonusStage )
		return;
	m_bIsBonusStage = false;
	m_bIsBonusStageEnd = false;
	auto pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
		pPlayer->OnBonusStageEnd();
}

void CMyLevel::OnPlayerBonusStageCrushed()
{
	if( !m_bIsBonusStage || m_bIsBonusStageEnd )
		return;
	m_bIsBonusStageEnd = true;
	IRenderSystem::Inst()->SetTimeScale( 0.0f, 0.25f );
}

SChunk * CMyLevel::GetCurLevelBarrier()
{
	for( auto pBlock = m_basements[0].layers[0].Get_BlockLayer(); pBlock; pBlock = pBlock->NextBlockLayer() )
	{
		if( pBlock->pParent->pOwner->nLevelBarrierType )
			return pBlock->pParent->pOwner;
	}
	return NULL;
}

void CMyLevel::UpdateBlocksMovement()
{
	m_vecUpdatedBlocks.reserve( 32 * 128 * 2 );
	m_vecUpdatedChunks.reserve( 32 * 128 );

	if( m_bIsBonusStage )
	{
		if( m_bIsBonusStageEnd )
			UpdateBlocksMovementBonusStageEnd();
		else
			UpdateBlocksMovementBonusStage();
	}
	else 
		UpdateBlocksMovementNormal();
	m_fShakeStrengthCurFrame = 0;

	SChunk* pLevelBarrier = NULL;
	for( auto pBlock = m_basements[0].layers[0].Get_BlockLayer(); pBlock; pBlock = pBlock->NextBlockLayer() )
	{
		if( pBlock->pParent->pOwner->nLevelBarrierType )
			pLevelBarrier = pBlock->pParent->pOwner;
	}

	while( pLevelBarrier && pLevelBarrier->pos.y + pLevelBarrier->nHeight * GetBlockSize() <= m_nHeight * GetBlockSize() )
		pLevelBarrier = CacheNextLevel( pLevelBarrier );

	m_vecUpdatedBlocks.resize( 0 );
	m_vecUpdatedChunks.resize( 0 );
}

void CMyLevel::UpdateBlocksMovementNormal()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( pPlayer->GetHp() <= 0 )
		{
			UpdateShake();
			return;
		}
	}

	int8 nTileTypes[] = { 0, 1, 1, 2, 3 };
	auto pMainUI = CMainUI::GetInst();
	auto& vecUpdatedBlocks = m_vecUpdatedBlocks;
	auto& vecUpdatedChunks = m_vecUpdatedChunks;
	uint32 nUpdatedBlock = 0;
	int32 nCurShakeStrength = ceil( m_fShakeStrength + m_fShakeStrengthCurFrame );
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
			basement.layers[iLayer].nCurShakeStrength = nCurShakeStrength;
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
			vecUpdatedChunks.push_back( pChunk );
			bool bHit = false;
			bool b1 = false;
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
				const int32 preY = pChunk->pos.y;
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
				int32 nMinY1 = nMinY;

				for( auto pChain = pChunk->Get_ChainEf2(); pChain; pChain = pChain->NextChainEf2() )
				{
					auto pEf2 = pChain->pEf2;
					if( pEf2 != pChunk )
					{
						int32 y1 = pEf2->pos.y - pChain->nYLim;
						if( y1 > nMinY )
						{
							nMinY = y1;
							nMaxFallSpeed = pEf2->nFallSpeed;
						}
						else if( y1 == nMinY )
						{
							nMaxFallSpeed = Min( nMaxFallSpeed, pEf2->nFallSpeed );
						}
					}
				}

				if( pChunk->nFallSpeed < 40 )
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
					for( int i = 0; i < pChunk->nWidth; i++ )
					{
						auto& basement = m_basements[nBaseX + i];

						SBlockLayer* pPreBlockLayer[2];
						SBlock* pPreBlock[2];
						for( int iLayer = 0; iLayer < 2; iLayer++ )
						{
							auto p = basement.layers[iLayer].pVisitedBlock;
							pPreBlockLayer[iLayer] = p;
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
								if( shakeStrength < 0 )
									shakeStrength = 0;
							}

							if( pPreBlock[iLayer] )
							{
								int32 nPreY = pPreBlock[iLayer]->pOwner->pos.y + pPreBlock[iLayer]->pOwner->nHeight * GetBlockSize();

								if( nPreY >= preY - basementLayer.nCurMargin && !pPreBlock[iLayer]->pOwner->bMovedLastFrame )
								{
									if( !( iLayer == 1 && pPreBlock[0] == pPreBlock[1] ) && pChunk->pChunkObject )
										pPreBlockLayer[iLayer]->fAppliedWeight = fAppliedWeight;
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
					b1 = true;
				}
				else
				{
					bHit = pChunk->pos.y == nMinY1 && pChunk->nFallSpeed == 0;
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
			if( pChunk->nLevelBarrierType )
			{
				pLevelBarrier = pChunk;
				break;
			}
		}
	}

	int32 nChunks0 = vecUpdatedChunks.size();
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
				vecUpdatedChunks.push_back( pChunk );
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

				for( auto pChain = pChunk->Get_ChainEf2(); pChain; pChain = pChain->NextChainEf2() )
				{
					auto pEf2 = pChain->pEf2;
					if( pEf2 != pChunk )
					{
						int32 y1 = pEf2->pos.y - pChain->nYLim;
						if( y1 > nMinY )
							nMinY = y1;
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
			}
		}
	}
	else
		m_fCurLvBarrierHeight = 100000;

	for( int iChunk = vecUpdatedChunks.size() - 1; iChunk >= 0; iChunk-- )
	{
		auto pChunk = vecUpdatedChunks[iChunk];
		pChunk->nUpdateCount = 0;

		for( auto pChain = pChunk->Get_ChainEf2(); pChain; pChain = pChain->NextChainEf2() )
		{
			if( pChain->pChainObject )
				pChain->pChainObject->Update();
		}

		if( iChunk <= nChunks0 )
		{
			if( pChunk->fDestroyBalance > 1 )
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

			int32 nBaseX = pChunk->pos.x / GetBlockSize();
			float fAppliedWeights[2] = { 0, 0 };

			for( int iLayer = 0; iLayer < 2; iLayer++ )
			{
				if( !pChunk->HasLayer( iLayer ) )
					continue;
				float& fWeight = fAppliedWeights[iLayer];
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					auto pBlock = pChunk->GetBlock( i, 0 );
					fWeight += pBlock->layers[iLayer].fAppliedWeight;
					pBlock->layers[iLayer].fAppliedWeight = 0;
				}
			}
			float fAppliedWeight = Max( fAppliedWeights[0], fAppliedWeights[1] );
			float wCoef = 1;
			if( pChunk->fDestroyWeight > 0 && fAppliedWeight >= pChunk->fDestroyWeight )
			{
				if( pChunk->fShakeWeightCoef == 0 || pChunk->pos.y == 0 )
				{
					KillChunk( pChunk, true );
					continue;
				}
				wCoef = 1 + pChunk->fShakeWeightCoef * ( fAppliedWeight - pChunk->fDestroyWeight );
			}

			if( wCoef > 1 )
			{
				for( int iLayer = 0; iLayer < 2; iLayer++ )
				{
					if( !pChunk->HasLayer( iLayer ) )
						continue;

					for( int i = 0; i < pChunk->nWidth; i++ )
					{
						auto& basement = m_basements[nBaseX + i];
						auto pBlock = pChunk->GetBlock( i, 0 );

						auto pPreLayer = pBlock->layers[iLayer].GetPrev( basement.layers[iLayer].Get_BlockLayer() );
						bool bHit = false;

						if( pPreLayer )
							pPreLayer->fAppliedWeight *= wCoef;
					}
				}
			}

			int32 nShakeDmg = pChunk->nCurShakeStrength - pChunk->nShakeDmgThreshold;
			pChunk->nCurShakeStrength = 0;
			if( nShakeDmg > 0 && pChunk->pChunkObject )
			{
				float fDmg = pChunk->fShakeDmg * Min<int32>( nShakeDmg, 64 ) * wCoef * GetStage()->GetElapsedTimePerTick();
				if( pChunk->bIsBeingRepaired )
					fDmg *= 0.2f;
				pChunk->pChunkObject->Damage( fDmg );
			}
		}
	}

	UpdateShake();
	CheckSpawn();
	m_fLastScrollPos = m_fCurScrollPos;
	if( m_fCurScrollPos < fTargetScrollPos )
		m_fCurScrollPos = Min( m_fCurScrollPos + Max( 1.0f, ceil( ( fTargetScrollPos - m_fCurScrollPos ) * 0.015f ) ), fTargetScrollPos );
}

void CMyLevel::UpdateBlocksMovementBonusStage()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( pPlayer->GetHp() <= 0 )
		{
			UpdateShake();
			return;
		}
	}

	int8 nTileTypes[] = { 0, 1, 1, 2, 3 };
	auto pMainUI = CMainUI::GetInst();
	auto& vecUpdatedBlocks = m_vecUpdatedBlocks;
	auto& vecUpdatedChunks = m_vecUpdatedChunks;
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
			basement.layers[iLayer].nCurShakeStrength = 0;
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
					}
					else
						nMinY = Max<int32>( nMinY, pBlock->nLowerMargin );
				}

				for( auto pChain = pChunk->Get_ChainEf2(); pChain; pChain = pChain->NextChainEf2() )
				{
					auto pEf2 = pChain->pEf2;
					if( pEf2 != pChunk )
					{
						int32 y1 = pEf2->pos.y - pChain->nYLim;
						if( y1 > nMinY )
						{
							nMinY = y1;
							nMaxFallSpeed = pEf2->nFallSpeed;
						}
						else if( y1 == nMinY )
						{
							nMaxFallSpeed = Min( nMaxFallSpeed, pEf2->nFallSpeed );
						}
					}
				}

				if( pChunk->nFallSpeed < 40 )
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
				}

			}
			else
				pChunk->bMovedLastFrame = false;

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
			if( pChunk->nLevelBarrierType )
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

				for( auto pChain = pChunk->Get_ChainEf2(); pChain; pChain = pChain->NextChainEf2() )
				{
					auto pEf2 = pChain->pEf2;
					if( pEf2 != pChunk )
					{
						int32 y1 = pEf2->pos.y - pChain->nYLim;
						if( y1 > nMinY )
							nMinY = y1;
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
			}
		}
	}
	else
		m_fCurLvBarrierHeight = 100000;

	for( auto pBlockLayer : vecUpdatedBlocks )
	{
		auto pBlock = pBlockLayer->pParent;
		auto pChunk = pBlock->pOwner;
		pChunk->nUpdateCount--;

		if( !pChunk->nUpdateCount )
		{
			for( auto pChain = pChunk->Get_ChainEf2(); pChain; pChain = pChain->NextChainEf2() )
			{
				if( pChain->pChainObject )
					pChain->pChainObject->Update();
			}

			if( pChunk->pos.y == 0 )
			{
				if( !pChunk->bMovedLastFrame )
				{
					if( pChunk->pChunkObject )
					{
						if( !pChunk->pChunkObject->CanDamage() )
							KillChunk( pChunk );
						else
						{
							float fDmg = pChunk->fShakeDmg * 50 * GetStage()->GetElapsedTimePerTick();
							pChunk->pChunkObject->Damage( fDmg );
						}
						continue;
					}
					
				}
			}

			/*int32 nShakeDmg = pChunk->nCurShakeStrength - pChunk->nShakeDmgThreshold;
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
			}*/
		}
	}

	UpdateShake();
	CheckSpawn();
	m_fLastScrollPos = m_fCurScrollPos;
	if( m_fCurScrollPos < fTargetScrollPos )
		m_fCurScrollPos = Min( m_fCurScrollPos + Max( 1.0f, ceil( ( fTargetScrollPos - m_fCurScrollPos ) * 0.015f ) ), fTargetScrollPos );
}

void CMyLevel::UpdateBlocksMovementBonusStageEnd()
{
	uint32* indices = (uint32*)alloca( m_basements.size() * sizeof( uint32 ) );
	for( int i = 0; i < m_basements.size(); i++ )
		indices[i] = i;
	SRand::Inst().Shuffle( indices, m_basements.size() );
	int n = 0;
	for( int i = 0; i < m_basements.size() && n < 8; i++ )
	{
		auto& basement = m_basements[indices[i]];
		for( int iLayer = 0; iLayer < ELEM_COUNT( basement.layers ); iLayer++ )
		{
			auto pBlockLayer = basement.layers[iLayer].Get_BlockLayer();
			if( !pBlockLayer )
				continue;
			auto pChunk = pBlockLayer->pParent->pOwner;
			if( pChunk->nLevelBarrierType )
				continue;
			KillChunk( pChunk, true );
			n++;
			if( n >= 8 )
				break;
		}
	}
	SChunk* pLevelBarrier = GetCurLevelBarrier();
	if( pLevelBarrier )
	{
		if( pLevelBarrier->pChunkObject )
			pLevelBarrier->pChunkObject->Kill();

		int32 maxY = pLevelBarrier->pos.y + pLevelBarrier->nHeight * GetBlockSize();
		n = 0;
		LINK_LIST_FOR_EACH_BEGIN( pEntity, m_pChildrenEntity, CEntity, ChildEntity )
			if( pEntity->y <= maxY )
			{
				auto pEnemy = SafeCast<CEnemy>( pEntity );
				bool bKill = false;
				if( pEnemy )
				{
					pEnemy->Kill();
					bKill = true;
				}
				auto pPickUp = SafeCast<CPickupCarrier>( pEntity );
				if( pPickUp )
				{
					pPickUp->Kill();
					bKill = true;
				}

				if( bKill )
				{
					n++;
					if( n >= 8 )
						break;
				}
			}
		LINK_LIST_FOR_EACH_END( pEntity, m_pChildrenEntity, CEntity, ChildEntity )
	}
	if( n == 0 )
		UpdateBlocksMovementBonusStage();
}

class CBlockVertexShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CBlockVertexShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_invDstSrcResolution, "InvDstSrcResolution" );
		GetShader()->GetShaderInfo().Bind( m_depth, "fDepth" );
		GetShader()->GetShaderInfo().Bind( m_inst, "g_insts", "InstBuffer" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, void* pInstData, uint32 nDataSize, const CVector2& DstResolution, const CVector2& SrcResolution, float fDepth = 0 )
	{
		CVector4 invDstSrcResolution( 1.0f / DstResolution.x, 1.0f / DstResolution.y, 1.0f / SrcResolution.x, 1.0f / SrcResolution.y );
		m_invDstSrcResolution.Set( pRenderSystem, &invDstSrcResolution );
		m_depth.Set( pRenderSystem, &fDepth );
		m_inst.InvalidConstantBuffer( pRenderSystem );
		m_inst.Set( pRenderSystem, pInstData, nDataSize );
	}
private:
	CShaderParam m_invDstSrcResolution;
	CShaderParam m_depth;
	CShaderParam m_inst;
};

IMPLEMENT_GLOBAL_SHADER( CBlockVertexShader, "Shader/Blocks.shader", "VSScreen", "vs_5_0" );

void CMyLevel::UpdateBlockRT()
{
	if( !m_pBlockElemRoot->Get_RenderChild() )
		return;

	vector<CReference<CEntity> > hitEntities;
	vector<CBlockObject*> hitBlocks;
	for( auto pChild = m_pBlockElemRoot->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		CRectangle& rect = pChild->globalAABB;

		SHitProxyPolygon polygon;
		polygon.nVertices = 4;
		polygon.vertices[0] = CVector2( rect.x, rect.y );
		polygon.vertices[1] = CVector2( rect.x + rect.width, rect.y );
		polygon.vertices[2] = CVector2( rect.x + rect.width, rect.y + rect.height );
		polygon.vertices[3] = CVector2( rect.x, rect.y + rect.height );
		polygon.CalcNormals();

		GetStage()->MultiHitTest( &polygon, globalTransform, hitEntities );
		for( CEntity* pEntity : hitEntities )
		{
			auto pBlockObject = SafeCast<CBlockObject>( pEntity );
			if( pBlockObject && !pBlockObject->m_bBlockRTActive )
			{
				pBlockObject->m_bBlockRTActive = true;
				hitBlocks.push_back( pBlockObject );
			}
		}
		hitEntities.resize( 0 );
	}

	if( !hitBlocks.size() )
		return;

	CReference<ITexture> pTex;
	IRenderSystem* pSystem = IRenderSystem::Inst();
	auto rtDesc = m_pBlockRT->GetTexture()->GetDesc();
	rtDesc.nDim1 /= 2;
	auto& rtPool = CRenderTargetPool::GetSizeIndependentPool();
	{
		CVector2 rtSize( rtDesc.nDim1, rtDesc.nDim2 );
		CRenderContext2D context;
		context.pRenderSystem = pSystem;
		context.screenRes = rtSize;
		context.lightMapRes = rtSize;
		context.dTime = pSystem->GetElapsedTime();
		context.nTimeStamp = 0;
		context.eRenderPass = eRenderPass_Color;
		context.rectScene = CRectangle( 0, 0, 1024, 1024 );
		context.rectViewport = CRectangle( 0, 0, rtSize.x, rtSize.y );
		SRenderGroup renderGroup;
		context.renderGroup = &renderGroup;
		context.Render( m_pBlockElemRoot );

		rtPool.AllocRenderTarget( pTex, rtDesc );
		pSystem->SetRenderTarget( pTex->GetRenderTarget(), NULL );
		pSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ) );

		SViewport viewport = {
			context.rectViewport.x,
			context.rectViewport.y,
			context.rectViewport.width,
			context.rectViewport.height,
			0,
			1
		};
		pSystem->SetViewports( &viewport, 1 );
		if( context.GetElemCount() )
		{
			context.FlushElements();
		}

		while( context.pUpdatedObjects )
		{
			CRenderObject2D* pObject = context.pUpdatedObjects;
			pObject->SetUpdated( false );
			pObject->RemoveFrom_UpdatedObject();
		}
	}

	pSystem->SetDepthStencilState( IDepthStencilState::Get<false>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CBlockVertexShader::Inst();
	auto pPixelShader = COneTexturePixelShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	pSystem->SetRenderTarget( m_pBlockRT->GetRenderTarget(), NULL );
	SViewport viewport = { 0, 0, rtDesc.nDim1 * 2, rtDesc.nDim2, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pPixelShader->SetParams( pSystem, pTex->GetShaderResource(),
		ISamplerState::Get<ESamplerFilterPPP, ETextureAddressModeBorder, ETextureAddressModeBorder, ETextureAddressModeBorder,
		0, 16, EComparisonNever, 0, 0, 0, 0>() );

	pSystem->SetBlendState( IBlendState::Get<>() );
	CVector4 instData[4096];
	int32 nInstData = 0;
	for( int i = 0; i < hitBlocks.size(); i++ )
	{
		auto pBlock = hitBlocks[i];

		pBlock->m_bBlockRTActive = false;
		auto pInstData = instData + nInstData * 2;
		if( pBlock->m_nBlockRTIndex < 0 )
		{
			pBlock->m_nBlockRTIndex = AllocBlockRTRect();
			pBlock->GetBlock()->pOwner->pChunkObject->CreateBlockRTLayer( pBlock );
			pInstData[0] = CVector4( pBlock->m_nBlockRTIndex & 63, 31 - ( ( pBlock->m_nBlockRTIndex >> 6 ) & 31 ), 1, 1 ) * 32;
			pInstData[1] = CVector4( -1, -1, 0, 0 );
			nInstData++;
			if( nInstData >= 2048 )
			{
				pVertexShader->SetParams( pSystem, instData, nInstData * sizeof( CVector4 ) * 2, CVector2( 32 * 64, 32 * 32 ), CVector2( 32 * 32, 32 * 32 ), 0 );
				pSystem->DrawInputInstanced( nInstData );
				nInstData = 0;
			}
		}
	}
	if( nInstData )
	{
		pVertexShader->SetParams( pSystem, instData, nInstData * sizeof( CVector4 ) * 2, CVector2( 32 * 64, 32 * 32 ), CVector2( 32 * 32, 32 * 32 ), 0 );
		pSystem->DrawInputInstanced( nInstData );
		nInstData = 0;
	}

	pSystem->SetBlendState( IBlendState::Get<false, false, 0xf, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd, EBlendOne, EBlendInvSrcAlpha, EBlendOpAdd>() );
	nInstData = 0;
	for( int i = 0; i < hitBlocks.size(); i++ )
	{
		auto pBlock = hitBlocks[i];

		pBlock->m_bBlockRTActive = false;
		auto pInstData = instData + nInstData * 2;

		pInstData = instData + nInstData * 2;
		pInstData[0] = CVector4( pBlock->m_nBlockRTIndex & 63, 31 - ( ( pBlock->m_nBlockRTIndex >> 6 ) & 31 ), 1, 1 ) * 32;
		pInstData[1] = CVector4( pBlock->globalTransform.GetPosition().x, 32 * 32 - 32 - pBlock->globalTransform.GetPosition().y, 32, 32 );
		nInstData++;
		if( nInstData >= 2048 )
		{
			pVertexShader->SetParams( pSystem, instData, nInstData * sizeof( CVector4 ) * 2, CVector2( 32 * 64, 32 * 32 ), CVector2( 32 * 32, 32 * 32 ), 0 );
			pSystem->DrawInputInstanced( nInstData );
			nInstData = 0;
		}
	}
	if( nInstData )
	{
		pVertexShader->SetParams( pSystem, instData, nInstData * sizeof( CVector4 ) * 2, CVector2( 32 * 64, 32 * 32 ), CVector2( 32 * 32, 32 * 32 ), 0 );
		pSystem->DrawInputInstanced( nInstData );
		nInstData = 0;
	}

	rtPool.Release( pTex );
}

uint8 CMyLevel::GetNavigationData( const TVector2<int32>& pos )
{
	auto& hitTestMgr = GetStage()->GetHitTestMgr();
	auto pGrid = hitTestMgr.GetGrid( pos );
	if( !pGrid )
		return 0;
	for( auto pProxyGrid = pGrid->Get_InGrid(); pProxyGrid; pProxyGrid = pProxyGrid->NextInGrid() )
	{
		auto pEntity = static_cast<CEntity*>( pProxyGrid->pHitProxy->pOwner );
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			return 0;
		auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject )
		{
			if( pBlockObject->GetBlock()->pOwner->nMoveType )
				return 2;
		}
	}
	return 1;
}

TRectangle<int32> CMyLevel::GetMapRect()
{
	return TRectangle<int32>( 0, 0, m_nWidth, m_nSpawnHeight );
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
		pMainUI->UpdateShakeSmallBar( i, nShakeHeight >= 0 ? Min( nShakeHeight, (int32)m_nHeight ) : (int32)m_nHeight );
	}
	m_fShakeStrength = Max( 0.0f, m_fShakeStrength * 0.95f - 0.02f );
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


void Game_ShaderImplement_Dummy_Level()
{

}