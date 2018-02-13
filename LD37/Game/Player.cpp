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
#include "PlayerData.h"
#include "Interfaces.h"
#include <algorithm>

CPlayer::CPlayer( const SClassCreateContext& context )
	: CCharacter( context )
	, m_walkData( context )
	, m_flyData( context )
	, m_fMoveXAxis( 0 )
	, m_fMoveYAxis( 0 )
	, m_aimAtOfs( 0, 0 )
	, m_bRoll( false )
	, m_bFiringDown( false )
	, m_bIsRepairing( false )
	, m_bIsWalkOrFly( false )
	, m_bCachedJump( false )
	, m_fCachedJumpTime( 0 )
	, m_hp( context )
	, m_sp( context )
	, m_nHpStore( 0 )
	, m_fHidingTime( 1 )
	, m_fHidingCurTime( 0 )
	, m_nRepairTime( 40 )
	, m_nRepairInterval( 10 )
	, m_nRepairShake( 8 )
	, m_nRepairHp( 16 )
	, m_nRepairTimeLeft( 0 )
	, m_nRepairIntervalLeft( 0 )
	, m_fAimSpeed( 0 )
	, m_fCrackEffectTime( 0 )
	, m_fChangeStageTime( -1 )
	, m_fHurtInvincibleTime( 0 )
	, m_fKnockbackInvincibleTime( 0 )
	, m_nAnimState( 0 )
	, m_pItems( NULL )
{
	m_flyData.bApplyExtraGravity = true;
	SET_BASEOBJECT_ID( CPlayer );
}

void CPlayer::ModifyHp( int32 nValue )
{
	m_hp.add1 += nValue;
	CMainUI* pMainUI = CMainUI::GetInst();
	if( pMainUI )
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
}

void CPlayer::ModifySp( int32 nValue )
{
	m_sp.add1 += nValue;
	CMainUI* pMainUI = CMainUI::GetInst();
	if( pMainUI )
		pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
}

void CPlayer::ModifyMoney( int32 nMoney )
{
	m_nMoney += nMoney;
	m_onMoneyChanged.Trigger( 0, NULL );
	CMainUI::GetInst()->OnModifyMoney( m_nMoney );
}

void CPlayer::AimAt( const CVector2& pos )
{
	CVector2 ofs = pos - GetPosition();
	if( m_fAimSpeed > 0 )
	{
		CVector2 d = m_aimAtOfs - ofs;
		float l = d.Normalize();
		float l1 = m_fAimSpeed * GetStage()->GetElapsedTimePerTick();
		if( l > l1 )
			m_aimAtOfs = ofs + d * ( l - l1 );
		else
			m_aimAtOfs = ofs;
	}
	else
		m_aimAtOfs = ofs;

	CMyLevel::GetInst()->GetCrosshair()->SetPosition( GetAimAt() );
	CMyLevel::GetInst()->GetCrosshair()->bVisible = m_pCurWeapon != NULL;

	{
		CVector2 d = m_pBlockDetectUI->GetPosition() - ofs;
		float l = d.Normalize();
		float l1 = 1000 * GetStage()->GetElapsedTimePerTick();
		if( l > l1 )
			m_pBlockDetectUI->SetPosition( ofs + d * ( l - l1 ) );
		else
			m_pBlockDetectUI->SetPosition( ofs );
	}
}

