#include "stdafx.h"
#include "SlimeBlister.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Render/Scene2DManager.h"
#include "Common/Rand.h"
#include "Stage.h"
#include "Player.h"
#include "Effects/DynamicTextures.h"
#include "Barrage.h"
#include "SlimeGenerator1.h"

class CSlimeBlisterDrawable : public CDefaultDrawable2D
{
public:
	void LoadXml( TiXmlElement* pRoot )
	{
		CDefaultDrawable2D::LoadXml( pRoot );
		auto pMaterial = pRoot->FirstChildElement( "material" );
		IShader* pPS = m_material.GetShader( EShaderType::PixelShader );
		if( pPS )
		{
			pPS->GetShaderInfo().Bind( m_paramTex, "Texture0" );
			pPS->GetShaderInfo().Bind( m_param, "fParam" );
		}
	}
protected:
	virtual bool OnFlushElement( CRenderContext2D& context, CElement2D* pElement, bool bBreak ) override
	{
		CSlimeBlister* pObj = (CSlimeBlister*)pElement->pInstData;
		m_paramTex.Set( context.pRenderSystem, pObj->m_canvas.GetTexture()->GetShaderResource() );
		m_param.Set( context.pRenderSystem, &pObj->m_param );
		return false;
	}
private:
	CShaderParamShaderResource m_paramTex;
	CShaderParam m_param;
};

CSlimeBlister::CSlimeBlister()
	: m_canvas( false, 256, 256, EFormat::EFormatR8G8B8A8UNorm, CCanvas::eDepthStencilType_None )
	, m_bBurst( false )
	, m_bCanBeHit( false )
	, m_fBleedCD( 0 )
	, m_nSlimeCount( 256 )
	, m_param( 0, 0, 0, 0 )
	, m_nState( 0 )
	, m_fStateTime( 2 )
	, m_tickAfterHitTest( this, &CSlimeBlister::OnTickAfterHitTest )
	, m_onPlayerAttack( this, &CSlimeBlister::OnPlayerAttack )
{
	static CSlimeBlisterDrawable *pDrawable = NULL;
	static CSlimeBlisterDrawable *pDrawable1 = NULL;
	static CDefaultDrawable2D *pDrawable2 = NULL;
	static CDefaultDrawable2D* pSlimeDrawable = NULL;
	static CParticleSystem* pParticleSystem = NULL;
	static CParticleSystem* pParticleSystem1 = NULL;
	if( !pDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/slime_blister.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDrawable = new CSlimeBlisterDrawable;
		pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render" ) );
		pDrawable1 = new CSlimeBlisterDrawable;
		pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "render_occlusion" ) );
		pDrawable2 = new CDefaultDrawable2D;
		pDrawable2->LoadXml( doc.RootElement()->FirstChildElement( "render1" ) );
		pSlimeDrawable = new CDefaultDrawable2D;
		pSlimeDrawable->LoadXml( doc.RootElement()->FirstChildElement( "slime" ) );
		pParticleSystem = new CParticleSystem();
		pParticleSystem->LoadXml( doc.RootElement()->FirstChildElement( "particle" ) );
		pParticleSystem->BindShaderResource( EShaderType::PixelShader, "Texture0", &CBloodSplashCanvas::Inst() );
		pParticleSystem1 = new CParticleSystem();
		pParticleSystem1->LoadXml( doc.RootElement()->FirstChildElement( "particle1" ) );
		pParticleSystem1->BindShaderResource( EShaderType::PixelShader, "Texture0", &CBloodSplashCanvas::Inst() );
		doc.Clear();
	}

	CImage2D* pImage2D = new CImage2D( pDrawable, pDrawable1, CRectangle( -128, -128, 256, 256 ), CRectangle( 0, 0, 1, 1 ) );
	SetRenderObject( pImage2D );
	pImage2D->SetInstData( this, 0 );
	AddCircle( 100, CVector2( 0, 0 ) );
	SetHitType( eEntityHitType_Enemy );

	m_canvas.SetRoot( new CRenderObject2D );
	m_vecSlimes.resize( 256 );
	for( int i = 0; i < 256; i++ )
	{
		CImage2D* pImage2D = new CImage2D( pSlimeDrawable, NULL, CRectangle( -32, -32, 64, 64 ), CRectangle( 0, 0, 1, 1 ) );
		m_vecSlimes[i].pRenderObject = pImage2D;
		float r = SRand::Inst().Rand( 0.0f, 120.0f );
		float angle = SRand::Inst().Rand( -PI, PI );
		pImage2D->x = r * cos( angle );
		pImage2D->y = r * sin( angle );
		r = SRand::Inst().Rand( 32.0f, 64.0f );
		angle = SRand::Inst().Rand( -PI, PI );
		m_vecSlimes[i].velocity = CVector2( r * cos( angle ), r * sin( angle ) );
		m_canvas.GetRoot()->AddChild( pImage2D );
	}
	SetZOrder( -1 );
	m_pParticleSystem = pParticleSystem;
	m_pParticleSystem1 = pParticleSystem1;

	CImage2D* pImage = new CImage2D( pDrawable2, NULL, CRectangle( -128, -128, 256, 256 ), CRectangle( 0, 0, 1, 1 ) );
	pImage->SetParam( 1, &CVector4( 0, 0, 0, 0 ), 0, 1, 0, 0, 0, 0 );
	AddChild( pImage );
	m_pHitImage = pImage;
}

