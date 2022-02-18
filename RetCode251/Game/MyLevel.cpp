#include "stdafx.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Render/TileMap2D.h"
#include "Stage.h"
#include "World.h"
#include "Player.h"
#include "GlobalCfg.h"
#include "MyGame.h"
#include "LightRendering.h"
#include <algorithm>
#include "Rand.h"

void CLightArea::OnRemovedFromStage()
{
	if( m_bAdded )
	{
		m_bAdded = false;
		auto p = GetLevel()->GetBlackRegion();
		if( p )
			p->UpdateLight( this, m_lastLightPos, false );
	}
}

void CLightArea::PostUpdate()
{
	auto p = GetLevel()->GetBlackRegion();
	if( !p )
		return;
	auto pos = GetGlobalTransform().GetPosition();
	if( m_bAdded )
	{
		if( pos == m_lastLightPos )
			return;
		p->UpdateLight( this, m_lastLightPos, false );
	}
	m_bAdded = true;
	p->UpdateLight( this, pos, true );
	m_lastLightPos = pos;
}

void CBlackRegion::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	SetRenderObject( NULL );

	auto pLevel = SafeCast<CMyLevel>( GetParentEntity() );
	if( pLevel )
	{
		auto size = pLevel->GetSize();
		auto fGridSize = GetGridSize();
		auto xMin = floor( size.x / fGridSize );
		auto yMin = floor( size.y / fGridSize );
		auto xMax = ceil( size.GetRight() / fGridSize );
		auto yMax = ceil( size.GetBottom() / fGridSize );
		m_size = TRectangle<int32>( xMin, yMin, xMax - xMin, yMax - yMin );
		m_vecGrid.resize( m_size.width * m_size.height );

		for( int i = 0; i < m_size.width; i++ )
		{
			for( int j = 0; j < m_size.height; j++ )
			{
				auto& grid = m_vecGrid[i + j * m_size.width];
				grid.tex.x = SRand::Inst<eRand_Render>().Rand( 0, 512 );
				grid.tex.y = SRand::Inst<eRand_Render>().Rand( 0, 512 );
				grid.texSpeed.x = SRand::Inst<eRand_Render>().Rand( -4, 5 );
				grid.texSpeed.y = SRand::Inst<eRand_Render>().Rand( -4, 5 );
			}
		}
	}
}

void CBlackRegion::OnPreview()
{
	SetRenderObject( NULL );
}

void CBlackRegion::PostUpdate( CPlayerCross* pPlayer, bool bPlayerAttached )
{
	m_nShowType = bPlayerAttached ? 0 : 1;
	float fGridSize = GetGridSize();
	auto p = pPlayer->globalTransform.GetPosition();
	int32 i = floor( p.x / fGridSize ) - m_size.x;
	int32 j = floor( p.y / fGridSize ) - m_size.y;
	bool bOpen = false;
	if( i >= 0 && j >= 0 && i < m_size.width && j < m_size.height )
	{
		auto& grid = m_vecGrid[i + j * m_size.width];
		if( grid.nLight )
			bOpen = true;
	}
	if( !bOpen )
	{
		CCharacter::SDamageContext context;
		context.nDamage = 100;
		pPlayer->Damage( context );
	}
}

void CBlackRegion::UpdateLight( CLightArea* pLightArea, const CVector2& p, bool bAdd )
{
	float fGridSize = GetGridSize();

	float fRad = pLightArea->GetRad();
	float fRad1 = pLightArea->GetRad1();
	CRectangle rect( p.x - fRad1, p.y - fRad1, fRad1 * 2, fRad1 * 2 );
	rect = rect / fGridSize;
	int32 xMin = Max( m_size.x, (int32)floor( rect.x ) );
	int32 yMin = Max( m_size.y, (int32)floor( rect.y ) );
	int32 xMax = Min( m_size.GetRight(), (int32)ceil( rect.GetRight() ) );
	int32 yMax = Min( m_size.GetBottom(), (int32)ceil( rect.GetBottom() ) );
	for( int x = xMin; x < xMax; x++ )
	{
		int32 i = x - m_size.x;
		for( int y = yMin; y < yMax; y++ )
		{
			int32 j = y - m_size.y;
			auto d = CVector2( ( x + 0.5f ) * fGridSize, ( y + 0.5f ) * fGridSize ) - p;
			if( d.Length2() < fRad1 * fRad1 )
			{
				auto& grid = m_vecGrid[i + j * m_size.width];
				if( bAdd )
					grid.bExplored = true;
				if( d.Length2() < fRad * fRad )
				{
					if( bAdd )
						grid.nLight++;
					else
						grid.nLight--;
				}
			}

		}
	}
}

void CBlackRegion::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_vecElems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

