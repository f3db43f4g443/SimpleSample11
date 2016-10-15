#include "stdafx.h"
#include "Organ.h"
#include "Face.h"
#include "Character.h"
#include "MyLevel.h"
#include "GUI/StageDirector.h"
#include "ResourceManager.h"
#include "Image2D.h"

void COrgan::OnAddedToStage()
{
	if( m_nFramesRowCount > 1 )
	{
		CMultiFrameImage2D* pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		pImage->SetFrames( 0, m_nFramesColumnCount, pImage->GetData()->fFramesPerSec );
	}

	m_pOrganActionPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strOrganAction.c_str() );
	m_pOrganTargetorPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strOrganTargetor.c_str() );
}

void COrgan::OnRemovedFromStage()
{
	ShowHpBar( NULL );
}

void COrgan::GetRange( vector<TVector2<int32> >& result )
{
	::GetRange( m_nRangeType, m_nRange, m_nRange1, result );
}

bool COrgan::IsInRange( const TVector2<int32>& pos )
{
	return ::IsInRange( m_nRangeType, m_nRange, m_nRange1, pos );
}

bool COrgan::CanAction( SOrganActionContext& actionContext )
{
	if( !m_pOrganActionPrefab )
		return false;
	CCharacter* pCharacter = actionContext.pCharacter;
	if( pCharacter->GetSp() < m_nCost )
		return false;
	return true;
}

bool COrgan::CheckActionTarget( SOrganActionContext & actionContext )
{
	CCharacter* pCharacter = actionContext.pCharacter;

	auto ofs = actionContext.target - pCharacter->GetGrid();
	ofs = RotateDirInv( ofs, pCharacter->GetDir() );
	if( !IsInRange( ofs ) )
		return false;

	if( m_nTargetType == eTargetType_Character )
	{
		auto pGrid = pCharacter->GetLevel()->GetGrid( actionContext.target.x, actionContext.target.y );
		if( !pGrid )
			return false;
		if( !pGrid->pCharacter )
			return false;
	}

	return true;
}

void COrgan::Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext )
{
	uint8 nState = CStageDirector::Inst()->GetState();
	CStageDirector::Inst()->SetState( CStageDirector::eState_Locked );

	CStageDirector::Inst()->FocusFaceView( -1, pContext );

	actionContext.bSucceed = true;
	if( m_nTargetType != eTargetType_None )
	{
		if( !actionContext.pCharacter->SelectTargetLevelGrid( pContext, actionContext ) )
			actionContext.bSucceed = false;
		else if( !actionContext.pOrgan->CheckActionTarget( actionContext ) )
			actionContext.bSucceed = false;
	}
	else
	{
		actionContext.target = actionContext.pCharacter->GetGrid();
	}

	if( actionContext.bSucceed )
	{
		actionContext.pCharacter->SetSp( actionContext.pCharacter->GetSp() - actionContext.pOrgan->GetCost() );

		TTempEntityHolder<COrganAction> pAction = SafeCast<COrganAction>( m_pOrganActionPrefab->GetRoot()->CreateInstance() );
		pAction->SetParentEntity( this );
		pAction->Action( pContext, actionContext );
	}

	CStageDirector::Inst()->SetState( nState );
}

void COrgan::ActionSelectTarget( CTurnBasedContext * pContext, SOrganActionContext & actionContext )
{
	TTempEntityHolder<COrganTargetor> pTargetor = SafeCast<COrganTargetor>( actionContext.pOrgan->GetTargetorPrefab()->GetRoot()->CreateInstance() );
	pTargetor->SetParentEntity( actionContext.pCharacter->GetParentEntity() );
	pTargetor->FindTargets( pContext, actionContext );
}

void COrgan::ActionSelectTarget( CTurnBasedContext * pContext, SOrganActionContext & actionContext, COrganTargetor::FuncOnFindTarget func )
{
	TTempEntityHolder<COrganTargetor> pTargetor = SafeCast<COrganTargetor>( actionContext.pOrgan->GetTargetorPrefab()->GetRoot()->CreateInstance() );
	pTargetor->SetParentBeforeEntity( actionContext.pCharacter->GetParentEntity() );
	pTargetor->SetFindTargetFunc( func );
	pTargetor->FindTargets( pContext, actionContext );
}

void COrgan::SetHp( uint32 nHp )
{
	uint32 nRow = Min( m_nFramesRowCount - 1, m_nHp ? ( m_nHp * m_nFramesRowCount - 1 ) / m_nMaxHp : 0 );
	m_nHp = nHp;
	uint32 nRow1 = Min( m_nFramesRowCount - 1, m_nHp ? ( m_nHp * m_nFramesRowCount - 1 ) / m_nMaxHp : 0 );

	if( m_nFramesRowCount > 1 )
	{
		CMultiFrameImage2D* pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		if( nRow1 != nRow )
			pImage->SetFrames( ( m_nFramesRowCount - 1 - nRow1 ) * m_nFramesColumnCount, ( m_nFramesRowCount - nRow1 ) * m_nFramesColumnCount, pImage->GetData()->fFramesPerSec );
	}

	Trigger_OnHpChanged( this );
}

void COrgan::Damage( uint32 nDmg )
{
	SetHp( Max( 0, (int32)( m_nHp - nDmg ) ) );

	if( !m_nHp )
	{
		m_pFace->KillOrgan( this );
		return;
	}

	ShowHpBar( true );
}

void COrgan::ShowHpBar( bool bShown )
{
	if( bShown )
	{
		if( !m_pHpBar )
		{
			COrganHpBar* pHpBar = SafeCast<COrganHpBar>( CResourceManager::Inst()->CreateResource<CPrefab>( "data/misc/hpbar_organ.pf" )->GetRoot()->CreateInstance() );
			m_pHpBar = pHpBar;
			pHpBar->SetOrgan( this );
			pHpBar->SetParentEntity( m_pFace->GetGUIRoot() );
		}
	}
	else
	{
		if( m_pHpBar )
		{
			m_pHpBar->SetOrgan( NULL );
			m_pHpBar->SetParentEntity( NULL );
			m_pHpBar = NULL;
		}
	}
}

bool COrganEditItem::IsValidGrid( CFace* pFace, const TVector2<int32>& pos )
{
	auto pGrid = pFace->GetGrid( pos.x, pos.y );
	if( !pGrid )
		return false;
	return pGrid->bEnabled && !pGrid->pOrgan;
}

void COrganEditItem::Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos )
{
	auto pOrgan = SafeCast<COrgan>( pPrefab->GetRoot()->CreateInstance() );
	pOrgan->m_nWidth = nWidth;
	pOrgan->m_nHeight = nHeight;
	pFace->AddOrgan( pOrgan, pos.x, pos.y );
}
