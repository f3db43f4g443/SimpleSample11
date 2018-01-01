#include "stdafx.h"
#include "EnemyCharacters.h"
#include "MyLevel.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "Door.h"
#include "Bullet.h"
#include "Common/ResourceManager.h"
#include "Entities/Bullets.h"
#include "Entities/Barrage.h"
#include "Entities/BlockItems/BlockItemsLv2.h"

void CEnemyCharacter::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_flyData.bHitChannel[eEntityHitType_Platform] = false;
	m_nState = 0;
	float angle = SRand::Inst().Rand( -PI, PI );
	m_curMoveDir = CVector2( cos( angle ), sin( angle ) );
	UpdateAnimFrame();
}

void CEnemyCharacter::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	CReference<CEntity> pTemp = this;

	UpdateMove();
	if( !GetStage() )
		return;
	UpdateFire();
	UpdateAnimFrame();
}

void CEnemyCharacter::UpdateAnimFrame()
{
	uint8 newAnimState = 0;
	if( !m_nFireStopTimeLeft && ( m_curMoveDir.x != 0 || m_curMoveDir.y != 0 ) )
		newAnimState = 1;
	newAnimState += m_curMoveDir.x > 0 ? 0 : 2;

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
			pImage->SetFrames( 7, 8, 0 );
			break;
		case 3:
			pImage->SetFrames( 8, 14, 12 );
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
		CVector2 fixedVelocity = m_walkData.UpdateMove( this, m_nFireStopTimeLeft || m_bLeader ? 0 : ( m_curMoveDir.x > 0 ? 1 : -1 ), false );
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
			if( pChunkObject && pChunkObject->GetChunk()->nMoveType )
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

			if( !m_nFireStopTimeLeft && !m_bLeader )
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

				auto pChunkObject = SafeCast<CChunkObject>( m_flyData.pLandedEntity.GetPtr() );
				if( pChunkObject && pChunkObject->GetChunk()->bIsRoom )
				{
					if( m_flyData.finalMoveAxis.x == 0 || m_curMoveDir.x < 0 && x <= roomBound.x || m_curMoveDir.x > 0 && x >= roomBound.GetRight() )
						m_curMoveDir.x = -m_curMoveDir.x;
					if( m_flyData.finalMoveAxis.y == 0 || m_curMoveDir.y < 0 && y <= roomBound.y || m_curMoveDir.y > 0 && y >= roomBound.GetBottom() )
						m_curMoveDir.y = -m_curMoveDir.y;
				}
				else
				{
					if( m_flyData.finalMoveAxis.x == 0 )
						m_curMoveDir.x = -m_curMoveDir.x;
					if( m_flyData.finalMoveAxis.y == 0 )
						m_curMoveDir.y = -m_curMoveDir.y;
				}
			}
			else
			{
				m_flyData.UpdateMove( this, CVector2( 0, 0 ) );
				if( !GetStage() )
					return;
			}
		}
		else
		{
			m_flyData.UpdateMove( this, CVector2( 0, 0 ) );
			if( !GetStage() )
				return;
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
}

