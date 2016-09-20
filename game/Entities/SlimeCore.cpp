#include "stdafx.h"
#include "SlimeCore.h"
#include "Stage.h"
#include "Player.h"
#include "Common/Rand.h"
#include "EffectObject.h"
#include "Render/ParticleSystem.h"
#include "Render/Sound.h"
#include "ResourceManager.h"
#include "DizzyRegion.h"
#include "Useable.h"
#include "Effects/DynamicTextures.h"

CSlimeCore::CSlimeCore()
	: m_tickAfterHitTest( this, &CSlimeCore::OnTickAfterHitTest )
	, m_onPlayerAttack( this, &CSlimeCore::OnPlayerAttack )
	, m_nCount( 0 )
	, m_nSlimeFullyBoundCount( 0 )
	, m_bCanBeHit( false )
	, m_fEnableBoundUntouchedSlimeTime( 0 )
	, m_nHpLeft( 40 )
	, m_fHitInterval( 0.15f )
	, m_fHitTimeLeft( 0 )
	, m_nGrowlInterval( 0 )
	, m_paramEmission( 1, 1, 1, 1 )
	, m_slimeColor( 1, 1, 1, 1 )
	, m_bSlimeBlink( false )
{
	SetHitType( eEntityHitType_Enemy );
}

void CSlimeCore::SetAnimSet( CAnimationSet* pAnimSet )
{
	SetAnim( pAnimSet, 0 );
	m_vecSlimes.resize( pAnimSet->GetSkeleton().GetBoneCount() );
	m_freeSlots.resize( pAnimSet->GetSkeleton().GetBoneCount() );
	for( int i = 0; i < m_freeSlots.size(); i++ )
	{
		m_freeSlots[i] = i;
	}
}

void CSlimeCore::OnSlimeFullyBound( CSlime* pSlime )
{
	pSlime->SetParentEntity( this );
	pSlime->SetTransformIndex( pSlime->m_nSlotIndex );
	m_nSlimeFullyBoundCount++;
}

void CSlimeCore::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	RegisterEntityEvent( eEntityEvent_PlayerAttack, &m_onPlayerAttack );
	Growl();
}

void CSlimeCore::OnRemovedFromStage()
{
	PlaySound( 2 );
	CCharacter::OnRemovedFromStage();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	if( m_onPlayerAttack.IsRegistered() )
		m_onPlayerAttack.Unregister();
}

void CSlimeCore::PlaySound( uint32 nType )
{
	static const char* fileNames[][3] =
	{
		{ "growl1.wav", "growl2.wav", "growl3.wav" },
		{ "atk1.wav", "atk2.wav", "atk3.wav" },
		{ "death1.wav", "death1.wav", "death1.wav" }
	};
	CSoundFile::PlaySound( fileNames[nType][m_nType] );
}

void CSlimeCore::Add( CSlime* pSlime )
{
	if( IsComplete() )
		return;

	pSlime->m_pSlimeCore = this;
	pSlime->m_pSlimeGround->RemoveUnboundSlime( pSlime );

	uint32 nFreeSlotIndex = SRand::Inst().Rand( 0u, m_freeSlots.size() );
	uint32 nIndex = m_freeSlots[nFreeSlotIndex];
	m_freeSlots[nFreeSlotIndex] = m_freeSlots.back();
	m_freeSlots.pop_back();
	m_vecSlimes[nIndex] = pSlime;
	pSlime->m_nSlotIndex = nIndex;
	if( m_bSlimeBlink )
		pSlime->Blink( m_slimeColor );
	else
		pSlime->ChangeColor( m_slimeColor );
}

void CSlimeCore::Clear()
{
	for( int i = 0; i < m_vecSlimes.size(); i++ )
	{
		CSlime* pSlime = m_vecSlimes[i];
		if( !pSlime )
			continue;
		pSlime->OnUnbound( m_nHpLeft <= 0 );
		m_vecSlimes[i] = NULL;
		m_freeSlots.push_back( i );
	}
	m_nSlimeFullyBoundCount = 0;
}

float CSlimeCore::GetVelocityWeight()
{
	return 5.0f / ( m_freeSlots.size() + 5.0f );
}

