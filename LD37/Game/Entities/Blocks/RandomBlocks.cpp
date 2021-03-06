#include "stdafx.h"
#include "RandomBlocks.h"
#include "Common/Rand.h"
#include "Render/DrawableGroup.h"
#include "Entities/EffectObject.h"
#include "MyLevel.h"


void CRandomChunk0::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
	auto rect = pImage2D->GetElem().rect;
	auto texRect = pImage2D->GetElem().texRect;
	uint32 nTileX = rect.width / ( 32 * m_nWidth );
	uint32 nTileY = rect.height / ( 32 * m_nHeight );
	rect.width = 32 * m_nWidth;
	rect.height = 32 * m_nHeight;
	texRect.width /= nTileX;
	texRect.height /= nTileY;

	int32 rx = SRand::Inst().Rand( 0u, nTileX );
	int32 ry = SRand::Inst().Rand( 0u, nTileY );

	pImage2D->SetRect( rect );
	pImage2D->SetTexRect( texRect.Offset( CVector2( rx * texRect.width, ry * texRect.height ) ) );
	CChunkObject::OnSetChunk( pChunk, pLevel );
}

void CRandomChunkTiledSimple::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}

	auto rect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;

	CRectangle texRect;
	int32 nX = pChunk->nWidth > 1 ? 2 : 1;
	int32 nY = pChunk->nHeight > 1 ? 2 : 1;
	if( nX > 1 && nY > 1 )
		texRect = m_texRect4;
	else if( nX > 1 )
		texRect = m_texRect2;
	else if( nY > 1 )
		texRect = m_texRect3;
	else
		texRect = m_texRect1;

	uint32 nTileX = rect0.width / 32;
	uint32 nTileY = rect0.height / 32;
	texRect0.width /= nTileX;
	texRect0.height /= nTileY;
	int32 nAltX = texRect.width / ( nX * texRect0.width );
	int32 nAltY = texRect.height / ( nY * texRect0.height );
	texRect.x += texRect.width / nAltX * SRand::Inst().Rand( 0, nAltX );
	texRect.y += texRect.height / nAltY * SRand::Inst().Rand( 0, nAltY );
	texRect.width = texRect0.width;
	texRect.height = texRect0.height;

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );

			float tX = iX == 0 ? 0 : ( iX == pChunk->nWidth - 1 ? 1 : 0.5f );
			float tY = iY == 0 ? 0 : ( iY == pChunk->nHeight - 1 ? 1 : 0.5f );

			auto tex = texRect.Offset( CVector2( texRect.width * tX, texRect.height * ( nY - 1 - tY ) ) );
			pImage2D->SetTexRect( tex );
			GetRenderObject()->AddChild( pImage2D );
			GetBlock( iX, iY )->rtTexRect = tex;

			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomChunkTiledSimple::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}


