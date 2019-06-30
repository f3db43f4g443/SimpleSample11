#include "stdafx.h"
#include "SpecialLv1.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Bullets.h"
#include "Explosion.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Common/Algorithm.h"
#include "Entities/Enemies/Lv1Enemies.h"

void CGarbageBinRed::Trigger()
{
	float fBaseAngle = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < m_nBulletCount; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pPrefab->GetRoot()->CreateInstance() );
		pBullet->SetCreator( this );
		pBullet->SetPosition( CVector2( m_pChunk->pos.x, m_pChunk->pos.y ) + CVector2( m_pChunk->nWidth * 0.5f, m_pChunk->nHeight * 0.5f ) * CMyLevel::GetBlockSize() );
		float fAngle = fBaseAngle + PI * 2 * i / m_nBulletCount;
		pBullet->SetRotation( fAngle );
		pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fMinSpeed, m_fMaxSpeed ) );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}

void CGarbageBinYellow::Trigger()
{
	ForceUpdateTransform();
	auto pExplosion = SafeCast<CExplosion>( m_pPrefab->GetRoot()->CreateInstance() );
	pExplosion->SetPosition( CVector2( m_pChunk->pos.x, m_pChunk->pos.y ) + CVector2( m_pChunk->nWidth * 0.5f, m_pChunk->nHeight * 0.5f ) * CMyLevel::GetBlockSize() );
	pExplosion->SetCreator( this );
	pExplosion->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}

void CGarbageBinGreen::Trigger()
{
	ForceUpdateTransform();
	float fBaseAngle = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < m_nBulletCount; i++ )
	{
		auto pBullet = SafeCast<CBulletWithBlockBuff>( m_pPrefab->GetRoot()->CreateInstance() );
		pBullet->SetCreator( this );
		pBullet->SetPosition( CVector2( m_pChunk->pos.x, m_pChunk->pos.y ) + CVector2( m_pChunk->nWidth * 0.5f, m_pChunk->nHeight * 0.5f ) * CMyLevel::GetBlockSize() );
		float fAngle = fBaseAngle + PI * 2 * i / m_nBulletCount;
		pBullet->SetRotation( fAngle );
		pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fMinSpeed, m_fMaxSpeed ) );
		pBullet->SetAcceleration( CVector2( 0, -m_fGravity ) );
		CBlockBuff::SContext context;
		context.nLife = m_nLife;
		context.nTotalLife = m_nLife;
		context.fParams[0] = m_fDamage;
		pBullet->Set( &context );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}

void CGarbageBinBlack::Trigger()
{
	ForceUpdateTransform();
	float fBaseAngle = SRand::Inst().Rand( -PI, PI );
	for( int i = 0; i < m_nCount; i++ )
	{
		auto pCharacter = SafeCast<CCharacter>( m_pPrefab->GetRoot()->CreateInstance() );
		float fAngle = fBaseAngle + PI * 2 * i / m_nCount;
		pCharacter->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fMinSpeed, m_fMaxSpeed ) );
		pCharacter->SetPosition( CVector2( m_pChunk->pos.x, m_pChunk->pos.y ) + CVector2( m_pChunk->nWidth * 0.5f, m_pChunk->nHeight * 0.5f ) * CMyLevel::GetBlockSize() );
		if( m_bSetAngle )
			pCharacter->SetRotation( fAngle );
		pCharacter->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
}

