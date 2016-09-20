#include "stdafx.h"
#include "MainUI.h"
#include "MyGame.h"
#include "Render/Scene2DManager.h"
#include "HurtEffect.h"
#include "HpBar.h"
#include "Common/Rand.h"
#include "Player.h"
#include "Render/Lighted2DRenderer.h"
#include "PostEffects.h"
#include "Render/CommonShader.h"
#include "Common/ResourceManager.h"

class CMainUIShader : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CMainUIShader );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_paramColor, "VignetteColor" );
		GetShader()->GetShaderInfo().Bind( m_tex, "Texture0" );
		GetShader()->GetShaderInfo().Bind( m_tex1, "Texture1" );
		GetShader()->GetShaderInfo().Bind( m_paramLinearSampler, "LinearSampler" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, IShaderResource* pTex, IShaderResource* pTex1, const CVector4& color )
	{
		m_paramColor.Set( pRenderSystem, &color );
		m_tex.Set( pRenderSystem, pTex );
		m_tex1.Set( pRenderSystem, pTex1 );
		m_paramLinearSampler.Set( pRenderSystem, ISamplerState::Get<ESamplerFilterLLL>() );
	}
private:
	CShaderParam m_paramColor;
	CShaderParamShaderResource m_tex;
	CShaderParamShaderResource m_tex1;
	CShaderParamSampler m_paramLinearSampler;
};

IMPLEMENT_GLOBAL_SHADER( CMainUIShader, "Shader/MainUI.shader", "PSMain", "ps_5_0" );

CMainUI::CMainUI()
	: m_bVisible( false )
	, m_pHpBar( NULL )
	, m_pMpBar( NULL )
	, m_pSpBar( NULL )
	, m_vignetteColor( 0, 0, 0, 1 )
	, m_vignetteColor1( 0, 0, 0, 1 )
	, m_fVignetteColorChangeTime( 0 )
	, m_tick( this, &CMainUI::Tick )
	, m_tickNewHpBarShake( this, &CMainUI::OnNewHpBarShake )
{
	m_element2D.SetDrawable( this );
	m_localBound = CRectangle( -10000, -10000, 20000, 20000 );

	m_pCanvas = new CCanvas( true, 0, 0, EFormat::EFormatR8G8B8A8UNorm, CCanvas::eDepthStencilType_UseDefault, false );
	m_pCanvas->SetRoot( new CRenderObject2D );
	m_guiCamera.SetPriority( 1 );
}

CMainUI::~CMainUI()
{
	delete m_pCanvas;
}

void CMainUI::InitResources()
{
	m_pScratchTex = CResourceManager::Inst()->CreateResource<CTextureFile>( "textures/scratch.png" );

	CHurtEffectMgr::Inst();
	CHpBar::Init();

	m_hpBarOrigPos = CVector2( -350, 250 );
	m_pHpBar = new CHpBar( CRectangle( 0, 0, 256, 8 ), 0.5f, 3, CVector4( 0.75f, 0, 0, 0 ) );
	m_pHpBar->x = m_hpBarOrigPos.x;
	m_pHpBar->y = m_hpBarOrigPos.y;
	m_pHpBar->SetZOrder( 1 );
	m_pCanvas->GetRoot()->AddChild( m_pHpBar );
	m_pMpBar = new CHpBar( CRectangle( 0, 0, 256, 8 ), 0.5f, 3, CVector4( 0, 0.75f, 0, 0 ) );
	m_pMpBar->x = m_hpBarOrigPos.x;
	m_pMpBar->y = m_hpBarOrigPos.y;
	m_pMpBar->SetZOrder( 1 );
	m_pCanvas->GetRoot()->AddChild( m_pMpBar );
	m_pSpBar = new CHpBar( CRectangle( 0, 0, 256, 8 ), 0.5f, 3, CVector4( 0, 0, 0.75f, 0 ) );
	m_pSpBar->x = m_hpBarOrigPos.x;
	m_pSpBar->y = m_hpBarOrigPos.y;
	m_pSpBar->SetZOrder( 1 );
	m_pCanvas->GetRoot()->AddChild( m_pSpBar );
}

