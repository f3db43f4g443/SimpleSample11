#include "stdafx.h"
#include "StageDirector.h"
#include "Common/ResourceManager.h"
#include "Common/MathUtil.h"
#include "UICommon/UIFactory.h"
#include "MyGame.h"
#include "World.h"
#include "Face.h"
#include "MyLevel.h"

CStageDirector::CStageDirector()
	: m_onClickMainStage( this, &CStageDirector::OnClickMainStage )
	, m_onMouseMove( this, &CStageDirector::OnMainStageMouseMove )
	, m_onClickPlayerStage( this, &CStageDirector::OnClickPlayerStage )
	, m_onTick( this, &CStageDirector::OnTick )
	, m_nState( eState_Free )
	, m_nViewportMoveTime( 0 )
	, m_nFaceToolboxMoveTime( 0 )
	, m_nFocusView( INVALID_8BITID )
	, m_pWorld( NULL )
{

}

void CStageDirector::OnInited()
{
	m_pMainStageViewport = GetChildByName<CUIViewport>( "main" );
	m_pMainStageViewport->Register( eEvent_Clicked, &m_onClickMainStage );
	m_pMainStageViewport->Register( eEvent_MouseMove, &m_onMouseMove );
	m_pSubStageViewport[0] = CFaceView::Create( GetChildByName<CUIViewport>( "sub0" ) );
	m_pSubStageViewport[1] = CFaceView::Create( GetChildByName<CUIViewport>( "sub1" ) );
	m_pSubStageViewport[0]->Register( eEvent_Clicked, &m_onClickPlayerStage );
	m_pSubStageViewport[0]->ReserveTexSize( CVector2( Pow2Ceil( m_pMainStageViewport->GetSize().width ), Pow2Ceil( m_pMainStageViewport->GetSize().height ) ) );
	m_pSubStageViewport[1]->ReserveTexSize( CVector2( Pow2Ceil( m_pMainStageViewport->GetSize().width ), Pow2Ceil( m_pMainStageViewport->GetSize().height ) ) );

	m_pFaceToolbox = new CFaceToolbox;
	CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/face_toolbox.xml" )->GetElement()->Clone( m_pFaceToolbox.GetPtr() );
	AddChild( m_pFaceToolbox );

	CGame::Inst().Register( 1, &m_onTick );
}

void CStageDirector::OnWorldCreated( CWorld* pWorld )
{
	m_pWorld = pWorld;
}

void CStageDirector::SetState( uint8 nState )
{
	if( m_nState == nState )
		return;
	
	auto pLevel = CMyLevel::GetInst();
	auto mousePos = GetMgr()->GetMousePos();
	bool bInside = m_pMainStageViewport->GetLocalBound().Contains( mousePos - m_pMainStageViewport->globalTransform.GetPosition() );
	if( !bInside )
		mousePos = m_pMainStageViewport->GetLocalBound().GetCenter();
	CVector2 fixOfs = m_pMainStageViewport->GetScenePos( GetMgr()->GetMousePos() );
	CVector2 v = ( fixOfs - pLevel->GetBaseOffset() ) / pLevel->GetGridScale();
	TVector2<int32> grid( floor( v.x + 0.5f ), floor( v.y + 0.5f ) );

	switch( m_nState )
	{
	case eState_SelectTarget:
		{
			auto pLevel = CMyLevel::GetInst();
			pLevel->GetSelectTile()->bVisible = false;
			auto& actionContext = *pLevel->GetTurnBasedContext()->pActionContext;
			actionContext.pOrgan->OnEndSelectTarget( actionContext );
		}
		break;
	default:
		break;
	}

	m_nState = nState;

	switch( m_nState )
	{
	case eState_SelectTarget:
		{
			auto pLevel = CMyLevel::GetInst();
			pLevel->GetSelectTile()->bVisible = true;
			auto& actionContext = *pLevel->GetTurnBasedContext()->pActionContext;
			actionContext.pOrgan->OnBeginSelectTarget( actionContext, grid );
		}
		break;
	default:
		break;
	}
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
	m_pSubStageViewport[nSlot]->SetSubStage( nStage );
	return true;
}

