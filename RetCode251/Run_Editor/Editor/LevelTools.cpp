#include "stdafx.h"
#include "LevelTools.h"
#include "MyLevel.h"
#include "UICommon/UIManager.h"
#include "UICommon/UIFactory.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Common/Utf8Util.h"
#include "Editor/Editors/ObjectDataEdit.h"
#include "Game/Physics/HitProxy.h"
#include "Editor/Editors/PrefabEditor.h"


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


class CLevelObjectTool : public CLevelTool
{
public:
	virtual void OnInited() override
	{
		m_bDragged = false;
		m_pPanel0 = GetChildByName<CUIElement>( "0" );
		m_pObjEditor = new CPrefabEditor( true );
		CReference<CUIResource> pEditorResource = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/object_tool_prefab_editor.xml" );
		pEditorResource->GetElement()->Clone( m_pObjEditor, true );
		m_pObjEditor->bVisible = false;
		m_pObjEditor->Replace( GetChildByName<CUIElement>( "obj_data" ) );
		m_onCalcSize.Set( this, &CLevelObjectTool::CalcSize );
		m_pPanel0->GetChildByName<CUIElement>( "auto_size" )->Register( eEvent_Action, &m_onCalcSize );

		m_pFiles = m_pPanel0->GetChildByName<CUIScrollView>( "files" );
		m_pSelectedFile = m_pPanel0->GetChildByName<CUILabel>( "selected" );
		m_pViewport = m_pPanel0->GetChildByName<CUIViewport>( "viewport" );
		m_pViewport->SetLight( false );
		m_pToolsPanel = GetChildByName<CUIElement>( "tools" );
		m_nCurSelectedTool = -1;
		m_pToolResource = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/tool_default.xml" );
		m_nLayerDisplay = 0;
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
					auto pPawnData = pPrefab->GetRoot()->GetStaticDataSafe<CEntity>();
					if( !pPawnData )
						return true;
					auto pToolSet = CLevelEditToolsetMgr::Inst().GetPrefabToolset( pPrefab->GetRoot()->GetClassData() );
					if( !pToolSet || !pToolSet->CheckPrefabTools( pPrefab ) )
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
				if( pPrefabNode && pPrefabNode->GetClassData()->Is( CClassMetaDataMgr::Inst().GetClassData<ILevelObjLayer>() ) )
					m_vecAllObjLayers.push_back( pPrefabNode );
			}
			for( int i = 0; i < m_vecAllObjLayers.size() / 2; i++ )
				swap( m_vecAllObjLayers[i], m_vecAllObjLayers[m_vecAllObjLayers.size() - 1 - i] );
			m_nObjLayer = -1;
			SelectObjLayer( 0 );
			RefreshTools();
		}
		else
		{
			for( int i = 0; i < m_vecAllObjLayers.size() - 1; i++ )
				m_vecAllObjLayers[i]->SetRenderParentAfter( m_vecAllObjLayers.back() );
			m_vecAllObjs.resize( 0 );
			m_vecAllObjLayers.resize( 0 );
			SelectObj( NULL );
			SelectFile( NULL );
			SelectTool( -1 );
			m_vecTools.resize( 0 );
			if( m_pPreview )
			{
				m_pPreview->RemoveThis();
				m_pPreview = NULL;
			}
			ShowObjEdit( false );
			m_mapItems.clear();
			m_pFiles->ClearContent();
		}
	}

	virtual void OnDebugDraw( IRenderSystem* pRenderSystem, class CUIViewport* pViewport ) override;
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override;
	virtual void OnViewportKey( SUIKeyEvent* pEvent );

	void SelectObjLayer( int32 n );
	void SelectFile( CPrefab* pPrefab );
	void SelectObj( CPrefabNode* pObjNode );
	void SelectTool( int32 n );
	void ShowObjEdit( bool bShow );
	CPrefabNode* GetObjLayer() { return m_nObjLayer >= 0 ? m_vecAllObjLayers[m_nObjLayer] : NULL; }
	CPrefabNode* Pick( const CVector2& p );
	CPrefabNode* AddObject( CPrefab* pPrefab, const CVector2& pos );
	void Erase( const CVector2& p, bool bPickFirst );
	void Erase( const CRectangle& rect );
protected:
	void RefreshTools();
	void RefreshLayerDisplay();
	void CalcSize();
	class CItem : public CUIButton
	{
	public:
		static CItem* Create( CLevelObjectTool* pOwner, CUIScrollView* pView, CPrefab* pPrefab )
		{
			static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/object_tool_item.xml" );
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
		CLevelObjectTool* m_pOwner;
		CReference<CPrefab> m_pPrefab;
		TClassTrigger<CItem> m_onSelect;
	};

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
	CReference<CUIElement> m_pToolsPanel;

	CReference<CUIElement> m_pPanel0;
	CReference<CUIScrollView> m_pFiles;
	CReference<CUILabel> m_pSelectedFile;
	CReference<CUIViewport> m_pViewport;

	CReference<CPrefabEditor> m_pObjEditor;

	CReference<CRenderObject2D> m_pPreview;
	CReference<CPrefab> m_pCurSelectedPrefab;
	CReference<CPrefabNode> m_pCurSelectedObj;
	int32 m_nObjLayer;
	int8 m_nLayerDisplay;
	vector<CReference<CPrefabNode> > m_vecAllObjs;
	vector<CReference<CPrefabNode> > m_vecAllObjLayers;
	vector<SLevelEditToolDesc> m_vecTools;
	vector<CReference<CUIElement> > m_vecToolsBtn;
	int32 m_nCurSelectedTool;
	int8 m_nToolType;
	CReference<CUIResource> m_pToolResource;

	bool m_bDragged;
	CReference<CObjectDataEditItem> m_pObjectData;
	TClassTrigger<CLevelObjectTool> m_onCalcSize;
	TClassTrigger1<CLevelObjectTool, int32> m_onObjDataChanged;
};


void CLevelObjectTool::OnDebugDraw( IRenderSystem* pRenderSystem, CUIViewport* pViewport )
{
	if( m_nCurSelectedTool >= 0 )
	{
		auto pTool = m_vecTools[m_nCurSelectedTool].pTool;
		pTool->OnDebugDraw( pViewport, pRenderSystem );
	}
}

bool CLevelObjectTool::OnViewportStartDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	m_bDragged = true;
	if( m_nCurSelectedTool >= 0 )
	{
		auto pTool = m_vecTools[m_nCurSelectedTool].pTool;
		return pTool->OnViewportStartDrag( pViewport, mousePos );
	}
	return true;
}

void CLevelObjectTool::OnViewportDragged( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_bDragged )
	{
		if( m_nCurSelectedTool >= 0 )
		{
			auto pTool = m_vecTools[m_nCurSelectedTool].pTool;
			pTool->OnViewportDragged( pViewport, mousePos );
		}
	}
}

