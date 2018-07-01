#include"stdafx.h"
#include "BlockItemsLv1.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Barrage.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Common/ResourceManager.h"
#include "Entities/Bullets.h"
#include "Common/Algorithm.h"

void CPipe0::Trigger()
{
	SBarrageContext context;
	context.pCreator = GetParentEntity();
	context.vecBulletTypes.push_back( m_strPrefab.GetPtr() );
	context.nBulletPageSize = 80;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [this]( CBarrage* pBarrage )
	{
		for( int i = 0; i < 20; i++ )
		{
			float fAngle = SRand::Inst().Rand( -0.2f, 0.2f );
			pBarrage->InitBullet( i * 4, -1, -1, CVector2( SRand::Inst().Rand( -12.0f, 12.0f ), 0 ), CVector2( 150 * sin( fAngle ), -150 * cos( fAngle ) ), CVector2( 0, 0 ), false, SRand::Inst().Rand( -PI, PI ), SRand::Inst().Rand( -6.0f, 6.0f ) );

			for( int j = 1; j <= 3; j++ )
				pBarrage->InitBullet( i * 4 + j, 0, i * 4, CVector2( SRand::Inst().Rand( -6.0f, 6.0f ), SRand::Inst().Rand( -6.0f, 6.0f ) ),
					CVector2( 0, 0 ), CVector2( 0, 0 ), true );

			pBarrage->Yield( 3 );
		}
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( globalTransform.GetPosition() );
	pBarrage->SetRotation( r );
	pBarrage->Start();
}

void CWindow::AIFunc()
{
	if( m_pDeathEffect )
		m_pDeathEffect->SetParentEntity( NULL );
	if( !CMyLevel::GetInst() )
		return;

	while( 1 )
	{
		m_pAI->Yield( 0.5f, true );
		CRectangle rect;
		Get_HitProxy()->CalcBound( globalTransform, rect );
		if( CMyLevel::GetInst()->GetBound().GetBottom() > rect.GetBottom() )
			break;
	}

	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBullet1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );
	m_pHead[0] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead.c_str() );
	m_pHead[1] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead1.c_str() );
	m_pHead[2] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead2.c_str() );
	m_pHead[3] = CResourceManager::Inst()->CreateResource<CPrefab>( m_strHead3.c_str() );
	CReference<CPrefab> pBullet1 = m_pBullet1;

	uint32 nAttackCount = 0;
	//alive
	while( 1 )
	{
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 1, 0 );
		m_pMan->bVisible = false;

		//closed
		while( 1 )
		{
			m_pAI->Yield( 0.5f, true );
			vector<CReference<CEntity> > hitEntities;
			GetStage()->MultiHitTest( Get_HitProxy(), globalTransform, hitEntities );
			bool bHit = false;
			for( CEntity* pEntity : hitEntities )
			{
				if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
				{
					bHit = true;
					break;
				}
			}
			if( bHit )
				continue;

			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
				if( m_openRect.Contains( pos ) )
					break;
			}

			auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
			if( pChunkObject && pChunkObject->GetMaxHp() && pChunkObject->GetHp() / pChunkObject->GetMaxHp() < 0.5f )
				break;
		}

		//opening
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlaySpeed( 1, false );
		m_pAI->Yield( 0.25f, false );
		m_pMan->bVisible = true;
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( 1, false );
		m_pAI->Yield( 0.75f, false );
		m_pMan->MoveToTopmost();

		//open
		while( 1 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				break;

			float fDeathChance = nAttackCount / ( nAttackCount + 1.0f );
			auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
			if( pChunkObject && pChunkObject->GetMaxHp() )
				fDeathChance = 1 - ( 1 - fDeathChance ) * ( pChunkObject->GetHp() / pChunkObject->GetMaxHp() );
			if( SRand::Inst().Rand( 0.0f, 1.0f ) < fDeathChance )
			{
				goto dead;
			}

			CVector2 playerPos = pPlayer->GetPosition();
			CVector2 dPos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
			if( !m_closeRect.Contains( dPos ) )
				break;

			//disappear
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 4, 8, 8 );
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( 1, false );
			m_pAI->Yield( 0.5f, false );
			m_pMan->bVisible = false;
			m_pAI->Yield( 0.5f, false );

			//attack
			nAttackCount++;
			for( int i = 0; i < 3; i++ )
			{
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( pPlayer )
					playerPos = pPlayer->GetPosition();
				dPos = playerPos - globalTransform.GetPosition();
				dPos = dPos + CVector2( SRand::Inst().Rand( -32.0f, 32.0f ), SRand::Inst().Rand( -32.0f, 32.0f ) );

				auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetPosition( globalTransform.GetPosition() );
				CVector2 acc = CVector2( 0, -100 );
				CVector2 velocity;

				float t = dPos.Length() / 250.0f * SRand::Inst().Rand( 0.9f, 1.1f ) + SRand::Inst().Rand( 0.1f, 0.4f );
				velocity.y = dPos.y / t - 0.5f * acc.y * t;
				velocity.x = dPos.x / t;

				pBullet->SetVelocity( velocity );
				pBullet->SetAcceleration( acc );
				pBullet->SetAngularVelocity( SRand::Inst().Rand( 4.0f, 8.0f ) * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 ) );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				pBullet->SetOnHit( [pBullet1]( CBullet* pThis, CEntity* pEntity )
				{
					SBarrageContext context;
					context.vecBulletTypes.push_back( pBullet1 );
					context.nBulletPageSize = 30;

					CBarrage* pBarrage = new CBarrage( context );
					CVector2 pos = pThis->globalTransform.GetPosition();
					pBarrage->AddFunc( []( CBarrage* pBarrage )
					{
						float fAngle0 = SRand::Inst().Rand( -PI, PI );
						for( int i = 0; i < 10; i++ )
						{
							for( int j = 0; j < 3; j++ )
							{
								float fAngle1 = fAngle0 + j * ( PI * 2 / 3 ) + SRand::Inst().Rand( -0.1f, 0.1f );
								float fSpeed = 200 - i * 3 + SRand::Inst().Rand( -20.0f, 20.0f );
								pBarrage->InitBullet( i * 3 + j, 0, -1, CVector2( 0, 0 ), CVector2( fSpeed * cos( fAngle1 ), fSpeed * sin( fAngle1 ) ), CVector2( 0, 0 ) );
							}
							pBarrage->Yield( 2 );
						}
						pBarrage->StopNewBullet();
					} );
					pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					pBarrage->SetPosition( pos );
					pBarrage->Start();
				} );

				m_pAI->Yield( 0.5f, false );
			}

			//reappear
			m_pMan->bVisible = true;
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 3, 7, 8 );
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlayPercent( 1 );
			static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( -1, false );
			m_pAI->Yield( 1.0f, false );
		}

		//closing
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.25f, false );
		m_pWindow->MoveToTopmost();
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.75f, false );
	}
	
	//dead
