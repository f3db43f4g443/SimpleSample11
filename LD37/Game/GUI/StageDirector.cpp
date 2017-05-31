#include "stdafx.h"
#include "StageDirector.h"
#include "Common/ResourceManager.h"
#include "Common/MathUtil.h"
#include "UICommon/UIFactory.h"
#include "MyGame.h"
#include "World.h"
#include "MyLevel.h"
#include "PostEffects.h"

CStageDirector::CStageDirector()
	: m_onClickMainStage( this, &CStageDirector::OnClickMainStage )
	, m_onMouseMove( this, &CStageDirector::OnMainStageMouseMove )
	, m_onTick( this, &CStageDirector::OnTick )
	, m_onPostProcess( this, &CStageDirector::OnPostProcess )
	, m_mousePos( 0, 0 )
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

void CStageDirector::OnWorldDestroyed( CWorld * pWorld )
{
	m_pWorld = NULL;
}

void CStageDirector::AfterPlayMainStage()
{
	/*auto size = m_pMainStageViewport->GetSize();
	float fScale = 800.0f / size.height;
	m_pMainStageViewport->SetCustomRender( CVector2( floor( size.width * fScale + 0.5f ), 800.0f ) );
	m_pMainStageViewport->RegisterOnPostProcess( &m_onPostProcess );*/
}

void CStageDirector::OnClickMainStage( CVector2* mousePos )
{
}

void CStageDirector::OnMainStageMouseMove( SUIMouseEvent * pEvent )
{
	m_mousePos = pEvent->mousePos;
}

void CStageDirector::OnTick()
{
	CGame::Inst().Register( 1, &m_onTick );
	if( m_pWorld )
	{
		CPlayer* pPlayer = m_pWorld->GetPlayer();
		if( pPlayer )
		{
			pPlayer->AimAt( m_pMainStageViewport->GetScenePos( m_mousePos ) );
		}
	}
}

void CStageDirector::OnPostProcess( class CPostProcessPass* pPass )
{
	static CPostProcessInvertColor effect;
	effect.SetPriority( 1 );
	pPass->Register( &effect );
}