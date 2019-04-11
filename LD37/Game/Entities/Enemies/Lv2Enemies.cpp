#include "stdafx.h"
#include "Lv2Enemies.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "MyLevel.h"
#include "Bullet.h"
#include "Entities/Barrage.h"
#include "Entities/Bullets.h"
#include "Common/ResourceManager.h"
#include "Common/MathUtil.h"
#include "Entities/SpecialEft.h"
#include "Common/Algorithm.h"
#include "Entities/Blocks/Lv2/SpecialLv2.h"

void CLimbs::OnAddedToStage()
{
	if( !m_bAuto )
	{
		m_bIgnoreHit = true;
		m_bIgnoreBullet = true;
	}
	else
		GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	CChunkObject* pParChunk = NULL;
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
			pParChunk = pChunk;
	}
	if( pParChunk && CMyLevel::GetInst() )
	{
		pParChunk->RegisterPostKilledEvent( &m_onChunkKilled );
		auto pEnemy = SafeCast<CEnemy>( GetParentEntity() );
		if( pEnemy )
		{
			m_bIgnoreHit = false;
			auto pChunk = SafeCast<CChunkObject>( pEnemy->GetParentEntity() );
			if( pChunk )
			{
				if( m_pLimbsEft )
				{
					m_pLimbsEft->SetPosition( m_pLimbsEft->GetPosition() + GetPosition() + pEnemy->GetPosition() );
					m_pLimbsEft->SetParentEntity( pChunk->GetDecoratorRoot() );
					m_pLimbsEft->SetRenderParent( pEnemy );
				}
				if( m_pLimbsAttackEft )
				{
					m_pLimbsAttackEft->SetPosition( m_pLimbsAttackEft->GetPosition() + GetPosition() + pEnemy->GetPosition() );
					m_pLimbsAttackEft->SetParentEntity( pChunk->GetDecoratorRoot() );
					m_pLimbsAttackEft->SetRenderParent( pEnemy );
				}
			}
		}
		else
			SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
	}
}

void CLimbs::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	CEnemy::OnRemovedFromStage();
}

void CLimbs::Damage( SDamageContext & context )
{
	int32 nDmg = context.nDamage;
	if( m_nState == 2 )
		m_nDamage += nDmg;

	CEnemy* pEnemy = SafeCast<CEnemy>( GetParentEntity() );
	if( pEnemy )
	{
		uint8 nHitType = m_nHitType;
		context.nDamage = ceil( context.nDamage * ( 1 - m_fDefence ) );
		pEnemy->Damage( context );
		context.nHitType = nHitType;
	}
	else
	{
		CEnemy::Damage( context );
		context.nDamage = nDmg;
	}
}

bool CLimbs::IsOwner( CEntity * pEntity )
{
	if( pEntity == this )
		return true;
	auto pParent = SafeCast<CEnemy>( GetParentEntity() );
	if( pParent )
		return pParent->IsOwner( pEntity );
	return false;
}

void CLimbs::Kill()
{
	if( m_bKilled )
		return;
	m_bKilled = true;
	if( m_pLimbsEft )
		m_pLimbsEft->SetParentEntity( NULL );
	m_bIgnoreBullet = true;
	float r1 = SRand::Inst().Rand( 0.0f, 1.0f ), r2 = SRand::Inst().Rand( 0.0f, 1.0f );
	CVector2 vel1( m_killSpawnVelMin1.x + r1 * ( m_killSpawnVelMax1.x - m_killSpawnVelMin1.x ),
		m_killSpawnVelMin1.y + r2 * ( m_killSpawnVelMax1.y - m_killSpawnVelMin1.y ) );
	CVector2 vel2( m_killSpawnVelMin2.x + r1 * ( m_killSpawnVelMax2.x - m_killSpawnVelMin2.x ),
		m_killSpawnVelMin2.y + r2 * ( m_killSpawnVelMax2.y - m_killSpawnVelMin2.y ) );
	m_vel1 = vel1;
	m_vel2 = vel2;
	float l = Max( 0.0f, m_fEftLen - m_fKillEftDist );
	CVector2 dir[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	int i;
	for( i = 0; i < m_nMaxSpawnCount && l > 0; i++, l -= m_fKillEftDist )
	{
		auto pKillSpawn = SafeCast<CCharacter>( m_pKillSpawn->GetRoot()->CreateInstance() );
		pKillSpawn->SetPosition( globalTransform.GetPosition() + dir[m_nAttackDir] * ( l + m_fKillEftDist * 0.5f ) );
		pKillSpawn->SetVelocity( vel1 + ( vel2 - vel1 ) * ( i * 1.0f / ( m_nMaxSpawnCount - 1 ) ) );
		pKillSpawn->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
	KillAttackEft();

	if( m_bAuto && i < m_nMaxSpawnCount )
	{
		m_nKillSpawnLeft = m_nMaxSpawnCount - i;
		OnTickBeforeHitTest();
		if( m_tickAfterHitTest.IsRegistered() )
			m_tickAfterHitTest.Unregister();
	}
	else
	{
		CEnemy::Kill();
	}
}

int8 CLimbs::IsOperateable( const CVector2& pos )
{
	if( m_bKilled )
		return 3;
	int8 n = 0;
	if( m_nState != 0 || m_tickAfterHitTest.IsRegistered() )
		n |= 2;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return n | 1;
	CVector2 p = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
	if( !m_attackRect.Contains( p ) )
		return n | 1;
	return n;
}

void CLimbs::Operate( const CVector2 & pos )
{
	m_nState = 1;
	m_nAmmoLeft = m_nAmmoCount;
	m_nFireCDLeft = m_nFireCD;
	m_fEftLen = 0;
	if( m_pLimbsEft )
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->CreateAttackEft( SafeCast<CLimbsEft>( m_pLimbsEft.GetPtr() ) );
	else
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->CreateAttackEft( NULL, NULL, 0 );
	m_bIgnoreBullet = false;
	GetStage()->RegisterAfterHitTest( m_nAttackTime, &m_tickAfterHitTest );
}

void CLimbs::OnHitPlayer( CPlayer * pPlayer, const CVector2 & normal )
{
	if( !m_bAuto && !SafeCast<CEnemy>( GetParentEntity() ) && !m_nState )
		return;
	CEnemy::OnHitPlayer( pPlayer, normal );
}

void CLimbs::SetMask( uint8 nMask )
{
	if( m_pLimbsEft )
		SafeCast<CLimbsEft>( m_pLimbsEft.GetPtr() )->SetMask( nMask );
}

void CLimbs::OnChunkKilled()
{
	CChunkObject* pParChunk = NULL;
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
			pParChunk = pChunk;
	}
	if( pParChunk && pParChunk->GetChunk() )
	{
		pParChunk->RegisterPostKilledEvent( &m_onChunkKilled );
		SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
	}
	else
		Kill();
}

void CLimbs::KillAttackEft()
{
	if( !m_nState )
		return;
	if( m_pKillEffect )
	{
		CVector2 dir[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
		for( float i = m_fKillEftDist * 0.5f; i < m_fEftLen; i += m_fKillEftDist )
		{
			auto pKillEffect = SafeCast<CEffectObject>( m_pKillEffect->GetRoot()->CreateInstance() );
			pKillEffect->SetState( 2 );
			pKillEffect->SetPosition( globalTransform.GetPosition() + dir[m_nAttackDir] * i );
			pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
	}

	SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->DestroyAttackEft();
	m_nDamage = 0;
	m_nState = 0;
	m_fEftLen = 0;
	if( !m_bAuto )
		m_bIgnoreBullet = true;
}

void CLimbs::OnTickBeforeHitTest()
{
	GetStage()->RegisterBeforeHitTest( 5, &m_tickBeforeHitTest );

	ForceUpdateTransform();
	CVector2 dir[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	auto pKillSpawn = SafeCast<CCharacter>( m_pKillSpawn->GetRoot()->CreateInstance() );
	pKillSpawn->SetPosition( globalTransform.GetPosition() + dir[m_nAttackDir] * ( m_fKillEftDist * 0.5f ) );
	pKillSpawn->SetVelocity( m_vel1 + ( m_vel2 - m_vel1 ) * ( ( m_nMaxSpawnCount - m_nKillSpawnLeft ) * 1.0f / ( m_nMaxSpawnCount - 1 ) ) );
	pKillSpawn->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	
	m_nKillSpawnLeft--;
	if( !m_nKillSpawnLeft )
		CEnemy::Kill();
	else
		KillEffect();
}

void CLimbs::OnTickAfterHitTest()
{
	bool bHitChannel[eEntityHitType_Count] = { true, true, false, false, false, false, false };
	if( m_bIgnoreHit )
		bHitChannel[eEntityHitType_WorldStatic] = false;
	if( m_nState >= 1 )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto type = static_cast<CEntity*>( pManifold->pOtherHitProxy )->GetHitType();
			if( bHitChannel[type] )
			{
				GetStage()->RegisterAfterHitTest( m_nAttackTime1, &m_tickAfterHitTest );
				CCharacterMoveUtil::Stretch( this, m_nAttackDir, -m_fEftLen );
				KillAttackEft();
				return;
			}
		}
		if( m_pBullet && m_nAmmoLeft )
		{
			if( m_nFireCDLeft )
				m_nFireCDLeft--;
			if( !m_nFireCDLeft )
			{
				float fAngle = SRand::Inst().Rand( -0.5f, 0.5f ) + m_nAttackDir * PI * 0.5f;
				auto vel = CVector2( cos( fAngle ), sin( fAngle ) ) * m_fBulletSpeed;
				for( int i = 0; i < m_nBulletCount; i++ )
				{
					auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
					pBullet->SetCreator( this );
					pBullet->SetPosition( globalTransform.GetPosition() + CVector2( SRand::Inst().Rand( -8.0f, 8.0f ), SRand::Inst().Rand( -8.0f, 8.0f ) ) );
					pBullet->SetVelocity( vel );
					pBullet->SetRotation( fAngle );
					pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
				}
				m_nFireCDLeft = m_nFireCD;
				m_nAmmoLeft--;
			}
		}
	}

	if( m_nState == 0 )
	{
		if( m_bAuto )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( !pPlayer )
			{
				GetStage()->RegisterAfterHitTest( m_nAITick, &m_tickAfterHitTest );
				return;
			}
			CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
			if( !m_detectRect.Contains( pos ) )
			{
				GetStage()->RegisterAfterHitTest( m_nAITick, &m_tickAfterHitTest );
				return;
			}
			for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				auto type = static_cast<CEntity*>( pManifold->pOtherHitProxy )->GetHitType();
				if( bHitChannel[type] )
				{
					GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
					return;
				}
			}
			if( !m_attackRect.Contains( pos ) )
			{
				GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
				return;
			}

			m_nState = 1;
			m_nAmmoLeft = m_nAmmoCount;
			m_nFireCDLeft = m_nFireCD;
			m_fEftLen = 0;
			if( m_pLimbsEft )
				SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->CreateAttackEft( SafeCast<CLimbsEft>( m_pLimbsEft.GetPtr() ) );
			else
				SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->CreateAttackEft( NULL, NULL, 0 );
			GetStage()->RegisterAfterHitTest( m_nAttackTime, &m_tickAfterHitTest );
		}
		return;
	}
	else if( m_nState == 1 )
	{
		GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
		float f = CCharacterMoveUtil::Stretch( this, m_nAttackDir, Min( m_fStretchLenPerFrame, m_fMaxLen - m_fEftLen ), bHitChannel );
		m_fEftLen += f;
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->SetAttackEftLen( m_fEftLen );
		if( f < m_fStretchLenPerFrame || m_fEftLen >= m_fMaxLen )
			m_nState = 2;
	}
	else
	{
		float fLen = Min( m_fBackLenPerFrame + m_nDamage * m_fBackLenPerDamage, m_fEftLen );
		m_nDamage = 0;
		CCharacterMoveUtil::Stretch( this, m_nAttackDir, -fLen );
		m_fEftLen -= fLen;

		if( m_fEftLen <= 0 )
		{
			SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->DestroyAttackEft();
			m_nState = 0;
			GetStage()->RegisterAfterHitTest( m_nAttackTime1, &m_tickAfterHitTest );
			if( !m_bAuto )
				m_bIgnoreBullet = true;
		}
		else
		{
			SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->SetAttackEftLen( m_fEftLen );
			GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
		}
	}
}


