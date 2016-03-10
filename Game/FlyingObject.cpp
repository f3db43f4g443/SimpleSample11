#include "stdafx.h"
#include "FlyingObject.h"
#include "Stage.h"

CFlyingObject::CFlyingObject()
	: m_tickBeforeHitTest( this, &CFlyingObject::OnTickBeforeHitTest )
	, m_bIsAlive( true )
{
	SetHitType( eEntityHitType_FlyingObject );
}

void CFlyingObject::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CFlyingObject::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CFlyingObject::Kill()
{
	if( !m_bIsAlive )
		return;
	
	m_bIsAlive = false;
	OnKilled();
}

void CFlyingObject::OnTickBeforeHitTest()
{
	UpdateAnim( GetStage()->GetFlyingObjectElapsedTime() );
	
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}