#include "Platform_Common.h"
#ifdef _PLATFORM_DEFAULT

void Init_PlatformSDK()
{
	static CDefaultSdkInterface g_inst;
	ISdkInterface::Init( &g_inst );
}
void Shutdown_PlatformSDK() {}

#endif
