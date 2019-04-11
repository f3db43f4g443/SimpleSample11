#pragma once
#include "Character.h"

class CCharacterMoveUtil
{
public:
	static float Stretch( CCharacter* pCharacter, uint8 nDir, float fMaxDeltaLen, bool bHitChannel[eEntityHitType_Count] = NULL );
	static float StretchEx( CCharacter* pCharacter, uint8 nDir, float fMinLen, float fMaxLen, float fMoveDist, bool bHitChannel[eEntityHitType_Count] = NULL );
};

struct SCharacterMovementData
{
	SCharacterMovementData() : bSleep( false ) { memset( bHitChannel, 0, sizeof( bHitChannel ) ); bHitChannel[eEntityHitType_WorldStatic] = bHitChannel[eEntityHitType_Platform] = bHitChannel[eEntityHitType_System] = true; }
	void TryMove( CCharacter* pCharacter, const CVector2& ofs, SRaycastResult* pHit = NULL );
	void TryMove( CCharacter* pCharacter, const CVector2& ofs, CVector2& velocity, SRaycastResult* pHit = NULL );

	bool ResolvePenetration( CCharacter* pCharacter );
	bool ResolvePenetration( CCharacter* pCharacter, const CVector2& dir, float fCos );
	bool HasAnyCollision();

	CEntity* DoSweepTest( CCharacter* pChar, const CMatrix2D& trans, const CVector2& sweepOfs, SRaycastResult* pResult = NULL, bool bIgnoreInverseNormal = false );

	bool bSleep;
	bool bHitChannel[eEntityHitType_Count];
};

struct SCharacterFlyData : public SCharacterMovementData
{
	SCharacterFlyData( const SClassCreateContext& context ) : bApplyExtraGravity( false ) { Reset(); }

	void UpdateMove( CCharacter* pCharacter, const CVector2& moveAxis, float fStopBaseSpd2 = FLT_MAX );

	void Roll( CCharacter* pCharacter, const CVector2& moveAxis );
	void Hooked( const CVector2& vel )
	{
		vecKnockback = vel;
		nState = eState_Hooked;
	}

	void SetLandedEntity( CEntity* pEntity )
	{
		pLandedEntity = pEntity;
		if( pEntity )
			lastLandedEntityTransform = pEntity->globalTransform;
	}

	float fMoveSpeed;
	float fRollMaxTime;
	float fRollMaxSpeed;

	void Knockback( float fTime, const CVector2& dir )
	{
		fKnockbackTime = fTime;
		vecKnockback = dir;
		nState = eState_Knockback;
	}

	void Reset()
	{
		nState = eState_Normal;
		pLandedEntity = NULL;
		fKnockbackTime = false;
		vecKnockback = CVector2( 0, 0 );
	}

	enum
	{
		eState_Normal,
		eState_Rolling,
		eState_Knockback,
		eState_Hooked,
	};

	uint8 nState;
	bool bApplyExtraGravity;
	float fRollTime;
	CVector2 rollDir;
	CVector2 finalMoveAxis;
	CReference<CEntity> pLandedEntity;
	CMatrix2D lastLandedEntityTransform;
	SRaycastResult hits[3];

	float fKnockbackTime;
	CVector2 vecKnockback;
};

struct SCharacterSimpleWalkData : public SCharacterMovementData
{
	SCharacterSimpleWalkData( const SClassCreateContext& context ) { Reset(); }

	CVector2 UpdateMove( CCharacter* pCharacter, const CVector2& extraVelocity, float fDir, bool bJump );

	float fMoveSpeed;
	float fGravity;
	float fMaxFallSpeed;
	float fJumpSpeed;

	void Knockback( float fTime, const CVector2& dir )
	{
		fKnockbackTime = fTime;
		vecKnockback = dir;
	}

	void Reset()
	{
		fFallSpeed = 0;
		bLanded = false;
		fKnockbackTime = false;
		vecKnockback = CVector2( 0, 0 );
	}

	SRaycastResult hits[3];
	float fFallSpeed;
	bool bLanded;

	float fKnockbackTime;
	CVector2 vecKnockback;
};

struct SCharacterWalkData : public SCharacterMovementData
{
	SCharacterWalkData( const SClassCreateContext& context ) { Reset(); }

	void UpdateMove( CCharacter* pCharacter, const CVector2& moveAxis );

	void Jump( CCharacter* pCharacter );
	void ReleaseJump( CCharacter* pCharacter );
	void ReleaseJump( CCharacter* pCharacter, float fTime ) { fJumpHoldingTime = Min( fTime, fJumpMaxHoldTime ); ReleaseJump( pCharacter ); }
	void ReleaseCachedJump( CCharacter* pCharacter, float fTime );
	bool Roll( CCharacter* pCharacter, const CVector2& moveAxis );
	void Hooked( const CVector2& vel )
	{
		velocity = vel;
		nState = eState_Hooked;
		fJumpHoldingTime = 0;
		nIsSlidingDownWall = 0;
		pLandedEntity = NULL;
	}

	void Knockback( float fTime, const CVector2& dir )
	{
		fKnockbackTime = fTime;
		vecKnockback = dir;
		velocity = dir;
		nState = eState_Knockback;
		fJumpHoldingTime = 0;
		nIsSlidingDownWall = 0;
	}

	float fMoveSpeed;
	float fMoveAcc;
	float fStopAcc;
	CVector2 gravity;
	float fMaxFallSpeed;
	float fMaxLandFallSpeed;
	float fJumpMaxSpeed;
	float fJumpMaxHoldTime;
	float fAirAcc;
	float fAirMaxSpeed;
	float fRollMaxTime;
	float fRollMaxSpeed;
	float fFindFloorDist;
	float fSlideDownSpeed;

