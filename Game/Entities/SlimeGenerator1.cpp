#include "stdafx.h"
#include "SlimeGenerator1.h"
#include "Stage.h"
#include "Player.h"
#include "Render/LightRendering.h"
#include "Render/Sound.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Common/Rand.h"
#include "Barrage.h"

void CSlime1::ChangeVelocity( bool bExplode )
{
	CSlime::ChangeVelocity( bExplode );
	uint32 nFrames = SRand::Inst().Rand( 60, 120 );
	float fAngle = SRand::Inst().Rand( -PI, PI );
	float fRad = bExplode? SRand::Inst().Rand( 16.1f, 24.0f ): SRand::Inst().Rand( 4.0f, 16.0f );
	fRad = fRad * fRad;
	m_velocity = CVector2( fRad * cos( fAngle ), fRad * sin( fAngle ) );
	if( m_pSlimeCore )
	{
		float fWeight = m_pSlimeCore->GetVelocityWeight();
		if( fWeight == 1 )
		{
			m_fSpeed = m_velocity.Length();
			m_nBoundState = 1;
			return;
		}
		CVector2 target = m_pSlimeCore->GetAnimController()->GetTransform( m_nSlotIndex ).GetPosition();
		CVector2 v1 = ( target - GetPosition() ) * ( 60.0f / nFrames );
		m_velocity = v1 * fWeight + m_velocity * ( 1 - fWeight );
	}
}

CSlimeCore1::CSlimeCore1()
	: m_onHit( this, &CSlimeCore1::OnEventHit )
{
	m_nType = 0;
	m_nState = 0;
	static CReference<CAnimationSet> pAnimSet = NULL;
	static CDefaultDrawable2D* pDrawable = NULL;
	if( !pAnimSet )
	{
		vector<char> content;
		GetFileContent( content, "anims/slime_core_1.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content[0] );
		pAnimSet = new CAnimationSet;
		pAnimSet->LoadXml( doc1.RootElement() );
		
		GetFileContent( content, "materials/slime_core_1.xml", true );
		TiXmlDocument doc2;
		doc2.LoadFromBuffer( &content[0] );
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc2.RootElement()->FirstChildElement() );
	}	

	CImage2D* pImage2D = new CImage2D( pDrawable, NULL, CRectangle( -16, -16, 32, 32 ), CRectangle( 0, 0, 1, 1 ) );
	pImage2D->SetInstData( &m_paramEmission, sizeof( m_paramEmission ) );
	SetRenderObject( pImage2D );

	AddCircle( 16, CVector2( 0, 0 ) );
	SetAnimSet( pAnimSet );

	m_velocity = CVector2( SRand::Inst().Rand( -50, 50 ), SRand::Inst().Rand( -50, 50 ) );
	m_nHpLeft = 5;
	m_fAttackTime = 0;
	m_bHit = false;
	m_fEnableBoundUntouchedSlimeTime = 2.0f;
	m_slimeColor = CVector4( 0.5f, 0, 0, 1 );
}

void CSlimeCore1::Clear()
{
	m_nState = 0;
	CSlimeCore::Clear();
	if( m_pCurAnim )
	{
		GetAnimController()->StopAnim( m_pCurAnim );
		m_pCurAnim = NULL;
	}
	m_fAttackTime = 0;
	m_bHit = false;
}

void CSlimeCore1::OnRemovedFromStage()
{
	CSlimeCore::OnRemovedFromStage(); 
	if( m_onHit.IsRegistered() )
		m_onHit.Unregister();
}

