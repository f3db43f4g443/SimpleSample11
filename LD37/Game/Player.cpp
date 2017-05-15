#include "stdafx.h"
#include "Player.h"
#include "Enemy.h"
#include "Stage.h"
#include "World.h"
#include "GUI/MainUI.h"
#include "Common/ResourceManager.h"
#include "Bullet.h"
#include "MyLevel.h"
#include "CharacterMove.h"
#include "Entities/Door.h"
#include "Pickup.h"
#include "GameState.h"

CPlayer::CPlayer( const SClassCreateContext& context )
	: CCharacter( context )
	, m_walkData( context )
	, m_flyData( context )
	, m_fMoveXAxis( 0 )
	, m_fMoveYAxis( 0 )
	, m_aimAt( 0, 0 )
	, m_bRoll( false )
	, m_bFiringDown( false )
	, m_bIsRepairing( false )
	, m_bIsWalkOrFly( false )
	, m_hp( context )
	, m_sp( context )
	, m_fHidingTime( 1 )
	, m_fHidingCurTime( 0 )
	, m_nRepairTime( 40 )
	, m_nRepairInterval( 10 )
	, m_nRepairShake( 8 )
	, m_nRepairHp( 16 )
	, m_nRepairTimeLeft( 0 )
	, m_nRepairIntervalLeft( 0 )
	, m_fCrackEffectTime( 0 )
	, m_fChangeStageTime( 0 )
	, m_fHurtInvincibleTime( 0 )
	, m_fKnockbackInvincibleTime( 0 )
	, m_nAnimState( 0 )
{
	SET_BASEOBJECT_ID( CPlayer );
}

void CPlayer::AimAt( const CVector2& pos )
{
	CVector2 pos1 = pos - GetPosition();
	float l = pos1.Normalize();
	pos1 = pos1 * Min( l, 400.0f ) + GetPosition();
	CMyLevel::GetInst()->GetCrosshair()->SetPosition( pos );
	m_aimAt = pos;
}

void CPlayer::DelayChangeStage( const char* szName, const char* szStartPoint )
{
	m_fChangeStageTime = 2.0f;
	m_strChangeStage = szName;
	m_strChangeStageStartPoint = szStartPoint;
}

bool CPlayer::IsRolling()
{
	if( m_bIsWalkOrFly )
		return m_walkData.nState == SCharacterWalkData::eState_Rolling;
	else
		return m_flyData.nState == SCharacterFlyData::eState_Rolling;
}

void CPlayer::Damage( int32 nValue )
{
	if( m_strChangeStage != "" )
		return;
	CMainUI* pMainUI = CMainUI::GetInst();

	m_hp.ModifyCurValue( -nValue );
	if( pMainUI )
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
	m_fHurtInvincibleTime = 0.5f;

	if( m_hp <= 0 )
	{
		m_walkData.Reset();
		m_flyData.Reset();
		CMainGameState::Inst().DelayResetStage();
	}
}

void CPlayer::RestoreHp( int32 nValue )
{
	CMainUI* pMainUI = CMainUI::GetInst();
	m_hp.ModifyCurValue( nValue );
	if( pMainUI )
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
}

void CPlayer::CostSp( int32 nValue )
{
	CMainUI* pMainUI = CMainUI::GetInst();
	m_sp.ModifyCurValue( -nValue );
	if( pMainUI )
		pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
}

void CPlayer::RestoreSp( int32 nValue )
{
	CMainUI* pMainUI = CMainUI::GetInst();
	m_sp.ModifyCurValue( nValue );
	if( pMainUI )
		pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
}

bool CPlayer::Knockback( const CVector2& vec )
{
	if( m_bIsWalkOrFly )
		m_walkData.Knockback( 0.25f, vec * 400 );
	else
		m_flyData.Knockback( 0.25f, vec * 400 );
	m_fKnockbackInvincibleTime = 0.2f;
	return true;
}

CVector2 CPlayer::GetKnockback()
{
	if( m_hp <= 0 )
		return CVector2( 0, 0 );
	if( m_bIsWalkOrFly )
		return m_walkData.vecKnockback / 400;
	else
		return m_flyData.vecKnockback / 400;
}