void CBlackRegion::UpdateImages( const CRectangle& viewRect )
{
	SetLocalBound( viewRect );
	m_vecElems.resize( 0 );
	m_vecParams.resize( 0 );
	float fGridSize = GetGridSize();
	int32 nTexSize = 512;
	int32 nLeft0 = floor( viewRect.x / fGridSize );
	int32 nTop0 = floor( viewRect.y / fGridSize );
	int32 nRight0 = ceil( viewRect.GetRight() / fGridSize );
	int32 nBottom0 = ceil( viewRect.GetBottom() / fGridSize );
	int32 nLeft = Max( m_size.x, nLeft0 );
	int32 nTop = Max( m_size.y, nTop0 );
	int32 nRight = Min( m_size.GetRight(), nRight0 );
	int32 nBottom = Min( m_size.GetBottom(), nBottom0 );
	for( int x = nLeft; x < nRight; x++ )
	{
		auto i = x - m_size.x;
		for( int y = nTop; y < nBottom; y++ )
		{
			auto j = y - m_size.y;
			auto& grid = m_vecGrid[i + j * m_size.width];
			grid.tex.x = ( grid.tex.x + grid.texSpeed.x + nTexSize ) % nTexSize;
			grid.tex.y = ( grid.tex.y + grid.texSpeed.y + nTexSize ) % nTexSize;
			if( !SRand::Inst<eRand_Render>().Rand( 0, 3 ) )
			{
				grid.tex.x = SRand::Inst<eRand_Render>().Rand( 0, nTexSize );
				grid.tex.y = SRand::Inst<eRand_Render>().Rand( 0, nTexSize );
				grid.texSpeed.x = SRand::Inst<eRand_Render>().Rand( -4, 5 );
				grid.texSpeed.y = SRand::Inst<eRand_Render>().Rand( -4, 5 );
			}
		}
	}

	CVector4 paramBorder( 0.25f, 0.25f, 0.25f, 1 );
	if( nLeft0 < nLeft )
	{
		for( int y = nTop; y < nBottom; y++ )
		{
			auto& grid = m_vecGrid[( y - m_size.y ) * m_size.width];
			auto rect = CRectangle( nLeft0, y, nLeft - nLeft0, 1 ) * fGridSize;
			auto texRect = CRectangle( grid.tex.x + 0.25f, grid.tex.y, 0.5f, fGridSize ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
		if( nTop0 < nTop )
		{
			auto& grid = m_vecGrid[0];
			auto rect = CRectangle( nLeft0, nTop0, nLeft - nLeft0, nTop - nTop0 ) * fGridSize;
			auto texRect = CRectangle( grid.tex.x + 0.25f, ( grid.tex.y + 31 ) % nTexSize + 0.25f, 0.5f, 0.5f ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
		if( nBottom0 > nBottom )
		{
			auto& grid = m_vecGrid[( m_size.height - 1 ) * m_size.width];
			auto rect = CRectangle( nLeft0, nBottom, nLeft - nLeft0, nBottom0 - nBottom ) * fGridSize;
			auto texRect = CRectangle( grid.tex.x + 0.25f, grid.tex.y + 0.25f, 0.5f, 0.5f ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
	}
	if( nRight0 > nRight )
	{
		for( int y = nTop; y < nBottom; y++ )
		{
			auto& grid = m_vecGrid[m_size.width - 1 + ( y - m_size.y ) * m_size.width];
			auto rect = CRectangle( nRight, y, nRight0 - nRight, 1 ) * fGridSize;
			auto texRect = CRectangle( ( grid.tex.x + 31 ) % nTexSize + 0.25f, grid.tex.y, 0.5f, fGridSize ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
		if( nTop0 < nTop )
		{
			auto& grid = m_vecGrid[m_size.width - 1];
			auto rect = CRectangle( nRight, nTop0, nRight0 - nRight, nTop - nTop0 ) * fGridSize;
			auto texRect = CRectangle( ( grid.tex.x + 31 ) % nTexSize + 0.25f, ( grid.tex.y + 31 ) % nTexSize + 0.25f, 0.5f, 0.5f ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
		if( nBottom0 > nBottom )
		{
			auto& grid = m_vecGrid[m_size.width - 1 + ( m_size.height - 1 ) * m_size.width];
			auto rect = CRectangle( nRight, nBottom, nRight0 - nRight, nBottom0 - nBottom ) * fGridSize;
			auto texRect = CRectangle( ( grid.tex.x + 31 ) % nTexSize + 0.25f, grid.tex.y + 0.25f, 0.5f, 0.5f ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
	}
	if( nTop0 < nTop )
	{
		for( int x = nLeft; x < nRight; x++ )
		{
			auto& grid = m_vecGrid[x - m_size.x];
			auto rect = CRectangle( x, nTop0, 1, nTop - nTop0 ) * fGridSize;
			auto texRect = CRectangle( grid.tex.x, ( grid.tex.y + 31 ) % nTexSize + 0.25f, fGridSize, 0.5f ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
	}
	if( nBottom0 > nBottom )
	{
		for( int x = nLeft; x < nRight; x++ )
		{
			auto& grid = m_vecGrid[x - m_size.x + ( m_size.height - 1 ) * m_size.width];
			auto rect = CRectangle( x, nBottom, 1, nBottom0 - nBottom ) * fGridSize;
			auto texRect = CRectangle( grid.tex.x, grid.tex.y + 0.25f, fGridSize, 0.5f ) / nTexSize;
			AddImage( rect, texRect, paramBorder );
		}
	}

	CVector4 paramUnexplored( 0.25f, 0.25f, 0.25f, 1 );
	CVector4 paramUnlit( 0.0625f, 0.0625f, 0.0625f, 0.25f );
	CVector4 paramGrid( 1, 1, 1, 1 );
	CVector4 paramGrid1( 2, 0.5f, 0.5f, 1 );
	CVector4 paramCursor( 2, 2, 2, 1 );
	auto pLevel = SafeCast<CMyLevel>( GetParentEntity() );
	auto cursorPos = pLevel->GetPlayer()->GetPosition();
	cursorPos.x = floor( cursorPos.x + 0.5f );
	cursorPos.y = floor( cursorPos.y + 0.5f );
	
	if( m_nShowType )
	{
		for( int x = nLeft0; x <= nRight0; x++ )
		{
			int32 i = x - m_size.x;
			int32 y0 = nTop0;
			bool b = false;
			for( int y = nTop0; y <= nBottom0; y++ )
			{
				int8 bOpen = 2;
				if( y < nBottom0 )
				{
					bOpen = 0;
					int32 j = y - m_size.y;
					if( j >= 0 && j < m_size.height )
					{
						for( int k = 0; k < 2; k++ )
						{
							int8 bOpen1;
							if( i - k < 0 || i - k >= m_size.width )
								bOpen1 = false;
							else
								bOpen1 = m_vecGrid[i - k + j * m_size.width].nLight > 0;
							bOpen += bOpen1;
						}
					}
				}

				if( bOpen == 2 )
				{
					if( y > y0 && b )
					{
						auto rect = CRectangle( x * fGridSize - 1, y0 * fGridSize, 2, ( y - y0 ) * fGridSize );
						AddImage1( rect, paramGrid1 );
					}
					y0 = y + 1;
				}
				else if( bOpen == 1 )
					b = true;
			}
		}
		for( int y = nTop0; y <= nBottom0; y++ )
		{
			int32 j = y - m_size.y;
			int32 x0 = nLeft0;
			bool b = false;
			for( int x = nLeft0; x <= nRight0; x++ )
			{
				int8 bOpen = 2;
				if( x < nRight0 )
				{
					bOpen = 0;
					int32 i = x - m_size.x;
					if( i >= 0 && i < m_size.width )
					{
						for( int k = 0; k < 2; k++ )
						{
							int8 bOpen1;
							if( j - k < 0 || j - k >= m_size.height )
								bOpen1 = false;
							else
								bOpen1 = m_vecGrid[i + ( j - k ) * m_size.width].nLight > 0;
							bOpen += bOpen1;
						}
					}
				}

				if( bOpen == 2 )
				{
					if( x > x0 && b )
					{
						auto rect = CRectangle( x0 * fGridSize, y * fGridSize - 1, ( x - x0 ) * fGridSize, 2 );
						AddImage1( rect, paramGrid1 );
					}
					x0 = x + 1;
				}
				else if( bOpen == 1 )
					b = true;
			}
		}
	}
	else
	{
		AddImage1( CRectangle( viewRect.x, cursorPos.y - 1, cursorPos.x - viewRect.x - 128, 2 ), paramGrid );
		AddImage1( CRectangle( cursorPos.x - 1, viewRect.y, 2, cursorPos.y - viewRect.y - 128 ), paramGrid );
		AddImage1( CRectangle( cursorPos.x + 128, cursorPos.y - 1, viewRect.GetRight() - cursorPos.x - 128, 2 ), paramGrid );
		AddImage1( CRectangle( cursorPos.x - 1, cursorPos.y + 128, 2, viewRect.GetBottom() - cursorPos.y - 128 ), paramGrid );

		for( int x = nLeft; x <= nRight; x++ )
		{
			int32 i = x - m_size.x;
			for( int y = nTop; y < nBottom; y++ )
			{
				int8 bOpen = false;
				int32 j = y - m_size.y;
				if( j >= 0 && j < m_size.height )
				{
					bOpen = true;
					for( int k = 0; k < 2; k++ )
					{
						int8 bOpen1;
						if( i - k < 0 || i - k >= m_size.width )
							bOpen1 = false;
						else
							bOpen1 = m_vecGrid[i - k + j * m_size.width].nLight > 0;
						bOpen = bOpen ^ bOpen1;
					}
				}

				if( !bOpen )
				{
					auto rect = CRectangle( x * fGridSize - 1, y * fGridSize, 2, fGridSize );
					AddImage1( rect, paramGrid );
				}
			}
		}
		for( int y = nTop; y <= nBottom; y++ )
		{
			int32 j = y - m_size.y;
			for( int x = nLeft; x < nRight; x++ )
			{
				int8 bOpen = false;
				int32 i = x - m_size.x;
				if( i >= 0 && i < m_size.width )
				{
					bOpen = true;
					for( int k = 0; k < 2; k++ )
					{
						int8 bOpen1;
						if( j - k < 0 || j - k >= m_size.height )
							bOpen1 = false;
						else
							bOpen1 = m_vecGrid[i + ( j - k ) * m_size.width].nLight > 0;
						bOpen = bOpen ^ bOpen1;
					}
				}

				if( !bOpen )
				{
					auto rect = CRectangle( x * fGridSize, y * fGridSize - 1, fGridSize, 2 );
					AddImage1( rect, paramGrid );
				}
			}
		}
	}

	for( int x = nLeft; x < nRight; x++ )
	{
		int32 i = x - m_size.x;
		for( int y = nTop; y < nBottom; y++ )
		{
			int32 j = y - m_size.y;
			auto& grid = m_vecGrid[i + j * m_size.width];
			auto rect = CRectangle( x, y, 1, 1 ) * fGridSize;
			auto texRect = CRectangle( grid.tex.x, grid.tex.y, fGridSize, fGridSize ) / nTexSize;
			bool bExplored = grid.bExplored;
			if( !bExplored )
			{
				AddImage( rect, texRect, paramUnexplored );
				continue;
			}

			if( grid.nLight > 0 )
				continue;
			AddImage( rect, texRect, paramUnlit );
		}
	}
	UpdateCharIndicators( pLevel, cursorPos, viewRect );

	for( int i = 0; i < m_vecElems.size(); i++ )
	{
		m_vecElems[i].nInstDataSize = sizeof( CVector4 );
		m_vecElems[i].pInstData = &m_vecParams[i];
	}
}

void CBlackRegion::UpdateCharIndicators( CMyLevel* pLevel, const CVector2& cursorPos, const CRectangle& viewRect )
{
	if( m_nShowType )
	{
		auto p = pLevel->GetPlayer()->GetCurLockedTarget();
		bool bAttach = pLevel->GetPlayer()->IsTryingToAttach();
		auto fPlayerPickRad = pLevel->GetPlayerPickRad();
		if( p != m_pLastTarget )
		{
			m_pLastTarget = p;
			m_targetRect[0] = CRectangle( cursorPos.x - fPlayerPickRad, cursorPos.y - fPlayerPickRad, fPlayerPickRad * 2, fPlayerPickRad * 2 );
			if( m_pLastTarget )
			{
				auto pos = m_pLastTarget->globalTransform.GetPosition();
				m_targetRect[1] = CRectangle( pos.x - 8, pos.y - 8, 16, 16 );
			}
		}
		else if( m_pLastTarget )
		{
			CRectangle r[2] = { CRectangle( cursorPos.x - fPlayerPickRad, cursorPos.y - fPlayerPickRad, fPlayerPickRad * 2, fPlayerPickRad * 2 ),
				m_pLastTarget->GetPlayerPickBound() };
			for( int i = 0; i < 2; i++ )
			{
				if( bAttach )
					m_targetRect[i] = m_targetRect[i] + r[i];
				else
					m_targetRect[i].SetCenter( r[i].GetCenter() );
			}
			if( bAttach )
				r[0] = r[1] = r[0] + r[1];
			for( int i = 0; i < 2; i++ )
			{
				float f[4] = { m_targetRect[i].GetCenterX(), m_targetRect[i].GetCenterY(), m_targetRect[i].width / 2, m_targetRect[i].height / 2 };
				float f1[4] = { r[i].GetCenterX(), r[i].GetCenterY(), r[i].width / 2, r[i].height / 2 };
				float d = 4;
				if( !bAttach && m_targetRect[i].width < r[i].width && m_targetRect[i].height < r[i].height)
					d = 1;
				for( int k = 0; k < 4; k++ )
					f[k] = Max( f[k] - d, Min( f[k] + d, f1[k] ) );
				m_targetRect[i] = CRectangle( f[0] - f[2], f[1] - f[3], f[2] * 2, f[3] * 2 );
			}
		}
		else
			m_targetRect[0] = CRectangle( cursorPos.x - fPlayerPickRad, cursorPos.y - fPlayerPickRad, fPlayerPickRad * 2, fPlayerPickRad * 2 );

		CVector4 param0( 2, 1, 1, 1 );
		CVector4 param( 2, 2, 2, 1 );
		AddImage1( CRectangle( cursorPos.x - 2, cursorPos.y - 2, 4, 4 ), param0 );

		AddImage1( CRectangle( m_targetRect[0].x, m_targetRect[0].y + m_targetRect[0].height / 4, 2, m_targetRect[0].height / 2 ), param );
		AddImage1( CRectangle( m_targetRect[0].x + m_targetRect[0].width / 4, m_targetRect[0].y, m_targetRect[0].width / 2, 2 ), param );
		AddImage1( CRectangle( m_targetRect[0].GetRight() - 2, m_targetRect[0].y + m_targetRect[0].height / 4, 2, m_targetRect[0].height / 2 ), param );
		AddImage1( CRectangle( m_targetRect[0].x + m_targetRect[0].width / 4, m_targetRect[0].GetBottom() - 2, m_targetRect[0].width / 2, 2 ), param );

		if( m_pLastTarget )
		{
			AddImage1( CRectangle( m_targetRect[1].x, m_targetRect[1].y, 16, 2 ), param );
			AddImage1( CRectangle( m_targetRect[1].x, m_targetRect[1].y, 2, 16 ), param );
			AddImage1( CRectangle( m_targetRect[1].GetRight() - 16, m_targetRect[1].y, 16, 2 ), param );
			AddImage1( CRectangle( m_targetRect[1].GetRight() - 2, m_targetRect[1].y, 2, 16 ), param );
			AddImage1( CRectangle( m_targetRect[1].x, m_targetRect[1].GetBottom() - 2, 16, 2 ), param );
			AddImage1( CRectangle( m_targetRect[1].x, m_targetRect[1].GetBottom() - 16, 2, 16 ), param );
			AddImage1( CRectangle( m_targetRect[1].GetRight() - 16, m_targetRect[1].GetBottom() - 2, 16, 2 ), param );
			AddImage1( CRectangle( m_targetRect[1].GetRight() - 2, m_targetRect[1].GetBottom() - 16, 2, 16 ), param );
		}

		for( auto pCharacter = pLevel->Get_Character( 0 ); pCharacter; pCharacter = pCharacter->NextCharacter() )
		{
			if( pCharacter == m_pLastTarget )
				continue;
			if( pCharacter->CanBeControlled() )
			{
				auto p = pCharacter->globalTransform.GetPosition();
				CRectangle rect1( p.x - 8, p.y - 8, 16, 16 );
				auto r1 = viewRect * rect1;
				if( r1.width > 0 && r1.height > 0 )
				{
					AddImage1( CRectangle( rect1.x, rect1.y, 16, 2 ), param );
					AddImage1( CRectangle( rect1.x, rect1.y, 2, 16 ), param );
					AddImage1( CRectangle( rect1.x, rect1.GetBottom() - 2, 16, 2 ), param );
					AddImage1( CRectangle( rect1.GetRight() - 2, rect1.y, 2, 16 ), param );
				}
			}
		}
	}
	else
	{
		m_pLastTarget = NULL;
	}
}

void CBlackRegion::AddImage( const CRectangle& rect, const CRectangle& texRect, const CVector4& param )
{
	m_vecElems.resize( m_vecElems.size() + 1 );
	auto& elem = m_vecElems.back();
	elem.rect = rect;
	elem.texRect = texRect;
	m_vecParams.push_back( param );
}

void CBlackRegion::AddImage1( const CRectangle& rect, const CVector4& param )
{
	if( rect.width <= 0 || rect.height <= 0 )
		return;
	int32 n0 = m_vecElems.size();
	m_vecElems.resize( m_vecElems.size() + 1 );
	auto& elem = m_vecElems.back();
	elem.rect = rect;
	int32 nTexSize = 512;

	for( int i = n0; i < m_vecElems.size(); )
	{
		auto rect = m_vecElems[i].rect;
		if( rect.width > rect.height )
		{
			if( rect.width >= SRand::Inst<eRand_Render>().Rand( 64, 256 ) )
			{
				auto w = SRand::Inst<eRand_Render>().Rand<int32>( 32, rect.width - 32 + 1 );
				m_vecElems[i].rect = CRectangle( rect.x, rect.y, w, rect.height );
				m_vecElems.resize( m_vecElems.size() + 1 );
				m_vecElems.back().rect = CRectangle( rect.x + w, rect.y, rect.width - w, rect.height );
				continue;
			}
		}
		else
		{
			if( rect.height >= SRand::Inst<eRand_Render>().Rand( 64, 256 ) )
			{
				auto h = SRand::Inst<eRand_Render>().Rand<int32>( 32, rect.height - 32 + 1 );
				m_vecElems[i].rect = CRectangle( rect.x, rect.y, rect.width, h );
				m_vecElems.resize( m_vecElems.size() + 1 );
				m_vecElems.back().rect = CRectangle( rect.x, rect.y + h, rect.width, rect.height - h );
				continue;
			}
		}
		m_vecElems[i].texRect = CRectangle( SRand::Inst<eRand_Render>().Rand<int32>( 0, nTexSize ),
			SRand::Inst<eRand_Render>().Rand<int32>( 0, nTexSize ), rect.width, rect.height ) / nTexSize;
		i++;
	}

	for( int i = m_vecElems.size(); i > n0; i-- )
		m_vecParams.push_back( param );
}

void CLevelEnvLayer::OnAddedToStage()
{
	auto pParent = GetParentEntity();
	CMyLevel* pLevel = NULL;
	while( pParent )
	{
		pLevel = SafeCast<CMyLevel>( pParent );
		if( pLevel )
			break;
		pParent = pParent->GetParentEntity();
	}
	if( pLevel )
	{
		m_pLevel = pLevel;
		if( Get_HitProxy() )
			pLevel->GetHitTestMgr( 0 ).Add( this );
	}
}

void CLevelEnvLayer::OnRemovedFromStage()
{
	if( m_pLevel )
	{
		if( Get_HitProxy() )
			m_pLevel->GetHitTestMgr( 0 ).Remove( this );
		m_pLevel = NULL;
	}
}

void CLevelEnvLayer::InitCtrlPoints()
{
	if( m_bCtrlPointsInited )
		return;
	m_bCtrlPointsInited = true;
	m_vecCtrlPointStates.resize( m_arrCtrlPoint.Size() + 2 );
	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& state = m_vecCtrlPointStates[iPoint];
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		state.nPointType = data.nPointType;

		float t0 = 0;
		auto nSize = data.arrPath.Size();
		for( int i = 0; i < nSize; i++ )
		{
			auto p0 = data.arrPath[i];
			auto p3 = data.arrPath[( i + 1 ) % nSize];
			if( i == 0 )
				p0.z = 0;
			auto p1 = p0 + data.arrTangent[i];
			auto p2 = p3 - data.arrTangent[( i + 1 ) % nSize];
			float t1 = p3.z;
			float s = 0;
			int32 iFrame = state.vecFrames.size();
			int32 nFrames1 = ceil( t1 );
			state.vecFrames.resize( nFrames1 );
			for( ; iFrame < nFrames1; iFrame++ )
			{
				auto& item = state.vecFrames[iFrame];
				float t = iFrame;
				float sx2, sx3;

				float a = -p0.z + 3 * p1.z - 3 * p2.z + p3.z;
				float b = 3 * p0.z - 6 * p1.z + 3 * p2.z;
				float c = -3 * p0.z + 3 * p1.z;
				float d = p0.z;

				for( ;; )
				{
					sx2 = s * s;
					sx3 = s * sx2;
					float dt = t - ( a * sx3 + b * sx2 + c * s + d );
					if( abs( dt ) < 0.001f )
						break;
					float dtds = 3 * a * sx2 + 2 * b * s + c;
					float ds = dt / dtds;
					s += ds;
				}

				float invs = 1 - s;
				float invsx2 = invs * invs;
				float invsx3 = invsx2 * invs;
				CVector4 coefs( invsx3, 3 * s * invsx2, 3 * sx2 * invs, sx3 );
				item.x = coefs.Dot( CVector4( p0.x, p1.x, p2.x, p3.x ) );
				item.y = coefs.Dot( CVector4( p0.y, p1.y, p2.y, p3.y ) );
				item = item - data.orig;
			}
		}

		state.p = data.orig;
		state.v = CVector2( 0, 0 );
		state.f1 = CVector2( 0, 0 );
		state.f2 = CVector2( 0, 0 );
		state.nCurFrame = 0;
	}

	for( int iLink = 0; iLink < m_arrCtrlLink.Size(); iLink++ )
	{
		auto& link = m_arrCtrlLink[iLink];
		if( link.n1 < -2 || link.n1 >= (int32)m_arrCtrlPoint.Size() || link.n2 < -2 || link.n2 >= (int32)m_arrCtrlPoint.Size() )
			continue;
		auto& pointData1 = link.n1 == -2 ? m_ctrlPoint1 : ( link.n1 == -1 ? m_ctrlPoint2 : m_arrCtrlPoint[link.n1] );
		auto& pointData2 = link.n2 == -2 ? m_ctrlPoint1 : ( link.n2 == -1 ? m_ctrlPoint2 : m_arrCtrlPoint[link.n2] );
		link.l0 = ( pointData2.orig + link.ofs2 - pointData1.orig - link.ofs1 ).Length();
	}
}

void CLevelEnvLayer::InitCtrlPointsState( float x, float y, float r, float s, bool bNoReset )
{
	InitCtrlPoints();
	if( !bNoReset )
	{
		for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
		{
			auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
			auto& state = m_vecCtrlPointStates[iPoint];
			state.p = data.orig;
			state.v = CVector2( 0, 0 );
			state.f1 = CVector2( 0, 0 );
			state.f2 = CVector2( 0, 0 );
			state.nCurFrame = 0;
		}
	}

	CMatrix2D trans;
	trans.Transform( x, y, r, s );

	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		if( data.nResetType )
		{
			auto& state = m_vecCtrlPointStates[iPoint];
			state.p = trans.MulVector2Pos( data.orig );
			if( state.vecFrames.size() )
				state.p = state.p - state.vecFrames[state.nCurFrame];
		}
	}
}

void CLevelEnvLayer::UpdateCtrlPoints()
{
	InitCtrlPoints();
	float dTime = 1.0f / 60;
	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		auto& state = m_vecCtrlPointStates[iPoint];

		if( state.vecFrames.size() )
		{
			state.nCurFrame++;
			if( state.nCurFrame >= state.vecFrames.size() )
				state.nCurFrame = 0;
		}
	}

	for( auto& item : m_vecExtraForces )
	{
		auto& data = item.nIndex < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[item.nIndex] : ( item.nIndex == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		auto& state = m_vecCtrlPointStates[item.nIndex];
		float t = 1.0f;
		if( item.nFadeType == 1 )
			t = item.nTimeLeft * 1.0f / item.nDuration;
		else if( item.nFadeType == 2 )
		{
			t = item.nTimeLeft * 1.0f / item.nDuration;
			t = t * t;
		}
		auto force = item.force * t;
		if( data.fWeight > 0 )
		{
			if( item.nType )
				state.f2 = state.f2 + force;
			else
				state.f1 = state.f1 + force;
		}
		item.nTimeLeft--;
	}
	for( int i = m_vecExtraForces.size() - 1; i >= 0; i-- )
	{
		if( m_vecExtraForces[i].nTimeLeft <= 0 )
		{
			m_vecExtraForces[i] = m_vecExtraForces.back();
			m_vecExtraForces.resize( m_vecExtraForces.size() - 1 );
		}
	}

	for( int i = 0; i < m_arrCtrlLink.Size(); i++ )
	{
		auto& link = m_arrCtrlLink[i];
		auto& point1 = m_vecCtrlPointStates[link.n1 < 0 ? link.n1 + m_vecCtrlPointStates.size() : link.n1];
		auto& point2 = m_vecCtrlPointStates[link.n2 < 0 ? link.n2 + m_vecCtrlPointStates.size() : link.n2];
		auto p1 = GetCtrlPointCurPos( point1 ) + link.ofs1;
		auto p2 = GetCtrlPointCurPos( point2 ) + link.ofs2;
		auto d = p2 - p1;
		auto d1 = d;
		d1.Normalize();
		d = d - d1 * link.l0;

		point1.f1 = point1.f1 + d * link.fStrength1;
		point1.f2 = point1.f2 + d * link.fStrength2;
		point2.f1 = point2.f1 - d * link.fStrength1;
		point2.f2 = point2.f2 - d * link.fStrength2;
	}

	for( int32 i = 0; i < m_arrCtrlSmoother.Size(); i++ )
	{
		auto& smoother = m_arrCtrlSmoother[i];
		switch( smoother.nType )
		{
		case eLevelCamCtrlPointSmoother_IgnoreSmallForce:
		{
			auto& state = m_vecCtrlPointStates[smoother.n < 0 ? smoother.n + m_vecCtrlPointStates.size() : smoother.n];
			if( abs( state.f1.x ) < smoother.params[0].x )
				state.f1.x = 0;
			if( abs( state.f1.y ) < smoother.params[0].y )
				state.f1.y = 0;
			if( abs( state.f2.x ) < smoother.params[0].z )
				state.f2.x = 0;
			if( abs( state.f2.y ) < smoother.params[0].w )
				state.f2.y = 0;
			break;
		}
		default:
			break;
		}
	}
	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& data = iPoint < m_arrCtrlPoint.Size() ? m_arrCtrlPoint[iPoint] : ( iPoint == m_arrCtrlPoint.Size() ? m_ctrlPoint1 : m_ctrlPoint2 );
		auto& state = m_vecCtrlPointStates[iPoint];
		state.p = state.p + data.g1 * dTime;
		auto v1 = state.v + data.g2 * dTime;
		if( data.fWeight )
		{
			state.p = state.p + state.f1 * ( dTime / data.fWeight );
			v1 = v1 + state.f2 * ( dTime / data.fWeight );
		}
		if( data.fDamping )
		{
			v1 = v1 * exp( -data.fDamping * dTime );
		}
		state.p = state.p + ( state.v + v1 ) * 0.5f * dTime;
		state.v = v1;
		state.f1 = CVector2( 0, 0 );
		state.f2 = CVector2( 0, 0 );
	}
	for( int32 i = 0; i < m_arrCtrlLimitor.Size(); i++ )
	{
		auto& limitor = m_arrCtrlLimitor[i];
		auto& point1 = m_vecCtrlPointStates[limitor.n1 < 0 ? limitor.n1 + m_vecCtrlPointStates.size() : limitor.n1];
		auto& point2 = m_vecCtrlPointStates[limitor.n2 < 0 ? limitor.n2 + m_vecCtrlPointStates.size() : limitor.n2];
		switch( limitor.nType )
		{
		case eLevelCamCtrlPoint1Limitor_Rect:
		{
			auto p2 = GetCtrlPointCurPos( point2 );
			CRectangle rect( p2.x + limitor.params[0].x, p2.y + limitor.params[0].y, limitor.params[0].z, limitor.params[0].w );
			point1.p.x = Min( rect.GetRight(), Max( rect.x, point1.p.x ) );
			point1.p.y = Min( rect.GetBottom(), Max( rect.y, point1.p.y ) );
			break;
		}
		default:
			break;
		}
	}

	for( int iPoint = 0; iPoint < m_vecCtrlPointStates.size(); iPoint++ )
	{
		auto& state = m_vecCtrlPointStates[iPoint];
		auto pos = GetCtrlPointCurPos( state );
		state.elemDebugDraw.rect = CRectangle( pos.x - 4, pos.y - 4, 8, 8 );
		state.elemDebugDraw.nInstDataSize = sizeof( state.debugDrawParam );
		state.elemDebugDraw.pInstData = state.debugDrawParam;
		state.debugDrawParam[0] = CVector4( 0, 0, 0, 0 );
		state.debugDrawParam[1] = CVector4( 0, 0.1, 0.5, 0 );
	}

	/*for( auto pChildEntity = Get_ChildEntity(); pChildEntity; pChildEntity = pChildEntity->NextChildEntity() )
	{
		auto p = SafeCast<CEyeChunk>( pChildEntity );
		if( p )
		{
			auto& point1 = m_vecCtrlPointStates[p->m_nIndex < 0 ? p->m_nIndex + m_vecCtrlPointStates.size() : p->m_nIndex];
			auto cur = GetCtrlPointCurPos( point1 );
			p->SetPosition( cur );
		}
	}*/
}

CVector4 CLevelEnvLayer::GetCtrlPointsTrans()
{
	InitCtrlPoints();
	auto p1 = GetCtrlPointCurPos( m_vecCtrlPointStates[m_vecCtrlPointStates.size() - 2] );
	auto p2 = GetCtrlPointCurPos( m_vecCtrlPointStates[m_vecCtrlPointStates.size() - 1] );
	p1 = ( p1 + p2 ) * 0.5f;
	auto q1 = m_ctrlPoint1.orig;
	auto q2 = m_ctrlPoint2.orig;
	q1 = ( q1 + q2 ) * 0.5f;

	auto d = p2 - p1;
	auto e = q2 - q1;
	e = e * 1.0f / e.Length2();
	e.y = -e.y;
	CVector2 de( d.x * e.x - d.y * e.y, d.x * e.y + d.y * e.x );

	float r = atan2( de.y, de.x );
	float s = de.Length();
	auto ofs0 = q1 * -1;
	ofs0 = CVector2( ofs0.x * de.x - ofs0.y * de.y, ofs0.x * de.y + ofs0.y * de.x );
	ofs0 = ofs0 + p1;
	return CVector4( ofs0.x, ofs0.y, r, s );
}

void CLevelEnvLayer::ApplyForce( int32 nIndex, int8 nType, float fForceX, float fForceY, int32 nDuration, int8 nFadeType )
{
	m_vecExtraForces.resize( m_vecExtraForces.size() + 1 );
	auto& item = m_vecExtraForces.back();
	if( nIndex < 0 )
		nIndex += m_arrCtrlPoint.Size() + 2;
	item.nIndex = nIndex;
	item.nType = nType;
	item.force.x = fForceX;
	item.force.y = fForceY;
	item.nDuration = item.nTimeLeft = nDuration;
	item.nFadeType = nFadeType;
}

CVector2 CLevelEnvLayer::GetCtrlPointCurPos( SCtrlPointState& point )
{
	auto p = point.p;
	if( point.vecFrames.size() )
		p = p + point.vecFrames[point.nCurFrame];
	if( point.nPointType == 1 )
		p = CMasterLevel::GetInst()->GetCtrlPointFollowPlayerPos( p );
	return p;
}

void CLevelObjLayer::OnPreview()
{
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
}

void CLevelBugIndicatorLayer::OnAddedToStage()
{
	SetRenderObject( NULL );
}

void CLevelBugIndicatorLayer::OnPreview()
{
	SetRenderObject( NULL );
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
}

void CLevelBugIndicatorLayer::Update()
{
	auto pLevel = SafeCast<CMyLevel>( GetParentEntity() );
	m_vecElems.resize( 0 );
	m_vecColors.resize( 0 );
	if( pLevel )
	{
		for( int i = 0; i < pLevel->m_arrBugLink.Size(); i++ )
		{
			auto& item = pLevel->m_arrBugLink[i];
			auto pBug1 = pLevel->m_vecBugListItems[item.a].pBug;
			auto pBug2 = pLevel->m_vecBugListItems[item.b].pBug;
			if( !pBug1 || !pBug2 || !pLevel->m_vecBugListItems[item.a].bDetected && !pLevel->m_vecBugListItems[item.b].bDetected )
				continue;
			auto color = CBug::GetGroupColor( pLevel->m_vecBugListItems[item.b].nGroup );
			UpdateImg( i, pLevel->m_vecBugListItems[item.b].origPos, color );
		}
	}
	CRectangle bound( 0, 0, 0, 0 );
	for( int i = 0; i < m_vecElems.size(); i++ )
	{
		bound = bound + m_vecElems[i].rect;
		m_vecElems[i].nInstDataSize = sizeof( CVector4 );
		m_vecElems[i].pInstData = &m_vecColors[i];
	}
	SetLocalBound( bound );
}

void CLevelBugIndicatorLayer::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_vecElems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

void CLevelBugIndicatorLayer::UpdateImg( int32 i, const CVector2& origPos, const CVector4 & color )
{
	auto pLevel = SafeCast<CMyLevel>( GetParentEntity() );
	auto& arrPath = pLevel->m_arrBugLink[i].arrPath;

	int8 nDir = arrPath[0] ? 0 : 1;
	int8 nDir0 = -1;
	TVector2<int32> p( 0, 0 );
	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	auto Func = [=, &p] ( int8 nImg )
	{
		m_vecElems.resize( m_vecElems.size() + 1 );
		auto& elem = m_vecElems.back();
		elem.rect = ( CRectangle( p.x - 0.5f, p.y - 0.5f, 1.0f, 1.0f ) * 32 ).Offset( origPos );
		elem.texRect = ( CRectangle( nImg % 4 * m_texRect.width, nImg / 4 * m_texRect.height,
			m_texRect.width, m_texRect.height ) * 0.25f ).Offset( CVector2( m_texRect.x, m_texRect.y ) );
		m_vecColors.resize( m_vecColors.size() + 1 );
		m_vecColors.back() = color;
	};
	for( int i = 0; i < arrPath.Size() - 1; i++ )
	{
		nDir = 1 ^ ( nDir & 1 );
		int32 nOfs = arrPath[i + 1];
		if( nOfs < 0 )
		{
			nOfs = -nOfs;
			nDir += 2;
		}
		for( int j = 0; j < nOfs; j++ )
		{
			int32 nImg;
			if( !j )
			{
				if( i )
				{
					if( Max<int8>( nDir ^ 2, nDir0 ) == 1 )
						nImg = 5;
					else if( Min<int8>( nDir ^ 2, nDir0 ) == 1 )
						nImg = 6;
					else if( Min<int8>( nDir ^ 2, nDir0 ) == 2 )
						nImg = 7;
					else
						nImg = 4;
				}
				else
					nImg = nDir;
			}
			else if( !( nDir & 1 ) )
				nImg = 8;
			else
				nImg = 9;
			Func( nImg );
			p = p + ofs[nDir];
		}
		nDir0 = nDir;
	}
	Func( nDir ^ 2 );
}

bool CPortal::CheckTeleport( CCharacter* pPlayer )
{
	/*auto pPlayerHits = pPlayer->GetAllHits();
	static vector<CHitProxy*> vecResult;
	for( int i = 0; i < 3; i++ )
	{
		auto mat = pPlayerHits[i]->GetGlobalTransform();
		vecResult.resize( 0 );
		auto pHitProxy = Get_HitProxy();
		auto pHitProxy1 = pPlayerHits[i]->Get_HitProxy();
		if( !SHitProxy::Contain( pHitProxy, pHitProxy1, GetGlobalTransform(), mat ) )
			return false;
	}
	return true;*/
	auto mat = pPlayer->GetGlobalTransform();
	auto pHitProxy = Get_HitProxy();
	auto pHitProxy1 = pPlayer->Get_HitProxy();
	if( !SHitProxy::Contain( pHitProxy, pHitProxy1, GetGlobalTransform(), mat ) )
		return false;
}

void CPortal::OnTickAfterHitTest()
{
	auto pPlayer = GetLevel()->GetPlayer();
	if( pPlayer )
	{
		if( CGame::Inst().IsKey( ' ' ) && CheckTeleport( pPlayer ) )
			CMasterLevel::GetInst()->TryTeleport( m_bUp );
	}
}

void CMyLevel::OnAddedToStage()
{
}

void CMyLevel::OnRemovedFromStage()
{
	ChangeToEnvLayer( NULL );
}

void CMyLevel::OnPreview()
{
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
	DisableLights( this );
}

void CMyLevel::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	BuildBugList();
	if( m_pBugIndicatorLayer )
		m_pBugIndicatorLayer->Update();
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
	{
		auto pObjLayer = SafeCastToInterface<ILevelObjLayer>( p );
		if( pObjLayer )
		{
			m_vecAllLayers.push_back( p );
			pObjLayer->Init();
		}
	}
	for( int i = 0; i < 2; i++ )
		m_hitTestMgr[i].Update();
	if( m_pBlackRegion )
		m_pBlackRegion->Init();
}

void CMyLevel::Begin()
{
	m_bBegin = true;
}

void CMyLevel::End()
{
	m_bBegin = false;
	m_bEnd = true;
}

void CMyLevel::PlayerEnter( CPlayerCross* pPlayerCross, CCharacter* pControlled )
{
	m_pPlayer = pPlayerCross;
	if( m_pStartPoint )
	{
		m_pPlayer->SetParentBeforeEntity( m_pStartPoint );
		if( pControlled )
			pControlled->SetParentBeforeEntity( m_pStartPoint );
	}
	else
	{
		m_pPlayer->SetParentEntity( this );
		if( pControlled )
			pControlled->SetParentEntity( this );
	}
}

void CMyLevel::PlayerLeave()
{
	m_pPlayer->SetParentEntity( NULL );
	m_pPlayer = NULL;
}

void CMyLevel::OnAddCharacter( CCharacter * p )
{
	Insert_Character( p, p->GetUpdateGroup() );
	if( p->Get_HitProxy() )
		m_hitTestMgr[p->GetUpdateGroup()].Add( p );
}

void CMyLevel::OnRemoveCharacter( CCharacter * p )
{
	if( p->Get_HitProxy() )
		m_hitTestMgr[p->GetUpdateGroup()].Remove( p );
	Remove_Character( p, p->GetUpdateGroup() );
}

CVector2 CMyLevel::GetGravityDir()
{
	/*CVector2 dir( 0, -1 );
	CMatrix2D mat;
	mat.Transform( 0, 0, m_levelTrans.z, 1 );
	return mat.MulTVector2Dir( dir );*/
	return CVector2( 0, -1 );
}

bool CMyLevel::CheckOutOfBound( CEntity* p )
{
	SHitProxyCircle hitProxyDefault;
	SHitProxy* pHitProxy;
	if( p->Get_HitProxy() )
		pHitProxy = p->Get_HitProxy();
	else
	{
		hitProxyDefault.center = CVector2( 0, 0 );
		hitProxyDefault.fRadius = 10;
		pHitProxy = &hitProxyDefault;
	}

	SHitProxyPolygon hit0;
	CMatrix2D trans0;
	trans0.Identity();
	hit0.nVertices = 4;
	hit0.vertices[0] = CVector2( m_size.x, m_size.y );
	hit0.vertices[1] = CVector2( m_size.x + m_size.width, m_size.y );
	hit0.vertices[2] = CVector2( m_size.x + m_size.width, m_size.y + m_size.height );
	hit0.vertices[3] = CVector2( m_size.x, m_size.y + m_size.height );
	hit0.CalcNormals();
	if( !SHitProxy::HitTest( pHitProxy, &hit0, p->GetGlobalTransform(), trans0 ) )
		return true;
	return false;
}

void CMyLevel::ChangeToEnvLayer( CLevelEnvLayer* pEnv )
{
	m_pCurEnvLayer = pEnv;
}

float CMyLevel::GetDeltaTime()
{
	return GetStage()->GetElapsedTimePerTick() * GetDeltaTick() / T_SCL;
}

CEntity* CMyLevel::Pick( const CVector2& pos, int8 nGroup )
{
	SHitProxyCircle hitProxy;
	hitProxy.center = pos;
	hitProxy.fRadius = 0.01f;
	CMatrix2D transform;
	transform.Identity();
	vector<CHitProxy*> vecResults;
	m_hitTestMgr[nGroup].HitTest( &hitProxy, transform, vecResults );

	CEntity* pEntity = NULL;
	uint32 nMinTraverseOrder = -1;
	for( int i = 0; i < vecResults.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( vecResults[i] );
		if( pEntity1->GetTraverseIndex() < nMinTraverseOrder )
		{
			nMinTraverseOrder = pEntity->GetTraverseIndex();
			pEntity = pEntity1;
		}
	}
	return pEntity;
}

void CMyLevel::MultiPick( const CVector2& pos, vector<CReference<CEntity> >& result, int8 nGroup )
{
	SHitProxyCircle hitProxy;
	hitProxy.center = pos;
	hitProxy.fRadius = 0.01f;
	CMatrix2D transform;
	transform.Identity();
	vector<CHitProxy*> vecResults;
	m_hitTestMgr[nGroup].HitTest( &hitProxy, transform, vecResults );

	for( int i = 0; i < vecResults.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( vecResults[i] );
		result.push_back( pEntity );
	}
}

CEntity* CMyLevel::DoHitTest( SHitProxy* pProxy, const CMatrix2D& transform, bool hitTypeFilter[eEntityHitType_Count], SHitTestResult* pResult, int8 nGroup )
{
	vector<CHitProxy*> tempResult;
	m_hitTestMgr[nGroup].HitTest( pProxy, transform, tempResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i] );
		if( !hitTypeFilter[pEntity->GetHitType()] )
			continue;
		return pEntity;
	}
	return NULL;
}

void CMyLevel::MultiHitTest( SHitProxy* pProxy, const CMatrix2D& transform, vector<CReference<CEntity> >& result, vector<SHitTestResult>* pResult, int8 nGroup )
{
	vector<CHitProxy*> tempResult;
	m_hitTestMgr[nGroup].HitTest( pProxy, transform, tempResult, pResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i] );
		result.push_back( pEntity );
	}
}

CEntity* CMyLevel::Raycast( const CVector2& begin, const CVector2& end, EEntityHitType hitType, SRaycastResult* pResult, int8 nGroup )
{
	vector<SRaycastResult> result;
	m_hitTestMgr[nGroup].Raycast( begin, end, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( hitType != eEntityHitType_Count && pEntity->GetHitType() != hitType )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

void CMyLevel::MultiRaycast( const CVector2& begin, const CVector2& end, vector<CReference<CEntity> >& result, vector<SRaycastResult>* pResult, int8 nGroup )
{
	vector<SRaycastResult> tempResult;
	m_hitTestMgr[nGroup].Raycast( begin, end, tempResult );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i].pHitProxy );
		if( pResult )
			pResult->push_back( tempResult[i] );
		result.push_back( pEntity );
	}
}

CEntity* CMyLevel::SweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, EEntityHitType hitType, SRaycastResult * pResult, bool bIgnoreInverseNormal, int8 nGroup )
{
	vector<SRaycastResult> result;
	m_hitTestMgr[nGroup].SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( hitType != eEntityEvent_Count && pEntity->GetHitType() != hitType )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

CEntity* CMyLevel::SweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, EEntityHitType hitType, bool hitTypeFilter[eEntityHitType_Count], SRaycastResult * pResult, bool bIgnoreInverseNormal, int8 nGroup )
{
	vector<SRaycastResult> result;
	m_hitTestMgr[nGroup].SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, result );
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( result[i].pHitProxy );
		if( !hitTypeFilter[pEntity->GetHitType()] && !pEntity->GetHitChannnel()[hitType] )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( pResult )
			*pResult = result[i];
		return pEntity;
	}
	return NULL;
}

CEntity* CMyLevel::SweepTest( CEntity* pEntity, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, SRaycastResult * pResult, bool bIgnoreInverseNormal, int8 nGroup )
{
	SHitProxy* pHitProxy = pEntity->Get_HitProxy();
	if( !pHitProxy )
		return NULL;
	vector<SRaycastResult> result;
	m_hitTestMgr[nGroup].SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, result );
	CEntity *p1 = NULL, *p2 = NULL;
	SRaycastResult *r1 = NULL, *r2 = NULL;
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		pEntity1->AddRef();
	}
	auto hitTypeFilter = pEntity->GetHitChannnel();
	for( int i = 0; i < result.size(); i++ )
	{
		if( result[i].pHitProxy == pEntity )
			continue;
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		if( !hitTypeFilter[pEntity1->GetHitType()] && !pEntity1->GetHitChannnel()[pEntity->GetHitType()] )
			continue;
		if( bIgnoreInverseNormal && result[i].normal.Dot( sweepOfs ) >= 0 )
			continue;
		if( !pEntity1->CheckImpact( pEntity, result[i], false ) || !pEntity->CheckImpact( pEntity1, result[i], true ) )
			continue;

		if( !p1 )
		{
			p1 = pEntity1;
			r1 = &result[i];
		}
		if( !pResult->bThresholdFail )
		{
			p2 = pEntity1;
			r2 = &result[i];
			break;
		}
	}
	if( p1 && p2 && p1 != p2 )
	{
		if( r1->normal.Dot( sweepOfs ) > r2->normal.Dot( sweepOfs ) )
		{
			p2 = p1;
			r2 = r1;
		}
	}
	if( pResult && r2 )
		*pResult = *r2;
	for( int i = 0; i < result.size(); i++ )
	{
		CEntity* pEntity1 = static_cast<CEntity*>( result[i].pHitProxy );
		pEntity1->Release();
	}
	return p2;
}

