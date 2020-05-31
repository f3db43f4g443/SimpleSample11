#include "stdafx.h"
#include "LevelEdit.h"
#include "MyLevel.h"
#include "UICommon/UIViewport.h"
#include "Editor/Editors/PrefabEditor.h"
#include "Common/Utf8Util.h"
#include "UICommon/UIFactory.h"

CLevelEdit::CLevelEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName )
	: CObjectDataEdit( pTreeView, pParent, pData, pMetaData, szName ), m_nObjType( 0 ), m_nDragType( 0 )
{
	auto pObj = (CMyLevel*)m_pData;
	m_nWidth = pObj->m_nWidth;
	m_nHeight = pObj->m_nHeight;
	auto& grids = pObj->m_arrGridData;
	if( grids.Size() != m_nWidth * m_nHeight )
		grids.Resize( m_nWidth * m_nHeight );

	m_nCurSelectedOpr = 0;
	m_nCurSelectedValue = 0;
}

void CLevelEdit::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform )
{
	if( !m_nWidth || !m_nHeight )
		return;
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	CVector2 ofs1[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	CVector4 colors[] = { { 1, 1, 0, 1 }, { 0, 1, 1, 1 }, { 1, 0, 1, 1 }, { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 } };
	{
		auto b = CVector2( m_nWidth, m_nHeight );
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = b * ofs[j];
			auto pt2 = b * ofs[( j + 1 ) % 4];
			pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( pt1 * LEVEL_GRID_SIZE ), transform.MulVector2Pos( pt2 * LEVEL_GRID_SIZE ), CVector4( 1, 1, 1, 1 ) );
		}
	}

	auto pObj = (CMyLevel*)m_pData;
	auto& grids = pObj->m_arrGridData;

	int32 nValueCount = GetCurOprValueCount();
	for( int x = 0; x < m_nWidth; x++ )
	{
		for( int y = 0; y < m_nHeight; y++ )
		{
			auto& grid = grids[x + y * m_nWidth];

			CVector4 color;
			if( m_nCurSelectedOpr == 0 )
				color = grid.bBlocked ? CVector4( 0.35f, 0.35f, 0.35f, 1 ) : CVector4( 0.35f, 0.8f, 0.8f, 1 );
			else
			{
				color = CVector4( 0.5f, 0.5f, 0.5f, 0.5f );
				auto n = Max( 0, Min( nValueCount - 1, grid.nNextStage ) );
				if( n > 0 )
				{
					color = colors[( n - 1 ) % ELEM_COUNT( colors )];
					color = color + ( CVector4( 0.35f, 0.35f, 0.35f, 0.35f ) - color ) * ( 1.0f * ( n - 1 ) / ( nValueCount - 1 ) );
				}
			}
			for( int j = 0; j < 2; j++ )
			{
				auto pt1 = CVector2( x + 0.5f, y + 0.5f ) + ofs1[j] * 0.35f;
				auto pt2 = CVector2( x + 0.5f, y + 0.5f ) + ofs1[( j + 2 ) % 4] * 0.35f;
				pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( pt1 * LEVEL_GRID_SIZE ), transform.MulVector2Pos( pt2 * LEVEL_GRID_SIZE ), color );
			}
		}
	}

	for( int i = 0; i < 2; i++ )
	{
		auto color = colors[i];
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = CVector2( m_nWidth - 1 - i, m_nHeight ) + ( m_nCurSelectedOpr == i ? ofs[j] : ofs[j] * 0.5f + CVector2( 0.25f, 0.25f ) );
			auto pt2 = CVector2( m_nWidth - 1 - i, m_nHeight ) + ( m_nCurSelectedOpr == i ? ofs[( j + 1 ) % 4]
				: ofs[( j + 1 ) % 4] * 0.5f + CVector2( 0.25f, 0.25f ) );
			pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( pt1 * LEVEL_GRID_SIZE ), transform.MulVector2Pos( pt2 * LEVEL_GRID_SIZE ), color );
		}
	}
	for( int i = 0; i < nValueCount; i++ )
	{
		CVector4 color( 1, 1, 1, 1 );
		if( i > 0 )
		{
			color = colors[( i - 1 ) % ELEM_COUNT( colors )];
			color = color + ( CVector4( 0.35f, 0.35f, 0.35f, 0.35f ) - color ) * ( 1.0f * ( i - 1 ) / ( nValueCount - 1 ) );
		}
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = CVector2( m_nWidth - 1 - i, -1 ) + ( m_nCurSelectedValue == i ? ofs[j] : ofs[j] * 0.5f + CVector2( 0.25f, 0.25f ) );
			auto pt2 = CVector2( m_nWidth - 1 - i, -1 ) + ( m_nCurSelectedValue == i ? ofs[( j + 1 ) % 4]
				: ofs[( j + 1 ) % 4] * 0.5f + CVector2( 0.25f, 0.25f ) );
			pViewport->DebugDrawLine( pRenderSystem, transform.MulVector2Pos( pt1 * LEVEL_GRID_SIZE ), transform.MulVector2Pos( pt2 * LEVEL_GRID_SIZE ), color );
		}
	}
	CObjectDataEdit::OnDebugDraw( pViewport, pRenderSystem, transform );
}

