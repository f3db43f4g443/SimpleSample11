#include "stdafx.h"
#include "PostEffects.h"
#include "Render/GlobalRenderResources.h"
#include "Render/CommonShader.h"
#include "RenderState.h"
#include "MyGame.h"
#include "Effects/CrackMeshGenerator.h"

class CPixelUpsampleShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CPixelUpsampleShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_paramFilter, "filter" );
		GetShader()->GetShaderInfo().Bind( m_paramColorShift, "colorShift" );
		GetShader()->GetShaderInfo().Bind( m_paramSrcRes, "srcRes" );
		GetShader()->GetShaderInfo().Bind( m_paramColorCenter, "colorCenter" );
		GetShader()->GetShaderInfo().Bind( m_paramColorEdge, "colorEdge" );
		GetShader()->GetShaderInfo().Bind( m_paramEdgeTexPow, "edgeTexPow" );
		GetShader()->GetShaderInfo().Bind( m_tex, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_paramPointSampler, "PointSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex, CVector4* pFilter, const CVector4& colorShift, const CVector2& srcRes,
		const CVector4& colorCenter, const CVector4& colorEdge, float fEdgeTexPow )
	{
		m_paramFilter.Set( pRenderSystem, pFilter );
		m_paramColorShift.Set( pRenderSystem, &colorShift );
		m_paramSrcRes.Set( pRenderSystem, &srcRes );
		m_paramColorCenter.Set( pRenderSystem, &colorCenter );
		m_paramColorEdge.Set( pRenderSystem, &colorEdge );
		m_paramEdgeTexPow.Set( pRenderSystem, &fEdgeTexPow );

		m_tex.Set( pRenderSystem, pTex );
		m_paramPointSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterPPP>() );
	}
private:
	CShaderParam m_paramFilter;
	CShaderParam m_paramColorShift;
	CShaderParam m_paramSrcRes;
	CShaderParam m_paramColorCenter;
	CShaderParam m_paramColorEdge;
	CShaderParam m_paramEdgeTexPow;

	CShaderParamShaderResource m_tex;
	CShaderParamSampler m_paramPointSampler;
};

IMPLEMENT_GLOBAL_SHADER( CPixelUpsampleShader, "Shader/PostProcess.shader", "PSPixelUpsample", "ps_5_0" );

void CPostProcessPixelUpsample::Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget )
{
	CReference<ITexture>& src = pPass->GetTarget();
	CReference<ITexture> dst;
	IRenderTarget* pTarget = pFinalTarget;
	auto& sizeDependentPool = CRenderTargetPool::GetSizeDependentPool();
	auto finalViewport = pPass->GetFinalViewport();
	if( !pTarget )
	{
		auto texDesc = src->GetDesc();
		texDesc.nDim1 = finalViewport.width;
		texDesc.nDim2 = finalViewport.height;
		sizeDependentPool.AllocRenderTarget( dst, texDesc );
		pTarget = dst->GetRenderTarget();
	}

	IRenderSystem* pSystem = pPass->GetRenderSystem();
	pSystem->SetRenderTarget( pTarget, NULL );
	
	CVector2 srcSize( src->GetDesc().nDim1, src->GetDesc().nDim2 );
	CVector2 dstSize( finalViewport.width, finalViewport.height );
	SViewport viewport = { finalViewport.x, finalViewport.y, finalViewport.width, finalViewport.height, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CPixelUpsampleShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	pVertexShader->SetParams( pSystem, CRectangle( 0, 0, dstSize.x, dstSize.y ), CRectangle( 0, 0, srcSize.x, srcSize.y ), dstSize, srcSize );

	CVector4 filter[64];
	CMatrix k0( 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
	/*CMatrix k1_1( 1, 0, 0, 0, -0.25f, 1, 0, 0, 0.2f, 0, 1, 0, 0.05f, 0, 0, 1 );
	CMatrix k1 = k1_1 * k0 * k1_1.Transpose();
	CMatrix k2;
	for( int i = 0; i < 16; i++ )
		( &k2.m00 )[i] = abs( ( &k1.m00 )[i] - ( &k0.m00 )[i] );*/
	CMatrix k1_1( 1, 0, 0, 0, -0.5f, 1, 0, 0, 0.33f, 0, 1, 0, 0.17f, 0, 0, 1 );
	CMatrix k1 = k1_1 * k0 * k1_1.Transpose();
	CMatrix k2;
	for( int i = 0; i < 16; i++ )
		( &k2.m00 )[i] = ( &k1.m00 )[i] - ( &k0.m00 )[i];

	for( int i = 0; i < 4; i++ )
	{
		auto a1 = Max( 1 - i, i - 2 );
		auto a2 = i >= 2 ? 1 : -1;
		for( int j = 0; j < 4; j++ )
		{
			auto b1 = Max( 1 - j, j - 2 );
			auto b2 = j >= 2 ? 1 : -1;

			CVector4* pBase = filter + i * 16 + j * 4;
			pBase[0] = CVector4( 1, 0, 0, 0 );
			pBase[1] = CVector4( 0, 0, ( i < 2 ? -1.0f : 1.0f ) / srcSize.x, 0 );
			pBase[2] = CVector4( 0, 0, 0, ( j < 2 ? -1.0f : 1.0f ) / srcSize.y );
			pBase[3] = CVector4( 0, 0, ( i < 2 ? -1.0f : 1.0f ) / srcSize.x, ( j < 2 ? -1.0f : 1.0f ) / srcSize.y );
			pBase[0].x = ( &k1.m00 )[a1 * 4 + b1];
			pBase[1].x = ( &k1.m00 )[( 3 - a1 ) * 4 + b1];
			pBase[2].x = ( &k1.m00 )[a1 * 4 + ( 3 - b1 )];
			pBase[3].x = ( &k1.m00 )[( 3 - a1 ) * 4 + ( 3 - b1 )];
			pBase[0].y = ( &k2.m00 )[a1 * 4 + b1] * b2;
			pBase[1].y = ( &k2.m00 )[( 3 - a1 ) * 4 + b1] * b2;
			pBase[2].y = ( &k2.m00 )[a1 * 4 + ( 3 - b1 )] * b2;
			pBase[3].y = ( &k2.m00 )[( 3 - a1 ) * 4 + ( 3 - b1 )] * b2;
		}
	}

	CVector4 colorShift( 0.6f, 0.3f, -0.9f, 0 );
	pPixelShader->SetParams( pSystem, src->GetShaderResource(), filter, colorShift, srcSize, m_colorCenter, m_colorEdge, m_fEdgeTexPow );

	pSystem->DrawInput();

	if( dst )
	{
		sizeDependentPool.Release( src );
		src = dst;
	}
}

void Game_ShaderImplement_Dummy_PostEffects()
{

}