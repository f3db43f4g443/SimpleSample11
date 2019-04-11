#include "stdafx.h"
#include "Decorator.h"
#include "Render/DrawableGroup.h"
#include "Common/Rand.h"
#include "Block.h"
#include "MyLevel.h"
#include "Common/Algorithm.h"

void CDecoratorRandomTex::Init( const CVector2 & size, SChunk* pPreParent )
{
	int32 sizeX = m_texSize.x;
	int32 sizeY = m_texSize.y;
	CVector2 maxSize = m_texSize * m_fTexelSize;
	int32 nX = ceil( size.x / maxSize.x );
	int32 nY = ceil( size.y / maxSize.y );
	auto& rnd = SRand::Inst<eRand_Render>();
	CVector2 baseTex( rnd.Rand( 0, sizeX ) * 1.0f / sizeX, rnd.Rand( 0, sizeY ) * 1.0f / sizeY );
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

void CDecorator9Patch::Init( const CVector2 & size, SChunk* pPreParent )
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

void CDecoratorFiber::Init( const CVector2 & size, SChunk* pPreParent )
{
	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	TVector2<int32> texelTexSize = TVector2<int32>( m_texSize.x, m_texSize.y );
	uint32 nWidth = m_bVertical ? size.x : size.y;
	float fMaxHeight = Min( ( m_nType == 1 ? CMyLevel::GetBlockSize() : size.y ) * m_fMaxHeightPercent, m_bVertical ? m_texSize.y * m_fTexelSize : m_texSize.x * m_fTexelSize );

	auto& rnd = SRand::Inst<eRand_Render>();
	uint32 nCount = ceil( nWidth / ( m_nType == 1 ? CMyLevel::GetBlockSize() : m_fWidth ) );
	if( m_nType == 1 )
	{
		CChunkObject* pChunkObject = NULL;
		for( auto pParent = GetParentEntity(); pParent && !pChunkObject; pParent = pParent->GetParentEntity() )
		{
			pChunkObject = SafeCast<CChunkObject>( pParent );
			if( pChunkObject )
				break;
		}

		float fWidth = CMyLevel::GetBlockSize();
		int32 iBegin = floor( ( m_bVertical ? x : y ) / CMyLevel::GetBlockSize() );
		int32 jBegin = floor( ( m_bVertical ? y : x ) / CMyLevel::GetBlockSize() );
		int32 jEnd = floor( ( m_bVertical ? y + size.y : x + size.x ) / CMyLevel::GetBlockSize() );
		for( int i = 0; i < nCount; i++ )
		{
			int j0 = 0;
			for( int j = 0; j <= jEnd - jBegin; j++ )
			{
				int32 nBlockX = i + iBegin;
				int32 nBlockY = j + jBegin;
				if( !m_bVertical )
					swap( nBlockX, nBlockY );
				auto pBlock = j < jEnd - jBegin ? pChunkObject->GetBlock( nBlockX, nBlockY ) : NULL;
				bool b = pBlock && pBlock->nTag >= m_nBlockTag && pBlock->nTag < m_nBlockTag1;
				if( !b )
				{
					if( j > j0 )
					{
						float fHeight = fMaxHeight * rnd.Rand( m_fMinHeightPercent, 1.0f );
						float h1 = ( j - j0 ) * CMyLevel::GetBlockSize();
						fHeight = Min( fHeight, h1 );
						uint32 nWidth = floor( fWidth / m_fTexelSize );
						uint32 nHeight = floor( fHeight / m_fTexelSize );
						uint32 nBaseX = rnd.Rand( 0u, texelTexSize.x - nWidth );
						uint32 nBaseY = rnd.Rand( 0u, texelTexSize.y - nHeight );
						CRectangle texRect( nBaseX, nBaseY, nWidth, nHeight );
						CRectangle rect( fWidth * i, 0, nWidth * m_fTexelSize, nHeight * m_fTexelSize );

						switch( m_nAlignment )
						{
						case 0:
							break;
						case 2:
							rect.y = j * CMyLevel::GetBlockSize() - rect.height;
							break;
						default:
							rect.y = floor( ( ( j + j0 ) * CMyLevel::GetBlockSize() - rect.height ) * 0.5f / m_fTexelSize ) * m_fTexelSize;
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
					j0 = j + 1;
				}
			}
		}
	}
	else
	{
		for( int i = 0; i < nCount; i++ )
		{
			float fWidth = i < nCount - 1 ? m_fWidth : nWidth - m_fWidth * ( nCount - 1 );
			float fHeight = fMaxHeight * rnd.Rand( m_fMinHeightPercent, 1.0f );
			uint32 nWidth = floor( fWidth / m_fTexelSize );
			uint32 nHeight = floor( fHeight / m_fTexelSize );
			uint32 nBaseX = rnd.Rand( 0u, texelTexSize.x - nWidth );
			uint32 nBaseY = rnd.Rand( 0u, texelTexSize.y - nHeight );
			CRectangle texRect( nBaseX, nBaseY, nWidth, nHeight );
			CRectangle rect( fWidth * i, 0, nWidth * m_fTexelSize, nHeight * m_fTexelSize );

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
	}

	SetRenderObject( NULL );
}

void CDecoratorDirt::Init( const CVector2 & size, SChunk* pPreParent )
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

	auto& rnd = SRand::Inst<eRand_Render>();
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
				if( rnd.Rand( 0.0f, 1.0f ) < r )
					splits[i].bSplit = true;
			}
		}

		if( splits[i].bSplit )
		{
			int32 nSplitPos = rnd.Rand( 0, r1 + r2 );
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

			bool bLeftFirst = rnd.Rand( 0, 2 );
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
		bool bY = rnd.Rand( 0, 2 );
		bool bLeft = rnd.Rand( 0, 2 );
		float fPercent = rnd.Rand( fMin, fMax );
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
			uint32 nMask = rnd.Rand( 0u, m_nMaskCols * m_nMaskRows );
			int32 dx = m_texSize.x - clippedRect.width;
			int32 dy = m_texSize.y - clippedRect.width;
			TRectangle<int32> texRect( rnd.Rand( 0, dx + 1 ), rnd.Rand( 0, dy + 1 ), clippedRect.width, clippedRect.height );
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

void CDecoratorTile0::Init( const CVector2& size, struct SChunk* pPreParent )
{
	CChunkObject* pChunkObject = NULL;
	for( auto pParent = GetParentEntity(); pParent && !pChunkObject; pParent = pParent->GetParentEntity() )
	{
		pChunkObject = SafeCast<CChunkObject>( pParent );
		if( pChunkObject )
			break;
	}
	auto pChunk = pChunkObject->GetChunk();
	vector<int8> vecTemp;
	vecTemp.resize( pChunk->nWidth * pChunk->nHeight );
	vector<TVector2<int32> > vec;
	for( int i = 0; i < pChunk->nWidth; i++ )
	{
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			if( pChunk->GetBlock( i, j )->nTag >= m_nBlockTag )
				vecTemp[i + j * pChunk->nWidth] = 1;
			else
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst<eRand_Render>().Shuffle( vec );

	swap( SRand::Inst().nSeed, SRand::Inst<eRand_Render>().nSeed );
	int32 s = m_fPercent * vec.size();
	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	int32 nTileSize = CMyLevel::GetBlockSize();
	for( auto& p : vec )
	{
		if( vecTemp[p.x + p.y * pChunk->nWidth] )
			continue;
		auto rect = PutRect( vecTemp, pChunk->nWidth, pChunk->nHeight, p, TVector2<int32>( 1, 1 ),
			TVector2<int32>( SRand::Inst().Rand( 1, m_nMaxSizeX + 1 ), SRand::Inst().Rand( 1, m_nMaxSizeY + 1 ) ),
			TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ), -1, 1 );
		if( rect.width > 0 )
		{
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				for( int j = rect.y; j < rect.GetBottom(); j++ )
				{
					auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
					AddChild( pImg );
					pImg->SetRect( CRectangle( nTileSize * i, nTileSize * j, nTileSize, nTileSize ) );

					uint32 texX = SRand::Inst().Rand( 0u, m_nTexCols );
					uint32 texY = SRand::Inst().Rand( 0u, m_nTexRows );
					pImg->SetTexRect( CRectangle( texX * 1.0f / m_nTexCols, texY * 1.0f / m_nTexRows, 1.0f / m_nTexCols, 1.0f / m_nTexRows ) );

					uint16 nParamCount;
					CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
					CVector4* dst = pImg->GetParam();
					if( nParamCount )
						memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
				}
			}
			s -= rect.width * rect.height;
			if( s <= 0 )
				break;
		}
	}
	SetRenderObject( NULL );
	swap( SRand::Inst().nSeed, SRand::Inst<eRand_Render>().nSeed );
}

void CDecoratorTile::Init( const CVector2& size, SChunk* pPreParent )
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
	if( pResource->GetType() == CDrawableGroup::eType_MultiFrame )
	{
		auto pImg = static_cast<CMultiFrameImage2D*>( pResource->CreateInstance() );
		pImg->SetAutoUpdateAnim( true );
		AddChild( pImg );
		pImg->SetPosition( CVector2( m_nTileSize * pos.x, m_nTileSize * pos.y ) );
		pImg->SetFrames( m_nTileBegin[nType], m_nTileBegin[nType] + m_nTileCount[nType], pImg->GetFramesPerSec() );
	}
	else
	{
		auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
		AddChild( pImg );
		pImg->SetRect( CRectangle( m_nTileSize * pos.x, m_nTileSize * pos.y, m_nTileSize, m_nTileSize ) );

		auto& rnd = SRand::Inst<eRand_Render>();
		uint32 nTex = m_nTileBegin[nType] + rnd.Rand( 0u, m_nTileCount[nType] );
		uint32 texY = nTex / m_nTexCols;
		uint32 texX = nTex - texY * m_nTexCols;
		pImg->SetTexRect( CRectangle( texX * 1.0f / m_nTexCols, ( m_nTexRows - 1 - texY ) * 1.0f / m_nTexCols, 1.0f / m_nTexCols, 1.0f / m_nTexRows ) );

		uint16 nParamCount;
		CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
		CVector4* dst = pImg->GetParam();
		if( nParamCount )
			memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
	}
}

void CDecoratorTile1::Init( const CVector2& size, SChunk* pPreParent )
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

	auto& rnd = SRand::Inst<eRand_Render>();
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

			uint32 nTex = m_nTileBegin[nType] + rnd.Rand( 0u, m_nTileCount[nType] );
			uint32 texY = nTex / m_nTexCols;
			uint32 texX = nTex - texY * m_nTexCols;
			pImg->SetTexRect( CRectangle( texX * 1.0f / m_nTexCols, ( m_nTexRows - 1 - texY ) * 1.0f / m_nTexCols, 1.0f / m_nTexCols, 1.0f / m_nTexRows ) );

			uint16 nParamCount;
			CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
			CVector4* dst = pImg->GetParam();
			if( nParamCount )
				memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
		}
	}

	SetRenderObject( NULL );
}

