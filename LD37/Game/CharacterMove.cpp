#include "stdafx.h"
#include "CharacterMove.h"
#include "Stage.h"
#include "GameUtil.h"
#include "Block.h"
#include "MyLevel.h"
#include "Common/MathUtil.h"

void SCharacterMovementData::TryMove( CCharacter * pCharacter, const CVector2& ofs, SRaycastResult* pHit )
{
	if( pHit )
		memset( pHit, 0, sizeof( SRaycastResult ) * 3 );

	SRaycastResult result;
	CVector2 moveOfs = ofs;
	if( moveOfs.Normalize() > 0 )
	{
		if( !DoSweepTest( pCharacter, pCharacter->globalTransform, ofs, &result, true ) )
		{
			pCharacter->SetPosition( pCharacter->GetPosition() + ofs );
		}
		else
		{
			if( pHit )
				pHit[0] = result;
			CVector2 ofs1 = moveOfs * result.fDist;
			CVector2 ofs2 = ofs - ofs1;
			ofs2 = ofs2 - result.normal * ( result.normal.Dot( ofs2 ) );
			CMatrix2D mat1 = pCharacter->globalTransform;
			mat1.SetPosition( mat1.GetPosition() + ofs1 );
			if( ofs2.Length2() > 0 )
			{
				SRaycastResult result1;
				if( DoSweepTest( pCharacter, mat1, ofs2, &result1, true ) )
				{
					if( pHit )
						pHit[1] = result1;
					CVector2 ofs3 = ofs2;
					ofs3.Normalize();
					ofs3 = ofs3 * result1.fDist;

					if( result.normal.Dot( result1.normal ) >= 0 )
					{
						CVector2 ofs4 = ofs2 - ofs3;
						ofs4 = ofs4 - result1.normal * ( result1.normal.Dot( ofs4 ) );

						CMatrix2D mat2 = mat1;
						mat2.SetPosition( mat2.GetPosition() + ofs3 );
						SRaycastResult result2;
						if( DoSweepTest( pCharacter, mat2, ofs4, &result2, true ) )
						{
							if( pHit )
								pHit[2] = result2;
							ofs4.Normalize();
							ofs4 = ofs4 * result2.fDist;
						}

						ofs2 = ofs3 + ofs4;
					}
					else
						ofs2 = ofs3;
				}
			}
			pCharacter->SetPosition( pCharacter->GetPosition() + ofs1 + ofs2 );
		}
	}
}

void SCharacterMovementData::TryMove( CCharacter * pCharacter, const CVector2& ofs, CVector2& velocity, SRaycastResult* pHit )
{
	if( pHit )
		memset( pHit, 0, sizeof( SRaycastResult ) * 3 );

	SRaycastResult result;
	CVector2 moveOfs = ofs;
	if( moveOfs.Normalize() > 0 )
	{
		if( !DoSweepTest( pCharacter, pCharacter->globalTransform, ofs, &result, true ) )
		{
			pCharacter->SetPosition( pCharacter->GetPosition() + ofs );
		}
		else
		{
			if( pHit )
				pHit[0] = result;
			velocity = velocity - result.normal * ( result.normal.Dot( velocity ) );
			CVector2 ofs1 = moveOfs * result.fDist;
			CVector2 ofs2 = ofs - ofs1;
			ofs2 = ofs2 - result.normal * ( result.normal.Dot( ofs2 ) );
			CMatrix2D mat1 = pCharacter->globalTransform;
			mat1.SetPosition( mat1.GetPosition() + ofs1 );
			if( ofs2.Length2() > 0 )
			{
				SRaycastResult result1;
				if( DoSweepTest( pCharacter, mat1, ofs2, &result1, true ) )
				{
					if( pHit )
						pHit[1] = result1;
					velocity = velocity - result1.normal * ( result1.normal.Dot( velocity ) );
					CVector2 ofs3 = ofs2;
					ofs3.Normalize();
					ofs3 = ofs3 * result1.fDist;

					if( result.normal.Dot( result1.normal ) >= 0 )
					{
						CVector2 ofs4 = ofs2 - ofs3;
						ofs4 = ofs4 - result1.normal * ( result1.normal.Dot( ofs4 ) );

						CMatrix2D mat2 = mat1;
						mat2.SetPosition( mat2.GetPosition() + ofs3 );
						SRaycastResult result2;
						if( DoSweepTest( pCharacter, mat2, ofs4, &result2, true ) )
						{
							if( pHit )
								pHit[2] = result2;
							velocity = velocity - result2.normal * ( result2.normal.Dot( velocity ) );
							ofs4.Normalize();
							ofs4 = ofs4 * result2.fDist;
							if( result.normal.Dot( result2.normal ) < 0 || result1.normal.Dot( result2.normal ) < 0 )
								velocity = CVector2( 0, 0 );
						}

						ofs2 = ofs3 + ofs4;
					}
					else
					{
						ofs2 = ofs3;
						velocity = CVector2( 0, 0 );
					}
				}
			}
			pCharacter->SetPosition( pCharacter->GetPosition() + ofs1 + ofs2 );
		}
	}
}

