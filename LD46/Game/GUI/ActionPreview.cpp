#include "stdafx.h"
#include "ActionPreview.h"
#include "MyLevel.h"
#include "Stage.h"
#include "MyGame.h"

void CActionPreview::Show( bool bShow )
{
	if( bShow )
	{
		CreateLevel();
	}
	else
	{
		Clear();
	}
}

void CActionPreview::Update()
{
	if( CGame::Inst().IsKeyDown( 'R' ) )
	{
		Clear();
		CreateLevel();
	}
	if( m_bFailed )
		return;
	if( !m_bWaitingInput )
	{
		auto nResult = m_pPreviewLevel->UpdateActionPreview();
		if( nResult == 2 )
		{
			m_bFailed = true;
			return;
		}
		else if( nResult == 1 )
		{
			m_nSelectedType = -1;
			m_nSelectedIndex = 0;
			auto pDrawable = static_cast<CDrawableGroup*>( m_pInputRoot->GetResource() );
			auto nCurDir = m_pPreviewPlayer->GetCurDir();
			int32 i0 = 0;
			int32 nMaxCount = 0;
			for( int i = 0; i < ePlayerStateSource_Count; i++ )
			{
				auto& v = m_pPreviewPlayer->m_vecActionPreviewInputItem[i];
				auto& v1 = m_vecText[i];
				int32 nCount = v.size();
				for( int j = 0; j < nCount; j++ )
				{
					auto p = new CRenderObject2D;
					v1.push_back( p );
					p->x = 128 * i0;
					p->y = ( j - nCount ) * 24;
					m_pInputRoot->AddChild( p );
					if( !v[j] )
					{
						CRectangle rect( 0, 0, 16, 16 );
						CRectangle texRect( 0.0625f * 15, 0, 0.0625f, 0.0625f );
						auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
						pImg->SetRect( rect );
						pImg->SetTexRect( texRect );
						p->AddChild( pImg );
						continue;
					}
					int32 n = 0;
					bool bChargeKey = false;
					for( int l = 0; l < 2; l++ )
					{
						if( l == 1 && !bChargeKey )
							break;
						auto str = l == 0 ? v[j]->strInput : m_pPreviewPlayer->m_strActionPreviewCharge;
						for( int k = 0; k < str.length(); k++ )
						{
							auto c = str[k];
							int32 nTex;
							if( c == '6' )
								nTex = nCurDir ? 4 : 0;
							else if( c == '9' )
								nTex = nCurDir ? 3 : 1;
							else if( c == '8' )
								nTex = 2;
							else if( c == '7' )
								nTex = nCurDir ? 1 : 3;
							else if( c == '4' )
								nTex = nCurDir ? 0 : 4;
							else if( c == '1' )
								nTex = nCurDir ? 7 : 5;
							else if( c == '2' )
								nTex = 6;
							else if( c == '3' )
								nTex = nCurDir ? 5 : 7;
							else if( c >= 'A' && c <= 'D' )
								nTex = c - 'A' + 8;
							else if( c == '#' )
							{
								bChargeKey = true;
								continue;
							}
							CRectangle rect( n * 16, 0, 16, 16 );
							CRectangle texRect( 0.0625f * nTex, 0.0625f * l, 0.0625f, 0.0625f );
							auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
							pImg->SetRect( rect );
							pImg->SetTexRect( texRect );
							p->AddChild( pImg );
							n++;
						}
					}
				}
				if( nCount )
				{
					if( nCount > nMaxCount )
						nMaxCount = nCount;
					i0++;
					if( m_nSelectedType < 0 )
					{
						m_nSelectedType = i;
						m_nSelectedIndex = nCount - 1;
					}
				}
			}
			auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImg->SetZOrder( -1 );
			pImg->SetRect( CRectangle( -32, -nMaxCount * 24 - 64, i0 * 128 + 64, nMaxCount * 24 + 72 ) );
			pImg->SetTexRect( CRectangle( 0, 0.25f, 0.125f, 0.125f ) );
			pImg->GetParam()[0] = CVector4( 0.95f, 0.95f, 0.95f, 4 );
			pImg->GetParam()[1] = CVector4( -0.04f, -0.05f, -0.08f, 4 );
			m_pInputRoot->AddChild( pImg );
			pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImg->SetRect( CRectangle( -8, -5000, 128, 10000 ) );
			pImg->SetTexRect( CRectangle( 0, 0.25f, 0.125f, 0.125f ) );
			pImg->GetParam()[0] = CVector4( 1, 1, 1, -4 );
			pImg->GetParam()[1] = CVector4( 0.09f, 0.09f, -0.03f, -4 );
			m_pInputRoot->SetRenderObject( pImg );
			pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImg->SetRect( CRectangle( -5000, -8, 10000, 24 ) );
			pImg->SetTexRect( CRectangle( 0, 0.25f, 0.125f, 0.125f ) );
			pImg->GetParam()[0] = CVector4( 0.95f, 0.95f, 0.95f, -4 );
			pImg->GetParam()[1] = CVector4( 0.25f, -0.03f, -0.05f, -4 );
			m_pInputRoot->GetRenderObject()->AddChild( pImg );

			m_bWaitingInput = true;
		}
	}
	if( m_bWaitingInput )
	{
		if( CGame::Inst().IsInputDown( eInput_Up ) )
		{
			m_nSelectedIndex++;
			if( m_nSelectedIndex >= m_vecText[m_nSelectedType].size() )
				m_nSelectedIndex = 0;
		}
		if( CGame::Inst().IsInputDown( eInput_Down ) )
		{
			m_nSelectedIndex--;
			if( m_nSelectedIndex < 0 )
				m_nSelectedIndex = m_vecText[m_nSelectedType].size() - 1;
		}
		if( CGame::Inst().IsInputDown( eInput_Left ) )
		{
			auto nType1 = m_nSelectedType;
			for( int k = 0; k < ePlayerStateSource_Count; k++ )
			{
				nType1--;
				if( nType1 < 0 )
					nType1 = ePlayerStateSource_Count - 1;
				if( m_vecText[nType1].size() )
					break;
			}
			if( nType1 != m_nSelectedType )
			{
				m_nSelectedIndex = Max<int32>( 0, m_vecText[nType1].size() - m_vecText[m_nSelectedType].size() + m_nSelectedIndex );
				m_nSelectedType = nType1;
			}
		}
		if( CGame::Inst().IsInputDown( eInput_Right ) )
		{
			auto nType1 = m_nSelectedType;
			for( int k = 0; k < ePlayerStateSource_Count; k++ )
			{
				nType1++;
				if( nType1 >= ePlayerStateSource_Count )
					nType1 = 0;
				if( m_vecText[nType1].size() )
					break;
			}
			if( nType1 != m_nSelectedType )
			{
				m_nSelectedIndex = Max<int32>( 0, m_vecText[nType1].size() - m_vecText[m_nSelectedType].size() + m_nSelectedIndex );
				m_nSelectedType = nType1;
			}
		}

		if( CGame::Inst().IsKeyDown( VK_RETURN ) || CGame::Inst().IsKeyDown( ' ' ) )
		{
			m_pPreviewPlayer->ActionPreviewInput( m_nSelectedType, m_nSelectedIndex );
			m_bWaitingInput = false;
		}
		RefreshText();
	}
}

