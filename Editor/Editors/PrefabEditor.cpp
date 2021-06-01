#include "stdafx.h"
#include "PrefabEditor.h"
#include "UICommon/UIFactory.h"
#include "Render/DrawableGroup.h"
#include "Render/Rope2D.h"
#include "Render/ParticleSystem.h"
#include "Common/Utf8Util.h"
#include "UICommon/UIManager.h"
#include "Render/Scene2DManager.h"
#include "Common/Rand.h"
#include "Editor.h"

CDragDropObjRef* CDragDropObjRef::Create( CPrefabNode* pNode )
{
	static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/dragdropobjref.xml" );
	auto pElem = new CDragDropObjRef;
	g_pRes->GetElement()->Clone( pElem );
	pElem->SetText( pNode->GetName() );
	pElem->m_pNode = pNode;
	return pElem;
}


class CPrefabListItem : public CUIButton
{
	friend class CPrefabEditor;
public:
	static void Create( CPrefabEditor* pPrefabView, CUIScrollView* pView, const char* szName )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/fileselect_item.xml" );
		auto pItem = new CPrefabListItem;
		g_pRes->GetElement()->Clone( pItem );
		pView->AddContent( pItem );
		pItem->m_pPrefabView = pPrefabView;
		pItem->m_name = szName;
		pItem->SetText( szName && szName[0] ? szName : "---ROOT---" );
	}

protected:
	virtual void OnInited() override
	{
		m_onSelect.Set( this, &CPrefabListItem::OnSelect );
		Register( eEvent_Action, &m_onSelect );
	}
	void OnSelect()
	{
		m_pPrefabView->SelectItem( m_name.c_str() );
	}
private:
	CPrefabEditor* m_pPrefabView;
	string m_name;
	TClassTrigger<CPrefabListItem> m_onSelect;
};


class CPrefabNodeTreeFolder : public CTreeFolder
{
	friend class CPrefabEditor;
public:
	static CUITreeView::CTreeViewContent* Create( CPrefabEditor* pView, CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, CPrefabNode* pNode, int8 nInsertType = 0 )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/prefabnode_treefolder.xml" );
		auto pTreeFolder = new CPrefabNodeTreeFolder;
		g_pRes->GetElement()->Clone( pTreeFolder );
		CUITreeView::CTreeViewContent* pContent;
		if( pParent )
		{
			if( nInsertType == 0 )
				pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pTreeFolder, pParent ) );
			else if( nInsertType == 1 )
				pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pTreeFolder, pParent, true ) );
			else
				pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentSibling( pTreeFolder, pParent ) );
		}
		else
			pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContent( pTreeFolder ) );
		pTreeFolder->m_nType = pNode->IsInstance() ? 1 : 0;
		pTreeFolder->m_pTreeView = pTreeView;
		pTreeFolder->m_pContent = pContent;
		auto pLabel = pTreeFolder->GetChildByName<CUIButton>( "label" );
		if( pTreeFolder->m_nType )
		{
			string str = "# ";
			str += pNode->GetName().c_str();
			pLabel->SetText( str.c_str() );
			pLabel->SetTextColor( CVector4( 0.5f, 0.7f, 1, 1 ) );
		}
		else
			pLabel->SetText( pNode->GetName() );
		pTreeFolder->m_pView = pView;
		pTreeFolder->m_pNode = pNode;
		pNode->OnEditorActive( false );
		pContent->fChildrenIndent = 4;
		return pContent;
	}
protected:
	virtual void OnInited() override
	{
		CTreeFolder::OnInited();
		m_onSelect.Set( this, &CPrefabNodeTreeFolder::OnSelect );
		m_onStartDrag.Set( this, &CPrefabNodeTreeFolder::OnLabelStartDrag );
		m_onDragged.Set( this, &CPrefabNodeTreeFolder::OnLabelDragged );
		auto p = GetChildByName<CUIButton>( "label" );
		p->Register( eEvent_Action, &m_onSelect );
		p->Register( eEvent_StartDrag, &m_onStartDrag );
		p->Register( eEvent_Dragged, &m_onDragged );
	}
	void OnSelect()
	{
		m_pView->SelectNode( m_pNode, m_pContent );
		GetMgr()->SetFocus( this );
	}
	void OnLabelStartDrag( SUIMouseEvent* mouseEvent )
	{
		CVector2& mousePos = mouseEvent->mousePos;
		m_dragBegin = mousePos;
	}
	void OnLabelDragged( SUIMouseEvent* mouseEvent )
	{
		CVector2& mousePos = mouseEvent->mousePos;
		if( ( mousePos - m_dragBegin ).Length2() >= 16 * 16 )
		{
			auto p = CDragDropObjRef::Create( m_pNode );
			GetMgr()->DragDrop( p );
		}
	}
	virtual void OnChar( uint32 nChar ) override
	{
		if( m_nType )
			return;
		switch( nChar )
		{
		case 'Q':
		case 'q':
			m_pView->ShowChildren( m_pNode, m_pContent );
			break;
		case 'W':
		case 'w':
			m_pView->MoveNodeUp( m_pNode, m_pContent );
			break;
		case 'S':
		case 's':
			m_pView->MoveNodeDown( m_pNode, m_pContent );
			break;
		case 'A':
		case 'a':
			m_pView->MoveNodeLeft( m_pNode, m_pContent );
			break;
		case 'D':
		case 'd':
			m_pView->MoveNodeRight( m_pNode, m_pContent );
			break;
		default:
			break;
		}
	}

	CPrefabEditor* m_pView;
	CPrefabNode* m_pNode;
	int8 m_nType;
	TClassTrigger<CPrefabNodeTreeFolder> m_onSelect;
	TClassTrigger1<CPrefabNodeTreeFolder, SUIMouseEvent*> m_onStartDrag;
	TClassTrigger1<CPrefabNodeTreeFolder, SUIMouseEvent*> m_onDragged;
	CVector2 m_dragBegin;
};

void CPrefabEditor::NewFile( const char* szFileName )
{
	SelectNode( NULL, NULL );
	m_pSceneView->ClearContent();
	m_pItemView->ClearContent();
	if( m_pSelectedPrefab )
	{
		m_pSelectedPrefab->RemoveThis();
		m_pSelectedPrefab = NULL;
	}
	m_mapClonedPrefabs.clear();

	m_pRes = CResourceManager::Inst()->CreateResource<CPrefab>( szFileName, true );
	auto pNode = new CPrefabNode( m_pRes );
	pNode->GetNameSpace().pNameSpaceKey = m_pRes->GetNameSpaceKey();
	m_mapClonedPrefabs[""] = pNode;
	RefreshItemView();
	SelectItem( "" );
	Save();
}

void CPrefabEditor::Refresh()
{
	SelectNode( NULL, NULL );
	m_pSceneView->ClearContent();
	m_pItemView->ClearContent();
	if( m_pSelectedPrefab )
	{
		m_pSelectedPrefab->RemoveThis();
		m_pSelectedPrefab = NULL;
	}
	m_mapClonedPrefabs.clear();

	if( m_pRes )
	{
		CPrefabNode* pNode = m_pRes->GetRoot()->Clone( m_pRes );
		m_mapClonedPrefabs[""] = pNode;
		auto& extraNodes = m_pRes->GetAllExtraNodes();
		for( auto& item : extraNodes )
			m_mapClonedPrefabs[item.first] = item.second->Clone( m_pRes );
		RefreshItemView();
		SelectItem( "" );
	}
}

CPrefabNode* CPrefabEditor::GetRootNode( const char* szName )
{
	auto itr = m_mapClonedPrefabs.find( szName );
	ASSERT( itr != m_mapClonedPrefabs.end() );
	return itr->second;
}

void CPrefabEditor::RefreshItemView()
{
	m_pItemView->ClearContent();
	CPrefabListItem::Create( this, m_pItemView, "" );
	for( auto& item : m_mapClonedPrefabs )
	{
		if( item.first != "" )
			CPrefabListItem::Create( this, m_pItemView, item.first );
	}
}

void CPrefabEditor::RefreshSceneView( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pParNodeItem, int8 nInsertType )
{
	auto pContent = CPrefabNodeTreeFolder::Create( this, m_pSceneView, pParNodeItem, pNode, nInsertType );
	auto pPatchedNode = pNode->GetPatchedNode();
	vector<CPrefabNode*> vecChildren;
	for( CRenderObject2D* pChild = pNode->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == pNode->m_pRenderObject )
		{
			if( !pPatchedNode )
				continue;
			vecChildren.push_back( pPatchedNode );
		}
		else
		{
			CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
			if( pNode )
				vecChildren.push_back( pNode );
		}
	}
	for( int i = vecChildren.size() - 1; i >= 0; i-- )
	{
		RefreshSceneView( vecChildren[i], pContent );
	}
}

