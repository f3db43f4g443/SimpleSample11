#include "stdafx.h"
#include "SpiderBossBullet.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Stage.h"
#include "EnemyBullet.h"
#include "Common/MathUtil.h"
#include "Player.h"

CSpiderBossBullet::CSpiderBossBullet( CEntity* pOwner, uint32 nType )
	: m_nType( nType )
	, m_pOwner( pOwner )
	, m_fKillTimeLeft( 1.0f )
	, m_fMovedDist( 0 )
	, m_tickAfterHitTest( this, &CSpiderBossBullet::OnTickAfterHitTest )
{
	static CParticleSystem* pParticleSystem = NULL;
	if( !pParticleSystem )
	{
		pParticleSystem = new CParticleSystem;
		vector<char> content;
		GetFileContent( content, "materials/spiderbossbullet_particle.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pParticleSystem->LoadXml( doc.RootElement() );
		doc.Clear();
	}
	SetRenderObject( pParticleSystem->CreateParticleSystemObject( GetAnimController(), m_pParticle.AssignPtr() ) );
}

void CSpiderBossBullet::OnAddedToStage()
{	
	CFlyingObject::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSpiderBossBullet::OnRemovedFromStage()
{
	CFlyingObject::OnRemovedFromStage();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	m_pOwner = NULL;
}

void CSpiderBossBullet::OnTickBeforeHitTest()
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
	}
	else
	{
		switch( m_nType )
		{
		case 0:
			{
				float fSpeed = 1024;
				float fDist = fSpeed * fElapsedTime;
				SRaycastResult result;
				if( GetStage()->Raycast( GetPosition(), GetPosition() + CVector2( 0, -fDist - 16 ), eEntityHitType_WorldStatic, &result ) )
				{
					fDist = result.fDist - 16;
					SetPosition( GetPosition() + CVector2( 0, -fDist ) );
					Kill();

					CSpiderBossBullet* pBullet = new CSpiderBossBullet( m_pOwner, 1 );
					pBullet->SetPosition( GetPosition() );
					pBullet->m_dir = CVector2( -1, 0 );
					pBullet->SetRotation( PI );
					pBullet->SetParentBeforeEntity( this );
					pBullet = new CSpiderBossBullet( m_pOwner, 1 );
					pBullet->SetPosition( GetPosition() );
					pBullet->m_dir = CVector2( 1, 0 );
					pBullet->SetRotation( 0 );
					pBullet->SetParentBeforeEntity( this );
				}
				else
					SetPosition( GetPosition() + CVector2( 0, -fDist ) );
			}
			break;
		case 1:
			{
				float fSpeed = 512;
				float fDist = fSpeed * fElapsedTime;
				CVector2 pos = GetPosition();
				float fMovedDist = m_fMovedDist;
				m_fMovedDist += fDist;
				bool bKill = false;
				if( m_fMovedDist >= 512 )
				{
					fDist -= m_fMovedDist - 512;
					m_fMovedDist = 512;
					bKill = true;
				}
				
				SRaycastResult result;
				if( GetStage()->Raycast( pos, pos + m_dir * ( fDist + 16 ), eEntityHitType_WorldStatic, &result ) )
				{
					fDist = result.fDist - 16;
					bKill = true;
				}

				SetPosition( pos + m_dir * fDist );
				if( bKill )
					Kill();

				uint32 n1 = floor( fMovedDist / 64 );
				uint32 n2 = floor( m_fMovedDist / 64 );
				for( int i = n1 + 1; i <= n2; i++ )
				{
					fDist = i * 64.0f - fMovedDist;
					float fDist1 = m_fMovedDist - i * 64.0f;
					CSpiderBossBullet* pBullet = new CSpiderBossBullet( m_pOwner, 2 );
					pBullet->SetPosition( pos + m_dir * fDist + CVector2( 0, fDist1 ) );
					pBullet->SetRotation( PI * 0.5f );
					pBullet->SetParentBeforeEntity( this );
				}
			}
			break;
		default:
			{
				float fSpeed = 512;
				float fDist = fSpeed * fElapsedTime;
				m_fMovedDist += fDist;
				if( m_fMovedDist >= 512 )
				{
					fDist -= m_fMovedDist - 512;
					m_fMovedDist = 512;
					SetPosition( GetPosition() + CVector2( 0, fDist ) );
					Kill();

					/*for( int i = 0; i < 3; i++ )
					{
						float fDir = -PI * 0.125f * ( i - 1 ) + SRand::Inst().Rand( -0.3f, 0.3f );
						CEnemyBullet* pBullet = new CEnemyBullet( m_pOwner, CVector2( sin( fDir ), cos( fDir ) ) * 100, CVector2( 0, -200 ), 8, 10, 10, 0, 0, 1 );
						pBullet->SetPosition( GetPosition() );
						pBullet->SetParentBeforeEntity( this );
					}*/
				}
				else
					SetPosition( GetPosition() + CVector2( 0, fDist ) );
			}
			break;
		}
	}

	CFlyingObject::OnTickBeforeHitTest();
}

void CSpiderBossBullet::OnTickAfterHitTest()
{
	if( IsAlive() )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( !pPlayer->IsInHorrorReflex() )
		{
			Kill();
			return;
		}

		if( pPlayer && pPlayer->CanBeHit() )
		{
			CEntity* pEntity = pPlayer->GetCrosshair();

			bool bHit = false;
			SHitProxyCircle circle;
			circle.center = CVector2( 0, 0 );
			circle.fRadius = 16;
			circle.CalcBoundGrid( globalTransform );
			bHit = pEntity->HitTest( &circle, globalTransform );

			if( bHit )
			{
				SDamage dmg;
				dmg.pSource = this;
				dmg.nHp = 10;
				dmg.nMp = 0;
				dmg.nSp = 0;
				pPlayer->Damage( dmg );
				Kill();
				return;
			}
		}
	}
	
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSpiderBossBullet::OnKilled()
{
	if( m_pParticle )
		m_pParticle->GetData().isEmitting = false;
}