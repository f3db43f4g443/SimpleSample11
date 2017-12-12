#include "stdafx.h"
#include "BlockItemsLv2.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Barrage.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Common/ResourceManager.h"
#include "Entities/Bullets.h"
#include "Entities/Enemies/Lv2Enemies.h"
#include "Entities/Blocks/Lv2/SpecialLv2.h"

void CCarSpawner::Trigger()
{
	uint8 nCar;
	for( nCar = 0; nCar < 4; nCar++ )
	{
		if( m_nSpawnCounts[nCar] )
			break;
	}
	if( nCar >= 4 )
		return;

	auto rect1 = m_carRect1;
	SHitProxyPolygon polygon;
	polygon.nVertices = 4;
	polygon.vertices[0] = CVector2( rect1.x, rect1.y );
	polygon.vertices[1] = CVector2( rect1.x + rect1.width, rect1.y );
	polygon.vertices[2] = CVector2( rect1.x + rect1.width, rect1.y + rect1.height );
	polygon.vertices[3] = CVector2( rect1.x, rect1.y + rect1.height );
	polygon.CalcNormals();

	vector<CReference<CEntity> > hitEntities;
	GetStage()->MultiHitTest( &polygon, globalTransform, hitEntities );
	bool bHit = false;
	for( CEntity* pEntity : hitEntities )
	{
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform )
		{
			bHit = true;
			break;
		}
	}
	if( bHit )
		return;

	CRectangle rect = m_carRect.Offset( GetPosition() );
	TRectangle<int32> excludeRect;
	excludeRect.x = floor( rect.x / CMyLevel::GetBlockSize() );
	excludeRect.y = floor( rect.y / CMyLevel::GetBlockSize() );
	excludeRect.width = ceil( rect.GetRight() / CMyLevel::GetBlockSize() ) - excludeRect.x;
	excludeRect.height = ceil( rect.GetBottom() / CMyLevel::GetBlockSize() ) - excludeRect.y;
	auto pCar = SafeCast<CCar>( m_pCarPrefabs[nCar]->GetRoot()->CreateInstance() );
	pCar->SetExcludeChunk( SafeCast<CChunkObject>( GetParentEntity() ), excludeRect );
	pCar->SetPosition( globalTransform.GetPosition() );
	pCar->SetVelocity( m_spawnVel );
	pCar->SetRotation( atan2( m_spawnVel.y, m_spawnVel.x ) );
	pCar->SetParentAfterEntity( CMyLevel::GetInst()->GetChunkRoot1() );
	m_nSpawnCounts[nCar]--;
	m_detectRect = m_detectRect1 = CRectangle( -10000, -10000, 20000, 20000 );
}

bool CCarSpawner::CheckTrigger()
{
	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunkObject->GetHp() < pChunkObject->GetMaxHp() )
		m_detectRect = m_detectRect1 = CRectangle( -10000, -10000, 20000, 20000 );
	return pChunkObject->GetChunk()->nFallSpeed < 10;
}

bool CHouseEntrance::CanEnter( CCharacter * pCharacter )
{
	auto pHouse = SafeCast<CHouse>( GetParentEntity() );
	if( !pHouse )
		pHouse = SafeCast<CHouse>( GetParentEntity()->GetParentEntity() );
	if( !pHouse )
		return false;
	return pHouse->CanEnter( pCharacter );
}

bool CHouseEntrance::Enter( CCharacter * pCharacter )
{
	auto pHouse = SafeCast<CHouse>( GetParentEntity() );
	if( !pHouse )
		pHouse = SafeCast<CHouse>( GetParentEntity()->GetParentEntity() );
	if( !pHouse )
		return false;
	return pHouse->Enter( pCharacter );
}

bool CHouseEntrance::Exit( CCharacter * pCharacter )
{
	if( globalTransform.GetPosition().y >= CMyLevel::GetInst()->GetBoundWithLvBarrier().GetBottom() - 32 )
		return false;

	auto rect1 = m_spawnRect1;
	SHitProxyPolygon polygon;
	polygon.nVertices = 4;
	polygon.vertices[0] = CVector2( rect1.x, rect1.y );
	polygon.vertices[1] = CVector2( rect1.x + rect1.width, rect1.y );
	polygon.vertices[2] = CVector2( rect1.x + rect1.width, rect1.y + rect1.height );
	polygon.vertices[3] = CVector2( rect1.x, rect1.y + rect1.height );
	polygon.CalcNormals();
	vector<CReference<CEntity> > hitEntities;
	GetStage()->MultiHitTest( &polygon, globalTransform, hitEntities );
	bool bHit = false;
	for( CEntity* pEntity : hitEntities )
	{
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform )
		{
			bHit = true;
			break;
		}
	}
	if( bHit )
		return false;

	pCharacter->SetPosition( globalTransform.GetPosition() + CVector2( m_spawnRect.x + SRand::Inst().Rand( 0.0f, m_spawnRect.width ), m_spawnRect.y + SRand::Inst().Rand( 0.0f, m_spawnRect.height ) ) );
	pCharacter->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
	return true;
}

