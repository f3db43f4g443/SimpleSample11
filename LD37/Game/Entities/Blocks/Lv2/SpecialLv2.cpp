#include "stdafx.h"
#include "SpecialLv2.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Bullets.h"
#include "Explosion.h"
#include "MyLevel.h"
#include "Entities/EnemyCharacters.h"
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

				if( nTag >= 3 && nTag <= 6 )
				{
					auto pEntrance = SafeCast<CHouseEntrance>( m_pEntrancePrefabs[nTag - 3]->GetRoot()->CreateInstance() );
					pEntrance->SetParentEntity( this );
					pEntrance->SetPosition( CVector2( iX + 0.5f, iY + 0.5f ) * CMyLevel::GetBlockSize() );
					m_houseEntrances.push_back( pEntrance );
				}
			}

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
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

void CHousePart::Explode()
{
	for( CHouseEntrance* pEntrance : m_houseEntrances )
	{
		auto pExp = SafeCast<CExplosionWithBlockBuff>( m_pExp->GetRoot()->CreateInstance() );
		pExp->SetPosition( pEntrance->globalTransform.GetPosition() );
		pExp->SetRotation( pEntrance->GetDir() * PI * 0.5f );
		pExp->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		CBlockBuff::SContext context;
		context.nLife = 600;
		context.nTotalLife = 600;
		context.fParams[0] = 3;
		pExp->Set( &context );
		pExp->SetCreator( this );
	}
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

void CHouse::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CChunkObject::OnRemovedFromStage();
}

bool CHouse::CanEnter( CCharacter * pCharacter )
{
	return !m_bExploded;
}

bool CHouse::Enter( CCharacter * pCharacter )
{
	if( m_bExploded )
		return false;
	m_characters.push_back( pair<CReference<CCharacter>, int32>( pCharacter, 10 ) );
	pCharacter->SetParentEntity( NULL );
	m_bAnyoneEntered = true;
}

void CHouse::OnCreateComplete( CMyLevel * pLevel )
{
	uint32 nGrids = m_pChunk->nWidth * m_pChunk->nHeight;
	for( int i = 0; i < 4; i++ )
	{
		if( !m_pInitCharPrefabs[i] )
			continue;
		float fCount = m_fInitCharPerGrid[i] * nGrids;
		int32 nCount = floor( fCount + SRand::Inst().Rand( 0.0f, 1.0f ) );
		for( int j = 0; j < nCount; j++ )
		{
			auto pChar = SafeCast<CCharacter>( m_pInitCharPrefabs[i]->GetRoot()->CreateInstance() );
			m_characters.push_back( pair<CReference<CCharacter>, int32>( pChar, 0 ) );
		}
	}

	for( auto pChild = Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto pEntrance = SafeCast<CHouseEntrance>( pChild );
		if( pEntrance )
		{
			m_houseEntrances.push_back( pEntrance );
			continue;
		}

		auto pPart = SafeCast<CHousePart>( pChild );
		if( pPart )
			m_houseParts.push_back( pPart );
	}

	for( int i = 0; i < 4; i++ )
	{
		if( m_pThrowObjPrefabs[i] )
		{
			int32 nCount = SRand::Inst().Rand( m_nThrowObjMin[i], m_nThrowObjMax[i] + 1 );
			for( int j = 0; j < nCount; j++ )
				m_throwObjs.push_back( i );
		}
	}

	if( pLevel )
		pLevel->GetStage()->RegisterAfterHitTest( 10, &m_onTick );
}

void CHouse::Explode()
{
	for( CHouseEntrance* pEntrance : m_houseEntrances )
	{
		auto pExp = SafeCast<CExplosionWithBlockBuff>( m_pExp->GetRoot()->CreateInstance() );
		pExp->SetPosition( pEntrance->globalTransform.GetPosition() );
		pExp->SetRotation( pEntrance->GetDir() * PI * 0.5f );
		pExp->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		CBlockBuff::SContext context;
		context.nLife = 600;
		context.nTotalLife = 600;
		context.fParams[0] = 3;
		pExp->Set( &context );
		pExp->SetCreator( this );
	}
	m_bExploded = true;
}

void CHouse::OnKilled()
{
	for( CHousePart* pHousePart : m_houseParts )
	{
		pHousePart->Explode();
	}

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

void CHouse::OnTick()
{
	if( m_bExploded )
	{
		for( CHouseEntrance* pEntrance : m_houseEntrances )
		{
			auto pEffect = SafeCast<CEffectObject>( m_pExpEft->GetRoot()->CreateInstance() );
			pEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			pEffect->SetRotation( pEntrance->GetDir() * PI * 0.5f );
			pEffect->SetPosition( pEntrance->globalTransform.GetPosition() + CVector2( cos( pEffect->r ), sin( pEffect->r ) ) * 16 );
			pEffect->SetState( 2 );
		}

		m_nEftCount++;
		if( m_nEftCount< 30 )
			GetStage()->RegisterAfterHitTest( 10, &m_onTick );
		return;
	}

	GetStage()->RegisterAfterHitTest( 10, &m_onTick );

	int32 nChar = -1;
	for( int i = 0; i < m_characters.size(); i++ )
	{
		if( m_characters[i].second )
			m_characters[i].second--;
		if( nChar == -1 && !m_characters[i].second )
			nChar = i;
	}
	if( nChar >= 0 && m_bAnyoneEntered )
	{
		auto pCharacter = m_characters[nChar].first;
		auto pThug = SafeCast<CThug>( pCharacter.GetPtr() );

		if( pThug && !SRand::Inst().Rand( 0, 2 ) )
		{
			SRand::Inst().Shuffle( m_houseParts );
			for( int i = 0; i < m_houseParts.size(); i++ )
			{
				if( !m_houseParts[i]->m_houseEntrances.size() )
					continue;
				SRand::Inst().Shuffle( m_houseParts[i]->m_houseEntrances );
				for( CHouseEntrance* pItem : m_houseParts[i]->m_houseEntrances )
				{
					if( pItem->Exit( pThug ) )
					{
						if( !m_throwObjs.size() )
						{
							Explode();
							return;
						}
						int32 nPrefab = m_throwObjs.back();
						m_throwObjs.pop_back();
						auto pThrowObj = SafeCast<CCharacter>( m_pThrowObjPrefabs[nPrefab]->GetRoot()->CreateInstance() );
						pThug->SetThrowObj( pThrowObj, CVector2( 0, -pThrowObj->GetRenderObject()->GetLocalBound().y ), true );

						m_characters[nChar] = m_characters.back();
						m_characters.pop_back();
						return;
					}
				}
			}
		}

		for( int i = m_houseEntrances.size(); i > 0; i-- )
		{
			int32 r = SRand::Inst().Rand( 0, i );
			if( m_houseEntrances[r]->Exit( pCharacter ) )
			{
				if( pThug )
				{
					if( !m_throwObjs.size() )
					{
						Explode();
						return;
					}
					int32 nPrefab = m_throwObjs.back();
					m_throwObjs.pop_back();
					auto pThrowObj = SafeCast<CCharacter>( m_pThrowObjPrefabs[nPrefab]->GetRoot()->CreateInstance() );
					pThug->SetThrowObj( pThrowObj, CVector2( 0, -pThrowObj->GetRenderObject()->GetLocalBound().y ), false );
				}

				m_characters[nChar] = m_characters.back();
				m_characters.pop_back();
				break;
			}
			else
			{
				if( r != i - 1 )
					swap( m_houseEntrances[r], m_houseEntrances.back() );
			}
		}
	}
}
