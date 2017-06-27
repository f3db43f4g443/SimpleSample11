#include "stdafx.h"
#include "DynamicTextures.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Render/Image2D.h"

//CBloodSplashCanvas::CBloodSplashCanvas()
//	: CDynamicTexture( 256, 256, EFormat::EFormatR8G8B8A8UNorm )
//{
//	vector<char> content;
//	GetFileContent( content, "materials/enemybullet_particle1.xml", true );
//	TiXmlDocument doc;
//	doc.LoadFromBuffer( &content[0] );
//	pParticleSystem = new CParticleSystem;
//	pParticleSystem->LoadXml( doc.RootElement()->FirstChildElement( "particle" ) );
//	pParticleSystem1 = new CParticleSystem();
//	pParticleSystem1->LoadXml( doc.RootElement()->FirstChildElement( "particle1" ) );
//	pParticleSystem1->BindShaderResource( EShaderType::PixelShader, "Texture0", this );
//	pUpdateDrawable = new CDefaultDrawable2D;
//	pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
//	doc.Clear();
//	
//	CRenderObject2D* pRoot = m_texCanvas.GetRoot();
//	CImage2D* pImage2D = new CImage2D( pUpdateDrawable, NULL, CRectangle( -128, -128, 256, 256 ), CRectangle( 0, 0, 1, 1 ) );
//	pRoot->AddChild( pImage2D );
//	for( int i = 0; i < 4; i++ )
//	{
//		CParticleSystemObject* pParticle = pParticleSystem->CreateParticleSystemObject( NULL );
//		pParticle->y = ( i * 2 - 3 ) * 32;
//		pRoot->AddChild( pParticle );
//	}
//}
//
//CBloodLaserCanvas::CBloodLaserCanvas()
//	: CDynamicTexture( 512, 512, EFormat::EFormatR8G8B8A8UNorm )
//{
//	vector<char> content;
//	GetFileContent( content, "materials/bloodlaser.xml", true );
//	TiXmlDocument doc;
//	doc.LoadFromBuffer( &content[0] );
//	pParticleSystem = new CParticleSystem;
//	pParticleSystem->LoadXml( doc.RootElement()->FirstChildElement( "particle" ) );
//	pParticleSystem1 = new CParticleSystem();
//	pParticleSystem1->LoadXml( doc.RootElement()->FirstChildElement( "particle1" ) );
//	pParticleSystem1->BindShaderResource( EShaderType::PixelShader, "Texture0", this );
//	pUpdateDrawable = new CDefaultDrawable2D;
//	pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
//	doc.Clear();
//	
//	CRenderObject2D* pRoot = m_texCanvas.GetRoot();
//	CImage2D* pImage2D = new CImage2D( pUpdateDrawable, NULL, CRectangle( -256, -256, 512, 512 ), CRectangle( 0, 0, 1, 1 ) );
//	pRoot->AddChild( pImage2D );
//	for( int i = 0; i < 4; i++ )
//	{
//		CParticleSystemObject* pParticle = pParticleSystem->CreateParticleSystemObject( NULL );
//		pParticle->x = ( i * 2 - 3 ) * 64;
//		pRoot->AddChild( pParticle );
//	}
//}