void CPrefabEditor::OnInited()
{
	Super::OnInited();
	m_nNodeDebugDrawType = 1;
	m_pSceneView = GetChildByName<CUITreeView>( "scene_view" );
	m_pItemView = m_pSceneView->GetChildByName<CUITreeView>( "item_view" );
	m_pNodeView = GetChildByName<CUITreeView>( "node_view" );
	m_pItemName = m_pItemView->GetChildByName<CUITextBox>( "new_itemname" );

	m_pNodeName = CCommonEdit::Create( "Name" );
	m_pNodeView->AddContent( m_pNodeName );
	m_pResFileName = CFileNameEdit::Create( "Resource", "mtl;pts;pf" );
	m_pNodeView->AddContent( m_pResFileName );
	m_pNodeClass = CDropDownBox::CreateClassSelectBox<CPrefabBaseNode>( "Class" );
	m_pNodeView->AddContent( m_pNodeClass );
	m_pTransform = CVectorEdit::Create( "Transform", 4 );
	m_pNodeView->AddContent( m_pTransform );
	m_pZOrder = CCommonEdit::Create( "ZOrder" );
	m_pNodeView->AddContent( m_pZOrder );
	
	m_onNodeNameChanged.Set( this, &CPrefabEditor::OnCurNodeNameChanged );
	m_pNodeName->Register( eEvent_Action, &m_onNodeNameChanged );
	m_onNodeResourceChanged.Set( this, &CPrefabEditor::OnCurNodeResourceChanged );
	m_pResFileName->Register( eEvent_Action, &m_onNodeResourceChanged );
	m_onNodeClassChanged.Set( this, &CPrefabEditor::OnCurNodeClassChanged );
	m_pNodeClass->Register( eEvent_Action, &m_onNodeClassChanged );
	m_onNodeObjDataChanged.Set( this, &CPrefabEditor::OnCurNodeObjDataChanged );
	m_onTransformChanged.Set( this, &CPrefabEditor::OnTransformChanged );
	m_pTransform->Register( eEvent_Action, &m_onTransformChanged );
	m_onZOrderChanged.Set( this, &CPrefabEditor::OnZOrderChanged );
	m_pZOrder->Register( eEvent_Action, &m_onZOrderChanged );
	
	m_onNew.Set( this, &CPrefabEditor::NewNode );
	m_pSceneView->GetChildByName( "new" )->Register( eEvent_Action, &m_onNew );
	m_onDelete.Set( this, &CPrefabEditor::DeleteNode );
	m_pSceneView->GetChildByName( "delete" )->Register( eEvent_Action, &m_onDelete );
	m_onSave.Set( this, &CPrefabEditor::Save );
	m_pNodeView->GetChildByName( "save" )->Register( eEvent_Action, &m_onSave );
	m_onCopy.Set( this, &CPrefabEditor::Copy );
	m_pSceneView->GetChildByName( "copy" )->Register( eEvent_Action, &m_onCopy );
	m_onPaste.Set( this, &CPrefabEditor::Paste );
	m_pSceneView->GetChildByName( "paste" )->Register( eEvent_Action, &m_onPaste );

	m_onNewItem.Set( this, &CPrefabEditor::NewItem );
	m_pItemView->GetChildByName( "new" )->Register( eEvent_Action, &m_onNewItem );
	m_onCloneItem.Set( this, &CPrefabEditor::CloneItem );
	m_pItemView->GetChildByName( "clone" )->Register( eEvent_Action, &m_onCloneItem );
	m_onRenameItem.Set( this, &CPrefabEditor::RenameItem );
	m_pItemView->GetChildByName( "rename" )->Register( eEvent_Action, &m_onRenameItem );
	m_onDeleteItem.Set( this, &CPrefabEditor::DeleteItem );
	m_pItemView->GetChildByName( "delete" )->Register( eEvent_Action, &m_onDeleteItem );
	
	static CDropDownBox::SItem g_fileTypeItems[] = 
	{
		{ "Resource", (void*)0 },
		{ "Directional Light", (void*)1 },
		{ "Point Light", (void*)2 },
	};
	m_pNewNodeType = CDropDownBox::Create( g_fileTypeItems, ELEM_COUNT( g_fileTypeItems ) );
	m_pNewNodeType->Replace( m_pSceneView->GetChildByName( "new_nodetype" ) );

	static CDefaultDrawable2D* pDrawable = NULL;
	if( !pDrawable )
	{
		vector<char> content;
		GetFileContent( content, "EditorRes/Drawables/transform.xml", true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDrawable = new CDefaultDrawable2D;
		pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "gui_pass" ) );
	}
	CImage2D* pImage2D = new CImage2D( pDrawable, NULL, CRectangle( -32, -32, 256, 256 ), CRectangle( 0, 0, 1, 1 ), true );
	m_pGizmo = pImage2D;
	pImage2D->bVisible = false;
	if( !IsLighted() )
		pImage2D->SetColorDrawable( pImage2D->GetGUIDrawable() );
	m_pViewport->GetRoot()->AddChild( pImage2D );
}

void CPrefabEditor::RefreshPreview()
{
	if( !m_pRes )
		return;
	for( auto& item : m_mapClonedPrefabs )
		item.second->FixNameSpace();
	m_pRes->RefreshBegin();
	m_pRes->ClearExtraNode();
	for( auto& item : m_mapClonedPrefabs )
		m_pRes->SetNode( item.second->Clone( m_pRes ), item.first.c_str() );
	m_pRes->RefreshEnd();
}

void CPrefabEditor::OnDebugDraw( IRenderSystem* pRenderSystem )
{
	if( m_pSelectedPrefab )
		m_pSelectedPrefab->UpdatePreview();
	if( m_nNodeDebugDrawType == 1 )
	{
		if( m_pCurNode )
			m_pCurNode->DebugDrawPreview( m_pViewport, pRenderSystem );
	}
	else if( m_nNodeDebugDrawType == 2 )
	{
		if( m_pSelectedPrefab )
			m_pSelectedPrefab->DebugDraw( m_pViewport, pRenderSystem );
	}
	if( m_pCurNode )
	{
		if( m_pNodeData )
			m_pNodeData->OnDebugDraw( m_pViewport, pRenderSystem, m_pCurNode->globalTransform );
		if( m_pObjectData )
			m_pObjectData->OnDebugDraw( m_pViewport, pRenderSystem, m_pCurNode->globalTransform );
	}
}

void CPrefabEditor::NewNode()
{
	if( !m_pCurNode )
		return;
	CPrefabNode* pNode = new CPrefabNode( m_pRes );
	pNode->m_nType = (uint8)m_pNewNodeType->GetSelectedItem()->pData;
	switch( pNode->m_nType )
	{
	case 1:
		pNode->SetRenderObject( new CDirectionalLightObject() );
		break;
	case 2:
		pNode->SetRenderObject( new CPointLightObject() );
		break;
	default:
		break;
	}
	m_pCurNode->AddChild( pNode );
	auto pContent = CPrefabNodeTreeFolder::Create( this, m_pSceneView, m_pCurNodeItem, pNode );
	SelectNode( pNode, pContent );
	pNode->OnEditorMove( m_pSelectedPrefab );
}

void CPrefabEditor::DeleteNode()
{
	CPrefabNode* pDeletedNode = m_pCurNode;
	if( !pDeletedNode || pDeletedNode->IsInstance() || pDeletedNode == m_pSelectedPrefab )
		return;
	CUITreeView::CTreeViewContent* pDeletedNodeItem = m_pCurNodeItem;
	CPrefabNode* pSelectedNode = static_cast<CPrefabNode*>( pDeletedNode->GetParent() );
	CUITreeView::CTreeViewContent* pSelectedNodeItem = m_pCurNodeItem->pParent;

	SelectNode( pSelectedNode, pSelectedNodeItem );
	m_pSceneView->RemoveContentTree( pDeletedNodeItem );
	m_pSelectedPrefab->NameSpaceClearNode( pDeletedNode );
	pDeletedNode->OnEditorMove( m_pSelectedPrefab );
	pDeletedNode->RemoveThis();
}

void CPrefabEditor::Copy()
{
	if( !m_pCurNode )
		return;

	m_pClipBoard = m_pCurNode->Clone( nullptr );
}

void CPrefabEditor::Paste()
{
	if( !m_pCurNode || !m_pClipBoard )
		return;
	CPrefabNode* pNode = m_pClipBoard->Clone( m_pRes.GetPtr(), &m_pRes->GetRoot()->GetNameSpace() );
	m_pCurNode->AddChild( pNode );

	SelectNode( NULL, NULL );
	m_pSceneView->ClearContent();
	RefreshSceneView( m_pSelectedPrefab, NULL );
	SelectNode( m_pSelectedPrefab, static_cast<CUITreeView::CTreeViewContent*>( m_pSceneView->Get_Content() ) );
	pNode->OnEditorMove( m_pSelectedPrefab );
}

void CPrefabEditor::NewItem()
{
	auto str = UnicodeToUtf8( m_pItemName->GetText() );
	auto itr = m_mapClonedPrefabs.find( str.c_str() );
	if( itr != m_mapClonedPrefabs.end() )
		return;

	auto pNode = new CPrefabNode( m_pRes );
	pNode->GetNameSpace().pNameSpaceKey = m_pRes->GetNameSpaceKey();
	m_mapClonedPrefabs[str.c_str()] = pNode;
	RefreshItemView();
	SelectItem( str.c_str() );
}

void CPrefabEditor::CloneItem()
{
	auto str = UnicodeToUtf8( m_pItemName->GetText() );
	auto itr = m_mapClonedPrefabs.find( str.c_str() );
	if( itr != m_mapClonedPrefabs.end() )
		return;

	auto pNode = m_pClipBoard->Clone( m_pRes.GetPtr(), NULL, NULL, NULL, true );
	if( !pNode )
	{
		pNode = new CPrefabNode( m_pRes );
		pNode->GetNameSpace().pNameSpaceKey = m_pRes->GetNameSpaceKey();
	}
	m_mapClonedPrefabs[str.c_str()] = pNode;
	RefreshItemView();
	SelectItem( str.c_str() );
}

void CPrefabEditor::RenameItem()
{
	if( m_strSelectedPrefab == "" )
		return;
	auto str = UnicodeToUtf8( m_pItemName->GetText() );
	auto itr = m_mapClonedPrefabs.find( str.c_str() );
	if( itr != m_mapClonedPrefabs.end() )
		return;

	auto pNode = m_pSelectedPrefab;
	m_mapClonedPrefabs.erase( m_strSelectedPrefab );
	m_mapClonedPrefabs[str.c_str()] = pNode;
	m_strSelectedPrefab = str.c_str();
	RefreshItemView();
}

