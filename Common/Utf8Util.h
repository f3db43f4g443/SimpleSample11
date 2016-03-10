#pragma once

#include <string>
using namespace std;

void Utf8ToUnicode( const char* szSrc, wstring& strDst );
void UnicodeToUtf8( const wchar_t* szSrc, string& strDst );