void CPlayer::DelayChangeStage( float fTime, const char* szName, const char* szStartPoint )
{
	m_fChangeStageTime = fTime;
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

void CPlayer::Damage( SDamageContext& context )
{
	if( m_hp <= 0 )
		return;
	CMainUI* pMainUI = CMainUI::GetInst();

	m_hp.ModifyCurValue( -context.nDamage );
	if( pMainUI )
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
	m_fHurtInvincibleTime = 0.5f;

	if( m_hp <= 0 )
	{
		m_walkData.Reset();
		m_flyData.Reset();
		IRenderSystem::Inst()->SetTimeScale( 0.0f, 0.25f );
		CMyLevel::GetInst()->OnPlayerKilled( this );
	}
}

void CPlayer::HealHp( int32 nValue )
{
	m_hp.ModifyCurValue( nValue );
	CMainUI* pMainUI = CMainUI::GetInst();
	if( pMainUI )
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
}

void CPlayer::RestoreHp( int32 nValue )
{
	CMainUI* pMainUI = CMainUI::GetInst();
	int32 nPreHp = m_hp;
	m_hp.ModifyCurValue( nValue );
	int32 nCurHp = m_hp;

	int32 nStore = nValue - ( nCurHp - nPreHp );
	if( nStore > 0 )
	{
		m_nHpStore += nStore;
		while( m_nHpStore >= m_hp.base )
		{
			m_nHpStore -= m_hp.base;
			m_hp.base += 2;
		}
		if( pMainUI )
			pMainUI->OnModifyHpStore( m_nHpStore, m_hp.GetMaxValue() );
	}

	if( pMainUI )
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
}

bool CPlayer::CheckCostSp( int32 nValue, int8 nType )
{
	if( m_sp >= nValue )
		return true;

	CMainUI* pMainUI = CMainUI::GetInst();
	if( pMainUI )
	{
		pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
		pMainUI->AddSpBarShake( nType == 0 ? CVector2( -8, 0 ) : CVector2( 0, 8 ), 20 );
	}
	return false;
}

void CPlayer::CostSp( int32 nValue, int8 nType )
{
	CMainUI* pMainUI = CMainUI::GetInst();
	m_sp.ModifyCurValue( -nValue );
	if( pMainUI )
	{
		pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
		pMainUI->AddSpBarShake( nType == 0 ? CVector2( -1, 0 ) : CVector2( 0, 2 ), 20 );
	}
}

void CPlayer::RecoverSp( int32 nValue )
{
	CMainUI* pMainUI = CMainUI::GetInst();
	m_sp.ModifyCurValue( nValue );
	if( pMainUI )
		pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
}

void CPlayer::Crush()
{
	CRectangle rect;
	Get_HitProxy()->CalcBound( globalTransform, rect );
	vector<CChunkObject*> chunkObjs;
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pBlock = SafeCast<CBlockObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pBlock )
		{
			auto pChunk = pBlock->GetBlock()->pOwner->pChunkObject;
			CVector2 pos = pChunk->globalTransform.GetPosition() + CVector2( pBlock->x, pBlock->y ) * CMyLevel::GetBlockSize();
			if( pChunk->GetCrushCost() && pos.y <= rect.GetBottom() && pos.y >= rect.GetTop() )
				chunkObjs.push_back( pChunk );
		}
	}

	if( !chunkObjs.size() )
	{
		SDamageContext context;
		context.nDamage = 1000;
		context.nType = 0;
		context.nSourceType = 0;
		context.hitPos = context.hitDir = CVector2( 0, 0 );
		context.nHitType = -1;
		Damage( context );

		return;
	}
	std::sort( chunkObjs.begin(), chunkObjs.end() );
	
	int i, j = 0;
	for( i = 1, j = 1; i < chunkObjs.size(); i++ )
	{
		if( chunkObjs[i] != chunkObjs[i - 1] )
			chunkObjs[j++] = chunkObjs[i];
	}

	int32 nCostSp = 0;
	bool bCertainlyCrush = false;
	for( i = 0; i < j; i++ )
	{
		if( chunkObjs[i]->GetCrushCost() > m_sp.GetMaxValue() )
			bCertainlyCrush = true;
		nCostSp += chunkObjs[i]->GetCrushCost();
	}

	if( !bCertainlyCrush && IsRolling() )
		nCostSp = Min<int32>( nCostSp, m_sp );

	if( !CheckCostSp( nCostSp, 1 ) )
	{
		SDamageContext context;
		context.nDamage = 1000;
		context.nType = 0;
		context.nSourceType = 0;
		context.hitPos = context.hitDir = CVector2( 0, 0 );
		context.nHitType = -1;
		Damage( context );

		return;
	}

	CostSp( nCostSp, 1 );
	for( i = 0; i < j; i++ )
		chunkObjs[i]->Crush();
	if( !m_walkData.ResolvePenetration( this ) )
	{
		SDamageContext context;
		context.nDamage = 1000;
		context.nType = 0;
		context.nSourceType = 0;
		context.hitPos = context.hitDir = CVector2( 0, 0 );
		context.nHitType = -1;
		Damage( context );

		return;
	}

	IRenderSystem::Inst()->SetTimeScale( 0.0f, 0.5f );
	//Knockback( CVector2( 0, -1 ) );

	SDamageContext context;
	context.nDamage = ( m_hp + 1 ) >> 1;
	context.nType = 0;
	context.nSourceType = 0;
	context.hitPos = context.hitDir = CVector2( 0, 0 );
	context.nHitType = -1;
	Damage( context );
}

bool CPlayer::Knockback( const CVector2& vec )
{
	if( m_bIsWalkOrFly )
		m_walkData.Knockback( 0.25f, vec * 320 );
	else
		m_flyData.Knockback( 0.25f, vec * 320 );
	m_fKnockbackInvincibleTime = 0.2f;
	return true;
}

