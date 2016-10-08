#include "stdafx.h"
#include "SpiderWebGround.h"
#include "Stage.h"
#include "Player.h"
#include "Effects/ParticleWebEmitter.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"

CSpiderWebGround::CSpiderWebGround( uint8 nType, float fDir )
	: m_nType( nType )
	, m_fDir( fDir )
	, m_fLife( 0 )
	, m_fMaxLife( 10 )
	, m_fDeathTime( 1.0f )
	, m_tickAfterHitTest( this, &CSpiderWebGround::OnTickAfterHitTest )
{
	if( m_nType == 0 )
	{
		m_nPolygonCount = 10;
		for( int i = 0; i < 10; i++ )
		{
			auto& polygon = m_polygons[i];
			polygon.nVertices = 3;
			polygon.vertices[0] = CVector2( 0, 0 );
			polygon.vertices[1] = CVector2( cos( PI * 0.2f * i + m_fDir ) * 192, sin( PI * 0.2f * i + m_fDir ) * 192 );
			polygon.vertices[2] = CVector2( cos( PI * 0.2f * ( i + 1 ) + m_fDir ) * 192, sin( PI * 0.2f * ( i + 1 ) + m_fDir ) * 192 );
			polygon.CalcNormals();
		}
	}
	else if( m_nType == 1 )
	{
		m_nPolygonCount = 3;
		for( int i = 0; i < 3; i++ )
		{
			auto& polygon = m_polygons[i];
			polygon.nVertices = 3;
			polygon.vertices[0] = CVector2( 0, 0 );
			polygon.vertices[1] = CVector2( cos( PI * 0.2f * ( i - 1.5f ) + m_fDir ) * 288, sin( PI * 0.2f * ( i - 1.5f ) + m_fDir ) * 288 );
			polygon.vertices[2] = CVector2( cos( PI * 0.2f * ( i - 0.5f ) + m_fDir ) * 288, sin( PI * 0.2f * ( i - 0.5f ) + m_fDir ) * 288 );
			polygon.CalcNormals();
		}
	}
	else
	{
		m_nPolygonCount = 1;
		auto& polygon = m_polygons[0];
		polygon.nVertices = 3;
		polygon.vertices[0] = CVector2( 0, 0 );
		polygon.vertices[1] = CVector2( cos( PI * 0.2f * -0.5f + m_fDir ) * 512, sin( PI * 0.2f * -0.5f + m_fDir ) * 512 );
		polygon.vertices[2] = CVector2( cos( PI * 0.2f * 0.5f + m_fDir ) * 512, sin( PI * 0.2f * 0.5f + m_fDir ) * 512 );
		polygon.CalcNormals();
	}
}

