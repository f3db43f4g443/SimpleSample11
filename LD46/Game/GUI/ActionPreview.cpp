#include "stdafx.h"
#include "ActionPreview.h"
#include "MyLevel.h"
#include "Stage.h"
#include "MyGame.h"

void CActionPreview::OnAddedToStage()
{
	m_pInputRoot->SetRenderObject( NULL );
	m_pRecordText->SetParentEntity( NULL );
	m_timeImgOrigRect = static_cast<CImage2D*>( m_pTime->GetRenderObject() )->GetElem().rect;
	auto pParam = static_cast<CImage2D*>( m_pTime->GetRenderObject() )->GetParam();
	m_timeImgOrigParam[0] = pParam[0];
	m_timeImgOrigParam[1] = pParam[1];
	m_pTime->SetRenderObject( NULL );
}

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
						const char* sz;
						int32 len;
						if( l == 0 )
						{
							SInputTableItem::SInputItr itr( *v[j] );
							itr.Next();
							sz = itr.sz;
							len = itr.l;
						}
						else
						{
							sz = m_pPreviewPlayer->m_strActionPreviewCharge.c_str();
							len = m_pPreviewPlayer->m_strActionPreviewCharge.length();
						}
						for( int k = 0; k < len; k++ )
						{
							auto c = sz[k];
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
			RefreshRecord();
		}
		else
			UpdateTime();
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
			AddRecord( m_pPreviewPlayer->m_vecActionPreviewInputItem[m_nSelectedType][m_nSelectedIndex] );
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
	m_pPreviewLevel->Init( 2 );
	auto pPlayer = GetStage()->GetMasterLevel()->GetCurLevel()->GetPlayer();
	m_pPreviewPlayer = pPlayer->InitActionPreviewLevel( m_pPreviewLevel, TVector2<int32>( m_nStartX, m_nStartY ) );
}

void CActionPreview::Clear()
{
	ClearRecord();
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

void CActionPreview::UpdateTime()
{
	if( !m_vecRecordItems.size() )
		return;
	m_vecRecordItems.back().nTime++;
	auto& last = m_vecRecordItems.back();
	auto p = static_cast<CImage2D*>( last.pTimeImg.GetPtr() );
	auto r = p->GetElem().rect;
	r.height += 2;
	p->SetRect( r );

	if( r.GetBottom() > m_timeImgOrigRect.GetBottom() )
	{
		for( int i = m_vecRecordItems.size() - 1; i >= 0; i-- )
		{
			auto& item = m_vecRecordItems[i];
			auto p = static_cast<CImage2D*>( item.pTimeImg.GetPtr() );
			auto r = p->GetElem().rect;
			if( item.pText->GetParentEntity() )
				item.pText->SetPosition( CVector2( item.pText->x, item.pText->y - 2 ) );
			if( i == 0 )
			{
				r.height -= 2;
				r.SetTop( m_timeImgOrigRect.y );
				if( r.height == 0 )
				{
					item.pText->SetParentEntity( NULL );
					item.pTimeImg->RemoveThis();
					m_vecRecordItems.pop_front();
				}
				else
					p->SetRect( r );
			}
			else
			{
				r.y -= 2;
				p->SetRect( r );
			}
		}
	}
}

void CActionPreview::AddRecord( SInputTableItem* pItem )
{
	auto y = m_vecRecordItems.size() ? static_cast<CImage2D*>( m_vecRecordItems.back().pTimeImg.GetPtr() )->GetElem().rect.GetBottom() : m_timeImgOrigRect.y;
	if( m_vecRecordItems.size() )
	{
		auto pImg = static_cast<CImage2D*>( m_vecRecordItems.back().pTimeImg.GetPtr() );
		auto r = pImg->GetElem().rect;
		r.height -= 2;
		pImg->SetRect( r );
	}
	m_vecRecordItems.resize( m_vecRecordItems.size() + 1 );
	auto& item = m_vecRecordItems.back();
	item.nTime = 0;
	item.pText = SafeCast<CSimpleText>( m_pRecordText->GetInstanceOwnerNode()->CreateInstance() );
	item.pTimeImg = static_cast<CDrawableGroup*>( m_pTime->GetResource() )->CreateInstance();
	for( int i = 0; i < 2; i++ )
		static_cast<CImage2D*>( item.pTimeImg.GetPtr() )->GetParam()[i] = m_timeImgOrigParam[i];
	m_pTime->AddChild( item.pTimeImg );
	auto r = m_timeImgOrigRect;
	r.y = y;
	r.height = 0;
	static_cast<CImage2D*>( item.pTimeImg.GetPtr() )->SetRect( r );

	auto pDrawable = static_cast<CDrawableGroup*>( m_pInputRoot->GetResource() );
	if( !pItem )
	{
		CRectangle rect( 96, 0, -8, 16 );
		CRectangle texRect( 0.0625f * 15, 0, 0.0625f, 0.0625f );
		auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
		pImg->SetRect( rect );
		pImg->SetTexRect( texRect );
		item.pText->AddChild( pImg );
	}
	else
	{
		auto nCurDir = m_pPreviewPlayer->GetCurDir();
		bool bChargeKey = false;
		int32 n = 0;
		for( int l = 0; l < 2; l++ )
		{
			if( l == 1 && !bChargeKey )
				break;
			const char* sz;
			int32 len;
			if( l == 0 )
			{
				SInputTableItem::SInputItr itr( *pItem );
				itr.Next();
				sz = itr.sz;
				len = itr.l;
			}
			else
			{
				sz = m_pPreviewPlayer->m_strActionPreviewCharge.c_str();
				len = m_pPreviewPlayer->m_strActionPreviewCharge.length();
			}
			for( int k = 0; k < len; k++ )
			{
				auto c = sz[k];
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
				CRectangle rect( 96 + n * 16, -8, 16, 16 );
				CRectangle texRect( 0.0625f * nTex, 0.0625f * l, 0.0625f, 0.0625f );
				auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
				pImg->SetRect( rect );
				pImg->SetTexRect( texRect );
				item.pText->AddChild( pImg );
				n++;
			}
		}
	}
}

void CActionPreview::RefreshRecord()
{
	if( m_vecRecordItems.size() <= 0 )
		return;
	auto& item = m_vecRecordItems.back();
	auto r = static_cast<CImage2D*>( item.pTimeImg.GetPtr() )->GetElem().rect;
	item.pText->SetPosition( CVector2( item.pText->x, r.GetBottom() ) );
	if( !item.pText->GetParentEntity() )
		item.pText->SetParentBeforeEntity( m_pTime );
	char sz[32];
	sprintf( sz, "+%d", item.nTime );
	item.pText->Set( sz, 1 );
}

void CActionPreview::ClearRecord()
{
	for( auto& item : m_vecRecordItems )
	{
		item.pText->SetParentEntity( NULL );
		item.pTimeImg->RemoveThis();
	}
	m_vecRecordItems.resize( 0 );
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
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordText, record_text )
		REGISTER_MEMBER_TAGGED_PTR( m_pTime, record_time )
	REGISTER_CLASS_END()
}