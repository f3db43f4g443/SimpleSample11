#include "stdafx.h"
#include "EffectObject.h"
#include "Stage.h"
#include "Render/Canvas.h"
#include "Common/ResourceManager.h"

CEffectObject::CEffectObject( const SClassCreateContext& context )
	: CEntity( context )
	, m_tickBeforeHitTest( this, &CEffectObject::OnTickBeforeHitTest )
	, m_fAnimTimeScale( 1 )
{
	SET_BASEOBJECT_ID( CEffectObject );
}

CEffectObject::CEffectObject( float fTime, CVector2 velocity, float fAngularVelocity )
	: m_tickBeforeHitTest( this, &CEffectObject::OnTickBeforeHitTest )
	, m_strSound( "" )
	, m_nState( 3 )
	, m_fTimeLeft( fTime )
	, m_vel( velocity )
	, m_velA( fAngularVelocity )
	, m_fAnimTimeScale( 1 )
{
	SET_BASEOBJECT_ID( CEffectObject );
}

void CEffectObject::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	for( int i = 0; i < ELEM_COUNT( m_pStates ); i++ )
	{
		if( m_pStates[i] )
			m_pStates[i]->SetParentEntity( NULL );
	}
	if( m_nState <= 2 )
		SetState( m_nState );

	if( m_strSound )
		m_strSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CEffectObject::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CEffectObject::OnTickBeforeHitTest()
{
	float fElapsedTime = GetStage()->GetElapsedTimePerTick();
	CVector2 vel = m_vel + m_a * fElapsedTime;
	SetPosition( GetPosition() + ( m_vel + vel ) * ( fElapsedTime * 0.5f ) );
	SetRotation( GetRotation() + m_velA * fElapsedTime );
	m_vel = vel;

	fElapsedTime *= m_fAnimTimeScale;
	if( m_fTimeLeft > 0 )
	{
		m_fTimeLeft -= fElapsedTime;
		if( m_fTimeLeft <= 0 )
		{
			if( m_nState >= 2 )
			{
				SetParentEntity( NULL );
				return;
			}
			else
				SetState( m_nState + 1 );
		}
	}
	UpdateAnim( fElapsedTime );
	if( m_nState <= 2 && m_pStates[m_nState] )
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