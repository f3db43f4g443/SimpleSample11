#include "stdafx.h"
#include "TutorialGame.h"
#include "Stage.h"
#include "Player.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"
#include "Render/Scene2DManager.h"
#include "GUI/HUDCircle.h"

void CTutorialGameElement::Update( CTutorialGame* pGame, CPlayer* pPlayer, float fTime )
{
	CReference<CTutorialGameElement> pTemp = this;
	UpdateAnim( fTime );
	OnUpdate( pGame, pPlayer, fTime );
	if( !GetParentEntity() )
		return;
	UpdateObj( pGame );
}

void CTutorialGameElement::UpdateObj( CTutorialGame* pGame )
{
	m_pGame = pGame;
	/*float fScale = pGame->GetCamDist() / ( m_pos.z + pGame->GetCamDist() );
	CVector2 dCenter = CVector2( m_pos.x, m_pos.y ) - pGame->GetCamCenter();
	dCenter = dCenter * fScale + pGame->GetCamCenter();
	CRenderObject2D* pObj = GetRenderObject();
	pObj->x = dCenter.x - m_pos.x;
	pObj->y = dCenter.y - m_pos.y;
	pObj->s = fScale;
	pObj->SetTransformDirty();*/
}

CTutorialGame::CTutorialGame( const SClassCreateContext& context )
	: CEntity( context )
	, m_dynamicTexture( 400, 300, EFormat::EFormatR8G8B8A8UNorm )
	, m_onStartGame( this, &CTutorialGame::StartGame )
	, m_onPlayerUpdated( this, &CTutorialGame::OnPlayerUpdated )
	, m_fCamZNear( 0.5f )
	, m_fCamDist( 2.5f )
	, m_camCenter( 0, 0 )
	, m_strCrosshair( context )
{
	m_pElementRoot = new CEntity;
	m_dynamicTexture.SetRoot( m_pElementRoot );
	m_dynamicTexture.SetClear( true );
	m_dynamicTexture.GetCamera().SetSize( 800, 600 );
	m_pGUIRoot = new CEntity;
	m_pGUIRoot->SetZOrder( 1 );
	m_pGUIRoot->SetParentEntity( m_pElementRoot );
}

void CTutorialGame::OnAddedToStage()
{
	CImage2D* pImage2D = dynamic_cast<CImage2D*>( GetRenderObject() );
	dynamic_cast<CDefaultDrawable2D*>( pImage2D->GetGUIDrawable() )->BindShaderResource( EShaderType::PixelShader, "Texture0", &m_dynamicTexture );;

	GetStage()->RegisterStageEvent( eStageEvent_Start, &m_onStartGame );
	GetStage()->RegisterStageEvent( eStageEvent_PlayerUpdated, &m_onPlayerUpdated );

	CScene2DManager::GetGlobalInst()->GetRoot()->AddChild( m_pElementRoot );
	GetStage()->AddEntity( m_pElementRoot );

	if( !m_pCrosshair )
	{
		m_pCrosshair = dynamic_cast<CEntity*>( CResourceManager::Inst()->CreateResource<CPrefab>( m_strCrosshair.c_str() )->GetRoot()->CreateInstance() );
		m_pCrosshair->SetParentEntity( m_pGUIRoot );
	}

	if( !CurGame() )
		CurGame() = this;
}

void CTutorialGame::OnRemovedFromStage()
{
	GetStage()->RemoveEntity( m_pElementRoot );
	m_pElementRoot->RemoveThis();

	if( m_onStartGame.IsRegistered() )
		m_onStartGame.Unregister();
	if( m_onPlayerUpdated.IsRegistered() )
		m_onPlayerUpdated.Unregister();
	
	if( this == CurGame() )
		CurGame() = NULL;
}

void CTutorialGame::OnPlayerUpdated()
{
	float fElapsedTime = GetStage()->GetGlobalElapsedTime();
	CPlayer* pPlayer = GetStage()->GetPlayer();
	m_camCenter = pPlayer->GetPosition();
	SetPosition( pPlayer->GetPosition() );
	m_pGUIRoot->SetPosition( m_camCenter );
	m_pCrosshair->SetPosition( pPlayer->GetCrosshair()->GetPosition() );
	m_pCrosshair->SetZOrder( 1 );

	LINK_LIST_FOR_EACH_BEGIN( pChild, m_pElementRoot->Get_ChildEntity(), CEntity, ChildEntity )
		CTutorialGameElement* pElem = dynamic_cast<CTutorialGameElement*>( pChild );
		if( pElem )
			pElem->Update( this, pPlayer, fElapsedTime );
	LINK_LIST_FOR_EACH_END( pChild, m_pElementRoot->Get_ChildEntity(), CEntity, ChildEntity )

	m_dynamicTexture.GetCamera().SetPosition( m_camCenter.x, m_camCenter.y );
}

