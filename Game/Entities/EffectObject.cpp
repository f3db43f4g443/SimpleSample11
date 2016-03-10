#include "stdafx.h"
#include "EffectObject.h"
#include "Stage.h"

CEffectObject::CEffectObject( float fTime, uint8 nType )
	: m_tickBeforeHitTest( this, &CEffectObject::OnTickBeforeHitTest )
	, m_fTimeLeft( fTime )
	, m_nType( nType )
{

}

void CEffectObject::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CEffectObject::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CEffectObject::OnTickBeforeHitTest()
{
	float fElapsedTime;
	switch( m_nType )
	{
	case eType_Character:
		fElapsedTime = GetStage()->GetGlobalElapsedTime();
		break;
	case eType_FlyingObject:
		fElapsedTime = GetStage()->GetFlyingObjectElapsedTime();
		break;
	default:
		fElapsedTime = GetStage()->GetElapsedTimePerTick();
		break;
	}
	m_fTimeLeft -= fElapsedTime;
	if( m_fTimeLeft <= 0 )
	{
		SetParentEntity( NULL );
		return;
	}

	UpdateAnim( fElapsedTime );
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}