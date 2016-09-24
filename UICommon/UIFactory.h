#pragma once
#include "UIElement.h"
#include "Common/Resource.h"

class CUIFactory
{
public:
	CUIFactory();

	CUIElement* LoadXml( TiXmlElement* pRoot );
	CUIElement* LoadXmlFile( const char* szFile );
	void LoadXml( CUIElement* pElem, TiXmlElement* pRoot );
	void LoadXmlFile( CUIElement* pElem, const char* szFile );

	DECLARE_GLOBAL_INST_REFERENCE( CUIFactory )
private:
	map<string, CUIElement*> m_pElementTypes;
};

class CUIResource : public CResource
{
public:
	enum EType
	{
		eResType = eEditorResType_UI,
	};
	CUIResource( const char* name, int32 type ) : CResource( name, type ) {}
	void Create();

	CUIElement* GetElement() { return m_pUIElement; }
private:
	CReference<CUIElement> m_pUIElement;
};