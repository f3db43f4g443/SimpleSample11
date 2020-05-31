#include "stdafx.h"
#include "WorldCfgEditor.h"
#include "UICommon/UIFactory.h"
#include "Common/Utf8Util.h"
#include "Render/CommonShader.h"
#include "Render/Scene2DManager.h"
#include "Render/Renderer.h"
#include "Game/MyLevel.h"

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

void CWorldCfgEditor::Refresh()
{
	for( auto& item : m_vecLevelData )
	{
		if( item.pLevelPreview )
			item.pLevelPreview->RemoveThis();
		if( item.pClonedLevelData )
			item.pClonedLevelData->RemoveThis();
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
		m_pCurDragEdit = NULL;
		if( m_pCurLevelEdit )
		{
			m_onLvDataEdit.Unregister();
			m_pCurLevelEdit = NULL;
		}
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
	m_pTreeView = GetChildByName<CUITreeView>( "node_view" );
	m_onSave.Set( this, &CWorldCfgEditor::Save );
	m_pTreeView->GetChildByName<CUIButton>( "save" )->Register( eEvent_Action, &m_onSave );
	m_onAutoLayout.Set( this, &CWorldCfgEditor::AutoLayout );
	m_pTreeView->GetChildByName<CUIButton>( "auto_layout" )->Register( eEvent_Action, &m_onAutoLayout );
	m_onLvDataEdit.Set( this, &CWorldCfgEditor::OnLevelDateEdit );
}

void CWorldCfgEditor::Save()
{
	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		CPrefab* pPrefab = m_pData->arrLevelData[i].pLevel;
		CBufFile buf;
		pPrefab->Save( buf );
		SaveFile( pPrefab->GetName(), buf.GetBuffer(), buf.GetBufLen() );
	}
	Super::Save();
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
	CBufFile buf;
	auto pClassData = CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>();
	pClassData->PackData( (uint8*)m_pData, buf, true );
	m_pRes->m_pWorldCfg = (SWorldCfg*)pClassData->NewObjFromData( buf, true, NULL );
	m_pRes->RefreshEnd();

	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		CPrefab* pPrefab = m_pData->arrLevelData[i].pLevel;
		pPrefab->RefreshBegin();
		auto pNode1 = m_vecLevelData[i].pClonedLevelData->Clone( pPrefab );
		pNode1->SetPosition( CVector2( 0, 0 ) );
		pPrefab->SetNode( pNode1 );
		pPrefab->RefreshEnd();
	}
}

