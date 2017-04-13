#include "stdafx.h"
#include "CommonBlockItems.h"
#include "Stage.h"
#include "Player.h"
#include "Common/ResourceManager.h"

void CDetectTrigger::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 15, &m_onTick );

	m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab.c_str() );
}

void CDetectTrigger::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDetectTrigger::OnTick()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
		if( m_detectRect1.Contains( pos ) )
		{
			GetStage()->RegisterAfterHitTest( m_nCD, &m_onTick );
			Trigger();
			return;
		}

		if( m_detectRect.Contains( pos ) )
		{
			GetStage()->RegisterAfterHitTest( 1, &m_onTick );
			return;
		}
	}
	GetStage()->RegisterAfterHitTest( 15, &m_onTick );
}
