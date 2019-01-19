#include "stdafx.h"
#include "BlockItemsLv2.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Barrage.h"
#include "MyLevel.h"
#include "Common/Rand.h"
#include "Common/ResourceManager.h"
#include "Entities/Bullets.h"
#include "Entities/Enemies/Lv2Enemies.h"
#include "Entities/Blocks/Lv2/SpecialLv2.h"
#include "Common/MathUtil.h"
#include "Render/LightRendering.h"
#include "Entities/Bullets.h"
#include "Common/Algorithm.h"

void CLv2Wall1Deco::Init( const CVector2& size, SChunk* pPreParent )
{
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

	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	CRenderObject2D* pRenderObject[2] = { new CRenderObject2D, new CRenderObject2D };
	AddChild( pRenderObject[0] );
	AddChild( pRenderObject[1] );
	pRenderObject[0]->SetZOrder( 2 );
	pRenderObject[0]->SetRenderParent( pChunkObject );
	pRenderObject[1]->SetRenderParentAfter( pChunkObject->GetRenderObject() );
	SetRenderObject( NULL );
	CImage2D* pImage;

	int32 nWidth = pChunkObject->GetChunk()->nWidth;
	int32 nHeight = pChunkObject->GetChunk()->nHeight;
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	vector<TVector2<int32> > vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}

	for( int i = 0; i < 2; i++ )
	{
		SRand::Inst<eRand_Render>().Shuffle( vec );

		int32 s = SRand::Inst().Rand( nWidth * nHeight / 3, nWidth * nHeight / 2 );
		for( auto& p : vec )
		{
			if( vecTemp[p.x + p.y * nWidth] )
				continue;

			int32 nFrame = SRand::Inst<eRand_Render>().Rand( 0, 13 );
			TVector2<int32> size;
			CRectangle rect;
			CRectangle texRect;
			if( nFrame < 2 )
			{
				size = TVector2<int32>( 1, 2 );
				rect = CRectangle( nTileSize / 4, nTileSize / 2, nTileSize * 0.5f, nTileSize );
				texRect = CRectangle( 0.125f * nFrame, 0, 0.125f, 0.25f );
			}
			else if( nFrame < 4 )
			{
				size = TVector2<int32>( 2, 1 );
				rect = CRectangle( nTileSize / 2, nTileSize / 4, nTileSize, nTileSize * 0.5f );
				texRect = CRectangle( 0.25f, 0.125f * ( nFrame - 2 ), 0.25f, 0.125f );
			}
			else if( nFrame < 10 )
			{
				size = TVector2<int32>( 2, 2 );
				rect = CRectangle( nTileSize / 2, nTileSize / 2, nTileSize, nTileSize );
				texRect = CRectangle( ( nFrame - 2 ) % 4 * 0.25f, ( nFrame - 2 ) / 4 * 0.25f, 0.25f, 0.25f );
			}
			else if( nFrame < 12 )
			{
				size = TVector2<int32>( 4, 2 );
				rect = CRectangle( nTileSize, nTileSize / 2, nTileSize * 2, nTileSize );
				texRect = CRectangle( 0.5f * ( nFrame - 10 ), 0.5f, 0.5f, 0.25f );
			}
			else
			{
				size = TVector2<int32>( 6, 2 );
				rect = CRectangle( nTileSize, nTileSize / 2, nTileSize * 4, nTileSize );
				texRect = CRectangle( 0, 0.75f, 1, 0.25f );
			}
			auto r = PutRect( vecTemp, nWidth, nHeight, p, size, size, TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
			if( r.width <= 0 )
				continue;
			s -= size.x * size.y;
			if( s <= 0 )
				break;
			texRect.width *= 0.5f;
			texRect.x = ( texRect.x + i ) * 0.5f;
			rect = rect.Offset( CVector2( r.x * nTileSize + SRand::Inst().Rand( -2, 3 ) * 2, r.y * nTileSize + SRand::Inst().Rand( -2, 3 ) * 2 ) );
			pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			pRenderObject[i]->AddChild( pImage );
		}
		for( int i = 0; i < vecTemp.size(); i++ )
			vecTemp[i] = 0;
	}
}

void CCarSpawner::OnAddedToStage()
{
	auto pChunk = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunk )
	{
		CReference<CRenderObject2D> pRenderObject = GetRenderObject();
		pRenderObject->RemoveThis();
		pRenderObject->SetPosition( GetPosition() );
		pChunk->GetDecoratorRoot()->AddChild( pRenderObject );
		pRenderObject->SetRenderParent( this );
		auto pHouse = SafeCast<CHouse>( pChunk );
		if( pHouse )
		{
			auto pParam = static_cast<CImage2D*>( GetRenderObject() )->GetParam();
			auto pParam1 = pHouse->GetParam( 1 );
			pParam[0].w = pParam1[0].w;
			pParam[1] = pParam1[1];
			pParam[2] = pParam1[2];
			if( CMyLevel::GetInst() )
				SetRenderParent( CMyLevel::GetInst()->GetChunkRoot1() );
		}
	}
	CDetectTrigger::OnAddedToStage();
}

void CCarSpawner::Trigger()
{
	uint8 nCar;
	for( nCar = 0; nCar < 4; nCar++ )
	{
		if( m_nSpawnCounts[nCar] )
			break;
	}
	if( nCar >= 4 )
		return;

	auto rect1 = m_carRect1;
	SHitProxyPolygon polygon;
	polygon.nVertices = 4;
	polygon.vertices[0] = CVector2( rect1.x, rect1.y );
	polygon.vertices[1] = CVector2( rect1.x + rect1.width, rect1.y );
	polygon.vertices[2] = CVector2( rect1.x + rect1.width, rect1.y + rect1.height );
	polygon.vertices[3] = CVector2( rect1.x, rect1.y + rect1.height );
	polygon.CalcNormals();

	vector<CReference<CEntity> > hitEntities;
	GetStage()->MultiHitTest( &polygon, globalTransform, hitEntities );
	bool bHit = false;
	for( CEntity* pEntity : hitEntities )
	{
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform || pEntity->GetHitType() == eEntityHitType_System )
		{
			bHit = true;
			break;
		}
	}
	if( bHit )
		return;

	CRectangle rect = m_carRect.Offset( GetPosition() );
	TRectangle<int32> excludeRect;
	excludeRect.x = floor( rect.x / CMyLevel::GetBlockSize() );
	excludeRect.y = floor( rect.y / CMyLevel::GetBlockSize() );
	excludeRect.width = ceil( rect.GetRight() / CMyLevel::GetBlockSize() ) - excludeRect.x;
	excludeRect.height = ceil( rect.GetBottom() / CMyLevel::GetBlockSize() ) - excludeRect.y;
	auto pCar = SafeCast<CCar>( m_pCarPrefabs[nCar]->GetRoot()->CreateInstance() );
	pCar->SetExcludeChunk( SafeCast<CChunkObject>( GetParentEntity() ), excludeRect );
	pCar->SetPosition( globalTransform.GetPosition() );
	pCar->SetVelocity( m_spawnVel );
	pCar->SetRotation( atan2( m_spawnVel.y, m_spawnVel.x ) );
	pCar->SetParentAfterEntity( CMyLevel::GetInst()->GetChunkRoot1() );
	m_nSpawnCounts[nCar]--;
	m_detectRect = m_detectRect1 = CRectangle( -10000, -10000, 20000, 20000 );
}