void CEnemyCharacter::UpdateFire()
{
	if( m_bLeader )
		return;

	bool bCanFire = CanFire();

	if( m_nFireCDLeft )
		m_nFireCDLeft--;

	if( !bCanFire )
	{
		m_nAmmoLeft = 0;
		if( m_nFireStopTimeLeft )
		{
			m_nFireStopTimeLeft = 0;
			OnEndFire();
		}
		return;
	}

	if( m_nFireStopTimeLeft )
	{
		m_nFireStopTimeLeft--;
		if( !m_nFireStopTimeLeft )
			OnEndFire();
	}
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
			OnBeginFire();
		}
	}

	if( m_nAmmoLeft )
	{
		if( m_nNextFireTime )
			m_nNextFireTime--;

		if( !m_nNextFireTime )
		{
			float fAngle = atan2( p.y, p.x );
			if( m_fPredict > 0 )
			{
				CVector2 vel = pPlayer->GetVelocity();
				float v = vel.Normalize();
				p.Normalize();
				float sn = p.x * vel.y - p.y * vel.x;
				float sn1 = sn / m_fBulletSpeed * v;
				if( sn1 > -1.0f && sn1 < 1.0f )
					fAngle += asin( sn ) * m_fPredict;
			}

			for( int i = 0; i < m_nBulletCount; i++ )
			{
				auto pBullet = SafeCast<CBullet>( m_strPrefab->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() );
				float r = fAngle + ( i - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
				pBullet->SetRotation( r );
				pBullet->SetVelocity( CVector2( cos( r ), sin( r ) ) * m_fBulletSpeed );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}

			CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );
			OnFire();

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

float CEnemyCharacter::GetCurMoveSpeed()
{
	return m_nState == 1 ? m_walkData.fMoveSpeed : m_flyData.fMoveSpeed;
}

bool CEnemyCharacter::CanTriggerItem()
{
	return m_nState == 0;
}

void CEnemyCharacterLeader::OnRemovedFromStage()
{
	for( auto& pChar : m_chars )
	{
		if( pChar )
			SafeCast<CEnemyCharacter>( pChar.GetPtr() )->SetLeader( false );
	}
	m_chars.clear();
}

void CEnemyCharacterLeader::OnBeginFire()
{
	SHitProxyCircle hitproxy;
	hitproxy.fRadius = m_fRadius;
	hitproxy.center = CVector2( 0, 0 );
	vector<CReference<CEntity> > vecHit;
	GetStage()->MultiHitTest( &hitproxy, globalTransform, vecHit );
	for( CEntity* pEntity : vecHit )
	{
		if( pEntity->GetTypeID() == CClassMetaDataMgr::Inst().GetClassID<CEnemyCharacter>() )
		{
			SafeCast<CEnemyCharacter>( pEntity )->SetLeader( true );
			m_chars.push_back( pEntity );
		}
	}
}

void CEnemyCharacterLeader::OnFire()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	CVector2 p = pPlayer->GetPosition() - globalTransform.GetPosition();
	float f = m_nAmmoLeft * 1.0f / m_nAmmoCount;

	for( auto& pChar : m_chars )
	{
		if( !pChar )
			continue;
		if( !pChar->GetStage() || !SafeCast<CEnemyCharacter>( pChar.GetPtr() )->CanFire() )
		{
			pChar = NULL;
			continue;
		}
		CVector2 p1 = pPlayer->GetPosition() - pChar->globalTransform.GetPosition();
		p1 = p * ( 1 - f ) + p1 * f;

		for( int i = 0; i < m_nBulletCount; i++ )
		{
			auto pBullet = SafeCast<CBullet>( m_strPrefab->GetRoot()->CreateInstance() );
			pBullet->SetPosition( pChar->globalTransform.GetPosition() );
			float angle = atan2( p1.y, p1.x ) + ( i - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
			pBullet->SetRotation( angle );
			pBullet->SetVelocity( CVector2( cos( angle ), sin( angle ) ) * m_fBulletSpeed );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
	}
}

void CEnemyCharacterLeader::OnEndFire()
{
	for( auto& pChar : m_chars )
	{
		if( pChar )
			SafeCast<CEnemyCharacter>( pChar.GetPtr() )->SetLeader( false );
	}
	m_chars.clear();
}

void CCop::OnAddedToStage()
{
	CEnemyCharacter::OnAddedToStage();
	m_pNav = CNavigationUnit::Alloc();
	m_pNav->Set( false, m_fMaxScanDist, m_nGridsPerStep );
	m_pNav->RegisterVisitGridEvent( &m_onVisitGrid );
	m_pNav->RegisterFindTargetEvent( &m_onFindPath );
}

void CCop::OnRemovedFromStage()
{
	if( m_onVisitGrid.IsRegistered() )
		m_onVisitGrid.Unregister();
	if( m_onFindPath.IsRegistered() )
		m_onFindPath.Unregister();
	m_pNav->Clear();
	CNavigationUnit::Free( m_pNav );
	CEnemyCharacter::OnRemovedFromStage();
}

void CCop::OnTickAfterHitTest()
{
	if( !m_nStateChangeTime )
	{
		if( m_nState == 0 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 dPos = pPlayer->globalTransform.GetPosition() - globalTransform.GetPosition();
				if( dPos.Length() >= m_fSight )
				{
					m_pNav->SetTarget( pPlayer );
					m_nState = 1;
					m_nStateChangeTime = 60;
				}
			}
		}
		else
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 dPos = pPlayer->globalTransform.GetPosition() - globalTransform.GetPosition();
				if( dPos.Length() < 200 )
				{
					m_pNav->SetTarget( NULL );
					m_nState = 0;
					m_nStateChangeTime = 60;
				}
			}
		}
	}

	m_pNav->Step( this );

	m_nFindPathDelay++;
	if( m_nStateChangeTime )
		m_nStateChangeTime--;
	if( m_fNearestDist != FLT_MAX )
	{
		if( !m_pNav->HasPath() && m_nState == 1 )
		{
			m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
		}
		if( m_nFindPathDelay >= 120 )
		{
			m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
			m_nFindPathDelay = 0;
		}
	}

	if( m_pNav->HasPath() )
		m_curMoveDir = m_pNav->FollowPath( this, GetCurMoveSpeed() );

	CEnemyCharacter::OnTickAfterHitTest();
}

