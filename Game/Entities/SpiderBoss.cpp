#include "stdafx.h"
#include "SpiderBoss.h"
#include "SpiderBossBullet.h"
#include "SpiderBossEgg.h"
#include "SpiderWebGround.h"
#include "BloodLaser.h"

#include "Stage.h"
#include "Player.h"
#include "EnemyBullet.h"
#include "Common/Rand.h"
#include "EffectObject.h"
#include "Render/ParticleSystem.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Render/TextureAtlas.h"
#include "Render/Image2D.h"
#include "Render/Rope2D.h"
#include "Physics/HitProxyData.h"

extern CParticleSystem* __getBloodEffect();
class CSpiderBossLimb : public CEntity
{
	friend class CSpiderBoss;
public:
	enum
	{
		eType_Leg_Begin,
		eType_Leg_End = eType_Leg_Begin + 8,
		eType_Head = eType_Leg_End,
		eType_Body,
		eType_Tail,

		eType_Count
	};

	CSpiderBossLimb( uint8 nType, CSpiderBoss* pPar )
		: m_nType( nType )
		, m_nState( 0 )
		, m_nHpLeft( 20 )
		, m_onPlayerAttack( this, &CSpiderBossLimb::OnPlayerAttack )
		, m_onHitHangRope( this, &CSpiderBossLimb::OnPlayerAttack )
		, m_tickAfterHitTest( this, &CSpiderBossLimb::OnTickAfterHitTest )
	{
		m_param = CVector4( 0.067f, 0.25f, 0, 0 );
		if( nType == eType_Body )
		{
			m_param = CVector4( 0.15f, 0.05f, 0, 0 );
			m_param1 = CVector4( 0.067f, 0.05f, 0, 0 );
			m_param2 = CVector4( 0.18f, 0.08f, 0, 0 );
		}
		m_paramEmission = CVector4( 0, 0, 0, 0 );
		SetTransformIndex( m_nType + 11 );
		SetHitType( eEntityHitType_Sensor );
		
		static CDefaultDrawable2D* pDrawable = NULL;
		static CDefaultDrawable2D* pDrawable1 = NULL;
		static CDefaultDrawable2D* pDrawableEmission = NULL;
		static CTextureAtlas* pTextureAtlas = NULL;
		static CHitProxyDataSet* pHitProxyDataSet = NULL;
		static CDefaultDrawable2D* pFootprintDrawable = NULL;
		if( !pDrawable )
		{
			vector<char> content;
			GetFileContent( content, "materials/spider_boss.xml", true );
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			pDrawable = new CDefaultDrawable2D;
			pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
			pDrawable1 = new CDefaultDrawable2D;
			pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
			pDrawableEmission = new CDefaultDrawable2D;
			pDrawableEmission->LoadXml( doc.RootElement()->FirstChildElement( "color_pass1" ) );
			pFootprintDrawable = new CDefaultDrawable2D;
			pFootprintDrawable->LoadXml( doc.RootElement()->FirstChildElement( "footprint" ) );
			doc.Clear();

			GetFileContent( content, "textures/spider_boss.xml", true );
			TiXmlDocument doc2;
			doc2.LoadFromBuffer( &content[0] );
			pTextureAtlas = new CTextureAtlas;
			pTextureAtlas->LoadXml( doc2.RootElement() );
			doc2.Clear();
			
			GetFileContent( content, "anims/spider_boss_hit.xml", true );
			TiXmlDocument doc3;
			doc3.LoadFromBuffer( &content[0] );
			pHitProxyDataSet = new CHitProxyDataSet;
			pHitProxyDataSet->LoadXml( doc3.RootElement() );
			doc3.Clear();

		}

		static const char* szImageNames[] = { "s_leg00", "s_leg01", "s_leg02", "s_leg03", "s_leg10", "s_leg11", "s_leg12", "s_leg13", "s_head", "s_body", "s_tail", };
		static const char* szImageNames1[] = { "s_leg00_1", "s_leg01_1", "s_leg02_1", "s_leg03_1", "s_leg10_1", "s_leg11_1", "s_leg12_1", "s_leg13_1", "s_head_1", "s_body_1", "s_tail_1", };
		static const char* szImageNames2[] = { "s_leg00_e", "s_leg01_e", "s_leg02_e", "s_leg03_e", "s_leg10_e", "s_leg11_e", "s_leg12_e", "s_leg13_e", NULL, "s_body_e", "s_tail_e", };
		
		auto pItem = pTextureAtlas->GetItem( szImageNames[m_nType] );
		const CVector2& size = pTextureAtlas->GetSize();
		m_pImage1 = new CImage2D( pDrawable, pDrawable1, pItem->frameRect.Offset( pItem->ofs ),
			pItem->rect * CVector2( 1.0f / size.x, 1.0f / size.y ) );
		m_pImage1->SetInstData( &m_param, sizeof( m_param ) );

		if( szImageNames2[m_nType] )
		{
			pItem = pTextureAtlas->GetItem( szImageNames2[m_nType] );
			auto pImage2D = new CImage2D( pDrawableEmission, NULL, pItem->frameRect.Offset( pItem->ofs ),
				pItem->rect * CVector2( 1.0f / size.x, 1.0f / size.y ) );
			pImage2D->SetInstData( &m_paramEmission, sizeof( m_paramEmission ) );
			m_pImage1->AddChild( pImage2D );
		}

		pItem = pTextureAtlas->GetItem( szImageNames1[m_nType] );
		CRectangle footprintRect = pItem->frameRect.Offset( pItem->ofs );
		CRectangle footprintTexRect = pItem->rect * CVector2( 1.0f / size.x, 1.0f / size.y );
		m_hangRopeOfs = pItem->ofs;
		m_pImage2 = new CImage2D( pDrawable, pDrawable1, footprintRect, footprintTexRect );
		m_pImage2->SetInstData( &m_param, sizeof( m_param ) );
		m_footprint.SetDrawable( pFootprintDrawable );
		m_footprint.rect = footprintRect;
		m_footprint.texRect = footprintTexRect;
		m_footprint.pInstData = &m_param;
		m_footprint.nInstDataSize = sizeof( m_param );

		if( m_nType == eType_Body )
		{
			pItem = pTextureAtlas->GetItem( "s_body_2" );
			footprintRect = pItem->frameRect;
			footprintTexRect = pItem->rect * CVector2( 1.0f / size.x, 1.0f / size.y );
			m_pImage3 = new CImage2D( pDrawable, pDrawable1, footprintRect, footprintTexRect );
			m_footprint.SetDrawable( pFootprintDrawable );
			m_footprint.rect = footprintRect;
			m_footprint.texRect = footprintTexRect;
			m_footprint.pInstData = &m_param1;
			m_footprint.nInstDataSize = sizeof( m_param1 );
			m_pImage3->x = pItem->ofs.x;
			m_pImage3->y = pItem->ofs.y;
			m_pImage3->SetZOrder( -1 );
			m_pImage3->SetInstData( &m_param1, sizeof( m_param1 ) );
			m_pImage2->AddChild( m_pImage3 );

			pItem = pTextureAtlas->GetItem( "s_body_3" );
			footprintRect = pItem->frameRect;
			footprintTexRect = pItem->rect * CVector2( 1.0f / size.x, 1.0f / size.y );
			m_pImage4 = new CImage2D( pDrawable, pDrawable1, footprintRect, footprintTexRect );
			m_footprint1.SetDrawable( pFootprintDrawable );
			m_footprint1.rect = footprintRect;
			m_footprint1.texRect = footprintTexRect;
			m_footprint1.pInstData = &m_param2;
			m_footprint1.nInstDataSize = sizeof( m_param2 );
			m_pImage4->x = pItem->ofs.x;
			m_pImage4->y = pItem->ofs.y;
			m_pImage4->SetInstData( &m_param2, sizeof( m_param2 ) );
			m_pImage2->AddChild( m_pImage4 );
		}
		SHitProxyData* pHitProxyData = pHitProxyDataSet->GetData( m_nType );
		AddProxy( *pHitProxyData );
		if( m_nType < eType_Leg_End || m_nType == eType_Body )
		{
			m_HRFireOfs = pHitProxyData->GetCenter();
		}

		SetRenderObject( m_pImage1 );

		SetParentEntity( pPar );
	}

