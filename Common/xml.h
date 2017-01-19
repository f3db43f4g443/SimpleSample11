#pragma once
#include "tinyxml/tinyxml.h"
#include <sstream>
using namespace std;

void LoadXmlFile(TiXmlDocument& doc, const char* szName);

template< class T >
T XmlGetAttr( TiXmlElement *XMLNode, const char* szAttr, T defaultValue = 0 );

template< class T >
T XmlGetValue( TiXmlElement *XMLNode, const char* szValue, T defaultValue = 0 );

template< class T >
T XmlGet( TiXmlElement *XMLNode, const char* szKey, T defaultValue = 0 );

template<>
const char* XmlGetAttr(TiXmlElement *XMLNode, const char* szAttr, const char* defaultValue);

template<>
const char* XmlGetValue(TiXmlElement *XMLNode, const char* szValue, const char* defaultValue);

template<>
const char* XmlGet( TiXmlElement *XMLNode, const char* szKey, const char* defaultValue );

template< class T >
T XmlGetAttr( TiXmlElement *XMLNode, const char* szAttr, T defaultValue )
{
	const char* szBuffer = XmlGetAttr<const char*>( XMLNode, szAttr );
	if( !szBuffer || !szBuffer[0] )
		return defaultValue;
	stringstream ss;
	ss << szBuffer;
	T t;
	ss >> t;
	return t;
}

template< class T >
T XmlGetValue( TiXmlElement *XMLNode, const char* szValue, T defaultValue )
{
	const char* szBuffer = XmlGetValue<const char*>( XMLNode, szValue );
	if( !szBuffer || !szBuffer[0] )
		return defaultValue;
	stringstream ss;
	ss << szBuffer;
	T t;
	ss >> t;
	return t;
}

template< class T >
T XmlGet( TiXmlElement *XMLNode, const char* szKey, T defaultValue )
{
	const char* szBuffer = XmlGet<const char*>( XMLNode, szKey );
	if( !szBuffer || !szBuffer[0] )
		return defaultValue;
	stringstream ss;
	ss << szBuffer;
	T t;
	ss >> t;
	return t;
}
