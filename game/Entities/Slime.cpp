#include "stdafx.h"
#include "Slime.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Stage.h"
#include "Common/Rand.h"
#include "SlimeCore.h"
#include "Render/Animation.h"

CSlimeTrap::CSlimeTrap( float fSizeSpeed, float fMoveSpeed, float fMaxMoveDist, const CRectangle& rect, const CRectangle& texRect )
	: m_fSizeSpeed( fSizeSpeed ), m_fMoveSpeed( fMoveSpeed ), m_fMaxMoveDist( fMaxMoveDist )
	, m_tickAfterHitTest( this, &CSlimeTrap::OnTickAfterHitTest )
	, m_bAlive( true )
	, m_fAlpha( 0 )
{
	static CDefaultDrawable2D* pDrawable = NULL;
	static CDefaultDrawable2D* pDrawable1 = NULL;
	if( !pDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/scar.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
		pDrawable1 = new CDefaultDrawable2D;
		pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
	}

	SetHitType( eEntityHitType_Sensor );

	CImage2D* pImage = new CImage2D( pDrawable, pDrawable1, rect, texRect );
	SetRenderObject( pImage );
	CVector4 data[2] = { CVector4( 0, 0, 0, 0 ), CVector4( 0, 0, 0, 0 ) };
	pImage->SetParam( 2, data, 0, 1, 1, 1, 0, 0 );
}

void CSlimeTrap::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSlimeTrap::OnRemovedFromStage()
{
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
}

