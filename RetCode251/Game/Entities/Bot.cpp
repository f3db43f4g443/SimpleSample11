#include "stdafx.h"
#include "Bot.h"
#include "MyLevel.h"


void CBot::OnAddedToStage()
{
	__super::OnAddedToStage();
	if( !m_bInited )
	{
		Init();
		m_bInited = true;
	}
}

void CBot::OnTickBeforeHitTest()
{
	if( m_bActivate )
	{
		if( m_nDamageTick )
			m_nDamageTick--;
		if( ( !m_bActivateOnDamage || !m_nDamageTick ) && ( !m_bActivateOnAlert || !IsAlerted() ) )
			Deactivate();
	}
	else
	{
		if( m_nDamageTick )
		{
			m_nDamageTick--;
			if( m_bActivateOnDamage && !m_nDamageTick )
				Activate();
		}
		if( !m_bActivate && m_bActivateOnAlert && IsAlerted() )
			Activate();
	}
	__super::OnTickBeforeHitTest();
}

void CBot::OnTickAfterHitTest()
{
	if( Get_HitProxy() )
		__super::OnTickAfterHitTest();
	UpdateModules( m_bActivate );
}

bool CBot::Damage( SDamageContext& context )
{
	if( m_bActivateOnDamage )
		m_nDamageTick = m_bActivate ? m_nDeactivateTime : m_nDamageActivateTime;
	return __super::Damage( context );
}

void CBot::Init()
{
	if( m_bHideOnInactivated )
		bVisible = m_bActivate;
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
	{
		auto pModule = SafeCastToInterface<IBotModule>( p );
		if( pModule )
		{
			p->bVisible = false;
			pModule->InitModule();
		}
	}
}

void CBot::UpdateModules( bool bActivated )
{
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
	{
		auto pModule = SafeCastToInterface<IBotModule>( p );
		if( pModule )
		{
			p->bVisible = bActivated;
			pModule->UpdateModule( bActivated );
		}
	}
}

void CBotTypeA::OnTickBeforeHitTest()
{
	if( m_bActivate && !m_bForceDeactivate )
	{
		if( m_nStateTransitCurTime < m_nStateTransitTime )
		{
			m_nStateTransitCurTime++;
			if( m_nStateTransitCurTime == m_nStateTransitTime )
				m_bActivateOK = true;
			UpdateHit();
		}
	}
	else
	{
		if( m_nStateTransitCurTime )
		{
			m_nStateTransitCurTime--;
			if( !m_nStateTransitCurTime )
			{
				m_bActivate = false;
				m_bActivateOK = false;
				m_bForceDeactivate = false;
			}
			UpdateHit();
		}
	}
	__super::OnTickBeforeHitTest();
}

void CBotTypeA::OnTickAfterHitTest()
{
	CEntity* pTestEntities[] = { this, m_p1 };
	int32 nTestEntities = m_p1->Get_HitProxy() ? 2 : 1;
	SIgnoreEntityHit ignoreHit( pTestEntities, nTestEntities );
	CVector2 gravity( 0, -1 );
	if( !HandlePenetration( pTestEntities, nTestEntities, gravity ) )
		return;

	auto pLevel = GetLevel();
	SRaycastResult res[3];
	auto dPos = HandleCommonMove();
	m_moveData.TryMove1( this, nTestEntities, pTestEntities, dPos, m_vel, m_fFrac, &gravity, res );
	PostMove( nTestEntities, pTestEntities );
	UpdateModules( m_bActivate && m_bActivateOK && !m_bForceDeactivate );
}

void CBotTypeA::Init()
{
	__super::Init();
	m_p1->bVisible = false;
	m_p1->m_bHasHitFilter = m_p1->m_bParentHitFilter = true;
}