void CHouse0::OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel )
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
	uint32 k = nTileY / 4;
	uint8 nType = SRand::Inst().Rand( 0u, k ), nType1 = SRand::Inst().Rand( k, k * 2 ), nType2 = SRand::Inst().Rand( k * 2, k * 3 );
	TRectangle<int32> rectType1( 0, 0, pChunk->nWidth, pChunk->nHeight );
	if( SRand::Inst().Rand( 0, 2 ) )
		swap( nType, nType1 );
	if( SRand::Inst().Rand( 0, 2 ) )
	{
		float f = pChunk->nWidth * 0.5f + SRand::Inst().Rand( -1.0f, 1.0f );
		int32 x = -1;
		float fDistMin = SRand::Inst().Rand( pChunk->nWidth / 4, pChunk->nWidth / 2 );
		for( int i = 1; i < pChunk->nWidth; i++ )
		{
			bool bOK = true;
			for( int j = 0; j < pChunk->nHeight; j++ )
			{
				if( pChunk->GetBlock( i - 1, j )->nTag == 1 && pChunk->GetBlock( i, j )->nTag == 1 )
				{
					bOK = false;
					break;
				}
			}
			if( bOK )
			{
				float fDist = abs( i - f );
				if( fDist < fDistMin )
				{
					fDistMin = fDist;
					x = i;
				}
			}
		}
		if( x > 0 )
			rectType1.SetRight( x );
	}
	else
	{
		float f = pChunk->nHeight * 0.5f + SRand::Inst().Rand( -1.0f, 1.0f );
		int32 y = -1;
		float fDistMin = SRand::Inst().Rand( pChunk->nHeight / 4, pChunk->nHeight / 2 );
		for( int j = 1; j < pChunk->nHeight; j++ )
		{
			bool bOK = true;
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				if( pChunk->GetBlock( i, j - 1 )->nTag == 1 && pChunk->GetBlock( i, j )->nTag == 1 )
				{
					bOK = false;
					break;
				}
			}
			if( bOK )
			{
				float fDist = abs( j - f );
				if( fDist < fDistMin )
				{
					fDistMin = fDist;
					y = j;
				}
			}
		}
		if( y > 0 )
			rectType1.SetBottom( y );
	}

	SetRenderObject( new CRenderObject2D );

	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			uint8 nTag = pChunk->blocks[iX + iY * pChunk->nWidth].nTag;
			if( nTag >= m_nPipeTag && nTag < m_nPipeTag + 4 || nTag == m_nPipe1Tag )
				continue;
			uint32 tX, tY;
			tX = SRand::Inst().Rand( 0u, nTileX );
			if( nTag >= m_nTag0 )
				tY = SRand::Inst().Rand( 0u, k ) + k * 3;
			else if( nTag >= m_nTag1 )
				tY = nType2;
			else if( iX >= rectType1.x && iX < rectType1.GetRight() && iY >= rectType1.y && iY < rectType1.GetBottom() )
				tY = nType1;
			else
				tY = nType;
			auto tex = texRect.Offset( CVector2( texRect.width * tX, texRect.height * tY ) );

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

	CRectangle texRect0 = m_texRectPipe;
	texRect0.width /= 2;
	texRect0.height /= 2;
	for( int iY = 0; iY < pChunk->nHeight - 1; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth - 1; iX++ )
		{
			uint8 nTag = pChunk->blocks[iX + iY * pChunk->nWidth].nTag;
			if( nTag >= m_nPipeTag && nTag < m_nPipeTag + 4 )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 64, 64 ) );
				int32 nX = ( nTag - m_nPipeTag ) & 1;
				int32 nY = ( nTag - m_nPipeTag ) >> 1;
				CVector2 ofs( nX * m_texRectPipe.width, nY * m_texRectPipe.height );
				pImage2D->SetTexRect( m_texRectPipe.Offset( ofs ) );
				GetRenderObject()->AddChild( pImage2D );
				for( int j = 0; j < 2; j++ )
				{
					for( int i = 0; i < 2; i++ )
					{
						int iX1 = iX + i;
						int iY1 = iY + j;
						auto pBlock = GetBlock( iX1, iY1 );
						pBlock->nTag = m_nTag0;
						pBlock->rtTexRect = texRect0.Offset( ofs + CVector2( texRect0.width * i, texRect0.height * ( 1 - j ) ) );
					}
				}

				CVector2 dirs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
				SPipe0 pipe0;
				CVector2 dir = dirs[nTag - m_nPipeTag];
				pipe0.ofs = ( CVector2( iX + 1, iY + 1 ) + dir ) * CMyLevel::GetBlockSize();
				pipe0.nType = nTag - m_nPipeTag;
				m_vecPipe0.push_back( pipe0 );
			}
		}
	}

	texRect0 = m_texRectPipe1;
	texRect0.width /= 3;
	texRect0.height /= 3;
	for( int iY = 0; iY < pChunk->nHeight - 2; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth - 2; iX++ )
		{
			uint8 nTag = pChunk->blocks[iX + iY * pChunk->nWidth].nTag;
			if( nTag == m_nPipe1Tag )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 96, 96 ) );
				pImage2D->SetTexRect( m_texRectPipe1 );
				GetRenderObject()->AddChild( pImage2D );
				for( int j = 0; j < 3; j++ )
				{
					for( int i = 0; i < 3; i++ )
					{
						int iX1 = iX + i;
						int iY1 = iY + j;
						auto pBlock = GetBlock( iX1, iY1 );
						pBlock->nTag = m_nTag0;
						pBlock->rtTexRect = texRect0.Offset( CVector2( texRect0.width * i, texRect0.height * ( 2 - j ) ) );
					}
				}

				SPipe1 pipe1;
				pipe1.ofs = CVector2( iX + 1.5f, iY + 1.5f ) * CMyLevel::GetBlockSize();
				m_vecPipe1.push_back( pipe1 );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;

	if( !pLevel )
		return;
	pLevel->GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CHouse0::OnCreateComplete( CMyLevel * pLevel )
{
	for( auto pChunk = m_pChunk->Get_SubChunk(); pChunk; pChunk = pChunk->NextSubChunk() )
	{
		if( pChunk->nSubChunkType == 2 && pChunk->pChunkObject )
		{
			m_vecSubChunk.push_back( pChunk->pChunkObject );
		}
	}
}

bool CHouse0::Damage( SDamageContext & context )
{
	for( auto& pChunkObject : m_vecSubChunk )
	{
		if( pChunkObject )
		{
			if( !pChunkObject->GetParentEntity() )
				pChunkObject = NULL;
			else
			{
				SDamageContext context1;
				context1.fDamage = context.fDamage * 2 * pChunkObject->GetMaxHp() / m_nMaxHp;
				context1.nType = context1.nSourceType = 0;
				pChunkObject->Damage( context1 );
			}
		}
	}

	return CChunkObject::Damage( context );
}

void CHouse0::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	m_vecPipe1.clear();
	CChunkObject::OnRemovedFromStage();
}

void CHouse0::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	for( auto& item : m_vecPipe1 )
	{
		CVector2 p = globalTransform.GetPosition() + item.ofs;
		if( p.y > 640 || p.y >= CMyLevel::GetInst()->GetBoundWithLvBarrier().GetBottom() )
			continue;

		if( item.nCD > 0 )
			item.nCD--;
		if( !item.nCD )
		{
			if( !item.pEft )
			{
				auto pEft = SafeCast<CWaterFall1>( m_pPipe1EftPrefab->GetRoot()->CreateInstance() );
				pEft->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
				pEft->Set( this, item.ofs );
				CHouse0Pipe1Eft* pFlyController = new CHouse0Pipe1Eft( m_pDrawable1, m_pFlyPrefab, m_pPrefab1 );
				pFlyController->SetParentEntity( pEft );
				item.pEft = pEft;
				item.nCD = m_nPipeEftCD;
			}
			else
			{
				auto pEft = SafeCast<CWaterFall1>( item.pEft.GetPtr() );
				pEft->Kill();
				item.pEft = NULL;
				item.nCD = m_nPipeEftCD1;
			}
		}
	}
}

void CHouse0Pipe1Eft::OnRemovedFromStage()
{
	m_vecItems.clear();
	CAIObject::OnRemovedFromStage();
}

