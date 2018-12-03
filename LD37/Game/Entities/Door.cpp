#include "stdafx.h"
#include "Door.h"
#include "Stage.h"
#include "Character.h"
#include "Player.h"
#include "Block.h"
#include "Render/Image2D.h"
#include "Entities/Blocks/Lv2/SpecialLv2.h"

CDoor::CDoor( const SClassCreateContext& context )
	: CEntity( context ), m_bOpen( false ), m_nOpenFrame( 0 ), m_onTick( this, &CDoor::OnTick )
{
	SET_BASEOBJECT_ID( CDoor );
}

void CDoor::OnAddedToStage()
{
	GetStage()->RegisterStageEvent( eStageEvent_PostHitTest, &m_onTick );

	CMultiFrameImage2D* pImage = static_cast<CMultiFrameImage2D*>( GetRenderObject() );
	pImage->SetPlaySpeed( 0, false );
	uint16 nParamCount;
	auto pParam = pImage->GetParam( nParamCount );
	if( nParamCount >= 3 )
	{
		pImage->SetFrameParams( 0, 0 );
		auto pHouse = SafeCast<CHouse>( GetParentEntity() );
		if( pHouse )
		{
			auto pParam = pImage->GetParam();
			auto pParam1 = pHouse->GetParam( 0 );
			pParam[0].w = pParam1[0].w;
			pParam[1] = pParam1[1];
			pParam[2] = pParam1[2];
		}
	}
}

void CDoor::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDoor::OnTick()
{
	bool bOpen = m_nOpenFrame > 0;
	m_nOpenFrame = Max( m_nOpenFrame - 1, 0 );
	if( !bOpen )
	{
		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pCharacter = SafeCast<CCharacter>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
			if( pCharacter && pCharacter->CanOpenDoor() )
			{
				bOpen = true;
				break;
			}
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
