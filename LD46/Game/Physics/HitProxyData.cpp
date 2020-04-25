#include "stdafx.h"
#include "HitProxyData.h"
#include "Common/xml.h"

CVector2 SHitProxyData::GetCenter()
{
	CVector2 center( 0, 0 );
	if( !nItems )
		return center;
	for( int i = 0; i < nItems; i++ )
	{
		auto& item = pItems[i];
		if( item.nType == eHitProxyType_Circle )
			center = center + item.center;
		else
		{
			CVector2 center1( 0, 0 );
			for( int i = 0; i < item.nVertices; i++ )
				center1 = center1 + item.vertices[i];
			center = center + center1 * ( 1.0f / item.nVertices );
		}
	}
	return center * ( 1.0f / nItems );
}

void SHitProxyData::LoadXml( TiXmlElement* pRoot )
{
	nItems = 0;
	for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
		nItems++;
	pItems = (SItem*)malloc( sizeof( SItem ) * nItems );
	
	nItems = 0;
	for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		SItem& item = pItems[nItems++];
		const char* szType = pElement->Value();
		if( !strcmp( szType, "circle" ) )
		{
			item.nType = eHitProxyType_Circle;
			item.fRadius = XmlGetAttr( pElement, "radius", 0.0f );
			item.center.x = XmlGetAttr( pElement, "x", 0.0f );
			item.center.y = XmlGetAttr( pElement, "y", 0.0f );
		}
		else if( !strcmp( szType, "rect" ) )
		{
			item.nType = eHitProxyType_Polygon;
			item.nVertices = 4;
			float x = XmlGetAttr( pElement, "x", 0.0f );
			float y = XmlGetAttr( pElement, "y", 0.0f );
			float width = XmlGetAttr( pElement, "width", 0.0f );
			float height = XmlGetAttr( pElement, "height", 0.0f );
			item.vertices[0] = CVector2( x, y );
			item.vertices[1] = CVector2( x + width, y );
			item.vertices[2] = CVector2( x + width, y + height );
			item.vertices[3] = CVector2( x, y + height );
		}
		else
		{
			item.nType = eHitProxyType_Polygon;
			item.nVertices = 0;
			const char* szVert = XmlGetAttr( pElement, "vert", "" );
			int i = 0;
			while( szVert[0] && item.nVertices < MAX_POLYGON_VERTEX_COUNT )
			{
				float f;
				sscanf( szVert, "%f", &f );
				if( i )
				{
					item.vertices[item.nVertices].y = f;
					item.nVertices++;
				}
				else
					item.vertices[item.nVertices].x = f;
					
				while( *szVert != ',' && *szVert != 0 )
					szVert++;
				if( *szVert )
					szVert++;
				i ^= 1;
			}
		}
	}
}

void CHitProxyDataSet::LoadXml( TiXmlElement* pRoot )
{
	uint32 nElemCount = 0;
	for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
		nElemCount++;
	m_data.resize( nElemCount );
	nElemCount = 0;
	for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		SHitProxyData& data = m_data[nElemCount++];
		data.LoadXml( pElement );
	}
}