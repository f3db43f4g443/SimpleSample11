#include "stdafx.h"
#include "Lightning.h"
#include "Stage.h"
#include "Player.h"
#include "Render/Rope2D.h"
#include "Entities/Door.h"
#include "Enemy.h"
#include "Bullet.h"

void CLightning::OnAddedToStage()
{
	SetAutoUpdateAnim( true );
	UpdateRenderObject();
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CLightning::OnRemovedFromStage()
{
	m_pCreator = NULL;
	Set( NULL, NULL, CVector2( 0, 0 ), CVector2( 0, 0 ), -1, -1 );
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	m_onHit = nullptr;
}

void CLightning::Set( CEntity * pBegin, CEntity * pEnd, const CVector2 & begin, const CVector2 & end, int16 nBeginTransIndex, int16 nEndTransIndex )
{
	if( m_pBegin != pBegin )
	{
		if( m_onBeginRemoved.IsRegistered() )
			m_onBeginRemoved.Unregister();
		m_pBegin = pBegin;
		if( m_pBegin )
			m_pBegin->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onBeginRemoved );
	}
	if( m_pEnd != pEnd )
	{
		if( m_onEndRemoved.IsRegistered() )
			m_onEndRemoved.Unregister();
		m_pEnd = pEnd;
		if( m_pEnd )
			m_pEnd->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onEndRemoved );
	}
	m_begin = begin;
	m_end = end;
	m_nBeginTransIndex = nBeginTransIndex;
	m_nEndTransIndex = nEndTransIndex;
	m_bSet = true;
	UpdateRenderObject();
}

