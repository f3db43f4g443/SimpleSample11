#include "stdafx.h"
#include "WorldCfgEditor.h"
#include "LevelTools.h"
#include "Editor/Editor.h"
#include "Common/Utf8Util.h"

void CWorldCfgEditor::NewFile( const char* szFileName )
{
	if( m_pData )
		delete m_pData;
	CBufFile buf;
	( (SClassMetaData*)NULL )->PackData( NULL, buf, true );
	m_pData = (SWorldCfg*)CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>()->NewObjFromData( buf, true, NULL );
	Validate( szFileName );
	m_nCurSelected = -1;
	Super::NewFile( szFileName );
}

void CWorldCfgEditor::OnOpenFile()
{
	auto szLevel = GetParam( "level" );
	if( szLevel )
	{
		auto& levelData = m_pRes->m_pWorldCfg->arrLevelData;
		for( int i = 0; i < levelData.Size(); i++ )
		{
			m_nCurSelected = i;
			SetCamOfs( levelData[i].displayOfs );
			return;
		}
	}
}

void CWorldCfgEditor::Refresh()
{
	for( auto& lvl : m_vecLevelData )
	{
		if( lvl.pLevelPreview )
			lvl.pLevelPreview->RemoveThis();
		if( lvl.pClonedLevelData )
		{
			lvl.pClonedLevelData->Invalidate();
			lvl.pClonedLevelData->RemoveThis();
		}
	}
	m_vecLevelData.resize( 0 );
	if( m_pRes )
	{
		auto nSelect = m_nCurSelected;
		m_nCurSelected = -1;
		CBufFile buf;
		auto pClassData = CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>();
		pClassData->PackData( (uint8*)m_pRes->m_pWorldCfg, buf, true );
		m_pData = (SWorldCfg*)pClassData->NewObjFromData( buf, true, NULL );
		Validate( m_pRes->GetName() );
		nSelect = Min<int32>( nSelect, m_pData->arrLevelData.Size() - 1 );
		Select( nSelect );
	}
	else
	{
		if( m_pData )
		{
			delete m_pData;
			m_pData = NULL;
		}
		m_nCurSelected = -1;
	}
}

void CWorldCfgEditor::OnInited()
{
	Super::OnInited();
	m_pPanel[0] = GetChildByName<CUIElement>( "0" );
	m_onSave.Set( this, &CWorldCfgEditor::Save );
	m_pPanel[0]->GetChildByName<CUIButton>( "save" )->Register( eEvent_Action, &m_onSave );
	m_onNewLevel.Set( this, &CWorldCfgEditor::BeginNewLevel );
	m_pPanel[0]->GetChildByName<CUIButton>( "new" )->Register( eEvent_Action, &m_onNewLevel );

	m_pNewTemplate = CFileNameEdit::Create( "Template", "pf", 80 );
	m_pNewTemplate->Replace( m_pPanel[0]->GetChildByName( "new_template_name" ) );

	m_pPanel[1] = GetChildByName<CUIElement>( "1" );
	m_pPanel[1]->SetVisible( false );
	m_pNewLevelName = m_pPanel[1]->GetChildByName<CUITextBox>( "file_name" );
	m_onNewLevelOK.Set( this, &CWorldCfgEditor::OnNewLevelOK );
	m_pPanel[1]->GetChildByName<CUIButton>( "ok" )->Register( eEvent_Action, &m_onNewLevelOK );
	m_onNewLevelCancel.Set( this, &CWorldCfgEditor::OnNewLevelCancel );
	m_pPanel[1]->GetChildByName<CUIButton>( "cancel" )->Register( eEvent_Action, &m_onNewLevelCancel );
}

void CWorldCfgEditor::Save()
{
	Super::Save();
	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		if( m_vecLevelData[i].bDirty )
		{
			m_vecLevelData[i].bDirty = false;
			CPrefab* pPrefab = m_pData->arrLevelData[i].pLevel;
			CBufFile buf;
			pPrefab->Save( buf );
			SaveFile( pPrefab->GetName(), buf.GetBuffer(), buf.GetBufLen() );
		}
	}
}