void CPlayer::BeginFire()
{
	m_bFiringDown = true;
}

void CPlayer::EndFire()
{
	m_bFiringDown = false;
}

void CPlayer::BeginRepair()
{
	//m_bIsRepairing = true;
	m_walkData.Jump( this );
}

void CPlayer::EndRepair()
{
	m_bIsRepairing = false;
	m_walkData.ReleaseJump( this );
}

void CPlayer::SetWeapon( CPlayerWeapon * pWeapon )
{
	if( m_pCurWeapon )
	{
		m_pCurWeapon->Remove( this );
	}
	m_pCurWeapon = pWeapon;
	if( pWeapon )
	{
		pWeapon->SetParentEntity( this );
		pWeapon->Add( this );
		pWeapon->Face( m_nAnimState & 2 );
	}
}

#define PLAYER_ROLL_TIME 0.5f
#define PLAYER_ROLL_SPEED 512.0f

void CPlayer::OnTickBeforeHitTest()
{
	float fTime = GetStage()->GetElapsedTimePerTick();

	if( m_fCrackEffectTime > 0 )
	{
		m_crackEffect.Update( fTime );
		m_fCrackEffectTime -= fTime;
		if( m_fCrackEffectTime <= 0 )
		{
			m_fCrackEffectTime = 0;
			m_crackEffect.Reset();
		}
	}

	if( m_fChangeStageTime > 0 )
	{
		m_fChangeStageTime = Max( m_fChangeStageTime - fTime, 0.0f );
		if( m_fChangeStageTime <= 0 )
		{
			SStageEnterContext context;
			context.strStartPointName = m_strChangeStageStartPoint;
			GetStage()->GetWorld()->EnterStage( m_strChangeStage.c_str(), context );
			return;
		}
	}
	UpdateFiring();

	m_fHurtInvincibleTime = Max( m_fHurtInvincibleTime - fTime, 0.0f );
	m_fKnockbackInvincibleTime = Max( m_fKnockbackInvincibleTime - fTime, 0.0f );
	bool bVisible = m_fHurtInvincibleTime * 10 - floor( m_fHurtInvincibleTime * 10 ) < 0.5f;
	GetRenderObject()->bVisible = bVisible;
	if( m_pCurWeapon )
		m_pCurWeapon->bVisible = bVisible;

	CCharacter::OnTickBeforeHitTest();
}

void CPlayer::UpdateMove()
{
	float fTime = GetStage()->GetElapsedTimePerTick();
	CVector2 moveAxis( m_fMoveXAxis, m_fMoveYAxis );
	CVector2 prePos = GetPosition();

	if( m_bRoll )
	{
		if( m_sp >= m_nRollSpCost && moveAxis.Length2() > 0 )
		{
			CostSp( m_nRollSpCost );
			if( m_bIsWalkOrFly )
			{
				CVector2 axis;
				if( moveAxis.x > 0 )
					axis = CVector2( 1, 0 );
				else if( moveAxis.x < 0 )
					axis = CVector2( -1, 0 );
				else if( m_aimAt.x > x )
					axis = CVector2( 1, 0 );
				else
					axis = CVector2( -1, 0 );

				m_walkData.Roll( this, axis );
			}
			else
				m_flyData.Roll( this, moveAxis );
		}
		m_bRoll = false;
	}

	if( m_bIsWalkOrFly )
	{
		m_walkData.UpdateMove( this, moveAxis );
		if( m_walkData.pLandedEntity )
			RestoreSp( m_nSpRegenPerFrame );
		else if( m_walkData.nIsSlidingDownWall )
			RestoreSp( m_nSpRegenPerFrameSlidingDown );
	}
	else
	{
		m_flyData.UpdateMove( this, moveAxis );
		RestoreSp( m_nSpRegenPerFrame );
	}

	CVector2 curPos = GetPosition();
	m_velocity = ( curPos - prePos ) / GetStage()->GetElapsedTimePerTick();
}

