#include "stdafx.h"
#include "LevelEditTools.h"
#include "LevelTools.h"
#include "UICommon/UIManager.h"
#include "Game/Character.h"
#include "Game/Interfaces.h"
#include "Game/Entities/CharacterMisc.h"
#include "Game/Entities/UtilEntities.h"

CRenderObject2D * CLevelEditToolDefault::CreateToolPreview()
{
	if( !m_pPreviewRoot )
		m_pPreviewRoot = new CRenderObject2D;
	return m_pPreviewRoot;
}

void CLevelEditToolDefault::ToolEnd()
{
	__super::ToolEnd();
	m_bDblClick = false;
	m_pTempSelectedNode = NULL;
	if( m_pQuickTool )
	{
		m_pQuickTool->ToolEnd();
		m_pQuickTool = NULL;
		if( m_pPreviewRoot )
			m_pPreviewRoot->RemoveAllChild();
	}
}

void CLevelEditToolDefault::OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
	CPrefabNode* pNode = m_pTempSelectedNode;
	if( !pNode )
		pNode = CLevelToolsView::Inst()->Pick( mousePos );
	if( !pNode )
		return;
	auto pEntityData = (CEntity*)pNode->GetStaticDataSafe<CEntity>();
	if( !pEntityData )
		return;
	auto rect = pEntityData->GetBoundForEditor();
	CVector2 verts[] = { { rect.x, rect.y }, { rect.GetRight(), rect.y }, { rect.GetRight(), rect.GetBottom() }, { rect.x, rect.GetBottom() } };
	CMatrix2D trans;
	trans.Transform( pNode->x, pNode->y, pNode->r, pNode->s );
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = trans.MulVector2Pos( verts[i] );
	CVector4 color = m_pTempSelectedNode ? CVector4( 1, 1, 1, 1 ) : CVector4( 0.5f, 0.5f, 0.5f, 0.5f );
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, verts[i], verts[( i + 1 ) % 4], color );

	if( m_pQuickTool )
		m_pQuickTool->OnDebugDraw( pViewport, pRenderSystem );
}

bool CLevelEditToolDefault::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_pQuickTool )
	{
		if( m_pQuickTool->OnViewportStartDrag( pViewport, mousePos ) )
		{
			m_bQuickToolDrag = true;
			return true;
		}
	}

	auto pNode = CLevelToolsView::Inst()->Pick( mousePos );
	if( !pNode )
		return false;
	m_bDrag = true;
	if( m_pQuickTool )
	{
		m_pQuickTool->ToolEnd();
		m_pQuickTool = NULL;
		m_pPreviewRoot->RemoveAllChild();
	}
	if( m_pTempSelectedNode == pNode )
		m_bDblClick = true;
	m_pTempSelectedNode = pNode;
	m_beginMousePos = mousePos;
	m_beginObjPos = pNode->GetPosition();
	return true;
}

void CLevelEditToolDefault::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_bQuickToolDrag )
	{
		m_pQuickTool->OnViewportDragged( pViewport, mousePos );
		return;
	}
	if( !m_bDrag )
		return;
	if( mousePos != m_beginMousePos )
		m_bDblClick = false;
	float fGridSize = CLevelToolsView::Inst()->GetGridSize();
	auto d = ( mousePos - m_beginMousePos ) / fGridSize;
	d = CVector2( floor( d.x + 0.5f ), floor( d.y + 0.5f ) ) * fGridSize;
	m_pTempSelectedNode->SetPosition( m_beginObjPos + d );
}

void CLevelEditToolDefault::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_bQuickToolDrag )
	{
		m_pQuickTool->OnViewportStopDrag( pViewport, mousePos );
		m_bQuickToolDrag = false;
		return;
	}
	if( !m_bDrag )
		return;
	m_bDrag = false;
	if( m_bDblClick )
		CLevelToolsView::Inst()->SelectObj( m_pTempSelectedNode );
	else if( m_pTempSelectedNode )
	{
		auto pToolset = CLevelEditToolsetMgr::Inst().GetObjectToolset( m_pTempSelectedNode->GetPatchedNode()->GetClassData() );
		m_pQuickTool = pToolset->CreateQuickObjectTool( m_pTempSelectedNode );
		if( m_pQuickTool )
		{
			m_pQuickTool->ToolBegin( m_pTempSelectedNode );
			auto pPreview = m_pQuickTool->CreateToolPreview();
			if( pPreview )
				m_pPreviewRoot->AddChild( pPreview );
		}
	}
}

bool CLevelEditToolDefault::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( m_pQuickTool )
	{
		if( m_pQuickTool->OnViewportKey( pEvent ) )
			return true;
	}
	return false;
}

void CLevelEditToolDefault::OnPreviewDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	if( m_pQuickTool )
		m_pQuickTool->OnPreviewDebugDraw( pViewport, pRenderSystem );
}

bool CLevelEditToolDefault::OnPreviewViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_pQuickTool )
		return m_pQuickTool->OnPreviewViewportStartDrag( pViewport, mousePos );
	return false;
}

void CLevelEditToolDefault::OnPreviewViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_pQuickTool )
		m_pQuickTool->OnPreviewViewportDragged( pViewport, mousePos );
}

void CLevelEditToolDefault::OnPreviewViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_pQuickTool )
		m_pQuickTool->OnPreviewViewportStopDrag( pViewport, mousePos );
}

void CLevelEditToolEditBase::ToolBegin()
{
	__super::ToolBegin();
	auto pCurLayerNode = CLevelToolsView::Inst()->GetCurLayerNode();
	if( pCurLayerNode->GetStaticDataSafe<CTerrain>() )
		m_pEditTool = &CLevelEditObjectToolTerrain::Inst();
	if( m_pEditTool )
		m_pEditTool->ToolBegin( pCurLayerNode );
}

void CLevelEditToolEditBase::ToolEnd()
{
	__super::ToolEnd();
	if( m_pEditTool )
	{
		m_pEditTool->ToolEnd();
		m_pEditTool = NULL;
	}
}

void CLevelEditToolEditBase::OnDebugDraw( CUIViewport * pViewport, IRenderSystem * pRenderSystem )
{
	if( m_pEditTool )
		m_pEditTool->OnDebugDraw( pViewport, pRenderSystem );
}

bool CLevelEditToolEditBase::OnViewportStartDrag( CUIViewport * pViewport, const CVector2 & mousePos )
{
	if( m_pEditTool )
		return m_pEditTool->OnViewportStartDrag( pViewport, mousePos );
	return false;
}

void CLevelEditToolEditBase::OnViewportDragged( CUIViewport * pViewport, const CVector2 & mousePos )
{
	if( m_pEditTool )
		m_pEditTool->OnViewportDragged( pViewport, mousePos );
}

void CLevelEditToolEditBase::OnViewportStopDrag( CUIViewport * pViewport, const CVector2 & mousePos )
{
	if( m_pEditTool )
		m_pEditTool->OnViewportStopDrag( pViewport, mousePos );
}

bool CLevelEditToolEditBase::OnViewportKey( SUIKeyEvent * pEvent )
{
	if( m_pEditTool )
		return m_pEditTool->OnViewportKey( pEvent );
	return false;
}

void CLevelEditToolErase::OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
	if( m_nMode == 0 )
	{
		pViewport->DebugDrawLine( pRenderSystem, mousePos + CVector2( -4, -4 ), mousePos + CVector2( 4, 4 ), CVector4( 0.8f, 0.1f, 0.1f, 1 ) );
		pViewport->DebugDrawLine( pRenderSystem, mousePos + CVector2( -4, 4 ), mousePos + CVector2( 4, -4 ), CVector4( 0.8f, 0.1f, 0.1f, 1 ) );
	}
	else
	{
		CVector2 verts[] = { { -3, -4 }, { 0, -1 }, { 3, -4 },
		{ 4, -3 }, { 1, 0 }, { 4, 3 },
		{ 3, 4 }, { 0, 1 }, { -3, 4 },
		{ -4, 3 }, { -1, 0 }, { -4, -3 } };
		for( int i = 0; i < ELEM_COUNT( verts ); i++ )
			verts[i] = verts[i] + mousePos;
		pViewport->DebugDrawTriangles( pRenderSystem, 12, verts, CVector4( 0.8f, 0.1f, 0.1f, 1 ) );
	}
}

bool CLevelEditToolErase::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	m_bDrag = true;
	CLevelToolsView::Inst()->Erase( mousePos, m_nMode == 0 );
	return true;
}

void CLevelEditToolErase::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( !m_bDrag )
		return;
	if( m_nMode == 1 )
		CLevelToolsView::Inst()->Erase( mousePos, false );
}

void CLevelEditToolErase::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	m_bDrag = false;
}

bool CLevelEditToolErase::OnViewportKey( SUIKeyEvent * pEvent )
{
	if( m_bDrag )
		return true;
	if( pEvent->bKeyDown )
	{
		if( pEvent->nChar == 'Q' )
		{
			m_nMode = 1 - m_nMode;
			return true;
		}
	}
	return false;
}


void CLevelEditToolsetMgr::GetDefaultTools( vector<SLevelEditToolDesc>& result )
{
	result.push_back( SLevelEditToolDesc( 2, 8, &CLevelEditToolDefault::Inst() ) );
	result.push_back( SLevelEditToolDesc( 4, 8, &CLevelEditToolEditBase::Inst() ) );
	result.push_back( SLevelEditToolDesc( 3, 8, &CLevelEditToolErase::Inst() ) );
}