void CCop::OnVisitGrid( CNavigationUnit::SGridData* pGrid )
{
	if( !pGrid )
	{
		if( m_nState == 0 )
		{
			if( m_fNearestDist != FLT_MAX )
				m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
			else
			{
				float r = SRand::Inst().Rand( -PI, PI );
				m_curMoveDir = CVector2( cos( r ), sin( r ) );
			}
		}
		m_fNearestDist = FLT_MAX;
		return;
	}

	if( m_nState == 1 )
	{
		if( pGrid->fAStarDist < m_fNearestDist )
		{
			m_fNearestDist = pGrid->fAStarDist;
			m_nearestGrid = pGrid->pos;
		}
	}
	else
	{
		CVector2 dir[2];
		dir[0] = CVector2( 0, 1 );
		dir[1] = CVector2( 0, 0 );
		int32 nDir = 1;
		float fWeight[2] = { 1 + Min( CMyLevel::GetInst()->GetShakeStrength(), 50.0f ) * 0.1f, 0 };

		float fDist = 0;
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
		{
			dir[1] = pPlayer->globalTransform.GetPosition() - globalTransform.GetPosition();
			float l = dir[1].Normalize();
			if( l > 0 )
				nDir = 2;
			fWeight[1] = -0.02f * Max( 600 - l, 0.0f );
		}

		for( int i = 0; i < 2; i++ )
		{
			CVector2 pos = m_pNav->GridToRect( pGrid->pos ).GetCenter();
			for( int j = 0; j < 3; j++ )
			{
				pos = pos + dir[i];
				if( CMyLevel::GetInst()->GetNavigationData( m_pNav->GetGridByPos( pos ) ) == 0 )
				{
					fDist += fWeight[i] * ( 3 - j );
					break;
				}
			}
		}

		if( fDist < m_fNearestDist )
		{
			m_fNearestDist = fDist;
			m_nearestGrid = pGrid->pos;
		}
	}
}

void CCop::OnFindPath( CNavigationUnit::SGridData* pGrid )
{
	if( pGrid )
		m_pNav->BuildPath( pGrid, this );
	else if( m_fNearestDist != FLT_MAX )
		m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
	else
	{
		float r = SRand::Inst().Rand( -PI, PI );
		m_curMoveDir = CVector2( cos( r ), sin( r ) );
	}
	m_nFindPathDelay = 0;
}

