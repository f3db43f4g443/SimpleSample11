#include "stdafx.h"
#include "Bullets.h"
#include "BlockBuffs.h"
#include "Entities/Barrage.h"
#include "Common/ResourceManager.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "Block.h"
#include "Entities/Door.h"
#include "MyLevel.h"
#include "Render/Rope2D.h"
#include "Common/Rand.h"
#include "Entities/Enemies/Lv1Enemies.h"
#include "Entities/EnemyCharacters.h"
#include "Entities/BlockItems/BlockItemsLv2.h"

void CBomb::OnAddedToStage()
{
	CBullet::OnAddedToStage();
	m_pExp->SetParentEntity( NULL );
}

void CBomb::OnHit( CEntity * pEntity )
{
	if( SafeCast<CBlockObject>( pEntity ) )
	{
		if( m_bExplodeOnHitBlock )
			Explode();
	}
	else if( SafeCast<CChunkObject>( pEntity ) )
	{
		if( m_bExplodeOnHitBlock )
			Explode();
	}
	else if( SafeCast<CCharacter>( pEntity ) )
	{
		if( m_bExplodeOnHitChar )
			Explode();
	}
	else
	{
		if( m_bExplodeOnHitWorld )
			Explode();
	}
	m_pExp = NULL;

	CBullet::OnHit( pEntity );
}

void CBomb::Kill()
{
	if( m_bExplodeOnHitWorld )
		Explode();
	CBullet::Kill();
}

void CBomb::Explode()
{
	if( m_pExp )
	{
		auto pParent = SafeCast<CBarrage>( GetParentEntity() );
		if( pParent )
		{
			m_pExp->SetPosition( globalTransform.GetPosition() );
			m_pExp->SetRotation( atan2( globalTransform.m10, globalTransform.m00 ) );
		}
		else
		{
			m_pExp->SetPosition( GetPosition() );
			m_pExp->SetRotation( GetRotation() );
		}
		auto pExplosion = SafeCast<CExplosion>( m_pExp.GetPtr() );
		if( pExplosion )
			pExplosion->SetCreator( m_pCreator );
		m_pExp->SetParentBeforeEntity( pParent ? (CEntity*)pParent : this );
		m_pExp = NULL;
	}
}

void CWaterFall::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	m_nLifeLeft = m_nLife;
	m_fY0 = GetPosition().y;
	UpdateEft();
	UpdateAnim( m_nLife * GetStage()->GetElapsedTimePerTick() );
}

void CWaterFall::OnTickAfterHitTest()
{
	auto pEft = static_cast<CRopeObject2D*>( GetRenderObject() );
	SCharacterMovementData movementData;
	movementData.ResolvePenetration( this, CVector2( 0, -1 ), 0.5f );
	float fDist = CCharacterMoveUtil::StretchEx( this, 3, m_fMinLen, m_fMaxLen, m_fFall, movementData.bHitChannel );
	if( m_nState == 0 && fDist <= 0 )
	{
		pEft->GetInstanceData()->GetData().isEmitting = false;
		m_nState = 1;
	}
	if( m_nState > 0 )
	{
		if( m_nLife1 )
			m_nLife1--;
		if( m_nLifeLeft )
		{
			m_nLifeLeft--;
			if( !m_nLifeLeft )
			{
				SetParentEntity( NULL );
				return;
			}
		}
	}
	UpdateEft();

	if( m_nState == 0 || m_nLife1 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

			CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
			if( pCharacter && pCharacter != pPlayer && !pCharacter->IsIgnoreBullet() )
			{
				if( !m_nDamage1 )
					continue;
				if( !SafeCast<CBullet>( pCharacter ) )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage1;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pManifold->hitPoint;
					context.hitDir = CVector2( 0, -1 );
					context.nHitType = -1;
					pCharacter->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				continue;
			}

			if( pPlayer && pPlayer->CanBeHit() && pEntity == pPlayer->GetCore() )
			{
				if( !m_nDamage && m_fKnockback <= 0 )
					continue;
				if( m_nDamage )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pManifold->hitPoint;
					context.hitDir = CVector2( 0, -1 );
					context.nHitType = -1;
					pPlayer->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				if( m_fKnockback > 0 && pPlayer->CanKnockback() )
				{
					CVector2 norm( 1, 0 );
					norm = ( pPlayer->globalTransform.GetPosition() - GetPosition() ).Dot( norm ) > 0 ? norm : norm * -1;
					pPlayer->Knockback( norm * m_fKnockback );
				}
			}
		}
	}

	CCharacter::OnTickAfterHitTest();
}