void CSlimeTrap::OnTickAfterHitTest()
{
	float fTime = GetStage()->GetGlobalElapsedTime();
	auto pParams = static_cast<CImage2D*>( GetRenderObject() )->GetParam();
	if( m_bAlive )
	{
		m_fAlpha = Min( 1.0f, m_fAlpha + fTime );
		pParams[0] = CVector4( m_fAlpha, m_fAlpha, m_fAlpha, m_fAlpha );
		pParams[1] = CVector4( 0, 0, 0, m_fAlpha * 0.04f );
		if( m_fAlpha >= 1 )
			return;
	}
	else
	{
		m_fAlpha = Max( 0.0f, m_fAlpha - fTime );
		pParams[0] = CVector4( m_fAlpha, m_fAlpha, m_fAlpha, m_fAlpha );
		pParams[1] = CVector4( 0, 0, 0, m_fAlpha * 0.04f );
		if( m_fAlpha <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
	}
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSlimeTrap::Kill()
{
	if( !m_bAlive )
		return;
	m_bAlive = false;
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

CSlime::CSlime( class CSlimeGround* pSlimeGround, const CVector2& velocity, float fSize )
	: m_velocity( velocity )
	, m_pSlimeGround( pSlimeGround )
	, m_nBoundState( 0 )
	, m_tickAfterHitTest( this, &CSlime::OnTickAfterHitTest )
	, m_tickChangeVelocity( this, &CSlime::OnChangeVelocity )
	, m_pSlimeCore( NULL )
	, m_fSpawnTime( 0.5f )
	, m_fSize( fSize )
	, m_param( 2, 2, 2, 1 )
	, m_targetParam( 2, 2, 2, 1 )
	, m_fParamChangeTimeLeft( 0 )
	, m_bBlink( false )
	, m_blinkColor( 0, 0, 0, 0 )
{
	static CDefaultDrawable2D* pDrawable = NULL;
	if( !pDrawable )
	{
		vector<char> content;
		GetFileContent( content, "materials/slime.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "footprint" ) );
	}
	
	m_pDynamicTextureRenderObject = new CImage2D( pDrawable, NULL, CRectangle( 0, 0, 0, 0 ), CRectangle( 0, 0, 1, 1 ) );
	m_pDynamicTextureRenderObject->SetInstData( &m_param, sizeof( m_param ) );

	AddCircle( fSize, CVector2( 0, 0 ) );
	SetBulletMode( true );
	SetHitType( eEntityHitType_Enemy );
}

void CSlime::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
	GetStage()->RegisterAfterHitTest( SRand::Inst().Rand( 60, 120 ), &m_tickChangeVelocity );
	m_pSlimeGround->AddUnboundSlime( this );
	m_pSlimeGround->m_dynamicTexture.GetRoot()->AddChild( m_pDynamicTextureRenderObject );
}

void CSlime::OnRemovedFromStage()
{
	CCharacter::OnRemovedFromStage();
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	if( m_tickChangeVelocity.IsRegistered() )
		m_tickChangeVelocity.Unregister();
	if( !m_pSlimeCore && !m_pTrapped )
		m_pSlimeGround->RemoveUnboundSlime( this );
	if( m_pDynamicTextureRenderObject )
	{
		m_pDynamicTextureRenderObject->RemoveThis();
		m_pDynamicTextureRenderObject = NULL;
	}
}

void CSlime::OnUnbound( bool bExplode )
{
	m_pSlimeGround->AddUnboundSlime( this );
	m_pSlimeCore = NULL;
	m_nSlotIndex = -1;
	m_nBoundState = 0;
	SetParentEntity( m_pSlimeGround );
	SetTransformIndex( -1 );
	globalTransform.Decompose( x, y, r, s );
	ChangeColor( CVector4( 2, 2, 2, 1 ) );
	
	ChangeVelocity( bExplode );
}

void CSlime::OnTickBeforeHitTest()
{
	if( x > 800 || y > 800 || x < -800 || y < -800 )
	{
		SetPosition( CVector2( 0, 0 ) );
	}

	float fTime = GetStage()->GetGlobalElapsedTime();
	if( m_fParamChangeTimeLeft > 0 )
	{
		float fPreTime = m_fParamChangeTimeLeft;
		m_fParamChangeTimeLeft = Max( m_fParamChangeTimeLeft - fTime, 0.0f );
		m_param = m_targetParam + ( m_param - m_targetParam ) * ( m_fParamChangeTimeLeft / fPreTime );
	}
	if( m_bBlink && m_fParamChangeTimeLeft <= 0 )
	{
		m_fParamChangeTimeLeft = SRand::Inst().Rand( 0.25f, 1.0f );
		float fAlpha = SRand::Inst().Rand( 0.5f, 1.0f ) * m_blinkColor.w;
		float fHue = SRand::Inst().Rand( 0.0f, 3.0f );
		CVector3 hue(
			Max( Min( Max( 1 - fHue, fHue - 2 ) * 2, 1.0f ), 0.0f ),
			Max( Min( Min( 2 - fHue, fHue ) * 2, 1.0f ), 0.0f ),
			Max( Min( Min( 3 - fHue, fHue - 1 ) * 2, 1.0f ), 0.0f ) );

		m_targetParam = CVector4( m_blinkColor.x * hue.x, m_blinkColor.y * hue.y, m_blinkColor.z * hue.z, fAlpha );
	}

	if( m_pTrapped )
	{
		CVector2 dPos = GetPosition() - m_trappedMoveTarget;
		float l = dPos.Length();
		if( l > 0 )
		{
			float l1 = Max( l - m_pTrapped->m_fMoveSpeed * fTime, 0.0f );
			SetPosition( m_trappedMoveTarget + dPos * ( l1 / l ) );
		}

		float fSize = m_pDynamicTextureRenderObject->GetElem().rect.width * 0.25f;
		float dSize = m_pTrapped->m_fSizeSpeed * m_fSize * fTime;
		fSize = Max( fSize - dSize, 0.0f );
		if( fSize <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
		
		m_pDynamicTextureRenderObject->SetRect( CRectangle( -fSize * 2, -fSize * 2, fSize * 4, fSize * 4 ) );
	}
	else
	{
		if( m_fSpawnTime > 0 )
		{
			m_fSpawnTime = Max( m_fSpawnTime - fTime, 0.0f );
			float fSize = ( 1 - m_fSpawnTime * m_fSpawnTime * 4 ) * m_fSize;
			m_pDynamicTextureRenderObject->SetRect( CRectangle( -fSize * 2, -fSize * 2, fSize * 4, fSize * 4 ) );
		}
	
		if( m_nBoundState == 0 )
			SetPosition( GetPosition() + m_velocity * fTime );
		else if( m_nBoundState == 1 )
		{
			CVector2 target = m_pSlimeCore->GetAnimController()->GetTransform( m_nSlotIndex ).GetPosition();
			CVector2 dPos = target - GetPosition();
			float l = dPos.Length();
			float dl = GetStage()->GetGlobalElapsedTime() * m_fSpeed;
			m_fSpeed += GetStage()->GetGlobalElapsedTime() * 64;
			if( dl >= l )
			{
				SetPosition( target );
				m_nBoundState = 2;
				m_pSlimeCore->OnSlimeFullyBound( this );
			}
			else
				SetPosition( GetPosition() + dPos * ( dl / l ) );
		}
	}

	CCharacter::OnTickBeforeHitTest();
}

void CSlime::OnTransformUpdated()
{
	if( m_pDynamicTextureRenderObject )
	{
		m_pDynamicTextureRenderObject->globalTransform = globalTransform;
		m_pDynamicTextureRenderObject->ResetTransformDirty();
	}
}

void CSlime::OnTickAfterHitTest()
{
	for( auto pManifold = Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( pManifold->pOtherHitProxy );
		if( pEntity )
		{
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			{
				if( m_velocity.Dot( pManifold->normal ) > 0 )
				{
					CVector2 norm = pManifold->normal;
					float l = norm.Normalize();
					if( l < 0.001f )
						continue;
					m_velocity = m_velocity - norm * ( 2 * m_velocity.Dot( norm ) );
				}
			}
			else if( !m_pTrapped && m_fSpawnTime == 0 && !m_pSlimeCore && m_velocity.Dot( m_velocity ) < 256 * 256 )
			{
				m_pTrapped = dynamic_cast<CSlimeTrap*>( pEntity );
				if( m_pTrapped )
				{
					m_pSlimeGround->RemoveUnboundSlime( this );
					if( m_velocity.Dot( m_velocity ) > 0 )
					{
						CVector2 norm = pManifold->normal;
						float l = norm.Normalize();
						if( l < 0.001f )
						{
							norm = m_pTrapped->GetPosition() - GetPosition();
							l = Min( norm.Normalize(), m_pTrapped->m_fMaxMoveDist );
							m_trappedMoveTarget = GetPosition() + norm * l;
						}
						else
						{
							m_trappedMoveTarget = GetPosition() + norm * m_pTrapped->m_fMaxMoveDist;
						}
					}
					else
						m_trappedMoveTarget = GetPosition();
				}
			}
		}
	}
	
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSlime::ChangeVelocity( bool bExplode )
{
	GetStage()->RegisterAfterHitTest( SRand::Inst().Rand( 30, 60 ), &m_tickChangeVelocity );
}

CSlimeForceField::CSlimeForceField( CSlimeGround* pSlimeGround, float fLife, float fMaxDist, float fStrength )
	: m_fLife( fLife )
	, m_fMaxDist( fMaxDist )
	, m_fStrength( fStrength )
	, m_pSlimeGround( pSlimeGround )
	, m_tickAfterHitTest( this, &CSlimeForceField::OnTickAfterHitTest )
{
}

void CSlimeForceField::OnAddedToStage()
{
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSlimeForceField::OnRemovedFromStage()
{
	if( m_tickAfterHitTest.IsRegistered() )
		m_tickAfterHitTest.Unregister();
	m_pSlimeGround = NULL;
}

void CSlimeForceField::OnTickAfterHitTest()
{
	float fTime = GetStage()->GetGlobalElapsedTime();
	for( auto pChild = m_pSlimeGround->Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		CSlime* pSlime = dynamic_cast<CSlime*>( pChild );
		if( pSlime && !pSlime->m_pTrapped )
		{
			CVector2 dPos = globalTransform.GetPosition() - pSlime->GetPosition();
			float l = dPos.Normalize();
			if( l > 0.001f && l <= m_fMaxDist )
			{
				pSlime->m_velocity = pSlime->m_velocity + dPos * m_fStrength * fTime;
			}
		}
	}

	m_fLife -= fTime;
	if( m_fLife < 0 )
	{
		SetParentEntity( NULL );
		return;
	}
	
	GetStage()->RegisterAfterHitTest( 1, &m_tickAfterHitTest );
}

void CSlimeGround::OnAddedToStage()
{
	vector<char> content;
	GetFileContent( content, "materials/slime_ground.xml", true );
	TiXmlDocument doc;
	doc.LoadFromBuffer( &content[0] );
	m_pUpdateDrawable = new CDefaultDrawable2D;
	m_pUpdateDrawable->LoadXml( doc.RootElement()->FirstChildElement( "update" ) );
	m_pColorDrawable = new CDefaultDrawable2D;
	m_pColorDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render" ) );
	m_pColorDrawable->BindShaderResource( EShaderType::PixelShader, "Texture0", &m_dynamicTexture );
	m_pOcclusionDrawable = new CDefaultDrawable2D;
	m_pOcclusionDrawable->LoadXml( doc.RootElement()->FirstChildElement( "render_occlusion" ) );
	m_pOcclusionDrawable->BindShaderResource( EShaderType::PixelShader, "Texture0", &m_dynamicTexture );
	doc.Clear();

	CImage2D* pImage2D = new CImage2D( m_pColorDrawable, m_pOcclusionDrawable, CRectangle( -512, -512, 1024, 1024 ), CRectangle( 0, 0, 1, 1 ) );
	SetRenderObject( pImage2D );
	
	CRenderObject2D* pRoot = m_dynamicTexture.GetRoot();
	pImage2D = new CImage2D( m_pUpdateDrawable, NULL, CRectangle( -512, -512, 1024, 1024 ), CRectangle( 0, 0, 1, 1 ) );
	pRoot->AddChild( pImage2D );
}

void CSlimeGround::OnRemovedFromStage()
{
	SetRenderObject( NULL );
	m_dynamicTexture.GetRoot()->RemoveAllChild();
	if( m_pUpdateDrawable )
	{
		delete m_pUpdateDrawable;
		m_pUpdateDrawable = NULL;
	}
	if( m_pColorDrawable )
	{
		delete m_pColorDrawable;
		m_pColorDrawable = NULL;
	}
	if( m_pOcclusionDrawable )
	{
		delete m_pOcclusionDrawable;
		m_pOcclusionDrawable = NULL;
	}
}