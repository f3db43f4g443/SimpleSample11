#include "stdafx.h"
#include "EffectObject.h"
#include "Stage.h"
#include "Render/Canvas.h"

CEffectObject::CEffectObject( float fTime, CDynamicTexture* pTexture, uint8 nType )
	: m_tickBeforeHitTest( this, &CEffectObject::OnTickBeforeHitTest )
	, m_fTimeLeft( fTime )
	, m_nType( nType )
	, m_pUpdateTexture( pTexture )
{

}

CEffectObject::CEffectObject( const SClassCreateContext& context )
	: CEntity( context )
	, m_tickBeforeHitTest( this, &CEffectObject::OnTickBeforeHitTest )
	, m_pUpdateTexture( NULL )
{

}

void CEffectObject::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
}

void CEffectObject::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
}

void CEffectObject::OnTickBeforeHitTest()
{
	float fElapsedTime;
	switch( m_nType )
	{
	case eType_Character:
		fElapsedTime = GetStage()->GetGlobalElapsedTime();
		break;
	case eType_FlyingObject:
		fElapsedTime = GetStage()->GetFlyingObjectElapsedTime();
		break;
	default:
		fElapsedTime = GetStage()->GetElapsedTimePerTick();
		break;
	}
	if( m_fTimeLeft > 0 )
	{
		m_fTimeLeft -= fElapsedTime;
		if( m_fTimeLeft <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
	}

	UpdateAnim( fElapsedTime );
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );

	if( m_pUpdateTexture )
		m_pUpdateTexture->Update( fElapsedTime );
}

void CEffectObject::Render( CRenderContext2D& context )
{
	if( context.eRenderPass == eRenderPass_Color )
	{
		if( m_pUpdateTexture )
			m_pUpdateTexture->Render( context );
	}
}