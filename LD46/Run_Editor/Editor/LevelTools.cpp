#include "stdafx.h"
#include "LevelTools.h"
#include "MyLevel.h"
#include "UICommon/UIManager.h"
#include "UICommon/UIFactory.h"
#include "Editor/Editors/UIComponentUtil.h"
#include "World.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Common/Utf8Util.h"
#include "Editor/Editors/ObjectDataEdit.h"

void CLevelTool::OnSetVisible( bool b )
{
	if( b )
		m_pLevelNode = GetView()->m_pLevelNode;
	else
		m_pLevelNode = NULL;
}

CLevelToolsView* CLevelTool::GetView()
{
	return (CLevelToolsView*)GetParent();
}

CMyLevel* CLevelTool::GetLevelData()
{
	return (CMyLevel*)m_pLevelNode->GetObjData();
}

CPrefab* CLevelTool::GetRes()
{
	return GetView()->m_pRes;
}

class CTileTool : public CLevelTool
{
public:
	CTileTool() : m_nCurSelectedOpr( 0 ), m_nCurSelectedValue( 0 ), m_nDragType( 0 ) {}
	virtual void OnSetVisible( bool bVisible ) override
	{
		CLevelTool::OnSetVisible( bVisible );
		if( bVisible )
			m_nCurSelectedOpr = m_nCurSelectedValue = 0;
	}
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem, class CUIViewport* pViewport ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportKey( SUIKeyEvent* pEvent );

	void UpdateDrag( class CUIViewport* pViewport, const TVector2<int32>& p );
private:
	int8 m_nDragType;
	CVector2 m_dragPos;
	TRectangle<int32> m_newSize;
	int32 m_nCurSelectedOpr;
	int32 m_nCurSelectedValue;
};

void CTileTool::OnDebugDraw( IRenderSystem* pRenderSystem, class CUIViewport* pViewport )
{
	auto mousePos = GetView()->GetViewportMousePos();
	TVector2<int32> p( floor( mousePos.x / LEVEL_GRID_SIZE_X ), floor( mousePos.y / LEVEL_GRID_SIZE_Y ) );
	auto pObj = GetLevelData();
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

	{
		CVector2 a( 0, 0 );
		auto b = CVector2( pObj->m_nWidth, pObj->m_nHeight ) * LEVEL_GRID_SIZE;
		if( m_nCurSelectedOpr == 3 )
		{
			auto r = pObj->GetMainAreaSize();
			a = CVector2( r.x, r.y );
			b = CVector2( r.width, r.height );
		}
		if( m_nDragType > 0 )
		{
			a = CVector2( m_newSize.x, m_newSize.y ) * LEVEL_GRID_SIZE;
			b = CVector2( m_newSize.width, m_newSize.height ) * LEVEL_GRID_SIZE;
		}
		CVector4 color1( 0.25f, 0.25f, 0.25f, 0.25f );
		if( m_nCurSelectedOpr == 3 )
			color1 = CVector4( 0.2f, 0.3f, 0.4f, 0.25f );
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = a + b * ofs[j];
			auto pt2 = a + b * ofs[( j + 1 ) % 4];
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, color1 );
		}

		{
			auto pt1 = a + CVector2( 0, b.y * 0.5f );
			auto pt2 = pt1 + CVector2( -32, 0 );
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.2f, 0.2f, 0.1f, 0.25f ) );
		}
		{
			auto pt1 = a + CVector2( b.x * 0.5f, 0 );
			auto pt2 = pt1 + CVector2( 0, -32 );
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.2f, 0.2f, 0.1f, 0.25f ) );
		}
		{
			auto pt1 = a + CVector2( b.x, b.y * 0.5f );
			auto pt2 = pt1 + CVector2( 32, 0 );
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.2f, 0.2f, 0.1f, 0.25f ) );
		}
		{
			auto pt1 = a + CVector2( b.x * 0.5f, b.y );
			auto pt2 = pt1 + CVector2( 0, 32 );
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.2f, 0.2f, 0.1f, 0.25f ) );
		}
	}
	{
		CVector2 ofs1[] = { { LEVEL_GRID_SIZE_X * 0.5f, 0 }, { 0, LEVEL_GRID_SIZE_Y * 0.5f }, { -LEVEL_GRID_SIZE_X * 0.5f, 0 }, { 0, -LEVEL_GRID_SIZE_Y * 0.5f } };
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = pObj->GetCamPos();
			auto pt2 = pt1 + ofs1[j];
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 1, 1, 1, 1 ) );
		}
	}

	if( p.x < 0 || p.y < 0 || p.x >= pObj->m_nWidth || p.y >= pObj->m_nHeight )
		return;
	auto x1 = !( ( p.x + p.y ) & 1 ) ? p.x + 1 : p.x - 1;
	if( x1 < 0 || x1 >= pObj->m_nWidth )
		return;

	if( m_nCurSelectedOpr < 2 )
	{
		CVector4 colors[] = { { 1, 1, 0, 1 }, { 0, 1, 1, 1 }, { 1, 0, 1, 1 }, { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 } };
		CRectangle rect( Min( p.x, x1 ) * LEVEL_GRID_SIZE_X, p.y * LEVEL_GRID_SIZE_Y, LEVEL_GRID_SIZE_X * 2, LEVEL_GRID_SIZE_Y );
		CVector4 color( 0.5f, 0.5f, 0.5f, 1 );
		if( m_nCurSelectedOpr == 0 )
			color = colors[m_nCurSelectedValue % ELEM_COUNT( colors )];
		else if( m_nCurSelectedValue > 0 )
			color = colors[( m_nCurSelectedValue - 1 ) % ELEM_COUNT( colors )];

		CVector2 a( rect.x, rect.y );
		auto b = rect.GetSize();
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = a + b * ofs[j];
			auto pt2 = a + b * ofs[( j + 1 ) % 4];
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, color );
		}
	}
}

bool CTileTool::OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos )
{
	auto pObj = GetLevelData();
	if( m_nCurSelectedOpr == 2 )
	{
		int32 x = floor( mousePos.x / ( LEVEL_GRID_SIZE_X / 2 ) + 0.5f );
		int32 y = floor( mousePos.y / ( LEVEL_GRID_SIZE_Y / 2 ) + 0.5f );
		pObj->m_camPos = CVector2( x, y ) * ( LEVEL_GRID_SIZE / 2 );
		return false;
	}

	CVector2 a( 0, 0 );
	auto b = CVector2( pObj->m_nWidth, pObj->m_nHeight ) * LEVEL_GRID_SIZE;
	if( m_nCurSelectedOpr == 3 )
	{
		auto r = pObj->GetMainAreaSize();
		a = CVector2( r.x, r.y );
		b = CVector2( r.GetRight(), r.GetBottom() );
	}
	CRectangle r[4] = { { a.x - 32, ( a.y + b.y ) * 0.5f - 16, 32, 32 },
	{ ( a.x + b.x ) * 0.5f - 16, a.y - 32, 32, 32 },
	{ b.x, ( a.y + b.y ) * 0.5f - 16, 32, 32 },
	{ ( a.x + b.x ) * 0.5f - 16, b.y, 32, 32 } };
	for( int i = 0; i < 4; i++ )
	{
		if( r[i].Contains( mousePos ) )
		{
			m_nDragType = i + 1;
			m_dragPos = mousePos;
			a.x = floor( a.x / LEVEL_GRID_SIZE_X + 0.5f );
			a.y = floor( a.y / LEVEL_GRID_SIZE_Y + 0.5f );
			b.x = floor( b.x / LEVEL_GRID_SIZE_X + 0.5f );
			b.y = floor( b.y / LEVEL_GRID_SIZE_Y + 0.5f );
			m_newSize = TRectangle<int32>( a.x, a.y, b.x - a.x, b.y - a.y );
			return true;
		}
	}

	int32 x = floor( mousePos.x / LEVEL_GRID_SIZE_X );
	int32 y = floor( mousePos.y / LEVEL_GRID_SIZE_Y );
	UpdateDrag( pViewport, TVector2<int32>( x, y ) );
	return true;
}

void CTileTool::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_nDragType > 0 )
	{
		auto pObj = GetLevelData();
		TVector2<int32> d( floor( ( mousePos.x - m_dragPos.x ) / LEVEL_GRID_SIZE_X + 0.5f ),
			floor( ( mousePos.y - m_dragPos.y ) / LEVEL_GRID_SIZE_Y + 0.5f ) );

		if( m_nCurSelectedOpr == 3 )
		{
			auto r = pObj->GetMainAreaSize();

			int32 x1 = floor( r.x / LEVEL_GRID_SIZE_X + 0.5f );
			int32 y1 = floor( r.y / LEVEL_GRID_SIZE_Y + 0.5f );
			int32 x2 = floor( r.GetRight() / LEVEL_GRID_SIZE_X + 0.5f );
			int32 y2 = floor( r.GetBottom() / LEVEL_GRID_SIZE_Y + 0.5f );
			m_newSize = TRectangle<int32>( x1, y1, x2 - x1, y2 - y1 );
		}
		else
			m_newSize = TRectangle<int32>( 0, 0, pObj->m_nWidth, pObj->m_nHeight );
		if( m_nDragType == 1 )
			m_newSize.SetLeft( m_newSize.x + d.x );
		else if( m_nDragType == 2 )
			m_newSize.SetTop( m_newSize.y + d.y );
		else if( m_nDragType == 3 )
			m_newSize.SetRight( m_newSize.GetRight() + d.x );
		else
			m_newSize.SetBottom( m_newSize.GetBottom() + d.y );
		return;
	}
	int32 x = floor( mousePos.x / LEVEL_GRID_SIZE_X );
	int32 y = floor( mousePos.y / LEVEL_GRID_SIZE_Y );
	UpdateDrag( pViewport, TVector2<int32>( x, y ) );
}

void CTileTool::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_nDragType > 0 )
	{
		if( m_nCurSelectedOpr == 3 )
		{
			auto pObj = GetLevelData();
			if( m_newSize == TRectangle<int32>( 0, 0, pObj->m_nWidth, pObj->m_nHeight ) )
				pObj->m_rectMainArea = CRectangle( 0, 0, 0, 0 );
			else
			{
				pObj->m_rectMainArea = CRectangle( m_newSize.x * LEVEL_GRID_SIZE_X, m_newSize.y * LEVEL_GRID_SIZE_Y,
					m_newSize.width * LEVEL_GRID_SIZE_X, m_newSize.height * LEVEL_GRID_SIZE_Y );
			}
		}
		else
		{
			if( !!( ( m_newSize.x + m_newSize.y ) & 1 ) )
				m_newSize.SetLeft( m_newSize.x - 1 );
			GetView()->ResizeLevel( m_newSize );
		}
		m_nDragType = 0;
	}
}

void CTileTool::OnViewportKey( SUIKeyEvent* pEvent )
{
	auto n = pEvent->nChar;
	if( n == 'Q' )
	{
		if( m_nCurSelectedOpr == 0 )
			return;
		m_nCurSelectedOpr = 0;
		m_nCurSelectedValue = 0;
	}
	else if( n == 'W' )
	{
		if( m_nCurSelectedOpr == 1 )
			return;
		m_nCurSelectedOpr = 1;
		m_nCurSelectedValue = 0;
	}
	else if( n == 'E' )
		m_nCurSelectedOpr = 2;
	else if( n == 'R' )
		m_nCurSelectedOpr = 3;
	else if( n >= '0' && n <= '9' )
	{
		auto n1 = n - '0';
		if( pEvent->bAltDown )
			n1 += 10;
		if( m_nCurSelectedOpr == 0 )
		{
			if( n1 >= GetLevelData()->m_arrTileData.Size() )
				return;
		}
		else if( m_nCurSelectedOpr == 1 )
		{
			if( n1 > GetLevelData()->m_arrNextStage.Size() )
				return;
		}
		m_nCurSelectedValue = n1;
	}
}

