#include "stdafx.h"
#include "Organ.h"
#include "Face.h"
#include "Character.h"
#include "MyLevel.h"

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

	if( m_nTargetType == eTargetType_Cbaracter )
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
	TTempEntityHolder<COrganAction> pAction = SafeCast<COrganAction>( m_pOrganActionPrefab->GetRoot()->CreateInstance() );
	pAction->SetParentEntity( this );
	pAction->Action( pContext, actionContext );
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
