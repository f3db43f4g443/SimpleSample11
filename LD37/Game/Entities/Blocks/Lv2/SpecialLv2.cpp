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
#include "Common/MathUtil.h"
#include "Entities/SpecialEft.h"

void CLv2RandomChunk1::OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel )
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
	texRect.height /= 3;
	int32 nSplit = Max<int32>( 1, pChunk->nWidth / 4 );
	vector<int32> vecLen;
	vecLen.resize( nSplit );
	if( nSplit == 1 )
		vecLen[0] = pChunk->nWidth;
	else
	{
		for( int i = 0; i < nSplit; i++ )
			vecLen[i] = 4;
		int32 s = pChunk->nWidth - nSplit * 4;
		for( int i = 0; i < s; i++ )
		{
			vecLen[SRand::Inst<eRand_Render>().Rand( 0, nSplit )]++;
		}
	}

	auto pRenderObject1 = new CRenderObject2D;
	SetRenderObject( pRenderObject1 );
	pRenderObject1->SetZOrder( 1 );

	int32 nX = 0;
	for( int i = 0; i < nSplit; i++ )
	{
		int32 nLen = vecLen[i];
		uint8 nType = nLen == 4 && i > 0 && i < nSplit - 1 && SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? 1 : 0;
		for( int i1 = 0; i1 < nLen; i1++ )
		{
			int32 iX = i1 + nX;
			for( int iY = 0; iY < pChunk->nHeight; iY++ )
			{
				CRectangle tex = texRect;
				if( iY == pChunk->nHeight - 1 || nType == 0 )
				{
					if( i1 == 0 )
						tex.x += tex.width * ( iX == 0 ? 0 : 2 );
					else if( i1 == nLen - 1 )
						tex.x += tex.width * ( iX == pChunk->nWidth - 1 ? 3 : 1 );
					else
						tex.x += tex.width * 0.5f;
				}
				else
					tex.x += tex.width * ( 4 + i1 );

				if( iY != pChunk->nHeight - 1 )
				{
					if( iY == 0 )
						tex.y += tex.height * 2;
					else if( iY == pChunk->nHeight - 2 )
						tex.y += tex.height * ( nType == 0 ? 1 : 0 );
					else
						tex.y += tex.height * ( nType == 0 ? 1.5f : 1 );
				}

				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
				pImage2D->SetTexRect( tex );
				pRenderObject1->AddChild( pImage2D );

				for( int i = 0; i < m_nDamagedEffectsCount; i++ )
				{
					CImage2D* pImage2D = static_cast<CImage2D*>( pDamageEftDrawableGroups[i]->CreateInstance() );
					pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
					pImage2D->SetTexRect( pDamageEftTex[i] );
					m_pDamagedEffects[i]->AddChild( pImage2D );
				}
			}
		}
		nX += nLen;
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CLv2RandomChunk1::OnCreateComplete( CMyLevel * pLevel )
{
	for( int i = 0; i < m_nDamagedEffectsCount; i++ )
	{
		m_pDamagedEffects[i]->SetRenderParentAfter( GetChunk()->blocks[0].pEntity );
	}
}

void CGarbageBin1::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
{
	float fHue = SRand::Inst<eRand_Render>().Rand( 0.5f, 2.0f );
	CVector3 mainColor;
	for( int i = 0; i < 3; i++ )
		*( &mainColor.x + i ) = Max( 0.0f, 1 - Min( abs( fHue - 3 - i ), abs( fHue - i ) ) );
	auto pParam = static_cast<CImage2D*>( GetRenderObject() )->GetParam();
	CVector3 subColor( mainColor.y, mainColor.z, mainColor.x ), subColor1( mainColor.z, mainColor.x, mainColor.y );
	pParam[0].w = subColor1.x;
	pParam[1] = CVector4( mainColor.x, mainColor.y, mainColor.z, subColor1.y );
	pParam[2] = CVector4( subColor.x, subColor.y, subColor.z, subColor1.z );
	CChunkObject::OnSetChunk( pChunk, pLevel );
}

void CGarbageBin1::OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed )
{
	if( m_nMaxTriggerCount && globalTransform.GetPosition().y <= m_fMaxTriggerHeight )
	{
		int32 n = nPreSpeed - nCurSpeed;
		if( n >= m_nTriggerImpact )
		{
			Trigger();
			m_nMaxTriggerCount--;
		}
	}
}

void CGarbageBin1::OnKilled()
{
	int32 nTrigger = Min( m_nMaxDeathTriggerCount, m_nMaxTriggerCount );
	for( int i = 0; i < nTrigger; i++ )
		Trigger();
	CChunkObject::OnKilled();
}

