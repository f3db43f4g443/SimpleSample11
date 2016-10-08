#pragma once
#include "UICommon/UITreeView.h"
#include "ResourceEditor.h"
#include "Render/Prefab.h"
#include "UIComponentUtil.h"
#include "ObjectDataEdit.h"

class CPrefabEditor : public TResourceEditor<CPrefab>
{
	typedef TResourceEditor<CPrefab> Super;
	friend class CPrefabNodeTreeFolder;
public:
	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;

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

	void Copy();
	void Paste();

	void MoveNodeUp( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void MoveNodeDown( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void MoveNodeLeft( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );
	void MoveNodeRight( CPrefabNode* pNode, CUITreeView::CTreeViewContent* pCurNodeItem );

	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportDragged( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportChar( uint32 nChar ) override;
private:
	void OnCurNodeNameChanged();
	void OnCurNodeResourceChanged();
	void OnCurNodeClassChanged();
	void OnTransformChanged();
	void OnZOrderChanged();
	void RefreshSceneView( CPrefabNode* pParNode, CUITreeView::CTreeViewContent* pParNodeItem );
	void RefreshGizmo();
	void RefreshCurNodeTransformByGizmo();

	CReference<CUITreeView> m_pSceneView;
	CReference<CUITreeView> m_pNodeView;
	CReference<CPrefabNode> m_pClonedPrefab;
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

	uint8 m_nDragType;
	CVector2 m_gizmoOrigPos;
	CTransform2D m_origTransform;
	CReference<CObjectDataEditItem> m_pCurDragEdit;
	TClassTrigger<CPrefabEditor> m_onNodeNameChanged;
	TClassTrigger<CPrefabEditor> m_onNodeResourceChanged;
	TClassTrigger<CPrefabEditor> m_onNodeClassChanged;
	TClassTrigger<CPrefabEditor> m_onTransformChanged;
	TClassTrigger<CPrefabEditor> m_onZOrderChanged;
	TClassTrigger<CPrefabEditor> m_onNew;
	TClassTrigger<CPrefabEditor> m_onDelete;
	TClassTrigger<CPrefabEditor> m_onCopy;
	TClassTrigger<CPrefabEditor> m_onPaste;
	TClassTrigger<CPrefabEditor> m_onSave;
};