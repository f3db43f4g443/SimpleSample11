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

	pImage2D->SetRect( rect );
	pImage2D->SetTexRect( texRect.Offset( CVector2( SRand::Inst().Rand( 0u, nTileX ) * texRect.width, SRand::Inst().Rand( 0u, nTileY ) * texRect.height ) ) );
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
	uint32 nTileX = rect.width / 32;
	uint32 nTileY = rect.height / 32;
	texRect.width /= nTileX;
	texRect.height /= nTileY;

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			if( m_bBlockTypeMask[pChunk->GetBlock( iX, iY )->eBlockType] )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );

				uint32 tX = SRand::Inst().Rand( 1u, nTileX - 1 );
				uint32 tY = SRand::Inst().Rand( 1u, nTileY - 1 );

				pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * tX, texRect.height * tY ) ) );
				GetRenderObject()->AddChild( pImage2D );
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

void CRandomChunkTiled::OnKilled()
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
				pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetInst()->GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
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
	uint32 nAltX = Max( 0u, m_nAltX );
	uint32 nAltY = Max( 0u, m_nAltY );
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

			pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * tX, texRect.height * ( nTileY - 1 - tY ) ) ) );
			GetRenderObject()->AddChild( pImage2D );

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
	if( m_pEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				auto pEffect = SafeCast<CEffectObject>( m_pEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetInst()->GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
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

	SetRenderObject( new CRenderObject2D );

	int32 nLength = !m_bVertical ? pChunk->nWidth : pChunk->nHeight;
	if( nLength >= 2 )
	{
		uint32 nSeg2Count = SRand::Inst().Rand( 0, nLength / 2 );
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
		SRand::Inst().Shuffle( bSeg2, nTotalSegCount );

		int nCurLen = 0;
		for( int i = 0; i < nTotalSegCount; i++ )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			bool b2 = bSeg2[i];
			pImage2D->SetRect( !m_bVertical ? CRectangle( nCurLen * 32, 0, b2 ? 64 : 32, m_nWidth * 32 )
				: CRectangle( 0, nCurLen * 32, m_nWidth * 32, b2 ? 64 : 32 ) );
			nCurLen += b2 ? 2 : 1;

			CRectangle texRect;
			if( i == 0 || i == nTotalSegCount - 1 )
			{
				if( b2 )
				{
					texRect = m_texRect2End;
					texRect.width /= m_nTexRect2EndX;
					texRect.x += texRect.width * SRand::Inst().Rand( 0u, m_nTexRect2EndX );
					texRect.height /= m_nTexRect2EndY;
					texRect.y += texRect.height * SRand::Inst().Rand( 0u, m_nTexRect2EndY );
				}
				else
				{
					texRect = m_texRect1End;
					texRect.width /= m_nTexRect1EndX;
					texRect.x += texRect.width * SRand::Inst().Rand( 0u, m_nTexRect1EndX );
					texRect.height /= m_nTexRect1EndY;
					texRect.y += texRect.height * SRand::Inst().Rand( 0u, m_nTexRect1EndY );
				}

				if( !m_bVertical )
				{
					texRect.width *= 0.5f;
					if( i == nTotalSegCount - 1 )
						texRect.x += texRect.width;
				}
				else
				{
					texRect.height *= 0.5f;
					if( i == nTotalSegCount - 1 )
						texRect.y += texRect.height;
				}
			}
			else
			{
				if( b2 )
				{
					texRect = m_texRect2;
					texRect.width /= m_nTexRect2X;
					texRect.x += texRect.width * SRand::Inst().Rand( 0u, m_nTexRect2X );
					texRect.height /= m_nTexRect2Y;
					texRect.y += texRect.height * SRand::Inst().Rand( 0u, m_nTexRect2Y );
				}
				else
				{
					texRect = m_texRect1;
					texRect.width /= m_nTexRect1X;
					texRect.x += texRect.width * SRand::Inst().Rand( 0u, m_nTexRect1X );
					texRect.height /= m_nTexRect1Y;
					texRect.y += texRect.height * SRand::Inst().Rand( 0u, m_nTexRect1Y );
				}
			}

			pImage2D->SetTexRect( texRect );
			GetRenderObject()->AddChild( pImage2D );
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
		texRect.x += texRect.width * SRand::Inst().Rand( 0u, m_nTexRect1X );
		texRect.height /= m_nTexRect1Y;
		texRect.y += texRect.height * SRand::Inst().Rand( 0u, m_nTexRect1Y );
		pImage2D->SetTexRect( texRect );
		GetRenderObject()->AddChild( pImage2D );
	}

	m_nMaxHp += m_nHpPerLength * nLength;
	m_fHp = m_nMaxHp;
}

void CRandomChunk2::OnKilled()
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
				pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetInst()->GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
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
		vecType[i].first = SRand::Inst().Rand() & 1;
		vecType[i].second = SRand::Inst().Rand() & 1;
	}

	auto rect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().rect;
	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
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
				static uint32 indexY[] = { 0, 1, 3, 2 };
				uint32 tX = indexX[x1 + x2 * 2];
				uint32 tY = indexX[y1 + y2 * 2];
				tX += 4 * SRand::Inst().Rand( 0u, nAltX );
				tY += 4 * SRand::Inst().Rand( 0u, nAltY );

				pImage2D->SetTexRect( texRect.Offset( CVector2( texRect.width * tX, texRect.height * tY ) ) );
				GetRenderObject()->AddChild( pImage2D );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CRandomChunk3::OnKilled()
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
				pEffect->SetPosition( GetPosition() + CVector2( i, j ) * CMyLevel::GetInst()->GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}
