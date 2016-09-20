#include "stdafx.h"
#include "Door.h"
#include "Stage.h"

CDoor::CDoor( const SClassCreateContext& context )
	: CEntity( context )
	, m_onUse( this, &CDoor::OnUse )
	, m_tickBeforeHitTest( this, &CDoor::OnTickBeforeHitTest )
{

}

void CDoor::OnAddedToStage()
{
	m_pObj1 = GetChildByName<CEntity>( "1" );
	m_pObj2 = GetChildByName<CEntity>( "2" );
	m_pHit = GetChildByName<CUseable>( "hit" );
	
	m_fDoorSize = m_pObj2->x;
	m_pHit->RegisterEntityEvent( eEntityEvent_PlayerUse, &m_onUse );
}

void CDoor::OnRemovedFromStage()
{
	if( m_onUse.IsRegistered() )
		m_onUse.Unregister();
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CDoor::OnTickBeforeHitTest()
{
	float fTime = GetStage()->GetGlobalElapsedTime();
	m_pObj1->x -= fTime * m_fDoorSize * 2;
	m_pObj2->x += fTime * m_fDoorSize * 2;
	m_pObj1->SetTransformDirty();
	m_pObj2->SetTransformDirty();
	if( m_pObj2->x >= m_fDoorSize * 3 )
	{
		m_pObj1->x = -m_fDoorSize * 3;
		m_pObj2->x = m_fDoorSize * 3;
		return;
	}
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CDoor::OnUse()
{
	m_pHit->SetEnabled( false );
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}