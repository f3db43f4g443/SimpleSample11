#include "Platform_Common.h"
#include "Common/Utf8Util.h"
#include <Windows.h>

void CDefaultSdkInterface::OpenExplorer( const char* sz )
{
	auto str = Utf8ToUnicode( sz );
	ShellExecute( NULL, L"open", str.c_str(), NULL, NULL, SW_SHOW );
}
