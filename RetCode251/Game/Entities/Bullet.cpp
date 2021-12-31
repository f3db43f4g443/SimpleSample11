#include "stdafx.h"
#include "Bullet.h"
#include "Stage.h"
#include "Player.h"
#include "xml.h"
#include "FileUtil.h"
#include "MyLevel.h"
#include "ParticleSystem.h"
#include "Explosion.h"
#include "Entities/CharacterMisc.h"

void CBullet::OnHit( CEntity* pEntity )
{
	if( m_onHit )
		m_onHit( this, pEntity );
}

void CBullet::Kill()
{
	if( m_pExp )
	{
		m_pExp->SetPosition( GetPosition() );
		m_pExp->SetRotation( GetRotation() );
		auto pExplosion = SafeCast<CExplosion>( m_pExp.GetPtr() );
		if( pExplosion )
			pExplosion->SetOwner( m_pOwner );
		m_pExp->SetParentBeforeEntity( this );
		m_pExp = NULL;
	}
	if( m_pDeathEffect )
	{
		CMatrix2D mat = globalTransform;
		CMatrix2D mat1;
		mat1.Transform( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		( mat1 * mat ).Decompose( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		m_pDeathEffect->SetParentBeforeEntity( this );
		m_pDeathEffect->SetState( 2 );
	}

	if( m_pParticle )
	{
		static_cast<CParticleSystemObject*>( m_pParticle.GetPtr() )->GetInstanceData()->GetData().isEmitting = false;
	}

	m_fDeathTime = m_fDeathFramesPerSec <= 0 ? 0 :
		( m_nDeathFrameEnd > m_nDeathFrameBegin ?( m_nDeathFrameEnd - m_nDeathFrameBegin ) / m_fDeathFramesPerSec : m_fDeathFramesPerSec );
	if( m_fDeathTime <= 0 )
	{
		CCharacter::Kill();
		return;
	}

	m_bKilled1 = true;
	if( m_nDeathFrameEnd > m_nDeathFrameBegin )
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( m_nDeathFrameBegin, m_nDeathFrameEnd, m_fDeathFramesPerSec );
	else
		GetRenderObject()->bVisible = false;
	Trigger( eCharacterEvent_Kill );
}

void CBullet::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	if( !m_bInited )
	{
		m_bInited = true;
		if( m_pExp )
			m_pExp->SetParentEntity( NULL );
		if( m_pDeathEffect )
			m_pDeathEffect->SetParentEntity( NULL );
		m_origImgRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	}
	m_pos0 = GetPosition();
}

void CBullet::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
}

void CBullet::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( !m_bKilled1 && !m_bAttached )
	{
		CVector2 newVelocity = m_vel + m_acc * GetLevel()->GetElapsedTimePerTick();
		m_pos0 = GetGlobalTransform().GetPosition();
		SetPosition( GetPosition() + ( m_vel + newVelocity ) * ( GetLevel()->GetElapsedTimePerTick() * 0.5f ) );
		m_vel = newVelocity;
		if( m_bTangentDir )
			SetRotation( atan2( m_vel.y, m_vel.x ) );
		else
			SetRotation( r + m_fAngularVelocity * GetLevel()->GetElapsedTimePerTick() );
	}
	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
			Kill();
	}
	if( m_bKilled1 )
	{
		m_fDeathTime -= GetLevel()->GetElapsedTimePerTick();
		if( m_fDeathTime <= 0 )
		{
			SetParentEntity( NULL );
		}
		return;
	}
	if( m_pLevel->CheckOutOfBound( this ) )
	{
		SetParentEntity( NULL );
		return;
	}

	if( m_nBulletType )
		UpdateTrail();
	else
		UpdateCommon();
}

CCharacter* CBullet::CheckHit( CEntity* pEntity )
{
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	if( pCharacter )
	{
		if( pCharacter->IsIgnoreDamageSource( 0 ) )
			pCharacter = NULL;
	}
	else
	{
		auto p = SafeCast<CDamageArea>( pEntity );
		if( p && !p->IsIgnoreDamageSource( 0 ) )
			pCharacter = SafeCast<CCharacter>( p->GetParentEntity() );
	}
	if( !pCharacter || pCharacter->IsKilled() )
		return NULL;
	if( pCharacter->IsOwner( m_pOwner ) )
		return NULL;
	return pCharacter;
}

