#include "stdafx.h"
#include "CommonShader.h"
#include "Material.h"
#include "Texture.h"

void CopyToRenderTarget( IRenderSystem* pRenderSystem, IRenderTarget* pDst, ITexture* pSrc, const CRectangle& dstRect, const CRectangle& srcRect, const CVector2& dstSize, const CVector2& srcSize, float fDepth, IBlendState* pBlend )
{
	if( pDst )
	{
		pRenderSystem->SetRenderTarget( pDst, NULL );
		SViewport viewport = { 0, 0, dstSize.x, dstSize.y, 0, 1 };
		pRenderSystem->SetViewports( &viewport, 1 );
	}

	bool bDepth = fDepth >= 0;
	pRenderSystem->SetBlendState( pBlend ? pBlend : IBlendState::Get<>() );
	if( !bDepth )
		pRenderSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pRenderSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = COneTexturePixelShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pRenderSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	pVertexShader->SetParams( pRenderSystem, dstRect, srcRect, dstSize, srcSize, bDepth ? fDepth : 0 );
	pPixelShader->SetParams( pRenderSystem, pSrc->GetShaderResource() );

	pRenderSystem->DrawInput();
}

IMPLEMENT_GLOBAL_SHADER( CDebugDrawShader, "Shader/Utils.shader", "VSDebugDraw", "vs_5_0" );
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
protected:
	virtual void SetMacros( SShaderMacroDef& macros ) override
	{
		char szBuf[32];
		sprintf( szBuf, "%d", InstCount );
		macros.Add( "EXTRA_INST_DATA", szBuf );
	}
};

IMPLEMENT_MATERIAL_SHADER( PSOneTextureR, "Shader/Utils.shader", "PSOneTextureR", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSOneColorMulTextureAlpha, "Shader/Utils.shader", "PSOneColorMulTextureAlpha", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSInstData, "Shader/Utils.shader", "PSInstData", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSOneColorMulInstData, "Shader/Utils.shader", "PSOneColorMulInstData", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSOneTextureMulInstData, "Shader/Utils.shader", "PSOneTextureMulInstData", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSOneTextureAlphaMulInstData, "Shader/Utils.shader", "PSOneTextureAlphaMulInstData", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSTwoTexCoordMasked, "Shader/Utils.shader", "PSTwoTexCoordMasked", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( Default2DVertexShader, "Shader/Default2D.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( SingleImage2DVertexShader, "Shader/SingleImage2D.shader", "VSMain", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( Rope2DVertexShader, "Shader/Rope2D.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( Rope2DVSStaticData1, "Shader/Rope2D.shader", "VSDefaultStaticData1", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DVertexShader1, CDefault2DVertexShaderExtraInstData<1>, "Shader/Default2D.shader", "VSDefaultExtraInstData", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER( Default2DWithPosVertexShader, "Shader/Default2DWithPos.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DWithPosVertexShader1, CDefault2DVertexShaderExtraInstData<1>, "Shader/Default2DWithPos.shader", "VSDefaultExtraInstData", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER( Default2DWithScrTexVertexShader, "Shader/Default2DWithScrTex.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DWithScrTexVertexShader1, CDefault2DVertexShaderExtraInstData<1>, "Shader/Default2DWithScrTex.shader", "VSDefaultExtraInstData", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER( Default2DWorldToTexVertexShader, "Shader/Default2DWorldToTex.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DWorldToTexVertexShader1, CDefault2DVertexShaderExtraInstData<1>, "Shader/Default2DWorldToTex.shader", "VSDefaultExtraInstData", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER( Default2DScrTiledVertexShader, "Shader/Default2DScrTiled.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DScrTiledVertexShader1, CDefault2DVertexShaderExtraInstData<1>, "Shader/Default2DScrTiled.shader", "VSDefaultExtraInstData", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSOneTextureScrTiled, "Shader/Default2DScrTiled.shader", "PSOneTextureScrTiled", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( Default2DColorPixelShader, "Shader/CommonMaterial.shader", "PSDefault", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( Default2DOcclusionPixelShader, "Shader/CommonMaterial.shader", "PSOcclusionDefault", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( ColorPixelShaderNoClip, "Shader/CommonMaterial.shader", "PSDefaultNoClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( OcclusionTextureAlphaPixelShaderNoClip, "Shader/CommonMaterial.shader", "PSOcclusionTextureAlphaNoClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSWithColorData, "Shader/CommonMaterial.shader", "PSWithColorData", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmission, "Shader/CommonMaterial.shader", "PSEmission", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionAlpha, "Shader/CommonMaterial.shader", "PSEmissionAlpha", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSAlphaToOcclusionColor, "Shader/AlphaToOcclusion.shader", "PSColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSAlphaToOcclusion, "Shader/AlphaToOcclusion.shader", "PSOcclusion", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( Default2DUIVertexShader, "Shader/Default2DUI.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DUIVertexShader1, CDefault2DVertexShaderExtraInstData<1>, "Shader/Default2DUI.shader", "VSDefaultExtraInstData", "vs_5_0" );

void Engine_ShaderImplement_Dummy_Light();
void Engine_ShaderImplement_Dummy()
{
	Engine_ShaderImplement_Dummy_Light();
}