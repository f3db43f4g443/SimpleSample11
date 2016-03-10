#include "Common.h"
#include "FileUtil.h"

#ifdef _WIN32

bool IsFileExist( const char* szFileName )
{
	FILE* f = fopen( szFileName, "r" );
	if( f )
	{
		fclose( f );
		return true;
	}
	return false;
}

uint32 GetFileContent( vector<char>& result, const char* szFileName, bool bText )
{
	FILE* f = fopen( szFileName, "rb" );
	fseek( f, 0, SEEK_END );
	uint32 nSize = ftell( f );
	fseek( f, 0, SEEK_SET );
	if( bText )
	{
		if( nSize > 3 )
		{
			char szBom[] = { "\xef\xbb\xbf" };
			char szBuf[3];
			fread( szBuf, 1, 3, f );
			if( memcmp( szBom, szBuf, 3 ) )
				fseek( f, 0, SEEK_SET );
			else
				nSize -= 3;
		}
	}
	result.resize( nSize + 1 );
	result[nSize] = 0;
	fread( &result[0], 1, nSize, f );
	fclose( f );
	return nSize;
}

const char* GetFileContent( const char* szFileName, bool bText, uint32& nLen )
{
	FILE* f = fopen( szFileName, "rb" );
	fseek( f, 0, SEEK_END );
	uint32 nSize = ftell( f );
	fseek( f, 0, SEEK_SET );
	if( bText )
	{
		if( nSize > 3 )
		{
			char szBom[] = { "\xef\xbb\xbf" };
			char szBuf[3];
			fread( szBuf, 1, 3, f );
			if( memcmp( szBom, szBuf, 3 ) )
				fseek( f, 0, SEEK_SET );
			else
				nSize -= 3;
		}
	}
	char* result = (char*)malloc( nSize + 1 );
	fread( &result[0], 1, nSize, f );
	result[nSize] = 0;
	fclose( f );
	nLen = nSize;
	return result;
}
#endif
