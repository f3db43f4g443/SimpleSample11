#pragma once
#include "Attribute.h"
#include "Character.h"
#include "CharacterMove.h"
#include "Interfaces.h"

enum
{
	ePlayerLevel_0,
	ePlayerLevel_Slide,
	ePlayerLevel_Kick,
	ePlayerLevel_Shoot,
	ePlayerLevel_Test_Scan,
	
	ePlayerLevel_VehicleMode,
	ePlayerLevel_CounterBullet,
	ePlayerLevel_Grab,
	ePlayerLevel_Slide_Air,
	ePlayerLevel_Glide,
	ePlayerLevel_Kick_Combo,
	ePlayerLevel_Kick_Rev,
	ePlayerLevel_Kick_Stomp,
	ePlayerLevel_Bomb,
	ePlayerLevel_Shield,
	ePlayerLevel_Glide_Fall_Block,
	ePlayerLevel_Kick_Mid,
	ePlayerLevel_Kick_Heavy,
	ePlayerLevel_Test_Scan_1,
	ePlayerLevel_Max,
};

class CPlayer : public CCharacter
{
	friend void RegisterGameClasses();
public:
	CPlayer( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	int32 GetPlayerLevel() { return m_nLevel; }
	void SetLevel( int32 nLevel ) { m_nLevel = nLevel; }
	CReference<CEntity>* GetAllHits() { return m_pHit; }

	virtual int8 CheckPush( SRaycastResult& hit, const CVector2& dir, float& fDist, SPush& context, int32 nPusher ) override;
	virtual void HandlePush( const CVector2& dir, float fDist, int8 nStep ) override;
	bool CounterBullet( CEntity* p, const CVector2& hitPos, const CVector2& hitDir, bool bHitPlayer );

	void UpdateInput();
	virtual bool Damage( SDamageContext& context ) override;
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;
	void PostUpdate();
	virtual bool CheckImpact( CEntity* pEntity, SRaycastResult& result, bool bCast ) override;
	int32 GetDir() { return m_nDir; }
	enum
	{
		eState_Stand,
		eState_Walk,
		eState_Jump,
		eState_Kick_Ready,
		eState_Kick,
		eState_Slide,
		eState_Slide_Air,
		eState_Slide_1,
		eState_Stand_1,
		eState_Stand_1_Ready,
		eState_Walk_1,
		eState_Hop,
		eState_Hop_Down,
		eState_Glide_Fall,
		eState_Glide,
		eState_Boost,
		eState_Fly,
		eState_Hop_Flip,
		eState_Dash,
		eState_Grab,
		eState_Dash_Grab,
		eState_Grip,
		eState_Grab_Edge,
		eState_Punch,
		eState_Dash_Fall,
		eState_Roll,
		eState_Roll_Stand_Up,
		eState_Roll_Stop,
		eState_Roll_Recover,
		eState_Roll_Grab_Edge_Recover,
		eState_Roll_Grab_Edge_Recover1,
		eState_Roll_Dash,
		eState_BackFlip,
		eState_BackFlip_1,
		eState_BackFlip_2,
		eState_Force_Roll,
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
		eAni_Kick_Spin_Slide,

		eAni_Kick_Recover,
		eAni_Kick_Recover_1,
	};

	void OnKickFirstHit( CEntity* pKick );
	void OnKickFirstExtentHit( CEntity* pKick );

	CVector2 GetLastLandPoint() { return m_lastLandPoint; }
	virtual bool Knockback( const CVector2& vec ) override;
	float GetFallCritical() { return Min( 1.0f, Max( 0.0f, ( m_fFallHeight - m_fFallDmgBegin ) * m_fFallDmgPerHeight / m_nMaxHp ) ); }
	void UpdateImpactLevel();
protected:
	bool CanHitPlatform();
	bool CanShoot();
	bool CanFindFloor();
	bool CanRecoverShield();
	bool IsDodging();
	float GetGravity();
	bool IsFalling();
	bool IsUsingWheel();
	bool IsVehicleMode();
	void FindFloor( const CVector2& gravityDir );
	void UpdateHit();
	void UpdateAnim( const CVector2& gravityDir );
	void UpdateEffect();
	void UpdateAnimState( const CVector2& gravityDir );

	void JumpCancel( const CVector2& gravityDir );
	void Kick( int8 nAni );
	void KickBreak();
	void KickMorph( CEntity* pEntity );
	void BeginRoll();
	void Slide1();
	void SlideCancel();
	void BeginVehicleMode();
	void Glide();
	void Fly( int8 nFlyDir );
	void Hop();
	void HopDown();
	void Dash();
	void DashGrab();
	void CheckGrab();
	void GrabControl( const CVector2& gravityDir );
	void GrabDetach();
	bool TryGrabEdge( const CVector2& gravityDir, const CVector2& pos0, const CVector2& ofs0, CVector2 v0 );
	void GrabEdgeRecover( const CVector2 &gravityDir );
	void GrabEdgeRecover1( const CVector2 &gravityDir );
	void Punch();
	void PunchHit( const CVector2& gravityDir, SRaycastResult hit[3] );
	void DashFall();
	void RollStandUp();
	void RollStop();
	void RollRecover( int8 nDir );
	void RollDash( int8 nDir );
	void BackFlip();
	void BackFlip1();
	void BackFlip2();
	void ForceRoll( bool bUpdate = false );
	bool HandBufferInputStand1();

	float m_fMoveSpeed;
	float m_fAirMaxSpeed;
	float m_fJumpSpeed;
	float m_fMoveAcc;
	float m_fStopAcc;
	float m_fAirAcc;
	float m_fMove1Speed;
	float m_fMove1Acc;
	float m_fStop1Acc;
	float m_fGlideAcc;
	float m_fVehicleStartUpSpeed;
	float m_fVehicleMaxAcc;
	float m_fVehicleMaxPower;
	float m_fVehicleFrac;
	float m_fVehicleBreakAcc;
	CVector2 m_kickVel[11];
	CVector3 m_kickOffset[11];
	CVector2 m_kickSpinSlideVel;
	float m_fKickDashSpeed[4];
	float m_nKickSpinMaxSpeed;
	float m_nKickSpinDashSpeed;
	float m_fSlideSpeed0;
	float m_fSlideAcc;
	float m_fSlideSpeed;
	CVector2 m_glideVel;
	float m_fGlideVelTransfer;
	CVector2 m_boostAcc;
	float m_fBoostThresholdSpeed;
	float m_fBoostMinSpeed;
	float m_fFlySpeed;
	float m_fFlySpeed1;
	float m_fFlyFuelConsume;
	float m_fFlyFuelConsume1;
	float m_fDashSpeed;
	float m_fDashSpeedY;
	float m_fRollAcc;
	float m_fRollAcc1;
	float m_fForceRollSpeed;
	float m_fBackFlipSpeed;
	float m_fBackFlipSpeedY;
	float m_fBackFlipAcc;
	float m_fGrabAttachSpeed;
	CVector2 m_punchSpeed;
	float m_fPunchBounceSpeed;
	float m_fShieldJumpSpeed;
	int8 m_nLandTime;
	int8 m_nJumpHoldTime;
	float m_fGravity;
	float m_fGravity1;
	float m_fMaxFallSpeed;
	float m_fFindFloorDist;
	float m_fFallDmgBegin;
	float m_fFallDmgPerHeight;
	float m_fImpactLevelFallHeight[4];
	float m_fMaxWheelHeight;
	int32 m_nMaxPunchFrame;
	int32 m_nFireCD;
	int32 m_nBombCD;
	int32 m_nHpRecoverCD;
	int32 m_nHpRecoverInterval;
	int32 m_nMaxShield;
	int32 m_nShieldRecoverCD;
	int32 m_nShieldHitRecoverCD;
	CReference<CEntity> m_pGrabDetect;
	CReference<CRenderObject2D> m_pShieldEffect[4];
	TResourceRef<CPrefab> m_pKick[4];
	TResourceRef<CPrefab> m_pKickSpin[4];
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBomb;
	TArray<CVector2> m_arrBulletOfs;
	CVector2 m_bombOfs;
	int32 m_nWalkAnimSpeed;
	float m_fJumpAnimVel[3];
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
	int32 m_nSlideAnimSpeed;
	int32 m_nSlideMaxFrame;
	int32 m_nSlideDashTime;
	int32 m_nSlideAirCD;
	int32 m_nWalk1AnimSpeed;
	int32 m_nStand1ReadyTime;
	int32 m_nHopDownTime;
	int32 m_nHopFlipAnimSpeed;
	int32 m_nWheeledHitFallTime;
	int32 m_nGlideFallTime;
	int32 m_nDashGrabBeginFrame;
	int32 m_nGrabMaxTime;
	int32 m_nDashFallEndFrame;
	int32 m_nStandUpAnimSpeed;
	int32 m_nRollStopFrames;
	int32 m_nRollRecoverAnimSpeed;
	float m_fRollRotateSpeed;
	int32 m_nBackFlipAnimSpeed;
	int32 m_nBackFlip1AnimSpeed;
	int32 m_nBackFlip2AnimSpeed;
	int32 m_nBackFlip2Time;
	CReference<CEntity> m_pHit[3];
	float m_fHitOfs1, m_fHitOfs2;

	int8 m_nCurState;
	int8 m_nDir;
	union
	{
		struct
		{
			int8 nTick;
			int8 nTick1;
			int8 nAni;
			uint8 nType0 : 2;
			uint8 nTap : 2;
			uint8 bHit : 1;
			uint8 nFlag : 2;
			uint8 nFlag1 : 1;
		} m_kickState;
		struct
		{
			int16 nTick;
			int16 nTick1;
		} m_boostState;
		int32 m_nStateTick;
	};
	CVector2 m_lastFramePos;

	int8 m_nMoveState;
	int8 m_nJumpState;
	int8 m_nBufferedInput;
	int8 m_nHitAreaType;
	int32 m_nLevel;
	CReference<CEntity> m_pCurAttackEffect;
	int32 m_nSlideAirCDLeft;
	float m_fFallHeight;
	int32 m_nCurImpactLevel;
	bool m_bKnockback;
	int32 m_nKnockBackTime;
	int32 m_nFireCDLeft;
	int32 m_nBombCDLeft;
	int32 m_nDamageLeft;
	int32 m_nHpRecoverCDLeft;
	int32 m_nShield;
	int32 m_nShieldRecoverCDLeft;
	float m_fGlideBaseVel;
	float m_fFuel;
	CVector2 m_vel;
	CMatrix2D lastLandedEntityTransform;
	CVector2 m_groundNorm;
	CReference<CCharacter> m_pLanded;
	CVector2 m_lastLandPoint;
	int32 m_nLastLandTime;
	SCharacterMovementData m_moveData;
	CReference<CEntity> m_pGrabbed;
	SGrabDesc m_grabDesc;
};