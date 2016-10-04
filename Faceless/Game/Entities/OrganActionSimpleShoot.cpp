#include "stdafx.h"
#include "OrganActionSimpleShoot.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Stage.h"
#include "World.h"
#include "Face.h"
#include "GUI/StageDirector.h"

void COrganActionSimpleShoot::Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext )
{
	auto dGridPos = actionContext.target - actionContext.pCharacter->GetGrid();
	
	uint32 nRemoved = 0;
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
			pBullet->SetParentEntity( this );
			pBullet->SetRotation( r );
			pBullet->SetVelocity( dir * pBullet->GetSpeed() );
			auto pTrigger = new CFunctionTrigger;
			pTrigger->bAutoDelete = true;
			pTrigger->Set( [pTrigger, &nRemoved] () {
				nRemoved++;
				pTrigger->Unregister();
			} );
			pBullet->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pTrigger );
			pContext->Yield( m_fInterval, false );
		}
	}
	while( nRemoved < m_nCount )
		pContext->Yield( 0, false );

	if( !actionContext.pOrgan->ActionSelectTarget( pContext, actionContext ) )
		return;
	
	for( auto pCharacter : actionContext.targetCharacters )
	{
		actionContext.pCurTarget = pCharacter;
		auto dPos = CCharacter::RotateDir( dGridPos, pCharacter->GetDir() );
		float r = atan2( dPos.y, dPos.x );
		CVector2 dir( dPos.x, dPos.y );
		dir.Normalize();

		auto pSubStage = GetStage()->GetWorld()->GetSubStage( pCharacter->ShowSubStage( 1 ) );
		CStageDirector::Inst()->FocusFaceView( pCharacter->GetSubStageShowSlot(), pContext );

		TVector2<int32> targetGrid = pCharacter->SelectTargetFaceGrid( pContext, actionContext );
		CVector2 targetPos = pSubStage->pFace->GetBaseOffset() + pSubStage->pFace->GetGridScale() * CVector2( targetGrid.x, targetGrid.y );
		CRectangle faceRect = pSubStage->pFace->GetFaceRect();
		float k = Max( dPos.x == 0 ? -10000.0f : ( dPos.x > 0 ? ( faceRect.GetLeft() - targetPos.x ) / dPos.x : ( faceRect.GetRight() - targetPos.x ) / dPos.x ),
			dPos.y == 0 ? -10000.0f : ( dPos.y > 0 ? ( faceRect.GetTop() - targetPos.y ) / dPos.y : ( faceRect.GetBottom() - targetPos.y ) / dPos.y ) );
		CVector2 srcPos = targetPos + CVector2( dPos.x, dPos.y ) * k;

		uint32 nRemoved = 0;
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
				auto pTrigger = new CFunctionTrigger;
				pTrigger->bAutoDelete = true;
				pTrigger->Set( [pTrigger, &nRemoved]() {
					nRemoved++;
					pTrigger->Unregister();
				} );
				pBullet->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pTrigger );
				pContext->Yield( m_fInterval, false );
			}

			while( nRemoved < m_nCount )
				pContext->Yield( 0, false );
			pContext->Yield( 1, false );
		}

		CStageDirector::Inst()->FocusFaceView( -1, pContext );
		pCharacter->HideSubStage();
	}

}