void CLevelEditPrefabToolEntityBrush::ToolBegin( CPrefab* pPrefab )
{
	__super::ToolBegin( pPrefab );
	m_bound = m_pObjData->GetBoundForEditor();
	m_bTiled = false;
	auto pTiled = SafeCastToInterface<IEditorTiled>( m_pObjData );
	if( pTiled )
	{
		m_bTiled = true;
		m_size = pTiled->GetMinSize();
		auto tileSize = pTiled->GetTileSize();
		m_bound = CRectangle( 0, 0, tileSize.x * m_size.x, tileSize.y * m_size.y ).Offset( pTiled->GetBaseOfs() );
	}
}

void CLevelEditPrefabToolEntityBrush::OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	float fGridSize = CLevelToolsView::Inst()->GetGridSize();
	CVector2 size1( ceil( m_bound.width / fGridSize ) * fGridSize, ceil( m_bound.height / fGridSize ) * fGridSize );
	CVector2 baseofs = size1 * 0.5f - m_bound.GetCenter();
	
	auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
	CVector2 p;
	if( !m_bDrag )
	{
		p = ( mousePos - size1 * 0.5f ) / fGridSize;
		TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
		p = CVector2( grid.x, grid.y ) * fGridSize;
	}
	else
	{
		p = ( mousePos - m_beginMousePos ) / size1;
		TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
		p = CVector2( grid.x, grid.y ) * size1 + m_beginBrushPos;
	}
	CVector2 entityPos = p + baseofs;
	CRectangle entityRect = m_bound.Offset( entityPos );

	CVector2 a( entityRect.x, entityRect.y );
	CVector2 b( entityRect.width, entityRect.height );
	CVector4 color( 1, 1, 1, 1 );
	CVector4 color1[] = { { 1, 0.8f, 0.2f, 1 }, { 0.3f, 0.8f, 0.7f, 1 } };
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for( int i = 0; i < 4; i++ )
	{
		pViewport->DebugDrawLine( pRenderSystem, p + size1 * ofs[i], p + size1 * ofs[( i + 1 ) % 4], color );
		pViewport->DebugDrawLine( pRenderSystem, a + b * ofs[i], a + b * ofs[( i + 1 ) % 4], color1[m_nMode] );
	}
}

bool CLevelEditPrefabToolEntityBrush::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	Brush( pViewport, mousePos, true );
	m_bDrag = true;
	return true;
}

void CLevelEditPrefabToolEntityBrush::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	Brush( pViewport, mousePos, false );
}

void CLevelEditPrefabToolEntityBrush::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	m_bDrag = false;
	m_brushedGrids.clear();
}

bool CLevelEditPrefabToolEntityBrush::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( m_bDrag )
		return true;
	if( pEvent->bKeyDown )
	{
		if( pEvent->nChar == 'Q' )
		{
			m_nMode = 1 - m_nMode;
			return true;
		}
		if( m_bTiled )
		{
			auto pTiled = SafeCastToInterface<IEditorTiled>( m_pObjData );
			if( pEvent->nChar == 'D' )
			{
				m_size.x = Min( m_size.x + 1, 100 );
				auto tileSize = pTiled->GetTileSize();
				m_bound = CRectangle( 0, 0, tileSize.x * m_size.x, tileSize.y * m_size.y ).Offset( pTiled->GetBaseOfs() );
			}
			if( pEvent->nChar == 'W' )
			{
				m_size.y = Min( m_size.y + 1, 100 );
				auto tileSize = pTiled->GetTileSize();
				m_bound = CRectangle( 0, 0, tileSize.x * m_size.x, tileSize.y * m_size.y ).Offset( pTiled->GetBaseOfs() );
			}
			if( pEvent->nChar == 'A' )
			{
				m_size.x = Max( m_size.x - 1, pTiled->GetMinSize().x );
				auto tileSize = pTiled->GetTileSize();
				m_bound = CRectangle( 0, 0, tileSize.x * m_size.x, tileSize.y * m_size.y ).Offset( pTiled->GetBaseOfs() );
			}
			if( pEvent->nChar == 'S' )
			{
				m_size.y = Max( m_size.y - 1, pTiled->GetMinSize().y );
				auto tileSize = pTiled->GetTileSize();
				m_bound = CRectangle( 0, 0, tileSize.x * m_size.x, tileSize.y * m_size.y ).Offset( pTiled->GetBaseOfs() );
			}
		}
	}
	return false;
}

void CLevelEditPrefabToolEntityBrush::Brush( CUIViewport* pViewport, const CVector2& mousePos, bool bBegin )
{
	float fGridSize = CLevelToolsView::Inst()->GetGridSize();
	CVector2 size1( ceil( m_bound.width / fGridSize ) * fGridSize, ceil( m_bound.height / fGridSize ) * fGridSize );
	CVector2 baseofs = size1 * 0.5f - m_bound.GetCenter();
	CVector2 p;
	if( bBegin )
	{
		p = ( mousePos - size1 * 0.5f ) / fGridSize;
		TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
		m_beginMousePos = mousePos;
		m_brushedGrids.insert( TVector2<int32>( 0, 0 ) );
		p = CVector2( grid.x, grid.y ) * fGridSize;
		m_beginBrushPos = p;
	}
	else
	{
		p = ( mousePos - m_beginMousePos ) / size1;
		TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
		if( m_brushedGrids.find( grid ) != m_brushedGrids.end() )
			return;
		m_brushedGrids.insert( grid );
		p = CVector2( grid.x, grid.y ) * size1 + m_beginBrushPos;
	}
	CVector2 entityPos = p + baseofs;
	CRectangle entityRect = m_bound.Offset( entityPos );

	if( m_nMode == 0 )
		CLevelToolsView::Inst()->Erase( CRectangle( entityRect.x + 0.01f, entityRect.y + 0.01f, entityRect.width - 0.02f, entityRect.height - 0.02f ) );
	auto pNode = CLevelToolsView::Inst()->AddObject( m_pPrefab, entityPos );
	if( m_bTiled )
	{
		auto pObjData1 = (CEntity*)pNode->GetPatchedNode()->GetStaticDataSafe<CEntity>();
		auto pTile = SafeCastToInterface<IEditorTiled>( pObjData1 );
		pTile->Resize( TRectangle<int32>( 0, 0, m_size.x, m_size.y ) );
	}
	pNode->OnEditorActive( false );
}

void CLevelEditPrefabToolEntityChunk::ToolBegin( CPrefab * pPrefab )
{
	__super::ToolBegin( pPrefab );
	auto pTile = SafeCastToInterface<IEditorTiled>( m_pObjData );
	auto size0 = pTile->GetTileSize() * CVector2( pTile->GetMinSize().x, pTile->GetMinSize().y );
	auto baseOfs = pTile->GetBaseOfs();
	m_bound = CRectangle( baseOfs.x, baseOfs.y, size0.x, size0.y );
}

void CLevelEditPrefabToolEntityChunk::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto pTile = SafeCastToInterface<IEditorTiled>( m_pObjData );
	float fGridSize = CLevelToolsView::Inst()->GetGridSize();
	auto tileSize = pTile->GetTileSize();
	CVector2 size1( ceil( m_bound.width / fGridSize ) * fGridSize, ceil( m_bound.height / fGridSize ) * fGridSize );
	CVector2 baseofs = size1 * 0.5f - m_bound.GetCenter();
	CVector2 entityPos;
	TVector2<int32> sizeOfs( 0, 0 );
	if( !m_bDrag )
	{
		auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
		CVector2 p = ( mousePos - size1 * 0.5f ) / fGridSize;
		TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
		m_beginMousePos = mousePos;
		p = CVector2( grid.x, grid.y ) * fGridSize;
		entityPos = p + baseofs;
	}
	else
	{
		entityPos = m_entityPos0;
		sizeOfs = m_curDragOfs;
	}

	TVector2<int32> size = pTile->GetMinSize();
	if( sizeOfs.x > 0 )
		size.x += sizeOfs.x;
	else
	{
		entityPos.x += sizeOfs.x * tileSize.x;
		size.x -= sizeOfs.x;
	}
	if( sizeOfs.y > 0 )
		size.y += sizeOfs.y;
	else
	{
		entityPos.y += sizeOfs.y * tileSize.y;
		size.y -= sizeOfs.y;
	}
	auto entitySize = tileSize * CVector2( size.x, size.y );
	auto entityRect = CRectangle( entityPos.x, entityPos.y, entitySize.x, entitySize.y ).Offset( pTile->GetBaseOfs() );

	CVector2 a( entityRect.x, entityRect.y );
	CVector2 b( entityRect.width, entityRect.height );
	CVector4 color( 1, 1, 1, 1 );
	CVector4 color1[] = { { 1, 0.8f, 0.2f, 1 }, { 0.3f, 0.8f, 0.7f, 1 } };
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for( int i = 0; i < 4; i++ )
	{
		pViewport->DebugDrawLine( pRenderSystem, a + b * ofs[i], a + b * ofs[( i + 1 ) % 4], color1[m_nMode] );
		pViewport->DebugDrawLine( pRenderSystem, a - CVector2( 8, 8 ) + ( b + CVector2( 8, 8 ) ) * ofs[i], a - CVector2( 8, 8 ) + ( b + CVector2( 8, 8 ) ) * ofs[( i + 1 ) % 4], color1[m_nMode] );
	}
}

bool CLevelEditPrefabToolEntityChunk::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	m_bDrag = true;
	float fGridSize = CLevelToolsView::Inst()->GetGridSize();
	CVector2 size1( ceil( m_bound.width / fGridSize ) * fGridSize, ceil( m_bound.height / fGridSize ) * fGridSize );
	CVector2 baseofs = size1 * 0.5f - m_bound.GetCenter();
	CVector2 p = ( mousePos - size1 * 0.5f ) / fGridSize;
	TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
	m_beginMousePos = mousePos;
	p = CVector2( grid.x, grid.y ) * fGridSize;
	m_entityPos0 = p + baseofs;
	m_curDragOfs = TVector2<int32>( 0, 0 );
	return true;
}