bool CBotTypeA::HandlePenetration( CEntity ** pTestEntities, int32 nTestEntities, const CVector2& gravity )
{
	if( !m_moveData.ResolvePenetration( this, &m_vel, m_fFrac, NULL, NULL, &gravity, pTestEntities, nTestEntities ) )
	{
		if( !m_p1->Get_HitProxy() )
		{
			//Crush();
			//return false;
		}
		m_bForceDeactivate = true;
	}
	else
		m_bForceDeactivate = false;
	return true;
}

void CBotTypeA::UpdateHit()
{
	auto& hitTestMgr = GetLevel()->GetHitTestMgr();
	if( m_p1->Get_HitProxy() )
		hitTestMgr.Remove( m_p1 );
	if( m_p1->Get_HitProxy() )
		m_p1->RemoveProxy( m_p1->Get_HitProxy() );
	if( m_nStateTransitCurTime > 0 )
	{
		CVector4 r = m_shapeParams[0] + ( m_shapeParams[1] - m_shapeParams[0] ) * m_nStateTransitCurTime / m_nStateTransitTime;
		if( m_nShapeType == 0 )
			m_p1->AddCircle( r.z * 0.5f, CVector2( r.x + r.z * 0.5f, r.y + r.w * 0.5f ) );
		else
			m_p1->AddRect( CRectangle( r.x, r.y, r.z, r.w ) );
		m_p1->bVisible = true;
	}
	else
		m_p1->bVisible = false;
	if( m_p1->Get_HitProxy() )
		hitTestMgr.Add( m_p1 );
}

void CBotTypeALeap::OnTickAfterHitTest()
{
	CEntity* pTestEntities[] = { this, m_p1 };
	int32 nTestEntities = m_p1->Get_HitProxy() ? 2 : 1;
	SIgnoreEntityHit ignoreHit( pTestEntities, nTestEntities );
	CVector2 gravity( 0, -1 );
	if( !HandlePenetration( pTestEntities, nTestEntities, gravity ) )
		return;

	auto pLevel = GetLevel();
	SRaycastResult res[3];
	auto dPos = HandleCommonMove();
	auto vel0 = m_vel;
	m_moveData.TryMove1( this, nTestEntities, pTestEntities, dPos, m_vel, m_bIsLeaping ? 0 : m_fFrac, &gravity, res );
	if( m_bIsLeaping )
	{
		CVector2 tangent( -gravity.y, gravity.x );
		tangent = tangent * m_nDir;
		auto d = m_vel - vel0;
		auto l = d.Dot( tangent );
		if( l < 0 )
		{
			m_vel = m_vel + tangent * l;
			m_nDir = -m_nDir;
		}
	}

	auto pPlayer = pLevel->GetPlayer();
	if( m_nLeapCDLeft )
		m_nLeapCDLeft--;
	if( m_bActivate && m_bActivateOK && !m_bForceDeactivate )
	{
		auto g = GetGlobalTransform();
		CMatrix2D mats[2] = { g, m_p1->GetGlobalTransform() };
		CVector2 ofs = gravity;
		SRaycastResult result;
		if( m_bIsLeaping )
		{
			auto pLandedEntity = SafeCast<CCharacter>( m_moveData.DoSweepTest1( this, nTestEntities, pTestEntities, mats, ofs, MOVE_SIDE_THRESHOLD, &gravity, &result ) );
			if( pLandedEntity && m_vel.Dot( result.normal ) < 1.0f && result.normal.Dot( gravity ) < -0.5f )
			{
				m_vel = gravity * m_vel.Dot( gravity );
				m_bIsLeaping = false;
			}
		}
		else if( !m_nKickCounter && !m_nLeapCDLeft && pPlayer && m_rectDetect.Contains( g.MulTVector2Pos( pPlayer->GetPosition() ) ) )
		{
			auto pLandedEntity = SafeCast<CCharacter>( m_moveData.DoSweepTest1( this, nTestEntities, pTestEntities, mats, ofs, MOVE_SIDE_THRESHOLD, &gravity, &result ) );
			if( pLandedEntity && m_vel.Dot( result.normal ) < 1.0f && result.normal.Dot( gravity ) < -0.5f )
			{
				CVector2 tangent( -gravity.y, gravity.x );
				auto d = pPlayer->GetPosition() - g.GetPosition();
				m_nDir = d.Dot( tangent ) >= 0 ? 1 : -1;
				m_vel = m_vel - gravity * m_leapVel.y + tangent * m_leapVel.x * m_nDir;
				m_bIsLeaping = true;
				m_nLeapCDLeft = m_nLeapCD;
			}
		}
	}
	else
		m_bIsLeaping = false;
	PostMove( nTestEntities, pTestEntities );

	if( m_bIsLeaping )
	{
		for( auto pManifold = m_p1->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto p = (CEntity*)pManifold->pOtherHitProxy;
			if( p == pPlayer )
			{
				SDamageContext context;
				context.nDamage = m_nLeapDamage;
				context.hitPos = pManifold->hitPoint;
				context.hitDir = m_leapHitDir;
				context.hitDir.x *= m_nDir;
				context.pSource = this;
				pPlayer->Damage( context );
			}
		}
	}
	UpdateModules( m_bActivate && m_bActivateOK && !m_bForceDeactivate );
	UpdateImg();
}

