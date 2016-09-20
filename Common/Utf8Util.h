#pragma once

#include <string>
using namespace std;

void Utf8ToUnicode( const char* szSrc, wstring& strDst );
wstring Utf8ToUnicode( const char* szSrc );
void UnicodeToUtf8( const wchar_t* szSrc, string& strDst );
string UnicodeToUtf8( const wchar_t* szSrc );