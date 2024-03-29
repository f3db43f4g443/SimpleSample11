#include "stdafx.h"
#include "LvBarriers.h"
#include "Stage.h"
#include "Player.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Entities/EffectObject.h"
#include "Entities/Bullets.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "Pickup.h"
#include "GlobalCfg.h"
#include "Enemy.h"
#include "Entities/Neutral.h"
#include "Common/Algorithm.h"

void CLvFloor1::OnCreateComplete( CMyLevel * pLevel )
{
	for( auto pEntity = Get_ChildEntity(); pEntity; pEntity = pEntity->NextChildEntity() )
	{
		auto pChunkObject = SafeCast<CChunkObject>( pEntity );
		if( pChunkObject )
		{
			if( pChunkObject->GetName() == m_strCrate.c_str() )
				m_vecCrates.push_back( pChunkObject );
		}

		auto pPickUp = SafeCast<CPickUpTemplate>( pEntity );
		if( pPickUp )
		{
			m_vecPickups.push_back( pPickUp );
		}
	}

	m_triggers.resize( m_vecPickups.size() + m_vecCrates.size() );
	for( int i = 0; i < m_vecPickups.size(); i++ )
	{
		CPickUp* pPickUp = SafeCast<CPickUpTemplate>( m_vecPickups[i].GetPtr() );
		m_triggers[i].Set( [i, this] () {
			m_vecPickups[i] = NULL;
			OnPickUp();
		} );
		pPickUp->RegisterPickupEvent( &m_triggers[i] );
		pPickUp->bVisible = false;
	}
	for( int i = 0; i < m_vecCrates.size(); i++ )
	{
		CChunkObject* pCrate = SafeCast<CChunkObject>( m_vecCrates[i].GetPtr() );
		m_triggers[i + m_vecPickups.size()].Set( [this, i] () {
			OnCrateKilled( i );
		} );
		pCrate->RegisterKilledEvent( &m_triggers[i + m_vecPickups.size()] );
	}
}

void CLvFloor1::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLvFloor1::OnCrateKilled( int32 i )
{
	if( !m_nKilledCrates )
	{
		SItemDropContext context;
		context.nDrop = m_vecPickups.size();
		auto pNode = CGlobalCfg::Inst().itemDropNodeContext.FindNode( m_strItemDrop );
		pNode->Generate( context );
		context.Drop();

		for( int i = 0; i < m_vecPickups.size(); i++ )
		{
			auto pPickUp = SafeCast<CPickUpTemplate>( m_vecPickups[i].GetPtr() );
			CEntity* pItem = SafeCast<CEntity>( context.dropItems[i].pPrefab->GetRoot()->CreateInstance() );
			pPickUp->Set( pItem, context.dropItems[i].nPrice );
		}
	}
	m_nKilledCrates++;
	if( m_pChunk )
		m_pChunk->fWeight = Max( m_pChunk->fWeight, m_fWeights[m_nKilledCrates - 1] );
	m_vecPickups[m_vecPickups.size() - 1 - i]->bVisible = true;
}

void CLvFloor1::OnPickUp()
{
	if( m_pChunk && m_pChunk->nLevelBarrierType == 3 )
		CMyLevel::GetInst()->OnPlayerBonusStageCrushed();
	Kill();
}

void CLvFloor1::Kill()
{
	if( m_bKilled )
		return;
	m_bKilled = true;
	for( auto& trigger : m_triggers )
	{
		if( trigger.IsRegistered() )
			trigger.Unregister();
	}

	for( auto& pCrate : m_vecCrates )
	{
		if( pCrate && pCrate->GetParentEntity() )
		{
			pCrate->Kill();
			pCrate = NULL;
		}
	}
	for( auto& pPickUp : m_vecPickups )
	{
		if( pPickUp )
		{
			SafeCast<CPickUp>( pPickUp.GetPtr() )->Kill();
			pPickUp = NULL;
		}
	}
	m_pChunk->fWeight = 1000000;
	OnTick();
}

void CLvFloor1::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
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

	if( y <= 0 )
	{
		if( m_pBonusStageReward )
		{
			auto pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 p( m_pChunk->nWidth * CMyLevel::GetBlockSize() * 0.5f, ( m_pChunk->nHeight - 1 ) * CMyLevel::GetBlockSize() );
				auto pBonusStageDrop = SafeCast<CBonusStageReward>( m_pBonusStageReward->GetRoot()->CreateInstance() );
				pBonusStageDrop->SetPosition( GetPosition() + p );
				pBonusStageDrop->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				SItemDropContext context;
				CGlobalCfg::Inst().bonusStageDrop.Drop( context, pPlayer->GetPoint() );
				pBonusStageDrop->Set( context, CGlobalCfg::Inst().Point2Reward( pPlayer->GetPoint() ) );
			}
		}

		CRandomChunkTiled::Kill();
	}
}

void CLvFloor2::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto rect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;

	SetRenderObject( new CRenderObject2D );
	auto pRenderObject1 = new CRenderObject2D;
	AddChild( pRenderObject1 );
	if( CMyLevel::GetInst() )
		pRenderObject1->SetRenderParent( CMyLevel::GetInst()->GetChunkRoot1() );
	pRenderObject1->SetZOrder( 1 );
	for( int j = 0; j < pChunk->nHeight; j += 2 )
	{
		for( int i = 0; i < pChunk->nWidth; i += 2 )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( i * 32, j * 32, 64, 64 ) );
			bool b = true;
			if( j == 0 )
			{
				if( i < 4 )
					pImage2D->SetTexRect( texRect0.Offset( CVector2( texRect0.width * ( 1 + i / 2 ), texRect0.height ) ) );
				else if( i >= pChunk->nWidth - 4 )
					pImage2D->SetTexRect( texRect0.Offset( CVector2( texRect0.width * ( 8 - ( pChunk->nWidth - i ) / 2 ), texRect0.height ) ) );
				else
				{
					pImage2D->SetTexRect( texRect0 );
					b = false;
				}
			}
			else
			{
				if( i < 4 )
					pImage2D->SetTexRect( texRect0.Offset( CVector2( texRect0.width * ( 1 + i / 2 ), 0 ) ) );
				else if( i >= pChunk->nWidth - 4 )
					pImage2D->SetTexRect( texRect0.Offset( CVector2( texRect0.width * ( 8 - ( pChunk->nWidth - i ) / 2 ), 0 ) ) );
				else if( i >= pChunk->nWidth / 2 - 2 && i < pChunk->nWidth / 2 + 2 )
					pImage2D->SetTexRect( texRect0.Offset( CVector2( texRect0.width * ( 5 + ( i - (int32)pChunk->nWidth / 2 ) / 2 ), 0 ) ) );
				else
					pImage2D->SetTexRect( texRect0.Offset( CVector2( texRect0.width * 3, 0 ) ) );
			}

			GetRenderObject()->AddChild( pImage2D );
			if( b )
			{
				pImage2D->SetRenderParent( pRenderObject1 );
			}
			else
			{
				for( int iX = i; iX < i + 2; iX++ )
				{
					for( int iY = j; iY < j + 2; iY++ )
					{
						GetBlock( iX, iY )->rtTexRect = CRectangle( texRect0.x + iX * texRect0.width / 2, texRect0.y + iY * texRect0.height / 2,
							texRect0.width / 2, texRect0.height / 2 );
					}
				}
			}
		}
	}
}

void CLvFloor2::OnCreateComplete( CMyLevel * pLevel )
{
	if( !pLevel )
		return;
	for( int i = 0; i < 8; i++ )
	{
		CEntity* pEntity;
		if( ( i & 1 ) == 0 )
			pEntity = SafeCast<CEntity>( m_pPrefab->GetRoot()->CreateInstance() );
		else
		{
			pEntity = SafeCast<CEntity>( m_pPrefab1->GetRoot()->CreateInstance() );
			auto pPickUp = SafeCast<CEntity>( m_pItemDropPrefab->GetRoot()->CreateInstance() );
			m_vecPickups.push_back( pPickUp );
			pPickUp->SetParentEntity( pEntity );
			auto pCrate = SafeCast<CEntity>( m_pCrate->GetRoot()->CreateInstance() );
			m_vecCrates.push_back( pCrate );
			pCrate->SetParentEntity( pEntity );
		}

		pEntity->SetPosition( CVector2( ( i * 4 + 2 ) * 32, 32 ) );
		pEntity->SetParentBeforeEntity( GetRenderObject() );
		pEntity->bVisible = i >= 1 && i <= 6;
		m_vecSegs.push_back( pEntity );
	}
	m_nDir = SRand::Inst().Rand( 0, 2 );

	m_triggers.resize( m_vecPickups.size() + m_vecCrates.size() );
	for( int i = 0; i < m_vecPickups.size(); i++ )
	{
		CPickUp* pPickUp = SafeCast<CPickUpTemplate>( m_vecPickups[i].GetPtr() );
		m_triggers[i].Set( [i, this] () {
			m_vecPickups[i] = NULL;
			OnPickUp();
		} );
		pPickUp->RegisterPickupEvent( &m_triggers[i] );
		pPickUp->bVisible = false;
	}
	for( int i = 0; i < m_vecCrates.size(); i++ )
	{
		CEnemy* pCrate = SafeCast<CEnemy>( m_vecCrates[i].GetPtr() );
		m_triggers[i + m_vecPickups.size()].Set( [this, i] () {
			OnCrateKilled( i );
		} );
		pCrate->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_triggers[i + m_vecPickups.size()] );
	}
}