void CRandomChunkTiled::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}

	auto pImage2D = static_cast<CImage2D*>( GetRenderObject() );
	auto rect = pImage2D->GetElem().rect;
	auto texRect = pImage2D->GetElem().texRect;
	auto texRect1 = pImage2D->GetElem().texRect;
	texRect1.x += Max( 1u, m_nTypeX ) * texRect1.width;
	if( m_nTypeX > 1 )
	{
		texRect.x += SRand::Inst().Rand( 0u, m_nTypeX ) * texRect.width;
		texRect1.x += SRand::Inst().Rand( 0u, m_nTypeX ) * texRect.width;
	}
	if( m_nTypeY > 1 )
	{
		texRect.y += SRand::Inst().Rand( 0u, m_nTypeY ) * texRect.height;
		texRect1.y += SRand::Inst().Rand( 0u, m_nTypeY ) * texRect.height;
	}
	uint32 nTileX = rect.width / 32;
	uint32 nTileY = rect.height / 32;
	texRect.width /= nTileX;
	texRect.height /= nTileY;
	texRect1.width /= nTileX;
	texRect1.height /= nTileY;

	TRectangle<int32> r( 0, 0, pChunk->nWidth, pChunk->nHeight );
	bool bTag1 = false;
	if( m_nGroupType )
	{
		if( SRand::Inst().Rand( 0, 2 ) )
		{
			swap( texRect, texRect1 );
			bTag1 = true;
		}
		uint32 n1 = SRand::Inst().Rand( 1, 4 );
		if( !!( n1 & 1 ) && r.width >= 2 )
		{
			switch( SRand::Inst().Rand( 0, 3 ) )
			{
			case 0:
				r.SetRight( r.x + SRand::Inst().Rand( 1, r.width ) );
				break;
			case 1:
				r.SetLeft( r.GetRight() - SRand::Inst().Rand( 1, r.width ) );
				break;
			case 2:
				r.width = SRand::Inst().Rand( 1, r.width );
				r.x += SRand::Inst().Rand( 0u, pChunk->nWidth - r.width + 1 );
				break;
			}
		}
		if( !!( n1 & 2 ) && r.height >= 2 )
		{
			switch( SRand::Inst().Rand( 0, 3 ) )
			{
			case 0:
				r.SetBottom( r.y + SRand::Inst().Rand( 1, r.height ) );
				break;
			case 1:
				r.SetTop( r.GetBottom() - SRand::Inst().Rand( 1, r.height ) );
				break;
			case 2:
				r.height = SRand::Inst().Rand( 1, r.height );
				r.y += SRand::Inst().Rand( 0u, pChunk->nHeight - r.height + 1 );
				break;
			}
		}
	}

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			uint32 tX = SRand::Inst().Rand( 0u, nTileX );
			uint32 tY = SRand::Inst().Rand( 0u, nTileY );
			bool b = iX >= r.x && iY >= r.y && iX < r.GetRight() && iY < r.GetBottom();
			auto t0 = b ? texRect : texRect1;
			auto tex = t0.Offset( CVector2( t0.width * tX, t0.height * tY ) );
			if( m_bBlockTypeMask[pChunk->GetBlock( iX, iY )->eBlockType] )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );

				pImage2D->SetTexRect( tex );
				GetRenderObject()->AddChild( pImage2D );
			}
			if( bTag1 ? b : !b )
				GetBlock( iX, iY )->nTag = 1;
			GetBlock( iX, iY )->rtTexRect = tex;

			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomChunkTiled::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CRandomChunkTiled1::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			uint8 nType = pChunk->GetBlock( iX, iY )->eBlockType;
			CRectangle texRect = m_texRects[nType];
			uint32 sizeX = m_sizeX[nType];
			uint32 sizeY = m_sizeY[nType];
			uint32 tX = SRand::Inst().Rand( 0u, sizeX );
			uint32 tY = SRand::Inst().Rand( 0u, sizeY );
			if( sizeX && sizeY )
			{
				texRect.width /= sizeX;
				texRect.height /= sizeY;
				auto tex = texRect.Offset( CVector2( texRect.width * tX, texRect.height * tY ) );
				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );

				pImage2D->SetTexRect( tex );
				GetRenderObject()->AddChild( pImage2D );
				GetBlock( iX, iY )->rtTexRect = tex;
			}

			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomChunkTiled1::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CRandomChunk1::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}

	auto rect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	uint32 nAltX = Max( 1u, m_nAltX );
	uint32 nAltY = Max( 1u, m_nAltY );
	uint32 nTileX = rect.width / 32;
	uint32 nTileY = rect.height / 32;
	texRect.width /= nTileX;
	texRect.height /= nTileY;
	uint32 nTileX1 = nTileX / nAltX;
	uint32 nTileY1 = nTileY / nAltY;

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );

			uint32 tX = iX == 0 ? 0 : ( iX == pChunk->nWidth - 1 ? nTileX1 - 1 : SRand::Inst().Rand( 1u, nTileX1 - 1 ) );
			uint32 tY = iY == 0 ? 0 : ( iY == pChunk->nHeight - 1 ? nTileY1 - 1 : SRand::Inst().Rand( 1u, nTileY1 - 1 ) );
			tX += nTileX1 * SRand::Inst().Rand( 0u, nAltX );
			tY += nTileY1 * SRand::Inst().Rand( 0u, nAltY );

			auto tex = texRect.Offset( CVector2( texRect.width * tX, texRect.height * ( nTileY - 1 - tY ) ) );
			pImage2D->SetTexRect( tex );
			GetRenderObject()->AddChild( pImage2D );
			GetBlock( iX, iY )->rtTexRect = tex;

			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomChunk1::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CRandomChunk2::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	auto rect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	uint32 nTileX = rect.width / 32;
	uint32 nTileY = rect.height / 32;
	texRect.width /= nTileX;
	texRect.height /= nTileY;

	int32 nLength = !m_bVertical ? pChunk->nWidth : pChunk->nHeight;
	if( nLength >= 2 )
	{
		SetRenderObject( new CRenderObject2D );
		uint32 nSeg2Count = m_nTexRect2X && m_nTexRect2Y ? SRand::Inst<eRand_Render>().Rand( 0, nLength / 2 ) : 0;
		uint32 nTotalSegCount = nLength - nSeg2Count;
		bool* bSeg2 = (bool*)alloca( nTotalSegCount );
		memset( bSeg2, 0, nTotalSegCount );
		if( m_nTexRect2X && m_nTexRect2Y )
		{
			for( int i = 0; i < nSeg2Count; i++ )
			{
				bSeg2[i] = true;
			}
		}
		SRand::Inst<eRand_Render>().Shuffle( bSeg2, nTotalSegCount );

		int nCurLen = 0;
		int8 b = SRand::Inst<eRand_Render>().Rand( 0, 2 );
		int8 bDir = SRand::Inst<eRand_Render>().Rand( 0, 2 );
		for( int i = 0; i < nTotalSegCount; i++ )
		{
			int8 b1;
			if( m_nFlip > 0 )
				b1 = SRand::Inst<eRand_Render>().Rand( 0, m_nFlip + 2 ) ? b : !b;
			else if( m_nFlip < 0 )
				b1 = SRand::Inst<eRand_Render>().Rand( 0, m_nFlip + 2 ) ? !b : b;
			else
				b1 = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			int32 nSubTex;
			if( i == 0 )
				nSubTex = b1 ? 1 : 0;
			else if( i == nTotalSegCount - 1 )
				nSubTex = b ? 1 : 0;
			else
			{
				if( bDir )
					nSubTex = b ? ( b1 ? 2 : 1 ) : ( b1 ? 3 : 0 );
				else
					nSubTex = b ? ( b1 ? 2 : 3 ) : ( b1 ? 1 : 0 );
			}
			b = b1;
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			bool b2 = bSeg2[i];
			TRectangle<int32> rect = !m_bVertical ? TRectangle<int32>( nCurLen, 0, b2 ? 2 : 1, m_nWidth )
				: TRectangle<int32>( 0, nCurLen, m_nWidth, b2 ? 2 : 1 );
			if( bDir )
			{
				if( !m_bVertical )
					rect.x = pChunk->nWidth - rect.GetRight();
				else
					rect.y = pChunk->nHeight - rect.GetBottom();
			}
			pImage2D->SetRect( CRectangle( rect.x * CMyLevel::GetBlockSize(), rect.y * CMyLevel::GetBlockSize(), rect.width * CMyLevel::GetBlockSize(), rect.height * CMyLevel::GetBlockSize() ) );
			nCurLen += b2 ? 2 : 1;

			CRectangle texRect;
			int32 i1 = bDir ? nTotalSegCount - 1 - i : i;
			if( i1 == 0 || i1 == nTotalSegCount - 1 )
			{
				if( b2 )
				{
					texRect = m_texRect2End;
					texRect.width /= m_nTexRect2EndX;
					texRect.height /= m_nTexRect2EndY;
					int32 n = SRand::Inst<eRand_Render>().Rand( 0u, m_nTexRect2EndX * m_nTexRect2EndY / 2 ) + m_nTexRect2EndX * m_nTexRect2EndY / 2 * nSubTex;
					texRect.x += texRect.width * ( m_bVertical ? n / m_nTexRect2EndY : n % m_nTexRect2EndX );
					texRect.y += texRect.height * ( m_bVertical ? n % m_nTexRect2EndY : n / m_nTexRect2EndX );
				}
				else
				{
					texRect = m_texRect1End;
					texRect.width /= m_nTexRect1EndX;
					texRect.height /= m_nTexRect1EndY;
					int32 n = SRand::Inst<eRand_Render>().Rand( 0u, m_nTexRect1EndX * m_nTexRect1EndY / 2 ) + m_nTexRect1EndX * m_nTexRect1EndY / 2 * nSubTex;
					texRect.x += texRect.width * ( m_bVertical ? n / m_nTexRect1EndY : n % m_nTexRect1EndX );
					texRect.y += texRect.height * ( m_bVertical ? n % m_nTexRect1EndY : n / m_nTexRect1EndX );
				}

				if( !m_bVertical )
				{
					texRect.width *= 0.5f;
					if( i1 == nTotalSegCount - 1 )
						texRect.x += texRect.width;
				}
				else
				{
					texRect.height *= 0.5f;
					if( i1 == 0 )
						texRect.y += texRect.height;
				}
			}
			else
			{
				if( b2 )
				{
					texRect = m_texRect2;
					texRect.width /= m_nTexRect2X;
					texRect.height /= m_nTexRect2Y;
					int32 n = SRand::Inst<eRand_Render>().Rand( 0u, m_nTexRect2X * m_nTexRect2Y / 4 ) * 4 + nSubTex;
					texRect.x += texRect.width * ( m_bVertical ? n / m_nTexRect2Y : n % m_nTexRect2X );
					texRect.y += texRect.height * ( m_bVertical ? n % m_nTexRect2Y : n / m_nTexRect2X );
				}
				else
				{
					texRect = m_texRect1;
					texRect.width /= m_nTexRect1X;
					texRect.height /= m_nTexRect1Y;
					int32 n = SRand::Inst<eRand_Render>().Rand( 0u, m_nTexRect1X * m_nTexRect1Y / 4 ) * 4 + nSubTex;
					texRect.x += texRect.width * ( m_bVertical ? n / m_nTexRect1Y : n % m_nTexRect1X );
					texRect.y += texRect.height * ( m_bVertical ? n % m_nTexRect1Y : n / m_nTexRect1X );
				}
			}

			pImage2D->SetTexRect( texRect );
			GetRenderObject()->AddChild( pImage2D );
			for( int iX = 0; iX < rect.width; iX++ )
			{
				for( int iY = 0; iY < rect.height; iY++ )
				{
					auto& rtRect = GetBlock( iX + rect.x, iY + rect.y )->rtTexRect;
					rtRect = texRect;
					rtRect.width /= rect.width;
					rtRect.height /= rect.height;
					rtRect.x += rtRect.width * iX;
					rtRect.y += rtRect.height * ( rect.height - 1 - iY );
				}
			}
		}
	}
	else
	{
		CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImage2D->SetRect( !m_bVertical ? CRectangle( 0, 0, 32, m_nWidth * 32 )
			: CRectangle( 0, 0, m_nWidth * 32, 32 ) );
		CRectangle texRect;
		texRect = m_texRect1;
		texRect.width /= m_nTexRect1X;
		texRect.x += texRect.width * SRand::Inst<eRand_Render>().Rand( 0u, m_nTexRect1X );
		texRect.height /= m_nTexRect1Y;
		texRect.y += texRect.height * SRand::Inst<eRand_Render>().Rand( 0u, m_nTexRect1Y );
		pImage2D->SetTexRect( texRect );
		SetRenderObject( pImage2D );
		CChunkObject::OnSetChunk( pChunk, pLevel );
	}

	m_nMaxHp += m_nHpPerLength * nLength;
	m_fHp = m_nMaxHp;
}