void CPrefabEditor::DeleteItem()
{
	if( m_strSelectedPrefab == "" )
		return;
	m_mapClonedPrefabs.erase( m_strSelectedPrefab );
	RefreshItemView();
	SelectItem( "" );
}

void CPrefabEditor::OnTransformChanged()
{
	CVector4 vec = m_pTransform->GetFloat4();
	if( m_pCurNode )
	{
		m_pCurNode->x = vec.x;
		m_pCurNode->y = vec.y;
		m_pCurNode->r = vec.z / 180 * PI;
		m_pCurNode->s = vec.w;
		m_pCurNode->SetTransformDirty();
	}
	RefreshGizmo();
}

void CPrefabEditor::OnZOrderChanged()
{
	int32 nZOrder = m_pZOrder->GetValue<int32>();

	if( m_pCurNode.GetPtr() != m_pSelectedPrefab.GetPtr() )
	{
		CPrefabNode* pNode = static_cast<CPrefabNode*>( m_pCurNode->GetParent() );

		bool bFind = false;
		uint32 nIndex = 0;
		for( auto pChild = pNode->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
		{
			if( pChild == pNode->m_pRenderObject )
				continue;

			if( bFind )
				nIndex++;
			else if( pChild == m_pCurNode )
				bFind = true;
		}

		m_pCurNode->SetZOrder( nZOrder );
		if( pNode->m_pRenderObject )
		{
			pNode->m_pRenderObject->SetZOrder( -1 );
			pNode->m_pRenderObject->SetZOrder( 0 );
		}

		bFind = false;
		uint32 nIndex1 = 0;
		for( auto pChild = pNode->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
		{
			if( pChild == pNode->m_pRenderObject )
				continue;

			if( bFind )
				nIndex1++;
			else if( pChild == m_pCurNode )
				bFind = true;
		}

		if( nIndex != nIndex1 )
			m_pSceneView->MoveChild( m_pCurNodeItem, nIndex, nIndex1 );
	}
	else
		m_pCurNode->SetZOrder( nZOrder );
}

class CDrawableGroupNodeData : public CPrefabEditor::CNodeData
{
public:
	CDrawableGroupNodeData( CUITreeView* pView, CPrefabEditor* pOwner, CPrefabNode* pNode )
		: CPrefabEditor::CNodeData( pOwner, pNode )
	{
		m_pView = pView;
		m_pResRoot = CTreeFolder::Create( pView, NULL, "Material Inst Data" );
		m_pRect = CVectorEdit::Create( "Rect", 4 );
		pView->AddContentChild( m_pRect, m_pResRoot );
		m_pTexRect = CVectorEdit::Create( "Tex Rect", 4 );
		pView->AddContentChild( m_pTexRect, m_pResRoot );
		CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( pNode->GetResource() );
		uint16 nCount = pDrawableGroup->GetParamCount();
		m_vecParams.resize( nCount );
		m_onRefresh.resize( nCount + 2 );

		for( auto& item : m_onRefresh )
		{
			item.Set( this, &CDrawableGroupNodeData::Refresh );
		}
		m_pRect->Register( CUIElement::eEvent_Action, &m_onRefresh[0] );
		m_pTexRect->Register( CUIElement::eEvent_Action, &m_onRefresh[1] );
		for( int i = 0; i < nCount; i++ )
		{
			char buf[64];
			sprintf( buf, "%d:", i );
			CVectorEdit* pEdit = CVectorEdit::Create( buf, 4 );
			pEdit->Register( CUIElement::eEvent_Action, &m_onRefresh[i + 2] );
			m_vecParams[i] = static_cast<CUITreeView::CTreeViewContent*>( pView->AddContentChild( pEdit, m_pResRoot ) );
		}

		RefreshData();
	}
	~CDrawableGroupNodeData()
	{
		m_pView->RemoveContentTree( m_pResRoot );
		for( auto& item : m_onRefresh )
		{
			if( item.IsRegistered() )
				item.Unregister();
		}
	}
protected:
	void Refresh()
	{
		CImage2D* pImage = static_cast<CImage2D*>( m_pNode->GetRenderObject() );
		CVector4 rect = m_pRect->GetFloat4();
		pImage->SetRect( CRectangle( rect.x, rect.y, rect.z, rect.w ) );
		pImage->SetBoundDirty();
		CVector4 texRect = m_pTexRect->GetFloat4();
		pImage->SetTexRect( CRectangle( texRect.x, texRect.y, texRect.z, texRect.w ) );
		
		CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( m_pNode->GetResource() );
		uint16 nCount = pDrawableGroup->GetParamCount();
		CVector4* pParams = pImage->GetParam( nCount );
		for( int i = 0; i < nCount; i++ )
		{
			pParams[i] = static_cast<CVectorEdit*>( m_vecParams[i]->pElement.GetPtr() )->GetFloat4();
		}
	}

	void RefreshData()
	{
		CImage2D* pImage = static_cast<CImage2D*>( m_pNode->GetRenderObject() );
		CRectangle rect = pImage->GetElem().rect;
		m_pRect->SetFloats( &rect.x );
		CRectangle texRect = pImage->GetElem().texRect;
		m_pTexRect->SetFloats( &texRect.x );
		
		CDrawableGroup* pDrawableGroup = static_cast<CDrawableGroup*>( m_pNode->GetResource() );
		uint16 nCount = pDrawableGroup->GetParamCount();
		CVector4* pParams = pImage->GetParam( nCount );
		for( int i = 0; i < nCount; i++ )
		{
			static_cast<CVectorEdit*>( m_vecParams[i]->pElement.GetPtr() )->SetFloats( &pParams[i].x );
		}
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	CReference<CVectorEdit> m_pRect;
	CReference<CVectorEdit> m_pTexRect;
	vector<CReference<CUITreeView::CTreeViewContent> > m_vecParams;

	vector<TClassTrigger<CDrawableGroupNodeData> > m_onRefresh;
};

class CRopeNodeData : public CPrefabEditor::CNodeData
{
public:
	struct SDataItem
	{
		CUITreeView* m_pView;
		CReference<CUITreeView::CTreeViewContent> m_pRoot;
		CRopeObject2D* m_pObject;
		uint32 m_nIndex;
		
		CReference<CVectorEdit> m_pCenter;
		CReference<CCommonEdit> m_pWidth;
		CReference<CVectorEdit> m_pTex0;
		CReference<CVectorEdit> m_pTex1;
		TClassTrigger<SDataItem> m_onRefresh[4];

		SDataItem( CUITreeView* pView, CUITreeView::CTreeViewContent* pParent, CRopeObject2D* pObject, uint32 nIndex )
		{
			m_pView = pView;
			m_pObject = pObject;
			m_nIndex = nIndex;
			char buf[32];
			sprintf( buf, "%d", nIndex );
			m_pRoot = CTreeFolder::Create( pView, pParent, buf );
			
			m_pCenter = CVectorEdit::Create( "Center", 2 );
			pView->AddContentChild( m_pCenter, m_pRoot );
			m_pWidth = CCommonEdit::Create( "Width" );
			pView->AddContentChild( m_pWidth, m_pRoot );
			m_pTex0 = CVectorEdit::Create( "Tex 0", 2 );
			pView->AddContentChild( m_pTex0, m_pRoot );
			m_pTex1 = CVectorEdit::Create( "Tex 1", 2 );
			pView->AddContentChild( m_pTex1, m_pRoot );

			for( int i = 0; i < ELEM_COUNT( m_onRefresh ); i++ )
				m_onRefresh[i].Set( this, &SDataItem::Refresh );
			m_pCenter->Register( CUIElement::eEvent_Action, m_onRefresh );
			m_pWidth->Register( CUIElement::eEvent_Action, m_onRefresh + 1 );
			m_pTex0->Register( CUIElement::eEvent_Action, m_onRefresh + 2 );
			m_pTex1->Register( CUIElement::eEvent_Action, m_onRefresh + 3 );
			RefreshData();
		}
		~SDataItem()
		{
			for( int i = 0; i < ELEM_COUNT( m_onRefresh ); i++ )
			{
				if( m_onRefresh[i].IsRegistered() )
					m_onRefresh[i].Unregister();
			}
			m_pView->RemoveContentTree( m_pRoot );
		}

		void Refresh()
		{
			m_pObject->SetData( m_nIndex, m_pCenter->GetFloat2(), m_pWidth->GetValue<float>(), m_pTex0->GetFloat2(), m_pTex1->GetFloat2() );
			m_pObject->SetTransformDirty();
		}

		void RefreshData()
		{
			auto& data = m_pObject->GetData().data[m_nIndex];
			m_pCenter->SetFloats( &data.center.x );
			m_pWidth->SetValue( data.fWidth );
			m_pTex0->SetFloats( &data.tex0.x );
			m_pTex1->SetFloats( &data.tex1.x );
		}
	};

	CRopeNodeData( CUITreeView* pView, CPrefabEditor* pOwner, CPrefabNode* pNode )
		: CPrefabEditor::CNodeData( pOwner, pNode )
	{
		m_pView = pView;
		m_pResRoot = CTreeFolder::Create( pView, NULL, "Material Inst Data" );
		m_pDataCount = CCommonEdit::Create( "Data Count" );
		pView->AddContentChild( m_pDataCount, m_pResRoot );
		m_onDataCountChanged.Set( this, &CRopeNodeData::OnDataCountChanged );
		m_pDataCount->Register( CUIElement::eEvent_Action, &m_onDataCountChanged );
		m_pSegmentsPerData = CCommonEdit::Create( "Segments Per Data" );
		pView->AddContentChild( m_pSegmentsPerData, m_pResRoot );
		m_onSegmentsPerDataChanged.Set( this, &CRopeNodeData::OnSegmentsPerDataChanged );
		m_pSegmentsPerData->Register( CUIElement::eEvent_Action, &m_onSegmentsPerDataChanged );

		RefreshData();
	}
	~CRopeNodeData()
	{
		for( int i = 0; i < m_items.size(); i++ )
			delete m_items[i];
		m_items.clear();
		if( m_onDataCountChanged.IsRegistered() )
			m_onDataCountChanged.Unregister();
		m_pView->RemoveContentTree( m_pResRoot );
	}
protected:
	void RefreshData()
	{
		for( int i = 0; i < m_items.size(); i++ )
			delete m_items[i];
		m_items.clear();

		CRopeObject2D* pRopeObject = static_cast<CRopeObject2D*>( m_pNode->GetRenderObject() );
		uint32 nDataCount = pRopeObject->GetData().data.size();
		m_pDataCount->SetValue( nDataCount );
		m_pSegmentsPerData->SetValue( pRopeObject->GetData().nSegmentsPerData );

		m_items.resize( nDataCount );
		for( int i = 0; i < nDataCount; i++ )
		{
			m_items[i] = new SDataItem( m_pView, m_pResRoot, pRopeObject, i );
		}
	}

	void OnDataCountChanged()
	{
		CRopeObject2D* pRopeObject = static_cast<CRopeObject2D*>( m_pNode->GetRenderObject() );
		uint32 nPreDataCount = m_items.size();
		uint32 nDataCount = m_pDataCount->GetValue<uint32>();
		pRopeObject->SetDataCount( nDataCount );

		for( int i = nDataCount; i < nPreDataCount; i++ )
		{
			delete m_items[i];
		}
		m_items.resize( nDataCount );
		for( int i = nPreDataCount; i < nDataCount; i++ )
		{
			m_items[i] = new SDataItem( m_pView, m_pResRoot, pRopeObject, i );
		}
		pRopeObject->SetTransformDirty();
	}
	void OnSegmentsPerDataChanged()
	{
		CRopeObject2D* pRopeObject = static_cast<CRopeObject2D*>( m_pNode->GetRenderObject() );
		pRopeObject->SetSegmentsPerData( m_pSegmentsPerData->GetValue<uint32>() );
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	CReference<CCommonEdit> m_pDataCount;
	CReference<CCommonEdit> m_pSegmentsPerData;
	vector<SDataItem*> m_items;

	TClassTrigger<CRopeNodeData> m_onDataCountChanged;
	TClassTrigger<CRopeNodeData> m_onSegmentsPerDataChanged;
};

class CMultiFrameNodeData : public CPrefabEditor::CNodeData
{
public:
	CMultiFrameNodeData( CUITreeView* pView, CPrefabEditor* pOwner, CPrefabNode* pNode )
		: CPrefabEditor::CNodeData( pOwner, pNode )
	{
		m_pView = pView;
		m_pResRoot = CTreeFolder::Create( pView, NULL, "Multi Frames Data" );
		m_pBeginFrame = CCommonEdit::Create( "Begin Frame" );
		pView->AddContentChild( m_pBeginFrame, m_pResRoot );
		m_onDataChanged[0].Set( this, &CMultiFrameNodeData::OnDataChanged );
		m_pBeginFrame->Register( CUIElement::eEvent_Action, &m_onDataChanged[0] );
		m_pEndFrame = CCommonEdit::Create( "End Frame" );
		pView->AddContentChild( m_pEndFrame, m_pResRoot );
		m_onDataChanged[1].Set( this, &CMultiFrameNodeData::OnDataChanged );
		m_pEndFrame->Register( CUIElement::eEvent_Action, &m_onDataChanged[1] );
		m_pFramesPerSec = CCommonEdit::Create( "Frames Per Sec" );
		pView->AddContentChild( m_pFramesPerSec, m_pResRoot );
		m_onDataChanged[2].Set( this, &CMultiFrameNodeData::OnDataChanged );
		m_pFramesPerSec->Register( CUIElement::eEvent_Action, &m_onDataChanged[2] );
		m_pPlaySpeed = CCommonEdit::Create( "Play Speed" );
		pView->AddContentChild( m_pPlaySpeed, m_pResRoot );
		m_onDataChanged[3].Set( this, &CMultiFrameNodeData::OnDataChanged );
		m_pPlaySpeed->Register( CUIElement::eEvent_Action, &m_onDataChanged[3] );
		m_pLoop = CBoolEdit::Create( "Loop" );
		pView->AddContentChild( m_pLoop, m_pResRoot );
		m_onDataChanged[4].Set( this, &CMultiFrameNodeData::OnDataChanged );
		m_pLoop->Register( CUIElement::eEvent_Action, &m_onDataChanged[4] );

		RefreshData();
	}
	~CMultiFrameNodeData()
	{
		for( int i = 0; i < ELEM_COUNT( m_onDataChanged ); i++ )
		{
			if( m_onDataChanged[i].IsRegistered() )
				m_onDataChanged[i].Unregister();
		}
		m_pView->RemoveContentTree( m_pResRoot );
	}
protected:
	void RefreshData()
	{
		CMultiFrameImage2D* pImage = static_cast<CMultiFrameImage2D*>( m_pNode->GetRenderObject() );
		m_pBeginFrame->SetValue( pImage->GetFrameBegin() );
		m_pEndFrame->SetValue( pImage->GetFrameEnd() );
		m_pFramesPerSec->SetValue( pImage->GetFramesPerSec() );
		m_pPlaySpeed->SetValue( pImage->GetPlaySpeed() );
		m_pLoop->SetChecked( pImage->IsLoop() );
	}

	void OnDataChanged()
	{
		CMultiFrameImage2D* pImage = static_cast<CMultiFrameImage2D*>( m_pNode->GetRenderObject() );
		pImage->SetFrames( m_pBeginFrame->GetValue<uint32>(), m_pEndFrame->GetValue<uint32>(), m_pFramesPerSec->GetValue<float>() );
		pImage->SetPlaySpeed( m_pPlaySpeed->GetValue<float>(), m_pLoop->IsChecked() );
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	CReference<CCommonEdit> m_pBeginFrame;
	CReference<CCommonEdit> m_pEndFrame;
	CReference<CCommonEdit> m_pFramesPerSec;
	CReference<CCommonEdit> m_pPlaySpeed;
	CReference<CBoolEdit> m_pLoop;

	TClassTrigger<CMultiFrameNodeData> m_onDataChanged[5];
};


class CTileMapNodeData : public CPrefabEditor::CNodeData
{
public:
	CTileMapNodeData( CUITreeView* pView, CPrefabEditor* pOwner, CPrefabNode* pNode )
		: CPrefabEditor::CNodeData( pOwner, pNode )
	{
		m_pView = pView;
		m_pResRoot = CTreeFolder::Create( pView, NULL, "Tile Map Data" );
		m_pTileSize = CVectorEdit::Create( "Tile Size", 2 );
		pView->AddContentChild( m_pTileSize, m_pResRoot );
		m_pBaseOffset = CVectorEdit::Create( "Base Offset", 2 );
		pView->AddContentChild( m_pBaseOffset, m_pResRoot );
		m_pWidth = CCommonEdit::Create( "Width" );
		pView->AddContentChild( m_pWidth, m_pResRoot );
		m_pHeight = CCommonEdit::Create( "Height" );
		pView->AddContentChild( m_pHeight, m_pResRoot );
		for( int i = 0; i < 4; i++ )
			m_onRefresh[i].Set( this, &CTileMapNodeData::Refresh );
		m_pTileSize->Register( CUIElement::eEvent_Action, &m_onRefresh[0] );
		m_pBaseOffset->Register( CUIElement::eEvent_Action, &m_onRefresh[1] );
		m_pWidth->Register( CUIElement::eEvent_Action, &m_onRefresh[2] );
		m_pHeight->Register( CUIElement::eEvent_Action, &m_onRefresh[3] );

		m_pTileExt = CVectorEdit::Create( "Tile Ext", 4 );
		pView->AddContentChild( m_pTileExt, m_pResRoot );
		m_pExt = static_cast<CUIButton*>( CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/button.xml" )->GetElement()->Clone() );
		pView->AddContentChild( m_pExt, m_pResRoot );
		m_onExt.Set( this, &CTileMapNodeData::OnExt );
		m_pExt->Register( CUIElement::eEvent_Action, &m_onExt );
		m_pExt->SetText( "Ext" );

		m_pEnableEdit = CBoolEdit::Create( "Enable Edit" );
		pView->AddContentChild( m_pEnableEdit, m_pResRoot );
		m_pCurEditType = CCommonEdit::Create( "Edit Type" );
		pView->AddContentChild( m_pCurEditType, m_pResRoot );
		m_pBrushSize = CCommonEdit::Create( "Brush Size" );
		pView->AddContentChild( m_pBrushSize, m_pResRoot );
		m_pBrushShape = CCommonEdit::Create( "Brush Shape" );
		pView->AddContentChild( m_pBrushShape, m_pResRoot );
		m_pFill = static_cast<CUIButton*>( CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/button.xml" )->GetElement()->Clone() );
		pView->AddContentChild( m_pFill, m_pResRoot );
		for( int i = 0; i < ELEM_COUNT( m_onEditTypeChanged ); i++ )
			m_onEditTypeChanged[i].Set( this, &CTileMapNodeData::OnEditTypeChanged );
		m_onFill.Set( this, &CTileMapNodeData::OnFill );
		m_pEnableEdit->Register( CUIElement::eEvent_Action, &m_onEditTypeChanged[0] );
		m_pCurEditType->Register( CUIElement::eEvent_Action, &m_onEditTypeChanged[1] );
		m_pBrushSize->Register( CUIElement::eEvent_Action, &m_onEditTypeChanged[2] );
		m_pBrushShape->Register( CUIElement::eEvent_Action, &m_onEditTypeChanged[3] );
		m_pFill->Register( CUIElement::eEvent_Action, &m_onFill );
		m_pFill->SetText( "Fill" );
		m_bEnableEdit = false;
		m_bDragging = false;
		m_nCurEditType = 0;
		m_nCurBrushSize = 1;
		m_nCurBrushShape = 0;

		RefreshData();
	}
	~CTileMapNodeData()
	{
		for( int i = 0; i < 4; i++ )
		{
			if( m_onRefresh[i].IsRegistered() )
				m_onRefresh[i].Unregister();
		}
		for( int i = 0; i < 2; i++ )
		{
			if( m_onEditTypeChanged[i].IsRegistered() )
				m_onEditTypeChanged[i].Unregister();
		}
		if( m_onFill.IsRegistered() )
			m_onFill.Unregister();
		if( m_onExt.IsRegistered() )
			m_onExt.Unregister();
		m_pView->RemoveContentTree( m_pResRoot );
	}
	
	virtual void OnViewportChar( uint32 nChar ) override
	{
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		
		uint32 nEditTypeCount = pTileMap->GetInfo()->editInfos.size();

		switch( nChar )
		{
		case '`':
			m_bEnableEdit = !m_bEnableEdit;
			m_pEnableEdit->SetChecked( m_bEnableEdit );
			break;
		case 'Q':
		case 'q':
			m_nCurEditType = m_nCurEditType > 0 ? m_nCurEditType - 1 : nEditTypeCount - 1;
			m_pCurEditType->SetValue( m_nCurEditType );
			break;
		case 'W':
		case 'w':
			m_nCurEditType = m_nCurEditType < nEditTypeCount - 1 ? m_nCurEditType + 1 : 0;
			m_pCurEditType->SetValue( m_nCurEditType );
			break;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			m_nCurEditType = Min( nChar - '0', nEditTypeCount - 1 );
			m_pCurEditType->SetValue( m_nCurEditType );
			break;
		case '=':
			m_nCurBrushSize = Min<int8>( m_nCurBrushSize + 1, 9 );
			m_pBrushSize->SetValue( (int32)m_nCurBrushSize );
			break;
		case '-':
			m_nCurBrushSize = Max<int8>( m_nCurBrushSize - 1, 1 );
			m_pBrushSize->SetValue( (int32)m_nCurBrushSize );
			break;
		case ']':
			m_nCurBrushShape = m_nCurBrushShape < 2 ? m_nCurBrushShape + 1 : 0;
			m_pBrushShape->SetValue( (int32)m_nCurBrushShape );
			break;
		case '[':
			m_nCurBrushShape = m_nCurBrushShape > 0 ? m_nCurBrushShape - 1 : 2;
			m_pBrushShape->SetValue( (int32)m_nCurBrushShape );
			break;
		default:
			break;
		}
	}
	bool OnEditMap( const CVector2& localPos )
	{
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
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
						if( dx * dx + dy * dy > ( m_nCurBrushSize - 1 ) * ( m_nCurBrushSize - 1 ) + 1 )
							continue;
					}
					else if( m_nCurBrushShape == 2 )
					{
						if( abs( x * 2 + 1 - p0.x ) + abs( y * 2 + 1 - p0.y ) > m_nCurBrushSize )
							continue;
					}
					pTileMap->EditTile( x, y, m_nCurEditType );
				}
			}
			return true;
		}
		return false;
	}
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override
	{
		if( !m_bEnableEdit )
			return false;
		auto matInv = transform.Inverse();
		auto localPos = matInv.MulVector2Pos( mousePos );
		if( OnEditMap( localPos ) )
		{
			m_bDragging = true;
			return true;
		}
		return false;
	}
	virtual bool OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override
	{
		if( !m_bDragging )
			return false;
		auto matInv = transform.Inverse();
		auto localPos = matInv.MulVector2Pos( mousePos );
		OnEditMap( localPos );
		return true;
	}
	virtual bool OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override
	{
		if( !m_bDragging )
			return false;
		m_bDragging = false;
		return true;
	}
protected:
	void Refresh()
	{
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		pTileMap->Set( m_pTileSize->GetFloat2(), m_pBaseOffset->GetFloat2(), m_pWidth->GetValue<uint32>(), m_pHeight->GetValue<uint32>() );
	}
	void RefreshData()
	{
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		m_pTileSize->SetFloat2( pTileMap->GetTileSize() );
		m_pBaseOffset->SetFloat2( pTileMap->GetBaseOffset() );
		m_pWidth->SetValue( pTileMap->GetWidth() );
		m_pHeight->SetValue( pTileMap->GetHeight() );
		m_pCurEditType->SetValue( m_nCurEditType );
		m_pBrushSize->SetValue( (int32)m_nCurBrushSize );
		m_pBrushShape->SetValue( m_nCurBrushShape );
	}

	void OnEditTypeChanged()
	{
		m_bEnableEdit = m_pEnableEdit->IsChecked();
		m_nCurEditType = m_pCurEditType->GetValue<uint32>();
		m_nCurBrushSize = m_pBrushSize->GetValue<uint32>();
		m_nCurBrushShape = m_pBrushShape->GetValue<uint32>();
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		if( m_nCurEditType >= pTileMap->GetInfo()->editInfos.size() )
		{
			m_nCurEditType = pTileMap->GetInfo()->editInfos.size() - 1;
			m_pCurEditType->SetValue( m_nCurEditType );
		}
		if( m_nCurBrushSize > 9 )
		{
			m_nCurBrushSize = 9;
			m_pBrushSize->SetValue( (int32)m_nCurBrushSize );
		}
		if( m_nCurBrushShape >= 3 )
		{
			m_nCurBrushShape = 2;
			m_pBrushShape->SetValue( m_nCurBrushShape );
		}
	}

	void OnFill()
	{
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		for( int i = 0; i <= pTileMap->GetWidth(); i++ )
		{
			for( int j = 0; j <= pTileMap->GetHeight(); j++ )
			{
				pTileMap->SetEditData( i, j, m_nCurEditType | ( SRand::Inst().Rand() << 16 ) );
			}
		}
		pTileMap->RefreshAll();
	}

	void OnExt()
	{
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		auto size = m_pTileExt->GetFloat4();
		pTileMap->Resize( TRectangle<int32>( size.x, size.y, size.z, size.w ) );
		RefreshData();
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	
	CReference<CVectorEdit> m_pTileSize;
	CReference<CVectorEdit> m_pTileExt;
	CReference<CVectorEdit> m_pBaseOffset;
	CReference<CCommonEdit> m_pWidth;
	CReference<CCommonEdit> m_pHeight;

	CReference<CBoolEdit> m_pEnableEdit;
	CReference<CCommonEdit> m_pCurEditType;
	CReference<CCommonEdit> m_pBrushSize;
	CReference<CCommonEdit> m_pBrushShape;
	CReference<CUIButton> m_pFill;
	CReference<CUIButton> m_pExt;

	bool m_bDragging;
	bool m_bEnableEdit;
	uint32 m_nCurEditType;
	uint8 m_nCurBrushSize;
	uint8 m_nCurBrushShape;
	
	TClassTrigger<CTileMapNodeData> m_onFill;
	TClassTrigger<CTileMapNodeData> m_onExt;
	TClassTrigger<CTileMapNodeData> m_onRefresh[4];
	TClassTrigger<CTileMapNodeData> m_onEditTypeChanged[4];
};

class CParticleNodeData : public CPrefabEditor::CNodeData
{
public:
	CParticleNodeData( CUITreeView* pView, CPrefabEditor* pOwner, CPrefabNode* pNode )
		: CPrefabEditor::CNodeData( pOwner, pNode )
	{
		m_pView = pView;
		m_pResRoot = CTreeFolder::Create( pView, NULL, "Particle Data" );
		m_pEmitterClass = CDropDownBox::CreateClassSelectBox<IParticleEmitter>( "Emitter" );
		m_pView->AddContentChild( m_pEmitterClass, m_pResRoot );
		m_onClassChanged.Set( this, &CParticleNodeData::OnClassChanged );
		m_pEmitterClass->Register( CUIElement::eEvent_Action, &m_onClassChanged );

		m_onRefresh.Set( this, &CParticleNodeData::Refresh );

		RefreshData();
	}
	~CParticleNodeData()
	{
		m_pEmitterData = NULL;
		m_pView->RemoveContentTree( m_pResRoot );
		if( m_onClassChanged.IsRegistered() )
			m_onClassChanged.Unregister();
		if( m_onRefresh.IsRegistered() )
			m_onRefresh.Unregister();
	}
protected:
	void Refresh( uint32 nParam = 1 )
	{
		if( nParam != 1 )
			return;
		CParticleSystemObject* pObj = static_cast<CParticleSystemObject*>( m_pNode->GetRenderObject() );
		auto pInstData = pObj->GetInstanceData();
		pInstData->Reset();
		m_obj.SetDirty();
		pInstData->SetEmitter( m_obj.CreateObject() );
	}

	void RefreshData()
	{
		if( m_onRefresh.IsRegistered() )
			m_onRefresh.Unregister();
		m_pEmitterData = NULL;
		CParticleSystemObject* pObj = static_cast<CParticleSystemObject*>( m_pNode->GetRenderObject() );
		auto pEmitter = pObj->GetInstanceData()->GetEmitter();
		auto pClassData = pEmitter ? CClassMetaDataMgr::Inst().GetClassData( pEmitter ) : NULL;
		if( pClassData )
			m_pEmitterClass->SetSelectedItem( pClassData->strDisplayName.c_str(), false );
		else
			m_pEmitterClass->SetSelectedItem( 0u, false );
		
		CBufFile buf;
		string className = pClassData ? pClassData->strClassName : "";
		buf.Write( className );
		pClassData->PackData( (uint8*)pEmitter, buf, false );
		
		m_obj.Load( buf, false, NULL );
		if( m_obj.GetClassData() )
		{
			m_pEmitterData = CObjectDataEditMgr::Inst().Create( m_pView, m_pResRoot, m_obj.GetObjData(), m_obj.GetClassData() );
			m_pEmitterData->Register( &m_onRefresh );
		}
	}

	void OnClassChanged()
	{
		if( m_onRefresh.IsRegistered() )
			m_onRefresh.Unregister();
		m_pEmitterData = NULL;
		auto pData = (SClassMetaData*)m_pEmitterClass->GetSelectedItem()->pData;
		m_obj.SetClassName( pData ? pData->strClassName.c_str() : NULL );
		if( m_obj.GetClassData() )
		{
			m_pEmitterData = CObjectDataEditMgr::Inst().Create( m_pView, m_pResRoot, m_obj.GetObjData(), m_obj.GetClassData() );
			m_pEmitterData->Register( &m_onRefresh );
		}
		
		Refresh();
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	CReference<CDropDownBox> m_pEmitterClass;
	CReference<CObjectDataEditItem> m_pEmitterData;
	TObjectPrototype<IParticleEmitter> m_obj;

	TClassTrigger1<CParticleNodeData, uint32> m_onRefresh;
	TClassTrigger<CParticleNodeData> m_onClassChanged;
};

class CDirectionalLightNodeData : public CPrefabEditor::CNodeData
{
public:
	CDirectionalLightNodeData( CUITreeView* pView, CPrefabEditor* pOwner, CPrefabNode* pNode )
		: CPrefabEditor::CNodeData( pOwner, pNode )
	{
		m_pView = pView;
		m_pResRoot = CTreeFolder::Create( pView, NULL, "Directional Light Data" );
		m_pDir = CVectorEdit::Create( "Direction", 2 );
		pView->AddContentChild( m_pDir, m_pResRoot );
		m_pShadowScale = CCommonEdit::Create( "Shadow Scale" );
		pView->AddContentChild( m_pShadowScale, m_pResRoot );
		m_pMaxShadowDist = CCommonEdit::Create( "Max Shadow Dist" );
		pView->AddContentChild( m_pMaxShadowDist, m_pResRoot );
		m_pBaseColor = CVectorEdit::Create( "Base Color", 3 );
		pView->AddContentChild( m_pBaseColor, m_pResRoot );
		m_onRefresh.resize( 4 );

		for( auto& item : m_onRefresh )
		{
			item.Set( this, &CDirectionalLightNodeData::Refresh );
		}
		m_pDir->Register( CUIElement::eEvent_Action, &m_onRefresh[0] );
		m_pShadowScale->Register( CUIElement::eEvent_Action, &m_onRefresh[1] );
		m_pMaxShadowDist->Register( CUIElement::eEvent_Action, &m_onRefresh[2] );
		m_pBaseColor->Register( CUIElement::eEvent_Action, &m_onRefresh[3] );
		RefreshData();
	}
	~CDirectionalLightNodeData()
	{
		m_pView->RemoveContentTree( m_pResRoot );
		for( auto& item : m_onRefresh )
		{
			if( item.IsRegistered() )
				item.Unregister();
		}
	}
protected:
	virtual void OnResourceRefreshEnd() override { RefreshData(); }

	void Refresh()
	{
		CDirectionalLightObject* pLight = static_cast<CDirectionalLightObject*>( m_pNode->GetRenderObject() );
		pLight->SetDir( m_pDir->GetFloat2() );
		pLight->fShadowScale = m_pShadowScale->GetValue<float>();
		pLight->fMaxShadowDist = m_pMaxShadowDist->GetValue<float>();
		pLight->baseColor = m_pBaseColor->GetFloat3();
		m_pBaseColor->SetFloats( &pLight->baseColor.x );
	}

	void RefreshData()
	{
		CDirectionalLightObject* pLight = static_cast<CDirectionalLightObject*>( m_pNode->GetRenderObject() );
		m_pDir->SetFloats( &pLight->Dir.x );
		m_pShadowScale->SetValue( pLight->fShadowScale );
		m_pMaxShadowDist->SetValue( pLight->fMaxShadowDist );
		m_pBaseColor->SetFloats( &pLight->baseColor.x );
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	CReference<CVectorEdit> m_pDir;
	CReference<CCommonEdit> m_pShadowScale;
	CReference<CCommonEdit> m_pMaxShadowDist;
	CReference<CVectorEdit> m_pBaseColor;

	vector<TClassTrigger<CDirectionalLightNodeData> > m_onRefresh;
};

class CPointLightNodeData : public CPrefabEditor::CNodeData
{
public:
	CPointLightNodeData( CUITreeView* pView, CPrefabEditor* pOwner, CPrefabNode* pNode )
		: CPrefabEditor::CNodeData( pOwner, pNode )
	{
		m_pView = pView;
		m_pResRoot = CTreeFolder::Create( pView, NULL, "Directional Light Data" );
		m_pAttenuationIntensity = CVectorEdit::Create( "Attenuation Params", 4 );
		pView->AddContentChild( m_pAttenuationIntensity, m_pResRoot );
		m_pShadowScale = CCommonEdit::Create( "Shadow Scale" );
		pView->AddContentChild( m_pShadowScale, m_pResRoot );
		m_pMaxRange = CCommonEdit::Create( "Max Range" );
		pView->AddContentChild( m_pMaxRange, m_pResRoot );
		m_pLightHeight = CCommonEdit::Create( "Light Height" );
		pView->AddContentChild( m_pLightHeight, m_pResRoot );
		m_pBaseColor = CVectorEdit::Create( "Base Color", 3 );
		pView->AddContentChild( m_pBaseColor, m_pResRoot );
		m_onRefresh.resize( 5 );

		for( auto& item : m_onRefresh )
		{
			item.Set( this, &CPointLightNodeData::Refresh );
		}
		m_pAttenuationIntensity->Register( CUIElement::eEvent_Action, &m_onRefresh[0] );
		m_pShadowScale->Register( CUIElement::eEvent_Action, &m_onRefresh[1] );
		m_pMaxRange->Register( CUIElement::eEvent_Action, &m_onRefresh[2] );
		m_pLightHeight->Register( CUIElement::eEvent_Action, &m_onRefresh[3] );
		m_pBaseColor->Register( CUIElement::eEvent_Action, &m_onRefresh[4] );
		RefreshData();
	}
	~CPointLightNodeData()
	{
		m_pView->RemoveContentTree( m_pResRoot );
		for( auto& item : m_onRefresh )
		{
			if( item.IsRegistered() )
				item.Unregister();
		}
	}
protected:
	virtual void OnResourceRefreshEnd() override { RefreshData(); }

	void Refresh()
	{
		CPointLightObject* pLight = static_cast<CPointLightObject*>( m_pNode->GetRenderObject() );
		pLight->AttenuationIntensity = m_pAttenuationIntensity->GetFloat4();
		pLight->fShadowScale = m_pShadowScale->GetValue<float>();
		pLight->fMaxRange = m_pMaxRange->GetValue<float>();
		pLight->fLightHeight = m_pLightHeight->GetValue<float>();
		pLight->baseColor = m_pBaseColor->GetFloat3();
		m_pBaseColor->SetFloats( &pLight->baseColor.x );
	}

	void RefreshData()
	{
		CPointLightObject* pLight = static_cast<CPointLightObject*>( m_pNode->GetRenderObject() );
		m_pAttenuationIntensity->SetFloats( &pLight->AttenuationIntensity.x );
		m_pShadowScale->SetValue( pLight->fShadowScale );
		m_pMaxRange->SetValue( pLight->fMaxRange );
		m_pLightHeight->SetValue( pLight->fLightHeight );
		m_pBaseColor->SetFloats( &pLight->baseColor.x );
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	CReference<CVectorEdit> m_pAttenuationIntensity;
	CReference<CCommonEdit> m_pShadowScale;
	CReference<CCommonEdit> m_pMaxRange;
	CReference<CCommonEdit> m_pLightHeight;
	CReference<CVectorEdit> m_pBaseColor;

	vector<TClassTrigger<CPointLightNodeData> > m_onRefresh;
};

void CPrefabEditor::SelectNode( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	if( pNode && pNode == m_pCurNode )
	{
		SetCamOfs( pNode->globalTransform.GetPosition() );
		return;
	}
	auto pItem0 = m_pCurNodeItem;

	m_pNodeData = NULL;
	if( m_onNodeObjDataChanged.IsRegistered() )
		m_onNodeObjDataChanged.Unregister();
	m_pObjectData = NULL;
	m_pCurNode = pNode;
	m_pCurNodeItem = pCurNodeItem;
	if( m_pCurNode )
	{
		m_pNodeView->SetVisible( true );
		CVector4 vec = CVector4( m_pCurNode->x, m_pCurNode->y, m_pCurNode->r / PI * 180, m_pCurNode->s );
		m_pTransform->SetFloats( &vec.x );
		m_pZOrder->SetValue( m_pCurNode->GetZOrder() );
		m_pNodeName->SetText( m_pCurNode->GetName() );
		if( m_pCurNode->GetClassData() )
			m_pNodeClass->SetSelectedItem( m_pCurNode->GetClassData()->strDisplayName.c_str(), false );
		else
			m_pNodeClass->SetSelectedItem( 0u, false );

		CNodeData* pNodeData = NULL;
		if( !m_pCurNode->m_nType )
		{
			CResource* pResource = m_pCurNode->GetResource();
			m_pResFileName->SetText( pResource ? pResource->GetName() : "" );
			if( pResource )
			{
				switch( pResource->GetResourceType() )
				{
				case eEngineResType_DrawableGroup:
					if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_Default )
						pNodeData = new CDrawableGroupNodeData( m_pNodeView, this, m_pCurNode );
					else if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_Rope )
						pNodeData = new CRopeNodeData( m_pNodeView, this, m_pCurNode );
					else if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_MultiFrame )
						pNodeData = new CMultiFrameNodeData( m_pNodeView, this, m_pCurNode );
					else if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_TileMap )
						pNodeData = new CTileMapNodeData( m_pNodeView, this, m_pCurNode );
					break;
				case eEngineResType_ParticleSystem:
					pNodeData = new CParticleNodeData( m_pNodeView, this, m_pCurNode );
					break;
				default:
					break;
				}
			}
		}
		else
		{
			m_pResFileName->SetText( "" );
			switch( m_pCurNode->m_nType )
			{
			case 1:
				pNodeData = new CDirectionalLightNodeData( m_pNodeView, this, m_pCurNode );
				break;
			case 2:
				pNodeData = new CPointLightNodeData( m_pNodeView, this, m_pCurNode );
				break;
			}
		}
		m_pNodeData = pNodeData;

		if( m_pCurNode->GetClassData() )
		{
			m_pObjectData = CObjectDataEditMgr::Inst().Create( m_pNodeView, NULL, m_pCurNode->GetObjData(), m_pCurNode->GetClassData() );
			if( m_pObjectData )
				m_pObjectData->Register( &m_onNodeObjDataChanged );
		}

		bool bInst = pNode->IsInstance();
		m_pZOrder->SetEnabled( !bInst );
		m_pNodeName->SetEnabled( !bInst );
	}

	if( pItem0 )
	{
		for( auto p = pItem0->pParent; p; p = p->pParent )
			static_cast<CPrefabNodeTreeFolder*>( p->pElement.GetPtr() )->m_pNode->OnEditorActive( false );
	}
	if( pCurNodeItem )
	{
		for( auto p = pCurNodeItem->pParent; p; p = p->pParent )
			static_cast<CPrefabNodeTreeFolder*>( p->pElement.GetPtr() )->m_pNode->OnEditorActive( true );
	}

	RefreshGizmo();
}

void CPrefabEditor::SelectItem( const char* szItem )
{
	SelectNode( NULL, NULL );
	if( m_pSelectedPrefab )
		m_pSelectedPrefab->RemoveThis();
	auto itr = m_mapClonedPrefabs.find( szItem );
	ASSERT( itr != m_mapClonedPrefabs.end() );
	m_strSelectedPrefab = szItem;
	m_pSelectedPrefab = itr->second;
	m_pViewport->GetRoot()->AddChild( m_pSelectedPrefab );
	m_pSceneView->ClearContent();
	RefreshSceneView( m_pSelectedPrefab, NULL );
	SelectNode( m_pSelectedPrefab, static_cast<CUITreeView::CTreeViewContent*>( m_pSceneView->Get_Content() ) );
}

void CPrefabEditor::MoveNodeUp( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	if( m_pCurNode->IsInstance() )
		return;
	auto pPrevItem = m_pSceneView->MoveUp( pCurNodeItem );
	if( !pPrevItem )
		return;
	CReference<CPrefabNode> pPrevNode = static_cast<CPrefabNodeTreeFolder*>( pPrevItem->pElement.GetPtr() )->m_pNode;
	pPrevNode->RemoveThis();
	pNode->GetParent()->AddChildBefore( pPrevNode, pNode );
	pNode->OnEditorMove( m_pSelectedPrefab );
}

void CPrefabEditor::MoveNodeDown( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	if( m_pCurNode->IsInstance() )
		return;
	auto pNextItem = m_pSceneView->MoveDown( pCurNodeItem );
	if( !pNextItem )
		return;
	CReference<CPrefabNode> pNextNode = static_cast<CPrefabNodeTreeFolder*>( pNextItem->pElement.GetPtr() )->m_pNode;
	pNextNode->RemoveThis();
	pNode->GetParent()->AddChildAfter( pNextNode, pNode );
	pNode->OnEditorMove( m_pSelectedPrefab );
}

void CPrefabEditor::MoveNodeLeft( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	if( m_pCurNode->IsInstance() )
		return;
	if( !pCurNodeItem->pParent || !pCurNodeItem->pParent->pParent )
		return;
	SelectNode( NULL, NULL );
	auto pParent = m_pSceneView->MoveLeft( pCurNodeItem );
	if( !pParent )
	{
		SelectNode( pNode, pCurNodeItem );
		return;
	}
	CReference<CPrefabNode> pTempNode = pNode;
	CReference<CPrefabNode> pParentNode = static_cast<CPrefabNodeTreeFolder*>( pParent->pElement.GetPtr() )->m_pNode;
	CReference<CPrefabNode> pParentParentNode = static_cast<CPrefabNodeTreeFolder*>( pParent->pParent->pElement.GetPtr() )->m_pNode;
	pNode->RemoveThis();
	pParentParentNode->AddChildBefore( pNode, pParentNode );
	pNode->OnEditorMove( m_pSelectedPrefab );
	pParentNode->OnEditorMove( m_pSelectedPrefab );
	SelectNode( pNode, pCurNodeItem );
}

void CPrefabEditor::MoveNodeRight( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	if( m_pCurNode->IsInstance() )
		return;
	SelectNode( NULL, NULL );
	auto pParent = m_pSceneView->MoveRight( pCurNodeItem );
	if( !pParent )
	{
		SelectNode( pNode, pCurNodeItem );
		return;
	}
	CReference<CPrefabNode> pTempNode = pNode;
	CReference<CPrefabNode> pParentNode = static_cast<CPrefabNodeTreeFolder*>( pParent->pElement.GetPtr() )->m_pNode;
	pNode->RemoveThis();
	pParentNode->AddChild( pNode );
	pNode->OnEditorMove( m_pSelectedPrefab );
	SelectNode( pNode, pCurNodeItem );
}

void CPrefabEditor::ShowChildren( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	m_pSceneView->SetContentFolded( pCurNodeItem, false );
	static_cast<CPrefabNodeTreeFolder*>( pCurNodeItem->pElement.GetPtr() )->SetChecked( false, false );
	if( pCurNodeItem->pTail && pCurNodeItem->NextContent() != pCurNodeItem->pTail )
	{
		for( auto p = ( CUITreeView::CTreeViewContent* )pCurNodeItem->NextContent(); p; p = m_pSceneView->GetNextSibling( p ) )
		{
			m_pSceneView->SetContentFolded( p, true );
			static_cast<CPrefabNodeTreeFolder*>( p->pElement.GetPtr() )->SetChecked( true, false );
		}
	}
}

void CPrefabEditor::OnCurNodeNameChanged()
{
	if( m_pCurNode->IsInstance() )
		return;
	m_pCurNode->m_strName = UnicodeToUtf8( m_pNodeName->GetText() ).c_str();
	m_pCurNodeItem->pElement->GetChildByName<CUIButton>( "label" )->SetText( m_pCurNode->m_strName.c_str() );
	m_pCurNode->OnEditorMove( m_pSelectedPrefab );
}

void CPrefabEditor::OnCurNodeResourceChanged()
{
	if( !m_pCurNode || m_pCurNode->m_nType )
		return;
	CResource* pResource = NULL;
	string strName = UnicodeToUtf8( m_pResFileName->GetText() );
	if( strName.length() )
	{
		if( !IsFileExist( strName.c_str() ) )
			return;
		const char* szExt = GetFileExtension( strName.c_str() );
		if( !strcmp( szExt, "mtl" ) )
			pResource = CResourceManager::Inst()->CreateResource<CDrawableGroup>( strName.c_str() );
		else if( !strcmp( szExt, "pts" ) )
			pResource = CResourceManager::Inst()->CreateResource<CParticleFile>( strName.c_str() );
		else if( !strcmp( szExt, "pf" ) )
			pResource = CResourceManager::Inst()->CreateResource<CPrefab>( strName.c_str() );
		if( !pResource )
			return;
	}

	m_pNodeData = NULL;
	if( m_pCurNode->GetResource() == pResource )
		return;
	if( m_pCurNode->GetPatchedNode() )
		DestroyPatchNode( m_pCurNode, m_pCurNodeItem );

	if( !m_pCurNode->SetResource( pResource ) )
	{
		CResource* pResource = m_pCurNode->GetResource();
		m_pResFileName->SetText( pResource ? pResource->GetName() : "" );
		if( m_pCurNode->GetPatchedNode() )
			CreatePatchNode( m_pCurNode, m_pCurNodeItem );
		return;
	}

	CNodeData* pNodeData = NULL;
	if( pResource )
	{
		switch( pResource->GetResourceType() )
		{
		case eEngineResType_DrawableGroup:
			if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_Default )
				pNodeData = new CDrawableGroupNodeData( m_pNodeView, this, m_pCurNode );
			else if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_Rope )
				pNodeData = new CRopeNodeData( m_pNodeView, this, m_pCurNode );
			else if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_MultiFrame )
				pNodeData = new CMultiFrameNodeData( m_pNodeView, this, m_pCurNode );
			else if( static_cast<CDrawableGroup*>( pResource )->GetType() == CDrawableGroup::eType_TileMap )
				pNodeData = new CTileMapNodeData( m_pNodeView, this, m_pCurNode );
			break;
		case eEngineResType_ParticleSystem:
			pNodeData = new CParticleNodeData( m_pNodeView, this, m_pCurNode );
			break;
		default:
			break;
		}
	}
	m_pNodeData = pNodeData;
	if( m_pCurNode->GetPatchedNode() )
		CreatePatchNode( m_pCurNode, m_pCurNodeItem );
	m_pCurNode->OnEditorActive( false );
}