void CGarbageBin1::Trigger()
{
	for( int i = 0; i < GetChunk()->nWidth; i++ )
	{
		CVector2 ofs( ( i + 0.5f ) * CMyLevel::GetBlockSize(), m_h );

		auto p = SafeCast<CCharacter>( m_pBullet->GetRoot()->CreateInstance() );
		CReference<CPrefab> pPrefab = m_pBullet1;
		CFunctionTrigger* pTrigger = new CFunctionTrigger;
		pTrigger->bAutoDelete = true;
		pTrigger->Set( [p, pPrefab] () {
			SBarrageContext context;
			context.vecBulletTypes.push_back( pPrefab );
			context.nBulletPageSize = 4 * 3;

			CBarrage* pBarrage = new CBarrage( context );
			CVector2 pos = p->globalTransform.GetPosition();
			pBarrage->AddFunc( [pos] ( CBarrage* pBarrage )
			{
				float fAngle0 = SRand::Inst().Rand( -PI, PI );
				for( int i = 0; i < 4; i++ )
				{
					for( int j = 0; j < 3; j++ )
					{
						float fAngle1 = fAngle0 + j * ( PI * 2 / 3 ) + SRand::Inst().Rand( -0.06f, 0.06f );
						float fSpeed = 225 - i * 3 + SRand::Inst().Rand( -20.0f, 20.0f );
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
		p->RegisterEntityEvent( eEntityEvent_RemovedFromStage, pTrigger );

		p->SetPosition( ofs + globalTransform.GetPosition() );
		p->SetVelocity( CVector2( SRand::Inst().Rand( -350.0f, 350.0f ), SRand::Inst().Rand( 150.0f, 200.0f ) ) );
		p->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}
}

bool CBarrel::Damage( SDamageContext& context )
{
	if( m_bKilled )
	{
		DEFINE_TEMP_REF_THIS();
		CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetBlockSize() * 0.5f;
		int32 nPreHp = -m_fHp;
		if( !CExplosiveChunk::Damage( context ) )
			return false;
		if( !GetParentEntity() )
			return true;
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
		return true;
	}
	else
		return CExplosiveChunk::Damage( context );
}

void CBarrel::Explode()
{
	CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetBlockSize() * 0.5f;
	ForceUpdateTransform();
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

bool CBarrel1::Damage( SDamageContext& context )
{
	if( m_bKilled )
	{
		DEFINE_TEMP_REF_THIS();
		CVector2 center = CVector2( m_pChunk->nWidth, m_pChunk->nHeight ) * CMyLevel::GetBlockSize() * 0.5f;
		int32 nPreHp = -m_fHp;
		if( !CExplosiveChunk::Damage( context ) )
			return false;
		if( !GetParentEntity() )
			return true;
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
		return true;
	}
	else
		return CExplosiveChunk::Damage( context );
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
	ForceUpdateTransform();
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
	texRect.width /= 4;
	texRect.height /= 4;

	auto& rnd = SRand::Inst<eRand_Render>();
	auto pParentChunk = SafeCast<CHouse>( GetParentEntity() );
	if( pParentChunk )
	{
		memcpy( m_params, pParentChunk->m_params, sizeof( m_params ) );
		memcpy( m_nY, pParentChunk->m_nY, sizeof( m_nY ) );
	}
	else
	{
		if( m_nType == 0 )
		{
			m_nY[0] = rnd.Rand( 0, 4 );
			m_nY[1] = rnd.Rand( m_nY[0], 4 );
			m_nY[2] = rnd.Rand( m_nY[1], 4 );
		}
		else
		{
			m_nY[0] = 0;
			m_nY[1] = rnd.Rand( 0, 4 );
			m_nY[2] = rnd.Rand( 0, 4 );
		}

		float fHue = rnd.Rand( 0.0f, 3.0f );
		CVector3 mainColor[3];
		for( int i = 0; i < 3; i++ )
			*( &mainColor[0].x + i ) = Max( 0.0f, 1 - Min( abs( fHue - 3 - i ), abs( fHue - i ) ) );
		mainColor[0].Normalize();

		mainColor[1] = mainColor[0];
		{
			uint8 n = m_nType == 1 || m_nY[0] == m_nY[1] ? rnd.Rand( 0, 2 ) : 0;
			if( n == 0 )
			{
				float f = 0.7f;
				mainColor[0] = mainColor[0] + ( CVector3( f, f, f ) - mainColor[0] ) * rnd.Rand( 0.7f, 0.75f );
			}
			else
			{
				CMatrix mat;
				float f = sqrt( 1.0f / 3 );
				float f1 = rnd.Rand( PI * 0.5f, PI * 1.5f );
				mat.Rotate( f, f, f, f1 );
				mainColor[1] = mat.MulVector3Dir( mainColor[1] );
				f = ( mainColor[1].x + mainColor[1].y + mainColor[1].z ) / 3;
				f1 = abs( f1 / PI - 1 ) * 1.5f;
				mainColor[1] = mainColor[1] + ( CVector3( f, f, f ) - mainColor[1] ) * f1;
			}
		}
		if( m_nType == 0 )
			mainColor[1] = mainColor[1] * CVector3( rnd.Rand( 0.95f, 1.05f ), rnd.Rand( 0.95f, 1.05f ), rnd.Rand( 0.95f, 1.05f ) );

		mainColor[2] = mainColor[1];
		if( m_nY[1] == m_nY[2] )
		{
			mainColor[2] = mainColor[2] * rnd.Rand( 0.6f, 0.7f );
		}
		mainColor[2] = mainColor[2] * CVector3( rnd.Rand( 0.95f, 1.05f ), rnd.Rand( 0.95f, 1.05f ), rnd.Rand( 0.95f, 1.05f ) );

		bool bRot = rnd.Rand( 0, 2 );
		for( int i = 0; i < 3; i++ )
		{
			m_params[i][0] = static_cast<CImage2D*>( GetRenderObject() )->GetParam()[0];
			CVector3 subColor( mainColor[i].y, mainColor[i].z, mainColor[i].x ), subColor1( mainColor[i].z, mainColor[i].x, mainColor[i].y );
			if( bRot )
				swap( subColor, subColor1 );
			m_params[i][0].w = subColor1.x;
			m_params[i][1] = CVector4( mainColor[i].x, mainColor[i].y, mainColor[i].z, subColor1.y );
			m_params[i][2] = CVector4( subColor.x, subColor.y, subColor.z, subColor1.z );
		}
	}
	m_params[1][0].z = m_fHeightOfs;
	m_params[2][0].z = m_fHeightOfs1;

	SetRenderObject( new CRenderObject2D );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			uint8 nTag = pChunk->GetBlock( iX, iY )->nTag;
			uint8 n = nTag >= m_nTag2 ? 2 : ( nTag >= m_nTag1 ? 1 : 0 );

			CRectangle tex = texRect;
			tex.x += tex.width * rnd.Rand( 0, 4 );
			tex.y += tex.height * m_nY[n];
			uint8 nTag1 = iY < pChunk->nHeight - 1 ? pChunk->GetBlock( iX, iY + 1 )->nTag : m_nTag2;
			if( !m_nType && nTag1 > nTag )
				tex.x += tex.width * 4;

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
			pImage2D->SetTexRect( tex );
			auto pParam = pImage2D->GetParam();
			for( int i = 0; i < 3; i++ )
				pParam[i] = m_params[n][i];
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
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			uint8 nTag = pChunk->GetBlock( iX, iY )->nTag;
			uint8 n = nTag >= m_nTag2 ? 2 : ( nTag >= m_nTag1 ? 1 : 0 );
			pChunk->GetBlock( iX, iY )->nTag += m_nY[n];
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
	for( auto& pWindow : m_windows )
		pWindow->SetState( 1 );
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
	}
	for( auto pChild = m_p1->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto pWindow = SafeCast<CHouseWindow>( pChild );
		if( pWindow )
		{
			m_windows.push_back( pWindow );
			continue;
		}
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
	for( auto& pWindow : m_windows )
		pWindow->SetState( 2 );
	m_nCount = 0;
	m_bExploding = true;
	GetStage()->RegisterAfterHitTest( 10, &m_onTick );
}

void CHouse::Explode()
{
	for( CHouseEntrance* pEntrance : m_houseEntrances )
	{
		if( pEntrance->GetDir() < 4 )
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
		pEntrance->SetState( 3 );
	}
	for( CHouseWindow* pWindow : m_windows )
	{
		pWindow->SetState( 3 );
	}
	GetStage()->RegisterAfterHitTest( 10, &m_onTick );
	m_bExploded = true;
}

CCharacter* CHouse::GetOneThrowObj()
{
	if( !m_throwObjs.size() )
		return NULL;
	int32 nPrefab = m_throwObjs.back();
	m_throwObjs.pop_back();
	auto pThrowObj = SafeCast<CCharacter>( m_pThrowObjPrefabs[nPrefab]->GetRoot()->CreateInstance() );
	return pThrowObj;
}

void CHouse::OnKilled()
{
	for( CHouseEntrance* pEntrance : m_houseEntrances )
	{
		if( pEntrance->GetDir() >= 4 )
		{
			auto pExp = SafeCast<CExplosionWithBlockBuff>( m_pExp1->GetRoot()->CreateInstance() );
			pExp->SetPosition( pEntrance->globalTransform.GetPosition() );
			pExp->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			CBlockBuff::SContext context;
			context.nLife = 600;
			context.nTotalLife = 600;
			context.fParams[0] = 3;
			pExp->Set( &context );
			pExp->SetCreator( this );
		}
	}
	CChunkObject::OnKilled();
}

void CHouse::OnTick()
{
	if( m_bExploded )
	{
		for( CHouseEntrance* pEntrance : m_houseEntrances )
		{
			if( pEntrance->GetDir() < 4 )
			{
				auto pEffect = SafeCast<CEffectObject>( m_pExpEft->GetRoot()->CreateInstance() );
				pEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
				pEffect->SetRotation( pEntrance->GetDir() * PI * 0.5f );
				pEffect->SetPosition( pEntrance->globalTransform.GetPosition() + CVector2( cos( pEffect->r ), sin( pEffect->r ) ) * 16 );
				pEffect->SetState( 2 );
			}
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
				SRand::Inst().Rand<float>( 0.0f, GetChunk()->nHeight * CMyLevel::GetBlockSize() ) ) );
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
	if( GetHp() < GetMaxHp() * 0.4f && !m_bAnyoneEntered )
	{
		m_bAnyoneEntered = true;
		for( auto& pEntrance : m_houseEntrances )
			pEntrance->SetState( 1 );
		for( CHouseWindow* pWindow : m_windows )
			pWindow->SetState( 1 );
	}
	if( nChar >= 0 && m_bAnyoneEntered )
	{
		auto pCharacter = m_characters[nChar].first;
		auto pThug = SafeCast<CThug>( pCharacter.GetPtr() );

		int32 k = pThug && !SRand::Inst().Rand( 0, 2 ) ? 1 : 0;
		for( ; k >= 0; k-- )
		{
			for( int i = m_houseEntrances.size(); i > 0; i-- )
			{
				int32 r = SRand::Inst().Rand( 0, i );
				uint8 nEntranceType = m_houseEntrances[r]->GetDir() == 4 ? 1 : 0;
				if( nEntranceType != k )
				{
					if( r != i - 1 )
						swap( m_houseEntrances[r], m_houseEntrances.back() );
					continue;
				}

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
						pThug->SetThrowObj( pThrowObj, CVector2( 0, -pThrowObj->GetRenderObject()->GetLocalBound().y ), k == 1 );
					}
					else
					{
						CVector2 dirs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
						if( m_houseEntrances[r]->GetDir() < 4 )
						{
							CVector2 ofs = dirs[m_houseEntrances[r]->GetDir()];
							ofs = ofs + CVector2( ofs.y, -ofs.x ) * SRand::Inst().Rand( -0.5f, 0.5f );
							ofs.Normalize();
							pCharacter->Knockback( ofs * SRand::Inst().Rand( 0.5f, 1.5f ) );
						}
					}

					m_characters[nChar] = m_characters.back();
					m_characters.pop_back();
					return;
				}
				else
				{
					if( r != i - 1 )
						swap( m_houseEntrances[r], m_houseEntrances.back() );
				}
			}
		}
	}
}

void CCargo1::OnAddedToStage()
{
	CChunkObject::OnAddedToStage();
	if( !CMyLevel::GetInst() )
		return;
	if( m_bType )
		GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}

void CCargo1::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CChunkObject::OnRemovedFromStage();
}

void CCargo1::OnSetChunk( SChunk* pChunk, CMyLevel* pLevel )
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
	auto tex1 = m_texRect1;
	auto& rnd = SRand::Inst<eRand_Render>();
	if( rnd.Rand( 0, 2 ) )
	{
		texRect.y += texRect.height;
		tex1.y += tex1.height;
	}
	texRect.width /= 4;
	texRect.height /= 4;
	SetRenderObject( new CRenderObject2D );
	tex1.width /= 4;
	auto pRenderObject1 = new CRenderObject2D;

	int32 wMin, wMax, hMin, hMax;
	bool bVertical = pChunk->nHeight > pChunk->nWidth && !m_bType;
	if( pChunk->nWidth <= rnd.Rand( 2, 4 ) || pChunk->nHeight <= rnd.Rand( 2, 4 ) )
	{
		wMin = wMax = pChunk->nWidth;
		hMin = hMax = pChunk->nHeight;
	}
	else
	{
		if( !bVertical )
		{
			wMin = rnd.Rand( 3, 7 );
			if( wMin > pChunk->nWidth / 2 )
				wMin = wMax = pChunk->nWidth;
			else
				wMax = Min( pChunk->nWidth / 2, pChunk->nWidth % wMin + wMin );
			hMin = hMax = pChunk->nHeight;
		}
		else
		{
			hMin = rnd.Rand( 3, 7 );
			if( hMin > pChunk->nHeight / 2 )
				hMin = hMax = pChunk->nHeight;
			else
				hMax = Min( pChunk->nHeight / 2, pChunk->nHeight % hMin + hMin );
			wMin = wMax = pChunk->nWidth;
		}
	}
	int32 nType1 = rnd.Rand( 0, 8 );
	int32 nType2 = rnd.Rand( 0, 16 );
	if( bVertical )
	{
		nType1 ^= 8;
		nType2 ^= 8;
	}

	uint8 bTag = 0;
	for( int iY = 0; iY < pChunk->nHeight && !bTag; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth && !bTag; iX++ )
		{
			if( GetBlock( iX, iY )->nTag )
				bTag = 1;
		}
	}

	function<void( const TRectangle<int32>& rect, uint8 nEdge, uint8 nEdge1, uint8 t )> Func;
	Func = [=, &Func, &pDamageEftDrawableGroups, &pDamageEftTex] ( const TRectangle<int32>& rect, uint8 nEdge, uint8 nEdge1, uint8 t )
	{
		auto& rnd = SRand::Inst<eRand_Render>();
		int8 k1 = rect.width + rnd.Rand( 0, 2 ) > rect.height ? 0 : 1;
		for( int k = 0; k < 2; k++ )
		{
			if( !!( k ^ k1 ) )
			{
				int32 w;
				if( t )
				{
					int32 l[2] = { 0 };
					bool b0;
					for( int k2 = 0; k2 < 2; k2++ )
					{
						b0 = false;
						for( ; l[k2] < rect.width; l[k2]++ )
						{
							int32 x = k2 ? rect.GetRight() - 1 - l[k2] : l[k2] + rect.x;
							bool b = false;
							for( int y = rect.y; y < rect.GetBottom(); y++ )
							{
								if( GetBlock( x, y )->nTag )
								{
									b = true;
									break;
								}
							}
							if( b )
								b0 = true;
							else if( b0 )
								break;
						}
					}
					if( !b0 )
						t = false;
					else
					{
						if( abs( l[0] - rect.width / 2 ) + rnd.Rand( 0, 2 ) > abs( l[1] - rect.width / 2 ) )
							w = rect.width - l[1];
						else
							w = l[0];
						if( w < 3 || rect.width - w < 3 )
							continue;
					}
				}
				if( !t )
				{
					if( rect.width <= wMax || !!( nEdge1 & 1 ) || !!( nEdge1 & 4 ) )
						continue;
					w = rnd.Rand( wMin, Max( wMin, Min( rect.width / 2, wMax ) ) + 1 );
					if( w >= rect.width - 1 )
						continue;
				}
				int32 a = !!( nEdge & 1 );
				int32 b = !!( nEdge & 4 );
				if( a + rnd.Rand( 0, 2 ) > b )
				{
					Func( TRectangle<int32>( rect.x, rect.y, w, rect.height ), nEdge & ~4 | ( !!( nEdge & 1 ) * 4 ), nEdge1 | 1, t );
					Func( TRectangle<int32>( rect.x + w, rect.y, rect.width - w, rect.height ), nEdge ^ 1, nEdge1, t );
				}
				else
				{
					if( !t )
						w = rect.width - w;
					Func( TRectangle<int32>( rect.x, rect.y, w, rect.height ), nEdge ^ 4, nEdge1, t );
					Func( TRectangle<int32>( rect.x + w, rect.y, rect.width - w, rect.height ), nEdge & ~1 | ( !!( nEdge & 4 ) * 1 ), nEdge1 | 4, t );
				}
			}
			else
			{
				int32 h;
				if( t )
				{
					int32 l[2] = { 0 };
					bool b0;
					for( int k2 = 0; k2 < 2; k2++ )
					{
						b0 = false;
						for( ; l[k2] < rect.height; l[k2]++ )
						{
							int32 y = k2 ? rect.GetBottom() - 1 - l[k2] : l[k2] + rect.y;
							bool b = false;
							for( int x = rect.x; x < rect.GetRight(); x++ )
							{
								if( GetBlock( x, y )->nTag )
								{
									b = true;
									break;
								}
							}
							if( b )
								b0 = true;
							else if( b0 )
								break;
						}
					}
					if( !b0 )
						t = false;
					else
					{
						if( abs( l[0] - rect.height / 2 ) + rnd.Rand( 0, 2 ) > abs( l[1] - rect.height / 2 ) )
							h = rect.height - l[1];
						else
							h = l[0];
						if( h < 3 || rect.height - h < 3 )
							continue;
					}
				}
				if( !t )
				{
					if( rect.height <= hMax || !!( nEdge1 & 2 ) || !!( nEdge1 & 8 ) )
						continue;
					h = rnd.Rand( hMin, Max( hMin, Min( rect.height / 2, hMax ) ) + 1 );
					if( h >= rect.height - 1 )
						continue;
				}
				int32 a = !!( nEdge & 2 );
				int32 b = !!( nEdge & 8 );
				if( a + rnd.Rand( 0, 2 ) > b )
				{
					Func( TRectangle<int32>( rect.x, rect.y, rect.width, h ), nEdge & ~8 | ( !!( nEdge & 2 ) * 8 ), nEdge1 | 2, t );
					Func( TRectangle<int32>( rect.x, rect.y + h, rect.width, rect.height - h ), nEdge ^ 2, nEdge1, t );
				}
				else
				{
					if( !t )
						h = rect.height - h;
					Func( TRectangle<int32>( rect.x, rect.y, rect.width, h ), nEdge ^ 8, nEdge1, t );
					Func( TRectangle<int32>( rect.x, rect.y + h, rect.width, rect.height - h ), nEdge & ~2 | ( !!( nEdge & 8 ) * 2 ), nEdge1 | 8, t );
				}
			}
			return;
		}

		for( int iY = rect.y; iY < rect.GetBottom(); iY++ )
		{
			for( int iX = rect.x; iX < rect.GetRight(); iX++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
				pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );

				CRectangle tex;
				if( m_bType && iY == pChunk->nHeight - 1 )
				{
					float tX = iX == 0 ? 0 : ( iX == pChunk->nWidth - 1 ? 3 : rnd.Rand( 2, 5 ) * 0.5f );
					tex = tex1.Offset( CVector2( tex1.width * tX, 0 ) );
				}
				else
				{
					float tX = !!( nEdge & 1 ) && iX == rect.x ? 0 : ( !!( nEdge & 4 ) && iX == rect.GetRight() - 1 ? 3 : rnd.Rand( 1, 6 ) * 0.5f );
					float tY = !!( nEdge & 2 ) && iY == rect.y ? 0 : ( !!( nEdge & 8 ) && iY == rect.GetBottom() - 1 ? 3 : rnd.Rand( 1, 6 ) * 0.5f );
					tex = texRect.Offset( CVector2( texRect.width * tX, texRect.height * ( 3 - tY ) ) );
				}
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

		if( t && !m_bType )
			return;
		int32 nType = nEdge == 5 || nEdge == 10 ? nType2 : nType1;
		if( nType < 0 )
			return;
		bool bRot = nType & 8;
		nType = nType & 7;
		CRectangle texRect1( ( nType & 3 ) * 0.25f, ( nType >> 2 ) * 0.5f, 0.125f, 0.125f );
		for( int iY = rect.y; iY < rect.GetBottom(); iY++ )
		{
			if( m_bType && iY == pChunk->nHeight - 1 )
				break;
			for( int iX = rect.x; iX < rect.GetRight(); iX++ )
			{
				CImage2D* pImage2D = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
				pImage2D->SetRect( CRectangle( -16, -16, 32, 32 ) );
				pImage2D->SetPosition( CVector2( iX * 32 + 16, iY * 32 + 16 ) );
				if( bRot )
					pImage2D->SetRotation( -PI * 0.5f );

				float tX = rnd.Rand( 0, 2 );
				float tY;
				if( bRot )
					tY = iX == rect.x ? 3 : ( iX == rect.GetRight() - 1 ? 0 : rnd.Rand( 1, 3 ) );
				else
					tY = iY == rect.y ? 3 : ( iY == rect.GetBottom() - 1 ? 0 : rnd.Rand( 1, 3 ) );

				auto tex = texRect1.Offset( CVector2( texRect1.width * tX, texRect1.height * tY ) );
				pImage2D->SetTexRect( tex );
				pRenderObject1->AddChild( pImage2D );
			}
		}
	};
	Func( TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ), 0xf, 0, bTag );
	GetRenderObject()->AddChild( pRenderObject1 );
	if( bTag )
	{
		swap( SRand::Inst().nSeed, SRand::Inst<eRand_Render>().nSeed );
		if( m_bType )
			GenLayer2();
		else
			GenLayer1();
		swap( SRand::Inst().nSeed, SRand::Inst<eRand_Render>().nSeed );
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CCargo1::OnKilled()
{
	if( m_pBlob )
	{
		m_pBlob->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
		m_pBlob->SetPosition( m_pBlob->globalTransform.GetPosition() );
		uint8* p = (uint8*)alloca( m_nMaxIndex );
		for( int i = 0; i < m_nMaxIndex; i++ )
			p[i] = Min<int16>( 255, m_blobMap[i].n );
		SafeCast<CManBlobEft>( m_pBlob.GetPtr() )->Kill( m_nMaxIndex, p );
	}
	CChunkObject::OnKilled();
}

void CCargo1::GenLayer1()
{
	enum
	{
		eConn = 1,
		eButton = 2,
		eOperateable_Begin = 3,
		eOperateable_End = 5,
		eAssembler = 5,
	};

	int32 nWidth = m_pChunk->nWidth;
	int32 nHeight = m_pChunk->nHeight;
	vector<int8> vecTemp;
	vecTemp.resize( ( nWidth + 1 ) * ( nHeight + 1 ) );
	for( int i = 0; i <= nWidth; i++ )
	{
		for( int j = 0; j <= nHeight; j++ )
		{
			int8 nType = 0;
			if( i == 0 || i == nWidth || j == 0 || j == nHeight )
				nType = -1;
			else
			{
				for( int x = i - 1; x < i + 1; x++ )
				{
					for( int y = j - 1; y < j + 1; y++ )
					{
						int8 nTag = m_pChunk->GetBlock( x, y )->nTag;
						if( nTag != eConn )
						{
							nType = -1;
							break;
						}
					}
					if( nType == -1 )
						break;
				}
			}
			vecTemp[i + j * ( nWidth + 1 )] = nType;
		}
	}

	for( int j = 0; j < nHeight; j += nHeight - 1 )
	{
		for( int i = 1; i < nWidth - 2; i++ )
		{
			int8 a = m_pChunk->GetBlock( i, j )->nTag;
			int8 b = m_pChunk->GetBlock( i + 1, j )->nTag;
			if( a == b && a >= eButton && a < eOperateable_End )
			{
				vecTemp[i + 1 + ( j == 0 ? 0 : nHeight ) * ( nWidth + 1 )] = eConn;
				vecTemp[i + 1 + ( j == 0 ? 1 : nHeight - 1 ) * ( nWidth + 1 )] = eConn;
				i++;
			}
		}
	}
	for( int i = 0; i < nWidth; i += nWidth - 1 )
	{
		for( int j = 1; j < nHeight - 2; j++ )
		{
			int8 a = m_pChunk->GetBlock( i, j )->nTag;
			int8 b = m_pChunk->GetBlock( i, j + 1 )->nTag;
			if( a == b && a >= eButton && a < eOperateable_End )
			{
				vecTemp[( i == 0 ? 0 : nWidth ) + ( j + 1 ) * ( nWidth + 1 )] = eConn;
				vecTemp[( i == 0 ? 1 : nWidth - 1 ) + ( j + 1 ) * ( nWidth + 1 )] = eConn;
				j++;
			}
		}
	}
	for( int i = 0; i < nWidth - 1; i++ )
	{
		for( int j = 0; j < nHeight - 1; j++ )
		{
			if( m_pChunk->GetBlock( i, j )->nTag == eAssembler )
			{
				if( m_pChunk->GetBlock( i + 1, j )->nTag == eAssembler
					&& m_pChunk->GetBlock( i, j + 1 )->nTag == eAssembler && m_pChunk->GetBlock( i + 1, j + 1 )->nTag == eAssembler )
				{
					vecTemp[i + ( j + 1 ) * ( nWidth + 1 )] = vecTemp[i + 2 + ( j + 1 ) * ( nWidth + 1 )]
						= vecTemp[i + 1 + j * ( nWidth + 1 )] = vecTemp[i + 1 + ( j + 2 ) * ( nWidth + 1 )] = 0;
					vecTemp[i + 1 + ( j + 1 ) * ( nWidth + 1 )] = eConn;
					m_pChunk->GetBlock( i + 1, j )->nTag = m_pChunk->GetBlock( i, j + 1 )->nTag = m_pChunk->GetBlock( i + 1, j + 1 )->nTag = -1;
				}
				else
					m_pChunk->GetBlock( i, j )->nTag = -1;
			}
		}
	}
	ConnectAll( vecTemp, nWidth + 1, nHeight + 1, eConn, 0 );
	auto nBlockSize = CMyLevel::GetBlockSize();
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( m_pChunk->GetBlock( i, j )->nTag >= eOperateable_Begin )
				continue;
			TVector2<int32> v[4] = { { i, j }, { i + 1, j }, { i, j + 1 }, { i + 1, j + 1 } };
			int8 n = 0;
			for( int k = 0; k < 4; k++ )
				n |= ( vecTemp[v[k].x + v[k].y * ( nWidth + 1 )] == 1 ) << k;
			if( !n )
				continue;
			auto pImage = static_cast<CImage2D*>( m_pDeco1->CreateInstance() );
			pImage->SetRect( CRectangle( i, j, 1, 1 ) * nBlockSize );
			pImage->SetTexRect( CRectangle( n % 4, n / 4, 1, 1 ) * 0.125f );
			GetRenderObject()->AddChild( pImage );
		}
	}

	vector<CReference<CEntity> > vecTemp1;
	vecTemp1.resize( vecTemp.size() );
	CVector2 operateableTex[] = { { 0, 0.5f }, { 0.5f, 0 } };
	for( int j = 0; j < nHeight; j += nHeight - 1 )
	{
		for( int i = 1; i < nWidth - 2; i++ )
		{
			int8 a = m_pChunk->GetBlock( i, j )->nTag;
			int8 b = m_pChunk->GetBlock( i + 1, j )->nTag;
			if( a == b )
			{
				if( a == eButton )
				{
					auto pImage = static_cast<CImage2D*>( m_pDeco1->CreateInstance() );
					pImage->SetRect( CRectangle( i + 0.5f, j, 1, 1 ) * nBlockSize );
					pImage->SetTexRect( CRectangle( 0, 0, 0.125f, 0.125f ) );
					GetRenderObject()->AddChild( pImage );
					auto p = SafeCast<CEntity>( m_pPrefab[0]->GetRoot()->CreateInstance() );
					p->SetPosition( CVector2( i + 1, j + 0.5f ) * nBlockSize );
					p->SetParentEntity( this );
					vecTemp1[i + 1 + ( j == 0 ? 0 : nHeight ) * ( nWidth + 1 )] = p;
				}
				else if( a >= eOperateable_Begin && a < eOperateable_End )
				{
					auto pImage = static_cast<CImage2D*>( m_pDeco1->CreateInstance() );
					pImage->SetRect( CRectangle( i, j, 2, 1 ) * nBlockSize );
					auto v = operateableTex[a - eOperateable_Begin];
					pImage->SetTexRect( CRectangle( 0.25f + v.x, ( j == 0 ? 0 : 0.125f ) + v.y, 0.25f, 0.125f ) );
					GetRenderObject()->AddChild( pImage );
					auto p = SafeCast<CEntity>( m_pPrefab[( j == 0 ? 4 : 2 ) + ( a - eOperateable_Begin ) * 4]->GetRoot()->CreateInstance() );
					p->SetPosition( CVector2( i + 1, j + 0.5f ) * nBlockSize );
					p->SetParentEntity( this );
					vecTemp1[i + 1 + ( j == 0 ? 0 : nHeight ) * ( nWidth + 1 )] = p;
				}
				i++;
			}
		}
	}
	for( int i = 0; i < nWidth; i += nWidth - 1 )
	{
		for( int j = 1; j < nHeight - 2; j++ )
		{
			int8 a = m_pChunk->GetBlock( i, j )->nTag;
			int8 b = m_pChunk->GetBlock( i, j + 1 )->nTag;
			if( a == b )
			{
				if( a == eButton )
				{
					auto pImage = static_cast<CImage2D*>( m_pDeco1->CreateInstance() );
					pImage->SetRect( CRectangle( i, j + 0.5f, 1, 1 ) * nBlockSize );
					pImage->SetTexRect( CRectangle( 0, 0, 0.125f, 0.125f ) );
					GetRenderObject()->AddChild( pImage );
					auto p = SafeCast<CEntity>( m_pPrefab[0]->GetRoot()->CreateInstance() );
					p->SetPosition( CVector2( i + 0.5f, j + 1 ) * nBlockSize );
					p->SetParentEntity( this );
					vecTemp1[( i == 0 ? 0 : nWidth ) + ( j + 1 ) * ( nWidth + 1 )] = p;
				}
				else if( a >= eOperateable_Begin && a < eOperateable_End )
				{
					auto pImage = static_cast<CImage2D*>( m_pDeco1->CreateInstance() );
					pImage->SetRect( CRectangle( i, j, 1, 2 ) * nBlockSize );
					auto v = operateableTex[a - eOperateable_Begin];
					pImage->SetTexRect( CRectangle( ( i == 0 ? 0.125f : 0.0f ) + v.x, v.y, 0.125f, 0.25f ) );
					GetRenderObject()->AddChild( pImage );
					auto p = SafeCast<CEntity>( m_pPrefab[( i == 0 ? 3 : 1 ) + ( a - eOperateable_Begin ) * 4]->GetRoot()->CreateInstance() );
					p->SetPosition( CVector2( i + 0.5f, j + 1 ) * nBlockSize );
					p->SetParentEntity( this );
					vecTemp1[( i == 0 ? 0 : nWidth ) + ( j + 1 ) * ( nWidth + 1 )] = p;
				}
				j++;
			}
		}
	}
	for( int i = 0; i < nWidth - 1; i++ )
	{
		for( int j = 0; j < nHeight - 1; j++ )
		{
			if( m_pChunk->GetBlock( i, j )->nTag == eAssembler )
			{
				auto pImage = static_cast<CImage2D*>( m_pDeco1->CreateInstance() );
				pImage->SetRect( CRectangle( i, j, 2, 2 ) * nBlockSize );
				pImage->SetTexRect( CRectangle( 0.75f, 0.25f, 0.25f, 0.25f ) );
				GetRenderObject()->AddChild( pImage );
				auto p = SafeCast<CEntity>( m_pPrefab[9]->GetRoot()->CreateInstance() );
				p->SetPosition( CVector2( i + 1, j + 1 ) * nBlockSize );
				p->SetParentEntity( this );
				vecTemp1[i + 1 + ( j + 1 ) * ( nWidth + 1 )] = p;
			}
		}
	}

	vector<TVector2<int32> > q;
	vector<CReference<CEntity> > vec;
	for( int i = 0; i <= nWidth; i++ )
	{
		for( int j = 0; j <= nHeight; j++ )
		{
			if( vecTemp[i + j * ( nWidth + 1 )] == eConn )
			{
				FloodFill( vecTemp, nWidth + 1, nHeight + 1, i, j, 0, q );
				for( auto& p : q )
				{
					auto pEntity = vecTemp1[p.x + p.y * ( nWidth + 1 )];
					if( pEntity )
						vec.push_back( pEntity );
				}
				for( CEntity* pEntity : vec )
				{
					auto pButton = SafeCast<COperateableButton>( pEntity );
					if( pButton )
					{
						for( CEntity* pEntity1 : vec )
						{
							if( SafeCast<COperateableButton>( pEntity1 ) )
								continue;
							pButton->AddOperateable( pEntity1 );
						}
					}
				}
				q.resize( 0 );
				vec.resize( 0 );
			}
		}
	}
}

void CCargo1::GenLayer2()
{
	int32 nWidth = m_pChunk->nWidth;
	int32 nHeight = m_pChunk->nHeight;
	vector<int8> vecTemp;
	vecTemp.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
			vecTemp[i + j * nWidth] = m_pChunk->GetBlock( i, j )->nTag;
	}
	TVector2<int32> minSize[3] = { { 2, 2 }, { 2, 2 }, { 2, 2 } };
	TVector2<int32> maxSize[3] = { { 8, 2 }, { 3, 4 }, { 4, 4 } };
	bool b = false;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			auto nType = vecTemp[i + j * nWidth];
			if( !nType )
				continue;
			nType--;
			auto rect = PutRect( vecTemp, nWidth, nHeight, TVector2<int32>( i, j ), minSize[nType], maxSize[nType], TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 0 );
			if( rect.width > 0 )
			{
				GenObj( rect, nType );
				if( nType == 0 )
				{
					int32 n1 = 20 + ( rect.width + 2 ) * 24 / ( rect.width * 2 - 2 );
					int32 y = rect.y + rect.GetBottom();
					for( int x = rect.x * 2 + 1; x <= rect.GetRight() * 2 - 1; x++ )
					{
						m_q.push_back( TVector2<int32>( x - nWidth, y - nHeight ) );
						auto& item = m_blobMap[GetMapItem( x - nWidth, y - nHeight )];
						item.n1 = n1;
						item.bFlag = 1;
					}
				}
				else if( nType == 1 )
				{
					int32 n1 = 20 + rect.height * 20 * 2 / ( rect.height * 2 - 1 );
					int32 x = rect.x + rect.GetRight();
					for( int y = rect.y * 2; y <= rect.GetBottom() * 2 - 1; y++ )
					{
						m_q.push_back( TVector2<int32>( x - nWidth, y - nHeight ) );
						auto& item = m_blobMap[GetMapItem( x - nWidth, y - nHeight )];
						item.n1 = n1;
						item.bFlag = 1;
					}
				}
				else
				{
					int32 n1 = 20 + rect.width * rect.height * 27 /
						( ( 1 + rect.width * 2 - 4 ) * ( 1 + rect.height * 2 - 4 ) );
					for( int x = rect.x * 2 + 2; x <= rect.GetRight() * 2 - 2; x++ )
					{
						for( int y = rect.y * 2 + 2; y <= rect.GetBottom() * 2 - 2; y++ )
						{
							m_q.push_back( TVector2<int32>( x - nWidth, y - nHeight ) );
							auto& item = m_blobMap[GetMapItem( x - nWidth, y - nHeight )];
							item.n1 = n1;
							item.bFlag = 1;
						}
					}
				}
			}
		}
	}

	if( !CMyLevel::GetInst() )
		return;
	m_pBlob = SafeCast<CEntity>( m_pPrefab[0]->GetRoot()->CreateInstance() );
	m_pBlob->SetPosition( CVector2( nWidth, nHeight ) * CMyLevel::GetBlockSize() * 0.5f );
	m_pBlob->SetParentEntity( this );
	m_pBlob->SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
}

