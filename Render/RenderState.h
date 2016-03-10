#pragma once

enum EBlend
{
	EBlendZero = 1,
	EBlendOne = 2,
	EBlendSrcColor = 3,
	EBlendInvSrcColor = 4,
	EBlendSrcAlpha = 5,
	EBlendInvSrcAlpha = 6,
	EBlendDestAlpha = 7,
	EBlendInvDestAlpha = 8,
	EBlendDestColor = 9,
	EBlendInvDestColor = 10,
	EBlendSrcAlphaSat = 11,
	EBlendBlendFactor = 14,
	EBlendInvBlendFactor = 15,
	EBlendSrc1Color = 16,
	EBlendInvSrc1Color = 17,
	EBlendSrc1Alpha = 18,
	EBlendInvSrc1Alpha = 19
};

enum EBlendOp
{
	EBlendOpAdd = 1,
	EBlendOpSubtract = 2,
	EBlendOpRevSubtract = 3,
	EBlendOpMin = 4,
	EBlendOpMax = 5
};

class IBlendState
{
public:
	template<bool AlphaToCoverageEnable = false, bool IndependentBlendEnable = false,
		uint8 WriteMask0 = 0xf, uint32 Src0 = EBlendOne, uint32 Dst0 = EBlendZero, uint32 Op0 = EBlendOpAdd, uint32 SrcAlpha0 = EBlendOne, uint32 DstAlpha0 = EBlendZero, uint32 OpAlpha0 = EBlendOpAdd,
		uint8 WriteMask1 = 0xf, uint32 Src1 = EBlendOne, uint32 Dst1 = EBlendZero, uint32 Op1 = EBlendOpAdd, uint32 SrcAlpha1 = EBlendOne, uint32 DstAlpha1 = EBlendZero, uint32 OpAlpha1 = EBlendOpAdd,
		uint8 WriteMask2 = 0xf, uint32 Src2 = EBlendOne, uint32 Dst2 = EBlendZero, uint32 Op2 = EBlendOpAdd, uint32 SrcAlpha2 = EBlendOne, uint32 DstAlpha2 = EBlendZero, uint32 OpAlpha2 = EBlendOpAdd,
		uint8 WriteMask3 = 0xf, uint32 Src3 = EBlendOne, uint32 Dst3 = EBlendZero, uint32 Op3 = EBlendOpAdd, uint32 SrcAlpha3 = EBlendOne, uint32 DstAlpha3 = EBlendZero, uint32 OpAlpha3 = EBlendOpAdd,
		uint8 WriteMask4 = 0xf, uint32 Src4 = EBlendOne, uint32 Dst4 = EBlendZero, uint32 Op4 = EBlendOpAdd, uint32 SrcAlpha4 = EBlendOne, uint32 DstAlpha4 = EBlendZero, uint32 OpAlpha4 = EBlendOpAdd,
		uint8 WriteMask5 = 0xf, uint32 Src5 = EBlendOne, uint32 Dst5 = EBlendZero, uint32 Op5 = EBlendOpAdd, uint32 SrcAlpha5 = EBlendOne, uint32 DstAlpha5 = EBlendZero, uint32 OpAlpha5 = EBlendOpAdd,
		uint8 WriteMask6 = 0xf, uint32 Src6 = EBlendOne, uint32 Dst6 = EBlendZero, uint32 Op6 = EBlendOpAdd, uint32 SrcAlpha6 = EBlendOne, uint32 DstAlpha6 = EBlendZero, uint32 OpAlpha6 = EBlendOpAdd,
		uint8 WriteMask7 = 0xf, uint32 Src7 = EBlendOne, uint32 Dst7 = EBlendZero, uint32 Op7 = EBlendOpAdd, uint32 SrcAlpha7 = EBlendOne, uint32 DstAlpha7 = EBlendZero, uint32 OpAlpha7 = EBlendOpAdd>
	static IBlendState* Get();
private:
	static IBlendState* Create( bool AlphaToCoverageEnable, bool IndependentBlendEnable,
		uint8 WriteMask0, uint32 Src0, uint32 Dst0, uint32 Op0, uint32 SrcAlpha0, uint32 DstAlpha0, uint32 OpAlpha0, 
		uint8 WriteMask1, uint32 Src1, uint32 Dst1, uint32 Op1, uint32 SrcAlpha1, uint32 DstAlpha1, uint32 OpAlpha1, 
		uint8 WriteMask2, uint32 Src2, uint32 Dst2, uint32 Op2, uint32 SrcAlpha2, uint32 DstAlpha2, uint32 OpAlpha2, 
		uint8 WriteMask3, uint32 Src3, uint32 Dst3, uint32 Op3, uint32 SrcAlpha3, uint32 DstAlpha3, uint32 OpAlpha3, 
		uint8 WriteMask4, uint32 Src4, uint32 Dst4, uint32 Op4, uint32 SrcAlpha4, uint32 DstAlpha4, uint32 OpAlpha4, 
		uint8 WriteMask5, uint32 Src5, uint32 Dst5, uint32 Op5, uint32 SrcAlpha5, uint32 DstAlpha5, uint32 OpAlpha5, 
		uint8 WriteMask6, uint32 Src6, uint32 Dst6, uint32 Op6, uint32 SrcAlpha6, uint32 DstAlpha6, uint32 OpAlpha6, 
		uint8 WriteMask7, uint32 Src7, uint32 Dst7, uint32 Op7, uint32 SrcAlpha7, uint32 DstAlpha7, uint32 OpAlpha7 );
};

