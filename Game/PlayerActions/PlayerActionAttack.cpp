#include "stdafx.h"
#include "PlayerActionAttack.h"
#include "Player.h"
#include "Render/ParticleSystem.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"

#include "Entities/EffectObject.h"
#include "GUI/HUDCircle.h"

void CPlayerActionAttack::Update( CPlayer* pPlayer, const CVector2& moveAxis, const CVector2& pushForce, float fDeltaTime )
{
	CEntity* pCrosshair = pPlayer->GetCrosshair();
	CVector2 pos = pCrosshair->GetPosition();
	pos = pos + ( moveAxis * m_fMoveSpeed + pushForce ) * fDeltaTime;
	float l = Max( pos.Length(), 0.001f );
	float l1 = l - Max( l - m_fMoveInnerRadius, 0.0f ) / m_fMoveOuterRadius * m_fMoveSpeed * fDeltaTime - 0.01f;
	if( l1 < 0 )
		l1 = 0;
	pos = pos * ( l1 / l );
	pCrosshair->SetPosition( pos );

	m_fCDLeft -= fDeltaTime;
	if( m_fCDLeft < 0 )
		m_fCDLeft = 0;
	CHUDCircle* pCircle = static_cast<CHUDCircle*>( m_pHitArea.GetPtr() );
	pCircle->SetPercent( 1 - ( m_fCDLeft / m_fCD ) );
}

void CPlayerActionAttack::OnEnterHR( CPlayer* pPlayer )
{
	m_pHitArea = new CHUDCircle( m_fHitAreaRadius, 24.0f, CVector4( 1, 0, 0, 0 ), CVector4( 1, 0, 0, 1 ), true );
	m_pHitArea->AddCircle( m_fHitAreaRadius, CVector2( 0, 0 ) );
	m_pHitArea->SetHitType( eEntityHitType_Sensor );
	m_pHitArea->SetBulletMode( true );
	m_pHitArea->SetParentEntity( pPlayer->GetCrosshair() );

	m_pInnerArea = new CHUDCircle( m_fMoveInnerRadius, 16.0f, CVector4( 0, 0, 1, 1 ), CVector4( 0.5, 0.5, 1, 1 ) );
	m_pInnerArea->SetZOrder( -1 );
	m_pInnerArea->SetParentEntity( pPlayer );

	m_pOuterArea = new CHUDCircle( m_fMoveInnerRadius + m_fMoveOuterRadius, 16.0f, CVector4( 0, 0, 1, 1 ), CVector4( 0.5, 0.5, 1, 1 ) );
	m_pOuterArea->SetZOrder( -1 );
	m_pOuterArea->SetParentEntity( pPlayer );
	m_fCDLeft = m_fCD;
}

void CPlayerActionAttack::OnLeaveHR( CPlayer* pPlayer )
{
	if( m_pHitArea )
	{
		m_pHitArea->RemoveThis();
		m_pHitArea = NULL;
	}
	if( m_pInnerArea )
	{
		m_pInnerArea->RemoveThis();
		m_pInnerArea = NULL;
	}
	if( m_pOuterArea )
	{
		m_pOuterArea->RemoveThis();
		m_pOuterArea = NULL;
	}
}

bool CPlayerActionAttack::OnDo( CPlayer* pPlayer )
{
	if( !m_pHitArea )
		return false;
	if( m_fCDLeft > 0 )
		return false;

	static CParticleSystem* pParticleSystem = NULL;
	if( !pParticleSystem )
	{
		pParticleSystem = new CParticleSystem;
		vector<char> content;
		GetFileContent( content, "materials/attack_hit_particle.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pParticleSystem->LoadXml( doc.RootElement() );
		doc.Clear();
	}
	CEffectObject* pEffectObject = new CEffectObject( 0.5f );
	pEffectObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pEffectObject->GetAnimController() ) );
	CVector2& pos = pPlayer->GetCrosshair()->GetGlobalTransform().GetPosition();
	pEffectObject->x = pos.x;
	pEffectObject->y = pos.y;
	pEffectObject->SetParentAfterEntity( pPlayer );

	m_fCDLeft = m_fCD;
	bool bHit = false;
	bool bCritical = false;
	for( auto pManifold = m_pHitArea->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity && pEntity != pPlayer )
		{
			SPlayerAttackContext context( this, pPlayer, pManifold->hitPoint, m_nDmg );
			pEntity->OnPlayerAttack( context );
			if( context.nResult & SPlayerAttackContext::eResult_Hit )
				bHit = true;
			if( context.nResult & SPlayerAttackContext::eResult_Critical )
				bCritical = true;
		}
	}

	return true;
}