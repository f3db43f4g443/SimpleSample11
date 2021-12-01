#include "stdafx.h"
#include "Player.h"
#include "Stage.h"
#include "World.h"
#include "Common/ResourceManager.h"
#include "MyLevel.h"
#include "MyGame.h"
#include "Entities/Bullet.h"
#include "Entities/PlayerMisc.h"

CPlayer::CPlayer( const SClassCreateContext& context )
	: CCharacter( context )
{
	SET_BASEOBJECT_ID( CPlayer );
	m_moveData.bPlatformChannel[eEntityHitType_Platform] = true;
	m_moveData.bPlatformChannel[eEntityHitType_Platform_1] = true;
	m_nDir = 1;
}

void CPlayer::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	UpdateAnim( CVector2( 0, -1 ) );
	if( GetLevel() )
	{
		auto& hitTestMgr = GetLevel()->GetHitTestMgr();
		for( int i = 0; i < 3; i++ )
			hitTestMgr.Add( m_pHit[i] );
	}
	m_lastFramePos = GetPosition();
}

void CPlayer::OnRemovedFromStage()
{
	if( GetLevel() )
	{
		auto& hitTestMgr = GetLevel()->GetHitTestMgr();
		for( int i = 0; i < 3; i++ )
			hitTestMgr.Remove( m_pHit[i] );
	}
	CCharacter::OnRemovedFromStage();
}

void CPlayer::UpdateInput()
{
	m_nMoveState = CGame::Inst().IsKey( 'D' ) - CGame::Inst().IsKey( 'A' );
	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();
	if( m_nCurState == eState_Stand || m_nCurState == eState_Walk || m_nCurState == eState_Jump )
	{
		if( m_nMoveState )
			m_nDir = m_nMoveState;
		if( m_pLanded )
		{
			m_nJumpState = m_nLandTime;
			m_nSlideAirCDLeft = m_nSlideAirCD;
		}
		else if( m_nJumpState > 0 )
			m_nJumpState--;
		if( m_nSlideAirCDLeft )
			m_nSlideAirCDLeft--;
		if( m_nLevel >= ePlayerLevel_Kick && CGame::Inst().IsKey( 'K' ) || CGame::Inst().IsKey( 'L' ) )
		{
			JumpCancel( gravityDir );
			if( CGame::Inst().IsKey( 'J' ) )
				CGame::Inst().ForceKeyRelease( 'J' );
			m_nCurState = eState_Kick_Ready;
			m_nStateTick = 0;
			m_nBufferedInput = 0;
			if( CGame::Inst().IsKey( 'K' ) )
				m_nBufferedInput |= 16;
			if( CGame::Inst().IsKey( 'L' ) )
				m_nBufferedInput |= 32;
			m_vel = CVector2( 0, 0 );
		}
		else if( m_nLevel >= ePlayerLevel_Slide && CGame::Inst().IsKey( 'S' ) && CGame::Inst().IsKey( 'J' ) )
		{
			CVector2 tangentDir( -gravityDir.y, gravityDir.x );
			if( m_pLanded )
			{
				m_nCurState = eState_Slide;
				m_vel = tangentDir * m_fSlideSpeed0 * m_nDir + gravityDir * m_vel.Dot( gravityDir );
				m_nStateTick = 0;
				CGame::Inst().ForceKeyRelease( 'J' );
			}
			else if( m_nLevel >= ePlayerLevel_Slide_Air && !m_nSlideAirCDLeft )
			{
				m_nCurState = eState_Slide_Air;
				m_vel = ( tangentDir * m_nDir + gravityDir ) * m_fSlideSpeed0;
				m_nStateTick = 0;
				CGame::Inst().ForceKeyRelease( 'J' );
			}
		}
		else
		{
			if( m_nJumpState > 0 )
			{
				if( CGame::Inst().IsKey( 'J' ) )
				{
					m_vel = m_vel - gravityDir * m_fJumpSpeed;
					m_nJumpState = -m_nJumpHoldTime;
				}
			}
			else if( m_nJumpState < 0 )
			{
				m_nJumpState++;
				if( !CGame::Inst().IsKey( 'J' ) )
					JumpCancel( gravityDir );
			}
		}
	}
	if( m_nCurState == eState_Slide_1 && m_nMoveState )
	{
		m_nCurState = eState_Stand_1;
		m_nStateTick = 0;
		m_nBufferedInput = 0;
	}
	if( m_nCurState == eState_Stand_1 || m_nCurState == eState_Walk_1 || m_nCurState == eState_Slide_1 && m_nStateTick >= m_nSlideDashTime )
	{
		if( CGame::Inst().IsKey( 'J' ) && m_nMoveState )
		{
			if( m_nMoveState * m_nDir < 0 )
			{
				BackFlip();
			}
			else if( m_pLanded )
			{
				m_nCurState = eState_Stand_1_Ready;
				m_nStateTick = 0;
			}
			else if( m_nCurState != eState_Glide || m_vel.Dot( gravityDir ) >= m_glideVel.y - 1 )
				DashGrab();
		}
		else
		{
			if( m_nLevel >= ePlayerLevel_Glide && CGame::Inst().IsKey( 'W' ) && !m_pLanded )
				Glide();
		}
	}
	if( m_nCurState == eState_Glide )
	{
		CVector2 tangent( -gravityDir.y, gravityDir.x );
		auto tanVel = m_vel.Dot( tangent ) * m_nDir;
		if( tanVel > m_fBoostThresholdSpeed && CGame::Inst().IsKey( 'J' ) && m_nMoveState == -m_nDir )
		{
			m_nCurState = eState_Boost;
			m_boostState.nTick = 0;
			m_boostState.nTick1 = ceil( ( tanVel - m_fBoostThresholdSpeed ) * 0.5f / ( m_boostAcc.x * GetStage()->GetElapsedTimePerTick() ) );
			m_vel = m_vel - tangent * m_nDir * ( m_fBoostThresholdSpeed - m_fBoostMinSpeed );
			m_fFuel = m_fBoostThresholdSpeed - m_fBoostMinSpeed;
		}
		else if( CGame::Inst().IsKey( 'J' ) && m_nMoveState == m_nDir )
			DashGrab();
		else if( CGame::Inst().IsKey( 'S' ) )
		{
			m_nCurState = eState_Glide_Fall;
			m_nStateTick = 0;
		}
	}
	if( m_nCurState == eState_Dash_Grab && !m_pGrabbed )
	{
		if( !CGame::Inst().IsKey( 'J' ) )
			DashFall();
	}
	
	if( m_nCurState == eState_Stand_1_Ready )
	{
		if( m_nMoveState * m_nDir < 0 )
			BackFlip();
	}
	if( m_nCurState == eState_Stand_1_Ready || m_nCurState == eState_Dash )
	{
		if( CGame::Inst().IsKeyDown( 'W' ) )
			m_nBufferedInput = 1;
	}
	if( m_nCurState == eState_Roll )
	{
		if( CGame::Inst().IsKey( 'J' ) )
		{
			/*if( !CGame::Inst().IsKey( 'W' ) && m_nMoveState && !m_pLanded )
				RollDashGrab( m_nMoveState );
			else
			{
				m_vel = gravityDir * -m_fJumpSpeed;
				RollStandUp();
			}*/
			m_vel = gravityDir * -m_fJumpSpeed;
			RollStandUp();
		}
		else if( CGame::Inst().IsKey( 'W' ) )
			RollStandUp();
		else if( CGame::Inst().IsKey( 'S' ) )
			RollStop();
	}
	if( m_nCurState == eState_Slide_1 && m_nStateTick < m_nSlideDashTime || m_nCurState == eState_Roll_Recover )
	{
		int8 bJump = m_nBufferedInput & 1;
		int8 nDir = m_nBufferedInput >> 1;
		if( m_nMoveState )
			nDir = m_nMoveState * m_nDir > 0 ? 1 : 2;
		if( CGame::Inst().IsKey( 'J' ) )
		{
			bJump = 1;
			CGame::Inst().ForceKeyRelease( 'J' );
		}
		m_nBufferedInput = bJump | ( nDir << 1 );
	}

	if( m_nCurState == eState_Kick_Ready )
	{
		if( CGame::Inst().IsKey( 'K' ) )
			m_nBufferedInput |= 16;
		if( CGame::Inst().IsKey( 'L' ) )
			m_nBufferedInput |= 32;
		auto lo = m_nBufferedInput & 15;
		if( CGame::Inst().IsKey( 'J' ) )
			lo = 4;
		if( lo < 4 )
		{
			if( CGame::Inst().IsKey( 'W' ) )
				lo = 2;
			if( m_nMoveState )
			{
				m_nDir = m_nMoveState;
				lo = lo | 1;
			}
		}
		m_nBufferedInput = lo | ( m_nBufferedInput & ~15 );
	}
	else if( m_nCurState == eState_Kick )
	{
		if( m_kickState.nAni < eAni_Kick_Special_Begin || m_kickState.nAni == eAni_Kick_Recover )
		{
			if( m_nLevel >= ePlayerLevel_Kick_Spin && CGame::Inst().IsKey( 'J' ) && ! m_kickState.nFlag1 )
			{
				Kick( eAni_Kick_Spin );
				m_nBufferedInput = 0;
			}
		}
		if( m_kickState.nAni < eAni_Kick_Base_End )
		{
			if( CGame::Inst().IsKeyDown( 'K' ) )
				m_nBufferedInput = m_nBufferedInput | 16;
			if( CGame::Inst().IsKeyDown( 'L' ) )
				m_nBufferedInput = m_nBufferedInput | 32;
			if( CGame::Inst().IsKeyDown( 'S' ) )
				m_nBufferedInput = m_nBufferedInput | 8;
			if( m_nDir == 1 && CGame::Inst().IsKeyDown( 'A' ) || m_nDir == -1 && CGame::Inst().IsKeyDown( 'D' ) )
				m_nBufferedInput = m_nBufferedInput | 4;
			if( m_nDir == 1 && CGame::Inst().IsKeyDown( 'D' ) || m_nDir == -1 && CGame::Inst().IsKeyDown( 'A' ) )
				m_nBufferedInput = m_nBufferedInput | 1;
			if( CGame::Inst().IsKeyDown( 'W' ) )
				m_nBufferedInput = m_nBufferedInput | 2;
		}
		else if( m_kickState.nAni == eAni_Kick_Recover )
		{
			if( m_kickState.bHit && ( CGame::Inst().IsKey( 'K' ) || CGame::Inst().IsKey( 'L' ) ) )
			{
				if( m_nLevel >= ePlayerLevel_Kick_Rev && m_nDir == 1 && CGame::Inst().IsKey( 'A' ) || m_nDir == -1 && CGame::Inst().IsKey( 'D' ) )
				{
					uint8 nFlag = m_kickState.nFlag;
					Kick( eAni_Kick_Rev );
					m_kickState.nFlag = nFlag;
					if( CGame::Inst().IsKey( 'K' ) )
						m_kickState.nType0 |= 1;
					if( CGame::Inst().IsKey( 'L' ) )
						m_kickState.nType0 |= 2;
				}
				else if( m_nLevel >= ePlayerLevel_Kick_Stomp && CGame::Inst().IsKey( 'S' ) )
				{
					uint8 nFlag = m_kickState.nFlag;
					Kick( eAni_Kick_Stomp );
					m_kickState.nFlag = nFlag;
					if( CGame::Inst().IsKey( 'K' ) )
						m_kickState.nType0 |= 1;
					if( CGame::Inst().IsKey( 'L' ) )
						m_kickState.nType0 |= 2;
				}
			}
		}
	}
	//Shoot( CGame::Inst().IsKey( 'J' ) );
}

