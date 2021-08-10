#include "stdafx.h"
#include "LogUI.h"
#include "MyGame.h"
#include "MyLevel.h"
#include "Render/Image2D.h"

void CLogUI::OnAddedToStage()
{
	m_docScrollOrigRect = static_cast<CImage2D*>( m_pDocContentScroll->GetRenderObject() )->GetElem().rect;
	m_recordScrollOrigRect = static_cast<CImage2D*>( m_pRecordScroll->GetRenderObject() )->GetElem().rect;
}

void CLogUI::Show( int8 nPage, int32 nIndex )
{
	const char* szPageItems[] = { "Documents", "Records" };
	for( int i = 0; i < ELEM_COUNT( szPageItems ); i++ )
		m_pPageItemText[i]->Set( szPageItems[i] );
	m_vecDocuments.resize( 0 );
	auto pLuaState = CLuaMgr::GetCurLuaState();
	pLuaState->Load( m_strScriptInit );
	pLuaState->PushLua( this );
	pLuaState->Call( 1, 0 );
	m_vecRecords.resize( 0 );
	GetStage()->GetMasterLevel()->GetWorldData().GetScenarioRecords( [this] ( int8 nType, const CVector4& color, const char* szText ) {
		SRecord record = { nType, color, szText };
		m_vecRecords.push_back( record );
	} );

	if( nPage >= 0 )
	{
		m_nSelectedPage = nPage;
		m_nSelectedIndex = nIndex;
	}
	Refresh( true );
}