void CActionPreview::CreateLevel()
{
	m_pPreviewLevel = SafeCast<CMyLevel>( m_pLevelPrefab->GetRoot()->CreateInstance() );
	m_pPreviewLevel->SetZOrder( -1 );
	m_pPreviewLevel->SetParentEntity( this );
	m_pPreviewLevel->SetPosition( m_pPreviewLevel->GetCamPos() * -1 );
	m_pPreviewLevel->Init( true );
	auto pPlayer = GetStage()->GetMasterLevel()->GetCurLevel()->GetPlayer();
	m_pPreviewPlayer = pPlayer->InitActionPreviewLevel( m_pPreviewLevel, TVector2<int32>( m_nStartX, m_nStartY ) );
}

void CActionPreview::Clear()
{
	for( int i = 0; i < ELEM_COUNT( m_vecText ); i++ )
		m_vecText[i].resize( 0 );
	m_pTip->bVisible = false;
	m_pInputRoot->SetRenderObject( NULL );
	m_pInputRoot->RemoveAllChild();
	m_pPreviewPlayer = NULL;
	m_pPreviewLevel->End();
	m_pPreviewLevel->SetParentEntity( NULL );
	m_pPreviewLevel = NULL;
	m_bWaitingInput = false;
	m_bFailed = false;
}

void CActionPreview::RefreshText()
{
	if( !m_bWaitingInput )
	{
		for( int i = 0; i < ELEM_COUNT( m_vecText ); i++ )
			m_vecText[i].resize( 0 );
		m_pTip->bVisible = false;
		m_pInputRoot->SetRenderObject( NULL );
		m_pInputRoot->RemoveAllChild();
		return;
	}
	m_pTip->bVisible = true;
	for( int i = 0; i < ELEM_COUNT( m_vecText ); i++ )
	{
		for( int j = 0; j < m_vecText[i].size(); j++ )
		{
			auto& p = m_vecText[i][j];
			if( i == m_nSelectedType && j == m_nSelectedIndex )
				m_pInputRoot->GetRenderObject()->SetPosition( p->GetPosition() );
			for( auto pChild = p->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
			{
				auto pImg = static_cast<CImage2D*>( pChild );
				pImg->GetParam()[0] = CVector4( 0, 0, 0, 0 );
				pImg->GetParam()[1] = i == m_nSelectedType && j == m_nSelectedIndex ? CVector4( 0.85f, 0.8f, 0.2f, 0 ) : CVector4( 0.5f, 0.3f, 0.05f, 0 );
			}
		}
	}
}

void RegisterGameClasses_ActionPreview()
{
	REGISTER_CLASS_BEGIN( CActionPreview )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_pLevelPrefab )
		REGISTER_MEMBER( m_nStartX )
		REGISTER_MEMBER( m_nStartY )
		REGISTER_MEMBER_TAGGED_PTR( m_pInputRoot, 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pTip, tp )
	REGISTER_CLASS_END()
}