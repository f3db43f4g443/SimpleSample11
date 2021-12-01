#include "stdafx.h"
#include "Terrain.h"
#include "Render/TileMap2D.h"
#include "MyLevel.h"

void CTerrain::OnPreview()
{
	for( auto p = Get_ChildEntity(); p; p = p->NextChildEntity() )
		p->OnPreview();
}

void CTerrain::OnAddedToStage()
{
	if( !m_bInited )
	{
		m_bInited = true;
		auto pTileMap = static_cast<CTileMap2D*>( GetRenderObject() );
		CVector2 tileSize = pTileMap->GetTileSize();
		CVector2 baseOfs = pTileMap->GetBaseOffset();
		int32 nWidth = pTileMap->GetWidth();
		int32 nHeight = pTileMap->GetHeight();

		for( int i = 0; i <= nWidth; i++ )
		{
			for( int j = 0; j <= nHeight; j++ )
			{
				uint32 n = pTileMap->GetEditData( i, j ) & 0xffff;
				if( n >= m_nBlockIndexBegin )
				{
					CRectangle rect( -tileSize.x * 0.5f, -tileSize.y * 0.5f, tileSize.x, tileSize.y );
					auto pEntity = new CCharacter;
					pEntity->SetPosition( baseOfs + tileSize * CVector2( i, j ) );
					pEntity->AddRect( rect );
					pEntity->SetParentEntity( this );
				}
			}
		}
		if( m_nBorder > 0 )
		{
			pTileMap->Resize( TRectangle<int32>( -m_nBorder, -m_nBorder, nWidth + m_nBorder * 2, nHeight + m_nBorder * 2 ), pTileMap->GetInfo()->editInfos.size() - 1 );

			CRectangle rect0( baseOfs.x - tileSize.x * 0.5f, baseOfs.y - tileSize.y * 0.5f, ( nWidth + 1 ) * tileSize.x, ( nHeight + 1 ) * tileSize.y );
			CRectangle rect[4] = { { rect0.x - m_nBorder * tileSize.x, rect0.y, m_nBorder * tileSize.x, rect0.height },
			{ rect0.GetRight(), rect0.y, m_nBorder * tileSize.x, rect0.height },
			{ rect0.x - m_nBorder * tileSize.x, rect0.y - m_nBorder * tileSize.y, rect0.width + m_nBorder * tileSize.x * 2, m_nBorder * tileSize.y },
			{ rect0.x - m_nBorder * tileSize.x, rect0.GetBottom(), rect0.width + m_nBorder * tileSize.x * 2, m_nBorder * tileSize.y } };
			for( int i = 0; i < 4; i++ )
			{
				auto pEntity = new CCharacter;
				pEntity->AddRect( rect[i] );
				pEntity->SetParentEntity( this );
			}
		}
	}
}

void CTerrain::InitFromTemplate( CEntity* p, const CRectangle& rect )
{
	auto pTerrain = SafeCast<CTerrain>( p );
	m_nBlockIndexBegin = pTerrain->m_nBlockIndexBegin;
	m_nBorder = pTerrain->m_nBorder;
	m_tileSize = pTerrain->m_tileSize;
	m_nTileX = floor( rect.width / m_tileSize.x );
	m_nTileY = floor( rect.height / m_tileSize.y );
	m_ofs = CVector2( rect.x, rect.y );
}
