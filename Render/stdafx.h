// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "Common.h"

enum class EComponentFormat
{
	Unknown = 0,
	UInt32 = 1,
	SInt32 = 2,
	Float32 = 3
};

enum class EFormat
{
	EFormatUNKNOWN = 0,
	EFormatR32G32B32A32Typeless = 1,
	EFormatR32G32B32A32Float = 2,
	EFormatR32G32B32A32UInt = 3,
	EFormatR32G32B32A32SInt = 4,
	EFormatR32G32B32Typeless = 5,
	EFormatR32G32B32Float = 6,
	EFormatR32G32B32UInt = 7,
	EFormatR32G32B32SInt = 8,
	EFormatR16G16B16A16Typeless = 9,
	EFormatR16G16B16A16Float = 10,
	EFormatR16G16B16A16UNorm = 11,
	EFormatR16G16B16A16UInt = 12,
	EFormatR16G16B16A16SNorm = 13,
	EFormatR16G16B16A16SInt = 14,
	EFormatR32G32Typeless = 15,
	EFormatR32G32Float = 16,
	EFormatR32G32UInt = 17,
	EFormatR32G32SInt = 18,
	EFormatR32G8X24Typeless = 19,
	EFormatD32FloatS8X24UInt = 20,
	EFormatR32FloatX8X24Typeless = 21,
	EFormatX32TypelessG8X24UInt = 22,
	EFormatR10G10B10A2Typeless = 23,
	EFormatR10G10B10A2UNorm = 24,
	EFormatR10G10B10A2UInt = 25,
	EFormatR11G11B10Float = 26,
	EFormatR8G8B8A8Typeless = 27,
	EFormatR8G8B8A8UNorm = 28,
	EFormatR8G8B8A8UNormSRGB = 29,
	EFormatR8G8B8A8UInt = 30,
	EFormatR8G8B8A8SNorm = 31,
	EFormatR8G8B8A8SInt = 32,
	EFormatR16G16Typeless = 33,
	EFormatR16G16Float = 34,
	EFormatR16G16UNorm = 35,
	EFormatR16G16UInt = 36,
	EFormatR16G16SNorm = 37,
	EFormatR16G16SInt = 38,
	EFormatR32Typeless = 39,
	EFormatD32Float = 40,
	EFormatR32Float = 41,
	EFormatR32UInt = 42,
	EFormatR32SInt = 43,
	EFormatR24G8Typeless = 44,
	EFormatD24UNormS8UInt = 45,
	EFormatR24UNormX8Typeless = 46,
	EFormatX24TypelessG8UInt = 47,
	EFormatR8G8Typeless = 48,
	EFormatR8G8UNorm = 49,
	EFormatR8G8UInt = 50,
	EFormatR8G8SNorm = 51,
	EFormatR8G8SInt = 52,
	EFormatR16Typeless = 53,
	EFormatR16Float = 54,
	EFormatD16UNorm = 55,
	EFormatR16UNorm = 56,
	EFormatR16UInt = 57,
	EFormatR16SNorm = 58,
	EFormatR16SInt = 59,
	EFormatR8Typeless = 60,
	EFormatR8UNorm = 61,
	EFormatR8UInt = 62,
	EFormatR8SNorm = 63,
	EFormatR8SInt = 64,
	EFormatA8UNorm = 65,
	EFormatR1UNorm = 66,
	EFormatR9G9B9E5SHAREDEXP = 67,
	EFormatR8G8B8G8UNorm = 68,
	EFormatG8R8G8B8UNorm = 69,
	EFormatBC1Typeless = 70,
	EFormatBC1UNorm = 71,
	EFormatBC1UNorm_SRGB = 72,
	EFormatBC2Typeless = 73,
	EFormatBC2UNorm = 74,
	EFormatBC2UNorm_SRGB = 75,
	EFormatBC3Typeless = 76,
	EFormatBC3UNorm = 77,
	EFormatBC3UNormSRGB = 78,
	EFormatBC4Typeless = 79,
	EFormatBC4UNorm = 80,
	EFormatBC4SNorm = 81,
	EFormatBC5Typeless = 82,
	EFormatBC5UNorm = 83,
	EFormatBC5SNorm = 84,
	EFormatB5G6R5UNorm = 85,
	EFormatB5G5R5A1UNorm = 86,
	EFormatB8G8R8A8UNorm = 87,
	EFormatB8G8R8X8UNorm = 88,
	EFormatR10G10B10XRBiasA2UNorm = 89,
	EFormatB8G8R8A8Typeless = 90,
	EFormatB8G8R8A8UNorm_SRGB = 91,
	EFormatB8G8R8X8Typeless = 92,
	EFormatB8G8R8X8UNorm_SRGB = 93,
	EFormatBC6HTypeless = 94,
	EFormatBC6HUF16 = 95,
	EFormatBC6HSF16 = 96,
	EFormatBC7Typeless = 97,
	EFormatBC7UNorm = 98,
	EFormatBC7UNormSRGB = 99,
	EFormatFORCEUInt = 0xffffffff
};

uint8 FormatToLength( EFormat eFormat );

enum class EPrimitiveType
{
	Undefined = 0,
	PointList = 1,
	LineList = 2,
	LineStrip = 3,
	TriangleList = 4,
	TriangleStrip = 5,
	LineListAdj = 10,
	LineStripAdj = 11,
	TriangleListAdj = 12,
	TriangleStripAdj = 13,
};

enum EComparison
{
	EComparisonNever = 1,
	EComparisonLess = 2,
	EComparisonEqual = 3,
	EComparisonLessEqual = 4,
	EComparisonGreater = 5,
	EComparisonNotEqual = 6,
	EComparisonGreaterEqual = 7,
	EComparisonAlways = 8
};

enum
{
	eEngineResType_Texture,
	eEngineResType_DrawableGroup,
	eEngineResType_ParticleSystem,
	eEngineResType_Prefab,
	eEngineResType_Font,
	eEngineResType_Sound,
	eEngineResType_DynamicTexture,

	eEngineResType_End,
};

#define MAX_CONSTANT_BUFFER_BIND_COUNT 14
#define MAX_CONSTANT_BUFFER_SIZE 0x10000
#define MAX_SHADER_RESOURCE_BIND_COUNT 128
#define MAX_SAMPLER_COUNT 16
#define MAX_VERTEX_BUFFER_BIND_COUNT 16
#define MAX_RENDER_TARGETS 8
#define MAX_VIEWPORTS 8
#define MAX_SO_TARGETS 4

void Engine_ShaderImplement_Dummy();