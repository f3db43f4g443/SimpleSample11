#include "stdafx.h"
#include "LightRendering.h"
#include "Lighted2DRenderer.h"
#include "GlobalRenderResources.h"
#include "CommonShader.h"
#include "SortedList.h"
#include "MathUtil.h"

CDirectionalLightObject::CDirectionalLightObject() : Dir( 0, -1 ), baseColor( 0, 0, 0 ), fShadowScale( 0 )
{
	m_localBound = CRectangle( -1000000, -1000000, 2000000, 2000000 );
}

CDirectionalLightObject::CDirectionalLightObject( const CVector2& Dir, const CVector3& baseColor, float fShadowScale, float fMaxShadowDist )
	: Dir( Dir ), baseColor( baseColor ), fShadowScale( fShadowScale ), fMaxShadowDist( fMaxShadowDist )
{
	m_localBound = CRectangle( -1000000, -1000000, 2000000, 2000000 );
}

CDirectionalLightObject::CDirectionalLightObject( const CDirectionalLightObject& obj )
	: Dir( obj.Dir ), baseColor( obj.baseColor ), fShadowScale( obj.fShadowScale ), fMaxShadowDist( obj.fMaxShadowDist )
{
	m_localBound = CRectangle( -1000000, -1000000, 2000000, 2000000 );
}

void CDirectionalLightObject::SetDir( const CVector2& dir )
{
	Dir = dir;
	if( Dir.Normalize() < 0.00001f )
		Dir = CVector2( 0, -1 );
}

void CDirectionalLightObject::Render( CRenderContext2D& context )
{
	if( context.eRenderPass != eRenderPass_Occlusion )
		return;
	m_light.Dir = globalTransform.MulVector2Dir( Dir );
	m_light.fShadowScale = fShadowScale;
	m_light.fMaxShadowDist = fMaxShadowDist;
	m_light.baseColor = baseColor;
	context.AddDirectionalLight( &m_light );
}

CPointLightObject::CPointLightObject()
	: AttenuationIntensity( 0, 0, 0, 0 ), fShadowScale( 0 ), fMaxRange( 0 )
{
	m_localBound = CRectangle( 0, 0, 0, 0 );
}

CPointLightObject::CPointLightObject( const CVector4& AttenuationIntensity, const CVector3& baseColor, float fShadowScale, float fMaxRange, float fLightHeight )
	: AttenuationIntensity( AttenuationIntensity ), baseColor( baseColor ), fShadowScale( fShadowScale ), fMaxRange( fMaxRange ), fLightHeight( fLightHeight )
{
	m_localBound = CRectangle( -fMaxRange, -fMaxRange, fMaxRange, fMaxRange );
}

CPointLightObject::CPointLightObject( const CPointLightObject& obj )
	: AttenuationIntensity( obj.AttenuationIntensity ), baseColor( obj.baseColor ), fShadowScale( obj.fShadowScale ), fMaxRange( obj.fMaxRange ), fLightHeight( obj.fLightHeight )
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
	m_light.baseColor = baseColor;
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

IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_CLight2DLinearBlurPS", CLight2DLinearBlurPS<3>, "Shader/Light2D.shader", "PSLinearBlur", "ps_5_0" );

