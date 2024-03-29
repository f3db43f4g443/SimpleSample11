#include "stdafx.h"
#include "Entity.h"
#include "Stage.h"
#include "Animation.h"
#include "Player.h"
#include "Scene2DManager.h"

CEntity::~CEntity()
{
	while( m_pChildrenEntity )
		m_pChildrenEntity->SetParentEntity( NULL );
}

void CEntity::OnAdded()
{
	CEntity* pEntity = dynamic_cast<CEntity*>( GetParent() );
	if( pEntity )
	{
		pEntity->Insert_ChildEntity( this );
		m_pParent = pEntity;
	}
}

void CEntity::OnRemoved()
{
	if( m_pParent )
	{
		RemoveFrom_ChildEntity();
		m_pParent = NULL;
	}
}

void CEntity::_setParentEntity( CEntity* pParent, CEntity* pAfter, CEntity* pBefore )
{
	if( pParent == GetParentEntity() )
		return;

	CReference<CEntity> pTempRef( this );
	CStage* pOldStage = m_pCurStage;
	CStage* pNewStage = pParent && !pParent->m_bIsChangingStage? pParent->m_pCurStage: NULL;

	if( pOldStage && pOldStage != pNewStage )
		pOldStage->RemoveEntity( this );
	RemoveThis();
	if( pParent )
	{
		if( pAfter )
			pParent->AddChildAfter( this, pAfter );
		else if( pBefore )
			pParent->AddChildBefore( this, pBefore );
		else
			pParent->AddChild( this );
	}
	if( pNewStage && pNewStage != pOldStage )
		pNewStage->AddEntity( this );
}

void CEntity::SetRenderObject( CRenderObject2D* pRenderObject )
{
	if( m_pRenderObject == pRenderObject )
		return;
	if( m_pRenderObject )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( m_pRenderObject.GetPtr() );
		if( pEntity )
			pEntity->SetParentEntity( NULL );
		else
			m_pRenderObject->RemoveThis();
		m_pRenderObject = NULL;
	}
	m_pRenderObject = pRenderObject;
	if( pRenderObject )
	{
		pRenderObject->SetZOrder( -1 );
		CEntity* pEntity = dynamic_cast<CEntity*>( pRenderObject );
		if( pEntity )
			pEntity->SetParentEntity( this );
		else
			AddChild( pRenderObject );
		pRenderObject->SetZOrder( 0 );
	}
}

uint32 CEntity::BeforeHitTest( uint32 nTraverseIndex )
{
	CRenderObject2D* pRenderObject = m_pChildren;
	for( ; pRenderObject; pRenderObject = pRenderObject->NextChild() )
	{
		if( pRenderObject->GetZOrder() < 0 )
			break;
		CEntity* pEntity = dynamic_cast<CEntity*>( pRenderObject );
		if( pEntity )
			nTraverseIndex = pEntity->BeforeHitTest( nTraverseIndex );
	}
	m_nTraverseIndex = nTraverseIndex++;
	for( ; pRenderObject; pRenderObject = pRenderObject->NextChild() )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( pRenderObject );
		if( pEntity )
			nTraverseIndex = pEntity->BeforeHitTest( nTraverseIndex );
	}
	return nTraverseIndex;
}

bool CEntity::CommonMove( float fMoveSpeed, float fTime, const CVector2& dPosition, float fMinDist )
{
	if( dPosition.Dot( dPosition ) > fMinDist * fMinDist )
	{
		float fMoveDist = fMoveSpeed * fTime;
		CVector2 dPos = dPosition;
		float l = dPos.Normalize();
		dPos = dPos * Min( fMoveDist, l );
		SetPosition( GetPosition() + dPos );
		return l >= fMoveDist;
	}
	return false;
}

bool CEntity::CommonMove( float fMoveSpeed, float fTurnSpeed, float fTime, const CVector2& dPosition, float fMinDist, float& dRotation )
{
	float fRotation = GetRotation();
	float fTargetAngle = atan2( -dPosition.x, dPosition.y );
	dRotation = fRotation - fTargetAngle;
	dRotation = ( dRotation + PI ) / ( PI * 2 );
	dRotation = ( dRotation - floor( dRotation ) ) * ( PI * 2 ) - PI;
	float fTurnAngle = fTurnSpeed * fTime;
	if( dRotation > 0 )
	{
		dRotation -= fTurnAngle;
		if( dRotation < 0 )
			dRotation = 0;
		SetRotation( fTargetAngle + dRotation );
	}
	else if( dRotation < 0 )
	{
		dRotation += fTurnAngle;
		if( dRotation > 0 )
			dRotation = 0;
		SetRotation( fTargetAngle + dRotation );
	}

	return CommonMove( fMoveSpeed, fTime, dPosition, fMinDist );
}

void CEntity::CommonTurn( float fTurnSpeed, float fTime, float fTargetAngle, float& dRotation )
{
	float fRotation = GetRotation();
	dRotation = fRotation - fTargetAngle;
	dRotation = ( dRotation + PI ) / ( PI * 2 );
	dRotation = ( dRotation - floor( dRotation ) ) * ( PI * 2 ) - PI;
	float fTurnAngle = fTurnSpeed * fTime;
	if( dRotation > 0 )
	{
		dRotation -= fTurnAngle;
		if( dRotation < 0 )
			dRotation = 0;
		SetRotation( fTargetAngle + dRotation );
	}
	else if( dRotation < 0 )
	{
		dRotation += fTurnAngle;
		if( dRotation > 0 )
			dRotation = 0;
		SetRotation( fTargetAngle + dRotation );
	}
}

void CEntity::FixPosition()
{
	CVector2 posFix( 0, 0 );
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity )
		{
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				posFix = posFix - pManifold->normal;
			}
		}
	}
	if( !( posFix == CVector2( 0, 0 ) ) )
		SetPosition( GetPosition() + posFix );
}

void CEntity::FixPositionAndCheckPlayerDizzy( CPlayer* pPlayer, SPlayerDizzyContext& result )
{
	CVector2 posFix( 0, 0 );
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity )
		{
			SPlayerDizzyContext context;
			pEntity->CheckPlayerDizzy( pPlayer, pManifold->pOther->hitPoint, pManifold->pOther->normal, context );
			result.Add( context );

			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				posFix = posFix - pManifold->normal;
			}
		}
	}
	if( !( posFix == CVector2( 0, 0 ) ) )
	{
		SetPosition( GetPosition() + posFix );
		CScene2DManager::GetGlobalInst()->UpdateDirty();
	}
}

void CEntity::OnPlayerAttack( SPlayerAttackContext& context )
{
	context.pTarget = this;
	m_trigger.Trigger( eEntityEvent_PlayerAttack, &context );
}

void CEntity::OnTransformUpdated()
{
	if( m_pCurStage )
		SetDirty();
}