dead:
	if( m_pDeathEffect )
	{
		m_pDeathEffect->SetParentEntity( this );
		m_pDeathEffect->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pDeathEffect->SetState( 2 );
	}
	if( m_pSpawner )
		m_pSpawner->SetEnabled( true );

	static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetFrames( 8, 12, 2 );
	static_cast<CMultiFrameImage2D*>( m_pMan.GetPtr() )->SetPlaySpeed( 1, false );
	float fLevelHeight = CMyLevel::GetInst()->GetBoundWithLvBarrier().height;
	auto pHead = SafeCast<CEntity>( m_pHead[SRand::Inst().Rand( 0, 2 ) * 2 + ( globalTransform.GetPosition().y < fLevelHeight * 0.5f ? 0 : 1 )]->GetRoot()->CreateInstance() );
	pHead->SetPosition( globalTransform.GetPosition() );
	pHead->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
}

void CWindow1::Init( const CVector2& size )
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

	auto pLayer1Drawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pLayer2Drawable = static_cast<CDrawableGroup*>( m_p1->GetResource() );
	auto pLayer1 = new CRenderObject2D;
	auto pLayer2 = new CRenderObject2D;
	SetRenderObject( pLayer1 );
	m_p1->SetRenderObject( pLayer2 );
	m_p1->SetRenderParent( pChunkObject );
	int32 n = CMyLevel::GetBlockSize();
	int32 nX = floor( size.x / n );
	int32 nY = floor( size.y / n );

	CImage2D* pImage;
	pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
	pImage->SetRect( CRectangle( -n, -n, n, n ) );
	pImage->SetTexRect( CRectangle( 0, 0.75f, 0.25f, 0.25f ) );
	pLayer1->AddChild( pImage );
	pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
	pImage->SetRect( CRectangle( size.x, -n, n, n ) );
	pImage->SetTexRect( CRectangle( 0.75f, 0.75f, 0.25f, 0.25f ) );
	pLayer1->AddChild( pImage );
	pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
	pImage->SetRect( CRectangle( -n, size.y, n, n ) );
	pImage->SetTexRect( CRectangle( 0, 0, 0.25f, 0.25f ) );
	pLayer1->AddChild( pImage );
	pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
	pImage->SetRect( CRectangle( size.x, size.y, n, n ) );
	pImage->SetTexRect( CRectangle( 0.75f, 0, 0.25f, 0.25f ) );
	pLayer1->AddChild( pImage );

	if( nX )
	{
		for( int i = 0; i < nX; i++ )
		{
			pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
			pImage->SetRect( CRectangle( i * n, -n, n, n ) );
			pImage->SetTexRect( CRectangle( 0.25f + SRand::Inst().Rand( 0, 2 ) * 0.25f, 0.75f, 0.25f, 0.25f ) );
			pLayer1->AddChild( pImage );
			pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
			pImage->SetRect( CRectangle( i * n, size.y, n, n ) );
			pImage->SetTexRect( CRectangle( 0.25f + SRand::Inst().Rand( 0, 2 ) * 0.25f, 0, 0.25f, 0.25f ) );
			pLayer1->AddChild( pImage );
		}
	}
	else
	{
		pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
		pImage->SetRect( CRectangle( 0, -n, size.x, n ) );
		pImage->SetTexRect( CRectangle( 0.25f + ( SRand::Inst().Rand( 0, 2 ) + ( n - size.x ) * 0.5f / n ) * 0.25f, 0.75f, 0.25f * size.x / n, 0.25f ) );
		pLayer1->AddChild( pImage );
		pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
		pImage->SetRect( CRectangle( 0, size.y, size.x, n ) );
		pImage->SetTexRect( CRectangle( 0.25f + ( SRand::Inst().Rand( 0, 2 ) + ( n - size.x ) * 0.5f / n ) * 0.25f, 0, 0.25f * size.x / n, 0.25f ) );
		pLayer1->AddChild( pImage );
	}
	if( nY )
	{
		for( int i = 0; i < nY; i++ )
		{
			pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
			pImage->SetRect( CRectangle( -n, i * n, n, n ) );
			pImage->SetTexRect( CRectangle( 0, 0.25f + SRand::Inst().Rand( 0, 2 ) * 0.25f, 0.25f, 0.25f ) );
			pLayer1->AddChild( pImage );
			pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
			pImage->SetRect( CRectangle( size.x, i * n, n, n ) );
			pImage->SetTexRect( CRectangle( 0.75f, 0.25f + SRand::Inst().Rand( 0, 2 ) * 0.25f, 0.25f, 0.25f ) );
			pLayer1->AddChild( pImage );
		}
	}
	else
	{
		pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
		pImage->SetRect( CRectangle( -n, 0, n, size.y ) );
		pImage->SetTexRect( CRectangle( 0, 0.25f + ( SRand::Inst().Rand( 0, 2 ) + ( n - size.y ) * 0.5f / n ) * 0.25f, 0.25f, 0.25f * size.y / n ) );
		pLayer1->AddChild( pImage );
		pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
		pImage->SetRect( CRectangle( size.x, 0, n, size.y ) );
		pImage->SetTexRect( CRectangle( 0.75f, 0.25f + ( SRand::Inst().Rand( 0, 2 ) + ( n - size.y ) * 0.5f / n ) * 0.25f, 0.25f, 0.25f * size.y / n ) );
		pLayer1->AddChild( pImage );
	}

	pImage = static_cast<CImage2D*>( pLayer1Drawable->CreateInstance() );
	pImage->SetRect( CRectangle( 0, 0, size.x, size.y ) );
	pImage->SetTexRect( CRectangle( 0.375f, 0.375f, 0.25f, 0.25f ) );
	pLayer2->AddChild( pImage );

	if( nX > 0 && nY > 0 )
	{
		if( m_nType == 0 )
		{
			for( int i = 0; i < nX; i++ )
			{
				for( int j = 0; j < nY; j++ )
				{
					pImage = static_cast<CImage2D*>( pLayer2Drawable->CreateInstance() );
					CRectangle rect( i * n, j * n, n, n );
					int32 nCol = SRand::Inst().Rand( 0u, m_nLayer2Cols );
					int32 nRow = SRand::Inst().Rand( 0u, m_nLayer2Rows );
					CRectangle texRect( nCol * 1.0f / m_nLayer2Cols, nRow * 1.0f / m_nLayer2Rows, 1.0f / m_nLayer2Cols, 1.0f / m_nLayer2Rows );
					if( j == 0 )
					{
						rect.SetTop( rect.y - 2 );
						if( nRow == 0 )
							texRect.height += 2 / ( n * m_nLayer2Rows );
						else
							texRect.SetTop( texRect.y - 2 / ( n * m_nLayer2Rows ) );
					}
					if( j == nY - 1 )
					{
						rect.height += 2;
						if( nRow == m_nLayer2Rows - 1 )
							texRect.SetTop( texRect.y - 2 / ( n * m_nLayer2Rows ) );
						else
							texRect.height += 2 / ( n * m_nLayer2Rows );
					}
					pImage->SetRect( rect );
					pImage->SetTexRect( texRect );
					pLayer2->AddChild( pImage );
				}
			}
		}
		else if( m_nType == 1 )
		{
			vector<CRenderObject2D*> vec;
			vec.resize( nX );
			CVector4 color( 1, 1, 1, 1 );
			color.x = SRand::Inst().Rand( 0.8f, 1.0f );
			color.y = SRand::Inst().Rand( 0.6f, 0.8f ) * color.x;
			color.z = SRand::Inst().Rand( 0.75f, 1.0f ) * color.y;
			SRand::Inst().Shuffle( &color.x, 3 );
			CVector4 color0 = color * 0.5f + CVector4( 0.5f, 0.5f, 0.5f, 0.5f );
			for( auto pChild = pLayer1->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
			{
				*static_cast<CImage2D*>( pChild )->GetParam() = color0;
			}
			for( int i = 0; i < nX; i++ )
			{
				vector<CRenderObject2D*> vec1;
				for( int j = 0; j < n * ( nY - 1 ); j++ )
				{
					CRectangle rt( 0, j, n, Min( nY * n, SRand::Inst().Rand( n, n * 2 ) ) );
					if( rt.GetBottom() > n * nY )
						rt.y = n * nY - rt.height;
					j = rt.GetBottom() - SRand::Inst().Rand( 2, 8 );
					CVector2 center = rt.GetCenter();
					float k = SRand::Inst().Rand( 0.0f, Min( rt.width / rt.height, rt.height / rt.width ) * 0.15f );
					float r = atan( k );
					float w = floor( ( rt.width - k * rt.height ) / sqrt( 1 - k * k ) / 2 ) * 2;
					float h = floor( ( rt.height - k * rt.width ) / sqrt( 1 - k * k ) / 2 ) * 2;
					if( SRand::Inst().Rand( 0, 2 ) )
					{
						swap( w, h );
						r += PI * 0.5f;
					}

					pImage = static_cast<CImage2D*>( m_pDrawable1->CreateInstance() );
					pImage->SetRect( CRectangle( -w * 0.5f, -h * 0.5f, w, h ) );
					pImage->SetTexRect( CRectangle( SRand::Inst().Rand( 0u, m_nDecoTexSize ), SRand::Inst().Rand( 0u, m_nDecoTexSize ),
						w, h ) * ( 1.0f / m_nDecoTexSize ) );
					pImage->SetPosition( center );
					pImage->SetRotation( SRand::Inst().Rand( 0, 2 ) ? -r : r );
					CVector4& param = *pImage->GetParam();
					param = CVector4( SRand::Inst().Rand( 0.85f, 1.15f ), SRand::Inst().Rand( 0.85f, 1.15f ), SRand::Inst().Rand( 0.85f, 1.15f ), 1 )
						* SRand::Inst().Rand( 0.35f, 0.7f );
					param.w = 1;
					vec1.push_back( pImage );
				}

				auto pRenderObject = new CRenderObject2D;
				pRenderObject->x = i * n;
				vec[i] = pRenderObject;
				SRand::Inst().Shuffle( vec1 );
				for( int i = 0; i < vec1.size(); i++ )
					pRenderObject->AddChild( vec1[i] );
				uint32 n1 = n / 2 + 2 + SRand::Inst().Rand( -4, 5 ) * 2;
				uint32 n2 = n + 4 - n1;
				CVector4 color1 = color;
				color1.x *= SRand::Inst().Rand( 0.8f, 1.0f );
				color1.y *= SRand::Inst().Rand( 0.8f, 1.0f );
				color1.z *= SRand::Inst().Rand( 0.8f, 1.0f );
				color1 = color1 + ( CVector4( 1, 1, 1, 1 ) - color1 ) * SRand::Inst().Rand( 0.0f, 0.5f );
				pImage = static_cast<CImage2D*>( pLayer2Drawable->CreateInstance() );
				pImage->SetRect( CRectangle( 0, -2, n, n1 ) );
				pImage->SetTexRect( CRectangle( SRand::Inst().Rand( 0u, m_nLayer2Cols ) * 1.0f / m_nLayer2Cols,
					1.0f - n1 * 1.0f / ( n * m_nLayer2Rows ), 1.0f / m_nLayer2Cols, n1 * 1.0f / ( n * m_nLayer2Rows ) ) );
				*pImage->GetParam() = color1;
				pRenderObject->AddChild( pImage );
				pImage = static_cast<CImage2D*>( pLayer2Drawable->CreateInstance() );
				pImage->SetRect( CRectangle( 0, size.y + 2 - n2, n, n2 ) );
				pImage->SetTexRect( CRectangle( SRand::Inst().Rand( 0u, m_nLayer2Cols ) * 1.0f / m_nLayer2Cols,
					0, 1.0f / m_nLayer2Cols, n2 * 1.0f / ( n * m_nLayer2Rows ) ) );
				*pImage->GetParam() = color1;
				pRenderObject->AddChild( pImage );
				for( int j = 0; j < nY - 1; j++ )
				{
					pImage = static_cast<CImage2D*>( pLayer2Drawable->CreateInstance() );
					pImage->SetRect( CRectangle( 0, j * n + n1 - 2, n, n ) );
					pImage->SetTexRect( CRectangle( SRand::Inst().Rand( 0u, m_nLayer2Cols ) * 1.0f / m_nLayer2Cols,
						SRand::Inst().Rand( 0u, m_nLayer2Rows ) * 1.0f / m_nLayer2Rows, 1.0f / m_nLayer2Cols, 1.0f / m_nLayer2Rows ) );
					*pImage->GetParam() = color1;
					pRenderObject->AddChild( pImage );
				}
			}

			SRand::Inst().Shuffle( vec );
			for( int i = 0; i < nX; i++ )
				pLayer2->AddChild( vec[i] );
		}
		else
		{
			int32 h1 = Max( 1, SRand::Inst().Rand( 0, nY / 2 + 1 ) );
			int32 h2 = SRand::Inst().Rand( 0, h1 + 1 );
			h1 -= h2;
			for( int i = 0; i < nX; i++ )
			{
				int32 texX = SRand::Inst().Rand( 0u, m_nLayer2Cols );
				for( int j = 0; j < nY; j++ )
				{
					int32 texY = j >= h1 && j < nY - h2 ? SRand::Inst().Rand( 1u, m_nLayer2Rows ) : 0;
					pImage = static_cast<CImage2D*>( pLayer2Drawable->CreateInstance() );
					pImage->SetRect( CRectangle( i * n, j * n, n, n ) );
					pImage->SetTexRect( CRectangle( texX * 1.0f / m_nLayer2Cols, texY * 1.0f / m_nLayer2Rows, 1.0f / m_nLayer2Cols, 1.0f / m_nLayer2Rows ) );
					pLayer2->AddChild( pImage );
				}
			}
		}
	}
}

