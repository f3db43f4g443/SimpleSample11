#include "stdafx.h"
#include "Decorator.h"
#include "Render/DrawableGroup.h"
#include "Common/Rand.h"
#include "Block.h"
#include "MyLevel.h"

void CDecoratorRandomTex::Init( const CVector2 & size )
{
	int32 sizeX = m_texSize.x;
	int32 sizeY = m_texSize.y;
	CVector2 maxSize = m_texSize * m_fTexelSize;
	int32 nX = ceil( size.x / maxSize.x );
	int32 nY = ceil( size.y / maxSize.y );
	CVector2 baseTex( SRand::Inst().Rand( 0, sizeX ) * 1.0f / sizeX, SRand::Inst().Rand( 0, sizeY ) * 1.0f / sizeY );
	if( nX == 1 && nY == 1 )
	{
		auto pImage = static_cast<CImage2D*>( GetRenderObject() );
		pImage->SetRect( CRectangle( 0, 0, size.x, size.y ) );
		pImage->SetTexRect( CRectangle( baseTex.x, baseTex.y, size.x / maxSize.x, size.y / maxSize.y ) );
	}
	else
	{
		auto pResource = static_cast<CDrawableGroup*>( GetResource() );

		for( int i = 0; i < nX; i++ )
		{
			for( int j = 0; j < nY; j++ )
			{
				auto pImage = static_cast<CImage2D*>( pResource->CreateInstance() );
				CRectangle rect( i * maxSize.x, ( nY - j - 1 ) * maxSize.y, Min( maxSize.x, size.x - i * maxSize.x ), Min( maxSize.y, size.y - ( nY - j - 1 ) * maxSize.y ) );
				pImage->SetRect( rect );
				pImage->SetTexRect( CRectangle( baseTex.x, baseTex.y, rect.width / maxSize.x, rect.height / maxSize.y ) );
				AddChild( pImage );
			}
		}

		SetRenderObject( NULL );
	}
}

void CDecorator9Patch::Init( const CVector2 & size )
{
	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	CRectangle rect = pImage->GetElem().rect;
	CRectangle texRect = pImage->GetElem().texRect;

	float fRectX[4] = { 0, rect.width * m_fX1, size.x - rect.width * m_fX2, size.x };
	float fRectY[4] = { 0, rect.height * m_fY1, size.y - rect.height * m_fY2, size.y };
	float fTexRectX[4] = { texRect.x, texRect.x + texRect.width * m_fX1, texRect.x + texRect.width * ( 1 - m_fX2 ), texRect.x + texRect.width };
	float fTexRectY[4] = { texRect.y + texRect.height, texRect.y + texRect.height * ( 1 - m_fY1 ), texRect.y + texRect.height * m_fY2, texRect.y };

	for( int i = 0; i < 3; i++ )
	{
		for( int j = 0; j < 3; j++ )
		{
			auto pImage = static_cast<CImage2D*>( pResource->CreateInstance() );
			pImage->SetRect( CRectangle( fRectX[i], fRectY[j], fRectX[i + 1] - fRectX[i], fRectY[j + 1] - fRectY[j] ) );
			pImage->SetTexRect( CRectangle( fTexRectX[i], fTexRectY[j + 1], fTexRectX[i + 1] - fTexRectX[i], fTexRectY[j] - fTexRectY[j + 1] ) );
			AddChild( pImage );
		}
	}

	SetRenderObject( NULL );
}

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

			bool bLeftFirst = SRand::Inst().Rand( 0, 2 );
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
		bool bY = SRand::Inst().Rand( 0, 2 );
		bool bLeft = SRand::Inst().Rand( 0, 2 );
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
	CChunkObject* pChunkObject = NULL;
	for( auto pParent = GetParentEntity(); pParent && !pChunkObject; pParent = pParent->GetParentEntity() )
	{
		pChunkObject = SafeCast<CChunkObject>( pParent );
		if( pChunkObject )
			break;
	}
	int32 nTileX = size.x / m_nTileSize;
	int32 nTileY = size.y / m_nTileSize;

	if( m_nTileCount[0] )
		AddTile( TVector2<int32>( 0, 0 ), 0, pChunkObject );
	if( m_nTileCount[2] )
		AddTile( TVector2<int32>( nTileX - 1, 0 ), 2, pChunkObject );
	if( m_nTileCount[6] )
		AddTile( TVector2<int32>( 0, nTileY - 1 ), 6, pChunkObject );
	if( m_nTileCount[8] )
		AddTile( TVector2<int32>( nTileX - 1, nTileY - 1 ), 8, pChunkObject );

	if( m_nTileCount[1] )
	{
		for( int i = 1; i < nTileX - 1; i++ )
			AddTile( TVector2<int32>( i, 0 ), 1, pChunkObject );
	}
	if( m_nTileCount[3] )
	{
		for( int i = 1; i < nTileY - 1; i++ )
			AddTile( TVector2<int32>( 0, i ), 3, pChunkObject );
	}
	if( m_nTileCount[5] )
	{
		for( int i = 1; i < nTileY - 1; i++ )
			AddTile( TVector2<int32>( nTileX - 1, i ), 5, pChunkObject );
	}
	if( m_nTileCount[7] )
	{
		for( int i = 1; i < nTileX - 1; i++ )
			AddTile( TVector2<int32>( i, nTileY - 1 ), 7, pChunkObject );
	}

	if( m_nTileCount[4] )
	{
		for( int i = 1; i < nTileX - 1; i++ )
		{
			for( int j = 1; j < nTileY - 1; j++ )
			{
				AddTile( TVector2<int32>( i, j ), 4, pChunkObject );
			}
		}
	}

	SetRenderObject( NULL );
}

