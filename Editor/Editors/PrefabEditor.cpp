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

class CPrefabNodeTreeFolder : public CTreeFolder
{
	friend class CPrefabEditor;
public:
	static CUITreeView::CTreeViewContent* Create( CPrefabEditor* pView, CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, CPrefabNode* pNode )
	{
		static CReference<CUIResource> g_pRes = CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/prefabnode_treefolder.xml" );
		auto pTreeFolder = new CPrefabNodeTreeFolder;
		g_pRes->GetElement()->Clone( pTreeFolder );
		CUITreeView::CTreeViewContent* pContent;
		if( pParent )
			pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContentChild( pTreeFolder, pParent ) );
		else
			pContent = static_cast<CUITreeView::CTreeViewContent*>( pTreeView->AddContent( pTreeFolder ) );
		pTreeFolder->m_pTreeView = pTreeView;
		pTreeFolder->m_pContent = pContent;
		pTreeFolder->GetChildByName<CUIButton>( "label" )->SetText( pNode->GetName() );
		pTreeFolder->m_pView = pView;
		pTreeFolder->m_pNode = pNode;
		pContent->fChildrenIndent = 4;
		return pContent;
	}
protected:
	virtual void OnInited() override
	{
		CTreeFolder::OnInited();
		m_onSelect.Set( this, &CPrefabNodeTreeFolder::OnSelect );
		GetChildByName<CUIButton>( "label" )->Register( eEvent_Action, &m_onSelect );
	}
	void OnSelect()
	{
		m_pView->SelectNode( m_pNode, m_pContent );
		GetMgr()->SetFocus( this );
	}

	virtual void OnChar( uint32 nChar ) override
	{
		switch( nChar )
		{
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
	TClassTrigger<CPrefabNodeTreeFolder> m_onSelect;
};

void CPrefabEditor::NewFile( const char* szFileName )
{
	SelectNode( NULL, NULL );
	m_pSceneView->ClearContent();
	if( m_pClonedPrefab )
	{
		m_pClonedPrefab->RemoveThis();
		m_pClonedPrefab = NULL;
	}
	m_pClonedPrefab = new CPrefabNode( NULL );
	m_pViewport->GetRoot()->AddChild( m_pClonedPrefab );
	SelectNode( m_pClonedPrefab, CPrefabNodeTreeFolder::Create( this, m_pSceneView, NULL, m_pClonedPrefab ) );

	Super::NewFile( szFileName );
}

void CPrefabEditor::Refresh()
{
	SelectNode( NULL, NULL );
	m_pSceneView->ClearContent();
	if( m_pClonedPrefab )
	{
		m_pClonedPrefab->RemoveThis();
		m_pClonedPrefab = NULL;
	}

	if( m_pRes )
	{
		CPrefabNode* pNode = m_pRes->GetRoot()->Clone( true );
		m_pClonedPrefab = pNode;
		m_pViewport->GetRoot()->AddChild( m_pClonedPrefab );
		RefreshSceneView( pNode, NULL );
		SelectNode( pNode, static_cast<CUITreeView::CTreeViewContent*>( m_pSceneView->Get_Content() ) );
	}
}

void CPrefabEditor::RefreshSceneView( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pParNodeItem )
{
	auto pContent = CPrefabNodeTreeFolder::Create( this, m_pSceneView, pParNodeItem, pNode );
	vector<CPrefabNode*> vecChildren;
	for( CRenderObject2D* pChild = pNode->Get_Child(); pChild; pChild = pChild->NextChild() )
	{
		if( pChild == pNode->m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			vecChildren.push_back( pNode );
	}
	for( int i = vecChildren.size() - 1; i >= 0; i-- )
	{
		RefreshSceneView( vecChildren[i], pContent );
	}
}

void CPrefabEditor::OnInited()
{
	Super::OnInited();
	m_pSceneView = GetChildByName<CUITreeView>( "scene_view" );
	m_pNodeView = GetChildByName<CUITreeView>( "node_view" );

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
	m_pViewport->GetRoot()->AddChild( pImage2D );
}

void CPrefabEditor::RefreshPreview()
{
	if( !m_pRes )
		return;
	m_pRes->RefreshBegin();
	m_pRes->m_pRoot = m_pClonedPrefab->Clone( true );
	m_pRes->RefreshEnd();
}

void CPrefabEditor::OnDebugDraw( IRenderSystem* pRenderSystem )
{
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
}

void CPrefabEditor::DeleteNode()
{
	CPrefabNode* pDeletedNode = m_pCurNode;
	if( !pDeletedNode || pDeletedNode == m_pClonedPrefab )
		return;
	CUITreeView::CTreeViewContent* pDeletedNodeItem = m_pCurNodeItem;
	CPrefabNode* pSelectedNode = static_cast<CPrefabNode*>( pDeletedNode->GetParent() );
	CUITreeView::CTreeViewContent* pSelectedNodeItem = m_pCurNodeItem->pParent;

	SelectNode( pSelectedNode, pSelectedNodeItem );
	m_pSceneView->RemoveContentTree( pDeletedNodeItem );
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
	CPrefabNode* pNode = m_pClipBoard->Clone( m_pRes.GetPtr() );
	m_pCurNode->AddChild( pNode );

	SelectNode( NULL, NULL );
	m_pSceneView->ClearContent();
	RefreshSceneView( m_pClonedPrefab, NULL );
	SelectNode( m_pClonedPrefab, static_cast<CUITreeView::CTreeViewContent*>( m_pSceneView->Get_Content() ) );
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

	if( m_pCurNode != m_pClonedPrefab )
	{
		CPrefabNode* pNode = static_cast<CPrefabNode*>( m_pCurNode->GetParent() );

		bool bFind = false;
		uint32 nIndex = 0;
		for( auto pChild = pNode->Get_Child(); pChild; pChild = pChild->NextChild() )
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
		for( auto pChild = pNode->Get_Child(); pChild; pChild = pChild->NextChild() )
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
			m_pObject->CalcLocalBound();
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
		pRopeObject->CalcLocalBound();
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
		CMultiFrameImage2D* pRopeObject = static_cast<CMultiFrameImage2D*>( m_pNode->GetRenderObject() );
		m_pBeginFrame->SetValue( pRopeObject->GetFrameBegin() );
		m_pEndFrame->SetValue( pRopeObject->GetFrameEnd() );
		m_pFramesPerSec->SetValue( pRopeObject->GetFramesPerSec() );
	}

	void OnDataChanged()
	{
		CMultiFrameImage2D* pRopeObject = static_cast<CMultiFrameImage2D*>( m_pNode->GetRenderObject() );
		pRopeObject->SetFrames( m_pBeginFrame->GetValue<uint32>(), m_pEndFrame->GetValue<uint32>(), m_pFramesPerSec->GetValue<uint32>() );
	}
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	CReference<CCommonEdit> m_pBeginFrame;
	CReference<CCommonEdit> m_pEndFrame;
	CReference<CCommonEdit> m_pFramesPerSec;

	TClassTrigger<CMultiFrameNodeData> m_onDataChanged[3];
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
		m_pTileSize->Register( CUIElement::eEvent_Action, &m_onRefresh[2] );
		m_pHeight->Register( CUIElement::eEvent_Action, &m_onRefresh[3] );

		m_pEnableEdit = CBoolEdit::Create( "Enable Edit" );
		pView->AddContentChild( m_pEnableEdit, m_pResRoot );
		m_pCurEditType = CCommonEdit::Create( "Edit Type" );
		pView->AddContentChild( m_pCurEditType, m_pResRoot );
		m_pFill = static_cast<CUIButton*>( CResourceManager::Inst()->CreateResource<CUIResource>( "EditorRes/UI/button.xml" )->GetElement()->Clone() );
		pView->AddContentChild( m_pFill, m_pResRoot );
		for( int i = 0; i < 2; i++ )
			m_onEditTypeChanged[i].Set( this, &CTileMapNodeData::OnEditTypeChanged );
		m_onFill.Set( this, &CTileMapNodeData::OnFill );
		m_pEnableEdit->Register( CUIElement::eEvent_Action, &m_onEditTypeChanged[0] );
		m_pCurEditType->Register( CUIElement::eEvent_Action, &m_onEditTypeChanged[1] );
		m_pFill->Register( CUIElement::eEvent_Action, &m_onFill );
		m_pFill->SetText( "Fill" );
		m_bEnableEdit = false;
		m_bDragging = false;
		m_nCurEditType = 0;

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
		m_pView->RemoveContentTree( m_pResRoot );
	}
	
	virtual void OnViewportChar( uint32 nChar ) override
	{
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		
		uint32 nEditTypeCount = pTileMap->GetInfo()->editInfos.size();
		if( nChar == 'Q' || nChar == 'q' )
			m_nCurEditType = m_nCurEditType > 0 ? m_nCurEditType - 1 : nEditTypeCount - 1;
		else if( nChar == 'W' || nChar == 'w' )
			m_nCurEditType = m_nCurEditType < nEditTypeCount - 1 ? m_nCurEditType + 1 : 0;
		else if( nChar >= '0' && nChar <= '9' )
			m_nCurEditType = Min( nChar - '0', nEditTypeCount - 1 );
		else
			return;
		m_pCurEditType->SetValue( m_nCurEditType );
	}
	virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override
	{
		if( !m_bEnableEdit )
			return false;
		auto matInv = transform.Inverse();
		auto localPos = matInv.MulVector2Pos( mousePos );
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		CVector2 grid = ( localPos - pTileMap->GetBaseOffset() ) * CVector2( 1.0f / pTileMap->GetTileSize().x, 1.0f / pTileMap->GetTileSize().y );
		uint32 x = floor( grid.x + 0.5f );
		uint32 y = floor( grid.y + 0.5f );
		if( x >= 0 && x <= pTileMap->GetWidth() && y >= 0 && y <= pTileMap->GetHeight() )
		{
			pTileMap->EditTile( x, y, m_nCurEditType );
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
		CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pNode->GetRenderObject() );
		CVector2 grid = ( localPos - pTileMap->GetBaseOffset() ) * CVector2( 1.0f / pTileMap->GetTileSize().x, 1.0f / pTileMap->GetTileSize().y );
		uint32 x = floor( grid.x + 0.5f );
		uint32 y = floor( grid.y + 0.5f );
		if( x >= 0 && x <= pTileMap->GetWidth() && y >= 0 && y <= pTileMap->GetHeight() )
			pTileMap->EditTile( x, y, m_nCurEditType );
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
	}

	void OnEditTypeChanged()
	{
		m_bEnableEdit = m_pEnableEdit->IsChecked();
		m_nCurEditType = m_pCurEditType->GetValue<uint32>();
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
private:
	CUITreeView* m_pView;
	CReference<CUITreeView::CTreeViewContent> m_pResRoot;
	
	CReference<CVectorEdit> m_pTileSize;
	CReference<CVectorEdit> m_pBaseOffset;
	CReference<CCommonEdit> m_pWidth;
	CReference<CCommonEdit> m_pHeight;

	CReference<CBoolEdit> m_pEnableEdit;
	CReference<CCommonEdit> m_pCurEditType;
	CReference<CUIButton> m_pFill;

	bool m_bDragging;
	bool m_bEnableEdit;
	uint32 m_nCurEditType;
	
	TClassTrigger<CTileMapNodeData> m_onFill;
	TClassTrigger<CTileMapNodeData> m_onRefresh[4];
	TClassTrigger<CTileMapNodeData> m_onEditTypeChanged[2];
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
		
		m_obj.Load( buf, false );
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

	m_pNodeData = NULL;
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
			m_pObjectData = CObjectDataEditMgr::Inst().Create( m_pNodeView, NULL, m_pCurNode->GetObjData(), m_pCurNode->GetClassData() );
	}
	RefreshGizmo();
}

void CPrefabEditor::MoveNodeUp( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	auto pPrevItem = m_pSceneView->MoveUp( pCurNodeItem );
	if( !pPrevItem )
		return;
	CReference<CPrefabNode> pPrevNode = static_cast<CPrefabNodeTreeFolder*>( pPrevItem->pElement.GetPtr() )->m_pNode;
	pPrevNode->RemoveThis();
	pNode->GetParent()->AddChildBefore( pPrevNode, pNode );
}

void CPrefabEditor::MoveNodeDown( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	auto pNextItem = m_pSceneView->MoveDown( pCurNodeItem );
	if( !pNextItem )
		return;
	CReference<CPrefabNode> pNextNode = static_cast<CPrefabNodeTreeFolder*>( pNextItem->pElement.GetPtr() )->m_pNode;
	pNextNode->RemoveThis();
	pNode->GetParent()->AddChildAfter( pNextNode, pNode );
}

void CPrefabEditor::MoveNodeLeft( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	if( !pCurNodeItem->pParent || !pCurNodeItem->pParent->pParent )
		return;
	auto pParent = m_pSceneView->MoveLeft( pCurNodeItem );
	if( !pParent )
		return;
	CReference<CPrefabNode> pTempNode = pNode;
	CReference<CPrefabNode> pParentNode = static_cast<CPrefabNodeTreeFolder*>( pParent->pElement.GetPtr() )->m_pNode;
	CReference<CPrefabNode> pParentParentNode = static_cast<CPrefabNodeTreeFolder*>( pParent->pParent->pElement.GetPtr() )->m_pNode;
	pNode->RemoveThis();
	pParentParentNode->AddChildBefore( pNode, pParentNode );
}

void CPrefabEditor::MoveNodeRight( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem )
{
	auto pParent = m_pSceneView->MoveRight( pCurNodeItem );
	if( !pParent )
		return;
	CReference<CPrefabNode> pTempNode = pNode;
	CReference<CPrefabNode> pParentNode = static_cast<CPrefabNodeTreeFolder*>( pParent->pElement.GetPtr() )->m_pNode;
	pNode->RemoveThis();
	pParentNode->AddChild( pNode );
}

void CPrefabEditor::OnCurNodeNameChanged()
{
	m_pCurNode->m_strName = UnicodeToUtf8( m_pNodeName->GetText() );
	m_pCurNodeItem->pElement->GetChildByName<CUIButton>( "label" )->SetText( m_pCurNode->m_strName.c_str() );
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
	if( !m_pCurNode->SetResource( pResource ) )
	{
		CResource* pResource = m_pCurNode->GetResource();
		m_pResFileName->SetText( pResource ? pResource->GetName() : "" );
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
}

void CPrefabEditor::OnCurNodeClassChanged()
{
	m_pObjectData = NULL;
	auto pData = (SClassMetaData*)m_pNodeClass->GetSelectedItem()->pData;
	m_pCurNode->SetClassName( pData ? pData->strClassName.c_str() : NULL );
	if( m_pCurNode->GetClassData() )
		m_pObjectData = CObjectDataEditMgr::Inst().Create( m_pNodeView, NULL, m_pCurNode->GetObjData(), m_pCurNode->GetClassData() );
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