void CHouse0Pipe1Eft::AIFunc()
{
	int32 nMaxSize = 32;
	float fPos = 0;
	float f = 48.0f;
	while( 1 )
	{
		Yield( 0, true );
		auto pParent = SafeCast<CWaterFall1>( GetParentEntity() );
		if( pParent->GetFadeIn() > 0 )
		{
			float dPos = pParent->GetFadeInSpeed() * 0.5f * GetStage()->GetElapsedTimePerTick();
			for( int i = 0; i < m_vecItems.size(); i++ )
			{
				auto& item = m_vecItems[i];
				if( item.pRenderObject )
				{
					item.ofs.y -= dPos;
					item.pRenderObject->SetPosition( item.ofs );
					CVector2 pos = item.ofs + globalTransform.GetPosition();
					if( pos.y < 0 || item.ofs.y > -pParent->GetFadeOut() )
					{
						if( item.nType == 0 )
						{
							float fAngle = SRand::Inst().Rand( -1.2f, 1.2f );
							for( int n = 0; n < 3; n++ )
							{
								auto pBullet = SafeCast<CBullet>( m_pPrefab1->GetRoot()->CreateInstance() );
								pBullet->SetPosition( CVector2( pos.x, 0 ) );
								pBullet->SetRotation( PI * 0.5f - fAngle );
								pBullet->SetVelocity( CVector2( sin( fAngle ), cos( fAngle ) ) * ( 180.0f + 30 * n ) );
								pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
							}
						}
						else if( item.nType == 1 )
						{
							for( int n = 0; n < 3; n++ )
							{
								float fAngle = SRand::Inst().Rand( -1.2f, 1.2f );
								auto pBullet = SafeCast<CBullet>( m_pPrefab1->GetRoot()->CreateInstance() );
								pBullet->SetPosition( CVector2( pos.x, 0 ) );
								pBullet->SetRotation( PI * 0.5f - fAngle );
								pBullet->SetVelocity( CVector2( sin( fAngle ), cos( fAngle ) ) * SRand::Inst().Rand( 180.0f, 240.0f ) );
								pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
							}
						}
						else if( item.nType == 2 )
						{
							for( int n = 0; n < 3; n++ )
							{
								float fAngle = SRand::Inst().Rand( -1.2f, 1.2f );
								for( int n1 = 0; n1 < 3; n1++ )
								{
									auto pBullet = SafeCast<CBullet>( m_pPrefab1->GetRoot()->CreateInstance() );
									pBullet->SetPosition( CVector2( pos.x, 0 ) );
									pBullet->SetRotation( PI * 0.5f - fAngle );
									pBullet->SetVelocity( CVector2( sin( fAngle ), cos( fAngle ) ) * ( 180.0f + 30 * n1 ) );
									pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
								}
							}
						}
						else
						{
							for( int n = 0; n < 2; n++ )
							{
								float fAngle = SRand::Inst().Rand( -1.2f, 1.2f );
								auto pFly = SafeCast<CFly>( m_pPrefab->GetRoot()->CreateInstance() );
								pFly->SetPosition( CVector2( pos.x, 0 ) );
								pFly->SetVelocity( CVector2( sin( fAngle ), cos( fAngle ) ) * 400 );
								pFly->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
							}
						}

						item.pRenderObject->RemoveThis();
						item.pRenderObject = NULL;
						m_vecFree.push_back( i );
					}
				}
			}

			if( !pParent->IsKilled() )
			{
				fPos += dPos;
				while( fPos > f )
				{
					fPos -= f;
					int32 i;
					if( m_vecFree.size() )
					{
						i = m_vecFree.back();
						m_vecFree.pop_back();
					}
					else
					{
						i = m_vecItems.size();
						m_vecItems.resize( m_vecItems.size() + 1 );
					}
					auto& item = m_vecItems[i];
					item.ofs = CVector2( SRand::Inst().Rand( -20.0f, 20.0f ), -fPos );
					auto pImage2D = static_cast<CImage2D*>( m_pDrawable->CreateInstance() );
					item.pRenderObject = pImage2D;
					int32 nTex = SRand::Inst().Rand( 0, 32 );
					if( nTex < 12 )
						item.nType = 0;
					else if( nTex < 24 )
						item.nType = 1;
					else if( nTex < 28 )
					{
						item.nType = 2;
						fPos -= f;
					}
					else
						item.nType = 3;
					pImage2D->SetRect( CRectangle( -16, -16, 32, 32 ) );
					pImage2D->SetTexRect( CRectangle( ( nTex & 7 ) / 8.0f, ( nTex >> 3 ) / 4.0f, 1 / 8.0f, 1 / 4.0f ) );
					pImage2D->SetPosition( item.ofs );
					AddChild( pImage2D );
				}
			}
		}
	}
}