void CWaterFall::UpdateEft()
{
	auto pEft = static_cast<CRopeObject2D*>( GetRenderObject() );
	auto pRect = static_cast<SHitProxyPolygon*>( Get_HitProxy() );
	if( pRect->nVertices != 4 )
		return;
	float fMinX = FLT_MAX, fMaxX = -FLT_MAX, fMinY = FLT_MAX, fMaxY = -FLT_MAX;
	for( int i = 0; i < 4; i++ )
	{
		fMinX = Min( fMinX, pRect->vertices[i].x );
		fMaxX = Max( fMaxX, pRect->vertices[i].x );
		fMinY = Min( fMinY, pRect->vertices[i].y );
		fMaxY = Max( fMaxY, pRect->vertices[i].y );
	}
	float fLen = fMaxY - fMinY;
	float t = m_nLifeLeft * 1.0f / m_nLife;
	if( fLen > 32 )
	{
		pEft->SetDataCount( 4 );
		pEft->SetData( 0, CVector2( 0, 0 ), m_fWidth, CVector2( 0, 0 ), CVector2( 1, 2 * Max( 1 - ( m_fY0 - GetPosition().y ) / ( fLen * 0.125f ), 0.0f ) ) );
		pEft->SetData( 1, CVector2( 0, -16 ), m_fWidth, CVector2( 0, 16 / m_fTexYTileLen ), CVector2( 1, 1 ) );
		pEft->SetData( 2, CVector2( 0, -( fLen - 16 ) ), m_fWidth, CVector2( 0, ( fLen - 16 ) / m_fTexYTileLen ), CVector2( 1, 1 ) );
		pEft->SetData( 3, CVector2( 0, -fLen ), m_fWidth, CVector2( 0, fLen / m_fTexYTileLen ), CVector2( 1, 0 ) );
	}
	else
	{
		pEft->SetDataCount( 3 );
		pEft->SetData( 0, CVector2( 0, 0 ), m_fWidth, CVector2( 0, 0 ), CVector2( 1, 2 * Max( 1 - ( m_fY0 - GetPosition().y ) / ( fLen * 0.125f ), 0.0f ) ) );
		pEft->SetData( 1, CVector2( 0, -fLen * 0.5f / 2 ), m_fWidth, CVector2( 0, fLen * 0.5f / m_fTexYTileLen ), CVector2( 1, 1 ) );
		pEft->SetData( 2, CVector2( 0, -fLen ), m_fWidth, CVector2( 0, fLen / m_fTexYTileLen ), CVector2( 1, 0 ) );

	}
	pEft->SetTransformDirty();
}

void CWaterFall1::OnRemovedFromStage()
{
	m_pEntity = NULL;
	CCharacter::OnRemovedFromStage();
}

void CWaterFall1::Set( CEntity * pEntity, const CVector2& ofs )
{
	m_pEntity = pEntity;
	m_ofs = ofs;
	UpdateEftPos();
	UpdateEftParams();
}

void CWaterFall1::Kill()
{
	m_bKilled = true;
	m_pEntity = NULL;
}

void CWaterFall1::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( m_pEntity && !m_pEntity->GetStage() )
		Kill();
	UpdateEftPos();

	float fLen = y + m_fFadeInLen;
	m_fFadeInPos = Min( fLen, m_fFadeInPos + m_fFadeInSpeed * GetStage()->GetElapsedTimePerTick() );
	if( m_bKilled )
	{
		m_fFadeOutPos = Min( fLen, m_fFadeOutPos + m_fFadeOutSpeed * GetStage()->GetElapsedTimePerTick() );
		if( m_fFadeOutPos >= fLen )
		{
			SetParentEntity( NULL );
			return;
		}
	}

	float fHitBegin = m_fFadeInPos - m_fFadeInLen;
	float fHitEnd = m_fFadeOutPos;
	if( fHitBegin > fHitEnd )
	{
		float yBegin = y - fHitBegin;
		float yEnd = y - fHitEnd;
		CRectangle rect( x - m_fHitWidth * 0.5f, yBegin, m_fHitWidth, yEnd - yBegin );
		CPlayer* pPlayer = GetStage()->GetPlayer();
		SHitProxyPolygon hitProxy( rect );
		hitProxy.CalcBoundGrid( CMatrix2D::GetIdentity() );
		SHitTestResult result;
		if( pPlayer->HitTest( &hitProxy, CMatrix2D::GetIdentity(), &result ) )
		{
			if( pPlayer->CanKnockback() )
			{
				CVector2 norm = result.normal * -1;
				norm.y = Min( norm.y, 0.0f );
				if( norm.Normalize() > 0 )
					pPlayer->Knockback( norm );
			}
		}
	}

	UpdateEftParams();
}

