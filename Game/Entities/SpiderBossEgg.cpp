#include "stdafx.h"
#include "SpiderBossEgg.h"
#include "Stage.h"
#include "Player.h"
#include "EffectObject.h"
#include "Render/ParticleSystem.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "PlayerDebuffs/PlayerDebuffSpiderWeb.h"

CSpiderBossEgg::CSpiderBossEgg( CEntity* pOwner, const CVector2& targetPos, float fLifeTime )
	: m_pOwner( pOwner )
	, m_targetPos( targetPos )
	, m_fLifeTime( fLifeTime )
	, m_fKillTime( 1.0f )
	, m_onPlayerAttack( this, &CSpiderBossEgg::OnPlayerAttack )
{
	static CDefaultDrawable2D* pDrawable = NULL;
	static CDefaultDrawable2D* pDrawable1 = NULL;
	static CDefaultDrawable2D* pFootprintDrawable = NULL;
	if( !pDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/spiderbossegg.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
		pDrawable1 = new CDefaultDrawable2D;
		pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
		pFootprintDrawable = new CDefaultDrawable2D;
		pFootprintDrawable->LoadXml( doc.RootElement()->FirstChildElement( "footprint" ) );
		doc.Clear();
	}

	CImage2D* pImage = new CImage2D( pDrawable, pDrawable1, CRectangle( -116, -116, 232, 232 ), CRectangle( 12.0f / 256, 12.0f / 256, 1 - 12.0f / 256, 1 - 12.0f / 256 ) );
	pImage->SetInstData( &m_param, sizeof( m_param ) );
	SetRenderObject( pImage );
	AddCircle( 100, CVector2( 0, 0 ) );
	
	m_param = CVector4( 0.1f, 0.15f, 0, 0 );
	m_elemFootprint.SetDrawable( pFootprintDrawable );
	m_elemFootprint.rect = CRectangle( -116, -116, 232, 232 );
	m_elemFootprint.texRect = CRectangle( 12.0f / 256, 12.0f / 256, 1 - 12.0f / 256, 1 - 12.0f / 256 );
	m_elemFootprint.pInstData = &m_param;
	m_elemFootprint.nInstDataSize = sizeof( m_param );
}

void CSpiderBossEgg::OnAddedToStage()
{
	RegisterEntityEvent( eEntityEvent_PlayerAttack, &m_onPlayerAttack );
	CFlyingObject::OnAddedToStage();
}

void CSpiderBossEgg::OnRemovedFromStage()
{
	CFlyingObject::OnRemovedFromStage();
	if( m_onPlayerAttack.IsRegistered() )
		m_onPlayerAttack.Unregister();
	m_pOwner = NULL;

	if( m_pFootprintWeb )
	{
		m_pFootprintWeb->ToPersistentCanvas();
		GetAnimController()->StopAnim( m_pParticleInstWeb );
		m_pFootprintWeb->GetAnimController()->PlayAnim( m_pParticleInstWeb );
		m_pFootprintWeb = NULL;
		m_pParticleInstWeb = NULL;
	}
	if( m_pFootprintBlood )
	{
		m_pFootprintBlood->ToPersistentCanvas();
		GetAnimController()->StopAnim( m_pParticleInstBlood );
		m_pFootprintBlood->GetAnimController()->PlayAnim( m_pParticleInstBlood );
		m_pFootprintBlood = NULL;
		m_pParticleInstBlood = NULL;
	}
}