void CSlimeBlister::OnAddedToStage()
{
	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( m_canvas.GetRoot() );
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	RegisterEntityEvent( eEntityEvent_PlayerAttack, &m_onPlayerAttack );
	CCharacter::OnAddedToStage();
}

void CSlimeBlister::OnRemovedFromStage()
{
	CCharacter::OnRemovedFromStage();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	if( m_onPlayerAttack.IsRegistered() )
		m_onPlayerAttack.Unregister();
	m_canvas.GetRoot()->RemoveThis();
	if( m_pEffect )
	{
		m_pEffect->SetTime( 4 );
		m_pParticleInst->GetData().isEmitting = false;
		m_pEffect = NULL;
		m_pParticleInst = NULL;
	}
	if( m_pSound )
	{
		m_pSound->FadeOut( 4 );
		m_pSound = NULL;
	}
}

void CSlimeBlister::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
	float fTime = GetStage()->GetGlobalElapsedTime();
	if( fTime == 0 )
		return;

	if( m_nState == 2 )
	{
		SetPosition( GetPosition() + m_velocity * fTime );
	}

	CSlimeGround* pSlimeGround = dynamic_cast<CSlimeGround*>( GetParentEntity() );
	if( m_bBurst )
	{
		if( m_nSlimeCount )
		{
			m_fBleedCD += fTime;
			while( m_fBleedCD >= 0.03f )
			{
				m_fBleedCD -= 0.03f;
				m_nSlimeCount--;
				if( pSlimeGround )
				{
					float fAngle = SRand::Inst().Rand( -PI, PI );
					float fRad = SRand::Inst().Rand( 256, 768 );
					CSlime* pSlime = new CSlime1( pSlimeGround, CVector2( fRad * cos( fAngle ), fRad * sin( fAngle ) ), 16 );
					pSlime->SetPosition( GetPosition() );
					pSlime->SetParentEntity( pSlimeGround );
				}
			}
		}
		else if( m_pEffect )
		{
			m_pEffect->SetTime( 4 );
			m_pParticleInst->GetData().isEmitting = false;
			m_pEffect = NULL;
			m_pParticleInst = NULL;
			m_pSound->FadeOut( 4 );
			m_pSound = NULL;

			CSlimeCoreGenerator1* pGenerator = new CSlimeCoreGenerator1( pSlimeGround );
			pGenerator->SetParentBeforeEntity( pSlimeGround );
		}
	}

	for( int i = 0; i < m_vecSlimes.size(); i++ )
	{
		if( !m_vecSlimes[i].pRenderObject )
			continue;
		if( i < m_nSlimeCount )
		{
			CVector2 pos = m_vecSlimes[i].pRenderObject->GetPosition() + m_vecSlimes[i].velocity * fTime;
			m_vecSlimes[i].pRenderObject->SetPosition( pos );
			if( pos.Dot( pos ) > 120 * 120 )
			{
				CVector2 v = ( pos * -1 + CVector2( pos.y, -pos.x ) * SRand::Inst().Rand( -0.5f, 0.5f ) )
					* SRand::Inst().Rand( 0.25f, 0.5f );
				m_vecSlimes[i].velocity = v;
			}
		}
		else
		{
			CVector2 pos = m_vecSlimes[i].pRenderObject->GetPosition();
			float l = pos.Length();
			if( l > 0 )
			{
				float l1 = Max( 0.0f, l - 256 * fTime );
				pos = pos * ( l1 / l );
				m_vecSlimes[i].pRenderObject->SetPosition( pos );
			}
			else
			{
				CImage2D* pImage = static_cast<CImage2D*>( m_vecSlimes[i].pRenderObject );
				float fSize = -pImage->GetElem().rect.x;
				fSize = Max( 0.0f, fSize - 64 * fTime );
				if( fSize > 0 )
				{
					pImage->SetRect( CRectangle( -fSize, -fSize, fSize * 2, fSize * 2 ) );
				}
				else
				{
					m_vecSlimes[i].pRenderObject = NULL;
					pImage->RemoveThis();
				}
			}
		}
	}

	if( m_bCanBeHit )
	{
		CPlayer* pPlayer = GetStage()->GetPlayer();
		if( pPlayer && !pPlayer->IsInHorrorReflex() )
			m_bCanBeHit = false;
	}
}

