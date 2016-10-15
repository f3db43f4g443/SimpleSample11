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
	{
		vector<TTempEntityHolder<CBulletBase> > bullets;
		bullets.resize( m_nCount );
		for( int i = 0; i < m_nCount; i++ )
		{
			CBulletBase* pBullet = SafeCast<CBulletBase>( m_pBulletPrefab->GetRoot()->CreateInstance() );
			bullets[i] = pBullet;
			pBullet->Emit( actionContext );
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

		auto pSubStage = GetStage()->GetWorld()->GetSubStage( pCharacter->ShowSubStage( 1 ) );
		CStageDirector::Inst()->FocusFaceView( pCharacter->GetSubStageShowSlot(), pContext );
		TVector2<int32> targetGrid = actionContext.pCharacter->SelectTargetFaceGrid( pContext, actionContext );

		{
			vector<TTempEntityHolder<CBulletBase> > bullets;
			bullets.resize( m_nCount );
			for( int i = 0; i < m_nCount; i++ )
			{
				CBulletBase* pBullet = SafeCast<CBulletBase>( m_pBulletPrefab->GetRoot()->CreateInstance() );
				bullets[i] = pBullet;
				pBullet->SetFaceTarget( targetGrid, actionContext );
				pContext->Yield( m_fInterval, false );
			}

			while( pSubStage->pFace->IsAwake() )
				pContext->Yield( 0, false );
		}

		CStageDirector::Inst()->FocusFaceView( -1, pContext );
		pCharacter->HideSubStage();
	}
}