template<bool AlphaToCoverageEnable, bool IndependentBlendEnable,
	uint8 WriteMask0, uint32 Src0, uint32 Dst0, uint32 Op0, uint32 SrcAlpha0, uint32 DstAlpha0, uint32 OpAlpha0,
	uint8 WriteMask1, uint32 Src1, uint32 Dst1, uint32 Op1, uint32 SrcAlpha1, uint32 DstAlpha1, uint32 OpAlpha1,
	uint8 WriteMask2, uint32 Src2, uint32 Dst2, uint32 Op2, uint32 SrcAlpha2, uint32 DstAlpha2, uint32 OpAlpha2,
	uint8 WriteMask3, uint32 Src3, uint32 Dst3, uint32 Op3, uint32 SrcAlpha3, uint32 DstAlpha3, uint32 OpAlpha3,
	uint8 WriteMask4, uint32 Src4, uint32 Dst4, uint32 Op4, uint32 SrcAlpha4, uint32 DstAlpha4, uint32 OpAlpha4,
	uint8 WriteMask5, uint32 Src5, uint32 Dst5, uint32 Op5, uint32 SrcAlpha5, uint32 DstAlpha5, uint32 OpAlpha5,
	uint8 WriteMask6, uint32 Src6, uint32 Dst6, uint32 Op6, uint32 SrcAlpha6, uint32 DstAlpha6, uint32 OpAlpha6,
	uint8 WriteMask7, uint32 Src7, uint32 Dst7, uint32 Op7, uint32 SrcAlpha7, uint32 DstAlpha7, uint32 OpAlpha7>
inline IBlendState* IBlendState::Get()
{
	static IBlendState* g_pState = Create( AlphaToCoverageEnable, IndependentBlendEnable,
		WriteMask0, Src0, Dst0, Op0, SrcAlpha0, DstAlpha0, OpAlpha0, 
		WriteMask1, Src1, Dst1, Op1, SrcAlpha1, DstAlpha1, OpAlpha1, 
		WriteMask2, Src2, Dst2, Op2, SrcAlpha2, DstAlpha2, OpAlpha2, 
		WriteMask3, Src3, Dst3, Op3, SrcAlpha3, DstAlpha3, OpAlpha3, 
		WriteMask4, Src4, Dst4, Op4, SrcAlpha4, DstAlpha4, OpAlpha4, 
		WriteMask5, Src5, Dst5, Op5, SrcAlpha5, DstAlpha5, OpAlpha5, 
		WriteMask6, Src6, Dst6, Op6, SrcAlpha6, DstAlpha6, OpAlpha6, 
		WriteMask7, Src7, Dst7, Op7, SrcAlpha7, DstAlpha7, OpAlpha7 );
	return g_pState;
}

enum EStencilOp
{
	EStencilOpKeep = 1,
	EStencilOpZero = 2,
	EStencilOpReplace = 3,
	EStencilOpIncrSat = 4,
	EStencilOpDecrSat = 5,
	EStencilOpInvert = 6,
	EStencilOpIncr = 7,
	EStencilOpDecr = 8
};

class IDepthStencilState
{
public:
	template<bool DepthWrite = true, uint32 DepthFunc = EComparisonAlways,
		uint32 StencilFailFront = EStencilOpKeep, uint32 DepthFailFront = EStencilOpKeep, uint32 PassFront = EStencilOpKeep, uint32 FuncFront = EComparisonAlways,
		uint32 StencilFailBack = EStencilOpKeep, uint32 DepthFailBack = EStencilOpKeep, uint32 PassBack = EStencilOpKeep, uint32 FuncBack = EComparisonAlways,
		uint8 ReadMask = 0xff, uint8 WriteMask = 0xff>
	static IDepthStencilState* Get();
private:
	static IDepthStencilState* Create( bool DepthWrite, uint32 DepthFunc,
		uint32 StencilFailFront, uint32 DepthFailFront, uint32 PassFront, uint32 FuncFront, 
		uint32 StencilFailBack, uint32 DepthFailBack, uint32 PassBack, uint32 FuncBack, 
		uint8 ReadMask, uint8 WriteMask );
};