void CLightning::OnTick()
{
	if( m_nLife )
	{
		m_nLife--;
		if( m_nLife <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
	}

	DEFINE_TEMP_REF_THIS();
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );

	float fHitWidth = m_nHitFrameCount ? 0 : m_fHitWidth;
	if( m_nHitFrameCount )
	{
		if( m_bBurst )
		{
			m_nFrame++;
			if( m_nFrame >= m_nHitFrameBegin + m_nHitFrameCount || m_nFrame < m_nHitFrameBegin )
				fHitWidth = 0;
			else
				fHitWidth = m_fHitWidth + ( m_nFrame - m_nHitFrameBegin ) * m_fHitWidthPerFrame;
		}
		else
		{
			m_nFrame = Min( m_nFrame + 1, m_nHitFrameBegin + m_nHitFrameCount - 1 );
			if( m_nFrame >= m_nHitFrameBegin )
				fHitWidth = m_fHitWidth + ( m_nFrame - m_nHitFrameBegin ) * m_fHitWidthPerFrame;
		}
	}

	CVector2 beginCenter, endCenter;
	{
		const CMatrix2D& worldMat = m_pBegin ? ( m_nBeginTransIndex >= 0 ? m_pBegin->GetTransform( m_nBeginTransIndex ) : m_pBegin->globalTransform ) : globalTransform;
		beginCenter = worldMat.MulVector2Pos( m_begin );
	}
	{
		const CMatrix2D& worldMat = m_pEnd ? ( m_nEndTransIndex >= 0 ? m_pEnd->GetTransform( m_nEndTransIndex ) : m_pEnd->globalTransform ) : globalTransform;
		endCenter = worldMat.MulVector2Pos( m_end );
	}

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( m_bIsBeam )
	{
		CVector2 dCenter = endCenter - beginCenter;
		float l = dCenter.Normalize();
		CVector2 dir = dCenter;
		CVector2 begin = beginCenter + dir * fHitWidth * 0.5f;
		vector<CReference<CEntity> > result;
		vector<SRaycastResult> raycastResult;
		GetStage()->MultiRaycast( begin, endCenter, result, &raycastResult );

		CEntity* pHitEntity = NULL;
		CReference<CEntity> pDamageEntity = NULL;
		SRaycastResult* pResult = NULL;

		for( auto& item : raycastResult )
		{
			auto pEntity = static_cast<CEntity*>( item.pHitProxy );
			if( pEntity == m_pCreator )
				continue;
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				CBlockObject* pBlockObject = SafeCast<CBlockObject>( pEntity );
				if( pBlockObject )
				{
					if( pBlockObject->GetBlock()->eBlockType != eBlockType_Block )
						continue;
					auto pChunk = pBlockObject->GetBlock()->pOwner->pChunkObject;
					if( pChunk == m_pCreator )
						continue;
					if( m_nType == 0 )
					{
						if( !pPlayer || !pPlayer->IsHiding() || pChunk != pPlayer->GetCurRoom() )
							continue;
					}
					pHitEntity = pBlockObject;
					pDamageEntity = pChunk;
				}
				else
					pHitEntity = pEntity;
				pResult = &item;
				break;
			}

			if( m_nType <= 1 )
			{
				CDoor* pDoor = SafeCast<CDoor>( pEntity );
				if( pDoor && !pDoor->IsOpen() )
				{
					auto pChunk = SafeCast<CChunkObject>( pDoor->GetParentEntity() );
					if( pChunk )
					{
						if( pChunk == m_pCreator )
							continue;
						if( m_nType == 0 )
						{
							if( !pPlayer || !pPlayer->IsHiding() || pChunk != pPlayer->GetCurRoom() )
								continue;
						}
						pHitEntity = pChunk;
						pDamageEntity = pChunk;
						pResult = &item;
					}
					break;
				}
			}
		}

		if( fHitWidth > 0 && !m_nHitCDLeft )
		{
			if( pDamageEntity )
			{
				auto pChunk = SafeCast<CChunkObject>( pDamageEntity.GetPtr() );
				if( pChunk )
				{
					uint32 nDmg = m_nType == 0 ? m_nDamage1 : m_nDamage;
					if( nDmg )
					{
						CVector2 hit = m_nType == 0 ? CVector2( 0, 0 ) : dir * 8;
						CChunkObject::SDamageContext dmgContext = { nDmg, 0, eDamageSourceType_Beam, hit };
						pChunk->Damage( dmgContext );
					}
				}
			}
			if( pHitEntity )
			{
				endCenter = begin + dir * pResult->fDist;

				OnHit( pHitEntity );
			}
		}
		else if( pHitEntity )
			endCenter = begin + dir * pResult->fDist;

		m_beamEnd = globalTransform.MulTVector2PosNoScale( endCenter );
		m_fBeamLen = pResult ? pResult->fDist : l;
		m_bIsBeamInited = true;
	}

	if( !GetStage() )
		return;
	auto dCenter = endCenter - beginCenter;
	if( fHitWidth > 0 && !m_nHitCDLeft && dCenter.Length2() > 0.01f )
	{
		switch( m_nType )
		{
		case 0:
			if( pPlayer && pPlayer->CanBeHit() )
			{
				SHitProxyPolygon polygon;
				polygon.nVertices = 4;
				dCenter.Normalize();
				dCenter = CVector2( dCenter.y, -dCenter.x ) * fHitWidth * 0.5f;

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
					OnHit( pPlayer );
				}
			}
			break;
		case 1:
		case 2:
		{
			SHitProxyPolygon polygon;
			polygon.nVertices = 4;
			dCenter.Normalize();
			CVector2 d = dCenter;
			dCenter = CVector2( dCenter.y, -dCenter.x ) * fHitWidth * 0.5f;

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

				if( pEntity == m_pCreator )
					continue;

				if( m_nType == 1 )
				{
					CEnemy* pEnemy = SafeCast<CEnemy>( pEntity );
					if( pEnemy )
					{
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

						OnHit( pEnemy );
						continue;
					}
				}
				else
				{
					CDoor* pDoor = SafeCast<CDoor>( pEntity );
					if( pDoor )
					{
						pDoor->OpenForFrame( 5 );
						continue;
					}

					CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
					if( pCharacter && pCharacter != pPlayer )
					{
						if( !m_nDamage2 )
							continue;
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
						OnHit( pCharacter );
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
						OnHit( pPlayer );
						return;
					}
				}
			}

			break;
		}
		}

		if( !GetStage() )
			return;
	}

	if( fHitWidth > 0 )
	{
		if( !m_nHitCDLeft )
			m_nHitCDLeft = m_nHitCD;
		if( m_nHitCDLeft )
			m_nHitCDLeft--;
	}
	UpdateRenderObject();
}

