#include "stdafx.h"
#include "PostEffects.h"
#include "Render/GlobalRenderResources.h"
#include "Render/CommonShader.h"
#include "RenderState.h"
#include "MyGame.h"
#include "Effects/CrackMeshGenerator.h"

class CInvertColorShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CInvertColorShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_paramPercent, "fPercent" );
		GetShader()->GetShaderInfo().Bind( m_tex, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex, float fPercent )
	{
		m_paramPercent.Set( pRenderSystem, &fPercent );
		m_tex.Set( pRenderSystem, pTex );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_paramPercent;
	CShaderParamShaderResource m_tex;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CInvertColorShader, "Shader/PostProcessInvertColor.shader", "PSMain", "ps_5_0" );

void CPostProcessInvertColor::Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget )
{
	CReference<ITexture>& src = pPass->GetTarget();
	CReference<ITexture> dst;
	IRenderTarget* pTarget = pFinalTarget;
	auto& sizeDependentPool = CRenderTargetPool::GetSizeDependentPool();
	if( !pTarget )
	{
		sizeDependentPool.AllocRenderTarget( dst, src->GetDesc() );
		pTarget = dst->GetRenderTarget();
	}

	IRenderSystem* pSystem = pPass->GetRenderSystem();
	pSystem->SetRenderTarget( pTarget, NULL );
	
	CVector2 size( src->GetDesc().nDim1, src->GetDesc().nDim2 );
	SViewport viewport = { 0, 0, size.x, size.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CInvertColorShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle rect( 0, 0, size.x, size.y );
	pVertexShader->SetParams( pSystem, rect, rect, size, size );
	pPixelShader->SetParams( pSystem, src->GetShaderResource(), m_fPercent );

	pSystem->DrawInput();

	if( dst )
	{
		sizeDependentPool.Release( src );
		src = dst;
	}
}

class CDizzyEffectShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CDizzyEffectShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_paramPercent, "fInvertPercent" );
		GetShader()->GetShaderInfo().BindArray( m_texOfs, 5, "texofs" );
		GetShader()->GetShaderInfo().BindArray( m_weights, 5, "weights" );
		GetShader()->GetShaderInfo().BindArray( m_t, 2, "t" );
		GetShader()->GetShaderInfo().BindArray( m_tx, 2, "tx" );
		GetShader()->GetShaderInfo().BindArray( m_ty, 2, "ty" );
		GetShader()->GetShaderInfo().BindArray( m_tb, 2, "tb" );
		GetShader()->GetShaderInfo().Bind( m_tex, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex, CVector4* texOfs, CVector4* weights, float fPercent,
		CVector4* t, CVector4* tx, CVector4* ty, CVector4* tb )
	{
		m_paramPercent.Set( pRenderSystem, &fPercent );
		for( int i = 0; i < 5; i++ )
		{
			m_texOfs[i].Set( pRenderSystem, &texOfs[i] );
			m_weights[i].Set( pRenderSystem, &weights[i] );
		}
		for( int i = 0; i < 2; i++ )
		{
			m_t[i].Set( pRenderSystem, &t[i] );
			m_tx[i].Set( pRenderSystem, &tx[i] );
			m_ty[i].Set( pRenderSystem, &ty[i] );
			m_tb[i].Set( pRenderSystem, &tb[i] );
		}
		m_tex.Set( pRenderSystem, pTex );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_paramPercent;
	CShaderParam m_texOfs[5];
	CShaderParam m_weights[5];
	CShaderParam m_t[2], m_tx[2], m_ty[2], m_tb[2];
	CShaderParamShaderResource m_tex;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CDizzyEffectShader, "Shader/PostProcessDizzyEffect.shader", "PSMain", "ps_5_0" );

void CPostProcessDizzyEffect::Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget )
{
	CReference<ITexture>& src = pPass->GetTarget();
	CReference<ITexture> dst;
	IRenderTarget* pTarget = pFinalTarget;
	auto& sizeDependentPool = CRenderTargetPool::GetSizeDependentPool();
	if( !pTarget )
	{
		sizeDependentPool.AllocRenderTarget( dst, src->GetDesc() );
		pTarget = dst->GetRenderTarget();
	}

	IRenderSystem* pSystem = pPass->GetRenderSystem();
	pSystem->SetRenderTarget( pTarget, NULL );
	
	CVector2 size( src->GetDesc().nDim1, src->GetDesc().nDim2 );
	SViewport viewport = { 0, 0, size.x, size.y, 0, 1 };
	pSystem->SetViewports( &viewport, 1 );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<>() );

	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CDizzyEffectShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CVector4 texOfs[5];
	CVector2 invSize( 1.0f / size.x, 1.0f / size.y );
	for( int i = 0; i < 5; i++ )
	{
		texOfs[i].x = invSize.x * m_texofs[i].x;
		texOfs[i].y = invSize.y * m_texofs[i].y;
		texOfs[i].z = invSize.x * m_texofs[i].z;
		texOfs[i].w = invSize.y * m_texofs[i].w;
	}

	double dt = CGame::Inst().GetTotalTime();
	CVector4 t[2] =
	{
		CVector4( dt * 8.4, dt * 7.33, dt * 8.34, dt * 6.18 ),
		CVector4( dt * 6.33, dt * 5.11, dt * 9.58, dt * 6.77 ),
	};
	CVector4 tx[2] =
	{
		CVector4( 0.0123, 0.0023, 0.0103, -0.0031 ) * size.x,
		CVector4( -0.0078, -0.0042, -0.0103, 0.0051 ) * size.x,
	};
	CVector4 ty[2] =
	{
		CVector4( 0.0087, -0.0101, 0.0074, -0.0098 ) * size.y,
		CVector4( -0.0063, -0.0031, 0.0028, 0.0107 ) * size.y,
	};
	CVector4 tb[2] =
	{
		CVector4( 1.03, 0.64, 0.43, 0.69 ),
		CVector4( 0.88, 0.86, 0.77, 0.32 ),
	};

	CRectangle rect( 0, 0, size.x, size.y );
	pVertexShader->SetParams( pSystem, rect, rect, size, size );
	pPixelShader->SetParams( pSystem, src->GetShaderResource(), texOfs, m_weights, m_fInvertPercent, t, tx, ty, tb );

	pSystem->DrawInput();

	if( dst )
	{
		sizeDependentPool.Release( src );
		src = dst;
	}
}

