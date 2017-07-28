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
	if( !m_nSpawnCount )
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
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
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
	auto pCar = SafeCast<CCar>( m_strPrefab->GetRoot()->CreateInstance() );
	pCar->SetExcludeChunk( SafeCast<CChunkObject>( GetParentEntity() ), excludeRect );
	pCar->SetPosition( globalTransform.GetPosition() );
	pCar->SetVelocity( m_spawnVel );
	pCar->SetRotation( atan2( m_spawnVel.y, m_spawnVel.x ) );
	pCar->SetParentAfterEntity( CMyLevel::GetInst()->GetChunkRoot1() );
	m_nSpawnCount--;
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
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
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