void CLevelObjectTool::OnViewportStopDrag( CUIViewport* pViewport, const CVector2& mousePos )
{
	if( m_bDragged )
	{
		m_bDragged = false;
		if( m_nCurSelectedTool >= 0 )
		{
			auto pTool = m_vecTools[m_nCurSelectedTool].pTool;
			pTool->OnViewportStopDrag( pViewport, mousePos );
		}
	}
}

void CLevelObjectTool::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( m_nCurSelectedTool >= 0 )
	{
		auto pTool = m_vecTools[m_nCurSelectedTool].pTool;
		if( pTool->OnViewportKey( pEvent ) )
			return;
	}

	if( pEvent->bKeyDown )
	{
		if( pEvent->nChar >= '0' && pEvent->nChar <= '9' )
		{
			int32 n = pEvent->nChar > '0' ? pEvent->nChar - '1' : 9;
			if( n < m_vecTools.size() && n != m_nCurSelectedTool )
				SelectTool( n );
		}
		if( pEvent->nChar == VK_ESCAPE )
		{
			if( m_pCurSelectedObj )
				SelectObj( NULL );
			else if( m_pCurSelectedPrefab )
				SelectFile( NULL );
		}
		if( pEvent->nChar == VK_TAB )
		{
			m_nLayerDisplay = !m_nLayerDisplay;
			RefreshLayerDisplay();
		}
		if( pEvent->nChar == 'O' )
		{
			SelectObjLayer( m_nObjLayer - 1 < 0 ? m_vecAllObjLayers.size() - 1 : m_nObjLayer - 1 );
		}
		if( pEvent->nChar == 'P' )
		{
			SelectObjLayer( m_nObjLayer + 1 >= m_vecAllObjLayers.size() ? 0 : m_nObjLayer + 1 );
		}
	}
}

void CLevelObjectTool::SelectObjLayer( int32 n )
{
	if( n == m_nObjLayer )
		return;
	SelectObj( NULL );
	RefreshTools();
	m_nObjLayer = n;
	CPrefabNode* pObjLayer = m_vecAllObjLayers[m_nObjLayer];
	RefreshLayerDisplay();
	m_vecAllObjs.resize( 0 );
	for( auto pChild = pObjLayer->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == pObjLayer->GetRenderObject() )
			continue;
		auto pPrefabNode = static_cast<CPrefabNode*>( pChild );
		if( !pPrefabNode->GetPatchedNode() )
			continue;
		auto pToolSet = CLevelEditToolsetMgr::Inst().GetObjectToolset( pPrefabNode->GetPatchedNode()->GetClassData() );
		if( !pToolSet || !pToolSet->CheckObjectTools( pPrefabNode ) )
			continue;
		m_vecAllObjs.push_back( pPrefabNode );
	}
}

void CLevelObjectTool::SelectFile( CPrefab* pPrefab )
{
	if( pPrefab == m_pCurSelectedPrefab )
		return;
	if( m_pPreview )
	{
		m_pPreview->RemoveThis();
		m_pPreview = NULL;
	}
	if( pPrefab )
		SelectObj( NULL );
	m_pCurSelectedPrefab = pPrefab;
	if( pPrefab )
	{
		auto p = SafeCast<CEntity>( pPrefab->GetRoot()->CreateInstance() );
		m_pPreview = p;
		m_pViewport->GetRoot()->AddChild( p );
		p->OnPreview();
		m_pSelectedFile->SetText( pPrefab->GetName() );
	}
	RefreshTools();
}

void CLevelObjectTool::SelectObj( CPrefabNode* pObjNode )
{
	if( pObjNode == m_pCurSelectedObj )
		return;
	if( m_onObjDataChanged.IsRegistered() )
		m_onObjDataChanged.Unregister();
	m_pObjectData = NULL;
	m_pCurSelectedObj = pObjNode;
	RefreshTools();
}

void CLevelObjectTool::SelectTool( int32 n )
{
	if( m_nCurSelectedTool >= 0 )
	{
		if( m_nToolType == 2 )
			static_cast<CLevelEditObjectTool*>( m_vecTools[m_nCurSelectedTool].pTool )->ToolEnd();
		else if( m_nToolType == 1 )
			static_cast<CLevelEditPrefabTool*>( m_vecTools[m_nCurSelectedTool].pTool )->ToolEnd();
		else
			static_cast<CLevelEditCommonTool*>( m_vecTools[m_nCurSelectedTool].pTool )->ToolEnd();
	}
	m_nCurSelectedTool = n;
	if( n >= 0 )
	{
		if( m_nToolType == 2 )
			static_cast<CLevelEditObjectTool*>( m_vecTools[n].pTool )->ToolBegin( m_pCurSelectedObj );
		else if( m_nToolType == 1 )
			static_cast<CLevelEditPrefabTool*>( m_vecTools[n].pTool )->ToolBegin( m_pCurSelectedPrefab );
		else
			static_cast<CLevelEditCommonTool*>( m_vecTools[n].pTool )->ToolBegin();
	}
}

void CLevelObjectTool::ShowObjEdit( bool bShow )
{
	if( bShow )
	{
		m_pObjEditor->InitQuickNodeEdit( GetRes(), m_pLevelNode, m_pCurSelectedObj, GetView()->GetViewport() );
		m_pObjEditor->SetVisible( true );
		m_pPanel0->SetVisible( false );
	}
	else
	{
		m_pObjEditor->SetVisible( false );
		m_pPanel0->SetVisible( true );
	}
}

CPrefabNode* CLevelObjectTool::Pick( const CVector2& p )
{
	for( int i = 0; i < m_vecAllObjs.size(); i++ )
	{
		CPrefabNode* pNode = m_vecAllObjs[i];
		CEntity* pEntityData = (CEntity*)pNode->GetPatchedNode()->GetObjData();
		auto bound = pEntityData->GetBoundForEditor();
		CMatrix2D mat;
		mat.Transform( pNode->x, pNode->y, pNode->r, pNode->s );
		auto localPos = mat.MulTVector2Pos( p );
		if( bound.Contains( localPos ) && pEntityData->PickInEditor( localPos ) )
			return pNode;
	}
	return NULL;
}

CPrefabNode* CLevelObjectTool::AddObject( CPrefab* pPrefab, const CVector2& pos )
{
	auto pLayer = GetObjLayer();
	CPrefabNode* pNode = new CPrefabNode( GetRes() );
	pNode->SetResource( pPrefab );
	pNode->SetPosition( pos );
	pLayer->AddChild( pNode );
	m_vecAllObjs.resize( m_vecAllObjs.size() + 1 );
	for( int i = m_vecAllObjs.size() - 1; i > 0; i-- )
		m_vecAllObjs[i] = m_vecAllObjs[i - 1];
	m_vecAllObjs[0] = pNode;
	return pNode;
}