void CLimbsController::OnAddedToStage()
{
	if( !m_vecLimbs.size() )
	{
		CChunkObject* pParChunk = SafeCast<CChunkObject>( GetParentEntity() );
		if( pParChunk )
		{
			auto pChunk = pParChunk->GetChunk();
			int32 nCount = pChunk->nWidth * pChunk->nHeight;
			m_nHp += pChunk->nWidth * pChunk->nHeight * 50;
			m_vecLimbs.resize( nCount );
			int32 n1 = nCount / 6 + 1;
			int32 n2 = nCount / 3 + 1;
			int32 nLimb = 0;
			for( int i = 0; i < pChunk->nWidth; i++ )
			{
				for( int j = 0; j < pChunk->nHeight; j++ )
				{
					auto p = SafeCast<CLimbs>( m_pLimbPrefab->GetRoot()->CreateInstance() );
					p->SetPosition( CVector2( i + 0.5f, j + 0.5f ) * CMyLevel::GetBlockSize() );
					p->SetParentEntity( this );
					if( nLimb >= n2 && nLimb < nCount - n2 )
						p->SetMask( 3 );
					else if( nLimb >= n1 && nLimb < nCount - n1 )
						p->SetMask( 2 );
					m_vecLimbs[nLimb++] = p;
				}
			}
			AddRect( CRectangle( 0, 0, pChunk->nWidth, pChunk->nHeight ) * CMyLevel::GetBlockSize() );
			GetStage()->GetHitTestMgr().ReAdd( this );
			m_attackRect = CRectangle( m_attackRect.x, m_attackRect.y,
				m_attackRect.width + pChunk->nWidth * CMyLevel::GetBlockSize(), m_attackRect.height + pChunk->nHeight * CMyLevel::GetBlockSize() );
			m_nWidth = pChunk->nWidth;
			m_nHeight = pChunk->nHeight;
		}
	}
	CEnemy::OnAddedToStage();
}

void CLimbsController::Kill()
{
	if( m_bKilled )
		return;
	for( CLimbs* p : m_vecLimbs )
	{
		p->Kill();
	}
	m_vecLimbs.clear();
	m_bKilled = true;
	KillEft();
	m_nKillEftCount = 15;
	m_nKillEftCD = 10;

	CChunkObject* pParChunk = NULL;
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CControlRoom>( pParent );
		if( pChunk && pChunk->GetController() )
		{
			pChunk->GetController()->KillAll();
		}
	}
}

void CLimbsController::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	if( m_bKilled )
	{
		m_nKillEftCD--;
		if( !m_nKillEftCD )
		{
			KillEft();
			m_nKillEftCount--;
			if( !m_nKillEftCount )
			{
				SetParentEntity( NULL );
				return;
			}
			m_nKillEftCD = 10;
		}
		return;
	}

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( m_attackRect.Contains( globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() ) ) )
		{
			for( int i = 1; i < m_vecLimbs.size() - 1; i++ )
			{
				CLimbs* p = m_vecLimbs[i];
				if( !( p->IsOperateable( CVector2( 0, 0 ) ) & 2 ) )
					p->Operate( CVector2( 0, 0 ) );
			}
		}
	}
}

void CLimbsController::KillEft()
{
	CVector2 ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	for( int i = 0; i < m_nWidth; i++ )
	{
		for( int j = 0; j < m_nHeight; j++ )
		{
			auto pEffect = SafeCast<CEffectObject>( m_pExpEft->GetRoot()->CreateInstance() );
			pEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
			pEffect->SetRotation( m_nDir * PI * 0.5f );
			pEffect->SetPosition( globalTransform.GetPosition() + ( CVector2( i + 0.5f, j + 0.5f ) - ofs[m_nDir] * 0.5f ) * 32 );
			pEffect->SetState( 2 );
		}
	}
}


void CLimbs1::Init( const CVector2 & size, SChunk* pPreParent )
{
	int32 nWidth = size.x / 8 - 1;
	int32 nHeight = size.y / 8 - 1;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_vecMask.resize( nWidth * nHeight );
	vector<TVector2<int32> > vec;
	for( int j = 0; j < nHeight; j++ )
	{
		for( int i = 0; i < nWidth; i++ )
		{
			vec.push_back( TVector2<int32>( i, j ) );
		}
	}
	SRand::Inst().Shuffle( vec );

	uint8 k = SRand::Inst().Rand( 0, 4 );
	CRectangle localBound( 0, 0, 0, 0 );
	for( auto& p : vec )
	{
		if( m_vecMask[p.x + p.y * nWidth] )
			continue;
		uint8 nSize = SRand::Inst().Rand( 0, 2 ) ? 3 : 2;
		auto rect = PutRect( m_vecMask, nWidth, nHeight, p, TVector2<int32>( nSize, nSize ), TVector2<int32>( nSize, nSize ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
		if( rect.width <= 0 )
			continue;
		
		SElem elem;
		elem.m_element2D.rect = CRectangle( ( rect.x + rect.width * 0.5f - 2 ) * 8 + 4, ( rect.y + rect.height * 0.5f - 2 ) * 8 + 4, 32, 32 );
		localBound = localBound.width == 0 ? elem.m_element2D.rect : elem.m_element2D.rect + localBound;
		elem.nX = SRand::Inst().Rand( 0, 4 );
		elem.nY = nSize == 2 ? 0 : 1;
		elem.nFrame = SRand::Inst().Rand( 0, 8 );
		m_elems.push_back( elem );
		m_vecKillSpawn.push_back( pair<CVector2, uint8>( elem.m_element2D.rect.GetCenter(), k ) );
		if( nSize == 3 )
			m_vecKillSpawn.push_back( pair<CVector2, uint8>( elem.m_element2D.rect.GetCenter(), ( k + 2 ) % 4 ) );
		k++;
		if( k >= 4 )
			k = 0;
	}
	m_attackRect.width += size.x;
	m_attackRect.height += size.y;
	SetLocalBound( localBound );
}

void CLimbs1::OnAddedToStage()
{
	auto p = static_cast<CImage2D*>( GetRenderObject() );
	m_pImg = p;
	SetRenderObject( NULL );
	if( CMyLevel::GetInst() )
		SetRenderParentBefore( CMyLevel::GetInst()->GetChunkEffectRoot() );
	GetStage()->RegisterAfterHitTest( m_nAITick, &m_onTick );
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			break;
		}
	}
}

