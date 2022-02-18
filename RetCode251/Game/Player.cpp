#include "stdafx.h"
#include "Player.h"
#include "Stage.h"
#include "World.h"
#include "Common/ResourceManager.h"
#include "MyLevel.h"
#include "MyGame.h"
#include "GlobalCfg.h"
#include "Entities/Bullet.h"
#include "Entities/PlayerMisc.h"
#include "Entities/UtilEntities.h"

void CPlayerCross::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
}

void CPlayerCross::OnTickAfterHitTest()
{
	if( m_bControlled )
	{
		m_bTryingToAttach = false;
		if( CGame::Inst().IsKeyDown( VK_SHIFT ) )
		{
			GetLevel()->TryDetach();
			if( !m_bControlled )
				CGame::Inst().ForceKeyRelease( VK_SHIFT );
		}
	}
	else
	{
		if( CGame::Inst().IsKey( VK_SHIFT ) )
		{
			m_pCurLockedTarget = GetLevel()->TryAttach( m_pCurLockedTarget );
			m_bTryingToAttach = m_pCurLockedTarget != NULL;
		}
		else
		{
			m_pCurLockedTarget = GetLevel()->FindAttach();
			m_bTryingToAttach = false;
		}
	}

	if( !m_bControlled )
	{
		if( m_bTryingToAttach )
		{
			auto p = m_pCurLockedTarget->globalTransform.GetPosition();
			auto d = GetPosition() - p;
			auto l = d.Length();
			float dl = m_fMoveSpeed * GetLevel()->GetDeltaTime();
			l = Max( l, dl );
			SetPosition( p + d * ( ( l - dl ) / l ) );
		}
		else
		{
			int32 nMoveX = CGame::Inst().IsKey( 'D' ) - CGame::Inst().IsKey( 'A' );
			int32 nMoveY = CGame::Inst().IsKey( 'W' ) - CGame::Inst().IsKey( 'S' );
			if( nMoveX || nMoveY )
			{
				CVector2 d( nMoveX, nMoveY );
				d.Normalize();
				d = d * m_fMoveSpeed * GetLevel()->GetDeltaTime();
				auto newPos = GetPosition() + d;
				auto levelSize = GetLevel()->GetSize();
				newPos.x = Max( levelSize.x, Min( levelSize.GetRight(), newPos.x ) );
				newPos.y = Max( levelSize.y, Min( levelSize.GetBottom(), newPos.y ) );
				SetPosition( GetPosition() + d );
			}
		}
	}
	else
	{
		SetPosition( GetLevel()->GetControlled()->globalTransform.GetPosition() );
	}
	GetLevel()->GetHitTestMgr( GetUpdateGroup() ).Update( this );
	CMasterLevel::GetInst()->UpdateTest();
	CMasterLevel::GetInst()->UpdateAlert();
}

void CPlayerCross::PostUpdate()
{
}

int8 CPlayer0::CheckPush( SRaycastResult & hit, const CVector2 & dir, float & fDist, SPush & context, int32 nPusher )
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
	CEntity* pTestEntity = this;
	CMatrix2D mat = globalTransform;
	fDist = GetLevel()->Push( this, context, dir, fDist, 1, &pTestEntity, &mat, MOVE_SIDE_THRESHOLD );
	return 1;
}

void CPlayer0::HandlePush( const CVector2 & dir, float fDist, int8 nStep )
{
	if( nStep == 0 )
	{
		if( fDist > 0 )
		{
			SetPosition( GetPosition() + dir * fDist );
			SetDirty();
		}
	}
	else if( nStep == 1 )
	{
		if( fDist > 0 )
			GetLevel()->GetHitTestMgr( GetUpdateGroup() ).Update( this );
	}
	else
	{
		if( m_pLanded && m_pLanded->nPublicFlag )
			lastLandedEntityTransform = m_pLanded->globalTransform;
		CEntity* pTestEntity = this;
		m_moveData.CleanUpOpenPlatforms( this, 1, &pTestEntity );
	}
}