void CSlimeCore1::OnTickBeforeHitTest()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	float fTime = GetStage()->GetGlobalElapsedTime();

	bool bFullyBound = m_nSlimeFullyBoundCount == m_vecSlimes.size();

	if( m_nState == 0 )
	{
		CVector2 acc( 0, 0 );
		if( pPlayer && bFullyBound )
		{
			float fAcc = 200;
			CVector2 dPos = pPlayer->GetPosition() - ( GetPosition() + m_velocity * ( m_velocity.Length() / fAcc ) * 0.25f );
			float f = dPos.Normalize();
			if( f > 0.001f )
				acc = dPos * fAcc;
		}
		SetPosition( GetPosition() + m_velocity * fTime + acc * fTime * fTime * 0.5f );
		m_velocity = m_velocity + acc * fTime;
	}
	else
	{
		if( pPlayer )
			CommonMove( 200, fTime, pPlayer->GetPosition() - GetPosition(), 0 );
	}

	if( pPlayer )
	{
		if( bFullyBound )
		{
			if( m_nState == 0 )
			{
				float l = ( pPlayer->GetPosition() - GetPosition() ).Length();
				if( l < 64 )
				{
					m_nState = 1;
					m_pCurAnim = GetAnimController()->PlayAnim( "attack", eAnimPlayMode_OnceNoRemove );
					m_pCurAnim->RegisterEvent( 0, &m_onHit );
				}
			}
		}
		if( !pPlayer->IsInHorrorReflex() )
		{
			if( m_pCurAnim && m_pCurAnim->GetCurTime() >= m_pCurAnim->GetTotalTime() )
			{
				Clear();
			}
			m_bCanBeHit = false;
		}
	}

	CSlimeCore::OnTickBeforeHitTest();

	if( m_fHitTimeLeft < -5.0f )
		Kill();
}

void CSlimeCore1::OnTickAfterHitTest()
{
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity && pEntity->GetHitType() == eEntityHitType_WorldStatic )
		{
			if( m_velocity.Dot( pManifold->normal ) > 0 )
			{
				CVector2 norm = pManifold->normal;
				float l = norm.Normalize();
				if( l < 0.001f )
					continue;
				m_velocity = m_velocity - norm * ( 2 * m_velocity.Dot( norm ) ) * SRand::Inst().Rand( 0.9f, 1.1f );
			}
		}
	}

	if( m_bHit )
	{
		m_bHit = false;
		OnHit();
	}
	CSlimeCore::OnTickAfterHitTest();
}

void CSlimeCore1::OnHit()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	if( !pPlayer->IsInHorrorReflex() )
	{
		if( pPlayer->CanBeHit() )
		{
			SDamage dmg;
			dmg.pSource = this;
			dmg.nHp = 10;
			pPlayer->Damage( dmg );
		}
	}
	else
	{
		SBarrageContext context;
		context.vecObjects.push_back( this );
		context.vecObjects.push_back( pPlayer->GetCrosshair() );
				
		CBarrage_Old* pBarrage = new CBarrage_Old( context );
		for( int iAngle = 0; iAngle < 24; iAngle++ )
		{
			pBarrage->RegisterBullet( 40, 350, iAngle * PI / 12 );
		}
		pBarrage->SetParentBeforeEntity( GetParentEntity() );
		pBarrage->SetPosition( GetPosition() );
		pBarrage->Start();
		m_bCanBeHit = true;
	}
	PlaySound( 1 );
	m_nGrowlInterval = SRand::Inst().Rand( 600, 1000 );
}

void CSlimeCore1::OnKilled()
{
	if( m_nHpLeft <= 0 )
	{
		CSlimeCoreGenerator* pGenerator = dynamic_cast<CSlimeCoreGenerator*>( GetParent() );
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
		pSlimeTrap->AddCircle( 24, CVector2( 0, 0 ) );
		pSlimeTrap->SetPosition( GetPosition() );
		pSlimeTrap->SetRotation( SRand::Inst().Rand( -PI, PI ) );
		pSlimeTrap->SetParentAfterEntity( pGenerator->GetSlimeGround() );
	}
	CSlimeCore::OnKilled();
}