void CThruster::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CThruster::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CThruster::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );

	m_bEnabled = m_bEnabled || CheckEnabled();
	if( m_bEnabled )
	{
		if( !m_pLightning )
		{
			m_pLightning = SafeCast<CLightning>( m_pEftPrefab->GetRoot()->CreateInstance() );
			m_pLightning->Set( NULL, NULL, CVector2( 0, 0 ), CVector2( 0, -m_fMaxEftHeight ), -1, -1 );
			m_pLightning->SetParentEntity( this );
		}
	}
	else
	{
		if( m_pLightning )
		{
			m_pLightning->SetParentEntity( NULL );
			m_pLightning = NULL;
		}
	}
}

bool CThruster::CheckEnabled()
{
	if( !m_nDuration )
		return false;
	auto pParent = SafeCast<CChunkObject>( GetParentEntity() );
	if( !pParent )
		return false;
	if( pParent->y > m_fEnableHeight )
		return false;
	if( pParent->GetChunk()->nFallSpeed < m_nEnableSpeed )
		return false;
	
	pParent->GetChunk()->bForceStop = true;
	m_nDuration--;
	return true;
}

void COperateableTurret1::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, 1, 24 );
	auto pLevel = CMyLevel::GetInst();
	if( pLevel )
		SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
}

void COperateableTurret1::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CEnemy::OnRemovedFromStage();
}

int8 COperateableTurret1::IsOperateable( const CVector2& pos )
{
	if( m_onTick.IsRegistered() )
		return 2;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer || !m_pDetectArea->HitTest( pPlayer->GetPosition() ) )
		return 1;
	return 0;
}

void COperateableTurret1::Operate( const CVector2& pos )
{
	m_nAmmoLeft = m_nAmmoCount;
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 1, 7, 24 );
	OnTick();
}

void COperateableTurret1::OnTick()
{
	if( !m_nAmmoLeft )
	{
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, 1, 24 );
		return;
	}

	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	for( int i = 0; i < m_nBulletCount; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
		pBullet->SetPosition( globalTransform.GetPosition() );
		float fAngle = r + ( i - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
		pBullet->SetRotation( fAngle );
		pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * m_fBulletSpeed );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		if( pChunkObject )
			pBullet->SetCreator( pChunkObject );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );
	m_nAmmoLeft--;
	GetStage()->RegisterAfterHitTest( m_nAmmoLeft ? m_nFireInterval : m_nFireCD, &m_onTick );
}

void CWindow3::OnAddedToStage()
{
	for( int i = 0; i < ELEM_COUNT( m_pParts ); i++ )
	{
		if( m_pParts[i] )
		{
			m_pParts[i]->bVisible = false;
			m_pParts[i]->SetTransparentRec( true );
		}
	}
}

void CWindow3::Kill()
{
	if( m_bKilled || !m_pAI || !m_pAI->IsRunning() )
		return;
	m_pAI->Throw( (uint32)0 );
}

CAIObject* CWindow3::TryPlay()
{
	if( m_pAI && !m_pAI->IsRunning() )
	{
		m_pAI->SetParentEntity( NULL );
		m_pAI = NULL;
	}
	if( m_pAI )
		return NULL;
	if( globalTransform.GetPosition().y > 32 * 18 )
		return NULL;
	auto pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return NULL;

	int32 nIndex[4] = { 0, 1, 2, 3 };
	SRand::Inst().Shuffle( nIndex, 4 );
	for( int i = 0; i < 4; i++ )
	{
		if( !m_pDetectArea[nIndex[i]] )
			continue;
		if( !m_pDetectArea[nIndex[i]]->HitTest( pPlayer->GetPosition() ) )
			continue;

		m_nDir = nIndex[i];
		m_pAI = new AI();
		m_pAI->SetParentEntity( this );
		return m_pAI;
	}
	return NULL;
}

