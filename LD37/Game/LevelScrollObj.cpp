#include "stdafx.h"
#include "LevelScrollObj.h"
#include "MyLevel.h"
#include "Stage.h"
#include "ParticleSystem.h"

void CLevelScrollObj::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
	static_cast<CParticleSystemObject*>( m_pEffect->GetRenderObject() )->GetInstanceData()->GetData().isEmitting = false;
	m_pEffect->SetAutoUpdateAnim( true );
}

void CLevelScrollObj::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLevelScrollObj::Update( uint32 nCur )
{
	uint32 nMinScrollPos = m_nMinHeight * CMyLevel::GetBlockSize();
	uint32 nMaxScrollPos = ( m_nMinHeight + m_nHeight ) * CMyLevel::GetBlockSize();
	if( nCur >= nMaxScrollPos )
	{
		SetRenderObject( NULL );
		auto& data = static_cast<CParticleSystemObject*>( m_pEffect->GetRenderObject() )->GetInstanceData()->GetData();
		data.isEmitting = false;
		if( data.nBegin == data.nEnd )
		{
			SetParentEntity( NULL );
			return;
		}
	}
	else if( nCur > nMinScrollPos )
	{
		SetPosition( CVector2( x, 0 ) );
		CImage2D* pImage2D = static_cast<CImage2D*>( GetRenderObject() );
		auto rect = pImage2D->GetElem().rect;
		auto texRect = pImage2D->GetElem().texRect;
		rect.height = nMaxScrollPos - nCur;
		texRect.height = rect.height / ( nMaxScrollPos - nMinScrollPos );
		pImage2D->SetRect( rect );
		pImage2D->SetTexRect( texRect );
		static_cast<CParticleSystemObject*>( m_pEffect->GetRenderObject() )->GetInstanceData()->GetData().isEmitting = true;
	}
	else
	{
		SetPosition( CVector2( x, nMinScrollPos - nCur ) );
	}
}

void CLevelScrollObj::OnTick()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );

	uint32 nLast = floor( CMyLevel::GetInst()->GetLastScrollPos() );
	uint32 nCur = floor( CMyLevel::GetInst()->GetCurScrollPos() );
	if( nCur != nLast )
		Update( nCur );
	else
	{
		static_cast<CParticleSystemObject*>( m_pEffect->GetRenderObject() )->GetInstanceData()->GetData().isEmitting = false;
	}
}
