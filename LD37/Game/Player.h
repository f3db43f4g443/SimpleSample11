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
	CEntity* GetCore() { return m_pCore; }
	bool IsHiding() { return m_bIsHiding; }
	float GetRepairPercent() { return ( m_nRepairTime - m_nRepairTimeLeft ) * 1.0f / m_nRepairTime; }

	void Move( float fXAxis, float fYAxis )
	{
		m_fMoveXAxis = fXAxis;
		m_fMoveYAxis = fYAxis;
	}
	void AimAt( const CVector2& pos ) { m_aimAt = pos; }
	void Roll() { m_bRoll = true; }
	const CVector2& GetAimAt() { return m_aimAt; }
	const CVector2& GetCam() { return m_cam; }
	void DelayChangeStage( const char* szName, const char* szStartPoint = "" );

	bool CanBeHit() { return m_fHurtInvincibleTime <= 0; }
	void Damage( int32 nValue = 1 );
	virtual void Crush() override { Damage( 1000 ); }
	void BeginFire();
	void EndFire();
	void BeginHide();
	void EndHide();

	CChunkObject* GetCurRoom() { return m_pCurRoom; }
	void SetWeapon( CPlayerWeapon* pWeapon );

	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	virtual void OnAddedToStage() override;

	virtual bool CanTriggerItem() override;
private:
	void UpdateMove();
	void UpdateFiring();
	void UpdateRepair();

	SAttribute m_hp;

	CReference<CEntity> m_pCore;
	CReference<CPlayerWeapon> m_pCurWeapon;
	CReference<CChunkObject> m_pCurRoom;
	CReference<CChunkUI> m_pCurRoomChunkUI;

	float m_fCrackEffectTime;
	float m_fHurtInvincibleTime;
	float m_fMoveXAxis, m_fMoveYAxis;

	SCharacterWalkData m_walkData;
	SCharacterFlyData m_flyData;
	bool m_bIsWalkOrFly;
	bool m_bRoll;
	bool m_bFiringDown;
	bool m_bIsHiding;

	uint32 m_nRepairTime;
	uint32 m_nRepairInterval;
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
};