#include "stdafx.h"
#include "Bullet.h"
#include "Stage.h"
#include "Face.h"
#include "Character.h"
#include "World.h"

void CBulletBase::OnAddedToStage()
{
	auto pStage = GetStage();
	auto pFace = pStage->GetRoot()->GetChildByName_Fast<CFace>( "" );
	m_pFace = pFace;
	m_pFace->KeepAwake( 10 );
	m_pEffectObject = GetChildByName_Fast<CEffectObject>( "" );
	if( m_pEffectObject )
		m_pEffectObject->SetState( 1 );
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CBulletBase::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

void CBulletBase::TickBeforeHitTest()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	m_pFace->KeepAwake( 10 );
}

void CBulletBase::TickAfterHitTest()
{
	CVector2 worldPos = globalTransform.GetPosition();
	if( !m_pFace->GetKillBound().Contains( worldPos ) )
	{
		m_bActive = false;
		SetParentEntity( NULL );
		return;
	}

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CBulletBase::Kill()
{
	m_bActive = false;
	if( m_pEffectObject )
	{
		m_pFace->KeepAwake( m_pEffectObject->GetDeathTime() / GetStage()->GetElapsedTimePerTick() + 10 );
		m_pEffectObject->SetState( 2 );
		if( !m_tickAfterHitTest.IsRegistered() )
			GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	}
	else
		SetParentEntity( NULL );
}

void CBullet::Emit( SOrganActionContext & actionContext )
{
	auto dGridPos = actionContext.target - actionContext.pCharacter->GetGrid();
	auto dPos = RotateDirInv( dGridPos, actionContext.pCharacter->GetDir() );
	float r = atan2( dPos.y, dPos.x );
	CVector2 dir( dPos.x, dPos.y );
	dir.Normalize();

	SetParentEntity( actionContext.pOrgan->GetParentEntity() );
	SetPosition( actionContext.pOrgan->GetPosition() );
	SetRotation( r );
	SetVelocity( dir * GetSpeed() );
}

void CBullet::SetFaceTarget( const TVector2<int32>& targetGrid, SOrganActionContext& actionContext )
{
	auto dGridPos = actionContext.target - actionContext.pCharacter->GetGrid();
	auto pCharacter = actionContext.pCurTarget;

	auto dPos = RotateDirInv( dGridPos, pCharacter->GetDir() );
	float r = atan2( dPos.y, dPos.x );
	CVector2 dir( dPos.x, dPos.y );
	dir.Normalize();

	auto pSubStage = pCharacter->GetSubStage();
	CVector2 targetPos = pSubStage->pFace->GetBaseOffset() + pSubStage->pFace->GetGridScale() * CVector2( targetGrid.x, targetGrid.y );
	CRectangle faceRect = pSubStage->pFace->GetFaceRect();
	float k = Max( dPos.x == 0 ? -10000.0f : ( dPos.x > 0 ? ( faceRect.GetLeft() - targetPos.x ) / dPos.x : ( faceRect.GetRight() - targetPos.x ) / dPos.x ),
		dPos.y == 0 ? -10000.0f : ( dPos.y > 0 ? ( faceRect.GetTop() - targetPos.y ) / dPos.y : ( faceRect.GetBottom() - targetPos.y ) / dPos.y ) );
	CVector2 srcPos = targetPos + CVector2( dPos.x, dPos.y ) * k;

	SetParentEntity( pSubStage->pFace );
	SetRotation( r );
	SetPosition( srcPos );
	SetVelocity( dir * GetSpeed() );
	SetActive( true );
}

void CBullet::TickBeforeHitTest()
{
	SetPosition( GetPosition() + m_velocity * GetStage()->GetElapsedTimePerTick() );
	CBulletBase::TickBeforeHitTest();
}

void CBullet::TickAfterHitTest()
{
	if( m_bActive )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			auto pOrgan = SafeCast<COrgan>( pEntity );
			if( pOrgan )
			{
				pOrgan->Damage( m_nDmg );
				Kill();
				return;
			}
		}
	}
	else
	{
		if( m_pEffectObject->GetParentEntity() != this )
		{
			m_pEffectObject = NULL;
			SetParentEntity( NULL );
			return;
		}
	}
	CBulletBase::TickAfterHitTest();
}

void CBullet::Kill()
{
	m_velocity = CVector2( 0, 0 );
	CBulletBase::Kill();
}