void CWorldCfgEditor::OnDebugDraw( IRenderSystem* pRenderSystem )
{
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		auto& data = m_pData->arrLevelData[i];
		auto pLevel = m_vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto size = pLevel->GetSize();

		CVector2 p0 = data.displayOfs;
		auto b = CVector2( size.x, size.y ) * LEVEL_GRID_SIZE;
		CVector4 color( 0.2, 0.8, 0.9, 1 );
		if( i == m_nCurSelected )
		{
			color = CVector4( 0.5, 1, 1, 1 );
			p0 = p0 - CVector2( 32, 32 );
			b = b + CVector2( 64, 64 );
		}
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = p0 + b * ofs[j];
			auto pt2 = p0 + b * ofs[( j + 1 ) % 4];
			m_pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, color );
		}
	}

	if( m_nCurSelected >= 0 && m_nCurSelected < m_pData->arrLevelData.Size() )
	{
		auto& curSelected = m_pData->arrLevelData[m_nCurSelected];
		CMatrix2D mat;
		mat.Translate( curSelected.displayOfs.x, curSelected.displayOfs.y );
		m_pCurLevelEdit->OnDebugDraw( m_pViewport, pRenderSystem, mat );
	}

	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		auto& data = m_pData->arrLevelData[i];
		auto& levelData = m_vecLevelData[i];
		auto pLevel = levelData.pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto size = pLevel->GetSize();
		for( int j = 0; j < levelData.vecLinks.size(); j++ )
		{
			auto& link = levelData.vecLinks[j];
			if( link.nTarget < 0 )
				continue;
			auto& nextLevel = pLevel->m_arrNextStage[j];
			auto pLevel1 = m_vecLevelData[link.nTarget].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
			auto beginBase = data.displayOfs;
			auto endBase = CVector2( -nextLevel.nOfsX, -nextLevel.nOfsY ) * LEVEL_GRID_SIZE + m_pData->arrLevelData[link.nTarget].displayOfs;
			CVector4 colors[4] = { { 0.4, 0.4, 0.8, 0.5 }, { 0.8, 0.8, 0.3, 1 }, { 0.5, 0.9, 0.3, 1 }, { 0.5, 0.5, 0.5, 0.5 } };
			for( auto& p : link.vecBegin )
			{
				int8 n = 0;
				if( i == m_nCurSelected )
					n = 1;
				else if( link.nTarget == m_nCurSelected )
					n = 2;
				auto p1 = p + TVector2<int32>( -nextLevel.nOfsX, -nextLevel.nOfsY );
				auto pGrid1 = pLevel1->GetGridData( p1 );
				if( pGrid1 == NULL || pGrid1->bBlocked )
				{
					if( n == 0 )
						continue;
					n = 3;
				}

				auto pt = CVector2( p.x + 0.5f, p.y + 0.5f ) * LEVEL_GRID_SIZE;
				m_pViewport->DebugDrawLine( pRenderSystem, pt + beginBase, pt + endBase, colors[n] );
				m_pViewport->DebugDrawLine( pRenderSystem, pt + endBase - CVector2( 4, 4 ), pt + endBase + CVector2( 4, 4 ), colors[n] );
				m_pViewport->DebugDrawLine( pRenderSystem, pt + endBase - CVector2( -4, 4 ), pt + endBase + CVector2( -4, 4 ), colors[n] );

				if( n == 0 )
					break;
			}
		}
	}
}