bool SCharacterMovementData::ResolvePenetration( CCharacter* pCharacter )
{
	struct SPenetration
	{
		CEntity* pEntity;
		CVector2 normal;
		float fNormalLength;
		float fInitialNormalLength;
	};

	vector<SPenetration> vecPenetrations;
	for( auto pManifold = pCharacter->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( bHitChannel[pEntity->GetHitType()] )
		{
			if( pCharacter->HasHitFilter() )
			{
				if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
					continue;
			}
			SPenetration penetration;
			penetration.pEntity = pEntity;
			CVector2 entityOfs = pEntity->globalTransform.GetPosition() - pEntity->GetLastPos();
			float fLength = entityOfs.Length();
			if( fLength == 0 )
			{
				entityOfs = pManifold->normal * -1;
				fLength = pManifold->normal.Length();
				if( fLength == 0 )
					continue;
			}

			CMatrix2D trans = pEntity->globalTransform;
			trans.SetPosition( trans.GetPosition() - entityOfs );
			SRaycastResult result;
			if( !pCharacter->SweepTest( pEntity->Get_HitProxy(), trans, entityOfs, &result ) )
				continue;

			penetration.fNormalLength = fLength - result.fDist;
			penetration.normal = entityOfs * ( penetration.fNormalLength / fLength * -1 );
			penetration.fInitialNormalLength = penetration.fNormalLength;
			vecPenetrations.push_back( penetration );
		}
	}
	if( !vecPenetrations.size() )
		return true;

	const CMatrix2D oldTransform = pCharacter->globalTransform;
	CMatrix2D newTransform = oldTransform;
	const CVector2 oldPos = oldTransform.GetPosition();
	CVector2 newPos = oldPos;

	bSleep = false;
	uint32 nPenetrations = vecPenetrations.size();
	while( nPenetrations )
	{
		uint32 nIndex;
		float maxLength = -1;
		for( int i = 0; i < nPenetrations; i++ )
		{
			if( vecPenetrations[i].fNormalLength > maxLength )
			{
				maxLength = vecPenetrations[i].fNormalLength;
				nIndex = i;
			}
		}

		SPenetration& curPenetration = vecPenetrations[nIndex];
		if( curPenetration.fNormalLength > curPenetration.fInitialNormalLength + 0.1f )
			return false;

		newPos = newPos - curPenetration.normal;
		if( nIndex != nPenetrations - 1 )
			curPenetration = vecPenetrations[nPenetrations - 1];
		nPenetrations--;
		newTransform.SetPosition( newPos );

		for( int i = nPenetrations - 1; i >= 0; i-- )
		{
			SHitTestResult result;
			if( pCharacter->HasHitFilter() )
			{
				if( !pCharacter->CanHit( vecPenetrations[i].pEntity ) || !vecPenetrations[i].pEntity->CanHit( pCharacter ) )
					continue;
			}
			if( !pCharacter->HitTest( vecPenetrations[i].pEntity, newTransform, vecPenetrations[i].pEntity->globalTransform, &result ) )
			{
				if( i != nPenetrations - 1 )
					vecPenetrations[i] = vecPenetrations[nPenetrations - 1];
				nPenetrations--;
				continue;
			}

			float fNewNormalLength = result.normal.Length();
			vecPenetrations[i].normal = result.normal;
			vecPenetrations[i].fNormalLength = fNewNormalLength;
		}
	}

	CMatrix2D trans = pCharacter->globalTransform;
	trans.SetPosition( newPos );
	vector<CHitProxy*> hitResult;
	vector<SHitTestResult> hitTestResult;
	pCharacter->GetStage()->GetHitTestMgr().HitTest( pCharacter->Get_HitProxy(), trans, hitResult, &hitTestResult );
	bool bHit = false;
	for( int i = 0; i < hitResult.size(); i++ )
	{
		auto pEntity = static_cast<CEntity*>( hitResult[i] );
		if( pEntity == pCharacter )
			continue;
		auto eHitType = static_cast<CEntity*>( hitResult[i] )->GetHitType();
		if( pCharacter->HasHitFilter() )
		{
			if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
				continue;
		}
		if( bHitChannel[eHitType] && hitTestResult[i].normal.Length2() > 0.05f * 0.05f )
		{
			bHit = true;
			break;
		}
	}
	if( bHit )
		return false;

	pCharacter->SetPosition( newPos );
	pCharacter->ForceUpdateTransform();
	return true;
}

bool SCharacterMovementData::ResolvePenetration( CCharacter * pCharacter, const CVector2 & dir, float fCos )
{
	float k = 0;
	bool bSucceed = true;
	struct SPenetration
	{
		CEntity* pEntity;
		CVector2 normal;
		float fNormalLength;
		float fInitialNormalLength;
	};

	vector<SPenetration> vecPenetrations;
	for( auto pManifold = pCharacter->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( bHitChannel[pEntity->GetHitType()] )
		{
			if( pCharacter->HasHitFilter() )
			{
				if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
					continue;
			}
			SPenetration penetration;
			penetration.pEntity = pEntity;
			CVector2 entityOfs = pEntity->globalTransform.GetPosition() - pEntity->GetLastPos();
			float fLength2 = entityOfs.Length2();
			if( fLength2 == 0 )
			{
				entityOfs = pManifold->normal * -1;
				fLength2 = pManifold->normal.Length2();
				if( fLength2 == 0 )
					continue;
			}
			float fDot = entityOfs.Dot( dir );
			if( fDot * fDot < fCos * fCos * fLength2 )
			{
				bSucceed = false;
				continue;
			}
			float fLength = fLength2 / fDot;
			entityOfs = dir * fLength;

			CMatrix2D trans = pEntity->globalTransform;
			trans.SetPosition( trans.GetPosition() - entityOfs );
			SRaycastResult result;
			if( !pCharacter->SweepTest( pEntity->Get_HitProxy(), trans, entityOfs, &result ) )
				continue;

			fLength = fLength - result.fDist;
			k = Max( k, fLength );
		}
	}

	if( k > 0 )
	{
		CVector2 newPos = pCharacter->GetPosition() + dir * k;
		pCharacter->SetPosition( newPos );
		if( !bSucceed )
			return false;
		CMatrix2D trans = pCharacter->globalTransform;
		trans.SetPosition( newPos );
		vector<CHitProxy*> hitResult;
		vector<SHitTestResult> hitTestResult;
		pCharacter->GetStage()->GetHitTestMgr().HitTest( pCharacter->Get_HitProxy(), trans, hitResult, &hitTestResult );
		bool bHit = false;
		for( int i = 0; i < hitResult.size(); i++ )
		{
			auto pEntity = static_cast<CEntity*>( hitResult[i] );
			auto eHitType = static_cast<CEntity*>( hitResult[i] )->GetHitType();
			if( pCharacter->HasHitFilter() )
			{
				if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
					continue;
			}
			if( bHitChannel[eHitType] && hitTestResult[i].normal.Length2() > 0.05f * 0.05f )
			{
				bHit = true;
				break;
			}
		}
		if( bHit )
			return false;
		pCharacter->ForceUpdateTransform();
	}
	return bSucceed;
}

bool SCharacterMovementData::HasAnyCollision()
{
	for( int i = 0; i < ELEM_COUNT( bHitChannel ); i++ )
	{
		if( bHitChannel[i] )
			return true;
	}
	return false;
}

CEntity * SCharacterMovementData::DoSweepTest( CCharacter * pChar, const CMatrix2D & trans, const CVector2 & sweepOfs, SRaycastResult * pResult, bool bIgnoreInverseNormal )
{
	return pChar->HasHitFilter() ?
		pChar->GetStage()->SweepTest( pChar, trans, sweepOfs, bHitChannel, pResult, bIgnoreInverseNormal ) :
		pChar->GetStage()->SweepTest( pChar->Get_HitProxy(), trans, sweepOfs, bHitChannel, pResult, bIgnoreInverseNormal );
}