bool CPlayer0::CounterBullet( CEntity* p, const CVector2& hitPos, const CVector2& hitDir, bool bHitPlayer )
{
	if( m_nKickLevel < eKickLevel_Combo )
		return false;
	if( !m_pCurAttackEffect )
		return false;
	auto pKick = SafeCast<CKick>( m_pCurAttackEffect.GetPtr() );
	if( pKick )
	{
		if( bHitPlayer )
		{
			/*auto gravityDir = GetLevel()->GetGravityDir();
			auto tangentDir = CVector2( -gravityDir.y, gravityDir.x ) * m_nDir;
			if( tangentDir.Dot( hitDir ) > 0 )
			return false;*/
			return false;
		}
		return pKick->CounterBullet( p, hitPos, hitDir );
	}
	return false;
}

void CPlayer0::UpdateInput()
{
	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();
	if( !m_bAttached )
	{
		JumpCancel( gravityDir );
		m_nJumpState = 0;
		return;
	}
	m_nMoveState = CGame::Inst().IsKey( 'D' ) - CGame::Inst().IsKey( 'A' );
	if( m_nCurState == eState_Stand || m_nCurState == eState_Walk || m_nCurState == eState_Jump )
	{
		if( m_nMoveState )
			m_nDir = m_nMoveState;
		if( m_pLanded )
			m_nJumpState = m_nLandTime;
		else if( m_nJumpState > 0 )
			m_nJumpState--;

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

		if( m_nKickLevel >= eKickLevel_Basic && CGame::Inst().IsKey( 'K' ) || CGame::Inst().IsKey( 'L' ) )
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
	}

	if( m_nCurState == eState_Kick_Ready )
	{
		if( CGame::Inst().IsKey( 'K' ) )
			m_nBufferedInput |= 16;
		if( CGame::Inst().IsKey( 'L' ) )
			m_nBufferedInput |= 32;
		auto lo = m_nBufferedInput & 7;
		if( CGame::Inst().IsKey( 'W' ) )
			lo = 2;
		if( m_nMoveState )
		{
			m_nDir = m_nMoveState;
			lo = lo | 1;
		}
		m_nBufferedInput = lo | ( m_nBufferedInput & ~7 );
	}
	else if( m_nCurState == eState_Kick )
	{
		if( m_kickState.nAni < eAni_Kick_Special_Begin || m_kickState.nAni == eAni_Kick_Recover )
		{
			if( CGame::Inst().IsKey( 'J' ) && m_kickState.bHit && !m_kickState.nFlag1 )
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
		if( m_kickState.nAni == eAni_Kick_Spin )
		{
			/*if( CGame::Inst().IsKey( 'S' ) )
			{
				m_nStateTick = eAni_Kick_Spin_Slide << 16;
				auto gravityDir = GetLevel()->GetGravityDir();
				CVector2 tangentDir( -gravityDir.y, gravityDir.x );
				tangentDir = tangentDir * m_nDir;
				m_vel = tangentDir * m_kickSpinSlideVel.x - gravityDir * m_kickSpinSlideVel.y;
			}*/
		}
		else if( m_kickState.nAni == eAni_Kick_Recover )
		{
			if( m_kickState.bHit && ( CGame::Inst().IsKey( 'K' ) || CGame::Inst().IsKey( 'L' ) ) )
			{
				if( m_nKickLevel >= eKickLevel_Combo && m_nDir == 1 && CGame::Inst().IsKey( 'A' ) || m_nDir == -1 && CGame::Inst().IsKey( 'D' ) )
				{
					uint8 nFlag = m_kickState.nFlag;
					Kick( eAni_Kick_Rev );
					m_kickState.nFlag = nFlag;
					if( CGame::Inst().IsKey( 'K' ) )
						m_kickState.nType0 |= 1;
					if( CGame::Inst().IsKey( 'L' ) )
						m_kickState.nType0 |= 2;
				}
				else if( m_nKickLevel >= eKickLevel_Combo && CGame::Inst().IsKey( 'S' ) )
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
}

void CPlayer0::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
}

void CPlayer0::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( m_bKilled )
	{
		bVisible = false;
		return;
	}

	auto pLevel = GetLevel();
	auto gravityDir = pLevel->GetGravityDir();

	const CMatrix2D oldTransform = GetGlobalTransform();
	CMatrix2D curTransform = oldTransform;
	const CVector2 oldPos = oldTransform.GetPosition();
	CVector2 curPos = oldPos;
	float fDeltaTime = GetLevel()->GetDeltaTime();
	int32 nDeltaTick = GetLevel()->GetDeltaTick();

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

			CVector2 landVelocity = ( newPos - oldPos ) / fDeltaTime - m_vel;

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
		m_nKnockBackTime = Max( 0, m_nKnockBackTime - nDeltaTick );
	if( m_bKnockback )
	{
		m_nBufferedInput = 0;

		switch( m_nCurState )
		{
		case eState_Kick_Ready:
		case eState_Kick:
			KickBreak();
			break;
		}
	}
	else
		UpdateInput();

	CEntity* pTestEntity = this;
	CVector2 gravity0( 0, 0 );
	CVector2* pGravity = CanHitPlatform() ? &gravityDir : &gravity0;
	if( !m_moveData.ResolvePenetration( this, &m_vel, 0, m_pLanded, &landedEntityOfs, pGravity, &pTestEntity, 1 ) )
	{
		if( pGravity == &gravity0 || ( pGravity = &gravity0, !m_moveData.ResolvePenetration( this, &m_vel, 0, m_pLanded, &landedEntityOfs, pGravity, &pTestEntity, 1 ) ) )
		{
			Crush();
			return;
		}
	}

	bool bFixOfs = false;
	CVector2 fixOfs( 0, 0 );

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
			if( m_kickState.nTick == nKickFrame * T_SCL )
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

			if( !m_kickState.nFlag1 && m_kickState.nTick <= nKickDashFrame* T_SCL )
			{
				if( CGame::Inst().IsKey( 'K' ) )
					m_kickState.nType0 |= 1;
				if( CGame::Inst().IsKey( 'L' ) )
					m_kickState.nType0 |= 2;
			}
			if( /*m_kickState.bHit &&*/ !m_kickState.nFlag1 && m_kickState.nTick == nKickDashFrame * T_SCL && m_nKickLevel >= eKickLevel_Heavy )
			{
				if( m_kickState.nType0 == 3 )
				{
					m_nBufferedInput = 0;
					m_kickState.nTick1 = m_nKickSpinDashTime[nKickType0] * T_SCL;
					m_kickState.nFlag1 = 1;
					m_nDir = -m_nDir;
					auto pKick = SafeCast<CCharacter>( m_pKickSpin[nKickType0]->GetRoot()->CreateInstance() );
					pKick->SetParentEntity( this );
					pKick->SetOwner( this );
					KickMorph( pKick );
				}
				else if( m_kickState.nType0 == 2 )
				{
					m_kickState.nTick1 = m_nKickDashTime * T_SCL;
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
				bFixOfs = true;
				if( m_kickState.nFlag1 )
				{
					m_vel = c * m_nKickSpinDashSpeed;
					fixOfs = m_vel * fDeltaTime;
				}
				else
				{
					auto nTick2 = m_kickState.nTick1 - nDeltaTick;
					m_vel = c * m_fKickDashSpeed[nKickType0] * nTick2 / m_nKickDashTime;
					fixOfs = c * m_fKickDashSpeed[nKickType0] * ( m_kickState.nTick1 + nTick2 ) * 0.5f / m_nKickDashTime * fDeltaTime;
				}
			}
			else if( m_kickState.nTick1 < 0 )
			{
				bFixOfs = true;
				fixOfs = m_vel = CVector2( 0, 0 );
			}
		}
		break;
	}
	default:
		break;
	}

	auto pos0 = GetPosition();
	CVector2 dPos = bFixOfs ? fixOfs : m_vel * fDeltaTime;

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
			if( m_nCurState <= eState_Jump || m_nCurState == eState_Kick || m_nCurState == eState_Kick_Ready )
			{
				moveDir = tangent * ( m_nCurState <= eState_Jump || m_nCurState == eState_Kick && m_kickState.nAni == eAni_Kick_Spin ? m_nMoveState : 0 );
				float fSpeedTarget = 0;
				float k = tangent.Dot( moveDir );
				float fTangentVelocity = m_vel.Dot( tangent );
				float fMaxSpeed = ( m_nCurState == eState_Kick ? m_nKickSpinMaxSpeed : ( m_pLanded ? m_fMoveSpeed : m_fAirMaxSpeed ) ) * k + fSpeedTarget;
				fSpeedTarget = m_fMoveSpeed * k + fSpeedTarget;
				if( !m_pLanded )
				{
					if( k > 0 )
						fSpeedTarget = Min( fMaxSpeed, Max( fSpeedTarget, fTangentVelocity ) );
					else if( k < 0 )
						fSpeedTarget = Max( fMaxSpeed, Min( fSpeedTarget, fTangentVelocity ) );
				}

				bool bIsMoveOrStop = k == 0 ? false : fTangentVelocity * k >= 0 && fTangentVelocity * k < fSpeedTarget * k;
				fAcc = m_pLanded ? ( bIsMoveOrStop ? m_fMoveAcc : m_fStopAcc ) : m_fAirAcc;
				fDeltaSpeed = fSpeedTarget - fTangentVelocity;
				if( fDeltaSpeed < 0 )
				{
					fDeltaSpeed = -fDeltaSpeed;
					tangent = tangent * -1;
				}
				fAcc += tangent.Dot( gravityDir ) * m_fGravity;
				t0 = Min( fDeltaTime, fDeltaSpeed / fAcc );
				t1 = fDeltaTime - t0;

				dVelocity = dVelocity + tangent * ( fAcc * t0 );
				dPos = dPos + tangent * ( fAcc * t0 * ( t0 * 0.5f + t1 ) );
			}
		}

		if( !m_pLanded )
		{
			float fTargetSpeed = m_fMaxFallSpeed;
			float fNormalVelocity = m_vel.Dot( gravityDir );
			fDeltaSpeed = fTargetSpeed - fNormalVelocity;
			float fYAcc = fDeltaSpeed < 0 ? -m_fGravity : m_fGravity;
			t0 = Min( fDeltaTime, fDeltaSpeed / fYAcc );
			t1 = fDeltaTime - t0;
			dVelocity = dVelocity + gravityDir * ( fYAcc * t0 );
			dPos = dPos + gravityDir * ( fYAcc * t0 * ( t0 * 0.5f + t1 ) );
		}
		m_vel = m_vel + dVelocity;

		dPos = dPos + landedEntityOfs;
	}

	GetLevel()->GetHitTestMgr( GetUpdateGroup() ).Update( this );
	auto v0 = m_vel;
	SRaycastResult res[3];
	if( dPos.Length2() > 0 )
		m_moveData.TryMove1( this, 1, &pTestEntity, dPos, m_vel, 0, CanHitPlatform() ? &gravityDir : &gravity0, res );
	if( bFixOfs )
		m_vel = v0;

	if( CanFindFloor() )
	{
		bool bPreLandedEntity = m_pLanded != NULL;
		FindFloor( gravityDir );
		if( m_bKnockback && m_pLanded )
			m_bKnockback = false;
	}
	else
		m_pLanded = NULL;

	if( m_nFireCDLeft )
		m_nFireCDLeft--;
	if( CanShoot() )
	{
		if( CGame::Inst().IsKey( 'K' ) && !m_nFireCDLeft )
		{
			m_nFireCDLeft = m_nFireCD;
			curTransform = GetGlobalTransform();
			for( int i = 0; i < m_arrBulletOfs.Size(); i++ )
			{
				auto ofs = m_arrBulletOfs[i];
				ofs.x *= m_nDir;
				auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				CVector2 bulletDir( -gravityDir.y, gravityDir.x );
				auto vel = bulletDir * m_nDir * 2000;
				auto pos = curTransform.MulVector2Pos( ofs );
				pBullet->SetPosition( pos );
				pBullet->SetBulletVelocity( vel );
				pBullet->SetOwner( this );
				pBullet->SetParentEntity( pLevel );
			}
		}
	}

	GetLevel()->GetHitTestMgr( GetUpdateGroup() ).Update( this );
}

