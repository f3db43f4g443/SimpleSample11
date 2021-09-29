#pragma once
#include "Game/SdkInterface.h"
void Init_PlatformSDK();
void Shutdown_PlatformSDK();

class CDefaultSdkInterface : public ISdkInterface
{
public:
	CDefaultSdkInterface() {}
	virtual void Update() override {}
	virtual void UnlockAchievement( const char* sz ) override {}
	virtual void OpenExplorer( const char* sz ) override;
private:
};

#ifndef _STEAM
#define _PLATFORM_DEFAULT
#endif