void SCharacterFlyData::UpdateMove( CCharacter* pCharacter, const CVector2& moveAxis, float fStopBaseSpd2 )
{
	CVector2 landedEntityOfs( 0, 0 );
	if( pLandedEntity )
	{
		if( pLandedEntity->GetStage() != pCharacter->GetStage() )
		{
			pLandedEntity = NULL;
			bSleep = false;
		}
		else if( lastLandedEntityTransform != pLandedEntity->globalTransform )
		{
			CVector2 oldPos = pCharacter->globalTransform.GetPosition();
			CVector2 localPos = lastLandedEntityTransform.MulTVector2PosNoScale( oldPos );
			CVector2 newPos = pLandedEntity->globalTransform.MulVector2Pos( localPos );
			landedEntityOfs = newPos - oldPos;
			lastLandedEntityTransform = pLandedEntity->globalTransform;
			bSleep = false;
		}
	}

	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}
	pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );

	CVector2 baseVel( 0, 0 );
	if( pLandedEntity )
	{
		auto pChunk = SafeCast<CChunkObject>( pLandedEntity->GetParentEntity() );
		if( pChunk )
			baseVel = pChunk->GetSurfaceVel( pCharacter->GetPosition() );
	}
	CVector2 moveOfs;
	float fDeltaTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	if( nState == eState_Rolling )
	{
		float fTime = pCharacter->GetStage()->GetElapsedTimePerTick();
		float fNewRollTime = Min( fRollTime + fTime, fRollMaxTime );
		float fDist0 = ( 2 - fRollTime / fRollMaxTime ) * fRollTime * fRollMaxSpeed * 0.5f;
		float fDist1 = ( 2 - fNewRollTime / fRollMaxTime ) * fNewRollTime * fRollMaxSpeed * 0.5f;
		CVector2 rollOfsLeft = rollDir * ( fRollMaxTime * fRollMaxSpeed * 0.5f - fDist0 );
		CVector2 moveOfs = rollDir * ( fDist1 - fDist0 );
		fRollTime = fNewRollTime;

		if( bApplyExtraGravity )
		{
			float fExtraGravity = 2 * Max( 0.0f, pCharacter->globalTransform.GetPosition().y -
				( CMyLevel::GetInst()->GetBound().GetBottom() - CMyLevel::GetInst()->GetHighGravityHeight() ) );
			moveOfs = moveOfs + CVector2( 0, -fExtraGravity * fTime );
		}

		moveOfs = moveOfs + landedEntityOfs + baseVel * fDeltaTime;
		TryMove( pCharacter, moveOfs, hits );

		if( fRollTime >= fRollMaxTime )
		{
			Reset();
		}
	}
	else
	{
		if( moveAxis.Length2() > 0 || nState >= eState_Knockback || landedEntityOfs.Length2() > 0 )
		{
			if( nState == eState_Hooked )
			{
				moveOfs = ( vecKnockback + baseVel ) * fDeltaTime;
				nState = eState_Normal;
			}
			else
			{
				if( nState == eState_Knockback )
				{
					float fKnockbackTime0 = fKnockbackTime;
					fKnockbackTime = Max( 0.0f, fKnockbackTime - fDeltaTime );
					vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
					moveOfs = ( vecKnockback + baseVel ) * fDeltaTime;
					if( fKnockbackTime <= 0 )
						nState = eState_Normal;
				}
				else
				{
					if( baseVel.Length2() <= fStopBaseSpd2 )
						moveOfs = ( moveAxis * fMoveSpeed + baseVel ) * fDeltaTime;
					else
						moveOfs = baseVel * fDeltaTime;
				}

				if( bApplyExtraGravity )
				{
					float fExtraGravity = 2 * Max( 0.0f, pCharacter->globalTransform.GetPosition().y -
						( CMyLevel::GetInst()->GetBound().GetBottom() - CMyLevel::GetInst()->GetHighGravityHeight() ) );
					moveOfs = moveOfs + CVector2( 0, -fExtraGravity * fDeltaTime );
				}

				moveOfs = moveOfs + landedEntityOfs;
			}
			CVector2 velocity = moveOfs;
			TryMove( pCharacter, moveOfs, velocity, hits );
			finalMoveAxis = velocity;
			finalMoveAxis.Normalize();
		}
	}

	pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );
}

void SCharacterFlyData::Roll( CCharacter* pCharacter, const CVector2& moveAxis )
{
	if( nState != eState_Rolling && nState < eState_Knockback )
	{
		nState = eState_Rolling;
		fRollTime = 0;
		rollDir = moveAxis;
		return;
	}
}

void SCharacterWalkData::UpdateMove( CCharacter * pCharacter, const CVector2& moveAxis )
{
	if( nState == eState_Rolling )
		HandleRoll( pCharacter, moveAxis );
	else
		HandleNormal( pCharacter, moveAxis );

	pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );
}

void SCharacterWalkData::HandleNormal( CCharacter* pCharacter, const CVector2& moveAxis )
{
	int8 nDir;
	if( moveAxis.x > 0 )
		nDir = 1;
	else if( moveAxis.x < 0 )
		nDir = -1;
	else
		nDir = 0;

	const CMatrix2D oldTransform = pCharacter->GetGlobalTransform();
	CMatrix2D curTransform = oldTransform;
	const CVector2 oldPos = oldTransform.GetPosition();
	CVector2 curPos = oldPos;
	float fDeltaTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	if( nState == eState_Knockback )
	{
		float fKnockbackTime0 = fKnockbackTime;
		fKnockbackTime = Max( 0.0f, fKnockbackTime - fDeltaTime );
		nDir = 0;
		vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
		if( fKnockbackTime <= 0)
			nState = eState_Normal;
	}

	CVector2 landedEntityOfs( 0, 0 );
	if( pLandedEntity )
	{
		if( pLandedEntity->GetStage() != pCharacter->GetStage() )
		{
			pLandedEntity = NULL;
			FallOff();
			bSleep = false;
		}
		else if( lastLandedEntityTransform != pLandedEntity->globalTransform )
		{
			landedEntityOfs = OnLandedEntityMoved( pCharacter, lastLandedEntityTransform, pLandedEntity->globalTransform );
			bSleep = false;
		}
	}

	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}

	if( nDir )
	{
		bSleep = false;
	}


	if( bSleep )
	{

	}

	if( nState == eState_JumpHolding )
	{
		fJumpHoldingTime += pCharacter->GetStage()->GetElapsedTimePerTick();
		if( fJumpHoldingTime >= fJumpMaxHoldTime )
		{
			fJumpHoldingTime = fJumpMaxHoldTime;
			//if( pLandedEntity || nIsSlidingDownWall )
			ReleaseJump( pCharacter );
			//else
				//FallOff();
		}
	}

	CVector2 dVelocity = CVector2( 0, 0 );
	CVector2 dPos = velocity * fDeltaTime;
	if( nState != eState_Hooked )
	{
		CVector2 moveDir;
		CVector2 tangent;
		float fSpeedTarget = 0;
		if( pLandedEntity )
		{
			tangent = CVector2( -groundNorm.y, groundNorm.x );
			moveDir = tangent * ( tangent.x > 0 ? 1 : -1 ) * nDir;
			auto pChunk = SafeCast<CChunkObject>( pLandedEntity->GetParentEntity() );
			if( pChunk )
			{
				CVector2 baseVel = pChunk->GetSurfaceVel( pCharacter->GetPosition() );
				fSpeedTarget = baseVel.Dot( tangent );
			}
		}
		else
		{
			tangent = CVector2( -gravity.y, gravity.x );
			tangent.Normalize();
			moveDir = tangent * ( tangent.x > 0 ? 1 : -1 ) * nDir;
		}
		velocity = velocity;
		float k = tangent.Dot( moveDir );
		float fTangentVelocity = velocity.Dot( tangent );
		float fMaxSpeed = ( pLandedEntity ? fMoveSpeed : fAirMaxSpeed ) * k + fSpeedTarget;
		fSpeedTarget = fMoveSpeed * k + fSpeedTarget;
		if( !pLandedEntity )
		{
			if( k > 0 )
				fSpeedTarget = Min( fMaxSpeed, Max( fSpeedTarget, fTangentVelocity ) );
			else if( k < 0 )
				fSpeedTarget = Max( fMaxSpeed, Min( fSpeedTarget, fTangentVelocity ) );
		}
		bool bIsMoveOrStop = k == 0 ? false : fTangentVelocity * k >= 0 && fTangentVelocity * k < fSpeedTarget * k;

		float fAcc = pLandedEntity ? ( bIsMoveOrStop ? fMoveAcc : fStopAcc ) : fAirAcc;
		float fDeltaSpeed = fSpeedTarget - fTangentVelocity;
		if( fDeltaSpeed < 0 )
		{
			fDeltaSpeed = -fDeltaSpeed;
			tangent = tangent * -1;
		}
		float t0 = Min( fDeltaTime, fDeltaSpeed / fAcc );
		float t1 = fDeltaTime - t0;

		dVelocity = dVelocity + tangent * ( fAcc * t0 );
		dPos = dPos + tangent * ( fAcc * t0 * ( t0 * 0.5f + t1 ) );

		if( !pLandedEntity )
		{
			CVector2 gravityDir = gravity;
			fAcc = gravityDir.Normalize();
			float fNormalVelocity = velocity.Dot( gravityDir );
			fDeltaSpeed = Max( 0.0f, fMaxFallSpeed - fNormalVelocity );
			t0 = Min( fDeltaTime, fDeltaSpeed / fAcc );
			t1 = fDeltaTime - t0;
			dVelocity = dVelocity + gravityDir * ( fAcc * t0 );
			dPos = dPos + gravityDir * ( fAcc * t0 * t0 * 0.5f );
		}
		velocity = velocity + dVelocity;

		float fExtraGravity = 2 * Max( 0.0f, pCharacter->globalTransform.GetPosition().y -
			( CMyLevel::GetInst()->GetBound().GetBottom() - CMyLevel::GetInst()->GetHighGravityHeight() ) );
		dPos = dPos + CVector2( 0, -fExtraGravity * fDeltaTime );
		dPos = dPos + landedEntityOfs;
	}

	pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
	CVector2 v0 = velocity;
	SRaycastResult res[3];
	TryMove( pCharacter, dPos, velocity, res );
	if( nState != eState_Hooked )
	{
		bool bPreLandedEntity = pLandedEntity != NULL;
		FindFloor( pCharacter );

		nIsSlidingDownWall = 0;
		if( !pLandedEntity && nState < eState_Knockback )
		{
			if( moveAxis.x > 0 && v0.x > velocity.x )
			{
				for( int i = 0; i < 3; i++ )
				{
					auto& result = res[i];
					if( !result.pHitProxy )
						break;
					if( static_cast<CEntity*>( result.pHitProxy )->GetHitType() != eEntityHitType_WorldStatic )
						continue;
					if( result.normal.x < 0 )
					{
						nIsSlidingDownWall = 1;
						break;
					}
				}
			}
			else if( moveAxis.x < 0 && v0.x < velocity.x )
			{
				for( int i = 0; i < 3; i++ )
				{
					auto& result = res[i];
					if( !result.pHitProxy )
						break;
					if( static_cast<CEntity*>( result.pHitProxy )->GetHitType() != eEntityHitType_WorldStatic )
						continue;
					if( result.normal.x > 0 )
					{
						nIsSlidingDownWall = -1;
						break;
					}
				}
			}
			if( nIsSlidingDownWall )
			{
				if( moveAxis.y < 0 )
					velocity.y = Max( velocity.y, -fSlideDownSpeed * 2 );
				else if( moveAxis.y == 0 )
					velocity.y = Max( velocity.y, -fSlideDownSpeed );
				else
					velocity.y = Max( velocity.y, 0.0f );
			}
		}
		if( nState == eState_JumpHolding && !nJumpCheck )
		{
			if( nIsSlidingDownWall )
				nJumpCheck = 2 + nIsSlidingDownWall;
			else if( pLandedEntity )
				nJumpCheck = 2;
		}
	}
	else
		nState = eState_Normal;
}