void CLimbs1::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
}

void CLimbs1::UpdateRendered( double dTime )
{
	m_fTime += dTime * 2.0f;
	m_fTime -= floor( m_fTime );
	int32 nFrame = floor( m_fTime * 8 );
	for( int i = 0; i < m_elems.size(); i++ )
	{
		auto& elem = m_elems[i];
		elem.m_element2D.worldMat = globalTransform;
		int32 nFrame1 = ( nFrame + elem.nFrame ) & 7;
		elem.m_element2D.texRect = CRectangle( elem.nX + ( nFrame1 >> 2 ) * 4, elem.nY + ( nFrame1 & 3 ) * 2, 1, 1 ) * 0.125f;
	}
}

void CLimbs1::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
		{
			for( int i = 0; i < m_elems.size(); i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pColorDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetColorParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		else if( pGUIDrawable )
		{
			for( int i = 0; i < m_elems.size(); i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pGUIDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetGUIParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem, 1 );
			}
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
		{
			for( int i = 0; i < m_elems.size(); i++ )
			{
				auto& elem = m_elems[i].m_element2D;
				elem.SetDrawable( pOcclusionDrawable );
				static_cast<CImage2D*>( m_pImg.GetPtr() )->GetOcclusionParam( elem.pInstData, elem.nInstDataSize );
				context.AddElement( &elem );
			}
		}
		break;
	default:
		break;
	}
}

void CLimbs1::Kill()
{
	if( m_bKilled )
		return;
	m_bKilled = true;
	CVector2 dir[] = { { 1, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 } };
	for( auto& p : m_vecKillSpawn )
	{
		auto pKillSpawn = SafeCast<CCharacter>( m_pKillSpawn->GetRoot()->CreateInstance() );
		pKillSpawn->SetPosition( globalTransform.MulVector2Pos( p.first ) );
		pKillSpawn->SetVelocity( dir[p.second] * 300 );
		pKillSpawn->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
}

void CLimbs1::OnTick()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer && m_attackRect.Contains( globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() ) ) )
	{
		CVector2 center = globalTransform.MulVector2Pos( m_attackRect.GetCenter() );
		CVector2 dir = pPlayer->GetPosition() - center;
		dir.Normalize();
		int i = 0;
		for( int y = 0; y < m_nHeight; y++ )
		{
			for( int x = 0; x < m_nWidth; x++, i++ )
			{
				if( !m_vecMask[i] )
					continue;
				auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
				pBullet->SetCreator( GetParentEntity() );
				pBullet->SetPosition( globalTransform.MulVector2Pos( CVector2( x * 8 + 4, y * 8 + 4 ) ) );
				pBullet->SetVelocity( dir * m_fBulletSpeed );
				pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			}
		}
		GetStage()->RegisterAfterHitTest( m_nAttackCD, &m_onTick );
	}
	else
		GetStage()->RegisterAfterHitTest( m_nAITick, &m_onTick );
}

void CLimbsHook::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			SetRenderParentBefore( pChunk->GetParentEntity() );
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			break;
		}
	}
}

void CLimbsHook::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	if( m_pHooked )
		m_pHooked->EndHooked();
	m_pHooked = NULL;
	CEnemy::OnRemovedFromStage();
}

void CLimbsHook::Kill()
{
	KillAttackEft();
	CEnemy::Kill();
}

void CLimbsHook::OnDetach()
{
	m_pHooked = NULL;
}

void CLimbsHook::OnEndHook()
{
	if( m_pHooked )
	{
		CVector2 d = m_pHooked->GetPosition() - globalTransform.GetPosition();
		float fAngle0 = atan2( d.y, d.x );
		for( int i = 0; i < 9; i++ )
		{
			auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
			float fAngle = fAngle0 + ( i - 4 + SRand::Inst().Rand( -0.3f, 0.3f ) ) * 0.15f;
			pBullet->SetPosition( globalTransform.GetPosition() );
			pBullet->SetRotation( fAngle );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 200 );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
	}
}

void CLimbsHook::KillAttackEft()
{
	if( m_fEftLen <= 0 )
		return;
	if( m_pKillEffect )
	{
		CVector2 dir( cos( m_pLimbsAttackEft->r ), sin( m_pLimbsAttackEft->r ) );
		for( float i = m_fKillEftDist * 0.5f; i < m_fEftLen; i += m_fKillEftDist )
		{
			auto pKillEffect = SafeCast<CEffectObject>( m_pKillEffect->GetRoot()->CreateInstance() );
			pKillEffect->SetState( 2 );
			pKillEffect->SetPosition( globalTransform.GetPosition() + dir * i );
			pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
	}

	SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->DestroyAttackEft();
	m_nState = 0;
	m_fEftLen = 0;
}

void CLimbsHook::OnTickBeforeHitTest()
{
	if( m_nState == 2 )
	{
		if( m_pHooked )
		{
			if( !m_pHooked->GetStage() )
			{
				OnEndHook();
				m_pHooked->EndHooked();
				m_pHooked = NULL;
			}
			else
			{
				SHitProxyPolygon polygon( m_rectDetach );
				GetStage()->GetHitTestMgr().CalcBound( &polygon, m_lastMatrix );
				if( !m_pHooked->HitTest( &polygon, m_lastMatrix ) )
				{
					OnEndHook();
					m_pHooked->EndHooked();
					m_pHooked = NULL;
				}
				else
				{
					CVector2 dir( cos( m_pLimbsAttackEft->r ), sin( m_pLimbsAttackEft->r ) );
					float dLen = Min( m_fEftLen / GetStage()->GetElapsedTimePerTick(), m_fBackSpeed );
					if( !m_pHooked->Hooked( dir * -dLen ) )
					{
						OnEndHook();
						m_pHooked->EndHooked();
						m_pHooked = NULL;
					}
				}
			}
		}
	}
}

void CLimbsHook::OnTickAfterHitTest()
{
	if( m_nState == 0 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( !pPlayer )
		{
			GetStage()->RegisterAfterHitTest( m_nAITick, &m_tickAfterHitTest );
			return;
		}
		CVector2 pos = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
		if( pos.Length2() > m_fDetectDist * m_fDetectDist )
		{
			GetStage()->RegisterAfterHitTest( m_nAITick, &m_tickAfterHitTest );
			return;
		}
		if( pos.Length2() > m_fAttackDist * m_fAttackDist )
		{
			GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
			return;
		}

		m_nState = 1;
		m_fEftLen = 8;
		m_pLimbsAttackEft->SetRotation( atan2( pos.y, pos.x ) );
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->CreateAttackEft( GetRenderObject(), NULL, ( 1 << 0 ) | ( 1 << 3 ) | ( 1 << 4 ) | ( 1 << 7 ) );
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->SetAttackEftLen( m_fEftLen );
		GetStage()->RegisterAfterHitTest( m_nAttackTime, &m_tickAfterHitTest );
		return;
	}
	else if( m_nState == 1 )
	{
		GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
		m_fEftLen = Min( m_fMaxLen, m_fEftLen + m_fStretchSpeed * GetStage()->GetElapsedTimePerTick() );
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->SetAttackEftLen( m_fEftLen );

		SHitProxyPolygon polygon( m_rectHook );
		CMatrix2D trans;
		trans.Translate( m_fEftLen, 0 );
		trans = m_pLimbsAttackEft->globalTransform * trans;
		m_lastMatrix = trans;
		vector<CReference<CEntity> > result;
		GetStage()->MultiHitTest( &polygon, trans, result );
		CCharacter* pHooked = NULL;
		for( CEntity* pEntity : result )
		{
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic || pEntity->GetHitType() == eEntityHitType_Platform )
			{
				auto pBlockObject = SafeCast<CBlockObject>( pEntity );
				if( pBlockObject && pBlockObject->GetBlock()->eBlockType != eBlockType_Block )
					continue;
				m_nState = 2;
				break;
			}

			if( !pHooked )
			{
				auto pCharacter = SafeCast<CCharacter>( pEntity );
				if( pCharacter && pCharacter->StartHooked( this ) )
				{
					pHooked = pCharacter;
				}
			}
		}

		if( m_nState == 1 )
		{
			if( pHooked )
			{
				GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
				m_pHooked = pHooked;
				m_nState = 2;
			}
			else if( m_fEftLen >= m_fMaxLen )
				m_nState = 2;
		}
	}
	else
	{
		m_fEftLen = Max( 0.0f, m_fEftLen - m_fBackSpeed * GetStage()->GetElapsedTimePerTick() );

		if( m_pHooked )
		{
			CMatrix2D trans;
			trans.Translate( m_fEftLen, 0 );
			trans = m_pLimbsAttackEft->globalTransform * trans;
			m_lastMatrix = trans;
			GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
		}
		if( m_fEftLen <= 0 )
		{
			SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->DestroyAttackEft();
			if( m_pHooked )
			{
				OnEndHook();
				m_pHooked->EndHooked();
				m_pHooked = NULL;
			}
			m_nState = 0;
			GetStage()->RegisterAfterHitTest( m_nAttackTime1, &m_tickAfterHitTest );
		}
		else
		{
			SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->SetAttackEftLen( m_fEftLen );
			GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
		}
	}
}

