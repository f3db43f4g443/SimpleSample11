#include "stdafx.h"
#include "PlayerCrosshair.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"

CPlayerCrosshair::CPlayerCrosshair()
	: m_percent( 1, 1, 1 )
{
	vector<char> content;
	GetFileContent( content, "materials/crosshair.xml", true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	LoadXml( doc.RootElement()->FirstChildElement( "gui_pass" ) );
	doc.Clear();

	IShader* pPixelShader = m_material.GetShader( EShaderType::PixelShader );
	if( pPixelShader )
	{
		auto& shaderInfo = pPixelShader->GetShaderInfo();
		shaderInfo.Bind( m_dirX, "dirX" );
		shaderInfo.Bind( m_dirY, "dirY" );
		shaderInfo.Bind( m_minDot, "minDot" );
		shaderInfo.Bind( m_radBegin, "radBegin" );
		shaderInfo.Bind( m_radEnd, "radEnd" );
	}

	m_localBound = CRectangle( -32, -32, 64, 64 );
	m_element2D.pInstData = this;
	m_element2D.rect = CRectangle( -32, -32, 64, 64 );
	m_element2D.texRect = CRectangle( 0, 0, 1, 1 );
	m_element2D.SetDrawable( this );
}

void CPlayerCrosshair::Render( CRenderContext2D& context )
{
	if( context.eRenderPass == eRenderPass_Color )
	{
		m_element2D.worldMat = globalTransform;
		context.AddElement( &m_element2D, 1 );
	}
}

void CPlayerCrosshair::OnApplyMaterial( CRenderContext2D& context )
{
	CVector3 angle = m_percent * PI;
	CVector3 dirX( sin( angle.x ), sin( angle.y ), sin( angle.z ) );
	CVector3 dirY( cos( angle.x ), cos( angle.y ), cos( angle.z ) );
	CVector3 minDot( dirY.x, dirY.y, dirY.z );
	CVector3 radBegin( 0.5f, 0.5f, 0.5f );
	CVector3 radEnd( 0.75f, 0.75f, 0.75f );

	IRenderSystem* pSystem = context.pRenderSystem;
	if( m_dirX.bIsBound )
		m_dirX.Set( pSystem, &dirX );
	if( m_dirY.bIsBound )
		m_dirY.Set( pSystem, &dirY );
	if( m_minDot.bIsBound )
		m_minDot.Set( pSystem, &minDot );
	if( m_radBegin.bIsBound )
		m_radBegin.Set( pSystem, &radBegin );
	if( m_radEnd.bIsBound )
		m_radEnd.Set( pSystem, &radEnd );
}