void CLevelEditPrefabToolEntityChunk::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( !m_bDrag )
		return;
	auto pTile = SafeCastToInterface<IEditorTiled>( m_pObjData );
	float fGridSize = CLevelToolsView::Inst()->GetGridSize();
	auto tileSize = pTile->GetTileSize();
	CVector2 p = ( mousePos - m_beginMousePos ) / tileSize;
	m_curDragOfs = TVector2<int32>( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
}

void CLevelEditPrefabToolEntityChunk::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( !m_bDrag )
		return;
	m_bDrag = false;
	auto pTile = SafeCastToInterface<IEditorTiled>( m_pObjData );
	float fGridSize = CLevelToolsView::Inst()->GetGridSize();
	auto tileSize = pTile->GetTileSize();
	CVector2 p = ( mousePos - m_beginMousePos ) / tileSize;
	TVector2<int32> ofs( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
	CVector2 entityPos = m_entityPos0;
	TVector2<int32> size = pTile->GetMinSize();
	if( ofs.x > 0 )
		size.x += ofs.x;
	else
	{
		entityPos.x += ofs.x * tileSize.x;
		size.x -= ofs.x;
	}
	if( ofs.y > 0 )
		size.y += ofs.y;
	else
	{
		entityPos.y += ofs.y * tileSize.y;
		size.y -= ofs.y;
	}
	auto entitySize = tileSize * CVector2( size.x, size.y );
	auto entityRect = CRectangle( entityPos.x, entityPos.y, entitySize.x, entitySize.y ).Offset( pTile->GetBaseOfs() );

	if( m_nMode == 0 )
		CLevelToolsView::Inst()->Erase( CRectangle( entityRect.x + 0.01f, entityRect.y + 0.01f, entityRect.width - 0.02f, entityRect.height - 0.02f ) );
	auto pNode = CLevelToolsView::Inst()->AddObject( m_pPrefab, entityPos );
	auto pObjData1 = (CEntity*)pNode->GetPatchedNode()->GetStaticDataSafe<CEntity>();
	SafeCastToInterface<IEditorTiled>( pObjData1 )->Resize( TRectangle<int32>( 0, 0, size.x, size.y ) );
	pNode->OnEditorActive( false );
}

bool CLevelEditPrefabToolEntityChunk::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( m_bDrag )
		return true;
	if( pEvent->bKeyDown )
	{
		if( pEvent->nChar == 'Q' )
		{
			m_nMode = 1 - m_nMode;
			return true;
		}
	}
	return false;
}


void CLevelEditPrefabToolEntityAttach::ToolBegin( CPrefab* pPrefab )
{
	__super::ToolBegin( pPrefab );
	m_bound = m_pObjData->GetBoundForEditor();
}

void CLevelEditPrefabToolEntityAttach::ToolEnd()
{
	__super::ToolEnd();
	m_pSelectedNode = NULL;
	m_bDrag = false;
}

void CLevelEditPrefabToolEntityAttach::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
	CPrefabNode* pNode = m_pSelectedNode;
	if( !pNode )
		pNode = CLevelToolsView::Inst()->Pick( mousePos );
	if( !pNode )
		return;
	auto pEntityData = (CEntity*)pNode->GetStaticDataSafe<CEntity>();
	if( !pEntityData )
		return;
	auto rect = pEntityData->GetBoundForEditor();
	CVector2 verts[] = { { rect.x, rect.y }, { rect.GetRight(), rect.y }, { rect.GetRight(), rect.GetBottom() }, { rect.x, rect.GetBottom() } };
	CMatrix2D trans;
	trans.Transform( pNode->x, pNode->y, pNode->r, pNode->s );
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = trans.MulVector2Pos( verts[i] );
	CVector4 color = m_pSelectedNode ? CVector4( 1, 1, 1, 1 ) : CVector4( 0.8f, 0.5f, 0.2f, 0.5f );
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, verts[i], verts[( i + 1 ) % 4], color );

	if( m_bDrag )
	{
		float fGridSize = CLevelToolsView::Inst()->GetGridSize();
		CVector2 size1( ceil( m_bound.width / fGridSize ) * fGridSize, ceil( m_bound.height / fGridSize ) * fGridSize );
		CVector2 baseofs = size1 * 0.5f - m_bound.GetCenter();

		CMatrix2D trans;
		trans.Transform( m_pSelectedNode->x, m_pSelectedNode->y, m_pSelectedNode->r, m_pSelectedNode->s );
		auto localMousePos = trans.MulTVector2Pos( mousePos );
		CVector2 p;
		p = ( localMousePos - size1 * 0.5f ) / fGridSize;
		TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
		p = CVector2( grid.x, grid.y ) * fGridSize;

		CVector2 entityPos = p + baseofs;
		CRectangle entityRect = m_bound.Offset( entityPos );

		CVector2 a( entityRect.x, entityRect.y );
		CVector2 b( entityRect.width, entityRect.height );
		CVector4 color( 1, 1, 1, 1 );
		CVector4 color1 = { 1, 0.8f, 0.2f, 1 };
		CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
		for( int i = 0; i < 4; i++ )
		{
			pViewport->DebugDrawLine( pRenderSystem, trans.MulVector2Pos( p + size1 * ofs[i] ), trans.MulVector2Pos( p + size1 * ofs[( i + 1 ) % 4] ), color );
			pViewport->DebugDrawLine( pRenderSystem, trans.MulVector2Pos( a + b * ofs[i] ), trans.MulVector2Pos( a + b * ofs[( i + 1 ) % 4] ), color1 );
		}
	}
}

bool CLevelEditPrefabToolEntityAttach::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto pNode = CLevelToolsView::Inst()->Pick( mousePos );
	if( !pNode )
		return false;
	if( pNode != m_pSelectedNode )
	{
		m_pSelectedNode = pNode;
		return true;
	}

	for( auto pChild = m_pSelectedNode->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pSelectedNode->GetRenderObject() )
			continue;

		auto pPrefabNode = static_cast<CPrefabNode*>( pChild );
		if( !pPrefabNode->GetPatchedNode() )
			continue;
		m_vecAllObjs.push_back( pPrefabNode );
	}
	m_bDrag = true;
	return true;
}

void CLevelEditPrefabToolEntityAttach::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( !m_bDrag )
		return;
}

void CLevelEditPrefabToolEntityAttach::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( !m_bDrag )
		return;

	CMatrix2D trans;
	trans.Transform( m_pSelectedNode->x, m_pSelectedNode->y, m_pSelectedNode->r, m_pSelectedNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );

	if( pViewport->GetMgr()->IsKey( 'Z' ) )
	{
		for( int32 i = 0; i < m_vecAllObjs.size(); i++ )
		{
			auto pNode = m_vecAllObjs[i];
			CEntity* pEntityData = (CEntity*)pNode->GetPatchedNode()->GetObjData();
			auto bound = pEntityData->GetBoundForEditor();
			CMatrix2D mat;
			mat.Transform( pNode->x, pNode->y, pNode->r, pNode->s );
			auto localPos = mat.MulTVector2Pos( localMousePos );
			if( bound.Contains( localPos ) && pEntityData->PickInEditor( localPos ) )
			{
				CLevelToolsView::Inst()->GetLevelNode()->NameSpaceClearNode( pNode );
				pNode->RemoveThis();
				break;
			}
		}
	}
	else
	{
		float fGridSize = CLevelToolsView::Inst()->GetGridSize();
		CVector2 size1( ceil( m_bound.width / fGridSize ) * fGridSize, ceil( m_bound.height / fGridSize ) * fGridSize );
		CVector2 baseofs = size1 * 0.5f - m_bound.GetCenter();
		CVector2 p;
		p = ( localMousePos - size1 * 0.5f ) / fGridSize;
		TVector2<int32> grid( floor( p.x + 0.5f ), floor( p.y + 0.5f ) );
		p = CVector2( grid.x, grid.y ) * fGridSize;

		CVector2 entityPos = p + baseofs;
		CRectangle entityRect = m_bound.Offset( entityPos );

		CPrefabNode* pNode = new CPrefabNode( m_pSelectedNode->GetPrefab() );
		pNode->SetResource( m_pPrefab );
		pNode->SetPosition( p );
		m_pSelectedNode->AddChild( pNode );
		pNode->OnEditorMove( m_pSelectedNode->GetPrefab()->GetRoot() );
		pNode->OnEditorActive( false );
	}
	m_bDrag = false;
	m_vecAllObjs.resize( 0 );
}

bool CLevelEditPrefabToolEntityAttach::OnViewportKey( SUIKeyEvent* pEvent )
{
	return false;
}


void CLevelEditObjectToolEntityDefault::ToolBegin( CPrefabNode* pPrefabNode )
{
	CLevelToolsView::Inst()->ShowObjEdit( true );
}

void CLevelEditObjectToolEntityDefault::ToolEnd()
{
	CLevelToolsView::Inst()->ShowObjEdit( false );
}


void CLevelEditObjectToolTerrain::ToolBegin( CPrefabNode* pPrefabNode )
{
	m_pPrefabNode = pPrefabNode;
	m_nEditType = 0;
	m_pObjData = (CTerrain*)pPrefabNode->GetStaticDataSafe<CTerrain>();
	CTileMap2D* pTileMap = GetTileMapData();
	pTileMap->SetTileSize( m_pObjData->m_tileSize );
	pTileMap->Resize( TRectangle<int32>( 0, 0, m_pObjData->m_nTileX - 1, m_pObjData->m_nTileY - 1 ) );
	pTileMap->SetBaseOffset( m_pObjData->m_ofs + m_pObjData->m_tileSize * 0.5f );
	m_selectedArea = TRectangle<int32>( 0, 0, 0, 0 );
	m_vecCopyData.resize( 0 );
	m_vecOrigData.resize( 0 );
}