void CTileTool::UpdateDrag( CUIViewport* pViewport, const TVector2<int32>& p )
{
	auto pObj = GetLevelData();
	if( p.x < 0 || p.y < 0 || p.x >= pObj->m_nWidth || p.y >= pObj->m_nHeight )
		return;
	auto x1 = !( ( p.x + p.y ) & 1 ) ? p.x + 1 : p.x - 1;
	if( x1 < 0 || x1 >= pObj->m_nWidth )
		return;

	auto& grids = pObj->m_arrGridData;
	auto& grid = grids[p.x + p.y * pObj->m_nWidth];
	auto& grid1 = grids[x1 + p.y * pObj->m_nWidth];
	if( m_nCurSelectedOpr == 0 )
	{
		if( grid.nTile == m_nCurSelectedValue )
			return;
		grid.nTile = grid1.nTile = m_nCurSelectedValue;
		grid.bBlocked = grid1.bBlocked = pObj->m_arrTileData[m_nCurSelectedValue].bBlocked;

		GetView()->RefreshTile( p.x, p.y );
		GetView()->RefreshTile( x1, p.y );
	}
	else if( m_nCurSelectedOpr == 1 )
	{
		if( grid.nNextStage == m_nCurSelectedValue )
			return;
		grid.nNextStage = grid1.nNextStage = m_nCurSelectedValue;
	}
}

class CPawnTool : public CLevelTool
{
public:
	virtual void OnInited() override
	{
		m_nOprType = 0;
		m_bNoAlign = false;
		m_bDragged = false;
		m_pFiles = GetChildByName<CUIScrollView>( "files" );
		m_pLayerScript = GetChildByName<CUIButton>( "layer_script" );
		m_pLayerScriptText = GetChildByName<CUITextBox>( "layer_script_text" );
		m_pSelectedFile = GetChildByName<CUILabel>( "selected" );
		m_pViewport = GetChildByName<CUIViewport>( "viewport" );
		m_pViewport->SetLight( false );
		m_onLayerScript.Set( this, &CPawnTool::OnLayerScript );
		m_pLayerScript->Register( eEvent_Action, &m_onLayerScript );
		m_onLayerScriptEditOK.Set( this, &CPawnTool::OnLayerScriptEditOK );
		m_onLayerScriptText.Set( this, &CPawnTool::OnLayerScriptText );
		m_pLayerScriptText->Register( eEvent_Action, &m_onLayerScriptText );
	}

	virtual void OnSetVisible( bool bVisible ) override
	{
		CLevelTool::OnSetVisible( bVisible );
		if( bVisible )
		{
			function<void( const char* )> FuncFolders;
			map<string, CReference<CPrefab>, _SLess > mapPrefab;
			FuncFolders = [&mapPrefab, &FuncFolders] ( const char* szPath ) {
				string strFind = szPath;
				strFind += "*.pf";
				FindFiles( strFind.c_str(), [&mapPrefab, szPath] ( const char* szFileName )
				{
					string strFullPath = szPath;
					strFullPath += szFileName;
					auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( strFullPath.c_str() );
					if( !pPrefab )
						return true;
					auto pPawnData = pPrefab->GetRoot()->GetStaticDataSafe<CPawn>();
					if( !pPawnData || pPawnData->IsHideInEditor() )
						return true;
					mapPrefab[strFullPath] = pPrefab;
					return true;
				}, true, false );

				strFind = szPath;
				strFind += "*";
				FindFiles( strFind.c_str(), [&FuncFolders, szPath] ( const char* szFileName )
				{
					string strFullPath = szPath;
					strFullPath += szFileName;
					strFullPath += "/";
					FuncFolders( strFullPath.c_str() );
					return true;
				}, false, true );
			};
			FuncFolders( "" );
			for( auto& item : mapPrefab )
				m_mapItems[item.first] = CItem::Create( this, m_pFiles, item.second );

			for( auto p = m_pLevelNode->Get_RenderChild(); p; p = p->NextRenderChild() )
			{
				if( p == m_pLevelNode->GetRenderObject() )
					continue;
				auto pPrefabNode = dynamic_cast<CPrefabNode*>( p );
				if( pPrefabNode && pPrefabNode->GetStaticDataSafe<CPawnLayer>() )
					m_vecAllPawnRoots.push_back( pPrefabNode );
			}
			m_vecAllPawnRoots.push_back( m_pLevelNode->GetChildByName<CPrefabNode>( "1" ) );
			for( int i = 0; i < m_vecAllPawnRoots.size() / 2; i++ )
				swap( m_vecAllPawnRoots[i], m_vecAllPawnRoots[m_vecAllPawnRoots.size() - 1 - i] );
			for( CPrefabNode* p : m_vecAllPawnRoots )
				p->bVisible = false;
			m_nPawnRoot = -1;
			SelectPawnRoot( 0 );
		}
		else
		{
			for( CPrefabNode* p : m_vecAllPawnRoots )
				p->bVisible = true;
			m_nOprType = 0;
			m_bNoAlign = false;
			m_bDragged = false;
			m_pPawnRoot = NULL;
			m_vecAllPawns.resize( 0 );
			m_vecAllPawnRoots.resize( 0 );
			SelectFile( NULL );
			if( m_pPreview )
			{
				m_pPreview->RemoveThis();
				m_pPreview = NULL;
			}
			m_mapItems.clear();
			m_pFiles->ClearContent();
		}
	}
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem, class CUIViewport* pViewport ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportKey( SUIKeyEvent* pEvent );

	void SelectPawnRoot( int32 n );
	void SelectFile( CPrefab* pPrefab );
private:
	class CItem : public CUIButton
	{
	public:
		static CItem* Create( CPawnTool* pOwner, CUIScrollView* pView, CPrefab* pPrefab )
		{
			static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/pawn_tool_item.xml" );
			auto pItem = new CItem;
			g_pRes->GetElement()->Clone( pItem );
			pView->AddContent( pItem );
			pItem->m_pOwner = pOwner;
			pItem->m_pPrefab = pPrefab;
			pItem->SetText( pPrefab->GetName() );
			return pItem;
		}

	protected:
		virtual void OnInited() override
		{
			m_onSelect.Set( this, &CItem::OnSelect );
			Register( eEvent_Action, &m_onSelect );
		}
		void OnSelect()
		{
			m_pOwner->SelectFile( m_pPrefab );
		}
	private:
		CPawnTool* m_pOwner;
		CReference<CPrefab> m_pPrefab;
		TClassTrigger<CItem> m_onSelect;
	};
	void OnLayerScript();
	void OnLayerScriptEditOK( const wchar_t* sz );
	void OnLayerScriptText();
	CPawn* GetPawnData( int32 n );
	int32 Pick( const TRectangle<int32>& p );
	void MultiPick( const TRectangle<int32>& p, vector<int32>& result );
	void Rotate( const TVector2<int32>& p, bool bUp );
	void Replace( const TVector2<int32>& p );
	void Add( const TVector2<int32>& p );
	void Remove( const TRectangle<int32>& p );

	struct _SLess
	{
		bool operator () ( const string& a, const string& b ) const
		{
			uint32 l1 = a.find_last_of( '/' );
			uint32 l2 = b.find_last_of( '/' );
			if( l1 != l2 )
			{
				int32 n = strncmp( a.c_str(), b.c_str(), Min( l1, l2 ) );
				if( n < 0 )
					return true;
				else if( n > 0 )
					return false;
				else
					return l1 < l2;
			}
			else
				return a < b;
		}
	};
	map<string, CReference<CItem>, _SLess > m_mapItems;
	CReference<CUIScrollView> m_pFiles;
	CReference<CUIButton> m_pLayerScript;
	CReference<CUITextBox> m_pLayerScriptText;
	CReference<CUILabel> m_pSelectedFile;
	CReference<CUIViewport> m_pViewport;

	CReference<CRenderObject2D> m_pPreview;
	CReference<CPrefab> m_pCurSelected;
	int8 m_nOprType;
	bool m_bNoAlign;
	int32 m_nPawnRoot;
	CReference<CPrefabNode> m_pPawnRoot;
	vector<CReference<CPrefabNode> > m_vecAllPawns;
	vector<CReference<CPrefabNode> > m_vecAllPawnRoots;
	TClassTrigger<CPawnTool> m_onLayerScript;
	TClassTrigger1<CPawnTool, const wchar_t*> m_onLayerScriptEditOK;
	TClassTrigger<CPawnTool> m_onLayerScriptText;

	bool m_bDragged;
	int32 m_nDragged;
	CVector2 m_draggedPos;
	CVector2 m_draggedp0;
};

void CPawnTool::OnDebugDraw( IRenderSystem* pRenderSystem, CUIViewport* pViewport )
{
	auto mousePos = GetView()->GetViewportMousePos();
	TVector2<int32> p( floor( mousePos.x / LEVEL_GRID_SIZE_X ), floor( mousePos.y / LEVEL_GRID_SIZE_Y ) );
	auto pObj = GetLevelData();
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	{
		CVector2 a( 0, 0 );
		auto b = CVector2( pObj->m_nWidth, pObj->m_nHeight ) * LEVEL_GRID_SIZE;
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = a + b * ofs[j];
			auto pt2 = a + b * ofs[( j + 1 ) % 4];
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.3f, 0.3f, 0, 0.25f ) * ( m_bNoAlign ? 1 : 0.5f ) );
		}
	}

	CVector4 color;
	if( m_bDragged )
		return;
	TVector2<int32> size( 1, 1 );
	if( m_nOprType == 1 || m_nOprType == 2 && !m_bNoAlign )
	{
		if( !!( ( p.x + p.y ) & 1 ) )
			p.x--;
		size.x = 2;
		if( m_pCurSelected )
		{
			auto pPawn = m_pCurSelected->GetRoot()->GetStaticDataSafe<CPawn>();
			size.x = pPawn->GetWidth();
			size.y = pPawn->GetHeight();
		}
	}
	CRectangle rect( p.x * LEVEL_GRID_SIZE_X, p.y * LEVEL_GRID_SIZE_Y, size.x * LEVEL_GRID_SIZE_X, size.y * LEVEL_GRID_SIZE_Y );
	CVector4 colors[4] = { { 0.5f, 0.5f, 0.5f, 0.5f }, { 0, 0, 0.5f, 0.5f }, { 0, 0.5f, 0, 0.5f }, { 0.5f, 0, 0, 0.5f } };
	{
		CVector2 a( rect.x, rect.y );
		CVector2 b( rect.width, rect.height );
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = a + b * ofs[j];
			auto pt2 = a + b * ofs[( j + 1 ) % 4];
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, colors[m_nOprType] );
		}
	}
}

bool CPawnTool::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto pObj = GetLevelData();
	int32 x = floor( mousePos.x / LEVEL_GRID_SIZE_X );
	int32 y = floor( mousePos.y / LEVEL_GRID_SIZE_Y );
	if( m_nOprType == 1 || m_nOprType == 2 && !m_bNoAlign )
	{
		if( !!( ( x + y ) & 1 ) )
			x--;
	}

	switch( m_nOprType )
	{
	case 0:
	{
		auto n = Pick( TRectangle<int32>( x, y, 1, 1 ) );
		if( n >= 0 )
		{
			m_bDragged = true;
			m_nDragged = n;
			m_draggedPos = mousePos;
			m_draggedp0 = m_vecAllPawns[n]->GetPosition();
			return true;
		}
		return false;
	}
	case 1:
		Replace( TVector2<int32>( x, y ) );
		return false;
	case 2:
		Add( TVector2<int32>( x, y ) );
		return false;
	case 3:
		Remove( TRectangle<int32>( x, y, 1, 1 ) );
		return false;
	}
	return false;
}

void CPawnTool::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_bDragged )
	{
		auto pLevel = GetLevelData();
		TVector2<int32> p0( floor( m_draggedp0.x / LEVEL_GRID_SIZE_X + 0.5f ), floor( m_draggedp0.y / LEVEL_GRID_SIZE_Y + 0.5f ) );
		auto d = ( mousePos - m_draggedPos ) / LEVEL_GRID_SIZE;
		TVector2<int32> ofs( floor( d.x + 0.5f ), floor( d.y + 0.5f ) );
		if( !m_bNoAlign )
		{
			ofs = TVector2<int32>( floor( ( d.x + d.y ) * 0.5f + 0.5f ), floor( ( d.x - d.y ) * 0.5f + 0.5f ) );
			ofs = TVector2<int32>( ofs.x + ofs.y, ofs.x - ofs.y );
		}
		auto p = p0 + ofs;
		m_vecAllPawns[m_nDragged]->SetPosition( CVector2( p.x, p.y ) * LEVEL_GRID_SIZE );
	}
}

