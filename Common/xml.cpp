#include "Common.h"
#include "xml.h"
#include "FileUtil.h"

void LoadXmlFile(TiXmlDocument& doc, const char* szName)
{
	vector<char> buffer;
	GetFileContent(buffer, szName, true);
	doc.LoadFromBuffer(&buffer[0]);
}

template<>
const char* XmlGetAttr(TiXmlElement *XMLNode, const char* szAttr, const char* defaultValue)
{
	const char* attr = XMLNode->Attribute(szAttr);
	if (attr)
		return attr;
	else
		return defaultValue;
}

template<>
const char* XmlGetValue(TiXmlElement *XMLNode, const char* szValue, const char* defaultValue)
{
	TiXmlElement* pElement = XMLNode->FirstChildElement(szValue);
	if (pElement)
		return pElement->GetText();
	else
		return defaultValue;
}

template<>
const char* XmlGet( TiXmlElement *XMLNode, const char* szKey, const char* defaultValue )
{
	const char* chr = strchr( szKey, '/' );
	const char* chr1 = strchr( szKey, '.' );
	bool IsGetValue = chr && !chr1;

	char* temp = (char*)alloca( strlen( szKey ) + 1 );
	strcpy( temp, szKey );
	char* c = temp;
	char* c0 = c;
	while( *c )
	{
		if( *c == '/' || *c == '.' )
		{
			*c = 0;
			if( c0 != c )
			{
				XMLNode = XMLNode->FirstChildElement( c0 );
				if( !XMLNode )
					return defaultValue;
			}
			c++;
			c0 = c;
		}
		else
			c++;
	}

	if( !*c0 )
		return defaultValue;
	return IsGetValue ? XmlGetValue( XMLNode, c0, defaultValue ) : XmlGetAttr( XMLNode, c0, defaultValue );
}