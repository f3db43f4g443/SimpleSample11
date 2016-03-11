#include "stdafx.h"
#include "EnemyBullet.h"
#include "Stage.h"
#include "Player.h"
#include "xml.h"
#include "FileUtil.h"

CParticleSystem* __getBloodEffect()
{
	static CParticleSystem* pParticleSystem = NULL;
	if( !pParticleSystem )
	{
		pParticleSystem = new CParticleSystem;
		vector<char> content;
		GetFileContent( content, "materials/enemybullet_particle1.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pParticleSystem->LoadXml( doc.RootElement() );
		doc.Clear();
	}
	return pParticleSystem;
}

CEnemyBullet::CEnemyBullet( CEntity* pOwner, CVector2 velocity, CVector2 acceleration, float fSize, float fLife, int32 nDmgHp, int32 nDmgMp, int32 nDmgSp, float fKillTime )
	: m_tickAfterHitTest( this, &CEnemyBullet::OnTickAfterHitTest )
	, m_pOwner( pOwner )
	, m_velocity( velocity )
	, m_acceleration( acceleration )
	, m_fLifeLeft( fLife )
	, m_fMaxKillTime( fKillTime )
	, m_fKillTimeLeft( fKillTime )
	, m_nDmgHp( nDmgHp )
	, m_nDmgMp( nDmgMp )
	, m_nDmgSp( nDmgSp )
{
	static CParticleSystem* pParticleSystem = NULL;
	if( !pParticleSystem )
	{
		pParticleSystem = new CParticleSystem;
		vector<char> content;
		GetFileContent( content, "materials/enemybullet_particle.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pParticleSystem->LoadXml( doc.RootElement() );
		doc.Clear();
	}
	SetRenderObject( pParticleSystem->CreateParticleSystemObject( GetAnimController(), m_pParticle.AssignPtr() ) );
	AddCircle( fSize, CVector2( 0, 0 ) );
	SetBulletMode( true );
	
	CPointLightObject* pPointLight = new CPointLightObject( CVector4( 1.5f, 0, 1000, -0.25f ), CVector3( 1, 0, 0 ), 10.0f, 0.05f, 0.4f );
	AddChild( pPointLight );
	m_pLight = pPointLight;
}

void CEnemyBullet::OnAddedToStage()
{
	CFlyingObject::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CEnemyBullet::OnRemovedFromStage()
{
	CFlyingObject::OnRemovedFromStage();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	m_pOwner = NULL;
}

void CEnemyBullet::OnTickBeforeHitTest()
{
	float fElapsedTime = GetStage()->GetFlyingObjectElapsedTime();

	if( !IsAlive() )
	{
		m_fKillTimeLeft -= fElapsedTime;
		if( m_fKillTimeLeft <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
		m_pLight->baseColor.x = m_fKillTimeLeft / m_fMaxKillTime;
	}
	else
	{
		m_fLifeLeft -= fElapsedTime;
		if( m_fLifeLeft <= 0 )
		{
			Kill();
		}
		else
		{
			CVector2 velocity = m_velocity;
			m_velocity = m_velocity + m_acceleration * fElapsedTime;
			SetPosition( GetPosition() + ( velocity + m_velocity ) * ( 0.5f * fElapsedTime ) );
		}
	}

	CFlyingObject::OnTickBeforeHitTest();
}

void CEnemyBullet::OnTickAfterHitTest()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );

	if( IsAlive() )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( !pPlayer->IsInHorrorReflex() )
		{
			Kill();
			return;
		}

		for( auto pManifold = m_pManifolds; pManifold; pManifold = pManifold->NextManifold() )
		{
			CEntity* pEntity = dynamic_cast<CEntity*>( pManifold->pOtherHitProxy );
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				CParticleSystem* pParticleSystem = __getBloodEffect();
				AddChild( pParticleSystem->CreateParticleSystemObject( GetAnimController() ) );
				Kill();
				return;
			}

			if( pEntity && pPlayer && pPlayer->CanBeHit() && pEntity == pPlayer->GetCrosshair() )
			{
				OnHitPlayer( pPlayer );
				Kill();
				return;
			}
		}
	}
}

void CEnemyBullet::OnKilled()
{
	if( m_pParticle )
		m_pParticle->GetData().isEmitting = false;
}

void CEnemyBullet::OnHitPlayer( CPlayer* pPlayer )
{
	SDamage dmg;
	dmg.pSource = this;
	dmg.nHp = m_nDmgHp;
	dmg.nMp = m_nDmgMp;
	dmg.nSp = m_nDmgSp;
	pPlayer->Damage( dmg );
}