void CWaterFall1::UpdateEftPos()
{
	auto pEft = static_cast<CRopeObject2D*>( GetRenderObject() );
	auto& data0 = pEft->GetData().data[0];
	auto& data1 = pEft->GetData().data[1];
	if( m_pEntity )
		SetPosition( m_pEntity->globalTransform.MulVector2Pos( m_ofs ) );
	data1.center = CVector2( 0, -y );
	data0.fWidth = data1.fWidth = m_fWidth;
	data1.tex0.y = data1.tex1.y = y / m_fTexYTileLen;
	data0.tex1.x = data1.tex1.x = data1.tex0.y;
}

void CWaterFall1::UpdateEftParams()
{
	auto pEft = static_cast<CRopeObject2D*>( GetRenderObject() );
	for( int i = 0; i < 2; i++ )
	{
		auto pParam = pEft->GetParam( i );
		if( !pParam )
			return;
		pParam->x = m_fTexYTileLen / m_fFadeInLen;
		pParam->y = m_fTexYTileLen / m_fFadeOutLen;
		pParam->z = m_fFadeInPos / m_fTexYTileLen;
		pParam->w = m_fFadeOutPos / m_fTexYTileLen;
	}
}

void CPulse::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );

	switch( m_nBoundType )
	{
	case 0:
		m_bound = CMyLevel::GetInst()->GetLargeBound();
		break;
	case 1:
		m_bound = CMyLevel::GetInst()->GetBound();
		break;
	default:
	{
		CRectangle rect1 = CMyLevel::GetInst()->GetBound();
		CRectangle rect2 = CMyLevel::GetInst()->GetLargeBound();
		m_bound = CRectangle( rect2.x, rect1.y, rect2.width, rect1.height );
		break;
	}
	}

	m_pos0 = GetPosition();
	UpdateRenderObject();
}

void CPulse::Kill()
{
	if( m_pDeathEffect )
	{
		CMatrix2D mat = globalTransform;
		CMatrix2D mat1;
		mat1.Transform( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		( mat1 * mat ).Decompose( m_pDeathEffect->x, m_pDeathEffect->y, m_pDeathEffect->r, m_pDeathEffect->s );
		m_pDeathEffect->SetParentBeforeEntity( this );
		m_pDeathEffect->SetState( 2 );
	}
	m_bKilled = true;
}

void CPulse::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
	if( !m_bKilled )
	{
		CVector2 newVelocity = m_velocity + m_acc * GetStage()->GetElapsedTimePerTick();
		SetPosition( GetPosition() + ( m_velocity + newVelocity ) * ( GetStage()->GetElapsedTimePerTick() * 0.5f ) );
		m_velocity = newVelocity;
	}

	CVector2 d = m_pos0 - GetPosition();
	float l2 = d.Length2();
	float fMaxLen = !m_bKilled ? m_fMaxLen : 0;
	if( l2 > fMaxLen * fMaxLen )
	{
		float l = sqrt( l2 );
		float l1 = l;
		if( m_bKilled )
		{
			l1 = Max( l1 - m_velocity.Length() * GetStage()->GetElapsedTimePerTick(), 0.0f );
			if( l1 <= 0 )
			{
				SetParentEntity( NULL );
				return;
			}
		}
		else
			l1 = Min( l1, fMaxLen );
		d = d * ( l1 / l );
		m_pos0 = GetPosition() + d;
	}

	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
			Kill();
	}
}