void CThug::OnAddedToStage()
{
	CEnemyCharacter::OnAddedToStage();
	m_pNav = CNavigationUnit::Alloc();
	m_fNearestDist = FLT_MAX;
	m_pNav->Set( false, m_fMaxScanDist, m_nGridsPerStep );
	m_pNav->RegisterVisitGridEvent( &m_onVisitGrid );
	m_pNav->RegisterFindTargetEvent( &m_onFindPath );
}

void CThug::OnRemovedFromStage()
{
	if( m_onVisitGrid.IsRegistered() )
		m_onVisitGrid.Unregister();
	if( m_onFindPath.IsRegistered() )
		m_onFindPath.Unregister();
	m_pNav->Clear();
	CNavigationUnit::Free( m_pNav );
	CEnemyCharacter::OnRemovedFromStage();
}

void CThug::OnTickAfterHitTest()
{
	if( m_pThrowObj && !m_pThrowObj->GetStage() )
		m_pThrowObj = NULL;
	if( m_nStateTime )
		m_nStateTime--;

	if( m_pThrowObj )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( m_pNav->GetTarget() != pPlayer )
			m_pNav->SetTarget( pPlayer );

		m_pNav->Step( this );
		if( m_pNav->HasPath() )
			m_curMoveDir = m_pNav->FollowPath( this, GetCurMoveSpeed() );
		if( pPlayer && !m_nStateTime )
		{
			CVector2 dPos = pPlayer->GetPosition() - globalTransform.GetPosition();
			if( m_bAtRoof || dPos.Length2() < m_fThrowDist * m_fThrowDist )
			{
				ThrowObj( pPlayer->GetPosition() );

				m_fNearestDist = FLT_MAX;
				m_pNav->SetTarget( NULL );
			}
		}
	}
	else
	{
		m_pNav->Step( this );
		if( !m_nStateTime )
		{
			if( m_pNav->HasPath() )
				m_curMoveDir = m_pNav->FollowPath( this, GetCurMoveSpeed() );

			for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

				auto pEntrance = SafeCast<CHouseEntrance>( pEntity );
				if( pEntrance && pEntrance->Enter( this ) )
					return;
			}
		}
	}

	CEnemyCharacter::OnTickAfterHitTest();
}

void CThug::Kill()
{
	if( m_pThrowObj )
	{
		m_pThrowObj->SetPosition( GetPosition() + m_pThrowObj->GetPosition() );
		m_pThrowObj->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		m_pThrowObj->Kill();
		m_pThrowObj = NULL;
	}
	CEnemyCharacter::Kill();
}

void CThug::SetThrowObj( CCharacter * pObj, const CVector2& ofs, bool bAtRoof )
{
	m_pThrowObj = pObj;
	m_pThrowObj->SetPosition( ofs + m_throwObjOfs );
	m_pThrowObj->SetParentEntity( this );
	m_nStateTime = SRand::Inst().Rand( 60, 120 );
	m_bAtRoof = bAtRoof;
}

void CThug::ThrowObj( const CVector2& target )
{
	CVector2 dPos = target - globalTransform.GetPosition();
	float l = dPos.Normalize();
	m_pThrowObj->SetPosition( GetPosition() + m_pThrowObj->GetPosition() );
	m_pThrowObj->SetVelocity( dPos * m_fThrowSpeed * ( m_bAtRoof ? 1.5f : 1 ) );
	if( m_bAtRoof )
	{
		auto pThrowObj = SafeCast<CThrowObj>( m_pThrowObj.GetPtr() );
		if( pThrowObj )
		{
			pThrowObj->SetLife( pThrowObj->GetLife() * 1.5f );
			pThrowObj->SetLife1( pThrowObj->GetLife1() * 1.5f );
		}
	}
	m_pThrowObj->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	m_pThrowObj = NULL;
	m_nFireCDLeft = m_nFireCD;
	m_nStateTime = 30;
	m_curMoveDir = CVector2( 0, 0 );
}