void CPawnTool::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	m_bDragged = false;
}

void CPawnTool::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( pEvent->nChar == 'Q' )
	{ m_nOprType = 0; return; }
	else if( pEvent->nChar == 'W' )
	{ m_nOprType = 1; return; }
	else if( pEvent->nChar == 'E' )
	{ m_nOprType = 2; return; }
	else if( pEvent->nChar == 'R' )
	{ m_nOprType = 3; return; }
	if( m_bDragged )
		return;

	auto pObj = GetLevelData();
	auto mousePos = GetView()->GetViewportMousePos();
	int32 x = floor( mousePos.x / LEVEL_GRID_SIZE_X );
	int32 y = floor( mousePos.y / LEVEL_GRID_SIZE_Y );
	if( pEvent->nChar >= '0' && pEvent->nChar <= '9' )
	{
		auto n = pEvent->nChar - '0';
		if( n >= m_vecAllPawnRoots.size() )
		{
			auto pNode = new CPrefabNode( GetRes() );
			pNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CPawnLayer>()->strClassName.c_str() );
			m_pLevelNode->AddChildBefore( pNode, m_vecAllPawnRoots.back() );
			pNode->OnEditorActive( false );
			m_vecAllPawnRoots.push_back( pNode );
			n = Min( n, m_vecAllPawnRoots.size() - 1 );
			char sz[32];
			itoa( n + 1, sz, 10 );
			pNode->SetName( sz );
		}
		SelectPawnRoot( n );
	}
	else if( pEvent->nChar == 'S' )
		SelectPawnRoot( m_nPawnRoot < m_vecAllPawnRoots.size() - 1 ? m_nPawnRoot + 1 : 0 );
	else if( pEvent->nChar == 'A' )
		SelectPawnRoot( m_nPawnRoot > 0 ? m_nPawnRoot - 1 : m_vecAllPawnRoots.size() - 1 );
	else if( pEvent->nChar == VK_TAB )
		m_bNoAlign = !m_bNoAlign;
	else if( m_nOprType == 0 )
	{
		if( pEvent->nChar == 'Z' )
			Rotate( TVector2<int32>( x, y ), true );
		else if( pEvent->nChar == 'X' )
			Rotate( TVector2<int32>( x, y ), false );
		else if( pEvent->nChar == 'C' )
		{
			auto n = Pick( TRectangle<int32>( x, y, 1, 1 ) );
			if( n >= 0 )
			{
				m_vecAllPawns[n]->OnEditorActive( true );
				auto p = GetPawnData( n );
				p->m_nInitDir = p->m_nInitDir ? 0 : 1;
				m_vecAllPawns[n]->GetPatchedNode()->OnEdit();
				m_vecAllPawns[n]->OnEditorActive( false );
			}
		}
		else if( pEvent->nChar == 'V' || pEvent->nChar == 'B' )
		{
			auto n = Pick( TRectangle<int32>( x, y, 1, 1 ) );
			if( n >= 0 )
			{
				auto p = GetPawnData( n );
				auto nStates = p->m_arrSubStates.Size();
				if( nStates > 0 )
				{
					m_vecAllPawns[n]->OnEditorActive( true );
					if( !p->m_bUseInitState )
					{
						p->m_bUseInitState = true;
						p->m_nInitState = pEvent->nChar == 'V' ? nStates - 1 : 0;
					}
					else
					{
						if( pEvent->nChar == 'V' )
						{
							if( p->m_nInitState == 0 )
								p->m_bUseInitState = false;
							else
								p->m_nInitState--;
						}
						else
						{
							if( p->m_nInitState == nStates - 1 )
							{
								p->m_bUseInitState = false;
								p->m_nInitState = 0;
							}
							else
								p->m_nInitState++;
						}
					}
					m_vecAllPawns[n]->GetPatchedNode()->OnEdit();
					m_vecAllPawns[n]->OnEditorActive( false );
				}
			}
		}
	}
}

void CPawnTool::SelectPawnRoot( int32 n )
{
	if( n == m_nPawnRoot )
		return;
	if( m_pPawnRoot )
		m_pPawnRoot->bVisible = false;
	m_nPawnRoot = n;
	m_pPawnRoot = m_vecAllPawnRoots[n];
	m_pPawnRoot->bVisible = true;
	m_vecAllPawns.resize( 0 );
	for( auto pChild = m_pPawnRoot->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pPawnRoot->GetRenderObject() )
			continue;
		auto pPrefabNode = static_cast<CPrefabNode*>( pChild );
		if( !pPrefabNode->GetPatchedNode() || !pPrefabNode->GetPatchedNode()->GetStaticDataSafe<CPawn>() )
			continue;
		m_vecAllPawns.push_back( pPrefabNode );
	}
	m_pLayerScript->SetText( m_pPawnRoot->GetName() );
	auto p = m_pPawnRoot->GetStaticDataSafe<CPawnLayer>();
	if( p )
		m_pLayerScriptText->SetText( p->GetCondition() );
	else
		m_pLayerScriptText->SetText( "" );
}

void CPawnTool::SelectFile( CPrefab* pPrefab )
{
	if( pPrefab == m_pCurSelected )
		return;
	if( m_pPreview )
	{
		m_pPreview->RemoveThis();
		m_pPreview = NULL;
	}
	m_pCurSelected = pPrefab;
	if( pPrefab )
	{
		auto p = SafeCast<CPawn>( pPrefab->GetRoot()->CreateInstance() );
		m_pPreview = p;
		m_pViewport->GetRoot()->AddChild( p );
		p->OnPreview();
		m_pSelectedFile->SetText( pPrefab->GetName() );
	}
}

void CPawnTool::OnLayerScript()
{
	auto p = m_pPawnRoot->GetStaticDataSafe<CPawnLayer>();
	if( !p )
		return;
	CTextEditDialog::Inst()->Show( Utf8ToUnicode( p->GetCondition() ).c_str(), &m_onLayerScriptEditOK );
}

void CPawnTool::OnLayerScriptEditOK( const wchar_t * sz )
{
	if( sz )
	{
		static_cast<CPawnLayer*>( m_pPawnRoot->GetFinalObjData() )->m_strCondition = UnicodeToUtf8( sz ).c_str();
		m_pPawnRoot->OnEdit();
	}
}

void CPawnTool::OnLayerScriptText()
{
	auto p = m_pPawnRoot->GetStaticDataSafe<CPawnLayer>();
	if( !p )
		return;
	static_cast<CPawnLayer*>( m_pPawnRoot->GetFinalObjData() )->m_strCondition = UnicodeToUtf8( m_pLayerScriptText->GetText() ).c_str();
	m_pPawnRoot->OnEdit();
}

CPawn* CPawnTool::GetPawnData( int32 n )
{
	return SafeCast<CPawn>( m_vecAllPawns[n]->GetPatchedNode()->GetFinalObjData() );
}

int32 CPawnTool::Pick( const TRectangle<int32>& rect )
{
	for( int i = 0; i < m_vecAllPawns.size(); i++ )
	{
		CPrefabNode* pNode = m_vecAllPawns[i];
		CPawn* pPawnData = (CPawn*)pNode->GetPatchedNode()->GetObjData();
		auto pos = pNode->GetPosition();
		auto p = TVector2<int32>( floor( pos.x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pos.y / LEVEL_GRID_SIZE_Y + 0.5f ) );
		auto r = rect * TRectangle<int32>( p.x, p.y, pPawnData->GetWidth(), pPawnData->GetHeight() );
		if( r.width > 0 && r.height > 0 )
			return i;
	}
	return -1;
}

void CPawnTool::MultiPick( const TRectangle<int32>& rect, vector<int32>& result )
{
	for( int i = 0; i < m_vecAllPawns.size(); i++ )
	{
		CPrefabNode* pNode = m_vecAllPawns[i];
		CPawn* pPawnData = (CPawn*)pNode->GetPatchedNode()->GetObjData();
		auto pos = pNode->GetPosition();
		auto p = TVector2<int32>( floor( pos.x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pos.y / LEVEL_GRID_SIZE_Y + 0.5f ) );
		auto r = rect * TRectangle<int32>( p.x, p.y, pPawnData->GetWidth(), pPawnData->GetHeight() );
		if( r.width > 0 && r.height > 0 )
			result.push_back( i );
	}
}

void CPawnTool::Rotate( const TVector2<int32>& p, bool bUp )
{
	vector<int32> result;
	MultiPick( TRectangle<int32>( p.x, p.y, 1, 1 ), result );
	if( result.size() <= 1 )
		return;
	CReference<CRenderObject2D> pTemp = new CRenderObject2D;
	for( int i = 0; i < result.size() - 1; i++ )
	{
		auto& pCur = m_vecAllPawns[bUp ? result[i] : result[result.size() - 1 - i]];
		auto& pNxt = m_vecAllPawns[bUp ? result[i + 1] : result[result.size() - 2 - i]];
		m_pPawnRoot->AddChildBefore( pTemp, pCur );
		pCur->RemoveThis();
		m_pPawnRoot->AddChildBefore( pCur, pNxt );
		pNxt->RemoveThis();
		m_pPawnRoot->AddChildBefore( pNxt, pTemp );
		pTemp->RemoveThis();
		swap( pCur, pNxt );
	}
}

void CPawnTool::Replace( const TVector2<int32>& p )
{
	if( !m_pCurSelected )
		return;
	auto pPawn = m_pCurSelected->GetRoot()->GetStaticDataSafe<CPawn>();
	Remove( TRectangle<int32>( p.x, p.y, pPawn->GetWidth(), pPawn->GetHeight() ) );
	Add( p );
}

void CPawnTool::Add( const TVector2<int32>& p )
{
	if( !m_pCurSelected )
		return;
	CPrefabNode* pNode = new CPrefabNode( GetRes() );
	pNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CLevelSpawnHelper>()->strClassName.c_str() );
	pNode->SetResource( m_pCurSelected );
	pNode->SetPosition( CVector2( p.x, p.y ) * LEVEL_GRID_SIZE );

	auto pPawnData = m_pCurSelected->GetRoot()->GetStaticDataSafe<CPawn>();
	if( pPawnData->m_nLevelDataType )
	{
		auto pSpawnHelper = SafeCast<CLevelSpawnHelper>( pNode->GetFinalObjData() );
		pSpawnHelper->m_nDataType = pPawnData->m_nLevelDataType;
		pSpawnHelper->m_nDeathState = pPawnData->GetStateIndexByName( "death" );
	}

	m_pPawnRoot->AddChild( pNode );
	pNode->OnEditorActive( false );
	m_vecAllPawns.resize( m_vecAllPawns.size() + 1 );
	for( int i = m_vecAllPawns.size() - 1; i > 0; i-- )
		m_vecAllPawns[i] = m_vecAllPawns[i - 1];
	m_vecAllPawns[0] = pNode;
}

void CPawnTool::Remove( const TRectangle<int32>& p )
{
	auto n = Pick( p );
	if( n < 0 )
		return;
	auto pNode = m_vecAllPawns[n];
	for( int i = n; i < m_vecAllPawns.size() - 1; i++ )
		m_vecAllPawns[i] = m_vecAllPawns[i + 1];
	m_vecAllPawns.resize( m_vecAllPawns.size() - 1 );
	m_pLevelNode->NameSpaceClearNode( pNode );
	pNode->RemoveThis();
}