void CPulse::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( !m_bKilled )
	{
		if( !m_bound.Contains( globalTransform.GetPosition() ) )
		{
			CVector2 globalPos = globalTransform.GetPosition();
			globalPos.x = Min( m_bound.GetRight(), Max( m_bound.x, globalPos.x ) );
			globalPos.y = Min( m_bound.GetBottom(), Max( m_bound.y, globalPos.y ) );
			globalTransform.SetPosition( globalPos );
			Kill();
		}
	}

	CPlayer* pPlayer = GetStage()->GetPlayer();
	CVector2 beginCenter = m_pos0;
	CVector2 endCenter = GetPosition();
	auto dCenter = endCenter - beginCenter;
	if( m_fHitWidth > 0 && !m_nHitCDLeft && dCenter.Length2() > 0.01f )
	{
		switch( m_nType )
		{
		case 0:
			if( pPlayer && pPlayer->CanBeHit() )
			{
				SHitProxyPolygon polygon;
				polygon.nVertices = 4;
				dCenter.Normalize();
				dCenter = CVector2( dCenter.y, -dCenter.x ) * m_fHitWidth * 0.5f;

				polygon.vertices[0] = beginCenter - dCenter;
				polygon.vertices[1] = beginCenter + dCenter;
				polygon.vertices[2] = endCenter + dCenter;
				polygon.vertices[3] = endCenter - dCenter;
				polygon.CalcNormals();

				CMatrix2D mat;
				mat.Identity();

				GetStage()->GetHitTestMgr().CalcBound( &polygon, mat );
				SHitTestResult hitResult;
				if( pPlayer->GetCore()->HitTest( &polygon, mat, &hitResult ) )
				{
					if( m_nDamage )
					{
						CCharacter::SDamageContext context;
						context.nDamage = m_nDamage;
						context.nType = 0;
						context.nSourceType = 0;
						context.hitPos = hitResult.hitPoint1;
						context.hitDir = endCenter - beginCenter;
						context.nHitType = -1;
						pPlayer->Damage( context );
						if( m_pDmgEft )
							m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
					}
					if( m_fKnockback > 0 && pPlayer->CanKnockback() )
					{
						CVector2 norm = hitResult.normal;
						norm.Normalize();
						pPlayer->Knockback( norm * -1 );
					}
					OnHit( pPlayer, hitResult.hitPoint1 );
				}
			}
			break;
		case 1:
		case 2:
		case 3:
		{
			SHitProxyPolygon polygon;
			polygon.nVertices = 4;
			dCenter.Normalize();
			CVector2 d = dCenter;
			dCenter = CVector2( dCenter.y, -dCenter.x ) * m_fHitWidth * 0.5f;

			polygon.vertices[0] = beginCenter - dCenter;
			polygon.vertices[1] = beginCenter + dCenter;
			polygon.vertices[2] = beginCenter + dCenter + d;
			polygon.vertices[3] = beginCenter - dCenter + d;
			polygon.CalcNormals();
			CMatrix2D mat;
			mat.Identity();

			vector<CReference<CEntity> > result;
			vector<SRaycastResult> testResult;
			GetStage()->MultiSweepTest( &polygon, mat, endCenter - beginCenter - d, result, &testResult );

			for( auto& hitResult : testResult )
			{
				CEntity* pEntity = static_cast<CEntity*>( hitResult.pHitProxy );
				if( !pEntity->GetStage() )
					continue;

				if( m_pCreator && pEntity->IsCreator( m_pCreator ) )
					continue;

				if( m_nType == 1 )
				{
					CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
					if( pEnemy && !pEnemy->IsIgnoreBullet() )
					{
						if( m_pCreator && pEnemy->IsOwner( m_pCreator ) )
							continue;
						CReference<CEntity> pTempRef = pEntity;

						CCharacter::SDamageContext context;
						context.nDamage = m_nDamage;
						context.nType = 0;
						context.nSourceType = 0;
						context.hitPos = hitResult.hitPoint;
						context.hitDir = endCenter - beginCenter;
						context.nHitType = -1;
						pEnemy->Damage( context );
						if( m_pDmgEft )
							m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );

						OnHit( pEnemy, hitResult.hitPoint );
						continue;
					}
				}
				else
				{
					if( m_nType == 3 )
					{
						CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
						if( pBlockObject )
						{
							if( pBlockObject->GetBlock()->eBlockType != eBlockType_Block )
								continue;
							auto pChunk = pBlockObject->GetBlock()->pOwner->pChunkObject;
							if( pChunk == m_pCreator )
								continue;
							OnHit( pBlockObject, hitResult.hitPoint );
						}
					}

					CDoor* pDoor = SafeCast<CDoor>( pEntity );
					if( pDoor )
					{
						pDoor->OpenForFrame( 5 );
						continue;
					}

					CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
					if( pCharacter && pCharacter != pPlayer && !pCharacter->IsIgnoreBullet() )
					{
						if( !m_nDamage2 )
							continue;
						if( m_pCreator )
						{
							auto pEnemy = SafeCast<CEnemy>( pEntity );
							if( pEnemy && pEnemy->IsOwner( m_pCreator ) )
								continue;
						}
						if( !SafeCast<CBullet>( pCharacter ) )
						{
							CCharacter::SDamageContext context;
							context.nDamage = m_nDamage2;
							context.nType = 0;
							context.nSourceType = 0;
							context.hitPos = hitResult.hitPoint;
							context.hitDir = endCenter - beginCenter;
							context.nHitType = -1;
							pCharacter->Damage( context );
							if( m_pDmgEft )
								m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
						}
						OnHit( pCharacter, hitResult.hitPoint );
						continue;
					}

					if( pPlayer && pPlayer->CanBeHit() && pEntity == pPlayer->GetCore() )
					{
						if( !m_nDamage1 && m_fKnockback <= 0 )
							continue;
						if( m_nDamage1 )
						{
							CCharacter::SDamageContext context;
							context.nDamage = m_nDamage1;
							context.nType = 0;
							context.nSourceType = 0;
							context.hitPos = hitResult.hitPoint;
							context.hitDir = endCenter - beginCenter;
							context.nHitType = -1;
							pPlayer->Damage( context );
							if( m_pDmgEft )
								m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
						}
						if( m_fKnockback > 0 && pPlayer->CanKnockback() )
						{
							CVector2 norm( d.y, -d.x );
							norm = ( pPlayer->globalTransform.GetPosition() - beginCenter ).Dot( norm ) > 0 ? norm : norm * -1;
							pPlayer->Knockback( norm );
						}
						OnHit( pPlayer, hitResult.hitPoint );
					}
				}
			}

			break;
		}
		}

		if( !GetStage() )
			return;
	}

	UpdateRenderObject();
}

