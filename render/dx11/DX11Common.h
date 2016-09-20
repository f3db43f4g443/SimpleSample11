#pragma once
#include "stdafx.h"
#include "DXUT/Core/DXUT.h"
#include <dsound.h>

inline DXGI_FORMAT GetDXGIFormat( EFormat eFormat )
{
	return (DXGI_FORMAT)eFormat;
}