void CLevelObjectTool::Erase( const CVector2& p, bool bPickFirst )
{
	auto Func = [=] ( int32 i ) {
		CPrefabNode* pNode = m_vecAllObjs[i];
		CEntity* pEntityData = (CEntity*)pNode->GetPatchedNode()->GetObjData();
		auto bound = pEntityData->GetBoundForEditor();
		CMatrix2D mat;
		mat.Transform( pNode->x, pNode->y, pNode->r, pNode->s );
		auto localPos = mat.MulTVector2Pos( p );
		if( bound.Contains( localPos ) && pEntityData->PickInEditor( localPos ) )
		{
			for( int i1 = i; i1 < m_vecAllObjs.size() - 1; i1++ )
				m_vecAllObjs[i1] = m_vecAllObjs[i1 + 1];
			m_vecAllObjs.resize( m_vecAllObjs.size() - 1 );
			m_pLevelNode->NameSpaceClearNode( pNode );
			pNode->RemoveThis();
			return true;
		}
		return false;
	};

	if( bPickFirst )
	{
		for( int i = 0; i < m_vecAllObjs.size(); i++ )
		{
			if( Func( i ) )
				return;
		}
	}
	else
	{
		for( int i = m_vecAllObjs.size() - 1; i >= 0; i-- )
			Func( i );
	}
}

void CLevelObjectTool::Erase( const CRectangle& rect )
{
	for( int i = m_vecAllObjs.size() - 1; i >= 0; i-- )
	{
		CPrefabNode* pNode = m_vecAllObjs[i];
		CEntity* pEntityData = (CEntity*)pNode->GetPatchedNode()->GetObjData();
		auto bound = pEntityData->GetBoundForEditor();

		SHitProxyPolygon hit1, hit2;
		hit1.vertices[0] = CVector2( rect.x, rect.y );
		hit1.vertices[1] = CVector2( rect.GetRight(), rect.y );
		hit1.vertices[2] = CVector2( rect.GetRight(), rect.GetBottom() );
		hit1.vertices[3] = CVector2( rect.x, rect.GetBottom() );
		hit1.nVertices = 4;
		hit1.CalcNormals();
		hit2.vertices[0] = CVector2( bound.x, bound.y );
		hit2.vertices[1] = CVector2( bound.GetRight(), bound.y );
		hit2.vertices[2] = CVector2( bound.GetRight(), bound.GetBottom() );
		hit2.vertices[3] = CVector2( bound.x, bound.GetBottom() );
		hit2.nVertices = 4;
		hit2.CalcNormals();
		CMatrix2D trans1, trans2;
		trans1.Identity();
		trans2.Transform( pNode->x, pNode->y, pNode->r, pNode->s );
		if( SHitProxy::HitTest( &hit1, &hit2, trans1, trans2 ) )
		{
			for( int i1 = i; i1 < m_vecAllObjs.size() - 1; i1++ )
				m_vecAllObjs[i1] = m_vecAllObjs[i1 + 1];
			m_vecAllObjs.resize( m_vecAllObjs.size() - 1 );
			m_pLevelNode->NameSpaceClearNode( pNode );
			pNode->RemoveThis();
		}
	}
}

void CLevelObjectTool::RefreshTools()
{
	SelectTool( -1 );
	for( auto& p : m_vecToolsBtn )
		p->RemoveThis();
	m_vecToolsBtn.clear();
	m_vecTools.clear();
	if( m_pCurSelectedObj )
	{
		auto pToolset = CLevelEditToolsetMgr::Inst().GetObjectToolset( m_pCurSelectedObj->GetPatchedNode()->GetClassData() );
		pToolset->CreateObjectTools( m_pCurSelectedObj, m_vecTools );
		m_nToolType = 2;
	}
	else if( m_pCurSelectedPrefab )
	{
		auto pToolset = CLevelEditToolsetMgr::Inst().GetPrefabToolset( m_pCurSelectedPrefab->GetRoot()->GetClassData() );
		pToolset->CreatePrefabTools( m_pCurSelectedPrefab, m_vecTools );
		m_nToolType = 1;
	}
	else
	{
		CLevelEditToolsetMgr::Inst().GetDefaultTools( m_vecTools );
		m_nToolType = 0;
	}

	m_vecToolsBtn.resize( m_vecTools.size() );
	for( int i = 0; i < m_vecTools.size(); i++ )
	{
		auto pIcon = static_cast<CUILabel*>( m_pToolResource->GetElement()->Clone() );
		for( int32 k = 0; k < 4; k++ )
		{
			pIcon->AddImage( k, "EditorRes/Drawables/icons.xml", CRectangle( 0, 0, 32, 32 ),
				CRectangle( m_vecTools[i].nIconX, m_vecTools[i].nIconY, 1, 1 ) * 32, 0 );
		}
		auto pHandler = new CFunctionTrigger( [this, i] () { SelectTool( i ); } );
		pHandler->bAutoDelete = true;
		pIcon->Register( CUIElement::eEvent_Action, pHandler );

		m_pToolsPanel->AddChild( pIcon );
		pIcon->SetPosition( CVector2( i * 32, 0 ) );
		m_vecToolsBtn[i] = pIcon;
	}

	if( bVisible && m_vecTools.size() )
		SelectTool( 0 );
}

void CLevelObjectTool::RefreshLayerDisplay()
{
	if( m_nLayerDisplay )
	{
		for( int i = 0; i < m_vecAllObjLayers.size() - 1; i++ )
			m_vecAllObjLayers[i]->SetRenderParentAfter( m_vecAllObjLayers.back() );
	}
	else
	{
		for( int i = 1; i < m_vecAllObjLayers.size(); i++ )
		{
			m_vecAllObjLayers[( i + m_nObjLayer ) % m_vecAllObjLayers.size()]
				->SetRenderParentAfter( m_vecAllObjLayers[m_nObjLayer == -1 ? m_vecAllObjLayers.size() - 1 : m_nObjLayer] );
		}
	}
}