void CDecoratorTile2::Init( const CVector2& size, SChunk* pPreParent )
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
	auto pChunk = pChunkObject->GetChunk();

	vector<bool> vecMask;
	vector<pair<uint8, uint8> > vecType;
	vecMask.resize( pChunk->nWidth * pChunk->nHeight );
	vecType.resize( ( pChunk->nWidth + 1 ) * ( pChunk->nHeight + 1 ) );

	for( int i = 0; i < pChunk->nWidth; i++ )
	{
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			uint8 n = pChunkObject->GetBlock( i, j )->nTag;
			if( n >= m_nBlockTag )
				continue;
			vecMask[i + j * pChunk->nWidth] = true;
		}
	}

	auto Set = [=, &vecType] ( int32 i, int32 j, int8 nDir )
	{
		switch( nDir )
		{
		case 0:
			vecType[( i + 1 ) + j * ( pChunk->nWidth + 1 )].first = 1;
			break;
		case 1:
			vecType[i + ( j + 1 ) * ( pChunk->nWidth + 1 )].second = 1;
			break;
		case 2:
			vecType[i + j * ( pChunk->nWidth + 1 )].first = 1;
			break;
		case 3:
			vecType[i + j * ( pChunk->nWidth + 1 )].second = 1;
			break;
		}
	};

	int32 nDirs[4] = { 0, 1, 2, 3 };
	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	auto& rnd = SRand::Inst<eRand_Render>();
	rnd.Shuffle( nDirs, 4 );
	int32 nDirCount = rnd.Rand( 1, 4 );
	for( int iDir = 0; iDir < nDirCount; iDir++ )
	{
		int32 nDir = nDirs[iDir];
		int32 nDir1 = nDir ^ 1 ^ ( rnd.Rand( 0, 2 ) << 1 );
		int32 w = nDir == 0 || nDir == 2 ? pChunk->nWidth : pChunk->nHeight;
		int32 h = nDir == 0 || nDir == 2 ? pChunk->nHeight : pChunk->nWidth;
		int32 i = ( nDir & 2 ) ? w - 1 : 0;
		int32 j = rnd.Rand( 0, h );
		if( !!( nDir & 1 ) )
			swap( i, j );
		Set( i, j, nDir ^ 2 );
		while( 1 )
		{
			if( !rnd.Rand( 0, 4 ) )
				swap( nDir, nDir1 );
			Set( i, j, nDir );
			i += ofs[nDir].x;
			j += ofs[nDir].y;
			if( i < 0 || i >= pChunk->nWidth || j < 0 || j >= pChunk->nHeight )
				break;
			int8 n = vecType[( i + 1 ) + j * ( pChunk->nWidth + 1 )].first
				+ vecType[i + ( j + 1 ) * ( pChunk->nWidth + 1 )].second
				+ vecType[i + j * ( pChunk->nWidth + 1 )].first
				+ vecType[i + j * ( pChunk->nWidth + 1 )].second;
			if( n >= 2 )
				break;
		}
	}

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
			if( !vecMask[i + j * pChunk->nWidth] )
				continue;

			uint8 x1 = vecType[i + j * ( pChunk->nWidth + 1 )].first;
			uint8 y1 = vecType[i + j * ( pChunk->nWidth + 1 )].second;
			uint8 x2 = vecType[( i + 1 ) + j * ( pChunk->nWidth + 1 )].first;
			uint8 y2 = vecType[i + ( j + 1 ) * ( pChunk->nWidth + 1 )].second;
			static uint32 indexX[] = { 3, 2, 0, 1 };
			static uint32 indexY[] = { 3, 0, 2, 1 };
			uint8 nType = indexX[x1 + x2 * 2] + indexY[y1 + y2 * 2] * 4;
			if( !m_nTileCount[nType] )
				continue;

			auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
			AddChild( pImg );
			pImg->SetRect( CRectangle( i * CMyLevel::GetBlockSize(), j * CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize() ) );

			uint32 nTex = m_nTileBegin[nType] + rnd.Rand( 0u, m_nTileCount[nType] );
			uint32 texY = nTex / m_nTexCols;
			uint32 texX = nTex - texY * m_nTexCols;
			pImg->SetTexRect( CRectangle( texX * 1.0f / m_nTexCols, texY * 1.0f / m_nTexCols, 1.0f / m_nTexCols, 1.0f / m_nTexRows ) );

			uint16 nParamCount;
			CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
			CVector4* dst = pImg->GetParam();
			if( nParamCount )
				memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
		}
	}

	SetRenderObject( NULL );
}

