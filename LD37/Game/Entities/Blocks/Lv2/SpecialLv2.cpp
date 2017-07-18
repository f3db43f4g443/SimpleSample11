#include "stdafx.h"
#include "SpecialLv2.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Bullets.h"
#include "Explosion.h"
#include "MyLevel.h"
#include "Common/Rand.h"

void CHousePart::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
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

	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	texRect.width /= 8;
	texRect.height /= 4;

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			CRectangle tex = texRect;
			if( iX == 0 && iY == 0 )
				tex = tex.Offset( CVector2( 0 * tex.width, 1 * tex.height ) );
			else if( iX == pChunk->nWidth - 1 && iY == 0 )
				tex = tex.Offset( CVector2( 1 * tex.width, 1 * tex.height ) );
			else if( iX == 0 && iY == pChunk->nHeight - 1 )
				tex = tex.Offset( CVector2( 0 * tex.width, 0 * tex.height ) );
			else if( iX == pChunk->nWidth - 1 && iY == pChunk->nHeight - 1 )
				tex = tex.Offset( CVector2( 1 * tex.width, 0 * tex.height ) );
			else if( iX == 0 )
				tex = tex.Offset( CVector2( 0 * tex.width, 0.5f * tex.height ) );
			else if( iY == 0 )
				tex = tex.Offset( CVector2( 0.5f * tex.width, 1 * tex.height ) );
			else if( iX == pChunk->nWidth - 1 )
				tex = tex.Offset( CVector2( 1 * tex.width, 0.5f * tex.height ) );
			else if( iY == pChunk->nHeight - 1 )
				tex = tex.Offset( CVector2( 0.5f * tex.width, 0 * tex.height ) );
			else
			{
				uint8 nTag = pChunk->GetBlock( iX, iY )->nTag;
				if( nTag == 3 )
					tex = tex.Offset( CVector2( 6 * tex.width, 2 * tex.height ) );
				else if( nTag == 4 )
					tex = tex.Offset( CVector2( 7 * tex.width, 2 * tex.height ) );
				else if( nTag == 5 )
					tex = tex.Offset( CVector2( 6 * tex.width, 3 * tex.height ) );
				else if( nTag == 6 )
					tex = tex.Offset( CVector2( 7 * tex.width, 3 * tex.height ) );
				else
					tex = tex.Offset( CVector2( 0.5f * tex.width, 0.5f * tex.height ) );
			}

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
			pImage2D->SetTexRect( tex );
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

void CHousePart::OnKilled()
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