bool CCarSpawner::CheckTrigger()
{
	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunkObject->GetHp() < pChunkObject->GetMaxHp() )
		m_detectRect = m_detectRect1 = CRectangle( -10000, -10000, 20000, 20000 );
	return pChunkObject->GetChunk()->nFallSpeed < 10;
}

void CHouseEntrance::OnAddedToStage()
{
	auto pChunk = SafeCast<CChunkObject>( GetParentEntity() );
	if( pChunk )
	{
		m_p1->RemoveThis();
		m_p1->SetPosition( m_p1->GetPosition() + GetPosition() );
		pChunk->GetDecoratorRoot()->AddChild( m_p1 );
		m_p2->RemoveThis();
		m_p2->SetPosition( m_p2->GetPosition() + GetPosition() );
		pChunk->GetDecoratorRoot()->AddChild( m_p2 );
		m_p2->SetRenderParent( this );
	}
	if( m_pSign )
		m_nFrameBegin = static_cast<CMultiFrameImage2D*>( m_pSign.GetPtr() )->GetFrameBegin();
	if( !SafeCast<CHouse>( GetParentEntity() ) )
		bVisible = false;
}

void CHouseEntrance::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CHouseEntrance::SetState( uint8 nState )
{
	auto pImg = m_pSign ? static_cast<CMultiFrameImage2D*>( m_pSign.GetPtr() ) : NULL;
	auto pLight = m_pLight ? static_cast<CPointLightObject*>( m_pLight.GetPtr() ) : NULL;
	switch( nState )
	{
	case 0:
		if( m_pSign )
			pImg->SetFrames( m_nFrameBegin, m_nFrameBegin + 1, 0 );
		pLight->baseColor = CVector3( 0.7, 0.7, 0 );
		break;
	case 1:
		if( m_pSign )
			pImg->SetFrames( m_nFrameBegin, m_nFrameBegin + 2, 12 );
		pLight->baseColor = CVector3( 0.9, 0.4, 0 );
		break;
	case 2:
		if( m_pSign )
			pImg->SetFrames( m_nFrameBegin + 2, m_nFrameBegin + 4, 12 );
		pLight->baseColor = CVector3( 1, 0, 0 );
		break;
	case 3:
		if( m_pSign )
			pImg->bVisible = false;
		if( pLight )
			pLight->bVisible = false;
		break;
	}
}

bool CHouseEntrance::CanEnter( CCharacter * pCharacter )
{
	auto pHouse = SafeCast<CHouse>( GetParentEntity() );
	if( !pHouse )
		pHouse = SafeCast<CHouse>( GetParentEntity()->GetParentEntity() );
	if( !pHouse )
		return false;
	return pHouse->CanEnter( pCharacter );
}

bool CHouseEntrance::Enter( CCharacter * pCharacter )
{
	auto pHouse = SafeCast<CHouse>( GetParentEntity() );
	if( !pHouse )
		pHouse = SafeCast<CHouse>( GetParentEntity()->GetParentEntity() );
	if( !pHouse )
		return false;
	if( !pHouse->Enter( pCharacter ) )
		return false;
	OpenDoor();
	return true;
}

bool CHouseEntrance::Exit( CCharacter * pCharacter )
{
	if( globalTransform.GetPosition().y >= CMyLevel::GetInst()->GetBoundWithLvBarrier().GetBottom() - 32 )
		return false;

	auto rect1 = m_spawnRect1;
	SHitProxyPolygon polygon;
	polygon.nVertices = 4;
	polygon.vertices[0] = CVector2( rect1.x, rect1.y );
	polygon.vertices[1] = CVector2( rect1.x + rect1.width, rect1.y );
	polygon.vertices[2] = CVector2( rect1.x + rect1.width, rect1.y + rect1.height );
	polygon.vertices[3] = CVector2( rect1.x, rect1.y + rect1.height );
	polygon.CalcNormals();
	vector<CReference<CEntity> > hitEntities;
	GetStage()->MultiHitTest( &polygon, globalTransform, hitEntities );
	bool bHit = false;
	for( CEntity* pEntity : hitEntities )
	{
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform || pEntity->GetHitType() == eEntityHitType_System )
		{
			bHit = true;
			break;
		}
	}
	if( bHit )
		return false;

	pCharacter->SetPosition( globalTransform.GetPosition() + CVector2( m_spawnRect.x + SRand::Inst().Rand( 0.0f, m_spawnRect.width ), m_spawnRect.y + SRand::Inst().Rand( 0.0f, m_spawnRect.height ) ) );
	pCharacter->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
	OpenDoor();
	return true;
}

void CHouseEntrance::OpenDoor()
{
	if( !m_onTick.IsRegistered() )
	{
		auto p = static_cast<CImage2D*>( m_p2.GetPtr() );
		auto texRect = p->GetElem().texRect;
		if( m_nDir == 1 )
			texRect.y += texRect.height;
		else
			texRect.x += texRect.width;
		p->SetTexRect( texRect );
	}
	GetStage()->RegisterAfterHitTest( 30, &m_onTick );
}

void CHouseEntrance::OnTick()
{
	auto p = static_cast<CImage2D*>( m_p2.GetPtr() );
	auto texRect = p->GetElem().texRect;
	if( m_nDir == 1 )
		texRect.y -= texRect.height;
	else
		texRect.x -= texRect.width;
	p->SetTexRect( texRect );
}

