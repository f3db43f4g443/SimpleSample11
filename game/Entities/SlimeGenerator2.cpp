#include "stdafx.h"
#include "SlimeGenerator2.h"
#include "Stage.h"
#include "Player.h"
#include "Barrage.h"
#include "Common/Rand.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"

CSlimeGenerator2::CSlimeGenerator2()
	: m_tickAfterHitTest( this, &CSlimeGenerator2::OnTickAfterHitTest )
{
	SetZOrder( -1 );
	m_slimeItems.resize( 256 );
	memset( m_pSlimeItemsGrid, 0, sizeof( m_pSlimeItemsGrid ) );
	m_fGenTime = 0;
	m_nGenCount = 0;
}

void CSlimeGenerator2::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	CSlimeGround* pSlimeGround = dynamic_cast<CSlimeGround*>( GetParentEntity() );

	m_vecGeneratePos.resize( 8 );
	for( int i = 0; i < 8; i++ )
	{
		m_vecGeneratePos[i] = CVector2( ( i - 4 + SRand::Inst().Rand( 0.0f, 1.0f ) ) * 112, SRand::Inst().Rand( 128.0f, 384.0f ) );
	}
}

void CSlimeGenerator2::OnRemovedFromStage()
{
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

void CSlimeGenerator2::OnTickAfterHitTest()
{
	CSlimeGround* pSlimeGround = dynamic_cast<CSlimeGround*>( GetParentEntity() );
	float fTime = GetStage()->GetGlobalElapsedTime();
	if( m_nGenCount < 32 )
	{
		m_fGenTime += fTime;
		while( m_nGenCount < 32 && m_fGenTime >= 1.0f )
		{
			m_fGenTime -= 1.0f;
			m_nGenCount++;
			for( auto& pos : m_vecGeneratePos )
			{
				CSlime* pSlime = new CSlime2( pSlimeGround, CVector2( 0, 0 ), 16 );
				pSlime->SetPosition( pos );
				pSlime->x += SRand::Inst().Rand( -2.0f, 2.0f );
				pSlime->y += SRand::Inst().Rand( -2.0f, 2.0f );
				pSlime->SetParentEntity( pSlimeGround );
			}
		}

		if( m_nGenCount >= 32 )
		{
			CSlimeCoreGenerator2* pSlimeCoreGenerator = new CSlimeCoreGenerator2( pSlimeGround );
			pSlimeCoreGenerator->SetParentEntity( pSlimeGround );
		}
	}

	uint32 iSlime = 0;
	for( auto pChild = pSlimeGround->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		CSlime* pSlime = dynamic_cast<CSlime*>( pChild );
		if( pSlime )
		{
			pSlime->SetVelocity( pSlime->GetVelocity() + CVector2( 0, -20 ) * fTime );
			auto& item = m_slimeItems[iSlime++];
			item.pSlime = pSlime;
			int32 x = Max( 0, Min( 15, (int32)floor( ( pSlime->x + 512 ) / 64 ) ) );
			int32 y = Max( 0, Min( 15, (int32)floor( ( pSlime->y + 512 ) / 64 ) ) );

			int32 x1 = x > 0 ? x - 1 : 0;
			int32 y1 = y > 0 ? y - 1 : 0;
			int32 x2 = x < 15 ? x + 1 : 15;
			int32 y2 = y < 15 ? y + 1 : 15;
			int32 k = 16;
			for( int i = x1; i < x2 && k; i++ )
			{
				for( int j = y1; j < y2 && k; j++ )
				{
					for( auto pItem = m_pSlimeItemsGrid[i][j]; pItem && k; pItem = pItem->NextItem() )
					{
						CVector2 dPos = pItem->pSlime->GetPosition() - pSlime->GetPosition();
						float l2 = dPos.Dot( dPos );
						if( l2 < 0.0001f || l2 > 64 * 64 )
							continue;

						float invl2 = 1.0f / l2;
						float invl4 = invl2 * invl2;
						float invl6 = invl4 * invl2;
						float fForce = Max( -1000.0f / sqrtf( l2 ), 64 * ( -invl2 + invl4 * 3 * 32 - invl6 * 2 * 32 * 32 ) );
						CVector2 force = dPos * fForce;
						pSlime->SetVelocity( pSlime->GetVelocity() + force * fTime );
						pItem->pSlime->SetVelocity( pItem->pSlime->GetVelocity() - force * fTime );
						k--;
					}
				}
			}

			item.InsertTo_Item( m_pSlimeItemsGrid[x][y] );
			if( iSlime >= 256 )
				break;
		}
	}

	for( auto pChild = pSlimeGround->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		CSlime* pSlime = dynamic_cast<CSlime*>( pChild );
		if( pSlime )
		{
			CVector2 curVelocity = pSlime->GetVelocity();
			float l = curVelocity.Normalize();
			float l1 = Max( 0.0f, l - 10 * fTime );
			pSlime->SetVelocity( curVelocity * l1 );
		}
	}

	memset( m_pSlimeItemsGrid, 0, sizeof( m_pSlimeItemsGrid ) );
	
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

CSlimeCore4::CSlimeCore4()
	: m_onHit( this, &CSlimeCore4::OnHit )
	, m_bHit( false )
	, m_bIsAttack( false )
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
	m_tailTransformIndex = pAnimSet->GetSkeleton().GetBoneIndex( "bone57" );
	m_fEnableBoundUntouchedSlimeTime = 1.0f;
	m_slimeColor = CVector4( 0.5, 0.5, 0.5, 1 );

	m_hangTargetPos = CVector2( 0, 460 );
	m_fHangLength = 700;
	SetRotation( PI );
}

void CSlimeCore4::OnAddedToStage()
{
	CSlimeCore::OnAddedToStage();
	m_pCurAnim = GetAnimController()->PlayAnim( "walk", eAnimPlayMode_Loop );
}

void CSlimeCore4::OnRemovedFromStage()
{
	CSlimeCore::OnRemovedFromStage();
	if( m_onHit.IsRegistered() )
		m_onHit.Unregister();
}

void CSlimeCore4::OnTickBeforeHitTest()
{
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
			//SetPosition( m_attackVelocity * fTime + GetPosition() );

			if( !m_pCurAnim->GetController() )
			{
				m_pCurAnim = GetAnimController()->PlayAnim( "idle", eAnimPlayMode_Loop );
				m_bIsAttack = false;
				m_fAttackCD = SRand::Inst().Rand( 3.0f, 6.0f );
			}
		}
		else if( m_fAttackCD <= 0 )
		{
			CVector2& hangPos = m_hangTargetPos;
			float x = pPlayer->GetPosition().x;
			float fMoveSpeed = 100;
			if( hangPos.x < x )
			{
				hangPos.x += fMoveSpeed * fTime;
				if( hangPos.x > x )
					hangPos.x = x;
			}
			else if( hangPos.x > x )
			{
				hangPos.x -= fMoveSpeed * fTime;
				if( hangPos.x < x )
					hangPos.x = x;
			}

			float& fHangLength = m_fHangLength;
			float y = m_nSlimeFullyBoundCount == m_vecSlimes.size() ? hangPos.y - ( pPlayer->GetPosition().y + 512 ) : 700;
			if( y < 128 )
				y = 128;
			float fMoveSpeed1 = 50;
			if( fHangLength < y )
			{
				fHangLength += fMoveSpeed1 * fTime;
				if( fHangLength > y )
					fHangLength = y;
			}
			else if( fHangLength > y )
			{
				fHangLength -= fMoveSpeed1 * fTime;
				if( fHangLength < y )
					fHangLength = y;
			}
			
			if( m_nSlimeFullyBoundCount == m_vecSlimes.size() && abs( hangPos.x - x ) < 256 && GetPosition().y - pPlayer->GetPosition().y > 256 )
			{
				m_pCurAnim = GetAnimController()->PlayAnim( "attack", eAnimPlayMode_Once );
				m_pCurAnim->RegisterEvent( 0, &m_onHit );
				m_bIsAttack = true;
			}
		}
		if( !pPlayer->IsInHorrorReflex() )
			m_bCanBeHit = false;
	}
	
	CSlimeCore::OnTickBeforeHitTest();
	UpdateDirty();
	
	const CMatrix2D& trans = GetAnimController()->GetTransform( m_tailTransformIndex );
	CVector2 hangPos1 = trans.MulVector2Pos( CVector2( 0, -m_fHangLength ) );
	SetPosition( GetPosition() + m_hangTargetPos - hangPos1 );
}

void CSlimeCore4::OnTickAfterHitTest()
{
	if( m_bHit )
	{
		m_bHit = false;
		OnHit();
	}

	CSlimeCore::OnTickAfterHitTest();
}

void CSlimeCore4::OnHit()
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

CSlimeCoreGenerator2::CSlimeCoreGenerator2( CSlimeGround* pSlimeGround )
	: CSlimeCoreGenerator( pSlimeGround )
{
	SetMaxCount( 10 );
	SGenerateItem items[] =
	{
		{
			1, 6, 160, [] () {
				CSlimeCore4* pSlimeCore4 = new CSlimeCore4;
				return pSlimeCore4;
			}
		},
		{
			1, 6, 160, [] () {
				CSlimeCore4* pSlimeCore4 = new CSlimeCore4;
				return pSlimeCore4;
			}
		},
		{
			1, 6, 160, [] () {
				CSlimeCore4* pSlimeCore4 = new CSlimeCore4;
				return pSlimeCore4;
			}
		}
	};
	SetItems( items, ELEM_COUNT( items ) );
}