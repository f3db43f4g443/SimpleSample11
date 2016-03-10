#include "stdafx.h"
#include "TextureAtlas.h"
#include "xml.h"
#include "ResourceManager.h"

CTextureAtlas::SItem* CTextureAtlas::GetItem( const char* szName )
{
	auto itr = m_mapItems.find( szName );
	if( itr == m_mapItems.end() )
		return false;
	return &itr->second;
}

void CTextureAtlas::LoadXml( TiXmlElement* pRoot )
{
	m_strTextureName = XmlGetAttr( pRoot, "imagePath", "" );
	m_size.x = XmlGetAttr( pRoot, "width", 0 );
	m_size.y = XmlGetAttr( pRoot, "height", 0 );
	
	for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		const char* szName = XmlGetAttr( pElement, "name", "" );
		SItem& item = m_mapItems[szName];
		item.rect.x = XmlGetAttr( pElement, "x", 0.0f );
		item.rect.y = XmlGetAttr( pElement, "y", 0.0f );
		item.rect.width = XmlGetAttr( pElement, "width", 0.0f );
		item.rect.height = XmlGetAttr( pElement, "height", 0.0f );
		item.ofs.x = XmlGetAttr( pElement, "ofsx", 0.0f );
		item.ofs.y = XmlGetAttr( pElement, "ofsy", 0.0f );
		
		float fFrameX = XmlGetAttr( pElement, "frameX", 0.0f );
		float fFrameY = XmlGetAttr( pElement, "frameY", 0.0f );
		float fFrameWidth = XmlGetAttr( pElement, "frameWidth", item.rect.width );
		float fFrameHeight = XmlGetAttr( pElement, "frameHeight", item.rect.height );
		item.frameRect = CRectangle( -fFrameX - fFrameWidth * 0.5f, -fFrameY - fFrameHeight * 0.5f, item.rect.width, item.rect.height );
		item.frameRect.y = -item.frameRect.y - item.rect.height;
	}
}