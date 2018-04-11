#include "stdafx.h"
#include "GlobalCfg.h"
#include "Common/xml.h"
#include "Common/FileUtil.h"
#include "Common/ResourceManager.h"
#include "Common/TabFile.h"

void CGlobalCfg::GetComboLevel( uint32 nCombo, int32& nLevel, float& fPercent )
{
	for( int i = 0; i < vecCombo2PointMul.size(); i++ )
	{
		if( nCombo < vecCombo2PointMul[i].first )
		{
			nLevel = i;
			fPercent = i == 0 ? nCombo * 1.0f / vecCombo2PointMul[i].first :
				( nCombo - vecCombo2PointMul[i - 1].first ) * 1.0f / ( vecCombo2PointMul[i].first - vecCombo2PointMul[i - 1].first );
			return;
		}
	}
	nLevel = vecCombo2PointMul.size();
	fPercent = 0;
}

float CGlobalCfg::Combo2PointMul( uint32 nCombo )
{
	int32 nLevel;
	float fPercent;
	GetComboLevel( nCombo, nLevel, fPercent );
	return nLevel == 0 ? 1.0f : vecCombo2PointMul[nLevel - 1].second;
}

uint32 CGlobalCfg::Point2Reward( uint32 nPoint )
{
	int i;
	for( i = 0; i < vecPoint2Reward.size(); i++ )
	{
		if( vecPoint2Reward[i].first > nPoint )
			break;
	}
	if( i == 0 )
		return vecPoint2Reward[0].second * nPoint / vecPoint2Reward[0].first;
	else if( i >= vecPoint2Reward.size() )
		return vecPoint2Reward.back().second;
	else
	{
		auto a = vecPoint2Reward[i - 1];
		auto b = vecPoint2Reward[i];
		return a.second + ( b.second - a.second ) * ( nPoint - a.first ) / ( b.first - a.first );
	}
}

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
		for( auto pEntry = pLevel->FirstChildElement( "entries" )->FirstChildElement(); pEntry; pEntry = pEntry->NextSiblingElement() )
		{
			vecLvEntries.push_back( XmlGetAttr( pEntry, "lv", 0 ) );
		}

		auto pPrefabs = doc.RootElement()->FirstChildElement( "prefabs" );
		for( auto pItem = pPrefabs->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			const char* szName = XmlGetAttr( pItem, "name", "" );
			const char* szPath = XmlGetAttr( pItem, "path", "" );
			mapPrefabPath[szName] = szPath;
		}

		auto pComboLevels = doc.RootElement()->FirstChildElement( "combo_level" );
		for( auto pItem = pComboLevels->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			uint32 nCombo = XmlGetAttr( pItem, "combo", 0 );
			float fMul = XmlGetAttr( pItem, "mul", 1.0f );
			vecCombo2PointMul.push_back( pair<uint32, float>( nCombo, fMul ) );
		}

		auto pPoint2Reward = doc.RootElement()->FirstChildElement( "point2reward" );
		for( auto pItem = pPoint2Reward->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			uint32 nPoint = XmlGetAttr( pItem, "point", 0 );
			uint32 nReward = XmlGetAttr( pItem, "reward", 0);
			vecPoint2Reward.push_back( pair<uint32, uint32>( nPoint, nReward ) );
		}
	}

	{
		levelGenerateNodeContext.vecPaths.push_back( "configs/generate/" );
		pRootGenerateFile = levelGenerateNodeContext.LoadFile( "root", "configs/generate/" );
	}

	{
		vector<char> content;
		GetFileContent( content, "configs/drop.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );

		auto pNodes = doc.RootElement()->FirstChildElement( "nodes" );
		for( auto pNode = pNodes->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement() )
			CItemDropNode::CreateNode( pNode, itemDropNodeContext );

		auto pBonus = doc.RootElement()->FirstChildElement( "bonus_stage_drop" );
		bonusStageDrop.Load( pBonus, itemDropNodeContext );
	}
}