void SCharacterWalkData::HandleRoll( CCharacter* pCharacter, const CVector2& moveAxis )
{
	float fTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	float fNewRollTime = Min( fRollTime + fTime, fRollMaxTime );
	float fDist0 = ( 2 - fRollTime / fRollMaxTime ) * fRollTime * fRollMaxSpeed * 0.5f;
	float fDist1 = ( 2 - fNewRollTime / fRollMaxTime ) * fNewRollTime * fRollMaxSpeed * 0.5f;
	CVector2 rollOfsLeft = rollDir * ( fRollMaxTime * fRollMaxSpeed * 0.5f - fDist0 );
	CVector2 moveOfs = rollDir * ( fDist1 - fDist0 );
	fRollTime = fNewRollTime;
	float fExtraGravity = 2 * Max( 0.0f, pCharacter->globalTransform.GetPosition().y -
		( CMyLevel::GetInst()->GetBound().GetBottom() - CMyLevel::GetInst()->GetHighGravityHeight() ) );
	moveOfs = moveOfs + CVector2( 0, -fExtraGravity * fTime );

	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}
	pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
	TryMove( pCharacter, moveOfs );

	if( fRollTime >= fRollMaxTime )
	{
		Reset();
	}
}

void SCharacterWalkData::Jump( CCharacter * pCharacter )
{
	if( nState == eState_Normal )
	{
		nState = eState_JumpHolding;
		if( nIsSlidingDownWall )
			nJumpCheck = 2 + nIsSlidingDownWall;
		else if( pLandedEntity )
			nJumpCheck = 2;
		else
			nJumpCheck = 0;
		fJumpHoldingTime = 0;
	}
}

void SCharacterWalkData::ReleaseJump( CCharacter * pCharacter )
{
	if( nState == eState_JumpHolding )
	{
		if( nJumpCheck )
		{
			CVector2 dVelocity;
			if( nJumpCheck > 2 )
			{
				dVelocity = CVector2( -0.5f, 0.732f );
				velocity.y = 0;
			}
			else if( nJumpCheck < 2 )
			{
				dVelocity = CVector2( 0.5f, 0.732f );
				velocity.y = 0;
			}
			else
				dVelocity = CVector2( 0, 1 );
			float t = fJumpHoldingTime < fJumpMaxHoldTime ? fJumpHoldingTime * 0.66f / fJumpMaxHoldTime : 1;
			dVelocity = dVelocity * ( sqrt( t ) * fJumpMaxSpeed );
			velocity.y = 0.0f;
			velocity = velocity + dVelocity;
			bSleep = false;
		}
		nState = eState_Normal;
	}
}

void SCharacterWalkData::ReleaseCachedJump( CCharacter * pCharacter, float fTime )
{
	CVector2 dVelocity = velocity;
	float l = dVelocity.Normalize();
	dVelocity = dVelocity * Min( fJumpMaxSpeed, fTime / fJumpMaxHoldTime * fJumpMaxSpeed + l );
	velocity = dVelocity;
	bSleep = false;
}

bool SCharacterWalkData::Roll( CCharacter * pCharacter, const CVector2 & moveAxis )
{
	if( nState != eState_Rolling && nState < eState_Knockback )
	{
		if( moveAxis.Length2() > 0 )
			rollDir = moveAxis;
		else
		{
			CVector2 v = velocity / fMoveSpeed;
			float l2 = v.Length2();
			if( l2 >= 1 )
				rollDir = v;
			else if( v.x != 0 )
				rollDir = v + ( v.x > 0 ? CVector2( 1, 0 ) : CVector2( -1, 0 ) ) * ( 1 - l2 );
			else
				return false;
		}

		nState = eState_Rolling;
		fRollTime = 0;
		pLandedEntity = NULL;
		return true;
	}
	return false;
}