void CPlayer::UpdateFiring()
{
	bool bIsFiring = m_bFiringDown
		&& m_strChangeStage == ""
		&& m_hp > 0
		&& ( m_bIsWalkOrFly || !m_bIsRepairing )
		&& ( m_bIsWalkOrFly ? m_walkData.nState != SCharacterWalkData::eState_Rolling : m_flyData.nState != SCharacterFlyData::eState_Rolling );
	if( m_pCurWeapon )
	{
		m_pCurWeapon->SetFiring( bIsFiring, this );
		m_pCurWeapon->Update( this );
	}
}

void CPlayer::UpdateRoom()
{
	bool bIsRolling = m_bIsWalkOrFly ? m_walkData.nState == SCharacterWalkData::eState_Rolling : m_flyData.nState == SCharacterFlyData::eState_Rolling;

	if( m_pCurRoom )
	{
		bool bCanHiding = !m_bIsRepairing && !m_bFiringDown;
		if( !bCanHiding )
			m_fHidingCurTime = 0;
		else
			m_fHidingCurTime = Min( m_fHidingCurTime + GetStage()->GetElapsedTimePerTick(), m_fHidingTime );
	}

	CChunkObject* pCurRoom = NULL;
	bool bDoor = false;
	LINK_LIST_FOR_EACH_BEGIN( pManifold, m_pManifolds, SHitProxyManifold, Manifold )
		CEnemy* pEnemy = SafeCast<CEnemy>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pEnemy )
		{
			pEnemy->OnHitPlayer( this, pManifold->normal );
			continue;
		}

		CPickUp* pPickUp = SafeCast<CPickUp>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pPickUp )
		{
			pPickUp->PickUp( this );
			continue;
		}

		CChunkObject* pChunkObject = SafeCast<CChunkObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pChunkObject && pChunkObject->GetChunk()->bIsRoom == 1 )
		{
			CRectangle rect( pChunkObject->GetChunk()->pos.x, pChunkObject->GetChunk()->pos.y,
				pChunkObject->GetChunk()->nWidth * CMyLevel::GetBlockSize(),
				pChunkObject->GetChunk()->nHeight * CMyLevel::GetBlockSize() );
			if( rect.Contains( GetPosition() ) )
				pCurRoom = pChunkObject;
		}

		CDoor* pDoor = SafeCast<CDoor>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pDoor )
			bDoor = true;
	LINK_LIST_FOR_EACH_END( pManifold, m_pManifolds, SHitProxyManifold, Manifold )

	if( pCurRoom != m_pCurRoom && ( !m_pCurRoom || !bDoor ) )
	{
		if( m_pCurRoom && m_pCurRoom->GetChunk() )
		{
			m_pCurRoom->GetChunk()->bIsBeingRepaired = false;
		}
		m_pCurRoom = pCurRoom;
		m_fHidingCurTime = 0;
		if( pCurRoom )
		{
			if( !m_pCurRoomChunkUI )
				m_pCurRoomChunkUI = SafeCast<CChunkUI>( CMyLevel::GetInst()->pChunkUIPrefeb->GetRoot()->CreateInstance() );
		}
		if( m_pCurRoomChunkUI )
			m_pCurRoomChunkUI->SetChunkObject( pCurRoom );
	}

	if( !bIsRolling )
	{
		bool bIsWalkOrFlyNew = m_pCurRoom == NULL;
		if( bIsWalkOrFlyNew != m_bIsWalkOrFly )
		{
			m_bIsWalkOrFly = bIsWalkOrFlyNew;
			if( m_bIsWalkOrFly )
			{
				m_walkData.Reset();
				m_walkData.velocity = m_velocity;
				m_walkData.fKnockbackTime = m_flyData.fKnockbackTime;
				m_walkData.vecKnockback = m_flyData.vecKnockback;
			}
			else
			{
				m_flyData.Reset();
				m_flyData.fKnockbackTime = m_walkData.fKnockbackTime;
				m_flyData.vecKnockback = m_walkData.vecKnockback;
			}
		}
	}
}