CSlimeCore2::CSlimeCore2()
{
	m_nType = 1;
	static CReference<CAnimationSet> pAnimSet = NULL;
	static CDefaultDrawable2D* pDrawable = NULL;
	if( !pAnimSet )
	{
		vector<char> content;
		GetFileContent( content, "anims/slime_core_2.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content[0] );
		pAnimSet = new CAnimationSet;
		pAnimSet->LoadXml( doc1.RootElement() );
		
		GetFileContent( content, "materials/slime_core_2.xml", true );
		TiXmlDocument doc2;
		doc2.LoadFromBuffer( &content[0] );
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc2.RootElement()->FirstChildElement() );
	}

	CImage2D* pImage2D = new CImage2D( pDrawable, NULL, CRectangle( -64, -64, 128, 128 ), CRectangle( 0, 0, 1, 1 ) );
	pImage2D->SetInstData( &m_paramEmission, sizeof( m_paramEmission ) );
	SetRenderObject( pImage2D );

	AddCircle( 64, CVector2( 0, 0 ) );
	SetAnimSet( pAnimSet );

	m_velocity = CVector2( 50, 50 );
	m_fAttackTime = 0;
	m_nHpLeft = 20;
	m_bOK = false;
	m_fEnableBoundUntouchedSlimeTime = 1.0f;
	m_bSlimeBlink = true;
	m_slimeColor = CVector4( 1.0f, 3, 3, 0.5f );
}

void CSlimeCore2::Clear()
{
	CSlimeCore::Clear();
	m_fAttackTime = 0;
	m_bOK = false;
	for( int i = 0; i < ELEM_COUNT( m_pAttackEffects ); i++ )
	{
		if( m_pAttackEffects[i] )
		{
			m_pAttackEffects[i]->RemoveThis();
			m_pAttackEffects[i] = NULL;
		}
	}
	m_nAttackType = 0;
}

float CSlimeCore2::GetVelocityWeight()
{
	return 10.0f / ( m_freeSlots.size() + ( m_bOK ? 0 : 1 ) + 10.0f );
}

void CSlimeCore2::OnTickBeforeHitTest()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	float fTime = GetStage()->GetGlobalElapsedTime();

	CVector2 acc( 0, 0 );
	if( pPlayer && IsComplete() )
	{
		float fAcc = 50;
		CVector2 dPos = pPlayer->GetPosition() - ( GetPosition() + m_velocity * ( m_velocity.Length() / fAcc ) * 0.5f );
		float f = dPos.Normalize();
		if( f > 0.001f )
			acc = dPos * fAcc;
	}

	SetPosition( GetPosition() + m_velocity * fTime + acc * fTime * fTime * 0.5f );
	m_velocity = m_velocity + acc * fTime;

	if( pPlayer )
	{
		if( IsComplete() && !m_bOK )
		{
			CVector2 dPos = GetPosition() + m_velocity * 0.5f - pPlayer->GetPosition();
			float fRad = pPlayer->GetMoveSpeed();
			if( dPos.Length() < 256 - fRad )
			{
				m_bOK = true;
			}
		}
		if( !pPlayer->IsInHorrorReflex() )
			m_bCanBeHit = false;
	}

	if( m_nSlimeFullyBoundCount == m_vecSlimes.size() )
	{
		UpdateAttackEffect();
		float fPreAttackTime = m_fAttackTime;
		m_fAttackTime += fTime;
		if( fPreAttackTime == 0 )
		{
			BeginAttack();
		}
		else if( fPreAttackTime < 0.5f && m_fAttackTime >= 0.5f )
		{
			OnHit();
		}
		else if( m_fAttackTime > 2.2f )
			Clear();
	}

	float fPreHitTime = m_fHitTimeLeft;
	CSlimeCore::OnTickBeforeHitTest();

	if( m_fHitTimeLeft < -10.0f )
		Kill();
}

void CSlimeCore2::OnTickAfterHitTest()
{
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity && pEntity->GetHitType() == eEntityHitType_WorldStatic )
		{
			if( m_velocity.Dot( pManifold->normal ) > 0 )
			{
				CVector2 norm = pManifold->normal;
				float l = norm.Normalize();
				if( l < 0.001f )
					continue;
				m_velocity = m_velocity - norm * ( 2 * m_velocity.Dot( norm ) ) * SRand::Inst().Rand( 0.9f, 1.1f );
			}
		}
	}
	CSlimeCore::OnTickAfterHitTest();
}