void SCharacterWalkData::FallOff()
{
	nState = eState_Normal;
	fJumpHoldingTime = 0;
}

CVector2 SCharacterWalkData::OnLandedEntityMoved( CCharacter* pCharacter, const CMatrix2D & oldTrans, const CMatrix2D & newTrans )
{
	CVector2 oldPos = pCharacter->globalTransform.GetPosition();
	CVector2 localPos = oldTrans.MulTVector2PosNoScale( oldPos );
	CVector2 newPos = newTrans.MulVector2Pos( localPos );

	CVector2 landVelocity = ( newPos - oldPos ) / pCharacter->GetStage()->GetElapsedTimePerTick() - velocity;

	CVector2 gravityDir = gravity;
	gravityDir.Normalize();
	float fNormalSpeed = landVelocity.Dot( gravityDir );
	if( fNormalSpeed > fMaxFallSpeed )
	{
		pLandedEntity = NULL;
		return CVector2( 0, 0 );
	}

	CVector2 normalVelocity = gravityDir * fNormalSpeed;
	velocity = velocity + normalVelocity;

	CVector2 ofs = newPos - oldPos;
	lastLandedEntityTransform = pLandedEntity->globalTransform;
	return ofs;
}

void SCharacterWalkData::FindFloor( CCharacter * pCharacter )
{
	CMatrix2D trans = pCharacter->globalTransform;
	trans.SetPosition( pCharacter->GetPosition() );
	CVector2 dir = gravity;
	dir.Normalize();
	CVector2 ofs = dir * fFindFloorDist;
	SRaycastResult result;
	auto pNewLandedEntity = pCharacter->GetStage()->SweepTest( pCharacter->Get_HitProxy(), trans, ofs, bHitChannel, &result );
	if( pNewLandedEntity && velocity.Dot( result.normal ) < 1.0f && result.normal.Dot( dir ) < -0.5f )
	{
		pLandedEntity = pNewLandedEntity;
		pCharacter->SetPosition( pCharacter->GetPosition() + dir * result.fDist );
		groundNorm = result.normal;
		velocity = velocity - groundNorm * velocity.Dot( groundNorm );
		lastLandedEntityTransform = pLandedEntity->globalTransform;
	}
	else
		pLandedEntity = NULL;
}

CVector2 SCharacterSimpleWalkData::UpdateMove( CCharacter * pCharacter, const CVector2& extraVelocity, float fDir, bool bJump )
{
	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return CVector2();
	}

	float fTime = pCharacter->GetStage()->GetElapsedTimePerTick();

	float dY = -fFallSpeed * fTime;
	float fFallSpeed1 = Min( fFallSpeed + fTime * fGravity, fMaxFallSpeed );
	float dFallSpeed = fFallSpeed1 - fFallSpeed;
	float t = dFallSpeed / fGravity;
	float t1 = fTime - t;
	dY -= dFallSpeed * ( t * 0.5f + t1 );

	float dX = fDir * fMoveSpeed * fTime;
	CVector2 dPos( dX, dY );
	CVector2 velocity( fDir * fMoveSpeed, -fFallSpeed1 );

	if( fKnockbackTime > 0 )
	{
		float fKnockbackTime0 = fKnockbackTime;
		fKnockbackTime = Max( 0.0f, fKnockbackTime - fTime );
		fDir = 0;
		vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
		velocity = vecKnockback;
	}
	else
		bLanded = false;
	velocity = velocity + extraVelocity;
	dPos = dPos + extraVelocity * fTime;

	CVector2 fixedVelocity = velocity;
	TryMove( pCharacter, dPos, fixedVelocity, hits );
	pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );

	if( fixedVelocity.y > velocity.y )
	{
		if( bJump )
		{
			fixedVelocity.y += fJumpSpeed;
		}
		else
			bLanded = true;
	}

	fFallSpeed = -fixedVelocity.y;
	return fixedVelocity;
}

void SCharacterPhysicsFlyData::UpdateMove( CCharacter * pCharacter, const CVector2 & moveTarget )
{
	bool bHasAnyCollision = HasAnyCollision();

	if( bHasAnyCollision && !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}

	CVector2 lastVelocity = pCharacter->GetVelocity();
	float fLastSpeed = lastVelocity.Length();
	float fStopTime = fLastSpeed / fMaxAcc;
	CVector2 stopPos = pCharacter->GetPosition() + lastVelocity * fStopTime * fStablity;

	CVector2 acc( 0, 0 );
	CVector2 d = moveTarget - stopPos;
	float l = d.Normalize();
	if( l > 1.0f )
		acc = d * fMaxAcc;

	float fTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	CVector2 curVelocity = lastVelocity + acc * fTime;
	CVector2 dPos = ( lastVelocity + curVelocity ) * ( fTime / 2 );
	bHit = false;
	if( bHasAnyCollision )
	{
		CVector2 curVelocity0 = curVelocity;
		CVector2 prePos = pCharacter->GetPosition();
		SRaycastResult hits[3];
		TryMove( pCharacter, dPos, curVelocity, hits );
		if( hits[0].pHitProxy )
		{
			bHit = true;
			dVelocity = curVelocity - curVelocity0;
			curVelocity = curVelocity + dVelocity;
			dVelocity = dVelocity * 2;
			pCharacter->SetPosition( pCharacter->GetPosition() * 2 - ( prePos + dPos ) );
		}
	}
	else
		pCharacter->SetPosition( pCharacter->GetPosition() + dPos );
	pCharacter->SetVelocity( curVelocity );
}

void SCharacterPhysicsFlyData1::UpdateMove( CCharacter* pCharacter, CVector2 acc )
{
	bool bHasAnyCollision = HasAnyCollision();

	if( bHasAnyCollision && !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}

	CVector2 lastVelocity = pCharacter->GetVelocity();
	CVector2 curVelocity = lastVelocity;
	float l = curVelocity.Normalize();
	curVelocity = curVelocity * Max( l - fMaxAcc * pCharacter->GetStage()->GetElapsedTimePerTick(), 0.0f );
	curVelocity = curVelocity + acc * pCharacter->GetStage()->GetElapsedTimePerTick();

	CVector2 dPos = ( lastVelocity + curVelocity ) * ( 0.5f * pCharacter->GetStage()->GetElapsedTimePerTick() );
	bHit = false;
	if( bHasAnyCollision )
	{
		CVector2 curVelocity0 = curVelocity;
		CVector2 prePos = pCharacter->GetPosition();
		TryMove( pCharacter, dPos, curVelocity, hits );
		if( hits[0].pHitProxy )
		{
			bHit = true;
			dVelocity = curVelocity - curVelocity0;
			curVelocity = curVelocity + dVelocity;
			dVelocity = dVelocity * 2;
		}
	}
	else
		pCharacter->SetPosition( pCharacter->GetPosition() + dPos );
	pCharacter->SetVelocity( curVelocity );
}