void CRandomChunk2::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CRandomChunk3::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	vector<bool> vecMask;
	vector<pair<uint8, uint8> > vecType;
	vecMask.resize( pChunk->nWidth * pChunk->nHeight );
	vecType.resize( ( pChunk->nWidth + 1 ) * ( pChunk->nHeight + 1 ) );
	
	for( int i = 0; i < vecMask.size(); i++ )
		vecMask[i] = m_bBlockTypeMask[pChunk->blocks[i].eBlockType];
	for( int i = 0; i < vecType.size(); i++ )
	{
		vecType[i].first = SRand::Inst().Rand( -1.0f, 1.0f ) + m_fXCoef >= 0 ? 1 : 0;
		vecType[i].second = SRand::Inst().Rand( -1.0f, 1.0f ) + m_fYCoef >= 0 ? 1 : 0;
	}

	auto rect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	SetRenderObject( new CRenderObject2D );
	uint32 nTileX = rect.width / 32;
	uint32 nTileY = rect.height / 32;
	texRect.width /= nTileX;
	texRect.height /= nTileY;
	uint32 nAltX = nTileX / 4;
	uint32 nAltY = nTileY / 4;

	for( int j = 0; j < pChunk->nHeight; j++ )
	{
		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			if( vecMask[i + j * pChunk->nWidth] )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );

				uint8 x1 = vecType[i + j * ( pChunk->nWidth + 1 )].first;
				uint8 y1 = vecType[i + j * ( pChunk->nWidth + 1 )].second;
				uint8 x2 = vecType[( i + 1 ) + j * ( pChunk->nWidth + 1 )].first;
				uint8 y2 = vecType[i + ( j + 1 ) * ( pChunk->nWidth + 1 )].second;

				static uint32 indexX[] = { 3, 2, 0, 1 };
				static uint32 indexY[] = { 3, 0, 2, 1 };
				uint32 tX = indexX[x1 + x2 * 2];
				uint32 tY = indexY[y1 + y2 * 2];
				tX += 4 * SRand::Inst().Rand( 0u, nAltX );
				tY += 4 * SRand::Inst().Rand( 0u, nAltY );

				auto tex = texRect.Offset( CVector2( texRect.width * tX, texRect.height * tY ) );
				pImage2D->SetTexRect( tex );
				GetRenderObject()->AddChild( pImage2D );
				GetBlock( i, j )->rtTexRect = tex;
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomChunk3::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CRandomChunk4::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		pDamageEftTex[i].width /= m_nTexWidth;
		pDamageEftTex[i].height /= m_nTexHeight;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}

	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	SetRenderObject( new CRenderObject2D );
	texRect.width /= m_nTexWidth;
	texRect.height /= m_nTexHeight;

	for( int j = 0; j < pChunk->nHeight; j++ )
	{
		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );
			uint32 tX, tY;
			if( i < m_nLeft )
				tX = i;
			else if( i >= pChunk->nWidth - m_nRight )
				tX = m_nTexWidth - pChunk->nWidth + i;
			else
				tX = SRand::Inst().Rand( m_nLeft, m_nTexWidth - m_nRight );
			if( j < m_nTop )
				tY = j;
			else if( j >= pChunk->nHeight - m_nBottom )
				tY = m_nTexHeight - pChunk->nHeight + j;
			else
				tY = SRand::Inst().Rand( m_nTop, m_nTexHeight - m_nBottom );
			tY = m_nTexHeight - 1 - tY;

			auto tex = texRect.Offset( CVector2( texRect.width * tX, texRect.height * tY ) );
			pImage2D->SetTexRect( tex );
			GetRenderObject()->AddChild( pImage2D );
			GetBlock( i, j )->rtTexRect = tex;

			for( int n = 0; n < m_nDamagedEffectsCount; n++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[n]->CreateInstance() );
				pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[n].Offset( CVector2( pDamageEftTex[n].width * tX, pDamageEftTex[n].height * tY ) ) );
				m_pDamagedEffects[n]->AddChild( pImage2D );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomChunk4::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CDefaultRandomRoom::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	auto rect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	texRect.width /= 4;
	texRect.height /= 4;
	SetRenderObject( new CRenderObject2D );
	for( int j = 0; j < pChunk->nHeight; j++ )
	{
		for( int i = 0; i < pChunk->nWidth; i++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );
			GetRenderObject()->AddChild( pImage2D );

			uint8 nType = pChunk->GetBlock( i, j )->eBlockType;
			CRectangle tex;
			if( nType != eBlockType_Block )
			{
				tex = texRect.Offset( CVector2( texRect.width * SRand::Inst().Rand( 1, 3 ), texRect.height * SRand::Inst().Rand( 1, 3 ) ) );
			}
			else
			{
				if( i == 0 && j == 0 )
				{
					if( pChunk->GetBlock( i + 1, j )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 1 ) );
					}
					else if( pChunk->GetBlock( i, j + 1 )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 2, texRect.height * 0 ) );
					}
					else
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 3 ) );
					}
				}
				else if( i == pChunk->nWidth - 1 && j == 0 )
				{
					if( pChunk->GetBlock( i - 1, j )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 1 ) );
					}
					else if( pChunk->GetBlock( i, j + 1 )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 1, texRect.height * 0 ) );
					}
					else
					{
						tex = texRect.Offset( CVector2( texRect.width * 3, texRect.height * 3 ) );
					}
				}
				else if( i == 0 && j == pChunk->nHeight - 1 )
				{
					if( pChunk->GetBlock( i + 1, j )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 2 ) );
					}
					else if( pChunk->GetBlock( i, j - 1 )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 2, texRect.height * 0 ) );
					}
					else
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 0 ) );
					}
				}
				else if( i == pChunk->nWidth - 1 && j == pChunk->nHeight - 1 )
				{
					if( pChunk->GetBlock( i - 1, j )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 1 ) );
					}
					else if( pChunk->GetBlock( i, j - 1 )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 2, texRect.height * 0 ) );
					}
					else
					{
						tex = texRect.Offset( CVector2( texRect.width * 3, texRect.height * 0 ) );
					}
				}

				else if( i == 0 || i == pChunk->nWidth - 1 )
				{
					if( pChunk->GetBlock( i, j - 1 )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 1 ) );
					}
					else if( pChunk->GetBlock( i, j + 1 )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 0, texRect.height * 2 ) );
					}
					else
					{
						tex = texRect.Offset( CVector2( texRect.width * 3, texRect.height * SRand::Inst().Rand( 1, 3 ) ) );
					}
				}
				else if( j == 0 || j == pChunk->nHeight - 1 )
				{
					if( pChunk->GetBlock( i - 1, j )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 2, texRect.height * 0 ) );
					}
					else if( pChunk->GetBlock( i + 1, j )->eBlockType != eBlockType_Block )
					{
						tex = texRect.Offset( CVector2( texRect.width * 1, texRect.height * 0 ) );
					}
					else
					{
						tex = texRect.Offset( CVector2( texRect.width * SRand::Inst().Rand( 1, 3 ), texRect.height * 3 ) );
					}
				}

			}
			pImage2D->SetTexRect( tex );
			GetBlock( i, j )->rtTexRect = tex;
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CDefaultRandomRoom::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}

