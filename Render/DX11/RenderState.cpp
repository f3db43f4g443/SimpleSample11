#include "stdafx.h"
#include "DX11RenderState.h"
#include "DX11RenderSystem.h"

CBlendState::CBlendState( bool AlphaToCoverageEnable, bool IndependentBlendEnable,
	uint8 WriteMask0, uint32 Src0, uint32 Dst0, uint32 Op0, uint32 SrcAlpha0, uint32 DstAlpha0, uint32 OpAlpha0,
	uint8 WriteMask1, uint32 Src1, uint32 Dst1, uint32 Op1, uint32 SrcAlpha1, uint32 DstAlpha1, uint32 OpAlpha1,
	uint8 WriteMask2, uint32 Src2, uint32 Dst2, uint32 Op2, uint32 SrcAlpha2, uint32 DstAlpha2, uint32 OpAlpha2,
	uint8 WriteMask3, uint32 Src3, uint32 Dst3, uint32 Op3, uint32 SrcAlpha3, uint32 DstAlpha3, uint32 OpAlpha3,
	uint8 WriteMask4, uint32 Src4, uint32 Dst4, uint32 Op4, uint32 SrcAlpha4, uint32 DstAlpha4, uint32 OpAlpha4,
	uint8 WriteMask5, uint32 Src5, uint32 Dst5, uint32 Op5, uint32 SrcAlpha5, uint32 DstAlpha5, uint32 OpAlpha5,
	uint8 WriteMask6, uint32 Src6, uint32 Dst6, uint32 Op6, uint32 SrcAlpha6, uint32 DstAlpha6, uint32 OpAlpha6,
	uint8 WriteMask7, uint32 Src7, uint32 Dst7, uint32 Op7, uint32 SrcAlpha7, uint32 DstAlpha7, uint32 OpAlpha7 )
{
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = AlphaToCoverageEnable;
	desc.IndependentBlendEnable = IndependentBlendEnable;

#define RENDER_TARGET_DESC( i ) \
	desc.RenderTarget[i].BlendEnable = Src##i != (uint32)EBlendOne || Dst##i != (uint32)EBlendZero || Op##i != (uint32)EBlendOpAdd \
	|| SrcAlpha##i != (uint32)EBlendOne || DstAlpha##i != (uint32)EBlendZero || OpAlpha##i != (uint32)EBlendOpAdd; \
	desc.RenderTarget[i].SrcBlend = (D3D11_BLEND)Src##i; \
	desc.RenderTarget[i].DestBlend = (D3D11_BLEND)Dst##i; \
	desc.RenderTarget[i].BlendOp = (D3D11_BLEND_OP)Op##i; \
	desc.RenderTarget[i].SrcBlendAlpha = (D3D11_BLEND)SrcAlpha##i; \
	desc.RenderTarget[i].DestBlendAlpha = (D3D11_BLEND)DstAlpha##i; \
	desc.RenderTarget[i].BlendOpAlpha = (D3D11_BLEND_OP)OpAlpha##i; \
	desc.RenderTarget[i].RenderTargetWriteMask = WriteMask##i;

	RENDER_TARGET_DESC( 0 )
	RENDER_TARGET_DESC( 1 )
	RENDER_TARGET_DESC( 2 )
	RENDER_TARGET_DESC( 3 )
	RENDER_TARGET_DESC( 4 )
	RENDER_TARGET_DESC( 5 )
	RENDER_TARGET_DESC( 6 )
	RENDER_TARGET_DESC( 7 )

#undef RENDER_TARGET_DESC

	( (CRenderSystem*)IRenderSystem::Inst() )->GetDevice()->CreateBlendState( &desc, m_pState.AssignPtr() );
}