void CWindow3::AIFunc()
{
	auto pEnemyPart = m_pParts[m_nDir];
	pEnemyPart->bVisible = true;
	auto pImg = static_cast<CMultiFrameImage2D*>( pEnemyPart->GetRenderObject() );
	try
	{
		pImg->SetFrames( 0, 4, 8 );
		pImg->SetPlaySpeed( 1, false );
		m_pAI->Yield( 0.5f, false );
		pEnemyPart->SetTransparentRec( false );
		m_pAI->Yield( 1.0f, false );

		if( m_nDir == 0 || m_nDir == 2 )
		{
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
			context.vecBulletTypes.push_back( m_pBullet2.GetPtr() );
			context.vecBulletTypes.push_back( m_pBullet3.GetPtr() );
			context.nBulletPageSize = 90;

			CBarrage* pBarrage = new CBarrage( context );
			CVector2 p = globalTransform.MulVector2Pos( m_fireOfs[m_nDir] );
			CVector2 dirs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			CVector2 dir = globalTransform.MulVector2Dir( dirs[m_nDir] );
			pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
			{
				auto pPlayer = GetStage()->GetPlayer();
				CVector2 playerPos = pPlayer ? pPlayer->GetPosition() : p + dir * 200;
				int32 nBullet = 0;
				CVector2 dirs[5];
				int32 r = SRand::Inst().Rand( 0, 2 );
				float v0 = 30;
				for( int i = 0; i < 5; i++ )
				{
					float fAngle = ( i - 2 ) * 0.5f;
					dirs[i] = CVector2( cos( fAngle ), sin( fAngle ) );
					dirs[i] = CVector2( dir.x * dirs[i].x - dir.y * dirs[i].y, dir.x * dirs[i].y + dir.y * dirs[i].x );
					pBarrage->InitBullet( nBullet++, 0, -1, p, dirs[i] * v0, CVector2( 0, 0 ), false );
				}
				pBarrage->Yield( 80 );
				for( int i = 1; i <= 3; i++ )
				{
					float fWeight = ( 3.5f - i ) / 3.5f;
					for( int j = 0; j < 5; j++ )
					{
						for( int k = 0; k < 5; k++ )
						{
							CVector2 p1 = p + dirs[k] * ( v0 * ( 80 * i + 7 * j ) / 60 );
							pBarrage->InitBullet( nBullet++, 2, -1, p1, dirs[k] * ( 200 - 15 * j ), CVector2( 0, 0 ) );
							if( j > 0 )
							{
								CVector2 d = playerPos - p1;
								if( d.Normalize() < 0.01f )
									d = dir;
								CVector2 dir0;
								dir0.Slerp( j * 0.2f, dirs[k], d );
								pBarrage->InitBullet( nBullet++, 2, -1, p1, dir0 * ( 200 - 10 * j ), CVector2( 0, 0 ) );
							}
						}
						pBarrage->Yield( 7 );
					}

					for( int j = 0; j < 9; j++ )
					{
						int j1 = j > 4 ? 8 - j : j;
						if( r )
							j1 = 4 - j1;
						for( int k = 0; k < 5; k++ )
						{
							CVector2 p1 = p + dirs[k] * ( v0 * ( 80 * i + 35 + j * 5 ) / 60 );
							CVector2 d = playerPos - p1;
							if( d.Normalize() < 0.01f )
								d = dir;
							CVector2 dir0;
							dir0.Slerp( fWeight, dirs[j1], d );
							pBarrage->InitBullet( nBullet++, 1, -1, p1, dir0 * ( 120 + j * 20 ), CVector2( 0, 0 ) );
						}
						pBarrage->Yield( 5 );
					}
					r = !r;

					if( pPlayer )
						playerPos = pPlayer->GetPosition();
				}

				pBarrage->Yield( 5 );
				for( int i = 0; i < 5; i++ )
					pBarrage->DestroyBullet( i );
				pBarrage->StopNewBullet();
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->Start();
		}
		else
		{
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
			context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
			context.vecBulletTypes.push_back( m_pBullet3.GetPtr() );
			context.vecLightningTypes.push_back( m_pBeam.GetPtr() );
			context.nBulletPageSize = 240;
			context.nLightningPageSize = 1;

			CBarrage* pBarrage = new CBarrage( context );
			CVector2 p = globalTransform.MulVector2Pos( m_fireOfs[m_nDir] );
			CVector2 dirs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			CVector2 dir = globalTransform.MulVector2Dir( dirs[m_nDir] );
			pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
			{
				pBarrage->InitBullet( 0, 0, -1, p, CVector2( 0, 0 ), CVector2( 0, 0 ), false );

				auto pPlayer = GetStage()->GetPlayer();
				CVector2 d = pPlayer ? pPlayer->GetPosition() : p;
				d = d - p;
				d.y = Min( -abs( d.x ) * 3.0f, d.y );
				float l = d.Normalize();
				if( l < 0.01f )
					d = dir;
				l = Max( 384.0f, Min( l + 256.0f, 768.0f ) );
				CVector2 dPos = d * l;
				if( dPos.y + p.y < 0 )
					dPos = dPos * ( p.y / -dPos.y );
				CVector2 dPos0 = d * 64.0f;

				CVector2 v0[6] = { { -600, -300 }, { -600, 300 }, { -100, -800 }, { -100, 800 }, { 500, 500 }, { 500, -500 } };
				CVector2 a[6];
				float t = 1.0f;
				for( int i = 0; i < 6; i++ )
				{
					v0[i] = CVector2( v0[i].x * d.x - v0[i].y * d.y, v0[i].x * d.y + v0[i].y * d.x );
					a[i] = ( dPos0 - v0[i] * t ) / ( t * t * 0.5f );
				}
				int32 nB = 1 + 12 * 6;
				float fAngle0 = SRand::Inst().Rand( -PI, PI );
				float dAngle = SRand::Inst().Rand( PI / 6, PI / 4 );
				int8 r = SRand::Inst().Rand( 0, 2 ) ? 1 : -1;
				for( int i = 0; i < 60; i++ )
				{
					for( int j = 0; j < 6; j++ )
					{
						pBarrage->InitBullet( 1 + ( i % 12 ) * 6 + j, 1, -1, p, v0[j], a[j] );
					}

					if( i == 18 )
					{
						pBarrage->InitLightning( 0, 0, 0, 0, CVector2( 0, 0 ), dPos, false );
					}
					else if( i >= 24 )
					{
						int32 i1 = i - 24;
						SLightningContext* pContext = pBarrage->GetLightningContext( 0 );
						if( pContext && pContext->pEntity )
						{
							CVector2 p1 = SafeCast<CLightning>( pContext->pEntity.GetPtr() )->GetBeamEnd();
							float fAngle = ( i1 / 9 ) * PI / 6;
							fAngle += ( i1 % 3 ) * 0.25f;
							fAngle += ( i1 % 9 ) / 3 * 0.06f;
							fAngle *= r;
							fAngle += fAngle0;
							for( int j = 0; j < 6; j++ )
							{
								float fAngle1 = fAngle + j * PI / 3;
								pBarrage->InitBullet( nB++, 2, -1, p1, CVector2( cos( fAngle1 ), sin( fAngle1 ) )
									* ( 160.0f + 30 * ( i1 % 3 ) - 10 * ( ( i1 % 9 ) / 3 ) ), CVector2( 0, 0 ) );
							}
						}
						else
						{
							*(int32*)0 = 0;
						}
					}
					
					pBarrage->Yield( 5 );
				}

				pBarrage->DestroyLightning( 0 );
				for( int i = 0; i < 12; i++ )
				{
					for( int j = 0; j < 6; j++ )
					{
						pBarrage->DestroyBullet( 1 + ( i % 12 ) * 6 + j );
					}
					pBarrage->Yield( 5 );
				}

				pBarrage->DestroyBullet( 0 );
				pBarrage->StopNewBullet();
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->Start();
		}

		m_pAI->Yield( 2.0f, false );
		pEnemyPart->SetTransparentRec( true );

		pImg->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.5f, false );
		pEnemyPart->bVisible = false;
	}
	catch( uint32 e )
	{
		m_bKilled = true;
		pEnemyPart->SetTransparentRec( true );
		pEnemyPart->KillEffect();
		pImg->SetFrames( 4, 5, 0 );
		pImg->SetPlaySpeed( 0, false );

		m_pAI->Yield( 1.0f, false );

		pImg->SetFrames( 4, 8, 8 );
		pImg->SetPlaySpeed( 1, false );
	}
}

void CWindow3Controller::OnAddedToStage()
{
	m_pAI = new AI();
	m_pAI->SetParentEntity( this );
}

void CWindow3Controller::AIFunc()
{
	m_pAI->Yield( 0.25f, false );

	CReference<CAIObject> pAIObject;
	CReference<CWindow3> pCurWindow;
	while( 1 )
	{
		if( pAIObject )
		{
			if( !pAIObject->GetParentEntity() || !pAIObject->IsRunning() )
				pAIObject = NULL;
		}
		if( !pAIObject )
		{
			if( pCurWindow && pCurWindow->IsKilled() )
				return;

			SRand::Inst().Shuffle( m_vecWindow3 );
			for( auto& pWindow : m_vecWindow3 )
			{
				if( !pWindow )
					continue;
				if( pWindow->IsKilled() )
				{
					pWindow = NULL;
					continue;
				}

				if( pCurWindow )
					pWindow->SetHp( pCurWindow->GetHp() );
				pCurWindow = pWindow;
				pAIObject = pWindow->TryPlay();
				if( pAIObject )
					break;
			}
			if( pAIObject )
			{
				m_pAI->Yield( 10.0f, false );
				continue;
			}
		}

		m_pAI->Yield( 0.25f, false );
	}
}