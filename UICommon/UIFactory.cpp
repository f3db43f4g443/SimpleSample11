#include "stdafx.h"
#include "UIFactory.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Common/ResourceManager.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UICheckBox.h"
#include "UITextBox.h"
#include "UIScrollBar.h"
#include "UIScrollView.h"
#include "UITreeView.h"
#include "UIViewport.h"

CUIFactory::CUIFactory()
{
	m_pElementTypes["element"] = new CUIElement;
	m_pElementTypes["label"] = new CUILabel;
	m_pElementTypes["button"] = new CUIButton;
	m_pElementTypes["checkbox"] = new CUICheckBox;
	m_pElementTypes["textbox"] = new CUITextBox;
	m_pElementTypes["scrollbar"] = new CUIScrollBar;
	m_pElementTypes["scrollview"] = new CUIScrollView;
	m_pElementTypes["treeview"] = new CUITreeView;
	m_pElementTypes["viewport"] = new CUIViewport;
}

CUIElement* CUIFactory::LoadXml( TiXmlElement* pRoot )
{
	const char* szType = XmlGetAttr( pRoot, "type", "" );
	auto itr = m_pElementTypes.find( szType );
	if( itr == m_pElementTypes.end() )
		return NULL;

	CUIElement* pElem = itr->second->CreateObject();
	LoadXml( pElem, pRoot );
	return pElem;
}

CUIElement* CUIFactory::LoadXmlFile( const char* szFile )
{
	vector<char> content;
	GetFileContent( content, szFile, true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	return LoadXml( doc.RootElement() );
}

void CUIFactory::LoadXml( CUIElement* pElem, TiXmlElement* pRoot )
{
	pElem->LoadXml( pRoot );

	for( auto pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		if( !strcmp( pElement->Value(), "template" ) )
		{
			const char* szName = XmlGetAttr( pElement, "type", "" );
			CUIResource* pUIResource = CResourceManager::Inst()->CreateResource<CUIResource>( szName );
			CUIElement* pElem1 = pUIResource->GetElement()->Clone( false );
			LoadXml( pElem1, pElement );
			pElem->AddChild( pElem1 );
		}
		else
		{
			CUIElement* pElem1 = LoadXml( pElement );
			if( pElem1 )
				pElem->AddChild( pElem1 );
		}
	}
}

void CUIFactory::LoadXmlFile( CUIElement* pElem, const char* szFile )
{
	vector<char> content;
	GetFileContent( content, szFile, true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	LoadXml( pElem, doc.RootElement() );
}

void CUIResource::Create()
{
	m_pUIElement = CUIFactory::Inst().LoadXmlFile( GetName() );
	if( !m_pUIElement )
		return;
	m_bCreated = true;
}