void CLevelObjectTool::CalcSize()
{
	CRectangle bound0( 0, 0, 0, 0 );
	for( CPrefabNode* pObjLayer : m_vecAllObjLayers )
	{
		CEntity* pEntityData = (CEntity*)pObjLayer->GetObjData();
		SafeCastToInterface<ILevelObjLayer>( pEntityData );
		CRectangle rect;
		if( SafeCastToInterface<ILevelObjLayer>( pEntityData )->GetBound( rect ) )
		{
			if( bound0.width <= 0 )
				bound0 = rect;
			else
				bound0 = rect + bound0;
			continue;
		}

		for( auto pChild = pObjLayer->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
		{
			if( pChild == pObjLayer->GetRenderObject() )
				continue;
			auto pPrefabNode = static_cast<CPrefabNode*>( pChild );
			if( !pPrefabNode->GetPatchedNode() )
				continue;
			CEntity* pEntityData = (CEntity*)pPrefabNode->GetPatchedNode()->GetObjData();
			if( !pEntityData->AccountInLevelBound() )
				continue;
			auto bound = pEntityData->GetBoundForEditor();
			bound = bound * pPrefabNode->s;
			bound = bound.Rotate( pPrefabNode->r );
			bound = bound.Offset( pPrefabNode->GetPosition() );
			if( bound0.width <= 0 )
				bound0 = bound;
			else
				bound0 = bound + bound0;
		}
	}
	GetLevelData()->SetSize( bound0 );
	GetView()->RefreshMask();
}

class CLevelBugsTool : public CLevelTool
{
public:
	virtual void OnInited() override
	{
		m_nDragType = 0;
	}
	virtual void OnSetVisible( bool bVisible ) override
	{
		if( bVisible )
		{
			CLevelTool::OnSetVisible( true );
			function<void( CPrefabNode* )> FuncSearch;
			FuncSearch = [this, &FuncSearch] ( CPrefabNode* p ) {
				if( p->GetClassData() && p->GetClassData()->Is( CClassMetaDataMgr::Inst().GetClassData<CBug>() ) )
				{
					p->PatchedNodeForceCalcTransform();
					( (CPrefabNode*)p->GetPatchedNodeOwner() )->OnEditorActive( true );
					m_vecBugList.resize( m_vecBugList.size() + 1 );
					m_vecBugList.back().p = p;
					return;
				}
				if( p->GetPatchedNode() )
					FuncSearch( p->GetPatchedNode() );
				for( auto pChild = p->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
				{
					if( pChild == p->GetRenderObject() )
						continue;
					FuncSearch( static_cast<CPrefabNode*>( pChild ) );
				}
			};
			FuncSearch( m_pLevelNode );
			auto pData = GetLevelData();
			pData->EditorFixBugListLoad( m_vecBugList );
			for( auto& item : m_vecBugList )
			{
				item.p->OnEdit();
				( (CPrefabNode*)item.p->GetPatchedNodeOwner() )->OnEditorActive( false );
			}
			Refresh();
			m_nDragType = 0;
			m_nDraggedBug = -1;
			m_nMode = 0;
		}
		else
		{
			FixData();
			m_vecBugList.clear();
			CLevelTool::OnSetVisible( false );
		}
	}

	virtual void OnDebugDraw( IRenderSystem* pRenderSystem, class CUIViewport* pViewport ) override
	{
		CVector2 ofs[] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		for( auto& item : m_vecBugList )
		{
			auto p = item.p->globalTransform.GetPosition();
			auto color = CBug::GetGroupColor( item.p->GetStaticDataSafe<CBug>()->m_nGroup );
			color.w = 1;

			CVector2 verts[] = { { -1, -1 }, { 1, 1 }, { 1, -1 },
			{ 1, 1 }, { -1, -1 }, { -1, 1 } };
			for( int i = 0; i < ELEM_COUNT( verts ); i++ )
				verts[i] = verts[i] * CVector2( 16, 16 ) + p;
			pViewport->DebugDrawTriangles( pRenderSystem, 6, verts, color );

			if( m_nMode == 0 )
			{
				if( item.par )
				{
					auto p1 = item.par->globalTransform.GetPosition();
					auto d = p1 - p;
					d.Normalize();
					pViewport->DebugDrawLine( pRenderSystem, p + CVector2( d.y, -d.x ) * 8, p1, color );
					pViewport->DebugDrawLine( pRenderSystem, p + CVector2( -d.y, d.x ) * 8, p1, color );
				}
			}
		}
		if( m_nMode == 0 )
		{
			if( m_nDragType )
			{
				auto& item = m_vecBugList[m_nDraggedBug];
				auto p = item.p->globalTransform.GetPosition();
				auto p1 = pViewport->GetScenePos( pViewport->GetMgr()->GetMousePos() );
				auto d = p1 - p;
				d.Normalize();
				pViewport->DebugDrawLine( pRenderSystem, p + CVector2( d.y, -d.x ) * 8, p1, CVector4( 1, 1, 1, 1 ) );
				pViewport->DebugDrawLine( pRenderSystem, p + CVector2( -d.y, d.x ) * 8, p1, CVector4( 1, 1, 1, 1 ) );
			}
		}
		else
			DebugDraw1( pRenderSystem, pViewport );
	}
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override
	{
		if( m_nMode == 0 )
		{
			for( int i = 0; i < m_vecBugList.size(); i++ )
			{
				auto& item = m_vecBugList[i];
				auto p = item.p->globalTransform.GetPosition();
				CRectangle rect( p.x - 16, p.y - 16, 32, 32 );
				if( rect.Contains( mousePos ) )
				{
					m_nDragType = 1;
					m_nDraggedBug = i;
					return true;
				}
			}

			for( int i = 0; i < m_vecBugList.size(); i++ )
			{
				auto& item = m_vecBugList[i];
				if( !item.par )
					continue;
				auto p = item.p->globalTransform.GetPosition();
				auto p1 = item.par->globalTransform.GetPosition();
				float l = ( p1 - p ).Length();
				if( l <= 0 )
					continue;
				float s = p.x * p1.y + p1.x * mousePos.y + mousePos.x * p.y - p.y * p1.x - p1.y * mousePos.x - mousePos.y * p.x;
				if( abs( s ) >= 8 * l )
					continue;
				if( ( p1 - p ).Dot( mousePos - p ) < 0 || ( p - p1 ).Dot( mousePos - p1 ) < 0 )
					continue;
				m_nMode = 1;
				m_nSelectedLink = i;
				m_nDragType = 0;
				return true;
			}
		}
		else
			return ViewportStartDrag1( pViewport, mousePos );
		return false;
	}
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos ) override
	{
		if( !m_nDragType )
			return;
		if( m_nMode == 1 )
			ViewportDragged1( pViewport, mousePos );
	}
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos ) override
	{
		if( !m_nDragType )
			return;
		if( m_nMode == 0 )
		{
			for( int i = 0; i < m_vecBugList.size(); i++ )
			{
				auto& item = m_vecBugList[i];
				auto p = item.p->globalTransform.GetPosition();
				CRectangle rect( p.x - 16, p.y - 16, 32, 32 );
				if( rect.Contains( mousePos ) )
				{
					if( i != m_nDraggedBug && m_vecBugList[m_nDraggedBug].par.GetPtr() != m_vecBugList[i].p )
					{
						m_vecBugList[m_nDraggedBug].par = m_vecBugList[i].p;
						m_vecBugList[m_nDraggedBug].vecPath.resize( 0 );
						FixData();
					}
					m_nDragType = 0;
					return;
				}
			}
			m_vecBugList[m_nDraggedBug].par = NULL;
			FixData();
		}
		else
			ViewportStopDrag1( pViewport, mousePos );
		m_nDragType = 0;
	}
	virtual void OnViewportKey( SUIKeyEvent* pEvent )
	{
		if( m_nDragType )
			return;
		auto mousePos = GetView()->GetViewportMousePos();
		if( m_nMode == 0 )
		{
			int32 nBug = -1;
			for( int i = 0; i < m_vecBugList.size(); i++ )
			{
				auto& item = m_vecBugList[i];
				auto p = item.p->globalTransform.GetPosition();
				CRectangle rect( p.x - 16, p.y - 16, 32, 32 );
				if( rect.Contains( mousePos ) )
				{
					nBug = i;
					break;
				}
			}
			if( nBug < 0 )
				return;
			auto pBugData = m_vecBugList[nBug].p->GetStaticDataSafe<CBug>();
			switch( pEvent->nChar )
			{
			case 'A':
				ChangeGroup( pBugData->m_nGroup, -1 );
				break;
			case 'S':
				ChangeGroup( pBugData->m_nGroup, 1 );
				break;
			}
		}
		else
		{
			switch( pEvent->nChar )
			{
			case VK_ESCAPE:
				m_nMode = 0;
				break;
			}
		}
	}

	void DebugDraw1( IRenderSystem* pRenderSystem, class CUIViewport* pViewport );
	void DebugDraw1Drag( IRenderSystem* pRenderSystem, class CUIViewport* pViewport );
	bool ViewportStartDrag1( class CUIViewport* pViewport, const CVector2& mousePos );
	void ViewportDragged1( class CUIViewport* pViewport, const CVector2& mousePos );
	void ViewportStopDrag1( class CUIViewport* pViewport, const CVector2& mousePos );
	void FixNodes( vector<int32>& vecPath );
	void Refresh() {}
protected:
	void FixData()
	{
		for( auto& item : m_vecBugList )
			( (CPrefabNode*)item.p->GetPatchedNodeOwner() )->OnEditorActive( true );
		auto pData = GetLevelData();
		pData->EditorFixBugListSave( m_vecBugList );
		for( auto& item : m_vecBugList )
		{
			item.p->OnEdit();
			( (CPrefabNode*)item.p->GetPatchedNodeOwner() )->OnEditorActive( false );
		}
	}
	void ChangeGroup( int32 nGroup, int8 nType )
	{
		vector<int32> vecGroup;
		vecGroup.resize( 8 );
		for( auto& item : m_vecBugList )
		{
			auto pBug = item.p->GetStaticDataSafe<CBug>();
			auto nGroup = pBug->m_nGroup;
			vecGroup.resize( Max<int32>( vecGroup.size(), nGroup + 1 ) );
			vecGroup[nGroup] = 1;
		}
		int32 nNewGroup = -1;

		for( int i = 1; i < vecGroup.size(); i++ )
		{
			int32 n = nType == 1 ? ( nGroup + i ) % vecGroup.size() : ( nGroup - i + vecGroup.size() ) % vecGroup.size();
			if( !vecGroup[n] )
			{
				nNewGroup = n;
				break;
			}
		}
		if( nNewGroup == -1 )
			nNewGroup = vecGroup.size() + 1;

		for( auto& item : m_vecBugList )
		{
			auto pBug = (CBug*)item.p->GetStaticDataSafe<CBug>();
			if( pBug->m_nGroup == nGroup )
				pBug->m_nGroup = nNewGroup;
		}
	}

	int8 m_nMode;
	int8 m_nDragType;
	int32 m_nDraggedBug;
	int32 m_nSelectedLink;
	int32 m_nDraggedPath;
	CVector2 m_dragBegin;
	CVector2 m_dragOfs;
	CVector2 m_dragNodePos;
	vector<CMyLevel::SEditorBugListItem> m_vecBugList;
};