void CThug::UpdateAnimFrame()
{
	uint8 newAnimState = 0;
	if( !m_nFireStopTimeLeft && ( m_curMoveDir.x != 0 || m_curMoveDir.y != 0 ) )
		newAnimState = 1;
	if( m_pThrowObj )
		newAnimState += 2;
	newAnimState += m_curMoveDir.x > 0 ? 0 : 4;

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
			pImage->SetFrames( 7, 8, 0 );
			break;
		case 3:
			pImage->SetFrames( 8, 14, 12 );
			break;
		case 4:
			pImage->SetFrames( 14, 15, 0 );
			break;
		case 5:
			pImage->SetFrames( 15, 21, 12 );
			break;
		case 6:
			pImage->SetFrames( 21, 22, 0 );
			break;
		case 7:
			pImage->SetFrames( 22, 28, 12 );
			break;
		default:
			break;
		}
		m_nAnimState = newAnimState;
	}
}

void CThug::OnVisitGrid( CNavigationUnit::SGridData * pGrid )
{
	if( !pGrid )
	{
		if( !m_pThrowObj )
		{
			if( m_fNearestDist != FLT_MAX )
				m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
			else
			{
				float r = SRand::Inst().Rand( -PI, PI );
				m_curMoveDir = CVector2( cos( r ), sin( r ) );
			}
		}
		m_fNearestDist = FLT_MAX;
		return;
	}

	if( m_pThrowObj )
	{
		if( pGrid->fAStarDist < m_fNearestDist )
		{
			m_fNearestDist = pGrid->fAStarDist;
			m_nearestGrid = pGrid->pos;
		}
	}
	else
	{
		if( pGrid->nType == 2 )
		{
			auto pHitTestGrid = GetStage()->GetHitTestMgr().GetGrid( pGrid->pos );
			for( auto pProxyGrid = pHitTestGrid->Get_InGrid(); pProxyGrid; pProxyGrid = pProxyGrid->NextInGrid() )
			{
				auto pEntity = static_cast<CEntity*>( pProxyGrid->pHitProxy->pOwner );

				auto pEntrance = SafeCast<CHouseEntrance>( pEntity );
				if( pEntrance && pEntrance->CanEnter( this ) )
				{
					if( pGrid->fDist < m_fNearestDist )
					{
						m_fNearestDist = pGrid->fDist;
						m_nearestGrid = pGrid->pos;
					}
				}
			}
		}
	}
}

void CThug::OnFindPath( CNavigationUnit::SGridData * pGrid )
{
	if( pGrid )
		m_pNav->BuildPath( pGrid, this );
	else if( m_fNearestDist != FLT_MAX )
		m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
	else
	{
		float r = SRand::Inst().Rand( -PI, PI );
		m_curMoveDir = CVector2( cos( r ), sin( r ) );
	}
}

void CWorker::OnAddedToStage()
{
	CEnemyCharacter::OnAddedToStage();
	m_pNav = CNavigationUnit::Alloc();
	m_fNearestDist = FLT_MAX;
	m_pNav->Set( false, m_fMaxScanDist, m_nGridsPerStep );
	m_pNav->RegisterVisitGridEvent( &m_onVisitGrid );
	m_pNav->RegisterFindTargetEvent( &m_onFindPath );
}

void CWorker::OnRemovedFromStage()
{
	if( m_onVisitGrid.IsRegistered() )
		m_onVisitGrid.Unregister();
	if( m_onFindPath.IsRegistered() )
		m_onFindPath.Unregister();
	SetTarget( NULL );
	m_pNav->Clear();
	CNavigationUnit::Free( m_pNav );
	CEnemyCharacter::OnRemovedFromStage();
}

