#include "stdafx.h"
#include "Splash.h"
#include "Stage.h"
#include "World.h"
#include "Render/Image2D.h"
#include "Render/Scene2DManager.h"
#include "Render/CommonShader.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"

void CSplashElem::OnAddedToStage()
{
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetPlaySpeed( 0, false );
}

void CSplash::OnAddedToStage()
{
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strElem );
	vector<CSplashElem*> vecElems;
	for( int i = 0; i < m_nTileX; i++ )
	{
		for( int j = 0; j < m_nTileY; j++ )
		{
			auto pElem = SafeCast<CSplashElem>( pPrefab->GetRoot()->CreateInstance() );
			vecElems.push_back( pElem );
			float x = ( i + SRand::Inst().Rand( 0.0f, 1.0f ) ) / m_nTileX;
			float y = ( j + SRand::Inst().Rand( 0.0f, 1.0f ) ) / m_nTileY;
			pElem->SetPosition( CVector2( m_elemRect.x + m_elemRect.width * x, m_elemRect.y + m_elemRect.height * y ) );
		}
	}
	SRand::Inst().Shuffle( vecElems );
	for( auto pElem : vecElems )
	{
		pElem->m_nBaseHeight = pElem->y;
		pElem->SetParentEntity( m_pElemLayer );
	}

	Update();
}

void CSplash::Set( int32 nFloodHeight, int32 nOfs )
{
	m_nFloodHeight = nFloodHeight;
	m_nScrollOfs = nOfs;
	if( GetStage() )
		Update();
}

void CSplash::Update()
{
	int32 nScrollHeight = m_nFloodHeight + m_nScrollOfs;
	for( auto pEntity = m_pElemLayer->Get_ChildEntity(); pEntity; pEntity = pEntity->NextChildEntity() )
	{
		auto pElem = SafeCast<CSplashElem>( pEntity );
		if( !pElem )
			continue;

		int32 nHeight = pElem->m_nBaseHeight - nScrollHeight;
		nHeight = nHeight % pElem->m_nScrollHeight;
		if( nHeight < 0 )
			nHeight += pElem->m_nScrollHeight;
		pElem->SetPosition( CVector2( pElem->x, nHeight + m_nFloodHeight ) );
		static_cast<CMultiFrameImage2D*>( pElem->GetRenderObject() )->SetPlayPercent( nHeight * 1.0f / pElem->m_nScrollHeight );
	}
}

void CSplashRenderer::OnAddedToStage()
{
	m_nSubStage = GetStage()->GetWorld()->PlaySubStage( m_strSplash, NULL );
	auto pStage = GetStage()->GetWorld()->GetSubStage( m_nSubStage )->pStage;
	m_canvasColor.SetRoot( pStage->GetRoot() );
	m_canvasOcclusion.SetRoot( pStage->GetRoot() );
}

void CSplashRenderer::OnRemovedFromStage()
{
	m_canvasColor.SetRoot( NULL );
	m_canvasOcclusion.SetRoot( NULL );
	GetStage()->GetWorld()->StopSubStage( m_nSubStage );
	m_nSubStage = -1;
}

CSplash* CSplashRenderer::GetSplash()
{
	auto pStage = GetStage()->GetWorld()->GetSubStage( m_nSubStage )->pStage;
	if( !pStage )
		return NULL;
	return SafeCast<CSplash>( pStage->GetRoot()->Get_ChildEntity() );
}

void CSplashRenderer::Render( CRenderContext2D & context )
{
	auto& canvas = context.eRenderPass == eRenderPass_Color ? m_canvasColor : m_canvasOcclusion;
	auto& cam = canvas.GetCamera();
	cam.SetViewArea( context.rectScene );
	CRectangle rectViewport = context.rectViewport;
	rectViewport.width /= 2;
	rectViewport.height /= 2;
	cam.SetViewport( rectViewport );
	canvas.SetSize( CVector2( rectViewport.width, rectViewport.height ) );

	canvas.Render( context );
	context.AddElement( &m_elem2D );
}