void CLevelBugsTool::DebugDraw1( IRenderSystem* pRenderSystem, class CUIViewport* pViewport )
{
	auto& vecPath = m_vecBugList[m_nSelectedLink].vecPath;
	auto p0 = m_vecBugList[m_nSelectedLink].p->globalTransform.GetPosition();
	if( !vecPath.size() )
		vecPath.push_back( 0 );
	int8 nDir = !vecPath[0];
	int8 nDir0 = -1;
	int32 nCount = vecPath.size() - 1;
	float fSize = 32;

	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	CVector2 nodeVerts[4] = { { -fSize * 0.25f, 0 }, { 0, -fSize * 0.25f }, { fSize * 0.25f, 0 }, { 0, fSize * 0.25f } };
	TVector2<int32> p( 0, 0 );
	if( nCount == 0 )
	{
		for( int j = 0; j < ELEM_COUNT( nodeVerts ); j++ )
		{
			auto pt1 = nodeVerts[j] + p0;
			auto pt2 = nodeVerts[( j + 1 ) % 4] + p0;
			pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0, 1, 1, 1 ) );
		}
	}
	else
	{
		for( int i = 1; i <= nCount; i++ )
		{
			nDir = 1 ^ ( nDir & 1 );
			int32 nOfs = vecPath[i];
			if( nOfs < 0 )
			{
				nOfs = -nOfs;
				nDir += 2;
			}
			auto p1 = p + ofs[nDir] * nOfs;
			pViewport->DebugDrawLine( pRenderSystem, p0 + CVector2( p.x, p.y ) * fSize, p0 + CVector2( p1.x, p1.y ) * fSize, CVector4( 0, 1, 1, 1 ) );
			p = p1;

			for( int j = 0; j < ELEM_COUNT( nodeVerts ); j++ )
			{
				auto pt1 = p0 + CVector2( p.x, p.y ) * fSize + nodeVerts[j];
				auto pt2 = p0 + CVector2( p.x, p.y ) * fSize + nodeVerts[( j + 1 ) % 4];
				pViewport->DebugDrawLine( pRenderSystem, pt1, pt2, CVector4( 0, 1, 1, 1 ) );
			}

			nDir0 = nDir;
		}
	}
	if( m_nDragType )
		DebugDraw1Drag( pRenderSystem, pViewport );
}

void CLevelBugsTool::DebugDraw1Drag( IRenderSystem* pRenderSystem, CUIViewport* pViewport )
{
	auto& vecPath = m_vecBugList[m_nSelectedLink].vecPath;
	auto p0 = m_vecBugList[m_nSelectedLink].p->globalTransform.GetPosition();
	int8 nDir = vecPath[0] ^ ( m_nDraggedPath & 1 );
	int8 nDir0 = -1;
	int32 nCount = vecPath.size() - 1;
	float fSize = 32;

	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	TVector2<int32> d( floor( m_dragOfs.x / fSize + 0.5f ), floor( m_dragOfs.y / fSize + 0.5f ) );
	if( m_nDragType == 2 )
	{
		TVector2<int32> ofs0( vecPath[m_nDraggedPath + 1] * -1, 0 );
		if( nDir )
			swap( ofs0.y, ofs0.x );
		if( m_nDraggedPath + 1 < nCount )
		{
			TVector2<int32> ofs1( vecPath[m_nDraggedPath + 2], 0 );
			if( !nDir )
				swap( ofs1.y, ofs1.x );
			auto r = TRectangle<int32>( ofs0.x, ofs0.y, 0, 0 ) + TRectangle<int32>( ofs1.x, ofs1.y, 0, 0 );
			d.x = Min( r.GetRight(), Max( r.x, d.x ) );
			d.y = Min( r.GetBottom(), Max( r.y, d.y ) );
			if( !d.x || !d.y )
				return;
			pViewport->DebugDrawLine( pRenderSystem, p0 + CVector2( d.x, 0 ) * fSize + m_dragNodePos,
				p0 + CVector2( d.x, d.y ) * fSize + m_dragNodePos, CVector4( 1, 1, 0, 1 ) );
			pViewport->DebugDrawLine( pRenderSystem, p0 + CVector2( 0, d.y ) * fSize + m_dragNodePos,
				p0 + CVector2( d.x, d.y ) * fSize + m_dragNodePos, CVector4( 1, 1, 0, 1 ) );
		}
		else
		{
			if( !d.x && !d.y )
				return;
			d = abs( m_dragOfs.x ) > abs( m_dragOfs.y ) ? TVector2<int32>( d.x, 0 ) : TVector2<int32>( 0, d.y );
			pViewport->DebugDrawLine( pRenderSystem, p0 + CVector2( d.x, d.y ) * fSize + m_dragNodePos,
				p0 + m_dragNodePos, CVector4( 1, 1, 0, 1 ) );
		}
	}
	else
	{
		d = abs( m_dragOfs.x ) > abs( m_dragOfs.y ) ? TVector2<int32>( d.x, 0 ) : TVector2<int32>( 0, d.y );
		pViewport->DebugDrawLine( pRenderSystem, p0 + CVector2( d.x, d.y ) * fSize + m_dragNodePos,
			p0 + m_dragNodePos, CVector4( 1, 1, 0, 1 ) );
	}
}

