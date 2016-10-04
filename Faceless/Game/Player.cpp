#include "stdafx.h"
#include "Player.h"
#include "Stage.h"
#include "World.h"
#include "Common/ResourceManager.h"
#include "MyLevel.h"
#include "Organ.h"
#include "Face.h"

CPlayer::CPlayer( const SClassCreateContext& context )
	: CCharacter( context )
{
	SET_BASEOBJECT_ID( CPlayer );
}

void CPlayer::OnTick()
{
}

void CPlayer::OnAddedToStage()
{
	//GetStage()->GetWorld()->PlaySubStage( this, 0 );
}

void CPlayer::OnRemovedFromStage()
{
	GetStage()->GetWorld()->StopSubStage( 0 );
}

struct SPlayerCommandMove
{
	uint8 nDir;
};

struct SPlayerCommandEndPhase
{

};

void CPlayer::PlayerCommandEndPhase()
{
	SPlayerCommandEndPhase cmd;
	PlayerCommand( ePlayerCommand_EndPhase, &cmd );
}

void CPlayer::PlayerCommandMove( uint8 nDir )
{
	SPlayerCommandMove cmd;
	cmd.nDir = nDir;
	PlayerCommand( ePlayerCommand_Move, &cmd );
}

void CPlayer::MovePhase( CTurnBasedContext* pContext )
{
	while( 1 )
	{
		try
		{
			CMessagePump msg( pContext );
			m_onPlayerCommand.Register( ePlayerCommand_EndPhase, msg.Register<SPlayerCommandEndPhase*>() );
			m_onPlayerCommand.Register( ePlayerCommand_Move, msg.Register<SPlayerCommandMove*>() );
			pContext->Yield();
		}
		catch( SPlayerCommandEndPhase* pEndPhase )
		{
			break;
		}
		catch( SPlayerCommandMove* pPlayerMove )
		{
			Move( pContext, pPlayerMove->nDir );
		}
	}
}

struct SPlayerCommandFaceEditItem
{
	CFaceEditItem* pItem;
	TVector2<int32> pos;
	bool bResult;
};

bool CPlayer::PlayerCommandFaceEditItem( CFaceEditItem* pItem, const TVector2<int32>& pos )
{
	SPlayerCommandFaceEditItem cmd;
	cmd.pItem = pItem;
	cmd.pos = pos;
	cmd.bResult = false;
	PlayerCommand( ePlayerCommand_FaceEdit, &cmd );
	return cmd.bResult;
}

void CPlayer::EmotePhase( CTurnBasedContext* pContext )
{
	CStageDirector::Inst()->SetFaceViewState( GetSubStageShowSlot(), CFaceView::eState_Edit );

	while( 1 )
	{
		try
		{
			CMessagePump msg( pContext );
			m_onPlayerCommand.Register( ePlayerCommand_EndPhase, msg.Register<SPlayerCommandEndPhase*>() );
			m_onPlayerCommand.Register( ePlayerCommand_FaceEdit, msg.Register<SPlayerCommandFaceEditItem*>() );
			pContext->Yield();
		}
		catch( SPlayerCommandEndPhase* pEndPhase )
		{
			break;
		}
		catch( SPlayerCommandFaceEditItem* pFaceEdit )
		{
			if( UseFaceEditItem( pContext, pFaceEdit->pItem, pFaceEdit->pos ) )
				pFaceEdit->bResult = true;
		}
	}
	CStageDirector::Inst()->SetFaceViewState( GetSubStageShowSlot(), CFaceView::eState_None );
}

struct SPlayerCommandAction
{
	COrgan* pOrgan;

	bool bResult;
};

bool CPlayer::PlayerCommandAction( COrgan * pOrgan )
{
	SPlayerCommandAction cmd;
	cmd.pOrgan = pOrgan;
	cmd.bResult = false;
	PlayerCommand( ePlayerCommand_Action, &cmd );
	return cmd.bResult;
}