	void SetHead( CSpiderBossLimb* pHead ) { m_pHead = pHead; }

	void OnTickAfterHitTest()
	{
		if( m_nState >= 1 )
		{
			CSpiderBoss* pSpider = static_cast<CSpiderBoss*>( GetParentEntity() );
			if( m_pImage3 )
			{
				m_footprint.worldMat = m_pImage3->globalTransform;
				pSpider->GetFootprint()->AddFootprint( &m_footprint, 5 );
				m_footprint1.worldMat = m_pImage4->globalTransform;
				pSpider->GetFootprint()->AddFootprint( &m_footprint1, 5 );
			}
			else
			{
				m_footprint.worldMat = m_pImage2->globalTransform;
				pSpider->GetFootprint()->AddFootprint( &m_footprint, 5 );
			}
		}
	}

	void UpdateDisplay( float fElapsedTime )
	{
		if( m_nState == 1 )
		{
			float& fFadeTime = m_param.z;
			fFadeTime -= fElapsedTime;
			if( fFadeTime < 0 )
				fFadeTime = 0;

			if( m_pImage3 )
			{
				m_param1.z = fFadeTime;
				m_pImage3->r -= fElapsedTime * 10.0f;
				if( m_pImage3->r < -PI )
					m_pImage3->r += PI * 2;
			}
			if( m_pImage4 )
			{
				m_param2.z = fFadeTime;
				m_pImage4->r += fElapsedTime * 10.0f;
				if( m_pImage4->r > PI )
					m_pImage4->r -= PI * 2;
			}

			GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
		}
		else
		{
			CSpiderBoss* pSpider = static_cast<CSpiderBoss*>( GetParentEntity() );
			CPlayer* pPlayer = GetStage()->GetPlayer();
			float fElapsedTime1 = GetStage()->GetElapsedTimePerTick();
			bool bCanBeHit = pPlayer->IsInHorrorReflex() && pSpider->IsHitInHR();
			if( pSpider->GetCurState() >= CSpiderBoss::eState_Hanging && m_nType != eType_Tail )
				bCanBeHit = false;

			if( bCanBeHit )
			{
				float fColor = m_paramEmission.x;
				fColor += fElapsedTime1;
				if( fColor > 1 )
					fColor = 1;
				m_paramEmission = CVector4( fColor, fColor, fColor, fColor );
			}
			else
				m_paramEmission = CVector4( 0, 0, 0, 0 );
		}

		if( fElapsedTime > 0 )
		{
			if( m_pSilk && !m_bHangedOK )
			{
				float fPreLength = m_fHangLength;
				m_fHangLength += fElapsedTime * 512;
				SRaycastResult result;
				if( GetStage()->Raycast( m_pSilk->globalTransform.MulVector2Pos( CVector2( 0, -fPreLength ) )
					, m_pSilk->globalTransform.MulVector2Pos( CVector2( 0, -m_fHangLength ) ), eEntityHitType_WorldStatic, &result ) )
				{
					m_fHangLength = fPreLength + result.fDist;
					m_hangTargetPos = m_pSilk->globalTransform.MulVector2Pos( CVector2( 0, -m_fHangLength ) );
					m_bHangedOK = true;
				}
				m_pSilk->SetData( 1, CVector2( 0, -m_fHangLength ), 8, CVector2( 0, m_fHangLength / 512 ), CVector2( 1, m_fHangLength / 512 ) );
			}

			if( m_pSilk1 )
				m_pSilk1->UpdateAnim( fElapsedTime );
			if( m_pSilk2 )
				m_pSilk2->UpdateAnim( fElapsedTime );
		}
	}