void CSlimeBlister::SetMoveTarget()
{
	m_nMoveCount++;
	CVector2 playerPos = GetStage()->GetPlayer()->GetPosition();
	float l = ( playerPos - GetPosition() ).Length();

	if( l < 250 )
	{
		if( SRand::Inst().Rand( 0.0f, 1.0f ) <= m_nMoveCount / ( m_nMoveCount + 6.0f ) )
		{
			m_fStateTime = 0;
			m_nState = 3;
			return;
		}
	}

	CVector2 randomPos = CVector2( SRand::Inst().Rand( -350.0f, 350.0f ), SRand::Inst().Rand( -350.0f, 350.0f ) );
	float fWeight = Min( 1.0f, Max( 0.0f, ( l - 100 ) / 400 ) );
	CVector2 targetPos = playerPos * ( 1 - fWeight ) + randomPos * fWeight;
	targetPos.x = Min( 350.0f, Max( -350.0f, targetPos.x ) );
	targetPos.y = Min( 350.0f, Max( -350.0f, targetPos.y ) );
	CVector2 dPos = targetPos - GetPosition();
	l = dPos.Normalize();
	if( l < 10 )
		m_velocity = CVector2( x > 0 ? -50 : 50, y > 0 ? -50 : 50 );
	else
	{
		float l1 = Min( l, SRand::Inst().Rand( 50.0f, 100.0f ) );
		m_velocity = dPos * l1;
	}
	
	m_fStateTime = 0;
	m_nState = 2;
	static const char* szSound[] = { "b1.wav", "b2.wav", "b3.wav" };
	CSoundFile::PlaySound( szSound[SRand::Inst().Rand( 0u, ELEM_COUNT( szSound ) )] );
}