class CLight2DLinearScenePS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DLinearScenePS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_lightBaseColor, "baseColor" );
		GetShader()->GetShaderInfo().Bind( m_shadowScale, "fShadowScale" );
		GetShader()->GetShaderInfo().Bind( m_shadowMap, "ShadowMap" );
		GetShader()->GetShaderInfo().Bind( m_occlusionMap, "OcclusionMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, const CVector3& baseColor, float fShadowScale, IShaderResource* pShadowMap, IShaderResource* pOcclusionMap )
	{
		m_lightBaseColor.Set( pRenderSystem, &baseColor );
		m_shadowScale.Set( pRenderSystem, &fShadowScale );
		m_shadowMap.Set( pRenderSystem, pShadowMap );
		m_occlusionMap.Set( pRenderSystem, pOcclusionMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_lightBaseColor;
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
		GetShader()->GetShaderInfo().Bind( m_baseOffset, "RadialBlurBaseLength" );
		GetShader()->GetShaderInfo().Bind( m_shadowMap, "ShadowMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pShadowMap, float fBaseOffset )
	{
		m_baseOffset.Set( pRenderSystem, &fBaseOffset );
		m_shadowMap.Set( pRenderSystem, pShadowMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_baseOffset;
	CShaderParamShaderResource m_shadowMap;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( "g_CLight2DRadialBlurPS", CLight2DRadialBlurPS<3>, "Shader/Light2D.shader", "PSRadialBlur", "ps_5_0" );

class CLight2DRadialScenePS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DRadialScenePS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_shadowMap, "ShadowMap" );
		GetShader()->GetShaderInfo().Bind( m_occlusionMap, "OcclusionMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pShadowMap, IShaderResource* pOcclusionMap )
	{
		m_shadowMap.Set( pRenderSystem, pShadowMap );
		m_occlusionMap.Set( pRenderSystem, pOcclusionMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
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
	pPixelShader->SetParams( pSystem, light.baseColor, light.fShadowScale, m_pShadowBuffer[0]->GetShaderResource(), m_pOcclusionBuffer->GetShaderResource() );

	pSystem->DrawInput();
}


void CLighted2DRenderer::RenderLightPointInstancing( IRenderSystem* pSystem, SPointLight2D* pLights )
{
	static const uint32 nExtension = 1;

	struct CalcLightSize
	{
		unsigned int operator () ( SPointLight2D* light )
		{
			return light->nSizeLog2;
		}
	};
	TSortedList<SPointLight2D*, CalcLightSize> list;
	while( pLights )
	{
		float left = floor( ( pLights->Pos.x - pLights->fMaxRange ) * m_lightMapRes.x );
		float top = floor( ( pLights->Pos.y - pLights->fMaxRange ) * m_lightMapRes.y );
		float right = ceil( ( pLights->Pos.x + pLights->fMaxRange ) * m_lightMapRes.x );
		float bottom = ceil( ( pLights->Pos.y + pLights->fMaxRange ) * m_lightMapRes.y );
		pLights->SceneRect = CRectangle( left, top, right - left, bottom - top );

		uint32 nSizeLog2 = 0;
		uint32 nSize = 1;
		float fSize = Max( pLights->SceneRect.width, pLights->SceneRect.height ) + 2;
		while( nSize < fSize )
		{
			nSize <<= 1;
			nSizeLog2++;
		}
		pLights->nSizeLog2 = nSizeLog2;

		list.insert( pLights );
		SPointLight2D* pPointLight = pLights->NextPointLight();
		pLights->RemoveFrom_PointLight();
		pLights = pPointLight;
	}
	
	uint32 nCur = 0;
	vector<SPointLight2D*> vecLights;
	for( auto itr = list.rev_begin(); !itr.End(); itr.Next() )
	{
		SPointLight2D* pLight = itr.Get();
		
		uint32 nWidth = 1 << pLight->nSizeLog2;
		uint16 x, y;
		ZCurveOrderInv( nCur, x, y );
		
		pLight->ShadowMapRect.x = x + 1;
		pLight->ShadowMapRect.y = y + 1;
		pLight->ShadowMapRect.width = pLight->SceneRect.width;
		pLight->ShadowMapRect.height = pLight->SceneRect.height;
		pLight->Pos1 = pLight->Pos + ( CVector2( pLight->ShadowMapRect.x, pLight->ShadowMapRect.y ) - CVector2( pLight->SceneRect.x, pLight->SceneRect.y ) )
			* CVector2( 1.0f / m_lightMapRes.x, 1.0f / m_lightMapRes.y );
		vecLights.push_back( pLight );

		nCur += nWidth * nWidth;
		if( nCur >= m_lightMapRes.x * m_lightMapRes.x || vecLights.size() >= 682 )
		{
			RenderLightPoint( pSystem, vecLights );

			vecLights.clear();
			nCur = 0;
		}
	}

	if( vecLights.size() )	
		RenderLightPoint( pSystem, vecLights );
}

void CLighted2DRenderer::RenderLightPoint( IRenderSystem* pSystem, vector<SPointLight2D*> vecLights )
{
	RenderShadowPoint( pSystem, vecLights );
	DrawSceneLightingPoint( pSystem, vecLights );
}

class CLight2DRadialBlurVS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DRadialBlurVS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_instData, "g_insts", "InstBuffer" );
		GetShader()->GetShaderInfo().Bind( m_invDstSrcResolution, "InvDstSrcResolution" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, vector<SPointLight2D*> vecLights, const CVector2& DstResolution, const CVector2& SrcResolution, bool bFirstPass )
	{
		uint32 nOffset = 0;
		if( bFirstPass )
		{
			for( int i = 0; i < vecLights.size(); i++ )
			{
				m_instData.Set( pRenderSystem, &vecLights[i]->SceneRect, 32, nOffset );
				CVector4 vec( vecLights[i]->Pos.x, vecLights[i]->Pos.y, vecLights[i]->fLightHeight, 0 );
				m_instData.Set( pRenderSystem, &vec, 16, nOffset + 32 );
				nOffset += 48;
			}
		}
		else
		{
			for( int i = 0; i < vecLights.size(); i++ )
			{
				m_instData.Set( pRenderSystem, &vecLights[i]->ShadowMapRect, 16, nOffset );
				m_instData.Set( pRenderSystem, &vecLights[i]->ShadowMapRect, 16, nOffset + 16 );
				CVector4 vec( vecLights[i]->Pos1.x, vecLights[i]->Pos1.y, vecLights[i]->fLightHeight, 0 );
				m_instData.Set( pRenderSystem, &vec, 16, nOffset + 32 );
				nOffset += 48;
			}
		}
		CVector4 invDstSrcResolution( 1.0f / DstResolution.x, 1.0f / DstResolution.y, 1.0f / SrcResolution.x, 1.0f / SrcResolution.y );
		m_invDstSrcResolution.Set( pRenderSystem, &invDstSrcResolution );
	}
private:
	CShaderParam m_instData;
	CShaderParam m_invDstSrcResolution;
};

class CLight2DRadialBlurVS1 : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLight2DRadialBlurVS1 );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_instData, "g_insts1", "InstBuffer1" );
		GetShader()->GetShaderInfo().Bind( m_invDstSrcResolution, "InvDstSrcResolution" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, vector<SPointLight2D*> vecLights, const CVector2& DstResolution, const CVector2& SrcResolution )
	{
		uint32 nOffset = 0;
		for( int i = 0; i < vecLights.size(); i++ )
		{
			m_instData.Set( pRenderSystem, &vecLights[i]->rect, 48, nOffset );
			m_instData.Set( pRenderSystem, &vecLights[i]->AttenuationIntensity, 16, nOffset + 48 );
			CVector4 vec( vecLights[i]->baseColor.x, vecLights[i]->baseColor.y, vecLights[i]->baseColor.z, vecLights[i]->fShadowScale );
			m_instData.Set( pRenderSystem, &vec, 16, nOffset + 64 );
			m_instData.Set( pRenderSystem, &vecLights[i]->Pos, 8, nOffset + 80 );
			nOffset += 96;
		}
		CVector4 invDstSrcResolution( 1.0f / DstResolution.x, 1.0f / DstResolution.y, 1.0f / SrcResolution.x, 1.0f / SrcResolution.y );
		m_invDstSrcResolution.Set( pRenderSystem, &invDstSrcResolution );
	}
private:
	CShaderParam m_instData;
	CShaderParam m_invDstSrcResolution;
};


IMPLEMENT_GLOBAL_SHADER( CLight2DRadialBlurVS, "Shader/Light2DInstancing.shader", "VSRadialBlur", "vs_5_0" );
IMPLEMENT_GLOBAL_SHADER( CLight2DRadialBlurVS1, "Shader/Light2DInstancing.shader", "VSRadialBlur1", "vs_5_0" );

void CLighted2DRenderer::RenderShadowPoint( IRenderSystem* pSystem, vector<SPointLight2D*> vecLights )
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

	auto pVertexShader = CLight2DRadialBlurVS::Inst();
	auto pPixelShader = CLight2DRadialBlurPS<3>::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	bool bFirstPass = true;
	pVertexShader->SetParams( pSystem, vecLights, CVector2( m_lightMapRes.x, m_lightMapRes.y ), CVector2( m_lightMapRes.x, m_lightMapRes.y ), true );
	uint32 nLights = vecLights.size();

	for( ;; )
	{
		pSystem->SetRenderTarget( pDst->GetRenderTarget(), NULL );
		pPixelShader->SetParams( pSystem, pSrc->GetShaderResource(), fBaseOffset );

		pSystem->DrawInputInstanced( nLights );
		
		CReference<ITexture> pTempTexture;
		pTempTexture = m_pShadowBuffer[0];
		m_pShadowBuffer[0] = m_pShadowBuffer[1];
		m_pShadowBuffer[1] = pTempTexture;
		pSrc = m_pShadowBuffer[0];
		pDst = m_pShadowBuffer[1];

		fBaseOffset = fBaseOffset * 4;
		while( nLights )
		{
			if( fBaseOffset < vecLights[nLights - 1]->fMaxRange )
				break;
			nLights--;
		}
		if( !nLights )
			break;
		if( bFirstPass )
		{
			bFirstPass = false;
			pVertexShader->SetParams( pSystem, vecLights, CVector2( m_lightMapRes.x, m_lightMapRes.y ), CVector2( m_lightMapRes.x, m_lightMapRes.y ), false );
		}
	}
}

void CLighted2DRenderer::DrawSceneLightingPoint( IRenderSystem* pSystem, vector<SPointLight2D*> vecLights )
{
	pSystem->SetRenderTarget( m_pLightAccumulationBuffer->GetRenderTarget(), NULL );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<false, false, 0xf, EBlendOne, EBlendOne, EBlendOpAdd>() );

	auto pVertexShader = CLight2DRadialBlurVS1::Inst();
	auto pPixelShader = CLight2DRadialScenePS::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	for( int i = 0; i < vecLights.size(); i++ )
	{
		CRectangle& srcRect = vecLights[i]->SceneRect;
		CRectangle dstRect( 0, 0, m_screenRes.x, m_screenRes.y );
		dstRect.SetSizeX( m_lightMapRes.x );
		dstRect.SetSizeY( m_lightMapRes.y );
		dstRect.x += srcRect.x;
		dstRect.y += srcRect.y;
		dstRect.width = srcRect.width;
		dstRect.height = srcRect.height;
		vecLights[i]->rect = dstRect;
	}

	pVertexShader->SetParams( pSystem, vecLights, CVector2( m_screenRes.x, m_screenRes.y ), CVector2( m_lightMapRes.x, m_lightMapRes.y ) );
	pPixelShader->SetParams( pSystem, m_pShadowBuffer[0]->GetShaderResource(), m_pOcclusionBuffer->GetShaderResource() );

	pSystem->DrawInputInstanced( vecLights.size() );
}

class CLightScenePixelShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CLightScenePixelShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_tex0, "ColorMap" );
		GetShader()->GetShaderInfo().Bind( m_tex1, "EmissionMap" );
		GetShader()->GetShaderInfo().Bind( m_tex2, "LightMap" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pColorMap, IShaderResource* pEmissionMap, IShaderResource* pLightMap )
	{
		m_tex0.Set( pRenderSystem, pColorMap );
		m_tex1.Set( pRenderSystem, pEmissionMap );
		m_tex2.Set( pRenderSystem, pLightMap );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParamShaderResource m_tex0;
	CShaderParamShaderResource m_tex1;
	CShaderParamShaderResource m_tex2;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CLightScenePixelShader, "Shader/Light2D.shader", "PSScene", "ps_5_0" );

void CLighted2DRenderer::RenderScene( IRenderSystem* pSystem, IRenderTarget* pTarget )
{
	pSystem->SetRenderTarget( pTarget, NULL );
	SViewport viewport = { 0, 0, m_screenRes.x, m_screenRes.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CLightScenePixelShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle dstRect( 0, 0, m_screenRes.x, m_screenRes.y );
	CRectangle srcRect( 0, 0, m_screenRes.x, m_screenRes.y );

	pVertexShader->SetParams( pSystem, dstRect, srcRect, dstRect.GetSize(), srcRect.GetSize() );
	pPixelShader->SetParams( pSystem, m_pColorBuffer->GetShaderResource(), m_pEmissionBuffer->GetShaderResource(), m_pLightAccumulationBuffer->GetShaderResource() );

	pSystem->DrawInput();
}

void Engine_ShaderImplement_Dummy_Light()
{

}