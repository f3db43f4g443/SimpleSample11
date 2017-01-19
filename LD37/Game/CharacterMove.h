#pragma once
#include "Character.h"

struct SCharacterMovementData
{
	SCharacterMovementData() : bSleep( false ) { memset( bHitChannel, 0, sizeof( bHitChannel ) ); bHitChannel[eEntityHitType_WorldStatic] = bHitChannel[eEntityHitType_Platform] = true; }
	void TryMove( CCharacter* pCharacter, const CVector2& ofs, SRaycastResult* pHit = NULL );
	void TryMove( CCharacter* pCharacter, const CVector2& ofs, CVector2& velocity, SRaycastResult* pHit = NULL );
	bool TryMoveAcrossWall( CCharacter* pCharacter, const CVector2& ofs, const CVector2& ofsLeft );

	bool ResolvePenetration( CCharacter* pCharacter );

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

	void Reset()
	{
		nState = eState_Normal;
		bRollingAcrossWall = false;
		pLandedEntity = NULL;
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
};

struct SCharacterSimpleWalkData : public SCharacterMovementData
{
	SCharacterSimpleWalkData( const SClassCreateContext& context ) { Reset(); }

	CVector2 UpdateMove( CCharacter* pCharacter, int8 nDir, bool bJump );

	float fMoveSpeed;
	float fGravity;
	float fMaxFallSpeed;
	float fJumpSpeed;

	void Reset()
	{
		fFallSpeed = 0;
		bLanded = false;
	}

	SRaycastResult hits[3];
	float fFallSpeed;
	bool bLanded;
};

struct SCharacterWalkData : public SCharacterMovementData
{
	SCharacterWalkData( const SClassCreateContext& context ) { Reset(); }

	void UpdateMove( CCharacter* pCharacter, const CVector2& moveAxis );

	void Jump( CCharacter* pCharacter );
	void ReleaseJump( CCharacter* pCharacter );
	void Roll( CCharacter* pCharacter, const CVector2& moveAxis );

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
		pLandedEntity = NULL;
	}
	uint8 nState;
	bool bRollingAcrossWall;
	CVector2 velocity;
	float fJumpHoldingTime;
	float fRollTime;
	CVector2 rollDir;
	uint8 nRollCount;

	CReference<CEntity> pLandedEntity;
	CMatrix2D lastLandedEntityTransform;
	CVector2 groundNorm;
protected:
	void HandleNormal( CCharacter* pCharacter, const CVector2& moveAxis );
	void HandleRoll( CCharacter* pCharacter, const CVector2& moveAxis );

	void FallOff();
	void OnLandedEntityMoved( CCharacter* pCharacter, const CMatrix2D& oldTrans, const CMatrix2D& newTrans );
	void FindFloor( CCharacter* pCharacter );
};