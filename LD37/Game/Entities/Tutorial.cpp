#include "stdafx.h"
#include "Tutorial.h"
#include "Stage.h"
#include "Player.h"
#include "MyLevel.h"

void CTutorialStaticBeam::OnAddedToStage()
{
	CLightning::OnAddedToStage();
	Set( NULL, NULL, m_beginOfs, m_endOfs, -1, -1 );
}

void CTutorialScreen::OnAddedToStage()
{
	CChunkObject::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 0, 4, 4 );
}

void CTutorialScreen::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CChunkObject::OnRemovedFromStage();
}

void CTutorialScreen::OnTick()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;

	CVector2 pos = pPlayer->GetPosition() / CMyLevel::GetBlockSize();

	switch( m_nState )
	{
	case 0:
		if( CRectangle( 6, 4, 4, 2 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 4, 8, 4 );
			m_nState = 1;
		}
		break;
	case 1:
		if( CRectangle( 1, 4, 3, 2 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 8, 12, 4 );
			m_nState = 2;
		}
		break;
	case 2:
		if( CRectangle( 1, 12, 2, 4 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 12, 16, 4 );
			m_nState = 3;
		}
		break;
	case 3:
		if( CRectangle( 25, 1, 6, 6 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 16, 17, 0 );
			m_nState = 4;
		}
		break;
	case 4:
		if( CRectangle( 8, 10, 16, 10 ).Contains( pos ) )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 17, 18, 0 );
			m_nState = 5;
		}
		break;
	case 5:
		if( pPlayer->GetWeapon() )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 18, 22, 8 );
			m_nState = 6;
		}
		break;
	case 6:
		if( CMyLevel::GetInst()->GetShakeStrength() >= 2.0f )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 22, 32, 10 );
			m_nState = 7;
		}
		break;
	case 7:
		if( CMyLevel::GetInst()->GetShakeStrength() <= 1.0f )
		{
			static_cast<CMultiFrameImage2D*>( m_pTips.GetPtr() )->SetFrames( 22, 23, 0 );
			m_nState = 6;
		}
		break;
	}

	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CTutorialChest::OnAddedToStage()
{
	CChunkObject::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, 1, 0 );
	static_cast<CMultiFrameImage2D*>( m_e.GetPtr() )->SetFrames( 0, 1, 0 );
	m_pPickUp->SetParentEntity( NULL );
}

void CTutorialChest::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( m_onPickUp.IsRegistered() )
		m_onPickUp.Unregister();
	CChunkObject::OnRemovedFromStage();
}

void CTutorialChest::OnKilled()
{
	if( m_pEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_pEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}

void CTutorialChest::OnTick()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;

	if( m_nState == 0 )
	{
		CRectangle rect( m_pChunk->pos.x, m_pChunk->pos.y, m_pChunk->nWidth * CMyLevel::GetInst()->GetBlockSize(), m_pChunk->nHeight * CMyLevel::GetInst()->GetBlockSize() );
		if( rect.Contains( pPlayer->GetPosition() ) )
		{
			static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 1, 4, 4 );
			static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetPlaySpeed( 1, false );
			static_cast<CMultiFrameImage2D*>( m_e.GetPtr() )->SetFrames( 1, 4, 4 );
			static_cast<CMultiFrameImage2D*>( m_e.GetPtr() )->SetPlaySpeed( 1, false );
			m_nState = 1;
			GetStage()->RegisterAfterHitTest( 30, &m_onTick );
		}
		else
			GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	}
	else if( m_nState == 1 )
	{
		if( !m_pPickUp )
			return;
		if( !m_pPickUp->GetParentEntity() )
		{
			m_pPickUp->SetParentEntity( this );
			m_pPickUp->RegisterPickupEvent( &m_onPickUp );
		}
		else if( m_pPickUp->y < 120 )
			m_pPickUp->SetPosition( m_pPickUp->GetPosition() + CVector2( 0, 1 ) );

		m_pEft1->SetRotation( m_pEft1->r + 0.097f );
		m_pEft2->SetRotation( m_pEft2->r - 0.093f );

		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	}
}
