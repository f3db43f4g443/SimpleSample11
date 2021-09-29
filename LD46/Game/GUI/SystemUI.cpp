#include "stdafx.h"
#include "SystemUI.h"
#include "MyGame.h"
#include "SdkInterface.h"
#include "GlobalCfg.h"

void CSystemUI::OnAddedToStage()
{
	m_pSelectY0 = m_pSelected->y;
	m_pText1->Set( "MUSIC VOLUME\nSFX VOLUME\nWALKTHROUGH\nMAIN MENU\nEXIT", 1 );
}

void CSystemUI::Show()
{
	Refresh();
}

void CSystemUI::Update()
{
	bool bRefresh = false;
	if( CGame::Inst().IsInputDown( eInput_Up ) )
	{
		m_nSelected--;
		if( m_nSelected < 0)
			m_nSelected = 4;
		m_bShowConfirm = false;
		bRefresh = true;
	}
	if( CGame::Inst().IsInputDown( eInput_Down ) )
	{
		m_nSelected++;
		if( m_nSelected == 5 )
			m_nSelected = 0;
		m_bShowConfirm = false;
		bRefresh = true;
	}
	if( m_nSelected == 0 || m_nSelected == 1 )
	{
		int32 nVolume = m_nSelected == 0 ? CGame::Inst().GetMusicVolume() : CGame::Inst().GetSfxVolume();
		auto n0 = nVolume;
		if( CGame::Inst().IsInputDown( eInput_Left ) )
		{
			if( nVolume > 0 )
				nVolume--;
		}
		if( CGame::Inst().IsInputDown( eInput_Right ) )
		{
			if( nVolume < 10 )
				nVolume++;
		}
		if( nVolume != n0 )
		{
			bRefresh = true;
			if( m_nSelected == 0 )
				CGame::Inst().SetMusicVolume( nVolume );
			else
				CGame::Inst().SetSfxVolume( nVolume );
		}
	}
	if( m_nSelected == 2 )
	{
		if( CGame::Inst().IsKeyDown( '\n' ) || CGame::Inst().IsKeyDown( ' ' ) )
		{
			if( !m_bShowConfirm )
			{
				m_bShowConfirm = true;
				bRefresh = true;
				if( ISdkInterface::Inst() )
					ISdkInterface::Inst()->OpenExplorer( CGlobalCfg::Inst().strWalkthrough.c_str() );
			}
		}
	}
	if( m_nSelected == 3 || m_nSelected == 4 )
	{
		if( CGame::Inst().IsKeyDown( '\n' ) || CGame::Inst().IsKeyDown( ' ' ) )
		{
			if( !m_bShowConfirm )
			{
				m_bShowConfirm = true;
				bRefresh = true;
			}
			else
			{
				if( m_nSelected == 3 )
					CGame::Inst().QuitToMainMenu();
				else
					CGame::Inst().Exit();
				return;
			}
		}
	}
	if( bRefresh )
		Refresh();
}

void CSystemUI::Refresh()
{
	m_pSelected->SetPosition( CVector2( m_pSelected->x, m_pSelectY0 - m_nSelected * 64 ) );

	stringstream ss;
	int32 nMusicVolume = CGame::Inst().GetMusicVolume();
	ss << ( m_nSelected == 0 ? "A- " : "   " );
	for( int i = 0; i < nMusicVolume; i++ )
		ss << "_";
	for( int i = nMusicVolume; i < 10; i++ )
		ss << ":";
	ss << ( m_nSelected == 0 ? " +D\n" : "   \n" );

	int32 nSfxVolume = CGame::Inst().GetSfxVolume();
	ss << ( m_nSelected == 1 ? "A- " : "   " );
	for( int i = 0; i < nSfxVolume; i++ )
		ss << "_";
	for( int i = nSfxVolume; i < 10; i++ )
		ss << ":";
	ss << ( m_nSelected == 1 ? " +D\n" : "   \n" );

	if( m_nSelected == 2 )
	{
		if( m_bShowConfirm )
			ss << "     ......";
		else
			ss << "     ENTER";
	}
	ss << "\n";
	if( m_nSelected == 3 )
	{
		if( m_bShowConfirm )
			ss << "     SURE?";
		else
			ss << "     ENTER";
	}
	ss << "\n";
	if( m_nSelected == 4 )
	{
		if( m_bShowConfirm )
			ss << "     SURE?";
		else
			ss << "     ENTER";
	}

	m_pText2->Set( ss.str().c_str() );
}


void RegisterGameClasses_SystemUI()
{
	REGISTER_CLASS_BEGIN( CSystemUI )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER_TAGGED_PTR( m_pSelected, select )
		REGISTER_MEMBER_TAGGED_PTR( m_pText1, text1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pText2, text2 )
	REGISTER_CLASS_END()
}