#include "stdafx.h"
#include "LightRendering.h"
#include "Lighted2DRenderer.h"
#include "GlobalRenderResources.h"
#include "CommonShader.h"

CDirectionalLightObject::CDirectionalLightObject() : Dir( 0, 0 ), fIntensity( 0 ), fShadowScale( 0 )
{
	m_localBound = CRectangle( -1000000, -1000000, 2000000, 2000000 );
}

CDirectionalLightObject::CDirectionalLightObject( const CVector2& Dir, float fIntensity, float fShadowScale, float fMaxShadowDist )
	: Dir( Dir ), fIntensity( fIntensity ), fShadowScale( fShadowScale ), fMaxShadowDist( fMaxShadowDist )
{
	m_localBound = CRectangle( -1000000, -1000000, 2000000, 2000000 );
}

void CDirectionalLightObject::Render( CRenderContext2D& context )
{
	if( context.eRenderPass != eRenderPass_Occlusion )
		return;
	m_light.Dir = globalTransform.MulVector2Dir( Dir );
	m_light.fIntensity = fIntensity;
	m_light.fShadowScale = fShadowScale;
	m_light.fMaxShadowDist = fMaxShadowDist;
	context.AddDirectionalLight( &m_light );
}

CPointLightObject::CPointLightObject()
	: AttenuationIntensity( 0, 0, 0, 0 ), fShadowScale( 0 ), fMaxRange( 0 )
{
	m_localBound = CRectangle( 0, 0, 0, 0 );
}

CPointLightObject::CPointLightObject( const CVector4& AttenuationIntensity, float fShadowScale, float fMaxRange, float fLightHeight )
	: AttenuationIntensity( AttenuationIntensity ), fShadowScale( fShadowScale ), fMaxRange( fMaxRange ), fLightHeight( fLightHeight )
{
	m_localBound = CRectangle( -fMaxRange, -fMaxRange, fMaxRange, fMaxRange );
}

void CPointLightObject::Render( CRenderContext2D& context )
{
	if( context.eRenderPass != eRenderPass_Occlusion )
		return;
	m_light.Pos = context.rectScene.GetTexCoord( CVector2( globalTransform.m02, globalTransform.m12 ) );
	m_light.AttenuationIntensity = AttenuationIntensity;
	float fScale = CVector2( globalTransform.m00, globalTransform.m10 ).Length();
	if( fScale <= 0 )
		return;
	m_light.AttenuationIntensity.y /= fScale;
	m_light.AttenuationIntensity.z /= fScale * fScale;
	m_light.fShadowScale = fShadowScale;
	m_light.fMaxRange = fMaxRange * fScale;
	m_light.fLightHeight = fLightHeight;
	context.AddPointLight( &m_light );
}