bool CPlayer::Damage( SDamageContext& context )
{
	if( !context.nDamage )
		return false;
	m_nShieldRecoverCDLeft = m_nShieldHitRecoverCD;

	auto gravityDir = GetLevel()->GetGravityDir();
	auto hitDir = context.hitDir;
	float l = hitDir.Normalize();
	if( hitDir.Dot( gravityDir * -1 ) >= 0.7f )
	{
		if( m_nLevel >= ePlayerLevel_Glide_Fall_Block && m_nCurState == eState_Glide_Fall )
		{
			m_nShield = m_nMaxShield;
			m_nCurState = eState_Stand_1;
			m_nShieldRecoverCDLeft = 0;
			CVector2 tangentDir( -gravityDir.y, gravityDir.x );
			m_vel = tangentDir * m_vel.Dot( tangentDir ) - gravityDir * Max( -gravityDir.Dot( m_vel ), context.fDamage1 * m_fShieldJumpSpeed );
			return true;
		}
		if( m_nShield )
		{
			m_nShield = Max( 0, m_nShield - context.nDamage );
			CVector2 tangentDir( -gravityDir.y, gravityDir.x );
			m_vel = tangentDir * m_vel.Dot( tangentDir ) - gravityDir * Max( -gravityDir.Dot( m_vel ), context.fDamage1 * m_fShieldJumpSpeed );
			return true;
		}
	}

	if( l > 0 )
	{
		if( !Knockback( context.hitDir ) && !context.nSourceType )
			return false;
	}
	m_nDamageLeft = Max( m_nDamageLeft, context.nDamage );
	m_nHpRecoverCDLeft = m_nHpRecoverCD;
	return true;
}

void CPlayer::OnTickBeforeHitTest()
{
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEnvLayer = SafeCast<CLevelEnvLayer>( (CEntity*)pManifold->pOtherHitProxy );
		if( pEnvLayer && pEnvLayer != GetLevel()->GetEnv() )
		{
			GetLevel()->ChangeToEnvLayer( pEnvLayer );
			break;
		}
	}
	CCharacter::OnTickBeforeHitTest();
}