void CLevelEditObjectToolTerrain::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto rect = m_pObjData->GetBoundForEditor();
	CVector2 verts[] = { { rect.x - 32, rect.y - 32 }, { rect.GetRight() + 32, rect.y - 32 },
	{ rect.GetRight() + 32, rect.GetBottom() + 32 }, { rect.x - 32, rect.GetBottom() + 32 } };
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = trans.MulVector2Pos( verts[i] );
	CVector4 color = CVector4( 0.8f, 0.8f, 0.2f, 0.6f );
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, verts[i], verts[( i + 1 ) % 4], color );
	if( m_nEditType == 0 )
		DebugDrawBrush( pViewport, pRenderSystem, localMousePos );
	else
		DebugDrawBlock( pViewport, pRenderSystem, localMousePos );
}

bool CLevelEditObjectToolTerrain::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( pViewport->GetMgr()->IsKey( VK_CONTROL ) )
		return false;
	auto rect = m_pObjData->GetBoundForEditor();
	auto tileSize = m_pObjData->GetTileSize();
	CRectangle rects[8] =
	{
		{ rect.GetRight(), rect.y, 32, rect.height },
		{ rect.x - 32, rect.y, 32, rect.height },
		{ rect.x, rect.GetBottom(), rect.width, 32 },
		{ rect.x, rect.y - 32, rect.width, 32 },
		{ rect.GetRight(), rect.GetBottom(), 32, 32 },
		{ rect.x - 32, rect.GetBottom(), 32, 32 },
		{ rect.GetRight(), rect.y - 32, 32, 32 },
		{ rect.x - 32, rect.y - 32, 32, 32 },
	};
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );

	if( m_nEditType == 0 )
	{
		if( OnBrush( localMousePos ) )
		{
			m_bDragging = true;
			return true;
		}

		for( int i = 0; i < 8; i++ )
		{
			if( rects[i].Contains( localMousePos ) )
			{
				m_nDragType1 = i;
				m_dragLocalMousePos = localMousePos;
				auto p = SafeCastToInterface<IEditorTiled>( m_pObjData );
				m_dragBeginSize = p->GetSize();
				m_dragCurSize = TRectangle<int32>( 0, 0, m_dragBeginSize.x, m_dragBeginSize.y );
				m_dragBeginOfs = p->GetBaseOfs();
				return true;
			}
		}
	}
	else
	{
		m_bDragging = true;
		m_nDragType1 = 0;
		m_dragLocalMousePos = localMousePos;
		m_dragBeginGrid = TVector2<int32>( floor( ( localMousePos.x - m_pObjData->GetBaseOfs().x ) / tileSize.x + 1 ), floor( ( localMousePos.y - m_pObjData->GetBaseOfs().y ) / tileSize.y + 1 ) );
		if( m_selectedArea.width && m_selectedArea.height )
		{
			CRectangle r( ( m_selectedArea.x - 0.5f ) * m_pObjData->GetTileSize().x + m_pObjData->GetBaseOfs().x,
				( m_selectedArea.y - 0.5f ) * m_pObjData->GetTileSize().y + m_pObjData->GetBaseOfs().y,
				m_selectedArea.width * m_pObjData->GetTileSize().x, m_selectedArea.height * m_pObjData->GetTileSize().y );
			if( r.Contains( localMousePos ) )
			{
				m_nDragType1 = 1;
				m_dragBeginArea = m_selectedArea;
			}
			else
			{
				RestoreTileData();
				m_vecCopyData.resize( 0 );
				m_vecOrigData.resize( 0 );
				m_selectedArea = TRectangle<int32>( 0, 0, 0, 0 );
			}
		}
		return true;
	}
	return false;
}

void CLevelEditObjectToolTerrain::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto p = SafeCastToInterface<IEditorTiled>( m_pObjData );
	auto tileSize = p->GetTileSize();
	TRectangle<int32> size1( 0, 0, m_dragBeginSize.x, m_dragBeginSize.y );

	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );

	if( m_nEditType == 0 )
	{
		if( m_bDragging )
		{
			OnBrush( localMousePos );
			return;
		}

		auto d = localMousePos - m_dragLocalMousePos;
		if( m_nDragType1 == 0 || m_nDragType1 == 4 || m_nDragType1 == 6 )
		{
			int32 dx = floor( d.x / tileSize.x + 0.5f );
			size1.width = Max( m_dragBeginSize.x + dx, 2 );
		}
		if( m_nDragType1 == 1 || m_nDragType1 == 5 || m_nDragType1 == 7 )
		{
			int32 dx = -floor( d.x / tileSize.x + 0.5f );
			size1.SetLeft( size1.GetRight() - Max( m_dragBeginSize.x + dx, 2 ) );
		}
		if( m_nDragType1 == 2 || m_nDragType1 == 4 || m_nDragType1 == 5 )
		{
			int32 dy = floor( d.y / tileSize.y + 0.5f );
			size1.height = Max( m_dragBeginSize.y + dy, 2 );
		}
		if( m_nDragType1 == 3 || m_nDragType1 == 6 || m_nDragType1 == 7 )
		{
			int32 dy = -floor( d.y / tileSize.y + 0.5f );
			size1.SetTop( size1.GetBottom() - Max( m_dragBeginSize.y + dy, 2 ) );
		}
		if( size1 != m_dragCurSize )
		{
			auto s = size1.Offset( TVector2<int32>( -m_dragCurSize.x, -m_dragCurSize.y ) );
			p->Resize( s );
			GetTileMapData()->Resize( TRectangle<int32>( s.x, s.y, s.width - 1, s.height - 1 ) );
			m_pPrefabNode->OnEdit();
			m_dragCurSize = size1;
		}
	}
	else
	{
		m_dragCurGrid = TVector2<int32>( floor( ( localMousePos.x - m_pObjData->GetBaseOfs().x ) / tileSize.x + 1 ), floor( ( localMousePos.y - m_pObjData->GetBaseOfs().y ) / tileSize.y + 1 ) );
		if( m_nDragType1 == 0 )
		{
			m_selectedArea = TRectangle<int32>( m_dragBeginGrid.x, m_dragBeginGrid.y, 0, 0 ) + TRectangle<int32>( m_dragCurGrid.x, m_dragCurGrid.y, 0, 0 );
		}
		else if( m_nDragType1 == 1 )
		{
			auto d = localMousePos - m_dragLocalMousePos;
			auto newSelectArea = m_dragBeginArea.Offset( TVector2<int32>( floor( d.x / tileSize.x + 0.5f ), floor( d.y / tileSize.y + 0.5f ) ) );
			if( newSelectArea != m_selectedArea )
			{
				RestoreTileData();
				m_selectedArea = newSelectArea;
				OverwriteTileData();
			}
		}
	}
}

void CLevelEditObjectToolTerrain::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_bDragging )
		m_bDragging = false;
	if( m_nEditType == 1 )
	{
		auto tileSize = m_pObjData->GetTileSize();
		CMatrix2D trans;
		trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
		auto localMousePos = trans.MulTVector2Pos( mousePos );

		m_dragCurGrid = TVector2<int32>( floor( ( localMousePos.x - m_pObjData->GetBaseOfs().x )/ tileSize.x + 1 ), floor( ( localMousePos.y - m_pObjData->GetBaseOfs().y ) / tileSize.y + 1 ) );

		if( m_nDragType1 == 0 )
		{
			m_selectedArea = TRectangle<int32>( m_dragBeginGrid.x, m_dragBeginGrid.y, 0, 0 ) + TRectangle<int32>( m_dragCurGrid.x, m_dragCurGrid.y, 0, 0 );
		}
		else if( m_nDragType1 == 1 )
		{
			auto d = localMousePos - m_dragLocalMousePos;
			auto newSelectArea = m_dragBeginArea.Offset( TVector2<int32>( floor( d.x / tileSize.x + 0.5f ), floor( d.y / tileSize.y + 0.5f ) ) );
			if( newSelectArea != m_selectedArea )
			{
				RestoreTileData();
				m_selectedArea = newSelectArea;
				OverwriteTileData();
			}
		}
	}
}

bool CLevelEditObjectToolTerrain::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( m_bDragging )
		return true;
	CTileMap2D* pTileMap = GetTileMapData();
	uint32 nEditTypeCount = pTileMap->GetInfo()->editInfos.size();

	char szTileKey[] = "QWERASDFZXCV";
	for( int i = 0; i < ELEM_COUNT( szTileKey ) - 1; i++ )
	{
		if( pEvent->nChar == szTileKey[i] )
		{
			if( i < nEditTypeCount )
				m_nCurBrushTileType = i;
			return true;
		}
	}
	if( pEvent->nChar == VK_TAB )
	{
		m_nEditType = 1 - m_nEditType;
		return true;
	}
	if( m_nEditType == 1 )
	{
		switch( pEvent->nChar )
		{
		case ' ':
			if( m_selectedArea.width && m_selectedArea.height )
			{
				if( !m_vecCopyData.size() )
					CopyTileData();
				else
				{
					m_vecCopyData.resize( 0 );
					m_vecOrigData.resize( 0 );
				}
			}
			return true;
		case VK_BACK:
			if( m_selectedArea.width && m_selectedArea.height )
			{
				if( m_vecCopyData.size() )
				{
					RestoreTileData();
					m_vecCopyData.resize( 0 );
					m_vecOrigData.resize( 0 );
				}
			}
			return true;
		}
	}
	else
	{
		switch( pEvent->nChar )
		{
		case 'B':
			m_nCurBrushTileType = m_nCurBrushTileType > 0 ? m_nCurBrushTileType - 1 : nEditTypeCount - 1;
			return true;
		case 'N':
			m_nCurBrushTileType = m_nCurBrushTileType < nEditTypeCount - 1 ? m_nCurBrushTileType + 1 : 0;
			return true;
		case 'T':
			m_nCurBrushSize = Min<int8>( m_nCurBrushSize + 1, 9 );
			return true;
		case 'G':
			m_nCurBrushSize = Max<int8>( m_nCurBrushSize - 1, 1 );
			return true;
		case 'Y':
			m_nCurBrushShape = m_nCurBrushShape < 2 ? m_nCurBrushShape + 1 : 0;
			return true;
		case 'H':
			m_nCurBrushShape = m_nCurBrushShape > 0 ? m_nCurBrushShape - 1 : 2;
			return true;
		}
	}
	return false;
}