void CDecoratorEdge1::Init( const CVector2& size, SChunk* pPreParent )
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
	auto pChunk = pChunkObject->GetChunk();
	auto pResource = static_cast<CDrawableGroup*>( GetResource() );
	auto Func = [this, pChunk] ( int32 x, int32 y ) -> bool 
	{
		auto pBlock = pChunk->GetBlock( x, y );
		if( !pBlock )
			return false;
		auto nTag = pBlock->nTag;
		return nTag >= m_nBlockTag && nTag < m_nBlockTag1;
	};
	auto Func1 = [this, pChunk] ( int32 x, int32 y ) -> bool
	{
		auto pBlock = pChunk->GetBlock( x, y );
		if( !pBlock )
			return true;
		auto nTag = pBlock->nTag;
		return nTag >= m_nBlockTag1;
	};

	vector<int32> vecTemp;
	auto& rnd = SRand::Inst<eRand_Render>();
	for( int k = 0; k < 2; k++ )
	{
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			int32 y = k ? pChunk->nHeight - 1 - j : j;
			int32 y1 = k ? y + 1 : y - 1;
			int32 i0 = 0;
			for( int i = 0; i <= pChunk->nWidth; i++ )
			{
				if( i < pChunk->nWidth && Func( i, y ) && Func1( i, y1 ) )
					continue;

				if( i > i0 )
				{
					int32 l = i0 * CMyLevel::GetBlockSize();
					int32 r = i * CMyLevel::GetBlockSize();
					if( Func1( i0 - 1, y ) )
						l += m_nCornerSize;
					if( Func1( i, y ) )
						r -= m_nCornerSize;

					int32 len = r - l;
					int32 len1 = floor( len * m_fPercent / 2 ) * 2;
					if( len1 > 0 )
					{
						int32 len2 = len - len1;
						do
						{
							int32 l1 = Min( len1, rnd.Rand( m_nMinLen / 2, m_nMaxLen / 2 + 1 ) * 2 );
							vecTemp.push_back( l1 );
							len1 -= l1;
						} while( len1 > 0 );
						int32 n1 = vecTemp.size() + 1;
						for( int i1 = 0; i1 < n1; i1++ )
						{
							int32 l1 = ( ( len2 / 2 + i1 ) / n1 ) * 2;
							if( l1 > 0 )
								vecTemp.push_back( -l1 );
						}
						rnd.Shuffle( vecTemp );

						for( auto item : vecTemp )
						{
							if( item > 0 )
							{
								CRectangle rect( l, ( y + k * 0.5f ) * CMyLevel::GetBlockSize(), item, CMyLevel::GetBlockSize() / 2 );
								int32 ty = rnd.Rand( Min( 3, item / 32 ), 4 );
								int32 tx = rnd.Rand( m_nCornerSize / 2, ( ty + 1 ) * 32 - m_nCornerSize / 2 - item / 2 + 1 );
								CRectangle texRect( ( 3 - ty ) * 0.125f + tx / 128.0f, ( k ? ty + 4.5f : 3 - ty ) * 0.125f, rect.width / 256.0f, rect.height / 256.0f );
								texRect.y = 1 - texRect.y - texRect.height;
								auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
								pImg->SetRect( rect );
								pImg->SetTexRect( texRect );
								uint16 nParamCount;
								CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
								if( nParamCount )
								{
									CVector4* dst = pImg->GetParam();
									memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
								}
								AddChild( pImg );
								l += item;
							}
							else
								l += -item;
						}

						vecTemp.resize( 0 );
					}
				}
				i0 = i + 1;
			}
		}

		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			int32 x = k ? pChunk->nWidth - 1 - i : i;
			int32 x1 = k ? x + 1 : x - 1;
			int32 j0 = 0;
			for( int j = 0; j <= pChunk->nHeight; j++ )
			{
				if( j < pChunk->nHeight && Func( x, j ) && Func1( x1, j ) )
					continue;

				if( j > j0 )
				{
					int32 l = j0 * CMyLevel::GetBlockSize();
					int32 r = j * CMyLevel::GetBlockSize();
					if( Func1( x, j0 - 1 ) )
						l += m_nCornerSize;
					if( Func1( x, j ) )
						r -= m_nCornerSize;

					int32 len = r - l;
					int32 len1 = floor( len * m_fPercent / 2 ) * 2;
					if( len1 > 0 )
					{
						int32 len2 = len - len1;
						do
						{
							int32 l1 = Min( len1, rnd.Rand( m_nMinLen / 2, m_nMaxLen / 2 + 1 ) * 2 );
							vecTemp.push_back( l1 );
							len1 -= l1;
						} while( len1 > 0 );
						int32 n1 = vecTemp.size() + 1;
						for( int i1 = 0; i1 < n1; i1++ )
						{
							int32 l1 = ( ( len2 / 2 + i1 ) / n1 ) * 2;
							if( l1 > 0 )
								vecTemp.push_back( -l1 );
						}
						rnd.Shuffle( vecTemp );

						for( auto item : vecTemp )
						{
							if( item > 0 )
							{
								CRectangle rect( ( x + k * 0.5f ) * CMyLevel::GetBlockSize(), l, CMyLevel::GetBlockSize() / 2, item );
								int32 tx = rnd.Rand( Min( 3, item / 32 ), 4 );
								int32 ty = rnd.Rand( m_nCornerSize / 2, ( tx + 1 ) * 32 - m_nCornerSize / 2 - item / 2 + 1 );
								CRectangle texRect( ( k ? tx + 4.5f : 3 - tx ) * 0.125f, ( 3 - tx ) * 0.125f + ty / 128.0f, rect.width / 256.0f, rect.height / 256.0f );
								texRect.y = 1 - texRect.y - texRect.height;
								auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
								pImg->SetRect( rect );
								pImg->SetTexRect( texRect );
								uint16 nParamCount;
								CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
								if( nParamCount )
								{
									CVector4* dst = pImg->GetParam();
									memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
								}
								AddChild( pImg );
								l += item;
							}
							else
								l += -item;
						}

						vecTemp.resize( 0 );
					}
				}
				j0 = j + 1;
			}
		}
	}

	for( int k1 = 0; k1 < 2; k1++ )
	{
		for( int k2 = 0; k2 < 2; k2++ )
		{
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				int32 x = k1 ? pChunk->nWidth - 1 - i : i;
				int32 x1 = k1 ? x + 1 : x - 1;
				for( int j = 0; j < pChunk->nHeight; j++ )
				{
					int32 y = k2 ? pChunk->nHeight - 1 - j : j;
					int32 y1 = k2 ? y + 1 : y - 1;

					if( Func( x, y ) && Func1( x, y1 ) && Func1( x1, y ) )
					{
						CRectangle rect( ( x + k1 * 0.5f ) * CMyLevel::GetBlockSize(), ( y + k2 * 0.5f ) * CMyLevel::GetBlockSize(), CMyLevel::GetBlockSize() / 2, CMyLevel::GetBlockSize() / 2 );
						int32 t = rnd.Rand( 0, 4 );
						CRectangle texRect( ( k1 ? t + 4.5f : 3 - t ) * 0.125f, ( k2 ? t + 4.5f : 3 - t ) * 0.125f, rect.width / 256.0f, rect.height / 256.0f );
						texRect.y = 1 - texRect.y - texRect.height;
						auto pImg = static_cast<CImage2D*>( pResource->CreateInstance() );
						pImg->SetRect( rect );
						pImg->SetTexRect( texRect );
						uint16 nParamCount;
						CVector4* src = static_cast<CImage2D*>( GetRenderObject() )->GetParam( nParamCount );
						if( nParamCount )
						{
							CVector4* dst = pImg->GetParam();
							memcpy( dst, src, nParamCount * sizeof( CVector4 ) );
						}
						AddChild( pImg );
					}
				}
			}
		}
	}

	SetRenderObject( NULL );
}