void CLvFloor2::OnAddedToStage()
{
	if( CMyLevel::GetInst() )
		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CLvFloor2::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLvFloor2::OnCrateKilled( int32 i )
{
	if( !m_nKilledCrates )
	{
		SItemDropContext context;
		context.nDrop = m_vecPickups.size();
		auto pNode = CGlobalCfg::Inst().itemDropNodeContext.FindNode( m_strItemDrop );
		pNode->Generate( context );
		context.Drop();

		for( int i = 0; i < m_vecPickups.size(); i++ )
		{
			auto pPickUp = SafeCast<CPickUpTemplate>( m_vecPickups[i].GetPtr() );
			CEntity* pItem = SafeCast<CEntity>( context.dropItems[i].pPrefab->GetRoot()->CreateInstance() );
			pPickUp->Set( pItem, context.dropItems[i].nPrice );
		}
	}
	m_nKilledCrates++;
	if( m_pChunk )
		m_pChunk->fWeight = m_fWeights[m_nKilledCrates - 1];
	m_vecPickups[i]->bVisible = true;
}

void CLvFloor2::OnPickUp()
{
	if( m_pChunk && m_pChunk->nLevelBarrierType == 3 )
		CMyLevel::GetInst()->OnPlayerBonusStageCrushed();
	Kill();
}

void CLvFloor2::Kill()
{
	if( m_bKilled )
		return;
	for( auto& trigger : m_triggers )
	{
		if( trigger.IsRegistered() )
			trigger.Unregister();
	}

	for( auto& pCrate : m_vecCrates )
	{
		if( pCrate && pCrate->GetParentEntity() )
		{
			SafeCast<CEnemy>( pCrate.GetPtr() )->Kill();
			pCrate = NULL;
		}
	}
	for( auto& pPickUp : m_vecPickups )
	{
		if( pPickUp )
		{
			SafeCast<CPickUp>( pPickUp.GetPtr() )->Kill();
			pPickUp = NULL;
		}
	}
	m_bKilled = true;
	m_pChunk->fWeight = 1000000;
}

void CLvFloor2::OnTick()
{
	if( !m_bKilled )
	{
		uint32 nSpeeds[] = { 1, 1, 2, 2, 3 };
		int32 nSpeed = nSpeeds[m_nKilledCrates] * ( ( m_nKilledCrates & 1 ) == 0 ? 1 : -1 ) * ( m_nDir ? 1 : -1 );
		for( int i = 0; i < m_vecSegs.size(); i++ )
		{
			float posX = m_vecSegs[i]->x + nSpeed;
			uint32 nWidth = m_pChunk->nWidth * CMyLevel::GetBlockSize();
			if( posX < 0 )
				posX += nWidth;
			else if( posX > nWidth )
				posX -= nWidth;
			m_vecSegs[i]->SetPosition( CVector2( posX, m_vecSegs[i]->y ) );
			m_vecSegs[i]->bVisible = posX > 64 && posX < nWidth - 64;
		}
	}
	else
	{
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
	}

	if( y <= 0 )
	{
		if( m_pBonusStageReward )
		{
			auto pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 p( m_pChunk->nWidth * CMyLevel::GetBlockSize() * 0.5f, ( m_pChunk->nHeight - 1 ) * CMyLevel::GetBlockSize() );
				auto pBonusStageDrop = SafeCast<CBonusStageReward>( m_pBonusStageReward->GetRoot()->CreateInstance() );
				pBonusStageDrop->SetPosition( GetPosition() + p );
				pBonusStageDrop->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				SItemDropContext context;
				CGlobalCfg::Inst().bonusStageDrop.Drop( context, pPlayer->GetPoint() );
				pBonusStageDrop->Set( context, CGlobalCfg::Inst().Point2Reward( pPlayer->GetPoint() ) );
			}
		}

		CChunkObject::Kill();
		return;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CLvBarrier1::OnRemovedFromStage()
{
	for( auto& item : m_triggers )
	{
		if( item.IsRegistered() )
			item.Unregister();
	}
	if( m_deathTick.IsRegistered() )
		m_deathTick.Unregister();
	m_vecRooms.clear();
	CChunkObject::OnRemovedFromStage();
}

void CLvBarrier1::OnCreateComplete( CMyLevel* pLevel )
{
	int32 iTrigger = 0;
	vector<CChunkObject*> vecChunkObjects;
	uint32 nWidth = m_pChunk->nWidth;
	uint32 nHeight = m_pChunk->nHeight;
	vecChunkObjects.resize( nWidth * nHeight );
	for( auto pEntity = Get_ChildEntity(); pEntity; pEntity = pEntity->NextChildEntity() )
	{
		auto pChunkObject = SafeCast<CChunkObject>( pEntity );
		if( pChunkObject )
		{
			auto pChunk = pChunkObject->GetChunk();
			if( pChunk->nSubChunkType != 2 )
				continue;

			int32 chunkX = pChunk->pos.x / CMyLevel::GetBlockSize();
			int32 chunkY = pChunk->pos.y / CMyLevel::GetBlockSize();
			for( int i = chunkX; i < chunkX + pChunk->nWidth; i++ )
			{
				for( int j = chunkY; j < chunkY + pChunk->nHeight; j++ )
				{
					vecChunkObjects[i + j * nWidth] = pChunkObject;
				}
			}

			if( pChunkObject->GetName() == m_strCore.c_str() )
				iTrigger++;

			if( pChunk->nMoveType )
				iTrigger += 4;
		}
	}

	m_triggers.resize( iTrigger );
	iTrigger = 0;
	for( auto pEntity = Get_ChildEntity(); pEntity; pEntity = pEntity->NextChildEntity() )
	{
		auto pChunkObject = SafeCast<CChunkObject>( pEntity );
		if( pChunkObject )
		{
			auto pChunk = pChunkObject->GetChunk();
			if( pChunk->nSubChunkType != 2 )
				continue;

			if( pChunkObject->GetName() == m_strCore.c_str() )
			{
				m_vecCores.push_back( pChunkObject );
				m_nCoreCount++;
				CFunctionTrigger& trigger = m_triggers[iTrigger++];
				trigger.Set( [this]()
				{
					OnCoreDestroyed();
				} );
				pChunkObject->RegisterKilledEvent( &trigger );
			}

			if( pChunk->nMoveType )
			{
				m_vecRooms.push_back( pChunkObject );
				int32 chunkX = pChunk->pos.x / CMyLevel::GetBlockSize();
				int32 chunkY = pChunk->pos.y / CMyLevel::GetBlockSize();

				if( chunkY > 0 )
				{
					for( int i = chunkX; i < chunkX + pChunk->nWidth; i++ )
					{
						auto pWall = vecChunkObjects[i + ( chunkY - 1 ) * nWidth];
						if( pWall && pWall->GetName() == m_strWall.c_str() )
						{
							CFunctionTrigger& trigger = m_triggers[iTrigger];
							trigger.Set( [pChunkObject]()
							{
								if( pChunkObject->GetStage() )
									pChunkObject->Kill();
							} );
							pWall->RegisterKilledEvent( &trigger );
							break;
						}
					}
				}
				iTrigger++;

				if( chunkY + pChunk->nHeight <= nHeight - 1 )
				{
					for( int i = chunkX; i < chunkX + pChunk->nWidth; i++ )
					{
						auto pWall = vecChunkObjects[i + ( chunkY + pChunk->nHeight ) * nWidth];
						if( pWall && pWall->GetName() == m_strWall.c_str() )
						{
							CFunctionTrigger& trigger = m_triggers[iTrigger];
							trigger.Set( [pChunkObject]()
							{
								if( pChunkObject->GetStage() )
									pChunkObject->Kill();
							} );
							pWall->RegisterKilledEvent( &trigger );
							break;
						}
					}
				}
				iTrigger++;

				if( chunkX > 0 )
				{
					for( int i = chunkY; i < chunkY + pChunk->nHeight; i++ )
					{
						auto pWall = vecChunkObjects[( chunkX - 1 ) + i * nWidth];
						if( pWall && pWall->GetName() == m_strWall.c_str() )
						{
							CFunctionTrigger& trigger = m_triggers[iTrigger];
							trigger.Set( [pChunkObject]()
							{
								if( pChunkObject->GetStage() )
									pChunkObject->Kill();
							} );
							pWall->RegisterKilledEvent( &trigger );
							break;
						}
					}
				}
				iTrigger++;

				if( chunkX + pChunk->nWidth <= nWidth - 1 )
				{
					for( int i = chunkY; i < chunkY + pChunk->nHeight; i++ )
					{
						auto pWall = vecChunkObjects[( chunkX + pChunk->nWidth ) + i * nWidth];
						if( pWall && pWall->GetName() == m_strWall.c_str() )
						{
							CFunctionTrigger& trigger = m_triggers[iTrigger];
							trigger.Set( [pChunkObject]()
							{
								if( pChunkObject->GetStage() )
									pChunkObject->Kill();
							} );
							pWall->RegisterKilledEvent( &trigger );
							break;
						}
					}
				}
				iTrigger++;
			}
		}
	}
}

void CLvBarrier1::OnCoreDestroyed()
{
	m_nCoreCount--;
	if( !m_nCoreCount )
	{
		Kill();
		return;
	}

	uint8 nPhase;
	float fPercent = ( m_nCoreCount - 1 ) * 1.0f / ( m_vecCores.size() - 1 );
	if( fPercent > 0.65f )
		nPhase = 0;
	else if( fPercent > 0.25f )
		nPhase = 1;
	else if( fPercent > 0 )
		nPhase = 2;
	else nPhase = 3;

	for( auto& pCore : m_vecCores )
	{
		if( !pCore )
			continue;
		if( !pCore->GetStage() )
		{
			pCore = NULL;
			continue;
		}

		SafeCast<CLvBarrier1Core>( pCore.GetPtr() )->SetPhase( nPhase );
	}
}

void CLvBarrier1::Kill()
{
	if( m_bKilled )
		return;
	m_triggerKilled.Trigger( 0, this );
	m_bKilled = true;
	m_fHp = 0;
	AddHitShake( CVector2( 8, 0 ) );
	Tick();
	OnKilled();
}

void CLvBarrier1::Tick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_deathTick );

	if( m_strKillEffect )
	{
		if( m_nKillEffectCDLeft )
			m_nKillEffectCDLeft--;
		if( !m_nKillEffectCDLeft )
		{
			CVector2 center = CVector2( SRand::Inst().Rand( 0u, m_pChunk->nWidth * CMyLevel::GetBlockSize() ), SRand::Inst().Rand( 0u, m_pChunk->nHeight * CMyLevel::GetBlockSize() ) );
			auto pEffect = SafeCast<CEffectObject>( m_strKillEffect->GetRoot()->CreateInstance() );
			pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			pEffect->SetPosition( GetPosition() + center );
			pEffect->SetState( 2 );
			m_nKillEffectCDLeft = m_nKillEffectInterval;
		}
	}

	m_nDeathTime--;
	if( !m_nDeathTime )
	{
		if( m_strEffect )
		{
			ForceUpdateTransform();
			for( int i = 0; i < m_pChunk->nWidth; i++ )
			{
				for( int j = 0; j < m_pChunk->nHeight; j++ )
				{
					auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
					pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
					pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
					pEffect->SetState( 2 );
				}
			}
		}

		auto pChunk = m_pChunk;
		if( pChunk )
		{
			m_pChunk = NULL;
			pChunk->pChunkObject = NULL;
			if( pChunk->nSubChunkType == 2 )
				delete pChunk;
			else
				CMyLevel::GetInst()->KillChunk( pChunk, this );
		}
		SetParentEntity( NULL );
	}
}