void CRoad0::OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel )
{
	int32 nWidth = pChunk->nWidth;
	int32 nHeight = pChunk->nHeight;
	m_nState = 0;
	m_nTime = 0;
	m_fHeight = -m_h1;
	m_nDir = nWidth < nHeight + SRand::Inst().Rand( 0, 2 );
	m_nDir += SRand::Inst().Rand( 0, 2 ) * 2;
	CVector2 dirs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

	vector<int8> data;
	data.resize( nWidth * pChunk->nHeight );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			data[i + j * nWidth] = pChunk->GetBlock( i, j )->nTag;
		}
	}

	vector<TRectangle<int32> > vecRect;
	vector<TVector2<int32> > vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( data[i + j * nWidth] )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		int8& n = data[p.x + p.y * nWidth];
		if( n != 1 && n != 2 )
			continue;
		auto rect1 = PutRect( data, nWidth, nHeight, p, n == 1 ? TVector2<int32>( 1, 1 ) : TVector2<int32>( 2, 2 ),
			n == 1 ? TVector2<int32>( nWidth, 1 ) : TVector2<int32>( nWidth, 2 ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, n );
		auto rect2 = PutRect( data, nWidth, nHeight, p, n == 1 ? TVector2<int32>( 1, 1 ) : TVector2<int32>( 2, 2 ),
			n == 1 ? TVector2<int32>( 1, nHeight ) : TVector2<int32>( 2, nHeight ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, n );
		auto rect = rect1.width * rect1.height >= rect2.width * rect2.height ? rect1 : rect2;
		if( rect.width * rect.height < ( n == 1 ? 2 : 4 ) )
		{
			n = 0;
			pChunk->GetBlock( p.x, p.y )->nTag = 0;
			continue;
		}

		vecRect.push_back( rect );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				data[i + j * nWidth] = 4;
			}
		}
	}
	bool b = vecRect.size();

	vec.clear();
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( !data[i + j * nWidth] )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( data[p.x + p.y * nWidth] )
			continue;
		auto rect1 = PutRect( data, nWidth, nHeight, p, TVector2<int32>( 1, 1 ), TVector2<int32>( 2, 2 ),
			TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 3 );
		vecRect.push_back( rect1 );
	}

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
	auto texRect = pImage2D->GetElem().texRect;
	texRect.width /= 4;
	texRect.height /= 16;

	SetRenderObject( new CRenderObject2D );

	for( auto& rect : vecRect )
	{
		int8 n = data[rect.x + rect.y * nWidth];
		uint32 tX, tY;
		uint8 nType = 0;
		do
		{
			if( n == 3 )
			{
				if( rect.width == 2 && rect.height == 2 )
				{
					tX = SRand::Inst().Rand( 0, 2 ) * 2;
					tY = 4;
				}
				else if( rect.width == 2 )
				{
					tX = 0;
					tY = SRand::Inst().Rand( 2, 4 );
				}
				else if( rect.height == 2 )
				{
					tX = SRand::Inst().Rand( 2, 4 );
					tY = 2;
				}
				else
				{
					tX = SRand::Inst().Rand( 0, 4 );
					tY = SRand::Inst().Rand( 0, 2 );
				}
				break;
			}
			if( rect.width == 2 && rect.height == 2 )
			{
				tX = SRand::Inst().Rand( 0, 2 ) * 2;
				tY = 6;
				SItem item = { { rect.x, rect.y }, 4 };
				m_vecSewers.push_back( item );
				break;
			}
			if( rect.height == 1 && rect.width == 2 )
			{
				int8 i1 = !!( rect.y == nHeight - 1 ) - !!( rect.y == 0 );
				if( i1 == -1 )
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, 1 };
							m_vecSewers.push_back( item );
						}
					}
					tX = 0; tY = 12; break;
				}
				else if( i1 == 1 )
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, 3 };
							m_vecSewers.push_back( item );
						}
					}
					tX = 2; tY = 12; break;
				}
			}
			if( rect.width == 1 && rect.height == 2 )
			{
				int8 i1 = !!( rect.x == nWidth - 1 ) - !!( rect.x == 0 );
				if( i1 == -1 )
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, 0 };
							m_vecSewers.push_back( item );
						}
					}
					tX = 0; tY = 8; break;
				}
				else if( i1 == 1 )
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, 2 };
							m_vecSewers.push_back( item );
						}
					}
					tX = 0; tY = 10; break;
				}
			}
			if( rect.width > rect.height )
			{
				nType = 1;
				if( rect.height == 1 )
				{
					int8 i1 = !!( rect.y == nHeight - 1 ) - !!( rect.y == 0 );
					if( i1 == -1 )
					{
						tX = 0; tY = 14;
					}
					else if( i1 == 1 )
					{
						tX = 0; tY = 15;
					}
					else
					{
						tX = 0; tY = 13;
					}

					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, SRand::Inst().Rand( 0, 2 ) * 2 + 1 };
							m_vecSewers.push_back( item );
						}
					}
					break;
				}
				else
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, y == rect.y ? 3 : 1 };
							m_vecSewers.push_back( item );
						}
					}
					tX = 0; tY = 14; break;
				}
			}
			else
			{
				nType = 2;
				if( rect.width == 1 )
				{
					int8 i1 = !!( rect.x == nWidth - 1 ) - !!( rect.x == 0 );
					if( i1 == -1 )
					{
						tX = 3; tY = 8;
					}
					else if( i1 == 1 )
					{
						tX = 2; tY = 8;
					}
					else
					{
						tX = 1; tY = 8;
					}

					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, SRand::Inst().Rand( 0, 2 ) * 2 };
							m_vecSewers.push_back( item );
						}
					}
					break;
				}
				else
				{
					for( int x = rect.x; x < rect.GetRight(); x++ )
					{
						for( int y = rect.y; y < rect.GetBottom(); y++ )
						{
							SItem item = { { x, y }, x == rect.x ? 2 : 0 };
							m_vecSewers.push_back( item );
						}
					}
					tX = 2; tY = 8; break;
				}
			}
		} while( 0 );

		for( int i = 0; i < rect.width; i++ )
		{
			uint32 tX1 = tX + ( nType == 1 ? ( i == 0 ? 0 : ( i == rect.width - 1 ? 3 : SRand::Inst().Rand( 1, 3 ) ) ) : i );
			for( int j = 0; j < rect.height; j++ )
			{
				uint32 tY1 = tY + ( nType == 2 ? ( j == 0 ? 3 : ( j == rect.height - 1 ? 0 : SRand::Inst().Rand( 1, 3 ) ) ) : rect.height - 1 - j );
				int32 x = i + rect.x;
				int32 y = j + rect.y;
				auto tex = texRect.Offset( CVector2( texRect.width * tX1, texRect.height * tY1 ) );

				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( x * 32, y * 32, 32, 32 ) );
				pImage2D->SetTexRect( tex );
				GetRenderObject()->AddChild( pImage2D );
				GetBlock( x, y )->rtTexRect = tex;
				if( b )
				{
					CImage2D* pImage2D1 = static_cast<CImage2D*>( m_pDrawable1->CreateInstance() );
					pImage2D1->SetRect( CRectangle( x * 32, y * 32, 32, 32 ) );
					pImage2D1->SetTexRect( CRectangle( tX1 / 4.0f, tY1 / 16.0f, 1.0f / 4, 1.0f / 16 ) );
					pImage2D->AddChild( pImage2D1 );
					pImage2D1->SetRenderParentBefore( m_pDamagedEffectsRoot );
					*pImage2D1->GetParam() = CVector4( 0, m_h1, dirs[m_nDir].x, dirs[m_nDir].y );
					m_vecSewerImage.push_back( pImage2D1 );
				}

				for( int i = 0; i < m_nDamagedEffectsCount; i++ )
				{
					CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
					pImage2D->SetRect( CRectangle( x * 32, y * 32, 32, 32 ) );
					pImage2D->SetTexRect( pDamageEftTex[i] );
					m_pDamagedEffects[i]->AddChild( pImage2D );
				}
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;

	if( !b )
		return;
	UpdateImages();

	if( !pLevel )
		return;
	pLevel->GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CRoad0::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CChunkObject::OnRemovedFromStage();
}

