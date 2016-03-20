#pragma once
#include "Math3D.h"

class IGame
{
public:
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void Update() = 0;
	
	virtual void OnMouseDown( const CVector2& pos ) {}
	virtual void OnMouseUp( const CVector2& pos ) {}
	virtual void OnMouseMove( const CVector2& pos ) {}
	virtual void OnKey( uint32 nChar, bool bKeyDown, bool bAltDown ) {}
	virtual void OnChar( uint32 nChar ) {}
};