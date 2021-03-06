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
		szPrefab = XmlGetValue( pPrefabs, "fall_eft", "" );
		pFallEftDrawable = CResourceManager::Inst()->CreateResource<CDrawableGroup>( szPrefab );
		szPrefab = XmlGetValue( pPrefabs, "interference_strip_eft", "" );
		pInterferenceStripEftPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
		szPrefab = XmlGetValue( pPrefabs, "tracer_spawn_eft", "" );
		pTracerSpawnEftPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
		szPrefab = XmlGetValue( pPrefabs, "common_link", "" );
		pCommonLinkPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szPrefab );
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
		for( auto pItem = pIndicator->FirstChildElement( "pawn0" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecPawn0Params.resize( lvIndicatorData.vecPawn0Params.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecPawn0Params.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "pawn_fall" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecPawnFallParams.resize( lvIndicatorData.vecPawnFallParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecPawnFallParams.back() );
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
		for( auto pItem = pIndicator->FirstChildElement( "stealth_back_1" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecStealthBackParams1.resize( lvIndicatorData.vecStealthBackParams1.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecStealthBackParams1.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "stealth_back_2" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecStealthBackParams2.resize( lvIndicatorData.vecStealthBackParams2.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecStealthBackParams2.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "stealth_alert" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecStealthAlertParams.resize( lvIndicatorData.vecStealthAlertParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecStealthAlertParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "stealth_detect" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecStealthDetectParams.resize( lvIndicatorData.vecStealthDetectParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecStealthDetectParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "stealth_hidden" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecStealthHiddenParams.resize( lvIndicatorData.vecStealthHiddenParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecStealthHiddenParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "stealth_alert_hidden" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecStealthAlertHiddenParams.resize( lvIndicatorData.vecStealthAlertHiddenParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecStealthAlertHiddenParams.back() );
		}
		for( auto pItem = pIndicator->FirstChildElement( "stealth_detect_hidden" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			lvIndicatorData.vecStealthDetectHiddenParams.resize( lvIndicatorData.vecStealthDetectHiddenParams.size() + 1 );
			ReadFunc( pItem, lvIndicatorData.vecStealthDetectHiddenParams.back() );
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

	{
		int32 nItems = 0;
		CVector4 param[2] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
		for( auto pItem = doc.RootElement()->FirstChildElement( "show_snap_shot_mask" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			int32 nFrame = XmlGetAttr( pItem, "t", 0 );
			CVector4 param1[2];
			param1[0].x = XmlGetAttr( pItem, "x0", 0.0f );
			param1[0].y = XmlGetAttr( pItem, "y0", 0.0f );
			param1[0].z = XmlGetAttr( pItem, "z0", 0.0f );
			param1[0].w = XmlGetAttr( pItem, "w0", 0.0f );
			param1[1].x = XmlGetAttr( pItem, "x1", 0.0f );
			param1[1].y = XmlGetAttr( pItem, "y1", 0.0f );
			param1[1].z = XmlGetAttr( pItem, "z1", 0.0f );
			param1[1].w = XmlGetAttr( pItem, "w1", 0.0f );
			showSnapShotMask.resize( nItems + nFrame );
			for( int i = 0; i < nFrame; i++ )
			{
				float t = ( i + 1.0f ) / nFrame;
				showSnapShotMask[nItems + i].first = param[0] + ( param1[0] - param[0] ) * t;
				showSnapShotMask[nItems + i].second = param[1] + ( param1[1] - param[1] ) * t;
			}
			param[0] = param1[0];
			param[1] = param1[1];
			nItems += nFrame;
		}
	}
	{
		int32 nItems = 0;
		CVector4 param[2];
		CVector4 param0[2];
		CVector4 param1[2];
		int32 nFrame0 = -1;
		for( auto pItem = doc.RootElement()->FirstChildElement( "battle_effect_mask" )->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
		{
			int32 nFrame = XmlGetAttr( pItem, "t", 0 );
			param1[0].x = XmlGetAttr( pItem, "x0", 0.0f );
			param1[0].y = XmlGetAttr( pItem, "y0", 0.0f );
			param1[0].z = XmlGetAttr( pItem, "z0", 0.0f );
			param1[0].w = XmlGetAttr( pItem, "w0", 0.0f );
			param1[1].x = XmlGetAttr( pItem, "x1", 0.0f );
			param1[1].y = XmlGetAttr( pItem, "y1", 0.0f );
			param1[1].z = XmlGetAttr( pItem, "z1", 0.0f );
			param1[1].w = XmlGetAttr( pItem, "w1", 0.0f );

			if( nFrame0 < 0 )
			{
				nFrame0 = nFrame;
				param0[0] = param1[0];
				param0[1] = param1[1];
			}
			else
			{
				battleEffectMask.resize( nItems + nFrame );
				for( int i = 0; i < nFrame; i++ )
				{
					float t = ( i + 1.0f ) / nFrame;
					battleEffectMask[nItems + i].first = param[0] + ( param1[0] - param[0] ) * t;
					battleEffectMask[nItems + i].second = param[1] + ( param1[1] - param[1] ) * t;
				}
				nItems += nFrame;
			}
			param[0] = param1[0];
			param[1] = param1[1];
		}
		battleEffectMask.resize( nItems + nFrame0 );
		for( int i = 0; i < nFrame0; i++ )
		{
			float t = ( i + 1.0f ) / nFrame0;
			battleEffectMask[nItems + i].first = param[0] + ( param0[0] - param[0] ) * t;
			battleEffectMask[nItems + i].second = param[1] + ( param0[1] - param[1] ) * t;
		}
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