void CPrefabEditor::OnCurNodeClassChanged()
{
	m_pObjectData = NULL;
	if( m_onNodeObjDataChanged.IsRegistered() )
		m_onNodeObjDataChanged.Unregister();
	auto pData = (SClassMetaData*)m_pNodeClass->GetSelectedItem()->pData;
	m_pCurNode->SetClassName( pData ? pData->strClassName.c_str() : NULL );
	if( m_pCurNode->GetClassData() )
	{
		m_pObjectData = CObjectDataEditMgr::Inst().Create( m_pNodeView, NULL, m_pCurNode->GetObjData(), m_pCurNode->GetClassData() );
		if( m_pObjectData )
			m_pObjectData->Register( &m_onNodeObjDataChanged );
	}
}

void CPrefabEditor::OnCurNodeObjDataChanged( int32 nAction )
{
	if( nAction != 1 )
		return;
	if( m_pCurNode->GetClassData() )
		m_pCurNode->OnEdit();
}

void CPrefabEditor::RefreshGizmo()
{
	if( !m_pCurNode )
	{
		m_pGizmo->bVisible = false;
		return;
	}

	CScene2DManager::GetGlobalInst()->UpdateDirty();
	m_pGizmo->SetPosition( m_pCurNode->globalTransform.GetPosition() );
	m_pGizmo->r = 0;
	m_pGizmo->s = 1;
	m_pGizmo->bVisible = true;
}

