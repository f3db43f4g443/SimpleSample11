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

	auto pAttackLevelColor = doc.RootElement()->FirstChildElement( "attack_level_color" );
	for( auto pItem = pAttackLevelColor->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
	{
		vecAttackLevelColor.push_back( CVector4( XmlGetAttr( pItem, "x", 0.0f ), XmlGetAttr( pItem, "y", 0.0f ),
			XmlGetAttr( pItem, "z", 0.0f ) , XmlGetAttr( pItem, "w", 0.0f ) ) );
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
