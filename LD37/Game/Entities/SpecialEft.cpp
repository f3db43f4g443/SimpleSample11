#include "stdafx.h"
#include "SpecialEft.h"
#include "Stage.h"
#include "Render/Image2D.h"

void CLimbsEft::OnAddedToStage()
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	SetRenderObject( NULL );

	auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y - m_fSize * 0.5f, m_fSize, m_fSize ) );
	pImg->SetTexRect( CRectangle( 0.5f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) * 0.125f, 0.25f, 0.25f ) );
	AddChild( pImg );
	m_pImg[1] = pImg;

	pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y - m_fSize * 0.5f + SRand::Inst().Rand( -2, 3 ) * ( 1.0f / 16 ), m_fSize, m_fSize ) );
	pImg->SetTexRect( CRectangle( 0, SRand::Inst<eRand_Render>().Rand( 0, 4 ) * 0.25f, 0.25f, 0.25f ) );
	AddChild( pImg );
	m_pImg[3] = pImg;

	pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y + m_fSize * ( SRand::Inst().Rand( 0, 4 ) * ( 1.0f / 16 )  - 0.5f ), m_fSize * 0.5f, m_fSize * 0.5f ) );
	pImg->SetTexRect( CRectangle( 0.75f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) * 0.125f, 0.125f, 0.125f ) );
	AddChild( pImg );
	m_pImg[0] = pImg;

	pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	pImg->SetRect( CRectangle( m_ofs.x, m_ofs.y - m_fSize * SRand::Inst().Rand( 0, 4 ) * ( 1.0f / 16 ), m_fSize * 0.5f, m_fSize * 0.5f ) );
	pImg->SetTexRect( CRectangle( 0.875f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) * 0.125f, 0.125f, 0.125f ) );
	AddChild( pImg );
	m_pImg[2] = pImg;

	m_nTick = SRand::Inst().Rand( 0, 4 );
	GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );
}

void CLimbsEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLimbsEft::OnTick()
{
	GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );

	CImage2D* pImage2D = static_cast<CImage2D*>( m_pImg[m_nTick].GetPtr() );
	auto rect = pImage2D->GetElem().rect;
	auto texRect = pImage2D->GetElem().texRect;
	switch( m_nTick )
	{
	case 0:
		rect.y = ( ( rect.y - m_ofs.y ) / m_fSize + 0.5f ) * 16;
		rect.y = Min( 3.0f, Max( 0.0f, rect.y + SRand::Inst<eRand_Render>().Rand( -1, 2 ) ) );
		rect.y = ( rect.y * ( 1.0f / 16 ) - 0.5f ) * m_fSize + m_ofs.y;
		texRect.x = texRect.x == 0.75f ? 0.875f : 0.75f;
		pImage2D->SetRect( rect );
		pImage2D->SetTexRect( texRect );
		pImage2D->SetBoundDirty();
		break;
	case 2:
		rect.y = ( ( rect.y - m_ofs.y ) / m_fSize ) * 16;
		rect.y = Min( 0.0f, Max( -3.0f, rect.y + SRand::Inst<eRand_Render>().Rand( -1, 2 ) ) );
		rect.y = ( rect.y * ( 1.0f / 16 ) ) * m_fSize + m_ofs.y;
		texRect.x = texRect.x == 0.75f ? 0.875f : 0.75f;
		pImage2D->SetRect( rect );
		pImage2D->SetTexRect( texRect );
		pImage2D->SetBoundDirty();
		break;
	case 1:
	{
		uint32 nY = texRect.y * 8;
		nY = nY + SRand::Inst().Rand( 2, 6 );
		if( nY >= 8 )
			nY -= 8;
		texRect.y = nY * 0.125f;
		pImage2D->SetTexRect( texRect );
		break;
	}
	case 3:
		texRect.x = texRect.x == 0 ? 0.25f : 0;
		pImage2D->SetTexRect( texRect );
		break;
	}
	m_nTick++;
	if( m_nTick == 4 )
		m_nTick = 0;
}

void CLimbsAttackEft::OnAddedToStage()
{
	SetRenderObject( NULL );
}