void CPulse::UpdateRenderObject()
{
	auto pEft = static_cast<CRopeObject2D*>( GetRenderObject() );
	auto& data0 = pEft->GetData().data[0];
	auto& data1 = pEft->GetData().data[1];
	data0.center = m_pos0 - GetPosition();
	data1.center = CVector2( 0, 0 );
	data0.fWidth = data1.fWidth = m_fWidth;
	data1.tex0.y = data1.tex1.y = data0.center.Length() / m_fTexYTileLen;
	data0.tex1.x = data1.tex1.x = data1.tex0.y;
	pEft->SetTransformDirty();
}

void CBulletWithBlockBuff::OnAddedToStage()
{
	CBullet::OnAddedToStage();
	m_pBlockBuff = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBlockBuff.c_str() );
}

void CBulletWithBlockBuff::OnHit( CEntity * pEntity )
{
	auto pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject && pBlockObject->GetStage() )
	{
		CBlockBuff::AddBuff( m_pBlockBuff, pBlockObject, &m_context );
	}
}

void CExplosionWithBlockBuff::OnAddedToStage()
{
	CExplosion::OnAddedToStage();
}

void CExplosionWithBlockBuff::OnHit( CEntity * pEntity )
{
	auto pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject && pBlockObject->GetStage() )
	{
		CBlockBuff::AddBuff( m_strBlockBuff, pBlockObject, &m_context );
	}
}

void CExplosionKnockback::OnHit( CEntity * pEntity )
{
	CCharacter* pChar = SafeCast<CCharacter>( pEntity );
	if( pChar )
	{
		CVector2 dir = pChar->globalTransform.GetPosition() - globalTransform.GetPosition();
		dir.Normalize();
		pChar->Knockback( dir * m_fKnockbackStrength );
	}
	CExplosion::OnHit( pEntity );
}

