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

	auto pWorld = doc.RootElement()->FirstChildElement( "world" );
	strCfgPath = XmlGetAttr( pWorld, "cfg_path", "" );
	strEntry = XmlGetAttr( pWorld, "entry", "" );
	playerEnterPos.x = XmlGetAttr( pWorld, "entry_x", 0 );
	playerEnterPos.y = XmlGetAttr( pWorld, "entry_y", 0 );
	nPlayerEnterDir = XmlGetAttr( pWorld, "entry_dir", 0 );
	const char* szLuaStart = XmlGetAttr( pWorld, "lua_start", "" );
	const char* szLuaWorldInit = XmlGetAttr( pWorld, "lua_world_init", "" );

	auto pPrefabs = doc.RootElement()->FirstChildElement( "prefabs" );
	{
		const char* szPrefab = XmlGetValue( pPrefabs, "fail_lightning_eft", "" );
		pFailLightningEffectPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
	}

	auto pSoundEfts = doc.RootElement()->FirstChildElement( "sound" );
	for( auto pItem = pSoundEfts->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
	{
		auto szName = XmlGetAttr( pItem, "name", "" );
		auto szPath = XmlGetAttr( pItem, "path", "" );
		mapSoundEffect[szName] = CResourceManager::Inst()->CreateResource<CSoundFile>( szPath );
	}

	auto pTransfer = doc.RootElement()->FirstChildElement( "level_transfer" );
	lvTransferData.fTransferCamSpeed = XmlGetAttr( pTransfer, "cam_speed", 5.0f );
	lvTransferData.nTransferFadeOutFrameCount = XmlGetAttr( pTransfer, "fadeout", 45 );
	for( auto pItem = pTransfer->FirstChildElement( "mask_frames" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
	{
		lvTransferData.vecTransferMaskParams.resize( lvTransferData.vecTransferMaskParams.size() + 1 );
		auto& item = lvTransferData.vecTransferMaskParams.back();
		item.first.x = XmlGetAttr( pItem, "x0", 0.0f );
		item.first.y = XmlGetAttr( pItem, "y0", 0.0f );
		item.first.z = XmlGetAttr( pItem, "z0", 0.0f );
		item.first.w = XmlGetAttr( pItem, "w0", 0.0f );
		item.second.x = XmlGetAttr( pItem, "x1", 0.0f );
		item.second.y = XmlGetAttr( pItem, "y1", 0.0f );
		item.second.z = XmlGetAttr( pItem, "z1", 0.0f );
		item.second.w = XmlGetAttr( pItem, "w1", 0.0f );
	}

	auto pIndicator = doc.RootElement()->FirstChildElement( "level_indicator" );
	{
		string str = XmlGetAttr( pIndicator, "prefab", "" );
		lvIndicatorData.pIndicatorPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( str.c_str() );

		typedef decltype( lvIndicatorData.NextLevelParamMin ) ParamType;
		auto ReadFunc = [] ( TiXmlElement* pItem, ParamType& param ) {
			param.ofs.x = XmlGetAttr( pItem, "ofs_x", 0 );
			param.ofs.y = XmlGetAttr( pItem, "ofs_y", 0 );
			param.params[0].x = XmlGetAttr( pItem, "x0", 0.0f );
			param.params[0].y = XmlGetAttr( pItem, "y0", 0.0f );
			param.params[0].z = XmlGetAttr( pItem, "z0", 0.0f );
			param.params[0].w = XmlGetAttr( pItem, "w0", 0.0f );
			param.params[1].x = XmlGetAttr( pItem, "x1", 0.0f );
			param.params[1].y = XmlGetAttr( pItem, "y1", 0.0f );
			param.params[1].z = XmlGetAttr( pItem, "z1", 0.0f );
			param.params[1].w = XmlGetAttr( pItem, "w1", 0.0f );
		};

		for( auto pItem = pIndicator->FirstChildElement( "pawn" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecPawnParams.resize( lvIndicatorData.vecPawnParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecPawnParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "use" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecUseParams.resize( lvIndicatorData.vecUseParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecUseParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "mount" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecMountParams.resize( lvIndicatorData.vecMountParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecMountParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "hit" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecHitParams.resize( lvIndicatorData.vecHitParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecHitParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "hit_blocked" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecHitBlockedParams.resize( lvIndicatorData.vecHitBlockedParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecHitBlockedParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "miss" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecMissParams.resize( lvIndicatorData.vecMissParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecMissParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "blocked" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecBlockedParams.resize( lvIndicatorData.vecBlockedParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecBlockedParams.back() );
		}
		lvIndicatorData.vecBlockedParams1.resize( lvIndicatorData.vecBlockedParams.size() );
		for( int i = 0; i < lvIndicatorData.vecBlockedParams.size(); i++ )
		{
			auto& param0 = lvIndicatorData.vecBlockedParams[i];
			lvIndicatorData.vecBlockedParams1[i].params[0] = CVector4( 1, 1, 1, param0.ofs.x );
			lvIndicatorData.vecBlockedParams1[i].params[1] = CVector4( 0, 0, 0, param0.ofs.y );
			lvIndicatorData.vecBlockedParams1[i].ofs = CVector2( 0, 0 );
		}

		auto pNextLevelCfg = pIndicator->FirstChildElement( "next_level" );
		ReadFunc( pNextLevelCfg->FirstChildElement( "min" ), lvIndicatorData.NextLevelParamMin );
		ReadFunc( pNextLevelCfg->FirstChildElement( "max" ), lvIndicatorData.NextLevelParamMax );
		lvIndicatorData.nNextLevelParamCount = XmlGetAttr( pNextLevelCfg, "count", 0 );
		auto pNextLevelBlockedCfg = pIndicator->FirstChildElement( "next_level_blocked" );
		ReadFunc( pNextLevelBlockedCfg->FirstChildElement( "min" ), lvIndicatorData.NextLevelBlockedParamMin );
		ReadFunc( pNextLevelBlockedCfg->FirstChildElement( "max" ), lvIndicatorData.NextLevelBlockedParamMax );
		lvIndicatorData.nNextLevelBlockedParamCount = XmlGetAttr( pNextLevelBlockedCfg, "count", 0 );
	}

	auto pGUI = doc.RootElement()->FirstChildElement( "main_ui" );
	{
		for( auto pFrame = pGUI->FirstChildElement( "action_eft_frames" )->FirstChildElement(); pFrame; pFrame = pFrame->NextSiblingElement() )
		{
			MainUIData.vecActionEftFrames.resize( MainUIData.vecActionEftFrames.size() + 1 );
			auto& frame = MainUIData.vecActionEftFrames.back();
			frame.nTotalHeight = XmlGetAttr( pFrame, "total_height", 0 );
			frame.nMaxImgHeight = XmlGetAttr( pFrame, "max_img_height", 0 );
			int32 i = 0;
			for( auto pParam = pFrame->FirstChildElement(); pParam && i < 6; pParam = pParam->NextSiblingElement(), i++ )
			{
				auto& param = frame.params[i];
				param.fOfs = XmlGetAttr( pParam, "ofs", 0.0f );
				param.fLum = XmlGetAttr( pParam, "lum", 0.0f );
				param.fOpaque = XmlGetAttr( pParam, "opaque", 0.0f );
			}
		}
		for( auto pItem = pGUI->FirstChildElement( "player_input_params" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			MainUIData.vecPlayerInputParams.resize( MainUIData.vecPlayerInputParams.size() + 1 );
			auto& item = MainUIData.vecPlayerInputParams.back();
			item.params[1].x = XmlGetAttr( pItem, "x", 0.0f );
			item.params[1].y = XmlGetAttr( pItem, "y", 0.0f );
			item.params[1].z = XmlGetAttr( pItem, "z", 0.0f );
			item.params[0].x = item.params[0].y = item.params[0].z = 1 - XmlGetAttr( pItem, "w", 0.0f );
			item.params[0].w = item.params[1].w = 0;
			item.params[2] = CVector4( 0.5f, 0.5f, 0.5f, 0 ) + item.params[0] * 0.5f;
			item.params[3] = item.params[1] * 0.5f;
			item.ofs.x = XmlGetAttr( pItem, "ofs_x", 0.0f );
			item.ofs.y = XmlGetAttr( pItem, "ofs_y", 0.0f );
		}
	}

	for( auto pItem = doc.RootElement()->FirstChildElement( "player_dmg_mask" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
	{
		playerDamagedMask.resize( playerDamagedMask.size() + 1 );
		auto& item = playerDamagedMask.back();
		item.first.x = XmlGetAttr( pItem, "x0", 0.0f );
		item.first.y = XmlGetAttr( pItem, "y0", 0.0f );
		item.first.z = XmlGetAttr( pItem, "z0", 0.0f );
		item.first.w = XmlGetAttr( pItem, "w0", 0.0f );
		item.second.x = XmlGetAttr( pItem, "x1", 0.0f );
		item.second.y = XmlGetAttr( pItem, "y1", 0.0f );
		item.second.z = XmlGetAttr( pItem, "z1", 0.0f );
		item.second.w = XmlGetAttr( pItem, "w1", 0.0f );
	}

	if( szLuaStart[0] )
	{
		uint32 nLen;
		const char* sz = GetFileContent( szLuaStart, true, nLen );
		CLuaMgr::Inst().Run( sz );
		free( (void*)sz );
	}
	if( szLuaWorldInit[0] )
	{
		uint32 nLen;
		const char* sz = GetFileContent( szLuaWorldInit, true, nLen );
		strWorldInitScript = sz;
		free( (void*)sz );
	}
}
