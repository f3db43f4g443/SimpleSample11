#include "stdafx.h"
#include "EnemyCharacters.h"
#include "MyLevel.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"

void CEnemyCharacter::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_flyData.bHitChannel[eEntityHitType_Platform] = false;
	m_nState = 0;
	float angle = SRand::Inst().Rand( -PI, PI );
	m_curMoveDir = CVector2( cos( angle ), sin( angle ) );
}

void CEnemyCharacter::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	CVector2 prePos = GetPosition();
	CReference<CEntity> pTemp = this;
	uint8 newAnimState = 0;
	if( m_nState == 1 )
	{
		CVector2 fixedVelocity = m_walkData.UpdateMove( this, m_curMoveDir.x > 0 ? 1 : -1, false );
		if( !GetStage() )
			return;

		auto levelBound = CMyLevel::GetInst()->GetBound();
		if( x < levelBound.x || x > levelBound.GetRight() || y < levelBound.y )
		{
			Kill();
			return;
		}

		if( fixedVelocity.x == 0 )
			m_curMoveDir.x = -m_curMoveDir.x;
		for( int i = 0; i < 3; i++ )
		{
			auto pEntity = static_cast<CEntity*>( m_walkData.hits[i].pHitProxy );
			if( !pEntity )
				break;
			auto pBlockObject = SafeCast<CBlockObject>( pEntity );
			if( pBlockObject && pBlockObject->GetBlock()->pOwner->bIsRoom )
			{
				m_flyData.Reset();
				m_flyData.SetLandedEntity( pBlockObject->GetBlock()->pOwner->pChunkObject );
				m_nState = 2;
			}
		}
	}
	else if( m_nState == 2 )
	{
		do
		{
			if( m_flyData.pLandedEntity && !m_flyData.pLandedEntity->GetStage() )
				m_flyData.SetLandedEntity( NULL );
			if( m_flyData.pLandedEntity )
			{
				bool bBlock = false;
				for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
				{
					auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
					if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform )
					{
						bBlock = true;
						break;
					}
				}

				auto pChunk = SafeCast<CChunkObject>( m_flyData.pLandedEntity.GetPtr() )->GetChunk();
				CRectangle roomBound( pChunk->pos.x, pChunk->pos.y, pChunk->nWidth * CMyLevel::GetInst()->GetBlockSize(), pChunk->nHeight * CMyLevel::GetInst()->GetBlockSize() );
				if( !bBlock && roomBound.Contains( GetPosition() ) )
				{
					m_flyData.bHitChannel[eEntityHitType_Platform] = false;
					m_nState = 0;
					break;
				}

				m_flyData.fMoveSpeed = m_fClimbSpeed;
				m_curMoveDir = roomBound.GetCenter() - globalTransform.GetPosition();
				m_curMoveDir.Normalize();
				m_flyData.UpdateMoveNoBlocking( this, m_curMoveDir );
			}
			if( !m_flyData.pLandedEntity )
			{
				m_walkData.Reset();
				m_nState = 1;
			}
		} while( 0 );
	}
	else
	{
		if( !m_flyData.pLandedEntity )
		{
			for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				auto pChunkObject = SafeCast<CChunkObject>( pEntity );
				if( pChunkObject )
				{
					m_flyData.SetLandedEntity( pChunkObject );
					break;
				}
			}
		}
		if( m_flyData.pLandedEntity && !m_flyData.pLandedEntity->GetStage() )
			m_flyData.SetLandedEntity( NULL );
		if( m_flyData.pLandedEntity )
		{
			m_flyData.fMoveSpeed = m_fOrigFlySpeed;

			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer && pPlayer->GetCurRoom() == m_flyData.pLandedEntity )
			{
				m_curMoveDir = pPlayer->globalTransform.GetPosition() - globalTransform.GetPosition();
				m_curMoveDir.Normalize();
			}

			m_flyData.UpdateMove( this, m_curMoveDir );
			if( !GetStage() )
				return;

			if( m_flyData.finalMoveAxis.x == 0 )
				m_curMoveDir.x = -m_curMoveDir.x;
			if( m_flyData.finalMoveAxis.y == 0 )
				m_curMoveDir.y = -m_curMoveDir.y;
		}
		if( !m_flyData.pLandedEntity )
		{
			m_flyData.bHitChannel[eEntityHitType_Platform] = true;
			m_walkData.Reset();
			m_nState = 1;
		}
	}

	CVector2 curPos = GetPosition();
	m_velocity = ( curPos - prePos ) / GetStage()->GetElapsedTimePerTick();

	CChunkObject* pCurRoom = NULL;
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		CChunkObject* pChunkObject = SafeCast<CChunkObject>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pChunkObject && pChunkObject->GetChunk()->bIsRoom )
		{
			CRectangle rect( pChunkObject->GetChunk()->pos.x, pChunkObject->GetChunk()->pos.y,
				pChunkObject->GetChunk()->nWidth * CMyLevel::GetInst()->GetBlockSize(),
				pChunkObject->GetChunk()->nHeight * CMyLevel::GetInst()->GetBlockSize() );
			if( rect.Contains( GetPosition() ) )
				pCurRoom = pChunkObject;
		}
	}


	if( m_curMoveDir.x != 0 || m_curMoveDir.y != 0 )
		newAnimState = 1;
	if( m_nState == 1 )
	{
		newAnimState += m_curMoveDir.x > 0 ? 0 : 3;
	}
	else
	{
		newAnimState += m_curMoveDir.x > 0 ? 0 : 3;
	}

	if( newAnimState != m_nAnimState )
	{
		auto pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		switch( newAnimState )
		{
		case 0:
			pImage->SetFrames( 0, 1, 0 );
			break;
		case 1:
			pImage->SetFrames( 1, 7, 12 );
			break;
		case 2:
			pImage->SetFrames( 11, 16, 9.9f );
			break;
		case 3:
			pImage->SetFrames( 16, 17, 0 );
			break;
		case 4:
			pImage->SetFrames( 17, 23, 12 );
			break;
		case 5:
			pImage->SetFrames( 27, 32, 9.9f );
			break;
		default:
			break;
		}
		m_nAnimState = newAnimState;
	}
}

bool CEnemyCharacter::CanTriggerItem()
{
	return m_nState == 0;
}