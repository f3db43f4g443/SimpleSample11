#include "stdafx.h"
#include "Character.h"
#include "Stage.h"

CCharacter::CCharacter()
	: m_tickBeforeHitTest( this, &CCharacter::OnTickBeforeHitTest )
{

}

void CCharacter::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CCharacter::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CCharacter::OnTickBeforeHitTest()
{
	UpdateAnim( GetStage()->GetGlobalElapsedTime() );
	
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}