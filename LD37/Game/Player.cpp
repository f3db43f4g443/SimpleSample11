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

CPlayer::CPlayer( const SClassCreateContext& context )
	: CCharacter( context )
	, m_walkData( context )
	, m_flyData( context )
	, m_fMoveXAxis( 0 )
	, m_fMoveYAxis( 0 )
	, m_aimAt( 0, 0 )
	, m_bRoll( false )
	, m_bFiringDown( false )
	, m_bIsHiding( false )
	, m_bIsWalkOrFly( false )
	, m_hp( 50 )
	, m_nRepairTime( 60 )
	, m_nRepairInterval( 10 )
	, m_nRepairHp( 25 )
	, m_nRepairTimeLeft( 0 )
	, m_nRepairIntervalLeft( 0 )
	, m_fCrackEffectTime( 0 )
	, m_fChangeStageTime( 0 )
	, m_fHurtInvincibleTime( 0 )
	, m_nAnimState( 0 )
{
	SET_BASEOBJECT_ID( CPlayer );
}

void CPlayer::DelayChangeStage( const char* szName, const char* szStartPoint )
{
	m_fChangeStageTime = 2.0f;
	m_strChangeStage = szName;
	m_strChangeStageStartPoint = szStartPoint;
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
		DelayChangeStage( "scene0.pf", "start" );
		//CMainUI::Inst()->SetVignetteColorFixedTime( CVector4( 1, 0, 0, 80 ), 2.0f );
	}
}

void CPlayer::BeginFire()
{
	m_bFiringDown = true;
}

void CPlayer::EndFire()
{
	m_bFiringDown = false;
}

void CPlayer::BeginHide()
{
	m_bIsHiding = true;
	m_walkData.Jump( this );
}

void CPlayer::EndHide()
{
	m_bIsHiding = false;
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
		if( moveAxis.Length2() > 0 )
		{
			if( m_bIsWalkOrFly )
				m_walkData.Roll( this, moveAxis );
			else
				m_flyData.Roll( this, moveAxis );
			m_bRoll = false;
		}
	}

	if( m_bIsWalkOrFly )
		m_walkData.UpdateMove( this, moveAxis );
	else
		m_flyData.UpdateMove( this, moveAxis );

	CVector2 curPos = GetPosition();
	m_velocity = ( curPos - prePos ) / GetStage()->GetElapsedTimePerTick();
}

void CPlayer::UpdateFiring()
{
	bool bIsFiring = m_bFiringDown
		&& m_strChangeStage == ""
		&& m_hp > 0
		&& !m_bIsHiding
		&& ( m_bIsWalkOrFly ? m_walkData.nState != SCharacterWalkData::eState_Rolling : m_flyData.nState != SCharacterFlyData::eState_Rolling );
	if( m_pCurWeapon )
	{
		m_pCurWeapon->SetFiring( bIsFiring, this );
		m_pCurWeapon->Update( this );
	}
}

void CPlayer::UpdateRepair()
{
	bool bRepair = m_pCurRoom
		&& !m_bIsHiding
		&& !m_bFiringDown
		&& m_fMoveXAxis == 0 && m_fMoveYAxis == 0
		&& m_flyData.nState != SCharacterFlyData::eState_Rolling;
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

				if( nAmount && m_pCurRoomChunkUI )
					m_pCurRoomChunkUI->ShowRepairEffect();
			}
		}
	}
}

void CPlayer::OnTickAfterHitTest()
{
	if( m_hp > 0 )
	{
		bool bIsRolling = m_bIsWalkOrFly ? m_walkData.nState == SCharacterWalkData::eState_Rolling : m_flyData.nState == SCharacterFlyData::eState_Rolling;

		float fSelfPosWeight = 0.1f;
		float fAimAtPosWeight = 0.05f;
		CVector2 dPos1 = globalTransform.GetPosition() - m_cam;
		CVector2 dPos2 = m_aimAt - m_cam;
		m_cam = m_cam + dPos1 * fSelfPosWeight + dPos2 * fAimAtPosWeight;

		UpdateMove();
		if( !GetStage() )
			return;

		CChunkObject* pCurRoom = NULL;
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CEnemy* pEnemy = SafeCast<CEnemy>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
			if( pEnemy )
			{
				pEnemy->OnHitPlayer( this );
				continue;
			}

			CChunkObject* pChunkObject = SafeCast<CChunkObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
			if( pChunkObject && pChunkObject->GetChunk()->bIsRoom )
			{
				CRectangle rect( pChunkObject->GetChunk()->pos.x, pChunkObject->GetChunk()->pos.y,
					pChunkObject->GetChunk()->nWidth * CMyLevel::GetInst()->GetBlockSize(),
					pChunkObject->GetChunk()->nHeight * CMyLevel::GetInst()->GetBlockSize() );
				if( rect.Contains( GetPosition() ) )
					pCurRoom = pChunkObject;
			}
		}
		if( pCurRoom != m_pCurRoom )
		{
			if( m_pCurRoom )
			{

			}
			m_pCurRoom = pCurRoom;
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
					m_walkData.Reset();
				else
					m_flyData.Reset();
			}
		}
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
	m_bIsHiding = false;
	m_nRepairTimeLeft = 0;
	m_nRepairIntervalLeft = 0;

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