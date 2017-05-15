#include "stdafx.h"
#include "MainUI.h"
#include "MyGame.h"
#include "Render/Scene2DManager.h"
#include "HpBar.h"
#include "Common/Rand.h"
#include "Player.h"
#include "TileMap2D.h"
#include "MyLevel.h"

CMainUI* CMainUI::s_pLevel;

CMainUI::CMainUI( const SClassCreateContext& context )
	: CEntity( context )
	, m_tick( this, &CMainUI::Tick )
	, m_tickNewHpBarShake( this, &CMainUI::OnNewHpBarShake )
{
}

void CMainUI::OnAddedToStage()
{
	m_hpBarOrigPos = m_pHpBar->GetPosition();
	m_hpBarOrigHeight = static_cast<CImage2D*>( m_pHpBar.GetPtr() )->GetElem().rect.height;
	m_shakeBarOrigPos = m_pShake->GetPosition();
	m_shakeBarOrigHeight = static_cast<CImage2D*>( m_pShake.GetPtr() )->GetElem().rect.height;
	m_shakeBarOrigTexRect = static_cast<CImage2D*>( m_pShake.GetPtr() )->GetElem().texRect;

	auto pMinimap = static_cast<CTileMap2D*>( m_pMinimap.GetPtr() );
	m_blockTypes.resize( pMinimap->GetWidth() * pMinimap->GetHeight() * 2 );
	memset( &m_blockTypes[0], -1, m_blockTypes.size() );

	auto pShakeSmallBar = static_cast<CImage2D*>( m_pShakeSmallBars[0].GetPtr() );
	for( int i = 1; i < ELEM_COUNT( m_pShakeSmallBars ); i++ )
	{
		m_pShakeSmallBars[i] = new CImage2D( pShakeSmallBar->GetColorDrawable(), NULL, pShakeSmallBar->GetElem().rect.Offset( CVector2( i * 4, 0 ) ), pShakeSmallBar->GetElem().texRect, false );
		pShakeSmallBar->GetParent()->AddChildAfter( m_pShakeSmallBars[i], m_pShakeSmallBars[i - 1] );
	}

	GetStage()->RegisterBeforeHitTest( 1, &m_tick );

	CPlayer* pPlayer = GetStage()->GetWorld()->GetPlayer();
	if( pPlayer )
		OnModifyHp( pPlayer->GetHp(), pPlayer->GetMaxHp() );
	s_pLevel = this;
}

void CMainUI::OnRemovedFromStage()
{
	if( m_tick.IsRegistered() )
		m_tick.Unregister();
	if( m_tickNewHpBarShake.IsRegistered() )
		m_tickNewHpBarShake.Unregister();
	s_pLevel = NULL;
}

void CMainUI::OnModifyHp( float fHp, float fMaxHp )
{
	float fPercent = fMaxHp > 0? fHp / fMaxHp: 0;
	fPercent = Max( Min( fPercent, 1.0f ), 0.0f );
	auto pHp = static_cast<CImage2D*>( m_pHpBar.GetPtr() );
	auto rect = pHp->GetElem().rect;
	rect.height = m_hpBarOrigHeight * fPercent;
	pHp->SetRect( rect );
}

void CMainUI::OnModifySp( float fSp, float fMaxSp )
{
	float fPercent = fMaxSp > 0? fSp / fMaxSp: 0;
	fPercent = Max( Min( fPercent, 1.0f ), 0.0f );
	auto pSp = static_cast<CImage2D*>( m_pSpBar.GetPtr() );
	auto rect = pSp->GetElem().rect;
	rect.height = m_hpBarOrigHeight * fPercent;
	pSp->SetRect( rect );
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

void CMainUI::Tick()
{
	float fElapsedTime = CGame::Inst().GetElapsedTimePerTick();

	CVector2 ofs( 0, 0 );
	for( int i = m_vecHpBarShakes.size() - 1; i >= 0; i-- )
	{
		auto& item = m_vecHpBarShakes[i];
		item.t++;
		if( item.t >= item.nMaxTime )
		{
			item = m_vecHpBarShakes.back();
			m_vecHpBarShakes.pop_back();
			continue;
		}

		float t = item.t * 1.0f / item.nMaxTime;
		t = t * t * t;
		t = 1 - abs( t - 0.5f ) * 2;
		ofs = ofs + item.maxOfs * t;
	}
	
	m_pHpBar->x = m_hpBarOrigPos.x + ofs.x;
	m_pHpBar->y = m_hpBarOrigPos.y + ofs.y;
	m_pHpBar->SetTransformDirty();

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

	CGame::Inst().Register( 1, &m_tick );
}

void CMainUI::OnNewHpBarShake()
{
	float fPercent = 0;
	CPlayer* pPlayer = GetStage()->GetWorld()->GetPlayer();
	if( pPlayer )
	{
		CVector3 hpPercent;
		hpPercent.x = 1 - pPlayer->GetHp() * 1.0f / pPlayer->GetMaxHp();
		hpPercent.y = hpPercent.z = 0;
		fPercent = hpPercent.Length() / 1.732f;
		fPercent = sqrt( fPercent );
	}

	SHpBarShake newShake;
	float fRadius = SRand::Inst().Rand( 1.0f + 3.0f * fPercent, 1.5f + 4.5f * fPercent );
	float fAngle = SRand::Inst().Rand<float>( 0, PI * 2 );
	newShake.maxOfs = CVector2( fRadius * cos( fAngle ) * 0.75f, fRadius * sin( fAngle ) * 1.5f );
	newShake.nMaxTime = SRand::Inst().Rand( 10, 60 );
	newShake.t = 0;
	m_vecHpBarShakes.push_back( newShake );

	CGame::Inst().Register( SRand::Inst().Rand( 50 - 40 * fPercent, 80 - 60 * fPercent ), &m_tickNewHpBarShake );
}

void Game_ShaderImplement_Dummy_MainUI()
{

}