void CPlayer::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( m_bKilled )
	{
		bVisible = false;
		return;
	}
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pCharacter = SafeCast<CCharacter>( (CEntity*)pManifold->pOtherHitProxy );
		if( pCharacter )
			m_nDamageLeft = Max( m_nDamageLeft, pCharacter->GetDmgToPlayer() );
	}
	CheckGrab();
	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();

	const CMatrix2D oldTransform = GetGlobalTransform();
	CMatrix2D curTransform = oldTransform;
	const CVector2 oldPos = oldTransform.GetPosition();
	CVector2 curPos = oldPos;
	float fDeltaTime = GetLevel()->GetElapsedTimePerTick();

	CVector2 landedEntityOfs( 0, 0 );
	if( m_pLanded )
	{
		if( m_pLanded->GetLevel() != GetLevel() || !CanFindFloor() )
			m_pLanded = NULL;
		else if( lastLandedEntityTransform != m_pLanded->GetGlobalTransform() )
		{
			auto oldTrans = lastLandedEntityTransform;
			auto newTrans = m_pLanded->GetGlobalTransform();
			CVector2 oldPos = GetPosition();
			CVector2 localPos = oldTrans.MulTVector2PosNoScale( oldPos );
			CVector2 newPos = newTrans.MulVector2Pos( localPos );

			CVector2 landVelocity = ( newPos - oldPos ) / GetLevel()->GetElapsedTimePerTick() - m_vel;

			float fNormalSpeed = landVelocity.Dot( gravityDir );
			if( fNormalSpeed > m_fMaxFallSpeed )
			{
				m_pLanded = NULL;
			}
			else
			{
				CVector2 normalVelocity = gravityDir * fNormalSpeed;
				m_vel = m_vel + normalVelocity;

				CVector2 ofs = newPos - oldPos;
				lastLandedEntityTransform = m_pLanded->GetGlobalTransform();
				landedEntityOfs = ofs;
			}
		}
	}

	if( m_nKnockBackTime > 0 )
		m_nKnockBackTime--;
	if( m_bKnockback )
	{
		m_nBufferedInput = 0;

		switch( m_nCurState )
		{
		case eState_Kick:
			KickBreak();
			break;
		case eState_Slide:
		case eState_Slide_Air:
			SlideCancel();
			break;
		case eState_BackFlip:
			m_nCurState = eState_Stand_1;
			m_nStateTick = 0;
			m_pHit[1]->SetPosition( m_pHit[0]->GetPosition() );
			break;
		default:
			if( m_nCurState < eState_Slide_1 )
			{
				m_nCurState = eState_Stand;
				m_nStateTick = 0;
				m_nJumpState = 0;
				m_pHit[2]->SetPosition( m_pHit[0]->GetPosition() );
			}
			else if( m_nCurState <= eState_Fly )
			{
				m_nCurState = eState_Stand_1;
				m_nStateTick = 0;
				m_pHit[1]->SetPosition( m_pHit[0]->GetPosition() );
			}
			else if( m_nCurState < eState_BackFlip_1 )
				ForceRoll();
		}
	}
	else
		UpdateInput();

	CEntity* pTestEntities[] = { m_pHit[0], m_pHit[1], m_pHit[2] };
	CVector2 gravity0( 0, 0 );
	CVector2* pGravity = CanHitPlatform() ? &gravityDir : &gravity0;
	if( !m_moveData.ResolvePenetration( this, &m_vel, 0, m_pLanded, &landedEntityOfs, pGravity, pTestEntities, ELEM_COUNT( pTestEntities ) ) )
	{
		if( pGravity == &gravity0 || ( pGravity = &gravity0, !m_moveData.ResolvePenetration( this, &m_vel, 0, m_pLanded, &landedEntityOfs, pGravity, pTestEntities, ELEM_COUNT( pTestEntities ) ) ) )
		{
			if( !m_moveData.ResolvePenetration( this, &m_vel, 0, m_pLanded, &landedEntityOfs, pGravity, pTestEntities, 1 ) )
			{
				Crush();
				return;
			}
			ForceRoll();
		}
	}

	float fHitMove1 = 0, fHitMove2 = 0;
	bool bFixVel = false;
	CVector2 fixVel( 0, 0 );

	switch( m_nCurState )
	{
	case eState_Kick:
	{
		if( m_kickState.nAni < eAni_Kick_Special_Begin )
		{
			int32 nKickType0;
			if( m_kickState.nAni >= eAni_Kick_Finish_Begin )
				nKickType0 = 1 + m_kickState.nFlag;
			else
				nKickType0 = m_kickState.nAni / 3;

			if( m_kickState.nTick == 0 )
			{
				CVector2 tangentDir( -gravityDir.y, gravityDir.x );
				tangentDir = tangentDir * m_nDir;
				auto kickVel = m_kickVel[m_kickState.nAni];
				m_vel = tangentDir * kickVel.x - gravityDir * kickVel.y;
			}
			if( m_kickState.nAni == eAni_Kick_Stomp && m_kickState.nTick == m_kickStompAnimFrame.x )
			{
				CVector2 tangentDir( -gravityDir.y, gravityDir.x );
				tangentDir = tangentDir * m_nDir;
				auto kickVel = m_kickVel[m_kickState.nAni];
				m_vel = tangentDir * kickVel.x + gravityDir * kickVel.y;
			}
			int32 nKickFrame = 0;
			if( m_kickState.nAni == eAni_Kick_Rev )
				nKickFrame = m_kickRevAnimFrame.x;
			else if( m_kickState.nAni == eAni_Kick_Stomp )
				nKickFrame = m_kickStompAnimFrame.x;
			if( m_kickState.nTick == nKickFrame )
			{
				auto pKick = SafeCast<CKick>( m_pKick[nKickType0]->GetRoot()->CreateInstance() );
				m_pCurAttackEffect = pKick;
				auto kickOfs = m_kickOffset[m_kickState.nAni];
				kickOfs.x *= m_nDir;
				kickOfs.z = m_nDir == 1 ? kickOfs.z : 180 - kickOfs.z;
				m_nJumpState = m_pLanded ? 0 : 1;
				pKick->SetPosition( CVector2( kickOfs.x, kickOfs.y ) );
				pKick->SetRotation( kickOfs.z / 180 * PI );
				pKick->SetParentEntity( this );
				pKick->SetOwner( this );
				pKick->SetRenderParent( GetLevel() );
			}
			auto nKickDashFrame = m_nKickDashDelay + nKickFrame;

			if( !m_kickState.nFlag1 && m_kickState.nTick <= nKickDashFrame )
			{
				if( CGame::Inst().IsKey( 'K' ) )
					m_kickState.nType0 |= 1;
				if( CGame::Inst().IsKey( 'L' ) )
					m_kickState.nType0 |= 2;
			}
			if( m_kickState.bHit && !m_kickState.nFlag1 && m_kickState.nTick == nKickDashFrame )
			{
				if( m_nLevel >= ePlayerLevel_Kick_Heavy && m_kickState.nType0 == 3 )
				{
					m_nBufferedInput = 0;
					m_kickState.nTick1 = m_nKickSpinDashTime[nKickType0];
					m_kickState.nFlag1 = 1;
					m_nDir = -m_nDir;
					auto pKick = SafeCast<CCharacter>( m_pKickSpin[nKickType0]->GetRoot()->CreateInstance() );
					pKick->SetParentEntity( this );
					pKick->SetOwner( this );
					KickMorph( pKick );
				}
				else if( m_nLevel >= ePlayerLevel_Kick_Mid && m_kickState.nType0 == 2 )
				{
					m_kickState.nTick1 = m_nKickDashTime;
					if( m_pCurAttackEffect )
						SafeCast<CKick>( m_pCurAttackEffect.GetPtr() )->Extent( m_nKickDashTime );
				}
			}
			if( m_kickState.nTick1 > 0 )
			{
				auto fAngle = m_kickOffset[m_kickState.nAni].z;
				fAngle = ( fAngle = ( m_kickState.nFlag1 ? -m_nDir : m_nDir ) == 1 ? fAngle : 180 - fAngle ) / 180 * PI;
				CVector2 c( cos( fAngle ), sin( fAngle ) );
				CVector2 tangentDir( -gravityDir.y, gravityDir.x );
				c = CVector2( c.x * tangentDir.x - c.y * tangentDir.y, c.x * tangentDir.y + c.y * tangentDir.x );
				bFixVel = true;
				if( m_kickState.nFlag1 )
					fixVel = m_vel = c * m_nKickSpinDashSpeed;
				else
					fixVel = m_vel = c * m_fKickDashSpeed[nKickType0] * ( m_kickState.nTick1 - 0.5f ) / m_nKickDashTime;
			}
			else if( m_kickState.nTick1 < 0 )
			{
				bFixVel = true;
				fixVel = m_vel = CVector2( 0, 0 );
			}
		}
		break;
	}
	case eState_Slide:
	{
		float fCur = m_pHit[2]->x * m_nDir;
		float fTarget = m_fHitOfs2 * ( m_nStateTick + 1 ) / ( m_nSlideAnimSpeed * 2 );
		fHitMove2 = fTarget - fCur;
		break;
	}
	case eState_Slide_Air:
	{
		float fCur = m_pHit[2]->x * m_nDir;
		float fTarget = m_fHitOfs2;
		fHitMove2 = fTarget - fCur;
		m_vel = CVector2( gravityDir.x - gravityDir.y * m_nDir, gravityDir.y + gravityDir.x ) * m_fSlideSpeed0;
		break;
	}
	case eState_Fly:
	{
		bFixVel = true;
		CVector2 speed( m_fFlySpeed * m_nDir, 0 );
		float fFuelConsume = m_fFlyFuelConsume;
		int8 nMoveState1 = CGame::Inst().IsKey( 'W' ) - CGame::Inst().IsKey( 'S' );
		if( m_nMoveState )
		{
			fFuelConsume += m_fFlyFuelConsume1;
			speed.x += m_nMoveState * m_fFlySpeed1;
		}
		if( nMoveState1 )
		{
			fFuelConsume += m_fFlyFuelConsume1;
			speed.y += nMoveState1 * m_fFlySpeed1;
		}
		m_fFuel = Max( 0.0f, m_fFuel - fFuelConsume * GetStage()->GetElapsedTimePerTick() );
		fixVel = m_vel = speed;
	}
	case eState_Dash:
	{
		CVector2 tangentDir( -gravityDir.y, gravityDir.x );
		tangentDir = tangentDir * m_nDir;
		m_vel = tangentDir * m_fDashSpeed - gravityDir * m_fDashSpeedY;
		break;
	}
	case eState_Dash_Grab:
	{
		CVector2 tangentDir( -gravityDir.y, gravityDir.x );
		tangentDir = tangentDir * m_nDir;
		m_vel = m_vel + tangentDir * ( m_fDashSpeed - m_vel.Dot( tangentDir ) );
		break;
	}
	case eState_Punch:
	{
		CVector2 tangentDir( -gravityDir.y, gravityDir.x );
		tangentDir = tangentDir * m_nDir;
		int8 yMove = CGame::Inst().IsKey( 'W' ) - CGame::Inst().IsKey( 'S' );
		bFixVel = true;
		fixVel = m_vel = tangentDir * m_punchSpeed.x - gravityDir * m_punchSpeed.y * yMove;
		break;
	}
	case eState_Roll_Stand_Up:
	{
		float fCur = m_pHit[1]->y - m_pHit[0]->y;
		float fTarget = m_fHitOfs1 * ( m_nStateTick + 1 ) / ( m_nStandUpAnimSpeed * 2 );
		fHitMove1 = fTarget - fCur;
		break;
	}
	case eState_Roll_Recover:
	{
		float fCur = m_pHit[2]->x * m_nDir;
		float fTarget = m_fHitOfs1 * ( m_nStateTick + 1 ) / ( m_nStandUpAnimSpeed * 2 );
		fHitMove2 = fTarget - fCur;
		break;
	}
	case eState_Roll_Dash:
	{
		float fCur = m_pHit[2]->x * m_nDir;
		float fTarget = m_fHitOfs1 * ( m_nStateTick + 1 ) / m_nStand1ReadyTime;
		fHitMove2 = fTarget - fCur;
		bFixVel = true;
		fixVel = m_vel = CVector2( 0, 0 );
		break;
	}
	case eState_BackFlip:
	{
		float fCur = m_pHit[1]->y - m_pHit[0]->y;
		float fTarget = m_fHitOfs1 * ( m_nStateTick + 1 ) / m_nBackFlipAnimSpeed;
		fHitMove1 = fTarget - fCur;
		break;
	}
	case eState_Force_Roll:
		ForceRoll( true );
		break;
	default:
		break;
	}

	auto pos0 = GetPosition(); 
	bool bFalling = IsFalling();
	CVector2 dPos = ( bFixVel ? fixVel : m_vel ) * fDeltaTime;
	CVector2 grabTarget( 0, 0 );
	if( m_pGrabbed )
	{
		grabTarget = m_pGrabbed->GetGlobalTransform().MulVector2Pos( m_grabDesc.grabOfs );
		m_pGrabDetect->ForceUpdateTransform();
		dPos = grabTarget - m_pGrabDetect->globalTransform.GetPosition();
		if( m_nCurState == eState_Dash_Grab && dPos.Length2() > m_fGrabAttachSpeed * m_fGrabAttachSpeed )
		{
			dPos.Normalize();
			dPos = dPos * m_fGrabAttachSpeed;
		}
	}
	else if( m_nCurState != eState_Slide_Air && !bFixVel )
	{
		CVector2 dVelocity = CVector2( 0, 0 );
		float fAcc, t0, t1, fDeltaSpeed;
		CVector2 moveDir;
		CVector2 tangent;
		if( m_pLanded )
			tangent = CVector2( m_groundNorm.y, -m_groundNorm.x );
		else
			tangent = CVector2( -gravityDir.y, gravityDir.x );

		if( !m_bKnockback )
		{
			if( m_nCurState == eState_Kick || m_nCurState == eState_Kick_Ready )
			{
				moveDir = tangent;
				float fTangentVelocity = m_vel.Dot( moveDir );
				if( fTangentVelocity < 0 )
				{
					fTangentVelocity = -fTangentVelocity;
					moveDir = moveDir * -1;
				}
				fAcc = m_pLanded ? m_fStopAcc : m_fAirAcc;
				t0 = Max( 0.0f, Min( fDeltaTime, fTangentVelocity / fAcc ) );
				t1 = fDeltaTime - t0;
				dVelocity = dVelocity - moveDir * ( fAcc * t0 );
				dPos = dPos - moveDir * ( fAcc * t0 * ( t0 * 0.5f + t1 ) );
			}
			if( m_nCurState == eState_Slide || m_nCurState == eState_Slide_1 )
			{
				moveDir = tangent * m_nDir;
				float fTangentVelocity = m_vel.Dot( moveDir );
				fAcc = m_fSlideAcc;
				t0 = Max( 0.0f, Min( fDeltaTime, ( m_fSlideSpeed - fTangentVelocity ) / m_fSlideAcc ) );
				t1 = fDeltaTime - t0;
				dVelocity = dVelocity + moveDir * ( fAcc * t0 );
				dPos = dPos + moveDir * ( fAcc * t0 * ( t0 * 0.5f + t1 ) );
			}
			else if( m_nCurState == eState_Roll || m_nCurState == eState_Roll_Stop )
			{
				if( m_pLanded )
				{
					moveDir = tangent * m_nDir;
					float fTangentVelocity = m_vel.Dot( moveDir );
					fAcc = m_fRollAcc - m_fRollAcc1 * m_nDir * m_nMoveState;
					t0 = Max( 0.0f, Min( fDeltaTime, fTangentVelocity / fAcc ) );
					t1 = fDeltaTime - t0;
					dVelocity = dVelocity + moveDir * ( -fAcc * t0 );
					dPos = dPos + moveDir * ( -fAcc * t0 * ( t0 * 0.5f + t1 ) );
				}
			}
			else if( m_nCurState == eState_Roll_Recover || m_nCurState == eState_Roll_Stand_Up )
			{
				if( m_pLanded )
				{
					int32 nMoveDir = m_nDir;
					moveDir = tangent * m_nDir;
					float fTangentVelocity = m_vel.Dot( moveDir );
					if( fTangentVelocity < 0 )
					{
						moveDir = moveDir * -1;
						nMoveDir = -nMoveDir;
						fTangentVelocity = fTangentVelocity * -1;
					}
					fAcc = m_fRollAcc - m_fRollAcc1 * nMoveDir * ( m_nCurState == eState_Roll_Recover ? m_nDir : 0 );
					t0 = Max( 0.0f, Min( fDeltaTime, fTangentVelocity / fAcc ) );
					t1 = fDeltaTime - t0;
					dVelocity = dVelocity + moveDir * ( -fAcc * t0 );
					dPos = dPos + moveDir * ( -fAcc * t0 * ( t0 * 0.5f + t1 ) );
				}
			}
			else if( m_nCurState == eState_BackFlip_2 )
			{
				moveDir = tangent * m_nDir;
				float fTangentVelocity = m_vel.Dot( moveDir );
				fAcc = m_fBackFlipAcc;
				t0 = Max( 0.0f, Min( fDeltaTime, -fTangentVelocity / fAcc ) );
				t1 = fDeltaTime - t0;
				dVelocity = dVelocity + moveDir * ( fAcc * t0 );
				dPos = dPos + moveDir * ( fAcc * t0 * ( t0 * 0.5f + t1 ) );
			}
			else if( m_nCurState <= eState_Jump || m_nCurState == eState_Stand_1 || m_nCurState == eState_Stand_1_Ready || m_nCurState == eState_Walk_1 )
			{
				moveDir = tangent * m_nMoveState;
				float fSpeedTarget = 0;
				float k = tangent.Dot( moveDir );
				float fTangentVelocity = m_vel.Dot( tangent );
				float fMaxSpeed = ( m_pLanded ? ( m_nCurState <= eState_Jump ? m_fMoveSpeed : m_fMove1Speed ) : m_fAirMaxSpeed ) * k + fSpeedTarget;
				fSpeedTarget = ( m_nCurState <= eState_Jump ? m_fMoveSpeed : m_fMove1Speed ) * k + fSpeedTarget;
				if( !m_pLanded )
				{
					if( k > 0 )
						fSpeedTarget = Min( fMaxSpeed, Max( fSpeedTarget, fTangentVelocity ) );
					else if( k < 0 )
						fSpeedTarget = Max( fMaxSpeed, Min( fSpeedTarget, fTangentVelocity ) );
				}

				bool bIsMoveOrStop = k == 0 ? false : fTangentVelocity * k >= 0 && fTangentVelocity * k < fSpeedTarget * k;
				fAcc = m_pLanded ? ( bIsMoveOrStop ? ( m_nCurState <= eState_Jump ? m_fMoveAcc : m_fMove1Acc ) :
					( m_nCurState <= eState_Jump ? m_fStopAcc : m_fStop1Acc ) ) : m_fAirAcc;
				fDeltaSpeed = fSpeedTarget - fTangentVelocity;
				if( fDeltaSpeed < 0 )
				{
					fDeltaSpeed = -fDeltaSpeed;
					tangent = tangent * -1;
				}
				t0 = Min( fDeltaTime, fDeltaSpeed / fAcc );
				t1 = fDeltaTime - t0;

				dVelocity = dVelocity + tangent * ( fAcc * t0 );
				dPos = dPos + tangent * ( fAcc * t0 * ( t0 * 0.5f + t1 ) );
			}
		}

		if( !m_pLanded )
		{
			float fTargetSpeed = m_fMaxFallSpeed;
			if( m_nCurState == eState_Glide )
				fTargetSpeed = m_glideVel.y;
			float fNormalVelocity = m_vel.Dot( gravityDir );
			fDeltaSpeed = fTargetSpeed - fNormalVelocity;
			float fYAcc = fDeltaSpeed < 0 ? -GetGravity() : GetGravity();
			if( m_nCurState == eState_Boost )
				fYAcc = -m_boostAcc.y;
			t0 = m_nCurState == eState_Boost ? fDeltaTime : Min( fDeltaTime, fDeltaSpeed / fYAcc );
			t1 = fDeltaTime - t0;
			dVelocity = dVelocity + gravityDir * ( fYAcc * t0 );
			dPos = dPos + gravityDir * ( fYAcc * t0 * ( t0 * 0.5f + t1 ) );
			if( m_nCurState == eState_Glide )
			{
				if( fYAcc < 0 && CGame::Inst().IsKey( 'J' ) && CGame::Inst().IsKey( 'W' ) )
				{
					float v = -fYAcc * t0 * m_fGlideVelTransfer;
					m_vel = m_vel + tangent * m_nDir * v;
				}
			}
			else if( m_nCurState == eState_Boost )
			{
				float fXAcc = -m_boostAcc.x;
				float fTargetSpeed = m_fBoostMinSpeed;
				float fTangentVelocity = m_vel.Dot( tangent ) * m_nDir;
				fDeltaSpeed = fTargetSpeed - fTangentVelocity;
				if( fDeltaSpeed >= 0 )
					Fly();
				else
				{
					t0 = Min( fDeltaTime, fDeltaSpeed / fXAcc );
					t1 = fDeltaTime - t0;
					dVelocity = dVelocity + tangent * m_nDir * ( fXAcc * t0 );
					dPos = dPos + tangent * m_nDir * ( fXAcc * t0 * ( t0 * 0.5f + t1 ) );
					m_fFuel += -fXAcc * t0;

					if( !CGame::Inst().IsKey( 'J' ) )
						Fly();
				}
			}
		}
		m_vel = m_vel + dVelocity;

		dPos = dPos + landedEntityOfs;
	}

	GetLevel()->GetHitTestMgr().Update( this );
	CVector2 v0 = m_vel;
	SRaycastResult res[3];
	if( dPos.Length2() > 0 )
		m_moveData.TryMove1( this, ELEM_COUNT( pTestEntities ), pTestEntities, dPos, bFixVel ? fixVel : m_vel, 0, CanHitPlatform() ? &gravityDir : &gravity0, res );
	if( m_nCurState == eState_Slide || m_nCurState == eState_Slide_Air || m_nCurState == eState_Slide_1 || m_nCurState == eState_Dash || m_nCurState == eState_Dash_Grab
		|| m_nCurState == eState_Dash_Fall || m_nCurState == eState_Roll || m_nCurState == eState_Roll_Stop || m_nCurState == eState_BackFlip_1 )
		m_vel = v0;

	if( m_nCurState == eState_Punch )
	{
		CVector2 tangentDir( -gravityDir.y, gravityDir.x );
		tangentDir = tangentDir * m_nDir;
		if( fixVel.Dot( tangentDir ) < v0.Dot( tangentDir ) - 10 )
			PunchHit( gravityDir, res );
	}

	if( m_pGrabbed )
	{
		m_pGrabDetect->ForceUpdateTransform();
		auto d = grabTarget - m_pGrabDetect->globalTransform.GetPosition();
		bool bOK = d.Length2() < m_grabDesc.fDropThreshold * m_grabDesc.fDropThreshold;
		if( bOK )
		{
			if( m_nCurState == eState_Dash_Grab )
			{
				m_nCurState = eState_Grip;
				m_nStateTick = 0;
				m_grabDesc.bAttached = true;
				SafeCastToInterface<IGrabbable>( m_pGrabbed.GetPtr() )->OnAttached( this );
			}
		}
		else
		{
			if( m_nCurState == eState_Grip )
			{
				GrabDetach();
				DashFall();
			}
		}
		if( m_nCurState == eState_Grip )
			GrabControl( gravityDir );
	}

	bool bTransformBlocked = false;
	if( fHitMove1 > 0 )
	{
		auto trans = m_pHit[1]->GetGlobalTransform();
		CVector2 ofs( 0, fHitMove1 );
		ofs = trans.MulVector2Dir( ofs );
		SRaycastResult result;
		result.fDist = fHitMove1;
		m_moveData.DoSweepTest( this, trans, ofs, &result, true, m_pHit[1] );
		float fMove = result.fDist;
		if( fMove < fHitMove1 )
		{
			CVector2 ofs0( 0, fMove - fHitMove1 - 0.01f );
			ofs0 = trans.MulVector2Dir( ofs0 );
			m_moveData.TryMove1( this, ELEM_COUNT( pTestEntities ), pTestEntities, ofs0, m_vel, 0, CanHitPlatform() ? &gravityDir : &gravity0, res );
			trans = m_pHit[1]->GetGlobalTransform();
			result.fDist = fHitMove1;
			m_moveData.DoSweepTest( this, trans, ofs, &result, true, m_pHit[1] );
		}
		m_pHit[1]->SetPosition( CVector2( 0, m_pHit[1]->y + result.fDist ) );
		if( result.fDist < fHitMove1 )
			bTransformBlocked = true;
	}
	if( fHitMove2 > 0 )
	{
		auto trans = m_pHit[2]->GetGlobalTransform();
		CVector2 ofs( fHitMove2 * m_nDir, 0 );
		ofs = trans.MulVector2Dir( ofs );
		SRaycastResult result;
		result.fDist = fHitMove2;
		m_moveData.DoSweepTest( this, trans, ofs, &result, true, m_pHit[2] );
		float fMove = result.fDist;
		if( fMove < fHitMove2 )
		{
			CVector2 ofs0( ( fMove - fHitMove2 - 0.01f ) * m_nDir, 0 );
			ofs0 = trans.MulVector2Dir( ofs0 );
			m_moveData.TryMove1( this, ELEM_COUNT( pTestEntities ), pTestEntities, ofs0, m_vel, 0, CanHitPlatform() ? &gravityDir : &gravity0, res );
			trans = m_pHit[2]->GetGlobalTransform();
			result.fDist = fHitMove2;
			m_moveData.DoSweepTest( this, trans, ofs, &result, true, m_pHit[2] );
		}
		m_pHit[2]->SetPosition( CVector2( m_pHit[2]->x + result.fDist * m_nDir, 0 ) );
		if( result.fDist < fHitMove2 )
			bTransformBlocked = true;
	}
	if( bTransformBlocked )
	{
		if( m_nCurState == eState_Slide || m_nCurState == eState_Slide_Air )
			SlideCancel();
		else if( m_nCurState == eState_BackFlip )
		{
			m_nCurState = eState_Stand_1;
			m_nStateTick = 0;
			m_pHit[1]->SetPosition( m_pHit[0]->GetPosition() );
		}
		else
			ForceRoll();
	}

	if( CanFindFloor() )
	{
		bool bPreLandedEntity = m_pLanded != NULL;
		FindFloor( gravityDir );
		if( m_bKnockback && m_pLanded )
			m_bKnockback = false;
	}
	else
		m_pLanded = NULL;

	if( bFalling )
	{
		m_fFallHeight = Max( 0.0f, m_fFallHeight + ( GetPosition() - pos0 ).Dot( gravityDir ) );
		bool bFall = m_pLanded != NULL;
		if( !bFall )
		{
			for( int i = 0; i < 3; i++ )
			{
				if( !res[i].pHitProxy )
					break;
				float f = res[i].normal.Dot( gravityDir );
				if( f < 0 )
				{
					bFall = true;
					break;
				}
			}
		}
		if( bFall )
		{
			int32 nDmg = Max( 0.0f, ( m_fFallHeight - m_fFallDmgBegin ) * m_fFallDmgPerHeight );
			m_nHp -= nDmg;
			m_fFallHeight = 0;
		}
	}
	else
		m_fFallHeight = 0;

	switch( m_nCurState )
	{
	case eState_Slide_Air:
		if( m_pLanded )
			Slide1();
		break;
	case eState_Glide:
		if( m_pLanded )
		{
			m_nCurState = eState_Stand_1;
			m_nStateTick = 0;
		}
		else
		{
			CVector2 tangentDir( -gravityDir.y, gravityDir.x );
			tangentDir = tangentDir * m_nDir;
			if( m_vel.Dot( tangentDir ) < m_glideVel.x - 10 )
			{
				if( CGame::Inst().IsKey( 'J' ) && m_nMoveState == m_nDir && m_vel.Dot( gravityDir ) >= m_glideVel.y - 1 )
					DashGrab();
				else
					DashFall();
			}
		}
		break;
	case eState_Dash:
		if( m_vel.Dot( gravityDir ) > 0 )
		{
			if( CGame::Inst().IsKey( 'W' ) )
				m_nBufferedInput = 1;
			if( m_nLevel >= ePlayerLevel_Glide && m_nBufferedInput == 1 )
				Glide();
			else
				DashFall();
			m_nBufferedInput = 0;
		}
		break;
	case eState_Dash_Grab:
	case eState_Dash_Fall:
		if( m_pLanded )
		{
			if( m_nCurState == eState_Dash_Grab )
				CGame::Inst().ForceKeyRelease( 'J' );
			BeginRoll();
		}
		break;
	case eState_Roll:
		if( m_nStateTick == 0 && m_vel.Dot( CVector2( -gravityDir.y, gravityDir.x ) ) * m_nDir <= 0 )
			RollStop();
		break;
	}

	if( m_nFireCDLeft )
		m_nFireCDLeft--;
	if( m_nBombCDLeft )
		m_nBombCDLeft--;
	if( CanShoot() )
	{
		if( m_nLevel >= ePlayerLevel_Shoot && CGame::Inst().IsKey( 'K' ) && !m_nFireCDLeft )
		{
			m_nFireCDLeft = m_nFireCD;
			curTransform = GetGlobalTransform();
			for( int i = 0; i < m_arrBulletOfs.Size(); i++ )
			{
				auto ofs = m_arrBulletOfs[i];
				ofs.x *= m_nDir;
				auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( curTransform.MulVector2Pos( ofs ) );
				CVector2 bulletDir( -gravityDir.y, gravityDir.x );
				pBullet->SetBulletVelocity( bulletDir * m_nDir * 2000 );
				pBullet->SetOwner( this );
				pBullet->SetParentEntity( pLevel );
			}
		}
		if( m_nLevel >= ePlayerLevel_Bomb && CGame::Inst().IsKey( 'L' ) && !m_nBombCDLeft )
		{
			m_nBombCDLeft = m_nBombCD;
			curTransform = GetGlobalTransform();
			auto ofs = m_bombOfs;
			ofs.x *= m_nDir;
			auto pBomb = SafeCast<CBullet>( m_pBomb->GetRoot()->CreateInstance() );
			pBomb->SetPosition( curTransform.MulVector2Pos( ofs ) );
			pBomb->SetBulletVelocity( m_vel );
			pBomb->SetOwner( this );
			pBomb->SetParentEntity( pLevel );
		}
	}
	UpdateHit();
	CMasterLevel::GetInst()->UpdateTest();
	CMasterLevel::GetInst()->UpdateAlert();
}