void CPrefabEditor::OnViewportStartDrag( SUIMouseEvent* pEvent )
{
	m_nDragType = 0;
	if( m_pGizmo->bVisible )
	{
		m_gizmoOrigPos = m_pGizmo->GetPosition();
		CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
		
		if( m_pNodeData )
		{
			if( m_pNodeData->OnViewportStartDrag( m_pViewport, fixOfs, m_pCurNode->globalTransform ) )
				return;
		}
		if( m_pObjectData )
		{
			m_pCurDragEdit = m_pObjectData->OnViewportStartDrag( m_pViewport, fixOfs, m_pCurNode->globalTransform );
			if( m_pCurDragEdit )
				return;
		}

		CVector2 localPos = fixOfs - m_pGizmo->GetPosition();

		if( CRectangle( -1, -1, 42, 42 ).Contains( localPos ) )
			m_nDragType = 1;
		else if( CRectangle( -1, -4, 200, 8 ).Contains( localPos ) )
			m_nDragType = 2;
		else if( CRectangle( -4, -1, 8, 200 ).Contains( localPos ) )
			m_nDragType = 3;
		else if( CRectangle( 80, 80, 9, 9 ).Contains( localPos ) )
			m_nDragType = 4;
		else if( CRectangle( 52, 52, 9, 9 ).Contains( localPos ) )
			m_nDragType = 5;
		if( m_nDragType )
		{
			CScene2DManager::GetGlobalInst()->UpdateDirty();
			m_origTransform = m_pCurNode->globalTransform;
			m_startDragPos = fixOfs;
			return;
		}
	}

	Super::OnViewportStartDrag( pEvent );
}

