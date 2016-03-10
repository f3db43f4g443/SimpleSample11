#pragma once

#include "DX11Common.h"
#include "Reference.h"
#include "RenderState.h"

class CBlendState : public IBlendState
{
public:
	CBlendState( bool AlphaToCoverageEnable, bool IndependentBlendEnable,
		uint8 WriteMask0, uint32 Src0, uint32 Dst0, uint32 Op0, uint32 SrcAlpha0, uint32 DstAlpha0, uint32 OpAlpha0,
		uint8 WriteMask1, uint32 Src1, uint32 Dst1, uint32 Op1, uint32 SrcAlpha1, uint32 DstAlpha1, uint32 OpAlpha1,
		uint8 WriteMask2, uint32 Src2, uint32 Dst2, uint32 Op2, uint32 SrcAlpha2, uint32 DstAlpha2, uint32 OpAlpha2,
		uint8 WriteMask3, uint32 Src3, uint32 Dst3, uint32 Op3, uint32 SrcAlpha3, uint32 DstAlpha3, uint32 OpAlpha3,
		uint8 WriteMask4, uint32 Src4, uint32 Dst4, uint32 Op4, uint32 SrcAlpha4, uint32 DstAlpha4, uint32 OpAlpha4,
		uint8 WriteMask5, uint32 Src5, uint32 Dst5, uint32 Op5, uint32 SrcAlpha5, uint32 DstAlpha5, uint32 OpAlpha5,
		uint8 WriteMask6, uint32 Src6, uint32 Dst6, uint32 Op6, uint32 SrcAlpha6, uint32 DstAlpha6, uint32 OpAlpha6,
		uint8 WriteMask7, uint32 Src7, uint32 Dst7, uint32 Op7, uint32 SrcAlpha7, uint32 DstAlpha7, uint32 OpAlpha7 );

	ID3D11BlendState* GetState() const { return m_pState; }
private:
	CReference<ID3D11BlendState> m_pState;
};

class CDepthStencilState : public IDepthStencilState
{
public:
	CDepthStencilState( bool DepthWrite, uint32 DepthFunc,
		uint32 StencilFailFront, uint32 DepthFailFront, uint32 PassFront, uint32 FuncFront,
		uint32 StencilFailBack, uint32 DepthFailBack, uint32 PassBack, uint32 FuncBack,
		uint8 ReadMask, uint8 WriteMask );

	ID3D11DepthStencilState* GetState() const { return m_pState; }
private:
	CReference<ID3D11DepthStencilState> m_pState;
};

class CRasterizerState : public IRasterizerState
{
public:
	CRasterizerState( uint32 FillMode, uint32 CullMode, bool FrontCCW, bool DepthClipEnable,
		bool ScissorEnable, bool MultisampleEnable, bool AntialiasedLineEnable );

	ID3D11RasterizerState* GetState() const { return m_pState; }
private:
	CReference<ID3D11RasterizerState> m_pState;
};

class CSamplerState : public ISamplerState
{
public:
	CSamplerState( uint32 Filter, uint32 AddressU, uint32 AddressV, uint32 AddressW,
		int32 MipLODBias, uint32 MaxAnisotropy, uint32 ComparisonFunc,
		uint32 BorderColor0, uint32 BorderColor1, uint32 BorderColor2, uint32 BorderColor3 );

	ID3D11SamplerState* GetState() const { return m_pState; }
private:
	CReference<ID3D11SamplerState> m_pState;
};