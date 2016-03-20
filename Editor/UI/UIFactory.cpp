#include "stdafx.h"
#include "UIFactory.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UITextBox.h"

CUIFactory::CUIFactory()
{
	m_pElementTypes["label"] = new CUILabel;
	m_pElementTypes["button"] = new CUIButton;
	m_pElementTypes["textbox"] = new CUITextBox;
}

CUIElement* CUIFactory::LoadXml( TiXmlElement* pRoot )
{
	const char* szType = XmlGetAttr( pRoot, "type", "" );
	auto itr = m_pElementTypes.find( szType );
	if( itr == m_pElementTypes.end() )
		return NULL;

	CUIElement* pElem = itr->second->CreateObject();
	pElem->LoadXml( pRoot );

	for( auto pElement = pRoot->FirstChildElement( "element" ); pElement; pElement = pElement->NextSiblingElement( "element" ) )
	{
		CUIElement* pElem1 = LoadXml( pElement );
		if( pElem1 )
			pElem->AddChild( pElem1 );
	}
	return pElem;
}

CUIElement* CUIFactory::LoadXmlFile( const char* szFile )
{
	string strPath = "EditorRes/UI/";
	strPath += szFile;
	
	vector<char> content;
	GetFileContent( content, strPath.c_str(), true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	return LoadXml( doc.RootElement() );
}

void CUIResource::Create()
{
	m_pUIElement = CUIFactory::Inst().LoadXmlFile( GetName() );
}