void CPlayer::PostUpdate()
{
	m_nHpRecoverCDLeft--;
	if( m_nHpRecoverCDLeft <= -m_nHpRecoverInterval )
	{
		if( m_nHp < m_nMaxHp )
		{
			m_nHp++;
			m_nHpRecoverCDLeft = 0;
		}
		else
			m_nHpRecoverCDLeft = m_nHpRecoverInterval;
	}
	if( m_nDamageLeft )
	{
		m_nHp--;
		m_nDamageLeft--;
	}
	if( m_nShieldRecoverCDLeft )
		m_nShieldRecoverCDLeft--;
	if( CanRecoverShield() )
	{
		if( !m_nShieldRecoverCDLeft )
			m_nShield = m_nMaxShield;
	}
	else
	{
		m_nShield = 0;
		m_nShieldRecoverCDLeft = Max( m_nShieldRecoverCDLeft, m_nShieldRecoverCD );
	}

	auto gravityDir = GetLevel()->GetGravityDir();
	UpdateAnim( gravityDir );
	UpdateAnimState( gravityDir );
	CleanUpOpenPlatforms();
	m_lastFramePos = GetPosition();
	if( m_nHp <= 0 )
		Kill();
}

void CPlayer::OnKickFirstHit( CEntity* pKick )
{
	m_fFallHeight = 0;
	if( m_nCurState == eState_Kick && m_pCurAttackEffect == pKick )
	{
		m_kickState.bHit = 1;
	}
}