void CWorldCfgEditor::RefreshPreview()
{
	if( !m_pRes )
		return;
	m_pRes->RefreshBegin();
	if( m_pRes->m_pWorldCfg )
	{
		delete m_pRes->m_pWorldCfg;
		m_pRes->m_pWorldCfg = NULL;
	}

	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		if( m_vecLevelData[i].bDirty )
		{
			RefreshLevelData( i );
			CPrefab* pPrefab = m_pData->arrLevelData[i].pLevel;
			pPrefab->RefreshBegin();
			auto pNode1 = m_vecLevelData[i].pClonedLevelData->Clone( pPrefab );
			pNode1->SetPosition( CVector2( 0, 0 ) );
			pPrefab->SetNode( pNode1 );
			pPrefab->RefreshEnd();
		}
	}

	CBufFile buf;
	auto pClassData = CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>();
	pClassData->PackData( (uint8*)m_pData, buf, true );
	m_pRes->m_pWorldCfg = (SWorldCfg*)pClassData->NewObjFromData( buf, true, NULL );
	m_pRes->RefreshEnd();
}

void CWorldCfgEditor::Validate( const char* szName )
{
	string strPath = szName;
	int32 n = strPath.find_last_of( '/' );
	strPath = strPath.substr( 0, n + 1 );
	string strFind = strPath + "*.pf";
	map<string, CReference<CPrefab> > mapLevelPrefabs;
	FindFiles( strFind.c_str(), [this, &strPath, &mapLevelPrefabs] ( const char* szFileName )
	{
		string strFullPath = strPath;
		strFullPath += szFileName;
		CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( strFullPath.c_str() );
		auto pLevelData = pPrefab->GetRoot()->GetStaticDataSafe<CMyLevel>();
		if( pLevelData )
			mapLevelPrefabs[strFullPath] = pPrefab;
		return true;
	}, true, false );

	{
		int32 i1 = 0;
		for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
		{
			auto itr = mapLevelPrefabs.find( m_pData->arrLevelData[i].pLevel.c_str() );
			if( itr != mapLevelPrefabs.end() )
			{
				if( i1 != i )
					m_pData->arrLevelData[i1] = m_pData->arrLevelData[i];
				m_pData->arrLevelData[i1].pLevel = itr->second;
				itr->second = NULL;
				i1++;
			}
		}
		m_pData->arrLevelData.Resize( i1 );
	}
	InitLevels( mapLevelPrefabs );

	map<CString, int32 > mapLevelIndex;
	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		mapLevelIndex[m_pData->arrLevelData[i].pLevel] = i;
	}
	for( int i = 0; i < m_vecLevelData.size(); i++ )
	{
		RefreshLevelData( i );
	}
}

void CWorldCfgEditor::InitLevels( map<string, CReference<CPrefab> >& mapLevelPrefabs )
{
	auto& arrLevelData = m_pData->arrLevelData;
	int32 i1 = arrLevelData.Size();
	for( auto& item : mapLevelPrefabs )
	{
		if( !item.second )
			continue;
		int32 n = arrLevelData.Size();
		arrLevelData.Resize( n + 1 );
		auto& data = arrLevelData[n];
		data.pLevel = item.first.c_str();
		data.pLevel = item.second;
	}

	float yMax = 0;
	float xMin = 0;
	CVector2 p0( yMax, xMin );
	for( int i = 0; i < i1; i++ )
	{
		auto& data = arrLevelData[i];
		auto pLevelData = data.pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
		yMax = Max( yMax, data.displayOfs.y + pLevelData->GetSize().GetBottom() );
		xMin = Min( xMin, data.displayOfs.x + pLevelData->GetSize().x );
	}
	for( int i = i1; i < arrLevelData.Size(); i++ )
	{
		auto& data = arrLevelData[i];
		auto pLevelData = data.pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
		bool b = false;
		CVector2 p = p0;
		p0.x += 512;
		data.displayOfs = p;
	}

	m_vecLevelData.resize( arrLevelData.Size() );
	for( int i = 0; i < arrLevelData.Size(); i++ )
		m_mapLevelDataIndex[arrLevelData[i].pLevel.c_str()] = i;
	for( int i = 0; i < arrLevelData.Size(); i++ )
	{
		auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( arrLevelData[i].pLevel.c_str() );
		InitLevel( i, pPrefab );
	}
}