void CLogUI::Refresh( bool bInit )
{
	for( int i = 0; i < ELEM_COUNT( m_pPages ); i++ )
		m_pPages[i]->bVisible = i == m_nSelectedPage;
	m_pPageSelectEffect->SetPosition( CVector2( m_nSelectedPage * m_fPageItemWidth, 0 ) );
	if( m_nSelectedPage == 0 )
	{
		for( int i = 0; i < m_vecDocuments.size(); i++ )
		{
			if( m_vecDocuments[m_nSelectedIndex].bUnlocked )
				break;
			m_nSelectedIndex++;
			if( m_nSelectedIndex >= m_vecDocuments.size() )
				m_nSelectedIndex = 0;
		}

		for( int i = 0; i < ELEM_COUNT( m_pDocTitleText ); i++ )
			m_pDocTitleText[i]->Set( "" );
		if( !m_vecDocuments[m_nSelectedIndex].bUnlocked )
		{
			for( int i = 0; i < Min( m_vecDocuments.size(), ELEM_COUNT( m_pDocTitleText ) ); i++ )
				m_pDocTitleText[i]->Set( "??????????????" );
			m_pDocContentText->Set( "" );
			m_nCurDocLineScroll = m_nMaxDocLineScroll = 0;
			m_pDocContentScroll->bVisible = false;
		}
		else
		{
			int32 nShowBegin = Min( bInit ? m_nSelectedIndex : m_nShowBeginIndex, Max<int32>( 0, m_vecDocuments.size() - ELEM_COUNT( m_pDocTitleText ) ) );
			nShowBegin = Max( Min( nShowBegin, m_nSelectedIndex ), m_nSelectedIndex - (int32)ELEM_COUNT( m_pDocTitleText ) + 1 );
			int32 nShowEnd = Min( m_vecDocuments.size(), nShowBegin + ELEM_COUNT( m_pDocTitleText ) );
			for( int i = nShowBegin; i < nShowEnd; i++ )
			{
				if( m_vecDocuments[i].bUnlocked )
					m_pDocTitleText[i - nShowBegin]->Set( m_vecDocuments[i].strName.c_str() );
				else
					m_pDocTitleText[i - nShowBegin]->Set( "??????????????" );
			}
			m_nShowBeginIndex = nShowBegin;
			m_pDocSelectEffect->SetPosition( CVector2( 0, ( m_nShowBeginIndex - m_nSelectedIndex ) * m_fDocTitleHeight ) );

			int32 nLineCount = 0;
			m_pDocContentText->CalcLineCount( m_vecDocuments[m_nSelectedIndex].strContent.c_str(), nLineCount, 0 );
			if( nLineCount <= m_nDocMaxLines )
			{
				m_pDocContentText->Set( m_vecDocuments[m_nSelectedIndex].strContent.c_str() );
				m_nCurDocLineScroll = m_nMaxDocLineScroll = 0;
				m_pDocContentScroll->bVisible = false;
			}
			else
			{
				m_nMaxDocLineScroll = nLineCount - m_nDocMaxLines;
				m_nCurDocLineScroll = Max( 0, Min( m_nMaxDocLineScroll, m_nCurDocLineScroll ) );
				auto l = m_nCurDocLineScroll;
				auto szBegin = m_pDocContentText->CalcLineCount( m_vecDocuments[m_nSelectedIndex].strContent.c_str(), l, 1 );
				m_pDocContentText->Set( szBegin, 0, m_nDocMaxLines );

				m_pDocContentScroll->bVisible = true;
				auto pImg = static_cast<CImage2D*>( m_pDocContentScroll->GetRenderObject() );
				auto rect = m_docScrollOrigRect;
				rect.y = m_docScrollOrigRect.GetBottom() - floor( m_docScrollOrigRect.height * ( m_nCurDocLineScroll + m_nDocMaxLines ) / nLineCount * 0.5f + 0.5f ) * 2;
				rect.SetBottom( m_docScrollOrigRect.GetBottom() - floor( m_docScrollOrigRect.height * m_nCurDocLineScroll / nLineCount * 0.5f + 0.5f ) * 2 );
				pImg->SetRect( rect );
				pImg->SetBoundDirty();
			}
		}
	}
	else
	{
		if( bInit )
			m_nShowBeginIndex = 0;
		m_nShowBeginIndex = Min( m_nShowBeginIndex, Max<int32>( 0, m_vecRecords.size() - ELEM_COUNT( m_pRecordItemText ) ) );
		int32 nShowEndIndex = Min( m_vecRecords.size(), m_nShowBeginIndex + ELEM_COUNT( m_pRecordItemText ) );
		for( int i = 0; i < ELEM_COUNT( m_pRecordItemText ); i++ )
		{
			int32 n = i + nShowEndIndex - ELEM_COUNT( m_pRecordItemText );
			if( n < 0 )
			{
				m_pRecordItemText[i]->Set( "" );
				continue;
			}
			auto& item = m_vecRecords[n];
			auto pText = m_pRecordItemText[i];
			if( item.nType == 0 )
			{
				m_pRecordItemText[i]->SetPosition( CVector2( m_fRecordTexLeft, pText->y ) );
				m_pRecordItemText[i]->SetParam( item.color );
				m_pRecordItemText[i]->Set( item.strText.c_str(), 0 );
			}
			else if( item.nType == 1 )
			{
				m_pRecordItemText[i]->SetPosition( CVector2( m_fRecordTexRight, pText->y ) );
				m_pRecordItemText[i]->SetParam( item.color );
				m_pRecordItemText[i]->Set( item.strText.c_str(), 1 );
			}
			else if( item.nType == 2 )
			{
				m_pRecordItemText[i]->SetPosition( CVector2( ( m_fRecordTexLeft + m_fRecordTexRight ) * 0.5f, pText->y ) );
				m_pRecordItemText[i]->SetParam( item.color );
				m_pRecordItemText[i]->Set( item.strText.c_str(), 2 );
			}
			else
			{
				m_pRecordItemText[i]->SetPosition( CVector2( ( m_fRecordTexLeft + m_fRecordTexRight ) / 2, pText->y ) );
				m_pRecordItemText[i]->SetParam( CVector4( 0.5, 0.65, 0.6, 0 ) );
				m_pRecordItemText[i]->Set( "================================================================================", 2 );
			}
		}
		auto pImg = static_cast<CImage2D*>( m_pRecordScroll->GetRenderObject() );
		int32 nMax = m_vecRecords.size();
		auto rect = m_recordScrollOrigRect;
		if( nMax )
		{
			rect.y = m_recordScrollOrigRect.y + floor( m_recordScrollOrigRect.height * m_nShowBeginIndex / nMax * 0.5f + 0.5f ) * 2;
			rect.SetBottom( m_recordScrollOrigRect.y + floor( m_recordScrollOrigRect.height * nShowEndIndex / nMax * 0.5f + 0.5f ) * 2 );
		}
		pImg->SetRect( rect );
		pImg->SetBoundDirty();
	}
}