void CSpiderBossEgg::OnTickBeforeHitTest()
{
	float fElapsedTime = GetStage()->GetFlyingObjectElapsedTime();
	if( IsAlive() )
	{
		float fTime = m_fLifeTime;
		CVector2 dPos = GetPosition() - m_targetPos;
		m_fLifeTime -= fElapsedTime;
		if( m_fLifeTime < 0 )
			m_fLifeTime = 0;
		if( fTime > 0 )
			SetPosition( m_targetPos + dPos * ( m_fLifeTime / fTime ) );
		
		bool bKill = m_fLifeTime == 0;
		float fDist = m_fLifeTime == 0 ? 128 : 64;
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
		{
			CVector2 dPos = pPlayer->GetPosition() - GetPosition();
			if( dPos.Dot( dPos ) < fDist * fDist )
			{
				pPlayer->EndHorrorReflex();
				pPlayer->GetDebuffLayer()->Add( new CPlayerDebuffSpiderWeb( 3 ) );
				bKill = true;
			}
		}
		if( bKill )
		{
			Kill();
		}
	}
	else
	{
		m_fKillTime -= fElapsedTime;
		if( m_fKillTime <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
		
		if( m_pFootprintWeb )
		{
			m_elemFootprintWeb.worldMat = globalTransform;
			m_pFootprintWeb->AddFootprint( &m_elemFootprintWeb, 100 );
		}
		if( m_pFootprintBlood )
		{
			m_elemFootprintBlood.worldMat = globalTransform;
			m_pFootprintBlood->AddFootprint( &m_elemFootprintBlood, 100 );
		}
	}

	CFlyingObject::OnTickBeforeHitTest();
}

void CSpiderBossEgg::OnKilled()
{
	static CFootprintUpdateDrawable* pUpdateDrawable = NULL;
	static CFootprintDrawable* pRenderDrawable = NULL;
	static CFootprintUpdateDrawable* pUpdateDrawable1 = NULL;
	static CFootprintDrawable* pRenderDrawable1 = NULL;
	static CFootprintDrawable* pRenderDrawable2 = NULL;
	static CParticleSystem* pParticleSystem1 = NULL;
	static CParticleSystem* pParticleSystem2 = NULL;
	if( !pUpdateDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/footprint_spiderbossegg.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pUpdateDrawable = new CFootprintUpdateDrawable;
		pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
		pRenderDrawable = new CFootprintDrawable;
		pRenderDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render" ) );
		pUpdateDrawable1 = new CFootprintUpdateDrawable;
		pUpdateDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "update1" ) );
		pRenderDrawable1 = new CFootprintDrawable;
		pRenderDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "render1_color" ) );
		pRenderDrawable2 = new CFootprintDrawable;
		pRenderDrawable2->LoadXml( doc.RootElement()->FirstChildElement( "render2_color" ) );
		doc.Clear();
		
		GetFileContent( content, "materials/spiderbossegg_particle.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content[0] );
		pParticleSystem1 = new CParticleSystem;
		pParticleSystem1->LoadXml( doc1.RootElement()->FirstChildElement( "particle1" ) );
		pParticleSystem2 = new CParticleSystem;
		pParticleSystem2->LoadXml( doc1.RootElement()->FirstChildElement( "particle2" ) );
		doc1.Clear();
	}

	m_pFootprintBlood = new CFootprintReceiver( pUpdateDrawable1, pRenderDrawable2, NULL, false );
	GetStage()->AddFootprint( m_pFootprintBlood );
	m_pParticleInstBlood = pParticleSystem2->CreateParticleSystemInst( GetAnimController(), &m_elemFootprintBlood );

	UpdateDirty();
	m_elemFootprint.worldMat = globalTransform;
	CFootprintReceiver* pFootprint = new CFootprintReceiver( pUpdateDrawable, pRenderDrawable, NULL, false );
	pFootprint->SetFootprintRectExtension( 10 );
	GetStage()->AddFootprint( pFootprint );
	pFootprint->AddFootprint( &m_elemFootprint, 5 );
	pFootprint->SetAutoRemove( true );

	m_pFootprintWeb = new CFootprintReceiver( pUpdateDrawable1, pRenderDrawable1, NULL, false );
	GetStage()->AddFootprint( m_pFootprintWeb );
	m_pParticleInstWeb = pParticleSystem1->CreateParticleSystemInst( GetAnimController(), &m_elemFootprintWeb );

	SetRenderObject( NULL );
}

extern CParticleSystem* __getBloodEffect();
void CSpiderBossEgg::OnPlayerAttack( SPlayerAttackContext* pContext )
{
	if( !IsAlive() )
		return;
	pContext->nResult |= SPlayerAttackContext::eResult_Hit | SPlayerAttackContext::eResult_Critical;
	Kill();
	CVector2& pos = pContext->hitPos;
	CEffectObject* pObject = new CEffectObject( 1 );
	CParticleSystem* pParticleSystem = __getBloodEffect();
	pObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pObject->GetAnimController() ) );
	pObject->x = pos.x;
	pObject->y = pos.y;
	pObject->SetParentBeforeEntity( this );
}