void CPlayerBulletMultiHit::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( m_bKilled )
	{
		m_fDeathTime -= GetStage()->GetElapsedTimePerTick();
		if( m_fDeathTime <= 0 )
		{
			SetParentEntity( NULL );
		}
		return;
	}

	if( !m_bound.Contains( globalTransform.GetPosition() ) )
	{
		CVector2 globalPos = globalTransform.GetPosition();
		globalPos.x = Min( m_bound.GetRight(), Max( m_bound.x, globalPos.x ) );
		globalPos.y = Min( m_bound.GetBottom(), Max( m_bound.y, globalPos.y ) );
		globalTransform.SetPosition( globalPos );
		Kill();
		return;
	}

	if( m_nHitCDLeft )
		m_nHitCDLeft--;
	if( m_nHitCDLeft )
		return;

	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity == m_pCreator )
			continue;
		CReference<CEntity> pTempRef = pEntity;

		CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
		if( pEnemy && !pEnemy->IsIgnoreBullet() )
		{
			if( m_pCreator && pEnemy->IsOwner( m_pCreator ) )
				continue;
			CReference<CEntity> pTempRef = pEntity;

			CCharacter::SDamageContext context;
			context.nDamage = m_nDamage;
			context.nType = 0;
			context.nSourceType = 0;
			context.hitPos = pManifold->hitPoint;
			context.hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
			context.nHitType = -1;
			pEnemy->Damage( context );
			if( m_pDmgEft )
				m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );

			OnHit( pEnemy );
			m_nHit++;
			m_nHitCDLeft = m_nHitCD;
			if( m_nHit >= m_nDamage2 )
				Kill();
			return;
		}

		CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject && pBlockObject->GetBlock()->eBlockType == eBlockType_Block )
		{
			auto pChunkObject = pBlockObject->GetBlock()->pOwner->pChunkObject;
			if( pChunkObject == m_pCreator )
				continue;
			CVector2 hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
			hitDir.Normalize();
			CReference<CEntity> pTempRef = pEntity;
			CChunkObject::SDamageContext dmgContext = { m_nDamage1, 0, eDamageSourceType_Bullet, hitDir * 8 };
			pChunkObject->Damage( dmgContext );
			OnHit( pBlockObject );
			if( m_bMultiHitBlock )
			{
				m_nHit++;
				m_nHitCDLeft = m_nHitCD;
				if( m_nHit >= m_nDamage2 )
					Kill();
			}
			else
				Kill();
			return;
		}

		CDoor* pDoor = SafeCast<CDoor>( pEntity );
		if( pDoor && !pDoor->IsOpen() )
		{
			auto pChunkObject = SafeCast<CChunkObject>( pDoor->GetParentEntity() );
			if( pChunkObject )
			{
				if( pChunkObject == m_pCreator )
					continue;
				CVector2 hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
				hitDir.Normalize();
				CReference<CEntity> pTempRef = pEntity;
				CChunkObject::SDamageContext dmgContext = { m_nDamage1, 0, eDamageSourceType_Bullet, hitDir * 8 };
				pChunkObject->Damage( dmgContext );
				OnHit( pChunkObject );
				if( m_bMultiHitBlock )
				{
					m_nHit++;
					m_nHitCDLeft = m_nHitCD;
					if( m_nHit >= m_nDamage2 )
						Kill();
				}
				else
					Kill();
				return;
			}
		}
	}
}

void CThrowObj::OnTickBeforeHitTest()
{
	CEnemy::OnTickBeforeHitTest();
	if( SafeCast<CCharacter>( GetParentEntity() ) )
		return;
	m_nCurLife++;
	SetPosition( GetPosition() + m_velocity * GetStage()->GetElapsedTimePerTick() );
}

void CThrowObj::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	if( SafeCast<CCharacter>( GetParentEntity() ) )
		return;
	if( m_nCurLife >= m_nLife1 )
	{
		Kill();
		return;
	}

	CRectangle rect = CMyLevel::GetInst()->GetBoundWithLvBarrier();
	CRectangle rect0;
	Get_HitProxy()->CalcBound( globalTransform, rect0 );
	if( !rect.Contains( rect0 ) )
	{
		Kill();
		return;
	}

	if( m_nCurLife >= m_nLife )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				Kill();
				return;
			}
		}
	}
}

void CWaterSplash::OnAddedToStage()
{
	uint32 nFrame = m_nFrameOffset * SRand::Inst().Rand( 0u, m_nFrameRows );
	auto pImg = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	pImg->SetFrames( pImg->GetFrameBegin() + nFrame, pImg->GetFrameEnd() + nFrame, pImg->GetFramesPerSec() );
	m_nDeathFrameBegin += nFrame;
	m_nDeathFrameEnd += nFrame;
	CBullet::OnAddedToStage();
}