void CWorldCfgEditor::AutoLayout()
{
	if( m_nCurSelected < 0 )
		return;
	vector<int32> q;
	q.push_back( m_nCurSelected );
	m_vecLevelData[m_nCurSelected].nFlag = 1;
	for( int i = 0; i < q.size(); i++ )
	{
		auto n = q[i];
		auto& data = m_pData->arrLevelData[n];
		auto& levelData = m_vecLevelData[n];
		auto pLevel = levelData.pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		for( int j = 0; j < levelData.vecLinks.size(); j++ )
		{
			auto& link = levelData.vecLinks[j];
			auto& levelData1 = m_vecLevelData[link.nTarget];
			if( levelData1.nFlag )
				continue;
			auto& nextLevel = pLevel->m_arrNextStage[j];
			auto ofs = data.displayOfs + CVector2( nextLevel.nOfsX, nextLevel.nOfsY ) * LEVEL_GRID_SIZE;
			m_pData->arrLevelData[link.nTarget].displayOfs = ofs;
			m_vecLevelData[link.nTarget].pClonedLevelData->SetPosition( ofs );
			m_vecLevelData[link.nTarget].pLevelPreview->SetPosition( ofs );
			levelData1.nFlag = 1;
			q.push_back( link.nTarget );
		}
	}
	for( auto& item : m_vecLevelData )
		item.nFlag = 0;
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
		if( pPrefab->GetRoot()->GetStaticDataSafe<CMyLevel>() )
			mapLevelPrefabs[strFullPath] = pPrefab;
		return true;
	}, true, false );

	int i1 = 0;
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
	for( auto& item : mapLevelPrefabs )
	{
		if( !item.second )
			continue;
		int32 n = m_pData->arrLevelData.Size();
		m_pData->arrLevelData.Resize( n + 1 );
		auto& data = m_pData->arrLevelData[n];
		data.pLevel = item.first.c_str();
		data.pLevel = item.second;
	}

	float yMax = 0;
	float xMin = 0;
	CVector2 p0( yMax, xMin );
	for( int i = 0; i < i1; i++ )
	{
		auto& data = m_pData->arrLevelData[i];
		auto pLevelData = data.pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
		yMax = Max( yMax, pLevelData->m_nHeight * LEVEL_GRID_SIZE_Y + data.displayOfs.y );
		xMin = Min( xMin, data.displayOfs.x );
	}
	for( int i = i1; i < m_pData->arrLevelData.Size(); i++ )
	{
		auto& data = m_pData->arrLevelData[i];
		auto pLevelData = data.pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
		auto& nxtStage = pLevelData->m_arrNextStage;
		bool b = false;
		CVector2 p;
		for( int j = 0; j < i; j++ )
		{
			TVector2<int32> ofs;
			auto& data1 = m_pData->arrLevelData[j];
			auto pLevelData1 = data1.pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
			auto& nxtStage1 = pLevelData1->m_arrNextStage;
			for( int k = 0; k < nxtStage1.Size(); k++ )
			{
				if( nxtStage1[k].pNxtStage.c_str() == data.pLevel.c_str() )
				{
					b = true;
					ofs = TVector2<int32>( nxtStage1[k].nOfsX, nxtStage1[k].nOfsY );
					break;
				}
			}
			if( !b )
			{
				for( int k = 0; k < nxtStage.Size(); k++ )
				{
					if( nxtStage[k].pNxtStage.c_str() == data1.pLevel.c_str() )
					{
						b = true;
						ofs = TVector2<int32>( -nxtStage[k].nOfsX, -nxtStage[k].nOfsY );
						break;
					}
				}
			}
			if( b )
			{
				p = CVector2( ofs.x, ofs.y ) * LEVEL_GRID_SIZE + data1.displayOfs;
				break;
			}
		}

		if( !b )
		{
			p = p0;
			p0.x += pLevelData->m_nWidth * LEVEL_GRID_SIZE_X;
		}
		yMax = Max( yMax, pLevelData->m_nHeight * LEVEL_GRID_SIZE_Y + p.y );
		if( b )
			p0 = CVector2( yMax, xMin );

		data.displayOfs = p;
		data.nDisplayLevel = 0;
	}

	m_vecLevelData.resize( m_pData->arrLevelData.Size() );
	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		auto& levelData = m_vecLevelData[i];
		auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_pData->arrLevelData[i].pLevel.c_str() );
		m_pData->arrLevelData[i].pLevel = pPrefab;
		auto pPrefabNode = pPrefab->GetRoot()->Clone( pPrefab );
		pPrefabNode->SetPosition( m_pData->arrLevelData[i].displayOfs );
		levelData.pClonedLevelData = pPrefabNode;

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

		auto pPreview = SafeCast<CMyLevel>( pPrefabNode->CreateInstance( false ) );
		pPreview->SetPosition( m_pData->arrLevelData[i].displayOfs );
		levelData.pLevelPreview = pPreview;
		m_pViewport->GetRoot()->AddChild( pPreview );
		pPreview->OnPreview();
	}
	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
		RefreshLevelDataLink( i );
}

void CWorldCfgEditor::Select( int32 n )
{
	if( m_nCurSelected == n )
		return;
	if( m_nCurSelected >= 0 && m_nCurSelected < m_vecLevelData.size() )
	{
		auto& item = m_vecLevelData[m_nCurSelected];
		item.pLevelPreview = SafeCast<CMyLevel>( item.pClonedLevelData->CreateInstance( false ) );
		m_pViewport->GetRoot()->AddChildBefore( item.pLevelPreview, item.pClonedLevelData );
		item.pClonedLevelData->RemoveThis();
		item.pLevelPreview->OnPreview();
		m_onLvDataEdit.Unregister();
		m_pCurLevelEdit = NULL;
	}
	m_nCurSelected = n;
	if( n >= 0 && n < m_vecLevelData.size() )
	{
		auto& item = m_vecLevelData[m_nCurSelected];
		m_pCurLevelEdit = new CLevelEdit( m_pTreeView, NULL, item.pClonedLevelData->GetObjData(), item.pClonedLevelData->GetClassData(), "Level Data" );
		m_pViewport->GetRoot()->AddChild( item.pClonedLevelData );
		item.pLevelPreview->RemoveThis();
		item.pLevelPreview = NULL;
		m_pCurLevelEdit->Register( &m_onLvDataEdit );
	}
}