void CPrefabEditor::OnViewportDragged( SUIMouseEvent* pEvent )
{
	CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
	if( m_pNodeData )
	{
		if( m_pNodeData->OnViewportDragged( m_pViewport, fixOfs, m_pCurNode->globalTransform ) )
			return;
	}
	if( m_pCurDragEdit )
	{
		m_pCurDragEdit->OnViewportDragged( m_pViewport, fixOfs, m_pCurNode->globalTransform );
		return;
	}

	if( m_nDragType == 0 )
	{
		Super::OnViewportDragged( pEvent );
		return;
	}
	
	CVector2 dPos = fixOfs - m_startDragPos;

	switch( m_nDragType )
	{
	case 1:
		m_pGizmo->SetPosition( m_gizmoOrigPos + dPos );
		break;
	case 2:
		m_pGizmo->x = m_gizmoOrigPos.x + dPos.x;
		m_pGizmo->SetTransformDirty();
		break;
	case 3:
		m_pGizmo->y = m_gizmoOrigPos.y + dPos.y;
		m_pGizmo->SetTransformDirty();
		break;
	case 4:
		{
			CVector2 ofs = fixOfs - m_gizmoOrigPos;
			CVector2 ofs0 = m_startDragPos - m_gizmoOrigPos;
			float fAngle = atan2( ofs.y, ofs.x );
			float fAngle0 = atan2( ofs0.y, ofs0.x );
			float dAngle = fAngle - fAngle0;
			m_pGizmo->r = dAngle;
			m_pGizmo->SetTransformDirty();
		}
		break;
	case 5:
		{
			CVector2 ofs = fixOfs - m_gizmoOrigPos;
			CVector2 ofs0 = m_startDragPos - m_gizmoOrigPos;
			float l = ofs.Length();
			float l0 = ofs0.Length();
			m_pGizmo->s = Max( l / l0, 0.0001f );
			m_pGizmo->SetTransformDirty();
		}
		break;
	}
	RefreshCurNodeTransformByGizmo();
}