void SCharacterCreepData::UpdateMove( CCharacter* pCharacter, int8 nTurnDir )
{
	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}

	float deltaTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	float fDir = pCharacter->GetRotation();
	if( fKnockbackTime <= 0 )
	{
		if( nTurnDir == 1 )
		{
			fDir += deltaTime * fTurnSpeed;
			pCharacter->SetRotation( fDir );
		}
		else if( nTurnDir == -1 )
		{
			fDir -= deltaTime * fTurnSpeed;
			pCharacter->SetRotation( fDir );
		}
	}

	bHitWall = false;
	bool bHitBlock = false;
	for( auto pManifold = pCharacter->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pCharacter->HasHitFilter() )
		{
			if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
				continue;
		}

		auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject && pBlockObject->GetHitType() != eEntityHitType_WorldStatic )
		{
			bHitWall = true;
			break;
		}
	}
	
	if( bHitWall )
	{
		if( fKnockbackTime > 0 )
		{
			float fKnockbackTime0 = fKnockbackTime;
			fKnockbackTime = Max( 0.0f, fKnockbackTime - deltaTime );
			CVector2 vel0 = vecKnockback;
			vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
			CVector2 vel = vecKnockback;
			CVector2 ofs = ( vel + vel0 ) * 0.5f * deltaTime;
			TryMove( pCharacter, ofs, vel );
			pCharacter->SetVelocity( vel );
		}
		else
		{
			CVector2 vel = CVector2( cos( fDir ), sin( fDir ) ) * fSpeed;
			CVector2 vel0 = vel;
			CVector2 ofs = vel * deltaTime;
			TryMove( pCharacter, ofs, vel );
			if( vel != vel0 )
			{
				vel = vel * 2 - vel0;
				fDir = atan2( vel.y, vel.x );
				pCharacter->SetRotation( fDir );
			}
			pCharacter->SetVelocity( vel );
		}
	}
	else
	{
		if( fKnockbackTime > 0 )
			fKnockbackTime = Max( 0.0f, fKnockbackTime - deltaTime );
		CVector2 vel = pCharacter->GetVelocity();
		CVector2 vel0 = vel;
		vel.y = Max( -fMaxFallSpeed, vel.y - fFallGravity * deltaTime );
		CVector2 ofs = ( vel0 + vel ) * 0.5f * deltaTime;

		SRaycastResult result[3];
		TryMove( pCharacter, ofs, vel, result );
		if( result[0].pHitProxy )
		{
			pCharacter->Crush();
			return;
		}

		fDir = atan2( vel.y, vel.x );
		pCharacter->SetRotation( fDir );
		pCharacter->SetVelocity( vel );
	}
}

void SCharacterSurfaceWalkData::UpdateMove( CCharacter * pCharacter, int8 nDir )
{
	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}

	float fTime = pCharacter->GetStage()->GetElapsedTimePerTick();

	if( bHitSurface )
	{
		CVector2 moveDir = CVector2( normal.y, -normal.x ) * nDir;

		CVector2 vel = moveDir * fSpeed;
		CVector2 ofs = vel * fTime;
		SRaycastResult result[6];
		TryMove( pCharacter, ofs, vel, result );
		CVector2 vel1 = normal * -fSpeed;
		CVector2 ofs1 = vel1 * fTime;
		pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
		TryMove( pCharacter, ofs1, vel1, result + 3 );

		float fMaxDot = -2.0f;
		int32 nMaxSurface = -1;
		for( int i = 0; i < 6; i++ )
		{
			if( !result[i].pHitProxy )
				continue;
			float fDot = -result[i].normal.Dot( normal );
			if( fDot > fMaxDot )
			{
				fMaxDot = fDot;
				nMaxSurface = i;
			}
		}
		
		if( nMaxSurface >= 0 )
		{
			bHitSurface = true;
			normal = result[nMaxSurface].normal;
			pCharacter->SetVelocity( vel + vel1 );
		}
		else
		{
			Fall( pCharacter, CVector2( normal.x * fFallInitSpeed, 0 ) );
		}
	}
	else
	{
		CVector2 vel = pCharacter->GetVelocity();
		CVector2 vel0 = vel;
		vel.y = Max( -fMaxFallSpeed, vel.y - fGravity * fTime );
		CVector2 ofs = ( vel0 + vel ) * 0.5f * fTime;

		SRaycastResult result[3];
		TryMove( pCharacter, ofs, vel, result );
		pCharacter->SetVelocity( vel );
		for( int i = 0; i < 3 && result[i].pHitProxy; i++ )
		{
			bHitSurface = true;
			normal = result[i].normal;
		}
	}
}

void SCharacterSurfaceWalkData::Fall( CCharacter* pCharacter, const CVector2 & vel )
{
	bHitSurface = false;
	pCharacter->SetVelocity( vel );
}

void SCharacterPhysicsMovementData::UpdateMove( CCharacter * pCharacter )
{
	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}

	float deltaTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	CVector2 vel = pCharacter->GetVelocity();

	CVector2 vel1 = CVector2( vel.x, vel.y - fGravity * deltaTime );
	vel1.y = Max( vel1.y, -fMaxFallSpeed );
	CVector2 ofs = ( vel1 + vel ) * 0.5f * deltaTime;

	CVector2 vel0 = vel1;
	TryMove( pCharacter, ofs, vel1 );
	CVector2 dVel = vel1 - vel0;
	float l = dVel.Normalize();
	float l0 = l;
	l = Max( 0.0f, l * fBounceCoef - ( fBounceCoef1 + fGravity * deltaTime ) );

	float l1 = vel1.Normalize();
	l1 = Max( l1 - ( l0 + l ) * fFriction, 0.0f );
	if( fRotCoef != 0 )
	{
		float dRot = l0 > 0 ? ( ofs.y * dVel.x - ofs.x * dVel.y ) / fRotCoef : fRot;
		fRot = dRot;
		pCharacter->SetRotation( pCharacter->GetRotation() + dRot );
	}
	vel1 = vel1 * l1 + dVel * l;

	pCharacter->SetVelocity( vel1 );
}

