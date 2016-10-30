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

void COrganTargetorSimpleShoot::ShowSelectTarget( SOrganActionContext& actionContext, bool bShow )
{
	auto pLevel = CMyLevel::GetInst();
	auto pTileMap = pLevel->GetSelectTile();

	vector<TVector2<int32> > range;
	actionContext.pOrgan->GetRange( range );
	for( auto& pos : range )
	{
		pos = RotateDir( pos, actionContext.pCharacter->GetDir() ) + actionContext.pCharacter->GetGrid();
		if( pos.x < 0 || pos.x >= pTileMap->GetWidth() || pos.y < 0 || pos.y >= pTileMap->GetHeight() )
			continue;
		if( bShow )
			pTileMap->AddTileLayer( pos.x, pos.y, CMyLevel::eSelectTile_InRange );
		else
			pTileMap->SetTile( pos.x, pos.y, 0, NULL );
	}

	bool InRange = true;
	if( m_selectGrid.x < 0 || m_selectGrid.x >= pTileMap->GetWidth() || m_selectGrid.y < 0 || m_selectGrid.y >= pTileMap->GetHeight() )
		InRange = false;
	if( !actionContext.pOrgan->IsInRange( RotateDirInv( m_selectGrid - actionContext.pCharacter->GetGrid(), actionContext.pCharacter->GetDir() ) ) )
		InRange = false;
	if( InRange )
	{
		auto pCharacter = actionContext.pCharacter;
		pLevel->RayCast( actionContext.pCharacter->GetGrid(), m_selectGrid, [pLevel, pTileMap, pCharacter, bShow] ( const TVector2<int32>& pos )
		{
			auto pChar = pLevel->GetGrid( pos.x, pos.y )->pCharacter;
			if( pChar == pCharacter )
				return true;
			if( bShow )
				pTileMap->AddTileLayer( pos.x, pos.y, CMyLevel::eSelectTile_InEffectRange );
			else
				pTileMap->SetTile( pos.x, pos.y, 0, NULL );

			if( pChar )
				return false;
			return true;
		} );
	}

	if( m_selectGrid.x < 0 || m_selectGrid.x >= pTileMap->GetWidth() || m_selectGrid.y < 0 || m_selectGrid.y >= pTileMap->GetHeight() )
		return;
	if( bShow )
		pTileMap->AddTileLayer( m_selectGrid.x, m_selectGrid.y, InRange ? CMyLevel::eSelectTile_TargetValid : CMyLevel::eSelectTile_TargetInvalid );
	else
		pTileMap->SetTile( m_selectGrid.x, m_selectGrid.y, 0, NULL );
}