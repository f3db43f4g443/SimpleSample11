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