class CLevelEnvTool : public CLevelTool
{
public:
	CLevelEnvTool() : m_nCurSelectedValue( 0 ), m_nDragType( 0 ) {}
	CLevelEnvEffect* GetData() { return (CLevelEnvEffect*)m_pEnvNode->GetObjData(); }
	virtual void OnInited() override
	{
		m_pLayerScript = GetChildByName<CUIButton>( "layer_script" );
		m_pLayerScriptText = GetChildByName<CUITextBox>( "layer_script_text" );
		m_onLayerScript.Set( this, &CLevelEnvTool::OnLayerScript );
		m_pLayerScript->Register( eEvent_Action, &m_onLayerScript );
		m_onLayerScriptEditOK.Set( this, &CLevelEnvTool::OnLayerScriptEditOK );
		m_onLayerScriptText.Set( this, &CLevelEnvTool::OnLayerScriptText );
		m_pLayerScriptText->Register( eEvent_Action, &m_onLayerScriptText );

		m_onImport.Set( this, &CLevelEnvTool::OnImport );
		m_onImport1.Set( this, &CLevelEnvTool::OnImport1 );
		m_onImport2.Set( this, &CLevelEnvTool::OnImport2 );
		m_onImport3.Set( this, &CLevelEnvTool::OnImport3 );
		m_onImportOK.Set( this, &CLevelEnvTool::OnImportOK );
		auto pImport = GetChildByName<CUIElement>( "import" );
		pImport->Register( eEvent_Action, &m_onImport );
		auto pImport1 = GetChildByName<CUIElement>( "import1" );
		pImport1->Register( eEvent_Action, &m_onImport1 );
		auto pImport2 = GetChildByName<CUIElement>( "import2" );
		pImport2->Register( eEvent_Action, &m_onImport2 );
		auto pImport3 = GetChildByName<CUIElement>( "import3" );
		pImport3->Register( eEvent_Action, &m_onImport3 );
		m_pNodeView = GetChildByName<CUITreeView>( "node_view" );
		m_onNodeObjDataChanged.Set( this, &CLevelEnvTool::OnCurNodeObjDataChanged );
	}
	virtual void OnSetVisible( bool bVisible ) override
	{
		CLevelTool::OnSetVisible( bVisible );
		if( bVisible )
		{
			m_nCurSelectedValue = 0;
			for( auto p = m_pLevelNode->Get_RenderChild(); p; p = p->NextRenderChild() )
			{
				if( p == m_pLevelNode->GetRenderObject() )
					continue;
				auto pPrefabNode = dynamic_cast<CPrefabNode*>( p );
				if( pPrefabNode && pPrefabNode->GetStaticDataSafe<CLevelEnvEffect>() )
				{
					pPrefabNode->bVisible = false;
					m_vecEnvNodes.push_back( pPrefabNode );
				}
			}
			if( !m_vecEnvNodes.size() )
			{
				auto pEnvNode = new CPrefabNode( GetRes() );
				pEnvNode->SetName( "env" );
				m_pLevelNode->AddChild( pEnvNode );
				pEnvNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CLevelEnvEffect>()->strClassName.c_str() );
				auto pData = (CLevelEnvEffect*)pEnvNode->GetObjData();
				pData->m_gridSize = LEVEL_GRID_SIZE * 0.5f;
				pData->m_nWidth = GetLevelData()->GetSize().x * 2;
				pData->m_nHeight = GetLevelData()->GetSize().y * 2;
				pEnvNode->OnEditorActive( false );
				pEnvNode->bVisible = false;
				m_vecEnvNodes.push_back( pEnvNode );
			}
			for( int i = 0; i < m_vecEnvNodes.size() / 2; i++ )
				swap( m_vecEnvNodes[i], m_vecEnvNodes[m_vecEnvNodes.size() - 1 - i] );
			m_nEnvNode = -1;
			SelectEnvNode( 0 );
		}
		else
		{
			if( m_onNodeObjDataChanged.IsRegistered() )
				m_onNodeObjDataChanged.Unregister();
			m_pObjectData = NULL;
			m_pEnvNode->OnEdit();
			m_pEftPreviewNode->RemoveThis();
			m_pEftPreviewNode = NULL;
			m_pEnvNode = NULL;
			m_nEnvNode = -1;
			for( auto& p : m_vecEnvNodes )
				p->bVisible = true;
			m_vecEnvNodes.resize( 0 );
		}
	}
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem, class CUIViewport* pViewport ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportKey( SUIKeyEvent* pEvent );
	void OnImport();
	void OnImport1();
	void OnImport2();
	void OnImport3();
	void OnImportOK( const char* szText );

	void UpdateDrag( class CUIViewport* pViewport, const TVector2<int32>& p );
private:
	void DebugDrawCtrlPoint( IRenderSystem* pRenderSystem, CUIViewport* pViewport, const SLevelCamCtrlPoint& p, int8 nType );
	void DebugDrawCtrlLink( IRenderSystem* pRenderSystem, CUIViewport* pViewport, const SLevelCamCtrlPointLink& l, const SLevelCamCtrlPoint& p1, const SLevelCamCtrlPoint& p2 );
	void RefreshMasks();
	void SelectEnvNode( int32 n );
	void OnLayerScript();
	void OnLayerScriptEditOK( const wchar_t* sz );
	void OnLayerScriptText();
	void OnCurNodeObjDataChanged( int32 nAction );

	int m_nOprType;
	int32 m_nCurSelectedValue;
	vector<CReference<CPrefabNode> > m_vecEnvNodes;
	int32 m_nEnvNode;
	CReference<CPrefabNode> m_pEnvNode;
	CReference<CLevelEnvEffect> m_pEftPreviewNode;
	CReference<CUIButton> m_pLayerScript;
	CReference<CUITextBox> m_pLayerScriptText;
	CReference<CUITreeView> m_pNodeView;
	CReference<CObjectDataEditItem> m_pObjectData;
	int8 m_nDragType;
	CVector2 m_dragPos;
	TRectangle<int32> m_newSize;
	int8 m_nImportType;
	TClassTrigger<CLevelEnvTool> m_onImport;
	TClassTrigger<CLevelEnvTool> m_onImport1;
	TClassTrigger<CLevelEnvTool> m_onImport2;
	TClassTrigger<CLevelEnvTool> m_onImport3;
	TClassTrigger1<CLevelEnvTool, const char*> m_onImportOK;
	TClassTrigger<CLevelEnvTool> m_onLayerScript;
	TClassTrigger1<CLevelEnvTool, const wchar_t*> m_onLayerScriptEditOK;
	TClassTrigger<CLevelEnvTool> m_onLayerScriptText;
	TClassTrigger1<CLevelEnvTool, int32> m_onNodeObjDataChanged;
};

void CLevelEnvTool::OnDebugDraw( IRenderSystem* pRenderSystem, CUIViewport* pViewport )
{
	auto mousePos = GetView()->GetViewportMousePos();
	auto pObj = GetData();
	if( m_nOprType == 0 )
	{
		TVector2<int32> p( floor( ( mousePos.x - pObj->m_gridOfs.x ) / pObj->m_gridSize.x ),
			floor( ( mousePos.y - pObj->m_gridOfs.y ) / pObj->m_gridSize.y ) );
		CVector2 ofs[4] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
		{
			auto a = pObj->m_gridOfs;
			auto b = CVector2( pObj->m_nWidth, pObj->m_nHeight ) * pObj->m_gridSize;
			if( m_nDragType > 0 )
			{
				a = a + CVector2( m_newSize.x, m_newSize.y ) * pObj->m_gridSize;
				b = CVector2( m_newSize.width, m_newSize.height ) * pObj->m_gridSize;
			}
			for( int j = 0; j < 4; j++ )
			{
				auto pt1 = a + b * ofs[j];
				auto pt2 = a + b * ofs[( j + 1 ) % 4];
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.4f, 1, 0.8f, 1 ) );
			}
			{
				auto pt1 = a + CVector2( 0, b.y * 0.5f );
				auto pt2 = pt1 + CVector2( -32, 0 );
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.8f, 0.8f, 0.2f, 1 ) );
			}
			{
				auto pt1 = a + CVector2( b.x * 0.5f, 0 );
				auto pt2 = pt1 + CVector2( 0, -32 );
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.8f, 0.8f, 0.2f, 1 ) );
			}
			{
				auto pt1 = a + CVector2( b.x, b.y * 0.5f );
				auto pt2 = pt1 + CVector2( 32, 0 );
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.8f, 0.8f, 0.2f, 1 ) );
			}
			{
				auto pt1 = a + CVector2( b.x * 0.5f, b.y );
				auto pt2 = pt1 + CVector2( 0, 32 );
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.8f, 0.8f, 0.2f, 1 ) );
			}
		}
		if( p.x >= 0 && p.y >= 0 && p.x < pObj->m_nWidth && p.y < pObj->m_nHeight )
		{
			CVector4 colors[] = { { 1, 1, 0, 1 }, { 0, 1, 1, 1 }, { 1, 0, 1, 1 }, { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 } };
			CRectangle rect( p.x * pObj->m_gridSize.x + pObj->m_gridOfs.x, p.y * pObj->m_gridSize.y + pObj->m_gridOfs.y, pObj->m_gridSize.x, pObj->m_gridSize.y );
			CVector4 color( 0.5f, 0.5f, 0.5f, 1 );
			if( m_nCurSelectedValue > 0 )
				color = colors[( m_nCurSelectedValue - 1 ) % ELEM_COUNT( colors )];
			else if( m_nCurSelectedValue < 0 )
				color = CVector4( 0.2f, 0.2f, 0.2f, 1 );

			CVector2 a( rect.x, rect.y );
			auto b = rect.GetSize();
			for( int j = 0; j < 4; j++ )
			{
				auto pt1 = a + b * ofs[j];
				auto pt2 = a + b * ofs[( j + 1 ) % 4];
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, color );
			}
			color.w *= 0.5f;
			TVector2<int32> d[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			for( int i = 0; i < pObj->m_nWidth; i++ )
			{
				for( int j = 0; j < pObj->m_nHeight; j++ )
				{
					auto& grid = pObj->m_arrEnvMap[i + j * pObj->m_nWidth];
					ASSERT( m_pEftPreviewNode->m_arrEnvMap[i + j * pObj->m_nWidth] == grid );
					if( grid != m_nCurSelectedValue )
						continue;
					CVector2 a( i * pObj->m_gridSize.x + pObj->m_gridOfs.x - 2, j * pObj->m_gridSize.y + pObj->m_gridOfs.y - 2 );
					CVector2 b( pObj->m_gridSize.x + 4, pObj->m_gridSize.y + 4 );
					for( int k = 0; k < 4; k++ )
					{
						auto x = i + d[k].x;
						auto y = j + d[k].y;
						if( x < 0 || y < 0 || x >= pObj->m_nWidth || y >= pObj->m_nHeight )
							continue;
						if( pObj->m_arrEnvMap[x + y * pObj->m_nWidth] == m_nCurSelectedValue )
							continue;
						auto pt1 = a + b * ofs[k];
						auto pt2 = a + b * ofs[( k + 1 ) % 4];
						pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, color );
					}
				}
			}
		}
	}
	//else if( m_nOprType == 1 )
	{
		DebugDrawCtrlPoint( pRenderSystem, pViewport, pObj->m_ctrlPoint1, 0 );
		DebugDrawCtrlPoint( pRenderSystem, pViewport, pObj->m_ctrlPoint2, 0 );
		for( int i = 0; i < pObj->m_arrCtrlPoint.Size(); i++ )
			DebugDrawCtrlPoint( pRenderSystem, pViewport, pObj->m_arrCtrlPoint[i], 1 );
		for( int i = 0; i < pObj->m_arrCtrlLink.Size(); i++ )
		{
			auto& link = pObj->m_arrCtrlLink[i];
			if( link.n1 < -2 || link.n1 >= (int32)pObj->m_arrCtrlPoint.Size() || link.n2 < -2 || link.n2 >= (int32)pObj->m_arrCtrlPoint.Size() )
				continue;
			auto& p1 = link.n1 == -2 ? pObj->m_ctrlPoint1 : ( link.n1 == -1 ? pObj->m_ctrlPoint2 : pObj->m_arrCtrlPoint[link.n1] );
			auto& p2 = link.n2 == -2 ? pObj->m_ctrlPoint1 : ( link.n2 == -1 ? pObj->m_ctrlPoint2 : pObj->m_arrCtrlPoint[link.n2] );

			DebugDrawCtrlLink( pRenderSystem, pViewport, link, p1, p2 );
		}
	}
}