void CHouse::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
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

	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	texRect.width /= 8;
	texRect.height /= 4;

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			uint8 nTag = pChunk->GetBlock( iX, iY )->nTag;
			if( nTag > 1 )
				continue;
			uint8 nTagX1 = iX > 0 ? pChunk->GetBlock( iX - 1, iY )->nTag : 1;
			uint8 nTagX2 = iX < pChunk->nWidth - 1 ? pChunk->GetBlock( iX + 1, iY )->nTag : 1;
			uint8 nTagY1 = iY > 0 ? pChunk->GetBlock( iX, iY - 1 )->nTag : 1;
			uint8 nTagY2 = iY < pChunk->nHeight - 1 ? pChunk->GetBlock( iX, iY + 1 )->nTag : 1;

			CRectangle tex = texRect;
			if( nTag == 0 )
			{
				if( nTagX1 + nTagX2 + nTagY1 + nTagY2 == 0 )
					tex = tex.Offset( CVector2( 0.5f * tex.width, 2.5f * tex.height ) );
				else if( nTagX1 == 1 && nTagY1 == 1 )
					tex = tex.Offset( CVector2( 0 * tex.width, 3 * tex.height ) );
				else if( nTagX1 == 1 && nTagY2 == 1 )
					tex = tex.Offset( CVector2( 0 * tex.width, 2 * tex.height ) );
				else if( nTagX2 == 1 && nTagY1 == 1 )
					tex = tex.Offset( CVector2( 1 * tex.width, 3 * tex.height ) );
				else if( nTagX2 == 1 && nTagY2 == 1 )
					tex = tex.Offset( CVector2( 1 * tex.width, 2 * tex.height ) );
				else if( nTagX1 == 1 )
				{
					if( nTagY1 > 1 )
						tex = tex.Offset( CVector2( 3 * tex.width, 3 * tex.height ) );
					else if( nTagY2 > 1 )
						tex = tex.Offset( CVector2( 3 * tex.width, 2 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 3 * tex.width, 2.5f * tex.height ) );
				}
				else if( nTagX2 == 1 )
				{
					if( nTagY1 > 1 )
						tex = tex.Offset( CVector2( 2 * tex.width, 3 * tex.height ) );
					else if( nTagY2 > 1 )
						tex = tex.Offset( CVector2( 2 * tex.width, 2 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 2 * tex.width, 2.5f * tex.height ) );
				}
				else if( nTagY1 == 1 )
				{
					if( nTagX1 > 1 )
						tex = tex.Offset( CVector2( 2 * tex.width, 0 * tex.height ) );
					else if( nTagX2 > 1 )
						tex = tex.Offset( CVector2( 3 * tex.width, 0 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 2.5 * tex.width, 0 * tex.height ) );
				}
				else if( nTagY2 == 1 )
				{
					if( nTagX1 > 1 )
						tex = tex.Offset( CVector2( 2 * tex.width, 1 * tex.height ) );
					else if( nTagX2 > 1 )
						tex = tex.Offset( CVector2( 3 * tex.width, 1 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 2.5 * tex.width, 1 * tex.height ) );
				}
				else
					tex = tex.Offset( CVector2( 0.5f * tex.width, 2.5f * tex.height ) );
			}
			else
			{
				if( iX == 1 )
					tex = tex.Offset( CVector2( 7 * tex.width, 0 * tex.height ) );
				else if( iX == 0 )
				{
					if( nTagX2 == 1 )
						tex = tex.Offset( CVector2( 6 * tex.width, 0 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 4 * tex.width, 3 * tex.height ) );
				}
				else if( iX == pChunk->nWidth - 2 )
					tex = tex.Offset( CVector2( 6 * tex.width, 1 * tex.height ) );
				else if( iX == pChunk->nWidth - 1 )
				{
					if( nTagX1 == 1 )
						tex = tex.Offset( CVector2( 7 * tex.width, 1 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 5 * tex.width, 3 * tex.height ) );
				}
				else if( iY == 1 )
					tex = tex.Offset( CVector2( 4 * tex.width, 0 * tex.height ) );
				else if( iY == 0 )
				{
					if( nTagY2 == 1 )
						tex = tex.Offset( CVector2( 4 * tex.width, 1 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 4 * tex.width, 2 * tex.height ) );
				}
				else if( iY == pChunk->nHeight - 2 )
					tex = tex.Offset( CVector2( 5 * tex.width, 1 * tex.height ) );
				else if( iY == pChunk->nHeight - 1 )
				{
					if( nTagY1 == 1 )
						tex = tex.Offset( CVector2( 5 * tex.width, 0 * tex.height ) );
					else
						tex = tex.Offset( CVector2( 5 * tex.width, 2 * tex.height ) );
				}
			}

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
			pImage2D->SetTexRect( tex );
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

void CHouse::OnKilled()
{
	if( m_strEffect )
	{
		ForceUpdateTransform();
		for( int i = 0; i < m_pChunk->nWidth; i++ )
		{
			for( int j = 0; j < m_pChunk->nHeight; j++ )
			{
				if( m_pChunk->GetBlock( i, j )->nTag > 1 )
					continue;
				auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
				pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				pEffect->SetPosition( globalTransform.GetPosition() + CVector2( i, j ) * CMyLevel::GetBlockSize() );
				pEffect->SetState( 2 );
			}
		}
	}
}