void CPrefabEditor::OnViewportStopDrag( SUIMouseEvent* pEvent )
{
	CVector2 fixOfs = m_pViewport->GetScenePos( pEvent->mousePos );
	if( m_pNodeData )
	{
		if( m_pNodeData->OnViewportStopDrag( m_pViewport, fixOfs, m_pCurNode->globalTransform ) )
			return;
	}
	if( m_pCurDragEdit )
	{
		m_pCurDragEdit->OnViewportStopDrag( m_pViewport, fixOfs, m_pCurNode->globalTransform );
		m_pCurDragEdit = NULL;
		return;
	}

	if( m_nDragType > 0 )
		RefreshGizmo();
}

void CPrefabEditor::OnViewportDrop( const CVector2& mousePos, CUIElement* pParam )
{
	CVector2 fixOfs = m_pViewport->GetScenePos( mousePos );
	if( m_pObjectData )
		m_pObjectData->OnViewportDrop( m_pViewport, fixOfs, pParam, m_pCurNode->globalTransform );
}

void CPrefabEditor::OnViewportKey( SUIKeyEvent* pEvent )
{
	if( pEvent->bKeyDown )
	{
		if( pEvent->nChar == VK_F1 )
			m_nNodeDebugDrawType = 0;
		else if( pEvent->nChar == VK_F2 )
			m_nNodeDebugDrawType = 1;
		else if( pEvent->nChar == VK_F3 )
			m_nNodeDebugDrawType = 2;
	}
}

