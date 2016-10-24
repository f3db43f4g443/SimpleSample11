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

void COrganActionSimpleShoot::Action( CTurnBasedContext* pContext )
{
	auto& actionContext = *pContext->pActionContext;
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

	actionContext.pOrgan->ActionSelectTarget( pContext );
	
	for( auto pCharacter : actionContext.targetCharacters )
	{
		actionContext.pCurTarget = pCharacter;

		auto pSubStage = GetStage()->GetWorld()->GetSubStage( pCharacter->ShowSubStage( 1 ) );
		CStageDirector::Inst()->FocusFaceView( pCharacter->GetSubStageShowSlot(), pContext );
		TVector2<int32> targetGrid = actionContext.pCharacter->SelectTargetFaceGrid( pContext );

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

void COrganActionSimpleShoot::OnBeginFaceSelectTarget( const SOrganActionContext & actionContext )
{
	auto pBullet = SafeCast<CBulletBase>( (CRenderObject2D*)( m_pBulletPrefab->GetRoot()->GetObjData() ) );
	m_faceSelectGrid = TVector2<int32>( 0, 0 );
	pBullet->ShowRange( actionContext, m_faceSelectGrid, true );
}

void COrganActionSimpleShoot::OnFaceSelectTargetMove( const SOrganActionContext & actionContext, TVector2<int32> grid )
{
	if( grid == m_faceSelectGrid )
		return;

	auto pBullet = SafeCast<CBulletBase>( (CRenderObject2D*)( m_pBulletPrefab->GetRoot()->GetObjData() ) );
	pBullet->ShowRange( actionContext, m_faceSelectGrid, false );
	m_faceSelectGrid = grid;
	pBullet->ShowRange( actionContext, m_faceSelectGrid, true );
}

void COrganActionSimpleShoot::OnEndFaceSelectTarget( const SOrganActionContext & actionContext )
{
	auto pBullet = SafeCast<CBulletBase>( (CRenderObject2D*)( m_pBulletPrefab->GetRoot()->GetObjData() ) );
	pBullet->ShowRange( actionContext, m_faceSelectGrid, false );
}