void CCargo1::GenObj( const TRectangle<int32>& rect, uint8 nType )
{
	TRectangle<int32> texRects[3] = { { 0, 0, 5, 2 }, { 0, 2, 3, 4 }, { 3, 2, 4, 4 } };
	auto texRect = texRects[nType];
	for( int i = 0; i < rect.width; i++ )
	{
		for( int j = 0; j < rect.height; j++ )
		{
			CImage2D* pImg = static_cast<CImage2D*>( m_pDeco1->CreateInstance() );
			pImg->SetRect( CRectangle( i + rect.x, j + rect.y, 1, 1 ) * CMyLevel::GetBlockSize() );
			int32 tX = i == 0 ? texRect.x : ( i == rect.width - 1 ? texRect.GetRight() - 1 : SRand::Inst().Rand( texRect.x + 1, texRect.GetRight() - 1 ) );
			int32 tY = j == 0 ? texRect.GetBottom() - 1 : ( j == rect.height - 1 ? texRect.y : SRand::Inst().Rand( texRect.y + 1, texRect.GetBottom() - 1 ) );
			pImg->SetTexRect( CRectangle( tX, tY, 1, 1 ) * 0.125f );
			GetRenderObject()->AddChild( pImg );
		}
	}
}

void CCargo1::OnTick()
{
	if( !m_pBlob )
		return;
	GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
	CPlayer* pPlayer = GetStage()->GetPlayer();
	CVector2 center( m_pChunk->nWidth * CMyLevel::GetBlockSize() * 0.5f, m_pChunk->nHeight * CMyLevel::GetBlockSize() * 0.5f );
	center = center + globalTransform.GetPosition();
	CVector2 playerOfs = pPlayer ? pPlayer->GetPosition() - center : CVector2( 0, 0 );
	int32 qSize = m_q.size();
	SRand::Inst().Shuffle( m_q );
	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	int32 nMaxIndex = -1;
	for( int i = 0; i < m_q.size(); i++ )
	{
		auto p = m_q[i];
		int32 nIndex = GetMapItem( p.x, p.y );
		nMaxIndex = Max( nIndex, nMaxIndex );
		auto pItem = &m_blobMap[nIndex];
		if( i < qSize )
			pItem->n = Min<int16>( pItem->n + pItem->n1, 4096 );
		if( pItem->n < 255 )
			pItem->n = Max<int16>( pItem->n - 20, 0 );
		else
		{
			SRand::Inst().Shuffle( ofs, 4 );
			CVector2 playerOfs1 = playerOfs - CVector2( p.x * 16, p.y * 16 );
			float l2 = playerOfs1.Length2();
			bool b = false;
			if( l2 < 200 * 200 )
			{
				playerOfs1 = playerOfs1 / ( l2 * 0.00125f );
				for( int j = 0; j < 4; j++ )
				{
					float fWeight = 1 + playerOfs1.Dot( CVector2( ofs[j].x, ofs[j].y ) );
					fWeight = Max( 0.0f, Min( fWeight, 16.0f ) );
					auto p1 = p + ofs[j];
					auto pItem1 = &m_blobMap[GetMapItem( p1.x, p1.y )];
					pItem = &m_blobMap[nIndex];
					int32 d = Max( 0, Min( 30, ( (int32)pItem->n - pItem1->n ) >> 1 ) );
					d = Min<int32>( pItem->n - 255, fWeight * d );
					pItem1->n = Min( 4096, pItem1->n + Max( 0, d - 2 ) );
					pItem->n -= d;
					if( pItem1->n && !pItem1->bFlag )
					{
						pItem1->bFlag = 1;
						m_q.push_back( p1 );
						b = true;
					}
				}
			}
			else
			{
				for( int j = 0; j < 4; j++ )
				{
					auto p1 = p + ofs[j];
					auto pItem1 = &m_blobMap[GetMapItem( p1.x, p1.y )];
					pItem = &m_blobMap[nIndex];
					int32 d = Max( 0, Min( 30, ( (int32)pItem->n - pItem1->n ) >> 1 ) );
					d = Min( pItem->n - 255, d );
					pItem1->n = Min( 4096, pItem1->n + Max( 0, d - 2 ) );
					pItem->n -= d;
					if( pItem1->n && !pItem1->bFlag )
					{
						pItem1->bFlag = 1;
						m_q.push_back( p1 );
						b = true;
					}
				}
			}
			pItem->n = Max<int16>( pItem->n - 20, b ? 255 : 0 );
		}
	}
	nMaxIndex++;
	m_nMaxIndex = nMaxIndex;
	if( nMaxIndex > 0 )
	{
		uint8* p = (uint8*)alloca( nMaxIndex );
		for( int i = 0; i < nMaxIndex; i++ )
			p[i] = Min<int16>( 255, m_blobMap[i].n );
		SafeCast<CManBlobEft>( m_pBlob.GetPtr() )->Set( nMaxIndex, p );
	}
	if( pPlayer && pPlayer->CanKnockback() )
	{
		float x = playerOfs.x / 16;
		float y = playerOfs.y / 16;
		int32 nX = floor( x + 0.5f );
		int32 nY = floor( y + 0.5f );
		uint8 n = GetMapItemValue( nX, nY );
		if( n == 255 )
		{
			auto d = CVector2( nX * 16, nY * 16 ) - playerOfs;
			if( d.Normalize() < 0.0001f )
				d = CVector2( 0, -1 );
			pPlayer->Knockback( d );
			CCharacter::SDamageContext context;
			context.nDamage = 1;
			context.nType = 0;
			context.nSourceType = 0;
			context.hitPos = context.hitDir = CVector2( 0, 0 );
			context.nHitType = -1;
			pPlayer->Damage( context );
		}
		else if( n > 0 )
		{
			TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			int32 nValue[4];
			bool bHit = false;
			for( int i = 0; i < 4; i++ )
			{
				nValue[i] = GetMapItemValue( nX + ofs[i].x, nY + ofs[i].y );
				if( nValue[i] != 255 )
					continue;
				if( !bHit )
				{
					auto d = CVector2( nX + ofs[i].x * ( 255 - n ) / 255.0f, nY + ofs[i].y * ( 255 - n ) / 255.0f ) * 16 - playerOfs;
					if( Max( abs( d.x ), abs( d.y ) ) < 8 )
						bHit = true;
				}
			}
			if( bHit )
			{
				CVector2 dir( nValue[2] - nValue[0], nValue[3] - nValue[1] );
				if( dir.Normalize() < 0.0001f )
					dir = CVector2( 0, -1 );
				pPlayer->Knockback( dir );
				CCharacter::SDamageContext context;
				context.nDamage = 1;
				context.nType = 0;
				context.nSourceType = 0;
				context.hitPos = context.hitDir = CVector2( 0, 0 );
				context.nHitType = -1;
				pPlayer->Damage( context );
			}
		}
	}

	for( int i = qSize; i < m_q.size(); i++ )
	{
		m_blobMap[GetMapItem( m_q[i].x, m_q[i].y )].bFlag = 0;
	}
	m_q.resize( qSize );
}

