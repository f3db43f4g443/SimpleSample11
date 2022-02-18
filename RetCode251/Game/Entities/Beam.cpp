#include "stdafx.h"
#include "Beam.h"
#include "MyLevel.h"
#include "Entities/CharacterMisc.h"

void CBeam::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	bVisible = false;
	m_origRect = static_cast<CImage2D*>( m_pBeamImg[1].GetPtr() )->GetElem().rect;
	for( int i = 0; i < 3; i++ )
		m_origTexRect[i] = static_cast<CImage2D*>( m_pBeamImg[i].GetPtr() )->GetElem().texRect;
}

void CBeam::OnTickAfterHitTest()
{
	auto nDeltaTick = GetLevel()->GetDeltaTick();
	if( m_nLife && m_nTick >= m_nLife * T_SCL )
	{
		Kill();
		return;
	}

	auto trans = GetGlobalTransform();
	float fDist = m_fMaxRange;
	bool bHit = !( m_nTick < m_nHitBeginFrame * T_SCL || m_nHitFrameCount && m_nTick >= ( m_nHitBeginFrame + m_nHitFrameCount ) * T_SCL );

	CReference<CEntity> pHit0;
	for( int k = 0; k < ( bHit ? 2 : 1 ); k++ )
	{
		CVector2 ofs( fDist, 0 );
		ofs = trans.MulVector2Dir( ofs );
		static vector<CReference<CEntity> > vecHitEntities;
		static vector<SRaycastResult> vecResult;
		auto pHit = m_pHit[k];
		if( !pHit )
			pHit = m_pHit[0];
		GetLevel()->MultiSweepTest( pHit->Get_HitProxy(), trans, ofs, 0, vecHitEntities, &vecResult );
		for( int i = 0; i < vecHitEntities.size(); i++ )
		{
			auto n = CheckHit( vecHitEntities[i] );
			if( k == 0 )
			{
				if( n == 1 )
				{
					fDist = vecResult[i].fDist;
					pHit0 = vecHitEntities[i];
					if( bHit )
						HandleHit( pHit0, vecResult[i].hitPoint );
					break;
				}
			}
			else if( n )
			{
				if( vecHitEntities[i] != pHit0.GetPtr() )
					HandleHit( vecHitEntities[i], vecResult[i].hitPoint );
			}
		}
		vecHitEntities.resize( 0 );
		vecResult.resize( 0 );
	}

	if( m_nHitInterval )
	{
		for( auto itr = m_hit.begin(); itr != m_hit.end(); )
		{
			itr->second = Max( 0, itr->second - nDeltaTick );
			if( !itr->second )
				itr = m_hit.erase( itr );
			else
				itr++;
		}
	}
	m_pHit[0]->SetPosition( CVector2( fDist, 0 ) );
	if( m_pHit[1] )
		m_pHit[1]->SetPosition( CVector2( fDist, 0 ) );
	UpdateImages();
	m_nTick = Min( m_nTick + nDeltaTick, Max( m_nHitBeginFrame + m_nHitFrameCount, m_nLife ) * T_SCL );
}

int8 CBeam::CheckHit( CEntity* pEntity )
{
	if( m_pOwner && pEntity->IsOwner( m_pOwner ) )
		return 0;
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	if( pCharacter )
	{
		if( pCharacter->IsIgnoreDamageSource( 2 ) )
			pCharacter = NULL;
	}
	else
	{
		auto p = SafeCast<CDamageArea>( pEntity );
		if( p && !p->IsIgnoreDamageSource( 2 ) )
			pCharacter = SafeCast<CCharacter>( p->GetParentEntity() );
	}
	if( !pCharacter || pCharacter->IsKilled() )
		return 0;
	if( m_pOwner && pCharacter->IsOwner( m_pOwner ) )
		return 0;
	if( pCharacter->GetHitType() == eEntityHitType_WorldStatic || pCharacter->IsAlwaysBlockBullet() )
		return 1;
	return -1;
}

void CBeam::HandleHit( CEntity* pEntity, const CVector2& hitPoint )
{
	if( !m_nDamage && !m_nDamage1 && m_fHitForce == 0 )
		return;
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	if( !pCharacter )
		return;
	if( m_hit.find( pEntity ) != m_hit.end() )
		return;
	SDamageContext context;
	context.nSourceType = 3;
	context.hitPos = hitPoint;
	context.hitDir = globalTransform.MulVector2Dir( CVector2( m_fHitForce, 0 ) );
	context.nHitType = -1;
	context.pSource = this;
	if( m_bAlertEnemy && pCharacter->IsEnemy() )
	{
		context.nType = eDamageHitType_Alert;
		pCharacter->Damage( context );
		return;
	}

	context.nDamage = m_nDamage;
	context.fDamage1 = m_nDamage1;
	if( pCharacter->Damage( context ) )
	{
		if( m_pDmgEft )
			m_pDmgEft->GetRoot()->GetStaticData<CDamageEft>()->OnDamage( pCharacter, context );

		OnHit( pCharacter );
		if( pCharacter->GetStage() )
			m_hit[pCharacter] = m_nHitInterval * T_SCL;
		return;
	}
}

void CBeam::UpdateImages()
{
	bVisible = true;
	auto rect = m_origRect;
	rect.width += m_pHit[0]->x;
	static_cast<CImage2D*>( m_pBeamImg[1].GetPtr() )->SetRect( rect );
	m_pBeamImg[1]->SetBoundDirty();
	m_pBeamImg[2]->SetPosition( CVector2( m_pHit[0]->x, 0 ) );

	int32 nFrame = m_nAnimTick / ( m_nFrameInterval * T_SCL );
	if( nFrame >= m_nBeginFrame )
		nFrame = ( nFrame - m_nBeginFrame ) % m_nLoopFrame + m_nBeginFrame;
	m_nAnimTick += GetLevel()->GetDeltaTick();
	if( m_nAnimTick >= m_nFrameInterval * T_SCL * ( m_nBeginFrame + m_nLoopFrame ) )
		m_nAnimTick -= m_nFrameInterval * T_SCL * m_nLoopFrame;

	for( int i = 0; i < 3; i++ )
	{
		auto texRect = m_origTexRect[i];
		texRect.y += nFrame * texRect.height;
		static_cast<CImage2D*>( m_pBeamImg[i].GetPtr() )->SetTexRect( texRect );
	}
}

void RegisterGameClasses_Beam()
{
	REGISTER_CLASS_BEGIN( CBeam )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER( m_nHitBeginFrame )
		REGISTER_MEMBER( m_nHitFrameCount )
		REGISTER_MEMBER( m_nDamage )
		REGISTER_MEMBER( m_nDamage1 )
		REGISTER_MEMBER( m_fMaxRange )
		REGISTER_MEMBER( m_fHitForce )
		REGISTER_MEMBER( m_nHitInterval )
		REGISTER_MEMBER( m_nBeginFrame )
		REGISTER_MEMBER( m_nLoopFrame )
		REGISTER_MEMBER( m_nFrameInterval )
		REGISTER_MEMBER( m_pDmgEft )
		REGISTER_MEMBER( m_bAlertEnemy )
		REGISTER_MEMBER_TAGGED_PTR( m_pHit[0], hit )
		REGISTER_MEMBER_TAGGED_PTR( m_pHit[1], hit1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pBeamImg[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pBeamImg[1], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pBeamImg[2], 2 )
	REGISTER_CLASS_END()
}