bool CPlayer::Knockback( const CVector2& vec )
{
	if( m_nKnockBackTime )
		return false;
	m_bKnockback = true;
	m_nKnockBackTime = 30;
	m_vel = vec;
	return true;
}

bool CPlayer::CanHitPlatform()
{
	return m_nCurState >= eState_Slide && m_nCurState <= eState_BackFlip && m_nCurState != eState_Roll_Stand_Up;
}

void CPlayer::CleanUpOpenPlatforms()
{
	auto& setPlatforms = m_moveData.setOpenPlatforms;
	auto setPlatformsOld = setPlatforms;
	setPlatforms.clear();
	CEntity* pTestEntities[] = { m_pHit[0], m_pHit[1], m_pHit[2] };
	for( int i = 0; i < ELEM_COUNT( pTestEntities ); i++ )
	{
		pTestEntities[i]->ForceUpdateTransform();
		for( auto pManifold = pTestEntities[i]->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( setPlatformsOld.find( pEntity ) != setPlatformsOld.end() )
				setPlatforms.insert( pEntity );
		}
	}
}

bool CPlayer::CanShoot()
{
	return m_nCurState == eState_Stand_1 || m_nCurState == eState_Walk_1 || m_nCurState == eState_Glide_Fall
		|| m_nCurState == eState_Glide || m_nCurState == eState_Fly;
}

bool CPlayer::CanFindFloor()
{
	if( m_nKnockBackTime )
		return false;
	if( m_nCurState == eState_Grip || m_nCurState == eState_Dash || m_nCurState == eState_BackFlip_1 || m_nCurState == eState_Boost || m_nCurState == eState_Fly )
		return false;
	if( m_nCurState <= eState_Jump )
	{
		if( m_nJumpState < 0 )
			return false;
	}
	return true;
}

bool CPlayer::CanRecoverShield()
{
	return m_nLevel >= ePlayerLevel_Shield && m_nCurState == eState_Stand_1 || m_nCurState == eState_Stand_1_Ready || m_nCurState == eState_Walk_1;
}

float CPlayer::GetGravity()
{
	if( m_nCurState >= eState_Stand_1 && m_nCurState <= eState_Glide )
		return m_fGravity1;
	return m_fGravity;
}

bool CPlayer::IsFalling()
{
	if( m_nCurState == eState_Glide || m_nCurState == eState_Boost || m_nCurState == eState_Fly || m_nCurState == eState_Grip || m_nCurState == eState_Punch )
		return false;
	return true;
}

void CPlayer::FindFloor( const CVector2& gravityDir )
{
	CMatrix2D trans = GetGlobalTransform();
	CVector2 dir = gravityDir;
	CVector2 ofs = dir * m_fFindFloorDist;
	CEntity* pTestEntities[] = { m_pHit[0], m_pHit[1], m_pHit[2] };
	CMatrix2D mats[] = { m_pHit[0]->GetGlobalTransform(), m_pHit[1]->GetGlobalTransform(), m_pHit[2]->GetGlobalTransform() };
	SRaycastResult result;
	CVector2 gravity0( 0, 0 );
	auto pNewLandedEntity = SafeCast<CCharacter>( m_moveData.DoSweepTest1( this, 3, pTestEntities, mats, ofs, CanHitPlatform() ? &dir : &gravity0, &result ) );

	if( pNewLandedEntity && m_vel.Dot( result.normal ) < 1.0f && result.normal.Dot( dir ) < -0.5f )
	{
		m_pLanded = pNewLandedEntity;
		SetPosition( GetPosition() + dir * result.fDist );
		m_groundNorm = result.normal;
		m_vel = m_vel - m_groundNorm * m_vel.Dot( m_groundNorm );
		lastLandedEntityTransform = m_pLanded->GetGlobalTransform();
	}
	else
		m_pLanded = NULL;
}

void CPlayer::UpdateHit()
{
	auto pLevel = GetLevel();
	int32 nNewHitAreaType;
	if( m_nCurState <= eState_Slide_Air || m_nCurState == eState_BackFlip_2 || m_nCurState == eState_Roll_Stand_Up && m_nStateTick < m_nStandUpAnimSpeed )
		nNewHitAreaType = 1;
	else if( m_nCurState < eState_Fly || m_nCurState == eState_Roll_Recover && m_nStateTick < m_nRollRecoverAnimSpeed )
		nNewHitAreaType = 2 + ( m_nDir == 1 ? 0 : 1 );
	else if( m_nCurState == eState_BackFlip || m_nCurState == eState_BackFlip_1 )
		nNewHitAreaType = 0;
	else
		nNewHitAreaType = 4;
	if( m_nHitAreaType == nNewHitAreaType )
	{
		pLevel->GetHitTestMgr().Update( this );
		return;
	}

	if( m_nHitAreaType )
	{
		if( pLevel )
			pLevel->GetHitTestMgr().Remove( this );
		RemoveProxy( Get_HitProxy() );
	}
	m_nHitAreaType = nNewHitAreaType;
	if( m_nHitAreaType == 1 )
		AddRect( CRectangle( -16, -16, 32, 64 ) );
	else if( m_nHitAreaType == 2 )
		AddRect( CRectangle( -16, -16, 48, 32 ) );
	else if( m_nHitAreaType == 3 )
		AddRect( CRectangle( -16, -32, 48, 32 ) );
	else if( m_nHitAreaType == 4 )
		AddRect( CRectangle( -16, -16, 32, 32 ) );
	if( pLevel && m_nHitAreaType )
	{
		pLevel->GetHitTestMgr().Add( this );
		pLevel->GetHitTestMgr().Update( this );
	}
}

