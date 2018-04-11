#include "stdafx.h"
#include "MainUI.h"
#include "MyGame.h"
#include "Render/Scene2DManager.h"
#include "HpBar.h"
#include "Common/Rand.h"
#include "Player.h"
#include "TileMap2D.h"
#include "MyLevel.h"
#include "GlobalCfg.h"
#include "Common/MathUtil.h"

CMainUI* CMainUI::s_pLevel;

CMainUI::CMainUI( const SClassCreateContext& context )
	: CEntity( context )
	, m_tick( this, &CMainUI::Tick )
{
}

void CMainUI::OnAddedToStage()
{
	CVector2 screenRes = IRenderSystem::Inst()->GetScreenRes();
	m_pC->SetPosition( CVector2( 0, 0 ) );
	m_pB->SetPosition( CVector2( 0, -screenRes.y / 2 ) );
	m_pRB->SetPosition( CVector2( screenRes.x / 2, -screenRes.y / 2 ) );
	m_pRT->SetPosition( CVector2( screenRes.x / 2, screenRes.y / 2 ) );
	//m_pMinimap->bVisible = false;
	m_pSkip->bVisible = false;
	m_shakeBarOrigPos = m_pShake->GetPosition();
	m_shakeBarOrigHeight = static_cast<CImage2D*>( m_pShake.GetPtr() )->GetElem().rect.height;
	m_shakeBarOrigTexRect = static_cast<CImage2D*>( m_pShake.GetPtr() )->GetElem().texRect;
	m_spBarOrigPos = m_pSpBarRoot->GetPosition();
	m_spBarOfs = CVector2( 0, 0 );
	m_pUse->bVisible = false;

	auto pMinimap = static_cast<CTileMap2D*>( m_pMinimap.GetPtr() );
	m_blockTypes.resize( pMinimap->GetWidth() * pMinimap->GetHeight() * 2 );
	memset( &m_blockTypes[0], -1, m_blockTypes.size() );

	auto pShakeSmallBar = static_cast<CImage2D*>( m_pShakeSmallBars[0].GetPtr() );
	for( int i = 1; i < ELEM_COUNT( m_pShakeSmallBars ); i++ )
	{
		m_pShakeSmallBars[i] = new CImage2D( pShakeSmallBar->GetGUIDrawable(), NULL, pShakeSmallBar->GetElem().rect.Offset( CVector2( i * 4, 0 ) ), pShakeSmallBar->GetElem().texRect, true );
		pShakeSmallBar->GetParent()->AddChildAfter( m_pShakeSmallBars[i], m_pShakeSmallBars[i - 1] );
	}

	OnEndBonusStage();
	GetStage()->RegisterBeforeHitTest( 1, &m_tick );

	CPlayer* pPlayer = GetStage()->GetWorld()->GetPlayer();
	if( pPlayer )
	{
		OnModifyHp( pPlayer->GetHp(), pPlayer->GetMaxHp() );
		OnModifyHpStore( pPlayer->GetHpStore(), pPlayer->GetMaxHp() );
		OnModifySp( pPlayer->GetSp(), pPlayer->GetMaxHp() );
		OnModifyMoney( pPlayer->GetMoney() );
	}
	s_pLevel = this;
}

void CMainUI::OnRemovedFromStage()
{
	if( m_tick.IsRegistered() )
		m_tick.Unregister();
	m_pUseTarget = NULL;
	s_pLevel = NULL;
}

void CMainUI::OnModifyHp( float fHp, float fMaxHp )
{
	int32 nMaxHp = fMaxHp;
	int32 nHalfMaxHp[2] = { nMaxHp - nMaxHp / 2, nMaxHp / 2 };
	int32 nHp = fHp;
	int32 nHalfHp[2] = { nHp - nHp / 2, nHp / 2 };

	for( int i = 0; i < 2; i++ )
	{
		auto pHp = static_cast<CImage2D*>( m_pHpBar[i].GetPtr() );
		int32 nCeilHp = Pow2Ceil( nHalfHp[i] );
		auto rect = pHp->GetElem().rect;
		rect.width = nHalfHp[i] * 8;
		pHp->SetRect( rect );
		auto texRect = pHp->GetElem().texRect;
		texRect.width = nCeilHp ? nHalfHp[i] * 1.0f / nCeilHp : 0;
		pHp->SetTexRect( texRect );
		pHp->GetParam()[0] = CVector4( nCeilHp, 1, 0, 0 );

		auto pHpBack = static_cast<CImage2D*>( m_pHpBarBack[i].GetPtr() );
		int32 nCeilMaxHp = Pow2Ceil( nHalfMaxHp[i] );
		rect = pHpBack->GetElem().rect;
		rect.width = nHalfMaxHp[i] * 8;
		pHpBack->SetRect( rect );
		texRect = pHpBack->GetElem().texRect;
		texRect.width = nCeilMaxHp ? nHalfMaxHp[i] * 1.0f / nCeilMaxHp : 0;
		pHpBack->SetTexRect( texRect );
		pHpBack->GetParam()[0] = CVector4( nCeilMaxHp, 1, 0, 0 );
	}

	m_pHpBarRoot->SetPosition( CVector2( -nHalfMaxHp[0] * 4 - 10, m_pHpBarRoot->y ) );
}