void CMyLevel::MultiSweepTest( SHitProxy * pHitProxy, const CMatrix2D & trans, const CVector2 & sweepOfs, float fSideThreshold, vector<CReference<CEntity>>& result, vector<SRaycastResult>* pResult, int8 nGroup )
{
	vector<SRaycastResult> tempResult;
	m_hitTestMgr[nGroup].SweepTest( pHitProxy, trans, sweepOfs, fSideThreshold, tempResult, true );
	for( int i = 0; i < tempResult.size(); i++ )
	{
		CEntity* pEntity = static_cast<CEntity*>( tempResult[i].pHitProxy );
		if( pResult )
			pResult->push_back( tempResult[i] );
		result.push_back( pEntity );
	}
}

bool CMyLevel::CheckTeleport( CCharacter* pPlayer, const CVector2& transferOfs )
{
	if( !m_size.Contains( pPlayer->GetPosition() - transferOfs ) )
		return false;
	/*auto pPlayerHits = pPlayer->GetAllHits();
	static vector<CHitProxy*> vecResult;
	for( int i = 0; i < 3; i++ )
	{
		auto mat = pPlayerHits[i]->GetGlobalTransform();
		mat.SetPosition( mat.GetPosition() - transferOfs );
		vecResult.resize( 0 );
		GetHitTestMgr().HitTest( pPlayerHits[i]->Get_HitProxy(), mat, vecResult );
		for( auto p : vecResult )
		{
			auto pEntity = static_cast<CEntity*>( p );
			if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
				return false;
		}
	}*/
	static vector<CHitProxy*> vecResult;
	vecResult.resize( 0 );
	auto mat = pPlayer->GetGlobalTransform();
	GetHitTestMgr( pPlayer->GetUpdateGroup() ).HitTest( pPlayer->Get_HitProxy(), mat, vecResult );
	for( auto p : vecResult )
	{
		auto pEntity = static_cast<CEntity*>( p );
		if( pEntity->GetHitType() == eEntityHitType_WorldStatic )
			return false;
	}
	return true;
}

