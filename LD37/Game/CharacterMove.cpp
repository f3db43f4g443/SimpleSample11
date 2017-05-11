#include "stdafx.h"
#include "CharacterMove.h"
#include "Stage.h"
#include "GameUtil.h"

void SCharacterMovementData::TryMove( CCharacter * pCharacter, const CVector2& ofs, SRaycastResult* pHit )
{
	if( pHit )
		memset( pHit, 0, sizeof( SRaycastResult ) * 3 );

	SRaycastResult result;
	CVector2 moveOfs = ofs;
	if( moveOfs.Normalize() > 0 )
	{
		if( !pCharacter->GetStage()->SweepTest( pCharacter->Get_HitProxy(), pCharacter->globalTransform, ofs, bHitChannel, &result ) )
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
				if( pCharacter->GetStage()->SweepTest( pCharacter->Get_HitProxy(), mat1, ofs2, bHitChannel, &result1 ) )
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
						if( pCharacter->GetStage()->SweepTest( pCharacter->Get_HitProxy(), mat2, ofs4, bHitChannel, &result2 ) )
						{
							if( pHit )
								pHit[2] = result2;
							ofs4.Normalize();
							ofs4 = ofs4 * result.fDist;
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
		if( !pCharacter->GetStage()->SweepTest( pCharacter->Get_HitProxy(), pCharacter->globalTransform, ofs, bHitChannel, &result ) )
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
				if( pCharacter->GetStage()->SweepTest( pCharacter->Get_HitProxy(), mat1, ofs2, bHitChannel, &result1 ) )
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
						if( pCharacter->GetStage()->SweepTest( pCharacter->Get_HitProxy(), mat2, ofs4, bHitChannel, &result2 ) )
						{
							if( pHit )
								pHit[2] = result2;
							velocity = velocity - result2.normal * ( result2.normal.Dot( velocity ) );
							ofs4.Normalize();
							ofs4 = ofs4 * result.fDist;
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

bool SCharacterMovementData::TryMoveAcrossWall( CCharacter * pCharacter, const CVector2& ofs, const CVector2& ofsLeft )
{
	CVector2 ofsDir = ofsLeft;
	float fDistLeft = ofsDir.Normalize();
	CMatrix2D trans = pCharacter->globalTransform;
	CVector2 moveOfs = ofs;

	float fMaxDist = fDistLeft;
	trans.SetPosition( pCharacter->globalTransform.GetPosition() + ofsLeft );
	vector<CHitProxy*> hitResult;
	pCharacter->GetStage()->GetHitTestMgr().HitTest( pCharacter->Get_HitProxy(), trans, hitResult );
	bool bHit = false;
	for( auto pHitProxy : hitResult )
	{
		auto eHitType = static_cast<CEntity*>( pHitProxy )->GetHitType();
		if( bHitChannel[eHitType] )
		{
			bHit = true;
			break;
		}
	}

	if( bHit )
	{
		vector<CReference<CEntity> > hitEntities;
		vector<SRaycastResult> result;
		pCharacter->GetStage()->MultiSweepTest( pCharacter->Get_HitProxy(), pCharacter->globalTransform, ofsLeft, hitEntities, &result );

		for( int i = result.size() - 1; i >= 0; i-- )
		{
			auto& raycastResult = result[i];
			auto eHitType = static_cast<CEntity*>( raycastResult.pHitProxy )->GetHitType();
			if( !bHitChannel[eHitType] )
				continue;

			fDistLeft = raycastResult.fDist;
			trans.SetPosition( pCharacter->globalTransform.GetPosition() + ofsDir * raycastResult.fDist );
			hitResult.clear();
			pCharacter->GetStage()->GetHitTestMgr().HitTest( pCharacter->Get_HitProxy(), trans, hitResult );
			bHit = false;
			for( auto pHitProxy : hitResult )
			{
				eHitType = static_cast<CEntity*>( pHitProxy )->GetHitType();
				if( pHitProxy != raycastResult.pHitProxy && bHitChannel[eHitType] )
				{
					bHit = true;
					break;
				}
			}
			if( !bHit )
				break;
		}
	}

	bool bIsMovingAcrossWall;
	if( bHit )
	{
		bIsMovingAcrossWall = true;
		pCharacter->SetPosition( pCharacter->globalTransform.GetPosition() + moveOfs );
	}
	else
	{
		float fMoveLength = moveOfs.Length();
		if( fMoveLength > fDistLeft )
		{
			moveOfs.Normalize();
			moveOfs = moveOfs * fDistLeft;

			bIsMovingAcrossWall = false;
			trans.SetPosition( pCharacter->globalTransform.GetPosition() + moveOfs );
		}
		else
		{
			trans.SetPosition( pCharacter->globalTransform.GetPosition() + moveOfs );
			hitResult.clear();
			pCharacter->GetStage()->GetHitTestMgr().HitTest( pCharacter->Get_HitProxy(), trans, hitResult );
			bHit = false;
			for( auto pHitProxy : hitResult )
			{
				auto eHitType = static_cast<CEntity*>( pHitProxy )->GetHitType();
				if( bHitChannel[eHitType] )
				{
					bHit = true;
					break;
				}
			}
			bIsMovingAcrossWall = bHit;
		}
		pCharacter->SetPosition( trans.GetPosition() );
	}
	return bIsMovingAcrossWall;
}

bool SCharacterMovementData::ResolvePenetration( CCharacter* pCharacter )
{
	CVector2 posFix( 0, 0 );

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
		auto eHitType = static_cast<CEntity*>( hitResult[i] )->GetHitType();
		if( bHitChannel[eHitType] && hitTestResult[i].normal.Length2() > 0.05f * 0.05f )
		{
			bHit = true;
			break;
		}
	}
	if( bHit )
		return false;

	pCharacter->SetPosition( newPos );
	return true;
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


void SCharacterFlyData::UpdateMove( CCharacter* pCharacter, const CVector2& moveAxis )
{
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

			pCharacter->SetPosition( newPos );
			pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
			pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
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

	CVector2 moveOfs;
	if( nState == eState_Normal )
	{
		if( moveAxis.Length2() > 0 || fKnockbackTime > 0 )
		{
			float fDeltaTime = pCharacter->GetStage()->GetElapsedTimePerTick();
			moveOfs = ( moveAxis * fMoveSpeed ) * fDeltaTime;

			if( fKnockbackTime > 0 )
			{
				float fKnockbackTime0 = fKnockbackTime;
				fKnockbackTime = Max( 0.0f, fKnockbackTime - fDeltaTime );
				vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
				moveOfs = vecKnockback * fDeltaTime;
			}

			CVector2 velocity = moveOfs;
			TryMove( pCharacter, moveOfs, velocity );
			finalMoveAxis = velocity;
			finalMoveAxis.Normalize();
		}
	}
	else
	{
		float fTime = pCharacter->GetStage()->GetElapsedTimePerTick();
		float fNewRollTime = Min( fRollTime + fTime, fRollMaxTime );
		float fDist0 = ( 2 - fRollTime / fRollMaxTime ) * fRollTime * fRollMaxSpeed * 0.5f;
		float fDist1 = ( 2 - fNewRollTime / fRollMaxTime ) * fNewRollTime * fRollMaxSpeed * 0.5f;
		CVector2 rollOfsLeft = rollDir * ( fRollMaxTime * fRollMaxSpeed * 0.5f - fDist0 );
		CVector2 moveOfs = rollDir * ( fDist1 - fDist0 );
		fRollTime = fNewRollTime;

		TryMove( pCharacter, moveOfs );

		/*if( bRollingAcrossWall )
		{
			bRollingAcrossWall = TryMoveAcrossWall( pCharacter, moveOfs, rollOfsLeft );
			if( !bRollingAcrossWall )
			{
				pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
				pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
				if( !ResolvePenetration( pCharacter ) )
				{
					pCharacter->Crush();
					return;
				}
			}
		}
		else
		{
			if( !ResolvePenetration( pCharacter ) )
			{
				pCharacter->Crush();
				return;
			}
			pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
			pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
			bRollingAcrossWall = TryMoveAcrossWall( pCharacter, moveOfs, rollOfsLeft );
		}*/

		if( fRollTime >= fRollMaxTime )
		{
			Reset();
		}
	}

	pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );
}

void SCharacterFlyData::UpdateMoveNoBlocking( CCharacter * pCharacter, const CVector2 & moveAxis )
{
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

			pCharacter->globalTransform.SetPosition( newPos );
			pCharacter->SetPosition( newPos );
			pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
			lastLandedEntityTransform = pLandedEntity->globalTransform;
			bSleep = false;
		}
	}

	if( moveAxis.Length2() > 0 )
	{
		CVector2 moveOfs = ( moveAxis * fMoveSpeed ) * pCharacter->GetStage()->GetElapsedTimePerTick();
		pCharacter->SetPosition( pCharacter->GetPosition() + moveOfs );
	}
}

void SCharacterFlyData::Roll( CCharacter* pCharacter, const CVector2& moveAxis )
{
	if( nState != eState_Rolling && fKnockbackTime <= 0 )
	{
		nState = eState_Rolling;
		fRollTime = 0;
		rollDir = moveAxis;
		bRollingAcrossWall = false;
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
	else if( velocity.x > 0 )
		nDir = -1;
	else if( velocity.x < 0 )
		nDir = 1;
	else
		nDir = 0;

	const CMatrix2D oldTransform = pCharacter->GetGlobalTransform();
	CMatrix2D curTransform = oldTransform;
	const CVector2 oldPos = oldTransform.GetPosition();
	CVector2 curPos = oldPos;
	float fDeltaTime = pCharacter->GetStage()->GetElapsedTimePerTick();
	if( fKnockbackTime > 0 )
	{
		float fKnockbackTime0 = fKnockbackTime;
		fKnockbackTime = Max( 0.0f, fKnockbackTime - fDeltaTime );
		nDir = 0;
		vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
	}

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
			OnLandedEntityMoved( pCharacter, lastLandedEntityTransform, pLandedEntity->globalTransform );
			pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
			pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
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
			ReleaseJump( pCharacter );
		}
	}

	CVector2 dVelocity = CVector2( 0, 0 );
	CVector2 dPos = velocity * fDeltaTime;
	CVector2 moveDir;

	if( pLandedEntity )
	{
		CVector2 groundTangent( -groundNorm.y, groundNorm.x );
		moveDir = groundTangent * ( groundTangent.x > 0 ? 1 : -1 ) * nDir;
	}
	else
	{
		CVector2 vec( -gravity.y, gravity.x );
		moveDir = vec * ( vec.x > 0 ? 1 : -1 ) * nDir;
		moveDir.Normalize();
	}
	float fTangentVelocity = velocity.Dot( moveDir );
	bool bIsMoveOrStop = fTangentVelocity >= 0;
	float fSpeedTarget = bIsMoveOrStop ? fMoveSpeed : 0;
	float fMaxSpeed = pLandedEntity ? fMoveSpeed : fAirMaxSpeed;

	if( fTangentVelocity > fMaxSpeed )
	{
		moveDir = moveDir * -1;
		fTangentVelocity = -fTangentVelocity;
		bIsMoveOrStop = false;
		fSpeedTarget = -fMaxSpeed;
	}
	float fAcc = pLandedEntity ? ( bIsMoveOrStop ? fMoveAcc : fStopAcc ) : fAirAcc;
	float fDeltaSpeed = fSpeedTarget - fTangentVelocity;
	float t0 = Min( fDeltaTime, fDeltaSpeed / fAcc );
	float t1 = fDeltaTime - t0;

	dVelocity = dVelocity + moveDir * ( fAcc * t0 );
	dPos = dPos + moveDir * ( fAcc * t0 * ( t0 * 0.5f + t1 ) );

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

		if( !nIsSlidingDownWall )
		{
			if( nState == eState_JumpHolding )
				nState = eState_Normal;
		}
	}
	velocity = velocity + dVelocity;

	pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
	CVector2 v0 = velocity;
	TryMove( pCharacter, dPos, velocity );
	FindFloor( pCharacter );

	nIsSlidingDownWall = 0;
	if( !pLandedEntity && fKnockbackTime <= 0 )
	{
		if( moveAxis.x > 0 && v0.x > velocity.x )
			nIsSlidingDownWall = 1;
		else if( moveAxis.x < 0 && v0.x < velocity.x )
			nIsSlidingDownWall = -1;
		if( nIsSlidingDownWall )
			velocity.y = Max( velocity.y, -fSlideDownSpeed );
	}
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

	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}
	pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
	TryMove( pCharacter, moveOfs );

	/*if( bRollingAcrossWall )
	{
		bRollingAcrossWall = TryMoveAcrossWall( pCharacter, moveOfs, rollOfsLeft );
		if( !bRollingAcrossWall )
		{
			pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
			pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
			if( !ResolvePenetration( pCharacter ) )
			{
				pCharacter->Crush();
				return;
			}
		}
	}
	else
	{
		if( !ResolvePenetration( pCharacter ) )
		{
			pCharacter->Crush();
			return;
		}
		pCharacter->globalTransform.SetPosition( pCharacter->GetPosition() );
		pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
		bRollingAcrossWall = TryMoveAcrossWall( pCharacter, moveOfs, rollOfsLeft );
	}*/

	if( fRollTime >= fRollMaxTime )
	{
		Reset();
	}
}