void CPrefabEditor::OnViewportChar( uint32 nChar )
{
	if( m_pNodeData )
		m_pNodeData->OnViewportChar( nChar );
}

void CPrefabEditor::RefreshCurNodeTransformByGizmo()
{
	CTransform2D trans = m_origTransform;
	trans.x += m_pGizmo->x - m_gizmoOrigPos.x;
	trans.y += m_pGizmo->y - m_gizmoOrigPos.y;
	trans.r += m_pGizmo->r;
	trans.sx *= m_pGizmo->s;
	trans.sy *= m_pGizmo->s;

	CMatrix2D mat = trans.ToMatrix();
	if( m_pCurNode->GetParent() )
	{
		CMatrix2D& par = m_pCurNode->GetParent()->globalTransform;
		mat = par.Inverse() * mat;
	}
	mat.Decompose( m_pCurNode->x, m_pCurNode->y, m_pCurNode->r, m_pCurNode->s );
	m_pCurNode->SetTransformDirty();
	
	CVector4 vec = CVector4( m_pCurNode->x, m_pCurNode->y, m_pCurNode->r / PI * 180, m_pCurNode->s );
	m_pTransform->SetFloats( &vec.x );
}

void CPrefabEditor::CreatePatchNode( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	auto pPatchedNode = pNode->GetPatchedNode();
	auto pNode1 = pNode->GetRenderObject()->NextRenderChild();
	if( !pNode1 )
	{
		RefreshSceneView( pPatchedNode, pCurNodeItem, 1 );
		return;
	}
	for( auto pChild = pCurNodeItem->NextContent(); pChild != pCurNodeItem->pTail; pChild = pChild->NextContent() )
	{
		auto pChildNode = static_cast<CPrefabNodeTreeFolder*>( pChild->pElement.GetPtr() )->m_pNode;
		if( pChildNode == pNode1 )
		{
			RefreshSceneView( pPatchedNode, static_cast<CUITreeView::CTreeViewContent*>( pChild ), 2 );
			return;
		}
	}
}

void CPrefabEditor::DestroyPatchNode( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	if( !pCurNodeItem->pTail )
		return;
	auto pPatchedNode = pNode->GetPatchedNode();
	for( auto pChild = pCurNodeItem->NextContent(); pChild != pCurNodeItem->pTail; pChild = pChild->NextContent() )
	{
		if( static_cast<CPrefabNodeTreeFolder*>( pChild->pElement.GetPtr() )->m_pNode == pPatchedNode )
		{
			m_pSceneView->RemoveContentTree( pChild );
			break;
		}
	}
}