void CLimbsAttackEft::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLimbsAttackEft::CreateAttackEft( CRenderObject2D* pRenderParent1, CRenderObject2D* pRenderParent2, uint8 nMask )
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	for( int i = 0; i < 8; i++ )
	{
		if( m_pAttackEft[i] )
		{
			m_pAttackEft[i]->RemoveThis();
			m_pAttackEft[i] = NULL;
		}
		if( !!( nMask & ( 1 << i ) ) )
			continue;

		m_pAttackEft[i] = new CRenderObject2D;
		m_pAttackEft[i]->y = i < 4 ? ( i - 1.5f ) * 8 : ( i - 5.5f ) * 8;
		if( i < 4 )
			m_pAttackEft[i]->y += i < 2 ? -1 : 1;
		else
			m_pAttackEft[i]->y += i < 6 ? 1 : -1;
		m_pAttackEft[i]->y = m_pAttackEft[i]->y * m_fSize / 32;

		AddChild( m_pAttackEft[i] );
		auto pRenderParent = i < 4 ? pRenderParent1 : pRenderParent2;
		if( pRenderParent )
			m_pAttackEft[i]->SetRenderParentBefore( pRenderParent );

		auto pImg = static_cast<CImage2D*>( pDrawable->CreateInstance() );
		pImg->SetRect( CRectangle( 0, -m_fSize * 0.25f, m_fSize * 0.25f, m_fSize * 0.5f ) );
		CRectangle texRect( 1 - 1.0f / 16, SRand::Inst<eRand_Render>().Rand( 0, 8 ) / 8.0f, 1.0f / 16, 1.0f / 8 );
		texRect = texRect * 0.5f;
		if( i < 4 )
			texRect.x += 0.5f;
		if( SRand::Inst<eRand_Render>().Rand( 0, 2 ) )
			texRect.y += 0.5f;
		pImg->SetTexRect( texRect );
		m_pAttackEft[i]->AddChild( pImg );
	}

	SRand::Inst<eRand_Render>().Shuffle( m_pAttackEft, ELEM_COUNT( m_pAttackEft ) );
	m_nTick = 0;
	m_fEftLen = 0;

	GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );
}

void CLimbsAttackEft::SetAttackEftLen( float fLen )
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	float fLenPerSeg = m_fSize;
	fLen = ceil( fLen );
	int32 nCurSeg = ceil( fLen / fLenPerSeg );
	int32 nLastSeg = ceil( m_fEftLen / fLenPerSeg );

	for( int i = 0; i < 8; i++ )
	{
		if( !m_pAttackEft[i] )
			continue;
		m_pAttackEft[i]->SetPosition( CVector2( fLen, m_pAttackEft[i]->y ) );
		auto pImg = static_cast<CImage2D*>( m_pAttackEft[i]->Get_TransformChild() );

		if( nLastSeg == 0 )
		{
			CRectangle rect = pImg->GetElem().rect;
			rect.width = m_fSize * 0.5f;
			pImg->SetRect( rect );
			pImg->SetBoundDirty();
			CRectangle texRect = pImg->GetElem().texRect;
			texRect.x = texRect.x > 0.5f ? 1.0f - ( 1.0f / 16 ) : 0.5f - ( 1.0f / 16 );
			texRect.width = 1.0f / 16;
			pImg->SetTexRect( texRect );
		}
		else if( nLastSeg < nCurSeg )
		{
			CRectangle rect = pImg->GetElem().rect;
			rect.SetLeft( rect.GetRight() - m_fSize );
			pImg->SetRect( rect );
			pImg->SetBoundDirty();
			CRectangle texRect = pImg->GetElem().texRect;
			texRect.SetLeft( texRect.GetRight() - rect.width / ( m_fSize * 8 ) );
			pImg->SetTexRect( texRect );
		}

		for( int n = nLastSeg; n > nCurSeg; n-- )
		{
			auto pNxt = pImg->NextTransformChild();
			pImg->RemoveThis();
			pImg = static_cast<CImage2D*>( pNxt );
		}
		for( int n = nLastSeg; n < nCurSeg; n++ )
		{
			float dY = -pImg->y;

			auto pNxt = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			CRectangle rect( -m_fSize * ( n + 1 ), -m_fSize * 0.25f, m_fSize, m_fSize * 0.5f );
			pNxt->SetRect( rect );
			CRectangle texRect( 0.5f, SRand::Inst<eRand_Render>().Rand( 0, 8 ) / 8.0f, 1.0f / 4, 1.0f / 8 );
			if( dY > 0 )
				texRect.x = 0;
			else if( dY < 0 )
				texRect.x = 0.25f;
			texRect = texRect * 0.5f;
			if( pImg->GetElem().texRect.x >= 0.5f )
				texRect.x += 0.5f;
			if( pImg->GetElem().texRect.y >= 0.5f )
				texRect.y += 0.5f;
			pNxt->SetTexRect( texRect );
			m_pAttackEft[i]->AddChild( pNxt );
			pImg = pNxt;
		}

		if( nCurSeg )
		{
			CRectangle rect = pImg->GetElem().rect;
			rect.SetLeft( Max( rect.GetRight() - m_fSize, -fLen ) );
			pImg->SetRect( rect );
			pImg->SetBoundDirty();
			CRectangle texRect = pImg->GetElem().texRect;
			texRect.SetLeft( texRect.GetRight() - rect.width / ( m_fSize * 8 ) );
			pImg->SetTexRect( texRect );
		}
	}

	m_fEftLen = fLen;
}

