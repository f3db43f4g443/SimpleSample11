#include "stdafx.h"
#include "PlayerMisc.h"
#include "Player.h"
#include "MyLevel.h"
#include "Interfaces.h"
#include "Entities/CharacterMisc.h"
#include "Entities/Bullet.h"

void CKick::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	m_origImgRect = pImg->GetElem().rect;
	m_origImgTexRect = pImg->GetElem().texRect;
	m_nDeathTime0 = m_nDeathTime;
	SetRenderParent( GetLevel() );
}

void CKick::OnHit( CEntity* pEntity )
{
	if( !m_bHit )
		OnFirstHit();
	if( m_nExtentTime && !m_bExtentHit )
		OnFirstExtentHit();
}

void CKick::Morph( CEntity* pEntity )
{
	auto pAttackEft = SafeCastToInterface<IAttackEft>( pEntity );
	for( auto& item : m_hit )
	{
		auto p = SafeCast<CCharacter>( item.first.GetPtr() );
		if( m_nReleaseFrame )
		{
			auto pBullet = SafeCast<CBullet>( p );
			if( pBullet )
			{
				pBullet->Pause( false );
				pBullet->SetBulletVelocity( pBullet->GetBulletVelocity() * -1 );
			}
			else
			{
				SDamageContext damageContext;
				damageContext.fDamage1 = 0;
				damageContext.nType = eDamageHitType_Kick_End;
				damageContext.pSource = this;
				p->Damage( damageContext );
			}
		}
		auto hitPos = p->GetGlobalTransform().MulVector2Pos( item.second.hitPos );
		pAttackEft->AddInitHit( p, hitPos, item.second.hitDir );
	}
	Kill();
}

void CKick::Cancel()
{
	if( m_nReleaseFrame )
	{
		for( auto& item : m_hit )
		{
			auto p = SafeCast<CCharacter>( item.first.GetPtr() );
			auto pBullet = SafeCast<CBullet>( p );
			if( pBullet )
			{
				pBullet->Pause( false );
				pBullet->SetBulletVelocity( pBullet->GetBulletVelocity() * -1 );
			}
			else
			{
				SDamageContext damageContext;
				damageContext.fDamage1 = 0;
				damageContext.nType = eDamageHitType_Kick_End;
				damageContext.pSource = this;
				p->Damage( damageContext );
			}
		}
	}
	Kill();
}

bool CKick::CounterBullet( CEntity* p, const CVector2& hitPos, const CVector2& hitDir )
{
	if( !m_nReleaseFrame )
		return false;

	auto pPrefab = SafeCast<CBullet>( p )->GetCounterBullet();
	auto pNewBullet = SafeCast<CBullet>( pPrefab->GetRoot()->CreateInstance() );
	pNewBullet->SetPosition( hitPos );
	auto vel0 = pNewBullet->GetBulletVelocity();
	auto vel = hitDir;
	vel.Normalize();
	vel = CVector2( vel.x * vel0.x - vel.y * vel0.y, vel.x * vel0.y + vel.y * vel0.x );
	pNewBullet->SetBulletVelocity( vel );
	pNewBullet->SetOwner( this );
	pNewBullet->Pause( true );
	pNewBullet->SetParentBeforeEntity( p );

	SDamageContext damageContext;
	damageContext.fDamage1 = m_nKickEffectTime;
	damageContext.nType = (EDamageType)( eDamageHitType_Kick_End + 1 + m_nKickType );
	damageContext.hitPos = hitPos;
	damageContext.hitDir = globalTransform.MulVector2Dir( CVector2( 1, 0 ) );
	damageContext.pSource = this;
	if( m_pDmgEft )
		m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( pNewBullet, damageContext );
	OnHit( pNewBullet );

	auto& hit = m_hit[pNewBullet];
	hit.n = 1;
	hit.hitPos = hitPos;
	hit.hitDir = damageContext.hitDir;

	return true;
}

void CKick::OnTickAfterHitTest()
{
	auto g = GetGlobalTransform();
	auto curGlobalPos = g.GetPosition();
	if( m_nTick < m_nHitFrames )
	{
		m_hitRect.x = m_hitBegin.x + m_hitDelta.x * m_nTick;
		m_hitRect.y = m_hitBegin.y + m_hitDelta.y * m_nTick;
		m_hitRect.width = m_hitBegin.width + m_hitDelta.width * m_nTick;
		m_hitRect.height = m_hitBegin.height + m_hitDelta.height * m_nTick;
		m_hitRect0 = m_hitRect1 = m_nTick == 0 ? m_hitRect : m_hitRect + m_hitRect0;

		HitTest( m_hitRect, g, m_fDamage0 );
		m_nTick++;
	}
	else
	{
		if( m_nExtentTime )
		{
			auto lastLocalPos = g.MulTVector2PosNoScale( m_lastGlobalPos );
			auto hitRect = m_hitRect + m_hitRect.Offset( lastLocalPos );
			m_hitRect1 = m_hitRect1.Offset( lastLocalPos ) + m_hitRect;
			HitTest( hitRect, g, m_fDamage0 );
			auto imgRect = m_origImgRect;
			imgRect.x += m_hitRect1.x - m_hitRect0.x;
			imgRect.width += m_hitRect1.width - m_hitRect0.width;
			auto pImg = static_cast<CImage2D*>( GetRenderObject() );
			pImg->SetRect( imgRect );
			pImg->SetBoundDirty();
			m_nExtentTime--;
		}
		else if( m_nReleaseFrame )
		{
			m_nReleaseFrame--;
			if( !m_nReleaseFrame )
			{
				for( auto& item : m_hit )
				{
					auto p = SafeCast<CCharacter>( item.first.GetPtr() );
					auto pBullet = SafeCast<CBullet>( p );
					if( pBullet )
					{
						pBullet->Pause( false );
						auto vel = item.second.hitDir * pBullet->GetBulletVelocity().Length();
						pBullet->SetBulletVelocity( vel );
					}
					else
					{
						SDamageContext damageContext;
						damageContext.fDamage1 = m_nKickEffectTime;
						damageContext.nType = (EDamageType)( eDamageHitType_Kick_End + 1 + m_nKickType );
						damageContext.hitPos = p->GetGlobalTransform().MulVector2Pos( item.second.hitPos );
						damageContext.hitDir = item.second.hitDir * m_fHitForce;
						damageContext.pSource = this;
						p->Damage( damageContext );
					}
				}
			}
		}
		else if( m_nDeathTime )
		{
			m_nDeathTime--;
			if( !m_nDeathTime )
			{
				Kill();
				return;
			}
		}
	}
	m_lastGlobalPos = curGlobalPos;
	UpdateImage();
}