void CPlayer::UpdateRepair()
{
	bool bRepair = m_pCurRoom
		&& m_bIsRepairing
		&& m_fMoveXAxis == 0 && m_fMoveYAxis == 0
		&& m_flyData.nState != SCharacterFlyData::eState_Rolling
		&& ( m_bIsWalkOrFly ? m_walkData.fKnockbackTime <= 0 : m_flyData.fKnockbackTime <= 0 );
	if( m_pCurRoom && m_pCurRoom->GetChunk() )
		m_pCurRoom->GetChunk()->bIsBeingRepaired = bRepair;
	if( !bRepair )
	{
		m_nRepairTimeLeft = m_nRepairTime;
		m_nRepairIntervalLeft = 0;
	}
	else
	{
		if( m_nRepairTimeLeft )
			m_nRepairTimeLeft--;
		else
		{
			if( m_nRepairIntervalLeft )
				m_nRepairIntervalLeft--;
			if( !m_nRepairIntervalLeft )
			{
				uint32 nAmount = m_pCurRoom->Repair( m_nRepairHp );
				m_nRepairIntervalLeft = m_nRepairInterval;

				if( nAmount )
				{
					CMyLevel::GetInst()->AddShakeStrength( m_nRepairShake );
					if( m_pCurRoomChunkUI )
						m_pCurRoomChunkUI->ShowRepairEffect();
				}
			}
		}
	}
}

void CPlayer::OnTickAfterHitTest()
{
	if( m_hp > 0 )
	{
		UpdateMove();
		if( !GetStage() )
			return;

		float fSelfPosWeight = 0.1f;
		float fAimAtPosWeight = 0.05f;
		CVector2 dPos1 = globalTransform.GetPosition() - m_cam;
		CVector2 dPos2 = m_aimAt - m_cam;
		m_cam = m_cam + dPos1 * fSelfPosWeight + dPos2 * fAimAtPosWeight;

		UpdateRoom();
		UpdateRepair();

		uint8 newAnimState = 0;
		if( m_fMoveXAxis != 0 || m_fMoveYAxis != 0 )
			newAnimState = 1;
		if( m_bIsWalkOrFly )
		{
			if( m_walkData.nState == SCharacterWalkData::eState_Rolling )
				newAnimState = 2 + ( m_walkData.rollDir.x > 0 ? 0 : 3 );
			else
				newAnimState += m_aimAt.x > x ? 0 : 3;
		}
		else
		{
			if( m_flyData.nState == SCharacterFlyData::eState_Rolling )
				newAnimState = 2 + ( m_flyData.rollDir.x > 0 ? 0 : 3 );
			else
				newAnimState += m_aimAt.x > x ? 0 : 3;
		}

		if( newAnimState != m_nAnimState )
		{
			auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
			switch( newAnimState )
			{
			case 0:
				pImage->SetFrames( 0, 1, 0 );
				break;
			case 1:
				pImage->SetFrames( 1, 7, 12 );
				break;
			case 2:
				pImage->SetFrames( 11, 16, 9.9f );
				break;
			case 3:
				pImage->SetFrames( 16, 17, 0 );
				break;
			case 4:
				pImage->SetFrames( 17, 23, 12 );
				break;
			case 5:
				pImage->SetFrames( 27, 32, 9.9f );
				break;
			default:
				break;
			}
			m_nAnimState = newAnimState;
			if( m_pCurWeapon )
				m_pCurWeapon->Face( newAnimState >= 3 );
		}
	}
	
	CCharacter::OnTickAfterHitTest();
	GetStage()->TriggerEvent( eStageEvent_PlayerUpdated );
}

void CPlayer::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	m_fChangeStageTime = 0;
	m_strChangeStage = "";
	m_strChangeStageStartPoint = "";
	m_walkData.Reset();
	m_flyData.Reset();
	m_pCurRoom = NULL;
	m_bIsRepairing = false;
	m_nRepairTimeLeft = 0;
	m_nRepairIntervalLeft = 0;

	m_aimAt = GetPosition();
	m_cam = GetPosition();
	m_hp.add2 = 0;

	CMainUI* pMainUI = CMainUI::GetInst();
	if( pMainUI )
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
	
	//CMainUI::Inst()->SetVignetteColorFixedTime( CVector4( 0, 0, 0, 1 ), 2.0f );
}

bool CPlayer::CanTriggerItem()
{
	return m_pCurRoom;
}