void CRoad0::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
	auto blockSize = CMyLevel::GetBlockSize();
	if( m_nState == 0 )
	{
		if( m_nTime < m_nTime1 )
			m_nTime++;
		if( m_nTime >= m_nTime1 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				return;
			CRectangle rect( m_pChunk->pos.x, m_pChunk->pos.y, m_pChunk->nWidth * blockSize, m_pChunk->nHeight * blockSize );
			rect.x += m_detectRect.x;
			rect.y += m_detectRect.y;
			rect.width += m_detectRect.width;
			rect.height += m_detectRect.height;
			if( rect.Contains( pPlayer->GetPosition() ) )
			{
				m_nTime = 0;
				m_nState = 1;
			}
		}
	}
	else if( m_nState == 1 )
	{
		float fMaxHeight = !!( m_nDir & 1 ) ? m_pChunk->nHeight * blockSize : m_pChunk->nWidth * blockSize;
		m_fHeight += m_fSpeed1 * ( fMaxHeight / 256.0f ) * GetStage()->GetElapsedTimePerTick();
		if( m_fHeight >= fMaxHeight )
		{
			m_fHeight = fMaxHeight;
			m_nState = 2;
			m_nTime = 0;
		}
	}
	else if( m_nState == 2 )
	{
		m_nTime++;
		if( m_nTime >= m_nTime2 )
		{
			m_nTime = 0;
			m_nState = 3;
		}
	}
	else
	{
		float fMaxHeight = !!( m_nDir & 1 ) ? m_pChunk->nHeight * blockSize : m_pChunk->nWidth * blockSize;
		float fLastHeight = m_fHeight;
		m_fHeight -= m_fSpeed1 * ( fMaxHeight / 256.0f ) * GetStage()->GetElapsedTimePerTick();
		if( m_fHeight <= -m_h1 )
		{
			m_fHeight = -m_h1;
			m_nState = 0;
			m_nTime = 0;
		}
		UpdateSewers( fLastHeight, m_fHeight );
	}
	UpdateImages();
}

void CRoad0::UpdateSewers( float fLastHeight, float fCurHeight )
{
	for( auto& item : m_vecSewers )
	{
		TRectangle<int32> r( item.pos.x, item.pos.y, 1, 1 );
		if( item.nType == 4 )
			r.width = r.height = 2;
		bool bBreak = false;
		for( int i = r.x; i < r.GetRight()&& !bBreak; i++ )
		{
			for( int j = r.y; j < r.GetBottom() && !bBreak; j++ )
			{
				for( auto pManifold = GetBlock( i, j )->pEntity->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
				{
					auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );
					if( pEntity->GetHitType() > eEntityHitType_Platform )
						continue;
					if( pManifold->normal.Length2() >= 4 * 4 )
					{
						bBreak = true;
						break;
					}
				}
			}
		}
		if( bBreak )
			continue;

		CVector2 center( item.pos.x, item.pos.y );
		center = center + ( item.nType == 4 ? CVector2( 1, 1 ) : CVector2( 0.5f, 0.5f ) );
		float h;
		if( m_nDir == 0 )
			h = center.x;
		else if( m_nDir == 1 )
			h = center.y;
		else if( m_nDir == 2 )
			h = m_pChunk->nWidth - center.x;
		else
			h = m_pChunk->nHeight - center.y;
		h *= CMyLevel::GetBlockSize();

		if( item.nType == 4 )
		{
			int32 n1 = Max( 0, Min( 8, (int32)floor( ( fLastHeight - ( h - 32 ) ) / 8.0f + 0.5f ) ) );
			int32 n2 = Max( 0, Min( 8, (int32)floor( ( fCurHeight - ( h - 32 ) ) / 8.0f + 0.5f ) ) );
			for( int i = n1; i > n2; i-- )
			{
				float fAngle = SRand::Inst().Rand( -PI, PI );
				CBullet* pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() + center * CMyLevel::GetBlockSize() );
				pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( m_fBulletSpeedMin, m_fBulletSpeedMax ) );
				pBullet->SetAcceleration( CVector2( 0, -m_fBulletGravity ) );
				pBullet->SetLife( m_nBulletLife );
				pBullet->SetCreator( this );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			}
		}
		else
		{
			if( fLastHeight > h && fCurHeight <= h )
			{
				CVector2 dirs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
				CBullet* pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() + center * CMyLevel::GetBlockSize() );
				pBullet->SetVelocity( dirs[item.nType] * SRand::Inst().Rand( m_fBulletSpeedMin, m_fBulletSpeedMax ) );
				pBullet->SetAcceleration( CVector2( 0, -m_fBulletGravity ) );
				pBullet->SetLife( m_nBulletLife );
				pBullet->SetCreator( this );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			}
		}
	}
}

void CRoad0::UpdateImages()
{
	float fSurface = m_fHeight;
	if( m_nDir == 0 )
		fSurface = globalTransform.GetPosition().x + fSurface;
	else if( m_nDir == 1 )
		fSurface = globalTransform.GetPosition().y + fSurface;
	else if( m_nDir == 2 )
		fSurface = -globalTransform.GetPosition().x - m_pChunk->nWidth * CMyLevel::GetBlockSize() + fSurface;
	else
		fSurface = -globalTransform.GetPosition().y - m_pChunk->nHeight * CMyLevel::GetBlockSize() + fSurface;
	for( CRenderObject2D* p : m_vecSewerImage )
	{
		auto pImage = static_cast<CImage2D*>( p );
		pImage->GetParam()[0].x = fSurface;
	}
}