void CManChunk1::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			SetRenderParentBefore( pChunk->GetParentEntity() );
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			m_nType = 1;
			break;
		}
	}
	SafeCast<CManChunkEft>( m_pManChunkEft.GetPtr() )->Set( 8 );
	if( CMyLevel::GetInst() )
		m_pManChunkEft->SetRenderParentAfter( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	auto pCircle = static_cast<SHitProxyCircle*>( m_pEnemyPart ? m_pEnemyPart->Get_HitProxy() : Get_HitProxy() );
	m_fHit0 = pCircle->fRadius;
}

void CManChunk1::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	CEnemy::OnRemovedFromStage();
}

void CManChunk1::Damage( SDamageContext & context )
{
	if( m_nState == 0 )
	{
		if( !m_nType )
			Crush();
		else
			m_nState = 1;
	}
	CEnemy::Damage( context );
}

void CManChunk1::Kill()
{
	m_pManChunkEft->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	m_pManChunkEft->SetPosition( m_pManChunkEft->globalTransform.GetPosition() );
	SafeCast<CManChunkEft>( m_pManChunkEft.GetPtr() )->Kill();
	CEnemy::Kill();
}

void CManChunk1::Crush()
{
	if( !m_nType )
	{
		m_bCrushed = true;
		m_nState = 1;
	}
	else
		CEnemy::Crush();
}

void CManChunk1::OnTickBeforeHitTest()
{
	CEnemy::OnTickBeforeHitTest();
	if( m_nCDLeft )
		m_nCDLeft--;
	auto p = SafeCast<CManChunkEft>( m_pManChunkEft.GetPtr() );
	float f = p->GetRadius();
	float f0 = f;
	if( m_nState == 1 )
	{
		f = Min( f + m_fSpeed * GetStage()->GetElapsedTimePerTick(), m_fMaxRadius );
		if( f == m_fMaxRadius )
			m_nState = 2;
	}
	else
	{
		if( !m_nCDLeft && f > 8.0f )
		{
			SBarrageContext context;
			context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
			context.vecBulletTypes.push_back( m_pBullet1.GetPtr() );
			context.nBulletPageSize = 4;

			class _CBarrage : public CBarrage
			{
			public:
				_CBarrage( const SBarrageContext& context ) : CBarrage( context ), m_nState( 0 ), m_nTime( 0 ) {}
			protected:
				virtual void OnTickAfterHitTest() override
				{
					if( m_nState == 0 )
					{
						if( m_nTime == 0 )
						{
							InitBullet( 0, -1, -1, CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), false,
								SRand::Inst().Rand( -PI, PI ), SRand::Inst().Rand( -6.0f, 6.0f ) );
							for( int i = 1; i <= 3; i++ )
							{
								InitBullet( i, 1, 0, CVector2( SRand::Inst().Rand( -8.0f, 8.0f ), SRand::Inst().Rand( -8.0f, 8.0f ) ),
									CVector2( 0, 0 ), CVector2( 0, 0 ) );
							}
						}
					}
					m_nTime++;
					if( m_nState == 0 )
					{
						CPlayer* pPlayer = GetStage()->GetPlayer();
						if( pPlayer )
						{
							auto pContext = GetBulletContext( 0 );
							pContext->SetBulletMove( CVector2( 0, 0 ), CVector2( 0, 0 ) );
							CVector2 dPos = pPlayer->GetPosition() - ( pContext->p0 + GetPosition() );
							if( m_nTime >= 30 && dPos.Length2() <= ( 50 + m_nTime * 2 ) * ( 50 + m_nTime * 2 ) )
							{
								m_nState = 1;
								m_nTime = 8;
								m_dir = dPos;
								if( m_dir.Normalize() < 0.01f )
									m_dir = CVector2( 1, 0 );
								m_n1 = SRand::Inst().Rand( 0, 2 ) * 2 - 1;
							}
							else
							{
								dPos.Normalize();
								pContext->SetBulletMove( dPos * Min( m_nTime, 120u ), CVector2( 0, 0 ) );
							}
						}
					}
					if( m_nState >= 1 && m_nState <= 3 )
					{
						if( m_nTime == 8 )
						{
							auto pContext0 = GetBulletContext( 0 );
							auto pContext = GetBulletContext( m_nState );
							if( pContext->IsValid() && pContext->pEntity )
							{
								float fAngle = m_n1 * ( m_nState - 2 ) * 0.3f;
								CVector2 dir( cos( fAngle ), sin( fAngle ) );
								dir = CVector2( dir.x * m_dir.x - dir.y * m_dir.y, dir.x * m_dir.y + dir.y * m_dir.x );
								InitBullet( m_nState, 0, -1, pContext0->p0, dir * ( 250 - m_nState * 50 ), CVector2( 0, 0 ), false,
									atan2( dir.y, dir.x ), m_n1 * 5 );
							}
							m_nState++;
							m_nTime = 0;
						}
					}
					if( m_nState > 3 && m_nTime == 2 )
						StopNewBullet();

					CBarrage::OnTickAfterHitTest();
				}

				uint8 m_nState;
				uint32 m_nTime;
				CVector2 m_dir;
				int8 m_n1;
			};
			auto pBarrage = new _CBarrage( context );
			pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			float r = sqrt( SRand::Inst().Rand( 0.0f, 1.0f ) ) * f;
			float fAngle = SRand::Inst().Rand( -PI, PI );
			pBarrage->SetPosition( globalTransform.GetPosition() + CVector2( cos( fAngle ), sin( fAngle ) ) * r );
			pBarrage->Start();
			m_nCDLeft = m_nAttackCD;
			if( m_bCrushed )
			{
				SDamageContext dmg;
				dmg.nDamage = 10;
				dmg.nType = 0;
				dmg.nSourceType = 0;
				dmg.hitPos = dmg.hitDir = CVector2( 0, 0 );
				dmg.nHitType = -1;
				Damage( dmg );
				return;
			}
		}
		if( m_nState == 0 )
			f = Max( f - m_fSpeed1 * GetStage()->GetElapsedTimePerTick(), 8.0f );
	}
	if( f != f0 )
	{
		p->Set( f );
		CEntity* pHit = this;
		if( m_pEnemyPart )
			pHit = m_pEnemyPart;
		auto pCircle = static_cast<SHitProxyCircle*>( pHit->Get_HitProxy() );
		pCircle->fRadius = Max( f, m_fHit0 );
		pHit->SetDirty();
	}
}

void CManChunk1::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();
	if( !m_nType && !m_bCrushed )
	{
		m_moveData.UpdateMove( this );
		if( !m_bCrushed && y <= m_fKillY )
		{
			Kill();
			return;
		}
	}
	if( m_nState == 0 )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer && ( pPlayer->GetPosition() - globalTransform.GetPosition() ).Length2() <= m_fAttackDist * m_fAttackDist )
		{
			if( !m_nType )
				Crush();
			else
				m_nState = 1;
		}
	}
	else if( m_nState == 2 && !m_bCrushed )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer && ( pPlayer->GetPosition() - globalTransform.GetPosition() ).Length2() > m_fStopDist * m_fStopDist )
			m_nState = 0;
	}
}