CTileMap2D* CLevelEditObjectToolTerrain::GetTileMapData()
{
	return (CTileMap2D*)m_pPrefabNode->GetRenderObject();
}

bool CLevelEditObjectToolTerrain::OnBrush( const CVector2& localPos )
{
	CTileMap2D* pTileMap = GetTileMapData();
	CVector2 grid = ( localPos - pTileMap->GetBaseOffset() ) * CVector2( 1.0f / pTileMap->GetTileSize().x, 1.0f / pTileMap->GetTileSize().y );
	TRectangle<int32> rect( floor( grid.x + 1 - m_nCurBrushSize * 0.5f ), floor( grid.y + 1 - m_nCurBrushSize * 0.5f ),
		m_nCurBrushSize, m_nCurBrushSize );
	TVector2<int32> p0( rect.x + rect.GetRight(), rect.y + rect.GetBottom() );
	rect = rect * TRectangle<int32>( 0, 0, pTileMap->GetWidth() + 1, pTileMap->GetHeight() + 1 );
	if( rect.width > 0 && rect.height > 0 )
	{
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( m_nCurBrushShape == 1 )
				{
					int32 dx = x * 2 + 1 - p0.x;
					int32 dy = y * 2 + 1 - p0.y;
					if( dx * dx + dy * dy >( m_nCurBrushSize - 1 ) * ( m_nCurBrushSize - 1 ) + 1 )
						continue;
				}
				else if( m_nCurBrushShape == 2 )
				{
					if( abs( x * 2 + 1 - p0.x ) + abs( y * 2 + 1 - p0.y ) > m_nCurBrushSize )
						continue;
				}
				pTileMap->EditTile( x, y, m_nCurBrushTileType );
			}
		}
		return true;
	}
	return false;
}

void CLevelEditObjectToolTerrain::CopyTileData()
{
	if( !m_selectedArea.width || !m_selectedArea.height )
		return;

	auto pTileData = GetTileMapData();
	m_vecCopyData.resize( m_selectedArea.width * m_selectedArea.height );
	m_vecOrigData.resize( m_selectedArea.width * m_selectedArea.height );
	for( int i = 0; i < m_selectedArea.width; i++ )
	{
		for( int j = 0; j < m_selectedArea.height; j++ )
		{
			auto x = Min<int32>( pTileData->GetWidth(), Max( 0, i + m_selectedArea.x ) );
			auto y = Min<int32>( pTileData->GetHeight(), Max( 0, j + m_selectedArea.y ) );
			m_vecOrigData[i + j * m_selectedArea.width] = m_vecCopyData[i + j * m_selectedArea.width] = pTileData->GetEditData( x, y );
		}
	}
}

void CLevelEditObjectToolTerrain::OverwriteTileData()
{
	if( !m_vecCopyData.size() )
		return;

	auto pTileData = GetTileMapData();
	for( int i = 0; i < m_selectedArea.width; i++ )
	{
		for( int j = 0; j < m_selectedArea.height; j++ )
		{
			auto x = i + m_selectedArea.x;
			auto y = j + m_selectedArea.y;
			if( x >= 0 && y >= 0 && x <= pTileData->GetWidth() && y <= pTileData->GetHeight() )
			{
				m_vecOrigData[i + j * m_selectedArea.width] = pTileData->GetEditData( x, y );
				pTileData->SetEditData( x, y, m_vecCopyData[i + j * m_selectedArea.width] );
			}
		}
	}
	TRectangle<int32> refreshRect( m_selectedArea.x - 1, m_selectedArea.y - 1, m_selectedArea.width + 1, m_selectedArea.height + 1 );
	refreshRect = refreshRect * TRectangle<int32>( 0, 0, pTileData->GetWidth(), pTileData->GetHeight() );
	for( int i = 0; i < refreshRect.width; i++ )
	{
		for( int j = 0; j < refreshRect.height; j++ )
		{
			pTileData->RefreshTile( i + refreshRect.x, j + refreshRect.y );
		}
	}
}

void CLevelEditObjectToolTerrain::RestoreTileData()
{
	if( !m_vecOrigData.size() )
		return;

	auto pTileData = GetTileMapData();
	for( int i = 0; i < m_selectedArea.width; i++ )
	{
		for( int j = 0; j < m_selectedArea.height; j++ )
		{
			auto x = i + m_selectedArea.x;
			auto y = j + m_selectedArea.y;
			if( x >= 0 && y >= 0 && x <= pTileData->GetWidth() && y <= pTileData->GetHeight() )
				pTileData->SetEditData( x, y, m_vecOrigData[i + j * m_selectedArea.width] );
		}
	}
	TRectangle<int32> refreshRect( m_selectedArea.x - 1, m_selectedArea.y - 1, m_selectedArea.width + 1, m_selectedArea.height + 1 );
	refreshRect = refreshRect * TRectangle<int32>( 0, 0, pTileData->GetWidth(), pTileData->GetHeight() );
	for( int i = 0; i < refreshRect.width; i++ )
	{
		for( int j = 0; j < refreshRect.height; j++ )
		{
			pTileData->RefreshTile( i + refreshRect.x, j + refreshRect.y );
		}
	}
}

void CLevelEditObjectToolTerrain::DebugDrawBrush( CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CVector2& localPos )
{
	CTileMap2D* pTileMap = GetTileMapData();
	CVector2 grid = ( localPos - pTileMap->GetBaseOffset() ) * CVector2( 1.0f / pTileMap->GetTileSize().x, 1.0f / pTileMap->GetTileSize().y );
	TRectangle<int32> rect( floor( grid.x + 1 - m_nCurBrushSize * 0.5f ), floor( grid.y + 1 - m_nCurBrushSize * 0.5f ),
		m_nCurBrushSize, m_nCurBrushSize );
	TVector2<int32> p0( rect.x + rect.GetRight(), rect.y + rect.GetBottom() );
	rect = rect * TRectangle<int32>( 0, 0, pTileMap->GetWidth() + 1, pTileMap->GetHeight() + 1 );
	if( rect.width > 0 && rect.height > 0 )
	{
		CVector2 ofs[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
		CVector4 colors[] = { { 1, 1, 0, 1 }, { 0, 1, 0, 1 }, { 0, 1, 1, 0 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 }, { 0.5, 0.5, 0.5, 1 } };
		CVector4 color = colors[m_nCurBrushTileType % ELEM_COUNT( colors )];
		CMatrix2D trans;
		trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
		for( int x = rect.x; x < rect.GetRight(); x++ )
		{
			for( int y = rect.y; y < rect.GetBottom(); y++ )
			{
				if( m_nCurBrushShape == 1 )
				{
					int32 dx = x * 2 + 1 - p0.x;
					int32 dy = y * 2 + 1 - p0.y;
					if( dx * dx + dy * dy > ( m_nCurBrushSize - 1 ) * ( m_nCurBrushSize - 1 ) + 1 )
						continue;
				}
				else if( m_nCurBrushShape == 2 )
				{
					if( abs( x * 2 + 1 - p0.x ) + abs( y * 2 + 1 - p0.y ) > m_nCurBrushSize )
						continue;
				}

				auto p = CVector2( x - 0.5f, y - 0.5f ) * pTileMap->GetTileSize() + pTileMap->GetBaseOffset();
				for( int i = 0; i < 4; i++ )
				{
					auto a = p + ofs[i] * pTileMap->GetTileSize();
					auto b = p + ofs[( i + 1 ) % 4] * pTileMap->GetTileSize();
					pViewport->DebugDrawLine( pRenderSystem, trans.MulVector2Pos( a ), trans.MulVector2Pos( b ), color );
				}
			}
		}
	}
}

void CLevelEditObjectToolTerrain::DebugDrawBlock( CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CVector2& localPos )
{
	if( m_selectedArea.width && m_selectedArea.height )
	{
		CMatrix2D trans;
		trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );

		CRectangle rect( ( m_selectedArea.x - 0.5f ) * m_pObjData->GetTileSize().x + m_pObjData->GetBaseOfs().x,
			( m_selectedArea.y - 0.5f ) * m_pObjData->GetTileSize().y + m_pObjData->GetBaseOfs().y,
			m_selectedArea.width * m_pObjData->GetTileSize().x, m_selectedArea.height * m_pObjData->GetTileSize().y );
		CVector2 verts[] = { { rect.x, rect.y }, { rect.GetRight(), rect.y },
		{ rect.GetRight(), rect.GetBottom() }, { rect.x, rect.GetBottom() } };
		for( int i = 0; i < ELEM_COUNT( verts ); i++ )
			verts[i] = trans.MulVector2Pos( verts[i] );
		CVector4 color = CVector4( 0.3f, 0.6f, 0.9f, 1 );
		for( int i = 0; i < 4; i++ )
			pViewport->DebugDrawLine( pRenderSystem, verts[i], verts[( i + 1 ) % 4], color );

		if( m_vecCopyData.size() )
		{
			rect = CRectangle( rect.x - 8, rect.y - 8, rect.width + 16, rect.height + 16 );
			CVector2 verts1[] = { { rect.x, rect.y }, { rect.GetRight(), rect.y },
			{ rect.GetRight(), rect.GetBottom() }, { rect.x, rect.GetBottom() } };
			for( int i = 0; i < ELEM_COUNT( verts1 ); i++ )
				verts1[i] = trans.MulVector2Pos( verts1[i] );
			for( int i = 0; i < 4; i++ )
				pViewport->DebugDrawLine( pRenderSystem, verts1[i], verts[( i + 1 ) % 4], color );
		}
	}
}