void CPlayer0::PostUpdate()
{
	auto gravityDir = GetLevel()->GetGravityDir();
	UpdateAni( gravityDir );
	UpdateAnimState( gravityDir );
	CEntity* pTestEntity = this;
	m_moveData.CleanUpOpenPlatforms( this, 1, &pTestEntity );
	m_lastFramePos = GetPosition();
}

void CPlayer0::OnKickFirstHit( CEntity * pKick )
{
	if( m_nCurState == eState_Kick && m_pCurAttackEffect == pKick )
	{
		m_kickState.bHit = 1;
	}
}

void CPlayer0::OnKickFirstExtentHit( CEntity * pKick )
{
	/*if( m_nCurState == eState_Kick && m_pCurAttackEffect == pKick )
	{
		m_kickState.nTick1 = 0;
		m_vel = CVector2( 0, 0 );
	}*/
}

bool CPlayer0::Knockback( const CVector2& vec )
{
	if( m_nKnockBackTime )
		return false;
	m_bKnockback = true;
	m_nKnockBackTime = 30 * T_SCL;
	CVector2 t( vec.y, -vec.x );
	m_vel = vec + t * m_vel.Dot( t ) / Max( 1.0f, t.Length2() );
	return true;
}

bool CPlayer0::CanFindFloor()
{
	if( m_nKnockBackTime )
		return false;
	if( m_nCurState <= eState_Jump )
	{
		if( m_nJumpState < 0 )
			return false;
	}
	return true;
}