int32 CCargo1::GetMapItem( int32 x, int32 y )
{
	uint32 n = ZCurveOrderSigned( x, y );
	if( n >= m_blobMap.size() )
		m_blobMap.resize( n + 1 );
	return n;
}

uint8 CCargo1::GetMapItemValue( int32 x, int32 y )
{
	uint32 n = ZCurveOrderSigned( x, y );
	if( n >= m_blobMap.size() )
		return 0;
	return Min<int16>( 255, m_blobMap[n].n );
}

void CCargoAutoColor::Init( const CVector2 & size, SChunk* pPreParent )
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
	CCargoAutoColor* p = NULL;
	if( pPreParent && pPreParent->pChunkObject )
	{
		auto pRoot = pPreParent->pChunkObject->GetDecoratorRoot();
		if( !pRoot )
			pRoot = pPreParent->pChunkObject;
		for( auto pChild = pRoot->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			p = SafeCast<CCargoAutoColor>( pChild );
			if( p )
				break;
		}
	}

	if( !p )
	{
		uint8 nType = SafeCast<CControlRoom>( pChunkObject ) != NULL;
		auto& rnd = SRand::Inst<eRand_Render>();
		float fHue = rnd.Rand( 0.0f, 1.0f );
		fHue = rnd.Rand( 0, 2 ) ? 1 - fHue * fHue * 0.5f : fHue * fHue * 0.5f;
		fHue = -0.15f + fHue * 2.15f;
		float f = fHue;
		if( fHue < 0 )
			fHue += 3.0f;
		for( int i = 0; i < 3; i++ )
			*( &m_colors[0].x + i ) = Max( 0.0f, 1 - Min( abs( fHue - 3 - i ), abs( fHue - i ) ) );
		m_colors[0].Normalize();
		m_colors[0] = m_colors[0] + CVector3( 0.1f, 0.1f, 0.1f ) * ( 2 - abs( fHue - 2 ) );
		float l1 = 1.0f / m_colors[0].Dot( CVector3( 0.3f, 0.5f, 0.2f ) );
		float fHue1 = fHue + rnd.Rand( 1.4f, 1.6f );
		fHue1 = fHue1 >= 3.0f ? fHue1 - 3.0f : fHue1;
		for( int i = 0; i < 3; i++ )
			*( &m_colors[2].x + i ) = Max( 0.0f, 1 - Min( abs( fHue1 - 3 - i ), abs( fHue1 - i ) ) );
		m_colors[2].Normalize();
		float l2 = 1.0f / m_colors[2].Dot( CVector3( 0.3f, 0.5f, 0.2f ) );
		if( nType || rnd.Rand( 0, 4 ) )
		{
			m_colors[1] = CVector3( 1, rnd.Rand( 0.8f, 1.0f ) * Min( 1.0f, abs( 1.0f - f ) ), 0 )
				* ( rnd.Rand( 0.13f, 0.15f ) + Min( 0.5f, Max( 0.0f, f - 0.7f ) ) * 0.15f );
			if( rnd.Rand( 0, 3 ) )
			{
				m_colors[0] = m_colors[0] + ( CVector3( 0.54f, 0.54f, 0.54f ) - CVector3( 0.07f, 0.07f, 0.07f ) * l1 - m_colors[0] ) * rnd.Rand( 0.5f, 0.65f );
				float k = rnd.Rand( 0.8f, 1.0f );
				m_colors[0] = m_colors[0] * k * ( 0.54f + l1 * 0.42f );
				m_colors[2] = m_colors[2] + ( CVector3( 0.8f, 0.8f, 0.8f ) - m_colors[2] ) * rnd.Rand( 0.65f, 0.75f );
				m_colors[2] = m_colors[2] * k * rnd.Rand( 1.2f, 1.3f ) * ( 0.45f + l2 * 0.4f );
			}
			else
			{
				m_colors[0] = m_colors[0] + ( CVector3( 0.1f, 0.1f, 0.1f ) + CVector3( 0.13f, 0.13f, 0.13f ) * l1 - m_colors[0] ) * rnd.Rand( 0.27f, 0.4f );
				float k = rnd.Rand( 0.6f, 0.7f );
				m_colors[0] = m_colors[0] * k * ( 0.57f + l1 * 0.19f );
				m_colors[2] = m_colors[2] + ( CVector3( 0.75f, 0.75f, 0.75f ) - m_colors[2] ) * rnd.Rand( 0.5f, 0.6f );
				m_colors[2] = m_colors[2] * rnd.Rand( 1.05f, 1.15f ) * ( 0.84f + l2 * 0.1f );
			}
		}
		else
		{
			swap( m_colors[0], m_colors[2] );
			m_colors[1] = CVector3( 1, rnd.Rand( 0.45f, 0.65f ), 0 ) * rnd.Rand( 0.3f, 0.34f )
				+ m_colors[2] * 0.02f;
			m_colors[0] = ( m_colors[0] + ( CVector3( 0.8f, 0.8f, 0.8f ) + CVector3( 0.1f, 0.1f, 0.1f ) * l2 - m_colors[0] ) * rnd.Rand( 0.85f, 0.95f ) )
				* rnd.Rand( 0.95f, 1.2f );
			m_colors[2] = m_colors[2] + ( CVector3( 0.5f, 0.5f, 0.5f ) + CVector3( 0.2f, 0.2f, 0.2f ) * l1 - m_colors[2] ) * rnd.Rand( 0.5f, 0.6f );
			m_colors[2] = m_colors[2] * rnd.Rand( 0.95f, 1.15f );
		}
	}
	else
		memcpy( m_colors, p->m_colors, sizeof( m_colors ) );

	if( GetRenderObject() )
	{
		auto pColorMat = static_cast<CImage2D*>( GetRenderObject() );
		pColorMat->SetRect( CRectangle( 0, 0, size.x, size.y ) );
		auto pParam = pColorMat->GetParam();
		pParam[0] = CVector4( m_colors[0].x, m_colors[1].x, m_colors[2].x, 0 );
		pParam[1] = CVector4( m_colors[0].y, m_colors[1].y, m_colors[2].y, 0 );
		pParam[2] = CVector4( m_colors[0].z, m_colors[1].z, m_colors[2].z, 0 );
	}
	else
	{
		auto pRoot = pChunkObject->GetRenderObject();
		if( !pRoot )
			return;
		auto pImage = SafeCast<CImage2D>( pRoot );
		if( pImage )
		{
			uint16 nParamCount;
			auto pParams = pImage->GetParam( nParamCount );
			if( nParamCount == 3 )
			{
				pParams[0].w = m_colors[0].z;
				pParams[1] = CVector4( m_colors[0].x, m_colors[1].x, m_colors[2].x, m_colors[1].z );
				pParams[2] = CVector4( m_colors[0].y, m_colors[1].y, m_colors[2].y, m_colors[2].z );
			}
		}
		for( auto pChild = pRoot->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			auto pImage = SafeCast<CImage2D>( pChild );
			if( !pImage )
				continue;
			uint16 nParamCount;
			auto pParams = pImage->GetParam( nParamCount );
			if( nParamCount != 3 )
				continue;
			pParams[0].w = m_colors[0].z;
			pParams[1] = CVector4( m_colors[0].x, m_colors[1].x, m_colors[2].x, m_colors[1].z );
			pParams[2] = CVector4( m_colors[0].y, m_colors[1].y, m_colors[2].y, m_colors[2].z );
		}
	}
}