void CLevelEditObjectQuickToolEntityResize::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto rect = m_pObjData->GetBoundForEditor();
	CVector2 verts[] = { { rect.x - 32, rect.y - 32 }, { rect.GetRight() + 32, rect.y - 32 },
	{ rect.GetRight() + 32, rect.GetBottom() + 32 }, { rect.x - 32, rect.GetBottom() + 32 } };
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = trans.MulVector2Pos( verts[i] );
	CVector4 color = CVector4( 0.8f, 0.8f, 0.2f, 0.6f );
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, verts[i], verts[( i + 1 ) % 4], color );
}

bool CLevelEditObjectQuickToolEntityResize::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto rect = m_pObjData->GetBoundForEditor();
	CRectangle rects[8] =
	{
		{ rect.GetRight(), rect.y, 32, rect.height },
		{ rect.x - 32, rect.y, 32, rect.height },
		{ rect.x, rect.GetBottom(), rect.width, 32 },
		{ rect.x, rect.y - 32, rect.width, 32 },
		{ rect.GetRight(), rect.GetBottom(), 32, 32 },
		{ rect.x - 32, rect.GetBottom(), 32, 32 },
		{ rect.GetRight(), rect.y - 32, 32, 32 },
		{ rect.x - 32, rect.y - 32, 32, 32 },
	};
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	for( int i = 0; i < 8; i++ )
	{
		if( rects[i].Contains( localMousePos ) )
		{
			m_nDragType = i;
			m_dragLocalMousePos = localMousePos;
			auto p = SafeCastToInterface<IEditorTiled>( m_pObjData );
			m_dragBeginSize = p->GetSize();
			m_dragCurSize = TRectangle<int32>( 0, 0, m_dragBeginSize.x, m_dragBeginSize.y );
			m_dragBeginOfs = p->GetBaseOfs();
			return true;
		}
	}
	return false;
}

void CLevelEditObjectQuickToolEntityResize::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto p = SafeCastToInterface<IEditorTiled>( m_pObjData );
	auto tileSize = p->GetTileSize();
	TRectangle<int32> size1( 0, 0, m_dragBeginSize.x, m_dragBeginSize.y );

	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	auto d = localMousePos - m_dragLocalMousePos;
	if( m_nDragType == 0 || m_nDragType == 4 || m_nDragType == 6 )
	{
		int32 dx = floor( d.x / tileSize.x + 0.5f );
		size1.width = Max( m_dragBeginSize.x + dx, 0 );
	}
	if( m_nDragType == 1 || m_nDragType == 5 || m_nDragType == 7 )
	{
		int32 dx = -floor( d.x / tileSize.x + 0.5f );
		size1.SetLeft( size1.GetRight() - Max( m_dragBeginSize.x + dx, 0 ) );
	}
	if( m_nDragType == 2 || m_nDragType == 4 || m_nDragType == 5 )
	{
		int32 dy = floor( d.y / tileSize.y + 0.5f );
		size1.height = Max( m_dragBeginSize.y + dy, 0 );
	}
	if( m_nDragType == 3 || m_nDragType == 6 || m_nDragType == 7 )
	{
		int32 dy = -floor( d.y / tileSize.y + 0.5f );
		size1.SetTop( size1.GetBottom() - Max( m_dragBeginSize.y + dy, 0 ) );
	}
	if( size1 != m_dragCurSize )
	{
		m_pPrefabNode->OnEditorActive( true );
		auto s = size1.Offset( TVector2<int32>( -m_dragCurSize.x, -m_dragCurSize.y ) );
		p->Resize( s );
		m_pPrefabNode->GetPatchedNode()->OnEdit();
		m_pPrefabNode->OnEditorActive( false );
		m_dragCurSize = size1;
	}
}

void CLevelEditObjectQuickToolAlertTriggerResize::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto rect = m_pObjData->m_alertRect;
	CVector2 verts[] = { { rect.x - 32, rect.y - 32 }, { rect.GetRight() + 32, rect.y - 32 },
	{ rect.GetRight() + 32, rect.GetBottom() + 32 }, { rect.x - 32, rect.GetBottom() + 32 } };
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = trans.MulVector2Pos( verts[i] );
	CVector4 color = CVector4( 0.8f, 0.8f, 0.2f, 0.6f );
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, verts[i], verts[( i + 1 ) % 4], color );
}

bool CLevelEditObjectQuickToolAlertTriggerResize::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto rect = m_pObjData->m_alertRect;
	CRectangle rects[8] =
	{
		{ rect.GetRight(), rect.y, 32, rect.height },
		{ rect.x - 32, rect.y, 32, rect.height },
		{ rect.x, rect.GetBottom(), rect.width, 32 },
		{ rect.x, rect.y - 32, rect.width, 32 },
		{ rect.GetRight(), rect.GetBottom(), 32, 32 },
		{ rect.x - 32, rect.GetBottom(), 32, 32 },
		{ rect.GetRight(), rect.y - 32, 32, 32 },
		{ rect.x - 32, rect.y - 32, 32, 32 },
	};
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	for( int i = 0; i < 8; i++ )
	{
		if( rects[i].Contains( localMousePos ) )
		{
			m_nDragType = i;
			m_dragLocalMousePos = localMousePos;
			auto size0 = m_pObjData->m_alertRect;
			m_dragBeginSize = TVector2<int32>( size0.width / 32, size0.height / 32 );
			m_dragCurSize = TRectangle<int32>( 0, 0, m_dragBeginSize.x, m_dragBeginSize.y );
			return true;
		}
	}
	return false;
}

void CLevelEditObjectQuickToolAlertTriggerResize::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	CVector2 tileSize( 32, 32 );
	TRectangle<int32> size1( 0, 0, m_dragBeginSize.x, m_dragBeginSize.y );

	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	auto d = localMousePos - m_dragLocalMousePos;
	if( m_nDragType == 0 || m_nDragType == 4 || m_nDragType == 6 )
	{
		int32 dx = floor( d.x / tileSize.x + 0.5f );
		size1.width = Max( m_dragBeginSize.x + dx, 0 );
	}
	if( m_nDragType == 1 || m_nDragType == 5 || m_nDragType == 7 )
	{
		int32 dx = -floor( d.x / tileSize.x + 0.5f );
		size1.SetLeft( size1.GetRight() - Max( m_dragBeginSize.x + dx, 0 ) );
	}
	if( m_nDragType == 2 || m_nDragType == 4 || m_nDragType == 5 )
	{
		int32 dy = floor( d.y / tileSize.y + 0.5f );
		size1.height = Max( m_dragBeginSize.y + dy, 0 );
	}
	if( m_nDragType == 3 || m_nDragType == 6 || m_nDragType == 7 )
	{
		int32 dy = -floor( d.y / tileSize.y + 0.5f );
		size1.SetTop( size1.GetBottom() - Max( m_dragBeginSize.y + dy, 0 ) );
	}
	if( size1 != m_dragCurSize )
	{
		m_pPrefabNode->OnEditorActive( true );
		auto s = size1.Offset( TVector2<int32>( -m_dragCurSize.x, -m_dragCurSize.y ) );
		auto& r = m_pObjData->m_alertRect;
		r = CRectangle( r.x + s.x * 32, r.y + s.y * 32, s.width * 32, s.height * 32 );
		m_pPrefabNode->GetPatchedNode()->OnEdit();
		m_pPrefabNode->OnEditorActive( false );
		m_dragCurSize = size1;
	}
}

void CLevelEditObjectQuickToolGate::ToolBegin( CPrefabNode * pPrefabNode )
{
	__super::ToolBegin( pPrefabNode );
	m_baseSize = m_pObjData->GetBoundForEditor();
}

void CLevelEditObjectQuickToolGate::OnDebugDraw( CUIViewport * pViewport, IRenderSystem * pRenderSystem )
{
	CVector2 ofs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	auto rect = m_baseSize.Offset( ofs[m_pObjData->m_nDir] * m_pObjData->m_fMaxLength );

	CVector2 verts[] = { { rect.x - 32, rect.y - 32 }, { rect.GetRight() + 32, rect.y - 32 },
	{ rect.GetRight() + 32, rect.GetBottom() + 32 }, { rect.x - 32, rect.GetBottom() + 32 } };
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = trans.MulVector2Pos( verts[i] );
	CVector4 color = CVector4( 0.8f, 0.8f, 0.2f, 0.6f );
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, verts[i], verts[( i + 1 ) % 4], color );
}

bool CLevelEditObjectQuickToolGate::OnViewportStartDrag( CUIViewport * pViewport, const CVector2 & mousePos )
{
	CVector2 ofs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	auto d = ofs[m_pObjData->m_nDir] * m_pObjData->m_fMaxLength;
	auto rect = m_baseSize.Offset( d );

	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );

	if( rect.Contains( localMousePos ) )
	{
		m_nDragType = 1;
		m_dragLocalMousePos = localMousePos;
		m_dragBeginSize = m_dragCurSize = TVector2<int32>( floor( d.x / rect.width * 2 + 0.5f ), floor( d.y / rect.height * 2 + 0.5f ) );
		return true;
	}

	return false;
}