void CMissile::Emit( SOrganActionContext & actionContext )
{
	auto dGridPos = actionContext.target - actionContext.pCharacter->GetGrid();
	auto dPos = RotateDirInv( dGridPos, actionContext.pCharacter->GetDir() );
	float r = atan2( dPos.y, dPos.x );
	CVector2 dir( dPos.x, dPos.y );
	dir.Normalize();

	SetParentEntity( actionContext.pOrgan->GetParentEntity() );
	SetPosition( actionContext.pOrgan->GetPosition() );
	SetRotation( r );
	m_velocity = dir * m_fSpeed;
	m_bTarget = false;
}

void CMissile::SetFaceTarget( const TVector2<int32>& targetGrid, SOrganActionContext & actionContext )
{
	auto dGridPos = actionContext.target - actionContext.pCharacter->GetGrid();
	auto pCharacter = actionContext.pCurTarget;

	auto dPos = RotateDirInv( dGridPos, pCharacter->GetDir() );
	float r = atan2( dPos.y, dPos.x );
	CVector2 dir( dPos.x, dPos.y );
	dir.Normalize();

	auto pSubStage = pCharacter->GetSubStage();
	CVector2 targetPos = pSubStage->pFace->GetBaseOffset() + pSubStage->pFace->GetGridScale() * CVector2( targetGrid.x, targetGrid.y );
	CRectangle faceRect = pSubStage->pFace->GetFaceRect();
	float k = Max( dPos.x == 0 ? -10000.0f : ( dPos.x > 0 ? ( faceRect.GetLeft() - targetPos.x ) / dPos.x : ( faceRect.GetRight() - targetPos.x ) / dPos.x ),
		dPos.y == 0 ? -10000.0f : ( dPos.y > 0 ? ( faceRect.GetTop() - targetPos.y ) / dPos.y : ( faceRect.GetBottom() - targetPos.y ) / dPos.y ) );
	CVector2 srcPos = targetPos + CVector2( dPos.x, dPos.y ) * k;

	m_target = targetGrid;
	m_bTarget = true;
	SetParentEntity( pSubStage->pFace );
	SetPosition( srcPos );
	SetActive( true );
}

void CMissile::TickBeforeHitTest()
{
	if( m_bTarget )
	{
		if( !m_bHit )
		{
			CVector2 pos = GetPosition();
			CVector2 targetPos = m_pFace->GetGridScale() * CVector2( m_target.x, m_target.y ) + m_pFace->GetBaseOffset();
			CVector2 dPos = pos - targetPos;
			float l = dPos.Length();
			float l1 = Max( 0.0f, l - m_fSpeed * GetStage()->GetElapsedTimePerTick() );
			dPos = dPos * ( l1 / l );
			SetPosition( targetPos + dPos );
			if( l1 == 0 )
				m_bHit = true;
		}
	}
	else
	{
		SetPosition( GetPosition() + m_velocity * GetStage()->GetElapsedTimePerTick() );
	}

	CBulletBase::TickBeforeHitTest();
}

void CMissile::TickAfterHitTest()
{
	if( !m_bActive )
	{
		if( m_pEffectObject->GetParentEntity() != this )
		{
			m_pEffectObject = NULL;
			SetParentEntity( NULL );
			return;
		}
	}
	else
	{
		if( m_bTarget )
		{
			if( m_bHit )
			{
				vector<TVector2<int32> > grids;
				GetRange( m_eRangeType, m_nRange, m_nRange1, grids );

				vector<CReference<COrgan> > hitOrgans;
				for( int i = 0; i < grids.size(); i++ )
				{
					auto grid = grids[i] + m_target;
					auto pGrid = m_pFace->GetGrid( grid.x, grid.y );
					if( pGrid->pOrgan && m_bCanDmgOrgan )
					{
						if( !pGrid->pOrgan->m_nVisitFlag )
						{
							pGrid->pOrgan->m_nVisitFlag = 1;
							hitOrgans.push_back( pGrid->pOrgan );
							pGrid->pOrgan->Damage( m_nDmg );
						}
					}
					else
					{
						if( m_bCanDmgSkin )
							m_pFace->DamageSkin( m_nDmg, grid.x, grid.y );
					}
				}
				for( COrgan* pOrgan : hitOrgans )
				{
					pOrgan->m_nVisitFlag = 0;
				}

				Kill();
				return;
			}
		}
	}

	CBulletBase::TickAfterHitTest();
}
