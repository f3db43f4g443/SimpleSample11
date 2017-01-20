#include "stdafx.h"
#include "StageDirector.h"
#include "Common/ResourceManager.h"
#include "Common/MathUtil.h"
#include "UICommon/UIFactory.h"
#include "MyGame.h"
#include "World.h"
#include "MyLevel.h"

CStageDirector::CStageDirector()
	: m_onClickMainStage( this, &CStageDirector::OnClickMainStage )
	, m_onMouseMove( this, &CStageDirector::OnMainStageMouseMove )
	, m_onTick( this, &CStageDirector::OnTick )
	, m_pWorld( NULL )
{

}

void CStageDirector::OnInited()
{
	m_pMainStageViewport = GetChildByName<CUIViewport>( "main" );
	m_pMainStageViewport->Register( eEvent_Clicked, &m_onClickMainStage );
	m_pMainStageViewport->Register( eEvent_MouseMove, &m_onMouseMove );
	m_pSubStageViewport = GetChildByName<CUIViewport>( "sub" );

	CGame::Inst().Register( 1, &m_onTick );
}

void CStageDirector::OnWorldCreated( CWorld* pWorld )
{
	m_pWorld = pWorld;
}

void CStageDirector::OnClickMainStage( CVector2* mousePos )
{
}

void CStageDirector::OnMainStageMouseMove( SUIMouseEvent * pEvent )
{
	CPlayer* pPlayer = m_pWorld->GetPlayer();
	if( pPlayer )
	{
		pPlayer->AimAt( m_pMainStageViewport->GetScenePos( pEvent->mousePos ) );
	}
}

void CStageDirector::OnTick()
{
	CGame::Inst().Register( 1, &m_onTick );
}