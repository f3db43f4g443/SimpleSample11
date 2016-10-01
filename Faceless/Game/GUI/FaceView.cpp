#include "stdafx.h"
#include "FaceView.h"
#include "World.h"
#include "Face.h"
#include "UICommon/UIFactory.h"
#include "Common/ResourceManager.h"
#include "Player.h"

CFaceView* CFaceView::Create( CUIElement* pElem )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "GUI/UI/faceview.xml" );
	auto pUIItem = new CFaceView;
	g_pRes->GetElement()->Clone( pUIItem );
	pUIItem->Replace( pElem );
	return pUIItem;
}

void CFaceView::OnFocused( bool bFocused )
{
	if( m_bFocused == bFocused )
		return;
	m_bFocused = bFocused;
	if( !m_bFocused )
	{
		switch( m_nState )
		{
		case eState_None:
			break;
		case eState_Edit:
			for( int i = m_curSelectedRect.x; i < m_curSelectedRect.GetRight(); i++ )
			{
				for( int j = m_curSelectedRect.y; j < m_curSelectedRect.GetBottom(); j++ )
				{
					m_pSubStage->pFace->RefreshEditTile( i, j, 0 );
				}
			}
			m_pSubStage->pFace->OnEndEdit();
			m_bGridMoved = false;
			break;
		case eState_SelectFaceTarget:
			break;
		}
	}
	else
	{
		switch( m_nState )
		{
		case eState_None:
			break;
		case eState_Edit:
			m_pSubStage->pFace->OnBeginEdit();
			break;
		case eState_SelectFaceTarget:
			break;
		}
	}
}

void CFaceView::SetState( uint8 nState )
{
	if( m_nState == nState )
		return;
	bool bFocused = m_bFocused;
	OnFocused( false );
	m_nState = nState;
	OnFocused( bFocused );
}

void CFaceView::OnInited()
{

}

void CFaceView::OnMouseMove( const CVector2& mousePos )
{
	CUIViewport::OnMouseMove( mousePos );
	if( !m_bFocused )
		return;
	if( m_nState == eState_Edit )
	{
		m_bIsEditValid = false;
		if( !m_pSubStage || !m_pSubStage->pFace )
		{
			m_curSelectedRect = TRectangle<int32>( 0, 0, 0, 0 );
			return;
		}

		auto pFace = m_pSubStage->pFace;
		auto preRect = m_curSelectedRect;
		if( m_curSelectedRect.width && m_curSelectedRect.height )
		{
			for( int i = m_curSelectedRect.x; i < m_curSelectedRect.GetRight(); i++ )
			{
				for( int j = m_curSelectedRect.y; j < m_curSelectedRect.GetBottom(); j++ )
				{
					pFace->RefreshEditTile( i, j, 0 );
				}
			}
			m_curSelectedRect = TRectangle<int32>( 0, 0, 0, 0 );
		}

		if( m_pFaceEditItem )
		{
			CVector2 fixOfs = GetScenePos( mousePos );
			CVector2 tileSize = pFace->GetEditTile()->GetTileSize();
			auto matInv = pFace->globalTransform.Inverse();
			auto localPos = matInv.MulVector2Pos( fixOfs );

			CRectangle rect( 0, 0, 0, 0 );
			rect.SetCenter( ( localPos - pFace->GetEditTile()->GetBaseOffset() ) * CVector2( 1.0f / tileSize.x, 1.0f / tileSize.y ) );
			rect.SetSize( CVector2( m_pFaceEditItem->nWidth, m_pFaceEditItem->nHeight ) );
			m_curSelectedRect = TRectangle<int32>( floor( rect.x + 0.5f ), floor( rect.y + 0.5f ), m_pFaceEditItem->nWidth, m_pFaceEditItem->nHeight );
			m_curSelectedRect.x = Min( Max( m_curSelectedRect.x, 0 ), (int32)( pFace->GetEditTile()->GetWidth() - m_curSelectedRect.width ) );
			m_curSelectedRect.y = Min( Max( m_curSelectedRect.y, 0 ), (int32)( pFace->GetEditTile()->GetHeight() - m_curSelectedRect.height ) );
		
			m_bIsEditValid = m_pFaceEditItem->nType == eFaceEditType_Organ ? true : false;
			for( int i = m_curSelectedRect.x; i < m_curSelectedRect.GetRight(); i++ )
			{
				for( int j = m_curSelectedRect.y; j < m_curSelectedRect.GetBottom(); j++ )
				{
					bool bIsValid = m_pFaceEditItem->IsValidGrid( pFace, TVector2<int32>( i, j ) );
					if( m_pFaceEditItem->nType == eFaceEditType_Organ )
						m_bIsEditValid = m_bIsEditValid && bIsValid;
					else
						m_bIsEditValid = m_bIsEditValid || bIsValid;
					pFace->RefreshEditTile( i, j, bIsValid ? 1 : 2 );
				}
			}

			if( preRect != m_curSelectedRect )
				m_bGridMoved = true;
		}
	}
}