void CMainUI::Render( CRenderContext2D& context )
{
	if( context.eRenderPass != eRenderPass_Color )
		return;
	context.AddElement( &m_element2D, 1 );
	m_pCanvas->SetSize( context.screenRes );
	m_pCanvas->Render( context );
}

void CMainUI::Flush( CRenderContext2D& context )
{
	IRenderSystem* pSystem = context.pRenderSystem;
	pSystem->SetBlendState( IBlendState::Get<false, false, 0xf, EBlendOne, EBlendInvSrcAlpha>() );
	
	pSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );

	auto pVertexShader = CScreenVertexShader::Inst();
	auto pPixelShader = CMainUIShader::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), pPixelShader->GetShader(), &pDesc, 1 );

	CRectangle rect( 0, 0, context.screenRes.x, context.screenRes.y );
	pVertexShader->SetParams( pSystem, rect, rect, context.screenRes, context.screenRes );
	pPixelShader->SetParams( pSystem, m_pCanvas->GetTexture()->GetShaderResource(), m_pScratchTex->GetTexture()->GetShaderResource(), m_vignetteColor );

	pSystem->DrawInput();

	m_pCanvas->ReleaseTexture();
	m_pElement->OnFlushed();
}

void CMainUI::SetVisible( bool bVisible )
{
	if( m_bVisible == bVisible )
		return;
	m_bVisible = bVisible;
	if( bVisible )
	{
		m_guiCamera.SetViewport( 0, 0, 800, 600 );
		m_guiCamera.SetPosition( 0, 0 );
		m_guiCamera.SetSize( 800, 600 );
		CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( this );
		CScene2DManager::GetGlobalInst()->AddActiveCamera( &m_guiCamera, this );

		CCamera2D& camInnerLayer = m_pCanvas->GetCamera();
		camInnerLayer.SetViewport( 0, 0, 800, 600 );
		camInnerLayer.SetPosition( 0, 0 );
		camInnerLayer.SetSize( 800, 600 );
		CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( m_pCanvas->GetRoot() );

		CGame::Inst().Register( 1, &m_tick );
		CGame::Inst().Register( 1, &m_tickNewHpBarShake );
	}
	else
	{
		if( m_tick.IsRegistered() )
			m_tick.Unregister();
		if( m_tickNewHpBarShake.IsRegistered() )
			m_tickNewHpBarShake.Unregister();
		m_pHpBar->OnHide();
		m_pMpBar->OnHide();
		m_pSpBar->OnHide();
		CScene2DManager::GetGlobalInst()->RemoveActiveCamera( &m_guiCamera );
		CScene2DManager::GetGlobalInst()->GetRoot()->RemoveChild( m_pCanvas->GetRoot() );
	}
}

void CMainUI::AddHurtEffect( const char* szEffect, const CVector2& ofs )
{
	CRenderObject2D* pObject = CHurtEffectMgr::Inst().Create( szEffect, ofs );
	if( pObject )
		m_pCanvas->GetRoot()->AddChild( pObject );

	SHpBarShake newShake;
	float fRadius = SRand::Inst().Rand( 12.0f, 15.0f );
	float fAngle = SRand::Inst().Rand<float>( PI * 0.5f, PI );
	newShake.maxOfs = CVector2( fRadius * cos( fAngle ), fRadius * sin( fAngle ) );
	newShake.nMaxTime = SRand::Inst().Rand( 20, 40 );
	newShake.t = 0;
	m_vecHpBarShakes.push_back( newShake );
}

void CMainUI::SetHpBarVisible( bool bVisible )
{
	m_pHpBar->bVisible = bVisible;
	m_pMpBar->bVisible = bVisible;
	m_pSpBar->bVisible = bVisible;
}

void CMainUI::OnModifyHp( float fHp, float fMaxHp )
{
	float fPercent = fMaxHp > 0? fHp / fMaxHp: 0;
	fPercent = Max( Min( fPercent, 1.0f ), 0.0f );
	m_pHpBar->UpdateHp( fPercent );
}

