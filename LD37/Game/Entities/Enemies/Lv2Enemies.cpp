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

void CLimbs::OnAddedToStage()
{
	if( !m_bAuto )
		SetTransparent( true );
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
	CEnemy::Damage( context );
	context.nDamage = nDmg;
}

void CLimbs::Kill()
{
	if( m_bKilled )
		return;
	m_bKilled = true;
	if( m_pLimbsEft )
		m_pLimbsEft->SetParentEntity( NULL );
	SetTransparent( true );
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
	if( m_nState != 0 || m_tickAfterHitTest.IsRegistered() )
		return 2;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return 1;
	CVector2 p = globalTransform.MulTVector2PosNoScale( pPlayer->GetPosition() );
	if( !m_attackRect.Contains( p ) )
		return 1;
	return 0;
}

void CLimbs::Operate( const CVector2 & pos )
{
	m_nState = 1;
	m_fEftLen = 0;
	if( m_pLimbsEft )
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->CreateAttackEft( SafeCast<CLimbsEft>( m_pLimbsEft.GetPtr() ) );
	else
		SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->CreateAttackEft( NULL, NULL, 0 );
	SetTransparent( false );
	GetStage()->RegisterAfterHitTest( m_nAttackTime, &m_tickAfterHitTest );
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
	if( m_fEftLen <= 0 )
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
		SetTransparent( true );
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
	if( m_nState >= 1 )
	{
		for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
		{
			auto type = static_cast<CEntity*>( pManifold->pOtherHitProxy )->GetHitType();
			if( type == eEntityHitType_WorldStatic || type == eEntityHitType_Platform )
			{
				GetStage()->RegisterAfterHitTest( m_nAttackTime1, &m_tickAfterHitTest );
				CCharacterMoveUtil::Stretch( this, m_nAttackDir, -m_fEftLen );
				KillAttackEft();
				return;
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
				if( type == eEntityHitType_WorldStatic || type == eEntityHitType_Platform )
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
		bool bHitChannel[eEntityHitType_Count] = { true, true, false, false, false, false, false };
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
				SetTransparent( true );
		}
		else
		{
			SafeCast<CLimbsAttackEft>( m_pLimbsAttackEft.GetPtr() )->SetAttackEftLen( m_fEftLen );
			GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
		}
	}
}

void CLimbs1::Init( const CVector2 & size, SChunk* pPreParent )
{
	SetRenderObject( NULL );
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );

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
	for( auto& p : vec )
	{
		if( m_vecMask[p.x + p.y * nWidth] )
			continue;
		uint8 nSize = SRand::Inst().Rand( 0, 2 ) ? 3 : 2;
		auto rect = PutRect( m_vecMask, nWidth, nHeight, p, TVector2<int32>( nSize, nSize ), TVector2<int32>( nSize, nSize ), TRectangle<int32>( 0, 0, nWidth, nHeight ), -1, 1 );
		if( rect.width <= 0 )
			continue;
		
		auto pImg = static_cast<CMultiFrameImage2D*>( pDrawable->CreateInstance() );
		pImg->SetPosition( CVector2( ( rect.x + rect.width * 0.5f ) * 8 + 4, ( rect.y + rect.height * 0.5f ) * 8 + 4 ) );
		pImg->SetRotation( PI * 0.5f * SRand::Inst().Rand( -2, 2 ) );
		uint32 n = nSize == 2 ? SRand::Inst().Rand( 0, 4 ) : SRand::Inst().Rand( 4, 8 );
		pImg->SetFrames( n * 2, n * 2 + 2, pImg->GetFramesPerSec() );
		pImg->SetPlayPercent( SRand::Inst().Rand( 0.0f, 1.0f ) );
		pImg->SetAutoUpdateAnim( true );
		AddChild( pImg );
		m_vecKillSpawn.push_back( pair<CVector2, uint8>( pImg->GetPosition(), k ) );
		if( nSize == 3 )
			m_vecKillSpawn.push_back( pair<CVector2, uint8>( pImg->GetPosition(), ( k + 2 ) % 4 ) );
		k++;
		if( k >= 4 )
			k = 0;
	}
	m_attackRect.width += size.x;
	m_attackRect.height += size.y;
}