bool CBotTypeALeap::Damage( SDamageContext& context )
{
	if( m_bIsLeaping )
		return CCharacter::Damage( context );
	return __super::Damage( context );
}

void CBotTypeALeap::UpdateImg()
{
	auto p = static_cast<CImage2D*>( m_p1->GetRenderObject() );
	int32 nTex = 0;
	p->bVisible = m_nStateTransitCurTime > 0;
	if( m_nStateTransitCurTime < m_nStateTransitTime )
		nTex = m_nStateTransitCurTime ? ( m_nStateTransitCurTime * 3 / m_nStateTransitTime ) + 1 : 0;
	else
	{
		nTex = 4;
		if( m_bIsLeaping )
		{
			CVector2 gravity( 0, -1 );
			float fSpeed = -m_vel.Dot( gravity );
			nTex = 7;
			for( int i = 0; i < 2; i++ )
			{
				if( fSpeed >= m_fLeapAnimSpd[i] )
				{
					nTex = 5 + i;
					break;
				}
			}
		}
	}
	auto texRect = CRectangle( nTex, 0, 1, 2 ) / 32;
	if( m_nDir == -1 )
		texRect.x = 2 - texRect.GetRight();
	p->SetTexRect( texRect );
}


void RegisterGameClasses_Bot()
{
	REGISTER_CLASS_BEGIN( CBot )
		REGISTER_BASE_CLASS( CCommonMoveableObject )
		REGISTER_MEMBER( m_nDamageActivateTime )
		REGISTER_MEMBER( m_nDeactivateTime )
		REGISTER_MEMBER( m_bActivateOnAlert )
		REGISTER_MEMBER( m_bActivateOnDamage )
		REGISTER_MEMBER( m_bHideOnInactivated )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBotTypeA )
		REGISTER_BASE_CLASS( CBot )
		REGISTER_MEMBER( m_nShapeType )
		REGISTER_MEMBER( m_shapeParams )
		REGISTER_MEMBER( m_nStateTransitTime )
		REGISTER_MEMBER_TAGGED_PTR( m_p1, 1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBotTypeALeap )
		REGISTER_BASE_CLASS( CBotTypeA )
		REGISTER_MEMBER( m_rectDetect )
		REGISTER_MEMBER( m_nLeapCD )
		REGISTER_MEMBER( m_leapVel)
		REGISTER_MEMBER( m_fLeapAnimSpd )
		REGISTER_MEMBER( m_nLeapDamage )
		REGISTER_MEMBER( m_leapHitDir )
	REGISTER_CLASS_END()
}