template<uint32 SampleCount>
class CLight2DLinearBlurPS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DLinearBlurPS );
protected:
	virtual void SetMacros( SShaderMacroDef& macros ) override
	{
		char szBuf[32];
		sprintf( szBuf, "%d", SampleCount );
		macros.Add( "MAX_OFFSET", szBuf );
	}
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().BindArray( m_texOffset, ELEM_COUNT( m_texOffset ), "texOffset" );
		GetShader()->GetShaderInfo().Bind( m_shadowMap, "ShadowMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pShadowMap, CVector3* pOffsets )
	{
		for( int i = 0; i < ELEM_COUNT( m_texOffset ); i++ )
		{
			m_texOffset[i].Set( pRenderSystem, pOffsets + i, 12 );
		}
		m_shadowMap.Set( pRenderSystem, pShadowMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_texOffset[SampleCount];
	CShaderParamShaderResource m_shadowMap;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CLight2DLinearBlurPS<3>, "Shader/Light2D.shader", "PSLinearBlur", "ps_5_0" );

class CLight2DLinearScenePS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DLinearScenePS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_lightAttenuationIntensity, "LightAttenuationIntensity" );
		GetShader()->GetShaderInfo().Bind( m_shadowScale, "fShadowScale" );
		GetShader()->GetShaderInfo().Bind( m_shadowMap, "ShadowMap" );
		GetShader()->GetShaderInfo().Bind( m_occlusionMap, "OcclusionMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, float fIntensity, float fShadowScale, IShaderResource* pShadowMap, IShaderResource* pOcclusionMap )
	{
		m_lightAttenuationIntensity.Set( pRenderSystem, &fIntensity, sizeof( float ), sizeof(float)* 3 );
		m_shadowScale.Set( pRenderSystem, &fShadowScale );
		m_shadowMap.Set( pRenderSystem, pShadowMap );
		m_occlusionMap.Set( pRenderSystem, pOcclusionMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_lightAttenuationIntensity;
	CShaderParam m_shadowScale;
	CShaderParamShaderResource m_shadowMap;
	CShaderParamShaderResource m_occlusionMap;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CLight2DLinearScenePS, "Shader/Light2D.shader", "PSLinearSceneLighting", "ps_5_0" );

template<uint32 SampleCount>
class CLight2DRadialBlurPS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DRadialBlurPS );
protected:
	virtual void SetMacros( SShaderMacroDef& macros ) override
	{
		char szBuf[32];
		sprintf( szBuf, "%d", SampleCount );
		macros.Add( "MAX_OFFSET", szBuf );
	}
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_center, "RadialBlurCenter" );
		GetShader()->GetShaderInfo().Bind( m_baseOffset, "RadialBlurBaseLength" );
		GetShader()->GetShaderInfo().Bind( m_lightHeight, "LightHeight" );
		GetShader()->GetShaderInfo().Bind( m_shadowMap, "ShadowMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pShadowMap, const CVector2& center, float fBaseOffset, float fLightHeight )
	{
		m_center.Set( pRenderSystem, &center );
		m_baseOffset.Set( pRenderSystem, &fBaseOffset );
		m_lightHeight.Set( pRenderSystem, &fLightHeight );
		m_shadowMap.Set( pRenderSystem, pShadowMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_center;
	CShaderParam m_baseOffset;
	CShaderParam m_lightHeight;
	CShaderParamShaderResource m_shadowMap;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CLight2DRadialBlurPS<3>, "Shader/Light2D.shader", "PSRadialBlur", "ps_5_0" );

class CLight2DRadialScenePS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DRadialScenePS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_center, "RadialBlurCenter" );
		GetShader()->GetShaderInfo().Bind( m_lightAttenuationIntensity, "LightAttenuationIntensity" );
		GetShader()->GetShaderInfo().Bind( m_shadowScale, "fShadowScale" );
		GetShader()->GetShaderInfo().Bind( m_shadowMap, "ShadowMap" );
		GetShader()->GetShaderInfo().Bind( m_occlusionMap, "OcclusionMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, const CVector2& center, const CVector4& vecAttenuationIntensity, float fShadowScale, IShaderResource* pShadowMap, IShaderResource* pOcclusionMap )
	{
		m_center.Set( pRenderSystem, &center );
		m_lightAttenuationIntensity.Set( pRenderSystem, &vecAttenuationIntensity );
		m_shadowScale.Set( pRenderSystem, &fShadowScale );
		m_shadowMap.Set( pRenderSystem, pShadowMap );
		m_occlusionMap.Set( pRenderSystem, pOcclusionMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_center;
	CShaderParam m_lightAttenuationIntensity;
	CShaderParam m_shadowScale;
	CShaderParamShaderResource m_shadowMap;
	CShaderParamShaderResource m_occlusionMap;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CLight2DRadialScenePS, "Shader/Light2D.shader", "PSRadialSceneLighting", "ps_5_0" );


void CLighted2DRenderer::RenderLightDirectional( IRenderSystem* pSystem, SDirectionalLight2D& light )
{
	RenderShadowDirectional( pSystem, light );
	DrawSceneLightingDirectional( pSystem, light );
}

void CLighted2DRenderer::RenderShadowDirectional( IRenderSystem* pSystem, SDirectionalLight2D& light )
{
	CVector3 texofs[3];
	for( int i = 0; i < 3; i++ )
	{
		texofs[i].x = -light.Dir.x * ( i + 1 ) / m_lightMapRes.x;
		texofs[i].y = light.Dir.y * ( i + 1 ) / m_lightMapRes.y;
		texofs[i].z = 1.0f / light.fMaxShadowDist * ( i + 1 );
	}

	ITexture* pSrc = m_pOcclusionBuffer;
	ITexture* pDst = m_pShadowBuffer[1];
	SViewport viewport = { 0, 0, m_lightMapRes.x, m_lightMapRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<false>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CLight2DLinearBlurPS<3>::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle rect( 0, 0, m_lightMapRes.x, m_lightMapRes.y );
	rect.SetSizeX( m_screenRes.x );
	rect.SetSizeY( m_screenRes.y );
	if( light.Dir.x > 0 )
		rect.SetLeft( rect.GetLeft() - light.Dir.x * light.fMaxShadowDist - 1 );
	else
		rect.SetRight( rect.GetRight() - light.Dir.x * light.fMaxShadowDist + 1 );
	if( -light.Dir.y > 0 )
		rect.SetTop( rect.GetTop() + light.Dir.y * light.fMaxShadowDist - 1 );
	else
		rect.SetBottom( rect.GetBottom() + light.Dir.y * light.fMaxShadowDist + 1 );
	rect = rect * CRectangle( 0, 0, m_lightMapRes.x, m_lightMapRes.y );

	pVertexShader->SetParams( pSystem, rect, rect, m_lightMapRes, m_lightMapRes );

	for( ;; )
	{
		pSystem->SetRenderTarget( pDst->GetRenderTarget(), NULL );
		pPixelShader->SetParams( pSystem, pSrc->GetShaderResource(), texofs );

		pSystem->DrawInput();
		
		CReference<ITexture> pTempTexture;
		pTempTexture = m_pShadowBuffer[0];
		m_pShadowBuffer[0] = m_pShadowBuffer[1];
		m_pShadowBuffer[1] = pTempTexture;
		pSrc = m_pShadowBuffer[0];
		pDst = m_pShadowBuffer[1];

		for( int i = 0; i < 3; i++ )
		{
			texofs[i] = texofs[i] * 4;
		}
		if( abs( texofs[0].x ) >= m_lightMapRes.x || abs( texofs[0].y ) >= m_lightMapRes.y || texofs[0].z >= 1 )
			break;
	}
}

void CLighted2DRenderer::DrawSceneLightingDirectional( IRenderSystem* pSystem, SDirectionalLight2D& light )
{
	pSystem->SetRenderTarget( m_pLightAccumulationBuffer->GetRenderTarget(), NULL );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpAdd>() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CLight2DLinearScenePS::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle dstRect( 0, 0, m_screenRes.x, m_screenRes.y );
	CRectangle srcRect( 0, 0, m_lightMapRes.x, m_lightMapRes.y );
	CVector2 srcRes = srcRect.GetSize();
	srcRect.SetSizeX( m_screenRes.x );
	srcRect.SetSizeY( m_screenRes.y );

	pVertexShader->SetParams( pSystem, dstRect, srcRect, dstRect.GetSize(), srcRes );
	pPixelShader->SetParams( pSystem, light.fIntensity, light.fShadowScale, m_pShadowBuffer[0]->GetShaderResource(), m_pOcclusionBuffer->GetShaderResource() );

	pSystem->DrawInput();
}

void CLighted2DRenderer::RenderLightPoint( IRenderSystem* pSystem, SPointLight2D& light )
{
	RenderShadowPoint( pSystem, light );
	DrawSceneLightingPoint( pSystem, light );
}

void CLighted2DRenderer::RenderShadowPoint( IRenderSystem* pSystem, SPointLight2D& light )
{
	float fBaseOffset = 1.0f / m_lightMapRes.x;

	ITexture* pSrc = m_pOcclusionBuffer;
	ITexture* pDst = m_pShadowBuffer[1];
	SViewport viewport = { 0, 0, m_lightMapRes.x, m_lightMapRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<false>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CLight2DRadialBlurPS<3>::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle dstRect( ( light.Pos.x - light.fMaxRange ) * m_lightMapRes.x,
		( light.Pos.y - light.fMaxRange ) * m_lightMapRes.y,
		light.fMaxRange * m_lightMapRes.x * 2, light.fMaxRange * m_lightMapRes.y * 2 );
	CRectangle srcRect = dstRect;

	pVertexShader->SetParams( pSystem, dstRect, srcRect, CVector2( m_lightMapRes.x, m_lightMapRes.y ), CVector2( m_lightMapRes.x, m_lightMapRes.y ) );

	for( ;; )
	{
		pSystem->SetRenderTarget( pDst->GetRenderTarget(), NULL );
		pPixelShader->SetParams( pSystem, pSrc->GetShaderResource(), light.Pos, fBaseOffset, light.fLightHeight );

		pSystem->DrawInput();
		
		CReference<ITexture> pTempTexture;
		pTempTexture = m_pShadowBuffer[0];
		m_pShadowBuffer[0] = m_pShadowBuffer[1];
		m_pShadowBuffer[1] = pTempTexture;
		pSrc = m_pShadowBuffer[0];
		pDst = m_pShadowBuffer[1];

		fBaseOffset = fBaseOffset * 4;
		if( fBaseOffset * fBaseOffset >= light.fMaxRange * light.fMaxRange )
			break;
	}
}

void CLighted2DRenderer::DrawSceneLightingPoint( IRenderSystem* pSystem, SPointLight2D& light )
{
	pSystem->SetRenderTarget( m_pLightAccumulationBuffer->GetRenderTarget(), NULL );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpAdd>() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CLight2DRadialScenePS::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle srcRect( ( light.Pos.x - light.fMaxRange ) * m_lightMapRes.x,
		( light.Pos.y - light.fMaxRange ) * m_lightMapRes.y,
		light.fMaxRange * m_lightMapRes.x * 2, light.fMaxRange * m_lightMapRes.y * 2 );
	CRectangle dstRect( 0, 0, m_screenRes.x, m_screenRes.y );
	dstRect.SetSizeX( m_lightMapRes.x );
	dstRect.SetSizeY( m_lightMapRes.y );
	dstRect.x += srcRect.x;
	dstRect.y += srcRect.y;
	dstRect.width = dstRect.height = light.fMaxRange * m_lightMapRes.x * 2;

	pVertexShader->SetParams( pSystem, dstRect, srcRect, CVector2( m_screenRes.x, m_screenRes.y ), CVector2( m_lightMapRes.x, m_lightMapRes.y ) );
	pPixelShader->SetParams( pSystem, light.Pos, light.AttenuationIntensity, light.fShadowScale, m_pShadowBuffer[0]->GetShaderResource(), m_pOcclusionBuffer->GetShaderResource() );

	pSystem->DrawInput();
}