void CRoadSign::Init( const CVector2& size, SChunk* pPreParent )
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
	auto pChunk = pChunkObject->GetChunk();
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	SetRenderObject( new CRenderObject2D );
	if( pChunk->nWidth <= 1 || pChunk->nHeight <= 1 )
		return;

	int8 nDir;
	if( pChunk->nWidth < pChunk->nHeight )
		nDir = 0;
	else
	{
		if( CMyLevel::GetInst() )
		{
			int32 x = pChunk->pos.x / CMyLevel::GetBlockSize();
			if( x * 2 + pChunk->nWidth + SRand::Inst<eRand_Render>().Rand( 0, 1 ) > CMyLevel::GetInst()->GetWidth() )
				nDir = 1;
			else
				nDir = 2;
		}
		else
			nDir = SRand::Inst<eRand_Render>().Rand( 1, 3 );
	}
	int32 nWidth = pChunk->nWidth;
	int32 nHeight = pChunk->nHeight;
	if( nDir )
	{
		swap( nWidth, nHeight );
		if( nDir == 1 )
		{
			GetRenderObject()->SetRotation( -PI * 0.5f );
			GetRenderObject()->y = pChunk->nHeight * CMyLevel::GetBlockSize();
		}
		else
		{
			GetRenderObject()->SetRotation( PI * 0.5f );
			GetRenderObject()->x = pChunk->nWidth * CMyLevel::GetBlockSize();
		}
	}

	int32 xSplit = nWidth / 2;
	int32 ySplit = SRand::Inst<eRand_Render>().Rand( nHeight / 10, nHeight / 6 + 1 );
	vector<int32> vecSplitX, vecSplitY;
	for( int i = 0; i < xSplit; i++ )
		vecSplitX.push_back( ( nWidth + i ) / xSplit );
	SRand::Inst<eRand_Render>().Shuffle( vecSplitX );
	if( ySplit )
	{
		vecSplitY.push_back( SRand::Inst<eRand_Render>().Rand<int32>( 1, ( nHeight + ySplit - 1 ) / ySplit + 1 ) );
		for( int i = 0; i < ySplit - 1; i++ )
			vecSplitY.push_back( ( nHeight + i ) / ySplit );
		if( ySplit > 1 )
			SRand::Inst<eRand_Render>().Shuffle( &vecSplitY[1], vecSplitY.size() - 1 );
	}

	int32 nYellowLine = -1;
	if( nDir == 0 && vecSplitX.size() > 1 && CMyLevel::GetInst() )
	{
		int32 x = pChunk->pos.x / CMyLevel::GetBlockSize();
		int32 nCenter = CMyLevel::GetInst()->GetWidth() / 2;
		if( x < nCenter && x + nWidth > nCenter )
			nYellowLine = ( vecSplitX.size() - SRand::Inst<eRand_Render>().Rand( 1, 3 ) ) / 2;
	}
	int32 curY = 0;
	for( int j = 0; j <= vecSplitY.size(); j++ )
	{
		int32 y1 = j < vecSplitY.size() ? curY + vecSplitY[j] - 1 : nHeight;
		int32 nArrowBegin = -1, nArrowEnd = -1;
		int32 nArrowLen = SRand::Inst<eRand_Render>().Rand( 3, 6 );
		if( y1 - curY >= nArrowLen + 2 )
		{
			nArrowBegin = SRand::Inst<eRand_Render>().Rand( curY + 1, y1 - 1 - nArrowLen + 1 );
			nArrowEnd = nArrowBegin + nArrowLen;
		}

		for( int y = curY; y < y1; y++ )
		{
			auto pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage->SetRect( CRectangle( 0, y * 32, 16, 32 ) );
			pImage->SetTexRect( CRectangle( 0.25f, 0, 0.25f, 0.25f ) );
			GetRenderObject()->AddChild( pImage );
			pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage->SetRect( CRectangle( nWidth * 32 - 16, y * 32, 16, 32 ) );
			pImage->SetTexRect( CRectangle( 0, 0, 0.25f, 0.25f ) );
			GetRenderObject()->AddChild( pImage );

			int32 curX = 0;
			for( int i = 0; i < xSplit; i++ )
			{
				int32 x1 = curX + vecSplitX[i];
				if( y >= nArrowBegin && y < nArrowEnd )
				{
					CRectangle texRect( 0, 0.75f, 0.5f, 0.25f );
					if( y == nArrowBegin )
					{
						texRect = CRectangle( 0.5f, 0, 0.5f, 0.25f );
						if( j == 0 && xSplit > SRand::Inst<eRand_Render>().Rand( 1, 4 ) )
						{
							if( i == 0 )
								texRect.y = 0.5f;
							else if( i == xSplit - 1 )
								texRect.y = 0.25f;
						}
					}
					else if( y == nArrowEnd - 1 )
						texRect.y = 0.5f;
					pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
					pImage->SetRect( CRectangle( ( curX + x1 - 1 ) * 16, y * 32, 32, 32 ) );
					pImage->SetTexRect( texRect );
					GetRenderObject()->AddChild( pImage );
				}

				if( i < xSplit - 1 )
				{
					CRectangle texRect( 0, i == nYellowLine ? 0.25f : 0.75f, 0.5f, 0.25f );
					if( i == nYellowLine || !!( y & 1 ) )
					{
						pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
						pImage->SetRect( CRectangle( x1 * 32 - 16, y * 32, 32, 32 ) );
						pImage->SetTexRect( texRect );
						GetRenderObject()->AddChild( pImage );
					}
					curX = x1;
				}
			}

		}
		if( j < vecSplitY.size() )
		{
			for( int x = 0; x < nWidth; x++ )
			{
				auto pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
				pImage->SetRect( CRectangle( x * 32, y1 * 32, 32, 32 ) );
				pImage->SetTexRect( CRectangle( 0.5f, 0.75f, 0.5f, 0.25f ) );
				GetRenderObject()->AddChild( pImage );
			}
			curY += vecSplitY[j];
		}
	}
}

