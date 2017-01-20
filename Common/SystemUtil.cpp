#include "Common.h"
#include "SystemUtil.h"
#include"Utf8Util.h"
#ifdef _WIN32
#include <windows.h>

string GetClipboard()
{
	string result;
	if( OpenClipboard( NULL ) )
	{
		HANDLE handle = GetClipboardData( CF_UNICODETEXT );
		if( handle )
		{
			// Convert the ANSI string to Unicode, then
			// insert to our buffer.
			WCHAR* pwszText = (WCHAR*)GlobalLock( handle );
			if( pwszText )
			{
				result = UnicodeToUtf8( pwszText );
				GlobalUnlock( handle );
			}
		}
		CloseClipboard();
	}
	return result;
}

void SetClipboard( const char * szText )
{
	if( szText && szText[0] && OpenClipboard( NULL ) )
	{
		EmptyClipboard();
		wstring wstr = Utf8ToUnicode( szText );

		HGLOBAL hBlock = GlobalAlloc( GMEM_MOVEABLE, sizeof( WCHAR ) * ( wstr.length() + 1 ) );
		if( hBlock )
		{
			WCHAR* pwszText = (WCHAR*)GlobalLock( hBlock );
			if( pwszText )
			{
				CopyMemory( pwszText, wstr.c_str(), wstr.length() * sizeof( WCHAR ) );
				pwszText[wstr.length()] = L'\0';  // Terminate it
				GlobalUnlock( hBlock );
			}
			SetClipboardData( CF_UNICODETEXT, hBlock );
		}
		CloseClipboard();
		// We must not free the object until CloseClipboard is called.
		if( hBlock )
			GlobalFree( hBlock );
	}
}

#endif