	void UpdateHRTime( float fTime0, float fTime1 )
	{
		if( m_nState == 0 )
			return;
		if( m_nType < eType_Leg_End )
		{
			if( fTime0 < 1.5f && fTime1 >= 1.5f )
			{
				CVector2 globalPos = GetGlobalTransform().MulVector2Pos( m_HRFireOfs );
				CVector2 targetPos = GetStage()->GetPlayer()->GetCrosshair()->globalTransform.GetPosition();
				CVector2 dir = targetPos - globalPos;
				if( dir.Dot( dir ) < 1 )
					dir = CVector2( 0, 1 );
				dir.Normalize();
			
				CEnemyBullet* pEnemyBullet = new CEnemyBullet( this, dir * 400, CVector2( 0, 0 ), 8, 3, 10, 0, 0, 1 );
				pEnemyBullet->x = globalPos.x;
				pEnemyBullet->y = globalPos.y;
				pEnemyBullet->r = r;
				pEnemyBullet->SetParentBeforeEntity( GetParentEntity() );
			}
		}
		else if( m_nType == eType_Tail )
		{
			if( fTime0 < 2.5f && fTime1 >= 2.5f )
			{
				uint32 nType = SRand::Inst().Rand( 0, 3 );
				CVector2 dir1, dir2;
				switch( nType )
				{
				case 0:
					dir1 = CVector2( -1024, 0 );
					dir2 = CVector2( 1024, 0 );
					break;
				case 1:
					dir1 = CVector2( -1024, 1024 );
					dir2 = CVector2( 1024, 1024 );
					break;
				default:
					dir1 = CVector2( 0, 1024 );
					dir2 = CVector2( 0, 1024 );
					break;
				}

				CBloodLaser* pBloodLaser = new CBloodLaser( dir1, 64 );
				pBloodLaser->SetPosition( CVector2( -10, -45 ) );
				pBloodLaser->SetParentEntity( this );
				pBloodLaser = new CBloodLaser( dir2, 64 );
				pBloodLaser->SetPosition( CVector2( 10, -45 ) );
				pBloodLaser->SetParentEntity( this );
			}
		}
		else if( m_nType == eType_Body )
		{
			if( fTime0 < 2.0f && fTime1 >= 2.0f )
			{
				CVector2 globalPos = GetGlobalTransform().MulVector2Pos( m_HRFireOfs );

				for( int i = 0; i < 8; i++ )
				{
					float r = i * PI / 4 + GetParentEntity()->GetRotation();
					CEnemyBullet* pEnemyBullet = new CEnemyBullet( this, CVector2( cos( r ) * 400, sin( r ) * 400 ), CVector2( 0, 0 ), 8, 3, 10, 0, 0, 1 );
					pEnemyBullet->x = globalPos.x;
					pEnemyBullet->y = globalPos.y;
					pEnemyBullet->r = r;
					pEnemyBullet->SetParentBeforeEntity( GetParentEntity() );
				}
			}
		}
	}