void CWorker::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	if( m_nStateTime )
		m_nStateTime--;
	if( m_pTarget && !m_pTarget->GetStage() )
		SetTarget( NULL );
	
	if( !m_nStateTime )
	{
		m_pNav->Step( this );
		if( m_pNav->HasPath() )
			m_curMoveDir = m_pNav->FollowPath( this, GetCurMoveSpeed() );

		if( m_pTarget )
		{
			if( IsFlee() )
			{
				m_pTarget = NULL;
				m_pNav->Reset();
			}
			else if( m_pTarget->Operate( this, true ) )
			{
				m_pNav->Reset();
				m_nStateTime = m_nOperateTime;
			}
		}
	}
	else
		m_curMoveDir = CVector2( 0, 0 );
	if( m_nStateTime == m_nOperateTime - m_nOperatePoint && m_pTarget )
		m_pTarget->Operate( this );

	if( GetStage() )
		CEnemyCharacter::OnTickAfterHitTest();
}

bool CWorker::CanFire()
{
	if( IsFlee() )
		return false;
	return !m_nStateTime && CEnemyCharacter::CanFire();
}

void CWorker::OnVisitGrid( CNavigationUnit::SGridData* pGrid )
{
	if( !pGrid )
	{
		if( m_fNearestDist != FLT_MAX )
			m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
		else
		{
			SetTarget( NULL );
			float r = SRand::Inst().Rand( -PI, PI );
			m_curMoveDir = CVector2( cos( r ), sin( r ) );
		}
		m_fNearestDist = FLT_MAX;
		return;
	}

	bool bFlee = IsFlee();
	if( pGrid->nType == 2 )
	{
		CChunkObject* pCurRoom = NULL;
		CChunkObject* pPlayerRoom = NULL;
		if( m_nState == 0 && m_flyData.pLandedEntity )
			pCurRoom = SafeCast<CChunkObject>( m_flyData.pLandedEntity.GetPtr() );
		bool bPlayerRoom = false;
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
			pPlayerRoom = pPlayer->GetCurRoom();

		auto pHitTestGrid = GetStage()->GetHitTestMgr().GetGrid( pGrid->pos );
		for( auto pProxyGrid = pHitTestGrid->Get_InGrid(); pProxyGrid; pProxyGrid = pProxyGrid->NextInGrid() )
		{
			auto pEntity = static_cast<CEntity*>( pProxyGrid->pHitProxy->pOwner );

			auto pOperatingArea = SafeCast<COperatingArea>( pEntity );
			if( pOperatingArea && pOperatingArea->CanOperate( this ) )
			{
				if( !bFlee && pCurRoom && pCurRoom->GetChunk() && pCurRoom->GetChunk()->bIsRoom )
				{
					if( !pCurRoom->GetRect().Contains( pOperatingArea->globalTransform.GetPosition() ) )
						continue;
				}
				if( pPlayer )
				{
					auto pPlayerRoom = pPlayer->GetCurRoom();
					if( pPlayerRoom && pPlayerRoom->GetChunk() && pPlayerRoom->GetChunk()->bIsRoom
						&& pPlayerRoom->GetRect().Contains( pOperatingArea->globalTransform.GetPosition() ) )
						continue;
				}

				if( pGrid->fDist < m_fNearestDist )
				{
					SetTarget( pOperatingArea );
					m_fNearestDist = pGrid->fDist;
					m_nearestGrid = pGrid->pos;
				}
			}

			auto pBlockObject = SafeCast<CBlockObject>( pEntity );
			if( pPlayerRoom && pBlockObject && pBlockObject->GetParentEntity() == pPlayerRoom )
				bPlayerRoom = true;
		}

		if( !m_pTarget && IsFlee() && !bPlayerRoom )
		{
			if( pGrid->fDist < m_fNearestDist )
			{
				m_fNearestDist = pGrid->fDist;
				m_nearestGrid = pGrid->pos;
			}
		}
	}
}