void CAirConditioner::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	uint8 bLeft = SRand::Inst().Rand( 0, 2 );
	m_bLeft = bLeft;
	auto pImage = static_cast<CImage2D*>( m_pDrawable1->CreateInstance() );
	pImage->SetRect( CRectangle( bLeft ? 0 : 1, 0, 2, 2 ) * CMyLevel::GetBlockSize() );
	pImage->SetTexRect( CRectangle( SRand::Inst().Rand( 0, 2 ), SRand::Inst().Rand( 0, 2 ), 1, 1 ) * 0.5f );
	GetRenderObject()->AddChild( pImage );
	pImage->SetRenderParent( this );

	auto pImage1 = static_cast<CImage2D*>( m_pDrawable2->CreateInstance() );
	pImage1->SetRect( CRectangle( bLeft ? 2 * CMyLevel::GetBlockSize() - 4 : 4, ( 0.25f + SRand::Inst().Rand( 0, 2 ) ) * CMyLevel::GetBlockSize(),
		CMyLevel::GetBlockSize(), 0.5f * CMyLevel::GetBlockSize() ) );
	pImage1->SetTexRect( CRectangle( SRand::Inst().Rand( 0, 2 ) * 0.5f, SRand::Inst().Rand( 0, 4 ) * 0.25f, 0.5f, 0.25f ) );
	GetRenderObject()->AddChild( pImage1 );
	pImage1->SetRenderParent( this );

	if( CMyLevel::GetInst() )
	{
		m_pAI = new AI();
		m_pAI->SetParentEntity( this );
	}
}

const CMatrix2D& CAirConditioner::GetTransform( uint16 nIndex )
{
	if( nIndex >= m_vecTrans.size() )
		return globalTransform;
	if( m_bDirty )
	{
		m_vecGlobalTrans[0] = globalTransform;
		m_vecGlobalTrans[0].SetPosition( m_vecGlobalTrans[0].GetPosition() + GetCenter() );
		for( int i = 1; i < m_vecGlobalTrans.size(); i++ )
			m_vecGlobalTrans[i] = globalTransform * m_vecTrans[i - 1];
		m_bDirty = false;
	}
	return m_vecGlobalTrans[nIndex];
}

void CAirConditioner::OnTransformUpdated()
{
	m_bDirty = true;
}

void CAirConditioner::OnRemovedFromStage()
{
	CChunkObject::OnRemovedFromStage();
	m_vec.clear();
}

void CAirConditioner::AIFunc()
{
	uint32 nType = SRand::Inst().Rand( 0, 3 );
	int32 nMaxSize = 32;
	m_vecTrans.resize( nMaxSize );
	m_vecGlobalTrans.resize( nMaxSize + 1 );
	m_vec.resize( nMaxSize );
	m_vecFree.reserve( nMaxSize );
	for( int i = 0; i < m_vecTrans.size(); i++ )
	{
		m_vecTrans[i].Identity();
	}
	if( nType == 0 )
		AIFunc1();
	else if( nType == 1 )
		AIFunc2();
	else
		AIFunc3();
}

void CAirConditioner::AIFunc1()
{
	uint32 n = 0;
	int8 b = SRand::Inst().Rand( 0, 2 ) * 2 - 1;

	while( 1 )
	{
		AIOnTick();
		for( int i = 4; i < 32; i++ )
		{
			if( m_vec[i] && !m_vec[i - 4] )
			{
				m_vec[i - 4] = m_vec[i];
				m_vec[i] = NULL;
				SafeCast<CFly>( m_vec[i - 4].GetPtr() )->Set( this, i - 4 );
			}
		}

		n = ( n + 1 ) % 384;
		float fAngle = n * PI * 2 / 384;
		CVector2 center = GetCenter();
		for( int i = 0; i < 32; i++ )
		{
			float fAngle1 = ( fAngle + ( i & 3 ) * PI * 0.5f ) * b;
			float fRad = ( i >> 2 ) * 48 + 24;
			m_vecTrans[i].Translate( cos( fAngle1 ) * fRad + center.x, sin( fAngle1 ) * fRad + center.y );
		}
		m_bDirty = true;
	}
}

void CAirConditioner::AIFunc2()
{
	CVector2 center = GetCenter();
	for( int i = 0; i < 32; i++ )
	{
		if( !( i % 8 ) )
		{
			float r = SRand::Inst().Rand( 250.0f, 500.0f );
			float a = SRand::Inst().Rand( -PI, PI );
			m_vecTrans[i].Translate( cos( a ) * r + center.x, sin( a ) * r + center.y );
		}
		else
			m_vecTrans[i].Identity();
	}
	CVector2 target[4];
	int32 n[4];
	while( 1 )
	{
		AIOnTick();
		for( int i = 1; i < 32; i++ )
		{
			if( m_vec[i] && !m_vec[i - 1] )
			{
				m_vec[i - 1] = m_vec[i];
				m_vec[i] = NULL;
				SafeCast<CFly>( m_vec[i - 1].GetPtr() )->Set( this, i - 1 );
			}
		}

		for( int i = 0; i < 4; i++ )
		{
			CVector2 p = m_vecTrans[i * 8].GetPosition() - center;
			if( ( p - target[i] ).Length2() <= 8 * 8 )
			{
				CVector2 tar = p * SRand::Inst().Rand( -0.5f, -1.0f ) + CVector2( -p.y, p.x ) * SRand::Inst().Rand( -1.0f, 1.0f );
				float fLen = SRand::Inst().Rand( 250.0f, 500.0f );
				tar.Normalize();
				tar = tar * fLen;
				target[i] = tar;
			}
			CVector2 tar = target[i];
			for( int j = 0; j < 8; j++ )
			{
				p = m_vecTrans[i * 8 + j].GetPosition();
				CVector2 d = tar - p;
				float l = d.Length();
				float fSpeed = l * 0.25f + 50;
				float dl = fSpeed * GetStage()->GetElapsedTimePerTick();
				if( l <= dl )
					p = tar;
				else
					p = p + d * ( ( l - dl ) / l );
				m_vecTrans[i * 8 + j].SetPosition( p + center );
				tar = p;
			}
		}
		m_bDirty = true;
	}
}