void CWorldCfgEditor::InitLevel( int32 nLevel, CPrefab* pPrefab )
{
	auto& levelData = m_vecLevelData[nLevel];
	auto& lv = m_pData->arrLevelData[nLevel];
	lv.pLevel = pPrefab;
	lv.displayOfs.x = floor( lv.displayOfs.x / 64 + 0.5f ) * 64;
	lv.displayOfs.y = floor( lv.displayOfs.y / 64 + 0.5f ) * 64;
	for( int i = 0; i < lv.arrOverlapLevel.Size(); i++ )
	{
		auto itr = m_mapLevelDataIndex.find( lv.arrOverlapLevel[i].c_str() );
		if( itr == m_mapLevelDataIndex.end() )
		{
			for( int j = i; j < lv.arrOverlapLevel.Size() - 1; j++ )
				lv.arrOverlapLevel[j] = lv.arrOverlapLevel[j + 1];
			lv.arrOverlapLevel.Resize( lv.arrOverlapLevel.Size() - 1 );
			i--;
			continue;
		}
	}

	auto pPrefabNode = pPrefab->GetRoot()->Clone( pPrefab );
	levelData.pClonedLevelData = pPrefabNode;
	CLevelToolsView::FixLevelData( pPrefabNode, pPrefab );
	pPrefabNode->SetPosition( lv.displayOfs );

	function<void( CPrefabNode* )> Func;
	Func = [&Func] ( CPrefabNode* pNode ) {
		auto pPatchedNode = pNode->GetPatchedNode();
		for( CRenderObject2D* pChild = pNode->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
		{
			if( pChild == pNode->GetRenderObject() )
			{
				if( !pPatchedNode )
					continue;
				pPatchedNode->OnEditorActive( false );
				Func( pPatchedNode );
			}
			else
			{
				CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
				if( pNode )
				{
					pNode->OnEditorActive( false );
					Func( pNode );
				}
			}
		}
	};
	Func( pPrefabNode );

	auto pPreview = CLevelToolsView::CreateLevelSimplePreview( pPrefabNode );
	pPreview->SetPosition( lv.displayOfs );
	levelData.pLevelPreview = pPreview;
	m_pViewport->GetRoot()->AddChild( pPreview );
}

void CWorldCfgEditor::Select( int32 n )
{
	if( m_nCurSelected == n )
		return;
	if( m_nCurSelected >= 0 && m_nCurSelected < m_vecLevelData.size() )
	{
		auto& item = m_vecLevelData[m_nCurSelected];
		item.pLevelPreview = CLevelToolsView::CreateLevelSimplePreview( item.pClonedLevelData );
		item.pLevelPreview->SetPosition( item.pClonedLevelData->GetPosition() );
		m_pViewport->GetRoot()->AddChildBefore( item.pLevelPreview, item.pClonedLevelData );
		item.pClonedLevelData->RemoveThis();
	}
	m_nCurSelected = n;
	if( n >= 0 && n < m_vecLevelData.size() )
	{
		auto& item = m_vecLevelData[m_nCurSelected];
		m_pViewport->GetRoot()->AddChild( item.pClonedLevelData );
		item.pLevelPreview->RemoveThis();
		item.pLevelPreview = NULL;

		m_pNewTemplate->SetText( m_pData->arrLevelData[m_nCurSelected].pLevel );
	}
}

void CWorldCfgEditor::RefreshLevelData( int32 nLevel )
{

}

void CWorldCfgEditor::OnLevelDataEdit( int32 nLevel )
{
	m_vecLevelData[nLevel].bDirty = true;
	m_vecLevelData[nLevel].pClonedLevelData->OnEdit();
}

void CWorldCfgEditor::ShowLevelTool()
{
	if( m_nCurSelected < 0 )
		return;
	m_pViewport->bVisible = false;
	auto& curLevelData = m_vecLevelData[m_nCurSelected];
	auto& cur = m_pData->arrLevelData[m_nCurSelected];
	CLevelToolsView::Inst()->Set( curLevelData.pClonedLevelData, [this, &curLevelData, &cur] () {
		m_pViewport->bVisible = true;
		curLevelData.pClonedLevelData->SetPosition( cur.displayOfs );
		OnLevelDataEdit( m_nCurSelected );
	}, &cur, NULL );

	for( int i = 0; i < cur.arrOverlapLevel.Size(); i++ )
	{
		auto& link = cur.arrOverlapLevel[i];
		int32 n = -1;
		for( int j = 0; j < m_pData->arrLevelData.Size(); j++ )
		{
			if( link == m_pData->arrLevelData[j].pLevel )
			{
				n = j;
				break;
			}
		}
		auto& nxt = m_pData->arrLevelData[n];
		CVector2 ofs = nxt.displayOfs - cur.displayOfs;

		CLevelToolsView::Inst()->AddNeighbor( m_vecLevelData[n].pClonedLevelData, ofs );
	}
	CLevelToolsView::Inst()->RefreshMask();
}

void CWorldCfgEditor::OpenLevelFile()
{
	if( m_nCurSelected < 0 )
		return;
	Save();
	string str = "world=";
	str += GetFileName();
	auto& cur = m_pData->arrLevelData[m_nCurSelected];
	CEditor::Inst().OpenFile( cur.pLevel.c_str(), str.c_str() );
}

int32 CWorldCfgEditor::PickLevel( const CVector2 & p )
{
	for( int32 i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		if( i != m_nCurSelected && m_vecLevelData[i].pLevelPreview && !m_vecLevelData[i].pLevelPreview->bVisible )
			continue;
		auto pLevelData = m_vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		if( pLevelData->GetSize().Offset( m_pData->arrLevelData[i].displayOfs ).Contains( p ) )
			return i;
	}
	return -1;
}

void CWorldCfgEditor::AddOverlap( int32 a, int32 b )
{
	auto& arrOverlap = m_pData->arrLevelData[a].arrOverlapLevel;
	for( int i = 0; i < arrOverlap.Size(); i++ )
	{
		if( arrOverlap[i] == m_pData->arrLevelData[b].pLevel )
			return;
	}
	arrOverlap.Resize( arrOverlap.Size() + 1 );
	arrOverlap[arrOverlap.Size() - 1] = m_pData->arrLevelData[b].pLevel;
}

void CWorldCfgEditor::RemoveOverlap( int32 a, int32 b )
{
	auto& arrOverlap = m_pData->arrLevelData[a].arrOverlapLevel;
	for( int i = 0; i < arrOverlap.Size(); i++ )
	{
		if( arrOverlap[i] == m_pData->arrLevelData[b].pLevel )
		{
			for( int j = i; j < arrOverlap.Size() - 1; j++ )
				arrOverlap[j] = arrOverlap[j + 1];
			arrOverlap.Resize( arrOverlap.Size() - 1 );
			return;
		}
	}
}

void CWorldCfgEditor::MoveZ( bool bDown )
{
	int32 newZ = bDown ? 1000000 : -1000000;
	bool b = false;
	for( int32 i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		auto pLevelData = m_vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		if( bDown ? pLevelData->GetLevelZ() <= m_z : pLevelData->GetLevelZ() >= m_z )
			continue;
		if( bDown ? pLevelData->GetLevelZ() < newZ : pLevelData->GetLevelZ() > newZ )
		{
			newZ = pLevelData->GetLevelZ();
			b = true;
		}
	}
	if( b )
	{
		m_z = newZ;
		for( int32 i = 0; i < m_pData->arrLevelData.Size(); i++ )
		{
			auto pLevelData = m_vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
			if( pLevelData->GetLevelZ() < m_z )
			{
				if( m_nCurSelected == i )
					Select( -1 );
				m_vecLevelData[i].pLevelPreview->bVisible = false;
			}
			else if( m_nCurSelected != i )
				m_vecLevelData[i].pLevelPreview->bVisible = true;
		}
	}
}

void CWorldCfgEditor::OnDebugDraw( IRenderSystem* pRenderSystem )
{
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

	if( m_nState >= 1 )
	{
		CVector2 p = m_pViewport->GetScenePos( m_pViewport->GetMgr()->GetMousePos() );
		auto newMapSize = m_newMapSize;
		if( m_nDragType )
		{
			TRectangle<int32> r( floor( m_dragBegin.x / 64 ), floor( m_dragBegin.y / 64 ), 1, 1 );
			auto d = p - m_dragBegin;
			int32 dx = floor( d.x / 64 + 0.5f );
			int32 dy = floor( d.y / 64 + 0.5f );
			if( dx < 0 )
				r.SetLeft( r.GetLeft() + dx );
			else
				r.width += dx;
			if( dy < 0 )
				r.SetTop( r.GetTop() + dy );
			else
				r.height += dy;
			newMapSize = CRectangle( r.x, r.y, r.width, r.height ) * 64;
		}
		CVector2 p0( newMapSize.x, newMapSize.y );
		CVector2 b( newMapSize.width, newMapSize.height );
		CVector4 color( 1, 1, 1, 1 );
		if( m_nState == 2 )
			color = CVector4( 0.5, 1, 1, 1 );
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = p0 + b * ofs[j];
			auto pt2 = p0 + b * ofs[( j + 1 ) % 4];
			m_pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, color );
		}
		m_pViewport->DebugDrawLine( pRenderSystem, m_newMapOfs + CVector2( -16, 0 ), m_newMapOfs + CVector2( 16, 0 ), color );
		m_pViewport->DebugDrawLine( pRenderSystem, m_newMapOfs + CVector2( 0, -16 ), m_newMapOfs + CVector2( 0, 16 ), color );
	}

	auto& arrLevelData = m_pData->arrLevelData;
	for( int i = 0; i < arrLevelData.Size(); i++ )
	{
		auto& data = arrLevelData[i];
		auto pLevel = m_vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto size = pLevel->GetSize();

		CVector2 p0 = data.displayOfs + CVector2( size.x, size.y );
		auto b = CVector2( size.width, size.height );
		CVector4 color( 0.1, 0.4, 0.45, 0.5 );
		if( i == m_nCurSelected )
			color = CVector4( 0.8, 0.8, 0.8, 0.5 );
		else if( m_nCurSelected >= 0 )
		{
			auto& curSelectedData = arrLevelData[m_nCurSelected];
			bool b = false;
			for( int j = 0; j < curSelectedData.arrOverlapLevel.Size(); j++ )
			{
				if( data.pLevel == curSelectedData.arrOverlapLevel[j] )
				{
					b = true;
					break;
				}
			}
			if( b )
				color = CVector4( 0.8, 0.8, 0.2, 0.5 );
		}
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = p0 + b * ofs[j];
			auto pt2 = p0 + b * ofs[( j + 1 ) % 4];
			m_pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, color );
		}
	}
}