void CWorker::OnFindPath( CNavigationUnit::SGridData* pGrid )
{
	if( pGrid )
		m_pNav->BuildPath( pGrid, this );
	else if( m_pTarget && m_fNearestDist != FLT_MAX )
		m_pNav->BuildPath( &m_pNav->GetGrid( m_nearestGrid ), this );
	else
	{
		SetTarget( NULL );
		float r = SRand::Inst().Rand( -PI, PI );
		m_curMoveDir = CVector2( cos( r ), sin( r ) );
	}
}

void CWorker::SetTarget( COperatingArea* pOperatingArea )
{
	if( m_pTarget && m_pTarget->GetStage() )
		m_pTarget->SetOperator( NULL );
	m_pTarget = pOperatingArea;
	if( m_pTarget )
		m_pTarget->SetOperator( this );
}

bool CWorker::IsFlee()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return false;
	auto pPlayerRoom = pPlayer->GetCurRoom();
	if( pPlayerRoom && pPlayerRoom->GetChunk() && pPlayerRoom->GetChunk()->bIsRoom )
	{
		auto rect = pPlayerRoom->GetRect();
		rect.x -= 32;
		rect.y -= 32;
		rect.width += 64;
		rect.height += 64;
		return rect.Contains( globalTransform.GetPosition() );
	}

	return false;
}

void CWorker::UpdateAnimFrame()
{
	uint8 newAnimState = 0;
	if( m_nStateTime )
		newAnimState = 2;
	else if( !m_nFireStopTimeLeft && ( m_curMoveDir.x != 0 || m_curMoveDir.y != 0 ) )
		newAnimState = 1;
	if( !m_nStateTime )
		newAnimState += m_curMoveDir.x > 0 ? 0 : 3;
	else
		newAnimState += m_nAnimState >= 3 ? 3 : 0;

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
			pImage->SetFrames( 7, 11, 12 );
			break;
		case 3:
			pImage->SetFrames( 11, 12, 0 );
			break;
		case 4:
			pImage->SetFrames( 12, 18, 12 );
			break;
		case 5:
			pImage->SetFrames( 18, 22, 12 );
			break;
		default:
			break;
		}
		m_nAnimState = newAnimState;
	}
}

void CWorker::OnFire()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	CVector2 dPos = pPlayer->GetPosition() - GetPosition();

	SBarrageContext context;
	context.pCreator = GetParentEntity();
	context.vecBulletTypes.push_back( m_strPrefab.GetPtr() );
	context.nBulletPageSize = 10;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [dPos] ( CBarrage* pBarrage )
	{
		CVector2 dir = dPos;
		float l = dir.Normalize();
		if( l <= 0.01f )
		{
			dir = CVector2( 1, 0 );
			l = 1.0f;
		}
		float t = l / 200.0f;
		float a = -150.0f;
		float v = l / t - a * t * 0.5f;
		float a1 = SRand::Inst().Rand( -180.0f, 180.0f );
		float v1 = -a1 * t * 0.5f;

		pBarrage->InitBullet( 0, -1, -1, CVector2( 0, 0 ), dir * v + CVector2( -dir.y, dir.x ) * v1, dir * a + CVector2( -dir.y, dir.x ) * a1,
			false, SRand::Inst().Rand( -PI, PI ), 3.0f * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 ) );
		for( int i = 0; i < 6; i++ )
		{
			float fAngle = PI * ( i - 2.5f ) / 5.0f;
			pBarrage->InitBullet( i + 1, 0, 0, CVector2( cos( fAngle ), sin( fAngle ) ) * 50.0f, CVector2( 0, 0 ), CVector2( 0, 0 ), false );
		}
		for( int i = 1; i < 4; i++ )
		{
			float fAngle = PI * ( i - 2.0f ) / 4.0f;
			pBarrage->InitBullet( i + 6, 0, 0, CVector2( cos( fAngle ) * 35.0f, sin( fAngle ) * 50.0f ), CVector2( 0, 0 ), CVector2( 0, 0 ), false );
		}

		pBarrage->Yield( 2 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetPosition( GetPosition() );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->Start();
}