CFootprintUpdateDrawable* CSpiderWebGround::s_pUpdateDrawable = NULL;
void CSpiderWebGround::OnAddedToStage()
{
	CFlyingObject::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );

	static CFootprintUpdateDrawable* pUpdateDrawable = NULL;
	static CFootprintDrawable* pRenderDrawable = NULL;
	static CParticleSystem* pParticleSystem[3] = { NULL };
	if( !pUpdateDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/footprint_spiderwebground.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pUpdateDrawable = new CFootprintUpdateDrawable;
		pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
		s_pUpdateDrawable = new CFootprintUpdateDrawable;
		s_pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update1" ) );
		pRenderDrawable = new CFootprintDrawable;
		pRenderDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render" ) );
		doc.Clear();
		
		GetFileContent( content, "materials/spiderwebground_particle.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content[0] );
		pParticleSystem[0] = new CParticleSystem;
		pParticleSystem[0]->LoadXml( doc1.RootElement()->FirstChildElement( "particle1" ) );
		pParticleSystem[1] = new CParticleSystem;
		pParticleSystem[1]->LoadXml( doc1.RootElement()->FirstChildElement( "particle2" ) );
		pParticleSystem[2] = new CParticleSystem;
		pParticleSystem[2]->LoadXml( doc1.RootElement()->FirstChildElement( "particle3" ) );
		doc1.Clear();
	}
	CFootprintReceiver* pFootprintReceiver = new CFootprintReceiver( pUpdateDrawable, pRenderDrawable, NULL );
	m_pParticleInst = pParticleSystem[m_nType]->CreateParticleSystemInst( GetAnimController(), &m_elemParticle );
	m_elemParticle.worldMat.Identity();
	GetStage()->AddFootprint( pFootprintReceiver );
	m_pFootprint = pFootprintReceiver;

	CParticleSubEmitter* pEmitter;
	CRectangle rect( 0, 0, 0, 0 );
	if( m_nType == 0 )
	{
		CParticleSubEmitter::SSubEmitter emitters[10];
		for( int i = 0; i < 10; i++ )
		{
			auto& emitter = emitters[i];
			emitter.size = CVector2( 1, 0 );
			emitter.s0 = CVector2( 0, 0 );
			CVector2 dir( cos( PI * 0.2f * i + m_fDir ), sin( PI * 0.2f * i + m_fDir ) );
			emitter.v = dir * 180;
			rect = rect + CRectangle( dir.x * 190, dir.y * 190, 0, 0 );
			emitter.a = CVector2( 0, 0 );
		}
		pEmitter = new CParticleWebEmitter( ELEM_COUNT( emitters ), 1, 10, 1, true, emitters );
	}
	else if( m_nType == 1 )
	{
		CParticleSubEmitter::SSubEmitter emitters[4];
		for( int i = 0; i < 4; i++ )
		{
			auto& emitter = emitters[i];
			emitter.size = CVector2( 1, 0 );
			emitter.s0 = CVector2( 0, 0 );
			CVector2 dir( cos( PI * 0.2f * ( i - 1.5f ) + m_fDir ), sin( PI * 0.2f * ( i - 1.5f ) + m_fDir ) );
			emitter.v = dir * 265;
			rect = rect + CRectangle( dir.x * 275, dir.y * 275, 0, 0 );
			emitter.a = CVector2( 0, 0 );
		}
		pEmitter = new CParticleWebEmitter( ELEM_COUNT( emitters ), 1, 10, 1, false, emitters );
	}
	else
	{
		CParticleSubEmitter::SSubEmitter emitters[2];
		for( int i = 0; i < 2; i++ )
		{
			auto& emitter = emitters[i];
			emitter.size = CVector2( 1, 0 );
			emitter.s0 = CVector2( 0, 0 );
			CVector2 dir( cos( PI * 0.2f * ( i - 0.5f ) + m_fDir ), sin( PI * 0.2f * ( i - 0.5f ) + m_fDir ) );
			emitter.v = dir * 460;
			rect = rect + CRectangle( dir.x * 480, dir.y * 480, 0, 0 );
			emitter.a = CVector2( 0, 0 );
		}
		pEmitter = new CParticleWebEmitter( ELEM_COUNT( emitters ), 1, 10, 1, false, emitters );
	}
	m_pParticleInst->SetEmitter( pEmitter );
	rect.x += x - 12;
	rect.y += y - 12;
	rect.width += 24;
	rect.height += 24;
	pFootprintReceiver->SetReservedBound( rect );
}

void CSpiderWebGround::OnRemovedFromStage()
{
	CFlyingObject::OnRemovedFromStage();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();

	if( m_pFootprint )
	{
		m_pFootprint->SetMgr( NULL );
		m_pFootprint->RemoveThis();
		m_pFootprint = NULL;
		GetAnimController()->StopAnim( m_pParticleInst );
		m_pParticleInst = NULL;
	}
}

void CSpiderWebGround::OnTickBeforeHitTest()
{
	CFlyingObject::OnTickBeforeHitTest();

	float fTime = GetStage()->GetFlyingObjectElapsedTime();
	if( IsAlive() )
	{
		m_fLife += fTime;
		if( m_fLife >= m_fMaxLife )
		{
			Kill();
			return;
		}

		if( m_pFootprint )
		{
			m_elemParticle.worldMat = globalTransform;
			m_pFootprint->AddFootprint( &m_elemParticle, 100 );
		}
	}
	else
	{
		m_fDeathTime -= fTime;
		if( m_fDeathTime < 0 )
			SetParentEntity( NULL );
	}
}

void CSpiderWebGround::OnTickAfterHitTest()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );

	if( IsAlive() )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( !pPlayer )
			return;
		CEntity* pEntity;
		if( pPlayer->IsInHorrorReflex() )
			pEntity = pPlayer->GetCrosshair();
		else
			pEntity = pPlayer;

		float fRadius;
		uint32 nCount;
		SHitProxyPolygon polygons[9];

		float fHit = 0;
		for( int i = 0; i < m_nPolygonCount; i++ )
		{
			auto polygon = m_polygons[i];
			for( int i = 0; i < polygon.nVertices; i++ )
			{
				polygon.vertices[i] = polygon.vertices[i] * Min( m_fLife, 1.0f );
			}
			polygon.CalcBoundGrid( globalTransform );
			SHitTestResult result;
			bool bHit = pEntity->HitTest( &polygon, globalTransform, &result );
			if( bHit )
				fHit = Max( fHit, Min( result.normal.Length() * ( 1.0f / 32 ), 1.0f ) );
		}
		fHit *= 0.25f;

		if( fHit > 0 )
			m_playerDizzy.AddPlayerDizzy( pPlayer, fHit );
	}
}

void CSpiderWebGround::OnKilled()
{
	if( m_pFootprint )
	{
		m_pFootprint->SetUpdateDrawable( s_pUpdateDrawable );
	}
}