bool CLevelBugsTool::ViewportStartDrag1( CUIViewport * pViewport, const CVector2 & mousePos )
{
	auto& vecPath = m_vecBugList[m_nSelectedLink].vecPath;
	auto p0 = m_vecBugList[m_nSelectedLink].p->globalTransform.GetPosition();
	if( !vecPath.size() )
		vecPath.push_back( 0 );
	int8 nDir = !vecPath[0];
	int8 nDir0 = -1;
	int32 nCount = vecPath.size() - 1;
	float fSize = 32;
	auto dragPos = mousePos - p0;

	TVector2<int32> p( 0, 0 );
	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	if( nCount == 0 )
	{
		if( abs( dragPos.x ) + abs( dragPos.y ) < fSize * 0.25f )
		{
			m_nDragType = 1;
			m_nDraggedPath = 0;
			m_dragBegin = dragPos;
			m_dragOfs = CVector2( 0, 0 );
			m_dragNodePos = CVector2( 0, 0 );
			return true;
		}
	}
	else
	{
		for( int i = 0; i < nCount; i++ )
		{
			nDir = 1 ^ ( nDir & 1 );
			int32 nOfs = vecPath[i + 1];
			if( nOfs < 0 )
			{
				nOfs = -nOfs;
				nDir += 2;
			}
			auto p1 = p + ofs[nDir] * nOfs;
			auto d = CVector2( p1.x, p1.y ) * fSize - dragPos;
			if( abs( d.x ) + abs( d.y ) < fSize * 0.25f )
			{
				m_nDragType = 2;
				m_nDraggedPath = i;
				m_dragBegin = dragPos;
				m_dragOfs = CVector2( 0, 0 );
				m_dragNodePos = CVector2( p1.x, p1.y ) * fSize;
				return true;
			}

			p = p1;
		}
	}
	return false;
}

void CLevelBugsTool::ViewportDragged1( CUIViewport * pViewport, const CVector2 & mousePos )
{
	if( !m_nDragType )
		return;
	auto p0 = m_vecBugList[m_nSelectedLink].p->globalTransform.GetPosition();
	auto dragPos = mousePos - p0;
	m_dragOfs = dragPos - m_dragBegin;
}

void CLevelBugsTool::ViewportStopDrag1( CUIViewport * pViewport, const CVector2 & mousePos )
{
	auto& vecPath = m_vecBugList[m_nSelectedLink].vecPath;
	auto p0 = m_vecBugList[m_nSelectedLink].p->globalTransform.GetPosition();
	if( !vecPath.size() )
		vecPath.push_back( 0 );
	int8 nDir = vecPath[0] ^ ( m_nDraggedPath & 1 );
	int8 nDir0 = -1;
	int32 nCount = vecPath.size() - 1;
	float fSize = 32;
	auto dragPos = mousePos - p0;
	m_dragOfs = dragPos - m_dragBegin;

	TVector2<int32> ofs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	TVector2<int32> d( floor( m_dragOfs.x / fSize + 0.5f ), floor( m_dragOfs.y / fSize + 0.5f ) );
	if( m_nDragType == 2 )
	{
		TVector2<int32> ofs0( vecPath[m_nDraggedPath + 1] * -1, 0 );
		if( nDir )
			swap( ofs0.y, ofs0.x );
		if( m_nDraggedPath + 1 < nCount )
		{
			TVector2<int32> ofs1( vecPath[m_nDraggedPath + 2], 0 );
			if( !nDir )
				swap( ofs1.y, ofs1.x );
			auto r = TRectangle<int32>( ofs0.x, ofs0.y, 0, 0 ) + TRectangle<int32>( ofs1.x, ofs1.y, 0, 0 );
			d.x = Min( r.GetRight(), Max( r.x, d.x ) );
			d.y = Min( r.GetBottom(), Max( r.y, d.y ) );
			if( !d.x || !d.y )
				return;
			int32 nOfs[4];
			if( ofs0.x )
			{
				nOfs[0] = -ofs0.x + d.x;
				nOfs[2] = -d.x;
				nOfs[1] = d.y;
				nOfs[3] = ofs1.y - d.y;
			}
			else
			{
				nOfs[0] = -ofs0.y + d.y;
				nOfs[2] = -d.y;
				nOfs[1] = d.x;
				nOfs[3] = ofs1.x - d.x;
			}
			vecPath.resize( vecPath.size() + 2 );
			for( int i = nCount - 1; i > m_nDraggedPath + 1; i-- )
				vecPath[i + 1 + 2] = vecPath[i + 1];
			for( int i = 0; i < 4; i++ )
				vecPath[m_nDraggedPath + i + 1] = nOfs[i];
		}
		else
		{
			if( !d.x && !d.y )
				return;
			int8 b = abs( m_dragOfs.x ) < abs( m_dragOfs.y );
			if( b == nDir )
				vecPath[nCount] += b ? d.y : d.x;
			else
			{
				vecPath.resize( vecPath.size() + 1 );
				vecPath.back() = b ? d.y : d.x;
			}
		}
		FixNodes( vecPath );
	}
	else
	{
		vecPath.resize( 2 );
		vecPath[0] = abs( m_dragOfs.x ) > abs( m_dragOfs.y ) ? 0 : 1;
		vecPath[1] = vecPath[0] ? d.y : d.x;
	}
}

void CLevelBugsTool::FixNodes( vector<int32>& vecPath )
{
	int32 nCount = vecPath.size() - 1;
	if( !nCount )
		return;
	int32 i0 = 0;
	for( int i = 1; i < nCount; i++ )
	{
		if( vecPath[i0 + 1] == 0 )
		{
			if( i0 )
			{
				i0--;
				vecPath[i0 + 1] += vecPath[i + 1];
			}
			else
			{
				vecPath[0] = !vecPath[0];
				vecPath[i0 + 1] = vecPath[i + 1];
			}
		}
		else
			vecPath[++i0 + 1] = vecPath[i + 1];
	}
	for( ; i0 >= 0 && !vecPath[i0 + 1]; i0-- );
	vecPath.resize( i0 + 1 + 1 );
}