bool CLevelEnvTool::OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos )
{
	auto pEnvData = GetData();

	auto a = pEnvData->m_gridOfs;
	auto b = CVector2( pEnvData->m_nWidth, pEnvData->m_nHeight ) * pEnvData->m_gridSize;
	CRectangle r[4] = { { a.x - 32, a.y + b.y * 0.5f - 16, 32, 32 },
	{ a.x + b.x * 0.5f - 16, a.y - 32, 32, 32 },
	{ a.x + b.x, a.y + b.y * 0.5f - 16, 32, 32 },
	{ a.x + b.x * 0.5f - 16, a.y + b.y, 32, 32 } };
	for( int i = 0; i < 4; i++ )
	{
		if( r[i].Contains( mousePos ) )
		{
			m_nDragType = i + 1;
			m_dragPos = mousePos;
			m_newSize = TRectangle<int32>( 0, 0, pEnvData->m_nWidth, pEnvData->m_nHeight );
			return true;
		}
	}

	int32 x = floor( ( mousePos.x - pEnvData->m_gridOfs.x ) / pEnvData->m_gridSize.x );
	int32 y = floor( ( mousePos.y - pEnvData->m_gridOfs.y ) / pEnvData->m_gridSize.y );
	UpdateDrag( pViewport, TVector2<int32>( x, y ) );
	return true;
}

void CLevelEnvTool::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	auto pEnvData = GetData();
	if( m_nDragType > 0 )
	{
		TVector2<int32> d( floor( ( mousePos.x - m_dragPos.x ) / pEnvData->m_gridSize.x + 0.5f ),
			floor( ( mousePos.y - m_dragPos.y ) / pEnvData->m_gridSize.y + 0.5f ) );
		m_newSize = TRectangle<int32>( 0, 0, pEnvData->m_nWidth, pEnvData->m_nHeight );
		if( m_nDragType == 1 )
			m_newSize.SetLeft( m_newSize.x + d.x );
		else if( m_nDragType == 2 )
			m_newSize.SetTop( m_newSize.y + d.y );
		else if( m_nDragType == 3 )
			m_newSize.SetRight( m_newSize.GetRight() + d.x );
		else
			m_newSize.SetBottom( m_newSize.GetBottom() + d.y );
		return;
	}
	int32 x = floor( ( mousePos.x - pEnvData->m_gridOfs.x ) / pEnvData->m_gridSize.x );
	int32 y = floor( ( mousePos.y - pEnvData->m_gridOfs.y ) / pEnvData->m_gridSize.y );
	UpdateDrag( pViewport, TVector2<int32>( x, y ) );
}

void CLevelEnvTool::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_nDragType > 0 )
	{
		auto pEnvData = GetData();
		pEnvData->Resize( m_newSize );
		m_pEftPreviewNode->Resize( m_newSize );
		m_pEftPreviewNode->Init();
		m_nDragType = 0;
	}
}

void CLevelEnvTool::OnViewportKey( SUIKeyEvent* pEvent )
{
	auto n = pEvent->nChar;
	if( n == 'C' )
	{
		auto pEnvData = GetData();
		pEnvData->Clear();
		m_pEftPreviewNode->Clear();
		m_pEftPreviewNode->Init();
		return;
	}
	if( n == 'F' )
	{
		auto pEnvData = GetData();
		auto mousePos = GetView()->GetViewportMousePos();
		TVector2<int32> p( floor( ( mousePos.x - pEnvData->m_gridOfs.x ) / pEnvData->m_gridSize.x ),
			floor( ( mousePos.y - pEnvData->m_gridOfs.y ) / pEnvData->m_gridSize.y ) );
		if( p.x < 0 || p.y < 0 || p.x >= pEnvData->m_nWidth || p.y >= pEnvData->m_nHeight )
			return;
		pEnvData->Fill( m_nCurSelectedValue, p );
		m_pEftPreviewNode->Fill( m_nCurSelectedValue, p );
		m_pEftPreviewNode->Init();
		return;
	}
	if( n == 'Q' )
	{
		m_nCurSelectedValue = -1;
		return;
	}
	if( n >= '0' && n <= '9' )
	{
		auto n1 = n - '0';
		if( n1 > GetData()->m_arrEnvDescs.Size() )
			return;
		m_nCurSelectedValue = n1;
		return;
	}
	if( n == 'B' )
	{
		GetView()->ToggleBattleEffect();
		return;
	}

	if( pEvent->nChar == 'P' )
	{
		auto n = m_vecEnvNodes.size();
		CPrefabNode* pEnvNode1 = m_vecEnvNodes.back();
		auto pNode = new CPrefabNode( GetRes() );
		pNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CLevelEnvEffect>()->strClassName.c_str() );
		m_pLevelNode->AddChildBefore( pNode, m_vecEnvNodes.back() );
		pNode->OnEditorActive( false );
		m_vecEnvNodes.push_back( pNode );
		n = Min( n, m_vecEnvNodes.size() - 1 );
		char sz[32];
		sprintf( sz, "env%d", n + 1 );
		pNode->SetName( sz );

		pNode->SetResource( pEnvNode1->GetResource() );
		auto pObj = (CLevelEnvEffect*)pEnvNode1->GetObjData();
		auto pData = (CLevelEnvEffect*)pNode->GetObjData();
		pData->m_gridSize = pObj->m_gridSize;
		pData->m_nWidth = pObj->m_nWidth;
		pData->m_nHeight = pObj->m_nHeight;
		pData->m_gridOfs = pObj->m_gridOfs;
		pData->m_arrEnvDescs = pObj->m_arrEnvDescs;
		for( int i = 0; i < pData->m_arrEnvMap.Size(); i++ )
		{
			if( pData->m_arrEnvMap[i] > pData->m_arrEnvDescs.Size() )
				pData->m_arrEnvMap[i] = 0;
		}
		SelectEnvNode( n );
	}
	else if( pEvent->nChar == 'A' )
		SelectEnvNode( m_nEnvNode < m_vecEnvNodes.size() - 1 ? m_nEnvNode + 1 : 0 );
	else if( pEvent->nChar == 'S' )
		SelectEnvNode( m_nEnvNode > 0 ? m_nEnvNode - 1 : m_vecEnvNodes.size() - 1 );
}

void CLevelEnvTool::OnImport()
{
	m_nImportType = 0;
	CFileSelectDialog::Inst()->Show( "pf", &m_onImportOK );
}

void CLevelEnvTool::OnImport1()
{
	m_nImportType = 1;
	CFileSelectDialog::Inst()->Show( "pf", &m_onImportOK );
}

void CLevelEnvTool::OnImport2()
{
	m_nImportType = 2;
	CFileSelectDialog::Inst()->Show( "pf", &m_onImportOK );
}

void CLevelEnvTool::OnImport3()
{
	m_nImportType = 3;
	CFileSelectDialog::Inst()->Show( "pf", &m_onImportOK );
}

void CLevelEnvTool::OnImportOK( const char* szText )
{
	if( szText )
	{
		CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szText );
		if( !pPrefab )
			return;
		if( pPrefab->GetRoot()->GetClassData() == CClassMetaDataMgr::Inst().GetClassData<CMyLevel>() )
		{
			auto pEnvNode1 = pPrefab->GetRoot()->GetChildByName<CPrefabNode>( "env" );
			if( !pEnvNode1 )
				return;
			if( pEnvNode1->GetClassData() != CClassMetaDataMgr::Inst().GetClassData<CLevelEnvEffect>() )
				return;
			m_pEnvNode->SetResource( pEnvNode1->GetResource() );
			auto pObj = (CLevelEnvEffect*)pEnvNode1->GetObjData();
			auto pData = GetData();

			if( m_nImportType == 0 || m_nImportType == 1 )
			{
				pData->m_nWidth = Max<int32>( 1, floor( pData->m_nWidth * pData->m_gridSize.x / pObj->m_gridSize.x + 0.5f ) );
				pData->m_nHeight = Max<int32>( 1, floor( pData->m_nHeight * pData->m_gridSize.y / pObj->m_gridSize.y + 0.5f ) );
				pData->m_arrEnvMap.Resize( pData->m_nWidth * pData->m_nHeight );
				pData->m_gridSize = pObj->m_gridSize;
				pData->m_arrEnvDescs = pObj->m_arrEnvDescs;
				pData->m_fScenarioFade = pObj->m_fScenarioFade;
				for( int i = 0; i < pData->m_arrEnvMap.Size(); i++ )
				{
					if( pData->m_arrEnvMap[i] > pData->m_arrEnvDescs.Size() )
						pData->m_arrEnvMap[i] = 0;
				}
			}
			if( m_nImportType == 0 || m_nImportType == 2 )
			{
				pData->m_gamma = pObj->m_gamma;
				for( int i = 0; i < ELEM_COUNT( pData->m_colorTranspose ); i++ )
					pData->m_colorTranspose[i] = pObj->m_colorTranspose[i];
				pData->m_bOverrideBackColor = pObj->m_bOverrideBackColor;
				pData->m_bCustomBattleEffectBackColor = pObj->m_bCustomBattleEffectBackColor;
				pData->m_backColor = pObj->m_backColor;
				pData->m_battleEffectBackColor = pObj->m_battleEffectBackColor;
			}
			if( m_nImportType == 0 || m_nImportType == 3 )
			{
				auto ofs = GetLevelData()->GetCamPos() - ( (CMyLevel*)pPrefab->GetRoot()->GetObjData() )->GetCamPos();
				pData->m_bCtrlPointValid = pObj->m_bCtrlPointValid;
				pData->m_fCtrlPointTransRemoveRot = pObj->m_fCtrlPointTransRemoveRot;
				pData->m_ctrlPoint1 = pObj->m_ctrlPoint1;
				pData->m_ctrlPoint2 = pObj->m_ctrlPoint2;
				pData->m_arrCtrlPoint = pObj->m_arrCtrlPoint;
				pData->m_arrCtrlLink = pObj->m_arrCtrlLink;
				pData->m_strCommonEvtScript = pObj->m_strCommonEvtScript;
				pData->m_ctrlPoint1.Offset( ofs );
				pData->m_ctrlPoint2.Offset( ofs );
				for( int i = 0; i < pData->m_arrCtrlPoint.Size(); i++ )
				{
					pData->m_arrCtrlPoint[i].Offset( ofs );
				}
			}

			m_pEftPreviewNode->RemoveThis();
			m_pEnvNode->OnEdit();
			m_pEftPreviewNode = SafeCast<CLevelEnvEffect>( m_pEnvNode->CreateInstance( false ) );
			m_pEnvNode->AddChild( m_pEftPreviewNode );
			m_pEftPreviewNode->Init();
			RefreshMasks();
			m_pObjectData->RefreshData();
		}
	}
}

void CLevelEnvTool::UpdateDrag( CUIViewport* pViewport, const TVector2<int32>& p )
{
	auto pEnvData = GetData();
	if( p.x < 0 || p.y < 0 || p.x >= pEnvData->m_nWidth || p.y >= pEnvData->m_nHeight )
		return;

	auto& grid = pEnvData->m_arrEnvMap[p.x + p.y * pEnvData->m_nWidth];
	if( grid == m_nCurSelectedValue )
		return;
	grid = m_nCurSelectedValue;
	m_pEftPreviewNode->m_arrEnvMap[p.x + p.y * pEnvData->m_nWidth] = m_nCurSelectedValue;
	m_pEftPreviewNode->Init();
}