void CLevelEditObjectQuickToolGate::OnViewportDragged( CUIViewport * pViewport, const CVector2 & mousePos )
{
	CVector2 tileSize( m_baseSize.width / 2, m_baseSize.height / 2 );
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	auto d = localMousePos - m_dragLocalMousePos;
	auto size1 = m_dragBeginSize + TVector2<int32>( floor( d.x / tileSize.x + 0.5f ), floor( d.y / tileSize.y + 0.5f ) );
	if( abs( localMousePos.x ) < abs( localMousePos.y ) )
		size1.x = 0;
	else
		size1.y = 0;

	if( size1 != m_dragCurSize )
	{
		m_pPrefabNode->OnEditorActive( true );
		if( size1.y == 0 )
		{
			if( size1.x < 0 )
			{
				m_pObjData->m_nDir = 2;
				m_pObjData->m_fMaxLength = -size1.x * tileSize.x;
			}
			else
			{
				m_pObjData->m_nDir = 0;
				m_pObjData->m_fMaxLength = size1.x * tileSize.x;
			}
		}
		else
		{
			if( size1.y < 0 )
			{
				m_pObjData->m_nDir = 3;
				m_pObjData->m_fMaxLength = -size1.y * tileSize.y;
			}
			else
			{
				m_pObjData->m_nDir = 1;
				m_pObjData->m_fMaxLength = size1.y * tileSize.y;
			}
		}
		m_pPrefabNode->GetPatchedNode()->OnEdit();
		m_pPrefabNode->OnEditorActive( false );
		m_dragCurSize = size1;
	}
}

void CLevelEditObjectQuickToolChunk1::OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	__super::OnDebugDraw( pViewport, pRenderSystem );
	auto p = (CChunk1*)m_pObjData;
	auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	auto tileSize = p->GetTileSize();
	auto base = p->GetBaseOfs();
	CVector2 grid = ( localMousePos - base ) * CVector2( 1.0f / tileSize.x, 1.0f / tileSize.y );
	int32 x = floor( grid.x );
	int32 y = floor( grid.y );
	if( x < 0 || y < 0 || x >= p->GetSize().x || y >= p->GetSize().y )
		return;

	CVector4 colors[] = { { 1, 1, 0, 1 }, { 0, 1, 0, 1 }, { 0, 1, 1, 0 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 }, { 0.5, 0.5, 0.5, 1 } };
	CVector4 colorX = p->GetColData( x ) >= 0 ? colors[p->GetColData( x ) % ELEM_COUNT( colors )] : CVector4( 0, 0, 0, 1 );
	CVector4 colorY = p->GetRowData( y ) >= 0 ? colors[p->GetRowData( y ) % ELEM_COUNT( colors )] : CVector4( 0, 0, 0, 1 );

	pViewport->DebugDrawLine( pRenderSystem, trans.MulVector2Pos( CVector2( x + 0.5f, 0 ) * tileSize + base ),
		trans.MulVector2Pos( CVector2( x + 0.5f, p->GetSize().y ) * tileSize + base ), colorX );
	pViewport->DebugDrawLine( pRenderSystem, trans.MulVector2Pos( CVector2( 0, y + 0.5f ) * tileSize + base ),
		trans.MulVector2Pos( CVector2( p->GetSize().x, y + 0.5f ) * tileSize + base ), colorY );
}

bool CLevelEditObjectQuickToolChunk1::OnViewportKey( SUIKeyEvent* pEvent )
{
	auto p = (CChunk1*)m_pObjData;
	auto mousePos = CLevelToolsView::Inst()->GetViewportMousePos();
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	auto tileSize = p->GetTileSize();
	CVector2 grid = ( localMousePos - p->GetBaseOfs() ) * CVector2( 1.0f / tileSize.x, 1.0f / tileSize.y );
	int32 x = floor( grid.x );
	int32 y = floor( grid.y );
	if( x < 0 || y < 0 || x >= p->GetSize().x || y >= p->GetSize().y )
		return false;
	if( pEvent->bKeyDown )
	{
		if( pEvent->nChar == 'E' )
		{
			auto n = p->GetEditGroupIndex();
			n = ( n + 1 ) % p->GetEditGroupCount();
			m_pPrefabNode->OnEditorActive( true );
			p->SetEditGroup( n );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
		else if( pEvent->nChar == 'Q' )
		{
			auto n = p->GetEditGroupIndex();
			n = ( n - 1 + p->GetEditGroupCount() ) % p->GetEditGroupCount();
			m_pPrefabNode->OnEditorActive( true );
			p->SetEditGroup( n );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
		else if( pEvent->nChar == 'D' )
		{
			auto n = Max( 0, p->GetColData( x ) );
			n = ( n + 1 ) % p->GetEditGroup().nTexX;
			m_pPrefabNode->OnEditorActive( true );
			p->SetColType( x, n );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
		else if( pEvent->nChar == 'A' )
		{
			auto n = Max( 0, p->GetColData( x ) );
			n = ( n - 1 + p->GetEditGroup().nTexX ) % p->GetEditGroup().nTexX;
			m_pPrefabNode->OnEditorActive( true );
			p->SetColType( x, n );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
		else if( pEvent->nChar == 'S' )
		{
			auto n = Max( 0, p->GetRowData( y ) );
			n = ( n + 1 ) % p->GetEditGroup().nTexY;
			m_pPrefabNode->OnEditorActive( true );
			p->SetRowType( y, n );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
		else if( pEvent->nChar == 'W' )
		{
			auto n = Max( 0, p->GetRowData( y ) );
			n = ( n - 1 + p->GetEditGroup().nTexY ) % p->GetEditGroup().nTexY;
			m_pPrefabNode->OnEditorActive( true );
			p->SetRowType( y, n );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
		else if( pEvent->nChar == 'R' )
		{
			m_pPrefabNode->OnEditorActive( true );
			p->SetColType( x, -1 );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
		else if( pEvent->nChar == 'F' )
		{
			m_pPrefabNode->OnEditorActive( true );
			p->SetRowType( y, -1 );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
			return true;
		}
	}
	return false;
}

CRenderObject2D* CLevelEditObjectQuickToolChunk2::CreateToolPreview()
{
	auto p = (CEntity*)m_pPrefabNode->GetPatchedNode()->CreateInstance( false );
	auto rect = ( (CImage2D*)p->GetRenderObject() )->GetElem().rect;
	float k = Max( rect.width / 400, rect.height / 300 );
	int32 nScale = 1;
	for( ; nScale < k; nScale *= 2 );
	rect = rect * ( 1.0f / nScale );
	( (CImage2D*)p->GetRenderObject() )->SetRect( rect );
	p->SetPosition( rect.GetCenter() * -1 );
	rect.SetCenter( CVector2( 0, 0 ) );
	m_previewImgRect = rect;
	m_nSelectedX = m_nSelectedY = 0;
	return p;
}

void CLevelEditObjectQuickToolChunk2::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	__super::OnDebugDraw( pViewport, pRenderSystem );
	auto mousePos = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );

	auto nTexX = ( (CChunk2*)m_pObjData )->GetTexX();
	auto nTexY = ( (CChunk2*)m_pObjData )->GetTexY();
	auto pChunk = SafeCast<CChunk2>( m_pObjData );
	CRectangle rect;
	if( m_nDragType <= 1 )
	{
		CVector2 grid = ( localMousePos - pChunk->GetBaseOfs() ) * CVector2( 1.0f / pChunk->GetTileSize().x, 1.0f / pChunk->GetTileSize().y );
		TVector2<int32> p( floor( grid.x ), floor( grid.y ) );
		if( !( p.x >= 0 && p.y >= 0 && p.x < pChunk->GetSize().x && p.y < pChunk->GetSize().y ) )
			return;
		rect = CRectangle( floor( grid.x ) * pChunk->GetTileSize().x, floor( grid.y ) * pChunk->GetTileSize().y, pChunk->GetTileSize().x, pChunk->GetTileSize().y );
		rect = rect.Offset( pChunk->GetBaseOfs() );
	}
	else
	{
		CVector2 grid = ( localMousePos - pChunk->GetBaseOfs() ) * CVector2( 1.0f / pChunk->GetTileSize().x, 1.0f / pChunk->GetTileSize().y );
		TVector2<int32> p( floor( grid.x ), floor( grid.y ) );
		CVector2 grid1 = ( m_dragBeginPos - pChunk->GetBaseOfs() ) * CVector2( 1.0f / pChunk->GetTileSize().x, 1.0f / pChunk->GetTileSize().y );
		TVector2<int32> p1( floor( grid1.x ), floor( grid1.y ) );

		auto r0 = TRectangle<int32>( p.x, p.y, 1, 1 ) + TRectangle<int32>( p1.x, p1.y, 1, 1 );
		r0 = r0 * TRectangle<int32>( 0, 0, pChunk->GetSize().x, pChunk->GetSize().y );
		rect = CRectangle( r0.x * pChunk->GetTileSize().x, r0.y * pChunk->GetTileSize().y, r0.width * pChunk->GetTileSize().x, r0.height * pChunk->GetTileSize().y );
		rect = rect.Offset( pChunk->GetBaseOfs() );
	}

	CVector2 a( rect.x, rect.y );
	CVector2 b( rect.width, rect.height );
	CVector4 color( 1, 1, 1, 1 );
	CVector4 color1[] = { { 1, 0.8f, 0.2f, 1 }, { 0.3f, 0.8f, 0.7f, 1 } };
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, trans.MulVector2Pos( a + b * ofs[i] ), trans.MulVector2Pos( a + b * ofs[( i + 1 ) % 4] ), CVector4( 1, 0.8f, 0.2f, 1 ) );
}

bool CLevelEditObjectQuickToolChunk2::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto pChunk = SafeCast<CChunk2>( m_pObjData );
	CMatrix2D trans;
	trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
	auto localMousePos = trans.MulTVector2Pos( mousePos );
	CVector2 grid = ( localMousePos - pChunk->GetBaseOfs() ) * CVector2( 1.0f / pChunk->GetTileSize().x, 1.0f / pChunk->GetTileSize().y );
	TVector2<int32> p( floor( grid.x ), floor( grid.y ) );
	if( p.x >= 0 && p.y >= 0 && p.x < pChunk->GetSize().x && p.y < pChunk->GetSize().y )
	{
		m_dragBeginPos = localMousePos;
		if( pViewport->GetMgr()->IsKey( VK_CONTROL ) )
		{
			m_nDragType = 2;
			return true;
		}
		if( pViewport->GetMgr()->IsKey( VK_SHIFT ) )
		{
			m_nDragType = 3;
			return true;
		}

		auto& arrData = pChunk->GetData();
		m_pPrefabNode->OnEditorActive( true );
		arrData[p.x + p.y * pChunk->GetSize().x] = m_nSelectedX + ( m_nSelectedY << 16 );
		m_pPrefabNode->GetPatchedNode()->OnEdit();
		m_pPrefabNode->OnEditorActive( false );
		m_nDragType = 1;
		return true;
	}

	return __super::OnViewportStartDrag( pViewport, mousePos );
}

void CLevelEditObjectQuickToolChunk2::OnViewportDragged( CUIViewport * pViewport, const CVector2 & mousePos )
{
	if( !m_nDragType )
	{
		__super::OnViewportDragged( pViewport, mousePos );
		return;
	}
	if( m_nDragType == 1 )
	{
		auto pChunk = SafeCast<CChunk2>( m_pObjData );
		CMatrix2D trans;
		trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
		auto localMousePos = trans.MulTVector2Pos( mousePos );
		CVector2 grid = ( localMousePos - pChunk->GetBaseOfs() ) * CVector2( 1.0f / pChunk->GetTileSize().x, 1.0f / pChunk->GetTileSize().y );
		TVector2<int32> p( floor( grid.x ), floor( grid.y ) );
		if( p.x >= 0 && p.y >= 0 && p.x < pChunk->GetSize().x && p.y < pChunk->GetSize().y )
		{
			auto& arrData = pChunk->GetData();
			m_pPrefabNode->OnEditorActive( true );
			arrData[p.x + p.y * pChunk->GetSize().x] = m_nSelectedX + ( m_nSelectedY << 16 );
			m_pPrefabNode->GetPatchedNode()->OnEdit();
			m_pPrefabNode->OnEditorActive( false );
		}
	}
}

void CLevelEditObjectQuickToolChunk2::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( !m_nDragType )
	{
		__super::OnViewportStopDrag( pViewport, mousePos );
		return;
	}
	if( m_nDragType > 1 )
	{
		auto pChunk = SafeCast<CChunk2>( m_pObjData );
		CMatrix2D trans;
		trans.Transform( m_pPrefabNode->x, m_pPrefabNode->y, m_pPrefabNode->r, m_pPrefabNode->s );
		auto localMousePos = trans.MulTVector2Pos( mousePos );
		auto& arrData = pChunk->GetData();
		CVector2 grid = ( localMousePos - pChunk->GetBaseOfs() ) * CVector2( 1.0f / pChunk->GetTileSize().x, 1.0f / pChunk->GetTileSize().y );
		TVector2<int32> p( floor( grid.x ), floor( grid.y ) );
		CVector2 grid1 = ( m_dragBeginPos - pChunk->GetBaseOfs() ) * CVector2( 1.0f / pChunk->GetTileSize().x, 1.0f / pChunk->GetTileSize().y );
		TVector2<int32> p1( floor( grid1.x ), floor( grid1.y ) );

		m_pPrefabNode->OnEditorActive( true );
		auto rect = TRectangle<int32>( p.x, p.y, 1, 1 ) + TRectangle<int32>( p1.x, p1.y, 1, 1 );
		rect = rect * TRectangle<int32>( 0, 0, pChunk->GetSize().x, pChunk->GetSize().y );
		for( int i = rect.x; i < rect.GetRight(); i++ )
		{
			for( int j = rect.y; j < rect.GetBottom(); j++ )
			{
				if( m_nDragType == 2 )
				{
					arrData[i + j * pChunk->GetSize().x] = m_nSelectedX + ( m_nSelectedY << 16 );
				}
				else
				{
					auto x = m_nSelectedX + ( i - p1.x );
					auto y = m_nSelectedY + ( p1.y - j );
					if( x < 0 || y < 0 || x >= pChunk->GetTexX() || y >= pChunk->GetTexY() )
						continue;
					arrData[i + j * pChunk->GetSize().x] = x + ( y << 16 );
				}
			}
		}
		m_pPrefabNode->GetPatchedNode()->OnEdit();
		m_pPrefabNode->OnEditorActive( false );
	}
	m_nDragType = 0;
}

bool CLevelEditObjectQuickToolChunk2::OnViewportKey( SUIKeyEvent* pEvent )
{
	return false;
}

void CLevelEditObjectQuickToolChunk2::OnPreviewDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto nTexX = ( (CChunk2*)m_pObjData )->GetTexX();
	auto nTexY = ( (CChunk2*)m_pObjData )->GetTexY();
	CRectangle rect( m_previewImgRect.x, m_previewImgRect.y, m_previewImgRect.width / nTexX, m_previewImgRect.height / nTexY );
	rect.x += m_nSelectedX * rect.width;
	rect.y += ( nTexY - 1 - m_nSelectedY ) * rect.height;

	CVector2 a( rect.x, rect.y );
	CVector2 b( rect.width, rect.height );
	CVector4 color( 1, 1, 1, 1 );
	CVector4 color1[] = { { 1, 0.8f, 0.2f, 1 }, { 0.3f, 0.8f, 0.7f, 1 } };
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for( int i = 0; i < 4; i++ )
		pViewport->DebugDrawLine( pRenderSystem, a + b * ofs[i], a + b * ofs[( i + 1 ) % 4], CVector4( 1, 0.8f, 0.2f, 1 ) );
}

bool CLevelEditObjectQuickToolChunk2::OnPreviewViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto nTexX = ( (CChunk2*)m_pObjData )->GetTexX();
	auto nTexY = ( (CChunk2*)m_pObjData )->GetTexY();
	int32 nSelectedX = floor( mousePos.x - m_previewImgRect.x ) * nTexX / m_previewImgRect.width;
	int32 nSelectedY = floor( mousePos.y - m_previewImgRect.y ) * nTexY / m_previewImgRect.height;
	nSelectedY = nTexY - 1 - nSelectedY;
	if( nSelectedX >= 0 && nSelectedX < nTexX && nSelectedY >= 0 && nSelectedY < nTexY )
	{
		m_nSelectedX = nSelectedX;
		m_nSelectedY = nSelectedY;
	}
	return false;
}