void CSlimeCore2::BeginAttack()
{
	m_nAttackType = SRand::Inst().Rand( 0, 2 );
	if( m_nAttackType == 0 )
	{
		CPointLightObject* pLight = new CPointLightObject( CVector4( 0.5, 0, 50, -1 ), CVector3( 5, 3, 3 ), 10.0f, 0.1f, 0.4f );
		pLight->x = 48;
		pLight->y = 52;
		m_pAttackEffects[0] = pLight;
		AddChild( pLight );
		pLight = new CPointLightObject( CVector4( 0.5, 0, 50, -1 ), CVector3( 5, 3, 3 ), 10.0f, 0.1f, 0.4f );
		pLight->x = -48;
		pLight->y = 52;
		pLight->s = 0.75f;
		m_pAttackEffects[1] = pLight;
		AddChild( pLight );
	}
	else
	{
		CPointLightObject* pLight = new CPointLightObject( CVector4( 0.5, 0, 50, -1 ), CVector3( 5, 3, 3 ), 10.0f, 0.1f, 0.4f );
		pLight->x = 0;
		pLight->y = -79;
		pLight->s = 1.5f;
		m_pAttackEffects[0] = pLight;
		AddChild( pLight );
	}
}

void CSlimeCore2::UpdateAttackEffect()
{
	float fPercent = m_fAttackTime < 0.2f ? m_fAttackTime * 5 : 1 - ( m_fAttackTime - 0.2f ) * 0.5f;
	fPercent = Min( 1.0f, Max( fPercent, 0.0f ) );
	for( int i = 0; i < ELEM_COUNT( m_pAttackEffects ); i++ )
	{
		if( !m_pAttackEffects[i] )
			return;
		CPointLightObject* pLight = dynamic_cast<CPointLightObject*>( m_pAttackEffects[i].GetPtr() );
		pLight->AttenuationIntensity.w = -2.0f + fPercent;
	}
}

void CSlimeCore2::OnHit()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	if( !pPlayer->IsInHorrorReflex() )
	{
		if( pPlayer->CanBeHit() )
		{
			SDamage dmg;
			dmg.pSource = this;
			dmg.nHp = 10;
			pPlayer->Damage( dmg );
		}
	}
	else
	{
		if( m_nAttackType == 0 )
		{
			for( int i = 0; i < 2; i++ )
			{
				SBarrageContext context;
				context.vecObjects.push_back( this );
				context.vecObjects.push_back( pPlayer->GetCrosshair() );
				CVector2 pos = m_pAttackEffects[i]->globalTransform.GetPosition();
				
				CBarrage_Old* pBarrage = new CBarrage_Old( context );
				for( int iWave = 0; iWave < 4; iWave++ )
				{
					for( int iAngle = 0; iAngle < 12; iAngle++ )
					{
						CVector2 dir( cos( iAngle * PI / 6 ), sin( iAngle * PI / 6 ) );
						pBarrage->RegisterBulletWithTarget( iWave * 60 + 1, 250, 0.25f, dir * 16, dir * 160 );
					}
				}
				pBarrage->SetParentBeforeEntity( GetParentEntity() );
				pBarrage->SetPosition( pos );
				pBarrage->Start();
				m_bCanBeHit = true;
			}
		}
		else
		{
			SBarrageContext context;
			context.vecObjects.push_back( this );
			context.vecObjects.push_back( pPlayer->GetCrosshair() );
			CVector2 pos = m_pAttackEffects[0]->globalTransform.GetPosition();
				
			CBarrage_Old* pBarrage = new CBarrage_Old( context );
			for( int iWave = 0; iWave < 4; iWave++ )
			{
				for( int iAngle = -3; iAngle <= 3; iAngle++ )
				{
					pBarrage->RegisterBulletWithTarget( iWave * 60 + 1, 300, iAngle * PI / 18 );
				}
				for( int iAngle = 0; iAngle <= 6; iAngle++ )
				{
					pBarrage->RegisterBulletWithTarget( iWave * 60 + 31, 300, ( iAngle - 2.5f ) * PI / 18 );
				}
			}
			pBarrage->RegisterBloodLaserWithTarget( 1, 4.0f, 0, PI / 6 );
			pBarrage->RegisterBloodLaserWithTarget( 1, 4.0f, 0, -PI / 6 );
			pBarrage->SetParentBeforeEntity( GetParentEntity() );
			pBarrage->SetPosition( pos );
			pBarrage->Start();
			m_bCanBeHit = true;
		}
	}
	PlaySound( 1 );
	m_nGrowlInterval = SRand::Inst().Rand( 600, 1000 );
}