bool CPlayer::StartHooked( CEntity *pEntity )
{
	if( m_pHook )
	{
		auto pHook = SafeCastToInterface<IHook>( m_pHook.GetPtr() );
		if( pHook )
			pHook->OnDetach();
	}
	m_pHook = pEntity;
	return true;
}

bool CPlayer::Hooked( const CVector2& vec )
{
	if( !IsHooked() )
		return false;
	if( m_bIsWalkOrFly )
		m_walkData.Hooked( vec );
	else
		m_flyData.Hooked( vec );
	return true;
}

bool CPlayer::EndHooked()
{
	if( !m_pHook )
		return false;
	m_pHook = NULL;
	return true;
}

bool CPlayer::IsHooked()
{
	return m_pHook != NULL;
}

CVector2 CPlayer::GetKnockback()
{
	if( m_hp <= 0 )
		return CVector2( 0, 0 );
	if( m_bIsWalkOrFly )
	{
		if( m_walkData.nState == SCharacterWalkData::eState_Knockback )
			return m_walkData.vecKnockback / 400;
	}
	else
	{
		if( m_flyData.nState == SCharacterFlyData::eState_Knockback )
			return m_flyData.vecKnockback / 400;
	}
	return CVector2( 0, 0 );
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
	if( m_bIsWalkOrFly )
		m_walkData.Jump( this );
	else
	{
		m_fCachedJumpTime = 0;
		m_bCachedJump = true;
	}
}

void CPlayer::EndRepair()
{
	m_bIsRepairing = false;
	m_walkData.ReleaseJump( this );
	m_bCachedJump = false;
}

void CPlayer::AddItem( CItem * pItem )
{
	DEFINE_TEMP_REF( pItem )
	pItem->SetParentEntity( NULL );
	CString strKey = pItem->GetKey();
	if( strKey.length() )
	{
		auto& pKeyItem = m_mapKeyItems[strKey];
		if( !pKeyItem )
		{
			pKeyItem = pItem;
			m_mapKeyItemLevels[strKey] = 1;
		}
		else
		{
			auto pUpgrade = pKeyItem->GetUpgrade();
			if( pUpgrade )
			{
				pKeyItem->Remove( this );
				pKeyItem = SafeCast<CItem>( pUpgrade->GetRoot()->CreateInstance() );
				m_mapKeyItemLevels[strKey]++;
			}

			pItem = pKeyItem;
		}
	}
	Insert_Item( pItem );

	pItem->Add( this );
	m_onItemChanged.Trigger( 0, NULL );
}

void CPlayer::RemoveItem( CItem * pItem )
{
	pItem->Remove( this );

	CString strKey = pItem->GetKey();
	if( strKey.length() )
	{
		m_mapKeyItems.erase( strKey );
		m_mapKeyItemLevels.erase( strKey );
	}
	Remove_Item( pItem );
	m_onItemChanged.Trigger( 0, NULL );
}

int32 CPlayer::CheckItemLevel( CItem* pItem )
{
	auto itr = m_mapKeyItemLevels.find( pItem->GetKey() );
	if( itr == m_mapKeyItemLevels.end() )
		return 0;

	auto itr1 = m_mapKeyItems.find( pItem->GetKey() );
	if( itr1->second->GetUpgrade() )
		return itr->second;
	else
		return -itr->second;
}

int8 CPlayer::CanAddConsumable( CConsumable * pConsumable )
{
	for( int i = 0; i < 6; i++ )
	{
		if( !m_pConsumables[i] )
			return i;
	}
	return INVALID_8BITID;
}

int8 CPlayer::AddConsumable( CConsumable * pConsumable )
{
	for( int i = 0; i < 6; i++ )
	{
		if( !m_pConsumables[i] )
		{
			m_pConsumables[i] = pConsumable;
			auto pMainUI = CMainUI::GetInst();
			if( pMainUI )
				pMainUI->OnAddConsumable( pConsumable, i );
			return i;
		}
	}
	return INVALID_8BITID;
}

bool CPlayer::UseConsumable( int32 i )
{
	if( i >= 6 || !m_pConsumables[i] )
		return false;
	if( !m_pConsumables[i]->Use( this ) )
		return false;
	m_pConsumables[i] = NULL;
	auto pMainUI = CMainUI::GetInst();
	if( pMainUI )
		pMainUI->OnRemoveConsumable( i );
	return true;
}