void CSlimeCore::OnPlayerAttack( SPlayerAttackContext* pContext )
{
	if( !m_bCanBeHit )
		return;
	pContext->nResult |= SPlayerAttackContext::eResult_Hit;
	CVector2& pos = pContext->hitPos;
	CEffectObject* pObject = new CEffectObject( 1, &CBloodSplashCanvas::Inst() );
	CParticleSystem* pParticleSystem = CBloodSplashCanvas::Inst().pParticleSystem1;
	pObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pObject->GetAnimController() ) );
	pObject->x = pos.x;
	pObject->y = pos.y;
	pObject->SetParentBeforeEntity( GetParentEntity()->GetParentEntity() );

	uint32 nDmg = pContext->nDmg;
	m_nHpLeft -= nDmg;
	if( m_nHpLeft <= 0 )
	{
		pContext->nResult |= SPlayerAttackContext::eResult_Critical;
		Kill();
	}
}

void CSlimeCore::Kill()
{
	OnKilled();
	SetParentEntity( NULL );
}

void CSlimeCore::OnTickAfterHitTest()
{
	float fTime = GetStage()->GetGlobalElapsedTime();
	if( !IsComplete() )
	{
		if( m_fHitTimeLeft <= 0 )
		{
			CSlime* pSlime = NULL;
			for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				CSlime* pSlime1 = dynamic_cast<CSlime*>( pManifold->pOtherHitProxy );
				if( pSlime1 && !pSlime1->m_pSlimeCore && !pSlime1->m_pTrapped )
				{
					pSlime = pSlime1;
					break;
				}
			}
			if( !pSlime && m_fEnableBoundUntouchedSlimeTime > 0 && m_fEnableBoundUntouchedSlimeTime < -m_fHitTimeLeft )
			{
				CSlimeCoreGenerator* pGenerator = dynamic_cast<CSlimeCoreGenerator*>( GetParent() );
				CSlimeGround* pSlimeGround = pGenerator->GetSlimeGround();
				pSlime = pSlimeGround->Get_UnboundSlime();
			}
			if( pSlime )
			{
				Add( pSlime );
				m_fHitTimeLeft = m_fHitInterval;
			}
		}
		m_fHitTimeLeft -= fTime;
	}

	if( m_bCanBeHit )
	{
		float fColor = m_paramEmission.x;
		fColor += GetStage()->GetElapsedTimePerTick();
		if( fColor > 1 )
			fColor = 1;
		m_paramEmission = CVector4( 1, 1, 1, 1 );
	}
	else
		m_paramEmission = CVector4( 1, 1, 1, 1 );

	if( m_nGrowlInterval )
		m_nGrowlInterval--;
	else
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer && ( pPlayer->GetPosition() - GetPosition() ).Length() <= 256 )
			Growl();
	}

	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSlimeCore::Growl()
{
	PlaySound( 0 );
	m_nGrowlInterval = SRand::Inst().Rand( 600, 1000 );
}

void CSlimeCoreGenerator::OnAddedToStage()
{
	GetStage()->RegisterBeforeHitTest( 1, &m_tickBeforeHitTest );
	GetStage()->RegisterBeforeHitTest( 300, &m_tickGenerate );
}

void CSlimeCoreGenerator::OnRemovedFromStage()
{
	if( m_tickBeforeHitTest.IsRegistered() )
		m_tickBeforeHitTest.Unregister();
	if( m_tickGenerate.IsRegistered() )
		m_tickGenerate.Unregister();
	if( m_onRestart.IsRegistered() )
		m_onRestart.Unregister();
	m_pSlimeGround = NULL;
}

void CSlimeCoreGenerator::OnAddChild( CRenderObject2D* pChild )
{
	CSlimeCore* pSlimeCore = dynamic_cast<CSlimeCore*>( pChild );
	if( pSlimeCore )
		m_nTotalCount += pSlimeCore->m_nCount;
}

void CSlimeCoreGenerator::OnRemoveChild( CRenderObject2D* pChild )
{
	CSlimeCore* pSlimeCore = dynamic_cast<CSlimeCore*>( pChild );
	if( pSlimeCore )
		m_nTotalCount -= pSlimeCore->m_nCount;

	if( GetStage() )
		CheckGenerate();
}

