#include "stdafx.h"
#include "OrganActionSimpleShoot.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Stage.h"
#include "World.h"
#include "Face.h"
#include "GUI/StageDirector.h"
#include "ResourceManager.h"

void COrganActionSimpleShoot::OnAddedToStage()
{
	m_pBulletPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBulletPrefab.c_str() );
}

void COrganActionSimpleShoot::Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext )
{
	auto dGridPos = actionContext.target - actionContext.pCharacter->GetGrid();
	
	{
		auto dPos = CCharacter::RotateDir( dGridPos, actionContext.pCharacter->GetDir() );
		float r = atan2( dPos.y, dPos.x );
		CVector2 dir( dPos.x, dPos.y );
		dir.Normalize();

		vector<TTempEntityHolder<CBullet> > bullets;
		bullets.resize( m_nCount );
		for( int i = 0; i < m_nCount; i++ )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
			bullets[i] = pBullet;
			pBullet->SetParentEntity( GetParentEntity() );
			pBullet->SetPosition( GetPosition() );
			pBullet->SetRotation( r );
			pBullet->SetVelocity( dir * pBullet->GetSpeed() );
			pContext->Yield( m_fInterval, false );
		}

		auto pFace = actionContext.pOrgan->GetFace();
		while( pFace->IsAwake() )
			pContext->Yield( 0, false );
	}

	actionContext.pOrgan->ActionSelectTarget( pContext, actionContext );
	
	for( auto pCharacter : actionContext.targetCharacters )
	{
		actionContext.pCurTarget = pCharacter;
		auto dPos = CCharacter::RotateDir( dGridPos, pCharacter->GetDir() );
		float r = atan2( dPos.y, dPos.x );
		CVector2 dir( dPos.x, dPos.y );
		dir.Normalize();

		auto pSubStage = GetStage()->GetWorld()->GetSubStage( pCharacter->ShowSubStage( 1 ) );
		CStageDirector::Inst()->FocusFaceView( pCharacter->GetSubStageShowSlot(), pContext );

		TVector2<int32> targetGrid = actionContext.pCharacter->SelectTargetFaceGrid( pContext, actionContext );
		CVector2 targetPos = pSubStage->pFace->GetBaseOffset() + pSubStage->pFace->GetGridScale() * CVector2( targetGrid.x, targetGrid.y );
		CRectangle faceRect = pSubStage->pFace->GetFaceRect();
		float k = Max( dPos.x == 0 ? -10000.0f : ( dPos.x > 0 ? ( faceRect.GetLeft() - targetPos.x ) / dPos.x : ( faceRect.GetRight() - targetPos.x ) / dPos.x ),
			dPos.y == 0 ? -10000.0f : ( dPos.y > 0 ? ( faceRect.GetTop() - targetPos.y ) / dPos.y : ( faceRect.GetBottom() - targetPos.y ) / dPos.y ) );
		CVector2 srcPos = targetPos + CVector2( dPos.x, dPos.y ) * k;

		{
			vector<TTempEntityHolder<CBullet> > bullets;
			bullets.resize( m_nCount );
			for( int i = 0; i < m_nCount; i++ )
			{
				CBullet* pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
				bullets[i] = pBullet;
				pBullet->SetParentEntity( pSubStage->pFace );
				pBullet->SetRotation( r );
				pBullet->SetPosition( srcPos );
				pBullet->SetVelocity( dir * pBullet->GetSpeed() );
				pBullet->SetActive( true );
				pContext->Yield( m_fInterval, false );
			}

			while( pSubStage->pFace->IsAwake() )
				pContext->Yield( 0, false );
		}

		CStageDirector::Inst()->FocusFaceView( -1, pContext );
		pCharacter->HideSubStage();
	}

}