	void CreateHangRope( bool bPhase1 )
	{
		static CRopeDrawable2D* pDrawable = NULL;
		static CAnimationSet* pAnimSet = NULL;
		if( !pDrawable )
		{
			vector<char> content;
			GetFileContent( content, "materials/hangrope.xml", true );
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			pDrawable = new CRopeDrawable2D;
			pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
			doc.Clear();

			GetFileContent( content, "anims/hangrope.xml", true );
			TiXmlDocument doc1;
			doc1.LoadFromBuffer( &content[0] );
			pAnimSet = new CAnimationSet;
			pAnimSet->LoadXml( doc1.RootElement() );
		}

		CRenderObject2D* pRenderObject = GetRenderObject();
		CRopeObject2D* pSilk;

		if( !bPhase1 )
		{
			pSilk = new CRopeObject2D( pDrawable, NULL );
			pSilk->SetZOrder( -1 );
			pSilk->x = m_hangRopeOfs.x;
			pSilk->y = -21 + m_hangRopeOfs.y;
			pRenderObject->AddChild( pSilk );
			pSilk->SetDataCount( 2 );
			pSilk->SetData( 0, CVector2( 0, 0 ), 8, CVector2( 0, 0 ), CVector2( 1, 0 ) );
			m_fHangLength = 1;
			pSilk->SetData( 1, CVector2( 0, -m_fHangLength ), 8, CVector2( 0, 1 ), CVector2( 1, 1 ) );

			m_pSilk = pSilk;
		}
		else
		{
			m_bHangedOK = false;
			if( pRenderObject == m_pImage2 )
			{
				pSilk = new CRopeObject2D( pDrawable, NULL );
				pSilk->SetZOrder( -1 );
				pSilk->x = m_hangRopeOfs.x;
				pSilk->y = m_hangRopeOfs.y;
				pSilk->SetAnim( pAnimSet, 0 );
				pSilk->GetAnimController()->PlayAnim( "anim", eAnimPlayMode_OnceNoRemove );
				pRenderObject->AddChild( pSilk );
				pSilk->SetDataCount( 6 );
				for( int i = 0; i <= 5; i++ )
				{
					pSilk->SetData( i, CVector2( 0, 0 ), 8, CVector2( 0, i * 0.1 ), CVector2( 1, i * 0.1 ), i + 1 );
				}
				m_pSilk1 = pSilk;

				pSilk = new CRopeObject2D( pDrawable, NULL );
				pSilk->SetZOrder( 1 );
				pSilk->x = m_hangRopeOfs.x;
				pSilk->y = m_hangRopeOfs.y;
				pSilk->SetAnim( pAnimSet, 0 );
				pSilk->GetAnimController()->PlayAnim( "anim", eAnimPlayMode_OnceNoRemove );
				pRenderObject->AddChild( pSilk );
				pSilk->SetDataCount( 6 );
				for( int i = 0; i <= 5; i++ )
				{
					pSilk->SetData( i, CVector2( 0, 0 ), 8, CVector2( 0, ( i + 5 ) * 0.1 ), CVector2( 1, ( i + 5 ) * 0.1 ), i == 5? 1: i + 6 );
				}
				m_pSilk2 = pSilk;

				m_pHangRopeHitArea = new CEntity;
				m_pHangRopeHitArea->SetPosition( m_hangRopeOfs + CVector2( 0, -21 ) );
				m_pHangRopeHitArea->AddRect( CRectangle( -16, -12, 32, 24 ) );
				m_pHangRopeHitArea->SetHitType( eEntityHitType_Sensor );
				m_pHangRopeHitArea->RegisterEntityEvent( eEntityEvent_PlayerAttack, &m_onHitHangRope );
				m_pHangRopeHitArea->SetParentEntity( this );
			}
		}

		GetParentEntity()->UpdateDirty();
	}

	void RemoveHangRope()
	{
		if( !m_pSilk && !m_pSilk1 && !m_pSilk2 )
			return;
		m_bHangedOK = false;
		CRenderObject2D* pObject = new CRenderObject2D;
		globalTransform.Decompose( pObject->x, pObject->y, pObject->r, pObject->s );
		GetParent()->GetParent()->AddChild( pObject );
		
		if( m_pSilk )
		{
			m_pSilk->RemoveThis();
			pObject->AddChild( m_pSilk );
			m_pSilk = NULL;
		}
		if( m_pSilk1 )
		{
			m_pSilk1->RemoveThis();
			pObject->AddChild( m_pSilk1 );
			m_pSilk1 = NULL;
		}
		if( m_pSilk2 )
		{
			m_pSilk2->RemoveThis();
			pObject->AddChild( m_pSilk2 );
			m_pSilk2 = NULL;
		}
		
		if( m_onHitHangRope.IsRegistered() )
			m_onHitHangRope.Unregister();
		if( m_pHangRopeHitArea )
		{
			m_pHangRopeHitArea->SetParentEntity( NULL );
			m_pHangRopeHitArea = NULL;
		}

		CSpiderBoss* pSpiderBoss = static_cast<CSpiderBoss*>( GetParentEntity() );
		pSpiderBoss->Stop();
	}
protected:
	virtual void OnAddedToStage() override
	{
		RegisterEntityEvent( eEntityEvent_PlayerAttack, &m_onPlayerAttack );
	}

	virtual void OnRemovedFromStage() override
	{
		if( m_onPlayerAttack.IsRegistered() )
			m_onPlayerAttack.Unregister();
		if( m_onHitHangRope.IsRegistered() )
			m_onHitHangRope.Unregister();
		if( m_tickAfterHitTest.IsRegistered() )
			m_tickAfterHitTest.Unregister();
	}

