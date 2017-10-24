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

int8 COperateableTurret1::IsOperateable( const CVector2& pos )
{
	if( m_onTick.IsRegistered() )
		return -1;
	return 0;
}

void COperateableTurret1::Operate( const CVector2& pos )
{
	m_nAmmoLeft = m_nAmmoCount;
	OnTick();
}

void COperateableTurret1::OnTick()
{
	if( !m_nAmmoLeft )
		return;

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
		m_pParts[i]->bVisible = false;
		m_pParts[i]->SetTransparentRec( true );
	}
	auto pController = GetParentEntity()->GetChildByName_Fast<CWindow3Controller>( GetName() );
	if( pController )
		pController->Add( this );
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
	auto pImg = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	auto pImg1 = static_cast<CMultiFrameImage2D*>( pEnemyPart->GetRenderObject() );

	pImg->SetFrames( 0, 0, 0 );
	pImg->SetPlaySpeed( 1, false );
	pImg1->SetFrames( 0, 0, 0 );
	pImg1->SetPlaySpeed( 1, false );
	m_pAI->Yield( 1.0f, false );
	pEnemyPart->SetTransparentRec( false );
	m_pAI->Yield( 1.0f, false );

	SBarrageContext context;
	context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
	context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
	context.nBulletPageSize = 100;

	CBarrage* pBarrage = new CBarrage( context );
	CVector2 p = globalTransform.MulVector2Pos( m_fireOfs[m_nDir] );
	CVector2 dirs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	CVector2 dir = dirs[m_nDir];
	pBarrage->AddFunc( [dir] ( CBarrage* pBarrage )
	{
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->Start();

	m_pAI->Yield( 1.0f, false );
	pEnemyPart->SetTransparentRec( true );
	
	pImg->SetFrames( 0, 0, 0 );
	pImg->SetPlaySpeed( -1, false );
	pImg1->SetFrames( 0, 0, 0 );
	pImg1->SetPlaySpeed( -1, false );
	m_pAI->Yield( 1.0f, false );
	pEnemyPart->bVisible = false;
}

void CWindow3Controller::AIFunc()
{
	m_pAI->Yield( 0.25f, false );

	CReference<CAIObject> pAIObject;
	while( 1 )
	{
		if( pAIObject )
		{
			if( !pAIObject->GetParentEntity() || !pAIObject->IsRunning() )
				pAIObject = NULL;
		}
		if( !pAIObject )
		{
			SRand::Inst().Shuffle( m_vecWindow3 );
			for( auto& pWindow : m_vecWindow3 )
			{
				if( !pWindow )
					continue;
				if( !pWindow->GetHp() )
				{
					pWindow = NULL;
					continue;
				}

				pAIObject = pWindow->TryPlay();
				if( pAIObject )
					break;
			}
			if( pAIObject )
			{
				m_pAI->Yield( 5.0f, false );
				continue;
			}
		}

		m_pAI->Yield( 0.25f, false );
	}
}