void CHouseWindow::Init( const CVector2 & size, SChunk* pPreParent )
{
	m_size = size;
	CChunkObject* pChunkObject = NULL;
	for( auto pParent = GetParentEntity(); pParent && !pChunkObject; pParent = pParent->GetParentEntity() )
	{
		pChunkObject = SafeCast<CChunkObject>( pParent );
		if( pChunkObject )
			break;
	}
	if( !pChunkObject )
		return;
	m_pChunkObject = pChunkObject;

	CVector4 params[3];
	auto pParam = static_cast<CImage2D*>( m_p2->GetRenderObject() )->GetParam();
	for( int i = 0; i < 3; i++ )
		params[i] = pParam[i];
	auto pHouse = SafeCast<CHouse>( pChunkObject );
	if( pHouse )
	{
		pParam = pHouse->GetParam( 1 );
		params[0].w = pParam[0].w;
		params[1] = pParam[1];
		params[2] = pParam[2];
	}

	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pDrawable1 = static_cast<CDrawableGroup*>( m_p1->GetResource() );
	auto pDrawable2 = static_cast<CDrawableGroup*>( m_p2->GetResource() );
	SetRenderObject( NULL );
	m_p1->SetRenderObject( NULL );
	m_p2->SetRenderObject( NULL );
	m_p1->SetRenderParent( pChunkObject );
	m_p2->SetRenderParent( pChunkObject );

	int32 n = CMyLevel::GetBlockSize();
	int32 nX = floor( size.x / n );
	int32 nY = floor( size.y / n );
	if( nX == 2 && nY == 2 )
		m_nType = 2;
	else if( nX > 1 )
		m_nType = 0;
	else
		m_nType = 1;

	if( m_nType == 0 )
	{
		for( int i = 0; i < nX; i++ )
		{
			CRectangle rect( i * n, 0, n, n );
			CRectangle texRect( 0, 0, 0.25f, 0.25f );
			int32 nFrame = 0;
			if( i == nX - 1 )
			{
				texRect.x += 0.5f;
				nFrame += 8;
			}
			else if( i > 0 )
			{
				texRect.x += 0.25f;
				nFrame += 4;
			}
			auto pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			AddChild( pImage );
			pImage = static_cast<CImage2D*>( pDrawable2->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			pParam = pImage->GetParam();
			for( int i = 0; i < 3; i++ )
				pParam[i] = params[i];
			m_p2->AddChild( pImage );

			auto pImage1 = static_cast<CMultiFrameImage2D*>( pDrawable1->CreateInstance() );
			pImage1->SetFrames( nFrame, nFrame + 1, 0 );
			pImage1->SetPosition( CVector2( i * n, 0 ) );
			pImage1->SetAutoUpdateAnim( true );
			m_p1->AddChild( pImage1 );
		}
	}
	else if( m_nType == 1 )
	{
		for( int i = 0; i < nY; i++ )
		{
			CRectangle rect( 0, i * n, n, n );
			CRectangle texRect( 0.75f, 0.5f, 0.25f, 0.25f );
			int32 nFrame = 12;
			if( i == nY - 1 )
			{
				texRect.y -= 0.5f;
				nFrame += 8;
			}
			else if( i > 0 )
			{
				texRect.y -= 0.25f;
				nFrame += 4;
			}
			auto pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			AddChild( pImage );
			pImage = static_cast<CImage2D*>( pDrawable2->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			pParam = pImage->GetParam();
			for( int i = 0; i < 3; i++ )
				pParam[i] = params[i];
			m_p2->AddChild( pImage );

			auto pImage1 = static_cast<CMultiFrameImage2D*>( pDrawable1->CreateInstance() );
			pImage1->SetFrames( nFrame, nFrame + 1, 0 );
			pImage1->SetPosition( CVector2( 0, i * n ) );
			pImage1->SetAutoUpdateAnim( true );
			m_p1->AddChild( pImage1 );
		}
	}
	else
	{
		CRectangle rect( 0, 0, n * 2, n * 2 );
		CRectangle texRect( 0, 0.25f, 0.5f, 0.5f );
		auto pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
		pImage->SetRect( rect );
		pImage->SetTexRect( texRect );
		AddChild( pImage );
		pImage = static_cast<CImage2D*>( pDrawable2->CreateInstance() );
		pImage->SetRect( rect );
		pImage->SetTexRect( texRect );
		pParam = pImage->GetParam();
		for( int i = 0; i < 3; i++ )
			pParam[i] = params[i];
		m_p2->AddChild( pImage );

		auto pImage1 = static_cast<CMultiFrameImage2D*>( pDrawable1->CreateInstance() );
		pImage1->SetFrames( 24, 25, 0 );
		pImage1->SetAutoUpdateAnim( true );
		m_p1->AddChild( pImage1 );
	}
}

void CHouseWindow::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CHouseWindow::SetState( uint8 nState )
{
	for( auto pChild = m_p1->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		auto pImg = static_cast<CMultiFrameImage2D*>( pChild );
		uint32 nFrameBegin = pImg->GetFrameBegin() / 4 * 4;
		switch( nState )
		{
		case 0:
			pImg->SetFrames( nFrameBegin, nFrameBegin + 1, 0 );
			break;
		case 1:
			GetStage()->RegisterAfterHitTest( SRand::Inst().Rand( 120, 240 ), &m_onTick );
			pImg->SetFrames( nFrameBegin, nFrameBegin + 2, 12 );
			break;
		case 2:
			pImg->SetFrames( nFrameBegin + 2, nFrameBegin + 4, 12 );
			break;
		case 3:
			pImg->bVisible = false;
			Break();
			break;
		}
	}
}

void CHouseWindow::Break()
{
	if( m_bBroken )
		return;
	m_bBroken = true;
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();

	CVector2 target;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
		target = pPlayer->GetPosition();
	else
		target = CMyLevel::GetInst()->GetBound().GetCenter();

	int32 n = CMyLevel::GetBlockSize();
	int32 nX = floor( m_size.x / n );
	int32 nY = floor( m_size.y / n );
	if( m_nType == 0 )
	{
		for( int i = 0; i < nX - 1; i++ )
		{
			CRectangle rect( ( i + 0.5f ) * n, 0, n, n );
			CRectangle texRect( SRand::Inst().Rand( 0, 2 ) * 0.25f, SRand::Inst().Rand( 0, 2 ) * 0.25f, 0.25f, 0.25f );

			auto pImage = static_cast<CImage2D*>( m_pCrack->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			m_p2->AddChild( pImage );
		}
	}
	else if( m_nType == 1 )
	{
		for( int i = 0; i < nY - 1; i++ )
		{
			CRectangle rect( 0, ( i + 0.5f ) * n, n, n );
			CRectangle texRect( SRand::Inst().Rand( 2, 4 ) * 0.25f, SRand::Inst().Rand( 0, 2 ) * 0.25f, 0.25f, 0.25f );

			auto pImage = static_cast<CImage2D*>( m_pCrack->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			m_p2->AddChild( pImage );
		}
	}
	else
	{
		CRectangle rect( 0, 0, n * 2, n * 2 );
		CRectangle texRect( SRand::Inst().Rand( 0, 2 ) * 0.5f, 0.5f, 0.5f, 0.5f );

		auto pImage = static_cast<CImage2D*>( m_pCrack->CreateInstance() );
		pImage->SetRect( rect );
		pImage->SetTexRect( texRect );
		m_p2->AddChild( pImage );
	}

	CVector2 p = GetPosition() + m_pChunkObject->globalTransform.GetPosition();
	if( m_nType == 0 || m_nType == 1 )
	{
		p.x += 0.5f * n;
		p.y += 0.5f * n;
		CVector2 dir;
		int8 nDir = ( m_nType == 0 ? target.y > p.y : target.x > p.x ) ? 1 : -1;
		int32 w = m_nType == 0 ? nX - 1 : nY - 1;
		for( int i = 0; i < w; i++ )
		{
			auto pEft = SafeCast<CEffectObject>( m_pEft->GetRoot()->CreateInstance() );
			pEft->SetPosition( m_nType == 0 ? CVector2( p.x + i * n, p.y ) : CVector2( p.x, p.y + i * n ) );
			pEft->SetRotation( m_nType == 0 ? ( nDir == 1 ? PI * 0.5f : PI * 1.5f ) : ( nDir == 1 ? 0: PI ) );
			pEft->SetState( 2 );
			pEft->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		}

		int32 nCount = ( m_nType == 0 ? m_size.x - 1 : m_size.y - 1 ) / 10;
		SBarrageContext context;
		context.pCreator = m_pChunkObject;
		context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
		context.nBulletPageSize = nCount + 1;

		auto pBarrage = new CBarrage( context );
		CVector2 p0 = p;
		CVector2 p1 = m_nType == 0 ? CVector2( p.x + m_size.x - n, p.y ) : CVector2( p.x, p.y + m_size.y - n );
		pBarrage->AddFunc( [p0, p1, nCount, target] ( CBarrage* pBarrage )
		{
			uint32 nBullet = 0;
			uint8 b = SRand::Inst().Rand( 0, 2 );
			CVector2 d[3] = { { b ? 1.0f : 0, 0.2f }, { SRand::Inst().Rand( 0.4f, 0.6f ), 0.6f }, { b ? 0 : 1.0f, 0.2f } };
			for( int i = 0; i < 3; i++ )
			{
				d[i] = ( target - ( p0 + ( p1 - p0 ) * d[i].x ) ) * d[i].y;
				if( d[i].Length2() < 1 )
					d[i] = CVector2( 0, -200 );
			}

			for( int i = 0; i <= nCount; i++ )
			{
				float t = i * 1.0f / nCount;
				if( b )
					t = 1.0f - t;
				CVector2 p = p0 + ( p1 - p0 ) * t;
				CVector2 target = p0 + d[0] + d[1] * ( 2 * t * ( 1 - t ) ) + d[2] * t * t;
				CVector2 v = target - p;
				if( v.Normalize() < 0 )
					v = CVector2( 0, -1 );
				v = v * ( 225.0f - ( i - 5 ) * ( i - 5 ) * 2 );
				pBarrage->InitBullet( nBullet++, 0, -1, p, v, CVector2( 0, 0 ) );
				pBarrage->Yield( 2 );
				if( !pBarrage->GetCreator()->GetStage() )
					break;
			}

			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->Start();
	}
	else
	{
		p = p + CVector2( n, n );
		CVector2 dPos = target - p;
		if( dPos.Normalize() < 0.0001f )
			dPos = CVector2( 0, -1 );
		for( int i = 0; i < 3; i++ )
		{
			auto pEft = SafeCast<CEffectObject>( m_pEft->GetRoot()->CreateInstance() );
			pEft->SetPosition( CVector2( p.x, p.y ) + CVector2( SRand::Inst().Rand( -8.0f, 8.0f ), SRand::Inst().Rand( -8.0f, 8.0f ) ) * i );
			pEft->SetRotation( atan2( dPos.y, dPos.x ) + SRand::Inst().Rand( -0.05f, 0.05f ) * i );
			pEft->SetState( 2 );
			pEft->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		}

		auto pHouse = SafeCast<CHouse>( m_pChunkObject );
		if( !pHouse )
			return;
		auto pObj = pHouse->GetOneThrowObj();
		if( !pObj )
			return;
		pObj->SetPosition( p );
		pObj->SetVelocity( dPos * 250.0f );
		auto pThrowObj = SafeCast<CThrowObj>( pObj );
		if( pThrowObj )
		{
			pThrowObj->SetLife( pThrowObj->GetLife() * 1.5f );
			pThrowObj->SetLife1( pThrowObj->GetLife1() * 1.5f );
		}
		pThrowObj->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
}

void CHouseWindow::OnTick()
{
	GetStage()->RegisterAfterHitTest( SRand::Inst().Rand( 60, 90 ), &m_onTick );

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	CRectangle rect( 0, 0, m_size.x, m_size.y );
	rect = rect.Offset( GetPosition() + m_pChunkObject->globalTransform.GetPosition() );
	if( m_nType == 0 )
	{
		rect.x -= 80;
		rect.y -= 500;
		rect.width += 160;
		rect.height += 1000;
	}
	else if( m_nType == 1 )
	{
		rect.x -= 500;
		rect.y -= 80;
		rect.width += 1000;
		rect.height += 160;
	}
	else
	{
		rect.x -= 300;
		rect.y -= 300;
		rect.width += 600;
		rect.height += 600;
	}
	if( rect.Contains( pPlayer->GetPosition() ) )
		Break();
}

void CThruster::OnAddedToStage()
{
	if( !CMyLevel::GetInst() )
		return;
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CThruster::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
}

void CThruster::OnTick()
{
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );

	m_bEnabled = CheckEnabled();
	if( m_bEnabled )
	{
		if( !m_pLightning )
		{
			m_pLightning = SafeCast<CLightning>( m_pEftPrefab->GetRoot()->CreateInstance() );
			m_pLightning->Set( NULL, NULL, m_ofs, m_ofs + CVector2( 0, -m_fMaxEftHeight ), -1, -1 );
			m_pLightning->SetParentEntity( this );
		}
	}
	else
	{
		if( m_pLightning )
		{
			m_pLightning->SetParentEntity( NULL );
			m_pLightning = NULL;
		}
	}
}

bool CThruster::CheckEnabled()
{
	if( !m_nDuration )
		return false;
	auto pParent = SafeCast<CChunkObject>( GetParentEntity() );
	if( !pParent )
		return false;
	if( pParent->y > m_fEnableHeight )
		return false;
	auto pChunk = pParent->GetChunk();
	if( !m_bEnabled && pChunk->nFallSpeed < m_nEnableSpeed )
		return false;
	int32 nDecelerate = m_nDecelerate / pChunk->nWidth;
	pChunk->nFallSpeed = Max<int32>( pChunk->nFallSpeed - nDecelerate, 0 );
	if( !pChunk->nFallSpeed )
		pParent->GetChunk()->bForceStop = true;
	m_nDuration--;
	CMyLevel::GetInst()->AddShakeStrength( m_fShake );
	return true;
}

void COperateableTurret1::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, 1, 24 );
	auto pLevel = CMyLevel::GetInst();
	if( pLevel )
		SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
}

void COperateableTurret1::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CEnemy::OnRemovedFromStage();
}

int8 COperateableTurret1::IsOperateable( const CVector2& pos )
{
	if( m_onTick.IsRegistered() )
		return 2;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer || !m_pDetectArea->HitTest( pPlayer->GetPosition() ) )
		return 1;
	return 0;
}

void COperateableTurret1::Operate( const CVector2& pos )
{
	m_nAmmoLeft = m_nAmmoCount;
	static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 1, 7, 24 );
	OnTick();
}

void COperateableTurret1::Kill()
{
	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	for( int i = 0; i < m_nAmmoCount; i++ )
	{
		for( int j = 0; j < m_nBulletCount; j++ )
		{
			auto pBullet = SafeCast<CBullet>( m_pBulletPrefab1->GetRoot()->CreateInstance() );
			pBullet->SetPosition( globalTransform.GetPosition() );
			float fAngle = r + ( j - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
			pBullet->SetRotation( fAngle );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * m_fBulletSpeed * ( 1.5 + i * 0.5f ) );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			if( pChunkObject )
				pBullet->SetCreator( pChunkObject );
		}
	}
	CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire * 3 );
	CEnemy::Kill();
}