void CSlimeCoreGenerator::SetItems( SGenerateItem* pItems, uint32 nItems )
{
	m_vecGenItems.resize( nItems );
	m_nTotalP = 0;
	for( int i = 0; i < nItems; i++ )
	{
		m_vecGenItems[i] = pItems[i];
		m_nTotalP += m_vecGenItems[i].nP;
	}
}

void CSlimeCoreGenerator::OnTickBeforeHitTest()
{

}

void CSlimeCoreGenerator::CheckGenerate()
{
	if( m_tickGenerate.IsRegistered() )
		return;
	if( m_nTotalCount >= m_nMaxCount )
		return;

	if( !m_nTotalCount && m_pSlimeGround->GetUnboundSlimeCount() < m_vecGenItems[0].nMinSlimeCount )
	{
		CleanUp();
		return;
	}

	uint32 r = SRand::Inst().Rand( 0u, m_nTotalP );
	int i;
	for( i = 0; i < m_vecGenItems.size(); i++ )
	{
		if( r < m_vecGenItems[i].nP )
			break;
		r -= m_vecGenItems[i].nP;
	}
	auto& item = m_vecGenItems[i];
	if( m_nTotalCount + item.nCount > m_nMaxCount || m_pSlimeGround->GetUnboundSlimeCount() < item.nMinSlimeCount )
	{
		GetStage()->RegisterBeforeHitTest( 60, &m_tickGenerate );
		return;
	}

	auto pObj = item.createFunc();
	pObj->m_nCount = item.nCount;
	pObj->SetParentEntity( this );

	GetStage()->RegisterBeforeHitTest( 300, &m_tickGenerate );
}

void CSlimeCoreGenerator::CleanUp()
{
	for( auto pSlime = m_pSlimeGround->Get_UnboundSlime(); pSlime; pSlime = pSlime->NextUnboundSlime() )
	{
		pSlime->SetVelocity( CVector2( 0, 0 ) );
		uint32 nIndex = SRand::Inst().Rand( 0, 8 );
		CSlimeTrap* pSlimeTrap;
		if( SRand::Inst().Rand() & 1 )
		{
			pSlimeTrap = new CSlimeTrap( 0.2f, 10.0f, 28.0f, CRectangle( -32, -32, 64, 64 ),
				CRectangle( 0.125f * ( nIndex >> 1 ), 0.125f * ( nIndex & 1 ) + 0.25f, 0.125f, 0.125f ) );
			pSlimeTrap->AddCircle( 24, CVector2( 0, 0 ) );
		}
		else
		{
			pSlimeTrap = new CSlimeTrap( 0.2f, 10.0f, 24.0f, CRectangle( -64, -16, 128, 32 ),
				CRectangle( 0.25f * ( nIndex & 1 ), 0.0625f * ( nIndex >> 1 ), 0.25f, 0.0625f ) );
			pSlimeTrap->AddRect( CRectangle( -56, -8, 112, 16 ) );
		}
		pSlimeTrap->SetPosition( pSlime->GetPosition() );
		pSlimeTrap->SetRotation( SRand::Inst().Rand( -PI, PI ) );
		pSlimeTrap->SetParentAfterEntity( GetSlimeGround() );
	}

	CSlimeTrap* pSlimeTrap = new CSlimeTrap( 0.2f, 10.0f, 24.0f, CRectangle( -128, -128, 256, 256 ),
		CRectangle( 0.5f, 0.5f, 0.5f, 0.5f ) );
	CUseable* pUseable = new CUseable( "Restart", 1.0f, 64 );
	pUseable->AddRect( CRectangle( -112, -112, 224, 224 ) );
	pUseable->SetParentEntity( pSlimeTrap );
	pUseable->RegisterEntityEvent( eEntityEvent_PlayerUse, &m_onRestart );
	pSlimeTrap->SetParentAfterEntity( GetSlimeGround() );
}

void CSlimeCoreGenerator::OnRestart( CUseable* pUseable )
{
	pUseable->SetEnabled( false );
	GetStage()->GetPlayer()->DelayChangeStage( "test" );
}