void CLvBarrier1Core::AIFunc()
{
	if( !CMyLevel::GetInst() )
		return;
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBulletPrefab1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );
	m_pBulletPrefab2 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet2.c_str() );

	while( 1 )
	{
		while( 1 )
		{
			m_pAI->Yield( 0.5f, true );

			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				return;
			if( m_nSpecialFires )
				break;
			float fOpenDist = m_fOpenDist[m_nPhase];
			if( globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() ).Length2() < fOpenDist * fOpenDist )
				break;
		}

		while( 1 )
		{
			uint8 nPhase = m_nPhase;
			CVector2 center = globalTransform.GetPosition() + CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * 16.0f;
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				return;
			CVector2 playerPos = pPlayer->GetPosition();
			CVector2 dPos = playerPos - center;
			CVector2 dir = dPos;
			float l = dir.Normalize();
			float fYield = 1.0f;

			if( m_nSpecialFires )
			{
				switch( nPhase )
				{
				case 0:
				{
					float fAngle = atan2( dPos.y, dPos.x );
					for( int i = 0; i < 5; i++ )
					{
						auto pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
						pBullet->SetPosition( center );
						float fAngle1 = ( SRand::Inst().Rand( -0.25f, 0.25f ) + i - 2 ) * 0.15f + fAngle;
						pBullet->SetRotation( fAngle1 );
						pBullet->SetVelocity( CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 200 );
						pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					}
					fYield = 1.0f;
					break;
				}
				case 1:
				{
					SBarrageContext context;
					context.pCreator = GetParentEntity();
					context.vecBulletTypes.push_back( m_pBulletPrefab );
					context.nBulletPageSize = 24;

					float fAngle = atan2( dPos.y, dPos.x );
					CBarrage* pBarrage = new CBarrage( context );
					pBarrage->AddFunc( [fAngle]( CBarrage* pBarrage )
					{
						uint32 nBullet = 0;
						
						for( int i = 0; i < 8; i++ )
						{
							float fAngle1 = SRand::Inst().Rand( -0.3f, 0.3f ) + fAngle;

							CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
							pBarrage->InitBullet( nBullet++, 0, -1, dir * -8 + CVector2( dir.y, -dir.x ) * 8, CVector2( cos( fAngle1 - 0.02f ), sin( fAngle1 - 0.02f ) ) * 175, CVector2( 0, 0 ), true );
							pBarrage->InitBullet( nBullet++, 0, -1, dir * 8, dir * 175, CVector2( 0, 0 ), true );
							pBarrage->InitBullet( nBullet++, 0, -1, dir * -8 + CVector2( -dir.y, dir.x ) * 8, CVector2( cos( fAngle1 + 0.02f ), sin( fAngle1 + 0.02f ) ) * 175, CVector2( 0, 0 ), true );
							pBarrage->Yield( 5 );
						}
						pBarrage->StopNewBullet();
					} );
					pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBarrage->SetPosition( center );
					pBarrage->Start();
					fYield = 1.5f;
					break;
				}
				case 2:
				{
					SBarrageContext context;
					context.pCreator = GetParentEntity();
					context.vecBulletTypes.push_back( m_pBulletPrefab );
					context.nBulletPageSize = 60;

					float fAngle = atan2( dPos.y, dPos.x );
					CBarrage* pBarrage = new CBarrage( context );
					pBarrage->AddFunc( [fAngle]( CBarrage* pBarrage )
					{
						uint32 nBullet = 0;
						for( int i = 0; i < 6; i++ )
						{
							float fAngle1 = ( i - 3.5f ) * 0.18f;

							for( int j = 0; j < 3; j++ )
							{
								float fAngle2 = ( j - 1 ) * 0.04f;
								pBarrage->InitBullet( nBullet++, 0, -1, CVector2( sin( fAngle + fAngle1 + fAngle2 ), -cos( fAngle + fAngle1 + fAngle2 ) ) * 16,
									CVector2( cos( fAngle + fAngle1 + fAngle2 ), sin( fAngle + fAngle1 + fAngle2 ) ) * ( 135 + j * 20 ), CVector2( 0, 0 ), true );
								pBarrage->InitBullet( nBullet++, 0, -1, CVector2( -sin( fAngle - fAngle1 - fAngle2 ), cos( fAngle - fAngle1 - fAngle2 ) ) * 16,
									CVector2( cos( fAngle - fAngle1 - fAngle2 ), sin( fAngle - fAngle1 - fAngle2 ) ) * ( 135 + j * 20 ), CVector2( 0, 0 ), true );
								pBarrage->Yield( 2 );
							}
							pBarrage->Yield( 2 );
						}

						pBarrage->StopNewBullet();
					} );
					pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBarrage->SetPosition( center );
					pBarrage->Start();
					fYield = 2.5f;
					break;
				}
				case 3:
				{
					SBarrageContext context;
					context.pCreator = GetParentEntity();
					context.vecBulletTypes.push_back( m_pBulletPrefab );
					context.vecBulletTypes.push_back( m_pBulletPrefab2 );
					context.nBulletPageSize = 52;

					float fAngle = atan2( dPos.y, dPos.x );
					float t = 1.0f;
					CVector2 v1 = CVector2( dir.y, -dir.x ) * SRand::Inst().Rand( l + 120.0f, l + 140.0f ) + dir * SRand::Inst().Rand( -( l * 0.3f + 30.0f ), ( l * 0.3f + 30.0f ) );
					CVector2 v2 = CVector2( -dir.y, dir.x ) * SRand::Inst().Rand( l + 120.0f, l + 140.0f ) + dir * SRand::Inst().Rand( -( l * 0.3f + 30.0f ), ( l * 0.3f + 30.0f ) );
					CVector2 a1 = ( dPos - v1 * t ) / ( t * t * 0.5f );
					CVector2 a2 = ( dPos - v2 * t ) / ( t * t * 0.5f );

					CBarrage* pBarrage = new CBarrage( context );
					pBarrage->AddFunc( [dPos, v1, v2, a1, a2]( CBarrage* pBarrage )
					{
						uint32 nBullet = 0;
						pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), v1, a1, true );
						pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), v2, a2, true );

						for( int i = 0; i < 10; i++ )
						{
							pBarrage->Yield( 3 );

							float t = ( i + 0.5f ) * 6.0f / 60;
							pBarrage->InitBullet( nBullet++, 1, -1, v1 * t + a1 * ( 0.5f * t * t ), ( CVector2( v1.y, -v1.x ) + CVector2( a1.y, -a1.x ) * t ) * 0.3f, CVector2( 0, 0 ), true );
							pBarrage->InitBullet( nBullet++, 1, -1, v1 * t + a1 * ( 0.5f * t * t ), ( CVector2( v1.y, -v1.x ) + CVector2( a1.y, -a1.x ) * t ) * -0.3f, CVector2( 0, 0 ), true );

							pBarrage->InitBullet( nBullet++, 1, -1, v2 * t + a2 * ( 0.5f * t * t ), ( CVector2( v2.y, -v2.x ) + CVector2( a2.y, -a2.x ) * t ) * 0.3f, CVector2( 0, 0 ), true );
							pBarrage->InitBullet( nBullet++, 1, -1, v2 * t + a2 * ( 0.5f * t * t ), ( CVector2( v2.y, -v2.x ) + CVector2( a2.y, -a2.x ) * t ) * -0.3f, CVector2( 0, 0 ), true );
							pBarrage->Yield( 3 );
						}

						float t = 1.0f;
						CVector2 v11 = v1 + a1 * t;
						CVector2 v21 = v2 + a2 * t;
						float fAngle1 = atan2( v11.y, v11.x );
						float fAngle2 = atan2( v21.y, v21.x );
						CMatrix2D mat1;
						mat1.Rotate( fAngle1 );
						CMatrix2D mat2;
						mat2.Rotate( fAngle2 );
						pBarrage->DestroyBullet( 0 );
						pBarrage->DestroyBullet( 1 );
						for( int i = 0; i < 12; i++ )
						{
							float fBaseAngle = i * PI / 6;
							CVector2 vel0 = CVector2( cos( fBaseAngle ) * 250, sin( fBaseAngle ) * 100 );
							CVector2 vel = mat1.MulVector2Dir( vel0 );
							pBarrage->InitBullet( nBullet++, 0, -1, dPos, vel, CVector2( 0, 0 ), true );
							vel = mat2.MulVector2Dir( vel0 );
							pBarrage->InitBullet( nBullet++, 0, -1, dPos, vel, CVector2( 0, 0 ), true );
						}

						pBarrage->Yield( 2 );
						pBarrage->StopNewBullet();
					} );
					pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBarrage->SetPosition( center );
					pBarrage->Start();
					fYield = 1.25f;
					break;
				}
				}
				m_nSpecialFires--;
			}
			else
			{
				if( dPos.Length2() > m_fCloseDist[m_nPhase] * m_fCloseDist[m_nPhase] )
					break;
				switch( nPhase )
				{
				case 0:
				{
					auto pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
					pBullet->SetPosition( center );
					pBullet->SetRotation( atan2( dir.y, dir.x ) );
					pBullet->SetVelocity( dir * 200 );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					fYield = 1.0f;
					break;
				}
				case 1:
				{
					float fAngle = atan2( dPos.y, dPos.x );
					for( int i = 0; i < 3; i++ )
					{
						auto pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
						pBullet->SetPosition( center );
						float fAngle1 = fAngle + ( i - 1 ) * 0.3f;
						pBullet->SetRotation( fAngle1 );
						pBullet->SetVelocity( CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 185 );
						pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					}
					fYield = 1.0f;
					break;
				}
				case 2:
				{
					SBarrageContext context;
					context.pCreator = GetParentEntity();
					context.vecBulletTypes.push_back( m_pBulletPrefab );
					context.nBulletPageSize = 13;

					float fAngle = atan2( dPos.y, dPos.x );
					CBarrage* pBarrage = new CBarrage( context );
					pBarrage->AddFunc( [fAngle]( CBarrage* pBarrage )
					{
						pBarrage->InitBullet( 0, -1, -1, CVector2( 0, 0 ), CVector2( 135 * cos( fAngle ), 135 * sin( fAngle ) ), CVector2( 0, 0 ), false, SRand::Inst().Rand( -PI, PI ), 2.0f );

						for( int i = 0; i < 6; i++ )
						{
							float fAngle1 = i * PI / 3;
							pBarrage->InitBullet( i * 2 + 1, -1, 0, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 28, CVector2( 0, 0 ), CVector2( 0, 0 ), false, fAngle1, 2.0f );
							pBarrage->InitBullet( i * 2 + 2, 0, i * 2 + 1, CVector2( 28, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), true );
						}
						pBarrage->Yield( 2 );
						pBarrage->StopNewBullet();
					} );
					pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBarrage->SetPosition( center );
					pBarrage->Start();
					fYield = 1.5f;
					break;
				}
				case 3:
				{
					SBarrageContext context;
					context.pCreator = GetParentEntity();
					context.vecBulletTypes.push_back( m_pBulletPrefab );
					context.vecBulletTypes.push_back( m_pBulletPrefab1 );
					context.nBulletPageSize = 25;

					float fAngle = atan2( dPos.y, dPos.x );
					CBarrage* pBarrage = new CBarrage( context );
					pBarrage->AddFunc( [dPos]( CBarrage* pBarrage )
					{
						float f0 = SRand::Inst().Rand( -PI, PI );
						CVector2 targetPos = dPos + CVector2( cos( f0 ), sin( f0 ) ) * 80.0f;

						pBarrage->InitBullet( 0, 1, -1, CVector2( 0, 0 ), targetPos * 0.5f, CVector2( 0, 0 ), false, SRand::Inst().Rand( -PI, PI ), 2.0f );
						pBarrage->Yield( 120 );

						uint32 nOrder[5] = { 0, 1, 2, 3, 4 };
						SRand::Inst().Shuffle( nOrder, 5 );
						float fAngle0 = SRand::Inst().Rand( -PI, PI );

						uint32 nBullet = 0;
						for( int i = 0; i < 5; i++ )
						{
							float fAngle = fAngle0 + nOrder[i] * PI * 2 / 5;
							CVector2 ofs( cos( fAngle ), sin( fAngle ) );
							CVector2 ofs1( ofs.y, -ofs.x );
							for( int j = 0; j < 5; j++ )
							{
								pBarrage->InitBullet( nBullet++, 0, -1, targetPos, ofs * 180 + ofs1 * ( ( j - 2 ) / 2.5f * tan( PI / 5 ) * 180 ), CVector2( 0, 0 ), true );
							}
							pBarrage->Yield( 4 );
						}

						pBarrage->Yield( 2 );
						pBarrage->StopNewBullet();
					} );
					pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBarrage->SetPosition( center );
					pBarrage->Start();
					fYield = 1.5f;
					break;
				}
				}
			}

			m_pAI->Yield( fYield, false );
		}
	}
}

