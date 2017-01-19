#include "stdafx.h"
#include "Block.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Common/ResourceManager.h"
#include "Entities/Barrage.h"
#include "Entities/EffectObject.h"
#include "Stage.h"

CBlockObject::CBlockObject( SBlock* pBlock, CEntity* pParent, CMyLevel* pLevel )
	: m_pBlock( pBlock )
{
	SET_BASEOBJECT_ID( CBlockObject );

	SetPosition( CVector2( pBlock->nX, pBlock->nY ) * pLevel->GetBlockSize() );
	if( pBlock->pBaseInfo->eBlockType != eBlockType_Wall )
	{
		AddRect( CRectangle( 0, 0, pLevel->GetBlockSize(), pLevel->GetBlockSize() ) );
		SetHitType( pBlock->pBaseInfo->eBlockType == eBlockType_Block ? eEntityHitType_WorldStatic : eEntityHitType_Platform );
	}
	SetParentEntity( pParent );
}

SChunk::SChunk( const SChunkBaseInfo& baseInfo, const TVector2<int32>& pos, const TVector2<int32>& size )
	: nWidth( size.x )
	, nHeight( size.y )
	, fWeight( baseInfo.fWeight + baseInfo.fWeightPerWidth * ( size.x - baseInfo.nWidth ) )
	, fDestroyWeight( baseInfo.fDestroyWeight + baseInfo.fDestroyWeightPerWidth * ( size.x - baseInfo.nWidth ) )
	, fDestroyBalance( baseInfo.fDestroyBalance )
	, fImbalanceTime( baseInfo.fImbalanceTime )
	, nAbsorbShakeStrength( baseInfo.nAbsorbShakeStrength + baseInfo.fAbsorbShakeStrengthPerHeight * ( size.y - baseInfo.nHeight ) )
	, nDestroyShake( baseInfo.nDestroyShake )
	, pos( pos )
	, nFallSpeed( 0 )
	, pPrefab( baseInfo.pPrefab )
	, pChunkObject( NULL )
	, nUpdateCount( 0 )
	, fAppliedWeight( 0 )
	, fBalance( 0 )
	, fCurImbalanceTime( 0 )
	, bIsRoom( baseInfo.bIsRoom )
	, bIsLevelBarrier( false )
	, bStopMove( false )
	, bForceStop( false )
	, bSpawned( false )
	, bIsSubChunk( false )
	, bMovedLastFrame( false )
	, bAbsorbedShake( false )
	, nVisitFlag( 0 )
	, m_pSubChunks( NULL )
	, m_pSpawnInfos( NULL )
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
			int x = i == size.x - 1 ? baseInfo.nWidth - 1 : Min<int32>( i, baseInfo.nWidth - 2 );
			int y = j == size.y - 1 ? baseInfo.nHeight - 1 : Min<int32>( j, baseInfo.nHeight - 2 );
			block.pBaseInfo = &baseInfo.blockInfos[x + y * baseInfo.nWidth];
		}
	}

	auto pChunkObject = baseInfo.pPrefab->GetRoot()->GetStaticData<CChunkObject>();
	if( pChunkObject )
		pChunkObject->OnChunkDataGenerated( this, baseInfo );
}

SChunk::~SChunk()
{
	while( Get_SpawnInfo() )
	{
		auto pSpawnInfo = Get_SpawnInfo();
		pSpawnInfo->RemoveFrom_SpawnInfo();
		delete pSpawnInfo;
	}
}

void SChunk::CreateChunkObject( CMyLevel* pLevel )
{
	if( pChunkObject )
		return;
	pChunkObject = SafeCast<CChunkObject>( pPrefab->GetRoot()->CreateInstance() );
	pChunkObject->SetChunk( this, pLevel );

	while( Get_SpawnInfo() )
	{
		auto pSpawnInfo = Get_SpawnInfo();
		auto pEntity = SafeCast<CEntity>( pSpawnInfo->pPrefab->GetRoot()->CreateInstance() );
		pEntity->SetPosition( pSpawnInfo->pos + pChunkObject->GetPosition() );
		pEntity->SetRotation( pSpawnInfo->r );
		pEntity->SetParentBeforeEntity( pChunkObject->GetParentEntity() );
		pSpawnInfo->RemoveFrom_SpawnInfo();
		delete pSpawnInfo;
	}
}

