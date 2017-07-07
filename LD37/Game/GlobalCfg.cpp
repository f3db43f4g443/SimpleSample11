#include "stdafx.h"
#include "GlobalCfg.h"
#include "Common/xml.h"
#include "Common/FileUtil.h"
#include "Common/ResourceManager.h"
#include "Common/TabFile.h"

void CGlobalCfg::Load()
{
	{
		vector<char> content;
		GetFileContent( content, "configs/global_cfg.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );

		auto pLevel = doc.RootElement()->FirstChildElement( "levels" );
		strTutorialLevelPrefab = XmlGet( pLevel, "tutorial.name", "" );
		strMainLevelPrefab = XmlGet( pLevel, "main.name", "" );
		strMainMenuLevel = XmlGet( pLevel, "mainmenu.name", "" );
		for( auto pGen = pLevel->FirstChildElement( "gen" )->FirstChildElement(); pGen; pGen = pGen->NextSiblingElement() )
		{
			vecLevels.push_back( XmlGetAttr( pGen, "name", "" ) );
		}

		auto pPrefabs = doc.RootElement()->FirstChildElement( "prefabs" );
		for( auto pItem = pPrefabs->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			const char* szName = XmlGetAttr( pItem, "name", "" );
			const char* szPath = XmlGetAttr( pItem, "path", "" );
			mapPrefabPath[szName] = szPath;
		}

		auto pSounds = doc.RootElement()->FirstChildElement( "sounds" );
		for( auto pItem = pSounds->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			const char* szName = XmlGetAttr( pItem, "name", "" );
			const char* szPath = XmlGetAttr( pItem, "path", "" );
			mapSoundPath[szName] = szPath;
		}
	}

	{
		levelGenerateNodeContext.vecPaths.push_back( "configs/generate/" );
		pRootGenerateFile = levelGenerateNodeContext.LoadFile( "root", "configs/generate/" );
	}
}
