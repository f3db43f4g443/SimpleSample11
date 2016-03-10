#pragma once

class IGame
{
public:
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void Update() = 0;
	
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) = 0;
};