float CMyLevel::Push( CCharacter* pCharacter, const CVector2& dir, float fDist )
{
	CCharacter::SPush context;
	CEntity* pTested = pCharacter;
	auto mat = pCharacter->GetGlobalTransform();
	Push( pCharacter, context, dir, fDist, 1, &pTested, &mat, MOVE_SIDE_THRESHOLD );
	for( int i = 1; i < context.vecChars.size(); i++ )
		context.vecChars[i].fMoveDist = 0;

	for( int i = 0; i < context.vecItems.size(); i++ )
	{
		auto& item = context.vecItems[i];
		auto& pushed = context.vecChars[item.nPushed];
		auto& par = context.vecChars[item.nPar];
		pushed.nDeg++;
		item.nNxtEdge = par.nFirstEdge;
		par.nFirstEdge = i;
	}

	vector<int32> q;
	q.push_back( 0 );
	for( int i = 0; i < q.size(); i++ )
	{
		auto& par = context.vecChars[q[i]];
		for( int iEdge = par.nFirstEdge; iEdge >= 0; )
		{
			auto& item = context.vecItems[iEdge];
			auto& pushed = context.vecChars[item.nPushed];
			pushed.fMoveDist = Max( pushed.fMoveDist, par.fMoveDist - item.fDist0 );
			pushed.nDeg--;
			if( !pushed.nDeg )
				q.push_back( item.nPushed );
			iEdge = item.nNxtEdge;
		}
	}

	for( int i = 0; i < context.vecChars.size(); i++ )
	{
		auto& c = context.vecChars[i];
		c.pChar->HandlePush( dir, c.fMoveDist, 0 );
	}
	for( int i = 0; i < context.vecChars.size(); i++ )
	{
		auto& c = context.vecChars[i];
		c.pChar->HandlePush( dir, c.fMoveDist, 1 );
	}
	for( int i = 0; i < context.vecChars.size(); i++ )
	{
		auto& c = context.vecChars[i];
		c.pChar->HandlePush( dir, c.fMoveDist, 2 );
	}
	for( int i = 0; i < context.vecChars.size(); i++ )
	{
		auto& c = context.vecChars[i];
		c.pChar->nPublicFlag = 0;
	}
	return context.vecChars[0].fMoveDist;
}