void CWindow2::OnAddedToStage()
{
	for( auto pChild = m_pLinks[0]->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		m_nLinkCount++;
	m_pAI = new AI();
	m_pAI->SetParentEntity( this );
}

void CWindow2::AIFunc()
{
	static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 1, 0 );
	m_pMan->bVisible = false;
	for( int i = 0; i < 2; i++ )
	{
		m_pEye[i]->bVisible = true;
		m_pHead[i]->bVisible = false;
		m_pLinks[i]->bVisible = false;
	}

	if( !CMyLevel::GetInst() )
		return;

	while( 1 )
	{
		m_pAI->Yield( 0.5f, true );
		CRectangle rect;
		Get_HitProxy()->CalcBound( globalTransform, rect );
		if( CMyLevel::GetInst()->GetBound().GetBottom() > rect.GetBottom() )
			break;
	}

	m_pBullet = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet.c_str() );
	m_pBullet1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBullet1.c_str() );

	uint32 nAttackCount = 0;
	//alive
	while( 1 )
	{
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 1, 0 );
		m_pMan->bVisible = false;

		//closed
		while( 1 )
		{
			m_pAI->Yield( 0.5f, true );
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
				if( m_openRect.Contains( pos ) )
					break;
			}

			auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
			if( pChunkObject && pChunkObject->GetMaxHp() && pChunkObject->GetHp() / pChunkObject->GetMaxHp() < 0.5f )
				break;
		}

		//opening
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlaySpeed( 1, false );
		m_pAI->Yield( 0.25f, false );
		m_pMan->bVisible = true;
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetPlaySpeed( 1, false );
		for( int i = 0; i < 2; i++ )
		{
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetFrames( 0, 4, 8 );
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetPlaySpeed( 1, false );
		}
		m_pAI->Yield( 0.75f, false );
		m_pMan->MoveToTopmost( true );

		//open
		while( 1 )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				break;

			float fDeathChance = Max( 0.0f, nAttackCount - 10.0f ) / ( nAttackCount + 10.0f );
			auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
			if( pChunkObject && pChunkObject->GetMaxHp() )
				fDeathChance = 1 - ( 1 - fDeathChance ) * Max( pChunkObject->GetHp() / pChunkObject->GetMaxHp() * 2 - 1, 0.0f );
			if( SRand::Inst().Rand( 0.0f, 1.0f ) < fDeathChance )
			{
				goto dead;
			}

			CVector2 playerPos = pPlayer->GetPosition();
			CVector2 dPos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
			if( !m_closeRect.Contains( dPos ) )
				break;

			//attack
			nAttackCount++;

			{
				CVector2 dPos1 = playerPos - m_pEye[0]->globalTransform.GetPosition();
				CVector2 dPos2 = playerPos - m_pEye[1]->globalTransform.GetPosition();
				float fAngle1 = atan2( dPos1.y, dPos1.x );
				float fAngle2 = atan2( dPos2.y, dPos2.x );
				CMatrix2D mat1;
				mat1.Rotate( fAngle1 );
				CMatrix2D mat2;
				mat2.Rotate( fAngle2 );
				dPos1.Normalize();
				dPos2.Normalize();

				for( int i = 0; i < 8; i++ )
				{
					float fBaseAngle = i * PI / 4;
					CVector2 vel0 = CVector2( cos( fBaseAngle ) * 80, sin( fBaseAngle ) * 40 );
					CVector2 vel = mat1.MulVector2Dir( vel0 );
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( m_pEye[0]->globalTransform.GetPosition() );
					pBullet->SetVelocity( vel + dPos1 * 180 );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
					vel = mat2.MulVector2Dir( vel0 );
					pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( m_pEye[1]->globalTransform.GetPosition() );
					pBullet->SetVelocity( vel + dPos2 * 180 );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				CVector2 shakeDir[2] = { CVector2( cos( fAngle1 ), sin( fAngle1 ) ), CVector2( cos( fAngle2 ), sin( fAngle2 ) ) };
				CVector2 basePos[2] = { m_pEye[0]->GetPosition(), m_pEye[1]->GetPosition() };
				for( int i = 30; i >= 0; i-- )
				{
					for( int k = 0; k < 2; k++ )
					{
						CVector2 ofs = shakeDir[k] * cos( ( 30 - i ) * 1.1243232f ) * ( i / 30.0f ) * 8;
						ofs = CVector2( floor( ofs.x + 0.5f ), floor( ofs.y + 0.5f ) );
						m_pEye[k]->SetPosition( basePos[k] + ofs );
					}
					m_pAI->Yield( 0, false );
				}
			}

			m_pAI->Yield( 1.2f, false );
		}

		//closing
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pMan->GetRenderObject() )->SetPlaySpeed( -1, false );
		for( int i = 0; i < 2; i++ )
		{
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetFrames( 0, 4, 8 );
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetPlayPercent( 1 );
			static_cast<CMultiFrameImage2D*>( m_pEye[i].GetPtr() )->SetPlaySpeed( -1, false );
		}
		m_pAI->Yield( 0.25f, false );
		m_pWindow->MoveToTopmost( true );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetFrames( 0, 4, 8 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlayPercent( 1 );
		static_cast<CMultiFrameImage2D*>( m_pWindow.GetPtr() )->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.75f, false );
	}
	//dead