void CWaterSplash::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	if( m_bKilled )
	{
		m_fDeathTime -= GetStage()->GetElapsedTimePerTick();
		if( m_fDeathTime <= 0 )
		{
			SetParentEntity( NULL );
		}
		return;
	}

	if( !m_bound.Contains( globalTransform.GetPosition() ) )
	{
		CVector2 globalPos = globalTransform.GetPosition();
		globalPos.x = Min( m_bound.GetRight(), Max( m_bound.x, globalPos.x ) );
		globalPos.y = Min( m_bound.GetBottom(), Max( m_bound.y, globalPos.y ) );
		globalTransform.SetPosition( globalPos );
		Kill();
		return;
	}

	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity == m_pCreator )
			continue;
		CReference<CEntity> pTempRef = pEntity;

		CMaggot* pMaggot = SafeCast<CMaggot>( pEntity );
		if( pMaggot )
		{
			pMaggot->Morph();
			OnHit( pMaggot );
			Kill();
			return;
		}

		if( m_nDamage )
		{
			CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
			if( pEnemy && !pEnemy->IsIgnoreBullet() )
			{
				if( m_pCreator && pEnemy->IsOwner( m_pCreator ) )
					continue;
				if( m_nDamage )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pManifold->hitPoint;
					context.hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
					context.nHitType = -1;
					pEnemy->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				OnHit( pEnemy );
				Kill();
				return;
			}
		}

		CDoor* pDoor = SafeCast<CDoor>( pEntity );
		if( pDoor && !pDoor->IsOpen() )
			pDoor->OpenForFrame( 3 );

		if( m_nDamage1 || m_fKnockback > 0 )
		{
			CPlayer* pPlayer = SafeCast<CPlayer>( pEntity );
			if( pPlayer && ( pPlayer->CanBeHit() || pPlayer->CanKnockback() ) )
			{
				if( m_fKnockback > 0 )
				{
					CVector2 dir = GetVelocity();
					dir.Normalize();
					pPlayer->Knockback( dir * m_fKnockback );
				}
				if( m_nDamage1 )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage1;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pManifold->hitPoint;
					context.hitDir = CVector2( globalTransform.m00, globalTransform.m10 );
					context.nHitType = -1;
					pPlayer->Damage( context );
					if( m_pDmgEft )
						m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( context );
				}
				OnHit( pPlayer );
				Kill();
				return;
			}
		}
	}
}

void CSawBlade::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
		auto pPlayer = SafeCast<CPlayer>( pEntity );
		if( pPlayer && ( pPlayer->CanBeHit() || pPlayer->CanKnockback() ) )
		{
			CVector2 knockback = pManifold->normal;
			knockback.Normalize();
			pPlayer->Knockback( knockback );
			CCharacter::SDamageContext context;
			context.nDamage = m_nDamage;
			context.nType = 0;
			context.nSourceType = 0;
			context.hitPos = pManifold->hitPoint;
			context.hitDir = knockback;
			context.nHitType = -1;
			pPlayer->Damage( context );
			continue;
		}
		auto pEnemy = SafeCast<CEnemyCharacter>( pEntity );
		if( pEnemy )
		{
			pEnemy->Crush();
			continue;
		}
	}
}

void CFlood::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	if( m_nType == 1 )
		m_fHeight = CMyLevel::GetInst()->GetBound().GetBottom();
	UpdateImage();
}

void CFlood::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	m_hit.clear();
}

CRectangle CFlood::GetRect()
{
	auto bound = CMyLevel::GetInst()->GetBound();
	if( m_nType == 0 )
	{
		bound.height = m_fHeight;
		bound.height = Max( 0.0f, bound.height - m_fHitDepth );
	}
	else if( m_nType == 1 )
	{
		bound.SetTop( m_fHeight );
		bound.SetTop( Min( bound.GetBottom(), bound.y + m_fHitDepth ) );
	}
	return bound;
}

void CFlood::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );

	if( m_fHeight != m_fTargetHeight )
	{
		if( m_fHeight < m_fTargetHeight )
			m_fHeight = Min( m_fTargetHeight, m_fHeight + m_fSpeed * GetStage()->GetElapsedTimePerTick() );
		else
			m_fHeight = Max( m_fTargetHeight, m_fHeight - m_fSpeed * GetStage()->GetElapsedTimePerTick() );
	}

	auto rect = UpdateImage();
	if( rect.height <= 0 && m_bAutoKill )
	{
		SetParentEntity( NULL );
		return;
	}
	if( m_nType == 0 )
	{
		rect.height = Max( 0.0f, rect.height - m_fHitDepth );
	}
	else if( m_nType == 1 )
	{
		rect.SetTop( Min( rect.GetBottom(), rect.y + m_fHitDepth ) );
	}
	if( rect.height > 0 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer && rect.Contains( pPlayer->GetPosition() ) )
		{
			if( m_hit.find( pPlayer ) == m_hit.end() )
			{
				if( m_nDamage )
				{
					CCharacter::SDamageContext context;
					context.nDamage = m_nDamage;
					context.nType = 0;
					context.nSourceType = 0;
					context.hitPos = pPlayer->GetPosition();
					context.hitDir = CVector2( 0, 0 );
					context.nHitType = -1;
					pPlayer->Damage( context );
				}
				if( m_fKnockback )
				{
					CVector2 dir;
					if( m_nType == 0 )
						dir = CVector2( 0, 1 );
					else
						dir = CVector2( 0, -1 );
					pPlayer->Knockback( dir * m_fKnockback );
				}
				if( pPlayer->GetStage() )
					m_hit[pPlayer] = m_nHitInterval;
			}
		}
	}

	if( m_nHitInterval )
	{
		for( auto itr = m_hit.begin(); itr != m_hit.end(); )
		{
			itr->second--;
			if( !itr->second )
				itr = m_hit.erase( itr );
			else
				itr++;
		}
	}
}