void CMainUI::OnModifySp( float fSp, float fMaxSp )
{
	int32 nMaxSp = fMaxSp / 20;
	int32 nSp = fSp / 20;

	for( int i = 0; i < 2; i++ )
	{
		auto pSpBack = static_cast<CImage2D*>( m_pSpBarBack[i].GetPtr() );
		auto rect = pSpBack->GetElem().rect;
		rect.width = nMaxSp * 2;
		pSpBack->SetRect( rect );
		pSpBack->GetParam()[0] = CVector4( nMaxSp / 10, 1, 0, 0 );
	}
	auto pSp = static_cast<CImage2D*>( m_pSpBar.GetPtr() );
	auto rect = pSp->GetElem().rect;
	rect.width = nSp * 2;
	pSp->SetRect( rect );
	pSp->GetParam()[0] = CVector4( nSp / 10.0f, 1, 0, 0 );

	m_spBarOrigPos.x = -nMaxSp - 18;
	m_pSpBarRoot->SetPosition( m_spBarOrigPos + m_spBarOfs );
}

void CMainUI::OnModifyHpStore( float fHpStore, float fMaxHp )
{
	int32 nHp = fHpStore;
	int32 nHalfHp[2] = { nHp - nHp / 2, nHp / 2 };

	for( int i = 0; i < 2; i++ )
	{
		auto pHp = static_cast<CImage2D*>( m_pHpStoreBar[i].GetPtr() );
		int32 nCeilHp = Pow2Ceil( nHalfHp[i] );
		auto rect = pHp->GetElem().rect;
		rect.width = nHalfHp[i] * 8;
		pHp->SetRect( rect );
		auto texRect = pHp->GetElem().texRect;
		texRect.width = nCeilHp ? nHalfHp[i] * 1.0f / nCeilHp : 0;
		pHp->SetTexRect( texRect );
		pHp->GetParam()[0] = CVector4( nCeilHp, 1, 0, 0 );
	}
}

void CMainUI::OnModifyMoney( uint32 nMoney )
{
	char szText[32];
	sprintf( szText, "%dG", nMoney );
	m_pMoney->Set( szText );
	m_pMoney->SetPosition( CVector2( -m_pMoney->GetTextRect().width / 2, m_pMoney->y ) );
}

void CMainUI::UpdateMinimap( uint32 x, uint32 y, uint32 z, int8 nType )
{
	auto pMinimap = static_cast<CTileMap2D*>( m_pMinimap.GetPtr() );
	if( x >= pMinimap->GetWidth() || y >= pMinimap->GetHeight() )
		return;
	m_blockTypes[z + ( x + y * pMinimap->GetWidth() ) * 2] = nType;

	int8 nType1 = m_blockTypes[1 + ( x + y * pMinimap->GetWidth() ) * 2];
	if( nType1 < 0 )
		nType1 = m_blockTypes[0 + ( x + y * pMinimap->GetWidth() ) * 2];

	if( nType1 >= 0 )
	{
		uint16 nTypes = nType1;
		pMinimap->SetTile( x, y, 1, &nTypes );
	}
	else
	{
		pMinimap->SetTile( x, y, 0, NULL );
	}
}

void CMainUI::UpdateShakeSmallBar( uint32 x, uint32 nHeight )
{
	auto pBar = static_cast<CImage2D*>( m_pShakeSmallBars[x].GetPtr() );
	if( !nHeight )
		pBar->bVisible = false;
	else
	{
		pBar->bVisible = true;
		auto rect = pBar->GetElem().rect;
		rect.height = nHeight * 4;
		pBar->SetRect( rect );
	}
}

void CMainUI::ClearMinimap()
{
	auto pMinimap = static_cast<CTileMap2D*>( m_pMinimap.GetPtr() );
	for( int i = 0; i < pMinimap->GetWidth(); i++ )
	{
		for( int j = 0; j < pMinimap->GetHeight(); j++ )
		{
			pMinimap->SetTile( i, j, 0, NULL );
		}
	}

	for( int i = 0; i < ELEM_COUNT( m_pShakeSmallBars ); i++ )
	{
		m_pShakeSmallBars[i]->bVisible = false;
	}
}

