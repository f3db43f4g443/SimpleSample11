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