void COperateableTurret1::OnTick()
{
	if( !m_nAmmoLeft )
	{
		static_cast<CMultiFrameImage2D*>( GetRenderObject() )->SetFrames( 0, 1, 24 );
		return;
	}

	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	for( int i = 0; i < m_nBulletCount; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pBulletPrefab->GetRoot()->CreateInstance() );
		pBullet->SetPosition( globalTransform.GetPosition() );
		float fAngle = r + ( i - ( m_nBulletCount - 1 ) * 0.5f ) * m_fBulletAngle;
		pBullet->SetRotation( fAngle );
		pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * m_fBulletSpeed );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		if( pChunkObject )
			pBullet->SetCreator( pChunkObject );
	}

	CMyLevel::GetInst()->AddShakeStrength( m_fShakePerFire );
	m_nAmmoLeft--;
	GetStage()->RegisterAfterHitTest( m_nAmmoLeft ? m_nFireInterval : m_nFireCD, &m_onTick );
}

void COperateableButton::OnAddedToStage()
{
	auto pChunkObject = SafeCast<CChunkObject>( GetParentEntity() );
	auto p = GetRenderObject();
	if( p && pChunkObject )
	{
		p->RemoveThis();
		pChunkObject->GetRenderObject()->AddChild( p );
		p->SetPosition( GetPosition() );
		p->SetRenderParent( this );
	}
}

