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
	float fPercent = m_pChunkObject->GetHp() * 1.0f / m_pChunkObject->GetMaxHp();
	if( fPercent > 0.75f )
		hpColor = ( CVector4( 0, 1, 0, 1 ) * ( fPercent - 0.75f ) + CVector4( 0.5, 0.5, 0, 1 ) * ( 1 - fPercent ) ) / 0.25f;
	else if( fPercent > 0.25f )
		hpColor = ( CVector4( 0.5, 0.5, 0, 1 ) * ( fPercent - 0.25f ) + CVector4( 1, 0, 0, 1 ) * ( 0.75f - fPercent ) ) / 0.5f;
	else
		hpColor = ( CVector4( 1, 0, 0, 1 ) * fPercent + CVector4( 0, 0, 0, 1 ) * ( 0.25f - fPercent ) ) * 4;
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