void CMainUI::ShowUseText( CRenderObject2D * pTarget, const CVector2 & ofs, const char * szText )
{
	if( m_pUseTarget == pTarget )
		return;
	m_pUseTarget = pTarget;
	if( pTarget )
	{
		m_pUse->bVisible = true;
		m_pUseText->Set( szText );
		m_useOfs = ofs;
	}
	else
		m_pUse->bVisible = false;
}

void CMainUI::ShowMinimap()
{
	m_pMinimap->bVisible = true;
}

void CMainUI::HideMinimap()
{
	m_pMinimap->bVisible = false;
}

void CMainUI::OnBeginBonusStage()
{
	m_pHpBarRoot->bVisible = false;
	m_pHpBar[0]->bVisible = m_pHpBar[1]->bVisible = false;
	m_pHpBarBack[0]->bVisible = m_pHpBarBack[1]->bVisible = false;
	m_pHpStoreBar[0]->bVisible = m_pHpStoreBar[1]->bVisible = false;
	m_pMoney->bVisible = false;
	m_pComboBar[0]->bVisible = m_pComboBar[1]->bVisible = m_pComboBar[2]->bVisible = true;
	m_pPoint->bVisible = true;

	OnModifyCombo( 0 );
	m_pPoint->Set( "0P" );
	m_pPoint->SetPosition( CVector2( -m_pPoint->GetTextRect().width / 2, m_pPoint->y ) );
	OnBonusStageText( "BONUS STAGE" );
}

void CMainUI::OnEndBonusStage()
{
	m_pHpBarRoot->bVisible = true;
	m_pHpBar[0]->bVisible = m_pHpBar[1]->bVisible = true;
	m_pHpBarBack[0]->bVisible = m_pHpBarBack[1]->bVisible = true;
	m_pHpStoreBar[0]->bVisible = m_pHpStoreBar[1]->bVisible = true;
	m_pMoney->bVisible = true;
	m_pComboBar[0]->bVisible = m_pComboBar[1]->bVisible = m_pComboBar[2]->bVisible = false;
	m_pPoint->bVisible = false;
}

void CMainUI::OnAddPoint( uint32 nPoint, uint32 nCurPoint, uint32 nCombo )
{
	char szText[32];
	int32 nLevel;
	float fPercent;
	CGlobalCfg::Inst().GetComboLevel( nCombo, nLevel, fPercent );
	auto pText = SafeCast<CSimpleText>( m_pFloatText->GetRoot()->CreateInstance() );
	sprintf( szText, "%d", nPoint );
	pText->SetParentEntity( this );
	pText->Set( szText );
	pText->SetParam( CVector4( 0.7f, 0.3f, 1.0f, 1.0f ) );
	pText->FadeAnim( CVector2( 0, 128 ), 1.0f, true );
	pText->SetPosition( CVector2( m_pComboBar[0]->x + 200 * fPercent, m_pComboBar[0]->y ) );

	sprintf( szText, "%dP", nCurPoint );
	m_pPoint->Set( szText );
	m_pPoint->SetPosition( CVector2( -m_pPoint->GetTextRect().width / 2, m_pPoint->y ) );
}

void CMainUI::OnModifyCombo( uint32 nCombo )
{
	int32 nLevel;
	float fPercent;
	CGlobalCfg::Inst().GetComboLevel( nCombo, nLevel, fPercent );
	float h1 = ceil( 8 * fPercent );
	CRectangle r[3];
	r[0] = CRectangle( 0, nLevel * 16 - 8 + h1, 200, 16 - h1 );
	r[1] = CRectangle( 0, r[0].GetBottom(), floor( 200 * fPercent + 0.5f ), h1 );
	r[2] = CRectangle( 200, nLevel * 16 - 16, 56, 16 );
	CRectangle rect[3], texRect[3];
	for( int i = 0; i < 3; i++ )
	{
		rect[i] = CRectangle( r[i].x - 128, 0, r[i].width, r[i].height );
		texRect[i] = r[i] * ( 1.0f / 256.0f );
	}
	rect[0].y += h1;
	for( int i = 0; i < 3; i++ )
	{
		if( texRect[i].y < 0 )
		{
			rect[i].SetBottom( rect[i].GetBottom() - rect[i].height * -texRect[i].y / texRect[i].height );
			texRect[i].SetTop( 0 );
		}
		if( texRect[i].GetBottom() > 1 )
		{
			rect[i].SetTop( rect[i].GetTop() + rect[i].height * ( texRect[i].GetBottom() - 1 ) / texRect[i].height );
			texRect[i].SetBottom( 1 );
		}
		auto pImage = static_cast<CImage2D*>( m_pComboBar[i].GetPtr() );
		pImage->SetRect( rect[i] );
		pImage->SetTexRect( texRect[i] );
		pImage->SetBoundDirty();
	}
}

