#include "stdafx.h"
#include "CharacterMove.h"
#include "Stage.h"
#include "GameUtil.h"
#include "MyLevel.h"
#include "Common/MathUtil.h"
#include <algorithm>

void SCharacterMovementData::TryMove( CCharacter* pCharacter, const CVector2& ofs, SRaycastResult* pHit )
{
	if( pHit )
		memset( pHit, 0, sizeof( SRaycastResult ) * 3 );

	SRaycastResult result;
	CVector2 moveOfs = ofs;
	if( moveOfs.Normalize() > 0 )
	{
		auto trans = pCharacter->GetGlobalTransform();
		if( !DoSweepTest( pCharacter, trans, ofs, MOVE_SIDE_THRESHOLD, &result, true ) )
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
			CMatrix2D mat1 = trans;
			mat1.SetPosition( mat1.GetPosition() + ofs1 );
			if( ofs2.Length2() > 0 )
			{
				SRaycastResult result1;
				if( DoSweepTest( pCharacter, mat1, ofs2, MOVE_SIDE_THRESHOLD, &result1, true ) )
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
						if( DoSweepTest( pCharacter, mat2, ofs4, MOVE_SIDE_THRESHOLD, &result2, true ) )
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

void SCharacterMovementData::TryMove( CCharacter* pCharacter, const CVector2& ofs, CVector2& velocity, SRaycastResult* pHit, float fFrac )
{
	if( pHit )
		memset( pHit, 0, sizeof( SRaycastResult ) * 3 );

	SRaycastResult result;
	CVector2 moveOfs = ofs;
	if( moveOfs.Normalize() > 0 )
	{
		auto trans = pCharacter->GetGlobalTransform();
		if( !DoSweepTest( pCharacter, trans, ofs, MOVE_SIDE_THRESHOLD, &result, true ) )
		{
			pCharacter->SetPosition( pCharacter->GetPosition() + ofs );
		}
		else
		{
			if( pHit )
				pHit[0] = result;
			auto v0 = result.pHitProxy->GetVelocity();
			velocity = HitVel( velocity, v0, result.normal, fFrac );

			CVector2 ofs1 = moveOfs * result.fDist;
			CVector2 ofs2 = ofs - ofs1;
			ofs2 = ofs2 - result.normal * ( result.normal.Dot( ofs2 ) );
			CMatrix2D mat1 = trans;
			mat1.SetPosition( mat1.GetPosition() + ofs1 );
			if( ofs2.Length2() > 0 )
			{
				SRaycastResult result1;
				if( DoSweepTest( pCharacter, mat1, ofs2, MOVE_SIDE_THRESHOLD, &result1, true ) )
				{
					if( pHit )
						pHit[1] = result1;
					auto v0 = result1.pHitProxy->GetVelocity();
					velocity = HitVel( velocity, v0, result1.normal, fFrac );
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
						if( DoSweepTest( pCharacter, mat2, ofs4, MOVE_SIDE_THRESHOLD, &result2, true ) )
						{
							if( pHit )
								pHit[2] = result2;
							auto v0 = result2.pHitProxy->GetVelocity();
							velocity = HitVel( velocity, v0, result2.normal, fFrac );
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

void SCharacterMovementData::TryMove1( CCharacter* pCharacter, int32 nTested, CEntity** pTested, const CVector2& ofs, CVector2& velocity, float fFrac, CVector2* pGravityDir, SRaycastResult* pHit )
{
	if( pHit )
		memset( pHit, 0, sizeof( SRaycastResult ) * 3 );

	SRaycastResult result;
	CVector2 moveOfs = ofs;
	if( moveOfs.Normalize() > 0 )
	{
		CMatrix2D* mats = (CMatrix2D*)alloca( sizeof( CMatrix2D ) * nTested );
		for( int i = 0; i < nTested; i++ )
			mats[i] = pTested[i]->GetGlobalTransform();

		if( !DoSweepTest1( pCharacter, nTested, pTested, mats, ofs, MOVE_SIDE_THRESHOLD, pGravityDir, &result, true ) )
		{
			pCharacter->SetPosition( pCharacter->GetPosition() + ofs );
		}
		else
		{
			if( pHit )
				pHit[0] = result;
			auto v0 = result.pHitProxy->GetVelocity();
			velocity = HitVel( velocity, v0, result.normal, fFrac );
			CVector2 ofs1 = moveOfs * result.fDist;
			CVector2 ofs2 = ofs - ofs1;
			ofs2 = ofs2 - result.normal * ( result.normal.Dot( ofs2 ) );
			for( int i = 0; i < nTested; i++ )
				mats[i].SetPosition( mats[i].GetPosition() + ofs1 );
			if( ofs2.Length2() > 0 )
			{
				SRaycastResult result1;
				if( DoSweepTest1( pCharacter, nTested, pTested, mats, ofs2, MOVE_SIDE_THRESHOLD, pGravityDir, &result1, true ) )
				{
					if( pHit )
						pHit[1] = result1;
					auto v0 = result1.pHitProxy->GetVelocity();
					velocity = HitVel( velocity, v0, result1.normal, fFrac );
					CVector2 ofs3 = ofs2;
					ofs3.Normalize();
					ofs3 = ofs3 * result1.fDist;

					if( result.normal.Dot( result1.normal ) >= 0 )
					{
						CVector2 ofs4 = ofs2 - ofs3;
						ofs4 = ofs4 - result1.normal * ( result1.normal.Dot( ofs4 ) );

						for( int i = 0; i < nTested; i++ )
							mats[i].SetPosition( mats[i].GetPosition() + ofs3 );
						SRaycastResult result2;
						if( DoSweepTest1( pCharacter, nTested, pTested, mats, ofs4, MOVE_SIDE_THRESHOLD, pGravityDir, &result2, true ) )
						{
							if( pHit )
								pHit[2] = result2;
							auto v0 = result2.pHitProxy->GetVelocity();
							velocity = HitVel( velocity, v0, result2.normal, fFrac );
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

struct SLP
{
	SLP() : bOpen( true ), bDead( false ) {}
	struct SItem
	{
		float a, b, c;//ax + by >= c
	};
	vector<SItem> vecItems;
	vector<int32> vecEdges;
	bool bOpen;
	bool bDead;

	void Reset()
	{
		vecItems.resize( 0 );
		vecEdges.resize( 0 );
		bOpen = true;
		bDead = false;
	}
	void AddItem( float a, float b, float c )
	{
		if( bDead )
			return;
		SItem item = { a, b, c };
		vecItems.push_back( item );
		if( vecEdges.size() == 0 )
		{
			vecEdges.push_back( 0 );
			return;
		}

		int32 nVerts = bOpen ? vecEdges.size() - 1 : vecEdges.size();
		float k0, kLast;
		int32 nIn = -1, nOut = -1;
		for( int i = 0; i < nVerts; i++ )
		{
			int32 n0 = vecEdges[i];
			int32 n1 = vecEdges[i == vecEdges.size() - 1 ? 0 : i + 1];

			float a0 = vecItems[n0].a, b0 = vecItems[n0].b, c0 = vecItems[n0].c;
			float a1 = vecItems[n1].a, b1 = vecItems[n1].b, c1 = vecItems[n1].c;
			float mx = c0 * b1 - c1 * b0, my = a0 * c1 - a1 * c0, n = a0 * b1 - a1 * b0;
			float k = ( a * mx + b * my - c * n ) * n;

			if( i == 0 )
				k0 = k;
			else if( kLast <= 0 && k > 0 )
				nIn = i;
			else if( kLast > 0 && k <= 0 )
				nOut = i;
			kLast = k;
		}

		if( bOpen )
		{
			int32 n0 = vecEdges[0];
			int32 n1 = vecEdges.back();
			float a0 = vecItems[n0].a, b0 = vecItems[n0].b, c0 = vecItems[n0].c;
			float a1 = vecItems[n1].a, b1 = vecItems[n1].b, c1 = vecItems[n1].c;
			float kInf0 = CVector2( -b0, a0 ).Dot( CVector2( a, b ) );
			float kInf1 = CVector2( b1, -a1 ).Dot( CVector2( a, b ) );
			if( nVerts )
			{
				if( kInf0 <= 0 && k0 > 0 )
					nIn = 0;
				else if( kInf0 > 0 && k0 <= 0 )
					nOut = 0;
				if( kLast <= 0 && kInf1 > 0 )
					nIn = nVerts;
				else if( kLast > 0 && kInf1 <= 0 )
					nOut = nVerts;
			}
			else
			{
				if( kInf0 == 0 && kInf1 == 0 && vecEdges.size() == 1 )
				{
					if( ( abs( a ) > abs( b ) ? a * a0 : b * b0 ) > 0 )
					{
						if( abs( a ) > abs( b ) ? a * c * a0 > a * c0 * a : b * c * b0 > b * c0 * b )
						{
							vecEdges[0] = vecItems.size() - 1;
						}
						return;
					}
				}
				if( kInf0 <= 0 && kInf1 > 0 )
					nIn = 0;
				else if( kInf0 > 0 && kInf1 <= 0 )
					nOut = 0;
			}
			if( nIn == -1 && nOut == -1 )
			{
				if( kInf0 <= 0 )
					bDead = true;
				return;
			}
			int32 nCount;
			int32* tmp;
			if( nOut == -1 )
			{
				nCount = nVerts - nIn + 2;
				tmp = (int32*)alloca( nCount * sizeof( int32 ) );
				tmp[0] = vecItems.size() - 1;
				for( int i = 1; i < nCount; i++ )
					tmp[i] = vecEdges[nIn + i - 1];
			}
			else if( nIn == -1 )
			{
				nCount = nOut + 2;
				tmp = (int32*)alloca( nCount * sizeof( int32 ) );
				for( int i = 0; i < nCount - 1; i++ )
					tmp[i] = vecEdges[i];
				tmp[nCount - 1] = vecItems.size() - 1;
			}
			else if( nIn > nOut )
			{
				nCount = nOut + 1 + ( nVerts - nIn + 1 ) + 1;
				tmp = (int32*)alloca( nCount * sizeof( int32 ) );
				nCount = 0;
				for( int i = 0; i <= nOut; i++ )
					tmp[nCount++] = vecEdges[i];
				tmp[nCount++] = vecItems.size() - 1;
				for( int i = nIn; i <= nVerts; i++ )
					tmp[nCount++] = vecEdges[i];
			}
			else
			{
				bOpen = false;
				nCount = nOut - nIn + 2;
				tmp = (int32*)alloca( nCount * sizeof( int32 ) );
				tmp[0] = vecItems.size() - 1;
				for( int i = 1; i < nCount; i++ )
					tmp[i] = vecEdges[nIn + i - 1];
			}
			vecEdges.resize( nCount );
			for( int i = 0; i < nCount; i++ )
				vecEdges[i] = tmp[i];
		}
		else
		{
			if( kLast <= 0 && k0 > 0 )
				nIn = 0;
			else if( kLast > 0 && k0 <= 0 )
				nOut = 0;
			if( nIn < 0 )
			{
				if( k0 <= 0 )
					bDead = true;
				return;
			}
			if( nOut < nIn )
				nOut += nVerts;
			int32 nCount = nOut - nIn + 2;
			int32* tmp = (int32*)alloca( nCount * sizeof( int32 ) );
			tmp[0] = vecItems.size() - 1;
			for( int i = 1; i < nCount; i++ )
				tmp[i] = vecEdges[( nIn + i - 1 ) % nVerts];
			vecEdges.resize( nCount );
			for( int i = 0; i < nCount; i++ )
				vecEdges[i] = tmp[i];
		}
	}
	bool Nearest( CVector2& p )
	{
		if( bDead )
			return false;
		if( !vecEdges.size() )
			return false;
		float dMax = 0;
		int32 iEdge = -1;
		for( int i = 0; i < vecEdges.size(); i++ )
		{
			int32 n = vecEdges[i];
			float a = vecItems[n].a, b = vecItems[n].b, c = vecItems[n].c;
			float d = -( a * p.x + b * p.y - c ) / sqrt( a * a + b * b );
			if( d > dMax )
			{
				dMax = d;
				iEdge = i;
			}
		}
		if( iEdge == -1 )
			return false;

		int32 n0 = vecEdges[iEdge];
		float a0 = vecItems[n0].a, b0 = vecItems[n0].b, c0 = vecItems[n0].c;
		CVector2 p0( a0 * c0 - a0 * b0 * p.y + b0 * b0 * p.x, b0 * c0 - a0 * b0 * p.x + a0 * a0 * p.y );
		p0 = p0 / ( a0 * a0 + b0 * b0 );
		p = p0;
		if( vecEdges.size() > 1 )
		{
			if( !bOpen || iEdge > 0 )
			{
				int32 n1 = vecEdges[iEdge == 0 ? vecEdges.size() - 1 : iEdge - 1];
				float a1 = vecItems[n1].a, b1 = vecItems[n1].b, c1 = vecItems[n1].c;
				if( a1 * p0.x + b1 * p0.y < c1 )
				{
					float mx = c0 * b1 - c1 * b0, my = a0 * c1 - a1 * c0, n = a0 * b1 - a1 * b0;
					p = CVector2( mx / n, my / n );
					return true;
				}
			}
			if( !bOpen || iEdge < vecEdges.size() - 1 )
			{
				int32 n1 = vecEdges[iEdge == vecEdges.size() - 1 ? 0 : iEdge + 1];
				float a1 = vecItems[n1].a, b1 = vecItems[n1].b, c1 = vecItems[n1].c;
				if( a1 * p0.x + b1 * p0.y < c1 )
				{
					float mx = c0 * b1 - c1 * b0, my = a0 * c1 - a1 * c0, n = a0 * b1 - a1 * b0;
					p = CVector2( mx / n, my / n );
					return true;
				}
			}
		}
		return true;
	}
};

#define PLATFORM_THRESHOLD 0.99f
bool SCharacterMovementData::ResolvePenetration( CCharacter* pCharacter, CVector2* pVel, float fFrac, CEntity* pLandedEntity, CVector2* pLandedOfs, const CVector2* pGravity,
	CEntity** pTested, int32 nTested )
{
	CEntity* pDefaultTested = pCharacter;
	if( !nTested )
	{
		pTested = &pDefaultTested;
		nTested = 1;
	}
	CMatrix2D* testedTransform = (CMatrix2D*)alloca( sizeof( CMatrix2D ) * nTested );
	for( int i = 0; i < nTested; i++ )
		testedTransform[i] = pTested[i]->GetGlobalTransform();

	static SLP lp;
	lp.Reset();
	static vector<CEntity*> vecHits;
	vecHits.resize( 0 );
	bool bFinished = true;

	for( int i = 0; i < nTested; i++ )
	{
		auto pTestedEntity = pTested[i];
		auto bHitChannel = pTestedEntity->GetHitChannnel();
		auto bPlatformChannel = pTestedEntity->GetPlatformChannel();

		for( auto pManifold = pTestedEntity->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			bool bHit = bHitChannel[pEntity->GetHitType()] || pEntity->GetHitChannnel()[pTestedEntity->GetHitType()];
			bool bPlatform = !bHit && bPlatformChannel[pEntity->GetHitType()] && pGravity;
			if( bPlatform )
			{
				if( setOpenPlatforms.find( pEntity ) != setOpenPlatforms.end() )
					continue;
			}
			if( bHit || bPlatform )
			{
				if( pTestedEntity->HasHitFilter() )
				{
					if( !pTestedEntity->CanHit( pEntity ) || !pEntity->CanHit( pTestedEntity ) )
						continue;
				}
				CMatrix2D trans = pEntity->globalTransform;
				CVector2 entityOfs = pManifold->normal * -2;
				float fLength = entityOfs.Length();
				if( fLength == 0 )
					continue;
				while( fLength < 1 )
				{
					fLength *= 2;
					entityOfs = entityOfs * 2;
				}

				bFinished = false;
				trans.SetPosition( trans.GetPosition() - entityOfs );
				SRaycastResult result;
				if( !pTestedEntity->SweepTest( pEntity->Get_HitProxy(), trans, entityOfs, 0, &result ) )
					continue;
				if( pTestedEntity->HasHitFilter() )
				{
					if( !pTestedEntity->CheckImpact( pEntity, result, false ) || !pEntity->CheckImpact( pTestedEntity, result, true ) )
						continue;
				}
				if( bPlatform )
				{
					float f = result.normal.Dot( *pGravity );
					if( f < PLATFORM_THRESHOLD )
					{
						setOpenPlatforms.insert( pEntity );
						continue;
					}
				}

				/*fLength = fLength - result.fDist;
				entityOfs = result.normal * -fLength;
				trans.SetPosition( pEntity->globalTransform.GetPosition() - entityOfs );
				if( !pTestedEntity->SweepTest( pEntity->Get_HitProxy(), trans, entityOfs, 0, &result ) )
					continue;*/

				CVector2 finalOfs = entityOfs * ( ( fLength - result.fDist ) / fLength );
				lp.AddItem( finalOfs.x, finalOfs.y, finalOfs.Length2() );
				if( pEntity == pLandedEntity )
					*pLandedOfs = *pLandedOfs - finalOfs * 2;
				vecHits.push_back( pEntity );
				if( pVel )
				{
					auto& velocity = *pVel;
					auto v0 = pEntity->GetVelocity();
					auto norm = finalOfs;
					if( norm.Normalize() > 0 )
						velocity = HitVel( velocity, v0, norm, fFrac );
				}
			}
		}
		if( lp.bDead )
			return false;
	}

	auto pLevel = pCharacter->GetLevel();
	const CMatrix2D oldTransform = pCharacter->GetGlobalTransform();
	CMatrix2D newTransform = oldTransform;
	const CVector2 oldPos = oldTransform.GetPosition();

	static vector<CHitProxy*> hitResult;
	static vector<CEntity*> hitTestedEntity;
	static vector<SHitTestResult> hitTestResult;
	bSleep = false;

	if( lp.vecItems.size() )
	{
		while( 1 )
		{
			CVector2 ofs( 0, 0 );
			if( !lp.Nearest( ofs ) )
				break;
			CVector2 newPos = oldPos + ofs;
			newTransform.SetPosition( newPos );
			bool bHit = false;
			hitResult.resize( 0 );
			hitTestedEntity.resize( 0 );
			hitTestResult.resize( 0 );
			for( int i = 0; i < nTested; i++ )
			{
				auto pTestedEntity = pTested[i];
				auto bHitChannel = pTestedEntity->GetHitChannnel();
				auto bPlatformChannel = pTestedEntity->GetPlatformChannel();
				CMatrix2D matTestedNewTrans = testedTransform[i];
				matTestedNewTrans.SetPosition( matTestedNewTrans.GetPosition() + ofs );

				int32 nHitSize0 = hitResult.size();
				pLevel->GetHitTestMgr().HitTest( pTestedEntity->Get_HitProxy(), matTestedNewTrans, hitResult, &hitTestResult );
				hitTestedEntity.resize( hitResult.size() );
				for( int k = nHitSize0; k < hitResult.size(); k++ )
					hitTestedEntity[k] = pTestedEntity;

				bFinished = true;
				for( int i = nHitSize0; i < hitResult.size(); i++ )
				{
					auto pEntity = static_cast<CEntity*>( hitResult[i] );
					if( pEntity == pTestedEntity )
					{
						hitResult[i] = NULL;
						continue;
					}
					auto eHitType = pEntity->GetHitType();
					bool bHit = bHitChannel[pEntity->GetHitType()] || pEntity->GetHitChannnel()[pTestedEntity->GetHitType()];
					bool bPlatform = !bHit && bPlatformChannel[pEntity->GetHitType()] && pGravity;
					if( bPlatform )
					{
						if( setOpenPlatforms.find( pEntity ) != setOpenPlatforms.end() )
							continue;
						float l = -hitTestResult[i].normal.Dot( *pGravity );
						if( l < PLATFORM_THRESHOLD * hitTestResult[i].normal.Length() )
						{
							setOpenPlatforms.insert( pEntity );
							continue;
						}
					}
					if( hitTestResult[i].normal.Length2() > 0 && bHit || bPlatform )
					{
						if( pTestedEntity->HasHitFilter() )
						{
							if( !pTestedEntity->CanHit( pEntity ) || !pEntity->CanHit( pTestedEntity ) )
							{
								hitResult[i] = NULL;
								continue;
							}
						}

						bFinished = false;
						bool b = false;
						for( auto p1 : vecHits )
						{
							if( p1 == pEntity )
							{
								b = true;
								break;
							}
						}
						if( b )
							continue;
						bHit = true;
						vecHits.push_back( pEntity );
						CVector2 ofs1 = hitTestResult[i].normal;
						lp.AddItem( ofs1.x, ofs1.y, ofs1.Dot( ofs1 + ofs ) );
						if( lp.bDead )
							return false;
					}
				}
			}
			if( !bHit )
				break;
		}
	}
	else
	{
		hitResult.resize( 0 );
		hitTestedEntity.resize( 0 );
		hitTestResult.resize( 0 );
		for( int i = 0; i < nTested; i++ )
		{
			auto pTestedEntity = pTested[i];
			for( auto pManifold = pTestedEntity->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				if( pEntity == pTestedEntity )
					continue;
				if( pTestedEntity->HasHitFilter() )
				{
					if( !pTestedEntity->CanHit( pEntity ) || !pEntity->CanHit( pTestedEntity ) )
						continue;
				}

				hitResult.push_back( pEntity );
				hitTestedEntity.push_back( pTestedEntity );
				SHitTestResult r;
				r.normal = pManifold->normal * -1;
				hitTestResult.push_back( r );
			}
		}
	}

	if( !bFinished )
	{
		CVector2 dPos( 0, 0 );
		for( int i = 0; i < hitResult.size(); i++ )
		{
			if( !hitResult[i] )
				continue;
			auto pEntity = static_cast<CEntity*>( hitResult[i] );
			auto eHitType = pEntity->GetHitType();
			bool bHit = hitTestedEntity[i]->GetHitChannnel()[pEntity->GetHitType()] || pEntity->GetHitChannnel()[hitTestedEntity[i]->GetHitType()];
			bool bPlatform = !bHit && hitTestedEntity[i]->GetPlatformChannel()[pEntity->GetHitType()] && pGravity;
			if( bPlatform )
			{
				if( setOpenPlatforms.find( pEntity ) != setOpenPlatforms.end() )
					continue;
			}

			if( bPlatform ||bHit && hitTestResult[i].normal.Length2() > 0 )
			{
				auto norm = hitTestResult[i].normal;
				norm.Normalize();
				if( bPlatform )
				{
					float l = -norm.Dot( *pGravity );
					if( l < PLATFORM_THRESHOLD )
					{
						setOpenPlatforms.insert( pEntity );
						continue;
					}
				}
				dPos = dPos + norm;
			}
		}
		if( dPos.Normalize() <= 0 )
			dPos = CVector2( 0, 1 );
		CVector2 ofs[] = { { dPos.x, dPos.y }, { -dPos.y, dPos.x }, { dPos.y, -dPos.x },
		{ dPos.x - dPos.y, dPos.x + dPos.y }, { dPos.x + dPos.y, dPos.x - dPos.y } };
		CVector2 newPos0 = newTransform.GetPosition();
		CVector2 ofs0 = newPos0 - oldPos;
		for( int i = 0; i < ELEM_COUNT( ofs ); i++ )
		{
			CVector2 newPos = newPos0 + ofs[i] * 0.001f;
			newTransform.SetPosition( newPos );
			bFinished = true;

			for( int i = 0; i < nTested; i++ )
			{
				auto pTestedEntity = pTested[i];
				CMatrix2D matTestedNewTrans = testedTransform[i];
				matTestedNewTrans.SetPosition( matTestedNewTrans.GetPosition() + ofs0 + ofs[i] * 0.001f );

				hitResult.resize( 0 );
				hitTestResult.resize( 0 );
				pLevel->GetHitTestMgr().HitTest( pTestedEntity->Get_HitProxy(), newTransform, hitResult, &hitTestResult );
				for( int i = 0; i < hitResult.size(); i++ )
				{
					auto pEntity = static_cast<CEntity*>( hitResult[i] );
					if( pEntity == pTestedEntity )
						continue;
					auto eHitType = pEntity->GetHitType();
					auto bHitChannel = pTestedEntity->GetHitChannnel();
					auto bPlatformChannel = pTestedEntity->GetPlatformChannel();
					bool bHit = bHitChannel[pEntity->GetHitType()] || pEntity->GetHitChannnel()[pTestedEntity->GetHitType()];
					bool bPlatform = !bHit && bPlatformChannel[pEntity->GetHitType()] && pGravity;
					if( bPlatform )
					{
						if( setOpenPlatforms.find( pEntity ) != setOpenPlatforms.end() )
							continue;
						float l = -hitTestResult[i].normal.Dot( *pGravity );
						if( l < PLATFORM_THRESHOLD * hitTestResult[i].normal.Length() )
						{
							setOpenPlatforms.insert( pEntity );
							continue;
						}
					}
					if( pTestedEntity->HasHitFilter() )
					{
						if( !pTestedEntity->CanHit( pEntity ) || !pEntity->CanHit( pTestedEntity ) )
							continue;
					}
					if( bHit && hitTestResult[i].normal.Length2() > 0 )
					{
						bFinished = false;
						break;
					}
				}
				if( !bFinished )
					break;
			}

			if( bFinished )
				break;
		}
		if( !bFinished )
			return false;
	}

	pCharacter->SetPosition( newTransform.GetPosition() );
	for( int i = 0; i < nTested; i++ )
	{
		auto pTestedEntity = pTested[i];
		pTestedEntity->ForceUpdateTransform();
	}
	return true;
}

bool SCharacterMovementData::ResolvePenetration( CCharacter* pCharacter, const CVector2& dir, float fCos )
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

	CMatrix2D trans0 = pCharacter->GetGlobalTransform();
	vector<SPenetration> vecPenetrations;
	auto bHitChannel = pCharacter->GetHitChannnel();
	for( auto pManifold = pCharacter->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( bHitChannel[pEntity->GetHitType()] || pEntity->GetHitChannnel()[pCharacter->GetHitType()] )
		{
			if( pCharacter->HasHitFilter() )
			{
				if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
					continue;
			}
			SPenetration penetration;
			penetration.pEntity = pEntity;
			CMatrix2D trans = pEntity->globalTransform;
			CVector2 entityOfs = trans.GetPosition() - pEntity->GetLastPos();
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

			trans.SetPosition( trans.GetPosition() - entityOfs );
			SRaycastResult result;
			if( !pCharacter->SweepTest( pEntity->Get_HitProxy(), trans, entityOfs, 0, &result ) )
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
		CMatrix2D trans = trans0;
		trans.SetPosition( newPos );
		vector<CHitProxy*> hitResult;
		vector<SHitTestResult> hitTestResult;
		pCharacter->GetLevel()->GetHitTestMgr().HitTest( pCharacter->Get_HitProxy(), trans, hitResult, &hitTestResult );
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
			bool bHit = bHitChannel[eHitType] && pEntity->GetHitChannnel()[pCharacter->GetHitType()];
			if( bHit && hitTestResult[i].normal.Length2() > 0.05f * 0.05f )
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

CVector2 SCharacterMovementData::HitVel( const CVector2& vel, const CVector2 & v0, const CVector2 & norm, float fFrac )
{
	auto velocity = vel;
	auto v1 = velocity - v0;
	float f = norm.Dot( v1 );
	if( f < 0 )
	{
		CVector2 tangent( -norm.y, norm.x );
		float f1 = v1.Dot( tangent );
		f1 = Min( Max( 0.0f, f1 + fFrac * f ), f1 - fFrac * f );
		velocity = tangent * f1 + v0;
	}
	return velocity;
}

CEntity* SCharacterMovementData::DoSweepTest( CCharacter* pChar, const CMatrix2D& trans, const CVector2& sweepOfs, float fSideThreshold, SRaycastResult* pResult, bool bIgnoreInverseNormal, CEntity* pTested )
{
	if( !pTested )
		pTested = pChar;
	return pChar->HasHitFilter() ?
		pChar->GetLevel()->SweepTest( pTested, trans, sweepOfs, fSideThreshold, pResult, bIgnoreInverseNormal ) :
		pChar->GetLevel()->SweepTest( pTested->Get_HitProxy(), trans, sweepOfs, fSideThreshold, pChar->GetHitType(), pChar->GetHitChannnel(), pResult, bIgnoreInverseNormal );
}

CEntity* SCharacterMovementData::DoSweepTest1( CCharacter* pChar, int32 nTested, CEntity** pTested, CMatrix2D* matTested, const CVector2& sweepOfs, float fSideThreshold, CVector2* pGravityDir, SRaycastResult* pResult, bool bIgnoreInverseNormal )
{
	CEntity* pHit = NULL;
	float l = FLT_MAX;

	vector<SRaycastResult> tempResult;
	for( int i = 0; i < nTested; i++ )
	{
		int32 n0 = tempResult.size();
		auto bHitChannel = pTested[i]->GetHitChannnel();
		auto bPlatformChannel = pTested[i]->GetPlatformChannel();
		pChar->GetLevel()->GetHitTestMgr().SweepTest( pTested[i]->Get_HitProxy(), matTested[i], sweepOfs, fSideThreshold, tempResult, false );
		for( int i1 = tempResult.size() - 1; i1 >= n0; i1-- )
		{
			auto pOtherEntity = static_cast<CEntity*>( tempResult[i1].pHitProxy );
			bool bHit = bHitChannel[pOtherEntity->GetHitType()] || pOtherEntity->GetHitChannnel()[pTested[i]->GetHitType()];
			bool bPlatform = !bHit && bPlatformChannel[pOtherEntity->GetHitType()] && pGravityDir;

			tempResult[i1].nUser = bPlatform;
			bool bOK = bHit || bPlatform;
			if( bOK && pTested[i]->HasHitFilter() )
			{
				if( !pTested[i]->CanHit( pOtherEntity ) || !pOtherEntity->CanHit( pTested[i] ) )
					bOK = false;
			}
			if( bOK && bIgnoreInverseNormal && tempResult[i1].normal.Dot( sweepOfs ) >= 0 )
				bOK = false;
			if( !bOK )
			{
				tempResult[i1] = tempResult.back();
				tempResult.resize( tempResult.size() - 1 );
			}
		}
	}

	std::sort( tempResult.begin(), tempResult.end(), [] ( const SRaycastResult& lhs, const SRaycastResult& rhs )
	{
		if( lhs.fDist < rhs.fDist )
			return true;
		if( lhs.fDist > rhs.fDist )
			return false;
		if( lhs.nUser < rhs.nUser )
			return true;
		if( lhs.nUser > rhs.nUser )
			return false;
		return lhs.pHitProxy < rhs.pHitProxy;
	} );
	for( auto& item : tempResult )
	{
		auto pOtherEntity = static_cast<CEntity*>( item.pHitProxy );
		bool bPlatform = item.nUser;
		if( bPlatform )
		{
			if( setOpenPlatforms.find( pOtherEntity ) != setOpenPlatforms.end() )
				continue;
			float f = -item.normal.Dot( *pGravityDir );
			if( f < PLATFORM_THRESHOLD )
			{
				setOpenPlatforms.insert( pOtherEntity );
				continue;
			}
		}
		if( pResult )
			*pResult = item;
		return pOtherEntity;
	}
	return NULL;
}

void SCharacterFlyData::UpdateMove( CCharacter* pCharacter, const CVector2& moveAxis, float fStopBaseSpd2 )
{
	CVector2 landedEntityOfs( 0, 0 );
	if( pLandedEntity )
	{
		if( pLandedEntity->GetLevel() != pCharacter->GetLevel() )
		{
			pLandedEntity = NULL;
			bSleep = false;
		}
		else if( lastLandedEntityTransform != pLandedEntity->GetGlobalTransform() )
		{
			CVector2 oldPos = pCharacter->GetPosition();
			CVector2 localPos = lastLandedEntityTransform.MulTVector2PosNoScale( oldPos );
			CVector2 newPos = pLandedEntity->GetGlobalTransform().MulVector2Pos( localPos );
			landedEntityOfs = newPos - oldPos;
			lastLandedEntityTransform = pLandedEntity->GetGlobalTransform();
			bSleep = false;
		}
	}

	if( !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}
	pCharacter->GetGlobalTransform().SetPosition( pCharacter->GetPosition() );
	pCharacter->GetLevel()->GetHitTestMgr().Update( pCharacter );

	CVector2 baseVel( 0, 0 );
	if( pLandedEntity )
	{
		/*auto pChunk = SafeCast<CChunkObject>( pLandedEntity->GetParentEntity() );
		if( pChunk )
			baseVel = pChunk->GetSurfaceVel( pCharacter->GetPosition() );*/
	}
	CVector2 moveOfs;
	float fDeltaTime = pCharacter->GetLevel()->GetElapsedTimePerTick();
	if( nState == eState_Rolling )
	{
		float fTime = pCharacter->GetLevel()->GetElapsedTimePerTick();
		float fNewRollTime = Min( fRollTime + fTime, fRollMaxTime );
		float fDist0 = ( 2 - fRollTime / fRollMaxTime ) * fRollTime * fRollMaxSpeed * 0.5f;
		float fDist1 = ( 2 - fNewRollTime / fRollMaxTime ) * fNewRollTime * fRollMaxSpeed * 0.5f;
		CVector2 rollOfsLeft = rollDir * ( fRollMaxTime * fRollMaxSpeed * 0.5f - fDist0 );
		CVector2 moveOfs = rollDir * ( fDist1 - fDist0 );
		fRollTime = fNewRollTime;

		/*if( bApplyExtraGravity )
		{
			float fExtraGravity = 2 * Max( 0.0f, pCharacter->globalTransform.GetPosition().y -
				( CMyLevel::GetInst()->GetBound().GetBottom() - CMyLevel::GetInst()->GetHighGravityHeight() ) );
			moveOfs = moveOfs + CVector2( 0, -fExtraGravity * fTime );
		}*/

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

				/*if( bApplyExtraGravity )
				{
					float fExtraGravity = 2 * Max( 0.0f, pCharacter->globalTransform.GetPosition().y -
						( CMyLevel::GetInst()->GetBound().GetBottom() - CMyLevel::GetInst()->GetHighGravityHeight() ) );
					moveOfs = moveOfs + CVector2( 0, -fExtraGravity * fDeltaTime );
				}*/

				moveOfs = moveOfs + landedEntityOfs;
			}
			CVector2 velocity = moveOfs;
			TryMove( pCharacter, moveOfs, velocity, hits );
			finalMoveAxis = velocity;
			finalMoveAxis.Normalize();
		}
	}

	//pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );
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
	float fDeltaTime = pCharacter->GetLevel()->GetElapsedTimePerTick();
	if( nState == eState_Knockback )
	{
		float fKnockbackTime0 = fKnockbackTime;
		fKnockbackTime = Max( 0.0f, fKnockbackTime - fDeltaTime );
		nDir = 0;
		vecKnockback = vecKnockback * ( fKnockbackTime / fKnockbackTime0 );
		if( fKnockbackTime <= 0 )
			nState = eState_Normal;
	}

	CVector2 landedEntityOfs( 0, 0 );
	if( pLandedEntity )
	{
		if( pLandedEntity->GetLevel() != pCharacter->GetLevel() )
		{
			pLandedEntity = NULL;
			FallOff();
			bSleep = false;
		}
		else if( lastLandedEntityTransform != pLandedEntity->GetGlobalTransform() )
		{
			landedEntityOfs = OnLandedEntityMoved( pCharacter, lastLandedEntityTransform, pLandedEntity->GetGlobalTransform() );
			bSleep = false;
		}
	}

	if( !ResolvePenetration( pCharacter, &velocity, 0, pLandedEntity, &landedEntityOfs ) )
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
		fJumpHoldingTime += pCharacter->GetLevel()->GetElapsedTimePerTick();
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
			/*auto pChunk = SafeCast<CChunkObject>( pLandedEntity->GetParentEntity() );
			if( pChunk )
			{
				CVector2 baseVel = pChunk->GetSurfaceVel( pCharacter->GetPosition() );
				fSpeedTarget = baseVel.Dot( tangent );
			}*/
		}
		else
		{
			tangent = CVector2( -gravity.y, gravity.x );
			tangent.Normalize();
			moveDir = tangent * ( tangent.x > 0 ? 1 : -1 ) * nDir;
		}
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

		dPos = dPos + landedEntityOfs;
	}

	pCharacter->SetPosition( pCharacter->GetPosition() );
	pCharacter->GetLevel()->GetHitTestMgr().Update( pCharacter );
	CVector2 v0 = velocity;
	SRaycastResult res[3];
	TryMove( pCharacter, dPos, velocity, res );
	if( nState != eState_Hooked )
	{
		bool bPreLandedEntity = pLandedEntity != NULL;
		FindFloor( pCharacter );

		if( nState == eState_JumpHolding && !nJumpCheck )
		{
			if( pLandedEntity )
				nJumpCheck = 2;
		}
	}
	else
		nState = eState_Normal;

	//pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );
}

void SCharacterWalkData::Jump( CCharacter * pCharacter )
{
	if( nState == eState_Normal )
	{
		nState = eState_JumpHolding;
		if( pLandedEntity )
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
			if( nJumpCheck < 2 )
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

void SCharacterWalkData::FallOff()
{
	nState = eState_Normal;
	fJumpHoldingTime = 0;
}

CVector2 SCharacterWalkData::OnLandedEntityMoved( CCharacter* pCharacter, const CMatrix2D & oldTrans, const CMatrix2D & newTrans )
{
	CVector2 oldPos = pCharacter->GetPosition();
	CVector2 localPos = oldTrans.MulTVector2PosNoScale( oldPos );
	CVector2 newPos = newTrans.MulVector2Pos( localPos );

	CVector2 landVelocity = ( newPos - oldPos ) / pCharacter->GetLevel()->GetElapsedTimePerTick() - velocity;

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
	lastLandedEntityTransform = pLandedEntity->GetGlobalTransform();
	return ofs;
}

void SCharacterWalkData::FindFloor( CCharacter * pCharacter )
{
	CMatrix2D trans = pCharacter->GetGlobalTransform();
	trans.SetPosition( pCharacter->GetPosition() );
	CVector2 dir = gravity;
	dir.Normalize();
	CVector2 ofs = dir * fFindFloorDist;
	SRaycastResult result;
	auto pNewLandedEntity = SafeCast<CCharacter>( pCharacter->GetLevel()->SweepTest( pCharacter->Get_HitProxy(), trans, ofs, MOVE_SIDE_THRESHOLD,
		pCharacter->GetHitType(), pCharacter->GetHitChannnel(), &result ) );
	if( pNewLandedEntity && velocity.Dot( result.normal ) < 1.0f && result.normal.Dot( dir ) < -0.5f )
	{
		pLandedEntity = pNewLandedEntity;
		pCharacter->SetPosition( pCharacter->GetPosition() + dir * result.fDist );
		groundNorm = result.normal;
		velocity = velocity - groundNorm * velocity.Dot( groundNorm );
		lastLandedEntityTransform = pLandedEntity->GetGlobalTransform();
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

	float fTime = pCharacter->GetLevel()->GetElapsedTimePerTick();

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
	//pCharacter->SetPosition( PosTrunc( pCharacter->GetPosition() ) );

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
	bool bHasAnyCollision = pCharacter->HasAnyCollision();

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

	float fTime = pCharacter->GetLevel()->GetElapsedTimePerTick();
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
	bool bHasAnyCollision = pCharacter->HasAnyCollision();

	if( bHasAnyCollision && !ResolvePenetration( pCharacter ) )
	{
		pCharacter->Crush();
		return;
	}

	CVector2 lastVelocity = pCharacter->GetVelocity();
	CVector2 curVelocity = lastVelocity;
	float l = curVelocity.Normalize();
	curVelocity = curVelocity * Max( l - fMaxAcc * pCharacter->GetLevel()->GetElapsedTimePerTick(), 0.0f );
	curVelocity = curVelocity + acc * pCharacter->GetLevel()->GetElapsedTimePerTick();

	CVector2 dPos = ( lastVelocity + curVelocity ) * ( 0.5f * pCharacter->GetLevel()->GetElapsedTimePerTick() );
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

	float deltaTime = pCharacter->GetLevel()->GetElapsedTimePerTick();
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

		/*auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject && pBlockObject->GetHitType() != eEntityHitType_WorldStatic )
		{
			bHitWall = true;
			break;
		}*/
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

	float fTime = pCharacter->GetLevel()->GetElapsedTimePerTick();

	if( bHitSurface )
	{
		CVector2 moveDir = CVector2( normal.y, -normal.x ) * nDir;

		CVector2 vel = moveDir * fSpeed;
		CVector2 ofs = vel * fTime;
		SRaycastResult result[6];
		TryMove( pCharacter, ofs, vel, result );
		CVector2 vel1 = normal * -fSpeed;
		CVector2 ofs1 = vel1 * fTime;
		pCharacter->SetPosition( pCharacter->GetPosition() );
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

	float deltaTime = pCharacter->GetLevel()->GetElapsedTimePerTick();
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
	float deltaTime = pCharacter->GetLevel()->GetElapsedTimePerTick();
	bHitWall = false;
	bHitWall1 = false;
	fDamage = 0;
	for( auto pManifold = pCharacter->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pCharacter->HasHitFilter() )
		{
			if( !pCharacter->CanHit( pEntity ) || !pEntity->CanHit( pCharacter ) )
			{
				bHitWall = true;
				bHitWall1 = true;
				continue;
			}
		}

		/*auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject )
		{
			bHitWall = true;
			if( pBlockObject->GetHitType() != eEntityHitType_WorldStatic )
			{
				auto pChunkObject = SafeCast<CChunkObject>( pBlockObject->GetParentEntity() );
				if( pChunkObject && pChunkObject->GetChunk()->nMoveType )
					bHitWall1 = true;
			}
		}*/
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

			/*auto pBlockObject = SafeCast<CBlockObject>( pEntity );
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
			}*/
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
	float fTime = pCharacters[nCharacters - 1]->GetLevel()->GetElapsedTimePerTick();
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

float CCharacterMoveUtil::Stretch( CCharacter * pCharacter, uint8 nDir, float fMaxDeltaLen )
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
		if( pCharacter->GetLevel()->SweepTest( pRect, pCharacter->GetGlobalTransform(), pCharacter->GetGlobalTransform().MulVector2Dir( ofs[nDir] )
			* fMaxDeltaLen, 0, pCharacter->GetHitType(), pCharacter->GetHitChannnel(), &result ) )
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
	pCharacter->GetLevel()->GetHitTestMgr().Update( pCharacter );
	return fDist;
}

float CCharacterMoveUtil::StretchEx( CCharacter* pCharacter, uint8 nDir, float fMinLen, float fMaxLen, float fMoveDist )
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
	if( pCharacter->GetLevel()->SweepTest( pRect, pCharacter->GetGlobalTransform(), pCharacter->GetGlobalTransform().MulVector2Dir( ofs[nDir] )
		* fMoveDist, 0, pCharacter->GetHitType(), pCharacter->GetHitChannnel(), &result, true ) )
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
	pCharacter->GetLevel()->GetHitTestMgr().Update( pCharacter );
	return fDist;
}