void CChunkObject::SetChunk( SChunk* pChunk, CMyLevel* pLevel )
{
	m_pChunk = pChunk;
	pChunk->bSpawned = true;
	pChunk->pChunkObject = this;
	SetPosition( CVector2( pChunk->pos.x, pChunk->pos.y ) );
	if( pChunk->bIsRoom )
	{
		AddRect( CRectangle( 0, 0, pChunk->nWidth * pLevel->GetBlockSize(), pChunk->nHeight * pLevel->GetBlockSize() ) );
		SetHitType( eEntityHitType_Sensor );
	}
	SetParentEntity( pLevel->GetChunkRoot() );

	if( m_strEffect.length() > 0 )
	{
		m_pEffect = CResourceManager::Inst()->CreateResource<CPrefab>( m_strEffect.c_str() );
	}

	for( auto& block : pChunk->blocks )
	{
		if( block.pBaseInfo->eBlockType != eBlockType_Wall )
		{
			block.pEntity = new CBlockObject( &block, this, pLevel );
		}
		if( block.pAttachedPrefab )
		{
			auto pEntity = SafeCast<CEntity>( block.pAttachedPrefab->GetRoot()->CreateInstance() );
			pEntity->SetPosition( CVector2( block.nX + block.attachedPrefabSize.x * 0.5f, block.nY + block.attachedPrefabSize.y * 0.5f ) * pLevel->GetBlockSize() );
			pEntity->SetZOrder( 1 );
			pEntity->SetParentEntity( this );
		}
	}

	if( m_pDamagedEffectsRoot )
	{
		for( auto pChild = m_pDamagedEffectsRoot->Get_Child(); pChild && m_nDamagedEffectsCount < 4; pChild = pChild->NextChild() )
		{
			pChild->bVisible = false;
			m_pDamagedEffects[m_nDamagedEffectsCount++] = pChild;
		}
	}
}

void CChunkObject::OnChunkDataGenerated( SChunk * pChunk, const SChunkBaseInfo& baseInfo ) const
{
	for( auto& item : baseInfo.subInfos )
	{
		pChunk->Insert_SubChunk( new SChunk( *item.first, item.second, TVector2<int32>( item.first->nWidth, item.first->nHeight ) ) );
	}
}

void CChunkObject::RemoveChunk()
{
	if( m_pChunk )
	{
		auto pChunk = m_pChunk;
		m_pChunk = NULL;
		pChunk->pChunkObject = NULL;
		CMyLevel::GetInst()->RemoveChunk( pChunk );
	}
}

void CChunkObject::OnKilled()
{
	if( m_pEffect )
	{
		auto pEffect = SafeCast<CEffectObject>( m_pEffect->GetRoot()->CreateInstance() );
		pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		pEffect->SetPosition( GetPosition() );
		pEffect->SetState( 2 );
	}
}