void CTutorialGameBullet::OnHitPlayer( CPlayer* pPlayer )
{
	CTutorialGame::GetCurGame()->Damage( m_nDmgHp, m_nDmgMp, m_nDmgSp );
	SetParentEntity( NULL );
}

CTutorialGameHpBar::CTutorialGameHpBar( const SClassCreateContext& context )
	: CEntity( context )
	, m_strElem( context )
	, m_strElem1( context )
	, m_strEffect( context )
	, m_nCurHp( 0 )
	, m_nMaxHp( 0 )
{
	
}
	
void CTutorialGameHpBar::OnAddedToStage()
{
	m_pElem = CResourceManager::Inst()->CreateResource<CPrefab>( m_strElem.c_str() );
	m_pElem1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strElem1.c_str() );
	m_pEffect = CResourceManager::Inst()->CreateResource<CPrefab>( m_strEffect.c_str() );
	uint32 nCur = m_nCurHp;
	uint32 nMax = m_nMaxHp;
	m_nCurHp = nCur;
	m_nMaxHp = nMax;
	SetHp( m_nCurHp, m_nMaxHp );
}

void CTutorialGameHpBar::OnRemovedFromStage()
{
	SetHp( 0, 0 );
}

void CTutorialGameHpBar::SetHp( uint32 nCur, uint32 nMax )
{
	nCur = Min( nCur, nMax );
	if( !m_pElem )
	{
		m_nCurHp = nCur;
		m_nMaxHp = nMax;
		return;
	}
	uint32 nElemCount = nMax * 2;
	uint32 nPreElemCount = m_vecImages.size();
	for( int i = nElemCount; i < nPreElemCount; i++ )
	{
		m_vecImages[i]->RemoveThis();
		m_vecImages[i] = NULL;
	}
	m_vecImages.resize( nElemCount );
	for( int i = nPreElemCount; i < nElemCount; i++ )
	{
		uint32 nHp = i / 2;
		uint32 nType = i % 2;
		m_vecImages[i] = ( nType == 1 ? m_pElem1 : m_pElem )->GetRoot()->CreateInstance();
		m_vecImages[i]->SetPosition( CVector2( 0, m_fHeightPerHp * nHp ) );
		if( nType == 1 )
			m_vecImages[i]->bVisible = false;
		AddChild( m_vecImages[i] );
	}
	m_nMaxHp = nMax;
	uint32 nPreCur = m_nCurHp;
	m_nCurHp = Min( m_nCurHp, m_nMaxHp );

	for( int i = m_nCurHp; i < nCur; i++ )
		m_vecImages[i * 2 + 1]->bVisible = true;
	for( int i = nCur; i < m_nCurHp; i++ )
		m_vecImages[i * 2 + 1]->bVisible = false;
	m_nCurHp = nCur;

	if( m_nCurHp < nPreCur )
	{
		CEntity* pEntity = dynamic_cast<CEntity*>( m_pEffect->GetRoot()->CreateInstance() );
		pEntity->SetPosition( m_vecImages[m_nCurHp * 2]->GetPosition() );
		pEntity->SetParentEntity( this );
	}
}

CTutorialGameFall::CTutorialGameFall( const SClassCreateContext& context )
	: CTutorialGame( context )
	, m_strBackground( context )
	, m_strChest( context )
	, m_strCoin( context )
	, m_strParticle0( context )
{
}

void CTutorialGameFall::OnAddedToStage()
{
	CTutorialGame::OnAddedToStage();
	pBackgroundPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBackground.c_str() );
	pCoinPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strCoin.c_str() );
	pChestPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strChest.c_str() );
	pParticle0 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strParticle0.c_str() );
}

void CTutorialGameFall::OnRemovedFromStage()
{
	m_pLastAddedElement = NULL;
	CTutorialGame::OnRemovedFromStage();
}

void CTutorialGameFall::AddGameElement( CTutorialGameElement* pElement )
{
	if( m_pLastAddedElement )
		pElement->SetParentAfterEntity( m_pLastAddedElement );
	else
		pElement->SetParentEntity( m_pElementRoot );
	m_pLastAddedElement = pElement;
	pElement->UpdateObj( this );
}

void CTutorialGameFall::StartGame()
{
	m_fGenTime = 2.0f;
	m_fBackgroundGenTime = 1.0f;
}

