#include "stdafx.h"
#include "PlayerDebuff.h"
#include "Stage.h"
#include "Player.h"

CPlayerDebuffLayer::CPlayerDebuffLayer()
	: m_pDebuffs( NULL )
	, m_tickBeforeHitTest( this, &CPlayerDebuffLayer::OnTickBeforeHitTest )
	, m_tickAfterHitTest( this, &CPlayerDebuffLayer::OnTickAfterHitTest )
{
}

CPlayerDebuffLayer::~CPlayerDebuffLayer()
{
	while( m_pDebuffs )
		Remove( m_pDebuffs );
}

CPlayerDebuff* CPlayerDebuffLayer::Add( CPlayerDebuff* pDebuff )
{
	CReference<CPlayerDebuff> pTemp = pDebuff;
	if( pDebuff->m_pDebuffLayer )
		return NULL;

	CPlayerDebuff* pAddTo = NULL;
	if( pDebuff->IsStackable() )
	{
		for( auto pDebuff1 = m_pDebuffs; pDebuff1; pDebuff1 = pDebuff1->NextDebuff() )
		{
			if( pDebuff1->GetDebuffID() == pDebuff->GetDebuffID() )
			{
				pAddTo = pDebuff1;
				break;
			}
		}
	}

	if( pAddTo )
	{
		pAddTo->OnAdded( pDebuff );
		return pAddTo;
	}
	else
	{
		Insert_Debuff( pDebuff );
		pDebuff->m_pDebuffLayer = this;
		pDebuff->OnAdded( NULL );
		return pDebuff;
	}
}

void CPlayerDebuffLayer::Remove( CPlayerDebuff* pDebuff )
{
	pDebuff->OnRemoved();
	pDebuff->m_pDebuffLayer = NULL;
	pDebuff->RemoveFrom_Debuff();
}

CPlayer* CPlayerDebuffLayer::GetPlayer()
{
	CEntity* pEntity = GetParentEntity();
	if( !pEntity )
		return NULL;
	return dynamic_cast<CPlayer*>( pEntity );
}
	
void CPlayerDebuffLayer::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CPlayerDebuffLayer::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

void CPlayerDebuffLayer::OnEnterHorrorReflex()
{
	for( auto pDebuff = m_pDebuffs; pDebuff; )
	{
		auto pDebuff1 = pDebuff->NextDebuff();
		pDebuff->OnEnterHorrorReflex();
		pDebuff = pDebuff1;
	}
}

void CPlayerDebuffLayer::OnEndHorrorReflex( float fSpRecover )
{
	for( auto pDebuff = m_pDebuffs; pDebuff; )
	{
		auto pDebuff1 = pDebuff->NextDebuff();
		pDebuff->OnEndHorrorReflex( fSpRecover );
		pDebuff = pDebuff1;
	}
}

void CPlayerDebuffLayer::OnTickBeforeHitTest()
{
	for( auto pDebuff = m_pDebuffs; pDebuff; )
	{
		auto pDebuff1 = pDebuff->NextDebuff();
		if( !pDebuff->UpdateBeforeHitTest() )
			Remove( pDebuff );
		pDebuff = pDebuff1;
	}
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CPlayerDebuffLayer::OnTickAfterHitTest()
{
	for( auto pDebuff = m_pDebuffs; pDebuff; )
	{
		auto pDebuff1 = pDebuff->NextDebuff();
		if( !pDebuff->UpdateAfterHitTest() )
			Remove( pDebuff );
		pDebuff = pDebuff1;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}