void CWorldCfgEditor::OnViewportStartDrag( SUIMouseEvent* pEvent )
{
	m_nDragType = 0;
	CVector2 p = m_pViewport->GetScenePos( pEvent->mousePos );

	if( m_nState >= 1 )
	{
		if( GetMgr()->IsKey( 'Z' ) )
		{
			if( m_nState == 1 && m_newMapSize.Contains( p ) );
			{
				m_newMapOfs = CVector2( floor( p.x / 64 + 0.5f ) * 64, floor( p.y / 64 + 0.5f ) * 64 );
				m_nDragType = -1;
				return;
			}

			Super::OnViewportStartDrag( pEvent );
			return;
		}

		if( m_nState == 2 )
		{
			m_newMapOfs = CVector2( floor( p.x / 64 + 0.5f ) * 64, floor( p.y / 64 + 0.5f ) * 64 );
			m_newMapSize = m_pNewMapTemplate->GetRoot()->GetStaticDataSafe<CMyLevel>()->GetSize().Offset( m_newMapOfs );
			m_nDragType = -1;
			return;
		}
		m_dragBegin = p;
		m_nDragType = 1;
		return;
	}

	auto& arrLevelData = m_pData->arrLevelData;
	if( m_nCurSelected >= 0 )
	{
		if( GetMgr()->IsKey( 'Z' ) )
		{
			auto n = PickLevel( p );
			if( n >= 0 && n != m_nCurSelected )
			{
				auto& arrOverlap = arrLevelData[m_nCurSelected].arrOverlapLevel;
				bool b = false;
				for( int i = 0; i < arrOverlap.Size(); i++ )
				{
					if( arrOverlap[i] == arrLevelData[n].pLevel )
					{
						RemoveOverlap( m_nCurSelected, n );
						RemoveOverlap( n, m_nCurSelected );
						b = true;
						break;
					}
				}
				if( !b )
				{
					AddOverlap( m_nCurSelected, n );
					AddOverlap( n, m_nCurSelected );
				}
				m_nDragType = 1;
				return;
			}
		}
	}

	for( int i = 0; i < arrLevelData.Size(); i++ )
	{
		if( i != m_nCurSelected && m_vecLevelData[i].pLevelPreview && !m_vecLevelData[i].pLevelPreview->bVisible )
			continue;
		auto pLevelData = m_vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto ofs = arrLevelData[i].displayOfs;
		auto r = pLevelData->GetSize().Offset( ofs );
		if( r.Contains( p ) )
		{
			m_nDragType = 1;
			m_worldDragBeginPos = p;
			m_curDisplayOfs0 = ofs;
			Select( i );
			return;
		}
	}

	Super::OnViewportStartDrag( pEvent );
}