void CChunkObject::Damage( uint32 nDmg, uint8 nType )
{
	if( !m_nMaxHp )
		return;
	uint32 nLastDamageFrame = Max<int32>( ( m_nHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 );
	m_nHp = Max( m_nHp - (int32)nDmg, 0 );
	uint32 nDamageFrame = Max<int32>( ( m_nHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 );
	if( nLastDamageFrame != nDamageFrame )
	{
		if( nLastDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nLastDamageFrame]->bVisible = false;
		if( nDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nDamageFrame]->bVisible = true;
	}

	if( m_nHp <= 0 )
	{
		m_nHp = 0;
		if( nType == 1 )
			Crush();
		else
			Kill();
	}
}

uint32 CChunkObject::Repair( uint32 nAmount )
{
	uint32 nLastDamageFrame = Max<int32>( ( m_nHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 );
	int32 nLastHp = m_nHp;
	m_nHp = Min( m_nHp + (int32)nAmount, m_nMaxHp );
	uint32 nDamageFrame = Max<int32>( ( m_nHp * ( m_nDamagedEffectsCount + 1 ) - 1 ) / m_nMaxHp, 0 );
	if( nLastDamageFrame != nDamageFrame )
	{
		if( nLastDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nLastDamageFrame]->bVisible = false;
		if( nDamageFrame < m_nDamagedEffectsCount )
			m_pDamagedEffects[nDamageFrame]->bVisible = true;
	}
	return m_nHp - nLastHp;
}

void CChunkObject::Kill()
{
	OnKilled();
	CMyLevel::GetInst()->pHitSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );

	auto pChunk = m_pChunk;
	m_pChunk = NULL;
	pChunk->pChunkObject = NULL;
	CMyLevel::GetInst()->KillChunk( pChunk );
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
	MoveToTopmost();

	if( !m_onHitShakeTick.IsRegistered() )
		GetStage()->RegisterBeforeHitTest( 1, &m_onHitShakeTick );
}

void CChunkObject::ClearHitShake()
{
	if( m_onHitShakeTick.IsRegistered() )
		m_onHitShakeTick.Unregister();
	m_hitShakeVector = CVector2( 0, 0 );
	HandleHitShake( CVector2( 0, 0 ) );
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

	CVector2 pos = m_hitShakeVector * cos( m_nHitShakeFrame * 1.275235 ) + CVector2( m_hitShakeVector.y, -m_hitShakeVector.x ) * sin( m_nHitShakeFrame * 1.58792 );
	pos = CVector2( floor( pos.x + 0.5f ), floor( pos.y + 0.5f ) );
	HandleHitShake( pos );

	m_nHitShakeFrame++;
	GetStage()->RegisterBeforeHitTest( 1, &m_onHitShakeTick );
}

void CChunkObject::HandleHitShake( const CVector2& ofs )
{
	if( GetRenderObject() )
		GetRenderObject()->SetPosition( ofs );
	if( m_p1 )
		m_p1->SetPosition( ofs );
	if( m_pDamagedEffectsRoot )
		m_pDamagedEffectsRoot->SetPosition( ofs );
}

void CSpecialChunk::OnAddedToStage()
{
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	CChunkObject::OnAddedToStage();
}

void CSpecialChunk::Trigger()
{
	CMyLevel::GetInst()->pExpSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CSpecialChunk1::Trigger()
{
	CSpecialChunk::Trigger();
	SBarrageContext context;
	context.pCreator = GetBlock( 0, 0 )->pEntity;
	context.vecBulletTypes.push_back( m_pBulletPrefab );
	context.nBulletPageSize = 8;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [this] ( CBarrage* pBarrage )
	{
		for( int i = 0; i < 4; i++ )
		{
			pBarrage->InitBullet( i * 2, 0, -1, CVector2( 32, i * 8 + 4 ), CVector2( 400, 0 ), CVector2( 0, 0 ), true );
			pBarrage->InitBullet( i * 2 + 1, 0, -1, CVector2( 0, i * 8 + 4 ), CVector2( -400, 0 ), CVector2( 0, 0 ), true );
			pBarrage->Yield( 5 );
		}
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( GetPosition() );
	pBarrage->Start();
}

void CSpecialChunk2::Trigger()
{
	CSpecialChunk::Trigger();
	SBarrageContext context;
	context.pCreator = GetBlock( 0, 0 )->pEntity;
	context.vecBulletTypes.push_back( m_pBulletPrefab );
	context.nBulletPageSize = 12;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( []( CBarrage* pBarrage )
	{
		CVector2 ofs[4] = { { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };
		CVector2 ofs1[12] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 0, 1 }, { 0, 0 }, { 1, 1 }, { 1, 0 }, { 1, 1 }, { 0, 0 }, { 1, 1 }, { 1, 0 }, { 0, 0 } };
		for( int i = 0; i < 12; i++ )
		{
			pBarrage->InitBullet( i, 0, -1, ofs1[i] * 32, ofs[i / 3] * 300, CVector2( 0, 0 ), true );
		}
		pBarrage->Yield( 1 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( GetPosition() );
	pBarrage->Start();
}

void CSpecialChunk3::Trigger()
{
	CSpecialChunk::Trigger();
	SBarrageContext context;
	context.pCreator = GetBlock( 0, 0 )->pEntity;
	context.vecBulletTypes.push_back( m_pBulletPrefab );
	context.nBulletPageSize = 12;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( []( CBarrage* pBarrage )
	{
		for( int i = 0; i < 12; i++ )
		{
			float fAngle = i * PI / 6;
			pBarrage->InitBullet( i, 0, -1, CVector2( 16, 16 ), CVector2( cos( fAngle ), sin( fAngle ) ) * 400, CVector2( 0, 0 ), true );
			pBarrage->Yield( 2 );
		}
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( GetPosition() );
	pBarrage->Start();
}

void CExplosiveChunk::Damage( uint32 nDmg, uint8 nType )
{
	if( m_bKilled )
	{
		m_nHp -= nDmg;
		if( -m_nHp >= m_nMaxKillHp )
			Explode();
		return;
	}
	else
		CChunkObject::Damage( nDmg, nType );
}

void CExplosiveChunk::Kill()
{
	if( m_bKilled )
		return;
	m_bKilled = true;
	m_nHp = 0;
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
			CMyLevel::GetInst()->pExpSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
			CVector2 center = CVector2( SRand::Inst().Rand( 0u, m_pChunk->nWidth * CMyLevel::GetInst()->GetBlockSize() ), SRand::Inst().Rand( 0u, m_pChunk->nHeight * CMyLevel::GetInst()->GetBlockSize() ) );
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
		Damage( m_nDeathDamage );
	}
}

void CBarrel::OnAddedToStage()
{
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBulletPrefab1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );
	CExplosiveChunk::OnAddedToStage();
}

void CBarrel::Damage( uint32 nDmg, uint8 nType )
{
	if( m_bKilled )
	{
		CReference<CBarrel> temp = this;
		CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetInst()->GetBlockSize() * 0.5f;
		int32 nPreHp = -m_nHp;
		CExplosiveChunk::Damage( nDmg );
		if( !GetParentEntity() )
			return;
		int32 nCurHp = -m_nHp;
		int32 nFireCount = ( nCurHp / m_nDeathDamage ) - ( nPreHp / m_nDeathDamage );
		if( nFireCount > 0 )
		{
			CMyLevel::GetInst()->AddShakeStrength( 10 );
			SBarrageContext context;
			context.pCreator = this;
			context.vecBulletTypes.push_back( m_pBulletPrefab );
			context.vecBulletTypes.push_back( m_pBulletPrefab1 );
			context.nBulletPageSize = 12 * nFireCount;

			CBarrage* pBarrage = new CBarrage( context );
			pBarrage->AddFunc( [nFireCount]( CBarrage* pBarrage )
			{
				for( int iFire = 0; iFire < nFireCount; iFire++ )
				{
					for( int i = 0; i < 12; i++ )
					{
						float fAngle = ( i + SRand::Inst().Rand( -0.5f, 0.5f ) ) * PI / 6;
						CVector2 dir( cos( fAngle ), sin( fAngle ) );
						float speed = SRand::Inst().Rand( 300.0f, 500.0f );
						uint32 nBullet = iFire * 12 + i;
						pBarrage->InitBullet( nBullet, SRand::Inst().Rand() & 1, -1, CVector2( 0, 0 ), dir * speed, dir * speed * -0.5f, true );
						CVector2 finalSpeed = dir * speed * 0.5f;
						pBarrage->AddDelayAction( 60, [=]() {
							auto pContext = pBarrage->GetBulletContext( nBullet );
							if( pContext )
								pContext->SetBulletMove( finalSpeed, CVector2( 0, 0 ) );
						} );
					}
					pBarrage->Yield( 5 );
				}
				pBarrage->StopNewBullet();
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( GetPosition() + center );
			pBarrage->Start();
		}
	}
	else
		CExplosiveChunk::Damage( nDmg );
}

void CBarrel::Explode()
{
	CMyLevel::GetInst()->pExpSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
	CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetInst()->GetBlockSize() * 0.5f;

	if( m_pEffect )
	{
		auto pEffect = SafeCast<CEffectObject>( m_pEffect->GetRoot()->CreateInstance() );
		pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		pEffect->SetPosition( GetPosition() + center );
		pEffect->SetState( 2 );
	}

	SBarrageContext context;
	context.pCreator = this;
	context.vecBulletTypes.push_back( m_pBulletPrefab );
	context.vecBulletTypes.push_back( m_pBulletPrefab1 );
	context.nBulletPageSize = 12 * 5;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( []( CBarrage* pBarrage )
	{
		for( int i = 0; i < 12; i++ )
		{
			float fAngle = ( i + SRand::Inst().Rand( -0.5f, 0.5f ) ) * PI / 6;
			float speed = SRand::Inst().Rand( 300.0f, 500.0f );
			for( int j = 0; j < 5; j++ )
			{
				float fAngle1 = fAngle + ( j - 2 ) * 0.08f;
				CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
				float fSpeed1 = 400 - abs( j - 2 ) * 25;
				pBarrage->InitBullet( i * 5 + j, SRand::Inst().Rand() & 1, -1, CVector2( 0, 0 ), dir * fSpeed1, CVector2( 0, 0 ), true );
			}
		}
		pBarrage->Yield( 1 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( GetPosition() + center );
	pBarrage->Start();

	CExplosiveChunk::Explode();
}

void CRandomEnemyRoom::SetChunk( SChunk* pChunk, class CMyLevel* pLevel )
{
	CChunkObject::SetChunk( pChunk, pLevel );

	CDrawableGroup* pDrawableGroup = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strRes.c_str() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle damageEftTexRects[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		damageEftTexRects[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}
	SetRenderObject( new CRenderObject2D );
	for( int i = 0; i < pChunk->nWidth; i++ )
	{
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			uint32 nTileX, nTileY;
			if( i == 0 && j == 0 )
			{
				nTileX = 10;
				nTileY = 5;
			}
			else if( i == 1 && j == 0 )
			{
				nTileX = 11;
				nTileY = 5;
			}
			else if( i == 0 && j == 1 )
			{
				nTileX = 10;
				nTileY = 4;
			}
			else if( i == pChunk->nWidth - 1 && j == 0 )
			{
				nTileX = 13;
				nTileY = 5;
			}
			else if( i == pChunk->nWidth - 2 && j == 0 )
			{
				nTileX = 12;
				nTileY = 5;
			}
			else if( i == pChunk->nWidth - 1 && j == 1 )
			{
				nTileX = 13;
				nTileY = 4;
			}

			else if( i == 0 && j == pChunk->nHeight - 1 )
			{
				nTileX = 10;
				nTileY = 2;
			}
			else if( i == 1 && j == pChunk->nHeight - 1 )
			{
				nTileX = 11;
				nTileY = 2;
			}
			else if( i == 0 && j == pChunk->nHeight - 2 )
			{
				nTileX = 10;
				nTileY = 3;
			}
			else if( i == pChunk->nWidth - 1 && j == pChunk->nHeight - 1 )
			{
				nTileX = 13;
				nTileY = 2;
			}
			else if( i == pChunk->nWidth - 2 && j == pChunk->nHeight - 1 )
			{
				nTileX = 12;
				nTileY = 2;
			}
			else if( i == pChunk->nWidth - 1 && j == pChunk->nHeight - 2 )
			{
				nTileX = 13;
				nTileY = 3;
			}

			else if( i == 0 )
			{
				nTileX = 14 + ( SRand::Inst().Rand() & 1 );
				nTileY = 3;
			}
			else if( i == pChunk->nWidth - 1 )
			{
				nTileX = 14 + ( SRand::Inst().Rand() & 1 );
				nTileY = 5;
			}
			else if( j == 0 )
			{
				nTileX = 14 + ( SRand::Inst().Rand() & 1 );
				nTileY = 4;
			}
			else if( j == pChunk->nHeight - 1 )
			{
				nTileX = 14 + ( SRand::Inst().Rand() & 1 );
				nTileY = 2;
			}
			else
			{
				nTileX = 11 + ( SRand::Inst().Rand() & 1 );
				nTileY = 3 + ( SRand::Inst().Rand() & 1 );
			}

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );
			pImage2D->SetTexRect( CRectangle( nTileX / 16.0f, nTileY / 16.0f, 1 / 16.0f, 1 / 16.0f ) );
			GetRenderObject()->AddChild( pImage2D );
			
			if( i == 0 || i == pChunk->nWidth - 1 || j == 0 || j == pChunk->nHeight - 1 )
			{
				for( int k = 0; k < m_nDamagedEffectsCount; k++ )
				{
					CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[k]->CreateInstance() );
					pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );
					pImage2D->SetTexRect( damageEftTexRects[k] );
					m_pDamagedEffects[k]->AddChild( pImage2D );
				}
			}
		}
	}
}

void CRandomEnemyRoom::OnKilled()
{
	if( m_pEffect )
	{
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_pEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetInst()->GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}

void CRandomChunk::OnChunkDataGenerated( SChunk * pChunk, const SChunkBaseInfo & baseInfo ) const
{
	int32 nMaxX = 0, nMaxY = 0;
	for( int i = 0; i < baseInfo.subInfos.size(); i++ )
	{
		nMaxX = Max( nMaxX, baseInfo.subInfos[i].second.x );
		nMaxY = Max( nMaxY, baseInfo.subInfos[i].second.y );
	}
	nMaxX++;
	nMaxY++;
	int32 nTilex0 = 0, nTilex1 = 0, nTiley0 = 0, nTiley1 = 0;
	int32 nTile0 = 0;
	vector<uint32> tiles;
#define GET_TILE_INDEX( x, y ) ( tiles[(x) + (y) * nMaxX] )
	tiles.resize( nMaxX * nMaxY );
	for( int i = 0; i < baseInfo.subInfos.size(); i++ )
	{
		auto& pos = baseInfo.subInfos[i].second;
		if( pos.x == 0 && pos.y == 0 )
			GET_TILE_INDEX( 0, 0 ) = i;
		else if( pos.x == nMaxX - 1 && pos.y == 0 )
			GET_TILE_INDEX( nMaxX - 1, 0 ) = i;
		else if( pos.x == 0 && pos.y == nMaxY - 1 )
			GET_TILE_INDEX( 0, nMaxY - 1 ) = i;
		else if( pos.x == nMaxX - 1 && pos.y == nMaxY - 1 )
			GET_TILE_INDEX( nMaxX - 1, nMaxY - 1 ) = i;
		else if( pos.x == 0 )
		{
			if( nTilex0 < nMaxY - 2 )
				GET_TILE_INDEX( 0, nTilex0 + 1 ) = i;
			nTilex0++;
		}
		else if( pos.x == nMaxX - 1 )
		{
			if( nTilex1 < nMaxY - 2 )
				GET_TILE_INDEX( nMaxX - 1, nTilex1 + 1 ) = i;
			nTilex1++;
		}
		else if( pos.y == 0 )
		{
			if( nTiley0 < nMaxX - 2 )
				GET_TILE_INDEX( nTiley0 + 1, 0 ) = i;
			nTiley0++;
		}
		else if( pos.y == nMaxY - 1 )
		{
			if( nTiley1 < nMaxX - 2 )
				GET_TILE_INDEX( nTiley1 + 1, nMaxY - 1 ) = i;
			nTiley1++;
		}
		else
		{
			if( nTile0 < ( nMaxX - 2 ) * ( nMaxY - 2 ) )
				GET_TILE_INDEX( nTile0 % ( nMaxX - 2 ) + 1, nTile0 / ( nMaxX - 2 ) + 1 ) = i;
			nTile0++;
		}
	}

	uint32 nWidth = pChunk->nWidth;
	uint32 nHeight = pChunk->nHeight;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			uint32 nIndex;
			if( i == 0 && j == 0 )
				nIndex = GET_TILE_INDEX( 0, 0 );
			else if( i == nWidth - 1 && j == 0 )
				nIndex = GET_TILE_INDEX( nMaxX - 1, 0 );
			else if( i == 0 && j == nHeight - 1 )
				nIndex = GET_TILE_INDEX( 0, nMaxY - 1 );
			else if( i == nWidth - 1 && j == nHeight - 1 )
				nIndex = GET_TILE_INDEX( nMaxX - 1, nMaxY - 1 );
			else if( i == 0 )
			{
				nIndex = GET_TILE_INDEX( 0, SRand::Inst().Rand( 0, nTilex0 ) + 1 );
			}
			else if( i == nWidth - 1 )
			{
				nIndex = GET_TILE_INDEX( nMaxX - 1, SRand::Inst().Rand( 0, nTilex1 ) + 1 );
			}
			else if( j == 0 )
			{
				nIndex = GET_TILE_INDEX( SRand::Inst().Rand( 0, nTiley0 ) + 1, 0 );
			}
			else if( j == nHeight - 1 )
			{
				nIndex = GET_TILE_INDEX( SRand::Inst().Rand( 0, nTiley1 ) + 1, nMaxY - 1 );
			}
			else
			{
				int32 nRand = SRand::Inst().Rand( 0, nTile0 );
				nIndex = GET_TILE_INDEX( nRand % ( nMaxX - 2 ) + 1, nRand / ( nMaxX - 2 ) + 1 );
			}

			pChunk->Insert_SubChunk( new SChunk( *baseInfo.subInfos[nIndex].first, TVector2<int32>( i, j ), TVector2<int32>( baseInfo.subInfos[nIndex].first->nWidth, baseInfo.subInfos[nIndex].first->nHeight ) ) );
		}
	}
}

void CRandomChunk::SetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CChunkObject::SetChunk( pChunk, pLevel );

	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}

	SetRenderObject( new CRenderObject2D );
	for( auto pSubChunk = pChunk->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
	{
		auto texRect = static_cast<CImage2D*>( pSubChunk->pPrefab->GetRoot()->GetRenderObject() )->GetElem().texRect;

		CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImage2D->SetRect( CRectangle( pSubChunk->pos.x * 32, pSubChunk->pos.y * 32, 32, 32 ) );
		pImage2D->SetTexRect( ( texRect * m_texScale ).Offset( m_texOfs ) );
		GetRenderObject()->AddChild( pImage2D );

		for( int i = 0; i < m_nDamagedEffectsCount; i++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
			pImage2D->SetRect( CRectangle( pSubChunk->pos.x * 32, pSubChunk->pos.y * 32, 32, 32 ) );
			pImage2D->SetTexRect( ( texRect * m_dmgTexScale[m_nDamagedEffectsCount - i - 1] ).Offset( m_dmgTexOfs[m_nDamagedEffectsCount - i - 1] ) );
			m_pDamagedEffects[i]->AddChild( pImage2D );
		}
	}
}

void CRandomChunk::OnKilled()
{
	if( m_pEffect )
	{
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_pEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetInst()->GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}

void CCharacterChunk::Crush()
{
	ClearHitShake();
	if( m_pCharacter )
	{
		m_pCharacter->globalTransform.Decompose( m_pCharacter->x, m_pCharacter->y, m_pCharacter->r, m_pCharacter->s );
		m_pCharacter->SetParentEntity( GetParentEntity() );
		auto pChar = SafeCast<CCharacter>( m_pCharacter.GetPtr() );
		if( pChar )
			pChar->Kill();

		m_pCharacter = NULL;
	}
	Kill();
}

void CCharacterChunk::OnKilled()
{
	CChunkObject::OnKilled();

	ClearHitShake();
	if( m_pCharacter )
	{
		m_pCharacter->globalTransform.Decompose( m_pCharacter->x, m_pCharacter->y, m_pCharacter->r, m_pCharacter->s );
		m_pCharacter->SetParentEntity( GetParentEntity() );
		auto pChar = SafeCast<CCharacter>( m_pCharacter.GetPtr() );
		if( pChar )
			pChar->Awake();

		m_pCharacter = NULL;
	}
}
