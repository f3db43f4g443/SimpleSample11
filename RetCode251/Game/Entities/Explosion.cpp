#include "stdafx.h"
#include "Explosion.h"
#include "Stage.h"
#include "Character.h"
#include "Player.h"
#include "MyLevel.h"
#include "Entities/CharacterMisc.h"
#include "Entities/UtilEntities.h"

void CExplosion::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	bVisible = false;
	if( m_pSound )
		m_pSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CExplosion::OnRemovedFromStage()
{
	m_pCreator = NULL;
	m_hit.clear();
	CCharacter::OnRemovedFromStage();
}

void CExplosion::AddInitHit( CEntity* pHitEntity, const CVector2& hitPos, const CVector2& hitDir )
{
	auto pCharacter = SafeCast<CCharacter>( pHitEntity );
	if( pCharacter )
		HandleHit( pCharacter, m_nDamage, m_nDamage1, hitPos, hitDir );
}

CCharacter* CExplosion::CheckHit( CEntity* pEntity )
{
	if( !pEntity->GetStage() )
		return NULL;
	if( !m_bHitPlayer && SafeCast<CPlayer>( pEntity ) )
		return NULL;
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	if( pCharacter )
	{
		if( m_bCheckIgnoreSourceType && pCharacter->IsIgnoreDamageSource( 1 ) )
			pCharacter = NULL;
	}
	else
	{
		auto p = SafeCast<CDamageArea>( pEntity );
		if( p && ( !m_bCheckIgnoreSourceType || !p->IsIgnoreDamageSource( 1 ) ) )
			pCharacter = SafeCast<CCharacter>( p->GetParentEntity() );
	}
	if( !pCharacter || pCharacter->IsKilled() || m_hit.find( pCharacter ) != m_hit.end() )
		return NULL;
	if( !m_bHitCreator && m_pOwner && pCharacter->IsOwner( m_pOwner ) )
		return NULL;
	return pCharacter;
}

void CExplosion::HandleHit( CCharacter* pCharacter, int32 nDmg, int32 nDmg1, const CVector2& hitPos, const CVector2& hitDir )
{
	CCharacter::SDamageContext context;
	context.nType = m_eDamageType;
	context.nSourceType = 2;
	context.hitPos = hitPos;
	context.hitDir = hitDir;
	context.nHitType = -1;
	context.pSource = this;
	if( m_bAlertEnemy && pCharacter->IsEnemy() )
	{
		context.nType = eDamageHitType_Alert;
		pCharacter->Damage( context );
		return;
	}

	context.nDamage = nDmg;
	context.fDamage1 = nDmg1;
	if( pCharacter->Damage( context ) )
	{
		if( m_pDmgEft )
			m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( pCharacter, context );
		OnHit( pCharacter );
		if( pCharacter->GetStage() )
			m_hit[pCharacter] = m_nHitInterval;
	}
}