void CLvBarrierReward1::OnCreateComplete( CMyLevel * pLevel )
{
	for( auto pEntity = Get_ChildEntity(); pEntity; pEntity = pEntity->NextChildEntity() )
	{
		auto pPickUp = SafeCast<CPickUp>( pEntity );
		if( pPickUp )
		{
			m_vecPickups.push_back( pPickUp );
			pPickUp->SetParentEntity( NULL );
		}
	}

	m_triggers.resize( m_vecPickups.size() );
	for( int i = 0; i < m_vecPickups.size(); i++ )
	{
		CPickUp* pPickUp = SafeCast<CPickUp>( m_vecPickups[i].GetPtr() );
		m_triggers[i].Set( [i, this] () {
			m_vecPickups[i] = NULL;
			OnPickUp();
		} );
		pPickUp->RegisterPickupEvent( &m_triggers[i] );
	}
}

void CLvBarrierReward1::OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed )
{
	if( m_pChunk->pos.y == 0 && !m_bPickupCreated )
	{
		m_bPickupCreated = true;
		SItemDropContext context;
		context.nDrop = m_vecPickups.size();
		auto pNode = CGlobalCfg::Inst().itemDropNodeContext.FindNode( m_strItemDrop );
		pNode->Generate( context );
		context.Drop();

		for( int i = 0; i < m_vecPickups.size(); i++ )
		{
			auto pPickUp = SafeCast<CPickUpTemplate>( m_vecPickups[i].GetPtr() );
			CEntity* pItem = SafeCast<CEntity>( context.dropItems[i].pPrefab->GetRoot()->CreateInstance() );
			pPickUp->SetParentEntity( this );
			pPickUp->Set( pItem, context.dropItems[i].nPrice );
		}
	}
}

void CLvBarrierReward1::OnPickUp()
{
	for( auto& trigger : m_triggers )
	{
		if( trigger.IsRegistered() )
			trigger.Unregister();
	}
	for( auto& pPickUp : m_vecPickups )
	{
		if( pPickUp && pPickUp->GetStage() )
		{
			SafeCast<CPickUp>( pPickUp.GetPtr() )->Kill();
			pPickUp = NULL;
		}
	}

	Kill();
}

void CLvBarrier2Label::OnAddedToStage()
{
	auto pRenderObject = static_cast<CImage2D*>( GetRenderObject() );
	m_origRect = pRenderObject->GetElem().rect;
	m_origTexRect = pRenderObject->GetElem().texRect;
	UpdatePercent( 0 );
	SafeCast<CLvBarrier2>( GetParentEntity() )->m_pLabel = this;
}

void CLvBarrier2Label::UpdatePercent( float fPercent )
{
	auto pRenderObject = static_cast<CImage2D*>( GetRenderObject() );
	auto rect = m_origRect;
	rect.height = m_origRect.height * fPercent;
	auto texRect = m_origTexRect;
	texRect.SetTop( texRect.GetBottom() - texRect.height * fPercent );
	pRenderObject->SetRect( rect );
	pRenderObject->SetTexRect( texRect );
}

void CLvBarrier2::OnAddedToStage()
{
	CChunkObject::OnAddedToStage();
	if( m_grids.size() && !m_pAI )
	{
		m_pAI = new AI();
		m_pAI->SetParentEntity( this );
	}
}

void CLvBarrier2::OnRemovedFromStage()
{
	if( m_onHitShakeTick.IsRegistered() )
		m_onHitShakeTick.Unregister();
	if( m_deathTick.IsRegistered() )
		m_deathTick.Unregister();
	CChunkObject::OnRemovedFromStage();
}

void CLvBarrier2::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto rect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;

	SetRenderObject( new CRenderObject2D );
	for( int i = 0; i < pChunk->nWidth; i++ )
	{
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );

			CRectangle texRect;
			static int32 t1[4] = { 3, 2, 0, 1 };
			static int32 t2[4] = { 3, 0, 2, 1 };
			if( GetBlock( i, j )->eBlockType == eBlockType_Block )
			{
				int32 nTexX = ( i > 0 && GetBlock( i - 1, j )->eBlockType == eBlockType_Block ? 1 : 0 )
					| ( i < pChunk->nWidth - 1 && GetBlock( i + 1, j )->eBlockType == eBlockType_Block ? 2 : 0 );
				int32 nTexY = ( j > 0 && GetBlock( i, j - 1 )->eBlockType == eBlockType_Block ? 1 : 0 )
					| ( j < pChunk->nHeight - 1 && GetBlock( i, j + 1 )->eBlockType == eBlockType_Block ? 2 : 0 );
				nTexX = t1[nTexX];
				nTexY = t2[nTexY];
				texRect = CRectangle( m_blockTex.x + nTexX * texRect0.width / 4, m_blockTex.y + nTexY * texRect0.height / 4,
					texRect0.width / 4, texRect0.height / 4 );
				if( SRand::Inst().Rand( 0, 2 ) )
					texRect.x += texRect0.width;
			}
			else
			{
				int32 nTexX = GetBlock( i, j )->nTag & 3;
				int32 nTexY = GetBlock( i, j )->nTag >> 2;
				nTexX = t1[nTexX];
				nTexY = t2[nTexY];
				texRect = CRectangle( texRect0.x + nTexX * texRect0.width / 4, texRect0.y + nTexY * texRect0.height / 4,
					texRect0.width / 4, texRect0.height / 4 );
			}

			pImage2D->SetTexRect( texRect );
			GetRenderObject()->AddChild( pImage2D );
			if( GetBlock( i, j )->eBlockType == eBlockType_Block )
				pImage2D->SetRenderParentBefore( m_p1 );
			GetBlock( i, j )->rtTexRect = texRect;
		}
	}
	m_pBoxLayer = new CRenderObject2D;
	m_p1->AddChild( m_pBoxLayer );
}

void CLvBarrier2::OnCreateComplete( CMyLevel * pLevel )
{
	if( !pLevel )
		return;
	int32 nWidth = m_pChunk->nWidth;
	int32 nHeight = m_pChunk->nHeight;
	m_grids.resize( nWidth * nHeight );

	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			m_grids[i + j * nWidth].nType = 1;
			m_grids[i + j * nWidth].nColor = -1;
			uint8 nTag = GetBlock( i, j )->nTag;
			if( nTag > 0 )
			{
				for( int k = 0; k < 4; k++ )
				{
					m_grids[i + j * nWidth].bDirs[k] = !!( nTag & ( 1 << k ) );
				}

				if( i < 2 || i >= nWidth - 2 || j >= nHeight - 2 )
					m_q.push_back( TVector2<int32>( i, j ) );
			}
		}
	}
	SRand::Inst().Shuffle( m_q );

	for( auto pSubChunk = m_pChunk->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
	{
		if( pSubChunk->nSubChunkType != 2 )
			continue;

		int32 nX = pSubChunk->pos.x / CMyLevel::GetBlockSize();
		int32 nY = pSubChunk->pos.y / CMyLevel::GetBlockSize();
		for( int j = 0; j < pSubChunk->nHeight; j++ )
		{
			for( int i = 0; i < pSubChunk->nWidth; i++ )
			{
				m_grids[i + nX + ( j + nY ) * nWidth].pChunkObject = pSubChunk->pChunkObject;
			}
		}
		if( pSubChunk->nWidth == 1 && pSubChunk->nHeight == 1 )
		{
			m_grids[nX + nY * nWidth].nType = 0;
		}
		else
		{
			m_bigChunks.resize( m_bigChunks.size() + 1 );
			m_bigChunks.back().pChunk = pSubChunk->pChunkObject;
			m_bigChunks.back().rect = TRectangle<int32>( pSubChunk->pos.x / CMyLevel::GetBlockSize(),
				pSubChunk->pos.y / CMyLevel::GetBlockSize(),
				pSubChunk->nWidth,
				pSubChunk->nHeight );
			m_bigChunks.back().nCurMoveDir = -1;
			m_bigChunks.back().bMoved = false;
			m_bigChunks.back().bMoveResult = false;
			m_nMaxCoreSize += pSubChunk->nWidth * pSubChunk->nHeight;
		}
	}
	m_nCoreSize = m_nMaxCoreSize;
	SRand::Inst().Shuffle( m_bigChunks );
	for( int i = 0; i < m_bigChunks.size(); i++ )
	{
		SafeCast<CLvBarrier2Core>( m_bigChunks[i].pChunk.GetPtr() )->Set( i );
	}

	if( GetStage() && !m_pAI )
	{
		m_pAI = new AI();
		m_pAI->SetParentEntity( this );
	}
}

void CLvBarrier2::Kill()
{
	if( m_bKilled )
		return;
	m_triggerKilled.Trigger( 0, this );
	m_bKilled = true;
	m_fHp = 0;
	AddHitShake( CVector2( 8, 0 ) );
	for( auto& pEft : m_vecEnergyEfts )
	{
		SafeCast<CCharacter>( pEft.GetPtr() )->Kill();
	}
	m_vecEnergyEfts.clear();
	KillTick();
	OnKilled();
}

void CLvBarrier2::KillTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_deathTick );

	if( m_strKillEffect )
	{
		if( m_nKillEffectCDLeft )
			m_nKillEffectCDLeft--;
		if( !m_nKillEffectCDLeft )
		{
			CVector2 center = CVector2( SRand::Inst().Rand( 0u, m_pChunk->nWidth * CMyLevel::GetBlockSize() ), SRand::Inst().Rand( 0u, m_pChunk->nHeight * CMyLevel::GetBlockSize() ) );
			auto pEffect = SafeCast<CEffectObject>( m_strKillEffect->GetRoot()->CreateInstance() );
			pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
			pEffect->SetPosition( GetPosition() + center );
			pEffect->SetState( 2 );
			m_nKillEffectCDLeft = m_nKillEffectInterval;
		}
	}

	m_nDeathTime--;
	if( !m_nDeathTime )
	{
		if( m_strEffect )
		{
			ForceUpdateTransform();
			for( int i = 0; i < m_pChunk->nWidth; i++ )
			{
				for( int j = 0; j < m_pChunk->nHeight; j++ )
				{
					auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
					pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
					pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
					pEffect->SetState( 2 );
				}
			}
		}

		auto pChunk = m_pChunk;
		if( pChunk )
		{
			m_pChunk = NULL;
			pChunk->pChunkObject = NULL;
			if( pChunk->nSubChunkType == 2 )
				delete pChunk;
			else
				CMyLevel::GetInst()->KillChunk( pChunk, this );
		}
		SetParentEntity( NULL );
	}
}

void CLvBarrier2::OnChunkRemove( uint32 nIndex )
{
	const TRectangle<int32>& rect = m_bigChunks[nIndex].rect;
	for( int i = rect.x; i < rect.GetRight(); i++ )
	{
		for( int j = rect.y; j < rect.GetBottom(); j++ )
		{
			m_grids[i + j * GetChunk()->nWidth].nType = 0;
			m_grids[i + j * GetChunk()->nWidth].pChunkObject = NULL;
		}
	}
	m_nCoreSize -= rect.width * rect.height;
	if( !m_nCoreSize )
		Kill();
}

void CLvBarrier2::OnBoxKilled( CChunkObject * pChunkObject )
{
	if( m_bKilled )
		return;
	CVector2 pos = pChunkObject->GetPosition() + CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize();
	auto pEft = SafeCast<CCharacter>( m_pEnergyEft->GetRoot()->CreateInstance() );
	pEft->SetPosition( pos + GetPosition() );
	float rad = SRand::Inst().Rand( 32.0f, 64.0f );
	float angle = SRand::Inst().Rand( -PI, PI );
	pEft->SetVelocity( CVector2( cos( angle ), sin( angle ) ) * rad );
	pEft->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
	m_vecEnergyEfts.push_back( pEft );
}