	void OnTickBeforeHitTest()
	{
		float fTime = GetStage()->GetGlobalElapsedTime();
		if( m_pSilk1 )
			m_pSilk1->UpdateAnim( fTime );
		if( m_pSilk2 )
			m_pSilk2->UpdateAnim( fTime );
	}
private:
	void OnPlayerAttack( SPlayerAttackContext* pContext )
	{
		CSpiderBoss* pSpider = static_cast<CSpiderBoss*>( GetParentEntity() );
		if( m_nState )
		{
			if( pContext->pTarget == this )
				return;
			
			pContext->nResult |= SPlayerAttackContext::eResult_Hit | SPlayerAttackContext::eResult_Critical;
			CVector2& pos = pContext->hitPos;
			CEffectObject* pObject = new CEffectObject( 1 );
			CParticleSystem* pParticleSystem = __getBloodEffect();
			pObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pObject->GetAnimController() ) );
			pObject->x = pos.x;
			pObject->y = pos.y;
			pObject->SetParentBeforeEntity( GetParentEntity() );
			RemoveHangRope();
			return;
		}

		if( pSpider->GetCurState() >= CSpiderBoss::eState_Hanging && m_nType != eType_Tail )
			return;

		if( pContext->pPlayer->IsInHorrorReflex() && pSpider->IsHitInHR() )
		{
			pContext->nResult |= SPlayerAttackContext::eResult_Hit;
			CVector2& pos = pContext->hitPos;
			CEffectObject* pObject = new CEffectObject( 1 );
			CParticleSystem* pParticleSystem = __getBloodEffect();
			pObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pObject->GetAnimController() ) );
			pObject->x = pos.x;
			pObject->y = pos.y;
			pObject->SetParentBeforeEntity( GetParentEntity() );

			uint32 nDmg = pContext->nDmg;
			m_nHpLeft -= nDmg;
			if( m_nHpLeft <= 0 )
			{
				pContext->nResult |= SPlayerAttackContext::eResult_Critical;
				m_nHpLeft = 0;
				m_nState = 1;
				m_param.z = 1;
				SetRenderObject( m_pImage2 );
				UpdateDisplay( 0 );
				GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
				pSpider->OnLimbDestroyed();
				RemoveHangRope();

				if( m_pHead )
				{
					m_pHead->m_nHpLeft = 0;
					m_pHead->m_nState = 1;
					m_pHead->m_param.z = 1;
					m_pHead->SetRenderObject( m_pHead->m_pImage2 );
					m_pHead->UpdateDisplay( 0 );
					GetStage()->RegisterAfterHitTest( 1, &m_pHead->m_tickAfterHitTest );
				}
			}
		}
	}

	uint8 m_nType;
	uint8 m_nState;
	int32 m_nHpLeft;
	CVector4 m_param, m_param1, m_param2;
	CVector4 m_paramEmission;
	CVector2 m_HRFireOfs;
	CVector2 m_hangRopeOfs;
	CVector2 m_hangTargetPos;
	float m_fHangLength;
	bool m_bHangedOK;
	
	CElement2D m_footprint;
	CElement2D m_footprint1;
	CReference<CImage2D> m_pImage1;
	CReference<CImage2D> m_pImage2;
	CReference<CImage2D> m_pImage3;
	CReference<CImage2D> m_pImage4;
	CReference<CRopeObject2D> m_pSilk;
	CReference<CRopeObject2D> m_pSilk1;
	CReference<CRopeObject2D> m_pSilk2;
	CReference<CEntity> m_pHangRopeHitArea;
	CReference<CSpiderBossLimb> m_pHead;

	TClassTrigger1<CSpiderBossLimb, SPlayerAttackContext*> m_onPlayerAttack;
	TClassTrigger1<CSpiderBossLimb, SPlayerAttackContext*> m_onHitHangRope;
	TClassTrigger<CSpiderBossLimb> m_tickAfterHitTest;
};

CSpiderBoss::CSpiderBoss()
	: m_nState( 0 )
	, m_nSkillCounter( 0 )
	, m_nDestroyedLimbs( 0 )
	, m_fTime( 0 )
	, m_fHRTime( 0 )
	, m_bHitInHR( false )
	, m_onHit( this, &CSpiderBoss::OnHit )
	, m_tickAfterHitTest( this, &CSpiderBoss::OnTickAfterHitTest )
{
	static CAnimationSet* pAnimSet = NULL;
	if( !pAnimSet )
	{
		vector<char> content;
		GetFileContent( content, "anims/spider.xml", true );
		TiXmlDocument doc1;
		doc1.LoadFromBuffer( &content[0] );
		pAnimSet = new CAnimationSet;
		pAnimSet->LoadXml( doc1.RootElement() );
	}
		
	SetHitType( eEntityHitType_Enemy );
	SetAnim( pAnimSet, 0 );

	for( int i = 0; i < CSpiderBossLimb::eType_Count; i++ )
	{
		m_pLimbs[i] = new CSpiderBossLimb( i, this );
	}
	m_pLimbs[CSpiderBossLimb::eType_Body]->SetHead( m_pLimbs[CSpiderBossLimb::eType_Head] );
}

void CSpiderBoss::OnTickBeforeHitTest()
{
	CStage* pStage = GetStage();
	float fTime = pStage->GetGlobalElapsedTime();
	if( m_nState == eState_Idle )
		Idle();
	else if( m_nState == eState_Move )
		Move();
	else if( m_nState == eState_Attack || m_nState == eState_Hit )
		Attack();
	else if( m_nState == eState_Hanging )
		HangUp();
	else if( m_nState == eState_Hanged )
		HangedMove();
	else if( m_nState == eState_HangedAttack || m_nState == eState_HangedHit )
		HangedAttack();
	else if( m_nState == eState_HangedThrow || m_nState == eState_HangedThrowHit )
		HangedThrow();
	CCharacter::OnTickBeforeHitTest();

	if( m_nState >= eState_Hanged )
		HangedFixPosition();
	for( auto pChild = Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		CSpiderBossLimb* pLimb = dynamic_cast<CSpiderBossLimb*>( pChild );
		if( pLimb )
			pLimb->UpdateDisplay( fTime );
	}
}

