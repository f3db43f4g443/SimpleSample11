#pragma once

#include <vector>
using namespace std;

bool IsFileExist( const char* szFileName );
uint32 GetFileContent( vector<char>& result, const char* szFileName, bool bText );
const char* GetFileContent( const char* szFileName, bool bText, uint32& nLen );