void SCharacterVehicleMovementData::UpdateMove( CCharacter * pCharacter )
{
	float deltaTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	bHitWall = false;
	bHitWall1 = false;
	fDamage = 0;
	for( auto pManifold = pCharacter->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pCharacter->HasHitFilter() )
		{
			if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
			{
				bHitWall = true;
				bHitWall1 = true;
				continue;
			}
		}

		if( pBlockObject )
		{
			bHitWall = true;
			if( pBlockObject->GetHitType() != eEntityHitType_WorldStatic )
			{
				auto pChunkObject = SafeCast<CChunkObject>( pBlockObject->GetParentEntity() );
				if( pChunkObject && pChunkObject->GetChunk()->nMoveType )
					bHitWall1 = true;
			}
		}
	}

	if( !ResolvePenetration( pCharacter ) )
	{
		for( auto pManifold = pCharacter->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pCharacter->HasHitFilter() )
			{
				if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
					continue;
			}

			auto pBlockObject = SafeCast<CBlockObject>( pEntity );
			if( pBlockObject )
			{
				if( pBlockObject->GetHitType() == eEntityHitType_WorldStatic )
				{
					if( pManifold->normal.y > 0 )
					{
						auto pChunk = pBlockObject->GetBlock()->pOwner->pChunkObject;
						for( auto pChunk1 = SafeCast<CChunkObject>( pChunk->GetParentEntity() ); pChunk1;
							pChunk1 = SafeCast<CChunkObject>( pChunk->GetParentEntity() ) )
							pChunk = pChunk1;
						float fFallSpeed = pChunk->GetChunk()->GetFallSpeed();
						CVector2 vel = pCharacter->GetVelocity();
						vel.y = Min( vel.y, -fFallSpeed );
						pCharacter->SetVelocity( vel );
						pChunk->GetChunk()->bForceStop = true;
					}
					fDamage += 500 * deltaTime;
				}
			}
		}
	}

	CVector2 vel = pCharacter->GetVelocity();
	if( bHitWall )
	{
		CVector2 vel0 = vel;
		CVector2 ofs;
		if( !bHitWall1 )
		{
			vel.y = Max( -fMaxFallSpeed, vel.y - fFallGravity * deltaTime );
			ofs = ( vel0 + vel ) * 0.5f * deltaTime;
		}
		else
			ofs = vel * 0.5f * deltaTime;
		TryMove( pCharacter, ofs, vel );
		if( vel != vel0 )
		{
			vel = vel + ( vel - vel0 ) * fBounce;
			fDamage += ( vel - vel0 ).Length();
		}
	}
	else
	{
		CVector2 vel0 = vel;
		vel.y = Max( -fMaxFallSpeed, vel.y - fFallGravity * deltaTime );
		CVector2 ofs = ( vel0 + vel ) * 0.5f * deltaTime;

		SRaycastResult result[3];
		TryMove( pCharacter, ofs, vel, result );
		if( result[0].pHitProxy )
		{
			pCharacter->Crush();
			return;
		}
	}
	CVector2 characterVel = vel;
	if( vel.Length2() > fCrushSpeed * fCrushSpeed )
	{
		//This makes hit characters sometimes fails to ResolvePenetration and be crushed by the car.
		characterVel.Normalize();
		characterVel = characterVel * fCrushSpeed;
		pCharacter->SetCurPos( pCharacter->GetPosition() + ( vel - characterVel ) * deltaTime );
	}
	pCharacter->SetVelocity( vel );
}

void SCharacterChainMovementData::SetCharacterCount( uint32 nCount )
{
	vecPos.resize( nCount );
	vecVel.resize( nCount );
	vecAcc.resize( nCount );
	vecDir.resize( nCount );

	vecLen.resize( nCount - 1 );
	vecK.resize( nCount - 1 );
	vecAngleLim.resize( nCount );
	vecK1.resize( nCount );
	vecInvWeight.resize( nCount );
	vecExtraAcc.resize( nCount );
	for( int i = 0; i < nCount; i++ )
	{
		vecPos[i] = vecVel[i] = vecAcc[i] = vecExtraAcc[i] = CVector2( 0, 0 );
	}
}

void SCharacterChainMovementData::Simulate( float fTime, uint32 nSteps, CCharacter** pCharacters, uint32 nCharacters )
{
	fTime /= nSteps;
	float f = pow( 0.5f, fTime * fDamping );
	int32 nSegs = vecPos.size();
	for( int iStep = 0; iStep < nSteps; iStep++ )
	{
		for( auto& item : vecAcc )
			item = CVector2( 0, 0 );

		for( int i = 0; i < nSegs - 1; i++ )
		{
			CVector2 dPos = vecPos[i + 1] - vecPos[i];
			float l = dPos.Normalize() - vecLen[i];
			CVector2 force = dPos * vecK[i] * l;
			vecAcc[i] = vecAcc[i] + force;
			vecAcc[i + 1] = vecAcc[i + 1] - force;
		}

		for( int i = 0; i < nSegs; i++ )
		{
			CVector2 dir0;
			float l0 = 0;
			if( i == 0 )
				dir0 = beginDir;
			else
			{
				dir0 = vecPos[i] - vecPos[i - 1];
				l0 = dir0.Normalize();
			}

			CVector2 dir1;
			float l1 = 0;
			if( i == nSegs - 1 )
				dir1 = endDir;
			else
			{
				dir1 = vecPos[i + 1] - vecPos[i];
				l1 = dir1.Normalize();
			}

			float dAngle = atan2( dir0.Dot( CVector2( dir1.y, -dir1.x ) ), dir0.Dot( dir1 ) );
			float fForce = ( dAngle > 0 ? Min( vecAngleLim[i] - dAngle, 0.0f ) : Max( -dAngle - vecAngleLim[i], 0.0f ) ) * vecK1[i];
			if( i > 0 )
			{
				vecAcc[i] = vecAcc[i] + CVector2( dir0.y, -dir0.x ) * fForce;
				vecAcc[i - 1] = vecAcc[i - 1] - CVector2( dir0.y, -dir0.x ) * fForce;
			}
			if( i < nSegs - 1 )
			{
				vecAcc[i] = vecAcc[i] + CVector2( dir1.y, -dir1.x ) * fForce;
				vecAcc[i + 1] = vecAcc[i + 1] - CVector2( dir1.y, -dir1.x ) * fForce;
			}
		}

		for( int i = 0; i < nSegs; i++ )
		{
			vecAcc[i] = vecAcc[i] * vecInvWeight[i] + vecExtraAcc[i];
			vecVel[i] = vecVel[i] * f + vecAcc[i] * fTime;
			vecPos[i] = vecPos[i] + vecVel[i] * fTime;
		}

		for( int i = 0; i < nSegs; i++ )
		{
			CVector2 dir0;
			float l0 = 0;
			if( i == 0 )
				vecDir[i] = vecPos[i + 1] - vecPos[i];
			else if( i == nSegs - 1 )
				vecDir[i] = vecPos[i] - vecPos[i - 1];
			else
			{
				CVector2 dir0 = vecPos[i] - vecPos[i - 1];
				float l0 = dir0.Normalize();

				CVector2 dir1 = vecPos[i + 1] - vecPos[i];
				float l1 = dir1.Normalize();
				vecDir[i] = dir1 * l0 + dir0 * l1;
			}
			vecDir[i].Normalize();

			if( i < nCharacters && pCharacters[i] != NULL )
			{
				pCharacters[i]->SetPosition( vecPos[i] );
				pCharacters[i]->SetRotation( atan2( -vecDir[i].y, -vecDir[i].x ) );
				pCharacters[i]->SetVelocity( vecVel[i] );
			}
		}
	}
}

void SCharacterQueueMovementData::Setup( CCharacter ** pCharacters, uint32 nCharacters )
{
	waypoints.resize( nCharacters );
	angles.resize( nCharacters );

	CVector2 pos( 0, 0 );
	float fAngle = 0;
	for( int i = nCharacters - 1; i >= 0; i-- )
	{
		if( pCharacters[i] )
		{
			pos = pCharacters[i]->GetPosition();
			fAngle = pCharacters[i]->GetRotation();
		}
		waypoints[i] = pos;
		angles[i] = fAngle;
	}
	nWaypointBegin = 0;
	fPercent = 0.0f;
}