void CManChunkEye::AIFunc()
{
	if( !CMyLevel::GetInst() )
		return;
	m_pAI->Yield( 0, true );
	CVector2 target( 0, 0 );
	while( 1 )
	{
		int32 nCD = m_nCD;
		int32 nFireCD = m_nFireCD;
		float fAngle0;
		float dAngle;
		TTempEntityHolder<CLightning> pLightning;
		for( int i = 0; i < m_nTraceTime1; )
		{
			if( nCD )
				nCD--;
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 p = pPlayer->GetPosition() - globalTransform.GetPosition();
				CVector2 d = p - target;
				float l = d.Length();
				float dl = m_fTraceSpeed * GetStage()->GetElapsedTimePerTick();
				target = target + d * dl / Max( dl, l );
				if( i < m_nTraceTime )
				{
					if( pLightning )
					{
						if( p.Length2() > m_fRange1 * m_fRange1 || ( target - p ).Length2() > m_fDist1 * m_fDist1 )
						{
							pLightning->SetParentEntity( NULL );
							pLightning = NULL;
						}
					}
					else if( !nCD )
					{
						if( p.Length2() <= m_fRange * m_fRange && ( target - p ).Length2() <= m_fDist * m_fDist )
						{
							pLightning = SafeCast<CLightning>( m_pLightning0->GetRoot()->CreateInstance() );
							pLightning->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
						}
					}
				}
				else if( pLightning )
				{
					if( i == m_nTraceTime )
					{
						pLightning->SetParentEntity( NULL );
						pLightning = NULL;
						pLightning = SafeCast<CLightning>( m_pLightning->GetRoot()->CreateInstance() );
						pLightning->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
						fAngle0 = SRand::Inst().Rand( -PI, PI );
						dAngle = m_fAngle *( SRand::Inst().Rand( 0, 2 ) * 2 - 1 );
					}
					nFireCD--;
					if( !nFireCD )
					{
						for( int i = 0; i < m_nBullet; i++ )
						{
							float fAngle = fAngle0 + i * PI / ( m_nBullet * 0.5f );
							CVector2 dir( cos( fAngle ), sin( fAngle ) );
							auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
							pBullet->SetCreator( this );
							pBullet->SetPosition( globalTransform.GetPosition() + target );
							pBullet->SetRotation( fAngle );
							pBullet->SetVelocity( dir * m_v );
							pBullet->SetAcceleration( dir * m_a );
							pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
						}
						fAngle0 += dAngle;
						nFireCD = m_nFireCD;
					}
				}

				auto pEft = SafeCast<CEyeEft>( m_pEft.GetPtr() );
				pEft->SetTarget( target + globalTransform.GetPosition() );
				if( pLightning )
				{
					CVector2 ofs = pEft->GetCenter();
					pLightning->Set( this, this, ofs, target, -1, -1 );
					i++;
				}
				else
				{
					i = 0;
					nFireCD = m_nFireCD;
				}
			}
			m_pAI->Yield( 0, true );
		}
	}
}

void CManChunkEyeBouns::OnAddedToStage()
{
	auto pEft = SafeCast<CEyeEft>( m_pEft.GetPtr() );
	pEft->SetEyeColor( 1 );
	CEnemy::OnAddedToStage();
}

void CManChunkEyeBouns::Damage( SDamageContext & context )
{
	DEFINE_TEMP_REF_THIS();
	CVector2 pos = globalTransform.GetPosition() + SafeCast<CEyeEft>( m_pEft.GetPtr() )->GetCenter();
	int32 nSpawn0 = ( m_nHp * m_nSpawn + m_nMaxHp - 1 ) / m_nMaxHp;
	CEnemy::Damage( context );
	int32 nSpawn1 = ( m_nHp * m_nSpawn + m_nMaxHp - 1 ) / m_nMaxHp;
	for( int i = nSpawn1; i < nSpawn0; i++ )
	{
		for( int j = 0; j < m_nSpawnCount; j++ )
		{
			auto p = SafeCast<CCharacter>( m_pSpawn->GetRoot()->CreateInstance() );
			p->SetPosition( pos );
			CVector2 vel = m_target;
			if( vel.Normalize() < 0.0001f )
				vel = CVector2( 0, -1 );
			float fAngle = ( SRand::Inst().Rand( -0.5f, 0.5f ) + j - ( m_nSpawnCount - 1 ) * 0.5f ) * m_fSpawnAngle / m_nSpawnCount;
			p->SetVelocity( vel * m_fSpawnSpeed );
			p->SetRotation( atan2( vel.y, vel.x ) );
			p->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
	}
}

void CManChunkEyeBouns::OnTickAfterHitTest()
{
	CEnemy::OnTickAfterHitTest();

	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		CVector2 p = pPlayer->GetPosition() - globalTransform.GetPosition();
		CVector2 d = p - m_target;
		float l = d.Length();
		float dl = m_fTraceSpeed * GetStage()->GetElapsedTimePerTick();
		m_target = m_target + d * dl / Max( dl, l );
	}

	auto pEft = SafeCast<CEyeEft>( m_pEft.GetPtr() );
	pEft->SetTarget( m_target + globalTransform.GetPosition() );
}

void CArmRotator::OnAddedToStage()
{
	if( !CMyLevel::GetInst() )
	{
		bVisible = false;
		return;
	}
	CEnemy::OnAddedToStage();
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			break;
		}
	}
	SetRenderParentAfter( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	SafeCast<CArmEft>( m_pEft.GetPtr() )->Init( CVector2( 0, m_fRad ), CVector2( 0, 0 ), CVector2( 8, 8 ), CVector2( -8, -8 ), TVector2<int32>( 3, 3 ), NULL );
	m_pEnd->SetPosition( CVector2( 0, m_fRad ) );
	m_fAngle = PI * 0.5f;
	m_dAngle = m_fASpeed * ( SRand::Inst().Rand( 0, 2 ) * 2 - 1 );

	CVector2 verts[4] = { { -m_fWidth * 0.5f, 0 }, { m_fWidth * 0.5f, 0 }, { m_fWidth * 0.5f, m_fRad }, { -m_fWidth * 0.5f, m_fRad } };
	AddPolygon( 4, verts );
}

void CArmRotator::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	CEnemy::OnRemovedFromStage();
}

void CArmRotator::Kill()
{
	if( m_pKillEffect )
	{
		CVector2 dir( cos( m_fAngle ), sin( m_fAngle ) );
		for( float i = m_fKillEftDist * 0.5f; i < m_fRad; i += m_fKillEftDist )
		{
			auto pKillEffect = SafeCast<CEffectObject>( m_pKillEffect->GetRoot()->CreateInstance() );
			pKillEffect->SetState( 2 );
			pKillEffect->SetPosition( globalTransform.GetPosition() + dir * i );
			pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		}
	}
	CEnemy::Kill();
}

void CArmRotator::OnTickBeforeHitTest()
{
	CEnemy::OnTickBeforeHitTest();
	m_fAngle = NormalizeAngle( m_fAngle + m_dAngle * GetStage()->GetElapsedTimePerTick() );
	CVector2 target( cos( m_fAngle ) * m_fRad, sin( m_fAngle ) * m_fRad );
	m_pEnd->SetPosition( target );
	SafeCast<CArmEft>( m_pEft.GetPtr() )->Set( target, CVector2( 0, 0 ) );

	CVector2 vec1( sin( m_fAngle ) * m_fWidth, -cos( m_fAngle ) * m_fWidth );
	auto pRect = static_cast<SHitProxyPolygon*>( Get_HitProxy() );
	pRect->vertices[0] = vec1 * -1;
	pRect->vertices[1] = vec1 * 1;
	pRect->vertices[2] = vec1 * 1 + target;
	pRect->vertices[3] = vec1 * -1 + target;
	pRect->CalcNormals();
	GetStage()->GetHitTestMgr().ReAdd( this );
}

void CManChunkEgg::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	CChunkObject* pParChunk = NULL;
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
			pParChunk = pChunk;
		else
			break;
	}
	if( pParChunk )
	{
		SetRenderParentBefore( pParChunk->GetParentEntity() );
		pParChunk->RegisterKilledEvent( &m_onChunkKilled );
	}
	SafeCast<CManChunkEft>( m_pManChunkEft.GetPtr() )->Set( static_cast<SHitProxyCircle*>( Get_HitProxy() )->fRadius );
}

void CManChunkEgg::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	CEnemy::OnRemovedFromStage();
}

void CManChunkEgg::Damage( SDamageContext & context )
{
	CEnemy* pEnemy = SafeCast<CEnemy>( GetParentEntity() );
	if( pEnemy )
	{
		uint8 nHitType = m_nHitType;
		context.nDamage = ceil( context.nDamage * ( 1 - m_fDefence ) );
		pEnemy->Damage( context );
		context.nHitType = nHitType;
		return;
	}
	CEnemy::Damage( context );
}

void CManChunkEgg::Kill()
{
	m_pManChunkEft->SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	m_pManChunkEft->SetPosition( m_pManChunkEft->globalTransform.GetPosition() );
	SafeCast<CManChunkEft>( m_pManChunkEft.GetPtr() )->Kill();
	CEnemy::Kill();
}