void CExplosion::OnTickAfterHitTest()
{
	bVisible = true;
	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
		{
			Kill();
			return;
		}
	}
	bool bTest = true;
	int32 nDmg = 0, nDmg1 = 0;
	if( m_nHitBeginFrame )
	{
		m_nHitBeginFrame--;
		bTest = false;
	}
	else if( m_nHitFrameCount > 0 && m_nHitFrame >= m_nHitFrameCount )
		bTest = false;
	else
	{
		nDmg = Max<int32>( m_nDamage + m_nHitFrame * m_nDeltaDamage, 0 );
		nDmg1 = Max<int32>( m_nDamage1 + m_nHitFrame * m_nDeltaDamage1, 0 );
		if( !nDmg && !nDmg1 )
			bTest = false;
	}

	vector<CReference<CEntity> > result;
	vector<SHitTestResult> hitResult;
	auto pImageRect = SafeCastToInterface<IImageRect>( GetRenderObject() );
	switch( m_nRangeType )
	{
	case 0:
	{
		SHitProxyCircle circle;
		circle.fRadius = m_fInitRange + m_nHitFrame * m_fDeltaRange;
		circle.center = CVector2( 0, 0 );
		if( bTest )
			GetLevel()->MultiHitTest( &circle, GetGlobalTransform(), result, &hitResult );
		if( pImageRect )
			pImageRect->SetRect( CRectangle( -circle.fRadius, -circle.fRadius, circle.fRadius * 2, circle.fRadius * 2 ) );
		break;
	}
	case 1:
	{
		CRectangle rect;
		rect.x = -m_fInitRange - m_nHitFrame * m_fDeltaRange;
		rect.y = -m_fInitRange1 - m_nHitFrame * m_fDeltaRange1;
		rect.width = m_fInitRange2 + m_nHitFrame * m_fDeltaRange2 - rect.x;
		rect.height = m_fInitRange3 + m_nHitFrame * m_fDeltaRange3 - rect.y;
		if( bTest )
		{
			SHitProxyPolygon polygon( rect );
			GetLevel()->MultiHitTest( &polygon, GetGlobalTransform(), result, &hitResult );
		}
		if( pImageRect )
			pImageRect->SetRect( rect );
		break;
	}
	default:
	{
		SHitProxyPolygon polygon;
		polygon.vertices[0] = CVector2( -m_fInitRange - m_nHitFrame * m_fDeltaRange, 0 );
		polygon.vertices[1] = CVector2( 0, -m_fInitRange1 - m_nHitFrame * m_fDeltaRange1 );
		polygon.vertices[2] = CVector2( m_fInitRange2 + m_nHitFrame * m_fDeltaRange2, 0 );
		polygon.vertices[3] = CVector2( 0, m_fInitRange3 + m_nHitFrame * m_fDeltaRange3 );
		polygon.nVertices = 4;
		polygon.CalcNormals();
		if( bTest )
			GetLevel()->MultiHitTest( &polygon, GetGlobalTransform(), result, &hitResult );
		if( pImageRect )
			pImageRect->SetRect( CRectangle( polygon.vertices[0].x, polygon.vertices[1].y,
				polygon.vertices[2].x - polygon.vertices[0].x, polygon.vertices[3].y - polygon.vertices[1].y ) );
		break;
	}
	}

	if( bTest )
	{
		for( int i = 0; i < hitResult.size(); i++ )
		{
			auto pCharacter = CheckHit( result[i] );
			if( pCharacter )
			{
				auto hitDir = m_hitDir.Length2() > 0 ? GetGlobalTransform().MulVector2Dir( m_hitDir ) : hitResult[i].normal;
				HandleHit( pCharacter, nDmg, nDmg1, hitResult[i].hitPoint1, hitDir );
			}
		}

		if( m_nHitFrameCount > 0 )
			m_nHitFrame++;
	}

	if( m_nHitInterval )
	{
		for( auto itr = m_hit.begin(); itr != m_hit.end(); )
		{
			itr->second--;
			if( !itr->second )
				itr = m_hit.erase( itr );
			else
				itr++;
		}
	}
}

void RegisterGameClasses_Explosion()
{
	REGISTER_CLASS_BEGIN( CExplosion )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_BASE_CLASS( IAttackEft )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_nHitBeginFrame )
		REGISTER_MEMBER( m_nHitFrameCount )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDeltaDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_nDeltaDamage1 )
		REGISTER_MEMBER( m_eDamageType )
		REGISTER_MEMBER( m_nRangeType )
		REGISTER_MEMBER( m_fInitRange )
		REGISTER_MEMBER( m_fDeltaRange )
		REGISTER_MEMBER( m_fInitRange1 )
		REGISTER_MEMBER( m_fDeltaRange1 )
		REGISTER_MEMBER( m_fInitRange2 )
		REGISTER_MEMBER( m_fDeltaRange2 )
		REGISTER_MEMBER( m_fInitRange3 )
		REGISTER_MEMBER( m_fDeltaRange3 )
		REGISTER_MEMBER( m_bHitPlayer )
		REGISTER_MEMBER( m_bHitCreator )
		REGISTER_MEMBER( m_bAlertEnemy )
		REGISTER_MEMBER( m_bCheckIgnoreSourceType )
		REGISTER_MEMBER( m_nHitInterval )
		REGISTER_MEMBER( m_hitDir )
		REGISTER_MEMBER( m_pDmgEft )
		REGISTER_MEMBER( m_pSound )
	REGISTER_CLASS_END()
}