void CDecoratorLabel::Init( const CVector2& size, SChunk* pPreParent )
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

	auto& rnd = SRand::Inst<eRand_Render>();
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
	auto initRect = pImage2D->GetElem().rect;
	auto initTexRect = pImage2D->GetElem().texRect;
	auto pRenderObject = new CRenderObject2D;
	SetRenderObject( pRenderObject );

	TVector2<int32> size1( floor( initRect.width * ( 1 + rnd.Rand( 0.0f, m_fMarginPercent ) ) * 0.5f + 0.5f ) * 2,
		floor( initRect.height * ( 1 + rnd.Rand( 0.0f, m_fMarginPercent ) ) * 0.5f + 0.5f ) * 2 );

	int32 nWidth = size.x / size1.x;
	int32 nHeight = size.y / size1.y;
	pRenderObject->SetPosition( CVector2( size.x - nWidth * size1.x, size.y - nHeight * size1.y ) * 0.5f );
	auto AddImage = [=] ( const TRectangle<int32>& rect, int8 nType, int8 nType1, CVector2 ofs )
	{
		int32 n1 = 2;
		auto& rnd = SRand::Inst<eRand_Render>();
		int8 b1 = rnd.Rand( 0, 2 );
		int32 s = rect.width * rect.height;

		auto color = m_param[nType] + ( m_param1[nType] - m_param[nType] )
			* CVector4( rnd.Rand( 0.0f, 1.0f ), rnd.Rand( 0.0f, 1.0f ), rnd.Rand( 0.0f, 1.0f ), rnd.Rand( 0.0f, 1.0f ) );
		for( int i = 0; i < rect.width; i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				auto pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
				pImage->SetRect( initRect.Offset( ofs + CVector2( size1.x * ( b1 ? i + rect.x : rect.GetRight() - 1 - i ), size1.y * j ) ) );
				bool b = rnd.Rand( 0, s ) < n1;
				int32 n = b ? rnd.Rand( 0, 10 ) : rnd.Rand( 10, 36 );
				if( !b )
					n1 *= 2;
				pImage->SetTexRect( CRectangle( initTexRect.x + ( n % 8 ) * initTexRect.width + ( nType1 & 1 ) * 0.5f,
					initTexRect.y + ( n / 8 ) * initTexRect.height + ( nType1 >> 1 ) * 0.5f,
					initTexRect.width, initTexRect.height ) );
				uint16 nParam;
				CVector4* pParam = pImage->GetParam( nParam );
				if( nParam )
					*pParam = color;
				pRenderObject->AddChild( pImage );
			}
		}
	};

	auto pChunk = pChunkObject->GetChunk();
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	vector<TVector2<int32> > vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			CRectangle r( x + pRenderObject->x + i * size1.x, y + pRenderObject->y + j * size1.y, size1.x, size1.y );
			int32 i1 = floor( r.x / CMyLevel::GetBlockSize() );
			int32 j1 = floor( r.y / CMyLevel::GetBlockSize() );
			int32 i2 = ceil( r.GetRight() / CMyLevel::GetBlockSize() );
			int32 j2 = ceil( r.GetBottom() / CMyLevel::GetBlockSize() );
			bool b = true;
			for( int x1 = i1; x1 < i2 && b; x1++ )
			{
				for( int y1 = j1; y1 < j2; y1++ )
				{
					if( pChunk->GetBlock( x1, y1 )->nTag )
					{
						b = false;
						break;
					}
				}
			}
			if( b )
				vec.push_back( TVector2<int32>( i, j ) );
			else
				vecTemp[i + j * nWidth] = 1;
		}
	}
	rnd.Shuffle( vec );
	int32 nCount0[ELEM_COUNT( m_param )];
	memset( nCount0, 0, sizeof( nCount0 ) );
	CalcCount( pChunk, nCount0 );
	int32 nCount[ELEM_COUNT( m_param )];
	memcpy( nCount, nCount0, sizeof( nCount ) );
	int32 s = 0;
	for( int i = 0; i < ELEM_COUNT( m_param ); i++ )
		s += nCount[i];
	float f = m_fPercent;
	f = f * nWidth * nHeight / Max<int32>( pChunk->nWidth * pChunk->nHeight, s );
	for( int i = 0; i < ELEM_COUNT( m_param ); i++ )
		nCount[i] = floor( nCount[i] * f );

	for( int i0 = 0; i0 < vec.size(); i0++ )
	{
		auto p = vec[i0];
		if( vecTemp[p.x + p.y * nWidth] )
			continue;
		int32 a = rnd.Rand<int32>( 0, ELEM_COUNT( m_param ) );
		int32 a1 = rnd.Rand<int32>( 0, 4 );
		bool b = false;
		for( int i = 0; i < 4; i++ )
		{
			if( nCount[a] > 0 )
			{
				b = true;
				break;
			}
			a++;
			if( a >= ELEM_COUNT( m_param ) )
				a = 0;
		}
		if( !b )
			break;
		int32 n = Min( nWidth, Max( 3, Min( rnd.Rand( 5, 9 ), nCount[a] ) ) );
		auto rect = PutRect( vecTemp, nWidth, nHeight, p, TVector2<int32>( 3, 1 ), TVector2<int32>( n, 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
		if( !rect.width )
			continue;
		CVector2 ofs( rnd.Rand<int32>( 0, ( size1.x - initRect.width ) / 2 + 1 ) * 2, rnd.Rand<int32>( 0, ( size1.y - initRect.height ) / 2 + 1 ) * 2 );
		AddImage( rect, a, a1, ofs );
		nCount[a] -= rect.width;
		int32 k1 = rnd.Rand( 0, 2 );
		for( int k = 0; k < 2; k++ )
		{
			int32 nDir = !!( k1 ^ k ) ? 1 : -1;
			int32 m1 = rnd.Rand( 0, 2 );
			for( int m = 0; m < 2; m++ )
			{
				int32 x = !!( m1 ^ m ) ? rect.x : rect.GetRight() - 1;
				for( int y = p.y + nDir; y >= 0 && y < nHeight && nCount[a] > 0; y += nDir )
				{
					if( rnd.Rand( 0, 2 ) || vecTemp[x + y * nWidth] )
						break;
					int32 n = Min( rnd.Rand( 1, rect.width ), nCount[a] );
					auto rect1 = PutRect( vecTemp, nWidth, nHeight, TVector2<int32>( x, y ), TVector2<int32>( 1, 1 ), TVector2<int32>( n, 1 ), TRectangle<int32>( rect.x, 0, rect.width, nHeight ), -1, 1 );
					AddImage( rect1, a, a1, ofs );
					nCount[a] -= rect1.width;
				}
			}
		}
	}

	if( m_p1 )
	{
		pRenderObject = new CRenderObject2D;
		AddChild( pRenderObject );
		int32 nBlockSize = CMyLevel::GetBlockSize();
		nWidth = size.x / nBlockSize;
		nHeight = size.y / nBlockSize;
		pRenderObject->SetPosition( CVector2( size.x - nWidth * nBlockSize, size.y - nHeight * nBlockSize ) * 0.5f );
		vecTemp.resize( nWidth * nHeight );
		vec.resize( 0 );
		for( int i = 0; i < nWidth; i++ )
		{
			for( int j = 0; j < nHeight; j++ )
			{
				CRectangle r( x + pRenderObject->x + i * nBlockSize, y + pRenderObject->y + j * nBlockSize, nBlockSize, nBlockSize );
				int32 i1 = floor( r.x / CMyLevel::GetBlockSize() );
				int32 j1 = floor( r.y / CMyLevel::GetBlockSize() );
				int32 i2 = ceil( r.GetRight() / CMyLevel::GetBlockSize() );
				int32 j2 = ceil( r.GetBottom() / CMyLevel::GetBlockSize() );
				bool b = true;
				for( int x1 = i1; x1 < i2 && b; x1++ )
				{
					for( int y1 = j1; y1 < j2; y1++ )
					{
						if( pChunk->GetBlock( x1, y1 )->nTag )
						{
							b = false;
							break;
						}
					}
				}
				if( b )
				{
					vecTemp[i + j * nWidth] = 0;
					vec.push_back( TVector2<int32>( i, j ) );
				}
				else
					vecTemp[i + j * nWidth] = 1;
			}
		}
		rnd.Shuffle( vec );

		memcpy( nCount, nCount0, sizeof( nCount ) );
		s = 0;
		for( int i = 0; i < ELEM_COUNT( m_param ); i++ )
			s += nCount[i];
		f = m_fPercent;
		f = f * nWidth * nHeight / Max<int32>( pChunk->nWidth * pChunk->nHeight, s );
		for( int i = 0; i < ELEM_COUNT( m_param ); i++ )
			nCount[i] = ceil( nCount[i] * f );

		for( auto& p : vec )
		{
			if( vecTemp[p.x + p.y * nWidth] )
				continue;

			int32 a = rnd.Rand<int32>( 0, 3 );
			bool b = false;
			for( int i = 0; i < 3; i++ )
			{
				if( nCount[a] > 0 )
				{
					b = true;
					break;
				}
				a++;
				if( a >= 3 )
					a = 0;
			}
			if( !b )
				break;

			TVector2<int32> size;
			if( a == 0 )
				size = nCount[a] >= 2 && rnd.Rand( 0, 2 ) ? TVector2<int32>( 2, 1 ) : TVector2<int32>( 1, 1 ) ;
			else
				size = nCount[a] >= 4 && rnd.Rand( 0, 2 ) ? TVector2<int32>( 2, 2 ) : TVector2<int32>( 2, 1 );
			auto rect = PutRect( vecTemp, nWidth, nHeight, p, size, size, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
			if( rect.width <= 0 )
				continue;

			auto pImage = static_cast<CImage2D*>( m_p1->CreateInstance() );
			pImage->SetRect( CRectangle( rect.x, rect.y, rect.width, rect.height ) * nBlockSize );
			nCount[a] -= rect.width * rect.height;
			if( a == 0 )
			{
				if( rect.width == 2 )
					pImage->SetTexRect( CRectangle( rnd.Rand( 0, 2 ) * 0.25f, rnd.Rand( 2, 4 ) * 0.125f, 0.25f, 0.125f ) );
				else
					pImage->SetTexRect( CRectangle( rnd.Rand( 0, 4 ) * 0.125f, rnd.Rand( 0, 2 ) * 0.125f, 0.125f, 0.125f ) );
			}
			else if( a == 1 )
			{
				if( rect.height == 2 )
					pImage->SetTexRect( CRectangle( rnd.Rand( 0, 2 ) * 0.25f, rnd.Rand( 2, 4 ) * 0.25f, 0.25f, 0.25f ) );
				else
					pImage->SetTexRect( CRectangle( rnd.Rand( 2, 4 ) * 0.25f, rnd.Rand( 6, 8 ) * 0.125f, 0.25f, 0.125f ) );
			}
			else
			{
				if( rect.height == 2 )
					pImage->SetTexRect( CRectangle( rnd.Rand( 2, 4 ) * 0.25f, rnd.Rand( 1, 3 ) * 0.25f, 0.25f, 0.25f ) );
				else
					pImage->SetTexRect( CRectangle( rnd.Rand( 2, 4 ) * 0.25f, rnd.Rand( 0, 2 ) * 0.125f, 0.25f, 0.125f ) );
			}
			pRenderObject->AddChild( pImage );
		}
	}
}

void CDecoratorLabel::CalcCount( SChunk* pChunk, int32* nCount )
{
	for( int i = 0; i < ELEM_COUNT( m_param ); i++ )
	{
		if( !!( pChunk->nChunkTag & ( 1 << i ) ) )
			nCount[i] += pChunk->nWidth * pChunk->nHeight;
	}
	for( auto pSubChunk = pChunk->Get_SubChunk(); pSubChunk; pSubChunk = pSubChunk->NextSubChunk() )
	{
		if( pSubChunk->nSubChunkType != 0 )
			continue;
		CalcCount( pSubChunk, nCount );
	}
}