void CAirConditioner::AIFunc3()
{
	SCharacterChainMovementData data;
	uint32 nSegs = 17;
	data.fDamping = 1.0f;
	data.SetCharacterCount( nSegs );
	for( int i = 0; i < nSegs; i++ )
	{
		data.vecPos[i] = CVector2( 0, 32 ) * i;
		data.vecVel[i] = CVector2( 0, 0 );
		if( i < nSegs - 1 )
		{
			data.vecLen[i] = 32;
			data.vecK[i] = 40.0f;
		}
		if( i > 0 )
		{
			data.vecInvWeight[i] = 1;
			if( i < nSegs - 1 )
			{
				data.vecExtraAcc[i] = CVector2( 0, -200 );
				data.vecInvWeight[i] = 5;
			}
		}
		data.vecAngleLim[i] = 0.4f;
		data.vecK1[i] = 100.0f;
	}
	uint32 n = 0;
	int32 n1 = 0;
	CVector2 p = GetCenter() + globalTransform.GetPosition();
	while( 1 )
	{
		AIOnTick();
		int32 nFly = 0;
		for( int i = 0; i < 32; i++ )
		{
			if( m_vec[i] )
				nFly++;
		}
		if( nFly )
		{
			int32 nFly1 = 0;
			for( int i = 0; i < 32; i++ )
			{
				if( m_vec[i] )
				{
					int32 nTarget = floor( ( nFly1 + 0.5f ) * 32 / nFly );
					if( nTarget < i )
					{
						if( !m_vec[i - 1] )
						{
							m_vec[i - 1] = m_vec[i];
							m_vec[i] = NULL;
							SafeCast<CFly>( m_vec[i - 1].GetPtr() )->Set( this, i - 1 );
						}
					}
					else if( nTarget > i )
					{
						if( !m_vec[i + 1] )
						{
							m_vec[i + 1] = m_vec[i];
							m_vec[i] = NULL;
							SafeCast<CFly>( m_vec[i + 1].GetPtr() )->Set( this, i + 1 );
							i++;
						}
					}
					nFly1++;
				}
			}
		}

		CVector2 center = GetCenter() + globalTransform.GetPosition();
		if( n )
			n--;
		if( !n1 && !n )
		{
			if( GetStage()->GetPlayer() )
			{
				p = GetStage()->GetPlayer()->globalTransform.GetPosition();
				CVector2 d = p - center;
				if( d.Length2() > 300 * 300 )
				{
					d.Normalize();
					p = d * 300 + center;
				}
				d = p - ( center + data.vecPos.back() );
				float l = d.Normalize();
				p = p + d * 25 + CVector2( 0, 60 );
				n1 = 60;
			}
		}
		if( n1 )
		{
			data.vecInvWeight.back() = 0.5f;
			CVector2 d = p - ( center + data.vecPos.back() );
			float l = d.Normalize();
			n1 = Max( 0, n1 - Max<int32>( floor( 200 - l ) / 40, 1 ) );
			data.vecExtraAcc.back() = d * 2500;
			if( !n1 )
				n = 300;
		}
		else
		{
			CVector2 acc( 0, 300 - data.vecPos.back().y );
			if( GetStage()->GetPlayer() )
			{
				p = GetStage()->GetPlayer()->globalTransform.GetPosition();
				CVector2 d = p - ( center + data.vecPos.back() );
				acc.x = d.x > 0 ? -100.0f : 100.0f;
			}
			data.vecExtraAcc.back() = acc;
			data.vecInvWeight.back() = 1;
		}

		data.Simulate( GetStage()->GetElapsedTimePerTick(), 3, NULL, 0 );
		for( int i = 0; i < m_vecTrans.size(); i++ )
		{
			if( !( i & 1 ) )
				m_vecTrans[i].SetPosition( GetCenter() + ( data.vecPos[i / 2] + data.vecPos[i / 2 + 1] ) * 0.5f );
			else
				m_vecTrans[i].SetPosition( GetCenter() + data.vecPos[i / 2 + 1] );
		}
		m_bDirty = true;
	}
}

void CAirConditioner::AIOnTick()
{
	m_pAI->Yield( 0, false );

	float r = SRand::Inst().Rand( 0.0f, 300.0f );
	float a = SRand::Inst().Rand( -PI, PI );
	SHitProxyCircle circle;
	circle.center = CVector2( cos( a ) * r, sin( a ) * r );
	circle.fRadius = 30;
	vector<CReference<CEntity> > result;
	GetStage()->MultiHitTest( &circle, globalTransform, result );
	m_vecFree.resize( 0 );
	for( int i = 0; i < m_vec.size(); i++ )
	{
		if( !m_vec[i] || !m_vec[i]->GetStage() )
		{
			m_vec[i] = NULL;
			m_vecFree.push_back( i );
		}
	}
	for( CEntity* pEntity : result )
	{
		auto pFly = SafeCast<CFly>( pEntity );
		if( pFly )
		{
			if( pFly->GetTarget() )
				continue;
		}
		else
		{
			auto p = SafeCast<CMaggot>( pEntity );
			if( !p )
				continue;
			pFly = p->Morph();
		}
		uint32 n;
		if( m_vecFree.size() )
		{
			int32 i = SRand::Inst().Rand<int32>( 0, m_vecFree.size() );
			n = m_vecFree[i];
			m_vecFree[i] = m_vecFree.back();
			m_vecFree.pop_back();
		}
		else
		{
			n = SRand::Inst().Rand<int32>( 0, m_vec.size() );
			SafeCast<CFly>( m_vec[n].GetPtr() )->Set( NULL );
			m_vec[n] = NULL;
		}
		m_vec[n] = pFly;
		pFly->Set( this, n );
	}
}

CVector2 CAirConditioner::GetCenter()
{
	return ( m_bLeft ? CVector2( 32, 32 ) : CVector2( 64, 32 ) );
}