void CSpiderBoss::OnTickAfterHitTest()
{
	float fTime = GetStage()->GetElapsedTimePerTick();
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( m_nState == eState_Hanging )
		{
			SPlayerDizzyContext context;
			context.fPercent = 1.0f;
			context.pushForce = CVector2( 0, -128 );
			pPlayer->AddDizzyThisFrame( context );
		}
		else if( m_nState == eState_Hanged )
		{
			if( m_fPlayerDizzyTime > 0 )
			{
				SPlayerDizzyContext context;
				context.fPercent = Min( m_fPlayerDizzyTime * 0.5f, 1.0f );
				context.pushForce = CVector2( 0, -128 * context.fPercent );
				pPlayer->AddDizzyThisFrame( context );

				m_fPlayerDizzyTime -= fTime;
				if( m_fPlayerDizzyTime < 0 )
					m_fPlayerDizzyTime = 0;
			}
		}
	}

	if( pPlayer && pPlayer->IsInHorrorReflex() )
	{
		float fTime0 = m_fHRTime;
		m_fHRTime += fTime;
		for( int i = 0; i < ELEM_COUNT( m_pLimbs ); i++ )
		{
			m_pLimbs[i]->UpdateHRTime( fTime0, m_fHRTime );
		}
	}
	else
	{
		m_fHRTime = 0;
		m_bHitInHR = false;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSpiderBoss::OnAddedToStage()
{
	static CFootprintUpdateDrawable* pUpdateDrawable = NULL;
	static CFootprintDrawable* pColorDrawable = NULL;
	if( !pUpdateDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/footprint.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pUpdateDrawable = new CFootprintUpdateDrawable;
		pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
		pColorDrawable = new CFootprintDrawable;
		pColorDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render" ) );
		doc.Clear();
	}

	m_pFootprint = new CFootprintReceiver( pUpdateDrawable, pColorDrawable, NULL, false );
	m_pFootprint->EnableAutoSplit( 1024 );
	m_pFootprint->SetFootprintRectExtension( 10 );
	GetStage()->AddFootprint( m_pFootprint );
	m_pAnim = GetAnimController()->PlayAnim( "idle", eAnimPlayMode_Loop );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	CCharacter::OnAddedToStage();
}

void CSpiderBoss::OnRemovedFromStage()
{
	CCharacter::OnRemovedFromStage();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	m_pFootprint->SetAutoRemove( true );
	m_pFootprint = NULL;
	Stop();
}

void CSpiderBoss::BeginMove()
{
	m_nState = eState_Move;
	m_fTime = 0;
	m_pAnim = GetAnimController()->PlayAnim( "walk", eAnimPlayMode_Loop );
	m_pAnim->FadeIn( 0.25f );
}

void CSpiderBoss::BeginAttack()
{
	m_nState = eState_Attack;
	m_fTime = 0;
	m_pAnim = GetAnimController()->PlayAnim( "attack", eAnimPlayMode_Once );
	m_pAnim->RegisterEvent( 0, &m_onHit );
}

void CSpiderBoss::BeginHangUp()
{
	m_nState = eState_Hanging;
	m_fTime = 0;
	m_pAnim = GetAnimController()->PlayAnim( "walk", eAnimPlayMode_Loop );
	m_pAnim->FadeIn( 0.25f );
}

void CSpiderBoss::BeginHangedAttack()
{
	m_nState = eState_HangedAttack;
	m_fTime = 0;
	m_pAnim = GetAnimController()->PlayAnim( "attack", eAnimPlayMode_Once );
	m_pAnim->FadeIn( 0.1f );
	m_pAnim->SetTimeScale( 0.5f );
	m_pAnim->RegisterEvent( 0, &m_onHit );

	CSpiderBossLimb* pLimb = m_pLimbs[CSpiderBossLimb::eType_Tail];
	float fHangLength = pLimb->m_fHangLength;
	m_fHangLength1 = fHangLength;
	m_fHangLength2 = Max( 128.0f, fHangLength - 256 );
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
		m_fHangLength3 = pLimb->m_hangTargetPos.y - pPlayer->GetPosition().y - 128.0f;
	else
		m_fHangLength3 = 512;
}

void CSpiderBoss::BeginHangedThrow()
{
	m_nState = eState_HangedThrow;
	m_fTime = 0;
	m_pAnim = GetAnimController()->PlayAnim( "attack", eAnimPlayMode_Once );
	m_pAnim->FadeIn( 0.1f );
	m_pAnim->RegisterEvent( 0, &m_onHit );
}

void CSpiderBoss::Stop()
{
	m_nState = eState_Idle;
	m_fTime = 0.5f;
	if( m_onHit.IsRegistered() )
		m_onHit.Unregister();
	m_pAnim = GetAnimController()->PlayAnim( "idle", eAnimPlayMode_Loop );
	m_pAnim->FadeIn( 0.25f );
}

void CSpiderBoss::HangedStop()
{
	m_nState = eState_Hanged;
	m_fTime = 0;
	if( m_onHit.IsRegistered() )
		m_onHit.Unregister();
	m_pAnim = GetAnimController()->PlayAnim( "walk", eAnimPlayMode_Loop );
	m_pAnim->FadeIn( 0.25f );
}

void CSpiderBoss::Move()
{
	float fMoveSpeed = 400;
	float fTurnSpeed = 8.0f;

	CStage* pStage = GetStage();
	CPlayer* pPlayer = pStage->GetPlayer();
	float fTime = pStage->GetGlobalElapsedTime();
	m_fTime += fTime;
	if( m_fTime >= 1.0f )
	{
		Stop();
		return;
	}
	CVector2 dPosition;
	float dRotation;
	if( m_nMoveType == 0 )
	{
		dPosition = m_moveTarget - GetPosition();
	}
	else
	{
		dPosition = pPlayer->GetPosition() - GetPosition();
	}
	
	bool bMoved = CommonMove( fMoveSpeed, fTurnSpeed, fTime, dPosition, 100, dRotation );
	
	if( !bMoved )
	{
		if( m_nMoveType == 1 )
		{
			if( dPosition.Dot( dPosition ) < 1 || dRotation == 0 )
			{
				BeginAttack();
			}
		}
		else
		{
			bool bCastWeb = false;
			do
			{
				dPosition = pPlayer->GetPosition() - GetPosition();
				float l = dPosition.Length();
				float r0 = GetRotation() + PI * 0.5f;
				if( SRand::Inst().Rand( 0.0f, 1.0f ) < 0.4f * Min( 1.0f, Max( ( 200 - l ) / 200, 0.0f ) ) )
				{
					CSpiderWebGround* pWeb = new CSpiderWebGround( 0, r0 );
					pWeb->SetPosition( GetPosition() );
					pWeb->SetParentAfterEntity( this );
					bCastWeb = true;
					break;
				}

				float r1 = atan2( dPosition.y, dPosition.x );
				float dr = ( r1 - r0 ) / ( PI * 2 );
				dr = ( dr - floor( dr ) ) * 2 - 1;

				if( SRand::Inst().Rand( 0.0f, 1.0f ) < 0.3f * Min( 1.0f, Max( ( 0.15f - abs( dr ) ) / 0.05f, 0.0f ) ) * Min( 1.0f, Max( ( 280 - l ) / 50, 0.0f ) ) )
				{
					CSpiderWebGround* pWeb = new CSpiderWebGround( 1, r0 + PI );
					pWeb->SetPosition( GetPosition() );
					pWeb->SetParentAfterEntity( this );
					bCastWeb = true;
					break;
				}
				
				if( dr > 0 )
				{
					uint32 i = floor( ( dr - 0.1f ) * 5 );
					if( i >= 0 && i < 4 && SRand::Inst().Rand( 0.0f, 1.0f ) < 0.25f )
					{
						CSpiderWebGround* pWeb = new CSpiderWebGround( 2, r0 + PI + PI * ( i + 1 ) * 0.2f );
						pWeb->SetPosition( GetPosition() );
						pWeb->SetParentAfterEntity( this );
						bCastWeb = true;
						break;
					}
				}
				else
				{
					uint32 i = floor( ( -dr - 0.1f ) * 5 );
					if( i >= 0 && i < 4 && SRand::Inst().Rand( 0.0f, 1.0f ) < 0.25f )
					{
						CSpiderWebGround* pWeb = new CSpiderWebGround( 2, r0 - PI - PI * ( i + 1 ) * 0.2f );
						pWeb->SetPosition( GetPosition() );
						pWeb->SetParentAfterEntity( this );
						bCastWeb = true;
						break;
					}
				}

			} while( false );
			Stop();
			if( bCastWeb )
				m_fTime = 1.0f;
		}
	}
}

void CSpiderBoss::Idle()
{
	CStage* pStage = GetStage();
	float fTime = pStage->GetGlobalElapsedTime();
	m_fTime -= fTime;
	if( m_fTime <= 0 && pStage->GetPlayer() )
	{
		if( m_nSkillCounter == INVALID_32BITID )
		{
			BeginHangUp();
			return;
		}

		CVector2 dPosition = pStage->GetPlayer()->GetPosition() - GetPosition();
		float l = dPosition.Length();
		float fAttack = m_nSkillCounter / ( m_nSkillCounter + 3.0f );
		fAttack = fAttack * Max( 0.0f, Min( ( 600 - l ) / 300, 1.0f ) );
		if( SRand::Inst().Rand( 0.0f, 1.0f ) < fAttack )
		{
			m_nMoveType = 1;
			m_nSkillCounter = 0;
		}
		else
		{
			m_nMoveType = 0;
			if( l < 1 )
			{
				dPosition = CVector2( 0, 1 );
				l = 1;
			}
			dPosition = ( dPosition * ( ( l + 200 ) ) + CVector2( dPosition.y, -dPosition.x ) * SRand::Inst().Rand( -150.0f, 150.0f ) ) * ( 1.0f / l );

			m_moveTarget = GetPosition() + dPosition;
			m_nSkillCounter++;
		}

		BeginMove();
		return;
	}
}

void CSpiderBoss::Attack()
{
	if( !m_pAnim->GetController() )
	{
		Stop();
		m_fTime = 2.0f;

		float r = m_nDestroyedLimbs / ( m_nDestroyedLimbs + 2.0f );
		if( SRand::Inst().Rand( 0.0f, 1.0f ) < r )
		{
			m_nDestroyedLimbs = 0;
			m_nSkillCounter = INVALID_32BITID;
		}
		return;
	}
}

void CSpiderBoss::HangedAttack()
{
	CStage* pStage = GetStage();
	if( !m_pAnim->GetController() )
	{
		HangedStop();
		m_fTime = SRand::Inst().Rand( 3.0f, 5.0f );
		return;
	}

	float fTime = pStage->GetGlobalElapsedTime();
	m_fTime += fTime;

	float& fHangLength = m_pLimbs[CSpiderBossLimb::eType_Tail]->m_fHangLength;
	if( m_fTime < 0.4f )
	{
		fHangLength = m_fHangLength1 + ( m_fHangLength2 - m_fHangLength1 ) * m_fTime * 2.5f;
	}
	else if( m_fTime < 0.6667f )
	{
		fHangLength = m_fHangLength2 + ( m_fHangLength3 - m_fHangLength2 ) * ( m_fTime - 0.4f ) / 0.2667f;
	}
	else
	{
		fHangLength = m_fHangLength3 - ( m_fTime - 0.6667f ) * 384;
	}
}

void CSpiderBoss::HangedThrow()
{
	if( !m_pAnim->GetController() )
	{
		HangedStop();
		m_fTime = SRand::Inst().Rand( 2.0f, 4.0f );
		return;
	}
}

void CSpiderBoss::OnHit()
{
	uint8 nState = m_nState;
	m_nState++;
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		if( nState == eState_HangedThrow )
		{
			const CMatrix2D& mat = GetAnimController()->GetTransform( CSpiderBossLimb::eType_Head );
			CVector2 pos = mat.GetPosition();
			CSpiderBossEgg* pEgg = new CSpiderBossEgg( this, pPlayer->GetPosition(), ( pos - pPlayer->GetPosition() ).Length() / 768.0f );
			pEgg->SetPosition( pos );
			pEgg->SetParentBeforeEntity( this );
			return;
		}

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
			const CMatrix2D& mat = GetAnimController()->GetTransform( CSpiderBossLimb::eType_Head );
			CVector2 pos = mat.GetPosition();
			if( nState == eState_HangedAttack )
			{
				CSpiderBossBullet* pBullet = new CSpiderBossBullet( this, 0 );
				pBullet->SetPosition( pos );
				pBullet->r = PI * 1.5f;
				pBullet->SetParentBeforeEntity( this );
			}
			else
			{
				for( int i = 0; i < 24; i++ )
				{
					float r = ( i + SRand::Inst().Rand( -0.3f, 0.3f ) ) * PI / 12;
					float fSpeed = SRand::Inst().Rand( 200, 300 );
					CEnemyBullet* pEnemyBullet = new CEnemyBullet( this, CVector2( fSpeed * cos( r + 0.5f * PI ), fSpeed * sin( r + 0.5f * PI ) ), CVector2( 0, 0 ), 8, 3, 10, 0, 0, 1 );
					pEnemyBullet->x = pos.x;
					pEnemyBullet->y = pos.y;
					pEnemyBullet->r = r;
					pEnemyBullet->SetParentBeforeEntity( this );
				}
			}
			m_bHitInHR = true;
		}
	}
}