void CSlimeCore2::OnKilled()
{
	if( m_nHpLeft <= 0 )
	{
		CSlimeCoreGenerator* pGenerator = dynamic_cast<CSlimeCoreGenerator*>( GetParent() );
		uint32 nIndex = SRand::Inst().Rand( 0, 8 );
		CSlimeTrap* pSlimeTrap = new CSlimeTrap( 0.2f, 10.0f, 50.0f, CRectangle( -64, -64, 128, 128 ),
			CRectangle( 0, 0.5f, 0.25f, 0.25f ) );
		pSlimeTrap->AddRect( CRectangle( -56, -56, 112, 112 ) );
		pSlimeTrap->SetPosition( GetPosition() );
		pSlimeTrap->SetRotation( GetRotation() );
		pSlimeTrap->SetParentAfterEntity( pGenerator->GetSlimeGround() );
	}
	CSlimeCore::OnKilled();
}

CSlimeCore3::CSlimeCore3()
	: m_onHit( this, &CSlimeCore3::OnEventHit )
	, m_bIsAttack( false )
	, m_bHit( false )
	, m_fAttackCD( 0 )
{
	m_nType = 2;
	static CReference<CAnimationSet> pAnimSet = NULL;
	static CDefaultDrawable2D* pDrawable = NULL;
	if( !pAnimSet )
	{
		vector<char> content;
		GetFileContent( content, "anims/slime_core_3.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content[0] );
		pAnimSet = new CAnimationSet;
		pAnimSet->LoadXml( doc1.RootElement() );
		
		GetFileContent( content, "materials/slime_core_3.xml", true );
		TiXmlDocument doc2;
		doc2.LoadFromBuffer( &content[0] );
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc2.RootElement()->FirstChildElement() );
	}

	CImage2D* pImage2D = new CImage2D( pDrawable, NULL, CRectangle( -64, -64, 128, 128 ), CRectangle( 0, 0, 1, 1 ) );
	pImage2D->SetInstData( &m_paramEmission, sizeof( m_paramEmission ) );
	SetRenderObject( pImage2D );

	AddCircle( 64, CVector2( 0, 0 ) );
	SetAnimSet( pAnimSet );
	m_fEnableBoundUntouchedSlimeTime = 1.0f;
	m_slimeColor = CVector4( 0, 2, 0, 1 );
}

void CSlimeCore3::OnAddedToStage()
{
	CSlimeCore::OnAddedToStage();
	m_pCurAnim = GetAnimController()->PlayAnim( "walk", eAnimPlayMode_Loop );
}

void CSlimeCore3::OnRemovedFromStage()
{
	CSlimeCore::OnRemovedFromStage();
	if( m_onHit.IsRegistered() )
		m_onHit.Unregister();
}

void CSlimeCore3::OnTickBeforeHitTest()
{
	float fPreHitTime = m_fHitTimeLeft;
	CSlimeCore::OnTickBeforeHitTest();

	if( m_fHitTimeLeft < -10.0f )
	{
		Kill();
		return;
	}

	float fTime = GetStage()->GetGlobalElapsedTime();
	if( m_fAttackCD > 0 )
	{
		m_fAttackCD -= fTime;
		if( m_fAttackCD <= 0 )
		{
			m_fAttackCD = 0;
			m_pCurAnim = GetAnimController()->PlayAnim( "walk", eAnimPlayMode_Loop );
		}
		else
			return;
	}

	CPlayer* pPlayer = GetStage()->GetPlayer();
	float dRotation;
	if( pPlayer )
	{
		if( m_bIsAttack )
		{
			if( !m_pCurAnim->GetController() )
			{
				m_pCurAnim = GetAnimController()->PlayAnim( "idle", eAnimPlayMode_Loop );
				m_bIsAttack = false;
				m_fAttackCD = 1.0f;
			}
		}
		else if( m_fAttackCD <= 0 )
		{
			bool bMoved = CommonMove( 200, 8.0f, fTime, pPlayer->GetPosition() - GetPosition(), 50, dRotation );
			if( !bMoved && m_nSlimeFullyBoundCount == m_vecSlimes.size() )
			{
				m_pCurAnim = GetAnimController()->PlayAnim( "attack", eAnimPlayMode_Once );
				m_pCurAnim->RegisterEvent( 0, &m_onHit );
				m_bIsAttack = true;
			}
		}
		if( !pPlayer->IsInHorrorReflex() )
			m_bCanBeHit = false;
	}
}