void CLvBarrier2::Move( bool bSpawnChunk )
{
	m_vecMovingGrids.clear();
	int32 nWidth = m_pChunk->nWidth;
	int32 nHeight = m_pChunk->nHeight;
	vector<TVector2<int32> > q, q1;
	for( int i = 0; i < m_q.size(); i++ )
	{
		auto& p = m_q[i];
		if( m_grids[p.x + p.y * nWidth].nType )
			continue;
		m_grids[p.x + p.y * nWidth].nType = 2;
		m_grids[p.x + p.y * nWidth].nParType = -1;
		m_grids[p.x + p.y * nWidth].nColor = i;
		q.push_back( p );
	}

	TVector2<int32> ofs[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	int i, i1;
	for( i = 0, i1 = 0;; )
	{
		TVector2<int32> p;
		if( i1 < q1.size() )
			p = q1[i1++];
		else if( i < q.size() )
			p = q[i++];
		else
			break;

		auto& grid = m_grids[p.x + p.y * nWidth];
		if( grid.pChunkObject && !grid.pChunkObject->GetStage() )
			grid.pChunkObject = NULL;

		int8 dirs[4];
		int8 nDirs;
		if( grid.nParType < 0 )
		{
			for( int k = 0; k < 4; k++ )
				dirs[k] = k;
			nDirs = 4;
			SRand::Inst().Shuffle( dirs, 4  );
		}
		else
		{
			dirs[0] = grid.nParType;
			dirs[1] = grid.nParType == 0 ? 3 : grid.nParType - 1;
			dirs[2] = grid.nParType == 3 ? 0 : grid.nParType + 1;
			if( SRand::Inst().Rand( 0, 2 ) )
				swap( dirs[1], dirs[2] );
			nDirs = 3;
		}

		for( int j = 0; j < nDirs; j++ )
		{
			int8 nDir = dirs[j];
			if( !grid.bDirs[nDir] )
				continue;
			auto p1 = p + ofs[nDir];
			if( p1.x < 0 || p1.y < 0 || p1.x >= nWidth || p1.y >= nHeight )
				continue;
			auto& grid1 = m_grids[p1.x + p1.y * nWidth];
			if( grid1.nType > 0 )
				continue;
			if( grid1.nColor >= 0 && grid1.nColor != grid.nColor && !grid.pChunkObject )
				continue;

			grid1.nType = 2;
			grid1.nParType = nDir;
			grid1.par = p;
			grid1.nColor = grid.nColor;
			( grid.pChunkObject ? q1 : q ).push_back( p1 );
		}
	}

	for( i = q1.size() - 1; i >= 0; i-- )
	{
		auto& p = q1[i];
		auto& grid = m_grids[p.x + p.y * nWidth];
		grid.nType = 0;

		auto& p1 = grid.par;
		auto& grid1 = m_grids[p1.x + p1.y * nWidth];
		if( grid1.pChunkObject )
		{
			if( grid.pChunkObject )
			{
				auto pBox = SafeCast<CLvBarrier2Box>( grid.pChunkObject.GetPtr() );
				auto pBox1 = SafeCast<CLvBarrier2Box>( grid1.pChunkObject.GetPtr() );
				if( pBox->GetState() && !pBox1->GetState() )
				{
					grid.pChunkObject->Damage( 1 );
					if( !grid.pChunkObject->GetParentEntity() )
						grid.pChunkObject = NULL;
				}
			}
			if( !grid.pChunkObject )
			{
				grid.pChunkObject = grid1.pChunkObject;
				grid1.pChunkObject = NULL;
				m_vecMovingGrids.push_back( p );
			}
		}
	}

	for( i = q.size() - 1; i >= 0; i-- )
	{
		auto& p = q[i];
		auto& grid = m_grids[p.x + p.y * nWidth];
		grid.nType = 0;

		if( bSpawnChunk && grid.nParType == -1 )
		{
			if( grid.pChunkObject )
			{
				auto pBox = SafeCast<CLvBarrier2Box>( grid.pChunkObject.GetPtr() );
				if( pBox->GetState() )
				{
					grid.pChunkObject->Damage( 1 );
					if( !grid.pChunkObject->GetParentEntity() )
						grid.pChunkObject = NULL;
				}
			}
			if( !grid.pChunkObject )
			{
				TVector2<int32> pos = p;
				if( p.x < 2 )
					pos.x--;
				else if( p.x >= nWidth - 2 )
					pos.x++;
				else
					pos.y++;
				auto pSubChunk = m_pCreateNode->AddSubChunk( this, TRectangle<int32>( pos.x, pos.y, 1, 1 ) );

				grid.pChunkObject = pSubChunk->pChunkObject;
				grid.pChunkObject->SetRenderParent( m_pBoxLayer );
				m_vecMovingGrids.push_back( p );
			}
		}
	}
}

bool CLvBarrier2::TryMoveBigChunk( SBigChunk& bigChunk, int8 nDir, uint8 nMoveType )
{
	if( bigChunk.bMoved )
		return bigChunk.bMoveResult;
	bigChunk.bMoved = true;
	bigChunk.nCurMoveDir = nDir;
	int32 nWidth = m_pChunk->nWidth;
	int32 nHeight = m_pChunk->nHeight;
	TVector2<int32> dirs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	auto dir = dirs[nDir];
	TRectangle<int32> rect = bigChunk.rect;
	auto& pChunkObject = bigChunk.pChunk;
	CLvBarrier2Core* pCore = SafeCast<CLvBarrier2Core>( pChunkObject.GetPtr() );
	if( rect.x + dir.x < 2 || rect.GetRight() + dir.x > nWidth - 2 || rect.y + dir.y < 0 || rect.GetBottom() + dir.y > nHeight - 2 )
		return false;

	TRectangle<int32> r, r1;
	if( dir.x < 0 )
	{
		r = TRectangle<int32>( rect.x - 1, rect.y, 1, rect.height );
		r1 = TRectangle<int32>( rect.GetRight() - 1, rect.y, 1, rect.height );
	}
	else if( dir.x > 0 )
	{
		r = TRectangle<int32>( rect.GetRight(), rect.y, 1, rect.height );
		r1 = TRectangle<int32>( rect.x, rect.y, 1, rect.height );
	}
	else if( dir.y < 0 )
	{
		r = TRectangle<int32>( rect.x, rect.y - 1, rect.width, 1 );
		r1 = TRectangle<int32>( rect.x, rect.GetBottom() - 1, rect.width, 1 );
	}
	else
	{
		r = TRectangle<int32>( rect.x, rect.GetBottom(), rect.width, 1 );
		r1 = TRectangle<int32>( rect.x, rect.y, rect.width, 1 );
	}

	bool bBlocked = false;
	for( int iX = r.x; iX < r.GetRight(); iX++ )
	{
		for( int iY = r.y; iY < r.GetBottom(); iY++ )
		{
			auto& pChunkObject = m_grids[iX + iY * nWidth].pChunkObject;
			if( pChunkObject && !pChunkObject->GetStage() )
				pChunkObject = NULL;
			if( pChunkObject )
			{
				auto pBigChunk = SafeCast<CLvBarrier2Core>( pChunkObject.GetPtr() );
				if( pBigChunk )
				{
					auto& bigChunk = m_bigChunks[pBigChunk->m_nIndex];
					if( nMoveType == 1 )
					{
						if( bigChunk.bMoved || !TryMoveBigChunk( bigChunk, nDir, nMoveType ) )
							return false;
					}
					else if( nMoveType == 2 )
					{
						bigChunk.nCurMoveDir = nDir;
						return false;
					}
					else if( nMoveType == 3 )
					{
						TryMoveBigChunk( bigChunk, nDir, nMoveType );
						bBlocked = true;
					}
				}
				else if( m_grids[iX + iY * nWidth].nType )
				{
					bBlocked = true;
					if( nMoveType < 3 )
						return false;
				}
			}
			else if( m_grids[iX + iY * nWidth].nType )
			{
				bBlocked = true;
				if( nMoveType < 3 )
					return false;
			}
		}
	}
	if( bBlocked )
		return false;

	for( int iX = r.x; iX < r.GetRight(); iX++ )
	{
		for( int iY = r.y; iY < r.GetBottom(); iY++ )
		{
			m_grids[iX + iY * nWidth].nType = 1;
			if( m_grids[iX + iY * nWidth].pChunkObject )
			{
				m_grids[iX + iY * nWidth].pChunkObject->Crush();
				//CreateExplosion( iX, iY, pChunkObject );
			}
			m_grids[iX + iY * nWidth].pChunkObject = pChunkObject;
		}
	}
	for( int iX = r1.x; iX < r1.GetRight(); iX++ )
	{
		for( int iY = r1.y; iY < r1.GetBottom(); iY++ )
		{
			m_grids[iX + iY * nWidth].nType = 0;
			m_grids[iX + iY * nWidth].pChunkObject = NULL;
		}
	}
	bigChunk.rect = rect.Offset( dir );
	bigChunk.bMoveResult = true;
	return true;
}

void CLvBarrier2::Move1()
{
	uint8 nMoveType = 1;

	if( nMoveType == 1 )
	{
		int8 nDir = m_bigChunks[0].nCurMoveDir;
		if( nDir == -1 )
			nDir = SRand::Inst().Rand( 0, 4 );

		int8 nDirs[] = { nDir, ( nDir + 1 ) & 3, ( nDir - 1 ) & 3, ( nDir + 2 ) & 3 };
		if( SRand::Inst().Rand( 0, 2 ) )
			swap( nDirs[1], nDirs[2] );
		for( int i = 0; i < 4; i++ )
		{
			bool bMoved = false;
			for( auto& bigChunk : m_bigChunks )
				bMoved = TryMoveBigChunk( bigChunk, nDirs[i], nMoveType ) || bMoved;
			for( auto& bigChunk : m_bigChunks )
				bigChunk.bMoved = bigChunk.bMoveResult = false;
			if( bMoved )
				break;
		}
	}
	else if( nMoveType == 2 )
	{
		for( auto& bigChunk : m_bigChunks )
		{
			int8 nDir = bigChunk.nCurMoveDir;
			if( nDir != 0 && nDir != 2 )
				nDir = SRand::Inst().Rand( 0, 2 ) * 2;
			bool bMoved = TryMoveBigChunk( bigChunk, nDir, nMoveType );
			if( !bMoved )
			{
				nDir = ( nDir + 2 ) & 3;
				bigChunk.bMoved = bigChunk.bMoveResult = false;
				TryMoveBigChunk( bigChunk, nDir, nMoveType );
			}
			bigChunk.bMoved = bigChunk.bMoveResult = false;
		}
	}
	else if( nMoveType == 3 )
	{
		for( auto& bigChunk : m_bigChunks )
		{
			int8 nDir = bigChunk.nCurMoveDir;
			if( nDir >= 0 )
				TryMoveBigChunk( bigChunk, nDir, nMoveType );
		}
		bool bMoved = false;
		for( auto& bigChunk : m_bigChunks )
		{
			if( !bigChunk.bMoveResult )
				bigChunk.nCurMoveDir = -1;
			else
				bMoved = true;
			bigChunk.bMoved = bigChunk.bMoveResult = false;
		}

		if( !bMoved )
		{
			int32 nMin = -1;
			uint32 nMinDist = 0xffffffff;
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 p = ( pPlayer->GetPosition() - globalTransform.GetPosition() ) / 32;
				TVector2<int32> target( p.x, p.y );
				for( int i = 0; i < m_bigChunks.size(); i++ )
				{
					auto& bigChunk = m_bigChunks[i];
					uint32 distX = Max( 0, Max( bigChunk.rect.x - target.x, target.x - bigChunk.rect.GetRight() ) );
					uint32 distY = Max( 0, Max( bigChunk.rect.y - target.y, target.y - bigChunk.rect.GetBottom() ) );
					uint32 dist1 = Max( distX, distY );
					uint32 dist2 = Min( distX, distY );
					uint32 dist = ( dist2 << 16 ) | ( ~dist1 & 0xffff );
					if( dist < nMinDist )
					{
						nMinDist = dist;
						nMin = i;
					}
				}
				if( nMin >= 0 )
				{
					auto& bigChunk = m_bigChunks[nMin];
					uint32 distX = Max( 0, Max( bigChunk.rect.x - target.x, target.x - bigChunk.rect.GetRight() ) );
					uint32 distY = Max( 0, Max( bigChunk.rect.y - target.y, target.y - bigChunk.rect.GetBottom() ) );
					if( distX >= distY )
						bigChunk.nCurMoveDir = bigChunk.rect.x + bigChunk.rect.GetRight() > target.x * 2 ? 2 : 0;
					else
						bigChunk.nCurMoveDir = bigChunk.rect.y + bigChunk.rect.GetBottom() > target.y * 2 ? 3 : 1;
				}
			}
		}
	}

	if( m_nEnergy >= 50 && !m_nFireCD )
	{
		/*m_nFireCD = GenBarrage2( m_nAttackType, m_pLabel->GetPosition() );
		m_nEnergy -= m_nFireCD * 4;*/

		if( m_nCurMoveType == 0 )
			m_nCurMoveType = SRand::Inst().Rand( 1, 3 );
		else
		{
			uint32 nMoveType = SRand::Inst().Rand( 1, 2 );
			if( nMoveType >= m_nCurMoveType )
				nMoveType++;
			m_nCurMoveType = nMoveType;
		}
		m_nEnergy = 0;
		m_pLabel->UpdatePercent( m_nEnergy / 50.0f );

		uint32 nAttackType = SRand::Inst().Rand( 0, 2 );
		if( nAttackType >= m_nAttackType )
			nAttackType++;
		m_nAttackType = nAttackType;
	}
	if( m_nFireCD )
		m_nFireCD--;
}

