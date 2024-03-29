#include "stdafx.h"
#include "FileExplorer.h"
#include "MaterialEditor.h"
#include "ParticleEditor.h"
#include "PrefabEditor.h"
#include "Editor.h"
#include "Common/FileUtil.h"
#include "Common/Utf8Util.h"

void CFileExplorer::AddTool( CUIElement* pElem )
{
	auto size = pElem->GetSize();
	m_pToolsPanel->AddChild( pElem );
	pElem->SetPosition( CVector2( 4, m_nToolY + 4 ) );
	m_nToolY += size.GetBottom() + 8;
}

void CFileExplorer::OnInited()
{
	CFileView::OnInited();
	m_pNewFileName = GetChildByName<CUITextBox>( "new_filename" );

	vector<CDropDownBox::SItem> vecFileTypeItems;
	vecFileTypeItems.resize( CEditor::Inst().GetRegisteredEditors().size() );
	int i = 0;
	for( auto& item : CEditor::Inst().GetRegisteredEditors() )
	{
		vecFileTypeItems[i].name = item.second.strDesc;
		vecFileTypeItems[i].pData = (void*)item.first.c_str();
		i++;
	}
	m_pNewFileType = CDropDownBox::Create( &vecFileTypeItems[0], vecFileTypeItems.size() );
	m_pNewFileType->Replace( GetChildByName( "new_filetype" ) );
	m_pToolsPanel = GetChildByName<CUIElement>( "tools" );
	m_pToolsPanel->bVisible = false;

	m_onNew.Set( this, &CFileExplorer::OnNew );
	m_onOpen.Set( this, &CFileExplorer::OnOpen );
	m_onRefresh.Set( this, &CFileView::Refresh );
	m_onTools.Set( this, &CFileExplorer::OnTools );
	GetChildByName( "btn_new" )->Register( eEvent_Action, &m_onNew );
	GetChildByName( "btn_open" )->Register( eEvent_Action, &m_onOpen );
	GetChildByName( "btn_refresh" )->Register( eEvent_Action, &m_onRefresh );
	GetChildByName( "btn_tools" )->Register( eEvent_Action, &m_onTools );

	m_nToolY = 0;
	Refresh();
}

void CFileExplorer::OnOpen()
{
	CEditor::Inst().OpenFile( m_strFile.c_str() );
}

void CFileExplorer::OnNew()
{
	string str = UnicodeToUtf8( m_pNewFileName->GetText() );
	if( !CheckFileName( str.c_str(), false ) )
		return;
	auto pItem = m_pNewFileType->GetSelectedItem();
	
	const char* szExt = (const char*)pItem->pData;
	str = m_strPath + str + "." + szExt;
	if( IsFileExist( str.c_str() ) )
		return;

	auto pEditor = CEditor::Inst().SetEditor( szExt );
	if( pEditor )
		pEditor->NewFile( str.c_str() );
	SelectFolder( m_strPath.c_str() );
}

void CFileExplorer::OnTools()
{
	m_pToolsPanel->SetVisible( !m_pToolsPanel->bVisible );
}