int8 COperateableButton::IsOperateable( const CVector2& pos )
{
	int8 n = -1;
	for( auto& p : m_vec )
	{
		auto pOperateable = SafeCastToInterface<IOperateable>( p.GetPtr() );
		if( !pOperateable )
			continue;
		if( n == -1 )
			n = 0;
		n |= pOperateable->IsOperateable( pos );
	}
	return n;
}

void COperateableButton::Operate( const CVector2 & pos )
{
	for( auto& p : m_vec )
	{
		auto pOperateable = SafeCastToInterface<IOperateable>( p.GetPtr() );
		if( !pOperateable )
			continue;
		pOperateable->Operate( pos );
	}
}

void CWindow3::OnAddedToStage()
{
	for( int i = 0; i < ELEM_COUNT( m_pParts ); i++ )
	{
		if( m_pParts[i] )
		{
			m_pParts[i]->bVisible = false;
			m_pParts[i]->SetTransparentRec( true );
		}
	}
}

void CWindow3::Kill()
{
	if( m_bKilled || !m_pAI || !m_pAI->IsRunning() )
		return;
	m_pAI->Throw( (uint32)0 );
}

CAIObject* CWindow3::TryPlay( uint8 nType )
{
	if( m_pAI && !m_pAI->IsRunning() )
	{
		m_pAI->SetParentEntity( NULL );
		m_pAI = NULL;
	}
	if( m_pAI )
		return NULL;
	auto pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return NULL;

	int32 nIndex[4] = { 0, 1, 2, 3 };
	SRand::Inst().Shuffle( nIndex, 4 );
	for( int i = 0; i < 4; i++ )
	{
		if( !m_pDetectArea[nIndex[i]] )
			continue;
		if( !m_pDetectArea[nIndex[i]]->HitTest( pPlayer->GetPosition() ) )
			continue;

		m_nDir = nIndex[i];
		m_pAI = new AI( nType );
		m_pAI->SetParentEntity( this );
		return m_pAI;
	}
	return NULL;
}

