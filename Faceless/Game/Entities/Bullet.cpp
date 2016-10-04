#include "stdafx.h"
#include "Bullet.h"
#include "Stage.h"
#include "Face.h"

void CBullet::OnAddedToStage()
{
	auto pStage = GetStage();
	auto pFace = pStage->GetRoot()->GetChildByName_Fast<CFace>( "" );
	m_pFace = pFace;
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CBullet::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

void CBullet::TickBeforeHitTest()
{
	if( !GetStage() )
		return;

	SetPosition( GetPosition() + m_velocity * GetStage()->GetElapsedTimePerTick() );
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CBullet::TickAfterHitTest()
{
	CVector2 worldPos = globalTransform.GetPosition();
	if( !m_pFace->GetKillBound().Contains( worldPos ) )
	{
		m_bActive = false;
		SetParentEntity( NULL );
		return;
	}

	if( m_bActive )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
			auto pOrgan = SafeCast<COrgan>( pEntity );
			if( pOrgan )
			{
				pOrgan->Damage( m_nDmg );
				m_bActive = false;
				SetParentEntity( NULL );
				return;
			}
		}
	}

	if( GetStage() )
		GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}
