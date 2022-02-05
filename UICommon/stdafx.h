#pragma once

#include "Common/Common.h"
#include "Render/stdafx.h"

enum
{
	eEditorResType_UI = eEngineResType_End,
	eEditorResType_End,
};

#define VK_CANCEL			0x03
#define VK_BACK				0x08
#define VK_TAB				0x09
#define VK_RETURN			0x0D

#define VK_SHIFT			0x10
#define VK_CONTROL			0x11
#define VK_ESCAPE			0x1B
#define VK_DELETE			0x2E

#define VK_LEFT				0x25
#define VK_UP				0x26
#define VK_RIGHT			0x27
#define VK_DOWN				0x28

#define VK_F1				0x70
#define VK_F2				0x71
#define VK_F3				0x72
#define VK_F4				0x73
#define VK_F5				0x74
#define VK_F6				0x75
#define VK_F7				0x76
#define VK_F8				0x77
#define VK_F9				0x78
#define VK_F10				0x79
#define VK_F11				0x7A
#define VK_F12				0x7B