void CLimbs1::OnAddedToStage()
{
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

void CWheel::OnAddedToStage()
{
	CEnemy::OnAddedToStage();
	m_nDir = SRand::Inst().Rand( 0, 2 ) * 2 - 1;
}

bool CWheel::Knockback( const CVector2& vec )
{
	CVector2 tangent( m_moveData.normal.y, -m_moveData.normal.x );
	float fTangent = tangent.Dot( vec );
	CVector2 vecKnockback = ( tangent * fTangent + m_moveData.normal ) * m_moveData.fFallInitSpeed * 2.5;
	if( m_moveData.bHitSurface )
		m_moveData.Fall( this, vecKnockback );
	else
		SetVelocity( GetVelocity() + vecKnockback );

	m_nKnockBackTimeLeft = m_nKnockbackTime;
	return true;
}

void CWheel::Kill()
{
	for( int i = 0; i < 12; i++ )
	{
		auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
		pBullet->SetPosition( GetPosition() );
		pBullet->SetRotation( SRand::Inst().Rand( -PI, PI ) );
		pBullet->SetVelocity( CVector2( cos( pBullet->r ), sin( pBullet->r ) ) * SRand::Inst().Rand( 150.0f, 200.0f ) );
		pBullet->SetLife( SRand::Inst().Rand( 60, 90 ) );
		pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	}

	CEnemy::Kill();
}

void CWheel::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	CVector2 lastPos = GetPosition();
	bool bLastHitSurface = m_moveData.bHitSurface;
	CVector2 lastVel = GetVelocity();
	m_moveData.UpdateMove( this, m_nDir );
	if( !GetStage() )
		return;

	if( !bLastHitSurface && m_moveData.bHitSurface )
	{
		CVector2 dir( m_moveData.normal.y, -m_moveData.normal.x );
		if( lastVel.Dot( dir ) * m_nDir < 0 )
			m_nDir = -m_nDir;
	}

	CVector2 dPos = GetPosition() - lastPos;
	float dRot = -m_fRotSpeed * m_nDir * GetStage()->GetElapsedTimePerTick();
	GetRenderObject()->SetRotation( GetRenderObject()->r + dRot );

	if( !m_nAIStepTimeLeft )
	{
		if( m_moveData.bHitSurface )
		{
			if( m_moveData.normal.y < -0.8f && SRand::Inst().Rand( 0.0f, 1.0f ) < m_fFallChance )
			{
				CPlayer* pPlayer = GetStage()->GetPlayer();
				if( pPlayer )
				{
					CVector2 dPos = pPlayer->GetPosition() - globalTransform.GetPosition();
					if( dPos.y < 0 )
					{
						float t = sqrt( -2 * dPos.y / m_moveData.fGravity );
						float vx = dPos.x / t;
						if( abs( vx ) < m_moveData.fFallInitSpeed * 2 )
						{
							m_moveData.Fall( this, CVector2( vx, 0 ) );
						}
					}
				}
			}
		}

		m_nAIStepTimeLeft = SRand::Inst().Rand( m_fAIStepTimeMin, m_fAIStepTimeMax ) / GetStage()->GetElapsedTimePerTick();
	}

	if( m_nAIStepTimeLeft )
		m_nAIStepTimeLeft--;
	if( m_nKnockBackTimeLeft )
		m_nKnockBackTimeLeft--;
	CEnemy::OnTickAfterHitTest();
}

void CSawBlade::OnAddedToStage()
{
	CVector2 vel = GetVelocity();
	if( vel.Normalize() < 0.01f )
		vel = CVector2( 0, 1 );
	SetVelocity( vel * m_fMoveSpeed );
	CEnemy::OnAddedToStage();
}

void CSawBlade::OnKnockbackPlayer( const CVector2 & vec )
{
	CCharacter::SDamageContext context;
	memset( &context, 0, sizeof( context ) );
	context.nDamage = m_nDamage;
	GetStage()->GetPlayer()->Damage( context );
}

void CSawBlade::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.UpdateMove( this );
	if( !GetStage() )
		return;
	float dRot = m_fRotSpeed * GetStage()->GetElapsedTimePerTick();
	GetRenderObject()->SetRotation( GetRenderObject()->r + dRot );

	CEnemy::OnTickAfterHitTest();
}

bool CGear::Knockback( const CVector2& vec )
{
	SetVelocity( vec * m_fMoveSpeed );
	m_nKnockBackTimeLeft = m_nKnockbackTime;
	return true;
}