void CScrap::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );

	auto pRenderObject = new CRenderObject2D;
	SetRenderObject( pRenderObject );
	int32 w = pChunk->nWidth;
	int32 h = pChunk->nHeight;
	bool bFlip = false;
	if( w < h )
	{
		bFlip = true;
		swap( w, h );
	}

	int32 nCount = ( w + 6 ) / 7;
	int32* nLens = (int32*)alloca( sizeof( int32 ) * nCount );
	vector<CImage2D*> vecImages;
	for( int j = 0; j < h; j++ )
	{
		for( int k = 0; k < 2; k++ )
		{
			int32 s = w;
			for( int i = 0; i < nCount; i++ )
			{
				int32 nMin = Max( 1, s - ( nCount - i - 1 ) * 7 );
				int32 nMax = Min( 7, s - ( nCount - i - 1 ) );
				int32 w1 = SRand::Inst().Rand( nMin, nMax + 1 );
				nLens[i] = w1;
				s -= w1;
			}
			SRand::Inst<eRand_Render>().Shuffle( nLens, nCount );
			s = 0;
			for( int i = 0; i < nCount; i++ )
			{
				auto pImage = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage->SetRect( CRectangle( k ? w - s - nLens[i] : s, j, nLens[i], 1 ) * CMyLevel::GetBlockSize() );
				pImage->SetTexRect( CRectangle( k ? 1 - nLens[i] / 8.0f : 0, SRand::Inst<eRand_Render>().Rand( 0, 8 ) / 8.0f, nLens[i] / 8.0f, 1.0f / 8 ) );
				vecImages.push_back( pImage );
				s += nLens[i];
			}
		}
	}
	SRand::Inst<eRand_Render>().Shuffle( vecImages );
	for( auto pImage : vecImages )
	{
		if( bFlip )
		{
			pImage->SetRotation( PI * 0.5f );
			pImage->SetPosition( CVector2( CMyLevel::GetBlockSize() * h, 0 ) );
		}

		pRenderObject->AddChild( pImage );
	}

	if( m_pDrawable1 )
	{
		static CRectangle signTexRectsH[] =
		{
			{ 0, 0, 4, 2 }, { 4, 0, 5, 2 }, { 9, 0, 5, 2 },
			{ 0, 2, 6, 2 }, { 6, 2, 6, 2 },
			{ 0, 4, 4, 2 }, { 4, 4, 8, 2 },
			{ 0, 6, 6, 2 }, { 6, 6, 4, 2 },
			{ 0, 8, 3, 2 }, { 3, 8, 3, 2 }, { 6, 8, 4, 2 },
			{ 0, 10, 4, 2 }, { 4, 10, 4, 2 },
			{ 0, 12, 4, 2 }, { 4, 12, 4, 2 },
			{ 0, 14, 4, 2 }, { 4, 14, 4, 2 },
		};
		static CRectangle signTexRectsV1[] =
		{
			{ 14, 0, 1, 2 }, { 15, 0, 1, 2 }, { 14, 2, 1, 2 }, { 15, 2, 1, 2 }, { 14, 4, 1, 2 }, { 15, 4, 1, 2 },
			{ 14, 6, 1, 2 }, { 15, 6, 1, 2 }, { 14, 8, 1, 2 }, { 15, 8, 1, 2 },
		};
		static CRectangle signTexRectsV2[] =
		{
			{ 14, 10, 2, 3 }, { 14, 13, 2, 3 },
			{ 12, 2, 2, 4 }, { 12, 6, 2, 5 }, { 12, 11, 2, 5 },
			{ 10, 6, 2, 6 }, { 10, 12, 2, 4 },
			{ 8, 10, 2, 3 }, { 8, 13, 2, 3 },
		};
		static bool bInit = false;
		if( !bInit )
		{
			bInit = true;
			SRand::Inst<eRand_Render>().Shuffle( signTexRectsH, ELEM_COUNT( signTexRectsH ) );
			SRand::Inst<eRand_Render>().Shuffle( signTexRectsV1, ELEM_COUNT( signTexRectsV1 ) );
			SRand::Inst<eRand_Render>().Shuffle( signTexRectsV2, ELEM_COUNT( signTexRectsV2 ) );
		}

		CRectangle* pRect = NULL;
		int32 nRect = 0;
		int32 h1 = 0;
		if( !bFlip )
		{
			if( h <= 1 )
				return;
			pRect = signTexRectsH;
			nRect = ELEM_COUNT( signTexRectsH );
			h1 = 2;
		}
		else
		{
			if( h <= 1 || w <= SRand::Inst().Rand( 3, 7 ) )
			{
				pRect = signTexRectsV1;
				nRect = ELEM_COUNT( signTexRectsV1 );
				h1 = 1;
			}
			else
			{
				pRect = signTexRectsV2;
				nRect = ELEM_COUNT( signTexRectsV2 );
				h1 = 2;
			}
		}
		int32 w1 = Max( h, w - SRand::Inst().Rand( 0, 3 ) );

		for( int i = 0; i <= h - h1; i += h1 )
		{
			CRectangle r( 0, 0, 0, 0 );
			for( int iRect = 0; iRect < nRect; iRect++ )
			{
				auto r1 = pRect[iRect];
				if( ( bFlip ? r1.height : r1.width ) < w1 )
				{
					r = r1;
					for( int i1 = iRect; i1 < nRect - 1; i1++ )
						pRect[i1] = pRect[i1 + 1];
					pRect[nRect - 1] = r1;
					break;
				}
			}
			if( r.width > 0 )
			{
				auto pImage = static_cast<CImage2D*>( m_pDrawable1->CreateInstance() );
				pImage->SetTexRect( r * ( 1.0f / 16 ) );
				if( bFlip )
				{
					r.x = i;
					r.y = SRand::Inst().Rand<int32>( 0, w - r.height + 1 );
				}
				else
				{
					r.x = SRand::Inst().Rand<int32>( 0, w - r.width + 1 );
					r.y = i;
				}
				pImage->SetRect( r * CMyLevel::GetBlockSize() );
				pRenderObject->AddChild( pImage );
			}
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CScrap::OnCreateComplete( CMyLevel* pLevel )
{
	GetRenderObject()->SetZOrder( 1 );
	if( pLevel )
		GetRenderObject()->SetRenderParent( m_pChunk->nShowLevelType ? pLevel->GetChunkRoot1() : pLevel->GetChunkRoot() );
}