void CPlayer0::FindFloor( const CVector2 & gravityDir )
{
	CMatrix2D trans = GetGlobalTransform();
	CVector2 dir = gravityDir;
	CVector2 ofs = dir * m_fFindFloorDist;
	CEntity* pTestEntitiy = this;
	CMatrix2D mat = GetGlobalTransform();
	SRaycastResult result;
	CVector2 gravity0( 0, 0 );
	auto pNewLandedEntity = SafeCast<CCharacter>( m_moveData.DoSweepTest1( this, 1, &pTestEntitiy, &mat, ofs, MOVE_SIDE_THRESHOLD, CanHitPlatform() ? &dir : &gravity0, &result ) );

	if( pNewLandedEntity && m_vel.Dot( result.normal ) < 5.0f && result.normal.Dot( dir ) < -0.5f )
	{
		m_pLanded = pNewLandedEntity;
		m_nLastLandTime = 100;
		m_lastLandPoint = result.hitPoint;
		SetPosition( GetPosition() + dir * result.fDist );
		m_groundNorm = result.normal;
		m_vel = m_vel - m_groundNorm * m_vel.Dot( m_groundNorm );
		lastLandedEntityTransform = m_pLanded->GetGlobalTransform();
	}
	else
	{
		m_pLanded = NULL;
		if( m_nLastLandTime )
			m_nLastLandTime--;
		else
			m_lastLandPoint = GetPosition() + gravityDir * 24;
	}
}

