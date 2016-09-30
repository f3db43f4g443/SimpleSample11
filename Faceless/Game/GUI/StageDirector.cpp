#include "stdafx.h"
#include "StageDirector.h"
#include "Common/ResourceManager.h"
#include "UICommon/UIFactory.h"
#include "MyGame.h"
#include "World.h"
#include "Face.h"
#include "MyLevel.h"

CStageDirector::CStageDirector()
	: m_onClickPlayerStage( this, &CStageDirector::OnClickPlayerStage )
	, m_onTick( this, &CStageDirector::OnTick )
	, m_nViewportMoveTime( 0 )
	, m_nFocusView( -1 )
	, m_pWorld( NULL )
{

}

void CStageDirector::OnInited()
{
	m_pMainStageViewport = GetChildByName<CUIViewport>( "main" );
	m_pSubStageViewport[0] = CFaceView::Create( GetChildByName<CUIViewport>( "sub0" ) );
	m_pSubStageViewport[1] = CFaceView::Create( GetChildByName<CUIViewport>( "sub1" ) );
	m_pSubStageViewport[0]->Register( eEvent_Action1, &m_onClickPlayerStage );

	m_pFaceToolbox = new CFaceToolbox;
	CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/face_toolbox.xml" )->GetElement()->Clone( m_pFaceToolbox.GetPtr() );
	AddChild( m_pFaceToolbox );

	CGame::Inst().Register( 1, &m_onTick );
}

void CStageDirector::OnWorldCreated( CWorld* pWorld )
{
	m_pWorld = pWorld;
}

bool CStageDirector::ShowSubStage( uint32 nStage, uint8 nSlot )
{
	auto pSubStage = m_pWorld->GetSubStage( nStage );
	if( !pSubStage || !pSubStage->pStage )
		return false;

	auto pPreSubStage = m_pSubStageViewport[nSlot]->GetSubStage();
	if( pPreSubStage == pSubStage )
		return false;
	if( pPreSubStage )
	{
		if( pPreSubStage->pCharacter )
			pPreSubStage->pCharacter->HideSubStage();
	}
	m_pSubStageViewport[nSlot]->SetSubStage( pSubStage );
	return true;
}

bool CStageDirector::HideSubStage( uint8 nSlot )
{
	m_pSubStageViewport[nSlot]->SetSubStage( NULL );
	if( nSlot == m_nFocusView )
		FocusFaceView( -1 );
	return true;
}

void CStageDirector::SetFaceViewState( uint8 nSlot, uint8 nState )
{
	CFaceView* pView = GetFaceView( nSlot );
	if( pView )
	{
		if( !pView->GetSubStage() )
			return;
		uint8 nPreState = pView->GetState();
		if( nPreState == nState )
			return;
		pView->SetState( nState );
		if( m_nFocusView == nSlot )
		{
			if( nState == CFaceView::eState_Edit )
			{
				m_targetFaceToolboxPos = CVector2( 0, 0 );
				m_nFaceToolboxMoveTime = 30;
				m_pFaceToolbox->Refresh( pView->GetSubStage()->pCharacter );
			}
			else if( nPreState == CFaceView::eState_Edit )
			{
				m_targetFaceToolboxPos = CVector2( -200, 0 );
				m_nFaceToolboxMoveTime = 30;
			}
		}
	}
}

