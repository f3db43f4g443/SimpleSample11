#pragma once
#include "Character.h"

struct SCharacterMovementData
{
	SCharacterMovementData() : bSleep( false ) { memset( bHitChannel, 0, sizeof( bHitChannel ) ); bHitChannel[eEntityHitType_WorldStatic] = bHitChannel[eEntityHitType_Platform] = true; }
	void TryMove( CCharacter* pCharacter, const CVector2& ofs, SRaycastResult* pHit = NULL );
	void TryMove( CCharacter* pCharacter, const CVector2& ofs, CVector2& velocity, SRaycastResult* pHit = NULL );

	bool ResolvePenetration( CCharacter* pCharacter );
	bool HasAnyCollision();

	bool bSleep;
	bool bHitChannel[eEntityHitType_Count];
};

struct SCharacterFlyData : public SCharacterMovementData
{
	SCharacterFlyData( const SClassCreateContext& context ) { Reset(); }

	void UpdateMove( CCharacter* pCharacter, const CVector2& moveAxis );
	void UpdateMoveNoBlocking( CCharacter* pCharacter, const CVector2& moveAxis );

	void Roll( CCharacter* pCharacter, const CVector2& moveAxis );

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
		nState = eState_Normal;
		bRollingAcrossWall = false;
	}

	void Reset()
	{
		nState = eState_Normal;
		bRollingAcrossWall = false;
		pLandedEntity = NULL;
		fKnockbackTime = false;
		vecKnockback = CVector2( 0, 0 );
	}

	enum
	{
		eState_Normal,
		eState_Rolling,
	};

	uint8 nState;
	bool bRollingAcrossWall;
	float fRollTime;
	CVector2 rollDir;
	CVector2 finalMoveAxis;
	CReference<CEntity> pLandedEntity;
	CMatrix2D lastLandedEntityTransform;

	float fKnockbackTime;
	CVector2 vecKnockback;
};

struct SCharacterSimpleWalkData : public SCharacterMovementData
{
	SCharacterSimpleWalkData( const SClassCreateContext& context ) { Reset(); }

	CVector2 UpdateMove( CCharacter* pCharacter, int8 nDir, bool bJump );

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
	void Roll( CCharacter* pCharacter, const CVector2& moveAxis );

	void Knockback( float fTime, const CVector2& dir )
	{
		fKnockbackTime = fTime;
		vecKnockback = dir;
		velocity = dir;
		nState = eState_Normal;
		fJumpHoldingTime = 0;
		bRollingAcrossWall = false;
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
	};
	void Reset()
	{
		velocity = CVector2( 0, 0 );
		nState = eState_Normal;
		fJumpHoldingTime = 0;
		bRollingAcrossWall = false;
		nIsSlidingDownWall = 0;
		pLandedEntity = NULL;
		fKnockbackTime = 0;
		vecKnockback = CVector2( 0, 0 );
	}
	uint8 nState;
	bool bRollingAcrossWall;
	uint8 nRollCount;
	int8 nIsSlidingDownWall;
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
	void OnLandedEntityMoved( CCharacter* pCharacter, const CMatrix2D& oldTrans, const CMatrix2D& newTrans );
	void FindFloor( CCharacter* pCharacter );
};

struct SCharacterPhysicsFlyData : public SCharacterMovementData
{
	SCharacterPhysicsFlyData( const SClassCreateContext& context ) : bHit( false ){ bHitChannel[eEntityHitType_WorldStatic] = bHitChannel[eEntityHitType_Platform] = false; }
	void UpdateMove( CCharacter* pCharacter, const CVector2& moveTarget );

	float fMaxAcc;
	float fStablity;
	bool bHit;
};

struct SCharacterCreepData : public SCharacterMovementData
{
	SCharacterCreepData( const SClassCreateContext& context ) : bHitWall( false ) {}
	void UpdateMove( CCharacter* pCharacter, int8 nTurnDir );

	float fSpeed;
	float fTurnSpeed;
	float fFallGravity;
	float fMaxFallSpeed;

	bool bHitWall;
};

struct SCharacterSurfaceWalkData : public SCharacterMovementData
{
	SCharacterSurfaceWalkData( const SClassCreateContext& context ) {}
	void UpdateMove( CCharacter* pCharacter, int8 nDir );
	void Fall( CCharacter* pCharacter, const CVector2& vel );

	float fSpeed;
	float fFallInitSpeed;
	float fGravity;
	float fMaxFallSpeed;

	bool bHitSurface;
	CVector2 normal;
};