dead:
	for( int i = 0; i < 2; i++ )
	{
		AIEye* pEye = new AIEye( i );
		m_pAIEye[i] = pEye;
		pEye->SetParentEntity( this );
	}
}

void CWindow2::AIFuncEye( uint8 nEye )
{
	struct SEyeKilled
	{

	};
	try
	{
		CMessagePump pump( m_pAIEye[nEye] );
		m_pHead[nEye]->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pump.Register<SEyeKilled*>() );

		m_pEye[nEye]->bVisible = false;
		m_pHead[nEye]->bVisible = true;
		m_pLinks[nEye]->bVisible = true;
		m_pHead[nEye]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pLinks[nEye]->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );

		switch( SRand::Inst().Rand( 0, 3 ) )
		{
		case 0:
			AIFuncEye1( nEye );
			break;
		case 1:
			AIFuncEye2( nEye );
			break;
		case 2:
			AIFuncEye3( nEye );
			break;
		}
	}
	catch( SEyeKilled* e )
	{
		m_pLinks[nEye]->bVisible = false;
	}
}

void CWindow2::AIFuncEye1( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	for( int k = 0; k < 2; k++ )
	{
		m_pEye[k]->ForceUpdateTransform();
		float fBaseAngle = SRand::Inst().Rand( -PI, PI );
		for( int i = 0; i < 8; i++ )
		{
			float fAngle = fBaseAngle + i * PI / 4;
			auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
			pBullet->SetPosition( m_pEye[k]->globalTransform.GetPosition() );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 200 );
			pBullet->SetRotation( fAngle );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
	}

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	float fForce = 1.0f;
	int32 nTickCount = SRand::Inst().Rand( 45, 60 );
	int32 nLastHp = pHead->GetHp();
	while( 1 )
	{
		if( GetStage()->GetPlayer() )
			target = GetStage()->GetPlayer()->GetPosition();
		CVector2 dPos = target - pHead->globalTransform.GetPosition();

		CVector2 force = dPos;
		float l = force.Normalize();
		force = force * Min( l * 10, 450.0f ) * fForce;
		CVector2 force1 = m_pEye[nEye]->GetPosition() - pHead->GetPosition();
		float l1 = force1.Normalize();
		force1 = force1 * ( 200 + l1 * 0.25f );
		force = force + force1;
		UpdateLink( nEye );
		pAI->Yield( 0, false );

		int32 nDeltaHp = nLastHp - pHead->GetHp();
		nLastHp = pHead->GetHp();
		fForce -= nDeltaHp / 25.0f;

		pHead->SetPosition( pHead->GetPosition() + force * GetStage()->GetElapsedTimePerTick() );
		pHead->SetRotation( atan2( dPos.y, dPos.x ) );
		fForce = Min( 1.0f, fForce + GetStage()->GetElapsedTimePerTick() * 1.0f );
		nTickCount = Max( nTickCount - 1, 0 );

		if( !nTickCount )
		{
			float fDist = dPos.Length();
			float fMinDist = 130;
			float fMaxDist = 250;
			float r = ( fDist - fMinDist ) / ( fMaxDist - fMinDist );
			if( SRand::Inst().Rand( 0.0f, 1.0f ) > r )
			{
				for( int i = 0; i < 9; i++ )
				{
					float fAngle = ( i - 3 + SRand::Inst().Rand( 0.25f, 0.75f ) ) * 0.18f + atan2( dPos.y, dPos.x );
					float fSpeed = SRand::Inst().Rand( 150.0f, 200.0f );
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * fSpeed );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				fForce = 0;
			}
			else
			{
				float fBaseAngle = SRand::Inst().Rand( -PI, PI );
				for( int i = 0; i < 6; i++ )
				{
					float fAngle = fBaseAngle + i * PI * 2 / 6;
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 150.0f );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}

				fForce = SRand::Inst().Rand( 0.4f, 0.6f );
			}
			nTickCount = SRand::Inst().Rand( 45, 60 );
		}

		pAI->Yield( 0, true );
	}
}