void CLvBarrier2::SetCurMoveType( uint8 nMoveType )
{
	m_nCurMoveType = nMoveType;
	for( auto& bigChunk : m_bigChunks )
		bigChunk.nCurMoveDir = -1;
}

void CLvBarrier2::UpdateEnergyEfts()
{
	CVector2 targetPos = m_pLabel->GetPosition();
	targetPos = targetPos + GetPosition();
	for( int32 i = m_vecEnergyEfts.size() - 1; i >= 0; i-- )
	{
		auto pCharacter = SafeCast<CCharacter>( m_vecEnergyEfts[i].GetPtr() );

		m_energyEftFlyData.UpdateMove( pCharacter, targetPos );
		if( ( pCharacter->GetPosition() - targetPos ).Length2() < 8 * 8 )
		{
			pCharacter->Kill();
			m_vecEnergyEfts[i] = m_vecEnergyEfts.back();
			m_vecEnergyEfts.pop_back();
			m_nEnergy = Min( m_nEnergy + 1, 50u );
			m_pLabel->UpdatePercent( m_nEnergy / 50.0f );
		}
	}
}

void CLvBarrier2::CreateExplosion( int32 iX, int32 iY, CChunkObject* pCreator )
{
	auto pExp = SafeCast<CExplosionWithBlockBuff>( m_pExplosion->GetRoot()->CreateInstance() );
	CBlockBuff::SContext buffcontext;
	buffcontext.nLife = 450;
	buffcontext.nTotalLife = 450;
	buffcontext.fParams[0] = 3;
	pExp->Set( &buffcontext );
	pExp->SetCreator( pCreator );
	pExp->SetPosition( GetPosition() + CVector2( iX + 0.5f, iY + 0.5f ) * CMyLevel::GetBlockSize() );
	pExp->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
}

void CLvBarrier2::AIFunc()
{
	m_nAttackType = SRand::Inst().Rand( 0, 3 );
	InitBarrage1();
	string str = m_strCreateNode;
	int32 n = str.find( '.' );
	if( n != string::npos )
	{
		str[n] = 0;
		m_pCreateNode = CGlobalCfg::Inst().levelGenerateNodeContext.FindFile( &str[0] )->FindNode( &str[n + 1] );
	}
	m_pAI->Yield( 1.0f, true );

	int32 nWidth = m_pChunk->nWidth;
	int32 nHeight = m_pChunk->nHeight;
	int32 iCount = 0;
	while( 1 )
	{
		Move( iCount == 0 );
		for( int i = 0; i < 16; i++ )
		{
			for( int i = 0; i < 4; i++ )
			{
				if( m_pBarrages1[i] )
					m_pBarrages1[i]->Update();
			}
			for( auto& p : m_vecMovingGrids )
			{
				auto& grid = m_grids[p.x + p.y * nWidth];
				auto pChunkObject = grid.pChunkObject;
				if( !pChunkObject )
					continue;
				if( !pChunkObject->GetStage() )
				{
					grid.pChunkObject = NULL;
					continue;
				}

				CVector2 targetPos( p.x * CMyLevel::GetBlockSize(), p.y * CMyLevel::GetBlockSize() );
				CVector2 p = grid.pChunkObject->GetPosition();
				if( p.x < targetPos.x )
					p.x += 2;
				else if( p.x > targetPos.x )
					p.x -= 2;
				if( p.y < targetPos.y )
					p.y += 2;
				else if( p.y > targetPos.y )
					p.y -= 2;
				grid.pChunkObject->SetPosition( p );
			}
			
			UpdateEnergyEfts();
			m_pAI->Yield( 0, true );
		}

		Move1();
		for( int i = 0; i < 16; i++ )
		{
			for( int i = 0; i < 4; i++ )
			{
				if( m_pBarrages1[i] )
					m_pBarrages1[i]->Update();
			}
			for( auto& bigChunk : m_bigChunks )
			{
				auto& pChunkObject = bigChunk.pChunk;
				if( pChunkObject && !pChunkObject->GetStage() )
					pChunkObject = NULL;
				if( !pChunkObject )
					continue;

				auto targetPos = CVector2( bigChunk.rect.x, bigChunk.rect.y ) * CMyLevel::GetBlockSize();
				CVector2 p = pChunkObject->GetPosition();
				if( p.x < targetPos.x )
					p.x += 2;
				else if( p.x > targetPos.x )
					p.x -= 2;
				if( p.y < targetPos.y )
					p.y += 2;
				else if( p.y > targetPos.y )
					p.y -= 2;
				pChunkObject->SetPosition( p );
			}

			UpdateEnergyEfts();
			m_pAI->Yield( 0, true );
		}

		if( iCount == 0 )
			iCount = m_nCoreSize * 4 / m_nMaxCoreSize + 1;
		iCount--;
	}
}

void CLvBarrier2Box::Kill()
{
	if( m_nState )
	{
		CChunkObject::Kill();
		return;
	}

	m_nState = 1;
	m_fHp = m_nMaxHp = m_nMaxHp1;
	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
	pImage2D->SetTexRect( pImage2D->GetElem().texRect.Offset( m_texOfs ) );
	for( auto& block : GetChunk()->blocks )
	{
		auto pBlockObject = SafeCast<CBlockObject>( block.pEntity.GetPtr() );
		pBlockObject->ClearBuffs();
		pBlockObject->ClearEfts();
		block.rtTexRect = block.rtTexRect.Offset( m_texOfs );
		block.eBlockType = eBlockType_Wall;
		block.pEntity->SetHitType( eEntityHitType_Sensor );
		block.bImmuneToBlockBuff = 1;
	}
	SafeCast<CLvBarrier2>( GetParentEntity() )->OnBoxKilled( this );
}

void CLvBarrier2Box::Crush()
{
	if( m_nState == 0 )
	{
		m_nState = 1;
		SafeCast<CLvBarrier2>( GetParentEntity() )->OnBoxKilled( this );
	}
	CChunkObject::Crush();
}

void CLvBarrier2Core::OnRemovedFromStage()
{
	CLvBarrier2* pOwner = SafeCast<CLvBarrier2>( GetParentEntity() );
	if( pOwner->GetStage() )
	{
		if( m_nIndex >= 0 )
			pOwner->OnChunkRemove( m_nIndex );
	}
	CChunkObject::OnRemovedFromStage();
}