bool CBullet::HandleHit( CCharacter* pCharacter, const CVector2& hitPoint )
{
	CReference<CEntity> pTempRef = pCharacter;
	SDamageContext context;
	context.nSourceType = 1;
	context.hitPos = hitPoint;
	context.hitDir = m_vel;
	if( m_fHitForce >= 0 )
	{
		context.hitDir.Normalize();
		context.hitDir = context.hitDir * m_fHitForce;
	}
	context.nHitType = -1;
	context.pSource = this;
	if( m_bAlertEnemy && pCharacter->IsEnemy() )
	{
		context.nType = eDamageHitType_Alert;
		pCharacter->Damage( context );
		return false;
	}
	context.nDamage = m_nDamage;
	context.fDamage1 = m_nDamage1;

	if( pCharacter->Damage( context ) )
	{
		if( m_pDmgEft )
			m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( pCharacter, context );

		OnHit( pCharacter );
		return true;
	}
	return false;
}

void CBullet::UpdateCommon()
{
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CCharacter* pCharacter = CheckHit( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pCharacter && HandleHit( pCharacter, pManifold->hitPoint ) )
		{
			Kill();
			return;
		}
	}
	if( m_bHitStatic )
	{
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( m_pOwner && pEntity->IsOwner( m_pOwner ) )
				continue;
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				Kill();
				return;
			}
		}
	}
}

void CBullet::UpdateTrail()
{
	auto rect = m_initTrailRect;
	rect.SetLeft( rect.x - m_fCurTrailLen );
	auto trans = GetGlobalTransform();
	CVector2 pos = trans.GetPosition();
	trans.SetPosition( m_pos0 );
	SHitProxyPolygon hit( rect );
	static vector<CReference<CEntity> > vecHitEntities;
	static vector<SRaycastResult> vecResult;
	GetLevel()->MultiSweepTest( &hit, trans, pos - m_pos0, 0, vecHitEntities, &vecResult );

	for( int i = 0; i < vecHitEntities.size(); i++ )
	{
		CCharacter* pCharacter = CheckHit( static_cast<CEntity*>( vecHitEntities[i].GetPtr() ) );
		if( pCharacter && HandleHit( pCharacter, vecResult[i].hitPoint ) )
		{
			Kill();
			goto finalize;
		}
	}
	if( m_bHitStatic )
	{
		for( int i = 0; i < vecHitEntities.size(); i++ )
		{
			CEntity* pEntity = vecHitEntities[i];
			if( m_pOwner && pEntity->IsOwner( m_pOwner ) )
				continue;
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				Kill();
				goto finalize;
			}
		}
	}
	float l = ( pos - m_pos0 ).Length();
	m_fCurTrailLen = Min( m_fCurTrailLen + l * m_fTrailSpeedScale, m_fTrailLen );
	auto imgRect = m_origImgRect;
	imgRect.SetLeft( imgRect.x - m_fCurTrailLen );
	static_cast<CImage2D*>( GetRenderObject() )->SetRect( imgRect );
finalize:
	vecHitEntities.resize( 0 );
	vecResult.resize( 0 );
}

void RegisterGameClasses_Bullet()
{
	REGISTER_CLASS_BEGIN( CBullet )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nDeathFrameBegin )
		REGISTER_MEMBER( m_nDeathFrameEnd )
		REGISTER_MEMBER( m_fDeathFramesPerSec )
		REGISTER_MEMBER( m_fDeathTime )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_bHitStatic )
		REGISTER_MEMBER( m_bAlertEnemy )
		REGISTER_MEMBER( m_vel )
		REGISTER_MEMBER( m_acc )
		REGISTER_MEMBER( m_bTangentDir )
		REGISTER_MEMBER( m_fAngularVelocity )
		REGISTER_MEMBER( m_fHitForce )
		REGISTER_MEMBER( m_pDmgEft )
		REGISTER_MEMBER( m_nBulletType )
		REGISTER_MEMBER( m_initTrailRect )
		REGISTER_MEMBER( m_fTrailLen )
		REGISTER_MEMBER( m_fTrailSpeedScale )
		REGISTER_MEMBER_TAGGED_PTR( m_pDeathEffect, death )
		REGISTER_MEMBER_TAGGED_PTR( m_pParticle, particle )
		REGISTER_MEMBER_TAGGED_PTR( m_pExp, exp )
	REGISTER_CLASS_END()
}