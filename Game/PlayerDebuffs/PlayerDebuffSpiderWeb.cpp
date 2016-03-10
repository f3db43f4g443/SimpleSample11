#include "stdafx.h"
#include "PlayerDebuffSpiderWeb.h"
#include "Common/MathUtil.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Render/Footprint.h"
#include "Effects/ParticleSubEmitter.h"
#include "Stage.h"
#include "Player.h"
#include "MyGame.h"

CPlayerDebuffSpiderWebObject::CPlayerDebuffSpiderWebObject()
	: m_param( 1, -1, 0, 0 )
	, m_fFade( 1 )
{
}

CFootprintUpdateDrawable* CPlayerDebuffSpiderWebObject::s_pUpdateDrawable = NULL;
void CPlayerDebuffSpiderWebObject::OnAddedToStage()
{
	static CFootprintUpdateDrawable* pUpdateDrawable = NULL;
	static CFootprintDrawable* pRenderDrawable = NULL;
	static CParticleSystem* pParticleSystem = NULL;
	if( !pUpdateDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/footprint_spiderwebobject.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pUpdateDrawable = new CFootprintUpdateDrawable;
		pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
		s_pUpdateDrawable = new CFootprintUpdateDrawable;
		s_pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update1" ) );
		pRenderDrawable = new CFootprintDrawable;
		pRenderDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render" ) );
		doc.Clear();
		
		GetFileContent( content, "materials/spiderwebobject_particle.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content[0] );
		pParticleSystem = new CParticleSystem;
		pParticleSystem->LoadXml( doc1.RootElement() );
		doc1.Clear();
	}

	CFootprintReceiver* pFootprintReceiver = new CFootprintReceiver( pUpdateDrawable, pRenderDrawable, NULL, true );
	pFootprintReceiver->SetMgr( GetStage()->GetFootprintMgr() );
	m_pParticleInst = pParticleSystem->CreateParticleSystemInst( GetAnimController(), &m_elemParticle );
	m_elemParticle.worldMat.Identity();
	SetRenderObject( pFootprintReceiver );
	m_pParticleInst->SetEmitter( new CParticleSubEmitter( 8, 2, 0, PI * 2, CVector2( 1, 0 ), CVector2( 1, 0 ),
		CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 48, 0 ), CVector2( 48, 0 ), CVector2( 0, -8 ), CVector2( 0, 8 ), true ) );

	CFlyingObject::OnAddedToStage();
}

void CPlayerDebuffSpiderWebObject::OnRemovedFromStage()
{
	CFlyingObject::OnRemovedFromStage();

	if( GetRenderObject() )
	{
		CFootprintReceiver* pReceiver = dynamic_cast<CFootprintReceiver*>( GetRenderObject() );
		pReceiver->SetMgr( NULL );
		GetAnimController()->StopAnim( m_pParticleInst );
		SetRenderObject( NULL );
		m_pParticleInst = NULL;
	}
}

void CPlayerDebuffSpiderWebObject::CreateBullets()
{
	static CParticleSystem* pParticleSystem = NULL;
	if( !pParticleSystem )
	{
		pParticleSystem = new CParticleSystem;
		vector<char> content;
		GetFileContent( content, "materials/spiderwebsilk_particle.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pParticleSystem->LoadXml( doc.RootElement() );
		doc.Clear();
	}

	float fBaseAngle = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < ELEM_COUNT( m_pBullet ); i++ )
	{
		if( m_pBullet[i] )
			continue;

		m_pBullet[i] = pParticleSystem->CreateBeamObject( GetAnimController() );
		m_pBullet[i]->SetZOrder( -1 );
		AddChild( m_pBullet[i] );
		float fAngle = SRand::Inst().Rand( PI * i * 2 / 3, PI * ( i + 1 ) * 2 / 3 ) + fBaseAngle;
		m_bulletDir[i] = CVector2( cos( fAngle ), sin( fAngle ) );
		m_bulletLen[i] = 1;
		UpdateBulletDisplay( i );
	}
}

void CPlayerDebuffSpiderWebObject::DestroyBullets()
{
	for( int i = 0; i < ELEM_COUNT( m_pBullet ); i++ )
	{
		if( m_pBullet[i] )
		{
			m_pBullet[i]->RemoveThis();
			m_pBullet[i] = NULL;
		}
	}
}

void CPlayerDebuffSpiderWebObject::OnTickBeforeHitTest()
{
	CFlyingObject::OnTickBeforeHitTest();
	float fTime = GetStage()->GetFlyingObjectElapsedTime();
	if( IsAlive() )
	{
		m_fFade -= fTime * 0.5f;
		if( m_fFade < 0 )
			m_fFade = 0;
	}
	else
	{
		m_fFade += fTime * 0.5f;
		if( m_fFade > 1 )
		{
			SetParentEntity( NULL );
			return;
		}
	}
	m_param.x = ( 1 - m_fFade ) * 4;
	m_param.y = m_param.x * -m_fFade;

	if( GetRenderObject() )
	{
		CFootprintReceiver* pReceiver = dynamic_cast<CFootprintReceiver*>( GetRenderObject() );
		pReceiver->AddFootprint( &m_elemParticle, 100 );
	}
	
	for( int i = 0; i < ELEM_COUNT( m_pBullet ); i++ )
	{
		if( !m_pBullet[i] )
			continue;

		m_bulletLen[i] += fTime * 200;
		if( m_bulletLen[i] > 1200 )
			m_bulletLen[i] = 1200;
		UpdateBulletDisplay( i );
	}
}