	enum
	{
		eState_Normal,
		eState_JumpHolding,
		eState_Rolling,
		eState_Knockback,
		eState_Hooked,
	};
	void Reset()
	{
		velocity = CVector2( 0, 0 );
		nState = eState_Normal;
		fJumpHoldingTime = 0;
		nIsSlidingDownWall = 0;
		pLandedEntity = NULL;
		fKnockbackTime = 0;
		vecKnockback = CVector2( 0, 0 );
	}
	uint8 nState;
	int8 nIsSlidingDownWall;
	int8 nJumpCheck;
	CVector2 velocity;
	float fJumpHoldingTime;
	float fRollTime;
	CVector2 rollDir;

	CReference<CEntity> pLandedEntity;
	CMatrix2D lastLandedEntityTransform;
	CVector2 groundNorm;

	float fKnockbackTime;
	CVector2 vecKnockback;
protected:
	void HandleNormal( CCharacter* pCharacter, const CVector2& moveAxis );
	void HandleRoll( CCharacter* pCharacter, const CVector2& moveAxis );

	void FallOff();
	CVector2 OnLandedEntityMoved( CCharacter* pCharacter, const CMatrix2D& oldTrans, const CMatrix2D& newTrans );
	void FindFloor( CCharacter* pCharacter );
};

struct SCharacterPhysicsFlyData : public SCharacterMovementData
{
	SCharacterPhysicsFlyData( const SClassCreateContext& context ) : bHit( false ){ bHitChannel[eEntityHitType_WorldStatic] = bHitChannel[eEntityHitType_Platform] = bHitChannel[eEntityHitType_System] = false; }
	void UpdateMove( CCharacter* pCharacter, const CVector2& moveTarget );

	float fMaxAcc;
	float fStablity;
	bool bHit;
	CVector2 dVelocity;
};

struct SCharacterPhysicsFlyData1 : public SCharacterMovementData
{
	SCharacterPhysicsFlyData1( const SClassCreateContext& context ) : bHit( false ) {}
	void UpdateMove( CCharacter* pCharacter, CVector2 acc = CVector2( 0, 0 ) );

	float fMaxAcc;
	bool bHit;
	CVector2 dVelocity;

	SRaycastResult hits[3];
};

struct SCharacterCreepData : public SCharacterMovementData
{
	SCharacterCreepData( const SClassCreateContext& context ) : bHitWall( true ), fKnockbackTime( 0 ) {}
	void UpdateMove( CCharacter* pCharacter, int8 nTurnDir );

	void Knockback( float fTime, const CVector2& dir )
	{
		fKnockbackTime = fTime;
		vecKnockback = dir;
	}

	float fSpeed;
	float fTurnSpeed;
	float fFallGravity;
	float fMaxFallSpeed;

	bool bHitWall;

	float fKnockbackTime;
	CVector2 vecKnockback;
};

struct SCharacterSurfaceWalkData : public SCharacterMovementData
{
	SCharacterSurfaceWalkData( const SClassCreateContext& context ) : bHitSurface( false ) {}
	void UpdateMove( CCharacter* pCharacter, int8 nDir );
	void Fall( CCharacter* pCharacter, const CVector2& vel );

	float fSpeed;
	float fFallInitSpeed;
	float fGravity;
	float fMaxFallSpeed;

	bool bHitSurface;
	CVector2 normal;
};

struct SCharacterPhysicsMovementData : public SCharacterMovementData
{
	SCharacterPhysicsMovementData( const SClassCreateContext& context ) : fRot( 0 ) {}
	void UpdateMove( CCharacter* pCharacter );

	float fGravity;
	float fMaxFallSpeed;
	float fFriction;
	float fBounceCoef;
	float fBounceCoef1;
	float fRotCoef;
	float fRot;
};

struct SCharacterVehicleMovementData : public SCharacterMovementData
{
	SCharacterVehicleMovementData( const SClassCreateContext& context ) : bHitWall( false ), fDamage( 0 ) {}
	void UpdateMove( CCharacter* pCharacter );

	float fBounce;
	float fCrushSpeed;
	float fFallGravity;
	float fMaxFallSpeed;

	bool bHitWall;
	bool bHitWall1;
	float fDamage;
};

struct SCharacterChainMovementData
{
	SCharacterChainMovementData() : beginDir( 0, 0 ), endDir( 0, 0 ), fDamping( 1 ) {}
	SCharacterChainMovementData( const SClassCreateContext& context ) {}

	void SetCharacterCount( uint32 nCount );
	void Simulate( float fTime, uint32 nSteps, CCharacter** pCharacters, uint32 nCharacters );

	vector<CVector2> vecPos;
	vector<CVector2> vecVel;
	vector<CVector2> vecAcc;
	vector<CVector2> vecDir;

	vector<float> vecLen;
	vector<float> vecK;
	vector<float> vecAngleLim;
	vector<float> vecK1;
	vector<float> vecInvWeight;
	vector<CVector2> vecExtraAcc;
	CVector2 beginDir;
	CVector2 endDir;
	float fDamping;
};

struct SCharacterQueueMovementData
{
	SCharacterQueueMovementData() : fSpeed( 1 ) {}

	void Setup( CCharacter** pCharacters, uint32 nCharacters );
	void UpdateMove( CCharacter** pCharacters, uint32 nCharacters );
	vector<CVector2> waypoints;
	vector<float> angles;
	int32 nWaypointBegin;
	float fPercent;

	float fSpeed;
};