float CMyLevel::Push( CCharacter* pCharacter, CCharacter::SPush& context, const CVector2& dir, float fDist, int32 nTested, CEntity** pTested, CMatrix2D* matTested, float fSideThreshold )
{
	auto nChar = pCharacter->nPublicFlag - 1;
	if( nChar < 0 )
	{
		nChar = context.vecChars.size();
		pCharacter->nPublicFlag = nChar + 1;
		CCharacter::SPush::SChar char0 = { pCharacter, 0, 0, -1 };
		context.vecChars.push_back( char0 );
	}
	else
		return context.vecChars[nChar].fMoveDist;
	auto& vecPush = context.vecItems;
	float fMoveDist = fDist;

	auto sweepOfs = dir * fDist;
	vector<SRaycastResult> tempResult;
	for( int i = 0; i < nTested; i++ )
	{
		int32 n0 = tempResult.size();
		auto bHitChannel = pTested[i]->GetHitChannnel();
		GetHitTestMgr( pCharacter->GetUpdateGroup() ).SweepTest( pTested[i]->Get_HitProxy(), matTested[i], sweepOfs, fSideThreshold, tempResult, false );
		for( int i1 = tempResult.size() - 1; i1 >= n0; i1-- )
		{
			auto pOtherEntity = static_cast<CEntity*>( tempResult[i1].pHitProxy );
			bool bHit = bHitChannel[pOtherEntity->GetHitType()] || pOtherEntity->GetHitChannnel()[pTested[i]->GetHitType()];
			bool bPlatform = !bHit && pOtherEntity->GetPlatformChannel()[pTested[i]->GetHitType()];

			tempResult[i1].nUser = bPlatform;
			bool bOK = bHit || bPlatform;
			if( bOK && pTested[i]->HasHitFilter() )
			{
				if( !pOtherEntity->CheckImpact( pTested[i], tempResult[i1], false ) || !pTested[i]->CheckImpact( pOtherEntity, tempResult[i1], true ) )
					bOK = false;
			}
			if( bOK && tempResult[i1].normal.Dot( sweepOfs ) >= 0 )
				bOK = false;
			if( !bOK )
			{
				tempResult[i1] = tempResult.back();
				tempResult.resize( tempResult.size() - 1 );
			}
		}
	}

	std::sort( tempResult.begin(), tempResult.end(), [] ( const SRaycastResult& lhs, const SRaycastResult& rhs )
	{
		if( lhs.fDist < rhs.fDist )
			return true;
		if( lhs.fDist > rhs.fDist )
			return false;
		if( lhs.nUser < rhs.nUser )
			return true;
		if( lhs.nUser > rhs.nUser )
			return false;
		return lhs.pHitProxy < rhs.pHitProxy;
	} );

	for( auto& item : tempResult )
	{
		auto pOtherEntity = static_cast<CEntity*>( item.pHitProxy );
		auto pOwner = GetEntityCharacterRootInLevel( pOtherEntity );
		if( !pOwner )
			continue;
		float fMoveDist1 = fDist;
		int32 n = pOwner->CheckPush( item, dir, fMoveDist1, context, nChar );
		if( n < 0 )
			continue;
		if( n > 0 )
		{
			CCharacter::SPush::SHit pushItem;
			pushItem.nPushed = pOwner->nPublicFlag - 1;
			pushItem.nPar = nChar;
			pushItem.fDist0 = 0;
			pushItem.nNxtEdge = -1;

			auto fDist0 = item.fDist;
			pushItem.fDist0 = item.fDist;
			auto fPushDist = fDist - item.fDist;
			fMoveDist1 = fDist + Min( fPushDist, fMoveDist1 ) - fPushDist;
			fMoveDist = Min( fMoveDist, fMoveDist1 );

			vecPush.push_back( pushItem );
		}
		else
		{
			fMoveDist = 0;
			break;
		}
	}
	context.vecChars[nChar].fMoveDist = fMoveDist;
	return fMoveDist;
}

CCharacter * CMyLevel::FindAttach()
{
	static vector<CHitProxy*> vecResult;
	vecResult.resize( 0 );
	SHitProxyCircle hitProxy;
	hitProxy.center = CVector2( 0, 0 );
	hitProxy.fRadius = GetPlayerPickRad();
	auto mat = m_pPlayer->GetGlobalTransform();
	GetHitTestMgr( 0 ).HitTest( &hitProxy, mat, vecResult );
	float fDist = FLT_MAX;
	CCharacter* pResult = NULL;

	for( auto p : vecResult )
	{
		auto pEntity = static_cast<CEntity*>( p );

		auto pCharacter = SafeCast<CCharacter>( pEntity );
		if( !pCharacter || !pCharacter->CanBeControlled() )
			continue;
		float f = pCharacter->CheckControl( m_pPlayer->GetPosition() );
		if( f < fDist )
		{
			fDist = f;
			pResult = pCharacter;
		}
	}
	return pResult;
}

CCharacter* CMyLevel::TryAttach( CCharacter* pChar0 )
{
	CCharacter* pResult = pChar0;
	if( pResult )
	{
		if( pResult->GetLevel() != this || !pResult->CanBeControlled() )
			pResult = NULL;
	}
	if( !pResult )
		pResult = FindAttach();
	if( pResult )
	{
		auto d = m_pPlayer->GetPosition() - pResult->GetPosition();
		if( d.Length2() <= 1 )
		{
			m_pControlled = pResult;
			pResult->BeginControl();
			m_pPlayer->OnAttachBegin();
			if( m_pBlackRegion )
				m_pBlackRegion->OnPlayerAttach( m_pPlayer );
			CMasterLevel::GetInst()->OnAttached();
			return NULL;
		}
	}
	return pResult;
}

void CMyLevel::TryDetach()
{
	CMasterLevel::GetInst()->OnDetached();
	if( m_pBlackRegion )
		m_pBlackRegion->OnPlayerDetach( m_pPlayer );
	m_pPlayer->OnAttachEnd();
	m_pControlled->EndControl();
	m_pControlled = NULL;
}

void CMyLevel::OnBugDetected( CBug* pBug )
{
	auto& item = m_vecBugListItems[pBug->m_nBugID - 1];
	item.bDetected = true;
	GetStage()->GetWorld()->GetWorldData().DetectBug( GetInstanceOwner()->GetName(), pBug->GetName() );
	if( m_pBugIndicatorLayer )
		m_pBugIndicatorLayer->Update();
}

void CMyLevel::ResetBug( CBug* pBug )
{
	auto& item = m_vecBugListItems[pBug->m_nBugID - 1];
	auto pRoot = CMyLevel::GetEntityCharacterRootInLevel( pBug, true );
	auto pNew = RecreateEntity( pRoot );
	ScanBug( pNew );
}

void CMyLevel::CheckBugs( bool bTest )
{
	for( auto& group : m_mapBugListGroupRange )
	{
		if( group.second.x < 0 )
			continue;

		if( bTest )
		{
			bool bCleared = true;
			for( int i = group.second.x; i < group.second.y; i++ )
			{
				auto& item = m_vecBugListItems[i];
				if( !item.pBug->IsFixed() && !item.pBug->IsCaught() )
				{
					bCleared = false;
					break;
				}
			}
			if( bCleared )
			{
				int32 nExp = 0;
				auto& worldData = GetStage()->GetWorld()->GetWorldData();
				for( int i = group.second.x; i < group.second.y; i++ )
				{
					auto& item = m_vecBugListItems[i];
					worldData.FixBug( GetInstanceOwner()->GetName(), item.pBug->GetName(), item.pBug->GetExp() );
					nExp += item.pBug->GetExp();
					item.pBug->Clear( true );
					item.pBug = NULL;
				}
				GetStage()->GetWorld()->SaveWorldData();
				group.second.x = group.second.y = -1;
				if( m_pBugIndicatorLayer )
					m_pBugIndicatorLayer->Update();
			}
		}
		else
		{
			for( int i = group.second.x; i < group.second.y; i++ )
			{
				auto& item = m_vecBugListItems[i];
				ResetBug( item.pBug );
			}
		}
	}
}

void CMyLevel::EditorFixBugListLoad( vector<SEditorBugListItem>& vecAllBugs )
{
	vector<int32> vec;
	auto _Less = [] ( const string& l, const string& r )
	{
		if( l.length() < r.length() )
			return true;
		if( r.length() < l.length() )
			return false;
		return l < r;
	};
	set<string, decltype( _Less )> setBugNames( _Less );
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		CPrefabNode* pNode = vecAllBugs[i].p;
		CBug* pData = (CBug*)pNode->GetStaticDataSafe<CBug>();
		if( pNode->GetPatchedNodeOwner()->GetName().length() )
		{
			if( setBugNames.find( pNode->GetPatchedNodeOwner()->GetName().c_str() ) != setBugNames.end() )
				pNode->GetPatchedNodeOwner()->SetName( "" );
			else
				setBugNames.insert( pNode->GetPatchedNodeOwner()->GetName().c_str() );
		}

		auto nID = pData->m_nBugID - 1;
		if( nID >= 0 )
		{
			vec.resize( Max<int32>( vec.size(), nID + 1 ), -1 );
			if( vec[nID] >= 0 )
				pData->m_nBugID = 0;
			else
				vec[nID] = i;
		}
	}
	string str;
	if( setBugNames.size() )
		str = *setBugNames.rbegin();
	else
		str = "0";
	int32 k;
	int32 n = 0;
	int32 n1 = 1;
	for( k = str.length(); k > 0; k-- )
	{
		if( str[k - 1] < '0' && str[k - 1] > '9' )
			break;
		n += ( str[k - 1] - '0' ) * n1;
		n1 *= 10;
	}
	str = str.substr( 0, k );

	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		CPrefabNode* pNode = vecAllBugs[i].p;
		if( !pNode->GetPatchedNodeOwner()->GetName().length() )
		{
			n++;
			char buf[100];
			sprintf( buf, "%s%d", str.c_str(), n );
			pNode->GetPatchedNodeOwner()->SetName( buf );
		}
	}

	for( int i = m_arrBugLink.Size() - 1; i >= 0; i-- )
	{
		auto& item = m_arrBugLink[i];
		bool bOK = false;
		if( item.a < vec.size() && item.b < vec.size() )
		{
			auto a = vec[item.a];
			auto b = vec[item.b];
			if( a >= 0 && b >= 0 )
			{
				bOK = true;
				vecAllBugs[b].par = vecAllBugs[a].p;
				vecAllBugs[b].vecPath.resize( item.arrPath.Size() );
				if( item.arrPath.Size() )
					memcpy( &vecAllBugs[b].vecPath[0], &item.arrPath[0], sizeof( int32 ) * item.arrPath.Size() );
			}
		}
		if( !bOK )
		{
			m_arrBugLink[i] = m_arrBugLink[m_arrBugLink.Size() - 1];
			m_arrBugLink.Resize( m_arrBugLink.Size() - 1 );
		}
	}
}