class CSplashShader : public CGlobalShader
{
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_tex, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_paramSampler, "Sampler" );
		GetShader()->GetShaderInfo().Bind( m_paramColor0, "color0" );
		GetShader()->GetShaderInfo().Bind( m_paramColor1, "color1" );
		GetShader()->GetShaderInfo().Bind( m_paramBlendBegin, "fBlendBegin" );
		GetShader()->GetShaderInfo().Bind( m_paramBlendScale, "fBlendScale" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex, const CVector4& color0, const CVector4& color1, float fBlendBegin, float fBlendScale )
	{
		m_tex.Set( pRenderSystem, pTex );
		m_paramSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterPPP>() );
		m_paramColor0.Set( pRenderSystem, &color0 );
		m_paramColor1.Set( pRenderSystem, &color1 );
		m_paramBlendBegin.Set( pRenderSystem, &fBlendBegin );
		m_paramBlendScale.Set( pRenderSystem, &fBlendScale );
	}
private:
	CShaderParamShaderResource m_tex;
	CShaderParamSampler m_paramSampler;
	CShaderParam m_paramColor0;
	CShaderParam m_paramColor1;
	CShaderParam m_paramBlendBegin;
	CShaderParam m_paramBlendScale;
};

class CSplashShaderColor : public CSplashShader
{
	DECLARE_GLOBAL_SHADER( CSplashShaderColor );
};

class CSplashShaderOcclusion : public CSplashShader
{
	DECLARE_GLOBAL_SHADER( CSplashShaderOcclusion );
};

IMPLEMENT_GLOBAL_SHADER( CSplashShaderColor, "Shader/Splash.shader", "PSColor", "ps_5_0" );
IMPLEMENT_GLOBAL_SHADER( CSplashShaderOcclusion, "Shader/Splash.shader", "PSOcclusion", "ps_5_0" );

void CSplashRenderer::Flush( CRenderContext2D & context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetBlendState( context.eRenderPass == eRenderPass_Color ? IBlendState::Get<>() : IBlendState::Get<false, false, 0xf, EBlendSrcAlpha, EBlendInvSrcAlpha, EBlendOpAdd, EBlendZero, EBlendOne>() );

	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	CSplashShader* pPixelShader;
	if( context.eRenderPass == eRenderPass_Color )
	{
		pPixelShader = CSplashShaderColor::Inst();

		static IShaderBoundState* g_pShaderBoundState = NULL;
		const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
		pRenderSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );
	}
	else
	{
		pPixelShader = CSplashShaderOcclusion::Inst();

		static IShaderBoundState* g_pShaderBoundState = NULL;
		const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
		pRenderSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );
	}

	CVector2 size( context.rectViewport.width, context.rectViewport.height );
	CRectangle rect( 0, 0, size.x, size.y );
	float fDepth = m_elem2D.depth * context.mat.m22 + context.mat.m23;
	pVertexShader->SetParams( pRenderSystem, rect, rect, size, size, fDepth );

	auto pSplash = GetSplash();
	float fHeight = pSplash->m_nFloodHeight;
	float fHeight1 = fHeight + pSplash->m_nBlendHeight;
	fHeight = 0.5f * ( 1 - context.mat.MulVector3Pos( CVector3( 0, fHeight, 0 ) ).y );
	fHeight1 = 0.5f * ( 1 - context.mat.MulVector3Pos( CVector3( 0, fHeight1, 0 ) ).y );
	
	auto& canvas = context.eRenderPass == eRenderPass_Color ? m_canvasColor : m_canvasOcclusion;
	if( context.eRenderPass == eRenderPass_Color )
		pPixelShader->SetParams( pRenderSystem, canvas.GetTexture()->GetShaderResource(), CVector4( 0, 0, 0, 0 ), CVector4( 0.5, 0, 0, 1 ), fHeight1, 1.0f / ( fHeight - fHeight1 ) );
	else
		pPixelShader->SetParams( pRenderSystem, canvas.GetTexture()->GetShaderResource(), CVector4( 1, 1, 1, 0 ), CVector4( 0.5, 0, 0, 1 ), fHeight1, 1.0f / ( fHeight - fHeight1 ) );

	pRenderSystem->DrawInput();

	m_elem2D.OnFlushed();
	canvas.ReleaseTexture();
}

void Game_ShaderImplement_Dummy_Splash()
{

}