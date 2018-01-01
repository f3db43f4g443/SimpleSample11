#include "stdafx.h"
#include "SpecialLv2.h"
#include "Stage.h"
#include "Player.h"
#include "Entities/Bullets.h"
#include "Explosion.h"
#include "MyLevel.h"
#include "Entities/EnemyCharacters.h"
#include "Common/Rand.h"
#include "Entities/Barrage.h"
#include "Effects/ParticleArrayEmitter.h"
#include "Common/Algorithm.h"

void CBarrel::Damage( SDamageContext& context )
{
	if( m_bKilled )
	{
		DEFINE_TEMP_REF_THIS();
		CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetBlockSize() * 0.5f;
		int32 nPreHp = -m_fHp;
		CExplosiveChunk::Damage( context );
		if( !GetParentEntity() )
			return;
		int32 nCurHp = -m_fHp;
		int32 nFireCount = ( nCurHp / m_nDeathDamage ) - ( nPreHp / m_nDeathDamage );
		if( nFireCount > 0 )
		{
			CMyLevel::GetInst()->AddShakeStrength( 10 );
			SBarrageContext context;
			context.pCreator = this;
			context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
			context.nBulletPageSize = 12 * nFireCount;

			CBarrage* pBarrage = new CBarrage( context );
			pBarrage->AddFunc( [nFireCount] ( CBarrage* pBarrage )
			{
				for( int iFire = 0; iFire < nFireCount; iFire++ )
				{
					for( int i = 0; i < 12; i++ )
					{
						float fAngle = ( i + SRand::Inst().Rand( -0.5f, 0.5f ) ) * PI / 6;
						CVector2 dir( cos( fAngle ), sin( fAngle ) );
						float speed = SRand::Inst().Rand( 300.0f, 500.0f );
						uint32 nBullet = iFire * 12 + i;
						pBarrage->InitBullet( nBullet, 0, -1, CVector2( 0, 0 ), dir * speed, dir * speed * -0.5f, true );
						CVector2 finalSpeed = dir * speed * 0.5f;
						pBarrage->AddDelayAction( 60, [=] () {
							auto pContext = pBarrage->GetBulletContext( nBullet );
							if( pContext->IsValid() )
								pContext->SetBulletMove( finalSpeed, CVector2( 0, 0 ) );
						} );
					}
					pBarrage->Yield( 5 );
				}
				pBarrage->StopNewBullet();
			} );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			pBarrage->SetPosition( globalTransform.GetPosition() + center );
			pBarrage->Start();
		}
	}
	else
		CExplosiveChunk::Damage( context );
}