void CPlayer::SetWeapon( CPlayerWeapon * pWeapon )
{
	if( m_pCurWeapon )
	{
		if( m_bFiringDown )
			m_pCurWeapon->EndFire( this );
		m_pCurWeapon->UnEquip( this );
		m_pCurWeapon->SetParentEntity( NULL );
	}
	m_pCurWeapon = pWeapon;
	if( pWeapon )
	{
		pWeapon->SetPosition( CVector2( 0, 0 ) );
		pWeapon->SetParentEntity( this );
		pWeapon->Equip( this );
		pWeapon->Face( m_nAnimState & 2 );
		if( m_bFiringDown )
			m_pCurWeapon->BeginFire( this );
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

	if( m_fChangeStageTime >= 0 )
	{
		m_fChangeStageTime = Max( m_fChangeStageTime - fTime, 0.0f );
		if( m_fChangeStageTime <= 0 )
		{
			m_fChangeStageTime = -1;
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
		if( moveAxis.Length2() > 0 && ( m_nSpecialFlags[eSpecialFlag_Boost] || CheckCostSp( m_nRollSpCost, 0 ) ) )
		{
			if( !m_nSpecialFlags[eSpecialFlag_Boost] )
				CostSp( m_nRollSpCost, 0 );
			if( m_bIsWalkOrFly )
			{
				CVector2 axis;
				if( moveAxis.x > 0 )
					axis = CVector2( 1, 0 );
				else if( moveAxis.x < 0 )
					axis = CVector2( -1, 0 );
				else if( m_aimAtOfs.x > 0 )
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
		if( m_nSpecialFlags[eSpecialFlag_Boost] )
			RecoverSp( m_nSpRegenPerFrame );
		if( m_walkData.pLandedEntity )
			RecoverSp( m_nSpRegenPerFrame );
		else if( m_walkData.nIsSlidingDownWall )
			RecoverSp( m_nSpRegenPerFrameSlidingDown );
	}
	else
	{
		m_flyData.UpdateMove( this, m_bCachedJump ? moveAxis * 0.5f : moveAxis );
		if( m_flyData.nState == SCharacterFlyData::eState_Rolling )
			m_bCachedJump = false;
		else
			m_fCachedJumpTime += fTime;
		RecoverSp( m_nSpRegenPerFrame );
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

	CReference<CEntity> pUseableEntity;
	LINK_LIST_FOR_EACH_BEGIN( pManifold, m_pManifolds, SHitProxyManifold, Manifold )
		CEnemy* pEnemy = SafeCast<CEnemy>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pEnemy )
		{
			pEnemy->OnHitPlayer( this, pManifold->normal );
			continue;
		}

		CPickUp* pPickUp = SafeCast<CPickUp>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pPickUp && pPickUp->CanPickUp( this ) )
		{
			if( !pPickUp->GetPrice() )
				pPickUp->PickUp( this );
			else
				pUseableEntity = pPickUp;
			continue;
		}

		CChunkObject* pChunkObject = SafeCast<CChunkObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pChunkObject && pChunkObject->GetChunk()->nMoveType == 1 )
		{
			CRectangle rect( 0, 0,
				pChunkObject->GetChunk()->nWidth * CMyLevel::GetBlockSize(),
				pChunkObject->GetChunk()->nHeight * CMyLevel::GetBlockSize() );
			rect = rect.Offset( pChunkObject->GetGlobalTransform().GetPosition() );
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
				if( m_flyData.nState >= SCharacterFlyData::eState_Knockback )
					m_walkData.nState = m_flyData.nState - SCharacterFlyData::eState_Knockback + SCharacterWalkData::eState_Knockback;

				if( m_bCachedJump )
				{
					m_bCachedJump = false;
					m_walkData.ReleaseCachedJump( this, m_fCachedJumpTime );
				}
			}
			else
			{
				m_flyData.Reset();
				m_flyData.fKnockbackTime = m_walkData.fKnockbackTime;
				m_flyData.vecKnockback = m_walkData.vecKnockback;
				if( m_walkData.nState >= SCharacterWalkData::eState_Knockback )
					m_flyData.nState = m_walkData.nState - SCharacterWalkData::eState_Knockback + SCharacterFlyData::eState_Knockback;
			}
		}
	}

	if( pUseableEntity && pUseableEntity->GetStage() )
	{
		auto pPickup = SafeCast<CPickUp>( pUseableEntity.GetPtr() );
		if( pPickup && pPickup->GetPrice() > m_nMoney )
		{
			pUseableEntity = NULL;
			pPickup = NULL;
		}

		if( pUseableEntity && m_bUse )
		{
			if( pPickup )
			{
				pPickup->PickUp( this );
				ModifyMoney( -(int32)pPickup->GetPrice() );
			}
			pUseableEntity = NULL;
		}
	}

	if( pUseableEntity && pUseableEntity->GetStage() )
		CMainUI::GetInst()->ShowUseText( pUseableEntity, CVector2( 0, 96 ), "BUY" );
	else
		CMainUI::GetInst()->ShowUseText( NULL, CVector2( 0, 0 ), "" );
	m_bUse = false;
}

