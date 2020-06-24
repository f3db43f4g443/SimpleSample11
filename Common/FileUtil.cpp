#include "Common.h"
#include "FileUtil.h"

#ifdef _WIN32
#include <io.h>
#include <direct.h>

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

const char* GetFileExtension( const char* szFileName )
{
	const char* szExt = "";
	for( const char* ch = szFileName; *ch; ch++ )
	{
		if( *ch == '.' )
			szExt = ch + 1;
	}
	return szExt;
}

uint32 GetFileContent( vector<char>& result, const char* szFileName, bool bText )
{
	FILE* f = fopen( szFileName, "rb" );
	if( !f )
		return INVALID_32BITID;
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
	if( bText )
	{
		result.resize( nSize + 1 );
		result[nSize] = 0;
	}
	else
		result.resize( nSize );
	if( nSize )
		fread( &result[0], 1, nSize, f );
	fclose( f );
	return nSize;
}

const char* GetFileContent( const char* szFileName, bool bText, uint32& nLen )
{
	FILE* f = fopen( szFileName, "rb" );
	if( !f )
		return NULL;
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

bool SaveFile( const char* szFileName, const void* pData, uint32 nLen )
{
	char* c = (char*)alloca( strlen( szFileName ) + 1 );
	strcpy( c, szFileName );

	for( char* c1 = c; *c1; c1++ )
	{
		if( *c1 == '/' )
		{
			*c1 = 0;
			if( access( c, 6 ) == -1 )
				mkdir( c );
			*c1 = '/';
		}
	}

	if( !pData )
	{
		FILE* f = fopen( szFileName, "rb" );
		if( f )
		{
			fclose( f );
			return false;
		}
	}
	FILE* f = fopen( szFileName, "wb" );
	if( !f )
		return false;
	if( pData )
		fwrite( pData, nLen, 1, f );
	fclose( f );
	return true;
}

bool CheckFileName( const char* szFileName, bool bWithExtension )
{
	if( !szFileName || !szFileName[0] )
		return false;
	for( ; *szFileName; szFileName++ )
	{
		char c = *szFileName;
		if( c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' )
			continue;
		if( c == '.' && bWithExtension )
			continue;
		return false;
	}
	return true;
}

void FindFiles( const char* szName, function<bool( const char* )> onFind, bool bFile, bool bFolder )
{
	_finddata_t file;
	int k, HANDLE;
	k = HANDLE = _findfirst( szName, &file );
	if( HANDLE == -1 )
		return;

	while( k != -1 )
	{
		if( strcmp( file.name, "." ) != 0 && strcmp( file.name, ".." ) != 0 )
		{
			if( file.attrib == _A_SUBDIR && bFolder || file.attrib != _A_SUBDIR && bFile )
			{
				if( !onFind( file.name ) )
					break;
			}
		}
		k = _findnext( HANDLE, &file );
	}
	_findclose( HANDLE );
}
#endif