void CWindow2::AIFuncEye2( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	int32 nTickCount = SRand::Inst().Rand( 240, 300 );
	while( 1 )
	{
		while( 1 )
		{
			if( GetStage()->GetPlayer() )
				target = GetStage()->GetPlayer()->GetPosition();
			CVector2 dPos = target - pHead->globalTransform.GetPosition();

			CVector2 force = dPos;
			float l = force.Normalize();
			force = force * Min( l * 10, 80.0f ) * Min( nTickCount / 120.0f, 1.0f );
			CVector2 force1 = m_pEye[nEye]->GetPosition() - pHead->GetPosition();
			force = force + force1;
			pAI->Yield( 0, false );

			pHead->SetPosition( pHead->GetPosition() + force * GetStage()->GetElapsedTimePerTick() );
			pHead->SetRotation( atan2( dPos.y, dPos.x ) );
			nTickCount = Max( nTickCount - 1, 0 );

			UpdateLink( nEye );

			if( !nTickCount )
			{
				nTickCount = SRand::Inst().Rand( 240, 300 );
				break;
			}
			pAI->Yield( 0, true );
		}

		pHead->SetDefence( -4.0f );
		CVector2 pos0 = pHead->GetPosition();
		CVector2 dir = CVector2( cos( pHead->r ), sin( pHead->r ) );
		int i;
		bool bHit = true;
		for( i = 1; i <= 60; i++ )
		{
			pAI->Yield( 0, false );

			float f = i / 60.0f;
			pHead->SetPosition( pos0 + dir * ( f * 600 + ( f * f ) * 200 ) );
			UpdateLink( nEye );
			pAI->Yield( 0, true );

			bool bHit1 = false;
			bool bForceBreak = false;
			for( auto pManifold = pHead->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				auto pEntity = static_cast<CEntity*>( pManifold->pOtherHitProxy );

				if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform )
				{
					auto pBlockObject = SafeCast<CBlockObject>( pEntity );
					if( pBlockObject && pBlockObject->GetBlock()->pOwner->nMoveType )
					{
						bForceBreak = true;
						break;
					}
					bHit1 = true;
					break;
				}
			}
			if( bForceBreak )
				break;
			if( bHit1 )
			{
				if( !bHit )
					break;
			}
			else
				bHit = false;

			if( GetStage()->GetPlayer() )
				target = GetStage()->GetPlayer()->GetPosition();
			if( ( target - pHead->globalTransform.GetPosition() ).Dot( dir ) < 0 )
				break;
		}

		SBarrageContext context;
		context.vecBulletTypes.push_back( m_pBullet );
		context.nBulletPageSize = 100;

		pHead->ForceUpdateTransform();
		CBarrage* pBarrage = new CBarrage( context );
		CVector2 pos = pHead->globalTransform.GetPosition();
		float fAngle0 = pHead->r;
		pBarrage->AddFunc( [fAngle0] ( CBarrage* pBarrage )
		{
			CMatrix2D mat;
			mat.Rotate( fAngle0 );
			int32 nBullet = 0;
			for( int i = 0; i < 5; i++ )
			{
				for( int j = 0; j < 20; j++ )
				{
					float r = 150 + i * 25;
					CVector2 center( 50 + i * 15, 0 );
					CVector2 vel = center + CVector2( cos( ( j + i * 0.5f ) * PI / 10 ), sin( ( j + i * 0.5f ) * PI / 10 ) ) * r;
					vel = mat.MulVector2Dir( vel );
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), vel, CVector2( 0, 0 ) );
				}
				pBarrage->Yield( 3 );
			}
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->SetPosition( pos );
		pBarrage->Start();

		pHead->SetDefence( 0.0f );
		for( i--; i >= 0; i-- )
		{
			float f = i / 60.0f;
			pHead->SetPosition( pos0 + dir * ( f * 600 + ( f * f ) * 200 ) );
			UpdateLink( nEye );
			pAI->Yield( 0, false );
		}

		pAI->Yield( 0, true );
	}
}