void CWorldCfgEditor::OnViewportDragged( SUIMouseEvent* pEvent )
{
	CVector2 p = m_pViewport->GetScenePos( pEvent->mousePos );
	if( !m_nDragType )
	{
		Super::OnViewportDragged( pEvent );
		return;
	}
	if( !m_nState )
	{
		auto& arrLevelData = m_pData->arrLevelData;
		auto& vecLevelData = m_vecLevelData;
		auto d = p - m_worldDragBeginPos;
		TVector2<int32> ofs( floor( d.x / 64 + 0.5f ), floor( d.y / 64 + 0.5f ) );
		auto& data = arrLevelData[m_nCurSelected];
		data.displayOfs = m_curDisplayOfs0 + CVector2( ofs.x, ofs.y ) * 64;
		vecLevelData[m_nCurSelected].pClonedLevelData->SetPosition( data.displayOfs );
	}
}

void CWorldCfgEditor::OnViewportStopDrag( SUIMouseEvent* pEvent )
{
	CVector2 p = m_pViewport->GetScenePos( pEvent->mousePos );
	if( !m_nDragType )
	{
		Super::OnViewportStopDrag( pEvent );
		return;
	}
	if( m_nState >= 1&& m_nDragType > 0 )
	{
		TRectangle<int32> r( floor( m_dragBegin.x / 64 ), floor( m_dragBegin.y / 64 ), 1, 1 );
		auto d = p - m_dragBegin;
		int32 dx = floor( d.x / 64 + 0.5f );
		int32 dy = floor( d.y / 64 + 0.5f );
		if( dx < 0 )
			r.SetLeft( r.GetLeft() + dx );
		else
			r.width += dx;
		if( dy < 0 )
			r.SetTop( r.GetTop() + dy );
		else
			r.height += dy;
		m_newMapSize = CRectangle( r.x, r.y, r.width, r.height ) * 64;
		m_newMapOfs = CVector2( r.x + r.width / 2, r.y + r.height / 2 ) * 64;
	}

	m_nDragType = 0;
}