void CWorldCfgEditor::OnLevelDateEdit()
{
	RefreshLevelDataLink( m_nCurSelected );
}

void CWorldCfgEditor::RefreshLevelDataLink( int32 n )
{
	auto pLevelData = m_vecLevelData[n].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
	auto& links = m_vecLevelData[n].vecLinks;
	links.resize( 0 );
	links.resize( pLevelData->m_arrNextStage.Size() );
	for( int i = 0; i < pLevelData->m_arrNextStage.Size(); i++ )
	{
		auto& link = links[i];
		link.nTarget = -1;
		auto& nextStage = pLevelData->m_arrNextStage[i];
		for( int j = 0; j < m_pData->arrLevelData.Size(); j++ )
		{
			CString str = m_pData->arrLevelData[j].pLevel;
			if( str == nextStage.pNxtStage.c_str() )
			{
				link.nTarget = j;
				break;
			}
		}

		link.vecBegin.resize( 0 );
		for( int x = 0; x < pLevelData->m_nWidth; x++ )
		{
			for( int y = 0; y < pLevelData->m_nHeight; y++ )
			{
				auto pGridData = pLevelData->GetGridData( TVector2<int32>( x, y ) );
				if( pGridData->nNextStage == i + 1 )
					link.vecBegin.push_back( TVector2<int32>( x, y ) );
			}
		}
	}
}