IBlendState* IBlendState::Create( bool AlphaToCoverageEnable, bool IndependentBlendEnable,
	uint8 WriteMask0, uint32 Src0, uint32 Dst0, uint32 Op0, uint32 SrcAlpha0, uint32 DstAlpha0, uint32 OpAlpha0,
	uint8 WriteMask1, uint32 Src1, uint32 Dst1, uint32 Op1, uint32 SrcAlpha1, uint32 DstAlpha1, uint32 OpAlpha1,
	uint8 WriteMask2, uint32 Src2, uint32 Dst2, uint32 Op2, uint32 SrcAlpha2, uint32 DstAlpha2, uint32 OpAlpha2,
	uint8 WriteMask3, uint32 Src3, uint32 Dst3, uint32 Op3, uint32 SrcAlpha3, uint32 DstAlpha3, uint32 OpAlpha3,
	uint8 WriteMask4, uint32 Src4, uint32 Dst4, uint32 Op4, uint32 SrcAlpha4, uint32 DstAlpha4, uint32 OpAlpha4,
	uint8 WriteMask5, uint32 Src5, uint32 Dst5, uint32 Op5, uint32 SrcAlpha5, uint32 DstAlpha5, uint32 OpAlpha5,
	uint8 WriteMask6, uint32 Src6, uint32 Dst6, uint32 Op6, uint32 SrcAlpha6, uint32 DstAlpha6, uint32 OpAlpha6,
	uint8 WriteMask7, uint32 Src7, uint32 Dst7, uint32 Op7, uint32 SrcAlpha7, uint32 DstAlpha7, uint32 OpAlpha7 )
{
	return new CBlendState( AlphaToCoverageEnable, IndependentBlendEnable,
		WriteMask0, Src0, Dst0, Op0, SrcAlpha0, DstAlpha0, OpAlpha0,
		WriteMask1, Src1, Dst1, Op1, SrcAlpha1, DstAlpha1, OpAlpha1, 
		WriteMask2, Src2, Dst2, Op2, SrcAlpha2, DstAlpha2, OpAlpha2, 
		WriteMask3, Src3, Dst3, Op3, SrcAlpha3, DstAlpha3, OpAlpha3, 
		WriteMask4, Src4, Dst4, Op4, SrcAlpha4, DstAlpha4, OpAlpha4, 
		WriteMask5, Src5, Dst5, Op5, SrcAlpha5, DstAlpha5, OpAlpha5, 
		WriteMask6, Src6, Dst6, Op6, SrcAlpha6, DstAlpha6, OpAlpha6, 
		WriteMask7, Src7, Dst7, Op7, SrcAlpha7, DstAlpha7, OpAlpha7 );
}

CDepthStencilState::CDepthStencilState( bool DepthWrite, uint32 DepthFunc,
	uint32 StencilFailFront, uint32 DepthFailFront, uint32 PassFront, uint32 FuncFront,
	uint32 StencilFailBack, uint32 DepthFailBack, uint32 PassBack, uint32 FuncBack,
	uint8 ReadMask, uint8 WriteMask )
{
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = DepthFunc != (uint32)EComparisonAlways;
	desc.DepthWriteMask = DepthWrite? D3D11_DEPTH_WRITE_MASK_ALL: D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = (D3D11_COMPARISON_FUNC)DepthFunc;
	desc.StencilEnable = StencilFailFront != (uint32)EStencilOpKeep || DepthFailFront != (uint32)EStencilOpKeep || PassFront != (uint32)EStencilOpKeep || FuncFront != (uint32)EComparisonAlways
		|| StencilFailBack != (uint32)EStencilOpKeep || DepthFailBack != (uint32)EStencilOpKeep || PassBack != (uint32)EStencilOpKeep || FuncBack != (uint32)EComparisonAlways;
	desc.StencilReadMask = ReadMask;
	desc.StencilWriteMask = WriteMask;
	desc.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)StencilFailFront;
	desc.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)DepthFailFront;
	desc.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)PassFront;
	desc.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)FuncFront;
	desc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)StencilFailBack;
	desc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)DepthFailBack;
	desc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)PassBack;
	desc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)FuncBack;

	( (CRenderSystem*)IRenderSystem::Inst() )->GetDevice()->CreateDepthStencilState( &desc, m_pState.AssignPtr() );
}