CObjectDataEditItem* CLevelEdit::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform )
{
	if( !m_nWidth || !m_nHeight )
		return CObjectDataEdit::OnViewportStartDrag( pViewport, mousePos, transform );

	auto pObj = (CMyLevel*)m_pData;
	auto dragPos = transform.MulTVector2PosNoScale( mousePos );
	auto rect = CRectangle( 0, 0, m_nWidth * LEVEL_GRID_SIZE_X, m_nHeight * LEVEL_GRID_SIZE_Y );
	if( dragPos.y >= m_nHeight * LEVEL_GRID_SIZE_Y && dragPos.y <= ( m_nHeight + 1 ) * LEVEL_GRID_SIZE_Y )
	{
		int32 i = floor( m_nWidth - dragPos.x / LEVEL_GRID_SIZE_X );
		if( i >= 0 && i < 3 )
		{
			m_nCurSelectedOpr = i;
			m_nCurSelectedValue = 0;
			return NULL;
		}
	}
	if( dragPos.y >= -LEVEL_GRID_SIZE_Y && dragPos.y <= 0 )
	{
		int32 i = floor( m_nWidth - dragPos.x / LEVEL_GRID_SIZE_X );
		if( i >= 0 && i < GetCurOprValueCount() )
		{
			m_nCurSelectedValue = i;
			return NULL;
		}
	}

	int32 x = floor( dragPos.x / LEVEL_GRID_SIZE_X );
	int32 y = floor( dragPos.y / LEVEL_GRID_SIZE_Y );
	if( x >= 0 && y >= 0 && x < m_nWidth && y < m_nHeight )
	{
		m_nDragType = 1;
		UpdateDrag( pViewport, TVector2<int32>( x, y ), transform );
		return this;
	}

	return CObjectDataEdit::OnViewportStartDrag( pViewport, mousePos, transform );
}

void CLevelEdit::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform )
{
	if( !m_nDragType )
		return;
	auto dragPos = transform.MulTVector2PosNoScale( mousePos );
	int32 x = floor( dragPos.x / LEVEL_GRID_SIZE_X );
	int32 y = floor( dragPos.y / LEVEL_GRID_SIZE_Y );
	UpdateDrag( pViewport, TVector2<int32>( x, y ), transform );
}

void CLevelEdit::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform )
{
	if( !m_nDragType )
		return;
	auto dragPos = transform.MulTVector2PosNoScale( mousePos );
	int32 x = floor( dragPos.x / LEVEL_GRID_SIZE_X );
	int32 y = floor( dragPos.y / LEVEL_GRID_SIZE_Y );
	UpdateDrag( pViewport, TVector2<int32>( x, y ), transform );
}

int32 CLevelEdit::GetCurOprValueCount()
{
	auto pObj = (CMyLevel*)m_pData;
	if( m_nCurSelectedOpr == 0 )
		return 2;
	return pObj->m_arrNextStage.Size() + 1;
}

void CLevelEdit::UpdateDrag( CUIViewport* pViewport, const TVector2<int32>& p, const CMatrix2D& transform )
{
	if( p.x < 0 || p.y < 0 || p.x >= m_nWidth || p.y >= m_nHeight )
		return;
	auto pObj = (CMyLevel*)m_pData;

	auto& grids = pObj->m_arrGridData;
	auto& grid = grids[p.x + p.y * m_nWidth];
	if( m_nCurSelectedOpr == 0 )
	{
		if( grid.bBlocked == m_nCurSelectedValue )
			return;
		grid.bBlocked = m_nCurSelectedValue;
	}
	else if( m_nCurSelectedOpr == 1 )
	{
		if( grid.nNextStage == m_nCurSelectedValue )
			return;
		grid.nNextStage = m_nCurSelectedValue;
	}
	if( m_pContent->pParent )
		m_pContent->pParent->pElement->Action( (void*)1 );
}

void CLevelEdit::OnEdit( uint32 nParam )
{
	auto pObj = (CMyLevel*)m_pData;
	if( m_nWidth != pObj->m_nWidth || m_nHeight != pObj->m_nHeight )
	{
		vector<SLevelGridData> vecTemp;
		vecTemp.resize( m_nWidth * m_nHeight );
		auto& grids = pObj->m_arrGridData;
		for( int i = 0; i < vecTemp.size(); i++ )
			vecTemp[i] = grids[i];
		grids.Resize( pObj->m_nWidth * pObj->m_nHeight );
		for( int x = 0; x < pObj->m_nWidth; x++ )
		{
			for( int y = 0; y < pObj->m_nHeight; y++ )
			{
				int32 i = Min( x, m_nWidth - 1 );
				int32 j = Min( y, m_nHeight - 1 );
				grids[x + y * pObj->m_nWidth] = vecTemp[i + j * m_nWidth];
			}
		}
		m_nWidth = pObj->m_nWidth;
		m_nHeight = pObj->m_nHeight;
	}
	CObjectDataEdit::OnEdit( nParam );
}