void CStageDirector::FocusFaceView( uint8 nSlot, class CTurnBasedContext* pContext )
{
	if( nSlot == m_nFocusView )
		return;
	if( nSlot >= 0 )
	{
		auto pSubStage = m_pSubStageViewport[nSlot]->GetSubStage();
		if( !pSubStage )
			return;
	}

	m_nViewportMoveTime = 30;
	if( nSlot == -1 )
	{
		m_targetViewportArea[0] = CRectangle( 0, 0, 500, 600 );
		m_targetViewportArea[1] = CRectangle( 500, 300, 300, 300 );
		m_targetViewportArea[2] = CRectangle( 500, 0, 300, 300 );
	}
	else if( nSlot == 0 )
	{
		m_targetViewportArea[0] = CRectangle( 0, 0, 200, 600 );
		m_targetViewportArea[1] = CRectangle( 200, 0, 600, 600 );
		m_targetViewportArea[2] = CRectangle( 500, 0, 300, 300 );
	}
	else
	{
		m_targetViewportArea[0] = CRectangle( 0, 0, 200, 600 );
		m_targetViewportArea[1] = CRectangle( 500, 300, 300, 300 );
		m_targetViewportArea[2] = CRectangle( 200, 0, 600, 600 );
	}

	if( m_nFocusView >= 0 )
	{
		CFaceView* pView = GetFaceView( m_nFocusView );
		pView->OnFocused( false );
		if( pView->GetState() == CFaceView::eState_Edit )
		{
			m_targetFaceToolboxPos = CVector2( -200, 0 );
			m_nFaceToolboxMoveTime = 30;
		}
	}
	if( nSlot >= 0 )
	{
		CFaceView* pView = GetFaceView( nSlot );
		pView->OnFocused( true );
		if( pView->GetState() == CFaceView::eState_Edit )
		{
			m_targetFaceToolboxPos = CVector2( 0, 0 );
			m_nFaceToolboxMoveTime = 30;
			m_pFaceToolbox->Refresh( pView->GetSubStage()->pCharacter );
		}
	}

	m_nFocusView = nSlot;
	if( pContext )
		pContext->Yield( 0.5f, false );
}

void CStageDirector::OnClickPlayerStage()
{
	auto pSubStage = m_pSubStageViewport[0]->GetSubStage();
	if( !pSubStage->pCharacter )
		return;

	if( GetFaceView( 0 )->GetState() == CFaceView::eState_Edit || GetFaceView( 0 )->GetState() == CFaceView::eState_Action )
	{
		FocusFaceView( 0 );
	}
}

void CStageDirector::OnTick()
{
	CGame::Inst().Register( 1, &m_onTick );

	if( m_nViewportMoveTime > 0 )
	{
		CRectangle curViewport[3] = {
			m_pMainStageViewport->GetSize(),
			m_pSubStageViewport[0]->GetSize(),
			m_pSubStageViewport[1]->GetSize()
		};
		for( int i = 0; i < 3; i++ )
		{
			curViewport[i].x = floor( ( curViewport[i].x * ( m_nViewportMoveTime - 1 ) + m_targetViewportArea[i].x ) / m_nViewportMoveTime + 0.5f );
			curViewport[i].y = floor( ( curViewport[i].y * ( m_nViewportMoveTime - 1 ) + m_targetViewportArea[i].y ) / m_nViewportMoveTime + 0.5f );
			curViewport[i].width = floor( ( curViewport[i].width * ( m_nViewportMoveTime - 1 ) + m_targetViewportArea[i].width ) / m_nViewportMoveTime + 0.5f );
			curViewport[i].height = floor( ( curViewport[i].height * ( m_nViewportMoveTime - 1 ) + m_targetViewportArea[i].height ) / m_nViewportMoveTime + 0.5f );
		}
		m_pMainStageViewport->Resize( curViewport[0] );
		m_pSubStageViewport[0]->Resize( curViewport[1] );
		m_pSubStageViewport[1]->Resize( curViewport[2] );

		m_nViewportMoveTime--;
	}

	if( m_nFaceToolboxMoveTime )
	{
		CVector2 faceToolboxPos = m_pFaceToolbox->GetPosition();
		faceToolboxPos.x = floor( ( faceToolboxPos.x * ( m_nFaceToolboxMoveTime - 1 ) + m_targetFaceToolboxPos.x ) / m_nFaceToolboxMoveTime + 0.5f );
		faceToolboxPos.y = floor( ( faceToolboxPos.y * ( m_nFaceToolboxMoveTime - 1 ) + m_targetFaceToolboxPos.y ) / m_nFaceToolboxMoveTime + 0.5f );
		m_pFaceToolbox->SetPosition( faceToolboxPos );

		m_nFaceToolboxMoveTime--;
	}
}