void CLvBarrier2::InitBarrage1()
{
	{
		class CBarrage1_1 : public CBarrage1
		{
		public:
			CBarrage1_1( const SBarrageContext& context ) : CBarrage1( context ) {}

			const int32 nBulletCount = 2;
			const int32 nLightningCount = 1;
		protected:
			virtual void UpdateItem( uint32 nIndex )
			{
				int32 nBullet = nIndex * nBulletCount;
				int32 nLightning = nIndex * nLightningCount;
				auto& item = m_items[nIndex];

				switch( item.nState )
				{
				case 0:
				{
					if( m_nTick == 0 )
					{
						CVector2 pos( item.initRect.x + 0.5f, item.initRect.y + 0.5f );
						pos = pos * CMyLevel::GetBlockSize();
						InitBullet( nBullet, 0, -1, pos, CVector2( 0, 0 ), CVector2( 0, 0 ) );
						InitBullet( nBullet + 1, 0, -1, pos, CVector2( 0, 0 ), CVector2( 0, 0 ) );
						InitLightning( nLightning, 0, nBullet, nBullet + 1, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
					}
					break;
				}
				case 1:
				case 2:
				case 3:
				case 4:
				{
					if( m_nTick == 0 )
					{
						CVector2 dirs[3] = { { 2, 0 }, { 1, 1 }, { 1, -1 } };
						CVector2 dir = dirs[SRand::Inst().Rand( 0, 3 )];
						GetBulletContext( nBullet )->SetBulletMove( CVector2( dir.x * item.dir.x - dir.y * item.dir.y, dir.x * item.dir.y + dir.y * item.dir.x ) * 64, CVector2( 0, 0 ) );
						GetBulletContext( nBullet + 1 )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
					}
					else if( m_nTick == 16 )
					{
						auto pBullet0 = GetBulletContext( nBullet );
						pBullet0->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
						GetBulletContext( nBullet + 1 )->MoveTowards( pBullet0->p0, 16 );
					}
					break;
				}
				case 5:
				{
					if( m_nTick == 0 )
					{
						DestroyBullet( nBullet );
						DestroyBullet( nBullet + 1 );
						DestroyLightning( nLightning );
						FreeItem( nIndex );
					}
				}
				}
			}
		};
		SBarrageContext context;
		context.pCreator = GetParentEntity();
		context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
		context.vecLightningTypes.push_back( m_pLightning.GetPtr() );
		context.nBulletPageSize = 100;
		context.nLightningPageSize = 50;
		context.fTimeScale = 32;
		context.bAutoDeletePages = false;
		m_pBarrages1[0] = new CBarrage1_1( context );
		m_pBarrages1[0]->SetParentEntity( this );
		m_pBarrages1[0]->SetRenderParent( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		m_pBarrages1[0]->Start();
	}
	{
		class CBarrage1_3 : public CBarrage1
		{
		public:
			CBarrage1_3( const SBarrageContext& context ) : CBarrage1( context ) {}

			const int32 nBulletCount = 6;
		protected:
			virtual void UpdateItem( uint32 nIndex )
			{
				int32 nBullet = nIndex * nBulletCount;
				auto& item = m_items[nIndex];
				CVector2 dir1[4];

				switch( item.nState )
				{
				case 0:
				{
					if( m_nTick == 0 )
					{
						CVector2 pos( item.initRect.x + item.initRect.width * ( item.dir.x + item.dir.y + 1 ) * 0.5f,
							item.initRect.y + item.initRect.height * ( item.dir.y - item.dir.x + 1 ) * 0.5f );
						CVector2 pos1( item.initRect.x + item.initRect.width * ( item.dir.x - item.dir.y + 1 ) * 0.5f,
							item.initRect.y + item.initRect.height * ( item.dir.x + item.dir.y + 1 ) * 0.5f );
						pos = pos * CMyLevel::GetBlockSize();
						pos1 = pos1 * CMyLevel::GetBlockSize();
						InitBullet( nBullet, 0, -1, pos, item.dir * 32, CVector2( 0, 0 ) );
						InitBullet( nBullet + 1, 0, -1, pos1, item.dir * 32, CVector2( 0, 0 ) );
					}
					break;
				}
				case 1:
				case 2:
				{
					nBullet += ( item.nState - 1 ) * 2;
					if( m_nTick == 0 )
					{
						if( GetBulletContext( nBullet )->pEntity )
						{
							GetBulletContext( nBullet )->SetBulletMove( item.dir * 160, CVector2( 0, 0 ) );
							GetBulletContext( nBullet )->SetBulletMove( item.dir * 160, CVector2( 0, 0 ) );
							InitBullet( nBullet + 2, 0, -1, GetBulletContext( nBullet )->p0, CVector2( 0, 0 ), CVector2( 0, 0 ) );
							GetBulletContext( nBullet + 2 )->MoveTowards( GetBulletContext( nBullet )->p0 + item.dir * 32
								+ CVector2( -item.dir.y, item.dir.x ) * SRand::Inst().Rand( 1, 3 ) * 16, 16 );
						}
						if( GetBulletContext( nBullet + 1 )->pEntity )
						{
							GetBulletContext( nBullet + 1 )->SetBulletMove( item.dir * 160, CVector2( 0, 0 ) );
							GetBulletContext( nBullet + 1 )->SetBulletMove( item.dir * 160, CVector2( 0, 0 ) );
							InitBullet( nBullet + 3, 0, -1, GetBulletContext( nBullet + 1 )->p0, CVector2( 0, 0 ), CVector2( 0, 0 ) );
							GetBulletContext( nBullet + 3 )->MoveTowards( GetBulletContext( nBullet + 1 )->p0 + item.dir * 32
								+ CVector2( item.dir.y, -item.dir.x ) * SRand::Inst().Rand( 1, 3 ) * 16, 16 );
						}
					}
					else if( m_nTick == 16 )
					{
						if( GetBulletContext( nBullet + 2 )->pEntity )
							GetBulletContext( nBullet + 2 )->SetBulletMove( item.dir * 128, CVector2( 0, 0 ) );
						if( GetBulletContext( nBullet + 3 )->pEntity )
							GetBulletContext( nBullet + 3 )->SetBulletMove( item.dir * 128, CVector2( 0, 0 ) );
					}
					break;
				}
				default:
				{
					if( m_nTick == 0 )
					{
						for( int i = 0; i < nBulletCount; i++ )
						{
							auto pBulletContext = GetBulletContext( nBullet + i );
							if( pBulletContext && pBulletContext->pEntity )
								return;
						}
						for( int i = 0; i < nBulletCount; i++ )
							DestroyBullet( nBullet + i );
						FreeItem( nIndex );
					}
					break;
				}
				}
			}
		};
		SBarrageContext context;
		context.pCreator = GetParentEntity();
		context.vecBulletTypes.push_back( m_pBullet3.GetPtr() );
		context.nBulletPageSize = 100;
		context.nLightningPageSize = 50;
		context.fTimeScale = 32;
		context.bAutoDeletePages = false;
		m_pBarrages1[2] = new CBarrage1_3( context );
		m_pBarrages1[2]->SetParentEntity( this );
		m_pBarrages1[2]->SetRenderParent( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		m_pBarrages1[2]->Start();
	}
}

void CLvBarrier2::GenBarrage1( uint32 nType, const TRectangle<int32>& rect, const CVector2& dir )
{
	return;
	switch( nType )
	{
	case 0:
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				TRectangle<int32> r( i, j, 0, 0 );
				m_pBarrages1[nType]->Fire( r, dir );
			}
		}
		break;
	case 1:
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				CVector2 pos = CVector2( i, j ) * CMyLevel::GetBlockSize() + GetPosition();
				CVector2 vel;
				if( dir.y > 0 )
				{
					float v = SRand::Inst().Rand( 140.0f, 160.0f );
					float angle = SRand::Inst().Rand( PI * 0.3f, PI * 0.7f );
					vel = CVector2( cos( angle ), sin( angle ) ) * v;
				}
				else if( dir.y < 0 )
				{
					vel = CVector2( SRand::Inst().Rand( -150.0f, -300.0f ) * ( SRand::Inst().Rand( 0, 2 ) - 0.5f ), SRand::Inst().Rand( -60.0f, -80.0f ) );
				}
				else
				{
					float v = SRand::Inst().Rand( 120.0f, 140.0f );
					float angle = SRand::Inst().Rand( PI * 0.0f, PI * 0.3f );
					vel = CVector2( cos( angle ), sin( angle ) ) * v;
					if( dir.x < 0 )
						vel.x = -vel.x;
				}

				auto pBullet = SafeCast<CBullet>( m_pBullet2->GetRoot()->CreateInstance() );
				pBullet->SetPosition( pos );
				pBullet->SetRotation( SRand::Inst().Rand( -PI, PI ) );
				pBullet->SetAngularVelocity( ( SRand::Inst().Rand( 0, 2 ) - 0.5f ) * 6.0f );
				pBullet->SetVelocity( vel );
				pBullet->SetAcceleration( CVector2( 0, -150.0f ) );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}
		}
		break;
	case 2:
		m_pBarrages1[nType]->Fire( rect, dir );
		break;
	default:
		break;
	}
}