void CMainUI::OnBonusStageText( const char * szText )
{
	auto pText = SafeCast<CSimpleText>( m_pFloatText->GetRoot()->CreateInstance() );
	pText->SetParentEntity( this );
	pText->Set( szText );
	pText->SetParam( CVector4( 1.0f, 0.2f, 1.0f, 1.0f ) );
	pText->FadeAnim( CVector2( 0, 128 ), 1.0f, false );
	pText->SetPosition( CVector2( m_pComboBar[0]->x - 128, m_pComboBar[0]->y ) );
}

void CMainUI::AddSpBarShake( const CVector2& dir, uint32 nTime )
{
	SSpBarShake newShake;
	float fAngle = SRand::Inst().Rand<float>( -PI * 0.1f, PI * 0.1f );
	CMatrix2D mat;
	mat.Rotate( fAngle );
	newShake.maxOfs = mat.MulVector2Dir( dir );
	newShake.nMaxTime = nTime;
	newShake.t = 0;
	m_vecSpBarShakes.push_back( newShake );
}

void CMainUI::OnAddConsumable( CConsumable * pConsumable, int8 i )
{
	m_pConsumableSlot[i]->AddChild( pConsumable->GetIcon()->CreateInstance() );
}

void CMainUI::OnRemoveConsumable( int8 i )
{
	m_pConsumableSlot[i]->RemoveAllChild();
}

void CMainUI::Tick()
{
	float fElapsedTime = CGame::Inst().GetElapsedTimePerTick();

	CVector2 ofs( 0, 0 );
	for( int i = m_vecSpBarShakes.size() - 1; i >= 0; i-- )
	{
		auto& item = m_vecSpBarShakes[i];
		item.t++;
		if( item.t >= item.nMaxTime )
		{
			item = m_vecSpBarShakes.back();
			m_vecSpBarShakes.pop_back();
			continue;
		}

		float t = item.t * 1.0f / item.nMaxTime;
		t = t * t * t;
		t = 1 - abs( t - 0.5f ) * 2;
		ofs = ofs + item.maxOfs * t;

		float t1 = item.t & 7;
		( t1 = abs( t1 - 4 ) - 2 ) / 2;
		ofs = ofs * t1;
	}
	ofs.x = floor( ofs.x + 0.5f );
	ofs.y = floor( ofs.y + 0.5f );
	m_spBarOfs = ofs;
	m_pSpBarRoot->SetPosition( m_spBarOrigPos + m_spBarOfs );

	float fShakeStrength = CMyLevel::GetInst()->GetShakeStrength();
	float fPercent0 = fShakeStrength / 64.0f;
	float fPercent = Min( fPercent0, 1.0f );
	fPercent0 -= fPercent;

	auto pShake = static_cast<CImage2D*>( m_pShake.GetPtr() );
	auto rect = pShake->GetElem().rect;
	rect.height = floor( m_shakeBarOrigHeight * fPercent + 0.5f );
	pShake->SetRect( rect );
	CRectangle texRect = m_shakeBarOrigTexRect;
	texRect.SetTop( texRect.y + texRect.height * ( 1 - rect.height / m_shakeBarOrigHeight ) );
	pShake->SetTexRect( texRect );

	CVector2 shake = fPercent0 > 0? CVector2( cos( IRenderSystem::Inst()->GetTotalTime() * 1.3592987 * 60 ), cos( IRenderSystem::Inst()->GetTotalTime() * 1.4112051 * 60 ) ) * 4.0f : CVector2( 0, 0 );
	pShake->SetPosition( m_shakeBarOrigPos + shake );

	if( m_pUseTarget )
	{
		CRectangle rect = m_pUseText->GetTextRect().Offset( m_pUseText->GetPosition() ) + static_cast<CImage2D*>( m_pUse.GetPtr() )->GetElem().rect;
		m_pUse->SetPosition( m_useOfs - rect.GetCenter() + m_pUseTarget->globalTransform.GetPosition()
			- CMyLevel::GetInst()->GetStage()->GetCamera().GetViewArea().GetCenter() );
	}

	CGame::Inst().Register( 1, &m_tick );
}

void Game_ShaderImplement_Dummy_MainUI()
{

}