#include "stdafx.h"
#include "CommonShader.h"
#include "Material.h"
#include "Texture.h"

void CopyToRenderTarget( IRenderSystem* pRenderSystem, IRenderTarget* pDst, ITexture* pSrc, const CRectangle& dstRect, const CRectangle& srcRect, const CVector2& dstSize, const CVector2& srcSize )
{
	pRenderSystem->SetRenderTarget( pDst, NULL );
	SViewport viewport = { 0, 0, pSrc->GetDesc().nDim1, pSrc->GetDesc().nDim2, 0, 1 };
	pRenderSystem->SetViewports( &viewport, 1 );

	pRenderSystem->SetBlendState( IBlendState::Get<>() );
	pRenderSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pRenderSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = COneTexturePixelShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pRenderSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	pVertexShader->SetParams( pRenderSystem, dstRect, srcRect, dstSize, srcSize );
	pPixelShader->SetParams( pRenderSystem, pSrc->GetShaderResource() );

	pRenderSystem->DrawInput();
}

IMPLEMENT_GLOBAL_SHADER( CScreenVertexShader, "Shader/Utils.shader", "VSScreen", "vs_5_0" );
IMPLEMENT_GLOBAL_SHADER( COneColorPixelShader, "Shader/Utils.shader", "PSOneColor", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER( COneTexturePixelShader, "Shader/Utils.shader", "PSOneTexture", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER( CTwoTextureMultiplyPixelShader, "Shader/Utils.shader", "PSTwoTextureMultiply", "ps_5_0" );

IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_COneTexturePixelShaderPerRT2", COneTexturePixelShaderPerRT<2>, "Shader/Utils.shader", "PSOneTexturePerRT", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_COneTexturePixelShaderPerRT3", COneTexturePixelShaderPerRT<3>, "Shader/Utils.shader", "PSOneTexturePerRT", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_COneTexturePixelShaderPerRT4", COneTexturePixelShaderPerRT<4>, "Shader/Utils.shader", "PSOneTexturePerRT", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_COneTexturePixelShaderPerRT5", COneTexturePixelShaderPerRT<5>, "Shader/Utils.shader", "PSOneTexturePerRT", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_COneTexturePixelShaderPerRT6", COneTexturePixelShaderPerRT<6>, "Shader/Utils.shader", "PSOneTexturePerRT", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_COneTexturePixelShaderPerRT7", COneTexturePixelShaderPerRT<7>, "Shader/Utils.shader", "PSOneTexturePerRT", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_COneTexturePixelShaderPerRT8", COneTexturePixelShaderPerRT<8>, "Shader/Utils.shader", "PSOneTexturePerRT", "ps_5_0" );

template<uint32 InstCount>
class CDefault2DVertexShaderExtraInstData : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CDefault2DVertexShaderExtraInstData );
protected:
	virtual void SetMacros( SShaderMacroDef& macros ) override
	{
		char szBuf[32];
		sprintf( szBuf, "%d", InstCount );
		macros.Add( "EXTRA_INST_DATA", szBuf );
	}
};

IMPLEMENT_MATERIAL_SHADER( Default2DVertexShader, "Shader/Default2D.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( SingleImage2DVertexShader, "Shader/SingleImage2D.shader", "VSMain", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( Rope2DVertexShader, "Shader/Rope2D.shader", "VSDefault", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DVertexShader1, CDefault2DVertexShaderExtraInstData<1>, "Shader/Default2D.shader", "VSDefaultExtraInstData", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER( Default2DColorPixelShader, "Shader/CommonMaterial.shader", "PSDefault", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( Default2DOcclusionPixelShader, "Shader/CommonMaterial.shader", "PSOcclusionDefault", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( ColorPixelShaderNoClip, "Shader/CommonMaterial.shader", "PSDefaultNoClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( OcclusionTextureAlphaPixelShaderNoClip, "Shader/CommonMaterial.shader", "PSOcclusionTextureAlphaNoClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionAlpha, "Shader/CommonMaterial.shader", "PSEmissionAlpha", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSAlphaToOcclusionColor, "Shader/AlphaToOcclusion.shader", "PSColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSAlphaToOcclusion, "Shader/AlphaToOcclusion.shader", "PSOcclusion", "ps_5_0" );