void CWorldCfgEditor::OnViewportMouseWheel( SUIMouseEvent* pEvent )
{
	m_fScale -= pEvent->nParam * 1.0f / 120;
	static float fScales[] = { 1, 1.5, 2, 3, 4, 6, 8, 12, 16 };
	m_fScale = Max( 0.0f, Min<float>( m_fScale, ELEM_COUNT( fScales ) - 1 ) );
	float fScale = fScales[(int32)floor( m_fScale )];
	auto viewSize = m_pViewport->GetCamera().GetViewport().GetSize();
	m_pViewport->GetCamera().SetSize( viewSize.x * fScale, viewSize.y * fScale );
}

void CWorldCfgEditor::OnViewportKey( SUIKeyEvent* pEvent )
{
	auto nChar = pEvent->nChar;
	if( pEvent->bKeyDown )
	{
		if( nChar == VK_F1 )
			ShowLevelTool();
		if( nChar == VK_F2 )
			OpenLevelFile();
		if( !m_nDragType )
		{
			if( m_nState )
			{
				if( nChar == VK_TAB )
				{
					m_nState = 3 - m_nState;
					if( m_nState == 2 )
						m_newMapSize = m_pNewMapTemplate->GetRoot()->GetStaticDataSafe<CMyLevel>()->GetSize().Offset( m_newMapOfs );
				}
			}
			else
			{
				if( nChar == 'W' )
					MoveZ( true );
				if( nChar == 'S' )
					MoveZ( false );
			}
		}
	}
}