void CLightning::UpdateRenderObject()
{
	CRopeObject2D* pRope = static_cast<CRopeObject2D*>( GetRenderObject() );
	if( !pRope )
		return;
	if( !m_bSet || m_bIsBeam && !m_bIsBeamInited )
	{
		pRope->bVisible = false;
		if( m_pBeginEft )
			m_pBeginEft->bVisible = false;
		if( m_pEndEft )
			m_pEndEft->bVisible = false;
		return;
	}

	pRope->bVisible = true;
	pRope->SetTransformDirty();
	bool bBegin = m_fBeginLen > 0;
	bool bEnd = m_fEndLen > 0;
	auto& data = pRope->GetData();
	data.SetDataCount( 2 + ( bBegin ? 1 : 0 ) + ( bEnd ? 1 : 0 ) );
	auto& begin = data.data[bBegin ? 1 : 0];
	auto& end = data.data[bBegin ? 2 : 1];
	begin.center = m_begin;
	begin.fWidth = m_fWidth;
	begin.pRefObj = m_pBegin;
	begin.nRefTransformIndex = m_nBeginTransIndex;

	end.fWidth = m_fWidth;
	if( m_bIsBeam )
	{
		end.center = m_beamEnd;
		end.pRefObj = NULL;
		end.nRefTransformIndex = -1;
	}
	else
	{
		end.center = m_end;
		end.pRefObj = m_pEnd;
		end.nRefTransformIndex = m_nEndTransIndex;
	}
	if( m_fTexYTileLen > 0 )
		end.tex0.y = end.tex1.y = ( m_bIsBeam ? m_fBeamLen : ( end.center - begin.center ).Length() ) / m_fTexYTileLen;

	if( bBegin )
	{
		auto& begin0 = data.data[0];
		begin0.center = CVector2( -m_fEndLen, 0 );
		begin0.fWidth = begin.fWidth;
		begin0.tex0.x = begin.tex0.x;
		begin0.tex1.x = begin.tex1.x;
		begin0.tex0.y = begin0.tex1.y = 0;
		begin0.bBegin = true;
		begin.tex0.y = begin.tex1.y = m_fBeginTexLen;
	}
	if( bEnd )
	{
		auto& end0 = data.data.back();
		end0.center = CVector2( m_fEndLen, 0 );
		end0.fWidth = end.fWidth;
		end0.tex0.x = end.tex0.x;
		end0.tex1.x = end.tex1.x;
		end0.tex0.y = end0.tex1.y = 1;
		end0.bEnd = true;
		end.tex0.y = end.tex1.y = 1 - m_fEndTexLen;
	}

	if( m_pBeginEft )
	{
		m_pBeginEft->bVisible = true;
		if( m_pBeginEft->GetParent() != pRope )
		{
			m_pBeginEft->RemoveThis();
			pRope->AddChild( m_pBeginEft );
		}
		m_pBeginEft->SetTransformIndex( 0 );
	}
	if( m_pEndEft )
	{
		m_pEndEft->bVisible = true;
		if( m_pEndEft->GetParent() != pRope )
		{
			m_pEndEft->RemoveThis();
			pRope->AddChild( m_pEndEft );
		}
		m_pEndEft->SetTransformIndex( 1 );
	}
}

void CLightning::OnBeginRemoved()
{
	m_begin = m_pBegin->globalTransform.MulVector2Pos( m_begin );
	m_begin = globalTransform.MulTVector2PosNoScale( m_begin );
	if( m_onBeginRemoved.IsRegistered() )
		m_onBeginRemoved.Unregister();
	m_pBegin = NULL;
	UpdateRenderObject();

	if( m_bAutoRemove )
		SetParentEntity( NULL );
}

void CLightning::OnEndRemoved()
{
	m_end = m_pEnd->globalTransform.MulVector2Pos( m_end );
	m_end = globalTransform.MulTVector2PosNoScale( m_end );
	if( m_onEndRemoved.IsRegistered() )
		m_onEndRemoved.Unregister();
	m_pEnd = NULL;
	UpdateRenderObject();

	if( m_bAutoRemove )
		SetParentEntity( NULL );
}