void CMainUI::OnModifyMp( float fMp, float fMaxMp )
{
	float fPercent = fMaxMp > 0? fMp / fMaxMp: 0;
	fPercent = Max( Min( fPercent, 1.0f ), 0.0f );
	m_pMpBar->UpdateHp( fPercent );
}

void CMainUI::OnModifySp( float fSp, float fMaxSp )
{
	float fPercent = fMaxSp > 0? fSp / fMaxSp: 0;
	fPercent = Max( Min( fPercent, 1.0f ), 0.0f );
	m_pSpBar->UpdateHp( fPercent );
}

void CMainUI::SetVignetteColor( const CVector4& color, float fTimeScale )
{
	m_vignetteColor1 = color;
	CVector4 dColor = m_vignetteColor1 - m_vignetteColor;
	m_fVignetteColorChangeTime = dColor.Length() * fTimeScale;
	if( m_fVignetteColorChangeTime <= 0 )
		m_vignetteColor = color;
}

void CMainUI::SetVignetteColorFixedTime( const CVector4& color, float fTime )
{
	m_vignetteColor1 = color;
	m_fVignetteColorChangeTime = fTime;
	if( m_fVignetteColorChangeTime <= 0 )
		m_vignetteColor = color;
}

void CMainUI::Tick()
{
	float fElapsedTime = CGame::Inst().GetElapsedTimePerTick();
	float fTime = m_fVignetteColorChangeTime;
	if( fTime > 0 )
	{
		m_fVignetteColorChangeTime -= fElapsedTime;
		if( m_fVignetteColorChangeTime < 0 )
			m_fVignetteColorChangeTime = 0;
		float t = m_fVignetteColorChangeTime / fTime;
		m_vignetteColor = m_vignetteColor1 + ( m_vignetteColor - m_vignetteColor1 ) * t;
	}

	CVector2 ofs( 0, 0 );
	for( int i = m_vecHpBarShakes.size() - 1; i >= 0; i-- )
	{
		auto& item = m_vecHpBarShakes[i];
		item.t++;
		if( item.t >= item.nMaxTime )
		{
			item = m_vecHpBarShakes.back();
			m_vecHpBarShakes.pop_back();
			continue;
		}

		float t = item.t * 1.0f / item.nMaxTime;
		t = t * t * t;
		t = 1 - abs( t - 0.5f ) * 2;
		ofs = ofs + item.maxOfs * t;
	}
	
	m_pHpBar->x = m_hpBarOrigPos.x + ofs.x;
	m_pHpBar->y = m_hpBarOrigPos.y + ofs.y;
	m_pHpBar->SetTransformDirty();
	m_pSpBar->x = m_hpBarOrigPos.x - ofs.x;
	m_pSpBar->y = m_hpBarOrigPos.y - ofs.y;
	m_pSpBar->SetTransformDirty();

	CGame::Inst().Register( 1, &m_tick );
}

void CMainUI::OnNewHpBarShake()
{
	/*float fPercent = 0;
	CPlayer* pPlayer = CGame::Inst().GetWorld()->GetPlayer();
	if( pPlayer )
	{
		CVector3 hpPercent;
		hpPercent.x = 1 - pPlayer->GetHp() * 1.0f / pPlayer->GetMaxHp();
		hpPercent.y = hpPercent.z = 0;
		fPercent = hpPercent.Length() / 1.732f;
		fPercent = sqrt( fPercent );
	}

	SHpBarShake newShake;
	float fRadius = SRand::Inst().Rand( 1.0f + 3.0f * fPercent, 1.5f + 4.5f * fPercent );
	float fAngle = SRand::Inst().Rand<float>( 0, PI * 2 );
	newShake.maxOfs = CVector2( fRadius * cos( fAngle ) * 0.75f, fRadius * sin( fAngle ) * 1.5f );
	newShake.nMaxTime = SRand::Inst().Rand( 10, 60 );
	newShake.t = 0;
	m_vecHpBarShakes.push_back( newShake );

	CGame::Inst().Register( SRand::Inst().Rand( 50 - 40 * fPercent, 80 - 60 * fPercent ), &m_tickNewHpBarShake );*/
}

void Game_ShaderImplement_Dummy_MainUI()
{

}