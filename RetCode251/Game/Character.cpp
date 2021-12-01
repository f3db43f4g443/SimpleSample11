#include "stdafx.h"
#include "Character.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Entities/EffectObject.h"

CCharacter::CCharacter()
	: m_pLevel( NULL )
	, m_nMaxHp( 0 )
	, m_bIgnoreBullet( false )
	, m_bAlwaysBlockBullet( true )
	, m_nDmgToPlayer( 0 )
	, m_nHp( 0 )
	, m_bCrushed( false )
	, m_pKillEffect( "" )
	, m_pKillSound( "" )
	, m_pCrushEffect( "" )
{
	SET_BASEOBJECT_ID( CCharacter );
}

CCharacter::CCharacter( const SClassCreateContext& context )
	: CEntity( context ), m_nHp( m_nMaxHp )
{
	SET_BASEOBJECT_ID( CCharacter );
}

void CCharacter::OnAddedToStage()
{
	auto pParent = GetParentEntity();
	CMyLevel* pLevel = NULL;
	while( pParent )
	{
		pLevel = SafeCast<CMyLevel>( pParent );
		if( pLevel )
			break;
		pParent = pParent->GetParentEntity();
	}
	if( pLevel )
	{
		m_pLevel = pLevel;
		pLevel->OnAddCharacter( this );
	}
}

void CCharacter::OnRemovedFromStage()
{
	m_pOwner = NULL;
	if( m_pLevel )
	{
		m_pLevel->OnRemoveCharacter( this );
		m_pLevel = NULL;
	}
}

CMatrix2D CCharacter::GetGlobalTransform()
{
	return CEntity::GetGlobalTransform();
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
		if( GetParentEntity() )
			ForceUpdateTransform();
		pKillEffect->SetState( 2 );
		pKillEffect->SetPosition( globalTransform.GetPosition() );
		pKillEffect->SetParentBeforeEntity( GetLevel() );
	}
	if( m_pKillSound )
		m_pKillSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

bool CCharacter::IsAlerted()
{
	auto pMasterLevel = CMasterLevel::GetInst();
	auto alertRect = pMasterLevel->GetAlertRect();
	if( alertRect.width <= 0 || GetLevel() != CMasterLevel::GetInst()->GetCurLevel() || !alertRect.Contains( globalTransform.GetPosition() ) )
		return false;
	return true;
}

bool CCharacter::Damage( SDamageContext & context )
{
	if( !m_nMaxHp || !context.nDamage )
		return context.nSourceType == 1 ? IsAlwaysBlockBullet() : false;
	m_nHp = Max( 0, m_nHp - context.nDamage );

	if( m_nHp <= 0 )
	{
		Kill();
	}
	return true;
}

void CCharacter::OnTickBeforeHitTest()
{
	UpdateAnim( GetLevel()->GetElapsedTimePerTick() );
}

void CCharacter::OnTickAfterHitTest()
{
}

void CDamageEft::OnDamage( CCharacter* pCharacter, CCharacter::SDamageContext& context ) const
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
		pDmgEffect->SetParentBeforeEntity( pCharacter->GetLevel() );
	}
}