class CCrackMeshDrawVS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CCrackMeshDrawVS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_invDstSrcResolution, "InvDstSrcResolution" );
		GetShader()->GetShaderInfo().Bind( m_time, "fTime" );
		GetShader()->GetShaderInfo().Bind( m_flyTime, "fFlyTime" );
		GetShader()->GetShaderInfo().Bind( m_flySpeed, "fFlySpeed" );
		GetShader()->GetShaderInfo().Bind( m_fRotSpeed, "fRotSpeed" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, const CVector2& resolution, float fTime, float fFlyTime, float fFlySpeed, float fRotSpeed )
	{
		CVector2 invResolution( 1.0f / resolution.x, 1.0f / resolution.y );
		m_invDstSrcResolution.Set( pRenderSystem, &invResolution );
		m_time.Set( pRenderSystem, &fTime );
		m_flyTime.Set( pRenderSystem, &fFlyTime );
		m_flySpeed.Set( pRenderSystem, &fFlySpeed );
		m_fRotSpeed.Set( pRenderSystem, &fRotSpeed );
	}
private:
	CShaderParam m_invDstSrcResolution;
	CShaderParam m_time;
	CShaderParam m_flyTime;
	CShaderParam m_flySpeed;
	CShaderParam m_fRotSpeed;
};

class CCrackMeshDrawPS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CCrackMeshDrawPS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_baseColor, "baseColor" );
		GetShader()->GetShaderInfo().Bind( m_tex, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex, const CVector3& color )
	{
		m_baseColor.Set( pRenderSystem, &color );
		m_tex.Set( pRenderSystem, pTex );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_baseColor;
	CShaderParamShaderResource m_tex;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CCrackMeshDrawVS, "Shader/CrackEffect.shader", "VSDraw", "vs_5_0" );
IMPLEMENT_GLOBAL_SHADER( CCrackMeshDrawPS, "Shader/CrackEffect.shader", "PSDraw", "ps_5_0" );

CPostProcessCrackEffect::~CPostProcessCrackEffect()
{
	if( m_pMesh )
		delete m_pMesh;
}

void CPostProcessCrackEffect::Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget )
{
	auto pSystem = pPass->GetRenderSystem();

	if( !m_pMesh )
		m_pMesh = new CCrackMeshGenerator;
	m_pMesh->Update( pSystem, m_fElapsedTime );
	m_fElapsedTime = 0;
	if( !m_pMesh->GetVertexBuffer() )
		return;

	CReference<ITexture>& src = pPass->GetTarget();
	CReference<ITexture> dst;
	IRenderTarget* pTarget = pFinalTarget;
	auto& sizeDependentPool = CRenderTargetPool::GetSizeDependentPool();
	if( !pTarget )
	{
		sizeDependentPool.AllocRenderTarget( dst, src->GetDesc() );
		pTarget = dst->GetRenderTarget();
	}

	CVector2 size( src->GetDesc().nDim1, src->GetDesc().nDim2 );
	CRectangle rect( 0, 0, size.x, size.y );
	CopyToRenderTarget( pSystem, pTarget, src, rect, rect, size, size );

	pSystem->SetBlendState( IBlendState::Get<>() );
	pSystem->SetDepthStencilState( IDepthStencilState::Get<>() );
	pSystem->SetRasterizerState( IRasterizerState::Get<EFillModeSolid, ECullModeNone>() );

	pSystem->SetVertexBuffer( 0, m_pMesh->GetVertexBuffer() );

	auto pVertexShader = CCrackMeshDrawVS::Inst();
	auto pPixelShader = CCrackMeshDrawPS::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc** ppDesc = m_pMesh->GetVBDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), ppDesc, 1 );

	pVertexShader->SetParams( pSystem, size, m_pMesh->GetTime(), 0.02f, 400, 40.0f );
	pPixelShader->SetParams( pSystem, src->GetShaderResource(), CVector3( 1, 1, 1 ) );

	pSystem->Draw( m_pMesh->GetVertCount(), 0 );

	if( dst )
	{
		sizeDependentPool.Release( src );
		src = dst;
	}
}

bool CPostProcessCrackEffect::HasEffect()
{
	return m_fElapsedTime > 0 || m_pMesh && m_pMesh->GetVertCount();
}

void CPostProcessCrackEffect::Reset()
{
	m_fElapsedTime = 0;
	if( m_pMesh )
		m_pMesh->Reset();
}

void Game_ShaderImplement_Dummy_PostEffects()
{

}