void CBillboardDeco::Init( const CVector2& size, SChunk* pPreParent )
{
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	SetRenderObject( NULL );
	int32 nWidth = size.x / 32;
	int32 nHeight = size.y / 32;
	if( nWidth <= 6 || nHeight <= 3 )
		return;
	int32 nMan = Min( ( nWidth + 5 ) / 6, 10 );
	int32 nManWidth = Min( nMan * 3, ( nWidth + 1 ) / 2 );
	nMan = nMan * 3 / 2;
	CImage2D* pImage;
	int32 nBuildingWidth = Max( nWidth / 2, nWidth - nManWidth );
	nBuildingWidth *= 32;
	{
		int32 x = -SRand::Inst<eRand_Render>().Rand( 0, 12 ) * 4;
		while( x < size.x )
		{
			int32 y = SRand::Inst<eRand_Render>().Rand( nHeight / 2, nHeight * 3 / 2 ) * 32 + SRand::Inst<eRand_Render>().Rand( 0, 8 ) * 4;
			y += floor( abs( x * 2 - size.x ) * 16 / size.x ) * 4;
			int32 nType = SRand::Inst<eRand_Render>().Rand( 0, 4 );
			int32 nType0 = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			int8 b = 0;
			if( ( y >> 5 ) >= nHeight )
			{
				y = ( y & 31 ) + nHeight * 32;
				b = 1;
			}
			for( ; y > 0; y -= 64 )
			{
				CRectangle rect( x, y - 64, 64, 64 );
				CRectangle texRect( 0.125f + nType0 * 0.0625f, 0.5f + nType * 0.125f + b * 0.0625f, 0.0625f, 0.0625f );
				auto rect1 = rect * CRectangle( 0, 0, size.x, size.y );
				texRect = CRectangle( texRect.x + texRect.width * ( rect1.x - rect.x ) / rect.width, texRect.y + texRect.height * ( rect.GetBottom() - rect1.GetBottom() ) / rect.height,
					texRect.width * rect1.width / rect.width, texRect.height * rect1.height / rect.height );
				rect = rect1;
				pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
				pImage->SetRect( rect );
				pImage->SetTexRect( texRect );
				AddChild( pImage );
				b = 1;
				if( !SRand::Inst<eRand_Render>().Rand( 0, 3 ) )
					nType = SRand::Inst<eRand_Render>().Rand( 0, 4 );
			}
			x += SRand::Inst<eRand_Render>().Rand( 14, 19 ) * 4;
		}
	}
	{
		int32 x = size.x + SRand::Inst<eRand_Render>().Rand( 0, 12 ) * 4;
		while( x > nBuildingWidth )
		{
			int32 y = Max( 16, 8 + ( x - nBuildingWidth ) * ( nHeight * 8 - 8 ) / ( nWidth * 32 - nBuildingWidth ) + SRand::Inst<eRand_Render>().Rand( 0, 16 ) ) * 4;
			int32 nType = SRand::Inst<eRand_Render>().Rand( 0, 4 );
			int32 nType0 = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			int8 b = 0;
			if( ( y >> 5 ) >= nHeight )
			{
				y = ( y & 31 ) + nHeight * 32;
				b = 1;
			}
			for( ; y > 0; y -= 64 )
			{
				CRectangle rect( x - 64, y - 64, 64, 64 );
				CRectangle texRect( nType0 * 0.0625f, 0.5f + nType * 0.125f + b * 0.0625f, 0.0625f, 0.0625f );
				auto rect1 = rect * CRectangle( 0, 0, size.x, size.y );
				texRect = CRectangle( texRect.x + texRect.width * ( rect1.x - rect.x ) / rect.width, texRect.y + texRect.height * ( rect.GetBottom() - rect1.GetBottom() ) / rect.height,
					texRect.width * rect1.width / rect.width, texRect.height * rect1.height / rect.height );
				rect = rect1;
				pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
				pImage->SetRect( rect );
				pImage->SetTexRect( texRect );
				AddChild( pImage );
				b = 1;
				if( !SRand::Inst<eRand_Render>().Rand( 0, 3 ) )
					nType = SRand::Inst<eRand_Render>().Rand( 0, 4 );
			}
			x -= SRand::Inst<eRand_Render>().Rand( 16, 22 ) * 4;
		}
	}
	{
		int32 nBeginX = -SRand::Inst<eRand_Render>().Rand( 0, 8 );
		int32 nEndX = nManWidth * 8 - SRand::Inst<eRand_Render>().Rand( 0, 8 );
		int32 nBeginY = nHeight <= 4 ? -8 : 0;
		int32 nEndY = -SRand::Inst<eRand_Render>().Rand( 4, 8 ) - ( nHeight <= 4 ? 8 : 0 );
		int32 nTypes[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		SRand::Inst<eRand_Render>().Shuffle( nTypes, 10 );
		for( int i = 0; i < nMan; i++ )
		{
			int32 x = 4 * ( nBeginX + ( ( nEndX - nBeginX ) * i + SRand::Inst<eRand_Render>().Rand( 0, nMan ) ) / nMan );
			int32 y = 4 * ( nBeginY + ( ( nEndY - nBeginY ) * i + SRand::Inst<eRand_Render>().Rand( 0, nMan ) ) / nMan );
			int32 nType = nTypes[i];
			CRectangle rect( x, y, 96, 128 );
			CRectangle texRect( ( nType % 5 ) * 0.0625f * 3, ( nType / 5 ) * 0.0625f * 4, 0.0625f * 3, 0.0625f * 4 );
			auto rect1 = rect * CRectangle( 0, 0, size.x, size.y );
			texRect = CRectangle( texRect.x + texRect.width * ( rect1.x - rect.x ) / rect.width, texRect.y + texRect.height * ( rect.GetBottom() - rect1.GetBottom() ) / rect.height,
				texRect.width * rect1.width / rect.width, texRect.height * rect1.height / rect.height );
			rect = rect1;
			pImage = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage->SetRect( rect );
			pImage->SetTexRect( texRect );
			AddChild( pImage );
		}
	}
}

void CControlRoomSubChunk::OnSetChunk( SChunk* pChunk, CMyLevel* pLevel )
{
	auto pParentChunk = pChunk->pParentChunk;
	if( !pParentChunk )
		return;
	auto pChunkObject = SafeCast<CControlRoom>( pParentChunk->pChunkObject );
	if( !pChunkObject )
		return;

	m_pBlob->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onBlobKilled );
	if( m_pBlobRenderObject )
	{
		if( m_nType1 == 2 && SafeCast<CEntity>( m_pBlobRenderObject.GetPtr() ) )
		{
			auto pEntity = SafeCast<CEntity>( m_pBlobRenderObject.GetPtr() );
			auto pDrawable = static_cast<CDrawableGroup*>( pEntity->GetResource() );
			auto texRect0 = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
			SetRenderObject( new CRenderObject2D );
			pEntity->SetRenderObject( NULL );
			auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
			if( pChunk->nWidth > 1 )
			{
				auto pImage = pDrawable->CreateInstance();
				pImage->SetPosition( CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
				pEntity->AddChild( pImage );
				pImage = pDrawable->CreateInstance();
				pImage->SetPosition( CVector2( pChunk->nWidth - 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
				pEntity->AddChild( pImage );
				int32 n = pChunk->pos.y == 0 ? 1 : pChunk->nHeight - 1;
				for( int i = 1; i < pChunk->nWidth; i++ )
				{
					pImage = pDrawable->CreateInstance();
					pImage->SetPosition( CVector2( i, n ) * CMyLevel::GetBlockSize() );
					pEntity->AddChild( pImage );
				}
				texRect0.width /= 3;
				for( int i = 0; i < pChunk->nWidth; i++ )
				{
					CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
					GetRenderObject()->AddChild( pImage2D );
					pImage2D->SetRect( CRectangle( i * 32, 0, 32, 32 ) );
					auto tex = CRectangle( texRect0.x + ( i == 0 ? 0 : ( i == pChunk->nWidth - 1 ? 2 : 1 ) ) * texRect0.width, texRect0.y, texRect0.width, texRect0.height );
					pImage2D->SetTexRect( tex );
					GetBlock( i, 0 )->rtTexRect = tex;
				}
			}
			else
			{
				auto pImage = pDrawable->CreateInstance();
				pImage->SetPosition( CVector2( 0.5f, 0.5f ) * CMyLevel::GetBlockSize() );
				pEntity->AddChild( pImage );
				pImage = pDrawable->CreateInstance();
				pImage->SetPosition( CVector2( 0.5f, pChunk->nHeight - 0.5f ) * CMyLevel::GetBlockSize() );
				pEntity->AddChild( pImage );
				int32 n = pChunk->pos.x == 0 ? 1 : pChunk->nWidth - 1;
				for( int i = 1; i < pChunk->nHeight; i++ )
				{
					pImage = pDrawable->CreateInstance();
					pImage->SetPosition( CVector2( n, i ) * CMyLevel::GetBlockSize() );
					pEntity->AddChild( pImage );
				}
				texRect0.height /= 3;
				for( int i = 0; i < pChunk->nHeight; i++ )
				{
					CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
					GetRenderObject()->AddChild( pImage2D );
					pImage2D->SetRect( CRectangle( 0, i * 32, 32, 32 ) );
					auto tex = CRectangle( texRect0.x, texRect0.y + ( i == 0 ? 2 : ( i == pChunk->nHeight - 1 ? 0 : 1 ) ) * texRect0.height, texRect0.width, texRect0.height );
					pImage2D->SetTexRect( tex );
					GetBlock( 0, i )->rtTexRect = tex;
				}
			}
		}
		else
		{
			auto p = static_cast<CMultiFrameImage2D*>( m_pBlobRenderObject.GetPtr() );
			int32 nFrame = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			p->SetFrames( nFrame * 2, nFrame * 2 + 2, p->GetFramesPerSec() );
		}
		for( auto pChild = m_pBlobRenderObject->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			auto p = static_cast<CMultiFrameImage2D*>( pChild );
			p->SetAutoUpdateAnim( true );
			int32 nFrame = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			p->SetFrames( nFrame * 2, nFrame * 2 + 2, p->GetFramesPerSec() );
		}
		m_pBlobRenderObject->RemoveThis();
		m_pBlobRenderObject->SetPosition( m_pBlobRenderObject->GetPosition() + m_pBlob->GetPosition() + GetPosition() );
		pChunkObject->GetLayer1()->AddChild( m_pBlobRenderObject );
	}

	int32 nX = pChunk->pos.x / CMyLevel::GetBlockSize();
	int32 nY = pChunk->pos.y / CMyLevel::GetBlockSize();
	m_vecTexs.resize( pChunk->blocks.size() );
	for( int i = 0; i < pChunk->blocks.size(); i++ )
	{
		auto& block = pChunk->blocks[i];
		auto& tex1 = m_vecTexs[i];
		tex1 = CRectangle( 0, 0, 0, 0 );
		if( m_nType == 1 )
		{
			if( pChunk->nHeight == 1 )
				tex1 = m_tex1.Offset( CVector2( -2 * m_tex1.width, 0 * m_tex1.height ) );
			else if( block.nY == 0 )
				tex1 = m_tex1.Offset( CVector2( -2 * m_tex1.width, -1 * m_tex1.height ) );
			else if( block.nY == pChunk->nHeight - 1 )
				tex1 = m_tex1.Offset( CVector2( -2 * m_tex1.width, 1 * m_tex1.height ) );
		}
		else if( m_nType == 3 )
		{
			if( pChunk->nHeight == 1 )
				tex1 = m_tex1.Offset( CVector2( 2 * m_tex1.width, 0 * m_tex1.height ) );
			else if( block.nY == 0 )
				tex1 = m_tex1.Offset( CVector2( 2 * m_tex1.width, -1 * m_tex1.height ) );
			else if( block.nY == pChunk->nHeight - 1 )
				tex1 = m_tex1.Offset( CVector2( 2 * m_tex1.width, 1 * m_tex1.height ) );
		}
		else if( m_nType == 2 )
		{
			if( pChunk->nWidth == 1 )
				tex1 = m_tex1.Offset( CVector2( 0 * m_tex1.width, 2 * m_tex1.height ) );
			else if( block.nX == 0 )
				tex1 = m_tex1.Offset( CVector2( 1 * m_tex1.width, 2 * m_tex1.height ) );
			else if( block.nX == pChunk->nWidth - 1 )
				tex1 = m_tex1.Offset( CVector2( -1 * m_tex1.width, 2 * m_tex1.height ) );
		}
		else if( m_nType == 4 )
		{
			if( pChunk->nWidth == 1 )
				tex1 = m_tex1.Offset( CVector2( 0 * m_tex1.width, -2 * m_tex1.height ) );
			else if( block.nX == 0 )
				tex1 = m_tex1.Offset( CVector2( 1 * m_tex1.width, -2 * m_tex1.height ) );
			else if( block.nX == pChunk->nWidth - 1 )
				tex1 = m_tex1.Offset( CVector2( -1 * m_tex1.width, -2 * m_tex1.height ) );
		}
		else
		{
			int32 blockX = nX + block.nX;
			int32 blockY = nY + block.nY;
			bool b[4];
			b[0] = blockX > 0 && pParentChunk->GetBlock( blockX - 1, blockY )->eBlockType > eBlockType_Wall;
			b[1] = blockY > 0 && pParentChunk->GetBlock( blockX, blockY - 1 )->eBlockType > eBlockType_Wall;
			b[2] = blockX < pParentChunk->nWidth - 1 && pParentChunk->GetBlock( blockX + 1, blockY )->eBlockType > eBlockType_Wall;
			b[3] = blockY < pParentChunk->nHeight - 1 && pParentChunk->GetBlock( blockX, blockY + 1 )->eBlockType > eBlockType_Wall;
			if( b[0] || b[1] || b[2] || b[3] )
			{
				float ofsX, ofsY;
				if( b[0] && b[2] )
					ofsX = 0;
				else if( b[0] )
					ofsX = 1;
				else if( b[2] )
					ofsX = -1;
				else
					ofsX = SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? -1.5f : 1.5f;
				if( b[1] && b[3] )
					ofsY = 0;
				else if( b[1] )
					ofsY = -1;
				else if( b[3] )
					ofsY = 1;
				else
					ofsY = SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? -1.5f : 1.5f;
				tex1 = m_tex1.Offset( CVector2( ofsX * m_tex1.width, ofsY * m_tex1.height ) );
			}
		}
	}

	if( m_nType1 < 2 )
	{
		pChunkObject->OnSubChunkAdded( this );
		CChunkObject::OnSetChunk( pChunk, pLevel );
	}
}

void CControlRoomSubChunk::Kill()
{
	if( m_pBlob )
	{
		if( m_onBlobKilled.IsRegistered() )
			m_onBlobKilled.Unregister();
		if( m_pBlobRenderObject )
			m_pBlobRenderObject->RemoveThis();
		m_pBlob->Kill();
		m_pBlob = NULL;
	}
	CChunkObject::Kill();
}

void CControlRoomSubChunk::OnRemovedFromStage()
{
	if( m_onBlobKilled.IsRegistered() )
		m_onBlobKilled.Unregister();
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	m_vecTexs.clear();
	CChunkObject::OnRemovedFromStage();
}

void CControlRoomSubChunk::DelayKill()
{
	GetStage()->RegisterAfterHitTest( m_nDist * 8, &m_onTick );
}

void CControlRoomSubChunk::OnBlobKilled()
{
	if( m_onBlobKilled.IsRegistered() )
		m_onBlobKilled.Unregister();
	m_pBlob = NULL;
	if( m_pBlobRenderObject )
		m_pBlobRenderObject->RemoveThis();

	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	auto pImg = SafeCast<CImage2D>( GetRenderObject() );
	if( !pImg )
	{
		for( auto pChild = GetRenderObject()->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			pImg = SafeCast<CImage2D>( GetRenderObject() );
			if( pImg )
				break;
		}
	}
	uint16 nParamCount;
	auto pParam = pImg ? pImg->GetParam( nParamCount ) : NULL;
	auto pRenderObject = new CRenderObject2D;
	for( int i = 0; i < m_vecTexs.size(); i++ )
	{
		auto& tex1 = m_vecTexs[i];
		auto& block = GetChunk()->blocks[i];
		if( tex1.width > 0 )
		{
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			pImage2D->SetRect( CRectangle( block.nX * 32, block.nY * 32, 32, 32 ) );
			pImage2D->SetTexRect( tex1 );
			if( pParam )
				memcpy( pImage2D->GetParam(), pParam, nParamCount * sizeof( CVector4 ) );
			pRenderObject->AddChild( pImage2D );
		}
		auto pBlockObject = SafeCast<CBlockObject>( block.pEntity.GetPtr() );
		pBlockObject->ClearBuffs();
		pBlockObject->ClearEfts();
		block.rtTexRect = tex1;
		block.eBlockType = eBlockType_Wall;
		block.pEntity->SetHitType( eEntityHitType_Sensor );
		block.bImmuneToBlockBuff = 1;
	}
	SetRenderObject( pRenderObject );

	if( m_nType1 < 2 )
	{
		auto pParentChunk = GetChunk()->pParentChunk;
		if( pParentChunk )
		{
			auto pChunkObject = SafeCast<CControlRoom>( pParentChunk->pChunkObject );
			if( pChunkObject )
				pChunkObject->OnSubChunkKilled( this );
		}
	}
}

void CControlRoom::OnRemovedFromStage()
{
	m_vecSub.clear();
	CChunkObject::OnRemovedFromStage();
}

void CControlRoom::OnSetChunk( SChunk * pChunk, CMyLevel * pLevel )
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

	vector<int8> vecTemp;
	vecTemp.resize( pChunk->nWidth * pChunk->nHeight * 4 );
	auto texRect = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	texRect.width /= 4;
	texRect.height /= 4;
	bool b[4] = { true, true, true, true };
	for( int x = 0; x < pChunk->nWidth; x++ )
	{
		if( !pChunk->GetBlock( x, 0 )->nTag || pChunk->GetBlock( x, 0 )->nTag >= 4 )
			b[1] = false;
		if( !pChunk->GetBlock( x, pChunk->nHeight - 1 )->nTag || pChunk->GetBlock( x, pChunk->nHeight - 1 )->nTag >= 4 )
			b[3] = false;
	}
	for( int y = 0; y < pChunk->nHeight; y++ )
	{
		if( !pChunk->GetBlock( 0, y )->nTag || pChunk->GetBlock( 0, y )->nTag >= 4 )
			b[0] = false;
		if( !pChunk->GetBlock( pChunk->nWidth - 1, y )->nTag || pChunk->GetBlock( pChunk->nWidth - 1, y )->nTag >= 4 )
			b[2] = false;
	}
	bool b1[2] = { false, false };
	if( pChunk->GetBlock( 0, pChunk->nHeight - 1 )->nTag == 3 )
	{
		for( int x = Min<int32>( pChunk->nWidth * 2 - 1, 5 ); x >= 0; x-- )
		{
			for( int y = Max<int32>( 0, pChunk->nHeight * 2 - 5 ); y < pChunk->nHeight * 2; y++ )
				vecTemp[x + y * pChunk->nWidth * 2] = 1;
		}
		b1[0] = true;
	}
	if( pChunk->GetBlock( pChunk->nWidth - 1, pChunk->nHeight - 1 )->nTag == 3 )
	{
		for( int x = Max<int32>( 0, pChunk->nWidth * 2 - 5 ); x < pChunk->nWidth * 2; x++ )
		{
			for( int y = Max<int32>( 0, pChunk->nHeight * 2 - 5 ); y < pChunk->nHeight * 2; y++ )
				vecTemp[x + y * pChunk->nWidth * 2] = 1;
		}
		b1[1] = true;
	}
	int32 nTypeX = SRand::Inst<eRand_Render>().Rand( 0, 2 ) * 2 + 4, nTypeY = SRand::Inst<eRand_Render>().Rand( 0, 2 ) * 2;

	SetRenderObject( new CRenderObject2D );
	m_pLayer1 = new CRenderObject2D;
	GetRenderObject()->AddChild( m_pLayer1 );
	for( int iY = 0; iY < pChunk->nHeight; iY++ )
	{
		for( int iX = 0; iX < pChunk->nWidth; iX++ )
		{
			int32 nX, nY;
			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
			if( GetBlock( iX, iY )->eBlockType == eBlockType_Wall )
			{
				GetRenderObject()->AddChildAfter( pImage2D, m_pLayer1 );
				nX = nTypeX + SRand::Inst<eRand_Render>().Rand( 0, 2 );
				nY = nTypeY + SRand::Inst<eRand_Render>().Rand( 0, 2 );
				vecTemp[iX * 2 + ( iY * 2 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + ( iY * 2 + 1 ) * pChunk->nWidth * 2]
					= vecTemp[iX * 2 + 1 + ( iY * 2 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + 1 + ( iY * 2 + 1 ) * pChunk->nWidth * 2] = 1;
			}
			else
			{
				GetRenderObject()->AddChild( pImage2D );
				if( b1[0] && iX < 3 && iY >= pChunk->nHeight - 3 )
				{
					nX = 5 + iX;
					nY = 10 + pChunk->nHeight - iY;
				}
				else if( b1[1] && iX >= pChunk->nWidth - 3 && iY >= pChunk->nHeight - 3 )
				{
					nX = 8 - pChunk->nWidth + iX;
					nY = 7 + pChunk->nHeight - iY;
				}
				else
				{
					if( b[0] && iX == 0 )
						nX = 0;
					else if( b[2] && iX == pChunk->nWidth - 1 )
						nX = 2;
					else
						nX = 1;
					if( b[1] && iY == 0 )
						nY = 2;
					else if( b[3] && iY == pChunk->nHeight - 1 )
						nY = 0;
					else
						nY = 1;
				}

				if( iX == 0 && b[0] )
				{
					vecTemp[iX * 2 + ( iY * 2 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + ( iY * 2 + 1 ) * pChunk->nWidth * 2] = 1;
					if( pChunk->GetBlock( iX, iY )->nTag == 2 )
					{
						auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
						if( iY == 0 && b[1] )
						{
							pImage->SetRect( CRectangle( iX, iY + 0.25f, 0.5f, 0.75f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.75f, 1 - 0.5f * 0.75f, 0.125f, 0.5f * 0.75f ) );
						}
						else if( iY == pChunk->nHeight - 1 && b[3] )
						{
							pImage->SetRect( CRectangle( iX, iY, 0.5f, 0.75f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.75f, 0, 0.125f, 0.5f * 0.75f ) );
						}
						else
						{
							pImage->SetRect( CRectangle( iX, iY, 0.5f, 1 ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.75f, 0.25f, 0.125f, 0.5f ) );
						}
						GetRenderObject()->AddChild( pImage );
					}
				}
				if( iY == 0 && b[1] )
				{
					vecTemp[iX * 2 + ( iY * 2 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + 1 + ( iY * 2 ) * pChunk->nWidth * 2] = 1;
					if( pChunk->GetBlock( iX, iY )->nTag == 2 )
					{
						auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
						if( iX == 0 && b[0] )
						{
							pImage->SetRect( CRectangle( iX + 0.25f, iY, 0.75f, 0.5f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.25f, 0.75f, 0.25f * 0.75f, 0.25f ) );
						}
						else if( iX == pChunk->nWidth - 1 && b[2] )
						{
							pImage->SetRect( CRectangle( iX, iY, 0.75f, 0.5f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.75f - 0.25f * 0.75f, 0.75f, 0.25f * 0.75f, 0.25f ) );
						}
						else
						{
							pImage->SetRect( CRectangle( iX, iY, 1, 0.5f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.375f, 0.75f, 0.25f, 0.25f ) );
						}
						GetRenderObject()->AddChild( pImage );
					}
				}
				if( iX == pChunk->nWidth - 1 && b[2] )
				{
					vecTemp[iX * 2 + 1 + ( iY * 2 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + 1 + ( iY * 2 + 1 ) * pChunk->nWidth * 2] = 1;
					if( pChunk->GetBlock( iX, iY )->nTag == 2 )
					{
						auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
						if( iY == 0 && b[1] )
						{
							pImage->SetRect( CRectangle( iX + 0.5f, iY + 0.25f, 0.5f, 0.75f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.875f, 1 - 0.5f * 0.75f, 0.125f, 0.5f * 0.75f ) );
						}
						else if( iY == pChunk->nHeight - 1 && b[3] )
						{
							pImage->SetRect( CRectangle( iX + 0.5f, iY, 0.5f, 0.75f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.875f, 0, 0.125f, 0.5f * 0.75f ) );
						}
						else
						{
							pImage->SetRect( CRectangle( iX + 0.5f, iY, 0.5f, 1 ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.875f, 0.25f, 0.125f, 0.5f ) );
						}
						GetRenderObject()->AddChild( pImage );
					}
				}
				if( iY == pChunk->nHeight - 1 && b[3] )
				{
					vecTemp[iX * 2 + ( iY * 2 + 1 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + 1 + ( iY * 2 + 1 ) * pChunk->nWidth * 2] = 1;
					if( pChunk->GetBlock( iX, iY )->nTag == 2 )
					{
						auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
						if( iX == 0 && b[0] )
						{
							pImage->SetRect( CRectangle( iX + 0.25f, iY + 0.5f, 0.75f, 0.5f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.25f, 0.5f, 0.25f * 0.75f, 0.25f ) );
						}
						else if( iX == pChunk->nWidth - 1 && b[2] )
						{
							pImage->SetRect( CRectangle( iX, iY + 0.5f, 0.75f, 0.5f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.75f - 0.25f * 0.75f, 0.5f, 0.25f * 0.75f, 0.25f ) );
						}
						else
						{
							pImage->SetRect( CRectangle( iX, iY + 0.5f, 1, 0.5f ) * CMyLevel::GetBlockSize() );
							pImage->SetTexRect( CRectangle( 0.375f, 0.5f, 0.25f, 0.25f ) );
						}
						GetRenderObject()->AddChild( pImage );
					}
				}
				if( pChunk->GetBlock( iX, iY )->nTag == 4 )
				{
					vecTemp[iX * 2 + ( iY * 2 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + ( iY * 2 + 1 ) * pChunk->nWidth * 2]
						= vecTemp[iX * 2 + 1 + ( iY * 2 ) * pChunk->nWidth * 2] = vecTemp[iX * 2 + 1 + ( iY * 2 + 1 ) * pChunk->nWidth * 2] = 1;
					auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
					pImage->SetRect( CRectangle( iX, iY, 1, 1 ) * CMyLevel::GetBlockSize() );
					pImage->SetTexRect( CRectangle( 0.5f, 0, 0.25f, 0.5f ) );
					GetRenderObject()->AddChild( pImage );
				}
			}

			auto tex = texRect.Offset( CVector2( nX * texRect.width, nY * texRect.height ) );
			pImage2D->SetTexRect( tex );
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

	int32 nWidth = pChunk->nWidth * 2;
	int32 nHeight = pChunk->nHeight * 2;
	vector<TVector2<int32>> vec;
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( !vecTemp[i + j * nWidth] )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst<eRand_Render>().Shuffle( vec );
	function<void( const TRectangle<int32>& ) > Func;
	Func = [=, &Func, &vecTemp] ( const TRectangle<int32>& rect ) {
		if( rect.width == 1 && rect.height == 1 )
		{
			auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
			pImage->SetRect( CRectangle( rect.x, rect.y, 1, 1 ) * 16 );
			pImage->SetTexRect( CRectangle( SRand::Inst<eRand_Render>().Rand( 0, 2 ) * 0.125f, SRand::Inst<eRand_Render>().Rand( 0, 2 ) * 0.25f, 0.125f, 0.25f ) );
			GetRenderObject()->AddChild( pImage );
		}
		else if( rect.height == 1 )
		{
			if( rect.width >= SRand::Inst<eRand_Render>().Rand( 6, 9 ) )
			{
				int32 n = ( rect.width + SRand::Inst<eRand_Render>().Rand( 0, 2 ) ) / 2;
				Func( TRectangle<int32>( rect.x, rect.y, n, rect.height ) );
				Func( TRectangle<int32>( rect.x + n, rect.y, rect.width - n, rect.height ) );
				return;
			}
			int32 n = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			for( int i = rect.x; i < rect.GetRight(); i++ )
			{
				auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
				pImage->SetRect( CRectangle( i, rect.y, 1, 1 ) * 16 );
				pImage->SetTexRect( CRectangle( ( i == rect.x ? 2 : ( i == rect.GetRight() - 1 ? 3 : 2.5f ) ) * 0.125f, n * 0.25f, 0.125f, 0.25f ) );
				GetRenderObject()->AddChild( pImage );
			}
		}
		else
		{
			if( rect.height >= SRand::Inst<eRand_Render>().Rand( 6, 9 ) )
			{
				int32 n = ( rect.height + SRand::Inst<eRand_Render>().Rand( 0, 2 ) ) / 2;
				Func( TRectangle<int32>( rect.x, rect.y, rect.width, n ) );
				Func( TRectangle<int32>( rect.x, rect.y + n, rect.width, rect.height - n ) );
				return;
			}
			int32 n = SRand::Inst<eRand_Render>().Rand( 0, 2 );
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				auto pImage = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
				pImage->SetRect( CRectangle( rect.x, j, 1, 1 ) * 16 );
				pImage->SetTexRect( CRectangle( n * 0.125f, ( j == rect.y ? 3 : ( j == rect.GetBottom() - 1 ? 2 : 2.5f ) ) * 0.25f, 0.125f, 0.25f ) );
				GetRenderObject()->AddChild( pImage );
			}
		}
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				vecTemp[i + j * nWidth] = 1;
			}
		}
	};
	for( auto& p : vec )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;
		{
			int x;
			for( x = p.x; x > 0; x-- )
			{
				if( vecTemp[x - 1 + p.y * nWidth] )
					break;
			}
			if( x < p.x )
				Func( TRectangle<int32>( x, p.y, p.x - x, 1 ) );
		}
		{
			int x;
			for( x = p.x; x < nWidth - 1; x++ )
			{
				if( vecTemp[x + 1 + p.y * nWidth] )
					break;
			}
			if( x > p.x )
				Func( TRectangle<int32>( p.x + 1, p.y, x - p.x, 1 ) );
		}
		{
			int y;
			for( y = p.y; y > 0; y-- )
			{
				if( vecTemp[p.x + ( y - 1 ) * nWidth] )
					break;
			}
			if( y < p.y )
				Func( TRectangle<int32>( p.x, y, 1, p.y - y ) );
		}
		{
			int y;
			for( y = p.y; y < nHeight - 1; y++ )
			{
				if( vecTemp[p.x + ( y + 1 ) * nWidth] )
					break;
			}
			if( y > p.y )
				Func( TRectangle<int32>( p.x, p.y + 1, 1, y - p.y ) );
		}
	}

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
}

void CControlRoom::OnCreateComplete( CMyLevel* pLevel )
{
	if( !pLevel )
		return;

	auto GetController = [=] () 
	{
		if( !m_pController )
		{
			m_pController = new CWindow3Controller();
			m_pController->SetParentEntity( this );
		}
		return m_pController;
	};
	auto pChunk = GetChunk();
	if( pChunk->GetBlock( 0, pChunk->nHeight - 1 )->nTag == 3 )
	{
		auto pWindow = SafeCast<CWindow3>( m_pWindow[1]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( 1.5f * CMyLevel::GetBlockSize(), ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		GetController()->Add( pWindow );

		pWindow = SafeCast<CWindow3>( m_pWindow[2]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( 10, ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		GetController()->Add( pWindow );
	}
	if( pChunk->GetBlock( pChunk->nWidth - 1, pChunk->nHeight - 1 )->nTag == 3 )
	{
		auto pWindow = SafeCast<CWindow3>( m_pWindow[0]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( ( m_pChunk->nWidth - 1.5f ) * CMyLevel::GetBlockSize(), ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		GetController()->Add( pWindow );

		pWindow = SafeCast<CWindow3>( m_pWindow[3]->GetRoot()->CreateInstance() );
		pWindow->SetPosition( CVector2( m_pChunk->nWidth * CMyLevel::GetBlockSize() - 10, ( m_pChunk->nHeight - 2 ) * CMyLevel::GetBlockSize() ) );
		pWindow->SetParentEntity( this );
		pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
		GetController()->Add( pWindow );
	}

	for( int x = 1; x <= pChunk->nWidth - 1; x++ )
	{
		if( pChunk->GetBlock( x - 1, pChunk->nHeight - 1 )->nTag == 2 && pChunk->GetBlock( x, pChunk->nHeight - 1 )->nTag == 2 )
		{
			auto pWindow = SafeCast<CWindow3>( m_pWindow[x * 2 < pChunk->nWidth + SRand::Inst().Rand( 0, 2 ) ? 1 : 0]->GetRoot()->CreateInstance() );
			pWindow->SetPosition( CVector2( x * CMyLevel::GetBlockSize(), m_pChunk->nHeight * CMyLevel::GetBlockSize() + 2 ) );
			pWindow->SetParentEntity( this );
			pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
			GetController()->Add( pWindow );
		}
	}
	for( int i = 0; i < 2; i++ )
	{
		int32 x = i == 0 ? 0 : pChunk->nWidth - 1;
		for( int y = 1; y < pChunk->nHeight; y++ )
		{
			if( pChunk->GetBlock( x, y - 1 )->nTag == 2 && pChunk->GetBlock( x, y )->nTag == 2 )
			{
				auto pWindow = SafeCast<CWindow3>( m_pWindow[2 + i]->GetRoot()->CreateInstance() );
				pWindow->SetPosition( CVector2( i ? pChunk->nWidth * CMyLevel::GetBlockSize() - 6 : 6, y * CMyLevel::GetBlockSize() ) );
				pWindow->SetParentEntity( this );
				pWindow->SetRenderParentBefore( pLevel->GetChunkEffectRoot() );
				GetController()->Add( pWindow );
			}
		}
	}
}

void CControlRoom::OnSubChunkAdded( CControlRoomSubChunk* p )
{
	if( !m_vecSub.size() )
		m_vecSub.resize( m_pChunk->nWidth * m_pChunk->nHeight );
	auto pChunk = p->GetChunk();
	TRectangle<int32> r( pChunk->pos.x / CMyLevel::GetBlockSize(), pChunk->pos.y / CMyLevel::GetBlockSize(), pChunk->nWidth, pChunk->nHeight );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_vecSub[i + j * m_pChunk->nWidth] = p;
		}
	}
}

void CControlRoom::OnSubChunkKilled( CControlRoomSubChunk* p )
{
	if( p->m_nType1 == 1 )
	{
		vector<CReference<CControlRoomSubChunk> > q;
		p->m_nDist = 0;
		q.push_back( p );
		for( int i = 0; i < q.size(); i++ )
		{
			CControlRoomSubChunk* pChunkObject = q[i];
			if( pChunkObject->m_nDist > 0 )
				pChunkObject->DelayKill();
			auto pChunk = pChunkObject->GetChunk();
			TRectangle<int32> r( pChunk->pos.x / CMyLevel::GetBlockSize(), pChunk->pos.y / CMyLevel::GetBlockSize(), pChunk->nWidth, pChunk->nHeight );
			for( int y = r.y - 1; y <= r.GetBottom(); y += r.height + 1 )
			{
				if( y < 0 || y >= m_pChunk->nHeight )
					continue;
				for( int x = r.x; x < r.GetRight(); x++ )
				{
					auto pChunkObject1 = m_vecSub[x + y * m_pChunk->nWidth];
					if( !pChunkObject1 || pChunkObject1->m_nDist >= 0 )
						continue;
					pChunkObject1->m_nDist = pChunkObject->m_nDist + 1;
					q.push_back( pChunkObject1 );
				}
			}
			for( int x = r.x - 1; x <= r.GetRight(); x += r.width + 1 )
			{
				if( x < 0 || x >= m_pChunk->nWidth )
					continue;
				for( int y = r.y; y < r.GetBottom(); y++ )
				{
					auto pChunkObject1 = m_vecSub[x + y * m_pChunk->nWidth];
					if( !pChunkObject1 || pChunkObject1->m_nDist >= 0 )
						continue;
					pChunkObject1->m_nDist = pChunkObject->m_nDist + 1;
					q.push_back( pChunkObject1 );
				}
			}
		}
		for( CControlRoomSubChunk* pChunkObject : q )
		{
			pChunkObject->m_nDist = -1;
		}
	}

	auto pChunk = p->GetChunk();
	TRectangle<int32> r( pChunk->pos.x / CMyLevel::GetBlockSize(), pChunk->pos.y / CMyLevel::GetBlockSize(), pChunk->nWidth, pChunk->nHeight );
	for( int i = r.x; i < r.GetRight(); i++ )
	{
		for( int j = r.y; j < r.GetBottom(); j++ )
		{
			m_vecSub[i + j * m_pChunk->nWidth] = NULL;
		}
	}
}

void CBillboard1::OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel )
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
	SetRenderObject( new CRenderObject2D );
	vector<int8> vecTemp;
	vector<TVector2<int32> > vec;
	int32 nWidth = pChunk->nWidth;
	int32 nHeight = pChunk->nHeight;

	vecTemp.resize( nWidth * nHeight );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			vecTemp[i + j * nWidth] = pChunk->GetBlock( i, j )->nTag;
			if( j == 0 && !vecTemp[i + j * nWidth] )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	swap( SRand::Inst().nSeed, SRand::Inst<eRand_Render>().nSeed );
	SRand::Inst().Shuffle( vec );

	function<void( const TRectangle<int32>& rect )> Func;
	Func = [=, &vecTemp, &Func] ( const TRectangle<int32>& rect ) {
		int32 h0 = SRand::Inst().Rand( 5, 8 );
		if( rect.height > h0 )
		{
			Func( TRectangle<int32>( rect.x, rect.y, rect.width, h0 ) );
			Func( TRectangle<int32>( rect.x, rect.y + h0, rect.width, rect.height - h0 ) );
			return;
		}
		if( rect.height <= 2 )
		{
			if( rect.y > nHeight - 2 )
				return;
			int32 y = Min( nHeight - 2, SRand::Inst().Rand( 0, rect.height ) + rect.y );
			int8 nDir = SRand::Inst().Rand( 0, 2 );
			for( int i = 0; i < rect.width; i++ )
			{
				int32 x = nDir ? rect.x + i : rect.GetRight() - 1 - i;
				if( SRand::Inst().Rand( 0, 2 ) )
				{
					vecTemp[x + y * nWidth] = 3;
					auto pImg = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
					m_p1->AddChild( pImg );
					pImg->SetRenderParentBefore( m_p1 );
					pImg->SetRect( CRectangle( x, y, 1, 1 ) * CMyLevel::GetBlockSize() );
					pImg->SetTexRect( CRectangle( SRand::Inst().Rand( 0, 4 ), 0, 1, 1 ) * 0.25f );
					i++;
				}
			}
			return;
		}
		if( rect.width < 3 )
			return;
		if( rect.width > SRand::Inst().Rand( 9, 13 ) )
		{
			int32 w = SRand::Inst().Rand( 4, rect.width - 4 + 1 );
			Func( TRectangle<int32>( rect.x, rect.y, w, rect.height ) );
			Func( TRectangle<int32>( rect.x + w, rect.y, rect.width - w, rect.height ) );
			return;
		}
		TRectangle<int32> r1( rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2 );
		while( r1.width > 0 )
		{
			int32 w = Min( r1.width, SRand::Inst().Rand( 2, 5 ) );
			if( w == r1.width - 1 )
				w = w < 4 ? w + 1 : 2;
			auto r2 = r1;
			if( SRand::Inst().Rand( 0, 2 ) )
			{
				r2.width = w;
				r1.SetLeft( r2.GetRight() + 1 );
			}
			else
			{
				r2.SetLeft( r2.GetRight() - w );
				r1.SetRight( r2.x - 1 );
			}

			int32 n = SRand::Inst().Rand( 0, 2 );
			for( int iX = r2.x; iX < r2.GetRight(); iX++ )
			{
				for( int iY = r2.y; iY < r2.GetBottom(); iY++ )
				{
					vecTemp[iX + iY * nWidth] = 4;
					CRectangle tex = texRect;
					tex.x += tex.width * ( n * 2 + 4 + SRand::Inst().Rand( 0, 2 ) );
					tex.y += tex.height * SRand::Inst().Rand( 0, 4 );

					CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
					pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
					pImage2D->SetTexRect( tex );
					GetRenderObject()->AddChild( pImage2D );
					GetBlock( iX, iY )->rtTexRect = tex;
				}
			}
		}
	};

	for( auto& p : vec )
	{
		if( vecTemp[p.x + p.y * nWidth] )
			continue;
		auto rect = PutRect( vecTemp, nWidth, nHeight, p, TVector2<int32>( 3, 3 ), TVector2<int32>( nWidth, nHeight ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 2 );
		if( rect.width <= 0 )
			continue;
		Func( rect );
	}
	vec.resize( 0 );
	for( int i = 0; i < nWidth; i++ )
	{
		for( int j = 0; j < nHeight; j++ )
		{
			if( vecTemp[i + j * nWidth] == 2 )
				vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );
	for( auto& p : vec )
	{
		if( vecTemp[p.x + p.y * nWidth] != 2 )
			continue;
		auto rect = PutRect( vecTemp, nWidth, nHeight, p, TVector2<int32>( 3, 1 ), TVector2<int32>( SRand::Inst().Rand( 4, 7 ), 1 ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 3 );
		if( rect.width <= 0 )
			continue;
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			auto pImg = static_cast<CImage2D*>( m_pDeco->CreateInstance() );
			m_p1->AddChild( pImg );
			pImg->SetRenderParentBefore( m_p1 );
			pImg->SetRect( CRectangle( i, rect.y, 1, 1 ) * CMyLevel::GetBlockSize() );
			pImg->SetTexRect( CRectangle( 0.75f, 0.25f, 0.25f, 0.25f ) );
		}
		TRectangle<int32> rect1( rect.x - 3, rect.y - 3, rect.width + 6, rect.height + 6 );
		rect1 = rect1 * TRectangle<int32>( 0, 0, nWidth, nHeight );
		for( int i = rect1.x; i < rect1.GetRight(); i++ )
		{
			for( int j = rect1.y; j < rect1.GetBottom(); j++ )
			{
				if( Max( 0, Max( rect.x - i, i - rect.GetRight() + 1 ) ) + Max( 0, Max( rect.y - j, j - rect.GetBottom() + 1 ) ) > 3 )
					continue;
				if( vecTemp[i + j * nWidth] == 2 )
					vecTemp[i + j * nWidth] = 3;
			}
		}
	}

	for( int iY = 0; iY < nHeight; iY++ )
	{
		for( int iX = 0; iX < nWidth; iX++ )
		{
			if( vecTemp[iX + iY * nWidth] == 4 )
				continue;
			CRectangle tex = texRect;
			tex.x += tex.width * SRand::Inst().Rand( 0, 4 );
			tex.y += tex.height * SRand::Inst().Rand( 0, 4 );

			CImage2D* pImage2D = static_cast<CImage2D*>( pDrawableGroup->CreateInstance() );
			pImage2D->SetRect( CRectangle( iX * 32, iY * 32, 32, 32 ) );
			pImage2D->SetTexRect( tex );
			GetRenderObject()->AddChild( pImage2D );
			GetBlock( iX, iY )->rtTexRect = tex;

			if( pChunk->GetBlock( iX, iY )->nTag != 1 )
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
	}
	swap( SRand::Inst().nSeed, SRand::Inst<eRand_Render>().nSeed );

	m_nMaxHp += m_nHpPerSize * pChunk->nWidth * pChunk->nHeight;
	m_fHp = m_nMaxHp;
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

bool CHouse2::Damage( SDamageContext & context )
{
	DEFINE_TEMP_REF_THIS();
	if( CChunkObject::Damage( context ) )
	{
		m_bDamaged = true;
		return true;
	}
	return false;
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
