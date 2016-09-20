#include "Common.h"
#include "Utf8Util.h"

void Utf8ToUnicode( const char* szSrc, wstring& strDst )
{
	const char* p = szSrc;
	while( *p )
	{
		wchar_t ch = 0;
		char c0 = *p++;
		if( c0 < 0x80 )
		{
			ch = c0;
		}
		else if( c0 < 0xe0 )
		{
			char c1 = *p++;
			if( c1 < 0xc0 )
				break;
			ch = (wchar_t)( ( (wchar_t)c0 & 0x1f ) << 6 |
				( (wchar_t)c1 & 0x3f ) );
		}
		else if( c0 < 0xf0 )
		{
			char c1 = *p++;
			if( c1 < 0xc0 )
				break;
			char c2 = *p++;
			if( c1 < 0xc0 )
				break;
			ch = (wchar_t)( ( (wchar_t)c0 << 12 ) |
				( ( (wchar_t)c1 & 0x3f ) << 6 ) |
				( (wchar_t)c2 & 0x3f ) );
		}
		else
			break;
		strDst.push_back( ch );
	}
}

wstring Utf8ToUnicode( const char* szSrc )
{
	wstring str;
	Utf8ToUnicode( szSrc, str );
	return str;
}

void UnicodeToUtf8( const wchar_t* szSrc, string& strDst )
{
	const wchar_t* p = szSrc;
	while( *p )
	{
		wchar_t c0 = *p++;
		if( c0 < 0x80 )
		{
			strDst.push_back( c0 );
		}
		else if( c0 < 0x800 )
		{
			strDst.push_back( (char)( 0xc0 | ( ( c0 >> 6 ) & 0x1f ) ) );
			strDst.push_back( (char)( 0x80 | ( c0 & 0x3f ) ) );
		}
		else
		{
			strDst.push_back( (char)( 0xe0 | ( ( c0 >> 12 ) & 0xf ) ) );
			strDst.push_back( (char)( 0x80 | ( ( c0 >> 6 ) & 0x3f ) ) );
			strDst.push_back( (char)( 0x80 | ( c0 & 0x3f ) ) );
		}
	}
}

string UnicodeToUtf8( const wchar_t* szSrc )
{
	string str;
	UnicodeToUtf8( szSrc, str );
	return str;
}