void CMyLevel::EditorFixBugListSave( vector<SEditorBugListItem>& vecAllBugs )
{
	struct SNode
	{
		SNode() : pNode( NULL ), pData( NULL ), nRoot( -1 ), nParent( -1 ), nGroup( -1 ), nNewID( -1 ) {}
		CPrefabNode* pNode;
		CBug* pData;
		int32 nRoot;
		int32 nGroup;
		int32 nParent;
		int32 nNewID;
	};
	vector<SNode> vecNode;
	vecNode.resize( vecAllBugs.size() );
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		item.pNode = vecAllBugs[i].p;
		item.pData = (CBug*)item.pNode->GetStaticDataSafe<CBug>();
		item.pData->m_nBugID = i + 1;
		item.nRoot = i;
	}
	auto GetRoot = [&vecNode] ( int32 i )
	{
		auto nCur = i;
		auto nRoot = vecNode[nCur].nRoot;
		for( ; nRoot != nCur; nCur = nRoot, nRoot = vecNode[nCur].nRoot );
		for( nCur = i; nCur != nRoot; )
		{
			auto& n1 = vecNode[nCur].nRoot;
			nCur = n1;
			n1 = nRoot;
		}
		return nRoot;
	};
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto pNode1 = vecAllBugs[i].par;
		if( pNode1 )
		{
			auto& item = vecNode[i];
			auto pData1 = (CBug*)pNode1->GetStaticDataSafe<CBug>();
			auto nParentID = pData1->m_nBugID - 1;
			auto nNewRoot = GetRoot( nParentID );
			if( nNewRoot == item.nRoot )
			{
				vecAllBugs[i].par = NULL;
				continue;
			}
			item.nRoot = nNewRoot;
			item.nParent = nParentID;
		}
	}

	map<int32, int32> mapGroupRoot;
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		auto nRoot = GetRoot( i );
		if( nRoot == i )
		{
			auto nGroup = item.pData->m_nGroup;
			auto itr = mapGroupRoot.find( nGroup );
			if( itr != mapGroupRoot.end() )
			{
				nGroup = mapGroupRoot.rbegin()->first + 1;
				item.pData->m_nGroup = nGroup;
			}
			mapGroupRoot[nGroup] = i;
		}
	}
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		auto nRoot = GetRoot( i );
		auto nGroup = vecNode[nRoot].pData->m_nGroup;
		item.nGroup = nGroup;
		item.pData->m_nGroup = nGroup;
	}

	vector<int32> vecIndex;
	vecIndex.resize( vecNode.size() );
	for( int i = 0; i < vecIndex.size(); i++ )
		vecIndex[i] = i;
	std::sort( vecIndex.begin(), vecIndex.end(), [&vecNode] ( int32 a, int32 b ) {
		auto& item1 = vecNode[a];
		auto& item2 = vecNode[b];
		if( item1.nGroup < item2.nGroup )
			return true;
		if( item1.nGroup > item2.nGroup )
			return false;
		return a < b;
	} );
	for( int i = 0; i < vecIndex.size(); i++ )
	{
		vecNode[vecIndex[i]].nNewID = i;
		vecNode[vecIndex[i]].pData->m_nBugID = i + 1;
	}

	m_arrBugLink.Resize( 0 );
	for( int i = 0; i < vecAllBugs.size(); i++ )
	{
		auto& item = vecNode[i];
		if( item.nParent >= 0 )
		{
			m_arrBugLink.Resize( m_arrBugLink.Size() + 1 );
			auto& l = m_arrBugLink[m_arrBugLink.Size() - 1];
			l.a = vecNode[item.nParent].nNewID;
			l.b = item.nNewID;
			auto& vecPath = vecAllBugs[i].vecPath;
			l.arrPath.Resize( vecPath.size() );
			for( int n = 0; n < vecPath.size(); n++ )
				l.arrPath[n] = vecPath[n];
		}
	}
}

void CMyLevel::Update()
{
	if( !IsBegin() )
		return;
	if( m_pPlayer && m_pPlayer->IsKilled() )
		return;

	int32 nDeltaTick[2];
	if( m_pControlled )
	{
		nDeltaTick[0] = GetSlowMotionScale() - m_nTick0;
		m_nTick0 = 0;
		m_nUpdateType = 0;
	}
	else
	{
		nDeltaTick[0] = 1;
		m_nTick0++;
		if( m_nTick0 >= GetSlowMotionScale() )
			m_nTick0 = 0;
		m_nUpdateType = 1;
	}
	nDeltaTick[1] = GetSlowMotionScale();

	for( m_nUpdatePhase = eStageUpdatePhase_BeforeHitTest; m_nUpdatePhase <= eStageUpdatePhase_PostUpdate; m_nUpdatePhase++ )
	{
		if( m_nUpdatePhase == eStageUpdatePhase_PostUpdate )
		{
			if( m_pBlackRegion )
				m_pBlackRegion->PostUpdate( m_pPlayer, m_pControlled != NULL );
		}
		for( int k = 0; k < 2; k++ )
		{
			m_nDeltaTick = nDeltaTick[k];
			CCharacter* pPlayer = k == 0 ? m_pControlled : (CCharacter*)m_pPlayer.GetPtr();

			if( m_nUpdatePhase == eStageUpdatePhase_BeforeHitTest )
			{
				if( pPlayer )
				{
					pPlayer->OnTickBeforeHitTest();
					pPlayer->SetUpdatePhase( 1 );
					if( pPlayer->GetStage() )
						pPlayer->Trigger( 0 );
				}
				LINK_LIST_FOR_EACH_BEGIN( pCharacter, m_pCharacters[k], CCharacter, Character )
				{
					if( pCharacter == pPlayer )
						continue;
					if( pCharacter->IsKilled() )
						continue;
					if( m_nUpdateType == 0 && pCharacter->IsKillOnPlayerAttach() )
					{
						pCharacter->Kill();
						continue;
					}
					DEFINE_TEMP_REF( pCharacter );
					pCharacter->OnTickBeforeHitTest();
					pCharacter->SetUpdatePhase( 1 );
					if( pCharacter->GetStage() )
						pCharacter->Trigger( 0 );
				}
				LINK_LIST_FOR_EACH_END( pCharacter, m_pCharacters[k], CCharacter, Character );
			}
			else if( m_nUpdatePhase == eStageUpdatePhase_HitTest )
			{
				m_hitTestMgr[k].Update( GetDeltaTime() );
			}
			else if( m_nUpdatePhase == eStageUpdatePhase_AfterHitTest )
			{
				if( pPlayer )
				{
					pPlayer->OnTickAfterHitTest();
					pPlayer->SetUpdatePhase( 2 );
					if( pPlayer->GetStage() )
						pPlayer->Trigger( 1 );
				}
				LINK_LIST_FOR_EACH_BEGIN( pCharacter, m_pCharacters[k], CCharacter, Character )
				{
					if( pCharacter == pPlayer )
						continue;
					if( pCharacter->IsKilled() )
						continue;
					DEFINE_TEMP_REF( pCharacter );
					pCharacter->OnTickAfterHitTest();
					pCharacter->SetUpdatePhase( 2 );
					if( pCharacter->GetStage() )
						pCharacter->Trigger( 1 );
				}
				LINK_LIST_FOR_EACH_END( pCharacter, m_pCharacters[k], CCharacter, Character );
			}
			else
			{
				for( auto pCharacter = m_pCharacters[k]; pCharacter; )
				{
					auto pNxt = pCharacter->NextCharacter();
					if( pCharacter != pPlayer )
					{
						if( pCharacter->IsKilled() )
							pCharacter->SetParentEntity( NULL );
						else
							pCharacter->PostUpdate();
					}
					pCharacter = pNxt;
				}
				if( pPlayer )
					pPlayer->PostUpdate();
			}
		}
	}

	for( CEntity* p : m_vecAllLayers )
		SafeCastToInterface<ILevelObjLayer>( p )->Update();
}

void CMyLevel::Update1()
{
	auto camTrans = CMasterLevel::GetInst()->GetCamTrans();
	CVector2 camPos( camTrans.x - x, camTrans.y - y );
	camTrans.x -= x;
	camTrans.y -= y;
	for( CEntity* p : m_vecAllLayers )
		SafeCastToInterface<ILevelObjLayer>( p )->UpdateScroll( camTrans );
	if( m_pBlackRegion )
	{
		CMatrix2D mat = m_pBlackRegion->GetGlobalTransform();
		mat = mat.Inverse();
		CRectangle localRect = GetStage()->GetCamRectBound( camTrans ) * mat;
		m_pBlackRegion->UpdateImages( localRect );
	}
}

CMyLevel* CMyLevel::GetEntityLevel( CEntity* pEntity )
{
	auto pParent = pEntity->GetParentEntity();
	CMyLevel* pLevel = NULL;
	while( pParent )
	{
		pLevel = SafeCast<CMyLevel>( pParent );
		if( pLevel )
			return pLevel;
		pParent = pParent->GetParentEntity();
	}
	return NULL;
}

CEntity* CMyLevel::GetEntityRootInLevel( CEntity* pEntity )
{
	auto p = pEntity;
	while( p )
	{
		auto pParent = p->GetParentEntity();
		if( SafeCast<CMyLevel>( pParent ) )
			return p;
		p = pParent;
	}
	return NULL;
}

CCharacter* CMyLevel::GetEntityCharacterRootInLevel( CEntity* pEntity, bool bFindResetable )
{
	auto pParent = pEntity->GetParentEntity();
	CMyLevel* pLevel = NULL;
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	while( pParent )
	{
		pLevel = SafeCast<CMyLevel>( pParent );
		if( pLevel )
			break;
		auto pCharacter1 = SafeCast<CCharacter>( pParent );
		if( pCharacter1 && ( !bFindResetable || pCharacter1->IsResetable() ) )
			pCharacter = pCharacter1;
		pParent = pParent->GetParentEntity();
	}
	return pCharacter;
}

void CMyLevel::DisableLights( CRenderObject2D* p )
{
	if( SafeCast<CDirectionalLightObject>( p ) || SafeCast<CPointLightObject>( p ) )
		p->bVisible = false;
	for( auto pChild = p->Get_TransformChild(); pChild; pChild = pChild->NextRenderChild() )
		DisableLights( pChild );
}

void CMyLevel::BuildBugList()
{
	if( m_bBugListReady )
		return;
	m_bBugListReady = true;
	ScanBug( this );
	for( int i = 0; i < m_arrBugLink.Size(); i++ )
	{
		auto& item = m_arrBugLink[i];
		m_vecBugListItems[item.b].nParent = item.a;
	}
	for( int i = 0; i < m_vecBugListItems.size(); i++ )
	{
		auto& item = m_vecBugListItems[i];
		if( item.pBug )
		{
			item.pBug->ForceUpdateTransform();
			item.origPos = globalTransform.MulTVector2Pos( item.pBug->globalTransform.GetPosition() );
			if( item.nParent >= 0 )
			{
				auto& parItem = m_vecBugListItems[item.nParent];
				item.nNxtSib = parItem.nFirstChild;
				parItem.nFirstChild = i;
			}
		}
	}
	for( int i = 0; i < m_vecBugListItems.size(); i++ )
	{
		auto nGroup = m_vecBugListItems[i].nGroup;
		if( nGroup < 0 )
			continue;
		auto itr = m_mapBugListGroupRange.find( nGroup );
		if( itr == m_mapBugListGroupRange.end() )
			m_mapBugListGroupRange[nGroup] = TVector2<int32>( i, i + 1 );
		else
			itr->second.y = i + 1;
	}
}

void CMyLevel::ScanBug( CEntity* p )
{
	auto pBug = SafeCast<CBug>( p );
	if( pBug )
	{
		m_vecBugListItems.resize( Max<int32>( m_vecBugListItems.size(), pBug->m_nBugID - 1 + 1 ) );
		auto& item = m_vecBugListItems[pBug->m_nBugID - 1];
		item.pBug = pBug;
		item.nGroup = pBug->m_nGroup;
		auto pWorld = CMasterLevel::GetInst()->GetStage()->GetWorld();
		auto nState = pWorld->GetWorldData().GetBugState( GetInstanceOwner()->GetName(), pBug->GetName() );
		if( nState == 2 )
		{
			item.pBug->Clear( false );
			item.pBug = NULL;
			item.nGroup = -1;
			return;
		}
		else if( nState == 1 )
			item.bDetected = true;
		pBug->bVisible = pBug->m_bDetected = item.bDetected;
		return;
	}
	for( auto pChild = p->Get_ChildEntity(); pChild; )
	{
		auto p1 = pChild->NextChildEntity();
		ScanBug( pChild );
		pChild = p1;
	}
}

CMasterLevel* CMasterLevel::s_pInst = NULL;
void CMasterLevel::OnAddedToStage()
{
	s_pInst = this;
	m_pMask1->bVisible = false;
	m_pTestUI->bVisible = false;
	m_pTestPlayerCross->bVisible = false;
	NewGame();
}

void CMasterLevel::OnRemovedFromStage()
{
	if( s_pInst == this )
		s_pInst = NULL;
}

