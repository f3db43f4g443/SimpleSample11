#include "stdafx.h"
#include "ChunkUI.h"
#include "MyLevel.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Door.h"

void CChunkUI::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	UpdateHp();
	UpdateEft();
	UpdateRepair();
	SetPosition( m_pChunkObject->GetPosition() );
}

void CChunkUI::OnRemovedFromStage()
{
	m_pChunkObject = NULL;
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CChunkUI::SetChunkObject( CChunkObject* pChunkObject )
{
	bool bIsReset = m_pChunkObject == pChunkObject;
	m_pChunkObject = pChunkObject;
	if( !pChunkObject )
	{
		SetParentEntity( NULL );
		return;
	}

	if( !m_pBlockBulletDrawable )
	{
		m_pBlockBulletDrawable = static_cast<CDrawableGroup*>( m_pBlockBulletEffect->GetResource() );
		m_pBlockBulletEffect->SetRenderObject( NULL );
	}
	if( !m_pRepairDrawable )
	{
		m_pRepairDrawable = static_cast<CParticleFile*>( m_pRepairEffect->GetResource() );
		m_pRepairEffect->SetRenderObject( NULL );
	}
	for( int i = 0; i < m_nUsingEftCount; i++ )
	{
		m_vecBlockBulletEfts[i]->bVisible = false;
	}
	for( int i = 0; i < m_nParticleCount; i++ )
	{
		m_vecRepairParticles[i]->bVisible = false;
		m_vecRepairParticles[i]->GetInstanceData()->ClearData();
		m_vecRepairParticles[i]->GetInstanceData()->GetData().isEmitting = false;
	}
	m_nUsingEftCount = 0;
	m_nParticleCount = 0;

	uint32 nBlockSize = CMyLevel::GetBlockSize();
	auto pChunk = pChunkObject->GetChunk();

	for( int i = 0; i < pChunk->nWidth; i++ )
	{
		for( int j = 0; j < pChunk->nHeight; j++ )
		{
			bool bBlock = false;
			auto eBlockType = pChunk->GetBlock( i, j )->eBlockType;
			if( eBlockType == eBlockType_Block )
			{
				bBlock = true;
			}
			else if( eBlockType == eBlockType_Door )
			{
				for( auto pManifold = pChunk->GetBlock( i, j )->pEntity->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
				{
					auto pDoor = SafeCast<CDoor>( static_cast<CEntity*>( pManifold->pOtherHitProxy ) );
					if( pDoor && pDoor->GetParentEntity() == pChunkObject && !pDoor->IsOpen() )
					{
						bBlock = true;
						break;
					}
				}
			}

			if( bBlock )
			{
				if( m_nUsingEftCount >= m_vecBlockBulletEfts.size() )
				{
					auto pImage2D = static_cast<CImage2D*>( m_pBlockBulletDrawable->CreateInstance() );
					m_pBlockBulletEffect->AddChild( pImage2D );
					m_vecBlockBulletEfts.push_back( pImage2D );
				}
				CImage2D* pImage2D = m_vecBlockBulletEfts[m_nUsingEftCount++];
				pImage2D->SetRect( CRectangle( i * nBlockSize, j * nBlockSize, nBlockSize, nBlockSize ) );
				pImage2D->bVisible = true;
			}
		}
	}

	for( int i = 0; i < pChunk->nWidth; i++ )
	{
		{
			if( m_nParticleCount >= m_vecRepairParticles.size() )
			{
				auto pParticleObject = m_pRepairDrawable->CreateInstance( NULL );
				m_pRepairEffect->AddChild( pParticleObject );
				m_vecRepairParticles.push_back( pParticleObject );
			}
			CParticleSystemObject* pParticle = m_vecRepairParticles[m_nParticleCount++];
			pParticle->bVisible = true;
			pParticle->GetInstanceData()->GetData().isEmitting = false;
			pParticle->SetPosition( CVector2( i * nBlockSize, 0 ) );
		}
		{
			if( m_nParticleCount >= m_vecRepairParticles.size() )
			{
				auto pParticleObject = m_pRepairDrawable->CreateInstance( NULL );
				m_pRepairEffect->AddChild( pParticleObject );
				m_vecRepairParticles.push_back( pParticleObject );
			}
			CParticleSystemObject* pParticle = m_vecRepairParticles[m_nParticleCount++];
			pParticle->bVisible = true;
			pParticle->GetInstanceData()->GetData().isEmitting = false;
			pParticle->SetPosition( CVector2( i * nBlockSize, pChunk->nHeight * nBlockSize ) );
		}
	}
	for( int i = 0; i < pChunk->nHeight; i++ )
	{
		{
			if( m_nParticleCount >= m_vecRepairParticles.size() )
			{
				auto pParticleObject = m_pRepairDrawable->CreateInstance( NULL );
				m_pRepairEffect->AddChild( pParticleObject );
				m_vecRepairParticles.push_back( pParticleObject );
			}
			CParticleSystemObject* pParticle = m_vecRepairParticles[m_nParticleCount++];
			pParticle->bVisible = true;
			pParticle->GetInstanceData()->GetData().isEmitting = false;
			pParticle->SetPosition( CVector2( 0, i * nBlockSize ) );
			pParticle->SetRotation( PI * 0.5f );
		}
		{
			if( m_nParticleCount >= m_vecRepairParticles.size() )
			{
				auto pParticleObject = m_pRepairDrawable->CreateInstance( NULL );
				m_pRepairEffect->AddChild( pParticleObject );
				m_vecRepairParticles.push_back( pParticleObject );
			}
			CParticleSystemObject* pParticle = m_vecRepairParticles[m_nParticleCount++];
			pParticle->bVisible = true;
			pParticle->GetInstanceData()->GetData().isEmitting = false;
			pParticle->SetPosition( CVector2( pChunk->nWidth * nBlockSize, i * nBlockSize ) );
			pParticle->SetRotation( PI * 0.5f );
		}
	}

	CRectangle chunkRect( 0, 0, pChunk->nWidth * nBlockSize, pChunk->nHeight * nBlockSize );
	static_cast<CImage2D*>( m_pFrameImg[0].GetPtr() )->SetRect( CRectangle( -8, -8, 8, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[1].GetPtr() )->SetRect( CRectangle( 0, -8, pChunk->nWidth * nBlockSize, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[2].GetPtr() )->SetRect( CRectangle( pChunk->nWidth * nBlockSize, -8, 8, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[3].GetPtr() )->SetRect( CRectangle( -8, 0, 8, pChunk->nHeight * nBlockSize ) );
	static_cast<CImage2D*>( m_pFrameImg[4].GetPtr() )->SetRect( CRectangle( pChunk->nWidth * nBlockSize, 0, 8, pChunk->nHeight * nBlockSize ) );
	static_cast<CImage2D*>( m_pFrameImg[5].GetPtr() )->SetRect( CRectangle( -8, pChunk->nHeight * nBlockSize, 8, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[6].GetPtr() )->SetRect( CRectangle( 0, pChunk->nHeight * nBlockSize, pChunk->nWidth * nBlockSize, 8 ) );
	static_cast<CImage2D*>( m_pFrameImg[7].GetPtr() )->SetRect( CRectangle( pChunk->nWidth * nBlockSize, pChunk->nHeight * nBlockSize, 8, 8 ) );

	m_fBlockBulletEftAlpha = -1;
	SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkRoot1() );
	UpdateEft();
}

void CChunkUI::ShowRepairEffect()
{
	for( int i = 0; i < m_nParticleCount; i++ )
	{
		m_vecRepairParticles[i]->GetInstanceData()->GetData().isEmitting = true;
	}
}

void CChunkUI::OnTick()
{
	auto pPlayer = GetStage()->GetPlayer();
	float fRepair = 0;
	if( pPlayer && pPlayer->GetCurRoom() == m_pChunkObject )
	{
		fRepair = pPlayer->GetRepairPercent();
	}
	if( fRepair != m_fRepairPercent )
	{
		m_fRepairPercent = fRepair;
		UpdateRepair();
	}

	UpdateEft();
	UpdateHp();

	SetPosition( m_pChunkObject->GetPosition() );

	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CChunkUI::UpdateHp()
{
	CVector4 hpColor;
	if( m_pChunkObject->GetMaxHp() == 0 )
		hpColor = CVector4( 1, 1, 1, 1 );
	else
	{
		float fPercent = m_pChunkObject->GetHp() * 1.0f / m_pChunkObject->GetMaxHp();
		if( fPercent > 0.75f )
			hpColor = ( CVector4( 0, 1, 0, 1 ) * ( fPercent - 0.75f ) + CVector4( 0.5, 0.5, 0, 1 ) * ( 1 - fPercent ) ) / 0.25f;
		else if( fPercent > 0.25f )
			hpColor = ( CVector4( 0.5, 0.5, 0, 1 ) * ( fPercent - 0.25f ) + CVector4( 1, 0, 0, 1 ) * ( 0.75f - fPercent ) ) / 0.5f;
		else
			hpColor = ( CVector4( 1, 0, 0, 1 ) * fPercent + CVector4( 0.25f, 0.25f, 0.25f, 1 ) * ( 0.25f - fPercent ) ) * 4;
	}
	hpColor.w = 1.0f;// 0.5f + 0.5f * m_fRepairPercent;
	for( int i = 0; i < ELEM_COUNT( m_pFrameImg ); i++ )
		static_cast<CImage2D*>( m_pFrameImg[i].GetPtr() )->GetParam()[0] = hpColor;
}

void CChunkUI::UpdateEft()
{
	float fAlpha = 0;
	auto pPlayer = GetStage()->GetPlayer();
	if( pPlayer && pPlayer->GetCurRoom() == m_pChunkObject )
	{
		fAlpha = pPlayer->GetHidingPercent();
	}
	if( fAlpha == m_fBlockBulletEftAlpha )
		return;
	m_fBlockBulletEftAlpha = fAlpha;

	m_pBlockBulletEffect->bVisible = m_fBlockBulletEftAlpha > 0;
	if( m_fBlockBulletEftAlpha > 0 )
	{
		for( int i = 0; i < m_nUsingEftCount; i++ )
		{
			m_vecBlockBulletEfts[i]->GetParam()[0].w = m_fBlockBulletEftAlpha;
		}
	}
}

void CChunkUI::UpdateRepair()
{
	if( m_fRepairPercent == 0 || m_fRepairPercent == 1 )
	{
		m_pRepairImg->bVisible = false;
		return;
	}

	uint32 nBlockSize = CMyLevel::GetBlockSize();
	uint32 nWidth = m_pChunkObject->GetChunk()->nWidth * nBlockSize;
	uint32 nHeight = m_pChunkObject->GetChunk()->nHeight * nBlockSize;

	int32 nMaxY = ceil( nHeight * m_fRepairPercent );
	int32 nMinY = Max( nMaxY - 8, 0 );
	m_pRepairImg->bVisible = true;
	static_cast<CImage2D*>( m_pRepairImg.GetPtr() )->SetRect( CRectangle( 0, nMinY, nWidth, nMaxY - nMinY ) );
}

void CBlockDetectUI::OnAddedToStage()
{
	GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
}

void CBlockDetectUI::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();

	for( auto pBlock : m_vecBlocks )
	{
		if( pBlock->m_pDetectUI )
		{
			pBlock->m_pDetectUI->RemoveThis();
			pBlock->m_pDetectUI = NULL;
		}
	}
	m_vecBlocks.clear();
	m_vecPool.clear();
}

void CBlockDetectUI::OnTick()
{
	CVector2 center = globalTransform.GetPosition();
	if( m_bShow )
		m_fDetectRange = Min( m_fDetectRange + m_fFadeInSpeed * GetStage()->GetElapsedTimePerTick(), m_fMaxDetectRange );
	else
		m_fDetectRange = Max( m_fDetectRange - m_fFadeInSpeed * GetStage()->GetElapsedTimePerTick(), 0.0f );

	if( m_fDetectRange > 0 )
	{
		SHitProxyCircle circle;
		circle.center = CVector2( 0, 0 );
		circle.fRadius = m_fDetectRange;
		vector<CReference<CEntity> > hitEntities;
		GetStage()->MultiHitTest( &circle, globalTransform, hitEntities );
		for( CEntity* pEntity : hitEntities )
		{
			auto pBlockObject = SafeCast<CBlockObject>( pEntity );
			if( !pBlockObject )
				continue;

			pBlockObject->nPublicFlag = 1;
			if( !pBlockObject->m_pDetectUI )
			{
				m_vecBlocks.push_back( pBlockObject );
				CReference<CRenderObject2D> pUI;
				if( m_vecPool.size() )
				{
					pUI = m_vecPool.back();
					m_vecPool.pop_back();
				}
				else
				{
					pUI = m_pUIDrawable->CreateInstance();
				}
				pBlockObject->AddChild( pUI );
				pBlockObject->m_pDetectUI = pUI;
				pUI->SetRenderParent( this );

				int32 nType;
				if( pBlockObject->GetBlock()->eBlockType != eBlockType_Block && pBlockObject->GetBlock()->eBlockType != eBlockType_LowBlock )
				{
					nType = pBlockObject->GetBlock()->pOwner->HasLayer( 1 ) ? 1 : 0;
				}
				else
				{
					nType = pBlockObject->GetBlock()->pOwner->HasLayer( 0 ) ? 3 : 2;
				}
				static_cast<CImage2D*>( pUI.GetPtr() )->SetTexRect( CRectangle( ( nType & 1 ) * 0.5f, ( nType >> 1 ) * 0.5f, 0.5f, 0.5f ) );
			}
		}
	}

	float fFadeEnd = m_fDetectRange / 32;
	float fFadeBegin = fFadeEnd - m_fFadeDist / 32;
	for( int i = m_vecBlocks.size() - 1; i >= 0; i-- )
	{
		auto pBlock = m_vecBlocks[i];
		auto pImage = static_cast<CImage2D*>( pBlock->m_pDetectUI.GetPtr() );
		auto& imageTexRect = pImage->GetElem().texRect;

		uint8 nPublicFlag = pBlock->nPublicFlag;
		pBlock->nPublicFlag = 0;
		if( !nPublicFlag || !pBlock->GetStage() )
		{
			m_vecPool.push_back( pBlock->m_pDetectUI );
			pBlock->m_pDetectUI->RemoveThis();
			pBlock->m_pDetectUI = NULL;

			if( i != m_vecBlocks.size() - 1 )
				m_vecBlocks[i] = m_vecBlocks.back();
			m_vecBlocks.pop_back();
			continue;
		}

		CVector2 dPos = center - pBlock->globalTransform.GetPosition();
		dPos = dPos * ( 1.0f / 32 );
		dPos.y = 1 - dPos.y;
		dPos = dPos + CVector2( imageTexRect.x, imageTexRect.y );

		auto& param = pImage->GetParam()[0];
		param.x = dPos.x;
		param.y = dPos.y;
		param.z = fFadeBegin;
		param.w = fFadeEnd;

		auto pChunkObject = pBlock->GetBlock()->pOwner->pChunkObject;
		CVector4 hpColor;
		if( pChunkObject->GetMaxHp() )
		{
			float fPercent = Min( 1.0f, Max( 0.0f, pChunkObject->GetHp() * 1.0f / pChunkObject->GetMaxHp() ) );
			if( fPercent > 0.75f )
				hpColor = ( CVector4( 0, 1, 0, 0.75f ) * ( fPercent - 0.75f ) + CVector4( 0.5, 0.5, 0, 0.75f ) * ( 1 - fPercent ) ) / 0.25f;
			else if( fPercent > 0.25f )
				hpColor = ( CVector4( 0.5, 0.5, 0, 0.75f ) * ( fPercent - 0.25f ) + CVector4( 1, 0, 0, 0.75f ) * ( 0.75f - fPercent ) ) / 0.5f;
			else
				hpColor = ( CVector4( 1, 0, 0, 0.75f ) * fPercent + CVector4( 0.5f, 0.5f, 0.5f, 1 ) * ( 0.25f - fPercent ) ) * 4;
		}
		else
			hpColor = CVector4( 1, 1, 1, 0.5f );
		pImage->GetParam()[1] = hpColor;
	}
}