void CDecoratorTile::AddTile( TVector2<int32> pos, uint8 nType, CChunkObject* pChunkObject )
{
	if( pChunkObject )
	{
		int32 nBlockX = floor( ( pos.x * m_nTileSize + x ) / CMyLevel::GetBlockSize() );
		int32 nBlockY = floor( ( pos.y * m_nTileSize + y ) / CMyLevel::GetBlockSize() );
		auto pBlock = pChunkObject->GetBlock( nBlockX, nBlockY );
		if( !pBlock || pBlock->nTag >= m_nBlockTag )
			return;
	}

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

void CDecoratorTile1::Init( const CVector2& size )
{
	CChunkObject* pChunkObject = NULL;
	for( auto pParent = GetParentEntity(); pParent && !pChunkObject; pParent = pParent->GetParentEntity() )
	{
		pChunkObject = SafeCast<CChunkObject>( pParent );
		if( pChunkObject )
			break;
	}
	if( !pChunkObject )
		return;

	int32 x0 = floor( x / CMyLevel::GetBlockSize() );
	int32 y0 = floor( y / CMyLevel::GetBlockSize() );
	int32 x1 = ceil( ( x + size.x ) / CMyLevel::GetBlockSize() );
	int32 y1 = ceil( ( y + size.y ) / CMyLevel::GetBlockSize() );
	x0 = Max( x0, 0 );
	y0 = Max( y0, 0 );
	uint32 nWidth = pChunkObject->GetChunk()->nWidth;
	uint32 nHeight = pChunkObject->GetChunk()->nHeight;
	x1 = Min<int32>( x1, nWidth );
	y1 = Min<int32>( y1, nHeight );
	SetPosition( CVector2( 0, 0 ) );

	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	for( int i = x0; i < x1; i++ )
	{
		for( int j = y0; j < y1; j++ )
		{
			uint8 n = pChunkObject->GetBlock( i, j )->nTag;
			if( n >= m_nBlockTag )
				continue;
			uint8 nLeft = i > 0 ? pChunkObject->GetBlock( i - 1, j )->nTag >= m_nBlockTag : false;
			uint8 nRight = i < nWidth - 1 ? pChunkObject->GetBlock( i + 1, j )->nTag >= m_nBlockTag : false;
			uint8 nTop = j > 0 ? pChunkObject->GetBlock( i, j - 1 )->nTag >= m_nBlockTag : false;
			uint8 nBottom = j < nHeight - 1 ? pChunkObject->GetBlock( i, j + 1 )->nTag >= m_nBlockTag : false;
			uint8 nType = nLeft | ( nTop << 1 ) | ( nRight << 2 ) | ( nBottom << 3 );
			if( !m_nTileCount[nType] )
				continue;

			auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
			AddChild( pImg );
			pImg->SetRect( CRectangle( i * CMyLevel::GetBlockSize(), j * CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize() ) );

			uint32 nTex = m_nTileBegin[nType] + SRand::Inst().Rand( 0u, m_nTileCount[nType] );
			uint32 texY = nTex / m_nTexCols;
			uint32 texX = nTex - texY * m_nTexCols;
			pImg->SetTexRect( CRectangle( texX * 1.0f / m_nTexCols, ( m_nTexRows - 1 - texY ) * 1.0f / m_nTexCols, 1.0f / m_nTexCols, 1.0f / m_nTexRows ) );

			uint16 nParamCount;
			CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
			CVector4* dst = pImg->GetParam();
			memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
		}
	}

	SetRenderObject( NULL );
}