void CLevelEnvTool::DebugDrawCtrlPoint( IRenderSystem* pRenderSystem, CUIViewport* pViewport, const SLevelCamCtrlPoint& p, int8 nType )
{
	CVector2 verts0[6] = { { -1, -1 }, { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 }, { 1, 1 } };
	CVector2 verts[6];
	for( int i = 0; i < ELEM_COUNT( verts ); i++ )
		verts[i] = verts0[i] * 8 + p.orig;
	pViewport->DebugDrawTriangles( pRenderSystem, 6, verts, CVector4( 0.3f, 0.5f, 0.8f, 1 ) );
	for( int i = 0; i < p.arrPath.Size(); i++ )
	{
		auto p1 = p.arrPath[i];
		auto p2 = p.arrPath[( i + 1 ) % p.arrPath.Size()];

		for( int i = 0; i < ELEM_COUNT( verts ); i++ )
			verts[i] = verts0[i] * 4 + CVector2( p1.x, p1.y );
		pViewport->DebugDrawTriangles( pRenderSystem, 6, verts, CVector4( 0.4f, 0.8f, 0.3f, 1 ) );
		pViewport->DebugDrawLine( pRenderSystem, CVector2( p1.x, p1.y ), CVector2( p2.x, p2.y ), CVector4( 0.4f, 0.8f, 0.3f, 0.5f ) );
		if( i == 0 )
			pViewport->DebugDrawLine( pRenderSystem, CVector2( p1.x, p1.y ), p.orig, CVector4( 0.3f, 0.5f, 0.8f, 0.5f ) );

		if( i < p.arrTangent.Size() )
		{
			auto t = p.arrTangent[i];
			pViewport->DebugDrawLine( pRenderSystem, CVector2( p1.x, p1.y ), CVector2( p1.x + t.x, p1.y + t.y ), CVector4( 0.3f, 0.8f, 0.6f, 0.4f ) );
		}
	}
}

void CLevelEnvTool::DebugDrawCtrlLink( IRenderSystem* pRenderSystem, CUIViewport* pViewport, const SLevelCamCtrlPointLink& l, const SLevelCamCtrlPoint& p1, const SLevelCamCtrlPoint& p2 )
{
	auto a = p1.orig + l.ofs1;
	auto b = p2.orig + l.ofs2;
	auto d1 = b - a;
	d1.Normalize();
	d1 = CVector2( d1.y, -d1.x ) * 2;
	pViewport->DebugDrawLine( pRenderSystem, a + d1, b + d1, CVector4( 0.8f, 0.7f, 0.2f, 1 ) );
	pViewport->DebugDrawLine( pRenderSystem, a - d1, b - d1, CVector4( 0.8f, 0.7f, 0.2f, 1 ) );
}

void CLevelEnvTool::RefreshMasks()
{
	auto pEnvData = GetData();
	CVector4 backColor( 0.08f, 0.08f, 0.08f, 0.5f );
	if( pEnvData->IsOverrideBackColor() )
	{
		backColor.x = pEnvData->GetBackColor().x;
		backColor.y = pEnvData->GetBackColor().y;
		backColor.z = pEnvData->GetBackColor().z;
	}
	if( pEnvData->m_bCustomBattleEffectBackColor )
		GetView()->SetMaskParams( backColor, pEnvData->m_battleEffectBackColor );
	else
		GetView()->SetMaskParams( backColor );

	auto pParams = GetView()->GetColorAdjustParams();
	
	pParams[0] = CVector4( 1 + pEnvData->m_colorTranspose[0].x, pEnvData->m_colorTranspose[0].y, pEnvData->m_colorTranspose[0].z, pow( 2, -pEnvData->m_gamma.x ) );
	pParams[1] = CVector4( pEnvData->m_colorTranspose[1].x, 1 + pEnvData->m_colorTranspose[1].y, pEnvData->m_colorTranspose[1].z, pow( 2, -pEnvData->m_gamma.y ) );
	pParams[2] = CVector4( pEnvData->m_colorTranspose[2].x, pEnvData->m_colorTranspose[2].y, 1 + pEnvData->m_colorTranspose[2].z, pow( 2, -pEnvData->m_gamma.z ) );
}

void CLevelEnvTool::SelectEnvNode( int32 n )
{
	if( m_nEnvNode == n )
		return;
	if( m_pEftPreviewNode )
	{
		m_pEftPreviewNode->RemoveThis();
		m_pEftPreviewNode = NULL;
	}
	if( m_pEnvNode )
	{
		m_pEnvNode->bVisible = false;
		m_pEnvNode->OnEdit();
	}
	m_nEnvNode = n;
	m_pEnvNode = m_vecEnvNodes[n];
	auto pData = GetData();
	pData->m_arrEnvMap.Resize( pData->m_nWidth * pData->m_nHeight );
	m_pEftPreviewNode = SafeCast<CLevelEnvEffect>( m_pEnvNode->CreateInstance( false ) );
	m_pEnvNode->AddChild( m_pEftPreviewNode );
	m_pEnvNode->bVisible = true;
	m_pEftPreviewNode->Init();

	m_pLayerScript->SetText( m_pEnvNode->GetName() );
	m_pLayerScriptText->SetText( pData->GetCondition() );

	if( m_onNodeObjDataChanged.IsRegistered() )
		m_onNodeObjDataChanged.Unregister();
	m_pObjectData = CObjectDataEditMgr::Inst().Create( m_pNodeView, NULL, (uint8*)pData, m_pEnvNode->GetClassData() );
	if( m_pObjectData )
		m_pObjectData->Register( &m_onNodeObjDataChanged );
	RefreshMasks();
}

void CLevelEnvTool::OnLayerScript()
{
	auto p = m_pEnvNode->GetStaticDataSafe<CLevelEnvEffect>();
	if( !p )
		return;
	CTextEditDialog::Inst()->Show( Utf8ToUnicode( p->GetCondition() ).c_str(), &m_onLayerScriptEditOK );
}

void CLevelEnvTool::OnLayerScriptEditOK( const wchar_t * sz )
{
	if( sz )
	{
		static_cast<CLevelEnvEffect*>( m_pEnvNode->GetFinalObjData() )->m_strCondition = UnicodeToUtf8( sz ).c_str();
		m_pEnvNode->OnEdit();
	}
}

void CLevelEnvTool::OnLayerScriptText()
{
	auto p = m_pEnvNode->GetStaticDataSafe<CLevelEnvEffect>();
	if( !p )
		return;
	static_cast<CLevelEnvEffect*>( m_pEnvNode->GetFinalObjData() )->m_strCondition = UnicodeToUtf8( m_pLayerScriptText->GetText() ).c_str();
	m_pEnvNode->OnEdit();
}

void CLevelEnvTool::OnCurNodeObjDataChanged( int32 nAction )
{
	if( nAction != 1 )
		return;

	m_pEftPreviewNode->RemoveThis();
	m_pEnvNode->OnEdit();
	m_pEftPreviewNode = SafeCast<CLevelEnvEffect>( m_pEnvNode->CreateInstance( false ) );
	m_pEnvNode->AddChild( m_pEftPreviewNode );
	m_pEftPreviewNode->Init();
	RefreshMasks();
}

void CLevelToolsView::OnInited()
{
	m_pViewport = GetChildByName<CUIViewport>( "viewport" );
	m_pViewport->SetLight( false );

	m_onViewportStartDrag.Set( this, &CLevelToolsView::OnViewportStartDrag );
	m_pViewport->Register( eEvent_StartDrag, &m_onViewportStartDrag );
	m_onViewportDragged.Set( this, &CLevelToolsView::OnViewportDragged );
	m_pViewport->Register( eEvent_Dragged, &m_onViewportDragged );
	m_onViewportStopDrag.Set( this, &CLevelToolsView::OnViewportStopDrag );
	m_pViewport->Register( eEvent_StopDrag, &m_onViewportStopDrag );
	m_onViewportMouseUp.Set( this, &CLevelToolsView::OnViewportMouseUp );
	m_pViewport->Register( eEvent_MouseUp, &m_onViewportMouseUp );
	m_onDebugDraw.Set( this, &CLevelToolsView::OnDebugDraw );
	m_pViewport->Register( eEvent_Action, &m_onDebugDraw );
	m_onViewportKey.Set( this, &CLevelToolsView::OnViewportKey );
	m_pViewport->Register( eEvent_Key, &m_onViewportKey );
	m_onViewportChar.Set( this, &CLevelToolsView::OnViewportChar );
	m_pViewport->Register( eEvent_Char, &m_onViewportChar );
	auto pOK = GetChildByName<CUIElement>( "OK" );
	m_onOK.Set( this, &CLevelToolsView::OnOK );
	pOK->Register( eEvent_Action, &m_onOK );

	m_pPlaceHolder = new CRenderObject2D;

	auto pTileTool = new CTileTool;
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/tile_tool.xml" )->GetElement()->Clone( pTileTool );
	pTileTool->bVisible = false;
	AddChild( pTileTool );
	m_vecTools.push_back( pTileTool );
	auto pPawnTool = new CPawnTool;
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/pawn_tool.xml" )->GetElement()->Clone( pPawnTool );
	pPawnTool->bVisible = false;
	AddChild( pPawnTool );
	m_vecTools.push_back( pPawnTool );
	auto pLevelEnvTool = new CLevelEnvTool;
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/env_tool.xml" )->GetElement()->Clone( pLevelEnvTool );
	pLevelEnvTool->bVisible = false;
	AddChild( pLevelEnvTool );
	m_vecTools.push_back( pLevelEnvTool );

	static CDefaultDrawable2D* pDrawable = NULL;
	static CDefaultDrawable2D* pDrawable1 = NULL;
	static CDefaultDrawable2D* pDrawable2 = NULL;
	static CDefaultDrawable2D* pDrawable3 = NULL;
	if( !pDrawable )
	{
		vector<char> content;
		GetFileContent( content, "EditorRes/Drawables/mask.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "back" ) );
		pDrawable1 = new CDefaultDrawable2D;
		pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "mask" ) );
		pDrawable2 = new CDefaultDrawable2D;
		pDrawable2->LoadXml( doc.RootElement()->FirstChildElement( "battle_effect" ) );
		pDrawable3 = new CDefaultDrawable2D;
		pDrawable3->LoadXml( doc.RootElement()->FirstChildElement( "color_adjust" ) );
	}
	m_pBackColor = new CImage2D( pDrawable, NULL, CRectangle( -2048, -2048, 4096, 4096 ), CRectangle( 0, 0, 1, 1 ) );
	m_pViewport->GetRoot()->AddChild( m_pBackColor );
	m_pMask = new CImage2D( pDrawable1, NULL, CRectangle( -2048, -2048, 4096, 4096 ), CRectangle( 0, 0, 1, 1 ) );
	m_pViewport->GetRoot()->AddChild( m_pMask );
	m_pBattleEffect = new CImage2D( pDrawable2, NULL, CRectangle( -2048, -2048, 4096, 4096 ), CRectangle( 0, 0, 1, 1 ) );
	m_pViewport->GetRoot()->AddChild( m_pBattleEffect );
	m_pColorAdjust = new CImage2D( pDrawable3, NULL, CRectangle( -2048, -2048, 4096, 4096 ), CRectangle( 0, 0, 1, 1 ) );
	m_pViewport->GetRoot()->AddChild( m_pColorAdjust );
	m_pColorAdjust->SetZOrder( 1 );
	static_cast<CImage2D*>( m_pBackColor.GetPtr() )->SetParam( 1, NULL, 0, 1, 0, 0, 0, 0 );
	static_cast<CImage2D*>( m_pMask.GetPtr() )->SetParam( 2, NULL, 0, 2, 0, 0, 0, 0 );
	static_cast<CImage2D*>( m_pBattleEffect.GetPtr() )->SetParam( 2, NULL, 0, 2, 0, 0, 0, 0 );
	static_cast<CImage2D*>( m_pColorAdjust.GetPtr() )->SetParam( 3, NULL, 0, 3, 0, 0, 0, 0 );
	m_pBattleEffect->bVisible = false;
	ResetMasks();

	SetVisible( false );
}

