#include "stdafx.h"
#include "Bug.h"
#include "MyLevel.h"
#include "Stage.h"

void CBug::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	GetStage()->RegisterStageEvent( eStageEvent_PostUpdate, &m_onTick );
	bVisible = m_bDetected;
}

void CBug::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	CCharacter::OnRemovedFromStage();
}

bool CBug::Damage( SDamageContext& context )
{
	if( !IsActivated() )
		return false;
	if( m_nFixType == eBugFixType_None )
		return true;
	if( m_nFixType != eBugFixType_Common )
	{
		EBugFixType nType = eBugFixType_System;
		auto pSource = context.pSource;
		auto pPlayer = GetLevel()->GetPlayer();
		bool bPlayer = pSource == pPlayer;
		if( !bPlayer )
		{
			auto pCharacter = SafeCast<CCharacter>( pSource );
			if( pCharacter && pCharacter->IsOwner( CMasterLevel::GetInst()->GetPlayer() ) )
				bPlayer = true;
		}
		if( bPlayer )
		{
			if( context.nType == eDamageHitType_Kick_Special || context.nType == eDamageHitType_Kick_Begin || context.nType == context.nType == eDamageHitType_Kick_End )
				nType = eBugFixType_Melee;
			else
				nType = eBugFixType_Range;
		}
		if( nType != m_nFixType )
			return false;
	}

	Fix();
	return true;
}

bool CBug::IsActivated()
{
	if( m_bFixed )
		return false;
	auto pMasterLevel = CMasterLevel::GetInst();
	if( pMasterLevel->GetCurLevel() != GetLevel() )
		return false;
	if( !pMasterLevel->GetTestState() )
		return false;
	if( !pMasterLevel->GetTestRect().Contains( GetGlobalTransform().GetPosition() ) )
		return false;
	return true;
}

void CBug::Fix()
{
	m_bFixed = true;
	auto pRoot = CMyLevel::GetEntityCharacterRootInLevel( this, true );
	pRoot->Deactivate();
	m_pSoundFixed->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CBug::Clear( bool bFirst )
{
	SetParentEntity( NULL );
	if( bFirst )
		m_pSoundCleared->CreateSoundTrack()->Play( ESoundPlay_KeepRef );
}

void CBug::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();

	if( !m_bFixed )
	{
		if( IsActivated() )
		{
			m_bCurActivated = true;
			if( !m_bDetected )
			{
				m_bDetected = true;
				GetLevel()->OnBugDetected( this );
			}
			bVisible = true;

			if( m_nFixType == eBugFixType_Common )
			{
				for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
				{
					auto pOther = (CEntity*)pManifold->pOtherHitProxy;
					if( pOther->GetHitType() == eEntityHitType_Player )
					{
						Fix();
						break;
					}
				}
			}
		}
		else
		{
			if( m_bCurActivated )
			{
				m_bCurActivated = false;
				GetLevel()->ResetBug( this );
				return;
			}
		}
	}
}

CVector4 CBug::GetGroupColor( int32 nGroup )
{
	static CVector4 colors[8] = { { 0.13, 0.75, 0.03, 0.3 }, { 0.8, 0.4, 0.1, 0.3 }, { 0.2, 0.27, 0.8, 0.3 }, { 0.7, 0.7, 0.1, 0.3 },
	{ 0.75, 0.15, 0.15, 0.3 }, { 0.4, 0.1, 0.8, 0.3 }, { 0.1, 0.7, 0.7, 0.3 }, { 0.75, 0, 0.45, 0.3 } };
	return colors[nGroup % 8];
}

void CBug::UpdateImg( bool bPreview )
{
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	pImage->SetTexRect( CRectangle( 27, m_nFixType, 1, 1 ) / 32 );
	CVector4& param = pImage->GetParam()[0];
	param = GetGroupColor( m_nGroup );
	if( !bPreview && !m_bCurActivated )
		param = param * 0.15f + CVector4( 0.35, 0.35, 0.35, 0.7 );
	m_pFixedEft->bVisible = m_bFixed;
}