void CPlayer::UpdateAnim( const CVector2& gravityDir )
{
	auto tangent = CVector2( -gravityDir.y, gravityDir.x ) * m_nDir;
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	CRectangle rect( 0, 0, 0, 0 );
	CRectangle texRect( 0, 0, 0, 0 );
	switch( m_nCurState )
	{
	case eState_Stand:
	case eState_Walk:
	case eState_Jump:
	{
		if( m_nMoveState )
			m_nDir = m_nMoveState;
		if( !m_pLanded )
		{
			m_nCurState = eState_Jump;
			m_nStateTick = 0;
			int32 n;
			float f = -m_vel.Dot( gravityDir );
			for( n = 0; n < 3; n++ )
			{
				if( f > m_fJumpAnimVel[n] )
					break;
			}
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 3 + n * 2, 3, 2, 3 ) / 32;
		}
		else if( m_nMoveState || tangent.Dot( m_vel ) > 0 )
		{
			if( m_nCurState != eState_Walk )
			{
				m_nCurState = eState_Walk;
				m_nStateTick = 0;
			}
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 5 + ( m_nStateTick / m_nWalkAnimSpeed ) * 2, 0, 2, 3 ) / 32;
		}
		else
		{
			m_nCurState = eState_Stand;
			m_nStateTick = 0;
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 3, 0, 2, 3 ) / 32;
		}
		break;
	}
	case eState_Kick_Ready:
		if( m_nBufferedInput == 2 )
		{
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 29, 0, 2, 3 ) / 32;
		}
		else if( m_nBufferedInput == 1 )
		{
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 16, 0, 2, 3 ) / 32;
		}
		else
		{
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 16, 3, 2, 3 ) / 32;
		}
		break;
	case eState_Kick:
	{
		if( m_kickState.nAni == eAni_Kick_A )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[0] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 3, 3, 3 ) / 32 : CRectangle( 18, 3, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_B )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[0] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 0, 3, 3 ) / 32 : CRectangle( 18, 0, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_C )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[0] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 96, 128 );
			texRect = n ? CRectangle( 29, 7, 3, 3 ) / 32 : CRectangle( 29, 3, 3, 4 ) / 32;
		}

		else if( m_kickState.nAni == eAni_Kick_A1 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[1] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 3, 3, 3 ) / 32 : CRectangle( 18, 3, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_B1 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[1] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 0, 3, 3 ) / 32 : CRectangle( 18, 0, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_C1 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[1] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 96, 128 );
			texRect = n ? CRectangle( 29, 7, 3, 3 ) / 32 : CRectangle( 29, 3, 3, 4 ) / 32;
		}

		else if( m_kickState.nAni == eAni_Kick_A2 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[2] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 3, 3, 3 ) / 32 : CRectangle( 18, 3, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_B2 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[2] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 0, 3, 3 ) / 32 : CRectangle( 18, 0, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_C2 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[2] ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 96, 128 );
			texRect = n ? CRectangle( 29, 7, 3, 3 ) / 32 : CRectangle( 29, 3, 3, 4 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_Rev )
		{
			if( m_kickState.nTick < m_kickRevAnimFrame.x )
			{
				rect = CRectangle( -32, -26, 96, 128 );
				texRect = CRectangle( 29, 10, 3, 4 ) / 32;
			}
			else if( m_kickState.nTick < m_kickRevAnimFrame.y )
			{
				rect = CRectangle( -32, 6, 96, 96 );
				texRect = CRectangle( 29, 14, 3, 3 ) / 32;
			}
			else if( m_kickState.nTick < m_kickRevAnimFrame.z )
			{
				rect = CRectangle( -32, 6, 96, 64 );
				texRect = CRectangle( 29, 17, 3, 2 ) / 32;
			}
			else
			{
				rect = CRectangle( -32, -26, 96, 96 );
				texRect = CRectangle( 29, 19, 3, 3 ) / 32;
			}
		}
		else if( m_kickState.nAni == eAni_Kick_Stomp )
		{
			rect = CRectangle( -32, -26, 96, 128 );
			texRect = CRectangle( 29, 22 + ( m_kickState.nTick >= m_kickStompAnimFrame.x ? 3 : 0 ), 3, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_Spin )
		{
			int32 nFrame = m_kickState.nTick / m_nKickSpinAnimSpeed;
			int32 n = nFrame / ( 2 * m_nKickSpinAnimRep );
			if( n == 0 )
			{
				rect = CRectangle( -32, -26, 96, 128 );
				texRect = CRectangle( 29, 3, 3, 4 ) / 32;
			}
			else if( n == 1 )
			{
				rect = CRectangle( -32, -26, 128, 96 );
				texRect = CRectangle( 18, 3, 4, 3 ) / 32;
			}
			else
			{
				rect = CRectangle( -32, -26, 128, 96 );
				texRect = CRectangle( 18, 0, 4, 3 ) / 32;
			}
			if( !( nFrame & 1 ) )
			{
				rect.x = -rect.GetRight();
				texRect.x = 2 - texRect.GetRight();
			}
		}
		else
		{
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 11, 3, 2, 3 ) / 32;
		}

		if( m_kickState.nFlag1 )
		{
			auto nFrame = m_kickState.nTick1 / m_nKickSpinDashAnimSpeed;
			if( !!( nFrame & 1 ) )
			{
				rect.x = -rect.GetRight();
				texRect.x = 2 - texRect.GetRight();
			}
		}
		break;
	}
	case eState_Slide:
		rect = CRectangle( -28, -26, 96, 96 );
		texRect = CRectangle( 3 + ( m_nStateTick / m_nSlideAnimSpeed ) * 3, 6, 3, 3 ) / 32;
		break;
	case eState_Slide_Air:
		rect = CRectangle( -28, -26, 96, 96 );
		texRect = CRectangle( 9, 6, 3, 3 ) / 32;
		break;
	case eState_Slide_1:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 3 + Min( 1, m_nStateTick / m_nSlideDashTime ) * 3, 9, 3, 2 ) / 32;
		break;
	case eState_Stand_1:
	case eState_Walk_1:
	{
		if( m_nMoveState || tangent.Dot( m_vel ) > 0 )
		{
			if( m_nCurState != eState_Walk_1 )
			{
				m_nCurState = eState_Walk_1;
				m_nStateTick = 0;
			}
			rect = CRectangle( -28, -26, 96, 64 );
			texRect = CRectangle( 0, 2 + ( m_nStateTick / m_nWalk1AnimSpeed ) * 2, 3, 2 ) / 32;
		}
		else
		{
			m_nCurState = eState_Stand_1;
			m_nStateTick = 0;
			rect = CRectangle( -28, -26, 96, 64 );
			texRect = CRectangle( 0, 0, 3, 2 ) / 32;
		}
		break;
	}
	case eState_Glide_Fall:
		rect = CRectangle( -32, -26, 96, 64 );
		texRect = CRectangle( 0, 18, 3, 2 ) / 32;
		break;
	case eState_Glide:
		rect = CRectangle( -32, -26, 96, 64 );
		texRect = CRectangle( 0, 16, 3, 2 ) / 32;
		break;
	case eState_Boost:
		rect = CRectangle( -32, -26, 96, 64 );
		texRect = CRectangle( 5 + ( m_boostState.nTick >= m_boostState.nTick1 ? 3 : 0 ), 16, 3, 2 ) / 32;
		break;
	case eState_Fly:
		rect = CRectangle( -32, -26, 96, 64 );
		texRect = CRectangle( 11, 16, 3, 2 ) / 32;
		break;
	case eState_Stand_1_Ready:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 0, 0, 3, 2 ) / 32;
		break;
	case eState_Dash:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 0, 10, 3, 2 ) / 32;
		break;
	case eState_Dash_Grab:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 0, 12, 3, 2 ) / 32;
		break;
	case eState_Grip:
		rect = CRectangle( -32, -32, 64, 64 );
		texRect = CRectangle( 3, 14, 2, 2 ) / 32;
		break;
	case eState_Punch:
		rect = CRectangle( -32, -32, 64, 64 );
		texRect = CRectangle( 3, 16, 2, 2 ) / 32;
		break;
	case eState_Dash_Fall:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 0, 14, 3, 2 ) / 32;
		break;
	case eState_Roll:
		rect = CRectangle( -32, -32, 64, 64 );
		texRect = CRectangle( 5, 14, 2, 2 ) / 32;
		break;
	case eState_Roll_Stop:
		rect = CRectangle( -32, -32, 64, 64 );
		texRect = CRectangle( 9, 9, 2, 2 ) / 32;
		break;
	case eState_Roll_Stand_Up:
		if( m_nMoveState )
			m_nDir = m_nMoveState;
		rect = CRectangle( -32, -26, 64, 96 );
		texRect = CRectangle( 3 + ( m_nStateTick / m_nStandUpAnimSpeed ) * 2, 11, 2, 3 ) / 32;
		break;
	case eState_Roll_Recover:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 7 + ( m_nStateTick / m_nRollRecoverAnimSpeed ) * 3, 14, 3, 2 ) / 32;
		break;
	case eState_Roll_Dash:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 7 + ( m_nStateTick * 2 / m_nStand1ReadyTime ) * 3, 14, 3, 2 ) / 32;
		break;
	case eState_BackFlip:
		rect = CRectangle( -28, -26, 96, 64 );
		texRect = CRectangle( 13, 14, 3, 2 ) / 32;
		break;
	case eState_BackFlip_1:
		rect = CRectangle( -28, -26, 96, 96 );
		texRect = CRectangle( 13, 0 + ( m_nStateTick / m_nBackFlip1AnimSpeed ) * 3, 3, 3 ) / 32;
		break;
	case eState_BackFlip_2:
		rect = CRectangle( -32, -26, 96, 96 );
		texRect = CRectangle( 7 + Min( 2, m_nStateTick / m_nBackFlip2AnimSpeed ) * 3, 11, 3, 3 ) / 32;
		break;
	case eState_Force_Roll:
	{
		float d1 = ( m_pHit[1]->y - m_pHit[0]->y ) / m_fHitOfs1;
		float d2 = ( m_pHit[2]->y - m_pHit[0]->y ) / m_fHitOfs2;
		if( d1 > d2 )
		{
			rect = CRectangle( -32, -26, 64, 96 );
			texRect = CRectangle( 3 + ( d1 > 0.5f ? 2 : 0 ) * 2, 11, 2, 3 ) / 32;
		}
		else
		{
			rect = CRectangle( -28, -26, 96, 64 );
			texRect = CRectangle( 7 + ( d2 > 0.5f ? 2 : 0 ) * 3, 14, 3, 2 ) / 32;
		}
		break;
	}
	}
	if( m_nDir == -1 )
	{
		rect.x = -rect.GetRight();
		texRect.x = 2 - texRect.GetRight();
	}
	p->SetRect( rect );
	p->SetTexRect( texRect );

	if( m_nCurState == eState_Roll )
		p->SetRotation( p->r - m_fRollRotateSpeed * GetLevel()->GetElapsedTimePerTick() * m_nDir );
	else if( m_nCurState == eState_Roll_Stop )
	{
		auto nStateTick = abs( m_nStateTick );
		if( nStateTick == m_nRollStopFrames )
		{
			float d = p->r * m_nDir;
			d = d / ( PI * 2 );
			d -= floor( d );
			p->r = ( d + 1 ) * ( PI * 2 ) * m_nDir;
		}
		p->SetRotation( p->r * ( nStateTick - 1 ) * ( nStateTick - 1 ) * 1.0f / ( nStateTick * nStateTick ) );
	}
	else
		p->SetRotation( 0 );
	UpdateEffect();
}

void CPlayer::UpdateEffect()
{
	auto pImgShieldBar = static_cast<CImage2D*>( m_pShieldEffect[0].GetPtr() );
	auto pImgShieldBack = static_cast<CImage2D*>( m_pShieldEffect[1].GetPtr() );
	auto pImgShieldCDBar = static_cast<CImage2D*>( m_pShieldEffect[2].GetPtr() );
	auto pImgShieldCDBack = static_cast<CImage2D*>( m_pShieldEffect[3].GetPtr() );
	pImgShieldBar->bVisible = false;
	pImgShieldBack->bVisible = false;
	pImgShieldCDBar->bVisible = false;
	pImgShieldCDBack->bVisible = false;
	if( CanRecoverShield() )
	{
		float w = 96;
		float fBorder = 2;

		if( !m_nShield )
		{
			pImgShieldBack->bVisible = true;
			pImgShieldBack->SetRect( CRectangle( -48, -26, 96, 32 ) );
			pImgShieldBack->SetTexRect( CRectangle( 0.75f, 0, 3 / 32.0f, 1 / 32.0f ) );
			pImgShieldBack->GetParam()[0] = CVector4( 1, 0, 0, 1 );
		}
		else if( m_nShield == m_nMaxShield )
		{
			pImgShieldBar->bVisible = true;
			pImgShieldBar->SetRect( CRectangle( -48, -26, 96, 32 ) );
			pImgShieldBar->SetTexRect( CRectangle( 0.75f, 0, 3 / 32.0f, 1 / 32.0f ) );
			pImgShieldBar->GetParam()[0] = CVector4( 0.5, 0.9, 1, 1 );
		}
		else
		{
			float l = fBorder + m_nShield * ( w - fBorder * 2 ) / m_nMaxShield;
			l = floor( l * 0.5f + 0.5f ) * 2;
			pImgShieldBack->bVisible = true;
			pImgShieldBack->SetRect( CRectangle( -48 + l, -26, 96 - l, 32 ) );
			pImgShieldBack->SetTexRect( CRectangle( 0.75f + l * 3 / ( w * 32.0f ), 0, ( 96 - l ) * 3 / ( w * 32.0f ), 1 / 32.0f ) );
			pImgShieldBack->GetParam()[0] = CVector4( 0.3, 0.25, 0.2, 0.25 );
			pImgShieldBar->bVisible = true;
			pImgShieldBar->SetRect( CRectangle( -48, -26, l, 32 ) );
			pImgShieldBar->SetTexRect( CRectangle( 0.75f, 0, l * 3 / ( w * 32.0f ), 1 / 32.0f ) );
			pImgShieldBar->GetParam()[0] = CVector4( 0.2, 0.7, 0.7, 1 );
		}


		if( m_nShieldRecoverCDLeft == 0 )
		{
			pImgShieldCDBack->bVisible = true;
			pImgShieldCDBack->SetRect( CRectangle( -48, -26, 96, 32 ) );
			pImgShieldCDBack->SetTexRect( CRectangle( 0.75f, 1 / 32.0f, 3 / 32.0f, 1 / 32.0f ) );
			pImgShieldCDBack->GetParam()[0] = CVector4( 0.5, 0.9, 1, 1 );
		}
		else if( m_nShieldRecoverCDLeft >= m_nShieldRecoverCD )
		{
			pImgShieldCDBar->bVisible = true;
			pImgShieldCDBar->SetRect( CRectangle( -48, -26, 96, 32 ) );
			pImgShieldCDBar->SetTexRect( CRectangle( 0.75f, 1 / 32.0f, 3 / 32.0f, 1 / 32.0f ) );
			pImgShieldCDBar->GetParam()[0] = CVector4( 1, 0, 0, 1 );
		}
		else
		{
			float l = fBorder + Min( m_nShieldRecoverCD, m_nShieldRecoverCDLeft ) * ( w - fBorder * 2 ) / m_nShieldRecoverCD;
			l = floor( l * 0.5f + 0.5f ) * 2;
			pImgShieldCDBack->bVisible = true;
			pImgShieldCDBack->SetRect( CRectangle( -48 + l, -26, 96 - l, 32 ) );
			pImgShieldCDBack->SetTexRect( CRectangle( 0.75f + l * 3 / ( w * 32.0f ), 1 / 32.0f, ( 96 - l ) * 3 / ( w * 32.0f ), 1 / 32.0f ) );
			pImgShieldCDBack->GetParam()[0] = CVector4( 0.2, 0.7, 0.7, 1 );
			pImgShieldCDBar->bVisible = true;
			pImgShieldCDBar->SetRect( CRectangle( -48, -26, l, 32 ) );
			pImgShieldCDBar->SetTexRect( CRectangle( 0.75f, 1 / 32.0f, l * 3 / ( w * 32.0f ), 1 / 32.0f ) );
			pImgShieldCDBar->GetParam()[0] = CVector4( 0.3, 0.25, 0.2, 0.25 );
		}
		for( int i = 0; i < 4; i++ )
			m_pShieldEffect[i]->SetPosition( CVector2( 16 * m_nDir, m_pShieldEffect[i]->y ) );
	}
}