void CWorldCfgEditor::OnViewportStartDrag( SUIMouseEvent* pEvent )
{
	m_nDragType = 0;
	CVector2 p = m_pViewport->GetScenePos( pEvent->mousePos );
	m_pCurDragEdit = NULL;
	if( m_nCurSelected >= 0 )
	{
		m_pCurDragEdit = m_pCurLevelEdit->OnViewportStartDrag( m_pViewport, p, m_vecLevelData[m_nCurSelected].pClonedLevelData->globalTransform );
		if( m_pCurDragEdit )
			return;
		auto pLevelData = m_vecLevelData[m_nCurSelected].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto ofs = m_pData->arrLevelData[m_nCurSelected].displayOfs;
		CRectangle r( ofs.x - 32, ofs.y - 32, pLevelData->m_nWidth * LEVEL_GRID_SIZE_X + 64, pLevelData->m_nHeight * LEVEL_GRID_SIZE_Y + 64 );
		if( r.Contains( p ) )
		{
			m_nDragType = 1;
			m_worldDragBeginPos = p;
			m_curDisplayOfs0 = ofs;
			return;
		}
	}
	for( int i = 0; i < m_pData->arrLevelData.Size(); i++ )
	{
		auto pLevelData = m_vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto ofs = m_pData->arrLevelData[i].displayOfs;
		CRectangle r( ofs.x, ofs.y, pLevelData->m_nWidth * LEVEL_GRID_SIZE_X, pLevelData->m_nHeight * LEVEL_GRID_SIZE_Y );
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
	if( m_pCurDragEdit )
	{
		m_pCurDragEdit->OnViewportDragged( m_pViewport, p, m_vecLevelData[m_nCurSelected].pClonedLevelData->globalTransform );
		return;
	}
	if( !m_nDragType )
	{
		Super::OnViewportDragged( pEvent );
		return;
	}

	auto d = p - m_worldDragBeginPos;
	TVector2<int32> ofs( floor( d.x / LEVEL_GRID_SIZE_X + 0.5f ), floor( d.y / LEVEL_GRID_SIZE_Y + 0.5f ) );
	auto& data = m_pData->arrLevelData[m_nCurSelected];
	data.displayOfs = m_curDisplayOfs0 + CVector2( ofs.x, ofs.y ) * LEVEL_GRID_SIZE;
	m_vecLevelData[m_nCurSelected].pClonedLevelData->SetPosition( data.displayOfs );
	m_pCurLevelEdit->RefreshData();
}

void CWorldCfgEditor::OnViewportStopDrag( SUIMouseEvent* pEvent )
{
	CVector2 p = m_pViewport->GetScenePos( pEvent->mousePos );
	if( m_pCurDragEdit )
	{
		m_pCurDragEdit->OnViewportStopDrag( m_pViewport, p, m_vecLevelData[m_nCurSelected].pClonedLevelData->globalTransform );
		m_pCurDragEdit = NULL;
		return;
	}
	if( !m_nDragType )
	{
		Super::OnViewportStopDrag( pEvent );
		return;
	}

	m_nDragType = 0;
}

void CWorldCfgEditor::OnViewportChar( uint32 nChar )
{
}

//void CWorldCfgEditor::OnCreate()
//{
//	if( m_nCurSelected < 0 )
//		return;
//	TRectangle<int32> rect;
//	if( !FindSpace( m_pCreateWidth->GetValue<uint32>(), m_pCreateHeight->GetValue<uint32>(), rect ) )
//		return;
//	string strFileName = m_data.vecContext[m_nCurSelected].strSceneResName;
//	CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( strFileName.c_str() );
//	if( !pPrefab )
//		return;
//	int32 nLast = strFileName.find( "__", strFileName.find_last_of( '/' ) );
//	if( nLast == string::npos )
//		nLast = strFileName.find_last_of( '.' );
//	strFileName = strFileName.substr( 0, nLast ) + "__";
//	do
//	{
//		char buf[32];
//		int32 i = m_data.vecContext.size();
//		for( ;; i++ )
//		{
//			itoa( i, buf, 10 );
//			auto str1 = strFileName + buf + ".pf";
//			if( !IsFileExist( str1.c_str() ) )
//			{
//				strFileName = str1;
//				break;
//			}
//		}
//	} while( 0 );
//
//	CReference<CPrefab> pNewPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( strFileName.c_str(), true );
//	auto pNode = pPrefab->GetRoot()->Clone( pNewPrefab.GetPtr() );
//	auto pLevel = (CMyLevel*)pNode->GetStaticDataSafe<CMyLevel>();
//	TVector2<int32> size( ceil( pLevel->GetBound().width / 1024 ), ceil( pLevel->GetBound().height / 1024 ) );
//	if( size != rect.GetSize() )
//	{
//		pLevel->SetBound( CRectangle( rect.width * -0.5f, rect.height * -0.5f, rect.width, rect.height ) * 1024 );
//		auto pTerrainNode = (CPrefabNode*)pNode->GetChildByName( "terrain" );
//		auto pTileMap = static_cast<CTileMap2D*>( pTerrainNode->GetRenderObject() );
//		TVector2<int32> mapSize1( rect.width * 1024 / pTileMap->GetTileSize().x, rect.height * 1024 / pTileMap->GetTileSize().y );
//		TRectangle<int32> newSize( ( (int32)pTileMap->GetWidth() - mapSize1.x ) / 2, ( (int32)pTileMap->GetHeight() - mapSize1.y ) / 2, mapSize1.x, mapSize1.y );
//		pTileMap->Resize( newSize );
//	}
//	pNewPrefab->SetNode( pNode );
//	CBufFile buf;
//	pNewPrefab->Save( buf );
//	SaveFile( pNewPrefab->GetName(), buf.GetBuffer(), buf.GetBufLen() );
//
//	CReference<CRenderObject2D> p = pNewPrefab->GetRoot()->CreateInstance();
//	auto n = Add( p, rect );
//	const char* szText = strFileName.c_str();
//	m_data.vecContext[n].strSceneResName = szText;
//	while( szText )
//	{
//		auto p = strchr( szText, '/' );
//		if( !p )
//			break;
//		szText = p + 1;
//	}
//	m_data.vecContext[n].strName = szText;
//	Select( n );
//}