void CPlayer::BattlePhase( CTurnBasedContext * pContext )
{
	CStageDirector::Inst()->SetFaceViewState( GetSubStageShowSlot(), CFaceView::eState_Action );

	while( 1 )
	{
		try
		{
			CMessagePump msg( pContext );
			m_onPlayerCommand.Register( ePlayerCommand_EndPhase, msg.Register<SPlayerCommandEndPhase*>() );
			m_onPlayerCommand.Register( ePlayerCommand_Action, msg.Register<SPlayerCommandAction*>() );
			pContext->Yield();
		}
		catch( SPlayerCommandEndPhase* pEndPhase )
		{
			break;
		}
		catch( SPlayerCommandAction* pAction )
		{
			auto pSubStage = GetSubStage();
			CFace* pFace = pSubStage->pFace;
			SOrganActionContext actionContext;
			if( !pAction->pOrgan->CanAction( actionContext ) )
				continue;

			pAction->bResult = true;
			CStageDirector::Inst()->SetFaceViewState( GetSubStageShowSlot(), CFaceView::eState_None );
			pAction->pOrgan->Action( pContext, actionContext );
			CStageDirector::Inst()->SetFaceViewState( GetSubStageShowSlot(), CFaceView::eState_Action );
		}
	}
	CStageDirector::Inst()->SetFaceViewState( GetSubStageShowSlot(), CFaceView::eState_None );
}

struct SPlayerCommandSelectTargetLevelGrid
{
	TVector2<int32> grid;
};

void CPlayer::PlayerCommandSelectTargetLevelGrid( const TVector2<int32>& grid )
{
	SPlayerCommandSelectTargetLevelGrid cmd;
	cmd.grid = grid;
	PlayerCommand( ePlayerCommand_SelectTargetLevelGrid, &cmd );
}

bool CPlayer::SelectTargetLevelGrid( CTurnBasedContext * pContext, SOrganActionContext & actionContext )
{
	uint8 nState = CStageDirector::Inst()->GetState();
	CStageDirector::Inst()->SetState( CStageDirector::eState_SelectTarget );

	bool bSucceed = false;
	while( 1 )
	{
		try
		{
			CMessagePump msg( pContext );
			m_onPlayerCommand.Register( ePlayerCommand_EndPhase, msg.Register<SPlayerCommandEndPhase*>() );
			m_onPlayerCommand.Register( ePlayerCommand_SelectTargetFaceGrid, msg.Register<SPlayerCommandSelectTargetLevelGrid*>() );
			pContext->Yield();
		}
		catch( SPlayerCommandEndPhase* pEndPhase )
		{
			bSucceed = false;
			break;
		}
		catch( SPlayerCommandSelectTargetLevelGrid* pSelectGrid )
		{
			actionContext.target = pSelectGrid->grid;
			if( actionContext.pOrgan->CheckActionTarget( actionContext ) )
			{
				bSucceed = true;
				break;
			}
		}
	}

	CStageDirector::Inst()->SetState( nState );
	return bSucceed;
}

struct SPlayerCommandSelectTargetFaceGrid
{
	TVector2<int32> grid;
};

void CPlayer::PlayerCommandSelectTargetFaceGrid( const TVector2<int32>& grid )
{
	SPlayerCommandSelectTargetFaceGrid cmd;
	cmd.grid = grid;
	PlayerCommand( ePlayerCommand_SelectTargetFaceGrid, &cmd );
}

TVector2<int32> CPlayer::SelectTargetFaceGrid( CTurnBasedContext * pContext, SOrganActionContext & actionContext )
{
	CStageDirector::Inst()->SetFaceViewState( actionContext.pCurTarget->GetSubStageShowSlot(), CFaceView::eState_SelectFaceTarget );

	TVector2<int32> res( 0, 0 );
	while( 1 )
	{
		try
		{
			CMessagePump msg( pContext );
			m_onPlayerCommand.Register( ePlayerCommand_SelectTargetFaceGrid, msg.Register<SPlayerCommandSelectTargetFaceGrid*>() );
			pContext->Yield();
		}
		catch( SPlayerCommandSelectTargetFaceGrid* pSelectGrid )
		{
			res = pSelectGrid->grid;
			break;
		}
	}
	CStageDirector::Inst()->SetFaceViewState( actionContext.pCurTarget->GetSubStageShowSlot(), CFaceView::eState_None );
	return res;
}
