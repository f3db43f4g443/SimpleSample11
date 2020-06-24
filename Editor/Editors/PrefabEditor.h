#pragma once
#include "UICommon/UITreeView.h"
#include "ResourceEditor.h"
#include "Render/Prefab.h"
#include "UIComponentUtil.h"
#include "ObjectDataEdit.h"


class CDragDropObjRef : public CUILabel
{
public:
	static CDragDropObjRef* Create( class CPrefabNode* pNode );
	CPrefabNode* GetNode() { return m_pNode; }
private:
	CReference<CPrefabNode> m_pNode;
};

class CPrefabEditor : public TResourceEditor<CPrefab>
{
	typedef TResourceEditor<CPrefab> Super;
	friend class CPrefabListItem;
	friend class CPrefabNodeTreeFolder;
public:
	CPrefabEditor() : m_strSelectedPrefab( "" ) {}
	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;
	CPrefabNode* GetPrefabNode() { return m_pSelectedPrefab; }
	CPrefabNode* GetRootNode( const char* szName = "" );
	CPrefabNode* GetCurNode() { return m_pCurNode; }
	void RefreshCurItem() { SelectItem( m_strSelectedPrefab ); }

	class CNodeData : public CReferenceObject
	{
	public:
		CNodeData( CPrefabEditor* pOwner, CPrefabNode* pNode )
			: m_onResourceRefreshBegin( this, &CNodeData::OnResourceRefreshBegin )
			, m_onResourceRefreshEnd( this, &CNodeData::OnResourceRefreshEnd )
			, m_pOwner( pOwner )
			, m_pNode( pNode )
		{
			if( pNode->GetResource() )
			{
				pNode->GetResource()->RegisterRefreshBegin( &m_onResourceRefreshBegin );
				pNode->GetResource()->RegisterRefreshEnd( &m_onResourceRefreshEnd );
			}
		}

		~CNodeData()
		{
			if( m_onResourceRefreshBegin.IsRegistered() )
				m_onResourceRefreshBegin.Unregister();
			if( m_onResourceRefreshEnd.IsRegistered() )
				m_onResourceRefreshEnd.Unregister();
		}
		
		virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) {}
		virtual void OnViewportChar( uint32 nChar ) {}
		virtual bool OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) { return false; }
		virtual bool OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) { return false; }
		virtual bool OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) { return false; }
	protected:
		virtual void OnResourceRefreshBegin() {}
		virtual void OnResourceRefreshEnd() { m_pOwner->OnCurNodeResourceChanged(); }
		
		CPrefabEditor* m_pOwner;
		CPrefabNode* m_pNode;
		TClassTrigger<CNodeData> m_onResourceRefreshBegin;
		TClassTrigger<CNodeData> m_onResourceRefreshEnd;
	};

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CPrefabEditor )
protected:
	virtual void OnInited() override;
	virtual void RefreshPreview() override;
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) override;

	void NewNode();
	void DeleteNode();
	void SelectNode( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void SelectItem( const char* szItem );

	void Copy();
	void Paste();

	void NewItem();
	void CloneItem();
	void RenameItem();
	void DeleteItem();

	void MoveNodeUp( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void MoveNodeDown( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void MoveNodeLeft( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void MoveNodeRight( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );

	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportDragged( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportDrop( const CVector2& mousePos, CUIElement* pParam ) override;
	virtual void OnViewportKey( SUIKeyEvent* pEvent ) override;
	virtual void OnViewportChar( uint32 nChar ) override;
private:
	void OnCurNodeNameChanged();
	void OnCurNodeResourceChanged();
	void OnCurNodeClassChanged();
	void OnCurNodeObjDataChanged( int32 nAction );
	void OnTransformChanged();
	void OnZOrderChanged();
	void RefreshItemView();
	void RefreshSceneView( CPrefabNode* pParNode, CUITreeView::CTreeViewContent* pParNodeItem, int8 nInsertType = 0 );
	void RefreshGizmo();
	void RefreshCurNodeTransformByGizmo();

	void CreatePatchNode( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void DestroyPatchNode( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );

	CReference<CUITreeView> m_pSceneView;
	CReference<CUIScrollView> m_pItemView;
	CReference<CUITreeView> m_pNodeView;
	CString m_strSelectedPrefab;
	CReference<CPrefabNode> m_pSelectedPrefab;
	map<CString, CReference<CPrefabNode> > m_mapClonedPrefabs;
	CReference<CUITextBox> m_pItemName;

	CReference<CPrefabNode> m_pCurNode;
	CReference<CUITreeView::CTreeViewContent> m_pCurNodeItem;
	CReference<CRenderObject2D> m_pGizmo;
	
	CReference<CDropDownBox> m_pNewNodeType;
	CReference<CCommonEdit> m_pNodeName;
	CReference<CFileNameEdit> m_pResFileName;
	CReference<CResource> m_pNodeResource;
	CReference<CDropDownBox> m_pNodeClass;

	CReference<CVectorEdit> m_pTransform;
	CReference<CCommonEdit> m_pZOrder;
	CReference<CNodeData> m_pNodeData;
	CReference<CObjectDataEditItem> m_pObjectData;

	CReference<CPrefabNode> m_pClipBoard;

	uint8 m_nNodeDebugDrawType;
	uint8 m_nDragType;
	CVector2 m_gizmoOrigPos;
	CTransform2D m_origTransform;
	CReference<CObjectDataEditItem> m_pCurDragEdit;
	TClassTrigger<CPrefabEditor> m_onNodeNameChanged;
	TClassTrigger<CPrefabEditor> m_onNodeResourceChanged;
	TClassTrigger<CPrefabEditor> m_onNodeClassChanged;
	TClassTrigger1<CPrefabEditor, int32> m_onNodeObjDataChanged;
	TClassTrigger<CPrefabEditor> m_onTransformChanged;
	TClassTrigger<CPrefabEditor> m_onZOrderChanged;
	TClassTrigger<CPrefabEditor> m_onNew;
	TClassTrigger<CPrefabEditor> m_onDelete;
	TClassTrigger<CPrefabEditor> m_onCopy;
	TClassTrigger<CPrefabEditor> m_onPaste;
	TClassTrigger<CPrefabEditor> m_onSave;
	TClassTrigger<CPrefabEditor> m_onNewItem;
	TClassTrigger<CPrefabEditor> m_onCloneItem;
	TClassTrigger<CPrefabEditor> m_onRenameItem;
	TClassTrigger<CPrefabEditor> m_onDeleteItem;
};