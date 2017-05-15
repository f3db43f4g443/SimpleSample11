#include "stdafx.h"
#include "EnemyCharacters.h"
#include "MyLevel.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "Door.h"
#include "Bullet.h"
#include "Common/ResourceManager.h"

void CEnemyCharacter::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_flyData.bHitChannel[eEntityHitType_Platform] = false;
	m_nState = 0;
	float angle = SRand::Inst().Rand( -PI, PI );
	m_curMoveDir = CVector2( cos( angle ), sin( angle ) );

	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab );
}

void CEnemyCharacter::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	CReference<CEntity> pTemp = this;
	uint8 newAnimState = 0;

	UpdateMove();
	if( !GetStage() )
		return;
	UpdateFire();

	if( m_curMoveDir.x != 0 || m_curMoveDir.y != 0 )
		newAnimState = 1;
	if( m_nState == 1 )
	{
		newAnimState += m_curMoveDir.x > 0 ? 0 : 3;
	}
	else
	{
		newAnimState += m_curMoveDir.x > 0 ? 0 : 3;
	}

	if( newAnimState != m_nAnimState )
	{
		auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		switch( newAnimState )
		{
		case 0:
			pImage->SetFrames( 0, 1, 0 );
			break;
		case 1:
			pImage->SetFrames( 1, 7, 12 );
			break;
		case 2:
			pImage->SetFrames( 11, 16, 9.9f );
			break;
		case 3:
			pImage->SetFrames( 16, 17, 0 );
			break;
		case 4:
			pImage->SetFrames( 17, 23, 12 );
			break;
		case 5:
			pImage->SetFrames( 27, 32, 9.9f );
			break;
		default:
			break;
		}
		m_nAnimState = newAnimState;
	}
}

void CEnemyCharacter::UpdateMove()
{
	CVector2 prePos = GetPosition();

	if( m_nState == 1 )
	{
		CVector2 fixedVelocity = m_walkData.UpdateMove( this, m_nFireStopTimeLeft ? 0 : ( m_curMoveDir.x > 0 ? 1 : -1 ), false );
		if( !GetStage() )
			return;

		auto levelBound = CMyLevel::GetInst()->GetBound();
		if( x < levelBound.x || x > levelBound.GetRight() || y < levelBound.y )
		{
			Kill();
			return;
		}

		if( fixedVelocity.x == 0 )
			m_curMoveDir.x = -m_curMoveDir.x;

		CChunkObject* pCurRoom = NULL;
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CChunkObject* pChunkObject = SafeCast<CChunkObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
			if( pChunkObject && pChunkObject->GetChunk()->bIsRoom )
			{
				CRectangle rect( pChunkObject->globalTransform.GetPosition().x, pChunkObject->globalTransform.GetPosition().y,
					pChunkObject->GetChunk()->nWidth * CMyLevel::GetBlockSize(),
					pChunkObject->GetChunk()->nHeight * CMyLevel::GetBlockSize() );
				if( rect.Contains( GetPosition() ) )
					pCurRoom = pChunkObject;
			}
		}

		if( pCurRoom )
		{
			m_flyData.Reset();
			m_flyData.SetLandedEntity( pCurRoom );
			m_flyData.fKnockbackTime = m_walkData.fKnockbackTime;
			m_flyData.vecKnockback = m_walkData.vecKnockback;
			m_nState = 0;

			auto pChunk = pCurRoom->GetChunk();
			CRectangle roomBound( pCurRoom->globalTransform.GetPosition().x, pCurRoom->globalTransform.GetPosition().y,
				pChunk->nWidth * CMyLevel::GetBlockSize(), pChunk->nHeight * CMyLevel::GetBlockSize() );
			CVector2 dir1 = m_velocity;
			CVector2 dir2 = roomBound.GetCenter() - GetPosition();
			dir1.Normalize();
			dir2.Normalize();
			m_curMoveDir = dir1 + ( dir2 - dir1 ) * SRand::Inst().Rand( 0.5f, 1.0f );
			m_curMoveDir.Normalize();
		}
	}
	else
	{
		if( !m_flyData.pLandedEntity )
		{
			for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				auto pChunkObject = SafeCast<CChunkObject>( pEntity );
				if( pChunkObject )
				{
					m_flyData.SetLandedEntity( pChunkObject );
					break;
				}
			}
		}
		if( m_flyData.pLandedEntity && !m_flyData.pLandedEntity->GetStage() )
			m_flyData.SetLandedEntity( NULL );
		if( m_flyData.pLandedEntity )
		{
			bool bHitChunkObject = false;
			for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				auto pChunkObject = SafeCast<CChunkObject>( pEntity );
				if( m_flyData.pLandedEntity == pChunkObject )
				{
					bHitChunkObject = true;
					break;
				}
			}
			if( !bHitChunkObject )
			{
				m_flyData.pLandedEntity = NULL;
			}
		}

		if( m_flyData.pLandedEntity )
		{
			m_flyData.fMoveSpeed = m_fOrigFlySpeed;

			if( !m_nFireStopTimeLeft )
			{
				auto pChunk = SafeCast<CChunkObject>( m_flyData.pLandedEntity.GetPtr() )->GetChunk();
				CRectangle roomBound( m_flyData.pLandedEntity->globalTransform.GetPosition().x, m_flyData.pLandedEntity->globalTransform.GetPosition().y,
					pChunk->nWidth * CMyLevel::GetBlockSize(), pChunk->nHeight * CMyLevel::GetBlockSize() );
				CRectangle selfBound;
				CMatrix2D mat;
				mat.Identity();
				Get_HitProxy()->CalcBound( mat, selfBound );
				roomBound.x -= selfBound.x - CMyLevel::GetBlockSize() *0.5f;
				roomBound.y -= selfBound.y - CMyLevel::GetBlockSize() *0.5f;
				roomBound.width -= selfBound.width + CMyLevel::GetBlockSize();
				roomBound.height -= selfBound.height + CMyLevel::GetBlockSize();

				m_flyData.UpdateMove( this, m_curMoveDir );
				if( !GetStage() )
					return;

				if( m_flyData.finalMoveAxis.x == 0 || m_curMoveDir.x < 0 && x <= roomBound.x || m_curMoveDir.x > 0 && x >= roomBound.GetRight() )
					m_curMoveDir.x = -m_curMoveDir.x;
				if( m_flyData.finalMoveAxis.y == 0 || m_curMoveDir.y < 0 && y <= roomBound.y || m_curMoveDir.y > 0 && y >= roomBound.GetBottom() )
					m_curMoveDir.y = -m_curMoveDir.y;
			}
			else
			{
				m_flyData.UpdateMove( this, CVector2( 0, 0 ) );
				if( !GetStage() )
					return;
			}
		}
		if( !m_flyData.pLandedEntity )
		{
			m_flyData.bHitChannel[eEntityHitType_Platform] = true;
			m_walkData.Reset();
			m_walkData.fKnockbackTime = m_flyData.fKnockbackTime;
			m_walkData.vecKnockback = m_flyData.vecKnockback;
			m_nState = 1;
		}
	}

	CVector2 curPos = GetPosition();
	m_velocity = ( curPos - prePos ) / GetStage()->GetElapsedTimePerTick();

	CChunkObject* pCurRoom = NULL;
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CChunkObject* pChunkObject = SafeCast<CChunkObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pChunkObject && pChunkObject->GetChunk()->bIsRoom )
		{
			CRectangle rect( pChunkObject->globalTransform.GetPosition().x, pChunkObject->globalTransform.GetPosition().y,
				pChunkObject->GetChunk()->nWidth * CMyLevel::GetBlockSize(),
				pChunkObject->GetChunk()->nHeight * CMyLevel::GetBlockSize() );
			if( rect.Contains( GetPosition() ) )
				pCurRoom = pChunkObject;
		}
	}
}

