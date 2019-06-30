#pragma once
#include "GlobalRenderResources.h"
#include "RenderSystem.h"

class CDebugDrawShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CDebugDrawShader );
};

class CScreenVertexShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CScreenVertexShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_dstRect, "DstRect" );
		GetShader()->GetShaderInfo().Bind( m_srcRect, "SrcRect" );
		GetShader()->GetShaderInfo().Bind( m_invDstSrcResolution, "InvDstSrcResolution" );
		GetShader()->GetShaderInfo().Bind( m_depth, "fDepth" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, const CRectangle& DstRect, const CRectangle& SrcRect, const CVector2& DstResolution, const CVector2& SrcResolution, float fDepth = 0 )
	{
		m_dstRect.Set( pRenderSystem, &DstRect );
		m_srcRect.Set( pRenderSystem, &SrcRect );
		CVector4 invDstSrcResolution( 1.0f / DstResolution.x, 1.0f / DstResolution.y, 1.0f / SrcResolution.x, 1.0f / SrcResolution.y );
		m_invDstSrcResolution.Set( pRenderSystem, &invDstSrcResolution );
		m_depth.Set( pRenderSystem, &fDepth );
	}
private:
	CShaderParam m_dstRect;
	CShaderParam m_srcRect;
	CShaderParam m_invDstSrcResolution;
	CShaderParam m_depth;
};

class COneColorPixelShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( COneColorPixelShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_paramColor, "vColor" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, const CVector4& color )
	{
		m_paramColor.Set( pRenderSystem, &color );
	}
private:
	CShaderParam m_paramColor;
};

class COneTexturePixelShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( COneTexturePixelShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_tex, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex )
	{
		m_tex.Set( pRenderSystem, pTex );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterPPP>() );
	}
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex, ISamplerState* pSampler )
	{
		m_tex.Set( pRenderSystem, pTex );
		m_paramLinearSampler.Set( pRenderSystem, pSampler );
	}
private:
	CShaderParamShaderResource m_tex;
	CShaderParamSampler m_paramLinearSampler;
};

void CopyToRenderTarget( IRenderSystem* pRenderSystem, IRenderTarget* pDst, ITexture* pSrc, const CRectangle& dstRect, const CRectangle& srcRect, const CVector2& dstSize, const CVector2& srcSize,
	float fDepth = -1, IBlendState* pBlend = NULL );

class CTwoTextureMultiplyPixelShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CTwoTextureMultiplyPixelShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_tex0, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_tex1, "Texture1" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex0, IShaderResource* pTex1 )
	{
		m_tex0.Set( pRenderSystem, pTex0 );
		m_tex1.Set( pRenderSystem, pTex1 );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParamShaderResource m_tex0;
	CShaderParamShaderResource m_tex1;
	CShaderParamSampler m_paramLinearSampler;
};

template<uint32 RTCount>
class COneTexturePixelShaderPerRT : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( COneTexturePixelShaderPerRT );
protected:
	virtual void SetMacros( SShaderMacroDef& macros ) override
	{
		char szBuf[32];
		sprintf( szBuf, "%d", RTCount );
		macros.Add( "MAX_TEXTURES", szBuf );
	}
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().BindArray( m_tex, RTCount, "Textures" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource** ppTex )
	{
		for( int i = 0; i < ELEM_COUNT( m_tex ); i++ )
		{
			m_tex[i].Set( pRenderSystem, ppTex[i] );
		}
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParamShaderResource m_tex[RTCount];
	CShaderParamSampler m_paramLinearSampler;
};