bool CManChunkEgg::Hatch()
{
	auto p = SafeCast<CManChunkEft>( m_pManChunkEft.GetPtr() );
	float f = p->GetRadius();
	float f0 = f;
	f = Min( f + m_fSpeed * GetStage()->GetElapsedTimePerTick(), m_fMaxRadius );
	if( f != f0 )
	{
		p->Set( f );
		auto pCircle = static_cast<SHitProxyCircle*>( Get_HitProxy() );
		pCircle->fRadius = f;
		SetDirty();
	}
	if( f >= m_fMaxRadius )
	{
		SetPosition( globalTransform.GetPosition() );
		SetParentAfterEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
		m_pManChunkEft->SetRenderParent( this );
		m_pAI = new AI();
		m_pAI->SetParentEntity( this );
		return true;
	}
	return false;
}

void CManChunkEgg::AIFunc()
{
	for( int i = 1; i <= 64; i++ )
	{
		float l = floor( 32.0f * i / 64 * 0.5f + 0.5f ) * 2;
		auto p = SafeCast<CManChunkEft>( m_pManChunkEft.GetPtr() );
		p->SetBound( CRectangle( -m_fMaxRadius, -m_fMaxRadius + l, m_fMaxRadius * 2, m_fMaxRadius * 2 - l ) );
		p->Set( m_fMaxRadius );
		m_pAI->Yield( 0, true );
	}
	float l = sqrt( m_fMaxRadius * m_fMaxRadius - 32.0f * 32.0f ) - 16;

	CVector2 moveTarget = GetPosition();
	int32 nTime = 0;
	int32 nTime1 = 0;
	while( 1 )
	{
		if( nTime )
			nTime--;
		if( !nTime )
		{
			CPlayer* pPlayer = GetStage()->GetPlayer();
			if( pPlayer )
			{
				CVector2 playerPos = pPlayer->GetPosition();
				moveTarget.x += SRand::Inst().Rand( 0.0f, 1.0f ) * 64.0f - 32.0f + ( playerPos.x - moveTarget.x ) * 0.05f;
				moveTarget.x = Min( Max( moveTarget.x, CMyLevel::GetInst()->GetBoundWithLvBarrier().x + 64.0f ), CMyLevel::GetInst()->GetBoundWithLvBarrier().GetRight() - 64.0f );
				moveTarget.y = playerPos.y + SRand::Inst().Rand( 360.0f, 400.0f );
				moveTarget.y = Min( 640.0f, Min( Max( moveTarget.y, CMyLevel::GetInst()->GetBoundWithLvBarrier().y + 64.0f ), CMyLevel::GetInst()->GetBoundWithLvBarrier().GetBottom() - 64.0f ) );
			}
			nTime = 30;
		}
		m_flyData.UpdateMove( this, moveTarget );

		if( nTime1 )
			nTime1--;
		if( !nTime1 )
		{
			auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
			pBullet->SetPosition( GetPosition() + CVector2( SRand::Inst().Rand( -l, l ), -m_fMaxRadius + 32 ) );
			pBullet->SetAcceleration( CVector2( 0, -200 ) );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
			nTime1 = 15;

			SDamageContext dmgContext;
			dmgContext.nDamage = 1;
			dmgContext.nType = 0;
			dmgContext.nSourceType = 0;
			dmgContext.hitPos = dmgContext.hitDir = CVector2( 0, 0 );
			dmgContext.nHitType = -1;
			Damage( dmgContext );
		}
		m_pAI->Yield( 0, true );
	}
}


