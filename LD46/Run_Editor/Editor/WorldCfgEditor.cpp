#include "stdafx.h"
#include "WorldCfgEditor.h"
#include "UICommon/UIFactory.h"
#include "Common/Utf8Util.h"
#include "Render/CommonShader.h"
#include "Render/Scene2DManager.h"
#include "Render/Renderer.h"
#include "Game/MyLevel.h"
#include "Editor/LevelTools.h"
#include "Game/Entities/MiscElem.h"
#include "Editor/Editor.h"

void CWorldCfgEditor::NewFile( const char* szFileName )
{
	if( m_pData )
		delete m_pData;
	CBufFile buf;
	( (SClassMetaData*)NULL )->PackData( NULL, buf, true );
	m_pData = (SWorldCfg*)CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>()->NewObjFromData( buf, true, NULL );
	Validate( szFileName );
	m_nCurSelected = -1;
	m_nCurRegion = -1;
	SelectRegion( m_pData->arrRegionData.Size() ? 0 : -1 );
	Super::NewFile( szFileName );
}

void CWorldCfgEditor::OnOpenFile()
{
	auto szLevel = GetParam( "level" );
	if( szLevel )
	{
		auto& regionData = m_pRes->m_pWorldCfg->arrRegionData;
		for( int i = 0; i < regionData.Size(); i++ )
		{
			auto levelData = regionData[i].arrLevelData;
			for( int j = 0; j < levelData.Size(); j++ )
			{
				if( levelData[j].pLevel == szLevel )
				{
					m_nCurRegion = i;
					m_nCurSelected = j;
					SetCamOfs( levelData[j].displayOfs );
					return;
				}
			}
		}
	}
}

void CWorldCfgEditor::Refresh()
{
	for( auto& reg : m_vecRegionData )
	{
		for( auto& lvl : reg.vecLevelData )
		{
			if( lvl.pLevelPreview )
				lvl.pLevelPreview->RemoveThis();
			if( lvl.pClonedLevelData )
			{
				lvl.pClonedLevelData->Invalidate();
				lvl.pClonedLevelData->RemoveThis();
			}
		}
		if( reg.pRoot )
			reg.pRoot->RemoveThis();
	}
	m_vecRegionData.resize( 0 );
	if( m_pRes )
	{
		auto nRegion = m_nCurRegion;
		auto nSelect = m_nCurSelected;
		m_nCurRegion = -1;
		m_nCurSelected = -1;
		CBufFile buf;
		auto pClassData = CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>();
		pClassData->PackData( (uint8*)m_pRes->m_pWorldCfg, buf, true );
		m_pData = (SWorldCfg*)pClassData->NewObjFromData( buf, true, NULL );
		Validate( m_pRes->GetName() );
		nRegion = Min<int32>( Max( 0, nRegion ), m_pData->arrRegionData.Size() - 1 );
		SelectRegion( nRegion );
		if( nRegion >= 0 )
		{
			nSelect = Min<int32>( nSelect, m_pData->arrRegionData[nRegion].arrLevelData.Size() - 1 );
			Select( nSelect );
		}
	}
	else
	{
		if( m_pData )
		{
			delete m_pData;
			m_pData = NULL;
		}
		m_nCurRegion = -1;
		m_nCurSelected = -1;
	}
}

