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

void CDecoratorDirt::Init( const CVector2 & size )
{
	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	TRectangle<int32> texelRect( 0, 0, floor( size.x / m_fTexelSize ), floor( size.y / m_fTexelSize ) );
	TRectangle<int32> firstSplitRect( -m_nMaxTexelSize, -m_nMaxTexelSize, texelRect.width + m_nMaxTexelSize * 2, texelRect.height + m_nMaxTexelSize * 2 );

	struct SSplit
	{
		SSplit( TRectangle<int32> rect, bool bSplit, int32 nParent ) : rect( rect ), bSplit( bSplit ), nParent( nParent ) {}
		TRectangle<int32> rect;
		bool bSplit;
		int32 nParent;
	};
	vector<SSplit> splits;

	splits.push_back( SSplit( firstSplitRect, false, -1 ) );
	for( int i = 0; i < splits.size(); i++ )
	{
		auto& curSplit = splits[i];
		auto& rect = curSplit.rect;

		int32 r1 = rect.width - m_nMinTexelSize * 2 + 1;
		int32 r2 = rect.height - m_nMinTexelSize * 2 + 1;
		if( rect.width > m_nMaxTexelSize || rect.height > m_nMaxTexelSize )
			splits[i].bSplit = true;
		else
		{
			if( r1 > 0 && r2 > 0 )
			{
				float r = ( r1 + r2 ) * 1.0f / ( ( m_nMaxTexelSize - m_nMinTexelSize * 2 + 1 ) * 2 );
				if( SRand::Inst().Rand( 0.0f, 1.0f ) < r )
					splits[i].bSplit = true;
			}
		}

		if( splits[i].bSplit )
		{
			int32 nSplitPos = SRand::Inst().Rand( 0, r1 + r2 );
			bool bSplitY = false;
			if( nSplitPos >= r1 )
			{
				nSplitPos -= r1;
				bSplitY = true;
			}
			nSplitPos += m_nMinTexelSize;

			TRectangle<int32> rect1 = rect, rect2 = rect;
			if( !bSplitY )
			{
				rect1.width = nSplitPos;
				rect2.SetLeft( nSplitPos + rect.x );
			}
			else
			{
				rect1.height = nSplitPos;
				rect2.SetTop( nSplitPos + rect.y );
			}

			bool bLeftFirst = SRand::Inst().Rand() & 1;
			if( bLeftFirst )
			{
				splits.push_back( SSplit( rect1, false, i ) );
				splits.push_back( SSplit( rect2, false, i ) );
			}
			else
			{
				splits.push_back( SSplit( rect2, false, i ) );
				splits.push_back( SSplit( rect1, false, i ) );
			}
		}
	}

	float fMin = Max( 0.0f, 2 * m_fPercent - 1 );
	float fMax = Min( 2 * m_fPercent, 1.0f );
	for( auto& split : splits )
	{
		if( split.bSplit )
			continue;

		auto rect = split.rect;
		bool bY = SRand::Inst().Rand() & 1;
		bool bLeft = SRand::Inst().Rand() & 1;
		float fPercent = SRand::Inst().Rand( fMin, fMax );
		if( !bY )
		{
			int32 nNewSize = Max( (uint32)floor( rect.width * fPercent ), m_nMinTexelSize );
			if( bLeft )
				rect.SetLeft( rect.GetRight() - nNewSize );
			else
				rect.width = nNewSize;
		}
		else
		{
			int32 nNewSize = Max( (uint32)floor( rect.height * fPercent ), m_nMinTexelSize );
			if( bLeft )
				rect.SetTop( rect.GetBottom() - nNewSize );
			else
				rect.height = nNewSize;
		}

		auto clippedRect = texelRect * rect;
		if( clippedRect.width && clippedRect.height )
		{
			uint32 nMask = SRand::Inst().Rand( 0u, m_nMaskCols * m_nMaskRows );
			int32 dx = m_texSize.x - clippedRect.width;
			int32 dy = m_texSize.y - clippedRect.width;
			TRectangle<int32> texRect( SRand::Inst().Rand( 0, dx + 1 ), SRand::Inst().Rand( 0, dy + 1 ), clippedRect.width, clippedRect.height );
			CRectangle texRect1( ( nMask % m_nMaskCols ) * 1.0f / m_nMaskCols, ( nMask / m_nMaskCols ) * 1.0f / m_nMaskRows, 1.0f / m_nMaskCols, 1.0f / m_nMaskRows );
			if( clippedRect != rect )
			{
				texRect1.x += texRect1.width * ( clippedRect.x - rect.x ) / rect.width;
				texRect1.y += texRect1.height * ( rect.GetBottom() - clippedRect.GetBottom() ) / rect.height;
				texRect1.width *= clippedRect.width * 1.0f / rect.width;
				texRect1.height *= clippedRect.height * 1.0f / rect.height;
			}

			auto pImage2D = static_cast<CImage2D*>( pResource->CreateInstance() );
			pImage2D->SetRect( CRectangle( clippedRect.x, clippedRect.y, clippedRect.width, clippedRect.height ) * m_fTexelSize );
			pImage2D->SetTexRect( CRectangle( texRect.x / m_texSize.x, texRect.y / m_texSize.y, texRect.width / m_texSize.x, texRect.height / m_texSize.y ) );
			pImage2D->GetParam()[0] = CVector4( texRect1.x, texRect1.y, texRect1.width, texRect1.height );
			AddChild( pImage2D );
		}
	}

	SetRenderObject( NULL );
}

