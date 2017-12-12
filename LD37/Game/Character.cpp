#include "stdafx.h"
#include "Character.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Entities/EffectObject.h"

CCharacter::CCharacter()
	: m_tickBeforeHitTest( this, &CCharacter::OnTickBeforeHitTest )
	, m_pKillEffect( "" )
	, m_pKillSound( "" )
	, m_velocity( 0, 0 )
{
	SET_BASEOBJECT_ID( CCharacter );
}

CCharacter::CCharacter( const SClassCreateContext& context )
	: CEntity( context )
	, m_tickBeforeHitTest( this, &CCharacter::OnTickBeforeHitTest )
	, m_tickAfterHitTest( this, &CCharacter::OnTickAfterHitTest )
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
	KillEffect();
	SetParentEntity( NULL );
}

void CCharacter::KillEffect()
{
	CPrefab* pKillEffectPrefab = m_bCrushed && m_pCrushEffect ? m_pCrushEffect : m_pKillEffect;
	if( pKillEffectPrefab )
	{
		auto pKillEffect = SafeCast<CEffectObject>( pKillEffectPrefab->GetRoot()->CreateInstance() );
		ForceUpdateTransform();
		pKillEffect->SetState( 2 );
		pKillEffect->SetPosition( globalTransform.GetPosition() );
		pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
	if( m_pKillSound )
		m_pKillSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
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

void CDamageEft::OnDamage( CCharacter::SDamageContext & context ) const
{
	if( context.nDamage <= 0 )
		return;
	if( context.nHitType >= ELEM_COUNT( m_prefabs ) )
		return;

	auto pPrefab = m_prefabs[context.nHitType];
	if( pPrefab )
	{
		auto pDmgEffect = SafeCast<CEffectObject>( pPrefab->GetRoot()->CreateInstance() );
		pDmgEffect->SetState( 2 );
		pDmgEffect->SetPosition( context.hitPos );
		pDmgEffect->SetRotation( atan2( context.hitDir.y, context.hitDir.x ) );
		pDmgEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
}