void CWorldCfgEditor::OnInited()
{
	Super::OnInited();
	m_pPanel[0] = GetChildByName<CUIElement>( "0" );
	m_onSave.Set( this, &CWorldCfgEditor::Save );
	m_pPanel[0]->GetChildByName<CUIButton>( "save" )->Register( eEvent_Action, &m_onSave );
	m_onAutoLayout.Set( this, &CWorldCfgEditor::AutoLayout );
	m_pPanel[0]->GetChildByName<CUIButton>( "auto_layout" )->Register( eEvent_Action, &m_onAutoLayout );
	m_onNewLevel.Set( this, &CWorldCfgEditor::BeginNewLevel );
	m_pPanel[0]->GetChildByName<CUIButton>( "new" )->Register( eEvent_Action, &m_onNewLevel );

	m_pRegionSelect = CDropDownBox::Create( NULL, 0 );
	m_pRegionSelect->Replace( m_pPanel[0]->GetChildByName( "region" ) );
	m_onSelectRegion.Set( this, &CWorldCfgEditor::OnRegionSelectChanged );
	m_pRegionSelect->Register( eEvent_Action, &m_onSelectRegion );
	m_pNewTemplate = CFileNameEdit::Create( "Template", "pf", 80 );
	m_pNewTemplate->Replace( m_pPanel[0]->GetChildByName( "new_template_name" ) );
	m_onBeginNewLevel.Set( this, &CWorldCfgEditor::BeginNewLevel );
	m_pNewTemplate->Register( eEvent_Action, &m_onBeginNewLevel );
	m_pBlueprint = CFileNameEdit::Create( "Blueprint", "pf", 80 );
	m_pBlueprint->Replace( m_pPanel[0]->GetChildByName( "blueprint" ) );
	m_onBlueprintChange.Set( this, &CWorldCfgEditor::OnBlueprintChange );
	m_pBlueprint->Register( eEvent_Action, &m_onBlueprintChange );
	m_pMap = CFileNameEdit::Create( "Map", "pf", 80 );
	m_pMap->Replace( m_pPanel[0]->GetChildByName( "map" ) );
	m_onMapChange.Set( this, &CWorldCfgEditor::OnMapChange );
	m_pMap->Register( eEvent_Action, &m_onMapChange );

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

	for( int i = 0; i < m_pData->arrRegionData.Size(); i++ )
	{
		auto& levelData = m_pData->arrRegionData[i].arrLevelData;
		for( int j = 0; j < levelData.Size(); j++ )
		{
			if( m_vecRegionData[i].vecLevelData[j].bDirty )
			{
				m_vecRegionData[i].vecLevelData[j].bDirty = false;
				CPrefab* pPrefab = levelData[j].pLevel;
				CBufFile buf;
				pPrefab->Save( buf );
				SaveFile( pPrefab->GetName(), buf.GetBuffer(), buf.GetBufLen() );
			}
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

	for( int i = 0; i < m_pData->arrRegionData.Size(); i++ )
	{
		auto& levelData = m_pData->arrRegionData[i].arrLevelData;
		for( int j = 0; j < levelData.Size(); j++ )
		{
			if( m_vecRegionData[i].vecLevelData[j].bDirty )
			{
				RefreshLevelData( i, j );
				CPrefab* pPrefab = levelData[j].pLevel;
				pPrefab->RefreshBegin();
				auto pNode1 = m_vecRegionData[i].vecLevelData[j].pClonedLevelData->Clone( pPrefab );
				pNode1->SetPosition( CVector2( 0, 0 ) );
				pPrefab->SetNode( pNode1 );
				pPrefab->RefreshEnd();
			}
		}
	}
	RefreshSnapShot();
	CBufFile buf;
	auto pClassData = CClassMetaDataMgr::Inst().GetClassData<SWorldCfg>();
	pClassData->PackData( (uint8*)m_pData, buf, true );
	m_pRes->m_pWorldCfg = (SWorldCfg*)pClassData->NewObjFromData( buf, true, NULL );
	m_pRes->RefreshEnd();
}

void CWorldCfgEditor::OnDebugDraw( IRenderSystem* pRenderSystem )
{
	CVector2 ofs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	if( m_nState == 1 )
	{
		CVector2 p = m_pViewport->GetScenePos( GetMgr()->GetMousePos() );
		auto d = ( p - m_newMapBase ) / LEVEL_GRID_SIZE;
		TVector2<int32> pt( floor( d.x ), floor( d.y ) );
		if( !!( ( pt.x + pt.y ) & 1 ) )
			pt.x--;
		CVector2 a = CVector2( pt.x, pt.y ) * LEVEL_GRID_SIZE + m_newMapBase;
		CVector2 b( LEVEL_GRID_SIZE_X * 2, LEVEL_GRID_SIZE_Y );
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = a + b * ofs[j];
			auto pt2 = a + b * ofs[( j + 1 ) % 4];
			m_pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.25f, 0.25f, 0.25f, 0.5f ) );
		}

		for( auto item : m_newMapTiles )
		{
			auto a1 = CVector2( item.x, item.y ) * LEVEL_GRID_SIZE + m_newMapBase;
			for( int j = 0; j < 4; j++ )
			{
				auto pt1 = a1 + b * ofs[j];
				auto pt2 = a1 + b * ofs[( j + 1 ) % 4];
				m_pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0, 0.5f, 0.35f, 0.5f ) );
			}
		}
		return;
	}

	if( m_nCurRegion < 0 )
		return;
	auto& arrLevelData = m_pData->arrRegionData[m_nCurRegion].arrLevelData;
	auto& vecLevelData = m_vecRegionData[m_nCurRegion].vecLevelData;
	for( int i = 0; i < arrLevelData.Size(); i++ )
	{
		auto& data = arrLevelData[i];
		auto pLevel = vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto size = pLevel->GetSize();

		CVector2 p0 = data.displayOfs;
		auto b = CVector2( size.x, size.y ) * LEVEL_GRID_SIZE;
		CVector4 color( 0.1, 0.4, 0.45, 0.5 );
		if( i == m_nCurSelected )
		{
			color = CVector4( 0.25, 0.5, 0.5, 0.5 );
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
	for( auto& item : m_vecRegionData[m_nCurRegion].vecExtLevel )
	{
		auto pLevelData = m_vecRegionData[item.nRegion].vecLevelData[item.nLevel].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto size = pLevelData->GetSize();
		CVector2 p0 = item.ofs;
		auto b = CVector2( size.x, size.y ) * LEVEL_GRID_SIZE;
		for( int j = 0; j < 4; j++ )
		{
			auto pt1 = p0 + b * ofs[j];
			auto pt2 = p0 + b * ofs[( j + 1 ) % 4];
			m_pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0.4, 0.1, 0.3, 0.5 ) );
		}
	}

	if( m_nCurSelected >= 0 && m_nCurSelected < arrLevelData.Size() )
	{
		auto& curSelected = arrLevelData[m_nCurSelected];
		CMatrix2D mat;
		mat.Translate( curSelected.displayOfs.x, curSelected.displayOfs.y );
	}

	for( int i = 0; i < arrLevelData.Size(); i++ )
	{
		auto& data = arrLevelData[i];
		auto& levelData = vecLevelData[i];
		auto pLevel = levelData.pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		auto size = pLevel->GetSize();
		for( int j = 0; j < levelData.vecLinks.size(); j++ )
		{
			auto& link = levelData.vecLinks[j];
			if( link.nTarget < 0 )
				continue;
			auto& nextLevel = pLevel->m_arrNextStage[j];
			auto pLevel1 = m_vecRegionData[link.nRegion].vecLevelData[link.nTarget].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
			auto beginBase = data.displayOfs;
			auto endBase = link.nRegion == m_nCurRegion ? CVector2( -nextLevel.nOfsX, -nextLevel.nOfsY )
				* LEVEL_GRID_SIZE + arrLevelData[link.nTarget].displayOfs : beginBase;
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
	if( m_nCurRegion < 0 || m_nCurSelected < 0 )
		return;
	auto& arrLevelData = m_pData->arrRegionData[m_nCurRegion].arrLevelData;
	auto& vecLevelData = m_vecRegionData[m_nCurRegion].vecLevelData;
	vector<int32> q;
	q.push_back( m_nCurSelected );
	vecLevelData[m_nCurSelected].nFlag = 1;
	for( int i = 0; i < q.size(); i++ )
	{
		auto n = q[i];
		auto& data = arrLevelData[n];
		auto& levelData = vecLevelData[n];
		auto pLevel = levelData.pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		for( int j = 0; j < levelData.vecLinks.size(); j++ )
		{
			auto& link = levelData.vecLinks[j];
			if( link.nRegion != m_nCurRegion )
				continue;
			auto& levelData1 = vecLevelData[link.nTarget];
			if( levelData1.nFlag )
				continue;
			auto& nextLevel = pLevel->m_arrNextStage[j];
			auto ofs = data.displayOfs + CVector2( nextLevel.nOfsX, nextLevel.nOfsY ) * LEVEL_GRID_SIZE;
			arrLevelData[link.nTarget].displayOfs = ofs;
			vecLevelData[link.nTarget].pClonedLevelData->SetPosition( ofs );
			vecLevelData[link.nTarget].pLevelPreview->SetPosition( ofs );
			levelData1.nFlag = 1;
			q.push_back( link.nTarget );
		}
	}
	for( auto& item : vecLevelData )
		item.nFlag = 0;
	RefreshExtLevel( m_nCurRegion );
}

void CWorldCfgEditor::Validate( const char* szName )
{
	string strPath = szName;
	int32 n = strPath.find_last_of( '/' );
	strPath = strPath.substr( 0, n + 1 );
	string strFind = strPath + "*.pf";
	map<string, map<string, CReference<CPrefab> > > mapmapLevelPrefabs;
	FindFiles( strFind.c_str(), [this, &strPath, &mapmapLevelPrefabs] ( const char* szFileName )
	{
		string strFullPath = strPath;
		strFullPath += szFileName;
		CReference<CPrefab> pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( strFullPath.c_str() );
		auto pLevelData = pPrefab->GetRoot()->GetStaticDataSafe<CMyLevel>();
		if( pLevelData )
			mapmapLevelPrefabs[pLevelData->m_strRegion.c_str()][strFullPath] = pPrefab;
		return true;
	}, true, false );

	for( int iRegion = 0; iRegion < m_pData->arrRegionData.Size(); iRegion++ )
	{
		auto itr = mapmapLevelPrefabs.find( m_pData->arrRegionData[iRegion].strName.c_str() );
		if( itr == mapmapLevelPrefabs.end() )
		{
			if( iRegion < m_pData->arrRegionData.Size() - 1 )
				m_pData->arrRegionData[iRegion] = m_pData->arrRegionData[m_pData->arrRegionData.Size() - 1];
			m_pData->arrRegionData.Resize( m_pData->arrRegionData.Size() - 1 );
			iRegion--;
			continue;
		}
		auto& mapLevelPrefabs = itr->second;

		auto& arrLevelData = m_pData->arrRegionData[iRegion].arrLevelData;
		int i1 = 0;
		for( int i = 0; i < arrLevelData.Size(); i++ )
		{
			auto itr = mapLevelPrefabs.find( arrLevelData[i].pLevel.c_str() );
			if( itr != mapLevelPrefabs.end() )
			{
				if( i1 != i )
					arrLevelData[i1] = arrLevelData[i];
				arrLevelData[i1].pLevel = itr->second;
				itr->second = NULL;
				i1++;
			}
		}
		arrLevelData.Resize( i1 );

		InitRegion( iRegion, mapLevelPrefabs );
		mapmapLevelPrefabs.erase( itr );
	}

	for( auto& item : mapmapLevelPrefabs )
	{
		auto& mapLevelPrefabs = item.second;
		int32 iRegion = m_pData->arrRegionData.Size();
		m_pData->arrRegionData.Resize( iRegion + 1 );
		m_pData->arrRegionData[iRegion].strName = item.first.c_str();
		InitRegion( iRegion, mapLevelPrefabs );
	}

	map<CString, TVector2<int32> > mapLevelIndex;
	for( int i = 0; i < m_pData->arrRegionData.Size(); i++ )
	{
		auto& levelData = m_pData->arrRegionData[i].arrLevelData;
		for( int j = 0; j < levelData.Size(); j++ )
			mapLevelIndex[levelData[j].pLevel] = TVector2<int32>( i, j );
	}
	for( int i = 0; i < m_vecRegionData.size(); i++ )
	{
		for( int j = 0; j < m_vecRegionData[i].vecLevelData.size(); j++ )
		{
			RefreshLevelData( i, j );
			RefreshLevelDataLink( i, j, &mapLevelIndex );
		}
		RefreshExtLevel( i );
	}

	vector<CDropDownBox::SItem> vecItems;
	for( int i = 0; i < m_pData->arrRegionData.Size(); i++ )
	{
		CDropDownBox::SItem item = { m_pData->arrRegionData[i].strName, (void*)i };
		vecItems.push_back( item );
	}
	std::sort( vecItems.begin(), vecItems.end(), [] ( const CDropDownBox::SItem& a, const CDropDownBox::SItem& b ) {
		return a.name < b.name;
	} );
	m_pRegionSelect->ResetItems( vecItems.size() ? &vecItems[0] : NULL, vecItems.size() );
}

void CWorldCfgEditor::InitRegion( int32 nRegion, map<string, CReference<CPrefab> >& mapLevelPrefabs )
{
	auto& arrLevelData = m_pData->arrRegionData[nRegion].arrLevelData;
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
		yMax = Max( yMax, pLevelData->m_nHeight * LEVEL_GRID_SIZE_Y + data.displayOfs.y );
		xMin = Min( xMin, data.displayOfs.x );
	}
	for( int i = i1; i < arrLevelData.Size(); i++ )
	{
		auto& data = arrLevelData[i];
		auto pLevelData = data.pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
		auto& nxtStage = pLevelData->m_arrNextStage;
		bool b = false;
		CVector2 p;
		for( int j = 0; j < i; j++ )
		{
			TVector2<int32> ofs;
			auto& data1 = arrLevelData[j];
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
	}

	m_vecRegionData.resize( Max<int32>( nRegion + 1, m_vecRegionData.size() ) );
	auto& vecLevelData = m_vecRegionData[nRegion].vecLevelData;
	auto pRegionRoot = new CRenderObject2D;
	m_vecRegionData[nRegion].pRoot = pRegionRoot;
	if( m_pData->arrRegionData[nRegion].pBlueprint.length() )
	{
		auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>(
			m_pData->arrRegionData[nRegion].pBlueprint );
		m_vecRegionData[nRegion].pBack = pPrefab->GetRoot()->CreateInstance();
		m_vecRegionData[nRegion].pBack->SetZOrder( -1 );
		m_vecRegionData[nRegion].pRoot->AddChild( m_vecRegionData[nRegion].pBack );
		m_pData->arrRegionData[nRegion].pBlueprint = pPrefab;
	}

	vecLevelData.resize( arrLevelData.Size() );
	for( int i = 0; i < arrLevelData.Size(); i++ )
	{
		auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( arrLevelData[i].pLevel.c_str() );
		InitLevel( nRegion, i, pPrefab );
	}
}

void CWorldCfgEditor::InitLevel( int32 nRegion, int32 nLevel, CPrefab* pPrefab )
{
	auto& levelData = m_vecRegionData[nRegion].vecLevelData[nLevel];
	auto& lv = m_pData->arrRegionData[nRegion].arrLevelData[nLevel];
	lv.pLevel = pPrefab;
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
	m_vecRegionData[nRegion].pRoot->AddChild( pPreview );
}

void CWorldCfgEditor::SelectRegion( int32 n )
{
	if( m_nCurRegion == n )
		return;
	Select( -1 );
	if( m_nCurRegion >= 0 && m_nCurRegion < m_vecRegionData.size() )
		m_vecRegionData[m_nCurRegion].pRoot->RemoveThis();
	m_nCurRegion = n;
	if( m_nCurRegion >= 0 && m_nCurRegion < m_vecRegionData.size() )
	{
		m_pViewport->GetRoot()->AddChild( m_vecRegionData[m_nCurRegion].pRoot );
		m_pRegionSelect->SetSelectedItem( m_pData->arrRegionData[m_nCurRegion].strName, false );
	}
	m_pBlueprint->SetText( m_pData->arrRegionData[m_nCurRegion].pBlueprint );
	m_pMap->SetText( m_pData->arrRegionData[m_nCurRegion].pMap );
}

void CWorldCfgEditor::Select( int32 n )
{
	if( m_nCurRegion < 0 || m_nCurRegion >= m_vecRegionData.size() )
		return;
	auto& vecLevelData = m_vecRegionData[m_nCurRegion].vecLevelData;
	if( m_nCurSelected == n )
		return;
	if( m_nCurSelected >= 0 && m_nCurSelected < vecLevelData.size() )
	{
		auto& item = vecLevelData[m_nCurSelected];
		item.pLevelPreview = CLevelToolsView::CreateLevelSimplePreview( item.pClonedLevelData );
		item.pLevelPreview->SetPosition( item.pClonedLevelData->GetPosition() );
		m_vecRegionData[m_nCurRegion].pRoot->AddChildBefore( item.pLevelPreview, item.pClonedLevelData );
		item.pClonedLevelData->RemoveThis();
	}
	m_nCurSelected = n;
	if( n >= 0 && n < vecLevelData.size() )
	{
		auto& item = vecLevelData[m_nCurSelected];
		m_vecRegionData[m_nCurRegion].pRoot->AddChild( item.pClonedLevelData );
		item.pLevelPreview->RemoveThis();
		item.pLevelPreview = NULL;

		m_pNewTemplate->SetText( m_pData->arrRegionData[m_nCurRegion].arrLevelData[m_nCurSelected].pLevel );
	}
}

void CWorldCfgEditor::OnLevelDataEdit( int32 nRegion, int32 nLevel )
{
	m_vecRegionData[nRegion].vecLevelData[nLevel].bDirty = true;
	m_vecRegionData[nRegion].vecLevelData[nLevel].pClonedLevelData->OnEdit();
	RefreshLevelDataLink( nRegion, nLevel );
}

void CWorldCfgEditor::RefreshLevelData( int32 nRegion, int32 nLevel )
{
	auto& levelData = m_pData->arrRegionData[nRegion].arrLevelData[nLevel];
	levelData.arrGrids.Resize( 0 );
	levelData.arrNxtStages.Resize( 0 );
	levelData.arrConsoles.Resize( 0 );
	levelData.arrIconData.Resize( 0 );
	auto pLevelNode = m_vecRegionData[nRegion].vecLevelData[nLevel].pClonedLevelData;
	auto pLevel = pLevelNode->GetStaticDataSafe<CMyLevel>();
	auto levelSize = pLevel->GetSize();
	for( int i = 0; i < levelSize.x; i++ )
	{
		for( int j = 0; j < levelSize.y; j++ )
		{
			if( !!( ( i + j ) & 1 ) )
				continue;
			auto& grid = pLevel->m_arrGridData[i + j * levelSize.x];
			if( grid.nNextStage > 0 )
			{
				levelData.arrNxtStages.Resize( levelData.arrNxtStages.Size() + 1 );
				levelData.arrNxtStages[levelData.arrNxtStages.Size() - 1] = CVector2( i, j );
			}
			else
			{
				auto nTile = grid.nTile;
				if( !pLevel->m_arrTileData[nTile].bBlocked )
				{
					levelData.arrGrids.Resize( levelData.arrGrids.Size() + 1 );
					levelData.arrGrids[levelData.arrGrids.Size() - 1] = CVector2( i, j );
				}
			}
		}
	}
	for( auto p = pLevelNode->Get_RenderChild(); p; p = p->NextRenderChild() )
	{
		if( p == pLevelNode->GetRenderObject() || p == pLevelNode->GetPatchedNode() )
			continue;
		auto pPrefabNode = static_cast<CPrefabNode*>( p );
		if( pPrefabNode->GetName() == "1" || pPrefabNode->GetClassData() && pPrefabNode->GetClassData()->Is( CClassMetaDataMgr::Inst().GetClassData<CPawnLayer>() ) )
		{
			auto pPawnLayer = pPrefabNode->GetStaticDataSafe<CPawnLayer>();
			for( auto p1 = pPrefabNode->Get_RenderChild(); p1; p1 = p1->NextRenderChild() )
			{
				if( p1 == pPrefabNode->GetRenderObject() || p1 == pPrefabNode->GetPatchedNode() )
					continue;
				auto pPawnNode = static_cast<CPrefabNode*>( p1 );
				if( !pPawnNode->GetPatchedNode() )
					continue;
				auto pSpawnHelper = pPawnNode->GetStaticDataSafe<CLevelSpawnHelper>();
				if( pSpawnHelper && pSpawnHelper->GetSpawnType() >= 2 )
					continue;
				auto pData = pPawnNode->GetPatchedNode()->GetClassData();
				if( pData )
				{
					if( pData->Is( CClassMetaDataMgr::Inst().GetClassData<CConsole>() ) )
					{
						levelData.arrConsoles.Resize( levelData.arrConsoles.Size() + 1 );
						levelData.arrConsoles[levelData.arrConsoles.Size() - 1] = CVector2( floor( pPawnNode->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pPawnNode->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
					}
					auto pPawn = pPawnNode->GetPatchedNode()->GetStaticDataSafe<CPawn>();
					if( pPawn )
						pPawn->CreateIconData( pPawnNode, pPawnLayer ? pPawnLayer->GetCondition().c_str() : "", levelData.arrIconData );

					auto pPatchedNode = pPawnNode->GetPatchedNode();
					for( auto pPawnChild = pPatchedNode->Get_RenderChild(); pPawnChild; pPawnChild = pPawnChild->NextRenderChild() )
					{
						if( pPawnChild == pPatchedNode->GetRenderObject() || pPawnChild == pPatchedNode->GetPatchedNode() )
							continue;
						auto pPrefabNode1 = static_cast<CPrefabNode*>( pPawnChild );
						if( pPrefabNode1->GetName() == "ai" )
						{
							auto pPawnAI = pPrefabNode1->GetStaticDataSafe<CPawnAI>();
							if( pPawnAI )
								pPawnAI->CreateIconData( pPawnNode, pPawnLayer ? pPawnLayer->GetCondition().c_str() : "", levelData.arrIconData );
						}
					}
				}
			}
		}
	}
}

void CWorldCfgEditor::RefreshLevelDataLink( int32 nRegion, int32 n, map<CString, TVector2<int32> >* pMap )
{
	auto& vecLevelData = m_vecRegionData[nRegion].vecLevelData;
	auto pLevelData = vecLevelData[n].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
	auto& links = vecLevelData[n].vecLinks;
	links.resize( 0 );
	links.resize( pLevelData->m_arrNextStage.Size() );
	for( int i = 0; i < pLevelData->m_arrNextStage.Size(); i++ )
	{
		auto& link = links[i];
		link.nRegion = link.nTarget = -1;
		auto& nextStage = pLevelData->m_arrNextStage[i];
		if( pMap )
		{
			auto itr = pMap->find( nextStage.pNxtStage );
			if( itr != pMap->end() )
			{
				link.nRegion = itr->second.x;
				link.nTarget = itr->second.y;
			}
		}
		else
		{
			for( int iRegion = 0; iRegion < m_vecRegionData.size() && link.nRegion == -1; iRegion++ )
			{
				auto& arrLevelData = m_pData->arrRegionData[iRegion].arrLevelData;
				for( int j = 0; j < arrLevelData.Size(); j++ )
				{
					CString str = arrLevelData[j].pLevel;
					if( str == nextStage.pNxtStage.c_str() )
					{
						link.nRegion = iRegion;
						link.nTarget = j;
						break;
					}
				}
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

void CWorldCfgEditor::RefreshExtLevel( int32 nRegion )
{
	auto& vecLevelData = m_vecRegionData[nRegion].vecLevelData;
	auto& vecExtLevel = m_vecRegionData[nRegion].vecExtLevel;
	vecExtLevel.resize( 0 );
	for( int i = 0; i < vecLevelData.size(); i++ )
	{
		auto pLevelData = vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
		for( int j = 0; j < vecLevelData[i].vecLinks.size(); j++ )
		{
			auto& link = vecLevelData[i].vecLinks[j];
			if( link.nRegion != nRegion )
			{
				auto& data = m_vecRegionData[link.nRegion].vecLevelData[link.nTarget];
				if( !data.nFlag )
				{
					data.nFlag = 1;
					auto& nextLevel = pLevelData->m_arrNextStage[j];
					auto ofs = m_pData->arrRegionData[nRegion].arrLevelData[i].displayOfs + CVector2( nextLevel.nOfsX, nextLevel.nOfsY ) * LEVEL_GRID_SIZE;
					SRegionData::SExtLevelData a = { link.nRegion, link.nTarget, ofs };
					vecExtLevel.push_back( a );
				}
			}
		}
	}
	for( auto& item : vecExtLevel )
		m_vecRegionData[item.nRegion].vecLevelData[item.nLevel].nFlag = 0;
}

void CWorldCfgEditor::RefreshSnapShot()
{
	vector<int8> vecTemp;
	TVector2<int32> ofs[7] = { { 0, 0 }, { 2, 0 }, { -2, 0 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
	for( int i = 0; i < m_pData->arrRegionData.Size(); i++ )
	{
		TRectangle<int32> regionSize( 0, 0, 0, 0 );
		auto& levelData = m_pData->arrRegionData[i].arrLevelData;
		int32 nTotalAreaSize = 0;
		for( int j = 0; j < levelData.Size(); j++ )
		{
			levelData[j].arrShowSnapShot.Resize( 0 );
			auto pLevel = levelData[j].pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
			auto size = pLevel->GetSize();
			auto bound1 = TRectangle<int32>( levelData[j].displayOfs.x / LEVEL_GRID_SIZE_X, levelData[j].displayOfs.y / LEVEL_GRID_SIZE_Y, size.x, size.y );
			bound1.x -= 2;
			bound1.y -= 1;
			bound1.width += 4;
			bound1.height += 2;
			regionSize = j == 0 ? bound1 : regionSize + bound1;
			nTotalAreaSize += ( bound1.width * bound1.height + 1 ) / 2;
		}

		struct SNode
		{
			int32 j;
			LINK_LIST( SNode, Node )
		};
		vector<SNode> vecNodes;
		vecNodes.resize( nTotalAreaSize );
		vector<SNode*> vecMap;
		vecMap.resize( regionSize.width * regionSize.height );
		int32 iNode = 0;
		for( int j = 0; j < levelData.Size(); j++ )
		{
			auto& data = levelData[j];
			auto pLevel = data.pLevel->GetRoot()->GetStaticDataSafe<CMyLevel>();
			auto size = pLevel->GetSize();
			auto bound0 = TRectangle<int32>( levelData[j].displayOfs.x / LEVEL_GRID_SIZE_X, levelData[j].displayOfs.y / LEVEL_GRID_SIZE_Y, size.x, size.y );
			auto bound1 = bound0;
			bound1.x -= 2;
			bound1.y -= 1;
			bound1.width += 4;
			bound1.height += 2;
			vecTemp.resize( bound1.width * bound1.height );
			set<string> setNames;

			for( int x = 0; x < size.x; x++ )
			{
				for( int y = 0; y < size.y; y++ )
				{
					if( !!( ( x + y ) & 1 ) )
						continue;
					auto pGridData = pLevel->GetGridData( TVector2<int32>( x, y ) );
					if( !pGridData->nTile )
						continue;
					for( int k = 0; k < ELEM_COUNT( ofs ); k++ )
					{
						auto p = TVector2<int32>( x, y ) + ofs[k];
						auto& b = vecTemp[p.x + 2 + ( p.y + 1 ) * bound1.width];
						if( b )
							continue;
						b = 1;

						auto pNode = &vecNodes[iNode++];
						pNode->j = j;
						SNode*& pHead = vecMap[p.x + bound0.x - regionSize.x + ( p.y + bound0.y - regionSize.y ) * regionSize.width];

						for( SNode* pNode = pHead; pNode; pNode = pNode->NextNode() )
						{
							auto& data1 = levelData[pNode->j];
							if( setNames.find( data1.pLevel.c_str() ) != setNames.end() )
								continue;
							setNames.insert( data1.pLevel.c_str() );
							data1.arrShowSnapShot.Resize( data1.arrShowSnapShot.Size() + 1 );
							data1.arrShowSnapShot[data1.arrShowSnapShot.Size() - 1] = data.pLevel.c_str();
							data.arrShowSnapShot.Resize( data.arrShowSnapShot.Size() + 1 );
							data.arrShowSnapShot[data.arrShowSnapShot.Size() - 1] = data1.pLevel.c_str();
						}

						pNode->InsertTo_Node( pHead );
					}
				}
			}
			vecTemp.resize( 0 );
		}
	}
}

void CWorldCfgEditor::ShowLevelTool()
{
	if( m_nCurRegion < 0 || m_nCurSelected < 0 )
		return;
	m_pViewport->bVisible = false;
	auto& curLevelData = m_vecRegionData[m_nCurRegion].vecLevelData[m_nCurSelected];
	auto& cur = m_pData->arrRegionData[m_nCurRegion].arrLevelData[m_nCurSelected];
	CRenderObject2D* pBack = NULL;
	if( m_pData->arrRegionData[m_nCurRegion].pBlueprint )
		pBack = m_pData->arrRegionData[m_nCurRegion].pBlueprint->GetRoot()->CreateInstance();
	CLevelToolsView::Inst()->Set( curLevelData.pClonedLevelData, [this, &curLevelData, &cur] () {
		m_pViewport->bVisible = true;
		curLevelData.pClonedLevelData->SetPosition( cur.displayOfs );
		OnLevelDataEdit( m_nCurRegion, m_nCurSelected );
		for( int i = 0; i < curLevelData.vecLinks.size(); i++ )
		{
			auto& link = curLevelData.vecLinks[i];
			OnLevelDataEdit( link.nRegion, link.nTarget );
		}
		RefreshExtLevel( m_nCurRegion );
	}, &cur, pBack );
	for( int i = 0; i < curLevelData.vecLinks.size(); i++ )
	{
		auto& link = curLevelData.vecLinks[i];
		auto& nxt = m_pData->arrRegionData[link.nRegion].arrLevelData[link.nTarget];
		CVector2 ofs;
		if( link.nRegion == m_nCurRegion )
			ofs = nxt.displayOfs - cur.displayOfs;
		else
		{
			auto& nxtStage = curLevelData.pClonedLevelData->GetStaticDataSafe<CMyLevel>()->m_arrNextStage[i];
			ofs = CVector2( nxtStage.nOfsX, nxtStage.nOfsY ) * LEVEL_GRID_SIZE;
		}

		CLevelToolsView::Inst()->AddNeighbor( m_vecRegionData[link.nRegion].vecLevelData[link.nTarget].pClonedLevelData, ofs );
	}
}

void CWorldCfgEditor::OpenLevelFile()
{
	if( m_nCurRegion < 0 || m_nCurSelected < 0 )
		return;
	Save();
	string str = "world=";
	str += GetFileName();
	auto& cur = m_pData->arrRegionData[m_nCurRegion].arrLevelData[m_nCurSelected];
	CEditor::Inst().OpenFile( cur.pLevel.c_str(), str.c_str() );
}

void CWorldCfgEditor::BeginNewLevel()
{
	if( m_nState != 0 || m_nCurRegion < 0 )
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
	if( m_nCurSelected >= 0 )
		m_newMapBase = m_pData->arrRegionData[m_nCurRegion].arrLevelData[m_nCurSelected].displayOfs;
	else
		m_newMapBase = CVector2( 0, 0 );
	Select( -1 );
	m_nState = 1;
	m_pPanel[0]->SetVisible( false );
	m_pPanel[1]->SetVisible( true );
}

void CWorldCfgEditor::EndNewLevel( bool bOK )
{
	if( m_nState != 1 )
		return;
	if( bOK && m_newMapTiles.size() )
	{
		TRectangle<int32> bound( 0, 0, 0, 0 );
		for( auto& p : m_newMapTiles )
			bound = bound.width ? bound + TRectangle<int32>( p.x, p.y, 2, 1 ) : TRectangle<int32>( p.x, p.y, 2, 1 );

		auto regionBound = bound.Offset( TVector2<int32>( floor( m_newMapBase.x / LEVEL_GRID_SIZE_X + 0.5f ), floor( m_newMapBase.y / LEVEL_GRID_SIZE_Y + 0.5f ) ) );
		auto& arrLevelData = m_pData->arrRegionData[m_nCurRegion].arrLevelData;
		auto& vecLevelData = m_vecRegionData[m_nCurRegion].vecLevelData;
		vector<TVector2<int32> > vecLinkLevels;
		for( int i = 0; i < vecLevelData.size(); i++ )
		{
			auto pLevel1 = SafeCast<CMyLevel>( vecLevelData[i].pClonedLevelData->GetFinalObjData() );
			auto bound1 = TRectangle<int32>( arrLevelData[i].displayOfs.x / LEVEL_GRID_SIZE_X, arrLevelData[i].displayOfs.y / LEVEL_GRID_SIZE_Y,
				pLevel1->GetSize().x, pLevel1->GetSize().y );
			auto boundX = bound1 * regionBound;
			if( boundX.width && boundX.height )
				vecLinkLevels.push_back( TVector2<int32>( i, -1 ) );
		}
		if( !!( ( regionBound.x + regionBound.y ) & 1 ) )
		{
			regionBound.SetLeft( regionBound.x - 1 );
			bound.SetLeft( bound.x - 1 );
		}

		int32 nWidth = bound.width;
		int32 nHeight = bound.height;
		string strName = m_pRes->GetName();
		int32 n = strName.find_last_of( '/' );
		strName = strName.substr( 0, n + 1 );
		strName = strName + UnicodeToUtf8( m_pNewLevelName->GetText() ) + ".pf";
		auto pPrefab = CLevelToolsView::NewLevelFromTemplate( m_pNewMapTemplate, strName.c_str(), nWidth, nHeight );
		if( !pPrefab )
			goto fail;
		auto pLevelData = SafeCast<CMyLevel>( pPrefab->GetRoot()->GetFinalObjData() );
		pLevelData->m_strRegion = m_pData->arrRegionData[m_nCurRegion].strName;

		for( auto& p : m_newMapTiles )
		{
			TVector2<int32> pt[2];
			pt[0] = p - TVector2<int32>( bound.x, bound.y );
			pt[1] = pt[0] + TVector2<int32>( 1, 0 );
			for( int i = 0; i < 2; i++ )
			{
				auto& grid = pLevelData->m_arrGridData[pt[i].x + pt[i].y * nWidth];
				grid.nTile = 1;
				grid.bBlocked = false;
			}

			for( auto& item : vecLinkLevels )
			{
				auto pLevel1 = SafeCast<CMyLevel>( vecLevelData[item.x].pClonedLevelData->GetFinalObjData() );
				auto ofs = arrLevelData[item.x].displayOfs - m_newMapBase;

				auto bound1 = TRectangle<int32>( floor( ofs.x / LEVEL_GRID_SIZE_X + 0.5f ), floor( ofs.y / LEVEL_GRID_SIZE_Y + 0.5f ),
					pLevel1->GetSize().x, pLevel1->GetSize().y );
				TVector2<int32> p1[2];
				p1[0] = p - TVector2<int32>( bound1.x, bound1.y );
				p1[1] = p1[0] + TVector2<int32>( 1, 0 );
				if( p1[0].x >= 0 && p1[0].y >= 0 && p1[1].x < bound1.width && p1[1].y < bound1.height )
				{
					auto& grid = pLevel1->m_arrGridData[p1[0].x + p1[0].y * bound1.width];
					if( !grid.nNextStage && !pLevel1->m_arrTileData[grid.nTile].bBlocked )
					{
						if( item.y < 0 )
						{
							item.y = pLevelData->m_arrNextStage.Size();
							pLevelData->m_arrNextStage.Resize( item.y + 1 );
							auto& data = pLevelData->m_arrNextStage[item.y];
							data.nOfsX = pt[0].x - p1[0].x;
							data.nOfsY = pt[0].y - p1[0].y;
							data.pNxtStage = arrLevelData[item.x].pLevel.c_str();

							pLevel1->m_arrNextStage.Resize( pLevel1->m_arrNextStage.Size() + 1 );
							auto& data1 = pLevel1->m_arrNextStage[pLevel1->m_arrNextStage.Size() - 1];
							data1.nOfsX = -data.nOfsX;
							data1.nOfsY = -data.nOfsY;
							data1.pNxtStage = strName.c_str();
						}

						for( int i = 0; i < 2; i++ )
						{
							pLevelData->m_arrGridData[pt[i].x + pt[i].y * nWidth].nNextStage = item.y + 1;
							pLevel1->m_arrGridData[p1[i].x + p1[i].y * bound1.width].nNextStage = pLevel1->m_arrNextStage.Size();
						}
						break;
					}
				}
			}
		}

		auto nNewLevel = m_pData->arrRegionData[m_nCurRegion].arrLevelData.Size();
		m_pData->arrRegionData[m_nCurRegion].arrLevelData.Resize( nNewLevel + 1 );
		m_vecRegionData[m_nCurRegion].vecLevelData.resize( nNewLevel + 1 );
		auto& levelData = m_pData->arrRegionData[m_nCurRegion].arrLevelData[nNewLevel];
		levelData.pLevel = strName.c_str();
		levelData.displayOfs = CVector2( bound.x, bound.y ) * LEVEL_GRID_SIZE + m_newMapBase;
		CBufFile buf;
		pPrefab->Save( buf );
		SaveFile( pPrefab->GetName(), buf.GetBuffer(), buf.GetBufLen() );
		InitLevel( m_nCurRegion, nNewLevel, pPrefab );
		RefreshLevelDataLink( m_nCurRegion, nNewLevel );
		for( auto& item : vecLinkLevels )
		{
			if( item.y >= 0 )
				OnLevelDataEdit( m_nCurRegion, item.x );
		}
		RefreshExtLevel( m_nCurRegion );
	}
fail:
	m_pNewMapTemplate = NULL;
	m_nState = 0;
	m_newMapTiles.clear();
	m_pPanel[0]->SetVisible( true );
	m_pPanel[1]->SetVisible( false );
}

void CWorldCfgEditor::MendLevels( const TVector2<int32>& p )
{
	auto& arrLevelData = m_pData->arrRegionData[m_nCurRegion].arrLevelData;
	auto& vecLevelData = m_vecRegionData[m_nCurRegion].vecLevelData;
	CMyLevel* pLevel0 = NULL;
	int32 nLevel = -1;
	TVector2<int32> p0;
	for( int i = 0; i < vecLevelData.size(); i++ )
	{
		auto pLevel1 = SafeCast<CMyLevel>( vecLevelData[i].pClonedLevelData->GetFinalObjData() );
		auto bound1 = TRectangle<int32>( arrLevelData[i].displayOfs.x / LEVEL_GRID_SIZE_X, arrLevelData[i].displayOfs.y / LEVEL_GRID_SIZE_Y,
			pLevel1->GetSize().x, pLevel1->GetSize().y );
		auto p1 = p - TVector2<int32>( bound1.x, bound1.y );
		if( !!( ( p1.x + p1.y ) & 1 ) )
			p1.x--;
		if( p1.x < 0 || p1.y < 0 || p1.x >= bound1.width || p1.y >= bound1.height )
			continue;
		auto& grid = pLevel1->m_arrGridData[p1.x + p1.y * bound1.width];
		if( !grid.nNextStage && !pLevel1->m_arrTileData[grid.nTile].bBlocked )
		{
			if( !pLevel0 )
			{
				pLevel0 = pLevel1;
				nLevel = i;
				p0 = p1;
			}
			else
			{
				int32 nNxt0 = -1, nNxt1 = -1;

				for( int k = 0; k < pLevel0->m_arrNextStage.Size(); k++ )
				{
					if( pLevel0->m_arrNextStage[k].pNxtStage == arrLevelData[i].pLevel.c_str() )
					{
						nNxt0 = k;
						break;
					}
				}
				if( nNxt0 == -1 )
				{
					pLevel0->m_arrNextStage.Resize( pLevel0->m_arrNextStage.Size() + 1 );
					auto& data = pLevel0->m_arrNextStage[pLevel0->m_arrNextStage.Size() - 1];
					data.nOfsX = p0.x - p1.x;
					data.nOfsY = p0.y - p1.y;
					data.pNxtStage = arrLevelData[i].pLevel.c_str();
					nNxt0 = pLevel0->m_arrNextStage.Size();
				}

				for( int k = 0; k < pLevel1->m_arrNextStage.Size(); k++ )
				{
					if( pLevel1->m_arrNextStage[k].pNxtStage == arrLevelData[nLevel].pLevel.c_str() )
					{
						nNxt1 = k;
						break;
					}
				}
				if( nNxt1 == -1 )
				{
					pLevel1->m_arrNextStage.Resize( pLevel1->m_arrNextStage.Size() + 1 );
					auto& data1 = pLevel1->m_arrNextStage[pLevel1->m_arrNextStage.Size() - 1];
					data1.nOfsX = p1.x - p0.x;
					data1.nOfsY = p1.y - p0.y;
					data1.pNxtStage = arrLevelData[nLevel].pLevel.c_str();
					nNxt1 = pLevel1->m_arrNextStage.Size();
				}

				for( int i = 0; i < 2; i++ )
				{
					pLevel0->m_arrGridData[p0.x + i + p0.y * pLevel0->GetSize().x].nNextStage = nNxt0;
					pLevel1->m_arrGridData[p1.x + i + p1.y * bound1.width].nNextStage = nNxt1;
				}
				OnLevelDataEdit( m_nCurRegion, nLevel );
				OnLevelDataEdit( m_nCurRegion, i );
				RefreshExtLevel( m_nCurRegion );
				return;
			}
		}
	}
}

void CWorldCfgEditor::OnBlueprintChange()
{
	auto strName = UnicodeToUtf8( m_pBlueprint->GetText() );
	CRenderObject2D* p = NULL;
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( strName.c_str() );
	if( pPrefab )
		p = pPrefab->GetRoot()->CreateInstance();
	if( m_vecRegionData[m_nCurRegion].pBack )
		m_vecRegionData[m_nCurRegion].pBack->RemoveThis();
	m_vecRegionData[m_nCurRegion].pBack = p;
	if( p )
	{
		p->SetZOrder( -1 );
		m_vecRegionData[m_nCurRegion].pRoot->AddChild( p );
	}
	m_pData->arrRegionData[m_nCurRegion].pBlueprint = strName.c_str();
	m_pData->arrRegionData[m_nCurRegion].pBlueprint = pPrefab;
}

void CWorldCfgEditor::OnMapChange()
{
	auto strName = UnicodeToUtf8( m_pMap->GetText() );
	m_pData->arrRegionData[m_nCurRegion].pMap = strName.c_str();
}

void CWorldCfgEditor::OnViewportStartDrag( SUIMouseEvent* pEvent )
{
	m_nDragType = 0;
	CVector2 p = m_pViewport->GetScenePos( pEvent->mousePos );
	if( m_nState == 1 )
	{
		if( GetMgr()->IsKey( 'Z' ) )
		{
			Super::OnViewportStartDrag( pEvent );
			return;
		}
		auto d = ( p - m_newMapBase ) / LEVEL_GRID_SIZE;
		TVector2<int32> pt( floor( d.x ), floor( d.y ) );
		if( !!( ( pt.x + pt.y ) & 1 ) )
			pt.x--;
		if( m_newMapTiles.find( pt ) == m_newMapTiles.end() )
			m_newMapTiles.insert( pt );
		else
			m_newMapTiles.erase( pt );
		m_nDragType = 1;
		return;
	}
	else if( m_nCurRegion >= 0 )
	{
		if( GetMgr()->IsKey( 'M' ) )
		{
			CVector2 p = m_pViewport->GetScenePos( GetMgr()->GetMousePos() );
			MendLevels( TVector2<int32>( floor( p.x / LEVEL_GRID_SIZE_X ), floor( p.y / LEVEL_GRID_SIZE_Y ) ) );
			return;
		}

		auto& arrLevelData = m_pData->arrRegionData[m_nCurRegion].arrLevelData;
		auto& vecLevelData = m_vecRegionData[m_nCurRegion].vecLevelData;
		for( int i = 0; i < arrLevelData.Size(); i++ )
		{
			auto pLevelData = vecLevelData[i].pClonedLevelData->GetStaticDataSafe<CMyLevel>();
			auto ofs = arrLevelData[i].displayOfs;
			CRectangle r( ofs.x, ofs.y, pLevelData->m_nWidth * LEVEL_GRID_SIZE_X, pLevelData->m_nHeight * LEVEL_GRID_SIZE_Y );
			if( r.Contains( p ) )
			{
				auto d = ( p - ofs ) / LEVEL_GRID_SIZE;
				auto pGridData = pLevelData->GetGridData( TVector2<int32>( floor( d.x ), floor( d.y ) ) );
				if( pGridData && pGridData->nTile )
				{
					m_nDragType = 1;
					m_worldDragBeginPos = p;
					m_curDisplayOfs0 = ofs;
					Select( i );
					return;
				}
			}
		}
		for( auto& item : m_vecRegionData[m_nCurRegion].vecExtLevel )
		{
			auto& level = m_vecRegionData[item.nRegion].vecLevelData[item.nLevel];
			auto pLevelData = level.pClonedLevelData->GetStaticDataSafe<CMyLevel>();
			CRectangle r( item.ofs.x, item.ofs.y, pLevelData->m_nWidth * LEVEL_GRID_SIZE_X, pLevelData->m_nHeight * LEVEL_GRID_SIZE_Y );
			if( r.Contains( p ) )
			{
				auto d = ( p - item.ofs ) / LEVEL_GRID_SIZE;
				auto pGridData = pLevelData->GetGridData( TVector2<int32>( floor( d.x ), floor( d.y ) ) );
				if( pGridData && pGridData->nTile )
				{
					SelectRegion( item.nRegion );
					Select( item.nLevel );
					SetCamOfs( m_camOfs + m_pData->arrRegionData[item.nRegion].arrLevelData[item.nLevel].displayOfs - item.ofs );
					break;
				}
			}
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

	if( m_nState == 0 )
	{
		auto& arrLevelData = m_pData->arrRegionData[m_nCurRegion].arrLevelData;
		auto& vecLevelData = m_vecRegionData[m_nCurRegion].vecLevelData;
		auto d = p - m_worldDragBeginPos;
		TVector2<int32> ofs( floor( d.x / LEVEL_GRID_SIZE_X + 0.5f ), floor( d.y / LEVEL_GRID_SIZE_Y + 0.5f ) );
		auto& data = arrLevelData[m_nCurSelected];
		data.displayOfs = m_curDisplayOfs0 + CVector2( ofs.x, ofs.y ) * LEVEL_GRID_SIZE;
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
	if( nChar == VK_F1 )
		ShowLevelTool();
	if( nChar == VK_F2 )
		OpenLevelFile();
}

void CWorldCfgEditor::OnRegionSelectChanged()
{
	SelectRegion( (int32)m_pRegionSelect->GetSelectedItem()->pData );
}