void CArmAdvanced::OnAddedToStage()
{
	if( !CMyLevel::GetInst() )
	{
		bVisible = false;
		return;
	}
	for( auto pParent = GetParentEntity(); pParent; pParent = pParent->GetParentEntity() )
	{
		auto pChunk = SafeCast<CChunkObject>( pParent );
		if( pChunk )
		{
			pChunk->RegisterKilledEvent( &m_onChunkKilled );
			break;
		}
	}
	SetRenderParentAfter( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	CEnemyTemplate::OnAddedToStage();
}

void CArmAdvanced::OnRemovedFromStage()
{
	if( m_onChunkKilled.IsRegistered() )
		m_onChunkKilled.Unregister();
	CEnemyTemplate::OnRemovedFromStage();
}

void CArmAdvanced::Damage( SDamageContext & context )
{
	if( m_vecItems.size() > 1 )
		m_nDamage += context.nDamage;
}

void CArmAdvanced::Kill()
{
	while( m_vecItems.size() )
		KillItem();
	CEnemy::Kill();
}

void CArmAdvanced::AIFunc()
{
	do
	{
		m_pAI->Yield( 0, true );
	} while( !CMyLevel::GetInst()->GetBoundWithLvBarrier().Contains( globalTransform.GetPosition() ) );
	Spawn();

	auto AutoTarget = [=] () {
		CRectangle bound = CMyLevel::GetInst()->GetBoundWithLvBarrier();
		bound = CRectangle( bound.x + 32, bound.y + 32, bound.width - 64, bound.height - 64 );
		int32 nSegs = m_vecItems.size();
		CVector2 p = globalTransform.GetPosition();
		float l = Max( 2, nSegs ) * m_fArmLen * SRand::Inst().Rand( 0.7f, 0.85f );
		CVector2 p1;
		p1.x = p.x > bound.GetCenterX() ? bound.x : bound.GetRight();
		p1.y = bound.y + SRand::Inst().Rand( bound.height / 2, bound.height );
		if( ( p1 - p ).Length2() > l )
			p1 = p + ( p1 - p ) * l / ( p1 - p ).Length();
		return p1;
	};

	CVector2 p = AutoTarget();
	CVector2 p0 = globalTransform.GetPosition();
	int32 nState = 0;
	int32 nAttachCD = 400;
	CReference<CEntity> pLastTarget;
	while( 1 )
	{
		if( nState == 0 )
		{
			CReference<COperateableAssembler> pTarget;
			while( 1 )
			{
				auto& item = m_vecItems.back();
				if( nAttachCD )
					nAttachCD--;
				if( item.nState )
				{
					if( item.fArmLen == m_fArmLen && m_vecItems.size() < 4 )
					{
						auto pBloodConsumer = SafeCast<CBloodConsumer>( m_pBloodConsumer.GetPtr() );
						if( pBloodConsumer->GetBloodCount() >= pBloodConsumer->GetMaxCount() )
						{
							pBloodConsumer->SetBloodCount( 0 );
							Spawn();
							p = AutoTarget();
						}
					}
				}
				else if( !pTarget )
				{
					if( !nAttachCD )
					{
						if( m_vecItems.size() >= 2 && item.fArmLen == m_fArmLen )
						{
							auto& assemblers = COperateableAssembler::GetAllInsts();
							float lMax = m_vecItems.size() * m_fArmLen * 0.9f;
							float lMin = m_vecItems.size() * m_fArmLen * 0.1f;
							float fMin = FLT_MAX;
							for( COperateableAssembler* p : assemblers )
							{
								if( !p->CanAttach( this, item.pComponent ) )
									continue;
								CVector2 pos = p->globalTransform.GetPosition();
								if( !CMyLevel::GetInst()->GetBoundWithLvBarrier().Contains( pos ) )
									continue;
								CVector2 d = globalTransform.MulTVector2PosNoScale( pos );
								float l2 = d.Length2();
								if( l2 < lMin * lMin || l2 > lMax * lMax )
									continue;
								l2 = ( item.targetPos - d ).Length2();
								if( l2 < fMin )
								{
									fMin = p == pLastTarget ? FLT_MAX : l2;
									pTarget = p;
								}
							}
						}
					}
				}
				if( pTarget )
				{
					if( !pTarget->GetStage() || !pTarget->CanAttach( this, item.pComponent ) || item.nState )
					{
						p = AutoTarget();
						pTarget = NULL;
					}
					else
					{
						p = pTarget->globalTransform.GetPosition();
						auto d = globalTransform.MulTVector2PosNoScale( p );
						float lMax = m_vecItems.size() * m_fArmLen * 0.94f;
						float lMin = m_vecItems.size() * m_fArmLen * 0.06f;
						float l2 = d.Length2();
						if( l2 > lMax * lMax || l2 < lMin * lMin )
						{
							p = AutoTarget();
							pTarget = NULL;
						}
					}
				}

				float fSpeed = 50.0f;
				CVector2 d = p - p0;
				float l = d.Length();
				float l1 = fSpeed * GetStage()->GetElapsedTimePerTick();
				p0 = p0 + d * l1 / Max( l1, l );
				if( l1 >= l )
				{
					if( pTarget )
					{
						Attach( pTarget, CVector2( 0, 0 ) );
						nState = 1;
						break;
					}
					else
						p = AutoTarget();
				}
				Step( p0 );
				m_pAI->Yield( 0, true );
			}
		}
		else if( nState == 1 )
		{
			int32 nWaitTime = 0;
			while( 1 )
			{
				auto& item = m_vecItems.back();
				if( !item.pTarget || !item.pTarget->GetStage() )
				{
					item.pTarget = NULL;
					p = AutoTarget();
					nState = 0;
					break;
				}
				auto pTarget = SafeCast<COperateableAssembler>( item.pTarget.GetPtr() );
				if( SafeCast<COperateableAssembler>( item.pTarget.GetPtr() )->IsRunning() )
				{
					nWaitTime = 0;
					auto componentPos = item.pComponent->GetPosition();
					if( SafeCast<CManChunkEgg>( item.pComponent.GetPtr() )->Hatch() )
					{
						pTarget->OnEntityDetach();
						item.pTarget = NULL;
						item.targetPos = componentPos;
						Hatch( m_vecItems.size() - 1, 0 );
						p = AutoTarget();
						nState = 0;
						break;
					}
				}
				else
				{
					nWaitTime++;
					if( nWaitTime >= m_nWaitTime )
					{
						pLastTarget = pTarget;
						pTarget->OnEntityDetach();
						item.pTarget = NULL;
						item.targetPos = item.pComponent->GetPosition();
						p = AutoTarget();
						nState = 0;
						nAttachCD = 400;
						break;
					}
				}

				Step( p0 );
				m_pAI->Yield( 0, true );
			}
		}
	}
}

void CArmAdvanced::Step( CVector2& moveTarget )
{
	if( m_vecItems.size() >= 1 )
	{
		auto& item = m_vecItems.back();
		if( !item.pTarget )
		{
			CVector2 p0 = m_vecItems.size() > 1 ? m_vecItems[m_vecItems.size() - 2].targetPos : CVector2( 0, 0 );
			item.fArmLen = Min( m_fArmLen, item.fArmLen + m_fArmLenSpeed * GetStage()->GetElapsedTimePerTick() );
			CVector2 d = item.targetPos - p0;
			float l = d.Normalize();
			if( l < 1 )
				d = CVector2( 0, 1 );
			item.targetPos = p0 + d * item.fArmLen;
			item.pComponent->SetPosition( item.targetPos );
		}
	}

	CVector2* pPos = (CVector2*)alloca( sizeof( CVector2 ) * ( m_vecItems.size() + 1 ) );
	pPos[0] = CVector2( 0, 0 );
	int32 i0 = 0;
	for( int i = 0; i < m_vecItems.size(); i++ )
	{
		auto& item = m_vecItems[i];
		pPos[i + 1] = item.pComponent->GetPosition();
		auto p = item.pTarget ? globalTransform.MulTVector2PosNoScale( item.pTarget->globalTransform.MulVector2Pos( item.targetPos ) )
			: ( i == m_vecItems.size() - 1 ? globalTransform.MulTVector2PosNoScale( moveTarget ) : item.targetPos );
		if( item.pTarget || i == m_vecItems.size() - 1 )
		{
			IK( pPos + i0, i + 2 - i0, p );
			for( int j = i0; j <= i; j++ )
			{
				m_vecItems[j].targetPos = pPos[j + 1];
			}
			if( item.pTarget )
			{
				if( i == m_vecItems.size() - 1 )
					moveTarget = globalTransform.MulVector2Pos( item.targetPos );
				if( ( pPos[i + 1] - p ).Length2() >= 1 )
				{
					SafeCastToInterface<IAttachableSlot>( item.pTarget.GetPtr() )->OnEntityDetach();
					item.pTarget = NULL;
				}
			}
			if( item.pTarget )
				item.targetPos = item.pTarget->globalTransform.MulTVector2PosNoScale( globalTransform.MulVector2Pos( item.targetPos ) );

			i0 = i + 1;
		}
	}

	CVector2 p0( 0, 0 );
	for( auto& item : m_vecItems )
	{
		CVector2 p = item.pTarget ? globalTransform.MulTVector2PosNoScale( item.pTarget->globalTransform.MulVector2Pos( item.targetPos ) ) : item.targetPos;

		SafeCast<CArmEft>( item.pArmEft.GetPtr() )->Set( p, p0 );
		CVector2 vec1 = p0 - p;
		vec1.Normalize();
		vec1 = CVector2( vec1.y * m_fArmWidth, -vec1.x * m_fArmWidth );
		auto pRect = static_cast<SHitProxyPolygon*>( item.pArm->Get_HitProxy() );
		pRect->vertices[0] = p + vec1 * -1;
		pRect->vertices[1] = p + vec1 * 1;
		pRect->vertices[2] = p0 + vec1 * 1 ;
		pRect->vertices[3] = p0 + vec1 * -1;
		pRect->CalcNormals();
		GetStage()->GetHitTestMgr().Update( item.pArm );

		item.pComponent->SetPosition( p );
		p0 = p;
	}

	if( m_vecItems.size() > 1 && m_nDamage >= m_nMaxHp )
	{
		m_nDamage = 0;
		KillItem();
		moveTarget = globalTransform.MulVector2Pos( m_vecItems.back().targetPos );
	}
}

void CArmAdvanced::Spawn()
{
	m_nDamage = 0;
	m_vecItems.resize( m_vecItems.size() + 1 );
	auto& item = m_vecItems.back();
	if( m_vecItems.size() >= 2 )
	{
		auto& item1 = m_vecItems[m_vecItems.size() - 2];
		item.targetPos = item1.targetPos;
	}
	else
		item.targetPos = CVector2( 0, 0 );
	item.fArmLen = 0;

	auto pArm = SafeCast<CEntity>( m_pPrefabArm->GetRoot()->CreateInstance() );
	item.pArm = pArm;
	pArm->SetZOrder( -1 );
	pArm->SetParentEntity( this );
	auto pEft = SafeCast<CArmEft>( pArm->GetChildByName_Fast( "eft" ) );
	item.pArmEft = pEft;
	pEft->Init( item.targetPos, item.targetPos, CVector2( 8, 8 ), CVector2( -8, -8 ), TVector2<int32>( 3, 3 ), NULL );

	if( m_vecItems.size() > 1 )
	{
		item.nType = -1;
		item.nState = 0;
		auto pEgg = SafeCast<CEntity>( m_pPrefabEgg->GetRoot()->CreateInstance() );
		pEgg->SetParentEntity( this );
		pEgg->SetPosition( item.targetPos );
		item.pComponent = pEgg;
	}
	else
		Hatch( 0, 0 );
}

void CArmAdvanced::Hatch( int32 n, int8 nType )
{
	auto& item = m_vecItems[n];
	item.nType = nType;
	item.nState = 1;
	auto pComponent = SafeCast<CEntity>( m_pPrefabComponent->GetRoot()->CreateInstance() );
	pComponent->SetParentEntity( this );
	pComponent->SetPosition( item.targetPos );
	if( item.pComponent && item.pComponent->GetParentEntity() == this )
	{
		item.pComponent->SetParentEntity( NULL );
		item.pComponent = NULL;
	}
	item.pComponent = pComponent;
}

void CArmAdvanced::Attach( CEntity* pEntity, const CVector2& ofs )
{
	auto& item = m_vecItems.back();
	auto pAssembler = SafeCast<COperateableAssembler>( pEntity );
	if( pAssembler && pAssembler->CanAttach( this, item.pComponent ) )
	{
		item.pTarget = pEntity;
		item.targetPos = ofs;
		pAssembler->Attach( this, item.pComponent );
	}
}

void CArmAdvanced::KillItem()
{
	auto& item = m_vecItems.back();
	if( item.pTarget )
	{
		SafeCastToInterface<IAttachableSlot>( item.pTarget.GetPtr() )->OnEntityDetach();
		item.targetPos = globalTransform.MulTVector2PosNoScale( item.pTarget->globalTransform.MulVector2Pos( item.targetPos ) );
		item.pTarget = NULL;
	}
	if( item.pComponent )
	{
		if( item.pComponent->GetParentEntity() == this )
		{
			auto pChar = SafeCast<CCharacter>( item.pComponent.GetPtr() );
			if( pChar )
				pChar->Kill();
			else
				item.pComponent->SetParentEntity( NULL );
		}
		item.pComponent = NULL;
	}
	if( item.pArm )
	{
		if( item.pArm->GetParentEntity() )
		{
			if( m_pKillEffect )
			{
				CVector2 p1 = item.targetPos;
				CVector2 p0 = m_vecItems.size() > 1 ? m_vecItems[m_vecItems.size() - 2].targetPos: CVector2( 0, 0 );
				CVector2 d = p1 - p0;
				float l = d.Length();
				for( float i = m_fKillEftDist * 0.5f; i < l; i += m_fKillEftDist )
				{
					auto pKillEffect = SafeCast<CEffectObject>( m_pKillEffect->GetRoot()->CreateInstance() );
					pKillEffect->SetState( 2 );
					CVector2 p = p0 + d * i / l;
					pKillEffect->SetPosition( globalTransform.MulVector2Pos( p ) );
					pKillEffect->SetParentBeforeEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
				}
			}
			item.pArm->SetParentEntity( NULL );
		}
		item.pArm = NULL;
		item.pArmEft = NULL;
	}
	m_vecItems.resize( m_vecItems.size() - 1 );
}

void CArmAdvanced::OnSlotDetach( CEntity* pTarget )
{
	for( auto& item : m_vecItems )
	{
		if( item.pTarget == pTarget )
		{
			item.pTarget = NULL;
			item.targetPos = item.pComponent->GetPosition();
		}
	}
}

bool CCar::CanHit( CEntity * pEntity )
{
	if( m_pExcludeChunkObject )
	{
		auto pBlockObject = SafeCast<CBlockObject>( pEntity );
		if( pBlockObject )
		{
			if( pBlockObject->GetParent() == m_pExcludeChunkObject )
			{
				auto pBlock = pBlockObject->GetBlock();
				if( pBlock->nX >= m_excludeRect.x && pBlock->nY >= m_excludeRect.y && pBlock->nX < m_excludeRect.GetRight() && pBlock->nY < m_excludeRect.GetBottom() )
				{
					m_bHitExcludeChunk = true;
					return false;
				}
			}
		}
	}
	return true;
}

bool CCar::CanHit1( CEntity * pEntity, SRaycastResult & result )
{
	auto pBlockObject = SafeCast<CBlockObject>( pEntity );
	if( pBlockObject )
	{
		if( m_pExcludeChunkObject )
		{
			if( pBlockObject->GetParent() == m_pExcludeChunkObject )
			{
				auto pBlock = pBlockObject->GetBlock();
				if( pBlock->nX >= m_excludeRect.x && pBlock->nY >= m_excludeRect.y && pBlock->nX < m_excludeRect.GetRight() && pBlock->nY < m_excludeRect.GetBottom() )
				{
					m_bHitExcludeChunk = true;
					return false;
				}
			}
		}

		auto pChunkObject = SafeCast<CChunkObject>( pBlockObject->GetParentEntity() );
		if( pChunkObject )
		{
			if( pChunkObject->GetCrushCost() > 0 )
			{
				CVector2 dir( globalTransform.m00, globalTransform.m10 );
				CVector2 vel1 = dir * GetVelocity().Dot( dir );
				float fDmg = vel1.Dot( result.normal ) * -m_fHitDmg1Coef * 1000.0f / pChunkObject->GetCrushCost();
				if( fDmg > 0 )
				{
					pChunkObject->Damage( fDmg );
					if( !pChunkObject->GetParentEntity() )
						return false;
				}
			}
		}
	}
	return true;
}

void CCar::OnRemovedFromStage()
{
	CEnemy::OnRemovedFromStage();
	m_pExcludeChunkObject = NULL;
}

void CCar::Damage( SDamageContext & context )
{
	DEFINE_TEMP_REF_THIS();
	CEnemy::Damage( context );
	if( GetStage() && m_nHp <= m_nBurnHp && !m_pBurnEffect )
	{
		m_pBurnEffect = SafeCast<CEntity>( m_strBurnEffect->GetRoot()->CreateInstance() );
		m_pBurnEffect->SetParentEntity( this );
		m_pBurnEffect->SetRenderParentAfter( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );
	}
}

void CCar::Kill()
{
	auto pExplosion = SafeCast<CExplosion>( m_strExplosion->GetRoot()->CreateInstance() );
	pExplosion->SetPosition( GetPosition() );
	pExplosion->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Player ) );

	switch( m_nExpType )
	{
	case 0:
	{
		float r = SRand::Inst().Rand( -PI, PI );
		for( int i = 0; i < 24; i++ )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
			float fAngle = i * PI / 12 + r;
			pBullet->SetPosition( GetPosition() );
			pBullet->SetRotation( fAngle );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * ( 150 + 50 * ( i & 1 ) ) );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
		break;
	}
	case 1:
	{
		float r = SRand::Inst().Rand( -PI, PI );
		for( int i = 0; i < 6; i++ )
		{
			CBullet* pBullet = SafeCast<CBullet>( m_strBullet->GetRoot()->CreateInstance() );
			float fAngle = i * PI / 3 + r;
			pBullet->SetPosition( GetPosition() );
			pBullet->SetRotation( fAngle );
			pBullet->SetVelocity( CVector2( cos( fAngle ), sin( fAngle ) ) * 100 );
			pBullet->SetLife( 200 );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}
		break;
	}
	case 2:
	{
		SBarrageContext context;
		context.vecBulletTypes.push_back( m_strBullet.GetPtr() );
		context.nBulletPageSize = 50;

		CBarrage* pBarrage = new CBarrage( context );
		pBarrage->AddFunc( [] ( CBarrage* pBarrage )
		{
			float r = SRand::Inst().Rand( -PI, PI );
			int32 nBullet = 0;
			for( int i = 0; i < 2; i++ )
			{
				for( int j = 0; j < 25; j++ )
				{
					float v = j / 5.0f;
					v = ( v - floor( v ) ) * 2 - 1;
					v = v * v * 125 + 125;
					float fAngle = r + j * PI / 12;
					CVector2 dir( cos( fAngle ), sin( fAngle ) );
					pBarrage->InitBullet( nBullet++, 0, -1, CVector2( 0, 0 ), dir * v, CVector2( dir.y, -dir.x ) * v * ( i * 2 - 1 ) );
				}
				pBarrage->Yield( 15 );
			}
			pBarrage->StopNewBullet();
		} );
		pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		pBarrage->SetPosition( GetPosition() );
		pBarrage->Start();
		break;
	}
	}

	CEnemy::Kill();
}