void CPlayer::UpdateAnimState( const CVector2& gravityDir )
{
	auto tangent = CVector2( -gravityDir.y, gravityDir.x ) * m_nDir;
	switch( m_nCurState )
	{
	case eState_Walk:
		m_nStateTick = ( m_nStateTick + 1 ) % ( m_nWalkAnimSpeed * 4 );
		break;
	case eState_Kick_Ready:
		m_nStateTick++;
		if( m_nStateTick >= m_nKickReadyTime )
		{
			auto lo = m_nBufferedInput & 15;
			if( lo >= 4 )
				Kick( eAni_Kick_Spin );
			else
				Kick( lo == 3 ? 0 : lo );
			m_kickState.nType0 = ( m_nBufferedInput & 48 ) >> 4;
			m_nBufferedInput = 0;
		}
		break;
	case eState_Kick:
	{
		m_kickState.nTick++;
		if( m_kickState.nTick1 > 0 )
		{
			m_kickState.nTick1--;
			if( !m_kickState.nTick1 )
			{
				if( m_kickState.nFlag1 )
				{
					if( m_pCurAttackEffect )
					{
						SafeCast<CCharacter>( m_pCurAttackEffect.GetPtr() )->Kill();
						m_pCurAttackEffect = NULL;
					}
				}
				if( m_kickState.nAni < eAni_Kick_A2 )
					m_kickState.nTick1 = -1;
			}
		}
		if( m_kickState.nAni < eAni_Kick_Base_End )
		{
			if( m_kickState.nTick >= m_nKickAnimFrame2[m_kickState.nAni / 3] )
			{
				bool b = false;
				if( m_kickState.bHit && m_kickState.nAni < eAni_Kick_Base_End )
				{
					if( CGame::Inst().IsKeyDown( 'K' ) )
						m_nBufferedInput = m_nBufferedInput | 16;
					if( CGame::Inst().IsKeyDown( 'L' ) )
						m_nBufferedInput = m_nBufferedInput | 32;
					if( m_nDir == 1 && CGame::Inst().IsKey( 'A' ) || m_nDir == -1 && CGame::Inst().IsKey( 'D' ) )
						m_nBufferedInput = m_nBufferedInput | 4;
					if( m_nDir == 1 && CGame::Inst().IsKey( 'D' ) || m_nDir == -1 && CGame::Inst().IsKey( 'A' ) )
						m_nBufferedInput = m_nBufferedInput | 1;
					if( CGame::Inst().IsKey( 'W' ) )
						m_nBufferedInput = m_nBufferedInput | 2;
					if( !!( m_nBufferedInput & 48 ) )
					{
						if( m_nLevel >= ePlayerLevel_Kick_Rev && !!( m_nBufferedInput & 4 ) )
						{
							uint8 nFlag = m_kickState.nAni / 3;
							Kick( eAni_Kick_Rev );
							m_kickState.nFlag = nFlag;
							b = true;
						}
						else if( m_nLevel >= ePlayerLevel_Kick_Stomp && !!( m_nBufferedInput & 8 ) )
						{
							uint8 nFlag = m_kickState.nAni / 3;
							Kick( eAni_Kick_Stomp );
							m_kickState.nFlag = nFlag;
							b = true;
						}
						else if( m_nLevel >= ePlayerLevel_Kick_Combo && m_kickState.nAni < eAni_Kick_A2 )
						{
							auto nBaseAni = 3 * ( m_kickState.nAni / 3 + 1 );
							auto nInput1 = m_nBufferedInput & 3;
							Kick( ( nInput1 == 3 ? 0 : nInput1 ) + nBaseAni );
							b = true;
						}
						if( b )
							m_kickState.nType0 = ( m_nBufferedInput & 48 ) >> 4;
					}
				}
				if( !b )
					Kick( eAni_Kick_Recover );
				m_nBufferedInput = 0;
			}
		}
		else if( m_kickState.nAni == eAni_Kick_Rev )
		{
			if( m_kickState.nTick >= m_kickRevAnimFrame.w )
				Kick( eAni_Kick_Recover );
		}
		else if( m_kickState.nAni == eAni_Kick_Stomp )
		{
			if( m_kickState.nTick >= m_kickStompAnimFrame.y )
				Kick( eAni_Kick_Recover );
		}
		else if( m_kickState.nAni == eAni_Kick_Spin )
		{
			if( m_kickState.nTick >= m_nKickSpinAnimRep * m_nKickSpinAnimSpeed * 6 )
				Kick( eAni_Kick_Recover );
		}
		else
		{
			if( m_kickState.nTick >= m_nKickRecoverTime )
			{
				m_kickState.nTick = m_nKickRecoverTime;
				if( m_pLanded )
				{
					m_nCurState = eState_Stand;
					m_nStateTick = 0;
					m_nJumpState = 0;
				}
			}
		}
		break;
	}
	case eState_Slide:
		m_nStateTick++;
		if( m_nStateTick == m_nSlideAnimSpeed * 2 )
			Slide1();
		break;
	case eState_Slide_1:
		m_nStateTick++;
		if( m_nStateTick == m_nSlideDashTime )
			HandBufferInputStand1();
		else if( m_nStateTick == m_nSlideMaxFrame )
		{
			m_nStateTick = 0;
			m_nCurState = eState_Stand_1;
		}
		break;
	case eState_Walk_1:
	{
		auto moveDir = m_nMoveState * m_nDir;
		if( !moveDir )
		{
			float f = tangent.Dot( m_vel );
			if( f > 0 )
				moveDir = 1;
			else if( f < 0 )
				moveDir = -1;
		}
		if( moveDir > 0 )
			m_nStateTick = ( m_nStateTick + 1 ) % ( m_nWalk1AnimSpeed * 4 );
		else if( moveDir < 0 )
			m_nStateTick = ( m_nStateTick + m_nWalk1AnimSpeed * 4 - 1 ) % ( m_nWalk1AnimSpeed * 4 );
		break;
	}
	case eState_Stand_1_Ready:
		m_nStateTick++;
		if( m_nStateTick == m_nStand1ReadyTime )
			Dash();
		break;
	case eState_Glide_Fall:
		m_nStateTick++;
		if( m_nStateTick == m_nGlideFallTime )
		{
			m_nStateTick = 0;
			m_nCurState = eState_Stand_1;
		}
		break;
	case eState_Boost:
		m_boostState.nTick++;
		break;
	case eState_Fly:
		if( m_fFuel <= 0 )
			Glide();
		break;
	case eState_Dash:
		/*if( m_nStateTick >= 0 )
		{
			m_nStateTick++;
			if( m_nStateTick == m_nDashGrabBeginFrame )
				DashGrab();
		}
		else
		{
			m_nStateTick--;
			if( m_nStateTick == -m_nDashGrabBeginFrame )
			{
				CVector2 tangentDir( -gravityDir.y, gravityDir.x );
				m_vel = tangentDir * m_vel.Dot( tangentDir ) + gravityDir * Max( 0.0f, -m_vel.Dot( gravityDir ) );
				DashFall();
			}
		}*/
		m_nStateTick++;
		if( m_nStateTick == m_nDashGrabBeginFrame )
		{
			CVector2 tangentDir( -gravityDir.y, gravityDir.x );
			if( CGame::Inst().IsKey( 'W' ) )
				m_nBufferedInput = 1;
			if( m_nLevel >= ePlayerLevel_Glide && m_nBufferedInput == 1 )
				Glide();
			else
			{
				m_vel = tangentDir * m_vel.Dot( tangentDir ) + gravityDir * Max( 0.0f, -m_vel.Dot( gravityDir ) );
				if( CGame::Inst().IsKey( 'J' ) )
					DashGrab();
				else
					DashFall();
			}
			m_nBufferedInput = 0;
		}
		break;
	case eState_Dash_Grab:
		if( m_pGrabbed )
		{
			m_nStateTick++;
			if( m_nStateTick == m_nGrabMaxTime )
			{
				m_pGrabbed = NULL;
				DashFall();
			}
			break;
		}
		//continue
	case eState_Dash_Fall:
		m_nStateTick++;
		if( m_nStateTick == m_nDashFallEndFrame )
		{
			if( m_nCurState == eState_Dash_Grab )
				CGame::Inst().ForceKeyRelease( 'J' );
			BeginRoll();
		}
		break;
	case eState_Roll:
		if( m_nStateTick > 0 )
			m_nStateTick--;
		break;
	case eState_Roll_Stand_Up:
		m_nStateTick++;
		if( m_nStateTick == m_nStandUpAnimSpeed * 2 )
		{
			m_nCurState = eState_Stand;
			m_nStateTick = 0;
			m_nJumpState = 0;
		}
		break;
	case eState_Roll_Stop:
	{
		auto nStateTick = abs( m_nStateTick );
		auto nStateDir = m_nStateTick / nStateTick;
		nStateDir = m_nMoveState ? m_nMoveState : nStateDir;
		nStateTick--;
		if( !nStateTick )
			RollRecover( nStateDir );
		else
			m_nStateTick = nStateDir * nStateTick;
		break;
	}
	case eState_Roll_Recover:
		m_nStateTick++;
		if( m_nStateTick == m_nRollRecoverAnimSpeed * 2 )
		{
			if( !HandBufferInputStand1() )
			{
				m_nStateTick = 0;
				m_nCurState = eState_Stand_1;
			}
		}
		break;
	case eState_Roll_Dash:
		m_nStateTick++;
		if( m_nStateTick == m_nStand1ReadyTime )
			Dash();
		break;
	case eState_BackFlip:
		m_nStateTick++;
		if( m_nStateTick == m_nBackFlipAnimSpeed )
			BackFlip1();
		break;
	case eState_BackFlip_1:
		m_nStateTick++;
		if( m_nStateTick == m_nBackFlip1AnimSpeed * 2 )
			BackFlip2();
		break;
	case eState_BackFlip_2:
		m_nStateTick++;
		if( m_nStateTick == m_nBackFlip2Time )
		{
			m_nCurState = eState_Stand;
			m_nStateTick = 0;
			m_nJumpState = 0;
		}
		break;
	default:
		break;
	}
}

void CPlayer::JumpCancel( const CVector2& gravityDir )
{
	if( m_nJumpState < 0 )
	{
		m_vel = m_vel - gravityDir * ( m_fJumpSpeed * m_nJumpState / m_nJumpHoldTime );
		m_nJumpState = 0;
	}
}

