#include "stdafx.h"
#include "BloodLaser.h"
#include "Render/Rope2D.h"
#include "Render/Scene2DManager.h"
#include "Stage.h"
#include "Player.h"
#include "MyGame.h"
#include "Effects/DynamicTextures.h"

CBloodLaser::CBloodLaser( const CVector2& end, float fWidth, float fDuration, float fRotSpeed, CEntity* pOwner )
	: m_tickBeforeHitTest( this, &CBloodLaser::OnTickBeforeHitTest )
	, m_tickAfterHitTest( this, &CBloodLaser::OnTickAfterHitTest )
	, m_pOwner( pOwner )
	, m_bAlive( true )
	, m_fTime( 0.5f )
	, m_fDuration( fDuration )
	, m_fRotSpeed( fRotSpeed )
	, m_fDeathTime( 1.0f )
	, m_end( end )
{
	CRopeObject2D* pRope = CBloodLaserCanvas::Inst().pParticleSystem1->CreateBeamObject( GetAnimController() );
	pRope->SetDataCount( 2 );
	float l = end.Length();
	pRope->SetData( 0, CVector2( 0, 0 ), fWidth, CVector2( 0, 0 ), CVector2( 1, 0 ) );
	pRope->SetData( 1, end, fWidth, CVector2( 0, l / 512.0f ), CVector2( 1, l / 512.0f ) );
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
	m_pOwner = NULL;
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
		{
			if( m_fRotSpeed )
			{
				SetRotation( GetRotation() + m_fRotSpeed * -m_fTime );
			}
			if( m_fDuration > 0 )
			{
				m_fDuration += m_fTime;
				if( m_fDuration < 0 )
				{
					m_bAlive = false;
					if( GetRenderObject() )
					{
						CRopeObject2D* pRope = dynamic_cast<CRopeObject2D*>( GetRenderObject() );
						pRope->GetInstanceData()->GetData().isEmitting = false;
					}
				}
			}
			m_fTime = 0;
		}
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
			dmg.pSource = m_pOwner ? m_pOwner : this;
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