void CPlayer0::UpdateAni( const CVector2 & gravityDir )
{
	auto tangent = CVector2( -gravityDir.y, gravityDir.x ) * m_nDir;
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
			texRect = CRectangle( 5 + ( m_nStateTick / ( m_nWalkAnimSpeed * T_SCL ) ) * 2, 0, 2, 3 ) / 32;
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
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[0] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 3, 3, 3 ) / 32 : CRectangle( 18, 3, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_B )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[0] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 0, 3, 3 ) / 32 : CRectangle( 18, 0, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_C )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[0] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 96, 128 );
			texRect = n ? CRectangle( 29, 7, 3, 3 ) / 32 : CRectangle( 29, 3, 3, 4 ) / 32;
		}

		else if( m_kickState.nAni == eAni_Kick_A1 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[1] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 3, 3, 3 ) / 32 : CRectangle( 18, 3, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_B1 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[1] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 0, 3, 3 ) / 32 : CRectangle( 18, 0, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_C1 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[1] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 96, 128 );
			texRect = n ? CRectangle( 29, 7, 3, 3 ) / 32 : CRectangle( 29, 3, 3, 4 ) / 32;
		}

		else if( m_kickState.nAni == eAni_Kick_A2 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[2] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 3, 3, 3 ) / 32 : CRectangle( 18, 3, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_B2 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[2] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 128, 96 );
			texRect = n ? CRectangle( 22, 0, 3, 3 ) / 32 : CRectangle( 18, 0, 4, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_C2 )
		{
			int32 n = m_kickState.nTick >= m_nKickAnimFrame1[2] * T_SCL ? 1 : 0;
			rect = n ? CRectangle( -32, -26, 96, 96 ) : CRectangle( -32, -26, 96, 128 );
			texRect = n ? CRectangle( 29, 7, 3, 3 ) / 32 : CRectangle( 29, 3, 3, 4 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_Rev )
		{
			if( m_kickState.nTick < m_kickRevAnimFrame.x * T_SCL )
			{
				rect = CRectangle( -32, -26, 96, 128 );
				texRect = CRectangle( 29, 10, 3, 4 ) / 32;
			}
			else if( m_kickState.nTick < m_kickRevAnimFrame.y * T_SCL )
			{
				rect = CRectangle( -32, 6, 96, 96 );
				texRect = CRectangle( 29, 14, 3, 3 ) / 32;
			}
			else if( m_kickState.nTick < m_kickRevAnimFrame.z * T_SCL )
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
			texRect = CRectangle( 29, 22 + ( m_kickState.nTick >= m_kickStompAnimFrame.x * T_SCL ? 3 : 0 ), 3, 3 ) / 32;
		}
		else if( m_kickState.nAni == eAni_Kick_Spin )
		{
			int32 nFrame = m_kickState.nTick / ( m_nKickSpinAnimSpeed * T_SCL );
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
			auto nFrame = m_kickState.nTick1 / ( m_nKickSpinDashAnimSpeed * T_SCL );
			if( !!( nFrame & 1 ) )
			{
				rect.x = -rect.GetRight();
				texRect.x = 2 - texRect.GetRight();
			}
		}
		break;
	}
	}
	if( m_nDir == -1 )
	{
		rect.x = -rect.GetRight();
		texRect.x = 2 - texRect.GetRight();
	}
	auto pImg = SafeCastToInterface<IImageRect>( GetRenderObject() );
	pImg->SetRect( rect );
	pImg->SetTexRect( texRect );
	UpdateEffect();
}