void CCar::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CEnemy::OnTickAfterHitTest();
	CVector2 dir( globalTransform.m00, globalTransform.m10 );
	SetVelocity( GetVelocity() + dir * ( m_fAcc * GetStage()->GetElapsedTimePerTick() ) );
	m_moveData.UpdateMove( this );
	if( !GetStage() )
		return;

	if( !m_bHitExcludeChunk )
		m_pExcludeChunkObject = NULL;
	m_bHitExcludeChunk = false;

	if( !m_pExcludeChunkObject && !m_bSpawned )
	{
		if( m_bDoorOpen )
		{
			if( m_nSpawnManTimeLeft )
			{
				m_nSpawnManTimeLeft--;
				float fOpenAngle[4] = { 2.0f, -2.0f, -2.0f, 2.0f };
				for( int i = 0; i < 4; i++ )
				{
					m_pDoors[i]->SetRotation( fOpenAngle[i] * ( m_nSpawnManTime - m_nSpawnManTimeLeft ) / m_nSpawnManTime );
				}
			}
			if( !m_nSpawnManTimeLeft )
			{
				int8 nValidDoor = 0;
				int8 validDoors[4];
				for( int i = 0; i < 4; i++ )
				{
					CVector2 spawnPos = globalTransform.MulVector2Pos( m_spawnManOfs[i] );
					SHitProxyCircle circle;
					circle.fRadius = m_spawnManRadius;
					circle.center = spawnPos;

					vector<CReference<CEntity> > hitEntities;
					GetStage()->MultiHitTest( &circle, CMatrix2D::GetIdentity(), hitEntities );
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

					auto pEntity = SafeCast<CEntity>( m_strMan->GetRoot()->CreateInstance() );
					pEntity->SetPosition( spawnPos );
					pEntity->SetParentBeforeEntity( CMyLevel::GetInst()->GetChunkEffectRoot() );
				}

				m_bSpawned = true;
			}
		}
		else if( GetVelocity().Length2() < m_fSpawnManSpeed * m_fSpawnManSpeed || m_pBurnEffect )
		{
			m_bDoorOpen = true;
			m_nSpawnManTimeLeft = m_nSpawnManTime;
		}
	}

	int32 nDmg = ceil( m_moveData.fDamage * m_fHitDamage );
	if( m_pBurnEffect )
	{
		if( m_nBurnDamageCD )
			m_nBurnDamageCD--;
		if( !m_nBurnDamageCD )
		{
			m_nBurnDamageCD = m_nBurnDamageInterval;
			nDmg += m_nBurnDamage;
		}
	}

	if( nDmg )
	{
		SDamageContext context;
		context.nDamage = nDmg;
		context.nType = 0;
		context.nSourceType = 0;
		context.hitPos = CVector2( 0, 0 );
		context.hitDir = CVector2( 0, 0 );
		context.nHitType = -1;
		Damage( context );
	}
}

void CThrowBox::Kill()
{
	CVector2 pos = globalTransform.GetPosition();
	float fAngle = SRand::Inst().Rand( -PI, PI );
	CVector2 dir( cos( fAngle ), sin( fAngle ) );
	CVector2 dirs[4] = { { dir.x, dir.y }, { -dir.y, dir.x }, { -dir.x, -dir.y }, { dir.y, -dir.x } };
	for( int i = 0; i < 4; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
		pBullet->SetPosition( pos );
		pBullet->SetVelocity( dirs[i] * 150 );
		pBullet->SetLife( 60 );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}
	CEnemy::Kill();
}