void CWindow3::AIFunc( uint8 nType )
{
	auto pEnemyPart = m_pParts[m_nDir];
	pEnemyPart->bVisible = true;
	auto pImg = static_cast<CMultiFrameImage2D*>( pEnemyPart->GetRenderObject() );
	try
	{
		pImg->SetFrames( 0, 4, 8 );
		pImg->SetPlaySpeed( 1, false );
		m_pAI->Yield( 0.5f, false );
		pEnemyPart->SetTransparentRec( false );
		m_pAI->Yield( 1.0f, false );

		CVector2 p = globalTransform.MulVector2Pos( m_fireOfs[m_nDir] );
		CVector2 dirs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
		CVector2 dir = globalTransform.MulVector2Dir( dirs[m_nDir] );
		SBarrageContext context;
		CBarrage* pBarrage;
		if( nType == 0 )
		{
			CMyLevel::GetInst()->AddShakeStrength( 10.0f );
			uint8 r = SRand::Inst().Rand( 0, 4 );
			switch( r )
			{
			case 0:
				context.vecBulletTypes.push_back( m_pBullet4.GetPtr() );
				context.nBulletPageSize = 9;
				pBarrage = new CBarrage( context );
				pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
				{
					CVector2 d = dir;
					CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
					if( pPlayer )
						d = pPlayer->GetPosition() - p;
					float fAngle = atan2( dir.y, dir.x );
					float fAngle1 = atan2( d.y, d.x );
					float dAngle = fAngle1 - fAngle;
					dAngle = NormalizeAngle( dAngle );
					dAngle = Min( Max( dAngle, -0.3f ), 0.3f );

					uint32 nBullet = 0;
					for( int i = 0; i < 5; i++ )
					{
						float fAngle1 = fAngle + dAngle + ( i - 2 ) * 0.25f;
						pBarrage->InitBullet( nBullet++, 0, -1, p, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 180.0f, CVector2( 0, 0 ) );
					}
					pBarrage->Yield( 15 );
					for( int i = 0; i < 4; i++ )
					{
						float fAngle1 = fAngle + dAngle + ( i - 1.5f ) * 0.25f;
						pBarrage->InitBullet( nBullet++, 0, -1, p, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 150.0f, CVector2( 0, 0 ) );
					}
					pBarrage->Yield( 2 );
					pBarrage->StopNewBullet();
				} );
				break;
			case 1:
				context.vecBulletTypes.push_back( m_pBullet4.GetPtr() );
				context.nBulletPageSize = 6;
				pBarrage = new CBarrage( context );
				pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
				{
					float fAngle = atan2( dir.y, dir.x );
					bool b[2] = { true, true };
					float a[2];
					for( int i = 0; i < 2; i++ )
						a[i] = fAngle + ( i - 0.5f + SRand::Inst().Rand( -0.1f, 0.1f ) ) * 1.0f;
					CVector2 p0[2] = { p, p };
					CVector2 v[2];
					for( int i = 0; ; i++ )
					{
						int32 n = 0;
						for( int i1 = 0; i1 < 2; i1++ )
						{
							if( !b[i1] )
								continue;
							v[i1] = CVector2( cos( a[i1] ), sin( a[i1] ) ) * 250;
							pBarrage->InitBullet( i1 + ( i & 1 ) * 2, 0, -1, p0[i1], v[i1], CVector2( 0, 0 ) );
						}
						pBarrage->Yield( 6 );

						for( int i1 = 0; i1 < 2; i1++ )
						{
							auto pContext = pBarrage->GetBulletContext( i1 + ( i & 1 ) * 2 );
							if( !pContext->IsValid() || !pContext->pEntity || !pContext->pEntity->GetStage() )
							{
								b[i1] = false;
								continue;
							}
							p0[i1] = p0[i1] + v[i1] * 0.1f;
							a[i1] += SRand::Inst().Rand( -0.3f, 0.3f );
							SafeCast<CBullet>( pContext->pEntity.GetPtr() )->Kill();
							n++;
						}
						if( !n )
							break;
					}
					pBarrage->Yield( 2 );
					pBarrage->StopNewBullet();
				} );
				break;
			case 2:
				context.vecBulletTypes.push_back( m_pBullet4.GetPtr() );
				context.vecBulletTypes.push_back( m_pBullet2.GetPtr() );
				context.vecBulletTypes.push_back( m_pBullet3.GetPtr() );
				context.nBulletPageSize = 50;
				pBarrage = new CBarrage( context );
				pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
				{
					float fAngle = atan2( dir.y, dir.x );
					for( int i = 0; i < 3; i++ )
					{
						float fAngle1 = fAngle + ( i - 1 + SRand::Inst().Rand( -0.5f, 0.5f ) ) * 0.6f;
						pBarrage->InitBullet( i, 0, -1, p, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * SRand::Inst().Rand( 200.0f, 320.0f ), CVector2( 0, 0 ) );
					}
					pBarrage->Yield( 30 );

					uint32 nBullet = 3;
					for( int i = 0; i < 3; i++ )
					{
						auto pContext = pBarrage->GetBulletContext( i );
						if( !pContext->IsValid() || !pContext->pEntity || !pContext->pEntity->GetStage() )
							continue;
						pContext->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
						SafeCast<CBullet>( pContext->pEntity.GetPtr() )->Kill();
						CVector2 p0 = pContext->p0;
						pBarrage->DestroyBullet( i );

						int32 nType = SRand::Inst().Rand( 1, 3 );
						
						if( nType == 1 )
						{
							int32 nCount = SRand::Inst().Rand( 6, 9 );
							float fAngle1 = -SRand::Inst().Rand( 0.1f, 0.12f ) * nCount;
							float fAngle2 = SRand::Inst().Rand( 0.1f, 0.12f ) * nCount;
							float fVel1 = SRand::Inst().Rand( 120.0f, 240.0f );
							float fVel2 = SRand::Inst().Rand( 120.0f, 240.0f );
							for( int j = 0; j < nCount; j++ )
							{
								float t = j / ( nCount - 1.0f );
								float angle = fAngle + fAngle1 + ( fAngle2 - fAngle1 ) * t;
								float speed = fVel1 + ( fVel2 - fVel1 ) * t;
								pBarrage->InitBullet( nBullet++, nType, -1, p0, CVector2( cos( angle ), sin( angle ) ) * speed, CVector2( 0, 0 ) );
							}
						}
						else
						{
							int32 nCount = SRand::Inst().Rand( 3, 5 );
							int32 nCount1 = SRand::Inst().Rand( 3, 5 );
							float fAngle1 = -SRand::Inst().Rand( 0.12f, 0.16f ) * nCount;
							float fAngle2 = SRand::Inst().Rand( 0.12f, 0.16f ) * nCount;
							float fVel1 = SRand::Inst().Rand( 100.0f, 180.0f );
							float fVel2 = fVel1 + SRand::Inst().Rand( 80.0f, 140.0f );
							for( int j = 0; j < nCount; j++ )
							{
								float t = j / ( nCount - 1.0f );
								float angle = fAngle + fAngle1 + ( fAngle2 - fAngle1 ) * t;
								for( int k = 0; k < nCount1; k++ )
								{
									float speed = fVel1 + ( fVel2 - fVel1 ) * ( k / ( nCount1 - 1.0f ) );
									pBarrage->InitBullet( nBullet++, nType, -1, p0, CVector2( cos( angle ), sin( angle ) ) * speed, CVector2( 0, 0 ) );
								}
							}

						}
					}
					pBarrage->Yield( 2 );
					pBarrage->StopNewBullet();
				} );
				break;
			case 3:
				context.vecBulletTypes.push_back( m_pBullet4.GetPtr() );
				context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
				context.nBulletPageSize = 36;
				pBarrage = new CBarrage( context );
				pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
				{
					for( int i = 0; i < 6; i++ )
					{
						float fAngle = i * PI / 3;
						CVector2 vel( sin( fAngle ) * 60 + 120, cos( fAngle ) * 240 );
						vel = CVector2( vel.x * dir.x - vel.y * dir.y, vel.x * dir.y + vel.y * dir.x );
						pBarrage->InitBullet( i, 0, -1, p, vel, CVector2( 0, 0 ), false );
					}
					pBarrage->Yield( 20 );
					for( int i = 0; i < 6; i++ )
						pBarrage->GetBulletContext( i )->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
					pBarrage->Yield( 40 );

					uint32 nIndex[] = { 0, 1, 2, 3, 4, 5 };
					SRand::Inst().Shuffle( nIndex, ELEM_COUNT( nIndex ) );
					uint32 nBullet = 36;
					for( int i = 0; i < 6; i++ )
					{
						auto pContext = pBarrage->GetBulletContext( nIndex[i] );
						if( !pContext->IsValid() || !pContext->pEntity || !pContext->pEntity->GetStage() )
							continue;
						float fAngle = SRand::Inst().Rand( -0.3f, 0.3f );
						CVector2 dir1( cos( fAngle ), sin( fAngle ) );
						dir1 = CVector2( dir.x * dir1.x - dir.y * dir1.y, dir.x * dir1.y + dir.y * dir1.x );
						CVector2 vel( 300, 0 );
						vel = CVector2( vel.x * dir1.x - vel.y * dir1.y, vel.x * dir1.y + vel.y * dir1.x );
						pContext->SetBulletMove( vel, CVector2( 0, 0 ) );
						CVector2 p1 = pContext->p0;
						for( int j = 1; j < 6; j++ )
						{
							fAngle = j * PI / 3;
							vel = CVector2( cos( fAngle ) * 80 + 220, sin( fAngle ) * 50 );
							vel = CVector2( vel.x * dir1.x - vel.y * dir1.y, vel.x * dir1.y + vel.y * dir1.x );
							pBarrage->InitBullet( nBullet++, 1, -1, p1, vel, CVector2( 0, 0 ) );
						}
						pBarrage->Yield( 20 );
					}

					pBarrage->Yield( 2 );
					pBarrage->StopNewBullet();
				} );
				break;
			}
		}
		else
		{
			if( m_nDir == 0 || m_nDir == 2 )
			{
				context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
				context.vecBulletTypes.push_back( m_pBullet2.GetPtr() );
				context.vecBulletTypes.push_back( m_pBullet3.GetPtr() );
				context.nBulletPageSize = 90;

				pBarrage = new CBarrage( context );
				pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
				{
					auto pPlayer = GetStage()->GetPlayer();
					CVector2 playerPos = pPlayer ? pPlayer->GetPosition() : p + dir * 200;
					int32 nBullet = 0;
					CVector2 dirs[5];
					int32 r = SRand::Inst().Rand( 0, 2 );
					float v0 = 30;
					for( int i = 0; i < 5; i++ )
					{
						float fAngle = ( i - 2 ) * 0.5f;
						dirs[i] = CVector2( cos( fAngle ), sin( fAngle ) );
						dirs[i] = CVector2( dir.x * dirs[i].x - dir.y * dirs[i].y, dir.x * dirs[i].y + dir.y * dirs[i].x );
						pBarrage->InitBullet( nBullet++, 0, -1, p, dirs[i] * v0, CVector2( 0, 0 ), false );
					}
					pBarrage->Yield( 80 );
					for( int i = 1; i <= 3; i++ )
					{
						float fWeight = ( 3.5f - i ) / 3.5f;
						for( int j = 0; j < 5; j++ )
						{
							for( int k = 0; k < 5; k++ )
							{
								CVector2 p1 = p + dirs[k] * ( v0 * ( 80 * i + 7 * j ) / 60 );
								pBarrage->InitBullet( nBullet++, 2, -1, p1, dirs[k] * ( 200 - 15 * j ), CVector2( 0, 0 ) );
								if( j > 0 )
								{
									CVector2 d = playerPos - p1;
									if( d.Normalize() < 0.01f )
										d = dir;
									CVector2 dir0;
									dir0.Slerp( j * 0.2f, dirs[k], d );
									pBarrage->InitBullet( nBullet++, 2, -1, p1, dir0 * ( 200 - 10 * j ), CVector2( 0, 0 ) );
								}
							}
							pBarrage->Yield( 7 );
						}

						for( int j = 0; j < 9; j++ )
						{
							int j1 = j > 4 ? 8 - j : j;
							if( r )
								j1 = 4 - j1;
							for( int k = 0; k < 5; k++ )
							{
								CVector2 p1 = p + dirs[k] * ( v0 * ( 80 * i + 35 + j * 5 ) / 60 );
								CVector2 d = playerPos - p1;
								if( d.Normalize() < 0.01f )
									d = dir;
								CVector2 dir0;
								dir0.Slerp( fWeight, dirs[j1], d );
								pBarrage->InitBullet( nBullet++, 1, -1, p1, dir0 * ( 120 + j * 20 ), CVector2( 0, 0 ) );
							}
							pBarrage->Yield( 5 );
						}
						r = !r;

						if( pPlayer )
							playerPos = pPlayer->GetPosition();
					}

					pBarrage->Yield( 5 );
					for( int i = 0; i < 5; i++ )
						pBarrage->DestroyBullet( i );
					pBarrage->StopNewBullet();
				} );
			}
			else
			{
				context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
				context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
				context.vecBulletTypes.push_back( m_pBullet3.GetPtr() );
				context.vecLightningTypes.push_back( m_pBeam.GetPtr() );
				context.nBulletPageSize = 240;
				context.nLightningPageSize = 1;

				pBarrage = new CBarrage( context );
				pBarrage->AddFunc( [this, p, dir] ( CBarrage* pBarrage )
				{
					pBarrage->InitBullet( 0, 0, -1, p, CVector2( 0, 0 ), CVector2( 0, 0 ), false );

					auto pPlayer = GetStage()->GetPlayer();
					CVector2 d = pPlayer ? pPlayer->GetPosition() : p;
					d = d - p;
					d.y = Min( -abs( d.x ) * 3.0f, d.y );
					float l = d.Normalize();
					if( l < 0.01f )
						d = dir;
					l = Max( 384.0f, Min( l + 256.0f, 768.0f ) );
					CVector2 dPos = d * l;
					if( dPos.y + p.y < 0 )
						dPos = dPos * ( p.y / -dPos.y );
					CVector2 dPos0 = d * 64.0f;

					CVector2 v0[6] = { { -600, -300 }, { -600, 300 }, { -100, -800 }, { -100, 800 }, { 500, 500 }, { 500, -500 } };
					CVector2 a[6];
					float t = 1.0f;
					for( int i = 0; i < 6; i++ )
					{
						v0[i] = CVector2( v0[i].x * d.x - v0[i].y * d.y, v0[i].x * d.y + v0[i].y * d.x );
						a[i] = ( dPos0 - v0[i] * t ) / ( t * t * 0.5f );
					}
					int32 nB = 1 + 12 * 6;
					float fAngle0 = SRand::Inst().Rand( -PI, PI );
					float dAngle = SRand::Inst().Rand( PI / 6, PI / 4 );
					int8 r = SRand::Inst().Rand( 0, 2 ) ? 1 : -1;
					for( int i = 0; i < 60; i++ )
					{
						for( int j = 0; j < 6; j++ )
						{
							pBarrage->InitBullet( 1 + ( i % 12 ) * 6 + j, 1, -1, p, v0[j], a[j] );
						}

						if( i == 18 )
						{
							pBarrage->InitLightning( 0, 0, 0, 0, CVector2( 0, 0 ), dPos, false );
						}
						else if( i >= 24 )
						{
							int32 i1 = i - 24;
							SLightningContext* pContext = pBarrage->GetLightningContext( 0 );
							if( pContext && pContext->pEntity )
							{
								CVector2 p1 = SafeCast<CLightning>( pContext->pEntity.GetPtr() )->GetBeamEnd();
								float fAngle = ( i1 / 9 ) * PI / 6;
								fAngle += ( i1 % 3 ) * 0.25f;
								fAngle += ( i1 % 9 ) / 3 * 0.06f;
								fAngle *= r;
								fAngle += fAngle0;
								for( int j = 0; j < 6; j++ )
								{
									float fAngle1 = fAngle + j * PI / 3;
									pBarrage->InitBullet( nB++, 2, -1, p1, CVector2( cos( fAngle1 ), sin( fAngle1 ) )
										* ( 160.0f + 30 * ( i1 % 3 ) - 10 * ( ( i1 % 9 ) / 3 ) ), CVector2( 0, 0 ) );
								}
							}
							else
							{
								*(int32*)0 = 0;
							}
						}

						pBarrage->Yield( 5 );
					}

					pBarrage->DestroyLightning( 0 );
					for( int i = 0; i < 12; i++ )
					{
						for( int j = 0; j < 6; j++ )
						{
							pBarrage->DestroyBullet( 1 + ( i % 12 ) * 6 + j );
						}
						pBarrage->Yield( 5 );
					}

					pBarrage->DestroyBullet( 0 );
					pBarrage->StopNewBullet();
				} );
			}
		}
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->Start();

		m_pAI->Yield( 2.0f, false );
		pEnemyPart->SetTransparentRec( true );

		pImg->SetPlaySpeed( -1, false );
		m_pAI->Yield( 0.5f, false );
		pEnemyPart->bVisible = false;
	}
	catch( uint32 e )
	{
		m_bKilled = true;
		pEnemyPart->SetTransparentRec( true );
		pEnemyPart->KillEffect();
		pImg->SetFrames( 4, 5, 0 );
		pImg->SetPlaySpeed( 0, false );

		m_pAI->Yield( 1.0f, false );

		pImg->SetFrames( 4, 8, 8 );
		pImg->SetPlaySpeed( 1, false );
	}
}

