#include "stdafx.h"
#include "LvBarriers.h"
#include "Stage.h"
#include "Player.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Entities/EffectObject.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "Pickup.h"
#include "GlobalCfg.h"
#include "Enemy.h"

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
		m_pChunk->fWeight = m_fWeights[m_nKilledCrates - 1];
	m_vecPickups[m_vecPickups.size() - 1 - i]->bVisible = true;
}

void CLvFloor1::OnPickUp()
{
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

	Kill();
}

void CLvFloor2::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto rect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;

	SetRenderObject( new CRenderObject2D );
	for( int i = 0; i < pChunk->nWidth; i += 2 )
	{
		CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImage2D->SetRect( CRectangle( i * 32, 0, 64, 64 ) );
		pImage2D->SetTexRect( texRect0 );
		GetRenderObject()->AddChild( pImage2D );
		for( int iX = i; iX < i + 2; iX++ )
		{
			for( int iY = 0; iY < 2; iY++ )
			{
				GetBlock( iX, iY )->rtTexRect = CRectangle( texRect0.x + iX * texRect0.width / 2, texRect0.y + iY * texRect0.height / 2,
					texRect0.width / 2, texRect0.height / 2 );
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
	m_bPicked = true;
	m_pChunk->fWeight = 1000000;
}

void CLvFloor2::OnTick()
{
	if( !m_bPicked )
	{
		uint32 nSpeeds[] = { 1, 2, 3, 4, 5 };
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

	if( y <= 0 )
	{
		Kill();
		return;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CLvFloor2::OnKilled()
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
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
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
				CMyLevel::GetInst()->KillChunk( pChunk );
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
						pBullet->SetParentEntity( CMyLevel::GetInst() );
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
					pBullet->SetParentEntity( CMyLevel::GetInst() );
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
						pBullet->SetParentEntity( CMyLevel::GetInst() );
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