CCharacter* CKick::CheckHit( CEntity* pEntity )
{
	if( !pEntity->GetStage() )
		return NULL;
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	if( !pCharacter )
	{
		auto p = SafeCast<CDamageArea>( pEntity );
		if( p )
			pCharacter = SafeCast<CCharacter>( p->GetParentEntity() );
	}
	if( !pCharacter || pCharacter->IsKilled() || m_hit.find( pCharacter ) != m_hit.end() )
		return NULL;
	if( SafeCast<CPlayer>( pCharacter ) )
		return NULL;
	if( m_pOwner && pCharacter->IsOwner( m_pOwner ) )
		return NULL;
	return pCharacter;
}

void CKick::HitTest( const CRectangle& rect, const CMatrix2D &g, float fDmg )
{
	vector<CReference<CEntity> > result;
	vector<SHitTestResult> hitResult;
	SHitProxyPolygon polygon( m_hitRect );
	GetLevel()->MultiHitTest( &polygon, g, result, &hitResult );

	for( int i = 0; i < hitResult.size(); i++ )
	{
		auto pCharacter = CheckHit( result[i] );
		if( pCharacter )
		{
			CCharacter::SDamageContext context;
			context.nDamage = 0;
			context.fDamage1 = 0;
			context.nType = eDamageHitType_Kick_Begin;
			context.nSourceType = 2;
			context.hitPos = hitResult[i].hitPoint1;
			auto hitDir = g.MulVector2Dir( CVector2( 1, 0 ) );
			context.hitDir = hitDir * fDmg;
			context.nHitType = -1;
			context.pSource = this;
			if( pCharacter->Damage( context ) )
			{
				if( m_pDmgEft )
					m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( pCharacter, context );
				OnHit( pCharacter );
				if( pCharacter->GetStage() )
				{
					auto& hit = m_hit[pCharacter];
					hit.n = 1;
					hit.hitPos = pCharacter->GetGlobalTransform().MulTVector2Pos( context.hitPos );
					hit.hitDir = hitDir;
				}
			}
		}
	}
}

void CKick::UpdateImage()
{
	int32 nFrame;
	if( !m_nReleaseFrame )
		nFrame = 4 + ( m_nDeathTime0 - m_nDeathTime ) * 4 / m_nDeathTime0;
	else if( m_nAnimTick < 9 )
	{
		nFrame = m_nAnimTick / 3;
		m_nAnimTick++;
	}
	else
		nFrame = 3;

	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	pImg->SetTexRect( m_origImgTexRect.Offset( CVector2( 0, nFrame * m_origImgTexRect.height ) ) );
}

void CKick::OnFirstHit()
{
	m_bHit = true;
	auto pPlayer = SafeCast<CPlayer>( m_pOwner.GetPtr() );
	if( pPlayer )
		pPlayer->OnKickFirstHit( this );
}

void CKick::OnFirstExtentHit()
{
	m_bExtentHit = true;
	auto pPlayer = SafeCast<CPlayer>( m_pOwner.GetPtr() );
	if( pPlayer )
		pPlayer->OnKickFirstExtentHit( this );
}

void CKickSpin::OnHit( CEntity* pEntity )
{
	if( m_bHit )
		return;
	m_bHit = true;
	auto pPlayer = SafeCast<CPlayer>( m_pOwner.GetPtr() );
	if( pPlayer )
		pPlayer->OnKickFirstHit( this );
}

void RegisterGameClasses_PlayerMisc()
{
	REGISTER_CLASS_BEGIN( CKick )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nKickType )
		REGISTER_MEMBER( m_fDamage0 )
		REGISTER_MEMBER( m_fHitForce )
		REGISTER_MEMBER( m_nKickEffectTime )
		REGISTER_MEMBER( m_nDeathTime )
		REGISTER_MEMBER( m_nHitFrames )
		REGISTER_MEMBER( m_hitBegin )
		REGISTER_MEMBER( m_hitDelta )
		REGISTER_MEMBER( m_nReleaseFrame )
		REGISTER_MEMBER( m_pDmgEft )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CKickSpin )
		REGISTER_BASE_CLASS( CExplosion )
	REGISTER_CLASS_END()
}