void CPlayer::UpdateRepair()
{
	bool bRepair = m_pCurRoom
		&& m_bIsRepairing
		&& m_fMoveXAxis == 0 && m_fMoveYAxis == 0
		&& m_flyData.nState < SCharacterFlyData::eState_Knockback;
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
		CVector2 dPos2 = GetAimAt() - m_cam;
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
				newAnimState += m_aimAtOfs.x > 0 ? 0 : 3;
		}
		else
		{
			if( m_flyData.nState == SCharacterFlyData::eState_Rolling )
				newAnimState = 2 + ( m_flyData.rollDir.x > 0 ? 0 : 3 );
			else
				newAnimState += m_aimAtOfs.x > 0 ? 0 : 3;
		}

		if( newAnimState != m_nAnimState )
		{
			auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
			auto pImage1 = static_cast<CMultiFrameImage2D*>( m_pCore->GetRenderObject() );
			switch( newAnimState )
			{
			case 0:
				pImage->SetFrames( 0, 1, 0 );
				pImage1->SetFrames( 0, 1, 0 );
				break;
			case 1:
				pImage->SetFrames( 1, 7, 12 );
				pImage1->SetFrames( 1, 7, 12 );
				break;
			case 2:
				pImage->SetFrames( 11, 16, 9.9f );
				pImage1->SetFrames( 11, 16, 9.9f );
				break;
			case 3:
				pImage->SetFrames( 16, 17, 0 );
				pImage1->SetFrames( 16, 17, 0 );
				break;
			case 4:
				pImage->SetFrames( 17, 23, 12 );
				pImage1->SetFrames( 17, 23, 12 );
				break;
			case 5:
				pImage->SetFrames( 27, 32, 9.9f );
				pImage1->SetFrames( 27, 32, 9.9f );
				break;
			default:
				break;
			}
			m_nAnimState = newAnimState;
			if( m_pCurWeapon )
				m_pCurWeapon->Face( newAnimState >= 3 );
		}
	}
	
	m_pCore->UpdateAnim( GetStage()->GetElapsedTimePerTick() );
	CCharacter::OnTickAfterHitTest();
	GetStage()->TriggerEvent( eStageEvent_PlayerUpdated );
}

void CPlayer::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	m_fChangeStageTime = -1;
	m_strChangeStage = "";
	m_strChangeStageStartPoint = "";
	m_walkData.Reset();
	m_flyData.Reset();
	m_pCurRoom = NULL;
	m_bIsRepairing = false;
	m_nRepairTimeLeft = 0;
	m_nRepairIntervalLeft = 0;
	m_pCurRoomChunkUI->SetChunkObject( NULL );
	m_bUse = false;

	AimAt( GetPosition() );
	m_cam = GetPosition();
	m_hp.add2 = 0;

	if( CPlayerData::Inst().bIsDesign )
	{
		m_nMoney = 1000;
	}

	CMainUI* pMainUI = CMainUI::GetInst();
	if( pMainUI )
	{
		pMainUI->OnModifyHp( m_hp, m_hp.GetMaxValue() );
		pMainUI->OnModifyHpStore( m_nHpStore, m_hp.GetMaxValue() );
		pMainUI->OnModifySp( m_sp, m_sp.GetMaxValue() );
		pMainUI->OnModifyMoney( m_nMoney );
	}

	auto pLevel = CMyLevel::GetInst();
	if( pLevel )
		pLevel->OnPlayerEntered( this );
	
	//CMainUI::Inst()->SetVignetteColorFixedTime( CVector4( 0, 0, 0, 1 ), 2.0f );
}

void CPlayer::OnRemovedFromStage()
{
	if( m_pHook )
	{
		auto pHook = SafeCastToInterface<IHook>( m_pHook.GetPtr() );
		if( pHook )
			pHook->OnDetach();
		m_pHook = NULL;
	}
	CCharacter::OnRemovedFromStage();
}

bool CPlayer::CanTriggerItem()
{
	return m_pCurRoom;
}