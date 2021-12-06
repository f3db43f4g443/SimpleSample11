#include "stdafx.h"
#include "Explosion.h"
#include "Stage.h"
#include "Character.h"
#include "Player.h"
#include "MyLevel.h"

void CExplosion::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	if( m_pSound )
		m_pSound->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
	if( GetLevel()->GetUpdatePhase() < eStageUpdatePhase_AfterHitTest )
		m_nHitBeginFrame++;
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

void CExplosion::HandleHit( CCharacter* pCharacter, int32 nDmg, int32 nDmg1, const CVector2& hitPos, const CVector2& hitDir )
{
	CCharacter::SDamageContext context;
	context.nDamage = nDmg;
	context.fDamage1 = nDmg1;
	context.nType = m_nDamageType;
	context.nSourceType = 2;
	context.hitPos = hitPos;
	context.hitDir = hitDir;
	context.nHitType = -1;
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
	if( m_nLife )
	{
		m_nLife--;
		if( !m_nLife )
		{
			Kill();
			return;
		}
	}
	if( m_nHitBeginFrame )
	{
		m_nHitBeginFrame--;
		return;
	}
	if( m_nHitFrameCount > 0 && m_nHitFrame >= m_nHitFrameCount )
		return;

	vector<CReference<CEntity> > result;
	vector<SHitTestResult> hitResult;

	int32 nDmg = Max<int32>( m_nDamage + m_nHitFrame * m_nDeltaDamage, 0 );
	int32 nDmg1 = Max<int32>( m_nDamage1 + m_nHitFrame * m_nDeltaDamage1, 0 );
	if( !nDmg && !nDmg1 )
		return;
	switch( m_nRangeType )
	{
	case 0:
	{
		SHitProxyCircle circle;
		circle.fRadius = m_fInitRange + m_nHitFrame * m_fDeltaRange;
		circle.center = CVector2( 0, 0 );
		GetLevel()->MultiHitTest( &circle, GetGlobalTransform(), result, &hitResult );
		break;
	}
	case 1:
	{
		CRectangle rect;
		rect.x = -m_fInitRange - m_nHitFrame * m_fDeltaRange;
		rect.y = -m_fInitRange1 - m_nHitFrame * m_fDeltaRange1;
		rect.width = m_fInitRange2 + m_nHitFrame * m_fDeltaRange2 - rect.x;
		rect.height = m_fInitRange3 + m_nHitFrame * m_fDeltaRange3 - rect.y;
		SHitProxyPolygon polygon( rect );
		GetLevel()->MultiHitTest( &polygon, GetGlobalTransform(), result, &hitResult );
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
		GetLevel()->MultiHitTest( &polygon, GetGlobalTransform(), result, &hitResult );
		break;
	}
	}

	for( int i = 0; i < hitResult.size(); i++ )
	{
		CEntity* pEntity = result[i];
		auto& res = hitResult[i];
		if( !pEntity->GetStage() )
			continue;
		if( m_hit.find( pEntity ) != m_hit.end() )
			continue;

		CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
		if( pCharacter && !pCharacter->IsKilled() && !pCharacter->IsIgnoreBullet() )
		{
			if( !m_bHitCreator && m_pOwner && pEntity->IsOwner( m_pOwner ) )
				continue;
			if( !m_bHitPlayer && SafeCast<CPlayer>( pEntity ) )
				continue;

			auto hitDir = m_hitDir.Length2() > 0 ? GetGlobalTransform().MulVector2Dir( m_hitDir ) : res.normal;
			HandleHit( pCharacter, nDmg, nDmg1, res.hitPoint1, hitDir );
		}
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
	if( m_nHitFrameCount > 0 )
		m_nHitFrame++;
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
		REGISTER_MEMBER( m_nDamageType )
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
		REGISTER_MEMBER( m_nHitInterval )
		REGISTER_MEMBER( m_hitDir )
		REGISTER_MEMBER( m_pDmgEft )
		REGISTER_MEMBER( m_pSound )
	REGISTER_CLASS_END()
}