void SCharacterQueueMovementData::UpdateMove( CCharacter ** pCharacters, uint32 nCharacters )
{
	float fTime = pCharacters[nCharacters - 1]->GetStage()->GetElapsedTimePerTick();
	fPercent += fTime * fSpeed;
	if( fPercent >= 1 )
	{
		fPercent -= 1;
		waypoints[nWaypointBegin] = pCharacters[nCharacters - 1]->GetPosition();
		angles[nWaypointBegin] = pCharacters[nCharacters - 1]->GetRotation();
		nWaypointBegin++;
		if( nWaypointBegin >= nCharacters )
			nWaypointBegin -= nCharacters;
	}

	for( int i = 0; i < nCharacters - 1; i++ )
	{
		int32 i0 = i + nWaypointBegin;
		if( i0 >= nCharacters )
			i0 -= nCharacters;
		int32 i1 = i0 + 1;
		if( i1 >= nCharacters )
			i1 -= nCharacters;
		if( pCharacters[i] )
		{
			pCharacters[i]->SetPosition( waypoints[i0] + ( waypoints[i1] - waypoints[i0] ) * fPercent );
			pCharacters[i]->SetRotation( InterpAngle( angles[i0], angles[i1], fPercent ) );
			pCharacters[i]->SetVelocity( ( waypoints[i1] - waypoints[i0] ) * fSpeed );
		}
	}
}

float CCharacterMoveUtil::Stretch( CCharacter * pCharacter, uint8 nDir, float fMaxDeltaLen, bool bHitChannel[eEntityHitType_Count] )
{
	SHitProxy* pHitProxy = pCharacter->Get_HitProxy();
	if( !pHitProxy )
		return 0;
	if( pHitProxy->nType != eHitProxyType_Polygon )
		return 0;
	auto pRect = static_cast<SHitProxyPolygon*>( pHitProxy );
	if( pRect->nVertices != 4 )
		return 0;
	float fMinX = FLT_MAX, fMaxX = -FLT_MAX, fMinY = FLT_MAX, fMaxY = -FLT_MAX;
	for( int i = 0; i < 4; i++ )
	{
		fMinX = Min( fMinX, pRect->vertices[i].x );
		fMaxX = Max( fMaxX, pRect->vertices[i].x );
		fMinY = Min( fMinY, pRect->vertices[i].y );
		fMaxY = Max( fMaxY, pRect->vertices[i].y );
	}

	CVector2 ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

	float fDist = fMaxDeltaLen;
	if( fMaxDeltaLen > 0 )
	{
		SRaycastResult result;
		if( pCharacter->GetStage()->SweepTest( pRect, pCharacter->globalTransform, pCharacter->globalTransform.MulVector2Dir( ofs[nDir] )
			* fMaxDeltaLen, bHitChannel, &result ) )
			fDist = result.fDist;
		if( fDist <= 0 )
			return 0;
		switch( nDir )
		{
		case 0:
			fMaxX += fDist;
			break;
		case 1:
			fMaxY += fDist;
			break;
		case 2:
			fMinX -= fDist;
			break;
		case 3:
			fMinY -= fDist;
			break;
		}
	}
	else
	{
		float fLen;
		switch( nDir )
		{
		case 0:
			fLen = Max( fMinX, fMaxX + fMaxDeltaLen );
			fDist = fLen - fMaxX;
			fMaxX = fLen;
			break;
		case 1:
			fLen = Max( fMinY, fMaxY + fMaxDeltaLen );
			fDist = fLen - fMaxY;
			fMaxY = fLen;
			break;
		case 2:
			fLen = Min( fMaxX, fMinX - fMaxDeltaLen );
			fDist = fLen - fMinX;
			fMinX = fLen;
			break;
		case 3:
			fLen = Min( fMaxY, fMinY - fMaxDeltaLen );
			fDist = fLen - fMinY;
			fMinY = fLen;
			break;
		}
	}

	pRect->vertices[0] = CVector2( fMinX, fMinY );
	pRect->vertices[1] = CVector2( fMaxX, fMinY );
	pRect->vertices[2] = CVector2( fMaxX, fMaxY );
	pRect->vertices[3] = CVector2( fMinX, fMaxY );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
	return fDist;
}

float CCharacterMoveUtil::StretchEx( CCharacter * pCharacter, uint8 nDir, float fMinLen, float fMaxLen, float fMoveDist, bool bHitChannel[eEntityHitType_Count] )
{
	SHitProxy* pHitProxy = pCharacter->Get_HitProxy();
	if( !pHitProxy )
		return 0;
	if( pHitProxy->nType != eHitProxyType_Polygon )
		return 0;
	auto pRect = static_cast<SHitProxyPolygon*>( pHitProxy );
	if( pRect->nVertices != 4 )
		return 0;
	float fMinX = FLT_MAX, fMaxX = -FLT_MAX, fMinY = FLT_MAX, fMaxY = -FLT_MAX;
	for( int i = 0; i < 4; i++ )
	{
		fMinX = Min( fMinX, pRect->vertices[i].x );
		fMaxX = Max( fMaxX, pRect->vertices[i].x );
		fMinY = Min( fMinY, pRect->vertices[i].y );
		fMaxY = Max( fMaxY, pRect->vertices[i].y );
	}

	CVector2 ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

	float fDist = fMoveDist;
	float fLen = nDir == 0 || nDir == 2 ? fMaxX - fMinX : fMaxY - fMinY;
	SRaycastResult result;
	if( pCharacter->GetStage()->SweepTest( pRect, pCharacter->globalTransform, pCharacter->globalTransform.MulVector2Dir( ofs[nDir] )
		* fMoveDist, bHitChannel, &result, true ) )
		fDist = Max( result.fDist, Min( fDist, fMinLen - fLen ) );
	if( fDist <= 0 )
		return 0;
	switch( nDir )
	{
	case 0:
		fMaxX += fDist;
		break;
	case 1:
		fMaxY += fDist;
		break;
	case 2:
		fMinX -= fDist;
		break;
	case 3:
		fMinY -= fDist;
		break;
	}
	float fDist1 = fLen + fDist - fMaxLen;
	if( fDist1 > 0 )
	{
		fDist -= fDist1;
		switch( nDir )
		{
		case 0:
			fMaxX -= fDist1;
			pCharacter->SetPosition( CVector2( pCharacter->x + fDist1, pCharacter->y ) );
			break;
		case 1:
			fMaxY -= fDist1;
			pCharacter->SetPosition( CVector2( pCharacter->x , pCharacter->y+ fDist1 ) );
			break;
		case 2:
			fMinX += fDist1;
			pCharacter->SetPosition( CVector2( pCharacter->x - fDist1, pCharacter->y ) );
			break;
		case 3:
			fMinY += fDist1;
			pCharacter->SetPosition( CVector2( pCharacter->x, pCharacter->y - fDist1 ) );
			break;
		}
		pCharacter->ForceUpdateTransform();
	}

	pRect->vertices[0] = CVector2( fMinX, fMinY );
	pRect->vertices[1] = CVector2( fMaxX, fMinY );
	pRect->vertices[2] = CVector2( fMaxX, fMaxY );
	pRect->vertices[3] = CVector2( fMinX, fMaxY );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
	return fDist;
}
