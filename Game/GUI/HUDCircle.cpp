#include "stdafx.h"
#include "HUDCircle.h"
#include "Render/DefaultDrawable2D.h"
#include "Common/FileUtil.h"
#include "Common/MathUtil.h"
#include "Common/xml.h"
#include "MyGame.h"

class CHUDCircleDrawable : public CDefaultDrawable2D
{
public:
	void LoadXml( TiXmlElement* pRoot )
	{
		CDefaultDrawable2D::LoadXml( pRoot );
		
		auto pMaterial = pRoot->FirstChildElement( "material" );
		IShader* pPS = m_material.GetShader( EShaderType::PixelShader );
		if( pPS )
		{
			pPS->GetShaderInfo().Bind( m_paramColor, "vColor" );
			pPS->GetShaderInfo().Bind( m_paramColor1, "vColor1" );
			pPS->GetShaderInfo().Bind( m_paramRad, "fRad" );
			pPS->GetShaderInfo().Bind( m_paramWidth, "fWidth" );
			pPS->GetShaderInfo().Bind( m_paramDir, "dir" );
			pPS->GetShaderInfo().Bind( m_paramMinDot, "minDot" );
		}
	}
protected:
	virtual bool OnFlushElement( CRenderContext2D& context, CElement2D* pElement, bool bBreak ) override
	{
		IRenderSystem* pRenderSystem = context.pRenderSystem;
		CHUDCircle* pObject = (CHUDCircle*)( pElement->pInstData );
		m_paramRad.Set( pRenderSystem, &pObject->m_fRadius );
		m_paramWidth.Set( pRenderSystem, &pObject->m_fWidth );
		m_paramColor.Set( pRenderSystem, &pObject->m_color );
		m_paramColor1.Set( pRenderSystem, &pObject->m_color1 );
		if( m_paramDir.bIsBound )
		{
			float fPercent = pObject->m_fPercent * PI;
			CVector2 dir( sin( fPercent ), cos( fPercent ) );
			float fMinDot = dir.y;
			m_paramDir.Set( pRenderSystem, &dir );
			m_paramMinDot.Set( pRenderSystem, &fMinDot );
		}
		return false;
	}

	CShaderParam m_paramColor;
	CShaderParam m_paramColor1;
	CShaderParam m_paramRad;
	CShaderParam m_paramWidth;
	CShaderParam m_paramDir;
	CShaderParam m_paramMinDot;
};

CHUDCircle::CHUDCircle( float fRadius, float fWidth, const CVector4& color, const CVector4& color1, bool bPercent )
	: m_fRadius( fRadius )
	, m_fWidth( fWidth )
	, m_color( color )
	, m_color1( color1 )
	, m_fPercent( 1 )
{
	CHUDCircleDrawable* pDrawable;
	if( bPercent )
	{
		static CHUDCircleDrawable* g_pDrawable = NULL;
		if( !g_pDrawable )
		{
			vector<char> content;
			GetFileContent( content, "materials/HUD_circle_percent.xml", true );
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			g_pDrawable = new CHUDCircleDrawable;
			g_pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "gui_pass" ) );
			doc.Clear();
		}
		pDrawable = g_pDrawable;
	}
	else
	{
		static CHUDCircleDrawable* g_pDrawable = NULL;
		if( !g_pDrawable )
		{
			vector<char> content;
			GetFileContent( content, "materials/HUD_circle.xml", true );
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			g_pDrawable = new CHUDCircleDrawable;
			g_pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "gui_pass" ) );
			doc.Clear();
		}
		pDrawable = g_pDrawable;
	}

	float fSize = fRadius + fWidth;
	m_localBound = CRectangle( -fSize, -fSize, fSize * 2, fSize * 2 );
	m_element2D.pInstData = this;
	m_element2D.rect = CRectangle( -fSize, -fSize, fSize * 2, fSize * 2 );
	m_element2D.texRect = CRectangle( 0, 0, 1, 1 );
	m_element2D.SetDrawable( pDrawable );

	static float g_radMin[4] = { 45, 27, 18, 9 };
	static float g_radMax[4] = { 55, 33, 22, 11 };
	static float g_timeScaleMin[4] = { 1.5f, 2.5f, 4.0f, 6.5f };
	static float g_timeScaleMax[4] = { 1.0f, 2.0f, 3.0f, 5.0f };
	for( int i = 0; i < 4; i++ )
	{
		m_fTexRad[i] = SRand::Inst().Rand( g_radMin[i], g_radMax[i] );
		m_fTimeScale[i] = SRand::Inst().Rand( g_timeScaleMin[i], g_timeScaleMax[i] ) * ( SRand::Inst().Rand() & 1 ? 1 : -1 );
	}
}

void CHUDCircle::Render( CRenderContext2D& context )
{
	if( context.eRenderPass == eRenderPass_Color )
	{
		m_element2D.worldMat = globalTransform;

		double time = CGame::Inst().GetTotalTime();
		CVector2 ofs( 0, 0 );
		for( int i = 0; i < ELEM_COUNT( m_fTimeScale ); i++ )
		{
			double t = time * m_fTimeScale[i];
			ofs = ofs + CVector2( cos( t ), sin( t ) ) * m_fTexRad[i];
		}

		float fSize = ( m_fRadius + m_fWidth ) / 512.0f;
		m_element2D.texRect = CRectangle( 0.5f - fSize + ofs.x / 512.0f, 0.5f - fSize + ofs.y / 512.0f, fSize * 2, fSize * 2 );
		context.AddElement( &m_element2D, 1 );
	}
}