CRectangle CFlood::UpdateImage()
{
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	auto bound = CMyLevel::GetInst()->GetBound();
	if( m_nType == 0 )
		bound.height = m_fHeight;
	else if( m_nType == 1 )
		bound.SetTop( m_fHeight );
	if( bound.height <= 0 )
	{
		pImage->bVisible = false;
		return bound;
	}

	pImage->bVisible = true;
	pImage->SetRect( bound );
	pImage->SetBoundDirty();
	auto& param = *pImage->GetParam();
	if( m_nType == 0 )
	{
		param.x = m_fHeight;
	}
	else if( m_nType == 1 )
	{
		param.x = -m_fHeight;
	}
	return bound;
}

void CBloodPower::OnTickBeforeHitTest()
{
	if( m_pTarget )
	{
		if( !m_pTarget->GetStage() )
			m_pTarget = NULL;
		else
		{
			auto p = SafeCast<CBloodConsumer>( m_pTarget.GetPtr() );
			if( p->GetBloodCount() >= p->GetMaxCount() )
				m_pTarget = NULL;
		}
	}
	if( !m_bKilled && m_pTarget )
	{
		CCharacter::OnTickBeforeHitTest();
		auto hitRect = SafeCast<CBloodConsumer>( m_pTarget.GetPtr() )->GetHitRect();
		CVector2 d = m_pTarget->globalTransform.MulTVector2PosNoScale( GetPosition() );
		CVector2 d1( d.x - Max( Min( d.x, hitRect.GetRight() ), hitRect.x ), d.y - Max( Min( d.y, hitRect.GetBottom() ), hitRect.y ) );
		float l = d1.Length();
		float v = m_velocity.Length();
		float l1 = v * GetStage()->GetElapsedTimePerTick();
		if( l1 >= l )
		{
			SetPosition( m_pTarget->globalTransform.MulVector2Pos( d1 ) );
			SafeCast<CBloodConsumer>( m_pTarget.GetPtr() )->SetBloodCount( SafeCast<CBloodConsumer>( m_pTarget.GetPtr() )->GetBloodCount() + 1 );
			ForceUpdateTransform();
			Kill();
			return;
		}
		auto d2 = d1 / l;
		SetPosition( m_pTarget->globalTransform.MulVector2Pos( d - d2 ) );
		m_velocity = m_pTarget->globalTransform.MulVector2Dir( d2 * v );
		if( m_bTangentDir )
			SetRotation( atan2( m_velocity.y, m_velocity.x ) );
		else
			SetRotation( r + m_fAngularVelocity * GetStage()->GetElapsedTimePerTick() );
		return;
	}
	CBullet::OnTickBeforeHitTest();
}

void CBloodPower::OnTickAfterHitTest()
{
	if( !m_bKilled && !m_pTarget )
	{
		if( m_nCheckTimeLeft )
			m_nCheckTimeLeft--;
		if( !m_nCheckTimeLeft )
		{
			m_nCheckTimeLeft = m_nCheckTime;
			static vector<CReference<CEntity> > vecTemp;
			vecTemp.resize( 0 );
			GetStage()->MultiPick( globalTransform.GetPosition(), vecTemp );
			for( CEntity* pEntity : vecTemp )
			{
				auto p = SafeCast<CBloodConsumer>( pEntity );
				if( !p || p->GetType() != 0 )
					continue;
				if( p->GetBloodCount() >= p->GetMaxCount() )
					continue;
				m_pTarget = p;
				break;
			}
		}
	}
	CBullet::OnTickAfterHitTest();
}