#include "stdafx.h"
#include "GlobalCfg.h"
#include "Common/xml.h"
#include "Common/FileUtil.h"
#include "Common/ResourceManager.h"

void CGlobalCfg::Load()
{
	vector<char> content;
	GetFileContent( content, "configs/global_cfg.xml", true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	auto pPlayer = doc.RootElement()->FirstChildElement( "player" );
	auto pExp = pPlayer->FirstChildElement( "exp" );
	for( auto pItem = pExp->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
	{
		vecLevelExp.push_back( XmlGetAttr( pItem, "exp", 0 ) );
	}
}

int32 CGlobalCfg::GetLevelByExp( int32 nExp )
{
	int32 i = 0;
	for( i = 0; i < vecLevelExp.size(); i++ )
	{
		if( nExp < vecLevelExp[i] )
			break;
	}
	return i;
}