void CRandomRoom1::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	CDrawableGroup* pDamageEftDrawableGroups[4];
	CRectangle pDamageEftTex[4];
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		pDamageEftDrawableGroups[i] = static_cast<CDrawableGroup*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetResource() );
		pDamageEftTex[i] = static_cast<CImage2D*>( SafeCast<CEntity>( m_pDamagedEffects[i] )->GetRenderObject() )->GetElem().texRect;
		SafeCast<CEntity>( m_pDamagedEffects[i] )->SetRenderObject( NULL );
	}

	auto rect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	texRect.width /= 4;
	texRect.height /= 4;
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
			GetBlock( iX, iY )->rtTexRect = texRect;
		}
	}

	SetRenderObject( new CRenderObject2D );

	for( int j = 1; j < pChunk->nHeight - 1; j++ )
	{
		for( int i = 1; i < pChunk->nWidth - 1; i++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( i * 32, j * 32, 32, 32 ) );
			pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * SRand::Inst().Rand( 1, 3 ), texRect.height * SRand::Inst().Rand( 1, 3 ) ) ) );
			GetRenderObject()->AddChild( pImage2D );
		}
	}

	{
		CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImage2D->SetRect( CRectangle( 0, 0, 32, 32 ) );
		pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * 0, texRect.height * 3 ) ) );
		GetRenderObject()->AddChild( pImage2D );

		pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImage2D->SetRect( CRectangle( ( pChunk->nWidth - 1 ) * 32, 0, 32, 32 ) );
		pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * 3, texRect.height * 3 ) ) );
		GetRenderObject()->AddChild( pImage2D );

		pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImage2D->SetRect( CRectangle( 0, ( pChunk->nHeight - 1 ) * 32, 32, 32 ) );
		pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * 0, texRect.height * 0 ) ) );
		GetRenderObject()->AddChild( pImage2D );

		pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImage2D->SetRect( CRectangle( ( pChunk->nWidth - 1 ) * 32, ( pChunk->nHeight - 1 ) * 32, 32, 32 ) );
		pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * 3, texRect.height * 0 ) ) );
		GetRenderObject()->AddChild( pImage2D );
	}

	for( int i = 0; i < 2; i++ )
	{
		int32 y = i * ( pChunk->nHeight - 1 );
		float ty = texRect.y + texRect.height * 3 * ( 1 - i );
		uint8 nType0 = eBlockType_Block;
		uint8 nBegin = 1;
		int32 x;
		for( x = 1; x < pChunk->nWidth - 1; x++ )
		{
			uint8 nType = pChunk->GetBlock( x, y )->eBlockType;

			if( nType0 != nType )
			{
				if( nType0 == eBlockType_Block )
				{
					int32 a = nBegin > 1 ? nBegin * 32 + 16 : 32;
					int32 b = x * 32 - 16;
					if( b > a )
					{
						CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
						pImage2D->SetRect( CRectangle( a, y * 32, b - a, 32 ) );
						pImage2D->SetTexRect( CRectangle( texRect.x + texRect.width, ty, texRect.width * 0.25f, texRect.height ) );
						GetRenderObject()->AddChild( pImage2D );
					}
				}
				else
				{
					int32 a = nBegin * 32 + 16;
					int32 b = x * 32 - 16;
					if( b > a )
					{
						CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
						pImage2D->SetRect( CRectangle( a, y * 32, b - a, 32 ) );
						pImage2D->SetTexRect( CRectangle( texRect.x + texRect.width * 1.875f, ty, texRect.width * 0.25f, texRect.height ) );
						GetRenderObject()->AddChild( pImage2D );
					}

					CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
					pImage2D->SetRect( CRectangle( nBegin * 32 - 16, y * 32, 32, 32 ) );
					pImage2D->SetTexRect( CRectangle( texRect.x + texRect.width * 1, ty, texRect.width, texRect.height ) );
					GetRenderObject()->AddChild( pImage2D );

					pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
					pImage2D->SetRect( CRectangle( x * 32 - 16, y * 32, 32, 32 ) );
					pImage2D->SetTexRect( CRectangle( texRect.x + texRect.width * 2, ty, texRect.width, texRect.height ) );
					GetRenderObject()->AddChild( pImage2D );
				}

				nBegin = x;
				nType0 = nType;
			}
		}

		int32 a = nBegin > 1 ? nBegin * 32 + 16 : 32;
		int32 b = x * 32;
		if( b > a )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( a, y * 32, b - a, 32 ) );
			pImage2D->SetTexRect( CRectangle( texRect.x + texRect.width, ty, texRect.width * 0.25f, texRect.height ) );
			GetRenderObject()->AddChild( pImage2D );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		int32 x = i * ( pChunk->nWidth - 1 );
		float tx = texRect.x + texRect.width * 3 * i;
		uint8 nType0 = eBlockType_Block;
		uint8 nBegin = 1;
		int32 y;
		for( y = 1; y < pChunk->nHeight - 1; y++ )
		{
			uint8 nType = pChunk->GetBlock( x, y )->eBlockType;

			if( nType0 != nType )
			{
				if( nType0 == eBlockType_Block )
				{
					int32 a = nBegin > 1 ? nBegin * 32 + 16 : 32;
					int32 b = y * 32 - 16;
					if( b > a )
					{
						CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
						pImage2D->SetRect( CRectangle( x * 32, a, 32, b - a ) );
						pImage2D->SetTexRect( CRectangle( tx, texRect.y + texRect.height, texRect.width, texRect.height * 0.25f ) );
						GetRenderObject()->AddChild( pImage2D );
					}
				}
				else
				{
					int32 a = nBegin * 32 + 16;
					int32 b = y * 32 - 16;
					if( b > a )
					{
						CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
						pImage2D->SetRect( CRectangle( x * 32, a, 32, b - a ) );
						pImage2D->SetTexRect( CRectangle( tx, texRect.y + texRect.height * 1.875f, texRect.width, texRect.height * 0.25f ) );
						GetRenderObject()->AddChild( pImage2D );
					}

					CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
					pImage2D->SetRect( CRectangle( x * 32, nBegin * 32 - 16, 32, 32 ) );
					pImage2D->SetTexRect( CRectangle( tx, texRect.y + texRect.height * 2, texRect.width, texRect.height ) );
					GetRenderObject()->AddChild( pImage2D );

					pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
					pImage2D->SetRect( CRectangle( x * 32, y * 32 - 16, 32, 32 ) );
					pImage2D->SetTexRect( CRectangle( tx, texRect.y + texRect.height * 1, texRect.width, texRect.height ) );
					GetRenderObject()->AddChild( pImage2D );
				}

				nBegin = y;
				nType0 = nType;
			}
		}

		int32 a = nBegin > 1 ? nBegin * 32 + 16 : 32;
		int32 b = y * 32;
		if( b > a )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( x * 32, a, 32, b - a ) );
			pImage2D->SetTexRect( CRectangle( tx, texRect.y + texRect.height, texRect.width, texRect.height * 0.25f ) );
			GetRenderObject()->AddChild( pImage2D );
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomRoom1::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}

