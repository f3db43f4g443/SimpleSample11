#include "stdafx.h"
#include "Character.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "World.h"
#include "Face.h"
#include "GUI/StageDirector.h"

CCharacter::CCharacter()
	: m_tickBeforeHitTest( this, &CCharacter::OnTick )
	, m_pLevel( NULL )
	, m_grid( 0, 0 )
	, m_nDir( 0 )
	, m_nDelay( 0 )
	, m_nCharacterStageID( 0 )
	, m_strSubStageName( "" )
	, m_nSubStage( INVALID_32BITID )
	, m_nSubStageShowSlot( INVALID_8BITID )
	, m_nMaxHp( 0 ), m_nMaxMp( 0 ), m_nMaxSp( 0 )
	, m_nHp( 0 ), m_nMp( 0 ), m_nSp( 0 )
{
	SET_BASEOBJECT_ID( CCharacter );
}

CCharacter::CCharacter( const SClassCreateContext& context )
	: CEntity( context )
	, m_tickBeforeHitTest( this, &CCharacter::OnTick )
	, m_pLevel( NULL )
	, m_grid( 0, 0 )
	, m_nDir( 0 )
	, m_nDelay( 0 )
	, m_nCharacterStageID( 0 )
	, m_nSubStage( INVALID_32BITID )
	, m_nSubStageShowSlot( INVALID_8BITID )
	, m_strSubStageName( context )
	, m_nHp( m_nMaxHp ), m_nMp( m_nMaxMp ), m_nSp( m_nMaxSp )
{
	SET_BASEOBJECT_ID( CCharacter );
}

void CCharacter::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );

	m_nSubStage = GetStage()->GetWorld()->PlaySubStage( this );
}

void CCharacter::OnRemovedFromStage()
{
	GetStage()->GetWorld()->StopSubStage( m_nSubStage );
	m_nSubStage = INVALID_32BITID;
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

uint8 CCharacter::ShowSubStage( uint8 nSlot )
{
	if( m_nSubStage == INVALID_32BITID || m_nSubStageShowSlot != INVALID_8BITID )
		return m_nSubStage;
	if( CStageDirector::Inst()->ShowSubStage( m_nSubStage, nSlot ) )
		m_nSubStageShowSlot = nSlot;
	return m_nSubStage;
}

void CCharacter::HideSubStage()
{
	if( m_nSubStageShowSlot == INVALID_8BITID )
		return;
	if( CStageDirector::Inst()->HideSubStage( m_nSubStageShowSlot ) )
		m_nSubStageShowSlot = INVALID_8BITID;
}

SSubStage * CCharacter::GetSubStage()
{
	return GetStage()->GetWorld()->GetSubStage( m_nSubStage );
}

bool CCharacter::MoveTo( uint32 gridX, uint32 gridY )
{
	return m_pLevel->MoveCharacter( this, gridX, gridY );
}

void CCharacter::Face( uint8 nDir )
{
	m_nDir = nDir;
	SetRotation( ( nDir - 3 ) * PI * 0.5f );
}

void CCharacter::SetDelay( uint16 nDelay )
{
	m_nDelay = nDelay;
	if( m_pLevel )
		m_pLevel->OnCharacterDelayChanged( this );
}

void CCharacter::OnTick()
{
	UpdateAnim( GetStage()->GetElapsedTimePerTick() );
	
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CCharacter::OnTurn( CTurnBasedContext* pContext )
{
	SetHp( m_nMaxHp, m_nMaxHp );
	SetMp( m_nMaxMp, m_nMaxMp );
	SetSp( m_nMaxSp, m_nMaxSp );

	MovePhase( pContext );
	EmotePhase( pContext );
	BattlePhase( pContext );

	SetDelay( 100 );
}

void CCharacter::MovePhase( CTurnBasedContext* pContext )
{
	Move( pContext, SRand::Inst().Rand( 0, 4 ) );
}

void CCharacter::EmotePhase( CTurnBasedContext* pContext )
{

}

void CCharacter::BattlePhase( CTurnBasedContext* pContext )
{

}

TVector2<int32> CCharacter::SelectTargetLevelGrid( CTurnBasedContext * pContext, SOrganActionContext & actionContext )
{
	return TVector2<int32>();
}

TVector2<int32> CCharacter::SelectTargetFaceGrid( CTurnBasedContext * pContext, SOrganActionContext& actionContext )
{
	return TVector2<int32>();
}

void CCharacter::SetHp( uint32 n, uint32 nMax )
{
	m_nHp = n;
	m_nMaxHp = nMax;
}

void CCharacter::SetMp( uint32 n, uint32 nMax )
{
	m_nMp = n;
	m_nMaxMp = nMax;
}

void CCharacter::SetSp( uint32 n, uint32 nMax )
{
	m_nSp = n;
	m_nMaxSp = nMax;
}

bool CCharacter::Move( CTurnBasedContext* pContext, uint8 nDir )
{
	if( m_nDir != nDir )
	{
		Face( nDir );
		return true;
	}
	else
	{
		auto newPos = GetDirOfs( nDir ) + m_grid;
		return MoveTo( newPos.x, newPos.y );
	}
}

bool CCharacter::UseFaceEditItem( CTurnBasedContext* pContext, CFaceEditItem* pItem, const TVector2<int32>& pos )
{
	if( pItem->nCost > m_nMp )
		return false;
	
	auto pSubStage = GetStage()->GetWorld()->GetSubStage( m_nSubStage );
	CFace* pFace = pSubStage->pFace;
	if( !pFace->IsEditValid( pItem, pos ) )
		return false;

	m_nMp -= pItem->nCost;
	pItem->Edit( this, pFace, pos );
	return true;
}