void CWindow2::AIFuncEye3( uint8 nEye )
{
	CVector2 target = globalTransform.GetPosition();

	auto pAI = m_pAIEye[nEye];
	auto pHead = m_pHead[nEye];
	int32 nTickCount = SRand::Inst().Rand( 400, 500 );
	uint8 nState = 0;
	float fLinear = 0;
	float fRadius = 0;
	float fFireCD = 200.0f;
	while( 1 )
	{
		pAI->Yield( 0, false );

		float fTargetLinear;
		float fTargetRadius;

		if( nTickCount )
			nTickCount--;
		switch( nState )
		{
		case 0:
			fTargetLinear = 200.0f;
			fTargetRadius = 100.0f;
			if( !nTickCount )
			{
				nTickCount = SRand::Inst().Rand( 300, 400 );
				pHead->SetDefence( -4.0f );
				nState = 1;
			}
			break;
		case 1:
			fTargetLinear = 600.0f;
			fTargetRadius = 250.0f;
			if( !nTickCount )
			{
				nTickCount = SRand::Inst().Rand( 600, 700 );
				pHead->SetDefence( 0.0f );
				nState = 0;
			}
			break;
		}

		if( fLinear < fTargetLinear )
			fLinear = Min( fLinear + GetStage()->GetElapsedTimePerTick() * ( 50.0f + fTargetLinear - fLinear ), fTargetLinear );
		else
			fLinear = Max( fLinear - GetStage()->GetElapsedTimePerTick() * 50.0f, fTargetLinear );
		if( fRadius < fTargetRadius )
			fRadius = Min( fRadius + GetStage()->GetElapsedTimePerTick() * 100.0f, fTargetRadius );
		else
			fRadius = Max( fRadius - GetStage()->GetElapsedTimePerTick() * 100.0f, fTargetRadius );

		pHead->SetRotation( pHead->r + ( nEye == 1 ? -fLinear / fRadius : fLinear / fRadius ) * GetStage()->GetElapsedTimePerTick() );
		pHead->SetPosition( CVector2( cos( pHead->r ), sin( pHead->r ) ) * fRadius );
		UpdateLink( nEye );

		fFireCD -= fLinear * GetStage()->GetElapsedTimePerTick();
		if( fFireCD <= 0 )
		{
			pAI->Yield( 0, true );
			if( nState == 0 )
			{
				auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
				pBullet->SetPosition( pHead->globalTransform.GetPosition() );
				pBullet->SetVelocity( CVector2( cos( pHead->r ), sin( pHead->r ) ) * 150.0f );
				pBullet->SetRotation( pHead->r );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				fFireCD = 30;
			}
			else
			{
				for( int i = 0; i < 5; i++ )
				{
					float fAngle = pHead->r + ( i - 2 ) * 0.45f;
					float fSpeed = 180 - abs( i - 2 ) * 15;
					auto pBullet = SafeCast<CBullet>( m_pBullet1->GetRoot()->CreateInstance() );
					pBullet->SetPosition( pHead->globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * fSpeed );
					pBullet->SetRotation( atan2( pBullet->GetVelocity().y, pBullet->GetVelocity().x ) );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}
				fFireCD = 120;
			}
		}
	}

}

