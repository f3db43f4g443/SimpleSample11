#include "stdafx.h"
#include "Character.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Entities/EffectObject.h"

CCharacter::CCharacter()
	: m_tickBeforeHitTest( this, &CCharacter::OnTickBeforeHitTest )
	, m_pKillEffect( "" )
	, m_velocity( 0, 0 )
{
	SET_BASEOBJECT_ID( CCharacter );
}

CCharacter::CCharacter( const SClassCreateContext& context )
	: CEntity( context )
	, m_tickBeforeHitTest( this, &CCharacter::OnTickBeforeHitTest )
	, m_tickAfterHitTest( this, &CCharacter::OnTickAfterHitTest )
	, m_pKillEffect( context )
	, m_velocity( 0, 0 )
{
	SET_BASEOBJECT_ID( CCharacter );
}

void CCharacter::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CCharacter::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

void CCharacter::Kill()
{
	if( m_pKillEffect )
	{
		auto pKillEffect = SafeCast<CEffectObject>( m_pKillEffect->GetRoot()->CreateInstance() );
		ForceUpdateTransform();
		pKillEffect->SetState( 2 );
		pKillEffect->SetPosition( globalTransform.GetPosition() );
		pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
	SetParentEntity( NULL );
}

void CCharacter::OnTickBeforeHitTest()
{
	UpdateAnim( GetStage()->GetElapsedTimePerTick() );
	
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CCharacter::OnTickAfterHitTest()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}