void CBarrel::Explode()
{
	CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetBlockSize() * 0.5f;

	if( m_strEffect )
	{
		auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
		pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		pEffect->SetPosition( GetPosition() + center );
		pEffect->SetState( 2 );
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );

	SBarrageContext context;
	context.pCreator = this;
	context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
	context.nBulletPageSize = 12 * 5;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [] ( CBarrage* pBarrage )
	{
		for( int i = 0; i < 12; i++ )
		{
			float fAngle = ( i + SRand::Inst().Rand( -0.5f, 0.5f ) ) * PI / 6;
			for( int j = 0; j < 5; j++ )
			{
				float fAngle1 = fAngle + ( j - 2 ) * 0.08f;
				CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
				float fSpeed1 = 400 - abs( j - 2 ) * 25;
				pBarrage->InitBullet( i * 5 + j, 0, -1, CVector2( 0, 0 ), dir * fSpeed1, CVector2( 0, 0 ), true );
			}
		}
		pBarrage->Yield( 1 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( globalTransform.GetPosition() + center );
	pBarrage->Start();

	auto pExp = SafeCast<CExplosionWithBlockBuff>( m_strExp->GetRoot()->CreateInstance() );
	pExp->SetPosition( globalTransform.GetPosition() + center );
	pExp->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	CBlockBuff::SContext buffcontext;
	buffcontext.nLife = 600;
	buffcontext.nTotalLife = 600;
	buffcontext.fParams[0] = 3;
	pExp->Set( &buffcontext );
	pExp->SetCreator( this );

	CExplosiveChunk::Explode();
}

void CBarrel1::Damage( SDamageContext& context )
{
	if( m_bKilled )
	{
		DEFINE_TEMP_REF_THIS();
		CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetBlockSize() * 0.5f;
		int32 nPreHp = -m_fHp;
		CExplosiveChunk::Damage( context );
		if( !GetParentEntity() )
			return;
		int32 nCurHp = -m_fHp;
		int32 nFireCount = ( nCurHp / m_nDeathDamage ) - ( nPreHp / m_nDeathDamage );

		for( int i = 0; i < nFireCount; i++ )
		{
			CVector2 ofs[2] = { CVector2( 32, 60 ), CVector2( 96, 60 ) };
			for( int j = 0; j < 2; j++ )
			{
				for( int k = 0; k < 3; k++ )
				{
					auto pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
					pBullet->SetPosition( ofs[j] + globalTransform.GetPosition() );
					pBullet->SetVelocity( CVector2( SRand::Inst().Rand( -200.0f, 200.0f ), SRand::Inst().Rand( 350.0f, 500.0f ) ) );
					pBullet->SetAcceleration( CVector2( 0, -100 ) );
					pBullet->SetLife( SRand::Inst().Rand( 60, 70 ) );
					pBullet->SetCreator( this );
					pBullet->SetTangentDir( true );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}
			}
		}
	}
	else
		CExplosiveChunk::Damage( context );
}

void CBarrel1::OnKilled()
{
	CVector2 ofs[2] = { CVector2( 32, 60 ), CVector2( 96, 60 ) };
	for( int i = 0; i < 2; i++ )
	{
		auto pExp = SafeCast<CExplosionWithBlockBuff>( m_strExp->GetRoot()->CreateInstance() );
		pExp->SetPosition( ofs[i] );
		pExp->SetParentEntity( this );
		pExp->SetRenderParentBefore( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		CBlockBuff::SContext buffcontext;
		buffcontext.nLife = 300;
		buffcontext.nTotalLife = 300;
		buffcontext.fParams[0] = 3;
		pExp->Set( &buffcontext );
		pExp->SetCreator( this );
	}
	CChunkObject::OnKilled();
}

void CBarrel1::Explode()
{
	CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetBlockSize() * 0.5f;

	if( m_strEffect )
	{
		auto pEffect = SafeCast<CEffectObject>( m_strEffect->GetRoot()->CreateInstance() );
		pEffect->SetParentEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		pEffect->SetPosition( GetPosition() + center );
		pEffect->SetState( 2 );
	}
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );

	SBarrageContext context;
	context.pCreator = this;
	context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
	context.nBulletPageSize = 12 * 2 * 4;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [] ( CBarrage* pBarrage )
	{
		CVector2 ofs[2] = { CVector2( -32, 32 ), CVector2( 32, 32 ) };
		uint32 nBullet = 0;
		float fAngle0 = SRand::Inst().Rand( -PI, PI );
		for( int i = 0; i < 12; i++ )
		{
			float fAngle = i * PI * 2 / 24 + fAngle0;
			for( int j = 0; j < 2; j++ )
			{
				for( int k = 0; k < 4; k++ )
				{
					float fAngle1 = ( fAngle + k * PI / 2 ) * ( j > 0 ? 1 : -1 );
					CVector2 dir( cos( fAngle1 ), sin( fAngle1 ) );
					pBarrage->InitBullet( nBullet++, 0, -1, ofs[j], dir * 150, ( dir + CVector2( j > 0 ? -1 : 1, 0 ) ) * 150 );
				}
			}
			pBarrage->Yield( 5 );
		}

		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( globalTransform.GetPosition() + center );
	pBarrage->Start();

	CVector2 ofs[2] = { CVector2( 32, 60 ), CVector2( 96, 60 ) };
	for( int j = 0; j < 2; j++ )
	{
		for( int k = 0; k < 3; k++ )
		{
			auto pBullet = SafeCast<CBomb>( m_strBullet1->GetRoot()->CreateInstance() );
			pBullet->SetPosition( ofs[j] + globalTransform.GetPosition() );
			pBullet->SetVelocity( CVector2( SRand::Inst().Rand( -350.0f, 350.0f ), SRand::Inst().Rand( 350.0f, 500.0f ) ) );
			pBullet->SetAcceleration( CVector2( 0, -200.0f ) );
			pBullet->SetTangentDir( true );
			pBullet->SetCreator( this );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );

			auto pExp = SafeCast<CExplosionWithBlockBuff>( pBullet->GetExplosion() );
			CBlockBuff::SContext buffcontext;
			buffcontext.nLife = 600;
			buffcontext.nTotalLife = 600;
			buffcontext.fParams[0] = 3;
			pExp->Set( &buffcontext );
		}
	}

	CExplosiveChunk::Explode();
}

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
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
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
	return !m_bExploding;
}

bool CHouse::Enter( CCharacter * pCharacter )
{
	if( m_bExploding )
		return false;
	m_characters.push_back( pair<CReference<CCharacter>, int32>( pCharacter, 5 ) );
	pCharacter->SetParentEntity( NULL );
	m_bAnyoneEntered = true;
	for( auto& pEntrance : m_houseEntrances )
		pEntrance->SetState( 1 );
	return true;
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

void CHouse::DelayExplode()
{
	for( auto& pEntrance : m_houseEntrances )
		pEntrance->SetState( 2 );
	m_nCount = 0;
	m_bExploding = true;
	GetStage()->RegisterAfterHitTest( 10, &m_onTick );
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
	if( m_strSoundEffect )
		m_strSoundEffect->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
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
			pEntrance->SetState( 3 );
		}

		m_nCount++;
		if( m_nCount < 30 )
			GetStage()->RegisterAfterHitTest( 10, &m_onTick );
		return;
	}
	if( m_bExploding )
	{
		uint32 nCount = GetChunk()->nWidth * GetChunk()->nHeight / 16;
		for( int i = 0; i < nCount; i++ )
		{
			auto pEffect = SafeCast<CEffectObject>( m_pEft1->GetRoot()->CreateInstance() );
			pEffect->SetParentEntity( this );
			pEffect->SetPosition( CVector2( SRand::Inst().Rand<float>( 0.0f, GetChunk()->nWidth * CMyLevel::GetBlockSize() ),
				SRand::Inst().Rand<float>( 0.0f, GetChunk()->nWidth * CMyLevel::GetBlockSize() ) ) );
			pEffect->SetState( 2 );
		}

		float fAngle = SRand::Inst().Rand( -PI, PI );
		AddHitShake( CVector2( cos( fAngle ), sin( fAngle ) ) * 8 );
		m_nCount++;
		if( m_nCount < 24 )
			GetStage()->RegisterAfterHitTest( 10, &m_onTick );
		else
			Explode();
		return;
	}

	GetStage()->RegisterAfterHitTest( 20, &m_onTick );

	int32 nChar = -1;
	for( int i = 0; i < m_characters.size(); i++ )
	{
		if( m_characters[i].second )
			m_characters[i].second--;
		if( nChar == -1 && !m_characters[i].second )
			nChar = i;
	}
	if( GetHp() < GetMaxHp() * 0.4f )
	{
		m_bAnyoneEntered = true;
		for( auto& pEntrance : m_houseEntrances )
			pEntrance->SetState( 1 );
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
							DelayExplode();
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
						DelayExplode();
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

void CControlRoom::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );

	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	texRect.x += SRand::Inst().Rand<int32>( 0, m_nAltX ) * texRect.width;
	texRect.y += SRand::Inst().Rand<int32>( 0, m_nAltY ) * texRect.height;
	texRect.width /= 4;
	texRect.height /= 4;

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			CRectangle tex = texRect;

			switch( m_nType )
			{
			case 0:
			{
				int32 nY;
				if( iY > 0 )
					nY = Min<int32>( 2, pChunk->nHeight - iY - 1 );
				else
					nY = 3;
				tex.y += tex.height * nY;

				if( nY > 0 )
				{
					int32 nX;
					if( iX < pChunk->nWidth - 1 )
						nX = Min( iX, 2 );
					else
						nX = 3;
					tex.x += tex.width * nX;
				}
				else
				{
					if( iX == 0 )
						;
					else if( iX == pChunk->nWidth - 1 )
						tex.x += tex.width * 3;
					else
					{
						if( iX * 2 + 1 < pChunk->nWidth )
						{
							if( iX * 2 + 2 == pChunk->nWidth )
								tex.x += tex.width;
							else
							{
								tex.x += tex.width;
								tex.width /= 2;
							}
						}
						else if( iX * 2 + 1 > pChunk->nWidth )
						{
							if( iX * 2 == pChunk->nWidth )
								tex.x += tex.width * 2;
							else
							{
								tex.x += tex.width * 2.5f;
								tex.width /= 2;
							}
						}
						else
							tex.x += tex.width * 1.5f;
					}
				}
				break;
			}
			case 1:
			case 2:
				tex.y += tex.height * Min<int32>( 3, pChunk->nHeight - iY - 1 );
				tex.x += tex.width * iX;
				break;
			}

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
			pImage2D->SetTexRect( tex );
			GetRenderObject()->AddChild( pImage2D );
			GetBlock( iX, iY )->rtTexRect = tex;
		}
	}

	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		m_pDmgParticles[i] = static_cast<CEntity*>( m_pDamagedEffects[i] );
		m_pDamagedEffects[i] = NULL;
		auto pParticle = static_cast<CParticleFile*>( m_pDmgParticles[i]->GetResource() );
		int32 n = pChunk->nWidth * pChunk->nHeight / 8;
		for( int k = 0; k < n; k++ )
		{
			auto p = pParticle->CreateInstance( NULL );
			p->GetInstanceData()->GetData().isEmitting = false;
			p->SetPosition( CVector2( SRand::Inst().Rand<float>( 0, ( pChunk->nWidth - 4 ) * CMyLevel::GetBlockSize() ) + 2 * CMyLevel::GetBlockSize(),
				SRand::Inst().Rand<float>( 0, ( pChunk->nHeight - 4 ) * CMyLevel::GetBlockSize() ) + 2 * CMyLevel::GetBlockSize() ) );
			m_pDmgParticles[i]->AddChild( p );
		}

		m_pDmgParticles[i]->SetRenderObject( NULL );
	}
	m_nDmgParticles = m_nDamagedEffectsCount;
	m_nDamagedEffectsCount = 0;
	m_pDamagedEffectsRoot = NULL;

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CControlRoom::OnCreateComplete( CMyLevel * pLevel )
{
	if( !pLevel )
		return;

	CWindow3Controller* pController = new CWindow3Controller( m_nType );
	pController->SetParentEntity( this );

	switch( m_nType )
	{
	case 0:
	{
		auto pWindow = SafeCast<CWindow3>( m_pWindow[0]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( ( m_pChunk->nWidth * 0.5f + 1 ) * CMyLevel::GetBlockSize() / 2 - 6, m_pChunk->nHeight * CMyLevel::GetBlockSize() - 14 ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );

		pWindow = SafeCast<CWindow3>( m_pWindow[1]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( ( m_pChunk->nWidth * 1.5f - 1 ) * CMyLevel::GetBlockSize() / 2 + 6, m_pChunk->nHeight * CMyLevel::GetBlockSize() - 14 ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );

		pWindow = SafeCast<CWindow3>( m_pWindow[2]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( 12, ( m_pChunk->nHeight - 1 ) * CMyLevel::GetBlockSize() / 2 ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );

		pWindow = SafeCast<CWindow3>( m_pWindow[3]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( m_pChunk->nWidth * CMyLevel::GetBlockSize() - 12, ( m_pChunk->nHeight - 1 ) * CMyLevel::GetBlockSize() / 2 ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );
		break;
	}
	case 1:
	{
		auto pWindow = SafeCast<CWindow3>( m_pWindow[0]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( ( m_pChunk->nWidth - 1.5f ) * CMyLevel::GetBlockSize(), ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );

		pWindow = SafeCast<CWindow3>( m_pWindow[3]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( m_pChunk->nWidth * CMyLevel::GetBlockSize() - 10, ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );
		break;
	}
	case 2:
	{
		auto pWindow = SafeCast<CWindow3>( m_pWindow[1]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( 1.5f * CMyLevel::GetBlockSize(), ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );

		pWindow = SafeCast<CWindow3>( m_pWindow[2]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( 10, ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		pController->Add( pWindow );
		break;
	}
	default:
		break;
	}
}

void CControlRoom::Damage( SDamageContext & context )
{
	DEFINE_TEMP_REF_THIS();
	uint32 nLastDamageFrame = Min<float>( m_nDmgParticles, Max<float>( ( m_fHp * ( m_nDmgParticles + 1 ) - 1 ) / m_nMaxHp, 0 ) );
	CChunkObject::Damage( context );
	if( !GetStage() )
		return;
	int32 nDamageFrame = Min<float>( m_nDmgParticles, Max<float>( ( m_fHp * ( m_nDmgParticles + 1 ) - 1 ) / m_nMaxHp, 0 ) );
	
	for( int i = nLastDamageFrame - 1; i >= nDamageFrame; i-- )
	{
		m_pDmgParticles[i]->bVisible = true;
		for( auto pChild = m_pDmgParticles[i]->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			auto pParticle = static_cast<CParticleSystemObject*>( pChild );
			pParticle->SetAutoUpdateAnim( true );
			pParticle->GetInstanceData()->GetData().isEmitting = true;
		}
	}
}

void CControlRoom::OnKilled()
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

void CHouse2::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
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

	CRectangle texRect1 = m_texRect1;
	texRect1.width /= 3;
	texRect1.height /= 2;
	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			CRectangle tex;
			if( iY == pChunk->nHeight - 1 )
				tex = m_texRect;
			else
			{
				tex = texRect1;
				if( iX == pChunk->nWidth - 1 )
					tex.x += tex.width * 2;
				else if( iX )
					tex.x += tex.width;
				if( iY == 0 )
					tex.y += tex.height;
			}

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
			pImage2D->SetTexRect( tex );
			GetRenderObject()->AddChild( pImage2D );
			GetBlock( iX, iY )->rtTexRect = tex;
			if( iY < pChunk->nHeight - 1 )
				m_vecBlock1.push_back( pImage2D );

			for( int i = 0; i < m_nDamagedEffectsCount; i++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( pDamageEftTex[i] );
				m_pDamagedEffects[i]->AddChild( pImage2D );
			}
		}
	}

	GenObjs();
	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
	m_nHp1 += m_nHpPerSize1 * pChunk->nWidth * pChunk->nHeight;

	for( int i = 0; i < 3; i++ )
	{
		m_pObj[i]->SetParentEntity( NULL );
		m_pObj[i] = NULL;
		m_pObj1[i]->SetParentEntity( NULL );
		m_pObj1[i] = NULL;
	}
	m_pDecorator->SetParentEntity( NULL );
	m_pDecorator = NULL;
	m_pDoor->SetParentEntity( NULL );
	m_pDoor = NULL;
}

void CHouse2::OnCreateComplete( CMyLevel * pLevel )
{
	if( pLevel )
	{
		m_pAI = new AI();
		m_pAI->SetParentEntity( this );
	}
}

void CHouse2::GenObjs()
{
	uint32 nWidth = GetChunk()->nWidth;
	uint32 nHeight = GetChunk()->nHeight - 1;
	if( nWidth < 4 || nHeight < 2 )
		return;

	vector<TVector2<int32> > vecObjs;
	uint32 l[] = { 3, 4, 6 };
	int32 l0 = nWidth - floor( nWidth * SRand::Inst().Rand( 0.0f, 0.2f ) );
	int32 nMaxWidth = -1;
	const char* patterns[] = { "102", "201", "210", "101", "1102", "2011" };
	const char* pattern;
	if( nHeight >= 4 )
	{
		SRand::Inst().Shuffle( patterns, ELEM_COUNT( patterns ) );
		int32 nMaxPattern = -1;
		for( int i = 0; i < ELEM_COUNT( patterns ); i++ )
		{
			const char* pattern = patterns[i];
			int32 nLen = strlen( pattern );
			int32 w = 0, k = 0;
			while( w + l[pattern[k] - '0'] <= l0 )
			{
				w += l[pattern[k] - '0'];
				k++;
				if( k >= nLen )
					k = 0;
			}
			if( w > nMaxWidth )
			{
				nMaxWidth = w;
				nMaxPattern = i;
			}
		}
		pattern = patterns[nMaxPattern];
	}
	else
	{
		pattern = "2";
		nMaxWidth = nWidth / 6 * 6;
	}

	{
		int32 nLen = strlen( pattern );
		int32 w = 0, k = 0;
		while( w + l[pattern[k] - '0'] <= l0 )
		{
			vecObjs.push_back( TVector2<int32>( w, pattern[k] - '0' ) );
			w += l[pattern[k] - '0'];
			k++;
			if( k >= nLen )
				k = 0;
		}
	}

	{
		int32 w = 0, k = 0;
		uint32 nWidthLeft = nWidth - nMaxWidth;
		int8* c = (int8*)alloca( nWidthLeft + vecObjs.size() );
		SRand::Inst().C( nWidthLeft, nWidthLeft + vecObjs.size(), c );
		for( int i = 0; i < nWidthLeft + vecObjs.size(); i++ )
		{
			if( c[i] )
				w++;
			else
				vecObjs[k++].x += w;
		}

		if( SRand::Inst().Rand( 0, 2 ) )
		{
			for( auto& v : vecObjs )
				v.x = nWidth - v.x - l[v.y];
		}
	}

	vector<int8> data;
	data.resize( nWidth * nHeight );

	int32 h1 = nHeight - ( nHeight - 4 ) / 2 - 4;
	int32 h2 = nHeight - 2;
	for( auto& v : vecObjs )
	{
		TRectangle<int32> rect;
		switch( v.y )
		{
		case 0:
			rect = TRectangle<int32>( v.x, 0, 3, 4 );
			break;
		case 1:
			rect = TRectangle<int32>( v.x, h1, 4, 4 );
			break;
		case 2:
			rect = TRectangle<int32>( v.x, h2, 6, 2 );
			break;
		}

		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				data[i + j * nWidth] = 1;
			}
		}
		GenObj( CVector2( rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f ) * CMyLevel::GetBlockSize(), v.y );
	}

	CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( m_pDecorator->GetResource() );
	vector<int32> vec;
	for( int i = 0; i < nWidth; i++ )
		vec.push_back( i );
	SRand::Inst().Shuffle( vec );
	vector<TVector2<int32> > vec1;
	CRectangle r0( 6, 2, nWidth * CMyLevel::GetBlockSize() - 12, nHeight * CMyLevel::GetBlockSize() - 2 );
	for( int i = 0; i < nWidth; i++ )
	{
		int32 x = vec[i];
		if( data[x] )
			continue;
		auto rect = PutRect( data, nWidth, nHeight, TVector2<int32>( x, 0 ), TVector2<int32>( 1, 1 ),
			TVector2<int32>( SRand::Inst().Rand<int32>( 1, 4 ), SRand::Inst().Rand<int32>( 2, nHeight ) ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 2 );
		if( rect.width > 0 )
		{
			CRectangle rt( rect.x * CMyLevel::GetBlockSize(), rect.y * CMyLevel::GetBlockSize(), rect.width * CMyLevel::GetBlockSize(), rect.height * CMyLevel::GetBlockSize() );
			rt.height += SRand::Inst().Rand( -16, 16 );
			rt = rt * r0;
			uint32 n = Max<uint32>( 1, floor( SRand::Inst().Rand( rt.height / 96, rt.height / 32 ) ) );
			for( int i = 0; i < n; i++ )
			{
				float h0 = floor( ( rt.height * i / n - SRand::Inst().Rand( 0, 8 ) ) * 0.5f ) * 2;
				float h1 = floor( ( rt.height * ( i + 1 ) / n + SRand::Inst().Rand( 0, 8 ) ) * 0.5f ) * 2;
				CRectangle rt1( rt.x, rt.y + h0, rt.width, h1 - h0 );

				float fOfs = SRand::Inst().Rand( -16, 16 );
				rt1.x += fOfs;
				rt1.SetSizeX( SRand::Inst().Rand( rt1.width - 16 + abs( fOfs ),
					rt1.width + 16 - abs( fOfs ) ) );
				rt1 = rt1 * r0;

				auto pImage = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage->SetRect( rt );
				pImage->SetTexRect( CRectangle( SRand::Inst().Rand( 0u, m_nDecoTexSize ), SRand::Inst().Rand( 0u, m_nDecoTexSize ),
					rt.width, rt.height ) * ( 1.0f / m_nDecoTexSize ) );
				CVector4& param = *pImage->GetParam();
				param = CVector4( SRand::Inst().Rand( 0.93f, 1.07f ), SRand::Inst().Rand( 0.93f, 1.07f ), SRand::Inst().Rand( 0.93f, 1.07f ), 1 )
					* SRand::Inst().Rand( 0.5f, 1.0f );
				param.w = 1;
				m_p1->AddChild( pImage );

				for( int k = rect.y; k < rect.GetBottom(); k++ )
				{
					vec1.push_back( TVector2<int32>( rect.x, k ) );
					vec1.push_back( TVector2<int32>( rect.GetRight() - 1, k ) );
				}
				for( int k = rect.x; k < rect.GetRight(); k++ )
				{
					vec1.push_back( TVector2<int32>( k, rect.GetBottom() - 1 ) );
				}
			}
		}
	}
	for( auto& n : data )
	{
		if( n == 2 )
			n = 0;
	}

	SRand::Inst().Shuffle( vec1 );
	for( auto& p : vec1 )
	{
		if( data[p.x + p.y * nWidth] )
			continue;

		TVector2<int32> sizeMax;
		sizeMax.x = sizeMax.y = SRand::Inst().Rand<int32>( 3, 5 );
		auto rect = PutRect( data, nWidth, nHeight, p, TVector2<int32>( 2, 2 ), sizeMax,
			TRectangle<int32>( 0, p.y, nWidth, nHeight - p.y ), -1, 2 );
		if( rect.width > 0 )
		{
			CRectangle rt( rect.x * CMyLevel::GetBlockSize(), rect.y * CMyLevel::GetBlockSize(), rect.width * CMyLevel::GetBlockSize(), rect.height * CMyLevel::GetBlockSize() );
			float fOfs = SRand::Inst().Rand( -16, 16 );
			rt.x += fOfs;
			rt.SetSizeX( SRand::Inst().Rand( rt.width - 16 + abs( fOfs ),
				rt.width + 16 - abs( fOfs ) ) );
			fOfs = SRand::Inst().Rand( -16, 16 );
			rt.y += fOfs;
			rt.SetSizeY( SRand::Inst().Rand( rt.height - 16 + abs( fOfs ),
				rt.height + 16 - abs( fOfs ) ) );
			rt = rt * r0;

			CVector2 center = rt.GetCenter();
			float k = SRand::Inst().Rand( 0.0f, Min( rt.width / rt.height, rt.height / rt.width ) * 0.15f );
			float r = atan( k );
			float w = floor( ( rt.width - k * rt.height ) / sqrt( 1 - k * k ) / 2 ) * 2;
			float h = floor( ( rt.height - k * rt.width ) / sqrt( 1 - k * k ) / 2 ) * 2;
			if( SRand::Inst().Rand( 0, 3 ) )
			{
				swap( w, h );
				r += PI * 0.5f;
			}

			auto pImage = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage->SetRect( CRectangle( -w * 0.5f, -h * 0.5f, w, h ) );
			pImage->SetTexRect( CRectangle( SRand::Inst().Rand( 0u, m_nDecoTexSize ), SRand::Inst().Rand( 0u, m_nDecoTexSize ),
				w, h ) * ( 1.0f / m_nDecoTexSize ) );
			pImage->SetPosition( center );
			pImage->SetRotation( SRand::Inst().Rand( 0, 2 ) ? -r : r );
			CVector4& param = *pImage->GetParam();
			param = CVector4( SRand::Inst().Rand( 0.93f, 1.07f ), SRand::Inst().Rand( 0.93f, 1.07f ), SRand::Inst().Rand( 0.93f, 1.07f ), 1 )
				* SRand::Inst().Rand( 0.5f, 1.0f );
			param.w = 1;
			m_p1->AddChild( pImage );
		}
	}
}

void CHouse2::GenObj( const CVector2& p, uint8 nType )
{
	m_vecObj.resize( m_vecObj.size() + 1 );
	auto& obj = m_vecObj.back();
	obj.nType = nType;
	obj.ofs = p;
	{
		CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( m_pObj[nType]->GetResource() );
		CImage2D* pImg = static_cast<CImage2D*>( m_pObj[nType]->GetRenderObject() );
		CImage2D* pImg1 = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
		pImg1->SetRect( pImg->GetElem().rect );
		pImg1->SetTexRect( pImg->GetElem().texRect );
		pImg1->SetPosition( p );
		m_p1->AddChild( pImg1 );
	}

	{
		CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( m_pObj1[nType]->GetResource() );
		CMultiFrameImage2D* pImg = static_cast<CMultiFrameImage2D*>( m_pObj1[nType]->GetRenderObject() );
		CMultiFrameImage2D* pImg1 = static_cast<CMultiFrameImage2D*>( pDrawableGroup->CreateInstance() );
		pImg1->SetFrames( pImg->GetFrameBegin(), pImg->GetFrameBegin() + 1, 0 );
		pImg1->SetPosition( p );
		m_p1->AddChild( pImg1 );
		pImg1->SetRenderParent( this );
		pImg1->SetAutoUpdateAnim( true );
		obj.pImg = pImg1;
	}

	if( nType == 0 )
	{
		CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( m_pDoor->GetResource() );
		CMultiFrameImage2D* pImg1 = static_cast<CMultiFrameImage2D*>( pDrawableGroup->CreateInstance() );
		pImg1->SetFrames( 0, 1, 0 );
		pImg1->SetPosition( p );
		m_p1->AddChild( pImg1 );
		pImg1->SetRenderParent( this );
		pImg1->SetAutoUpdateAnim( true );
		obj.pImg1 = pImg1;
	}
	else
	{
		CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( m_pDecorator->GetResource() );
		CImage2D* pImg1 = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );

		CRectangle rect;
		if( nType == 1 )
			rect = CRectangle( -32, -32, 64, 64 );
		else
			rect = CRectangle( -64, -8, 128, 16 );

		pImg1->SetRect( rect );
		pImg1->SetTexRect( CRectangle( SRand::Inst().Rand( 0u, m_nDecoTexSize ), SRand::Inst().Rand( 0u, m_nDecoTexSize ),
			rect.width, rect.height ) * ( 1.0f / m_nDecoTexSize ) );
		pImg1->SetPosition( p );
		m_p1->AddChild( pImg1 );
		pImg1->SetRenderParent( this );
		obj.pImg1 = pImg1;
	}
}

void CHouse2::AIFunc()
{
	while( true )
	{
		//closed
		while( true )
		{
			m_pAI->Yield( 0.1f, true );
			if( globalTransform.GetPosition().y + GetChunk()->nHeight * CMyLevel::GetBlockSize()
				<= CMyLevel::GetInst()->GetBoundWithLvBarrier().GetBottom() )
			{
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( pPlayer )
				{
					CRectangle rect( -64, -320, GetChunk()->nWidth * CMyLevel::GetBlockSize() + 128,
						GetChunk()->nHeight * CMyLevel::GetBlockSize() + 320 );
					if( m_bDamaged )
					{
						rect = CRectangle( -512, -1024, GetChunk()->nWidth * CMyLevel::GetBlockSize() + 1024,
							GetChunk()->nHeight * CMyLevel::GetBlockSize() + 1024 );
						m_bDamaged = false;
					}
					rect = rect.Offset( globalTransform.GetPosition() );
					if( rect.Contains( pPlayer->GetPosition() ) )
						break;
				}
			}
		}
		//open
		for( auto& obj : m_vecObj )
		{
			auto pImage = static_cast<CMultiFrameImage2D*>( obj.pImg.GetPtr() );
			pImage->SetFrames( pImage->GetFrameBegin(), pImage->GetFrameBegin() + 4, 8 );
			pImage->SetPlaySpeed( 1, false );
			if( obj.nType == 0 )
			{
				auto pImage1 = static_cast<CMultiFrameImage2D*>( obj.pImg1.GetPtr() );
				pImage1->SetFrames( 0, 4, 8 );
				pImage1->SetPlaySpeed( 1, false );
			}
		}
		for( int i = 31; i >= 0; i-- )
		{
			m_pAI->Yield( 0, true );
			for( auto& obj : m_vecObj )
			{
				if( obj.nType == 0 )
					continue;
				auto pImage = static_cast<CImage2D*>( obj.pImg1.GetPtr() );
				auto rect = pImage->GetElem().rect;
				auto texRect = pImage->GetElem().texRect;
				float h = obj.nType == 1 ? i * 2 : floor( i * 0.5f );
				rect.SetTop( rect.GetBottom() - h );
				texRect.height = h / m_nDecoTexSize;
				pImage->SetRect( rect );
				pImage->SetTexRect( texRect );
				pImage->SetBoundDirty();
			}
		}
		m_pAI->Yield( 0.5f, true );

		//attack
		int32 nType[3];
		for( int i = 0; i < 3; i++ )
			nType[i] = SRand::Inst().Rand( 0, 2 );
		for( int i = 0; i < 3; i++ )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
				break;
			CRectangle rect( -512, -1024, GetChunk()->nWidth * CMyLevel::GetBlockSize() + 1024,
				GetChunk()->nHeight * CMyLevel::GetBlockSize() + 1024 );
			rect = rect.Offset( globalTransform.GetPosition() );
			if( !rect.Contains( pPlayer->GetPosition() ) )
				break;

			for( auto& obj : m_vecObj )
			{
				CPrefab* pPrefabs[4] = { m_pBullet[0].GetPtr(), m_pBullet[1].GetPtr(), m_pBullet[2].GetPtr(), m_pBullet[3].GetPtr() };
				CreateBarrage( obj.nType + nType[obj.nType] * 3, m_p1, obj.ofs, pPrefabs );
			}

			m_pAI->Yield( 2.5f, true );
		}

		//close
		for( auto& obj : m_vecObj )
		{
			auto pImage = static_cast<CMultiFrameImage2D*>( obj.pImg.GetPtr() );
			pImage->SetFrames( pImage->GetFrameBegin(), pImage->GetFrameBegin() + 4, 8 );
			pImage->SetPlayPercent( 1.0f );
			pImage->SetPlaySpeed( -1, false );
			if( obj.nType == 0 )
			{
				auto pImage1 = static_cast<CMultiFrameImage2D*>( obj.pImg1.GetPtr() );
				pImage1->SetFrames( 0, 4, 8 );
				pImage1->SetPlayPercent( 1.0f );
				pImage1->SetPlaySpeed( -1, false );
			}
		}
		for( int i = 1; i <= 32; i++ )
		{
			m_pAI->Yield( 0, true );
			for( auto& obj : m_vecObj )
			{
				if( obj.nType == 0 )
					continue;
				auto pImage = static_cast<CImage2D*>( obj.pImg1.GetPtr() );
				auto rect = pImage->GetElem().rect;
				auto texRect = pImage->GetElem().texRect;
				float h = obj.nType == 1 ? i * 2 : floor( i * 0.5f );
				rect.SetTop( rect.GetBottom() - h );
				texRect.height = h / m_nDecoTexSize;
				pImage->SetRect( rect );
				pImage->SetTexRect( texRect );
				pImage->SetBoundDirty();
			}
		}

		m_bDamaged = false;
		m_pAI->Yield( 5.0f, true );
	}
}

void CHouse2::CreateBarrage( uint32 nType, CEntity * pRef, const CVector2 & ofs, CPrefab* pBullets[4] )
{
	CReference<CEntity> pEntity = pRef;

	SBarrageContext context;
	context.pCreator = pRef;
	CBarrage* pBarrage;
	switch( nType )
	{
	case 0:
		context.vecBulletTypes.push_back( pBullets[0] );
		context.nBulletPageSize = 30;

		pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [ofs] ( CBarrage* pBarrage )
		{
			CVector2 p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs - CVector2( 0, 16 ) );
			CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
			float fAngle = pPlayer ? atan2( pPlayer->y - p0.y, pPlayer->x - p0.x ) : -PI / 2;
			for( int i = 0; i < 6; i++ )
			{
				float fAngle1 = fAngle + SRand::Inst().Rand( -1.25f, 1.25f );
				pBarrage->InitBullet( i * 5, -1, -1, p0, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 200.0f, CVector2( 0, 0 ), false,
					0, SRand::Inst().Rand( 1.2f, 1.5f ) * ( SRand::Inst().Rand( 0, 2 ) ? -1 : 1 ) );
				for( int j = 0; j < 4; j++ )
				{
					float f = j - 1.5f;
					pBarrage->InitBullet( i * 5 + 1 + j, 0, i * 5, CVector2( 0, f * 24 ), CVector2( 0, f * 12 ), CVector2( 0, 0 ) );
				}
				pBarrage->Yield( 25 );
				if( !pBarrage->GetCreator()->GetStage() )
					break;
				p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
				pPlayer = pBarrage->GetStage()->GetPlayer();
				float fAngle = pPlayer ? atan2( pPlayer->y - p0.y, pPlayer->x - p0.x ) : -PI / 2;
			}
			pBarrage->StopNewBullet();
		} );
		break;
	case 1:
		context.vecBulletTypes.push_back( pBullets[3] );
		context.nBulletPageSize = 12;

		pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [ofs] ( CBarrage* pBarrage )
		{
			CVector2 p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
			CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
			float fAngle = pPlayer ? atan2( pPlayer->y - p0.y, pPlayer->x - p0.x ) : -PI / 2;
			int32 nBullet = 0;
			for( int i = 0; i < 3; i++ )
			{
				uint32 n = 3 + i;
				for( int k = 0; k < n; k++ )
				{
					float fAngle1 = fAngle + ( k - ( n - 1 ) * 0.5f ) * 0.5f;
					pBarrage->InitBullet( nBullet++, 0, -1, p0, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 240, CVector2( 0, 0 ) );
				}
				pBarrage->Yield( 15 );
				if( !pBarrage->GetCreator()->GetStage() )
					break;
				p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
			}
			pBarrage->Yield( 35 );
			pPlayer = pBarrage->GetStage()->GetPlayer();
			fAngle = pPlayer ? atan2( pPlayer->y - p0.y, pPlayer->x - p0.x ) : -PI / 2;
			pBarrage->StopNewBullet();
		} );
		break;
	case 2:
		context.vecBulletTypes.push_back( pBullets[0] );
		context.vecBulletTypes.push_back( pBullets[2] );
		context.nBulletPageSize = 48;

		pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [ofs] ( CBarrage* pBarrage )
		{
			CVector2 p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
			CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
			CVector2 d = pPlayer ? pPlayer->GetPosition() - p0 : CVector2( 0, -300 );
			CVector2 d1 = d;
			d.x *= SRand::Inst().Rand( 0.1f, 0.3f );
			d.Normalize();
			d = d * 100.0f + CVector2( SRand::Inst().Rand( -50.0f, 50.0f ), SRand::Inst().Rand( -50.0f, 50.0f ) );
			d1.x *= SRand::Inst().Rand( 0.1f, 0.3f );
			d1.Normalize();
			d1 = d1 * 300.0f + CVector2( SRand::Inst().Rand( -100.0f, 100.0f ), SRand::Inst().Rand( -100.0f, 100.0f ) );
			d1 = d1 - d;
			if( d1.Normalize() < 0.01f )
				d1 = CVector2( 0, -1 );
			d1 = d1 * SRand::Inst().Rand( 180.0f, 220.0f );

			CVector2 d2 = d1;
			d1 = d1 * SRand::Inst().Rand( 0.4f, 0.6f ) + CVector2( d1.y, -d1.x ) * SRand::Inst().Rand( -0.4f, 0.4f );

			float h1 = Min( d2.y, Min( d.y, d1.y ) ) + p0.y;
			if( h1 < 10.0f )
			{
				d.y += 10.0f - h1;
				d1.y += 10.0f - h1;
				d2.y += 10.0f - h1;
			}

			float fDir = SRand::Inst().Rand( 0, 2 ) ? 1 : -1;
			for( int i = 0; i < 12; i++ )
			{
				float t = ( i + 0.5f ) / 12.0f;
				pBarrage->InitBullet( i, -1, -1, p0 + CVector2( fDir * ( t - 0.5f ) * 128, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ),
					false, SRand::Inst().Rand( -PI, PI ), SRand::Inst().Rand( -2.0f, 2.0f ) );
				pBarrage->GetBulletContext( i )->MoveTowards( p0 + d + d1 * ( 2 * t * ( 1 - t ) ) + d2 * t * t, 60 );
				for( int j = 0; j < 3; j++ )
				{
					pBarrage->InitBullet( i + 12 + j * 12, 0, i, CVector2( SRand::Inst().Rand( -8.0f, 8.0f ), SRand::Inst().Rand( -8.0f, 8.0f ) ), CVector2( 0, 0 ), CVector2( 0, 0 ) );
				}
				pBarrage->Yield( 1 );
				p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
			}
			pBarrage->Yield( 60 );

			float fAngle = SRand::Inst().Rand( -PI, PI );
			float dAngle = PI / 18 * ( SRand::Inst().Rand( 0, 2 ) ? 1 : -1 )
				+ PI * 2 / 3 * ( SRand::Inst().Rand( 0, 2 ) ? 1 : -1 );
			for( int k = 0; k < 3; k++ )
			{
				for( int i = 0; i < 12; i++ )
				{
					auto pContext = pBarrage->GetBulletContext( i );
					pContext->SetBulletMove( pContext->v, pContext->a );
					auto pContext1 = pBarrage->GetBulletContext( i + 12 + k * 12 );
					if( pContext1->pEntity && pContext1->pEntity->GetParentEntity() )
						pBarrage->InitBullet( i + 12 + k * 12, 1, -1, pContext->p0, CVector2( cos( fAngle ), sin( fAngle ) ) * ( 150 + i * 10 ), CVector2( 0, 0 ),
							false, 0, 5.0f );

					fAngle += dAngle;
					pBarrage->Yield( 1 );
				}
				pBarrage->Yield( 8 );
			}

			pBarrage->StopNewBullet();
		} );
		break;
	case 3:
		context.vecBulletTypes.push_back( pBullets[0] );
		context.vecBulletTypes.push_back( pBullets[1] );
		context.nBulletPageSize = 48;

		pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [ofs] ( CBarrage* pBarrage )
		{
			CVector2 p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs - CVector2( 0, 64 ) );

			for( int i = 0; i < 12; i++ )
			{
				if( pBarrage->GetCreator()->GetStage() )
				{
					CVector2 v( SRand::Inst().Rand( -60.0f, 60.0f ), SRand::Inst().Rand( -300.0f, -100.0f ) );
					pBarrage->InitBullet( i, -1, -1, p0 + CVector2( 0, SRand::Inst().Rand( 0.0f, 96.0f ) ),
						v, v * -0.5f, false, SRand::Inst().Rand( -PI, PI ), SRand::Inst().Rand( -2.0f, 2.0f ) );
					for( int j = 0; j < 3; j++ )
					{
						pBarrage->InitBullet( i + 12 + j * 12, 0, i, CVector2( SRand::Inst().Rand( -8.0f, 8.0f ), SRand::Inst().Rand( -8.0f, 8.0f ) ), CVector2( 0, 0 ), CVector2( 0, 0 ) );
					}
				}
				pBarrage->Yield( 3 );
				p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs - CVector2( 0, 64 ) );
			}
			pBarrage->Yield( 120 - 12 * 3 );

			for( int i = 0; i < 12; i++ )
			{
				auto pContext = pBarrage->GetBulletContext( i );
				if( pContext->IsValid() )
				{
					pContext->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
					for( int k = 0; k < 3; k++ )
					{
						auto pContext1 = pBarrage->GetBulletContext( i + 12 + k * 12 );
						if( pContext1->pEntity && pContext1->pEntity->GetParentEntity() )
						{
							float fAngle = SRand::Inst().Rand( -PI, PI );
							pBarrage->InitBullet( i + 12 + k * 12, 1, -1, pContext->p0, CVector2( cos( fAngle ), sin( fAngle ) ) * SRand::Inst().Rand( 150.0f, 200.0f ), CVector2( 0, 0 ) );
						}
					}
					pBarrage->DestroyBullet( i );
				}

				pBarrage->Yield( 3 );
			}

			pBarrage->StopNewBullet();
		} );
		break;
	case 4:
		context.vecBulletTypes.push_back( pBullets[3] );
		context.nBulletPageSize = 6;

		pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [ofs] ( CBarrage* pBarrage )
		{
			CVector2 p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
			CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
			float fAngle = pPlayer ? atan2( pPlayer->y - p0.y, pPlayer->x - p0.x ) : -PI / 2;
			int32 nBullet = 0;
			for( int j = 0; j < 6; j++ )
			{
				uint32 n = 3;
				float fAngle1 = fAngle + SRand::Inst().Rand( -0.25f, 0.25f );
				pBarrage->InitBullet( nBullet++, 0, -1, p0, CVector2( cos( fAngle1 ), sin( fAngle1 ) ) * 320, CVector2( 0, 0 ) );
				pBarrage->Yield( 25 );
				if( !pBarrage->GetCreator()->GetStage() )
					break;
				p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
			}
			pPlayer = pBarrage->GetStage()->GetPlayer();
			fAngle = pPlayer ? atan2( pPlayer->y - p0.y, pPlayer->x - p0.x ) : -PI / 2;
			pBarrage->StopNewBullet();
		} );
		break;
	case 5:
		context.vecBulletTypes.push_back( pBullets[1] );
		context.nBulletPageSize = 33;

		pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [ofs] ( CBarrage* pBarrage )
		{
			uint32 nBullet = 0;
			CVector2 p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );

			for( int i = 0; i < 3; i++ )
			{
				CPlayer* pPlayer = pBarrage->GetStage()->GetPlayer();
				uint8 b = SRand::Inst().Rand( 0, 2 );
				CVector2 d[3] = { { b ? 1.0f : -1.0f, 0.2f }, { SRand::Inst().Rand( -0.3f, 0.3f ), 0.6f }, { b ? -1.0f : 1.0f, 0.2f } };
				for( int i = 0; i < 3; i++ )
				{
					d[i] = pPlayer ? ( pPlayer->GetPosition() - ( p0 + CVector2( d[i].x * 64, 0 ) ) ) * d[i].y : CVector2( 0, 0 );
					if( d[i].Length2() < 1 )
						d[i] = CVector2( 0, -200 );
				}

				for( int i = 0; i < 11; i++ )
				{
					float t = i / 10.0f;
					CVector2 p = p0 + CVector2( ( b ? -1 : 1 ) * ( -64 + 128 * t ), 0 );
					CVector2 target = p0 + d[0] + d[1] * ( 2 * t * ( 1 - t ) ) + d[2] * t * t;
					CVector2 v = target - p;
					if( v.Normalize() < 0 )
						v = CVector2( 0, -1 );
					v = v * ( 225.0f - ( i - 5 ) * ( i - 5 ) * 2 );
					pBarrage->InitBullet( nBullet++, 0, -1, p, v, CVector2( 0, 0 ) );
					pBarrage->Yield( 2 );
					if( !pBarrage->GetCreator()->GetStage() )
						break;
					p0 = pBarrage->GetCreator()->globalTransform.MulVector2Pos( ofs );
				}
				pBarrage->Yield( 32 );
				if( !pBarrage->GetCreator()->GetStage() )
					break;
			}

			pBarrage->StopNewBullet();
		} );
		break;
	}
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->Start();
}