void CLimbsAttackEft::DestroyAttackEft()
{
	for( int i = 0; i < 8; i++ )
	{
		if( m_pAttackEft[i] )
		{
			m_pAttackEft[i]->RemoveThis();
			m_pAttackEft[i] = NULL;
		}
	}
	m_fEftLen = 0;
	m_nTick = 0;
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CLimbsAttackEft::OnTick()
{
	GetStage()->RegisterAfterHitTest( m_nFrames, &m_onTick );

	if( m_fEftLen == 0 )
	{
		m_nTick++;
		if( m_nTick == 8 )
		{
			for( int i = 0; i < 8; i++ )
			{
				if( !m_pAttackEft[i] )
					continue;
				auto pImg = static_cast<CImage2D*>( m_pAttackEft[i]->Get_TransformChild() );
				auto rect = pImg->GetElem().rect;
				auto texRect = pImg->GetElem().texRect;
				if( texRect.width < 1.0f / 16 )
				{
					rect.width = m_fSize * 0.5f;
					texRect.SetLeft( texRect.GetRight() - 1.0f / 16 );
					pImg->SetRect( rect );
					pImg->SetTexRect( texRect );
					pImg->SetBoundDirty();
				}
				else if( texRect.GetRight() == 0.5f || texRect.GetRight() == 1 )
				{
					texRect.x -= 1.0f / 16;
					pImg->SetTexRect( texRect );
				}
			}

			m_nTick = 0;
		}
		return;
	}

	if( m_pAttackEft[m_nTick] )
	{
		auto pImg = static_cast<CImage2D*>( m_pAttackEft[m_nTick]->Get_TransformChild() );
		float fCurY = 0;
		float dY = m_fSize / 16;

		while( pImg )
		{
			pImg->SetPosition( CVector2( pImg->x, fCurY ) );
			auto pNxt = pImg->NextTransformChild();
			if( !pNxt )
				break;

			float fNxtY = pNxt->y;
			auto pNxt1 = pNxt->NextTransformChild();
			if( !pNxt1 )
			{
				if( fNxtY == 0 )
					fNxtY = SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? -dY : dY;
				else
					fNxtY = 0;
			}
			else
			{
				float fNxt1Y = pNxt1->y;
				if( fNxt1Y - fCurY == dY || fCurY - fNxt1Y == dY )
				{
					if( fNxtY == fCurY )
						fNxtY = fNxt1Y;
					else
						fNxt1Y = fCurY;
				}
				else if( fNxt1Y == fCurY )
				{
					if( fNxtY == 0 )
					{
						if( fCurY == 0 )
							fNxtY = SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? -dY : dY;
						else
							fNxtY = fCurY;
					}
					else
						fNxtY = 0;
				}
				else
					fNxtY = 0;
			}

			auto texRect = pImg->GetElem().texRect;
			float fBaseX = texRect.x >= 0.5f ? 0.5f : 0;
			if( fNxtY > fCurY )
				texRect.x = fBaseX;
			else if( fNxtY < fCurY )
				texRect.x = fBaseX + 0.125f;
			else
				texRect.x = fBaseX + 0.25f;
			pImg->SetTexRect( texRect );

			fCurY = fNxtY;
			pImg = static_cast<CImage2D*>( pNxt );
		}
	}

	m_nTick++;
	if( m_nTick == 8 )
		m_nTick = 0;
}