class CLevelEnvTool : public CLevelTool
{
public:
protected:
};

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
	Register( eEvent_Key, &m_onViewportKey );
	m_onViewportChar.Set( this, &CLevelToolsView::OnViewportChar );
	m_pViewport->Register( eEvent_Char, &m_onViewportChar );
	m_onViewportMouseWheel.Set( this, &CLevelToolsView::OnViewportMouseWheel );
	m_pViewport->Register( eEvent_MouseWheel, &m_onViewportMouseWheel );
	auto pOK = GetChildByName<CUIElement>( "OK" );
	m_onOK.Set( this, &CLevelToolsView::OnOK );
	pOK->Register( eEvent_Action, &m_onOK );

	m_pPlaceHolder = new CRenderObject2D;

	auto pPawnTool = new CLevelObjectTool;
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/object_tool.xml" )->GetElement()->Clone( pPawnTool );
	pPawnTool->bVisible = false;
	AddChild( pPawnTool );
	m_vecTools.push_back( pPawnTool );
	auto pLevelBugTool = new CLevelBugsTool;
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/bugs_tool.xml" )->GetElement()->Clone( pLevelBugTool );
	pLevelBugTool->bVisible = false;
	AddChild( pLevelBugTool );
	m_vecTools.push_back( pLevelBugTool );
	auto pLevelEnvTool = new CLevelEnvTool;
	CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/level_tools/env_tool.xml" )->GetElement()->Clone( pLevelEnvTool );
	pLevelEnvTool->bVisible = false;
	AddChild( pLevelEnvTool );
	m_vecTools.push_back( pLevelEnvTool );

	static CDefaultDrawable2D* pDrawable1 = NULL;
	if( !pDrawable1 )
	{
		vector<char> content;
		GetFileContent( content, "EditorRes/Drawables/mask.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDrawable1 = new CDefaultDrawable2D;
		pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "mask" ) );
	}
	m_pMask = new CImage2D( pDrawable1, NULL, CRectangle( -2048, -2048, 4096, 4096 ), CRectangle( 0, 0, 1, 1 ) );
	m_pViewport->GetRoot()->AddChild( m_pMask );
	CVector4 param[2] = { { 0.5f, 0.5f, 0.5f, 0 }, { 0, 0, 0, 0 } };
	static_cast<CImage2D*>( m_pMask.GetPtr() )->SetParam( 2, param, 0, 2, 0, 0, 0, 0 );

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
		//m_pBack->SetPosition( m_pLevelData->displayOfs * -1 );
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
	m_pViewport->GetCamera().SetPosition( 0, 0 );
	m_camOfs = CVector2( 0, 0 );
	RefreshMask();

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

float CLevelToolsView::GetGridSize()
{
	return 32.0f;
}

CPrefabNode* CLevelToolsView::GetCurLayerNode()
{
	return static_cast<CLevelObjectTool*>( m_vecTools[0].GetPtr() )->GetObjLayer();
}

CVector2 CLevelToolsView::GetViewportMousePos()
{
	return m_pViewport->GetScenePos( GetMgr()->GetMousePos() );
}

void CLevelToolsView::OnDebugDraw( IRenderSystem* pRenderSystem )
{
	m_pViewport->DebugDrawLine( pRenderSystem, CVector2( -512, 0 ), CVector2( 512, 0 ), CVector4( 0.2f, 0.2f, 0.2f, 0.2f ), 1 );
	m_pViewport->DebugDrawLine( pRenderSystem, CVector2( 0, -512 ), CVector2( 0, 512 ), CVector4( 0.2f, 0.2f, 0.2f, 0.2f ), 1 );
	auto fGridSize = GetGridSize();
	auto mousePos = GetViewportMousePos();
	int32 x1 = ceil( ( mousePos.x - 256.0f ) / fGridSize );
	int32 x2 = floor( ( mousePos.x + 256.0f ) / fGridSize );
	int32 y1 = ceil( ( mousePos.y - 256.0f ) / fGridSize );
	int32 y2 = floor( ( mousePos.y + 256.0f ) / fGridSize );
	float r = 256.0f;
	for( int i = x1; i <= x2; i++ )
	{
		float x = i * fGridSize;
		float l = r * r - ( x - mousePos.x ) * ( x - mousePos.x );
		if( l <= 0 )
			continue;
		l = sqrt( l );
		m_pViewport->DebugDrawLine( pRenderSystem, CVector2( x, mousePos.y - l ), CVector2( x, mousePos.y + l ), CVector4( 0.2f, 0.2f, 0.2f, 0.2f ), 1 );
	}
	for( int j = y1; j <= y2; j++ )
	{
		float y = j * fGridSize;
		float l = r * r - ( y - mousePos.y ) * ( y - mousePos.y );
		if( l <= 0 )
			continue;
		l = sqrt( l );
		m_pViewport->DebugDrawLine( pRenderSystem, CVector2( mousePos.x - l, y ), CVector2( mousePos.x + l, y ), CVector4( 0.2f, 0.2f, 0.2f, 0.2f ), 1 );
	}

	GetCurTool()->OnDebugDraw( pRenderSystem, m_pViewport );
}

void CLevelToolsView::OnViewportStartDrag( SUIMouseEvent* pEvent )
{
	CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
	if( GetCurTool()->OnViewportStartDrag( m_pViewport, fixOfs ) )
		m_bToolDrag = true;
	else
	{
		m_bToolDrag = false;
		m_startDragPos = pEvent->mousePos;
	}
}

void CLevelToolsView::OnViewportDragged( SUIMouseEvent* pEvent )
{
	if( m_bToolDrag )
	{
		CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
		GetCurTool()->OnViewportDragged( m_pViewport, fixOfs );
	}
	else
	{
		CVector2 dPos = m_pViewport->GetScenePos( pEvent->mousePos ) - m_pViewport->GetScenePos( m_startDragPos );
		m_startDragPos = pEvent->mousePos;
		auto& cam = m_pViewport->GetCamera();
		m_camOfs = m_camOfs - dPos;
		cam.SetPosition( m_camOfs.x, m_camOfs.y );
	}
}