void CWorldCfgEditor::BeginNewLevel()
{
	if( m_nState != 0 )
		return;

	auto strName = UnicodeToUtf8( m_pNewTemplate->GetText() );
	m_pNewMapTemplate = CResourceManager::Inst()->CreateResource<CPrefab>( strName.c_str() );
	if( !m_pNewMapTemplate )
		return;
	if( !m_pNewMapTemplate->GetRoot()->GetStaticDataSafe<CMyLevel>() )
	{
		m_pNewMapTemplate = NULL;
		return;
	}

	Select( -1 );
	m_nState = 1;
	m_pPanel[0]->SetVisible( false );
	m_pPanel[1]->SetVisible( true );

	m_newMapOfs = m_pViewport->GetCamera().GetViewArea().GetCenter();
	m_newMapOfs = CVector2( floor( m_newMapOfs.x / 64 + 0.5f ) * 64, floor( m_newMapOfs.y / 64 + 0.5f ) * 64 );
	m_newMapSize = CRectangle( m_newMapOfs.x - 64, m_newMapOfs.y - 64, 128, 128 );
}

void CWorldCfgEditor::EndNewLevel( bool bOK )
{
	if( m_nState == 0 )
		return;
	if( bOK )
	{
		auto& arrLevelData = m_pData->arrLevelData;
		auto& vecLevelData = m_vecLevelData;
		vector<int32> vecLinkLevels;
		for( int i = 0; i < vecLevelData.size(); i++ )
		{
			auto pLevel1 = SafeCast<CMyLevel>( vecLevelData[i].pClonedLevelData->GetFinalObjData() );
			auto ofs = pLevel1->GetSize().Offset( arrLevelData[i].displayOfs );
			auto boundX = ofs * m_newMapSize;
			if( boundX.width && boundX.height )
				vecLinkLevels.push_back( i );
		}

		string strName = m_pRes->GetName();
		int32 n = strName.find_last_of( '/' );
		strName = strName.substr( 0, n + 1 );
		strName = strName + UnicodeToUtf8( m_pNewLevelName->GetText() ) + ".pf";
		auto pPrefab = CLevelToolsView::NewLevelFromTemplate( m_pNewMapTemplate, strName.c_str(), m_newMapSize.Offset( m_newMapOfs * -1 ), m_z, m_nState == 2 );
		if( !pPrefab )
			goto fail;
		auto pLevelData = SafeCast<CMyLevel>( pPrefab->GetRoot()->GetFinalObjData() );
		auto nNewLevel = arrLevelData.Size();
		arrLevelData.Resize( nNewLevel + 1 );
		vecLevelData.resize( nNewLevel + 1 );
		auto& levelData = arrLevelData[nNewLevel];
		levelData.pLevel = strName.c_str();
		for( auto i : vecLinkLevels )
		{
			AddOverlap( i, nNewLevel );
			AddOverlap( nNewLevel, i );
		}

		levelData.displayOfs = m_newMapOfs;
		CBufFile buf;
		pPrefab->Save( buf );
		SaveFile( pPrefab->GetName(), buf.GetBuffer(), buf.GetBufLen() );
		InitLevel( nNewLevel, pPrefab );
		for( auto i : vecLinkLevels )
			OnLevelDataEdit( i );
	}
fail:
	m_pNewMapTemplate = NULL;
	m_nState = 0;
	m_pPanel[0]->SetVisible( true );
	m_pPanel[1]->SetVisible( false );
}