int32 CLvBarrier2::GenBarrage2( uint32 nType, const CVector2& pos )
{
	return 0;
	switch( nType )
	{
	case 0:
	{
		SBarrageContext context;
		context.pCreator = GetParentEntity();
		context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
		context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
		context.vecLightningTypes.push_back( m_pLightning.GetPtr() );
		context.vecLightningTypes.push_back( m_pLightning0.GetPtr() );
		context.nBulletPageSize = 400;
		context.nLightningPageSize = 240;
		context.fTimeScale = 32;

		CBarrage* pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [] ( CBarrage* pBarrage )
		{
			float fAngle0 = SRand::Inst().Rand( -PI, PI );
			uint32 n = 16;
			float v0 = 64;
			float v1 = 64;
			int32 nWaves = 5;
			CVector2 vel;
			float dx = pBarrage->GetPosition().x - 512;
			float r = SRand::Inst().Rand( 0, 1024 );
			if( r < dx - 256 )
				vel = CVector2( -8, -12 );
			else if( r >= dx + 256 )
				vel = CVector2( 8, -12 );
			else
				vel = CVector2( 0, -14 );
			pBarrage->InitBullet( 0, 0, -1, CVector2( 0, 0 ), vel, CVector2( 0, 0 ), false );

			for( int k = 0; k <= nWaves; k++ )
			{
				for( int j = 0; j < k; j++ )
				{
					int32 nB0 = 1 + ( 1 + n * 2 ) * j * 2;
					int32 nB1 = 1 + ( 1 + n * 2 ) * ( j * 2 + 1 );
					for( int i = 0; i < n; i++ )
					{
						pBarrage->GetBulletContext( nB0 + 1 + n + i )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
						pBarrage->GetBulletContext( nB1 + 1 + n + i )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
					}
				}
				pBarrage->Yield( 32 );

				int32 nBullet0 = 1 + ( 1 + n * 2 ) * k * 2;
				pBarrage->InitBullet( nBullet0, -1, 0, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false, PI / n * ( k & 1 ) );
				for( int i = 0; i < n; i++ )
				{
					pBarrage->InitBullet( nBullet0 + 1 + i, -1, nBullet0, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false, fAngle0 + PI * 2 * i / n );
					pBarrage->InitBullet( nBullet0 + 1 + n + i, 1, nBullet0 + 1 + i, CVector2( 0, 0 ), CVector2( v0, 0 ), CVector2( 0, 0 ) );
					pBarrage->InitLightning( i, 0, 0, nBullet0 + 1 + n + i, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
				}
				for( int j = 0; j < k; j++ )
				{
					int32 nDir = !!( ( j + k ) & 1 ) ? 1 : -1;
					int32 nB0 = 1 + ( 1 + n * 2 ) * j * 2;
					int32 nB1 = 1 + ( 1 + n * 2 ) * ( j * 2 + 1 );
					pBarrage->GetBulletContext( nB0 )->SetBulletMoveA( PI / n * nDir, 0 );
					pBarrage->GetBulletContext( nB1 )->SetBulletMoveA( PI / n * nDir, 0 );
					for( int i = 0; i < n; i++ )
					{
						pBarrage->InitLightning( i + ( j + 1 ) * n, 0, nB0 + 1 + n + i, nB1 + 1 + n + i, CVector2( 0, 0 ), CVector2( 0, 0 ), true );

						int32 i1 = i == n - 1 ? 0 : i + 1;
						if( !!( ( i + ( k & 1 ? j : 0 ) + k ) & 1 ) )
							pBarrage->InitLightning( i + ( j + k + 1 ) * n, 1, nB1 + 1 + n + i, nB1 + 1 + n + i1, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
						else
							pBarrage->InitLightning( i + ( j + k + 1 ) * n, 1, nB0 + 1 + n + i, nB0 + 1 + n + i1, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
					}
				}
				pBarrage->Yield( 32 );

				for( int j = 0; j < k; j++ )
				{
					int32 nB0 = 1 + ( 1 + n * 2 ) * j * 2;
					int32 nB1 = 1 + ( 1 + n * 2 ) * ( j * 2 + 1 );
					pBarrage->GetBulletContext( nB0 )->SetBulletMoveA( 0, 0 );
					pBarrage->GetBulletContext( nB1 )->SetBulletMoveA( 0, 0 );
				}
				for( int i = 0; i < n; i++ )
				{
					pBarrage->GetBulletContext( nBullet0 + 1 + n + i )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
				}
				pBarrage->Yield( 32 );

				int32 nBullet1 = 1 + ( 1 + n * 2 ) * ( k * 2 + 1 );
				pBarrage->InitBullet( nBullet1, -1, 0, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false, PI / n * ( k & 1 ) );
				for( int i = 0; i < n; i++ )
				{
					pBarrage->InitBullet( nBullet1 + 1 + i, -1, nBullet1, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false, fAngle0 + PI * 2 * i / n );
					pBarrage->InitBullet( nBullet1 + 1 + n + i, 1, nBullet1 + 1 + i, CVector2( 0, 0 ), CVector2( v0, 0 ), CVector2( 0, 0 ) );
					pBarrage->InitLightning( i, 0, 0, nBullet1 + 1 + n + i, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
					pBarrage->GetBulletContext( nBullet0 + 1 + n + i )->SetBulletMove( CVector2( v1, 0 ), CVector2( 0, 0 ) );
				}
				for( int j = 0; j <= k; j++ )
				{
					int32 nB0 = 1 + ( 1 + n * 2 ) * j * 2;
					int32 nB1 = 1 + ( 1 + n * 2 ) * ( j * 2 + 1 );
					for( int i = 0; i < n; i++ )
					{
						if( j < k )
						{
							pBarrage->GetBulletContext( nB0 + 1 + n + i )->SetBulletMove( CVector2( v1, 0 ), CVector2( 0, 0 ) );
							pBarrage->GetBulletContext( nB1 + 1 + n + i )->SetBulletMove( CVector2( v1, 0 ), CVector2( 0, 0 ) );
						}
						int32 i1 = i == n - 1 ? 0 : i + 1;
						if( !!( ( i + ( k & 1 ? j : 0 ) + k ) & 1 ) )
							pBarrage->InitLightning( i + ( j + 1 ) * n, 0, nB1 + 1 + n + i, nB1 + 1 + n + i1, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
						else
							pBarrage->InitLightning( i + ( j + 1 ) * n, 0, nB0 + 1 + n + i, nB0 + 1 + n + i1, CVector2( 0, 0 ), CVector2( 0, 0 ), true );

						pBarrage->InitLightning( i + ( j + k + 2 ) * n, 1, nB0 + 1 + n + i, nB1 + 1 + n + i, CVector2( 0, 0 ), CVector2( 0, 0 ), true );
					}
				}
				pBarrage->Yield( 32 );
			}

			int32 nBullet1 = 1 + ( 1 + n * 2 ) * ( nWaves * 2 + 1 );
			for( int i = 0; i < n; i++ )
			{
				pBarrage->GetBulletContext( nBullet1 + 1 + n + i )->SetBulletMove( CVector2( v1, 0 ), CVector2( 0, 0 ) );
				pBarrage->DestroyLightning( i );
			}
			pBarrage->DestroyBullet( 0 );

			pBarrage->Yield( 2 );
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetPosition( pos + GetPosition() );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->Start();
		return 24;
	}
	case 1:
	{
		SBarrageContext context;
		context.pCreator = GetParentEntity();
		context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
		context.vecBulletTypes.push_back( m_pBullet2.GetPtr() );
		context.nBulletPageSize = 400;
		context.fTimeScale = 64;

		CVector2 beginPos = pos + GetPosition();
		CBarrage* pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [beginPos] ( CBarrage* pBarrage )
		{
			CVector2 p[16];
			p[0] = beginPos;
			p[1] = CVector2( SRand::Inst().Rand( 0.4f, 0.6f ) * 1024, 768 );
			for( int i = 0; i < 2; i++ )
				p[i + 2] = CVector2( ( SRand::Inst().Rand( 0.4f, 0.6f ) + i ) * 0.5f * 1024, 512 );
			for( int i = 0; i < 4; i++ )
				p[i + 4] = CVector2( ( SRand::Inst().Rand( 0.4f, 0.6f ) + ( i >> 1 ) + ( i & 1 ) * 2 ) * 0.25f * 1024, 384 );
			for( int i = 0; i < 8; i++ )
				p[i + 8] = CVector2( ( SRand::Inst().Rand( 0.4f, 0.6f ) + ( i >> 1 ) + ( i & 1 ) * 4 ) * 0.125f * 1024, 256 );
			CVector2 a( 0, -2400.0f );
			CVector2 a1( 0, -200.0f );
			float t = 1.0f;
			CVector2 v[16];
			for( int i = 1; i < 16; i++ )
			{
				CVector2 dPos = CVector2( p[i].x, 0 ) - p[i >> 1];
				v[i] = dPos / t - a * ( t * 0.5f );
			}

			int32 nB1 = 16;
			for( int iWave = 0; iWave < 4; iWave++ )
			{
				int32 nBegin = 1 << iWave;
				for( int iBullet = nBegin; iBullet < nBegin * 2; iBullet++ )
				{
					pBarrage->InitBullet( iBullet, 0, -1, p[iBullet >> 1], v[iBullet], a, false );
				}

				pBarrage->Yield( 2 );
				for( int iTime = 0; iTime < 16; iTime++ )
				{
					float t = ( iTime * 2 + 1 ) / 32.0f;
					for( int iBullet = nBegin; iBullet < nBegin * 2; iBullet++ )
					{
						pBarrage->InitBullet( nB1++, 1, -1, p[iBullet >> 1] + v[iBullet] * t + a * ( 0.5f * t * t ),
							CVector2( 0, 0 ), a1, false, iTime * PI / 8 );
					}
					if( iTime < 15 )
						pBarrage->Yield( 4 );
				}
				pBarrage->Yield( 2 );

				for( int iBullet = nBegin; iBullet < nBegin * 2; iBullet++ )
				{
					pBarrage->GetBulletContext( iBullet )->SetBulletMove( CVector2( p[iBullet].x, 0 ), CVector2( 0, p[iBullet].y ), CVector2( 0, 0 ) );
				}
				pBarrage->Yield( 2 );
				for( int iTime = 0; iTime < 16; iTime++ )
				{
					float t = ( iTime * 2 + 1 ) / 32.0f;
					for( int iBullet = nBegin; iBullet < nBegin * 2; iBullet++ )
					{
						pBarrage->InitBullet( nB1++, 1, -1, CVector2( p[iBullet].x, p[iBullet].y * t ),
							CVector2( 160 - iWave * 32, 64 ) * ( ( 1 - t ) * ( 1 - t ) ), a1, false, iTime * PI / 8, 6.0f * ( 1 - t ) );
						pBarrage->InitBullet( nB1++, 1, -1, CVector2( p[iBullet].x, p[iBullet].y * t ),
							CVector2( -160 + iWave * 32, 64 ) * ( ( 1 - t ) * ( 1 - t ) ), a1, false, iTime * PI / 8, -6.0f * ( 1 - t ) );
					}
					if( iTime < 15 )
						pBarrage->Yield( 4 );
				}
				pBarrage->Yield( 2 );
				for( int iBullet = nBegin; iBullet < nBegin * 2; iBullet++ )
				{
					pBarrage->DestroyBullet( iBullet );
				}
			}

			pBarrage->Yield( 2 );
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->Start();
		return 24;
	}
	case 2:
	{
		SBarrageContext context;
		context.pCreator = GetParentEntity();
		context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
		context.vecBulletTypes.push_back( m_pBullet3.GetPtr() );
		context.vecLightningTypes.push_back( m_pBeam.GetPtr() );
		context.nBulletPageSize = 400;
		context.nLightningPageSize = 2;
		context.fTimeScale = 64;

		CVector2 beginPos = pos + GetPosition();
		CBarrage* pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [beginPos] ( CBarrage* pBarrage )
		{
			CVector2 p = beginPos;
			CVector2 playerPos( 512, 0 );
			pBarrage->InitBullet( 0, 0, -1, p, CVector2( 0, 0 ), CVector2( 0, 0 ), false );
			uint32 nBullet1 = 3;
			uint32 nBullet2 = 3 + 32;
			
			for( int i = 0; i < 2; i++ )
			{
				if( pBarrage->GetStage()->GetPlayer() )
					playerPos = pBarrage->GetStage()->GetPlayer()->GetPosition();

				CVector2 dPos = p - playerPos;
				float l = dPos.Length();
				float dl = SRand::Inst().Rand( 200.0f, 250.0f );
				float fTargetl = SRand::Inst().Rand( Min( l + dl, Max( l - dl, 496.0f ) ), Min( l + dl, Max( l - dl, 560.0f ) ) );
				float fAngle = atan2( dPos.y, dPos.x );
				fAngle += SRand::Inst().Rand( 0.9f, 1.1f ) * 400.0f / ( fTargetl + 1.0f ) * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 );
				CVector2 p1( cos( fAngle ), sin( fAngle ) );
				p1 = p1 * fTargetl + playerPos;
				pBarrage->GetBulletContext( 0 )->MoveTowards( p1, 32 );
				CVector2 dir = playerPos - ( p + p1 ) * 0.5f;
				dir.Normalize();
				for( int i = 0; i < 32; i++ )
				{
					pBarrage->InitBullet( nBullet1 + i, 1, -1, ( p * ( 31 - i ) + p1 * i ) / 32, dir * SRand::Inst().Rand( 35.0f, 75.0f ), CVector2( 0, 0 ) );
					pBarrage->Yield( 1 );
				}

				pBarrage->GetBulletContext( 0 )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
				for( int i = 0; i < 32; i++ )
				{
					CVector2 v = pBarrage->GetBulletContext( nBullet1 + i )->v;
					pBarrage->GetBulletContext( nBullet1 + i )->SetBulletMove( v * 3, CVector2( 0, 0 ) );
					pBarrage->Yield( 1 );
				}

				p = p1;
				if( pBarrage->GetStage()->GetPlayer() )
					playerPos = pBarrage->GetStage()->GetPlayer()->GetPosition();
				dPos = playerPos - p;
				if( dPos.Normalize() < 0.01f )
					dPos = CVector2( 1, 0 );
				CVector2 dir1( -dPos.x + dPos.y, -dPos.y - dPos.x );
				CVector2 dir2( -dPos.x - dPos.y, -dPos.y + dPos.x );
				pBarrage->InitBullet( 1, 0, -1, p, dir1 * 384, CVector2( 0, 0 ), false );
				pBarrage->InitBullet( 2, 0, -1, p, dir2 * 384, CVector2( 0, 0 ), false );
				pBarrage->Yield( 16 );

				pBarrage->GetBulletContext( 1 )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
				pBarrage->GetBulletContext( 2 )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
				pBarrage->InitLightning( 0, 0, 1, 1, CVector2( 0, 0 ), dPos * 1024, false );
				pBarrage->InitLightning( 1, 0, 2, 2, CVector2( 0, 0 ), dPos * 1024, false );
				pBarrage->Yield( 16 );

				pBarrage->GetBulletContext( 0 )->SetBulletMove( dPos * 1024, CVector2( 0, 0 ) );
				for( int i = 0; i < 32; i++ )
				{
					pBarrage->InitBullet( nBullet2 + i, 1, -1, p + dPos * 32 * i, dir1 * 8, CVector2( 0, 0 ) );
					pBarrage->InitBullet( nBullet2 + 32 + i, 1, -1, p + dPos * 32 * i, dir2 * 8, CVector2( 0, 0 ) );
					pBarrage->Yield( 2 );
				}

				for( int i = 0; i < 32; i++ )
				{
					pBarrage->InitBullet( nBullet2 + i, 1, -1, p + dPos * 32 * i, dir1 * 128, CVector2( 0, 0 ) );
					pBarrage->InitBullet( nBullet2 + 32 + i, 1, -1, p + dPos * 32 * i, dir2 * 128, CVector2( 0, 0 ) );
					pBarrage->Yield( 2 );
				}
				pBarrage->GetBulletContext( 0 )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
				pBarrage->DestroyLightning( 0 );
				pBarrage->DestroyLightning( 1 );
				pBarrage->DestroyBullet( 1 );
				pBarrage->DestroyBullet( 2 );
				nBullet1 += 96;
				nBullet2 += 96;
				p = dPos * 1024 + p;
			}

			pBarrage->DestroyBullet( 0 );
			pBarrage->Yield( 2 );
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->Start();
		return 24;
	}
	}
	return 0;
}