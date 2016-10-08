#include "stdafx.h"
#include "Spider.h"
#include "Stage.h"
#include "Player.h"
#include "EnemyBullet.h"
#include "Common/Rand.h"
#include "EffectObject.h"
#include "Render/ParticleSystem.h"
#include "Effects/DynamicTextures.h"

CSpider::CSpider()
	: m_nState( 0 )
	, m_fTime( 0 )
	, m_onHit( this, &CSpider::OnHit )
	, m_onPlayerAttack( this, &CSpider::OnPlayerAttack )
{
}

void CSpider::OnTickBeforeHitTest()
{
	CStage* pStage = GetStage();
	float fTime = pStage->GetGlobalElapsedTime();
	if( m_nState == 0 )
		Idle();
	else if( m_nState == 1 )
		Move();
	else if( m_nState == 2 || m_nState == 3 )
		Attack();
	CCharacter::OnTickBeforeHitTest();
}

void CSpider::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	m_nHeadBoneIndex = GetAnimController()->GetAnimSet()->GetSkeleton().GetBoneIndex( "s_head" );

	uint16 nTailBoneIndex = GetAnimController()->GetAnimSet()->GetSkeleton().GetBoneIndex( "s_tail" );
	for( CEntity* pChild = Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		if( pChild->GetTransformIndex() == nTailBoneIndex )
		{
			if( !pChild->Get_HitProxy() )
			{
				pChild->SetHitType( eEntityHitType_Sensor );
				pChild->AddRect( CRectangle( -60, -79, 120, 158 ) );
			}
			pChild->RegisterEntityEvent( eEntityEvent_PlayerAttack, &m_onPlayerAttack );
			break;
		}
	}
}

void CSpider::OnRemovedFromStage()
{
	CCharacter::OnRemovedFromStage();
	if( m_onPlayerAttack.IsRegistered() )
		m_onPlayerAttack.Unregister();
	Stop();
}

void CSpider::BeginMove()
{
	m_nState = 1;
	m_fTime = 0;
	m_pAnim = GetAnimController()->PlayAnim( "walk", eAnimPlayMode_Loop );
}

void CSpider::BeginAttack()
{
	m_nState = 2;
	m_fTime = 0;
	m_pAnim = GetAnimController()->PlayAnim( "attack", eAnimPlayMode_Once );
	m_pAnim->RegisterEvent( 0, &m_onHit );
}

void CSpider::Stop()
{
	m_nState = 0;
	m_fTime = 0;
	if( m_pAnim )
	{
		if( m_onHit.IsRegistered() )
			m_onHit.Unregister();
		if( m_pAnim->GetController() )
			GetAnimController()->StopAnim( m_pAnim );
		m_pAnim = NULL;
	}
}

void CSpider::Move()
{
	float fMoveSpeed = 200;
	float fTurnSpeed = 2.0f;

	CStage* pStage = GetStage();
	float fTime = pStage->GetGlobalElapsedTime();
	m_fTime += fTime;
	if( m_fTime >= 1.0f )
	{
		Stop();
		return;
	}

	CPlayer* pPlayer = pStage->GetPlayer();
	CVector2 dPosition = pPlayer->GetPosition() - GetPosition();
	float fRotation = GetRotation();
	float fTargetAngle = atan2( -dPosition.x, dPosition.y );
	float dRotation = fRotation - fTargetAngle;
	dRotation = ( dRotation + PI ) / ( PI * 2 );
	dRotation = ( dRotation - floor( dRotation ) ) * ( PI * 2 ) - PI;
	float fTurnAngle = fTurnSpeed * fTime;
	if( dRotation > 0 )
	{
		dRotation -= fTurnAngle;
		if( dRotation < 0 )
			dRotation = 0;
		SetRotation( fTargetAngle + dRotation );
	}
	else if( dRotation < 0 )
	{
		dRotation += fTurnAngle;
		if( dRotation > 0 )
			dRotation = 0;
		SetRotation( fTargetAngle + dRotation );
	}

	if( dPosition.Dot( dPosition ) >= 100 * 100 )
	{
		float fMoveDist = fMoveSpeed * fTime;
		dPosition.Normalize();
		dPosition = dPosition * fMoveDist;
		SetPosition( GetPosition() + dPosition );
	}
	else if( dPosition.Dot( dPosition ) < 1 || dRotation == 0 )
	{
		BeginAttack();
	}
}

void CSpider::Idle()
{
	CStage* pStage = GetStage();
	float fTime = pStage->GetGlobalElapsedTime();
	m_fTime += fTime;
	if( m_fTime >= 2.0f )
	{
		BeginMove();
		return;
	}
}

void CSpider::Attack()
{
	if( !m_pAnim->GetController() )
	{
		Stop();
		return;
	}
}

void CSpider::OnHit()
{
	m_nState = 3;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( !pPlayer->IsInHorrorReflex() )
		{
			if( pPlayer->CanBeHit() )
			{
				SDamage dmg;
				dmg.pSource = this;
				dmg.nHp = 10;
				pPlayer->Damage( dmg );
			}
		}
		else
		{
			const CMatrix2D& mat = GetAnimController()->GetTransform( m_nHeadBoneIndex );
			CVector2 pos = mat.GetPosition();
			for( int i = 0; i < 24; i++ )
			{
				float r = ( i + SRand::Inst().Rand( -0.3f, 0.3f ) ) * PI / 12;
				float fSpeed = SRand::Inst().Rand( 200, 300 );
				CEnemyBullet* pEnemyBullet = new CEnemyBullet( this, CVector2( fSpeed * cos( r + 0.5f * PI ), fSpeed * sin( r + 0.5f * PI ) ), CVector2( 0, 0 ), 8, 3, 10, 0, 0, 1 );
				pEnemyBullet->x = pos.x;
				pEnemyBullet->y = pos.y;
				pEnemyBullet->r = r;
				pEnemyBullet->SetParentEntity( GetParentEntity() );
			}
		}
	}
}

void CSpider::OnPlayerAttack( SPlayerAttackContext* pContext )
{
	if( pContext->pPlayer->IsInHorrorReflex() && m_nState == 3 )
	{
		pContext->nResult |= SPlayerAttackContext::eResult_Hit;
		CVector2& pos = pContext->hitPos;
		CEffectObject* pObject = new CEffectObject( 1, &CBloodSplashCanvas::Inst() );
		CParticleSystem* pParticleSystem = CBloodSplashCanvas::Inst().pParticleSystem1;
		pObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pObject->GetAnimController() ) );
		pObject->x = pos.x;
		pObject->y = pos.y;
		pObject->SetParentBeforeEntity( this );
	}
}