void CLevelEditPrefabToolsetCommon::CreatePrefabTools( CPrefab* pPrefab, vector<SLevelEditToolDesc>& result )
{
	result.push_back( SLevelEditToolDesc( 0, 8, &CLevelEditPrefabToolEntityBrush::Inst() ) );
	if( pPrefab->GetRoot()->GetClassData()->Is( CClassMetaDataMgr::Inst().GetClassData<IEditorTiled>() ) )
		result.push_back( SLevelEditToolDesc( 1, 8, &CLevelEditPrefabToolEntityChunk::Inst() ) );
	result.push_back( SLevelEditToolDesc( 0, 9, &CLevelEditPrefabToolEntityAttach::Inst() ) );
}

void CLevelEditObjectToolsetEntity::CreateObjectTools( CPrefabNode* pPrefab, vector<SLevelEditToolDesc>& result )
{
	result.push_back( SLevelEditToolDesc( 0, 8, &CLevelEditObjectToolEntityDefault::Inst() ) );
}

CLevelEditObjectTool * CLevelEditObjectToolsetEntity::CreateQuickObjectTool( CPrefabNode* pNode )
{
	if( pNode->GetPatchedNode()->GetClassData()->Is( CClassMetaDataMgr::Inst().GetClassData<IEditorTiled>() ) )
		return &CLevelEditObjectQuickToolEntityResize::Inst();
	return NULL;
}

CLevelEditObjectTool* CLevelEditObjectToolsetChunk1::CreateQuickObjectTool( CPrefabNode* pNode )
{
	return &CLevelEditObjectQuickToolChunk1::Inst();
}

CLevelEditObjectTool * CLevelEditObjectToolsetChunk2::CreateQuickObjectTool( CPrefabNode * pNode )
{
	return &CLevelEditObjectQuickToolChunk2::Inst();
}

CLevelEditObjectTool * CLevelEditObjectToolsetAlertTrigger::CreateQuickObjectTool( CPrefabNode * pNode )
{
	return &CLevelEditObjectQuickToolAlertTriggerResize::Inst();
}

CLevelEditObjectTool * CLevelEditObjectToolsetGate::CreateQuickObjectTool( CPrefabNode * pNode )
{
	return &CLevelEditObjectQuickToolGate::Inst();
}

void RegisterToolsets()
{
	auto& mgr = CLevelEditToolsetMgr::Inst();
	mgr.RegisterPrefab<CCharacter, CLevelEditPrefabToolsetCommon>();
	mgr.RegisterPrefab<CSimpleText, CLevelEditPrefabToolsetCommon>();
	mgr.RegisterPrefab<CGamePointLight, CLevelEditPrefabToolsetCommon>();

	mgr.RegisterObject<CEntity, CLevelEditObjectToolsetEntity>();
	mgr.RegisterObject<CChunk1, CLevelEditObjectToolsetChunk1>();
	mgr.RegisterObject<CChunk2, CLevelEditObjectToolsetChunk2>();
	mgr.RegisterObject<CAlertTrigger, CLevelEditObjectToolsetAlertTrigger>();
	mgr.RegisterObject<CGate, CLevelEditObjectToolsetGate>();
}