void SCharacterWalkData::Jump( CCharacter * pCharacter )
{
	if( ( pLandedEntity || nIsSlidingDownWall ) && nState == eState_Normal && fKnockbackTime <= 0 )
	{
		nState = eState_JumpHolding;
		fJumpHoldingTime = 0;
	}
}

void SCharacterWalkData::ReleaseJump( CCharacter * pCharacter )
{
	if( nState == eState_JumpHolding )
	{
		CVector2 dVelocity;
		if( nIsSlidingDownWall > 0 )
		{
			dVelocity = CVector2( -0.5f, 0.732f );
			velocity.y = 0;
		}
		else if( nIsSlidingDownWall < 0 )
		{
			dVelocity = CVector2( 0.5f, 0.732f );
			velocity.y = 0;
		}
		else
			dVelocity = CVector2( 0, 1 );
		dVelocity = dVelocity * ( fJumpHoldingTime / fJumpMaxHoldTime * fJumpMaxSpeed );
		velocity = velocity + dVelocity;
		bSleep = false;
		nState = eState_Normal;
	}
}

void SCharacterWalkData::Roll( CCharacter * pCharacter, const CVector2 & moveAxis )
{
	if( nState != eState_Rolling && nRollCount && fKnockbackTime <= 0 )
	{
		nState = eState_Rolling;
		fRollTime = 0;
		rollDir = moveAxis;
		bRollingAcrossWall = false;
		nRollCount--;
		pLandedEntity = NULL;
		return;
	}
}