void CLevelToolsView::OnViewportStopDrag( SUIMouseEvent* pEvent )
{
	if( m_bToolDrag )
	{
		m_bToolDrag = false;
		CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
		GetCurTool()->OnViewportStopDrag( m_pViewport, fixOfs );
	}
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

void CLevelToolsView::OnViewportMouseWheel( SUIMouseEvent* pEvent )
{
	m_fScale -= pEvent->nParam * 1.0f / 120;
	static float fScales[] = { 1, 1.5, 2, 3, 4, 6, 8, 12, 16 };
	m_fScale = Max( 0.0f, Min<float>( m_fScale, ELEM_COUNT( fScales ) - 1 ) );
	float fScale = fScales[(int32)floor( m_fScale )];
	auto viewSize = m_pViewport->GetCamera().GetViewport().GetSize();
	m_pViewport->GetCamera().SetSize( viewSize.x * fScale, viewSize.y * fScale );
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
	GetMgr()->EndModal();
	m_FuncOK();
}

CPrefabNode* CLevelToolsView::Pick( const CVector2& p )
{
	return static_cast<CLevelObjectTool*>( m_vecTools[0].GetPtr() )->Pick( p );
}

void CLevelToolsView::SelectObj( CPrefabNode* pObj )
{
	return static_cast<CLevelObjectTool*>( m_vecTools[0].GetPtr() )->SelectObj( pObj );
}

CPrefabNode* CLevelToolsView::AddObject( CPrefab* pPrefab, const CVector2& pos )
{
	return static_cast<CLevelObjectTool*>( m_vecTools[0].GetPtr() )->AddObject( pPrefab, pos );
}

void CLevelToolsView::Erase( const CVector2& p, bool bPickFirst )
{
	static_cast<CLevelObjectTool*>( m_vecTools[0].GetPtr() )->Erase( p, bPickFirst );
}

void CLevelToolsView::Erase( const CRectangle& rect )
{
	static_cast<CLevelObjectTool*>( m_vecTools[0].GetPtr() )->Erase( rect );
}

void CLevelToolsView::ShowObjEdit( bool bShow )
{
	static_cast<CLevelObjectTool*>( m_vecTools[0].GetPtr() )->ShowObjEdit( bShow );
}

void CLevelToolsView::RefreshMask()
{
	auto pObj = (CMyLevel*)m_pLevelNode->GetObjData();
	auto rect = pObj->GetSize();
	rect = CRectangle( rect.x - 256, rect.y - 256, rect.width + 512, rect.height + 512 );
	for( auto& item : m_vecNeightborData )
		rect = rect + item.pLevelNode->GetStaticDataSafe<CMyLevel>()->GetSize().Offset( item.displayOfs );
	static_cast<CImage2D*>( m_pMask.GetPtr() )->SetRect( rect );
}

CPrefab* CLevelToolsView::NewLevelFromTemplate( CPrefab* pTemplate, const char* szFileName, const CRectangle& size, int32 z, bool bCopy )
{
	auto pTemplateData = pTemplate->GetRoot()->GetStaticDataSafe<CMyLevel>();
	if( !pTemplateData )
		return NULL;
	if( !SaveFile( szFileName, NULL, 0 ) )
		return NULL;
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szFileName, true );
	if( bCopy )
	{
		auto pNode = pTemplate->GetRoot()->Clone( pPrefab, &pPrefab->GetRoot()->GetNameSpace() );
		auto pLevelData = SafeCast<CMyLevel>( pNode->GetFinalObjData() );
		pLevelData->m_nLevelZ = z;
		pPrefab->SetNode( pNode );
		return pPrefab;
	}
	auto pLevelNode = new CPrefabNode( pPrefab );
	pPrefab->SetNode( pLevelNode );
	pLevelNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CMyLevel>()->strClassName.c_str() );
	auto pLevelData = SafeCast<CMyLevel>( pLevelNode->GetFinalObjData() );
	pLevelData->SetSize( size );
	pLevelData->m_nLevelZ = z;

	vector<CReference<CPrefabNode> > vecObjLayers;
	for( auto p = pTemplate->GetRoot()->Get_RenderChild(); p; p = p->NextRenderChild() )
	{
		if( p == pTemplate->GetRoot()->GetRenderObject() )
			continue;
		auto pPrefabNode = dynamic_cast<CPrefabNode*>( p );
		if( pPrefabNode && pPrefabNode->GetClassData()->Is( CClassMetaDataMgr::Inst().GetClassData<ILevelObjLayer>() ) )
		{
			auto pNewNode = new CPrefabNode( pPrefab );
			pNewNode->SetName( pPrefabNode->GetName() );
			pNewNode->SetClassName( pPrefabNode->GetClassData()->strClassName.c_str() );
			pNewNode->SetResource( pPrefabNode->GetResource() );
			auto pData = (CEntity*)pNewNode->GetObjData();
			SafeCastToInterface<ILevelObjLayer>( pData )->InitFromTemplate( (CEntity*)pPrefabNode->GetObjData(), size );
			vecObjLayers.push_back( pNewNode );
		}
		else if( pPrefabNode->GetName() == "start" )
		{
			auto pNewNode = new CPrefabNode( pPrefab );
			pNewNode->SetName( pPrefabNode->GetName() );
			pNewNode->SetClassName( pPrefabNode->GetClassData()->strClassName.c_str() );
			pNewNode->SetResource( pPrefabNode->GetResource() );
			vecObjLayers.push_back( pNewNode );
		}
	}
	for( int i = vecObjLayers.size() - 1; i >= 0; i-- )
		pLevelNode->AddChild( vecObjLayers[i] );

	auto pTemplateEnv = pTemplate->GetRoot()->GetChildByName<CPrefabNode>( "env" );
	if( pTemplateEnv && pTemplateEnv->GetClassData() == CClassMetaDataMgr::Inst().GetClassData<CLevelEnvLayer>() )
	{
		auto pTemplateEnvData = (CLevelEnvLayer*)pTemplateEnv->GetObjData();
		auto pEnvNode = new CPrefabNode( pPrefab );
		pEnvNode->SetName( "env" );
		pLevelNode->AddChild( pEnvNode );
		pEnvNode->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CLevelEnvLayer>()->strClassName.c_str() );
		pEnvNode->SetResource( pTemplateEnv->GetResource() );
		auto pEnvData = (CLevelEnvLayer*)pEnvNode->GetObjData();
		pEnvData->m_ctrlPoint1 = pTemplateEnvData->m_ctrlPoint1;
		pEnvData->m_ctrlPoint2 = pTemplateEnvData->m_ctrlPoint1;
		pEnvData->m_arrCtrlPoint = pTemplateEnvData->m_arrCtrlPoint;
		pEnvData->m_arrCtrlLink = pTemplateEnvData->m_arrCtrlLink;
		pEnvData->m_arrCtrlLimitor = pTemplateEnvData->m_arrCtrlLimitor;
		pEnvData->m_nFadeTime = pTemplateEnvData->m_nFadeTime;
	}

	return pPrefab;
}

CRenderObject2D* CLevelToolsView::CreateLevelSimplePreview( CPrefabNode* pNode )
{
	auto p = SafeCast<CMyLevel>( pNode->CreateInstance( false ) );
	p->OnPreview();
	return p;
}

void CLevelToolsView::FixLevelData( CPrefabNode* pNode, CPrefab* pRes )
{
	auto pObj = (CMyLevel*)pNode->GetObjData();

	/*auto p1 = pNode->GetChildByName<CPrefabNode>( "1" );
	if( !p1 )
	{
		p1 = new CPrefabNode( pRes );
		p1->SetName( "1" );
		pNode->AddChild( p1 );
		p1->SetClassName( CClassMetaDataMgr::Inst().GetClassData<CLevelObjLayer>()->strClassName.c_str() );
		p1->OnEditorActive( false );
	}*/
}