void CFaceView::OnClick( const CVector2& mousePos )
{
	if( !m_bFocused )
	{
		m_events.Trigger( eEvent_Clicked, NULL );
		return;
	}

	if( m_nState == eState_SelectFaceTarget )
	{
		auto pFace = m_pSubStage->pFace;
		CVector2 fixOfs = GetScenePos( mousePos );
		CVector2 tileSize = pFace->GetEditTile()->GetTileSize();
		auto matInv = pFace->globalTransform.Inverse();
		auto localPos = matInv.MulVector2Pos( fixOfs );

		CVector2 v = ( localPos - pFace->GetEditTile()->GetBaseOffset() * CVector2( 1.0f / tileSize.x, 1.0f / tileSize.y ) );
		auto pPlayer = m_pSubStage->pCharacter->GetStage()->GetPlayer();
		if( pPlayer )
			pPlayer->PlayerCommandSelectTargetFaceGrid( TVector2<int32>( floor( v.x ), floor( v.y ) ) );
	}

	if( m_nState == eState_Action )
	{
		auto pFace = m_pSubStage->pFace;
		CVector2 fixOfs = GetScenePos( mousePos );
		CVector2 tileSize = pFace->GetEditTile()->GetTileSize();
		auto matInv = pFace->globalTransform.Inverse();
		auto localPos = matInv.MulVector2Pos( fixOfs );
		auto pGrid = pFace->GetGrid( floor( localPos.x ), floor( localPos.y ) );
		if( pGrid->pOrgan )
		{
			auto pPlayer = m_pSubStage->pCharacter->GetStage()->GetPlayer();
			if( pPlayer )
			{
				pPlayer->PlayerCommandAction( pGrid->pOrgan );
			}
		}
	}
}

void CFaceView::OnStartDrag( const CVector2& mousePos )
{
	CUIViewport::OnStartDrag( mousePos );
	if( !m_bFocused )
		return;
	if( m_nState == eState_Edit && m_bGridMoved && TryEdit() )
		m_bGridMoved = false;
}

void CFaceView::OnDragged( const CVector2& mousePos )
{
	CUIViewport::OnDragged( mousePos );
	if( !m_bFocused )
		return;
	if( m_nState == eState_Edit && m_bGridMoved && TryEdit() )
		m_bGridMoved = false;
}

bool CFaceView::TryEdit()
{
	if( !m_pSubStage || !m_pSubStage->pFace || !m_bIsEditValid )
		return false;

	auto pPlayer = SafeCast<CPlayer>( m_pSubStage->pCharacter );
	if( pPlayer )
	{
		if( !pPlayer->PlayerCommandFaceEditItem( m_pFaceEditItem, TVector2<int32>( m_curSelectedRect.x, m_curSelectedRect.y ) ) )
			return false;
	}
	else
		m_pFaceEditItem->Edit( m_pSubStage->pCharacter, m_pSubStage->pFace, TVector2<int32>( m_curSelectedRect.x, m_curSelectedRect.y ) );

	m_bIsEditValid = m_pFaceEditItem->nType == eFaceEditType_Organ ? true : false;
	for( int i = m_curSelectedRect.x; i < m_curSelectedRect.GetRight(); i++ )
	{
		for( int j = m_curSelectedRect.y; j < m_curSelectedRect.GetBottom(); j++ )
		{
			bool bIsValid = m_pFaceEditItem->IsValidGrid( m_pSubStage->pFace, TVector2<int32>( i, j ) );
			if( m_pFaceEditItem->nType == eFaceEditType_Organ )
				m_bIsEditValid = m_bIsEditValid && bIsValid;
			else
				m_bIsEditValid = m_bIsEditValid || bIsValid;
			m_pSubStage->pFace->RefreshEditTile( i, j, bIsValid ? 1 : 2 );
		}
	}
	return true;
}

void CFaceView::OnStopDrag( const CVector2& mousePos )
{
	CUIViewport::OnStopDrag( mousePos );
}