void CTutorialGameFall::OnPlayerUpdated()
{
	CTutorialGame::OnPlayerUpdated();
	float fElapsedTime = GetStage()->GetGlobalElapsedTime();
	CPlayer* pPlayer = GetStage()->GetPlayer();

	m_fBackgroundGenTime -= fElapsedTime;
	if( m_fBackgroundGenTime <= 0 )
	{
		CTutorialGameFlyingObject* pElement = dynamic_cast<CTutorialGameFlyingObject*>( pBackgroundPrefab->GetRoot()->CreateInstance() );
		auto texRect = static_cast<CImage2D*>( pElement->GetRenderObject() )->GetElem().texRect;
		texRect.x = SRand::Inst().Rand( 0.0f, 1 - texRect.width );
		texRect.y = SRand::Inst().Rand( 0.0f, 1 - texRect.height );
		static_cast<CImage2D*>( pElement->GetRenderObject() )->SetTexRect( texRect );
		pElement->SetPos( CVector3( 0, 0, 50.0f + m_fBackgroundGenTime * pElement->GetSpeed() ) );
		AddGameElement( pElement );
		m_fBackgroundGenTime += 1.0f;
	}

	m_fGenTime -= fElapsedTime;
	if( m_fGenTime <= 0 )
	{
		CTutorialGameFlyingObject* pElement;
		if( SRand::Inst().Rand( 0, 2 ) )
			pElement = dynamic_cast<CTutorialGameFlyingObject*>( pCoinPrefab->GetRoot()->CreateInstance() );
		else
			pElement = dynamic_cast<CTutorialGameFlyingObject*>( pChestPrefab->GetRoot()->CreateInstance() );

		pElement->SetPos( CVector3( SRand::Inst().Rand( -250.0f, 250.0f ), SRand::Inst().Rand( -250.0f, 250.0f ), 50.0f + m_fGenTime * pElement->GetSpeed() ) );
		AddGameElement( pElement );

		m_fGenTime += SRand::Inst().Rand( 0.5f, 2.0f );
	}
	
	m_dynamicTexture.GetRoot()->SortChildren( [] ( CRenderObject2D* a, CRenderObject2D* b ) {
		auto p1 = static_cast<CTutorialGameElement*>( a );
		auto p2 = static_cast<CTutorialGameElement*>( b );
		if( p1->GetPos().z < p2->GetPos().z )
			return true;
		if( p1->GetPos().z > p2->GetPos().z )
			return false;
		return p1->GetLevel() > p2->GetLevel();
	} );
}

void CTutorialGameFlyingObject::OnUpdate( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime )
{
	if( m_fLife > 0 )
	{
		m_fLife -= fTime;
		if( m_fLife <= 0 )
		{
			SetParentEntity( NULL );
			return;
		}
	}

	CVector3 pos = GetPos();
	pos.z -= m_fFlyingSpeed * fTime;
	if( pos.z < pGame->GetCamZNear() - pGame->GetCamDist() )
	{
		SetParentEntity( NULL );
		return;
	}
	SetPos( pos );
}

void CTutorialGameHitObject::OnUpdate( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime )
{
	CVector3 pos = GetPos();
	CTutorialGameFlyingObject::OnUpdate( pGame, pPlayer, fTime );
	if( !GetParentEntity() )
		return;
	CVector3 pos1 = GetPos();
	CTutorialGameFall* pTutorialGame = static_cast<CTutorialGameFall*>( pGame );
	if( pos.z > 0 && pos1.z <= 0 )
	{
		if( HitTest( pPlayer ) )
		{
			auto pEffect = dynamic_cast<CTutorialGameFlyingObject*>( pTutorialGame->pParticle0->GetRoot()->CreateInstance() );
			pEffect->SetPos( CVector3( pos1.x, pos1.y, 0 ) );
			pGame->AddGameElement( pEffect );
			SetParentEntity( NULL );
			return;
		}
	}
}

void CTutorialGamePlayerActionAttack::OnEnterHR( CPlayer* pPlayer )
{
	m_pHitArea = new CHUDCircle( m_fHitAreaRadius, 24.0f, CVector4( 1, 0, 0, 0 ), CVector4( 1, 0, 0, 1 ), true );
	m_pHitArea->AddCircle( m_fHitAreaRadius, CVector2( 0, 0 ) );
	m_pHitArea->SetHitType( eEntityHitType_Sensor );
	m_pHitArea->SetBulletMode( true );
	m_pHitArea->SetParentEntity( m_pGame->GetCrosshair() );

	m_pInnerArea = new CHUDCircle( m_fMoveInnerRadius, 16.0f, CVector4( 0, 0, 1, 1 ), CVector4( 0.5, 0.5, 1, 1 ) );
	m_pInnerArea->SetZOrder( -1 );
	m_pInnerArea->SetParentEntity( m_pGame->GetGUIRoot() );

	m_pOuterArea = new CHUDCircle( m_fMoveInnerRadius + m_fMoveOuterRadius, 16.0f, CVector4( 0, 0, 1, 1 ), CVector4( 0.5, 0.5, 1, 1 ) );
	m_pOuterArea->SetZOrder( -1 );
	m_pOuterArea->SetParentEntity( m_pGame->GetGUIRoot() );
	m_fCDLeft = m_fCD;
}