bool CStageDirector::HideSubStage( uint8 nSlot )
{
	m_pSubStageViewport[nSlot]->SetSubStage( -1 );
	if( nSlot == m_nFocusView )
		FocusFaceView( INVALID_8BITID );
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
	if( nSlot != INVALID_8BITID )
	{
		auto pSubStage = m_pSubStageViewport[nSlot]->GetSubStage();
		if( !pSubStage )
			return;
	}

	m_nViewportMoveTime = 30;
	if( nSlot == INVALID_8BITID )
	{
		m_targetViewportArea[0] = CRectangle( 0, 0, 500, 600 );
		m_targetViewportArea[1] = CRectangle( 500, 300, 300, 300 );
		m_targetViewportArea[2] = CRectangle( 500, 0, 300, 300 );
		m_pMainStageViewport->MoveToTopmost( true );
	}
	else if( nSlot == 0 )
	{
		m_targetViewportArea[0] = CRectangle( 0, 0, 200, 600 );
		m_targetViewportArea[1] = CRectangle( 200, 0, 600, 600 );
		m_targetViewportArea[2] = CRectangle( 500, 0, 300, 300 );
		m_pSubStageViewport[0]->MoveToTopmost( true );
	}
	else
	{
		m_targetViewportArea[0] = CRectangle( 0, 0, 200, 600 );
		m_targetViewportArea[1] = CRectangle( 500, 300, 300, 300 );
		m_targetViewportArea[2] = CRectangle( 200, 0, 600, 600 );
		m_pSubStageViewport[1]->MoveToTopmost( true );
	}

	if( m_nFocusView != INVALID_8BITID )
	{
		CFaceView* pView = GetFaceView( m_nFocusView );
		pView->OnFocused( false );
		if( pView->GetState() == CFaceView::eState_Edit )
		{
			m_targetFaceToolboxPos = CVector2( -200, 0 );
			m_nFaceToolboxMoveTime = 30;
		}
	}
	if( nSlot != INVALID_8BITID )
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

void CStageDirector::OnClickMainStage( CVector2* mousePos )
{
	if( m_nState == eState_SelectTarget )
	{
		auto pLevel = CMyLevel::GetInst();
		if( !pLevel )
			return;
		
		CVector2 fixOfs = m_pMainStageViewport->GetScenePos( *mousePos );
		CVector2 v = ( fixOfs - pLevel->GetBaseOffset() ) / pLevel->GetGridScale();
		TVector2<int32> grid( floor( v.x + 0.5f ), floor( v.y + 0.5f ) );

		auto pPlayer = pLevel->GetStage()->GetPlayer();
		if( pPlayer )
			pPlayer->PlayerCommandSelectTargetLevelGrid( grid );
	}
}

void CStageDirector::OnMainStageMouseMove( SUIMouseEvent * pEvent )
{
	if( m_nState == eState_SelectTarget )
	{
		auto pLevel = CMyLevel::GetInst();
		CVector2 fixOfs = m_pMainStageViewport->GetScenePos( pEvent->mousePos );
		CVector2 v = ( fixOfs - pLevel->GetBaseOffset() ) / pLevel->GetGridScale();
		TVector2<int32> grid( floor( v.x + 0.5f ), floor( v.y + 0.5f ) );

		auto& actionContext = *pLevel->GetTurnBasedContext()->pActionContext;
		actionContext.pOrgan->OnSelectTargetMove( actionContext, grid );
	}
}

void CStageDirector::OnClickPlayerStage()
{
	if( m_nState != eState_Free )
		return;

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

	if( m_nFaceToolboxMoveTime > 0 )
	{
		CVector2 faceToolboxPos = m_pFaceToolbox->GetPosition();
		faceToolboxPos.x = floor( ( faceToolboxPos.x * ( m_nFaceToolboxMoveTime - 1 ) + m_targetFaceToolboxPos.x ) / m_nFaceToolboxMoveTime + 0.5f );
		faceToolboxPos.y = floor( ( faceToolboxPos.y * ( m_nFaceToolboxMoveTime - 1 ) + m_targetFaceToolboxPos.y ) / m_nFaceToolboxMoveTime + 0.5f );
		m_pFaceToolbox->SetPosition( faceToolboxPos );

		m_nFaceToolboxMoveTime--;
	}
}