void CSlimeBlister::OnTickAfterHitTest()
{
	float fTime = GetStage()->GetGlobalElapsedTime();
	CPlayer* pPlayer = GetStage()->GetPlayer();

	CVector4& param = *m_pHitImage->GetParam();
	if( m_bCanBeHit )
	{
		float fColor = param.x;
		float fElapsedTime1 = GetStage()->GetElapsedTimePerTick();
		fColor += fElapsedTime1;
		if( fColor > 1 )
			fColor = 1;
		param = CVector4( fColor, fColor, fColor, fColor );
	}
	else
		param = CVector4( 0, 0, 0, 0 );

	if( m_bBurst )
	{
		if( !m_nSlimeCount )
		{
			m_fStateTime = Max( m_fStateTime - fTime, 0.0f );
			if( m_fStateTime <= 0 )
			{
				SetParentEntity( NULL );
				return;
			}
		}
		float fAlpha = m_fStateTime / 4.0f;
		m_param = CVector4( fAlpha, 1, 0, 1 );
		GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
		return;
	}

	if( m_nState == 0 )
	{
		m_fStateTime += fTime;
		m_param.x = Max( 0.0f, 1.0f - m_fStateTime * 0.5f );
		if( m_fStateTime >= 5.0f )
		{
			SetPosition( CVector2( SRand::Inst().Rand( -300.0f, 300.0f ), SRand::Inst().Rand( -300.0f, 300.0f ) ) );
			m_fStateTime = 0;
			m_nMoveCount = 0;
			m_nState = 1;
		}
	}
	else if( m_nState == 1 )
	{
		m_fStateTime += fTime;
		m_param.x = Min( m_fStateTime * 0.5f, 1.0f );
		if( m_param.x >= 1.0f )
		{
			SetMoveTarget();
		}
	}
	else if( m_nState == 2 )
	{
		m_fStateTime += fTime;
		float dRotation;

		CVector2 dir = m_velocity;
		dir.Normalize();
		m_param.y = 1 / ( 2 - abs( m_fStateTime * 2 - 1 ) );
		m_param.z = -dir.x;
		m_param.w = dir.y;
		if( m_fStateTime >= 1 )
		{
			m_param.y = 1;
			SetMoveTarget();
		}
	}
	else
	{
		float fPreTime = m_fStateTime;
		m_fStateTime += fTime;
		CVector2 dPos = pPlayer->GetPosition() - GetPosition();
		if( fPreTime <= 2 && m_fStateTime > 2 )
		{
			if( pPlayer->IsInHorrorReflex() )
			{
				m_bCanBeHit = true;
				SBarrageContext context;
				context.vecObjects.push_back( this );
				context.vecObjects.push_back( pPlayer->GetCrosshair() );
				
				CBarrage_Old* pBarrage = new CBarrage_Old( context );
				for( int iAngle = 0; iAngle < 1; iAngle++ )
				{
					pBarrage->RegisterBulletWithTarget( 1, 480, 0 );
				}
				for( int iAngle = 0; iAngle < 4; iAngle++ )
				{
					float k = iAngle - 1.5f;
					k = k * k * 0.1f + 1;
					pBarrage->RegisterBulletWithTarget( 1, 420 * k, ( iAngle - 1.5f ) * PI / 24 );
				}
				for( int iAngle = 0; iAngle < 7; iAngle++ )
				{
					float k = iAngle - 3;
					k = k * k * 0.1f + 1;
					pBarrage->RegisterBulletWithTarget( 1, 360 * k, ( iAngle - 3 ) * PI / 24 );
				}
				for( int iAngle = 0; iAngle < 10; iAngle++ )
				{
					float k = iAngle - 4.5f;
					k = k * k * 0.1f + 1;
					pBarrage->RegisterBulletWithTarget( 1, 300 * k, ( iAngle - 4.5f ) * PI / 24 );
				}
				pBarrage->SetParentBeforeEntity( GetParentEntity() );
				pBarrage->SetPosition( GetPosition() );
				pBarrage->Start();
			}
			else
			{
				SDamage dmg;
				dmg.pSource = this;
				dmg.nHp = 10;
				pPlayer->Damage( dmg );
				CEffectObject* pObject = new CEffectObject( 2, &CBloodSplashCanvas::Inst() );
				CParticleSystem* pParticleSystem = m_pParticleSystem;
				pObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pObject->GetAnimController() ) );
				pObject->SetPosition( GetPosition() );
				pObject->r = atan2( dPos.y, dPos.x );
				pObject->SetParentBeforeEntity( GetParentEntity()->GetParentEntity() );
			}
			
			CSoundFile::PlaySound( "c.wav" );
		}
		if( m_fStateTime >= 2.5f && !pPlayer->IsInHorrorReflex() )
		{
			m_fStateTime = 0;
			m_nState = 0;
		}
		
		float l = dPos.Normalize();
		if( l < 0.001f )
			dPos = CVector2( 0, 1 );
		m_param.z = dPos.x;
		m_param.w = -dPos.y;
		if( m_fStateTime < 1.5f )
			m_param.y = 1.0f / ( m_fStateTime + 1 );
		else if( m_fStateTime < 2.0f )
			m_param.y = 1.0f / ( 8.5f - 4 * m_fStateTime );
		else if( m_fStateTime < 2.5f )
			m_param.y = 1.0f / ( m_fStateTime - 1.5f );
		else
			m_param.y = 1.0f;
	}
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSlimeBlister::Render( CRenderContext2D& context )
{
	if( context.eRenderPass == eRenderPass_Color )
	{
		m_canvas.ReleaseTexture();
		m_canvas.Render( context );
	}
}

void CSlimeBlister::OnPlayerAttack( SPlayerAttackContext* pContext )
{
	if( !m_bCanBeHit )
		return;
	pContext->nResult |= SPlayerAttackContext::eResult_Hit | SPlayerAttackContext::eResult_Critical;
	CVector2& pos = pContext->hitPos;
	CEffectObject* pObject = new CEffectObject( 0, &CBloodSplashCanvas::Inst() );
	CParticleSystem* pParticleSystem = m_pParticleSystem1;
	pObject->SetRenderObject( pParticleSystem->CreateParticleSystemObject( pObject->GetAnimController(), m_pParticleInst.AssignPtr() ) );
	pObject->x = pos.x;
	pObject->y = pos.y;
	pObject->SetParentAfterEntity( this );
	m_pEffect = pObject;
	m_bBurst = true;
	m_fStateTime = 4.0f;
	m_pSound = CSoundFile::PlaySound( "a.wav", true );
}