void CDecoratorTile::Init( const CVector2& size )
{
	int32 nTileX = size.x / m_nTileSize;
	int32 nTileY = size.y / m_nTileSize;

	if( m_nTileCount[0] )
		AddTile( TVector2<int32>( 0, 0 ), 0 );
	if( m_nTileCount[2] )
		AddTile( TVector2<int32>( nTileX - 1, 0 ), 2 );
	if( m_nTileCount[6] )
		AddTile( TVector2<int32>( 0, nTileY - 1 ), 6 );
	if( m_nTileCount[8] )
		AddTile( TVector2<int32>( nTileX - 1, nTileY - 1 ), 8 );

	if( m_nTileCount[1] )
	{
		for( int i = 1; i < nTileX - 1; i++ )
			AddTile( TVector2<int32>( i, 0 ), 1 );
	}
	if( m_nTileCount[3] )
	{
		for( int i = 1; i < nTileY - 1; i++ )
			AddTile( TVector2<int32>( 0, i ), 3 );
	}
	if( m_nTileCount[5] )
	{
		for( int i = 1; i < nTileY - 1; i++ )
			AddTile( TVector2<int32>( nTileX - 1, i ), 5 );
	}
	if( m_nTileCount[7] )
	{
		for( int i = 1; i < nTileX - 1; i++ )
			AddTile( TVector2<int32>( i, nTileY - 1 ), 7 );
	}

	if( m_nTileCount[4] )
	{
		for( int i = 1; i < nTileX - 1; i++ )
		{
			for( int j = 1; j < nTileY - 1; j++ )
			{
				AddTile( TVector2<int32>( i, j ), 4 );
			}
		}
	}

	SetRenderObject( NULL );
}

void CDecoratorTile::AddTile( TVector2<int32> pos, uint8 nType )
{
	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
	AddChild( pImg );
	pImg->SetRect( CRectangle( m_nTileSize * pos.x, m_nTileSize * pos.y, m_nTileSize, m_nTileSize ) );

	uint32 nTex = m_nTileBegin[nType] + SRand::Inst().Rand( 0u, m_nTileCount[nType] );
	uint32 texY = nTex / m_nTexCols;
	uint32 texX = nTex - texY * m_nTexCols;
	pImg->SetTexRect( CRectangle( texX * 1.0f / m_nTexCols, ( m_nTexRows - 1 - texY ) * 1.0f / m_nTexCols, 1.0f / m_nTexCols, 1.0f / m_nTexRows ) );

	uint16 nParamCount;
	CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
	CVector4* dst = pImg->GetParam();
	memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
}