void CMasterLevel::NewGame()
{
	GetStage()->GetWorld()->GetWorldData().NewDay();
	m_nKillTickLeft = m_nKillTick;

	m_pCurLevelPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strBeginLevel );
	m_pPlayer = SafeCast<CPlayerCross>( m_pPlayerPrefab->GetRoot()->CreateInstance() );
	int32 nExp = GetStage()->GetWorld()->GetWorldData().nPlayerExp;
	//m_pPlayer->SetLevel( CGlobalCfg::Inst().GetLevelByExp( nExp ) );
	CreateCurLevel();
	auto pCurEnv = CurEnv();
	auto p = m_pCurLevel->GetStartPoint()->GetPosition();
	m_camTrans.x = p.x;
	m_camTrans.y = p.y;
	m_pCurEnvLayer = pCurEnv;
	m_pCurEnvLayer->InitCtrlPointsState( m_camTrans.x, m_camTrans.y, m_camTrans.z, m_camTrans.w );
	m_pPlayer->SetPosition( p );
	m_pCurLevel->PlayerEnter( m_pPlayer, NULL );
	BufferLevels();
	BeginCurLevel();
}

void CMasterLevel::TransferTo( const char* szNewLevel, int8 nTransferType, int32 nTransferParam )
{
	auto pLevelData0 = GetStage()->GetWorld()->GetWorldCfg().GetLevelData( m_pCurLevelPrefab->GetName() );
	auto pLevelData = GetStage()->GetWorld()->GetWorldCfg().GetLevelData( szNewLevel );
	m_pTransferTo = CResourceManager::Inst()->CreateResource<CPrefab>( szNewLevel );
	m_transferOfs = pLevelData->displayOfs - pLevelData0->displayOfs;
	m_nTransferType = nTransferType;
	m_nTransferParam = nTransferParam;

	struct _STemp
	{
		static uint32 Func( void* pThis )
		{
			( (CMasterLevel*)pThis )->TransferFunc();
			return 1;
		}
	};
	m_pTransferCoroutine = TCoroutinePool<0x10000>::Inst().Alloc();
	m_pTransferCoroutine->Create( &_STemp::Func, this );
	m_pTransferCoroutine->Resume();
	if( m_pTransferCoroutine->GetState() == ICoroutine::eState_Stopped )
	{
		TCoroutinePool<0x10000>::Inst().Free( m_pTransferCoroutine );
		m_pTransferCoroutine = NULL;
	}
}

void CMasterLevel::OpenTestConsole()
{
	struct _STemp
	{
		static uint32 Func( void* pThis )
		{
			( (CMasterLevel*)pThis )->TestConsoleFunc();
			return 1;
		}
	};
	m_pTestConsoleCoroutine = TCoroutinePool<0x10000>::Inst().Alloc();
	m_pTestConsoleCoroutine->Create( &_STemp::Func, this );
	m_pTestConsoleCoroutine->Resume();
	if( m_pTestConsoleCoroutine->GetState() == ICoroutine::eState_Stopped )
	{
		TCoroutinePool<0x10000>::Inst().Free( m_pTestConsoleCoroutine );
		m_pTestConsoleCoroutine = NULL;
	}
}

CMyLevel* CMasterLevel::CreateLevel( CPrefab* pPrefab )
{
	return SafeCast<CMyLevel>( pPrefab->GetRoot()->CreateInstance() );
}

void CMasterLevel::CreateCurLevel()
{
	auto itr = m_mapBufferedLevels.find( m_pCurLevelPrefab->GetName() );
	if( itr != m_mapBufferedLevels.end() )
	{
		m_pCurLevel = itr->second;
		m_mapBufferedLevels.erase( m_pCurLevelPrefab->GetName() );
		m_mapBufferedLevelPrefabs.erase( m_pCurLevelPrefab->GetName() );
	}
	else
		m_pCurLevel = CreateLevel( m_pCurLevelPrefab );
	if( m_pLastLevel )
		m_pCurLevel->SetParentAfterEntity( m_pLevelFadeMask );
	else
		m_pCurLevel->SetParentBeforeEntity( m_pLevelFadeMask );
	m_pCurLevel->Init();
}

void CMasterLevel::BeginCurLevel()
{
	m_pCurLevel->Begin();
}

void CMasterLevel::EndCurLevel()
{
	m_pCurLevel->End();
	m_pLastLevel = m_pCurLevel;
	m_pLastLevelPrefab = m_pCurLevelPrefab;
	m_pCurLevel = NULL;
	m_pCurLevelPrefab = NULL;
}

void CMasterLevel::Update()
{
	if( m_pPlayer->IsKilled() && !m_nKillTickLeft )
	{
		Kill();
		return;
	}

	m_maskParams[0] = CVector4( 0, 0, 0, 0 );
	m_maskParams[1] = CVector4( 0, 0, 0, 0 );
	m_maskParams[2] = CVector4( 0, 0, 0, 0 );
	if( m_pTransferCoroutine )
	{
		m_pTransferCoroutine->Resume();
		if( m_pTransferCoroutine->GetState() == ICoroutine::eState_Stopped )
		{
			TCoroutinePool<0x10000>::Inst().Free( m_pTransferCoroutine );
			m_pTransferCoroutine = NULL;
		}
	}

	if( !m_pTransferCoroutine )
	{
		if( m_pTestConsoleCoroutine )
		{
			m_pTestConsoleCoroutine->Resume();
			if( m_pTestConsoleCoroutine->GetState() == ICoroutine::eState_Stopped )
			{
				TCoroutinePool<0x10000>::Inst().Free( m_pTestConsoleCoroutine );
				m_pTestConsoleCoroutine = NULL;
			}
		}
		else if( !m_pPlayer->IsKilled() && CGame::Inst().IsKey( VK_RETURN ) )
		{
			CGame::Inst().ForceKeyRelease( VK_RETURN );
			if( m_nTestState )
				EndTest();
			else
				OpenTestConsole();
		}
	}

	if( !m_pTransferCoroutine && !m_pTestConsoleCoroutine )
	{
		if( m_pCurLevel )
		{
			m_pCurLevel->Update();
			if( GetTestState() )
				m_pCurLevel->CheckBugs( true );
		}
		if( m_pPlayer )
		{
			/*if( m_pCurLevel && m_pCurLevel->IsBegin() )
				m_pPlayer->PostUpdate();
*/
			UpdateTestMasks( m_nTestState, m_nTestDir, m_testOrig, 1 );
		}

		UpdateCtrlPoints( 1.0f );
		auto camTrans = GetCamTrans();
		auto pLevelData = GetStage()->GetWorld()->GetWorldCfg().GetLevelData( m_pCurLevelPrefab->GetName() );
		CVector2 ofs = CVector2( camTrans.x, camTrans.y ) * ( CVector2( 1, 1 ) - m_backOfsScale ) - pLevelData->displayOfs * m_backOfsScale;
		m_pBack->SetPosition( ofs );
		m_pLevelFadeMask->SetPosition( ofs );

		if( m_pPlayer->IsKilled() )
		{
			if( m_nKillTickLeft )
				m_nKillTickLeft--;
			m_bTryTeleport = false;
			if( m_nTestState )
				EndTest();
			if( m_bAlert )
				EndAlert();
		}
		else if( m_bTryTeleport )
		{
			const char* sz = CheckTeleport( m_bTeleportUp );
			if( sz )
				TransferTo( sz, 0, 0 );
			m_bTryTeleport = false;
		}
	}
	else
		UpdateCtrlPoints( 1.0f );
	if( m_pLastLevel )
		m_pLastLevel->Update1();
	if( m_pCurLevel )
		m_pCurLevel->Update1();
	auto pParam = static_cast<CImage2D*>( m_pLayer1.GetPtr() )->GetParam();
	auto camTrans = GetCamTrans();
	m_pLayer1->SetPosition( CVector2( camTrans.x, camTrans.y ) );
	pParam[0] = CVector4( 1, 0, 0, 0 ) + m_maskParams[0];
	pParam[1] = CVector4( 0, 1, 0, 0 ) + m_maskParams[1];
	pParam[2] = CVector4( 0, 0, 1, 0 ) + m_maskParams[2];
	for( int i = 0; i < 3; i++ )
		pParam[i].w = pow( 2, -pParam[i].w );
}

void CMasterLevel::Kill()
{
	EndCurLevel();
	if( m_pLastLevel )
	{
		m_pLastLevel->SetParentEntity( NULL );
		m_pLastLevel = NULL;
	}
	for( auto& item : m_mapBufferedLevels )
		item.second->SetParentEntity( NULL );
	m_mapBufferedLevels.clear();

	SStageEnterContext context;
	GetStage()->GetWorld()->EnterStage( "1.pf", context );
}

CVector2 CMasterLevel::GetCtrlPointFollowPlayerPos( const CVector2 & p )
{
	return p + m_pPlayer->GetPosition() + m_camTransPlayerOfs;
}

CVector4 CMasterLevel::GetCamTrans()
{
	auto trans = m_camTrans;
	// temp solution
	{
		trans.x = floor( trans.x / 1 + 0.5f ) * 1;
		trans.y = floor( trans.y / 1 + 0.5f ) * 1;
		trans.z = 0;
		trans.w = 1;
	}
	return trans;
}

int32 CMasterLevel::GetTestState()
{
	return m_nTestState;
}

CRectangle CMasterLevel::GetTestRect()
{
	return GetTestRect( m_nTestState, m_nTestDir, m_testOrig );
}

CRectangle CMasterLevel::GetTestRect( int32 nState, int8 nDir, const CVector2& orig )
{
	if( nState == eTest_Static )
		return m_testRect[0].Offset( orig );
	else if( nState >= eTest_Scan_0 )
	{
		auto rect = m_testRect[nState - 1];
		if( nDir == 2 || nDir == 3 )
			rect.x = -rect.GetRight();
		if( nDir == 1 || nDir == 3 )
			rect = CRectangle( rect.y, rect.x, rect.height, rect.width );
		return rect.Offset( orig );
	}
	return CRectangle( 0, 0, 0, 0 );
}

void CMasterLevel::BeginTest( int32 nType, int8 nDir )
{
	m_nTestState = nType;
	m_testOrig = GetPlayer()->GetPosition();
	m_nTestDir = nDir;
}

void CMasterLevel::UpdateTest()
{
	if( m_nTestState )
	{
		if( m_nTestState >= eTest_Scan_0 )
		{
			auto fSpeed = m_fTestScanSpeed[m_nTestState - eTest_Scan_0];
			CVector2 ofs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			m_testOrig = m_testOrig + ofs[m_nTestDir] * fSpeed * GetStage()->GetElapsedTimePerTick();
			if( m_nTestDir == 1 || m_nTestDir == 3 )
				m_testOrig.x = m_pPlayer->x;
			else
				m_testOrig.y = m_pPlayer->y;
		}
		auto testRect = GetTestRect();
		if( !testRect.Contains( m_pPlayer->GetPosition() ) )
			EndTest();
	}
}

void CMasterLevel::EndTest()
{
	m_nTestState = 0;
	if( m_pCurLevel )
		m_pCurLevel->CheckBugs( false );
}

void CMasterLevel::BeginAlert( const CRectangle& rect, const CVector2& vel )
{
	m_bAlert = true;
	m_alertRect = rect;
	m_alertVel = vel;
	m_pAlertSound = m_pSoundFileAlert->CreateSoundTrack();
	m_pAlertSound->Play( ESoundPlay_KeepRef | ESoundPlay_Loop );
}

void CMasterLevel::UpdateAlert()
{
	if( m_bAlert )
	{
		m_alertRect = m_alertRect.Offset( m_alertVel * GetStage()->GetElapsedTimePerTick() );
		if( !m_alertRect.Contains( m_pPlayer->GetPosition() ) )
			EndAlert();
	}
}

void CMasterLevel::EndAlert()
{
	m_bAlert = false;
	if( m_pAlertSound )
	{
		m_pAlertSound->FadeOut( 0.1f );
		m_pAlertSound = NULL;
	}
}

const char* CMasterLevel::CheckTeleport( bool bUp )
{
	if( m_nTestState || m_bAlert )
		return false;
	auto& worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	auto pLevelData = worldCfg.GetLevelData( m_pCurLevelPrefab->GetName() );
	for( int i = 0; i < pLevelData->arrOverlapLevel.Size(); i++ )
	{
		if( CheckTeleport( pLevelData->arrOverlapLevel[i].c_str(), bUp ? -1 : 1 ) )
			return pLevelData->arrOverlapLevel[i];
	}
	return NULL;
}

bool CMasterLevel::CheckTeleport( const char* sz, int8 bUp )
{
	auto& worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	auto pLevelData = worldCfg.GetLevelData( m_pCurLevelPrefab->GetName() );
	auto pLevelData1 = worldCfg.GetLevelData( sz );
	auto transferOfs = pLevelData1->displayOfs - pLevelData->displayOfs;
	auto pBufferedLevel = m_mapBufferedLevels[sz];
	if( bUp > 0 && pBufferedLevel->m_nLevelZ < m_pCurLevel->m_nLevelZ || bUp < 0 && pBufferedLevel->m_nLevelZ > m_pCurLevel->m_nLevelZ )
		return false;
	if( pBufferedLevel->CheckTeleport( m_pPlayer, transferOfs ) )
		return true;
	return false;
}

CLevelEnvLayer* CMasterLevel::CurEnv()
{
	if( m_pCurLevel && m_pCurLevel->GetEnv() )
		return m_pCurLevel->GetEnv();
	return m_pDefaultEnvLayer;
}

void CMasterLevel::UpdateCtrlPoints( float fWeight )
{
	if( m_pCurEnvLayer )
	{
		m_pCurEnvLayer->UpdateCtrlPoints();
		auto trans1 = m_pCurEnvLayer->GetCtrlPointsTrans();
		m_camTrans = ( trans1 - m_camTrans ) * fWeight + m_camTrans;
		if( m_pCurLevel )
		{
			m_camTrans.x += m_pCurLevel->x;
			m_camTrans.y += m_pCurLevel->y;
		}
	}
}

