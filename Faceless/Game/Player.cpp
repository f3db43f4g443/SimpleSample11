#include "stdafx.h"
#include "Player.h"
#include "Stage.h"
#include "World.h"
#include "Common/ResourceManager.h"
#include "MyLevel.h"

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
}