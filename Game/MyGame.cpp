#include "stdafx.h"
#include <Windows.h>
#include "RenderSystem.h"
#include "Lighted2DRenderer.h"
#include "MyGame.h"
#include "Image2D.h"
#include "LightRendering.h"
#include "DefaultDrawable2D.h"
#include "Scene2DManager.h"
#include "FileUtil.h"
#include "xml.h"
#include "Player.h"
#include "GUI/MainUI.h"
#include "Animation.h"
#include "TextureAtlas.h"
#include "Entities/SpiderBoss.h"
#include "Entities/DizzyRegion.h"

CGame::CGame() : m_pStage( NULL )
	, m_key( 128 )
	, m_keyDown( 128 )
	, m_keyUp( 128 )
{
	m_screenResolution = CVector2( 800, 600 );
}

void CGame::Start()
{
	m_pStage = new CStage;
	{
		vector<char> content;
		GetFileContent( content, "materials/background.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		CDefaultDrawable2D* pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
		CDefaultDrawable2D* pDrawable1 = new CDefaultDrawable2D;
		pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
		CImage2D* pImage = new CImage2D( pDrawable, pDrawable1, CRectangle( -1024, -512, 2048, 1024 ), CRectangle( 0, 0, 1, 1 ) );
		CEntity* pEntity = new CEntity;
		pEntity->SetRenderObject( pImage );
		pEntity->SetParentEntity( m_pStage->GetRoot() );
		
		CDirectionalLightObject* pDirectionalLight = new CDirectionalLightObject( CVector2( -0.6f, -0.8f ), CVector3( 0.1f, 0.1f, 0.1f), 8, 256.0f );
		pEntity->AddChild( pDirectionalLight );

		CRenderObject2D* pRenderObject = new CRenderObject2D;
		pEntity->AddChild( pRenderObject );
		m_pStage->SetFootprintRoot( pRenderObject );

		vector<char> content1;
		GetFileContent( content1, "materials/orb.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content1[0] );
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc1.RootElement()->FirstChildElement( "color_pass" ) );
		pDrawable1 = new CDefaultDrawable2D;
		pDrawable1->LoadXml( doc1.RootElement()->FirstChildElement( "occlusion_pass" ) );
		pImage = new CImage2D( pDrawable, pDrawable1, CRectangle( -128, -128, 256, 256 ), CRectangle( 0, 0, 1, 1 ) );
		CDizzyRegion* pDizzyRegion = new CDizzyRegion( 0.25f, 32 );
		pDizzyRegion->SetHitType( eEntityHitType_Enemy );
		pDizzyRegion->x = 512;
		pDizzyRegion->SetRenderObject( pImage );
		pDizzyRegion->AddCircle( 128, CVector2( 0, 0 ) );
		pDizzyRegion->SetParentEntity( pEntity );

		CEntity* pEntity1 = new CEntity;
		pEntity1->SetParentEntity( pEntity );
		pEntity1->AddRect( CRectangle( -1024, -512, 2048, 52 ) );
		pEntity1 = new CEntity;
		pEntity1->SetParentEntity( pEntity );
		pEntity1->AddRect( CRectangle( -1024, -512, 57, 1024 ) );
		pEntity1 = new CEntity;
		pEntity1->SetParentEntity( pEntity );
		pEntity1->AddRect( CRectangle( 1024, 512, -2048, -52 ) );
		pEntity1 = new CEntity;
		pEntity1->SetParentEntity( pEntity );
		pEntity1->AddRect( CRectangle( 1024, 512, -57, -1024 ) );
	}

	{
		CEntity* pEntity = new CSpiderBoss;
		pEntity->y = -64;
		pEntity->SetParentEntity( m_pStage->GetRoot() );
	}

	{
		CPointLightObject* pPointLight = new CPointLightObject( CVector4( 0.1f, 0, 500, -0.05f ), CVector3( 1, 1, 1 ), 10.0f, 0.2f, 0.4f );
		CPlayer* pPlayer = new CPlayer;
		pPlayer->SetRenderObject( pPointLight );
		pPlayer->y = -128;
		pPlayer->AddCircle( 32, CVector2( 0, 0 ) );
		pPlayer->SetParentEntity( m_pStage->GetRoot() );
	}

	CMainUI::Inst()->InitResources();

	m_pStage->Start();
	CMainUI::Inst()->SetVisible( true );
}

void CGame::Stop()
{
	CMainUI::Inst()->SetVisible( false );
	m_pStage->Stop();
	m_trigger.Clear();
	delete m_pStage;
}

void CGame::Update()
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	double dLastTime = pRenderSystem->GetLastTime();
	double dTotalTime = pRenderSystem->GetTotalTime();
	m_dTotalTime = dTotalTime;
	const uint32 nFPS = 60;
	uint32 nFrames = floor( dTotalTime * nFPS ) - floor( dLastTime * nFPS );

	CPlayer* pPlayer = m_pStage->GetPlayer();
	const float invsqrt2 = sqrt( 0.5f );
	if( pPlayer )
	{
		CVector2 moveAxis;
		moveAxis.x = (int)m_key.GetBit( 'D' ) - (int)m_key.GetBit( 'A' );
		moveAxis.y = (int)m_key.GetBit( 'W' ) - (int)m_key.GetBit( 'S' );
		moveAxis.Normalize();
		pPlayer->Move( moveAxis.x, moveAxis.y );
	}

	for( int i = 0; i < nFrames; i++ )
	{
		m_trigger.UpdateTime();
		m_pStage->Update();
	}

	if( m_keyDown.GetBit( 'J' ) )
		pPlayer->EnterHorrorReflex( 0 );
	if( m_keyDown.GetBit( 'K' ) )
		pPlayer->EnterHorrorReflex( 1 );
	if( m_keyDown.GetBit( 'L' ) )
		pPlayer->EnterHorrorReflex( 2 );

	if( m_keyDown.GetBit( 'I' ) )
		pPlayer->Action();
	if( m_keyUp.GetBit( 'I' ) )
		pPlayer->StopAction();

	m_keyDown.Clear();
	m_keyUp.Clear();
}

void CGame::OnKey( uint32 nChar, bool bKeyDown, bool bAltDown )
{
	if( nChar >= 128 )
		return;
	if( nChar >= 'a' && nChar <= 'z' )
		nChar -= 'a' - 'A';
	if( bKeyDown && !m_key.GetBit( nChar ) )
		m_keyDown.SetBit( nChar, true );
	if( !bKeyDown && m_key.GetBit( nChar ) )
		m_keyUp.SetBit( nChar, true );
	m_key.SetBit( nChar, bKeyDown );
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	IRenderSystem* pRenderSystem = IRenderSystem::Inst();
	pRenderSystem->SetRenderer( new CLighted2DRenderer );
	pRenderSystem->SetGame( &CGame::Inst() );
	SDeviceCreateContext context;
	context.resolution = CVector2( 800, 600 );
	pRenderSystem->CreateDevice( context );
	pRenderSystem->Start();
	exit( 0 );
}

