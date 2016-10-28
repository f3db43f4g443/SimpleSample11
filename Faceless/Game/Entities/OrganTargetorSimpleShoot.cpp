#include "stdafx.h"
#include "OrganTargetorSimpleShoot.h"
#include "MyLevel.h"
#include "Stage.h"

void COrganTargetorSimpleShoot::FindTargets( CTurnBasedContext* pContext )
{
	auto& actionContext = *pContext->pActionContext;
	m_pEffectObject = GetChildByName_Fast<CEffectObject>( "" );
	m_pEffectObject->SetState( 1 );

	auto pCharacter = actionContext.pCharacter;
	auto pLevel = actionContext.pCharacter->GetLevel();
	auto src = actionContext.pCharacter->GetGrid();
	auto target = actionContext.target;
	auto delta = target - src;
	if( !delta.x && !delta.y )
		return;
	SetPosition( pCharacter->GetPosition() );
	SetRotation( atan2( delta.y, delta.x ) );

	CCharacter* pFindChar = NULL;
	pLevel->RayCast( src, target, [pLevel, &pFindChar, &pCharacter] ( const TVector2<int32>& pos )
	{
		auto pChar = pLevel->GetGrid( pos.x, pos.y )->pCharacter;
		if( pChar && pChar != pCharacter )
		{
			pFindChar = pChar;
			return false;
		}
		return true;
	} );

	CVector2 dPos( delta.x, delta.y );
	dPos = dPos * pLevel->GetGridScale();
	float dist = dPos.Length();
	float fMaxDist = dist;

	if( pFindChar )
	{
		auto n = dPos;
		n.Normalize();
		fMaxDist = Min( dist, n.Dot( pFindChar->GetPosition() - GetPosition() ) );
	}

	float fDist1 = 0;
	while( fDist1 < fMaxDist )
	{
		pContext->Yield( 0, false );
		fDist1 = Min( fDist1 + m_fFlyingSpeed * GetStage()->GetElapsedTimePerTick(), dist );
		CVector2 newPos( src.x, src.y );
		newPos = newPos * pLevel->GetGridScale() + dPos * ( fDist1 / dist ) + pLevel->GetBaseOffset();
		SetPosition( newPos );
	}

	if( m_pEffectObject )
		m_pEffectObject->SetState( 2 );
	if( pFindChar )
		FindTarget( pFindChar, pContext );
	if( m_pEffectObject )
	{
		while( m_pEffectObject->GetParentEntity() == this )
			pContext->Yield( 0, false );
		m_pEffectObject = NULL;
	}
}

void COrganTargetorSimpleShoot::OnBeginSelectTarget( SOrganActionContext & actionContext )
{
}

void COrganTargetorSimpleShoot::OnSelectTargetMove( SOrganActionContext & actionContext, TVector2<int32> grid )
{
}

void COrganTargetorSimpleShoot::OnEndSelectTarget( SOrganActionContext & actionContext )
{
}