void CPlayerDebuffSpiderWebObject::OnKilled()
{
	if( GetRenderObject() )
	{
		CFootprintReceiver* pReceiver = dynamic_cast<CFootprintReceiver*>( GetRenderObject() );
		pReceiver->SetUpdateDrawable( s_pUpdateDrawable );
	}
}

float CPlayerDebuffSpiderWebObject::CheckHitPlayer( CPlayer* pPlayer )
{
	for( int i = 0; i < ELEM_COUNT( m_pBullet ); i++ )
	{
		if( !m_pBullet[i] )
			continue;
		
		CVector2 vec1( -m_bulletDir[i].y * 16, m_bulletDir[i].x * 16 );
		CVector2 vec2 = m_bulletDir[i] * m_bulletLen[i];
		SHitProxyPolygon polygon;
		polygon.nVertices = 4;
		polygon.vertices[0] = vec1;
		polygon.vertices[1] = vec1 * -1;
		polygon.vertices[2] = vec1 * -1 + vec2;
		polygon.vertices[3] = vec1 + vec2;
		polygon.CalcNormals();
		polygon.CalcBoundGrid( globalTransform );
		SHitTestResult result;
		bool bHit = pPlayer->GetCrosshair()->HitTest( &polygon, globalTransform, &result );
		if( bHit )
		{
			return Min( result.normal.Length() * 0.125f, 1.0f ) * 0.75f;
		}
	}
	return 0;
}

void CPlayerDebuffSpiderWebObject::UpdateBulletDisplay( uint32 i )
{
	CRopeObject2D* pBullet = m_pBullet[i];
	CVector2 dir = m_bulletDir[i];
	float l = m_bulletLen[i] + 16;
	pBullet->SetDataCount( 2 );
	pBullet->SetSegmentsPerData( l / 8 );
	pBullet->SetData( 0, CVector2( 0, 0 ), 16, CVector2( 0, 0 ), CVector2( 1, 0 ) );
	pBullet->SetData( 1, dir * l, 16, CVector2( 0, l ), CVector2( 1, l ) );
	pBullet->CalcLocalBound();
}

CPlayerDebuffSpiderWeb::CPlayerDebuffSpiderWeb( uint32 nObjectCount )
	: CPlayerDebuff( ePlayerDebuffID_SpiderWeb, true )
	, m_nObjectCount( nObjectCount )
{
}

void CPlayerDebuffSpiderWeb::OnAdded( CPlayerDebuff* pAddedDebuff )
{
	uint32 nAdd = 0;
	if( pAddedDebuff )
	{
		CPlayerDebuffSpiderWeb* pDebuff = dynamic_cast<CPlayerDebuffSpiderWeb*>( pAddedDebuff );
		if( !pDebuff )
			return;

		nAdd = pDebuff->m_nObjectCount;
		m_nObjectCount += pDebuff->m_nObjectCount;
	}
	else
	{
		nAdd = m_nObjectCount;
		m_pObjects.resize( 12 );
	}
	if( m_nObjectCount >= 12 )
	{
		nAdd -= m_nObjectCount - 12;
		m_nObjectCount = 12;
	}

	uint32 nIndex[12];
	for( int i = 0; i < ELEM_COUNT( nIndex ); i++ )
		nIndex[i] = i;
	SRand::Inst().Shuffle( nIndex, ELEM_COUNT( nIndex ) );

	for( int i = 0; i < 12 && nAdd; i++ )
	{
		uint32 i1 = nIndex[i];
		auto& pObject = m_pObjects[i1];
		if( pObject )
			continue;
		pObject = new CPlayerDebuffSpiderWebObject;

		float angle, radius;
		if( i1 < 4 )
		{
			angle = SRand::Inst().Rand( i1 * PI / 2, ( i1 + 1 ) * PI / 2 );
			radius = SRand::Inst().Rand( 50.0f, 150.0f );
		}
		else
		{
			angle = SRand::Inst().Rand( ( i1 - 4 ) * PI / 4, ( i1 - 3 ) * PI / 4 );
			radius = SRand::Inst().Rand( 150.0f, 300.0f );
		}
		pObject->SetPosition( CVector2( radius * cos( angle ), radius * sin( angle ) ) );
		pObject->SetParentEntity( GetDebuffLayer() );
		nAdd--;
	}
}

void CPlayerDebuffSpiderWeb::OnRemoved()
{
	for( int i = 0; i < m_pObjects.size(); i++ )
	{
		if( m_pObjects[i] )
		{
			m_pObjects[i]->Kill();
			m_pObjects[i] = NULL;
		}
	}
}

void CPlayerDebuffSpiderWeb::OnEnterHorrorReflex()
{
	for( int i = 0; i < m_pObjects.size(); i++ )
	{
		if( m_pObjects[i] )
			m_pObjects[i]->CreateBullets();
	}
}

void CPlayerDebuffSpiderWeb::OnEndHorrorReflex( float fSpRecover )
{
	for( int i = 0; i < m_pObjects.size(); i++ )
	{
		if( m_pObjects[i] )
			m_pObjects[i]->DestroyBullets();
	}

	if( fSpRecover >= 1 )
		GetDebuffLayer()->Remove( this );
}

bool CPlayerDebuffSpiderWeb::UpdateAfterHitTest()
{
	float fHit;
	CPlayer* pPlayer = GetDebuffLayer()->GetPlayer();
	if( pPlayer->IsInHorrorReflex() )
	{
		fHit = 0;
		for( int i = 0; i < m_pObjects.size(); i++ )
		{
			if( m_pObjects[i] )
				fHit = Max( fHit, m_pObjects[i]->CheckHitPlayer( pPlayer ) );
		}
	}
	else
		fHit = 0.25f;

	if( fHit > 0 )
		m_playerDizzy.AddPlayerDizzy( pPlayer, fHit );

	return true;
}