template<bool DepthWrite, uint32 DepthFunc,
	uint32 StencilFailFront, uint32 DepthFailFront, uint32 PassFront, uint32 FuncFront,
	uint32 StencilFailBack, uint32 DepthFailBack, uint32 PassBack, uint32 FuncBack,
	uint8 ReadMask, uint8 WriteMask>
inline IDepthStencilState* IDepthStencilState::Get()
{
	static IDepthStencilState* g_pState = Create( DepthWrite, DepthFunc,
		StencilFailFront, DepthFailFront, PassFront, FuncFront,
		StencilFailBack, DepthFailBack, PassBack, FuncBack,
		ReadMask, WriteMask );
	return g_pState;
}

enum EFillMode
{
	EFillModeWireFrame = 2,
	EFillModeSolid = 3
};

enum ECullMode
{
	ECullModeNone = 1,
	ECullModeFront = 2,
	ECullModeBack = 3
};

class IRasterizerState
{
public:
	template<uint32 FillMode = EFillModeSolid, uint32 CullMode = ECullModeBack, bool FrontCCW = false, bool DepthClipEnable = true,
		bool ScissorEnable = false, bool MultisampleEnable = false, bool AntialiasedLineEnable = false>
	static IRasterizerState* Get();
private:
	static IRasterizerState* Create( uint32 FillMode, uint32 CullMode, bool FrontCCW, bool DepthClipEnable,
		bool ScissorEnable, bool MultisampleEnable, bool AntialiasedLineEnable );
};

template<uint32 FillMode, uint32 CullMode, bool FrontCCW, bool DepthClipEnable,
	bool ScissorEnable, bool MultisampleEnable, bool AntialiasedLineEnable>
inline IRasterizerState* IRasterizerState::Get()
{
	static IRasterizerState* g_pState = Create( FillMode, CullMode, FrontCCW, DepthClipEnable,
		ScissorEnable, MultisampleEnable, AntialiasedLineEnable );
	return g_pState;
}

enum ESamplerFilter
{
	ESamplerFilterPPP = 0,
	ESamplerFilterPPL = 0x1,
	ESamplerFilterPLP = 0x4,
	ESamplerFilterPLL = 0x5,
	ESamplerFilterLPP = 0x10,
	ESamplerFilterLPL = 0x11,
	ESamplerFilterLLP = 0x14,
	ESamplerFilterLLL = 0x15,
	ESamplerFilterAnisotropic = 0x55,
	ESamplerFilterCPPP = 0x80,
	ESamplerFilterCPPL = 0x81,
	ESamplerFilterCPLP = 0x84,
	ESamplerFilterCPLL = 0x85,
	ESamplerFilterCLPP = 0x90,
	ESamplerFilterCLPL = 0x91,
	ESamplerFilterCLLP = 0x94,
	ESamplerFilterCLLL = 0x95,
	ESamplerFilterCAnisotropic = 0xd5,
};

enum ETextureAddressMode
{
	ETextureAddressModeWrap = 1,
	ETextureAddressModeMirror = 2,
	ETextureAddressModeClamp = 3,
	ETextureAddressModeBorder = 4,
	ETextureAddressModeMirrorOnce = 5
};

class ISamplerState
{
public:
	template<uint32 Filter = ESamplerFilterPPP, uint32 AddressU = ETextureAddressModeClamp, uint32 AddressV = ETextureAddressModeClamp, uint32 AddressW = ETextureAddressModeClamp,
		int32 MipLODBias = 0, uint32 MaxAnisotropy = 16, uint32 ComparisonFunc = EComparisonNever,
		uint32 BorderColor0 = 0, uint32 BorderColor1 = 0, uint32 BorderColor2 = 0, uint32 BorderColor3 = 0>
	static ISamplerState* Get();
private:
	static ISamplerState* Create( uint32 Filter, uint32 AddressU, uint32 AddressV, uint32 AddressW,
		int32 MipLODBias, uint32 MaxAnisotropy, uint32 ComparisonFunc,
		uint32 BorderColor0, uint32 BorderColor1, uint32 BorderColor2, uint32 BorderColor3 );
};

template<uint32 Filter, uint32 AddressU, uint32 AddressV, uint32 AddressW,
	int32 MipLODBias, uint32 MaxAnisotropy, uint32 ComparisonFunc,
	uint32 BorderColor0, uint32 BorderColor1, uint32 BorderColor2, uint32 BorderColor3>
inline ISamplerState* ISamplerState::Get()
{
	static ISamplerState* g_pState = Create( Filter, AddressU, AddressV, AddressW,
		MipLODBias, MaxAnisotropy, ComparisonFunc,
		BorderColor0, BorderColor1, BorderColor2, BorderColor3 );
	return g_pState;
}