#include "stdafx.h"
#include "EffectObject.h"
#include "Stage.h"
#include "Render/Canvas.h"
#include "Common/ResourceManager.h"

CEffectObject::CEffectObject( const SClassCreateContext& context )
	: CEntity( context )
	, m_tickBeforeHitTest( this, &CEffectObject::OnTickBeforeHitTest )
{
	SET_BASEOBJECT_ID( CEffectObject );
}

void CEffectObject::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	m_pStates[0] = GetChildByName<CEntity>( "birth" );
	m_pStates[1] = GetChildByName<CEntity>( "stand" );
	m_pStates[2] = GetChildByName<CEntity>( "death" );
	m_fTimeLeft = m_fBirthTime;
	m_nState = 0;
}

void CEffectObject::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CEffectObject::OnTickBeforeHitTest()
{
	float fElapsedTime = GetStage()->GetElapsedTimePerTick();
	if( m_fTimeLeft > 0 )
	{
		m_fTimeLeft -= fElapsedTime;
		if( m_fTimeLeft <= 0 )
		{
			if( m_nState == 2 )
			{
				SetParentEntity( NULL );
				return;
			}
			else
				SetState( m_nState + 1 );
		}
	}

	UpdateAnim( fElapsedTime );
	m_pStates[m_nState]->UpdateAnim( fElapsedTime );
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CEffectObject::SetState( uint8 nState )
{
	if( m_pStates[m_nState] )
		m_pStates[m_nState]->SetParentEntity( NULL );
	m_nState = nState;
	if( m_pStates[nState] )
		m_pStates[nState]->SetParentEntity( this );
	if( nState == 0 )
		m_fTimeLeft = m_fBirthTime;
	else if( nState == 2 )
		m_fTimeLeft = m_fDeathTime;
	else
		m_fTimeLeft = 0;
}