void CEnemyCharacter::UpdateFire()
{
	bool bCanFire;
	if( m_nState == 0 )
		bCanFire = true;
	else
		bCanFire = m_walkData.bLanded;

	if( m_nFireCDLeft )
		m_nFireCDLeft--;

	if( !bCanFire )
	{
		m_nAmmoLeft = 0;
		m_nFireStopTimeLeft = 0;
		return;
	}

	if( m_nFireStopTimeLeft )
		m_nFireStopTimeLeft--;
	if( m_nNextFireTime )
		m_nNextFireTime--;

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	CVector2 p = pPlayer->GetPosition() - globalTransform.GetPosition();

	if( !m_nFireCDLeft )
	{
		if( p.Length2() > m_fSight * m_fSight )
			m_nFireCDLeft = 30;
		else
		{
			m_nFireCDLeft = m_nFireCD;
			m_nFireStopTimeLeft = m_nFireStopTime;
			m_nAmmoLeft = m_nAmmoCount;
			m_nNextFireTime = 0;
		}
	}

	if( m_nAmmoLeft )
	{
		if( m_nNextFireTime )
			m_nNextFireTime--;

		if( !m_nNextFireTime )
		{
			for( int i = 0; i < m_nBulletCount; i++ )
			{
				auto pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() );
				float r = atan2( p.y, p.x ) + ( i - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
				pBullet->SetRotation( atan2( p.y, p.x ) );
				pBullet->SetVelocity( CVector2( cos( r ), sin( r ) ) * m_fBulletSpeed );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}

			CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );

			m_nAmmoLeft--;
			m_nNextFireTime = m_nFireInterval;
		}
	}
}

bool CEnemyCharacter::Knockback( const CVector2& vec )
{
	if( m_nState == 1 )
	{
		if( m_walkData.fKnockbackTime <= 0 )
			m_walkData.Knockback( 0.5f, vec * 500 );
	}
	else
	{
		if( m_flyData.fKnockbackTime <= 0 )
			m_flyData.Knockback( 0.5f, vec * 500 );
	}
	return true;
}

bool CEnemyCharacter::IsKnockback()
{
	if( m_nState == 1 )
	{
		if( m_walkData.fKnockbackTime > 0 )
			return true;
	}
	else
	{
		if( m_flyData.fKnockbackTime > 0 )
			return true;
	}
	return false;
}

bool CEnemyCharacter::CanTriggerItem()
{
	return m_nState == 0;
}