void CSpiderBoss::HangUp()
{
	float fTurnSpeed = 4.0f;
	CStage* pStage = GetStage();
	float fTime = pStage->GetGlobalElapsedTime();
	float dRotation;
	CommonTurn( fTurnSpeed, fTime, PI, dRotation );

	if( dRotation == 0 )
	{
		if( m_fTime == 0 )
		{
			m_pAnim = GetAnimController()->PlayAnim( "idle", eAnimPlayMode_Loop );
			m_pAnim->FadeIn( 0.25f );
			m_pLimbs[CSpiderBossLimb::eType_Tail]->CreateHangRope( true );
		}
		else if( m_fTime > 0.5f )
		{
			CSpiderBossLimb* pLimb = m_pLimbs[CSpiderBossLimb::eType_Tail];
			if( !pLimb->m_pSilk )
				pLimb->CreateHangRope( false );
			if( pLimb->m_bHangedOK )
			{
				m_nSkillCounter = 0;
				HangedStop();
				m_fTime = 3.0f;
				m_fPlayerDizzyTime = 1.0f;
			}
			return;
		}
		m_fTime += fTime;
	}
}

void CSpiderBoss::HangedMove()
{
	float fTime = GetStage()->GetGlobalElapsedTime();

	CSpiderBossLimb* pLimb = m_pLimbs[CSpiderBossLimb::eType_Tail];
	CPlayer* pPlayer = GetStage()->GetPlayer();
	if( pPlayer )
	{
		CVector2& hangPos = pLimb->m_hangTargetPos;
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

		float& fHangLength = pLimb->m_fHangLength;
		float y = hangPos.y - ( pPlayer->GetPosition().y + 512 );
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

		m_fTime -= fTime;
		if( m_fTime <= 0 )
		{
			m_fTime = 0;
			if( abs( hangPos.x - x ) < 256 )
			{
				float r = m_nSkillCounter;
				r *= Min( 1.0f, Max( 0.0f, 1 - ( y - fHangLength ) / 256 ) );
				r *= Min( 1.0f, Max( 0.0f, ( -128 - pPlayer->GetPosition().y ) / 128 ) );
				r = r / ( r + 3.0f );

				if( SRand::Inst().Rand( 0.0f, 1.0f ) < r )
				{
					BeginHangedAttack();
					m_nSkillCounter = 0;
				}
				else
				{
					BeginHangedThrow();
					m_nSkillCounter++;
				}
			}
		}
	}
}

void CSpiderBoss::HangedFixPosition()
{
	UpdateDirty();
	
	CSpiderBossLimb* pLimb = m_pLimbs[CSpiderBossLimb::eType_Tail];
	CVector2& hangPos = pLimb->m_hangTargetPos;

	CRopeObject2D* pSilk = pLimb->m_pSilk;
	CVector2 hangPos1 = pSilk->globalTransform.MulVector2Pos( CVector2( 0, -pLimb->m_fHangLength ) );
	SetPosition( GetPosition() + hangPos - hangPos1 );
}