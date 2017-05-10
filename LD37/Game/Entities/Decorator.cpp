#include "stdafx.h"
#include "Decorator.h"
#include "Render/DrawableGroup.h"
#include "Common/Rand.h"

void CDecoratorFiber::Init( const CVector2 & size )
{
	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	TVector2<int32> texelTexSize = TVector2<int32>( m_texSize.x, m_texSize.y );
	uint32 nWidth = m_bVertical ? size.x : size.y;
	uint32 nCount = ceil( nWidth / m_fWidth );
	float fMaxHeight = Min( size.y * m_fMaxHeightPercent, m_bVertical ? m_texSize.y * m_fTexelSize : m_texSize.x * m_fTexelSize );

	for( int i = 0; i < nCount; i++ )
	{
		float fWidth = i < nCount - 1 ? m_fWidth : nWidth - m_fWidth * ( nCount - 1 );
		float fHeight = fMaxHeight * SRand::Inst().Rand( m_fMinHeightPercent, 1.0f );
		uint32 nWidth = floor( fWidth / m_fTexelSize );
		uint32 nHeight = floor( fHeight / m_fTexelSize );
		uint32 nBaseX = SRand::Inst().Rand( 0u, texelTexSize.x - nWidth );
		uint32 nBaseY = SRand::Inst().Rand( 0u, texelTexSize.y - nHeight );
		CRectangle texRect( nBaseX, nBaseY, nWidth, nHeight  );
		CRectangle rect( m_fWidth * i, 0, nWidth * m_fTexelSize, nHeight * m_fTexelSize );

		switch( m_nAlignment )
		{
		case 0:
			break;
		case 2:
			rect.y = size.y - rect.height;
			break;
		default:
			rect.y = floor( ( size.y - rect.height ) * 0.5f / m_fTexelSize ) * m_fTexelSize;
			break;
		}

		if( !m_bVertical )
		{
			swap( rect.x, rect.y );
			swap( rect.width, rect.height );
			swap( texRect.x, texRect.y );
			swap( texRect.width, texRect.height );
		}

		texRect = texRect * CVector2( 1.0f / texelTexSize.x, 1.0f / texelTexSize.y );

		auto pImage2D = static_cast<CImage2D*>( pResource->CreateInstance() );
		pImage2D->SetRect( rect );
		pImage2D->SetTexRect( texRect );
		AddChild( pImage2D );
	}

	SetRenderObject( NULL );
}
