#pragma once
#include "Attribute.h"
#include "Common/Trigger.h"
#include "PostEffects.h"
#include "Render/Image2D.h"
#include "Common/StringUtil.h"
#include "Character.h"
#include "CharacterMove.h"
#include "PlayerWeapon.h"
#include "Block.h"
#include "GUI/ChunkUI.h"
#include "Item.h"

class CPlayer : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CPlayer( const SClassCreateContext& context );

	enum
	{
		eState_NormalWalking,
		eState_Rolling,
	};

	int32 GetHp() { return m_hp; }
	int32 GetMaxHp() { return m_hp.GetMaxValue(); }
	int32 GetSp() { return m_sp; }
	int32 GetMaxSp() { return m_sp.GetMaxValue(); }
	void ModifyHp( int32 nValue );
	void ModifySp( int32 nValue );
	int32 GetMoney() { return m_nMoney; }
	void AddMoney( int32 nMoney ) { m_nMoney += nMoney; }
	CEntity* GetCore() { return m_pCore; }
	bool IsHiding() { return m_fHidingCurTime >= m_fHidingTime; }
	float GetHidingPercent() { return m_fHidingCurTime / m_fHidingTime; }
	float GetRepairPercent() { return ( m_nRepairTime - m_nRepairTimeLeft ) * 1.0f / m_nRepairTime; }

	void Move( float fXAxis, float fYAxis )
	{
		m_fMoveXAxis = fXAxis;
		m_fMoveYAxis = fYAxis;
	}
	void AimAt( const CVector2& pos );
	void Roll() { m_bRoll = true; }
	const CVector2& GetAimAt() { return m_aimAt; }
	const CVector2& GetCam() { return m_cam; }
	void DelayChangeStage( const char* szName, const char* szStartPoint = "" );

	bool IsFiring() { return m_bFiringDown; }
	bool IsRolling();
	bool CanBeHit() { return m_fHurtInvincibleTime <= 0 &&
		( m_bIsWalkOrFly ? m_walkData.nState != SCharacterWalkData::eState_Rolling : m_flyData.nState != SCharacterFlyData::eState_Rolling ); }
	float GetInvicibleTimeLeft() { return m_fHurtInvincibleTime; }
	bool CanKnockback() { return m_fKnockbackInvincibleTime <= 0; }
	virtual void Damage( int32 nValue ) override;
	void RestoreHp( int32 nValue );
	void CostSp( int32 nValue );
	void RestoreSp( int32 nValue );
	virtual void Crush() override;
	virtual bool Knockback( const CVector2& vec ) override;
	CVector2 GetKnockback();
	void BeginFire();
	void EndFire();
	void BeginRepair();
	void EndRepair();

	void AddItem( CItem* pItem );
	void RemoveItem( CItem* pItem );

	CChunkObject* GetCurRoom() { return m_pCurRoom; }
	CPlayerWeapon* GetWeapon() { return m_pCurWeapon; }
	void SetWeapon( CPlayerWeapon* pWeapon );
	void RefreshCurRoomUI() { m_pCurRoomChunkUI->SetChunkObject( m_pCurRoom ); }

	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	virtual void OnAddedToStage() override;

	virtual bool CanTriggerItem() override;
private:
	void UpdateMove();
	void UpdateFiring();
	void UpdateRoom();
	void UpdateRepair();

	SAttribute m_hp;
	SAttribute m_sp;
	int32 m_nSpRegenPerFrame;
	int32 m_nSpRegenPerFrameSlidingDown;
	int32 m_nRollSpCost;
	int32 m_nMoney;

	CReference<CEntity> m_pCore;
	CReference<CPlayerWeapon> m_pCurWeapon;
	CReference<CChunkObject> m_pCurRoom;
	CReference<CChunkUI> m_pCurRoomChunkUI;

	float m_fCrackEffectTime;
	float m_fHurtInvincibleTime;
	float m_fKnockbackInvincibleTime;
	float m_fMoveXAxis, m_fMoveYAxis;

	SCharacterWalkData m_walkData;
	SCharacterFlyData m_flyData;
	bool m_bIsWalkOrFly;
	bool m_bRoll;
	bool m_bFiringDown;
	bool m_bIsRepairing;
	bool m_bCachedJump;
	float m_fCachedJumpTime;

	float m_fHidingTime;
	float m_fHidingCurTime;

	uint32 m_nRepairTime;
	uint32 m_nRepairInterval;
	uint32 m_nRepairShake;
	uint32 m_nRepairHp;
	uint32 m_nRepairTimeLeft;
	uint32 m_nRepairIntervalLeft;

	CVector2 m_aimAt;
	CVector2 m_cam;
	
	string m_strChangeStage;
	string m_strChangeStageStartPoint;
	float m_fChangeStageTime;

	uint8 m_nAnimState;

	CPostProcessCrackEffect m_crackEffect;

	map<CString, CItem*> m_mapKeyItems;
	LINK_LIST_REF_HEAD( m_pItems, CItem, Item )
};