void CGear::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.UpdateMove( this );
	if( !GetStage() )
		return;

	if( m_velocity.Length2() == 0 )
	{
		for( int i = 0; i < 8; i++ )
		{
			auto pBullet = SafeCast<CBullet>( m_pBullet->GetRoot()->CreateInstance() );
			pBullet->SetPosition( GetPosition() );
			pBullet->SetRotation( i * PI / 4 );
			pBullet->SetVelocity( CVector2( cos( pBullet->r ), sin( pBullet->r ) ) * 180.0f );
			pBullet->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
		}

		float fAngle = SRand::Inst().Rand( -PI, PI );
		CVector2 dir( cos( fAngle ), sin( fAngle ) );
		dir = dir * SRand::Inst().Rand( 0.0f, 1024.0f );
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer )
		{
			CVector2 dPos = pPlayer->GetPosition() - GetPosition();
			dir = dir + dPos;
		}

		CVector2 dirs[8] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0.707f, 0.707f }, { -0.707f, 0.707f }, { 0.707f, -0.707f }, { -0.707f, -0.707f } };
		float fMaxDot = -1000000;
		uint32 maxi = 0;
		for( int i = 0; i < 8; i++ )
		{
			float fDot = dirs[i].Dot( dir );
			if( fDot > fMaxDot )
			{
				fMaxDot = fDot;
				maxi = i;
			}
		}

		SetVelocity( dirs[maxi] * m_fMoveSpeed );
		if( dir.x > 0 )
			m_nTargetDir = m_nTargetDir >= 3 ? 0 : m_nTargetDir + 1;
		else
			m_nTargetDir = m_nTargetDir <= 0 ? 3 : m_nTargetDir - 1;
	}

	float dAngle = NormalizeAngle( m_nTargetDir * PI * 0.5f - GetRotation() );
	float dAngle1 = m_fRotSpeed * GetStage()->GetElapsedTimePerTick();
	if( abs( dAngle ) <= dAngle1 )
		SetRotation( m_nTargetDir * PI * 0.5f );
	else if( dAngle > 0 )
		SetRotation( GetRotation() + dAngle1 );
	else
		SetRotation( GetRotation() - dAngle1 );
	
	if( m_nKnockBackTimeLeft )
		m_nKnockBackTimeLeft--;
	CEnemy::OnTickAfterHitTest();
}

void CExplosiveBall::OnAddedToStage()
{
	m_moveTarget = GetPosition();
	m_moveData.bHitChannel[eEntityHitType_WorldStatic] = m_moveData.bHitChannel[eEntityHitType_Platform] = m_moveData.bHitChannel[eEntityHitType_System] = true;
	CEnemy::OnAddedToStage();
}

void CExplosiveBall::OnKnockbackPlayer( const CVector2 & vec )
{
	GetStage()->GetPlayer()->Knockback( vec * -2.5f );
}

void CExplosiveBall::Kill()
{
	SBarrageContext context;
	context.vecBulletTypes.push_back( m_pBullet.GetPtr() );
	context.nBulletPageSize = 4 * 2;

	CBarrage* pBarrage = new CBarrage( context );
	pBarrage->AddFunc( [] ( CBarrage* pBarrage )
	{
		CVector2 ofs[4] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
		bool b[4] = { true, true, true, true };
		CVector2 p0[4] = { CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ), CVector2( 0, 0 ) };
		CVector2 v[4];
		for( int i = 0; i < 10; i++ )
		{
			int32 n = 0;
			for( int i1 = 0; i1 < 4; i1++ )
			{
				if( !b[i1] )
					continue;
				v[i1] = ofs[i1] * 300;
				pBarrage->InitBullet( i1 + ( i & 1 ) * 4, 0, -1, p0[i1], v[i1], CVector2( 0, 0 ) );
			}
			pBarrage->Yield( 6 );

			for( int i1 = 0; i1 < 4; i1++ )
			{
				auto pContext = pBarrage->GetBulletContext( i1 + ( i & 1 ) * 4 );
				if( !pContext->IsValid() || !pContext->pEntity || !pContext->pEntity->GetStage() )
				{
					b[i1] = false;
					continue;
				}
				p0[i1] = p0[i1] + v[i1] * 0.1f;
				SafeCast<CBullet>( pContext->pEntity.GetPtr() )->Kill();
				n++;
			}
			if( !n )
				break;
		}
		pBarrage->Yield( 2 );
		pBarrage->StopNewBullet();
	} );
	pBarrage->SetParentEntity( CMyLevel::GetInst()->GetBulletRoot( CMyLevel::eBulletLevel_Enemy ) );
	pBarrage->SetPosition( GetPosition() );
	pBarrage->Start();

	CEnemy::Kill();
}

void CExplosiveBall::OnTickAfterHitTest()
{
	DEFINE_TEMP_REF_THIS();
	m_moveData.UpdateMove( this, m_moveTarget );
	if( !GetStage() )
		return;

	if( !m_nAITickLeft )
	{
		float fAngle = SRand::Inst().Rand( -PI, PI );
		CVector2 dir( cos( fAngle ), sin( fAngle ) );
		m_moveTarget = m_moveTarget + dir * 32;
		m_nAITickLeft = m_nAITick;
	}

	if( m_nAITickLeft )
		m_nAITickLeft--;
	CEnemy::OnTickAfterHitTest();
}