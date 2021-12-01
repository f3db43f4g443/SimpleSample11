#include "stdafx.h"
#include "GUI.h"
#include "Stage.h"
#include "MyGame.h"
#include "GlobalCfg.h"

void CDayResultUI::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );

	auto& worldData = GetStage()->GetWorld()->GetWorldData();
	char buf[100];
	sprintf( buf, "Day%d over", worldData.nDay );
	m_pText1->Set( buf, 2 );
	sprintf( buf, "You gained %d exp\nPress ENTER", worldData.nPlayerExp - worldData.nLastPlayerExp );
	m_pText2->Set( buf, 2 );

	m_nLevelBegin = CGlobalCfg::Inst().GetLevelByExp( worldData.nLastPlayerExp );
	m_nLevelEnd = CGlobalCfg::Inst().GetLevelByExp( worldData.nPlayerExp );
}

void CDayResultUI::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CDayResultUI::OnTick()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
	if( CGame::Inst().IsKey( VK_RETURN ) )
	{
		CGame::Inst().ForceKeyRelease( VK_RETURN );
		if( m_nPhase == 0 )
			m_nPhase = m_nLevelBegin;
		m_nPhase++;
		if( m_nPhase > m_nLevelEnd )
		{
			SStageEnterContext context;
			GetStage()->GetWorld()->EnterStage( "0.pf", context );
		}
		else
		{
			m_p0->SetParentEntity( NULL );

			char buf[100];
			sprintf( buf, "data/tips/%d.pf", m_nPhase );
			auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( buf );
			m_p0 = SafeCast<CEntity>( pPrefab->GetRoot()->CreateInstance() );
			m_p0->SetParentEntity( this );
		}
	}
}

void RegisterGameClasses_GUI()
{
	REGISTER_CLASS_BEGIN( CDayResultUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_p0, 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pText1, 0/t1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pText2, 0/t2 )
	REGISTER_CLASS_END()
}