void CPlayer0::UpdateEffect()
{
	auto pImgEffect = SafeCastToInterface<IImageEffectTarget>( GetRenderObject() );
	auto nImpactLevel = GetCurImpactLevel();
	if( nImpactLevel )
		pImgEffect->SetCommonEffectEnabled( eImageCommonEffect_Phantom, true, CGlobalCfg::Inst().vecAttackLevelColor[nImpactLevel - 1] );
	else
		pImgEffect->SetCommonEffectEnabled( eImageCommonEffect_Phantom, false, CVector4( 0, 0, 0, 0 ) );
}

void CPlayer0::UpdateAnimState( const CVector2 & gravityDir )
{
	auto tangent = CVector2( -gravityDir.y, gravityDir.x ) * m_nDir;
	auto nDeltaTick = GetLevel()->GetDeltaTick();
	switch( m_nCurState )
	{
	case eState_Walk:
		m_nStateTick = ( m_nStateTick + nDeltaTick ) % ( m_nWalkAnimSpeed * T_SCL * 4 );
		break;
	case eState_Kick_Ready:
		m_nStateTick += nDeltaTick;
		if( m_nStateTick >= m_nKickReadyTime * T_SCL )
		{
			auto lo = m_nBufferedInput & 7;
			Kick( lo == 3 ? 0 : lo );
			m_kickState.nType0 = ( m_nBufferedInput & 48 ) >> 4;
			m_nBufferedInput = 0;
		}
		break;
	case eState_Kick:
	{
		m_kickState.nTick += nDeltaTick;
		if( m_kickState.nTick1 > 0 )
		{
			m_kickState.nTick1 = Max( 0, m_kickState.nTick1 - nDeltaTick );
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
			if( m_kickState.nTick >= m_nKickAnimFrame2[m_kickState.nAni / 3] * T_SCL )
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
					if( !!( m_nBufferedInput & 48 ) && m_nKickLevel >= eKickLevel_Combo )
					{
						if( !!( m_nBufferedInput & 4 ) )
						{
							uint8 nFlag = m_kickState.nAni / 3;
							Kick( eAni_Kick_Rev );
							m_kickState.nFlag = nFlag;
							b = true;
						}
						else if( !!( m_nBufferedInput & 8 ) )
						{
							uint8 nFlag = m_kickState.nAni / 3;
							Kick( eAni_Kick_Stomp );
							m_kickState.nFlag = nFlag;
							b = true;
						}
						else if( m_kickState.nAni < eAni_Kick_A2 )
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
			if( m_kickState.nTick >= m_kickRevAnimFrame.w * T_SCL )
				Kick( eAni_Kick_Recover );
		}
		else if( m_kickState.nAni == eAni_Kick_Stomp )
		{
			if( m_kickState.nTick >= m_kickStompAnimFrame.y * T_SCL )
				Kick( eAni_Kick_Recover );
		}
		else if( m_kickState.nAni == eAni_Kick_Spin )
		{
			if( m_kickState.nTick >= m_nKickSpinAnimRep * m_nKickSpinAnimSpeed * 6 * T_SCL )
				Kick( eAni_Kick_Recover );
		}
		else
		{
			if( m_kickState.nTick >= m_nKickRecoverTime * T_SCL )
			{
				m_kickState.nTick = m_nKickRecoverTime * T_SCL;
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
	}
}

void CPlayer0::JumpCancel( const CVector2 & gravityDir )
{
	if( m_nJumpState < 0 )
	{
		m_vel = m_vel - gravityDir * ( m_fJumpSpeed * m_nJumpState / m_nJumpHoldTime );
		m_nJumpState = 0;
	}
}

void CPlayer0::Kick( int8 nAni )
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

	if( m_pCurAttackEffect && SafeCast<CKickSpin>( m_pCurAttackEffect.GetPtr() ) )
	{
		SafeCast<CCharacter>( m_pCurAttackEffect.GetPtr() )->Kill();
		m_pCurAttackEffect = NULL;
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
	m_nStateTick = 0;
	m_nStateTickEx = nAni;
	if( pLastAttackEffect == m_pCurAttackEffect.GetPtr() )
		m_pCurAttackEffect = NULL;
	m_kickState.nFlag = nFlag;
	if( nAni == eAni_Kick_Stomp )
	{
		if( !m_pLanded )
			m_kickState.nTick = m_kickStompAnimFrame.x * T_SCL;
	}
	if( nAni == eAni_Kick_Recover )
		m_kickState.bHit = bHit;
}

void CPlayer0::KickBreak()
{
	Kick( eAni_Kick_Recover_1 );
	m_nCurState = eState_Stand;
	m_nStateTick = 0;
	m_nJumpState = 0;
}

void CPlayer0::KickMorph( CEntity * pEntity )
{
	if( m_pCurAttackEffect )
	{
		auto pKick = SafeCast<CKick>( m_pCurAttackEffect.GetPtr() );
		if( pKick )
			pKick->Morph( pEntity );
	}
	m_pCurAttackEffect = pEntity;
}

void RegisterGameClasses_Player()
{
	REGISTER_CLASS_BEGIN( CPlayerCross )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_fMoveSpeed )
	REGISTER_CLASS_END()
	
	REGISTER_CLASS_BEGIN( CPlayer0 )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_fMoveSpeed )
		REGISTER_MEMBER( m_fAirMaxSpeed )
		REGISTER_MEMBER( m_fJumpSpeed )
		REGISTER_MEMBER( m_fMoveAcc )
		REGISTER_MEMBER( m_fStopAcc )
		REGISTER_MEMBER( m_fAirAcc )
		REGISTER_MEMBER( m_fGravity )
		REGISTER_MEMBER( m_fMaxFallSpeed )
		REGISTER_MEMBER( m_fFindFloorDist )
		REGISTER_MEMBER( m_nLandTime )
		REGISTER_MEMBER( m_nJumpHoldTime )
		REGISTER_MEMBER( m_nKickLevel )
		REGISTER_MEMBER( m_nWalkAnimSpeed )
		REGISTER_MEMBER( m_fJumpAnimVel )
		REGISTER_MEMBER( m_kickVel )
		REGISTER_MEMBER( m_kickOffset )
		REGISTER_MEMBER( m_kickSpinSlideVel )
		REGISTER_MEMBER( m_fKickDashSpeed )
		REGISTER_MEMBER( m_nKickSpinMaxSpeed )
		REGISTER_MEMBER( m_nKickSpinDashSpeed )
		REGISTER_MEMBER( m_nKickReadyTime )
		REGISTER_MEMBER( m_nKickAnimFrame1 )
		REGISTER_MEMBER( m_nKickAnimFrame2 )
		REGISTER_MEMBER( m_nKickSpinAnimSpeed )
		REGISTER_MEMBER( m_nKickSpinAnimRep )
		REGISTER_MEMBER( m_kickRevAnimFrame )
		REGISTER_MEMBER( m_kickStompAnimFrame )
		REGISTER_MEMBER( m_nKickDashDelay )
		REGISTER_MEMBER( m_nKickDashTime )
		REGISTER_MEMBER( m_nKickSpinDashTime )
		REGISTER_MEMBER( m_nKickSpinDashAnimSpeed )
		REGISTER_MEMBER( m_nKickRecoverTime )
		REGISTER_MEMBER( m_nFireCD )
		REGISTER_MEMBER( m_arrBulletOfs )
		REGISTER_MEMBER( m_pKick )
		REGISTER_MEMBER( m_pKickSpin )
		REGISTER_MEMBER( m_pBullet )
	REGISTER_CLASS_END()
}