void CWindow2::UpdateLink( uint8 nEye )
{
	CVector2 eyePos = m_pEye[nEye]->GetPosition();
	CVector2 headPos = m_pHead[nEye]->GetPosition();
	int i = 0;
	for( auto pChild = m_pLinks[nEye]->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild(), i++ )
	{
		float f = ( i + 0.5f ) / m_nLinkCount;
		pChild->SetPosition( eyePos * f + headPos * ( 1 - f ) );
	}
}

void CHouse0Deco::Init( const CVector2& size )
{
	const uint32 nTexSize = 256;
	const uint32 nTexScale = 2;
	uint32 nTileSize = CMyLevel::GetBlockSize();
	CChunkObject* pChunkObject = NULL;
	for( auto pParent = GetParentEntity(); pParent && !pChunkObject; pParent = pParent->GetParentEntity() )
	{
		pChunkObject = SafeCast<CChunkObject>( pParent );
		if( pChunkObject )
			break;
	}
	if( !pChunkObject )
		return;

	SetRenderParentBefore( pChunkObject->GetDamagedEffectsRoot() );
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pRenderObject = new CRenderObject2D;
	SetRenderObject( pRenderObject );
	CImage2D* pImage;

	int32 nWidth = pChunkObject->GetChunk()->nWidth;
	int32 nHeight = pChunkObject->GetChunk()->nHeight;
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
			vecTemp[i + j * nWidth] = pChunkObject->GetChunk()->GetBlock( i, j )->nTag;
	}
	int32 count1 = Max( SRand::Inst().Rand( -3, 3 ), -nHeight + 4 );
	for( int j = 1; j < nHeight; j++ )
	{
		float fMaxLen = 0;
		int32 nMax = 0;
		int32 nLen = 0;
		int32 nType1 = -1;
		TRectangle<int32> rect;
		for( int k = 0; k < 2 && nType1 < 0; k++ )
		{
			for( int i = 0; i < nWidth; i++ )
			{
				if( vecTemp[i + j * nWidth] == 0 && ( k == 1 || vecTemp[i + ( j - 1 ) * nWidth] == 1  ))
				{
					nLen++;
					float fLen = nLen + SRand::Inst().Rand( 0.0f, 1.0f );
					if( fLen > fMaxLen )
					{
						fMaxLen = fLen;
						nMax = i - nLen + 1;
					}
				}
				else
					nLen = 0;
			}
			nLen = floor( fMaxLen );
			if( nLen > 0 )
			{
				TRectangle<int32> initRect( nMax, j, nLen, 1 );
				if( initRect.x > 0 && vecTemp[initRect.x - 1 + initRect.y * nWidth] == 0 && vecTemp[initRect.x - 1 + ( initRect.y - 1 ) * nWidth] == 0 )
					initRect.SetLeft( initRect.x - 1 );
				if( initRect.GetRight() < nWidth && vecTemp[initRect.GetRight() + initRect.y * nWidth] == 0 && vecTemp[initRect.GetRight() + ( initRect.y - 1 ) * nWidth] == 0 )
					initRect.width++;
				int32 nMinWidth = Max( 4, initRect.width );
				int32 nMaxWidth = Max( nMinWidth, SRand::Inst().Rand( initRect.width, Max( nWidth - 2, initRect.width + 1 ) ) );
				int32 nMaxHeight = initRect.height + Max( 0, SRand::Inst().Rand( -1, 2 ) + SRand::Inst().Rand( 0, 2 ) );
				rect = PutRect( vecTemp, nWidth, nHeight, initRect, TVector2<int32>( nMinWidth, 1 ),
					TVector2<int32>( nMaxWidth, nMaxHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, -1, 0 );
				if( rect.width > 0 )
				{
					nType1 = SRand::Inst().Rand( 0, 3 ) * 2 + ( 1 - k );
					if( k == 1 )
						count1 = 0;
				}
			}
			if( nType1 < 0 && k == 0 )
			{
				if( count1 < 2 )
				{
					count1++;
					break;
				}
			}
		}

		if( nType1 < 0 )
			continue;
		int32 nType1a = SRand::Inst().Rand( 0, 4 );
		int32 hMins[] = { 14, 7, 8, 5, 8, 3 };
		int32 hMaxs[] = { 16, 11, 12, 9, 8, 6 };
		int32 hStrides[] = { 16, 16, 16, 16, 8, 8 };
		int32 texXBegins[] = { 0, 0, 128, 128, 0, 0 };
		int32 texYBegins[] = { 0, 64, 0, 64, 128, 160 };
		float fMaxK[] = { 2, 2, 0, 1, 2, 2, 2, 2,
							2, 2, 1, 1, 2, 2, 2, 2,
							1, 1, 1, 1, 2, 2, 2, 2 };
		int32 hMin = hMins[nType1];
		int32 hMax = hMaxs[nType1];
		TVector2<int32> texBegin( texXBegins[nType1], texYBegins[nType1] + nType1a * hStrides[nType1] );
		float maxK = fMaxK[nType1 * 4 + nType1a];
		float h1 = SRand::Inst().Rand( 0.0f, float( rect.height * 16 - hMax ) );
		float h2 = SRand::Inst().Rand( 0.0f, float( rect.height * 16 - hMax ) );
		if( h2 > h1 )
			h2 = Min( h2, h1 + ( rect.width - 1 ) * maxK * 0.5f );
		else
			h1 = Min( h1, h2 + ( rect.width - 1 ) * maxK * 0.5f );
		int8 a1 = SRand::Inst().Rand( 0, 2 );
		int8 b1 = SRand::Inst().Rand( 0, 2 );
		for( int i = 0; i < rect.width; i++ )
		{
			int32 x = a1 ? i : rect.width - 1 - i;
			float h = h1 + ( h2 - h1 ) * x / ( rect.width - 1 );
			int32 nTile;
			if( x == 0 )
				nTile = 0;
			else if( x == rect.width - 1 )
				nTile = 7;
			else
				nTile = floor( 1 + 6.0f * ( x + SRand::Inst().Rand( -0.7f, -0.3f ) ) / ( rect.width - 2 ));

			CRectangle r( ( rect.x + x ) * nTileSize, rect.y * nTileSize + nTexScale * floor( h + 0.5f ), nTileSize, hMax * nTexScale );
			if( !b1 )
			{
				int32 j1;
				for( j1 = j; j1 > 0; j1-- )
				{
					if( vecTemp[x + rect.x + ( j1 - 1 ) * nWidth] != 0 )
						break;
				}
				int32 nType2x = SRand::Inst().Rand( 0, 8 );
				int32 nType2y = SRand::Inst().Rand( 0, 2 );
				if( j - j1 >= 2 )
				{
					int32 y = r.y + ( hMax - hMin ) * nTexScale - nTileSize;
					if( nType2x >= 6 )
						y = Max<int32>( y - SRand::Inst().Rand( 4, 16 ) * 2, ( j1 + 1 ) * nTileSize );
					int32 n1 = ( y - j1 * nTileSize ) / nTileSize;
					n1 = Min( n1, SRand::Inst().Rand( 3, 6 ) );
					for( int k = 0; k <= n1; y -= nTileSize, k++ )
					{
						int32 texOfs = k == 0 ? 0 : ( k == n1 ? 16 : 8 );
						pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
						pImage->SetRect( CRectangle( r.x, y, nTileSize, nTileSize ) );
						pImage->SetTexRect( CRectangle( 16 * nType2x, 192 + 32 * nType2y + texOfs,
							16, 16 ) * ( 1.0f / nTexSize ) );
						pRenderObject->AddChild( pImage );
					}
					b1 = SRand::Inst().Rand( 2, 4 );
				}
			}
			else
				b1--;

			pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage->SetRect( r );
			pImage->SetTexRect( CRectangle( texBegin.x + 16 * nTile, texBegin.y, 16, hMax ) * ( 1.0f / nTexSize ) );
			pRenderObject->AddChild( pImage );
		}
	}

	vector<int32> vecHeight;
	vecHeight.resize( nWidth );
	for( int i = 0; i < nWidth; i++ )
	{
		int32 h1 = Min( nHeight, SRand::Inst().Rand( 3, 5 ) );
		vecHeight[i] = h1;
		for( int j = 0; j < h1; j++ )
		{
			if( vecTemp[i + j * nWidth] != 0 )
			{
				vecHeight[i] = j;
				break;
			}
		}
	}
	int32 i0 = 0;
	int32 nType3[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	SRand::Inst().Shuffle( nType3, 8 );
	for( int i = 0; i <= nWidth; i++ )
	{
		if( i == nWidth || vecHeight[i] < 2 )
		{
			if( i - i0 >= SRand::Inst().Rand( 2, 5 ) )
			{
				int8 a1 = SRand::Inst().Rand( 0, 2 );
				for( int32 x0 = SRand::Inst().Rand( 0, 5 ) * 2;; )
				{
					int32 x1 = x0 + SRand::Inst().Rand( 10, 14 ) * 2;
					if( x1 > ( i - i0 ) * nTileSize )
						break;
					int32 h = Min( vecHeight[a1 ? i0 + x0 / nTileSize : i - 1 - x0 / nTileSize],
						vecHeight[a1 ? i0 + ( x1 - 1 ) / nTileSize : i - 1 - ( x1 - 1 ) / nTileSize] );
					h = ( h - 1 ) * nTileSize + SRand::Inst().Rand( 0, 16 ) * 2;

					CRectangle rect( a1 ? i0 * nTileSize + x0 : i * nTileSize - x1, 0, x1 - x0, h );
					CRectangle texRect( 128 + nType3[SRand::Inst().Rand( 0, 2 )] * 16, 128, rect.width / nTexScale, rect.height / nTexScale );
					texRect.x += SRand::Inst().Rand<int32>( 0, 16 - texRect.width + 1 );
					texRect.y += SRand::Inst().Rand<int32>( 0, 128 - texRect.height + 1 );
					pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
					pImage->SetRect( rect );
					pImage->SetTexRect( texRect * ( 1.0f / nTexSize ) );
					pRenderObject->AddChild( pImage );
					x0 = x1;
				}
			}

			i0 = i + 1;
		}
	}
}