void CWindow3Controller::OnAddedToStage()
{
	m_pAI = new AI();
	m_pAI->SetParentEntity( this );
}

void CWindow3Controller::AIFunc()
{
	m_pAI->Yield( 0.25f, false );

	CReference<CAIObject> pAIObject;
	CReference<CWindow3> pCurWindow;
	while( 1 )
	{
		if( pAIObject )
		{
			if( !pAIObject->GetParentEntity() || !pAIObject->IsRunning() )
				pAIObject = NULL;
		}
		if( !pAIObject )
		{
			if( pCurWindow && pCurWindow->IsKilled() )
				return;

			SRand::Inst().Shuffle( m_vecWindow3 );
			for( auto& pWindow : m_vecWindow3 )
			{
				if( !pWindow )
					continue;
				if( pWindow->IsKilled() )
				{
					pWindow = NULL;
					continue;
				}

				if( pCurWindow )
					pWindow->SetHp( pCurWindow->GetHp() );
				pCurWindow = pWindow;
				pAIObject = pWindow->TryPlay( /*m_nType*/ 0 );
				if( pAIObject )
					break;
			}
			if( pAIObject )
			{
				m_pAI->Yield( 8.0f, false );
				continue;
			}
		}

		m_pAI->Yield( 0.25f, false );
	}
}