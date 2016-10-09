#include "stdafx.h"
#include "Organ.h"
#include "Face.h"
#include "Character.h"
#include "MyLevel.h"
#include "GUI/StageDirector.h"
#include "ResourceManager.h"

void COrgan::OnAddedToStage()
{
	m_pOrganActionPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strOrganAction.c_str() );
	m_pOrganTargetorPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strOrganTargetor.c_str() );
}

void COrgan::GetRange( vector<TVector2<int32> >& result )
{
	switch( m_nRangeType )
	{
	case eRangeType_Normal:
		for( int32 i = -(int32)m_nRange; i <= m_nRange; i++ )
		{
			result.push_back( TVector2<int32>( i, 0 ) );
			int32 nRange1 = m_nRange * m_nRange - i * i;
			for( int j = 1; j * j <= nRange1; j++ )
			{
				result.push_back( TVector2<int32>( i, -j ) );
			}
		}
		break;
	default:
		break;
	}
}

bool COrgan::IsInRange( const TVector2<int32>& pos )
{
	switch( m_nRangeType )
	{
	case eRangeType_Normal:
		return pos.x * pos.x + pos.y * pos.y <= m_nRange * m_nRange;
	default:
		break;
	}
	return false;
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

void COrgan::Damage( uint32 nDmg )
{
	m_nHp = Max( 0, (int32)( m_nHp - nDmg ) );
	
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