IDepthStencilState* IDepthStencilState::Create( bool DepthWrite, uint32 DepthFunc,
	uint32 StencilFailFront, uint32 DepthFailFront, uint32 PassFront, uint32 FuncFront,
	uint32 StencilFailBack, uint32 DepthFailBack, uint32 PassBack, uint32 FuncBack,
	uint8 ReadMask, uint8 WriteMask )
{
	return new CDepthStencilState( DepthWrite, DepthFunc,
		StencilFailFront, DepthFailFront, PassFront, FuncFront,
		StencilFailBack, DepthFailBack, PassBack, FuncBack,
		ReadMask, WriteMask );
}

CRasterizerState::CRasterizerState( uint32 FillMode, uint32 CullMode, bool FrontCCW, bool DepthClipEnable,
	bool ScissorEnable, bool MultisampleEnable, bool AntialiasedLineEnable )
{
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = (D3D11_FILL_MODE)FillMode;
	desc.CullMode = (D3D11_CULL_MODE)CullMode;
	desc.FrontCounterClockwise = FrontCCW;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0;
	desc.SlopeScaledDepthBias = 0;
	desc.DepthClipEnable = DepthClipEnable;
	desc.ScissorEnable = ScissorEnable;
	desc.MultisampleEnable = MultisampleEnable;
	desc.AntialiasedLineEnable = AntialiasedLineEnable;

	( (CRenderSystem*)IRenderSystem::Inst() )->GetDevice()->CreateRasterizerState( &desc, m_pState.AssignPtr() );
}

IRasterizerState* IRasterizerState::Create( uint32 FillMode, uint32 CullMode, bool FrontCCW, bool DepthClipEnable,
	bool ScissorEnable, bool MultisampleEnable, bool AntialiasedLineEnable )
{
	return new CRasterizerState( FillMode, CullMode, FrontCCW, DepthClipEnable, ScissorEnable, MultisampleEnable, AntialiasedLineEnable );
}

CSamplerState::CSamplerState( uint32 Filter, uint32 AddressU, uint32 AddressV, uint32 AddressW,
	int32 MipLODBias, uint32 MaxAnisotropy, uint32 ComparisonFunc,
	uint32 BorderColor0, uint32 BorderColor1, uint32 BorderColor2, uint32 BorderColor3 )
{
	D3D11_SAMPLER_DESC desc;
	desc.Filter = (D3D11_FILTER)Filter;
	desc.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)AddressU;
	desc.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)AddressV;
	desc.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)AddressW;
	desc.MipLODBias = MipLODBias;
	desc.MaxAnisotropy = MaxAnisotropy;
	desc.ComparisonFunc = (D3D11_COMPARISON_FUNC)ComparisonFunc;
	desc.BorderColor[0] = BorderColor0 / 255.0f;
	desc.BorderColor[1] = BorderColor1 / 255.0f;
	desc.BorderColor[2] = BorderColor2 / 255.0f;
	desc.BorderColor[3] = BorderColor3 / 255.0f;
	desc.MinLOD = 0;
	desc.MaxLOD = FLT_MAX;
	
	( (CRenderSystem*)IRenderSystem::Inst() )->GetDevice()->CreateSamplerState( &desc, m_pState.AssignPtr() );
}

ISamplerState* ISamplerState::Create( uint32 Filter, uint32 AddressU, uint32 AddressV, uint32 AddressW,
	int32 MipLODBias, uint32 MaxAnisotropy, uint32 ComparisonFunc,
	uint32 BorderColor0, uint32 BorderColor1, uint32 BorderColor2, uint32 BorderColor3 )
{
	return new CSamplerState( Filter, AddressU, AddressV, AddressW, MipLODBias, MaxAnisotropy, ComparisonFunc, BorderColor0, BorderColor1, BorderColor2, BorderColor3 );
}