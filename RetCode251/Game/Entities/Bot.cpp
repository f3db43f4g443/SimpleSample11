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
			m_nDamageTick = Max( 0, m_nDamageTick - GetLevel()->GetDeltaTick() );
		if( ( !m_bActivateOnDamage || !m_nDamageTick ) && ( !m_bActivateOnAlert || !IsAlerted() ) )
			Deactivate();
	}
	else
	{
		if( m_nDamageTick )
		{
			m_nDamageTick = Max( 0, m_nDamageTick - GetLevel()->GetDeltaTick() );
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
	if( !m_bActivate && !m_bActivateOK )
		context.nDamage = 0;
	bool b = __super::Damage( context );
	if( ( b || context.nType == eDamageHitType_Alert ) && m_bActivateOnDamage )
	{
		if( m_bActivate )
			m_nDamageTick = m_nDeactivateTime * T_SCL;
		else if( !m_nDamageTick )
			m_nDamageTick = m_nDamageActivateTime * T_SCL;
	}
	return b;
}

bool CBot::ImpactHit( int32 nLevel, const CVector2& vec, CEntity* pEntity )
{
	if( __super::ImpactHit( nLevel, vec, pEntity ) )
	{
		if( m_bActivate )
			m_nDamageTick = m_nDeactivateTime * T_SCL;
		else if( !m_nDamageTick )
			m_nDamageTick = m_nDamageActivateTime * T_SCL;
		return true;
	}
	return false;
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
	auto nDeltaTick = GetLevel()->GetDeltaTick();
	if( m_nRespawnTick )
	{
		m_nRespawnTick = Max( 0, m_nRespawnTick - nDeltaTick );
		if( !m_nRespawnTick )
			m_nHp = m_nMaxHp;
	}
	else if( m_nMaxHp && m_nHp <= 0 )
	{
		Deactivate();
		m_bActivateOK = false;
		m_nStateTransitCurTime = 0;
		m_nRespawnTick = m_nRespawnTime * T_SCL;
		UpdateHit();
	}

	if( m_bActivate && !m_bForceDeactivate )
	{
		if( m_nStateTransitCurTime < m_nStateTransitTime * T_SCL && !m_nRespawnTick )
		{
			m_nStateTransitCurTime += nDeltaTick;
			if( m_nStateTransitCurTime >= m_nStateTransitTime * T_SCL )
				m_bActivateOK = true;
			UpdateHit();
		}
	}
	else
	{
		if( m_nStateTransitCurTime )
		{
			m_nStateTransitCurTime = Max( 0, m_nStateTransitCurTime - nDeltaTick );
			if( !m_nStateTransitCurTime )
			{
				m_bActivate = false;
				m_bActivateOK = false;
				m_bForceDeactivate = false;
			}
			UpdateHit();
		}
	}
	if( m_nRespawnTick )
		CCommonMoveableObject::OnTickBeforeHitTest();
	else
		__super::OnTickBeforeHitTest();
}

void CBotTypeA::OnTickAfterHitTest()
{
	CEntity* pTestEntities[] = { this, m_p1 };
	int32 nTestEntities = m_p1->Get_HitProxy() ? 2 : 1;
	SIgnoreEntityHit ignoreHit( pTestEntities, nTestEntities );
	CVector2 gravity( 0, -1 );
	CVector2 gravity0( 0, 0 );
	auto pGravity = CanHitPlatform() ? &gravity : &gravity0;
	if( !HandlePenetration( pTestEntities, nTestEntities, pGravity ) )
		return;

	auto pLevel = GetLevel();
	SRaycastResult res[3];
	auto dPos = HandleCommonMove();
	m_moveData.TryMove1XY( this, nTestEntities, pTestEntities, dPos, m_vel, m_fFrac, pGravity, res );
	PostMove( nTestEntities, pTestEntities );
	UpdateModules( m_bActivate && m_bActivateOK && !m_bForceDeactivate );
	m_p1->bVisible = m_nStateTransitCurTime > 0;
}

int8 CBotTypeA::CheckPush( SRaycastResult & hit, const CVector2 & dir, float & fDist, SPush & context, int32 nPusher )
{
	if( hit.nUser )
	{
		auto pPusher = context.vecChars[nPusher].pChar;
		if( m_moveData.setOpenPlatforms.find( pPusher ) != m_moveData.setOpenPlatforms.end() )
			return -1;
		float f = hit.normal.Dot( CanHitPlatform() ? CVector2( 0, -1 ) : CVector2( 0, 0 ) );
		if( f < PLATFORM_THRESHOLD )
		{
			m_moveData.setOpenPlatforms.insert( pPusher );
			return -1;
		}
	}
	CEntity* pTestEntities[] = { this, m_p1 };
	CMatrix2D mat[] = { globalTransform, m_p1->globalTransform };
	fDist = GetLevel()->Push( this, context, dir, fDist, 1, pTestEntities, mat, MOVE_SIDE_THRESHOLD );
	return 1;
}

void CBotTypeA::HandlePush( const CVector2 & dir, float fDist, int8 nStep )
{
	if( nStep == 0 )
	{
		if( fDist > 0 )
		{
			SetPosition( GetPosition() + dir * fDist );
			SetDirty();
			m_p1->SetDirty();
		}
	}
	else if( nStep == 1 )
	{
		if( fDist > 0 )
		{
			auto& hitTestMgr = GetLevel()->GetHitTestMgr( GetUpdateGroup() );
			hitTestMgr.Update( this );
			hitTestMgr.Update( m_p1.GetPtr() );
		}
	}
	else
	{
		CEntity* pTestEntities[] = { this, m_p1 };
		m_moveData.CleanUpOpenPlatforms( this, 2, pTestEntities );
	}
}

void CBotTypeA::Init()
{
	__super::Init();
	m_p1->bVisible = false;
	m_p1->m_bHasHitFilter = m_p1->m_bParentHitFilter = true;
}

bool CBotTypeA::HandlePenetration( CEntity ** pTestEntities, int32 nTestEntities, CVector2* pGravity, CEntity* pLandedEntity, CVector2* pLandedOfs )
{
	return m_moveData.ResolvePenetration( this, &m_vel, m_fFrac, pLandedEntity, pLandedOfs, pGravity, pTestEntities, nTestEntities );
}

void CBotTypeA::UpdateHit()
{
	auto& hitTestMgr = GetLevel()->GetHitTestMgr( GetUpdateGroup() );
	if( m_p1->Get_HitProxy() )
		hitTestMgr.Remove( m_p1 );
	if( m_p1->Get_HitProxy() )
		m_p1->RemoveProxy( m_p1->Get_HitProxy() );
	if( m_nStateTransitCurTime > 0 )
	{
		CVector4 r = m_shapeParams[0] + ( m_shapeParams[1] - m_shapeParams[0] ) * m_nStateTransitCurTime / ( m_nStateTransitTime * T_SCL );
		if( m_nShapeType == 0 )
			m_p1->AddCircle( r.z * 0.5f, CVector2( r.x + r.z * 0.5f, r.y + r.w * 0.5f ) );
		else
			m_p1->AddRect( CRectangle( r.x, r.y, r.z, r.w ) );
	}
	if( m_p1->Get_HitProxy() )
		hitTestMgr.Add( m_p1 );
}

void CBotTypeALeap::OnTickAfterHitTest()
{
	CEntity* pTestEntities[] = { this, m_p1 };
	int32 nTestEntities = m_p1->Get_HitProxy() ? 2 : 1;
	SIgnoreEntityHit ignoreHit( pTestEntities, nTestEntities );
	CVector2 gravity( 0, -1 );
	CVector2 gravity0( 0, 0 );
	auto pGravity = CanHitPlatform() && AICanHitPlatform() ? &gravity : &gravity0;
	m_bForceDeactivate = false;
	if( !HandlePenetration( pTestEntities, nTestEntities, pGravity ) )
	{
		pGravity = &gravity0;
		if( !HandlePenetration( pTestEntities, nTestEntities, pGravity ) )
		{
			m_bForceDeactivate = true;
			return;
		}
		m_bForceDeactivate = true;
		return;
	}

	auto pLevel = GetLevel();
	SRaycastResult res[3];
	CVector2 dPos;
	if( m_nLeapFixedVelTimeLeft )
	{
		float fDeltaTime = GetLevel()->GetDeltaTime();
		dPos = m_vel * fDeltaTime;
	}
	else
		dPos = HandleCommonMove();
	auto vel0 = m_vel;
	auto dPosY = dPos.Dot( gravity );
	if( m_bPushLeap && m_bIsLeaping && dPosY < 0 )
	{
		auto dx = dPos - gravity * dPosY;
		float f = GetLevel()->Push( this, gravity * -1, -dPosY );
		if( f < dPosY )
		{
			CVector2 tangent( -gravity.y, gravity.x );
			m_vel = tangent * m_vel.Dot( tangent );
		}
		m_moveData.TryMove1XY( this, nTestEntities, pTestEntities, dx, m_vel, m_bIsLeaping ? 0 : m_fFrac, pGravity, res );
	}
	else
		m_moveData.TryMove1XY( this, nTestEntities, pTestEntities, dPos, m_vel, m_bIsLeaping ? 0 : m_fFrac, pGravity, res );
	if( m_bIsLeaping )
	{
		CVector2 tangent( -gravity.y, gravity.x );
		tangent = tangent * m_nDir;
		auto d = m_vel - vel0;
		auto l = d.Dot( tangent );

		if( m_nLeapFixedVelTimeLeft )
			m_vel = vel0;
		if( l < 0 )
		{
			m_vel = m_vel + tangent * l;
			m_nDir = -m_nDir;
		}
	}

	auto pPlayer = pLevel->GetPlayer();
	if( m_nLeapCDLeft )
		m_nLeapCDLeft--;
	if( m_nLeapFixedVelTimeLeft )
		m_nLeapFixedVelTimeLeft--;
	if( m_bActivate && m_bActivateOK && !m_bForceDeactivate )
	{
		auto g = GetGlobalTransform();
		CMatrix2D mats[2] = { g, m_p1->GetGlobalTransform() };
		CVector2 ofs = gravity;
		SRaycastResult result;
		if( !m_bIsLeaping )
		{
			if( !m_nKickCounter && !m_nImpactLevel && !m_nLeapCDLeft && pPlayer && m_rectDetect.Contains( g.MulTVector2Pos( pPlayer->GetPosition() ) ) )
			{
				auto pLandedEntity = SafeCast<CCharacter>( m_moveData.DoSweepTest1( this, nTestEntities, pTestEntities, mats, ofs, MOVE_SIDE_THRESHOLD, pGravity, &result ) );
				if( pLandedEntity && m_vel.Dot( result.normal ) < 1.0f && result.normal.Dot( gravity ) < -0.5f )
				{
					if( m_nLeapReadyTimeLeft )
					{
						m_nLeapReadyTimeLeft--;
						if( !m_nLeapReadyTimeLeft )
						{
							CVector2 tangent( -gravity.y, gravity.x );
							auto d = pPlayer->GetPosition() - g.GetPosition();
							auto dx = d.Dot( tangent );
							float vx = m_leapVel.x;
							if( m_nSmartLeap )
							{
								float t;
								float a = m_fGravity;
								float b = m_leapVel.y / a;
								float c = -d.Dot( gravity ) * 2 / a;
								float delta = b * b - c;
								if( delta > 0 )
								{
									delta = sqrt( delta );
									t = b - delta;
									if( t * vx < abs( dx ) )
										t = b + delta;
								}
								else
									t = b;
								vx = Min( m_leapVel.x, Max( -m_leapVel.x, dx / t ) );
							}
							else
							{
								if( dx < 0 )
									vx = -vx;
							}
							m_vel = m_vel - gravity * m_leapVel.y + tangent * vx;
							m_nDir = vx > 0 ? 1 : -1;

							m_bIsLeaping = true;
							m_nLeapCDLeft = m_nLeapCD;
							m_nLeapFixedVelTimeLeft = m_nLeapFixedVelTime;
						}
					}
					else
						m_nLeapReadyTimeLeft = m_nLeapReadyTime;
				}
			}
			else if( m_nLeapReadyTimeLeft )
				m_nLeapReadyTimeLeft = m_nLeapReadyTime;
		}
		if( m_bIsLeaping )
		{
			auto pLandedEntity = SafeCast<CCharacter>( m_moveData.DoSweepTest1( this, nTestEntities, pTestEntities, mats, ofs, MOVE_SIDE_THRESHOLD, pGravity, &result ) );
			if( pLandedEntity && m_vel.Dot( result.normal ) < 1.0f && result.normal.Dot( gravity ) < -0.5f )
			{
				m_vel = gravity * m_vel.Dot( gravity );
				m_bIsLeaping = false;
			}
		}
	}
	else
	{
		m_bIsLeaping = false;
		m_nLeapReadyTimeLeft = 0;
	}
	PostMove( nTestEntities, pTestEntities );

	if( m_bIsLeaping )
	{
		SHitProxyPolygon hitRect( m_leapDmgRect );
		SHitTestResult result;
		if( pPlayer->HitTest( &hitRect, globalTransform, &result ) )
		{
			SDamageContext context;
			context.nDamage = m_nLeapDamage;
			context.hitPos = result.hitPoint2;
			context.hitDir = m_leapHitDir;
			context.hitDir.x *= m_nDir;
			context.pSource = this;
			pPlayer->Damage( context );
		}
	}
	bool bModuleActivated = m_bActivate && m_bActivateOK && !m_bForceDeactivate && m_nLeapFixedVelTimeLeft > 0;
	UpdateModules( bModuleActivated );
	UpdateImg();
}

bool CBotTypeALeap::Damage( SDamageContext& context )
{
	if( !m_bActivate && !m_bActivateOK )
		context.nDamage = 0;

	bool b;
	if( m_bIsLeaping && m_nLeapFixedVelTimeLeft )
		b = CCharacter::Damage( context );
	else
	{
		b = CCommonMoveableObject::Damage( context );
		if( b )
		{
			if( context.nType >= eDamageHitType_Kick_Special )
			{
				m_bIsLeaping = false;
				m_nLeapFixedVelTimeLeft = 0;
			}
		}
	}
	if( ( b || context.nType == eDamageHitType_Alert ) && m_bActivateOnDamage )
	{
		if( m_bActivate )
			m_nDamageTick = m_nDeactivateTime;
		else if( !m_nDamageTick )
			m_nDamageTick = m_nDamageActivateTime;
	}
	return b;
}

bool CBotTypeALeap::AICanHitPlatform()
{
	if( m_bIsLeaping )
	{
		auto pPlayer = SafeCast<CPlayer0>( GetLevel()->GetPlayer() );
		if( pPlayer->GetLastLandPoint().y + m_fLandPointCheckOfs < y )
			return false;
	}
	return true;
}

void CBotTypeALeap::UpdateImg()
{
	m_p1->bVisible = m_nRespawnTick || m_nStateTransitCurTime > 0;
	auto p = static_cast<CImage2D*>( m_p1->GetRenderObject() );
	int32 nTex = 0;
	if( m_nLeapReadyTimeLeft )
		nTex = 8;
	else if( m_nStateTransitCurTime < m_nStateTransitTime * T_SCL )
		nTex = m_nStateTransitCurTime ? ( m_nStateTransitCurTime * 3 / ( m_nStateTransitTime * T_SCL ) ) + 1 : 0;
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
	auto texRect = m_texRect0;
	texRect.x += texRect.width * nTex;
	if( m_nDir == -1 )
		texRect.x = 2 - texRect.GetRight();
	p->SetTexRect( texRect );
}

void CBotTypeAPatrol::OnTickAfterHitTest()
{
	CEntity* pTestEntities[] = { this, m_p1 };
	int32 nTestEntities = m_p1->Get_HitProxy() ? 2 : 1;
	SIgnoreEntityHit ignoreHit( pTestEntities, nTestEntities );
	CVector2 gravity( 0, -1 );
	CVector2 gravity0( 0, 0 );
	auto pGravity = CanHitPlatform() ? &gravity : &gravity0;

	CVector2 landedEntityOfs( 0, 0 );
	if( m_pLanded )
	{
		if( m_pLanded->GetLevel() != GetLevel() || !CanFindFloor() )
			m_pLanded = NULL;
		else if( m_lastLandedEntityTransform != m_pLanded->GetGlobalTransform() )
		{
			auto oldTrans = m_lastLandedEntityTransform;
			auto newTrans = m_pLanded->GetGlobalTransform();
			CVector2 oldPos = GetPosition();
			CVector2 localPos = oldTrans.MulTVector2PosNoScale( oldPos );
			CVector2 newPos = newTrans.MulVector2Pos( localPos );

			CVector2 landVelocity = ( newPos - oldPos ) / GetLevel()->GetDeltaTime() - m_vel;

			float fNormalSpeed = landVelocity.Dot( gravity );
			if( fNormalSpeed > m_fMaxFallSpeed )
			{
				m_pLanded = NULL;
			}
			else
			{
				CVector2 normalVelocity = gravity * fNormalSpeed;
				m_vel = m_vel + normalVelocity;

				CVector2 ofs = newPos - oldPos;
				m_lastLandedEntityTransform = m_pLanded->GetGlobalTransform();
				landedEntityOfs = ofs;
			}
		}
	}
	CVector2 tangent( -gravity.y, gravity.x );
	m_bForceDeactivate = false;
	if( !HandlePenetration( pTestEntities, nTestEntities, pGravity, m_pLanded, &landedEntityOfs ) )
	{
		pGravity = &gravity0;
		if( !m_moveData.ResolvePenetration( this, &m_vel, 0, m_pLanded, &landedEntityOfs, pGravity, pTestEntities, nTestEntities ) )
		{
			m_bForceDeactivate = true;
			return;
		}
	}
	
	m_bIsWalking = m_bActivate && m_bActivateOK && m_nAttackTimeLeft == 0 && m_pLanded;
	auto pLevel = GetLevel();
	SRaycastResult res[3];
	CVector2 dPos;
	auto moveDir = m_pLanded ? CVector2( m_groundNorm.y, -m_groundNorm.x ) : CVector2( -gravity.y, gravity.x );
	if( m_bIsWalking )
		m_vel = moveDir * m_nDir * m_fMoveSpeed;
	dPos = HandleCommonMove();
	dPos = dPos + landedEntityOfs;
	auto vel0 = m_vel;
	m_moveData.TryMove1( this, nTestEntities, pTestEntities, dPos, m_vel, m_fFrac, pGravity, res );
	if( m_bIsWalking )
	{
		for( int i = 0; i < ELEM_COUNT( res ); i++ )
		{
			if( !res[i].pHitProxy )
				break;
			if( res[i].normal.Dot( moveDir * m_nDir ) < -0.7f )
			{
				m_nDir = -m_nDir;
				break;
			}
		}
	}
	if( CanFindFloor() )
	{
		FindFloor( pTestEntities, nTestEntities, gravity );
		if( m_pLanded && m_bIsWalking && FallDetect( gravity ) )
			m_nDir = -m_nDir;
	}
	else
		m_pLanded = NULL;

	auto pPlayer = pLevel->GetPlayer();
	if( m_nAttackCDLeft )
		m_nAttackCDLeft--;
	m_bModuleActive = false;
	if( m_bActivate && m_bActivateOK && !m_bForceDeactivate )
	{
		auto g = GetGlobalTransform();
		CMatrix2D mats[2] = { g, m_p1->GetGlobalTransform() };
		CVector2 ofs = gravity;
		SRaycastResult result;
		if( m_nAttackTimeLeft )
		{
			if( m_nAttackTimeLeft <= m_nAttackTime - m_nAttackReadyTime )
				m_bModuleActive = true;
			else if( m_nKickCounter || m_nImpactTick )
			{
				m_nAttackCDLeft = m_nAttackCD;
				m_nAttackTimeLeft = m_nAttackTime;
			}
			m_nAttackTimeLeft--;
		}
		else if( !m_nKickCounter && !m_nImpactTick && !m_nAttackCDLeft && pPlayer )
		{
			auto p = g.MulTVector2Pos( pPlayer->GetPosition() );
			if( m_rectDetect.Contains( p ) && m_pLanded )
			{
				m_nAttackCDLeft = m_nAttackCD;
				m_nAttackTimeLeft = m_nAttackTime;
				m_vel = gravity * m_vel.Dot( gravity );
				m_nDir = p.x > 0 ? 1 : -1;
			}
		}
	}
	else
		m_nAttackTimeLeft = 0;
	PostMove( nTestEntities, pTestEntities );

	UpdateModules( m_bModuleActive );
	UpdateImg();
}

bool CBotTypeAPatrol::Damage( SDamageContext & context )
{
	if( !m_bActivate && !m_bActivateOK )
		context.nDamage = 0;
	bool b = CCommonMoveableObject::Damage( context );
	if( ( b || context.nType == eDamageHitType_Alert ) && m_bActivateOnDamage )
	{
		if( m_bActivate )
			m_nDamageTick = m_nDeactivateTime;
		else if( !m_nDamageTick )
			m_nDamageTick = m_nDamageActivateTime;
	}
	return b;
}

bool CBotTypeAPatrol::FallDetect( const CVector2& gravityDir )
{
	auto detectRect = m_fallDetectRect;
	if( m_nDir == -1 )
		detectRect.x = -detectRect.GetRight();
	SHitProxyPolygon hit( detectRect );
	static vector<CHitProxy*> vecResult;
	static vector<SHitTestResult> vecHitResult;
	auto& hitTestMgr = GetLevel()->GetHitTestMgr( GetUpdateGroup() );
	hitTestMgr.HitTest( &hit, globalTransform, vecResult, &vecHitResult );
	bool b = true;
	for( int i = 0; i < vecResult.size(); i++ )
	{
		auto pEntity = static_cast<CEntity*>( vecResult[i] );
		if( GetHitChannnel()[pEntity->GetHitType()] )
		{
			if( pEntity->CanHit( this ) && CanHit( pEntity ) )
			{
				b = false;
				break;
			}
		}
		else if( ( pEntity == m_pLanded || m_pLanded->GetHitType() == eEntityHitType_Platform && GetPlatformChannel()[pEntity->GetHitType()] ) )
		{
			auto normal = vecHitResult[i].normal;
			normal.Normalize();
			if( normal.Dot( gravityDir ) <= -0.99f && pEntity->CanHit( this ) && CanHit( pEntity ) )
			{
				b = false;
				break;
			}
		}
	}
	vecResult.resize( 0 );
	vecHitResult.resize( 0 );
	return b;
}

bool CBotTypeAPatrol::CanFindFloor()
{
	return !m_nKickCounter && !m_nImpactTick;
}

void CBotTypeAPatrol::FindFloor( CEntity** pTestEntities, int32 nTestEntities, const CVector2& gravityDir )
{
	CMatrix2D trans = GetGlobalTransform();
	CVector2 dir = gravityDir;
	CVector2 ofs = dir * 1.0f;
	CMatrix2D mats[] = { GetGlobalTransform(), m_p1->GetGlobalTransform() };
	SRaycastResult result;
	CVector2 gravity0( 0, 0 );
	auto pGravity = CanHitPlatform() ? &dir : &gravity0;
	auto pNewLandedEntity = SafeCast<CCharacter>( m_moveData.DoSweepTest1( this, nTestEntities, pTestEntities, mats, ofs, MOVE_SIDE_THRESHOLD, pGravity, &result ) );

	if( pNewLandedEntity && m_vel.Dot( result.normal ) < 1.0f && result.normal.Dot( dir ) < -0.5f )
	{
		m_pLanded = pNewLandedEntity;
		SetPosition( GetPosition() + dir * result.fDist );
		m_groundNorm = result.normal;
		m_vel = m_vel - m_groundNorm * m_vel.Dot( m_groundNorm );
		m_lastLandedEntityTransform = m_pLanded->GetGlobalTransform();
	}
	else
		m_pLanded = NULL;
}

void CBotTypeAPatrol::UpdateImg()
{
	auto nDeltaTick = GetLevel()->GetDeltaTick();
	m_p1->bVisible = m_nRespawnTick || m_nStateTransitCurTime > 0;
	auto p = static_cast<CImage2D*>( m_p1->GetRenderObject() );
	int32 nTex = 0;
	if( m_nAttackTimeLeft )
		nTex = 2 + m_nStateTransitFrames + m_nWalkAnimFrames;
	else if( m_nStateTransitCurTime < m_nStateTransitTime * T_SCL )
		nTex = m_nStateTransitCurTime ? ( m_nStateTransitCurTime * m_nStateTransitFrames / ( m_nStateTransitTime * T_SCL ) ) + 1 : 0;
	else
	{
		nTex = m_nStateTransitFrames + 1;
		if( m_bIsWalking )
		{
			nTex = m_nStateTransitFrames + 2 + m_nAnimTick / ( m_nWalkAnimSpeed * T_SCL );
			m_nAnimTick += nDeltaTick;
			if( m_nAnimTick >= m_nWalkAnimFrames * m_nWalkAnimSpeed * T_SCL )
				m_nAnimTick = 0;
		}
		else
			m_nAnimTick = 0;
	}
	auto texRect = m_texRect0;
	texRect.x += texRect.width * nTex;
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
		REGISTER_MEMBER( m_nStateTransitFrames )
		REGISTER_MEMBER( m_nRespawnTime )
		REGISTER_MEMBER_TAGGED_PTR( m_p1, 1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBotTypeALeap )
		REGISTER_BASE_CLASS( CBotTypeA )
		REGISTER_MEMBER( m_rectDetect )
		REGISTER_MEMBER( m_texRect0 )
		REGISTER_MEMBER( m_nLeapCD )
		REGISTER_MEMBER( m_nLeapReadyTime )
		REGISTER_MEMBER( m_leapVel )
		REGISTER_MEMBER( m_nSmartLeap )
		REGISTER_MEMBER( m_bPushLeap )
		REGISTER_MEMBER( m_nLeapFixedVelTime )
		REGISTER_MEMBER( m_fLeapAnimSpd )
		REGISTER_MEMBER( m_nLeapDamage )
		REGISTER_MEMBER( m_leapDmgRect )
		REGISTER_MEMBER( m_leapHitDir )
		REGISTER_MEMBER( m_fLandPointCheckOfs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBotTypeAPatrol )
		REGISTER_BASE_CLASS( CBotTypeA )
		REGISTER_MEMBER( m_rectDetect )
		REGISTER_MEMBER( m_texRect0 )
		REGISTER_MEMBER( m_nAttackCD )
		REGISTER_MEMBER( m_nAttackTime )
		REGISTER_MEMBER( m_nAttackReadyTime )
		REGISTER_MEMBER( m_fMoveSpeed )
		REGISTER_MEMBER( m_nWalkAnimFrames )
		REGISTER_MEMBER( m_nWalkAnimSpeed )
		REGISTER_MEMBER( m_fallDetectRect )
	REGISTER_CLASS_END()
}