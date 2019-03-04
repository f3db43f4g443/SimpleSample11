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
	m_bFirstTick = true;
}

void CDoor::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDoor::OnTick()
{
	if( m_bFirstTick )
	{
		m_bFirstTick = false;
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
			else
			{
				auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
				if( !pChunkObject )
					return;
				CCargoAutoColor* p = NULL;
				auto pRoot = pChunkObject->GetDecoratorRoot();
				if( !pRoot )
					pRoot = pChunkObject;
				for( auto pChild = pRoot->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
				{
					p = SafeCast<CCargoAutoColor>( pChild );
					if( p )
						break;
				}
				if( p )
				{
					auto pParam = pImage->GetParam();
					auto pColor = p->GetColors();
					pParam[0].w = pColor[0].z;
					pParam[1] = CVector4( pColor[0].x, pColor[1].x, pColor[2].x, pColor[1].z );
					pParam[2] = CVector4( pColor[0].y, pColor[1].y, pColor[2].y, pColor[2].z );
				}
			}
		}
	}

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