void CLogUI::Update()
{
	bool bChangePage = false;
	if( CGame::Inst().IsInputDown( eInput_Left ) )
	{
		m_nSelectedPage--;
		if( m_nSelectedPage < 0 )
			m_nSelectedPage = ELEM_COUNT( m_pPages ) - 1;
		bChangePage = true;
	}
	if( CGame::Inst().IsInputDown( eInput_Right ) )
	{
		m_nSelectedPage++;
		if( m_nSelectedPage >= ELEM_COUNT( m_pPages ) )
			m_nSelectedPage = 0;
		bChangePage = true;
	}

	bool bDirty = bChangePage;
	if( m_nSelectedPage == 0 )
	{
		if( bChangePage )
		{
			m_nSelectedIndex = 0;
			for( int i = 0; i < m_vecDocuments.size(); i++ )
			{
				if( m_vecDocuments[m_nSelectedIndex].bUnlocked )
					break;
				m_nSelectedIndex++;
				if( m_nSelectedIndex >= m_vecDocuments.size() )
					m_nSelectedIndex = 0;
			}
		}

		if( m_vecDocuments[m_nSelectedIndex].bUnlocked )
		{
			if( CGame::Inst().IsInputDown( eInput_Up ) )
			{
				bDirty = true;
				for( int i = 0; i < m_vecDocuments.size(); i++ )
				{
					m_nSelectedIndex--;
					if( m_nSelectedIndex < 0 )
						m_nSelectedIndex = m_vecDocuments.size() - 1;
					if( m_vecDocuments[m_nSelectedIndex].bUnlocked )
					{
						m_nCurDocLineScroll = 0;
						break;
					}
				}
			}
			else if( CGame::Inst().IsInputDown( eInput_Down ) )
			{
				bDirty = true;
				for( int i = 0; i < m_vecDocuments.size(); i++ )
				{
					m_nSelectedIndex++;
					if( m_nSelectedIndex >= m_vecDocuments.size() )
						m_nSelectedIndex = 0;
					if( m_vecDocuments[m_nSelectedIndex].bUnlocked )
					{
						m_nCurDocLineScroll = 0;
						break;
					}
				}
			}
			if( m_nMaxDocLineScroll )
			{
				if( CGame::Inst().IsInput( eInput_D ) && m_nCurDocLineScroll > 0 )
				{
					m_nCurDocLineScroll--;
					bDirty = true;
				}
				if( CGame::Inst().IsInput( eInput_B ) && m_nCurDocLineScroll < m_nMaxDocLineScroll )
				{
					m_nCurDocLineScroll++;
					bDirty = true;
				}
			}
		}
	}
	else
	{
		if( CGame::Inst().IsInput( eInput_Up ) )
		{
			if( m_nShowBeginIndex < m_vecRecords.size() - ELEM_COUNT( m_pRecordItemText ) )
			{
				bDirty = true;
				m_nShowBeginIndex++;
			}
		}
		else if( CGame::Inst().IsInput( eInput_Down ) )
		{
			if( m_nShowBeginIndex > 0 )
			{
				bDirty = true;
				m_nShowBeginIndex--;
			}
		}
	}
	if( bDirty )
		Refresh( false );
}

void CLogUI::AddDoc( bool bUnlocked, const char* szName, const char* szContent )
{
	m_vecDocuments.resize( m_vecDocuments.size() + 1 );
	auto& item = m_vecDocuments.back();
	item.bUnlocked = bUnlocked;
	if( szName )
		item.strName = szName;
	if( szContent )
		item.strContent = szContent;
}


void RegisterGameClasses_LogUI()
{
	REGISTER_CLASS_BEGIN( CLogUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_BEGIN( m_strScriptInit )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_fPageItemWidth )
		REGISTER_MEMBER( m_fDocTitleHeight )
		REGISTER_MEMBER( m_fRecordTexLeft )
		REGISTER_MEMBER( m_fRecordTexRight )
		REGISTER_MEMBER( m_nDocMaxLines )
		REGISTER_MEMBER_TAGGED_PTR( m_pPages[0], documents )
		REGISTER_MEMBER_TAGGED_PTR( m_pPages[1], records )
		REGISTER_MEMBER_TAGGED_PTR( m_pPageItemText[0], page1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pPageItemText[1], page2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pPageSelectEffect, selected )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[0], documents/t1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[1], documents/t2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[2], documents/t3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[3], documents/t4 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[4], documents/t5 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[5], documents/t6 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[6], documents/t7 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocTitleText[7], documents/t8 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocSelectEffect, documents/selected )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocContentText, documents/content )
		REGISTER_MEMBER_TAGGED_PTR( m_pDocContentScroll, documents/scroll )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[0], records/t0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[1], records/t1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[2], records/t2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[3], records/t3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[4], records/t4 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[5], records/t5 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[6], records/t6 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[7], records/t7 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[8], records/t8 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[9], records/t9 )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[10], records/ta )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[11], records/tb )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[12], records/tc )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[13], records/td )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[14], records/te )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordItemText[15], records/tf )
		REGISTER_MEMBER_TAGGED_PTR( m_pRecordScroll, records/scroll )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( AddDoc )
	REGISTER_CLASS_END()
}