void CMasterLevel::BufferLevels()
{
	auto& worldCfg = GetStage()->GetWorld()->GetWorldCfg();
	auto pLevelData = worldCfg.GetLevelData( m_pCurLevelPrefab->GetName() );
	for( int i = 0; i < pLevelData->arrOverlapLevel.Size(); i++ )
	{
		const char* szName = pLevelData->arrOverlapLevel[i].c_str();
		if( !m_mapBufferedLevels[szName] )
		{
			if( m_pLastLevelPrefab && 0 == strcmp( szName, m_pLastLevelPrefab->GetName() ) )
			{
				m_mapBufferedLevels[szName] = m_pLastLevel;
				m_mapBufferedLevelPrefabs[szName] = m_pLastLevelPrefab;
				m_pLastLevel->SetParentEntity( GetStage()->GetRoot() );
				m_pLastLevel->SetPosition( CVector2( 0, 0 ) );
				m_pLastLevel = NULL;
				m_pLastLevelPrefab = NULL;
			}
			else
			{
				auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szName );
				m_mapBufferedLevelPrefabs[szName] = pPrefab;
				CMyLevel* pLevel = CreateLevel( pPrefab );
				m_mapBufferedLevels[szName] = pLevel;
				pLevel->SetParentEntity( GetStage()->GetRoot() );
				pLevel->Init();
			}
		}
	}

	vector<string> vecRemove;
	for( auto& item : m_mapBufferedLevels )
	{
		bool b = false;
		for( int i = 0; i < pLevelData->arrOverlapLevel.Size(); i++ )
		{
			if( pLevelData->arrOverlapLevel[i].c_str() == item.first )
			{
				b = true;
				break;
			}
		}
		if( !b )
			vecRemove.push_back( item.first );
	}
	/*for( auto& item : vecRemove )
	{
		m_mapBufferedLevels.erase( item );
		m_mapBufferedLevelPrefabs.erase( item );
	}*/
}

void CMasterLevel::TransferFunc()
{
	if( m_pCurLevel )
		EndCurLevel();
	m_pCurLevelPrefab = m_pTransferTo;
	CreateCurLevel();
	m_pLastLevel->SetPosition( m_transferOfs * -1 );
	m_pCurLevel->SetPosition( CVector2( 0, 0 ) );
	m_pCurEnvLayer = CurEnv();
	m_pCurEnvLayer->InitCtrlPointsState( m_camTrans.x - m_transferOfs.x, m_camTrans.y - m_transferOfs.y, m_camTrans.z, m_camTrans.w );
	m_camTransPlayerOfs = m_transferOfs * -1;

	for( int i = 1; i <= 60; i++ )
	{
		m_pTransferCoroutine->Yield( 0 );
		UpdateMasks( i / 60.0f );
	}

	m_pCurLevel->SetRenderParentBefore( m_pLastLevel );
	m_pLastLevel->SetRenderParentAfter( m_pLevelFadeMask );
	m_pLastLevel->PlayerLeave();
	m_pPlayer->SetPosition( m_pPlayer->GetPosition() - m_transferOfs );
	m_pCurLevel->PlayerEnter( m_pPlayer, m_pLastLevel->m_pControlled );
	m_camTransPlayerOfs = CVector2( 0, 0 );

	for( int i = 59; i >= 0; i-- )
	{
		m_pTransferCoroutine->Yield( 0 );
		UpdateMasks( i / 60.0f );
	}
	BufferLevels();
	if( m_pLastLevel )
	{
		m_pLastLevel->SetParentEntity( NULL );
		m_pLastLevel = NULL;
		m_pLastLevelPrefab = NULL;
	}
	BeginCurLevel();
}

void CMasterLevel::TestConsoleFunc()
{
	m_pMask1->bVisible = true;
	m_pTestUI->bVisible = true;
	int32 nSelectedType = 0;
	int8 nSelectedDir = 0;
	CVector2 orig = m_pPlayer->GetPosition();
	m_pTestUI->SetPosition( orig );
	//m_pTestUIItems[1]->bVisible = m_pPlayer->GetPlayerLevel() >= ePlayerLevel_Test_Scan;
	m_pTestUIItems[2]->bVisible = false;
	while( true )
	{
		auto p = CVector2( 0, 0 );
		if( nSelectedDir )
		{
			CVector2 ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			p = ofs[nSelectedDir - 1] * nSelectedType * 100;
		}
		m_pTestUIItems[0]->SetPosition( p );

		UpdateTestMasks( nSelectedType + 1, nSelectedDir - 1, orig, 0 );
		m_pTestConsoleCoroutine->Yield( 0 );

		if( /*m_pPlayer->GetPlayerLevel() >= ePlayerLevel_Test_Scan*/true )
		{
			int8 nX = CGame::Inst().IsKeyDown( 'D' ) - CGame::Inst().IsKeyDown( 'A' );
			int8 nY = CGame::Inst().IsKeyDown( 'W' ) - CGame::Inst().IsKeyDown( 'S' );
			if( /*m_pPlayer->GetPlayerLevel() >= ePlayerLevel_Test_Scan_1*/true )
			{
				if( nX )
				{
					if( nSelectedDir == 2 - nX )
						nSelectedType = Min( nSelectedType + 1, 3 );
					else if( nSelectedDir == 2 + nX )
					{
						nSelectedType--;
						if( !nSelectedType )
							nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 1;
						nSelectedDir = 2 - nX;
					}
				}
				if( nY )
				{
					if( nSelectedDir == 3 - nY )
						nSelectedType = Min( nSelectedType + 1, 3 );
					else if( nSelectedDir == 3 + nY )
					{
						nSelectedType--;
						if( !nSelectedType )
							nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 1;
						nSelectedDir = 3 - nY;
					}
				}
			}
			else
			{
				if( nX )
				{
					if( nSelectedDir == 2 - nX )
						nSelectedType = 2;
					else if( nSelectedDir == 2 + nX )
					{
						nSelectedType = 0;
						nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 2;
						nSelectedDir = 2 - nX;
					}
				}
				if( nY )
				{
					if( nSelectedDir == 3 - nY )
						nSelectedType = 2;
					else if( nSelectedDir == 3 + nY )
					{
						nSelectedType = 0;
						nSelectedDir = 0;
					}
					else
					{
						nSelectedType = 2;
						nSelectedDir = 3 - nY;
					}
				}
			}
		}

		if( CGame::Inst().IsKey( VK_RETURN ) )
		{
			CGame::Inst().ForceKeyRelease( VK_RETURN );
			m_pMask1->bVisible = false;
			m_pTestUI->bVisible = false;
			return;
		}
		if( CGame::Inst().IsKeyDown( ' ' ) )
			break;
	}

	m_pTestUI->bVisible = false;
	for( int i = 1; i <= 60; i++ )
	{
		m_pTestConsoleCoroutine->Yield( 0 );
		UpdateTestMasks( nSelectedType + 1, nSelectedDir - 1, orig, i / 60.0f );
	}
	BeginTest( nSelectedType + 1, nSelectedDir - 1 );
	m_pMask1->bVisible = false;
}

void CMasterLevel::UpdateMasks( float fFade )
{
	auto pMask = static_cast<CImage2D*>( m_pLevelFadeMask->GetRenderObject() );
	auto maskParams = pMask->GetParam();
	maskParams[0] = CVector4( fFade, fFade, fFade, fFade );
}

void CMasterLevel::UpdateTestMasks( int32 nType, int8 nDir, const CVector2& orig, float fEnterTest )
{
	if( nType > 0 )
	{
		m_pMask1->bVisible = true;
		auto rect = GetTestRect( nType, nDir, orig );
		static_cast<CImage2D*>( m_pMask1->GetRenderObject() )->SetRect( rect );
		m_pMask1->GetRenderObject()->SetBoundDirty();

		m_pTestPlayerCross->bVisible = fEnterTest >= 1;
		if( m_pTestPlayerCross->bVisible )
			m_pTestPlayerCross->SetPosition( m_pPlayer->GetPosition() );

		for( int i = 0; i < 3; i++ )
			m_maskParams[i] = m_maskParams[i] + m_testConsoleParam[i] * ( 1 - fEnterTest ) + m_testParam[i] * fEnterTest;
	}
	else
	{
		m_pMask1->bVisible = false;
		m_pTestPlayerCross->bVisible = false;
	}
	if( m_bAlert )
	{
		m_pMask2->bVisible = true;
		auto pImg = static_cast<CImage2D*>( m_pMask2->GetRenderObject() );
		pImg->SetRect( m_alertRect );
		float t = abs( ( CGame::Inst().GetTimeStamp() & 127 ) - 64 ) / 64.0f;
		pImg->GetParam()[0] = CVector4( 1.2f, 1.2f, 0.95f, 0 ) + CVector4( 0.4f, 0.4f, -0.1f, 0 ) * t;
		pImg->GetParam()[1] = CVector4( 0.02f, 0.02f, 0.02f, 0 ) + CVector4( 0.04f, 0.04f, 0.04f, 0 ) * t;
		pImg->SetBoundDirty();
	}
	else
		m_pMask2->bVisible = false;
}

void RegisterGameClasses_Level()
{
	REGISTER_ENUM_BEGIN( ELevelCamCtrlPointSmootherType )
		REGISTER_ENUM_ITEM( eLevelCamCtrlPointSmoother_IgnoreSmallForce )
	REGISTER_ENUM_END()

	REGISTER_ENUM_BEGIN( ELevelCamCtrlPoint1LimitorType )
		REGISTER_ENUM_ITEM( eLevelCamCtrlPoint1Limitor_Rect )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( SLevelCamCtrlPoint )
		REGISTER_MEMBER( nPointType )
		REGISTER_MEMBER( nResetType )
		REGISTER_MEMBER( fWeight )
		REGISTER_MEMBER( fDamping )
		REGISTER_MEMBER( orig )
		REGISTER_MEMBER( g1 )
		REGISTER_MEMBER( g2 )
		REGISTER_MEMBER( arrPath )
		REGISTER_MEMBER( arrTangent )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelCamCtrlPointLink )
		REGISTER_MEMBER( n1 )
		REGISTER_MEMBER( n2 )
		REGISTER_MEMBER( ofs1 )
		REGISTER_MEMBER( ofs2 )
		REGISTER_MEMBER( fStrength1 )
		REGISTER_MEMBER( fStrength2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelCamCtrlPointSmoother )
		REGISTER_MEMBER( n )
		REGISTER_MEMBER( nType )
		REGISTER_MEMBER( params )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SLevelCamCtrlPoint1Limitor )
		REGISTER_MEMBER( n1 )
		REGISTER_MEMBER( n2 )
		REGISTER_MEMBER( nType )
		REGISTER_MEMBER( params )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLightArea )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_fRad )
		REGISTER_MEMBER( m_fRad1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CBlackRegion )
		REGISTER_BASE_CLASS( CEntity )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelEnvLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_ctrlPoint1 )
		REGISTER_MEMBER( m_ctrlPoint2 )
		REGISTER_MEMBER( m_arrCtrlPoint )
		REGISTER_MEMBER( m_arrCtrlLink )
		REGISTER_MEMBER( m_arrCtrlSmoother )
		REGISTER_MEMBER( m_arrCtrlLimitor )
		REGISTER_MEMBER( m_nFadeTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelObjLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( ILevelObjLayer )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLevelBugIndicatorLayer )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( ILevelObjLayer )
		REGISTER_MEMBER( m_texRect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPortal )
		REGISTER_BASE_CLASS( CCharacter )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SBugLink )
		REGISTER_MEMBER( a )
		REGISTER_MEMBER( b )
		REGISTER_MEMBER( arrPath )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMyLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nLevelZ )
		REGISTER_MEMBER( m_size )
		REGISTER_MEMBER( m_arrBugLink )
		REGISTER_MEMBER_TAGGED_PTR( m_pStartPoint, start )
		REGISTER_MEMBER_TAGGED_PTR( m_pCurEnvLayer, env )
		REGISTER_MEMBER_TAGGED_PTR( m_pBugIndicatorLayer, bug_l )
		REGISTER_MEMBER_TAGGED_PTR( m_pBlackRegion, black_region )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CMasterLevel )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_dmgParam )
		REGISTER_MEMBER( m_testConsoleParam )
		REGISTER_MEMBER( m_testParam )
		REGISTER_MEMBER( m_strBeginLevel )
		REGISTER_MEMBER( m_pPlayerPrefab )
		REGISTER_MEMBER( m_nKillTick )
		REGISTER_MEMBER( m_testRect )
		REGISTER_MEMBER( m_fTestScanSpeed )
		REGISTER_MEMBER( m_backOfsScale )
		REGISTER_MEMBER( m_pSoundFileAlert )
		REGISTER_MEMBER_TAGGED_PTR( m_pLayer1, 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pLevelFadeMask, mask )
		REGISTER_MEMBER_TAGGED_PTR( m_pDefaultEnvLayer, env )
		REGISTER_MEMBER_TAGGED_PTR( m_pMask1, m1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMask2, m2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pBack, back )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUI, test_ui )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUIItems[0], test_ui/0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUIItems[1], test_ui/1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestUIItems[2], test_ui/2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pTestPlayerCross, test_cross )
	REGISTER_CLASS_END()
}
