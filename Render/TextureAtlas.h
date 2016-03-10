#pragma once
#include "Texture.h"
#include "Math3D.h"

class TiXmlElement;
class CTextureAtlas : public CReferenceObject
{
public:
	struct SItem
	{
		CRectangle rect;
		CRectangle frameRect;
		CVector2 ofs;
	};

	const CVector2& GetSize() { return m_size; }
	SItem* GetItem( const char* szName );
	const char* GetTexture() { return m_strTextureName.c_str(); }

	void LoadXml( TiXmlElement* pRoot );
private:
	string m_strTextureName;
	map<string, SItem> m_mapItems;
	CVector2 m_size;
};