void SCharacterWalkData::FallOff()
{
}

void SCharacterWalkData::OnLandedEntityMoved( CCharacter* pCharacter, const CMatrix2D & oldTrans, const CMatrix2D & newTrans )
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
		return;
	}

	CVector2 normalVelocity = gravityDir * fNormalSpeed;
	velocity = velocity + normalVelocity;

	pCharacter->globalTransform.SetPosition( newPos );
	pCharacter->SetPosition( newPos );
	pCharacter->GetStage()->GetHitTestMgr().Update( pCharacter );
	lastLandedEntityTransform = pLandedEntity->globalTransform;
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
		nRollCount = 1;
	}
	else
		pLandedEntity = NULL;
}

CVector2 SCharacterSimpleWalkData::UpdateMove( CCharacter * pCharacter, int8 nDir, bool bJump )
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

	float dX = nDir * fMoveSpeed * fTime;
	CVector2 dPos( dX, dY );
	CVector2 velocity( nDir * fMoveSpeed, -fFallSpeed1 );

	if( fKnockbackTime > 0 )
	{
		float fKnockbackTime0 = fKnockbackTime;
		fKnockbackTime = Max( 0.0f, fKnockbackTime - fTime );
		nDir = 0;
		vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
		velocity = vecKnockback;
	}

	CVector2 fixedVelocity = velocity;
	TryMove( pCharacter, dPos, fixedVelocity, hits );
	pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );

	bLanded = false;
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
			curVelocity = curVelocity * 2 - curVelocity0;
			pCharacter->SetPosition( pCharacter->GetPosition() * 2 - ( prePos + dPos ) );
		}
	}
	else
		pCharacter->SetPosition( pCharacter->GetPosition() + dPos );
	pCharacter->SetVelocity( curVelocity );
}

void SCharacterCreepData::UpdateMove( CCharacter* pCharacter, uint8 nTurnDir )
{
	float fDir = pCharacter->GetRotation();

}
