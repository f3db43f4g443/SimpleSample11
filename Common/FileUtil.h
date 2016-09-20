#pragma once

#include <vector>
#include <functional>
using namespace std;

bool IsFileExist( const char* szFileName );
const char* GetFileExtension( const char* szFileName );
uint32 GetFileContent( vector<char>& result, const char* szFileName, bool bText );
const char* GetFileContent( const char* szFileName, bool bText, uint32& nLen );
void SaveFile( const char* szFileName, const void* pData, uint32 nLen );
bool CheckFileName( const char* szFileName, bool bWithExtension );
void FindFiles( const char* szName, function<bool( const char* )> onFind, bool bFile, bool bFolder );
