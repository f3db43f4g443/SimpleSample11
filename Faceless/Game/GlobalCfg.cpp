#include "stdafx.h"
#include "GlobalCfg.h"
#include "Common/xml.h"
#include "Common/FileUtil.h"
#include "Common/ResourceManager.h"

void CGlobalCfg::Load()
{
	vector<char> content;
	GetFileContent( content, "materials/enemybullet_particle1.xml", true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );

	{
		auto pResources = doc.RootElement()->FirstChildElement( "resources" );
		const char* szFaceEditTile = XmlGetAttr( pResources, "face_edit_tile", "" );
		if( szFaceEditTile )
			pFaceEditTile = CResourceManager::Inst()->CreateResource<CDrawableGroup>( szFaceEditTile );
		const char* szFaceSelectTile = XmlGetAttr( pResources, "face_select_tile", "" );
		if( szFaceSelectTile )
			pFaceSelectTile = CResourceManager::Inst()->CreateResource<CDrawableGroup>( szFaceSelectTile );
		const char* szFaceSelectRed = XmlGetAttr( pResources, "face_select_red", "" );
		if( szFaceSelectRed )
			pFaceSelectRed = CResourceManager::Inst()->CreateResource<CDrawableGroup>( szFaceSelectRed );
		const char* szFaceSelectBullet = XmlGetAttr( pResources, "face_select_bullet", "" );
		if( szFaceSelectBullet )
			pFaceSelectBullet = CResourceManager::Inst()->CreateResource<CDrawableGroup>( szFaceSelectBullet );
	}
}
