#include "stdafx.h"
#include "Door.h"
#include "Stage.h"
#include "Character.h"
#include "Player.h"
#include "Enemy.h"
#include "Block.h"
#include "Render/Image2D.h"

void CDoor::OnAddedToStage()
{
	GetStage()->RegisterStageEvent( eStageEvent_PostHitTest, &m_onTick );

	CMultiFrameImage2D* pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	pImage->SetPlaySpeed( 0, false );
}

void CDoor::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDoor::OnTick()
{
	bool bOpen = false;
	for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
	{
		auto pPlayer = SafeCast<CPlayer>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pPlayer )
		{
			bOpen = true;
			break;
		}
		auto pEnemy = SafeCast<CEnemy>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
		if( pEnemy )
		{
			bOpen = true;
			break;
		}
	}

	if( bOpen != m_bOpen )
	{
		m_bOpen = bOpen;
		CMultiFrameImage2D* pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
		pImage->SetPlaySpeed( bOpen ? 1 : -1, false );

		CChunkObject* pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pChunkObject && pPlayer && pPlayer->GetCurRoom() == pChunkObject )
		{
			pPlayer->RefreshCurRoomUI();
		}
	}
}