void CHouse2::Kill()
{
	if( m_pAI )
	{
		m_pAI->SetParentEntity( NULL );
		m_pAI = NULL;
	}

	if( m_nState )
	{
		CChunkObject::Kill();
		return;
	}

	m_nState = 1;
	m_nMaxHp = m_nHp1;
	Repair( -1 );

	m_p1->SetParentEntity( NULL );
	m_p1 = NULL;
	CVector2 texOfs( m_texRect2.x - m_texRect1.x, m_texRect2.y - m_texRect1.y );
	for( auto& pRenderObject : m_vecBlock1 )
	{
		auto pImage2D = static_cast<CImage2D*>( pRenderObject.GetPtr() );
		pImage2D->SetTexRect( pImage2D ->GetElem().texRect.Offset( texOfs ) );
	}

	for( int i = 0; i < GetChunk()->nWidth; i++ )
	{
		for( int j = 0; j < GetChunk()->nHeight - 1; j++ )
		{
			auto& block = *GetChunk()->GetBlock( i, j );
			auto pBlockObject = SafeCast<CBlockObject>( block.pEntity.GetPtr() );
			pBlockObject->ClearBuffs();
			pBlockObject->ClearEfts();
			block.rtTexRect = block.rtTexRect.Offset( texOfs );
			block.eBlockType = eBlockType_LowBlock;
		}
	}

	for( auto& obj : m_vecObj )
	{
		auto pEntity = SafeCast<CEntity>( m_pObjPrefab[obj.nType]->GetRoot()->CreateInstance() );
		pEntity->SetPosition( obj.ofs );
		pEntity->SetParentEntity( this );

		class AI1 : public CAIObject
		{
		public:
			AI1( uint8 nType, CPrefab* pBullets[4] ) : m_nType( nType )
			{ for( int i = 0; i < 4; i++ ) m_pBullets[i] = pBullets[i]; }
		protected:
			virtual void AIFunc() override
			{
				Yield( 0.5f, true );
				while( true )
				{
					CPrefab* pPrefabs[4] = { m_pBullets[0].GetPtr(), m_pBullets[1].GetPtr(), m_pBullets[2].GetPtr(), m_pBullets[3].GetPtr() };
					CreateBarrage( m_nType + SRand::Inst().Rand( 0, 2 ) * 3, GetParentEntity(), CVector2( 0, 0 ), pPrefabs );

					Yield( 5.0f, true );
				}
			}
			CReference<CPrefab> m_pBullets[4];
			uint8 m_nType;
		};
		CPrefab* pPrefabs[4] = { m_pBullet[0].GetPtr(), m_pBullet[1].GetPtr(), m_pBullet[2].GetPtr(), m_pBullet[3].GetPtr() };
		auto pAI1 = new AI1( obj.nType, pPrefabs );
		pAI1->SetParentEntity( pEntity );
	}
	m_vecObj.clear();

	OnKilled();
}

void CHouse2::Crush()
{
	m_triggerCrushed.Trigger( 0, this );
	CChunkObject::Kill();
}

void CHouse2::Damage( SDamageContext & context )
{
	m_bDamaged = true;
	CChunkObject::Damage( context );
}

void CHouse2::OnKilled()
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