void CPlayer::Kick( int8 nAni )
{
	int8 nLastAni = m_nCurState == eState_Kick ? m_kickState.nAni : -1;
	int32 nKickType0;
	if( nLastAni >= eAni_Kick_Finish_Begin && nLastAni < eAni_Kick_Finish_End )
		nKickType0 = 1 + m_kickState.nFlag;
	else if( nLastAni >= 0 && nLastAni < eAni_Kick_Base_End )
		nKickType0 = nLastAni / 3;
	else
		nKickType0 = 0;
	auto pLastAttackEffect = m_pCurAttackEffect;

	if( nLastAni == eAni_Kick_Spin )
	{
		if( m_pCurAttackEffect )
		{
			SafeCast<CCharacter>( m_pCurAttackEffect.GetPtr() )->Kill();
			m_pCurAttackEffect = NULL;
		}
	}

	uint8 nFlag = 0;
	bool bHit = m_kickState.bHit;
	if( nAni == eAni_Kick_Recover )
	{
		if( nLastAni >= eAni_Kick_Finish_Begin )
			nAni = eAni_Kick_Recover_1;
		else
			nFlag = nLastAni / 3;
	}
	if( nAni == eAni_Kick_Spin )
	{
		nFlag = nKickType0;
		auto pKick = SafeCast<CCharacter>( m_pKickSpin[nKickType0]->GetRoot()->CreateInstance() );
		pKick->SetParentEntity( this );
		pKick->SetOwner( this );
		if( m_kickState.nTick1 )
			m_vel = CVector2( 0, 0 );
		KickMorph( pKick );
	}

	m_nCurState = eState_Kick;
	m_nStateTick = nAni << 16;
	if( pLastAttackEffect == m_pCurAttackEffect.GetPtr() )
		m_pCurAttackEffect = NULL;
	m_kickState.nFlag = nFlag;
	if( nAni == eAni_Kick_Stomp )
	{
		if( !m_pLanded )
			m_kickState.nTick = m_kickStompAnimFrame.x;
	}
	if( nAni == eAni_Kick_Recover )
		m_kickState.bHit = bHit;
}

void CPlayer::KickBreak()
{
	Kick( eAni_Kick_Recover_1 );
	m_nCurState = eState_Stand;
	m_nStateTick = 0;
	m_nJumpState = 0;
}

void CPlayer::KickMorph( CEntity* pEntity )
{
	if( m_pCurAttackEffect )
	{
		auto pKick = SafeCast<CKick>( m_pCurAttackEffect.GetPtr() );
		if( pKick )
			pKick->Morph( pEntity );
	}
	m_pCurAttackEffect = pEntity;
}

void CPlayer::BeginRoll()
{
	m_nCurState = eState_Roll;
	m_nStateTick = 0;
	m_pHit[1]->SetPosition( m_pHit[0]->GetPosition() );
	m_pHit[2]->SetPosition( m_pHit[0]->GetPosition() );
}

void CPlayer::Slide1()
{
	m_nCurState = eState_Slide_1;
	m_nStateTick = 0;
	m_pHit[1]->SetPosition( m_pHit[0]->GetPosition() );
}

void CPlayer::SlideCancel()
{
	m_nCurState = eState_Stand;
	m_nStateTick = 0;
	m_nJumpState = 0;
	m_pHit[2]->SetPosition( m_pHit[0]->GetPosition() );
}

void CPlayer::Glide()
{
	auto gravityDir = GetLevel()->GetGravityDir();
	CVector2 tangentDir( -gravityDir.y, gravityDir.x );
	tangentDir = tangentDir * m_nDir;
	m_nCurState = eState_Glide;
	m_nStateTick = 0;
	m_vel = gravityDir * m_vel.Dot( gravityDir ) + tangentDir * m_glideVel.x;
}

void CPlayer::Fly()
{
	m_nCurState = eState_Fly;
	m_nStateTick = 0;
}

void CPlayer::Dash()
{
	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();
	CVector2 tangentDir( -gravityDir.y, gravityDir.x );
	m_vel = tangentDir * m_fDashSpeed * m_nDir + gravityDir * -m_fDashSpeedY;
	m_nCurState = eState_Dash;
	m_nStateTick = 0;
}

void CPlayer::DashGrab()
{
	if( m_nLevel < ePlayerLevel_Grab )
	{
		DashFall();
		return;
	}
	m_nCurState = eState_Dash_Grab;
	m_nStateTick = 0;
	m_pHit[2]->SetPosition( m_pHit[0]->GetPosition() );
	m_pGrabDetect->x = abs( m_pGrabDetect->x ) * m_nDir;
	m_pGrabDetect->SetTransformDirty();
}

void CPlayer::CheckGrab()
{
	if( m_nCurState != eState_Dash_Grab && m_nCurState != eState_Grip )
		return;
	if( m_nKnockBackTime )
	{
		if( m_pGrabbed && m_grabDesc.bAttached )
			SafeCastToInterface<IGrabbable>( m_pGrabbed.GetPtr() )->OnDetached( this );
		m_pGrabbed = NULL;
		DashFall();
		return;
	}
	if( m_pGrabbed )
	{
		auto p = SafeCastToInterface<IGrabbable>( m_pGrabbed.GetPtr() );
		if( CMyLevel::GetEntityLevel( m_pGrabbed ) != GetLevel() || !p->CheckGrab( this, m_grabDesc ) )
		{
			GrabDetach();
			DashFall();
			return;
		}
	}
	else
	{
		m_grabDesc.bAttached = false;
		static vector<CReference<CEntity> > vecResult;
		GetLevel()->MultiHitTest( m_pGrabDetect->Get_HitProxy(), m_pGrabDetect->globalTransform, vecResult );

		for( CEntity* pEntity : vecResult )
		{
			auto p = SafeCastToInterface<IGrabbable>( pEntity );
			if( p && p->CheckGrab( this, m_grabDesc ) )
			{
				m_pGrabbed = pEntity;
				m_nStateTick = 0;
				break;
			}
		}
		vecResult.resize( 0 );
	}
}

void CPlayer::GrabControl( const CVector2& gravityDir )
{
	if( m_grabDesc.nDetachType >= SGrabDesc::eDetachType_Release )
	{
		if( !CGame::Inst().IsKey( 'J' ) )
		{
			if( m_grabDesc.nDetachType == SGrabDesc::eDetachType_Walljump )
			{
				if( m_nMoveState == m_nDir )
				{
					GrabDetach();
					Punch();
					return;
				}
				if( m_nMoveState == -m_nDir )
				{
					GrabDetach();
					RollDash( -m_nDir );
					return;
				}
				else if( CGame::Inst().IsKey( 'W' ) )
				{
					GrabDetach();
					m_vel = gravityDir * -m_fJumpSpeed;
					RollStandUp();
					return;
				}
			}
			GrabDetach();
			DashFall();
		}
	}
}

void CPlayer::GrabDetach()
{
	if( m_grabDesc.bAttached )
		SafeCastToInterface<IGrabbable>( m_pGrabbed.GetPtr() )->OnDetached( this );
	m_vel = ( GetPosition() - m_lastFramePos ) / GetStage()->GetElapsedTimePerTick();
	m_pGrabbed = NULL;
}

void CPlayer::Punch()
{
	m_nCurState = eState_Punch;
	m_pHit[2]->SetPosition( m_pHit[0]->GetPosition() );
}

void CPlayer::PunchHit( const CVector2& gravityDir, SRaycastResult hit[3] )
{
	m_nCurState = eState_Punch;
	m_nDir = -m_nDir;
	CVector2 tangentDir( -gravityDir.y, gravityDir.x );
	tangentDir = tangentDir * m_nDir;
	m_vel = tangentDir * m_fPunchBounceSpeed;
	BeginRoll();
}

void CPlayer::DashFall()
{
	if( m_nCurState != eState_Dash_Grab )
		m_nStateTick = 0;
	m_nCurState = eState_Dash_Fall;
	m_pHit[2]->SetPosition( m_pHit[0]->GetPosition() );
}

void CPlayer::RollStandUp()
{
	m_nCurState = eState_Roll_Stand_Up;
	m_nStateTick = 0;
}

void CPlayer::RollStop()
{
	m_nCurState = eState_Roll_Stop;
	m_nStateTick = ( m_nMoveState ? m_nMoveState : m_nDir ) * m_nRollStopFrames;
}

void CPlayer::RollRecover( int8 nDir )
{
	m_nDir = nDir;
	m_nCurState = eState_Roll_Recover;
	m_nStateTick = 0;
}

void CPlayer::RollDash( int8 nDir )
{
	m_nDir = nDir;
	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();
	CVector2 tangentDir( -gravityDir.y, gravityDir.x );
	m_vel = tangentDir * m_fDashSpeed * m_nDir + gravityDir * ( m_nDashGrabBeginFrame * pLevel->GetElapsedTimePerTick() * m_fGravity - m_fDashSpeedY );
	m_nCurState = eState_Roll_Dash;
	m_nStateTick = 0;
}

void CPlayer::BackFlip()
{
	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();
	CVector2 tangentDir( -gravityDir.y, gravityDir.x );
	if( m_pLanded )
		tangentDir = CVector2( m_groundNorm.y, -m_groundNorm.x );
	m_nCurState = eState_BackFlip;
	m_nStateTick = 0;
}

void CPlayer::BackFlip1()
{
	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();
	CVector2 tangentDir( -gravityDir.y, gravityDir.x );
	m_vel = tangentDir * -m_fBackFlipSpeed * m_nDir - gravityDir * m_fDashSpeedY;
	m_nCurState = eState_BackFlip_1;
	m_nStateTick = 0;
}

void CPlayer::BackFlip2()
{
	m_nCurState = eState_BackFlip_2;
	m_nStateTick = 0;
	m_pHit[2]->SetPosition( m_pHit[0]->GetPosition() );
}

void CPlayer::ForceRoll( bool bUpdate )
{
	m_nCurState = eState_Roll;
	m_nStateTick = -1;
	for( int k = 0; k < 2; k++ )
	{
		auto trans = m_pHit[0]->GetGlobalTransform();
		float fMaxLen = k == 0 ? m_pHit[1]->y - m_pHit[0]->y : m_pHit[2]->x * m_nDir;
		if( bUpdate )
			fMaxLen = Max( 0.0f, fMaxLen - m_fForceRollSpeed * GetLevel()->GetElapsedTimePerTick() );
		if( fMaxLen <= 0 )
		{
			fMaxLen = 0;
			continue;
		}

		m_nCurState = eState_Force_Roll;
		m_nStateTick = 0;
		CVector2 ofs = k == 0 ? CVector2( 0, fMaxLen ) : CVector2( fMaxLen * m_nDir, 0 );
		SRaycastResult result;
		result.fDist = fMaxLen;
		m_moveData.DoSweepTest( this, trans, ofs, &result, true, k == 0 ? m_pHit[1] : m_pHit[2] );
		if( k == 0 )
			m_pHit[1]->SetPosition( CVector2( 0, m_pHit[0]->y + result.fDist ) );
		else
			m_pHit[2]->SetPosition( CVector2( result.fDist * m_nDir, 0 ) );
	}
}

bool CPlayer::HandBufferInputStand1()
{
	bool b = false;
	if( !!( m_nBufferedInput & 1 ) )
	{
		b = true;
		if( !!( m_nBufferedInput & 4 ) )
			BackFlip();
		else
		{
			m_nCurState = eState_Stand_1_Ready;
			m_nStateTick = 0;
		}
	}
	m_nBufferedInput = 0;
	return b;
}
