#include "stdafx.h"
#include "OrganHpBar.h"
#include "Face.h"
#include "Image2D.h"
#include "Organ.h"
#include "Common/MathUtil.h"

void COrganHpBar::SetOrgan( COrgan * pOrgan )
{
	if( !pOrgan )
	{
		if( m_onHpChanged.IsRegistered() )
			m_onHpChanged.Unregister();
		return;
	}
	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );

	uint32 nWidth = pOrgan->GetWidth();
	uint32 nHeight = pOrgan->GetHeight();
	
	SetPosition( pOrgan->GetPosition() );
	CRectangle rect( nWidth * -0.5f, nHeight * -0.5f, nWidth, nHeight );
	rect = rect.Scale( pOrgan->GetFace()->GetGridScale() );
	rect.SetTop( rect.GetBottom() - 2 );
	rect.SetLeft( rect.GetLeft() + 4 );
	rect.SetRight( rect.GetRight() - 4 );
	pImage2D->SetRect( rect );
	pImage2D->GetParam()[0].z = rect.width;

	pOrgan->Register_OnHpChanged( &m_onHpChanged );
	OnOrganHpChanged( pOrgan );
}

void COrganHpBar::SetHp( uint32 nHp, uint32 nMaxHp )
{
	m_nHp = nHp;
	m_nMaxHp = nMaxHp;
	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );

	pImage2D->GetParam()[0].x = nHp;
	pImage2D->GetParam()[0].y = nMaxHp;

	pImage2D->GetParam()[0].w = HighestBit( nMaxHp ) - 0.5f;
}

void COrganHpBar::OnOrganHpChanged( COrgan * pOrgan )
{
	SetHp( pOrgan->GetCurHp(), pOrgan->GetMaxHp() );
}