void CLevelToolsView::Set( CPrefabNode* p, function<void()> FuncOK, SLevelData* pLevelData, CRenderObject2D* pBack )
{
	m_pLevelNode = p;
	m_pRes = p->GetPrefab();
	m_pLevelData = pLevelData;
	m_pBack = pBack;
	if( m_pBack )
	{
		m_pBack->SetPosition( m_pLevelData->displayOfs * -1 );
		m_pViewport->GetRoot()->AddChildAfter( m_pBack, m_pMask );
	}
	m_pPlaceHolder->x = p->x;
	m_pPlaceHolder->y = p->y;
	m_pPlaceHolder->r = p->r;
	m_pPlaceHolder->s = p->s;
	m_pPlaceHolder->SetZOrder( p->GetZOrder() );
	m_pLevelNode->GetParent()->AddChildBefore( m_pPlaceHolder, p );
	p->RemoveThis();
	p->x = p->y = p->r = 0;
	p->s = 1;
	m_pViewport->GetRoot()->AddChild( p );

	FixLevelData( m_pLevelNode, m_pRes );
	auto pObj = (CMyLevel*)m_pLevelNode->GetObjData();
	m_pViewport->GetCamera().SetPosition( pObj->m_nWidth * 0.5f * LEVEL_GRID_SIZE_X, pObj->m_nHeight * 0.5f * LEVEL_GRID_SIZE_Y );
	m_vecTileDrawable.resize( pObj->m_arrTileData.Size() );
	for( int i = 0; i < m_vecTileDrawable.size(); i++ )
	{
		if( !pObj->m_arrTileData[i].pTileDrawable.length() )
			continue;
		m_vecTileDrawable[i] = CResourceManager::Inst()->CreateResource<CDrawableGroup>( pObj->m_arrTileData[i].pTileDrawable.c_str() );
	}
	auto p1 = m_pLevelNode->GetChildByName<CPrefabNode>( "1" );
	m_pTileRoot = new CRenderObject2D;
	m_pTileRoot->SetZOrder( -1 );
	m_pLevelNode->AddChildAfter( m_pTileRoot, p1 );
	m_vecTiles.resize( pObj->m_nWidth * pObj->m_nHeight );
	RefreshAllTiles();

	CRectangle lvRect = pObj->GetMainAreaSize();
	lvRect.SetSize( lvRect.GetSize() + CVector2( 64, 64 ) );
	static_cast<CImage2D*>( m_pBattleEffect.GetPtr() )->SetRect( lvRect );
	m_pBattleEffect->SetBoundDirty();

	ResetMasks();
	m_nCurTool = 0;
	m_vecTools[m_nCurTool]->SetVisible( true );
	m_FuncOK = FuncOK;
	GetMgr()->DoModal( this );
}

void CLevelToolsView::AddNeighbor( CPrefabNode* p, const CVector2& displayOfs )
{
	int32 nIndex = 0;
	for( int i = 0; i < m_vecNeightborData.size(); i++ )
	{
		if( m_vecNeightborData[i].pLevelNode == p )
			return;
	}
	auto pRes = p->GetPrefab();
	FixLevelData( p, pRes );
	auto pPreview = CreateLevelSimplePreview( p );
	SNeighborData data = { p, pPreview, displayOfs };
	m_vecNeightborData.push_back( data );
	pPreview->SetPosition( displayOfs );
	m_pViewport->GetRoot()->AddChildAfter( pPreview, m_pMask );
}

CVector2 CLevelToolsView::GetViewportMousePos()
{
	return m_pViewport->GetScenePos( GetMgr()->GetMousePos() );
}

void CLevelToolsView::OnDebugDraw( IRenderSystem* pRenderSystem )
{
	auto pObj = (CMyLevel*)m_pLevelNode->GetObjData();
	if( !pObj->m_nWidth || !pObj->m_nHeight )
		return;
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	CVector2 ofs1[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	CVector4 colors[] = { { 1, 1, 0, 1 }, { 0, 1, 1, 1 }, { 1, 0, 1, 1 }, { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 } };

	int32 nNextStageCount = pObj->m_arrNextStage.Size();
	auto& grids = pObj->m_arrGridData;
	for( int x = 0; x < pObj->m_nWidth; x++ )
	{
		for( int y = 0; y < pObj->m_nHeight; y++ )
		{
			auto& grid = grids[x + y * pObj->m_nWidth];

			auto n = Max( 0, Min( nNextStageCount, grid.nNextStage ) );
			if( n <= 0 )
				continue;

			auto color = colors[( n - 1 ) % ELEM_COUNT( colors )];

			for( int j = 0; j < 2; j++ )
			{
				auto pt1 = CVector2( x + 0.5f, y + 0.5f ) + ofs1[j] * 0.25f;
				auto pt2 = CVector2( x + 0.5f, y + 0.5f ) + ofs1[( j + 2 ) % 4] * 0.25f;
				m_pViewport->DebugDrawLine( pRenderSystem, pt1 * LEVEL_GRID_SIZE, pt2 * LEVEL_GRID_SIZE, color );
			}
		}
	}

	GetCurTool()->OnDebugDraw( pRenderSystem, m_pViewport );
}

void CLevelToolsView::OnViewportStartDrag( SUIMouseEvent* pEvent )
{
	CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
	if( GetCurTool()->OnViewportStartDrag( m_pViewport, fixOfs ) )
		m_bToolDrag = true;
}

void CLevelToolsView::OnViewportDragged( SUIMouseEvent* pEvent )
{
	if( !m_bToolDrag )
		return;
	CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
	GetCurTool()->OnViewportDragged( m_pViewport, fixOfs );
}

void CLevelToolsView::OnViewportStopDrag( SUIMouseEvent* pEvent )
{
	if( !m_bToolDrag )
		return;
	CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
	GetCurTool()->OnViewportStopDrag( m_pViewport, fixOfs );
}

void CLevelToolsView::OnViewportMouseUp( SUIMouseEvent* pEvent )
{
	auto pDragDropObj = GetMgr()->GetDragDropObject();
	if( pDragDropObj )
		OnViewportDrop( pEvent->mousePos, pDragDropObj );
}

void CLevelToolsView::OnViewportDrop( const CVector2& mousePos, CUIElement* pParam )
{
	CVector2 fixOfs = m_pViewport->GetScenePos( mousePos );
	GetCurTool()->OnViewportDrop( m_pViewport, fixOfs, pParam );
}

void CLevelToolsView::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( !pEvent->bKeyDown )
		return;
	if( pEvent->nChar >= VK_F1 && pEvent->nChar <= VK_F12 )
	{
		auto nTool = pEvent->nChar - VK_F1;
		if( nTool < m_vecTools.size() && nTool != m_nCurTool )
		{
			GetCurTool()->SetVisible( false );
			m_bToolDrag = false;
			ResetMasks();
			m_nCurTool = nTool;
			GetCurTool()->SetVisible( true );
		}
	}
	GetCurTool()->OnViewportKey( pEvent );
}

void CLevelToolsView::OnViewportChar( uint32 nChar )
{
	GetCurTool()->OnViewportChar( nChar );
}

void CLevelToolsView::OnOK()
{
	m_pLevelNode->OnEdit();
	DEFINE_TEMP_REF( m_pLevelNode );
	DEFINE_TEMP_REF( m_pRes );
	m_vecTools[m_nCurTool]->SetVisible( false );
	for( auto& data : m_vecNeightborData )
		data.pSimplePreview->RemoveThis();
	m_vecNeightborData.resize( 0 );
	m_pLevelNode->RemoveThis();
	m_pLevelNode->x = m_pPlaceHolder->x;
	m_pLevelNode->y = m_pPlaceHolder->y;
	m_pLevelNode->r = m_pPlaceHolder->r;
	m_pLevelNode->s = m_pPlaceHolder->s;
	m_pLevelNode->SetZOrder( m_pPlaceHolder->GetZOrder() );
	m_pPlaceHolder->GetParent()->AddChildBefore( m_pLevelNode, m_pPlaceHolder );
	m_pPlaceHolder->RemoveThis();
	m_pLevelNode = NULL;
	m_pRes = NULL;
	m_pLevelData = NULL;
	if( m_pBack )
	{
		m_pBack->RemoveThis();
		m_pBack = NULL;
	}
	m_vecTiles.resize( 0 );
	m_vecTileDrawable.resize( 0 );
	m_pTileRoot->RemoveThis();
	m_pTileRoot = NULL;
	GetMgr()->EndModal();
	m_FuncOK();
}

CVector4* CLevelToolsView::GetColorAdjustParams()
{
	return static_cast<CImage2D*>( m_pColorAdjust.GetPtr() )->GetParam();
}

void CLevelToolsView::SetMaskParams( const CVector4& backColor, const CVector3& battleEffectColor )
{
	static_cast<CImage2D*>( m_pBackColor.GetPtr() )->GetParam()[0] = backColor;
	static_cast<CImage2D*>( m_pMask.GetPtr() )->GetParam()[0] = CVector4( backColor.w, backColor.w, backColor.w, 0 );
	static_cast<CImage2D*>( m_pMask.GetPtr() )->GetParam()[1] = backColor * CVector4( 1 - backColor.w, 1 - backColor.w, 1 - backColor.w, 0 );

	auto pParams = static_cast<CImage2D*>( m_pBattleEffect.GetPtr() )->GetParam();
	CVector4 color1( battleEffectColor.x, battleEffectColor.y, battleEffectColor.z, 0 );
	pParams[0] = color1;
	pParams[1] = color1 * backColor * -1;
	pParams[1].w = 16;
}

