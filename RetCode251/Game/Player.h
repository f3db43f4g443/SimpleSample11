#pragma once
#include "Character.h"
#include "CharacterMove.h"
#include "Interfaces.h"

class CPlayerCross : public CCharacter
{
	friend void RegisterGameClasses_Player();
public:
	CPlayerCross( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CPlayerCross ); }

	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	virtual void PostUpdate() override;
	CCharacter* GetCurLockedTarget() { return m_pCurLockedTarget; }
	bool IsTryingToAttach() { return m_bTryingToAttach; }

	void OnAttachBegin() { m_bControlled = true; }
	void OnAttachEnd() { m_bControlled = false; }
private:
	float m_fMoveSpeed;

	bool m_bControlled;
	bool m_bTryingToAttach;
	CReference<CCharacter> m_pCurLockedTarget;
};

class CPlayer0 : public CCharacter
{
	friend void RegisterGameClasses_Player();
public:
	CPlayer0( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CPlayer0 ); }

	virtual int8 CheckPush( SRaycastResult& hit, const CVector2& dir, float& fDist, SPush& context, int32 nPusher ) override;
	virtual void HandlePush( const CVector2& dir, float fDist, int8 nStep ) override;
	bool CounterBullet( CEntity* p, const CVector2& hitPos, const CVector2& hitDir, bool bHitPlayer );

	void UpdateInput();
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	virtual void PostUpdate() override;
	int8 GetDir() { return m_nDir; }
	enum
	{
		eState_Stand,
		eState_Walk,
		eState_Jump,
		eState_Kick_Ready,
		eState_Kick,
	};
	enum
	{
		eAni_Kick_A,
		eAni_Kick_B,
		eAni_Kick_C,
		eAni_Kick_A1,
		eAni_Kick_B1,
		eAni_Kick_C1,
		eAni_Kick_A2,
		eAni_Kick_B2,
		eAni_Kick_C2,
		eAni_Kick_Base_End,

		eAni_Kick_Finish_Begin = eAni_Kick_Base_End,
		eAni_Kick_Rev = eAni_Kick_Finish_Begin,
		eAni_Kick_Stomp,
		eAni_Kick_Finish_End,

		eAni_Kick_Special_Begin = eAni_Kick_Finish_End,
		eAni_Kick_Spin = eAni_Kick_Special_Begin,

		eAni_Kick_Recover,
		eAni_Kick_Recover_1,
	};
	void OnKickFirstHit( CEntity* pKick );
	void OnKickFirstExtentHit( CEntity* pKick );

	CVector2 GetLastLandPoint() { return m_lastLandPoint; }
	virtual bool Knockback( const CVector2& vec ) override;

	virtual bool CanBeControlled() override { return true; }
	void BeginControl() { m_bAttached = true; }
	void EndControl() { m_bAttached = false; }
protected:
	virtual bool CanHitPlatform() { return true; }
	virtual bool CanShoot() { return m_nFireCD > 0; }
	virtual bool CanFindFloor();
	virtual int8 GetCurImpactLevel() { return 0; }
	void FindFloor( const CVector2& gravityDir );
	void UpdateAni( const CVector2& gravityDir );
	void UpdateEffect();
	void UpdateAnimState( const CVector2& gravityDir );

	void JumpCancel( const CVector2& gravityDir );
	void Kick( int8 nAni );
	void KickBreak();
	void KickMorph( CEntity* pEntity );

	enum
	{
		eKickLevel_None,
		eKickLevel_Basic,
		eKickLevel_Combo,
		eKickLevel_Heavy,
	};
	float m_fMoveSpeed;
	float m_fAirMaxSpeed;
	float m_fJumpSpeed;
	float m_fMoveAcc;
	float m_fStopAcc;
	float m_fAirAcc;
	float m_fGravity;
	float m_fMaxFallSpeed;
	float m_fFindFloorDist;
	int8 m_nLandTime;
	int8 m_nJumpHoldTime;
	int8 m_nKickLevel;
	int32 m_nWalkAnimSpeed;
	float m_fJumpAnimVel[3];
	CVector2 m_kickVel[11];
	CVector3 m_kickOffset[11];
	CVector2 m_kickSpinSlideVel;
	float m_fKickDashSpeed[4];
	float m_nKickSpinMaxSpeed;
	float m_nKickSpinDashSpeed;
	int32 m_nKickReadyTime;
	int32 m_nKickAnimFrame1[3];
	int32 m_nKickAnimFrame2[3];
	int32 m_nKickSpinAnimSpeed;
	int32 m_nKickSpinAnimRep;
	CVector4 m_kickRevAnimFrame;
	CVector2 m_kickStompAnimFrame;
	int32 m_nKickDashDelay;
	int32 m_nKickDashTime;
	int32 m_nKickSpinDashTime[4];
	int32 m_nKickSpinDashAnimSpeed;
	int32 m_nKickRecoverTime;
	int32 m_nFireCD;
	TArray<CVector2> m_arrBulletOfs;
	TResourceRef<CPrefab> m_pKick[4];
	TResourceRef<CPrefab> m_pKickSpin[4];
	TResourceRef<CPrefab> m_pBullet;

	bool m_bAttached;
	int8 m_nCurState;
	int8 m_nDir;
	union
	{
		struct
		{
			int16 nTick;
			int16 nTick1;
			int8 nAni;
			uint8 nType0 : 2;
			uint8 nTap : 2;
			uint8 bHit : 1;
			uint8 nFlag : 2;
			uint8 nFlag1 : 1;
		} m_kickState;
		struct
		{
			int32 m_nStateTick;
			int32 m_nStateTickEx;
		};
	};
	int8 m_nMoveState;
	int8 m_nJumpState;
	int8 m_nBufferedInput;
	bool m_bKnockback;
	int32 m_nKnockBackTime;
	int32 m_nFireCDLeft;
	CReference<CEntity> m_pCurAttackEffect;
	CVector2 m_lastFramePos;
	CVector2 m_vel;
	CMatrix2D lastLandedEntityTransform;
	CVector2 m_groundNorm;
	CReference<CCharacter> m_pLanded;
	CVector2 m_lastLandPoint;
	int32 m_nLastLandTime;
	SCharacterMovementData m_moveData;
};