void CSlimeCore3::OnTickAfterHitTest()
{
	if( m_bHit )
	{
		m_bHit = false;
		OnHit();
	}

	CSlimeCore::OnTickAfterHitTest();
}

void CSlimeCore3::OnHit()
{
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( !pPlayer )
		return;
	if( !pPlayer->IsInHorrorReflex() )
	{
		if( pPlayer->CanBeHit() )
		{
			SDamage dmg;
			dmg.pSource = this;
			dmg.nHp = 10;
			pPlayer->Damage( dmg );
		}
	}
	else
	{
		uint16 nIndex = GetAnimController()->GetAnimSet()->GetSkeleton().GetBoneIndex( "head" );
		const CMatrix2D& mat = GetAnimController()->GetTransform( nIndex );
		CVector2 pos = mat.GetPosition();

		SBarrageContext context;
		context.vecObjects.push_back( this );
		context.vecObjects.push_back( pPlayer );
		CBarrage_Old* pBarrage = new CBarrage_Old( context );
		for( int i = 0; i < 72; i++ )
			pBarrage->RegisterBullet( i * 2 + 1, 200 + i * 2, i * PI / 12 );
		for( int i = 0; i < 4; i++ )
			pBarrage->RegisterBloodLaser( 30, 0, PI * 0.2, i * PI * 0.5f );
		pBarrage->SetParentBeforeEntity( GetParentEntity() );
		pBarrage->SetPosition( pos );
		pBarrage->r = r;
		pBarrage->Start();
		m_bCanBeHit = true;
	}
	PlaySound( 1 );
	m_nGrowlInterval = SRand::Inst().Rand( 600, 1000 );
}

void CSlimeCore3::OnKilled()
{
	if( m_nHpLeft <= 0 )
	{
		CSlimeCoreGenerator* pGenerator = dynamic_cast<CSlimeCoreGenerator*>( GetParent() );
		uint32 nIndex = SRand::Inst().Rand( 0, 8 );
		CSlimeTrap* pSlimeTrap = new CSlimeTrap( 0.2f, 10.0f, 50.0f, CRectangle( -64, -64, 128, 128 ),
			CRectangle( 0, 0.75f, 0.25f, 0.25f ) );
		pSlimeTrap->AddRect( CRectangle( -56, -56, 112, 112 ) );
		pSlimeTrap->SetPosition( GetPosition() );
		pSlimeTrap->SetRotation( GetRotation() );
		pSlimeTrap->SetParentAfterEntity( pGenerator->GetSlimeGround() );
	}
	CSlimeCore::OnKilled();
}

CSlimeCoreGenerator1::CSlimeCoreGenerator1( CSlimeGround* pSlimeGround )
	: CSlimeCoreGenerator( pSlimeGround )
{
	SetMaxCount( 10 );
	SGenerateItem items[] =
	{
		{
			5, 1, 15, [] () {
				CSlimeCore1* pSlimeCore1 = new CSlimeCore1;
				pSlimeCore1->x = SRand::Inst().Rand( -400, 400 );
				pSlimeCore1->y = SRand::Inst().Rand( -400, 400 );
				return pSlimeCore1;
			}
		},
		{
			2, 3, 80, [] () {
				CSlimeCore2* pSlimeCore2 = new CSlimeCore2;
				pSlimeCore2->x = SRand::Inst().Rand( -400, 400 );
				pSlimeCore2->y = SRand::Inst().Rand( -400, 400 );
				return pSlimeCore2;
			}
		},
		{
			1, 6, 160, [] () {
				CSlimeCore3* pSlimeCore3 = new CSlimeCore3;
				pSlimeCore3->x = SRand::Inst().Rand( -300, 300 );
				pSlimeCore3->y = SRand::Inst().Rand( -300, 300 );
				return pSlimeCore3;
			}
		}
	};
	SetItems( items, ELEM_COUNT( items ) );
}