void CLevelToolsView::ResizeLevel( const TRectangle<int32>& newSize )
{
	auto pObj = (CMyLevel*)m_pLevelNode->GetObjData();
	int32 nOrigWidth = pObj->m_nWidth;
	int32 nOrigHeight = pObj->m_nHeight;
	auto origData = pObj->m_arrGridData;

	pObj->m_nWidth = newSize.width;
	pObj->m_nHeight = newSize.height;
	pObj->m_arrGridData.Resize( newSize.width * newSize.height );
	for( int i = 0; i < newSize.width; i++ )
	{
		for( int j = 0; j < newSize.height; j++ )
		{
			auto x = i + newSize.x;
			auto y = j + newSize.y;
			auto& data = pObj->m_arrGridData[i + j * newSize.width];
			if( x >= 0 && y >= 0 && x < nOrigWidth && y < nOrigHeight && !( i == 0 && !!( ( i + j ) & 1 ) || i == newSize.width - 1 && !( ( i + j ) & 1 ) ) )
				data = origData[x + y * nOrigWidth];
			else
			{
				data.nTile = 0;
				data.bBlocked = true;
				data.nNextStage = 0;
			}
		}
	}
	auto d = CVector2( newSize.x, newSize.y ) * LEVEL_GRID_SIZE;
	if( m_pLevelData )
		m_pLevelData->displayOfs = m_pLevelData->displayOfs + d;
	for( auto p = m_pLevelNode->Get_RenderChild(); p; p = p->NextRenderChild() )
	{
		if( p == m_pLevelNode->GetRenderObject() )
			continue;
		auto pPrefabNode = dynamic_cast<CPrefabNode*>( p );
		if( pPrefabNode && pPrefabNode->GetStaticDataSafe<CLevelEnvEffect>() )
		{
			auto pEnvData = (CLevelEnvEffect*)pPrefabNode->GetObjData();
			pEnvData->m_gridOfs = pEnvData->m_gridOfs - d;
		}
	}

	for( int i = 0; i < pObj->m_arrNextStage.Size(); i++ )
	{
		pObj->m_arrNextStage[i].nOfsX -= newSize.x;
		pObj->m_arrNextStage[i].nOfsY -= newSize.y;
	}
	for( auto& item : m_vecNeightborData )
	{
		item.pSimplePreview->SetPosition( item.pSimplePreview->GetPosition() - d );
		auto p = (CMyLevel*)item.pLevelNode->GetObjData();
		for( int i = 0; i < p->m_arrNextStage.Size(); i++ )
		{
			if( p->m_arrNextStage[i].pNxtStage == m_pRes->GetName() )
			{
				p->m_arrNextStage[i].nOfsX += newSize.x;
				p->m_arrNextStage[i].nOfsY += newSize.y;
			}
		}
	}
	if( m_pBack )
		m_pBack->SetPosition( m_pBack->GetPosition() - d );

	for( auto& pTile : m_vecTiles )
	{
		if( pTile )
		{
			pTile->RemoveThis();
			pTile = NULL;
		}
	}
	m_vecTiles.resize( pObj->m_nWidth * pObj->m_nHeight );
	m_pViewport->GetCamera().SetPosition( pObj->m_nWidth * 0.5f * LEVEL_GRID_SIZE_X, pObj->m_nHeight * 0.5f * LEVEL_GRID_SIZE_Y );
	RefreshAllTiles();

	auto FuncMovePawn = [d] ( CPrefabNode* pPawnRoot ) {
		for( auto pChild = pPawnRoot->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
		{
			if( pChild == pPawnRoot->GetRenderObject() )
				continue;
			auto pPrefabNode = static_cast<CPrefabNode*>( pChild );
			if( !pPrefabNode->GetPatchedNode() || !pPrefabNode->GetPatchedNode()->GetStaticDataSafe<CPawn>() )
				continue;
			pPrefabNode->SetPosition( pPrefabNode->GetPosition() - d );
		}
	};
	FuncMovePawn( m_pLevelNode->GetChildByName<CPrefabNode>( "1" ) );
	for( auto p = m_pLevelNode->Get_RenderChild(); p; p = p->NextRenderChild() )
	{
		if( p == m_pLevelNode->GetRenderObject() )
			continue;
		auto pPrefabNode = dynamic_cast<CPrefabNode*>( p );
		if( pPrefabNode && pPrefabNode->GetStaticDataSafe<CPawnLayer>() )
			FuncMovePawn( pPrefabNode );
	}
}

void CLevelToolsView::RefreshTile( int32 x, int32 y )
{
	if( !!( 1 & ( x + y ) ) )
		return;
	auto pObj = (CMyLevel*)m_pLevelNode->GetObjData();

	auto& pTile = m_vecTiles[x + y * pObj->m_nWidth];
	if( pTile )
	{
		pTile->RemoveThis();
		pTile = NULL;
	}
	auto& tile = pObj->m_arrGridData[x + y * pObj->m_nWidth];
	tile.nTile = Min<int32>( tile.nTile, pObj->m_arrTileData.Size() - 1 );
	auto& tileData = pObj->m_arrTileData[tile.nTile];
	if( m_vecTileDrawable[tile.nTile] )
	{
		auto pImg = static_cast<CImage2D*>( m_vecTileDrawable[tile.nTile]->CreateInstance() );
		pTile = pImg;
		auto rect = pImg->GetElem().rect;
		rect.width *= tileData.texRect.width;
		rect.height *= tileData.texRect.height;
		pImg->SetRect( rect );
		pImg->SetTexRect( tileData.texRect );
		pImg->SetPosition( CVector2( x, y ) * LEVEL_GRID_SIZE );
		m_pTileRoot->AddChild( pImg );
	}
}

void CLevelToolsView::RefreshAllTiles()
{
	auto pObj = (CMyLevel*)m_pLevelNode->GetObjData();
	for( int x = 0; x < pObj->m_nWidth; x++ )
	{
		for( int y = 0; y < pObj->m_nHeight; y++ )
		{
			RefreshTile( x, y );
		}
	}
}

void CLevelToolsView::ResetMasks()
{
	SetMaskParams( CVector4( 0.08f, 0.08f, 0.08f, 0.5f ) );
	auto pParams = GetColorAdjustParams();
	pParams[0] = CVector4( 1, 0, 0, 1 );
	pParams[1] = CVector4( 0, 1, 0, 1 );
	pParams[2] = CVector4( 0, 0, 1, 1 );
	m_pBattleEffect->bVisible = false;
}

CPrefab* CLevelToolsView::NewLevelFromTemplate( CPrefab* pTemplate, const char* szFileName, int32 nWidth, int32 nHeight )
{
	auto pTemplateData = pTemplate->GetRoot()->GetStaticDataSafe<CMyLevel>();
	if( !pTemplateData )
		return NULL;
	if( !SaveFile( szFileName, NULL, 0 ) )
		return NULL;
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szFileName, true );
	auto pLevelNode = new CPrefabNode( pPrefab );
	pPrefab->SetNode( pLevelNode );
	pLevelNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CMyLevel>()->strClassName.c_str() );
	auto pLevelData = SafeCast<CMyLevel>( pLevelNode->GetFinalObjData() );
	pLevelData->m_nWidth = nWidth;
	pLevelData->m_nHeight = nHeight;
	pLevelData->m_camPos = CVector2( pLevelData->m_nWidth * 0.5f, pLevelData->m_nHeight * 0.5f ) * LEVEL_GRID_SIZE;
	pLevelData->m_arrTileData = pTemplateData->m_arrTileData;
	pLevelData->m_pTileDrawable = pTemplateData->m_pTileDrawable;
	pLevelData->m_arrGridData.Resize( nWidth * nHeight );
	for( int i = 0; i < pLevelData->m_arrGridData.Size(); i++ )
		pLevelData->m_arrGridData[i].bBlocked = true;
	pLevelData->m_strBeginScript = pTemplateData->m_strBeginScript;
	pLevelData->m_nBGMPriority = pTemplateData->m_nBGMPriority;

	auto p1 = new CPrefabNode( pPrefab );
	p1->SetName( "1" );
	pLevelNode->AddChild( p1 );
	p1->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CEntity>()->strClassName.c_str() );

	auto pTemplateEnv = pTemplate->GetRoot()->GetChildByName<CPrefabNode>( "env" );
	if( pTemplateEnv && pTemplateEnv->GetClassData() == CClassMetaDataMgr::Inst().GetClassData<CLevelEnvEffect>() )
	{
		auto pTemplateEnvData = (CLevelEnvEffect*)pTemplateEnv->GetObjData();
		auto pEnvNode = new CPrefabNode( pPrefab );
		pEnvNode->SetName( "env" );
		pLevelNode->AddChild( pEnvNode );
		pEnvNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CLevelEnvEffect>()->strClassName.c_str() );
		pEnvNode->SetResource( pTemplateEnv->GetResource() );
		auto pEnvData = (CLevelEnvEffect*)pEnvNode->GetObjData();
		pEnvData->m_gridSize = pTemplateEnvData->m_gridSize;
		pEnvData->m_arrEnvDescs = pTemplateEnvData->m_arrEnvDescs;
		pEnvData->m_nWidth = pLevelData->GetSize().x * LEVEL_GRID_SIZE_X / pEnvData->m_gridSize.x;
		pEnvData->m_nHeight = pLevelData->GetSize().y * LEVEL_GRID_SIZE_Y / pEnvData->m_gridSize.y;
		pEnvData->m_arrEnvMap.Resize( pEnvData->m_nWidth * pEnvData->m_nHeight );
	}

	return pPrefab;
}

CRenderObject2D* CLevelToolsView::CreateLevelSimplePreview( CPrefabNode* pNode )
{
	CRenderObject2D* p = new CRenderObject2D;
	auto pObj = (CMyLevel*)pNode->GetObjData();

	vector<CReference<CDrawableGroup> > vecDrawables;
	vecDrawables.resize( pObj->m_arrTileData.Size() );
	for( int i = 0; i < pObj->m_arrTileData.Size(); i++ )
	{
		if( !pObj->m_arrTileData[i].pTileDrawable.length() )
			continue;
		vecDrawables[i] = CResourceManager::Inst()->CreateResource<CDrawableGroup>( pObj->m_arrTileData[i].pTileDrawable.c_str() );
	}

	for( int x = 0; x < pObj->m_nWidth; x++ )
	{
		for( int y = 0; y < pObj->m_nHeight; y++ )
		{
			if( !!( 1 & ( x + y ) ) )
				continue;
			auto& tile = pObj->m_arrGridData[x + y * pObj->m_nWidth];
			tile.nTile = Min<int32>( tile.nTile, pObj->m_arrTileData.Size() - 1 );
			auto& tileData = pObj->m_arrTileData[tile.nTile];

			if( vecDrawables[tile.nTile] )
			{
				auto pImg = static_cast<CImage2D*>( vecDrawables[tile.nTile]->CreateInstance() );
				auto rect = pImg->GetElem().rect;
				rect.width *= tileData.texRect.width;
				rect.height *= tileData.texRect.height;
				pImg->SetRect( rect );
				pImg->SetTexRect( tileData.texRect );
				pImg->SetPosition( CVector2( x, y ) * LEVEL_GRID_SIZE );
				p->AddChild( pImg );
			}
		}
	}

	auto p1 = pNode->GetChildByName<CPrefabNode>( "1" );
	vector<CReference<CPrefabBaseNode> > vecPawns;
	if( p1 )
	{
		for( auto pChild = p1->Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			if( pChild == p1->GetRenderObject() )
				continue;
			auto pPrefabNode = static_cast<CPrefabNode*>( pChild );
			auto pFinalClassData = pPrefabNode->GetFinalClassData();
			if( pFinalClassData && pFinalClassData->Is( CClassMetaDataMgr::Inst().GetClassData<CLevelSpawnHelper>() ) )
			{
				vecPawns.push_back( SafeCast<CLevelSpawnHelper>( pPrefabNode->CreateInstance( false ) ) );
				continue;
			}
			if( pFinalClassData && pFinalClassData->Is( CClassMetaDataMgr::Inst().GetClassData<CPawn>() ) )
			{
				auto pPawn = SafeCast<CPawn>( pPrefabNode->CreateInstance( false ) );
				pPawn->m_pos = pPawn->m_moveTo = TVector2<int32>( floor( pPawn->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pPawn->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
				pPawn->m_nCurDir = pPawn->m_nInitDir;
				vecPawns.push_back( pPawn );
				continue;
			}
		}
	}
	std::sort( vecPawns.begin(), vecPawns.end(), [] ( CPrefabBaseNode* a, CPrefabBaseNode* b ) {
		if( a->y > b->y )
			return true;
		if( a->y < b->y )
			return false;
		auto pPawnA = SafeCast<CPawn>( a );
		if( !pPawnA )
			pPawnA = SafeCast<CPawn>( SafeCast<CLevelSpawnHelper>( a )->GetRenderObject() );
		auto pPawnB = SafeCast<CPawn>( b );
		if( !pPawnB )
			pPawnB = SafeCast<CPawn>( SafeCast<CLevelSpawnHelper>( b )->GetRenderObject() );
		return pPawnA->GetRenderOrder() < pPawnB->GetRenderOrder();
	} );
	for( CPrefabBaseNode* pPawn : vecPawns )
	{
		p->AddChild( pPawn );
		pPawn->OnPreview();
	}

	return p;
}

void CLevelToolsView::FixLevelData( CPrefabNode* pNode, CPrefab* pRes )
{
	auto pObj = (CMyLevel*)pNode->GetObjData();
	if( !pObj->m_arrTileData.Size() )
	{
		pObj->m_arrTileData.Resize( 2 );
		pObj->m_arrTileData[0].bBlocked = true;
		pObj->m_arrTileData[1].pTileDrawable = pObj->m_pTileDrawable;
		pObj->m_arrTileData[1].texRect = CRectangle( 0, 0, 1, 1 );
		for( int i = 0; i < pObj->m_arrGridData.Size(); i++ )
			pObj->m_arrGridData[i].nTile = pObj->m_arrGridData[i].bBlocked ? 0 : 1;
	}

	for( int i = 0; i < pObj->m_arrGridData.Size(); i++ )
	{
		pObj->m_arrGridData[i].nTile = Max( 0, Min<int32>( pObj->m_arrGridData[i].nTile, pObj->m_arrTileData.Size() - 1 ) );
		pObj->m_arrGridData[i].nNextStage = Max( 0, Min<int32>( pObj->m_arrGridData[i].nNextStage, pObj->m_arrNextStage.Size() ) );
	}

	auto p1 = pNode->GetChildByName<CPrefabNode>( "1" );
	if( !p1 )
	{
		p1 = new CPrefabNode( pRes );
		p1->SetName( "1" );
		pNode->AddChild( p1 );
		p1->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CEntity>()->strClassName.c_str() );
		p1->OnEditorActive( false );
	}
}
