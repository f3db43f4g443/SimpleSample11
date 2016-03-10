#include "stdafx.h"
#include "BloodLaser.h"
#include "Render/Rope2D.h"
#include "Render/ParticleSystem.h"
#include "Render/Scene2DManager.h"
#include "Render/Canvas.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Stage.h"
#include "Player.h"
#include "MyGame.h"

class CBloodLaserDrawable : public CRopeDrawable2D
{
public:
	CBloodLaserDrawable( CParticleSystemData* pData ) : CRopeDrawable2D( pData ) {}
	static CParticleSystemDrawable* AllocFunc( CParticleSystemData* pData ) { return new CBloodLaserDrawable( pData ); }

	virtual void LoadXml( TiXmlElement* pRoot ) override
	{
		CRopeDrawable2D::LoadXml( pRoot );
		auto pMaterial = pRoot->FirstChildElement( "material" );
		IShader* pPS = m_material.GetShader( EShaderType::PixelShader );
		if( pPS )
			pPS->GetShaderInfo().Bind( m_paramTex, "Texture0" );
	}
protected:
	virtual void OnFlushElement( CRenderContext2D& context, CElement2D* pElement ) override
	{
		SRopeData* pRopeData = (SRopeData*)pElement->pInstData;
		CCanvas* pCanvas = (CCanvas*)pRopeData->pExtraData;
		m_paramTex.Set( context.pRenderSystem, pCanvas->GetTexture()->GetShaderResource() );
	}
private:
	CShaderParamShaderResource m_paramTex;
};

class CBloodLaserCanvas
{
public:
	CBloodLaserCanvas()
		: m_texCanvas( false, 512, 512, EFormat::EFormatR8G8B8A8UNorm, CCanvas::eDepthStencilType_None )
		, m_fTotalTime( 0 )
		, m_nTimeStamp( 0 )
	{
		vector<char> content;
		GetFileContent( content, "materials/bloodlaser.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pParticleSystem = new CParticleSystem;
		pParticleSystem->LoadXml( doc.RootElement()->FirstChildElement( "particle" ) );
		pParticleSystem1 = new CParticleSystem( &CBloodLaserDrawable::AllocFunc );
		pParticleSystem1->LoadXml( doc.RootElement()->FirstChildElement( "beam" ) );
		pUpdateDrawable = new CDefaultDrawable2D;
		pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
		doc.Clear();
	
		CRenderObject2D* pRoot = new CRenderObject2D;
		CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( pRoot );
		m_texCanvas.SetRoot( pRoot );
		CImage2D* pImage2D = new CImage2D( pUpdateDrawable, NULL, CRectangle( -256, -256, 512, 512 ), CRectangle( 0, 0, 1, 1 ) );
		pRoot->AddChild( pImage2D );
		for( int i = 0; i < 4; i++ )
		{
			CParticleSystemObject* pParticle = pParticleSystem->CreateParticleSystemObject( NULL );
			pParticle->x = ( i * 2 - 3 ) * 64;
			pRoot->AddChild( pParticle );
		}
	}
	~CBloodLaserCanvas()
	{
		m_texCanvas.GetRoot()->RemoveThis();
	}

	void Update( float fTime )
	{
		int32 timeStamp = CGame::Inst().GetTimeStamp();
		if( timeStamp == m_nTimeStamp )
			return;
		m_nTimeStamp = timeStamp;

		CRenderObject2D* pRoot = m_texCanvas.GetRoot();
		for( CRenderObject2D* pRenderObject = pRoot->Get_Child(); pRenderObject; pRenderObject = pRenderObject->NextChild() )
		{
			CParticleSystemObject* pParticle = dynamic_cast<CParticleSystemObject*>( pRenderObject );
			if( pParticle )
				pParticle->UpdateAnim( fTime );
		}
	}

	void Render( CRenderContext2D& context )
	{
		double totalTime = CGame::Inst().GetTotalTime();
		if( totalTime == m_fTotalTime )
			return;
		m_fTotalTime = totalTime;
		m_texCanvas.Render( context );
	}

	double m_fTotalTime;
	int32 m_nTimeStamp;
	CParticleSystem* pParticleSystem;
	CParticleSystem* pParticleSystem1;
	CDefaultDrawable2D* pUpdateDrawable;
	CCanvas m_texCanvas;

	DECLARE_GLOBAL_INST_REFERENCE( CBloodLaserCanvas );
};

CBloodLaser::CBloodLaser( const CVector2& end, float fWidth )
	: m_tickBeforeHitTest( this, &CBloodLaser::OnTickBeforeHitTest )
	, m_tickAfterHitTest( this, &CBloodLaser::OnTickAfterHitTest )
	, m_bAlive( true )
	, m_fTime( 0.5f )
	, m_fDeathTime( 1.0f )
	, m_end( end )
{
	CRopeObject2D* pRope = CBloodLaserCanvas::Inst().pParticleSystem1->CreateBeamObject( GetAnimController() );
	pRope->SetDataCount( 2 );
	float l = end.Length();
	pRope->SetData( 0, CVector2( 0, 0 ), fWidth, CVector2( 0, 0 ), CVector2( 1, 0 ) );
	pRope->SetData( 1, end, fWidth, CVector2( 0, l / 512.0f ), CVector2( 1, l / 512.0f ) );
	pRope->SetExtraData( &CBloodLaserCanvas::Inst().m_texCanvas );
	pRope->CalcLocalBound();
	SetRenderObject( pRope );

	m_width = m_end;
	m_width.Normalize();
	m_width = CVector2( -m_width.y * fWidth * 0.5f, m_width.x * fWidth * 0.5f );
}

CBloodLaser::~CBloodLaser()
{
}

void CBloodLaser::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CBloodLaser::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

void CBloodLaser::OnTickBeforeHitTest()
{
	float fTime = GetStage()->GetElapsedTimePerTick();
	CBloodLaserCanvas::Inst().Update( fTime );

	UpdateAnim( fTime );
	if( !m_bAlive )
	{
		m_fDeathTime -= fTime;
		if( m_fDeathTime <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
	}
	else
	{
		m_fTime -= fTime;
		if( m_fTime < 0 )
			m_fTime = 0;
	}

	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CBloodLaser::OnTickAfterHitTest()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	if( !m_bAlive )
		return;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer->IsInHorrorReflex() )
	{
		m_bAlive = false;
		if( GetRenderObject() )
		{
			CRopeObject2D* pRope = dynamic_cast<CRopeObject2D*>( GetRenderObject() );
			pRope->GetInstanceData()->GetData().isEmitting = false;
		}
		return;
	}

	if( m_fTime <= 0 )
	{
		SHitProxyPolygon polygon;
		polygon.nVertices = 4;
		polygon.vertices[0] = m_width;
		polygon.vertices[1] = m_width * -1;
		polygon.vertices[2] = m_width * -1 + m_end;
		polygon.vertices[3] = m_width + m_end;
		polygon.CalcNormals();
		polygon.CalcBoundGrid( globalTransform );
		if( pPlayer->GetCrosshair()->HitTest( &polygon, globalTransform, NULL ) )
		{
			SDamage dmg;
			dmg.pSource = this;
			dmg.nHp = 10;
			pPlayer->Damage( dmg );
		}
	}
}

void CBloodLaser::Render( CRenderContext2D& context )
{
	if( context.eRenderPass == eRenderPass_Color )
		CBloodLaserCanvas::Inst().Render( context );
}