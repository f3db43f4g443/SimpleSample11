#pragma once
#include "UIComponentUtil.h"
#include "Common/ClassMetaData.h"

class CObjectDataEditItem : public CReferenceObject
{
public:
	CObjectDataEditItem( CUITreeView* pTreeView, uint8* pData )
		: m_pTreeView( pTreeView ), m_pData( pData ) {}
	~CObjectDataEditItem() { if( m_pContent ) m_pTreeView->RemoveContentTree( m_pContent ); }
	CUITreeView::CTreeViewContent* GetContent() { return m_pContent; }
	virtual void RefreshData() {}

	void TreeViewFocus();
	virtual CObjectDataEditItem* GetChildItem( uint8* pData ) { if( m_pData == pData ) return this; return NULL; }
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) {}
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) { return NULL; }
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) {}
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) {}
	virtual CObjectDataEditItem* OnViewportDrop( class CUIViewport* pViewport, const CVector2& mousePos, CUIElement* pParam, const CMatrix2D& transform ) { return NULL; }

	void Register( CTrigger* pTrigger ) { m_pContent->pElement->Register( CUIElement::eEvent_Action, pTrigger ); }
protected:
	CUITreeView* m_pTreeView;
	CReference<CUITreeView::CTreeViewContent> m_pContent;
	uint8* m_pData;

	LINK_LIST_REF( CObjectDataEditItem, Item );
};

class CObjectDataBoolEdit : public CObjectDataEditItem
{
public:
	CObjectDataBoolEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName = NULL );
	~CObjectDataBoolEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	void OnEdit();
	CReference<CBoolEdit> m_pEdit;
	TClassTrigger<CObjectDataBoolEdit> m_onEdit;
};

class CObjectDataCommonEdit : public CObjectDataEditItem
{
public:
	CObjectDataCommonEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName = NULL );
	~CObjectDataCommonEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	uint8 m_nDataType;
	void OnEdit();
	CReference<CCommonEdit> m_pEdit;
	TClassTrigger<CObjectDataCommonEdit> m_onEdit;
};

class CObjectDataVectorEdit : public CObjectDataEditItem
{
public:
	CObjectDataVectorEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName = NULL );
	~CObjectDataVectorEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	uint8 m_nDataType;
	void OnEdit();
	CReference<CVectorEdit> m_pEdit;
	TClassTrigger<CObjectDataVectorEdit> m_onEdit;
};

class CObjectDataStringEdit : public CObjectDataEditItem
{
public:
	CObjectDataStringEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName = NULL );
	~CObjectDataStringEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	void OnEdit();
	CReference<CUILabel> m_pEdit;
	TClassTrigger<CObjectDataStringEdit> m_onEdit;
};

class CObjectDataEnumEdit : public CObjectDataEditItem
{
public:
	CObjectDataEnumEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName = NULL );
	~CObjectDataEnumEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	void OnEdit();
	CReference<CDropDownBox> m_pEdit;
	TClassTrigger<CObjectDataEnumEdit> m_onEdit;
};

class CObjectDataObjRefEdit : public CObjectDataEditItem
{
public:
	CObjectDataObjRefEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName = NULL );
	~CObjectDataObjRefEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	void OnEdit( CUIElement* pParam );
	CReference<CDropTargetEdit> m_pEdit;
	TClassTrigger1<CObjectDataObjRefEdit, CUIElement*> m_onEdit;
};

class CObjectArrayEdit : public CObjectDataEditItem
{
	friend class CTreeFolderArrayEdit;
public:
	CObjectArrayEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData, const char* szName = NULL );
	~CObjectArrayEdit();
	virtual void RefreshData() override;

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* GetChildItem( uint8* pData ) override;;
private:
	void OnEdit( uint32 nParam )
	{
		if( nParam != 1 )
			return;
		if( m_pContent->pParent )
			m_pContent->pParent->pElement->Action( (void*)1 );
	}
	void OnResize( int32 nSize );
	void CreateItemEdit();
	TClassTrigger1<CObjectArrayEdit, uint32> m_onEdit;
	SClassMetaData::SMemberData* m_pMemberData;
	LINK_LIST_REF_HEAD( m_pChildren, CObjectDataEditItem, Item );
};

class CObjectDataEdit : public CObjectDataEditItem
{
public:
	CObjectDataEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName = NULL );
	~CObjectDataEdit();
	virtual void RefreshData() override;

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* OnViewportDrop( class CUIViewport* pViewport, const CVector2& mousePos, CUIElement* pParam, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* GetChildItem( uint8* pData ) override;
protected:
	virtual void OnEdit( uint32 nParam )
	{ 
		if( nParam != 1 )
			return;
		if( m_pContent->pParent )
			m_pContent->pParent->pElement->Action( (void*)1 );
	}
	LINK_LIST_REF_HEAD( m_pChildren, CObjectDataEditItem, Item );
	TClassTrigger1<CObjectDataEdit, uint32> m_onEdit;
};

class CObjectDataEditMgr
{
public:
	template<class T, class T1>
	void Register()
	{
		m_mapCreateFuncs[typeid( T ).name()] = [] ( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName )
		{ return new T1( pTreeView, pParent, pData, pMetaData, szName ); };
	}
	CObjectDataEditItem* Create( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName = NULL );

	DECLARE_GLOBAL_INST_REFERENCE( CObjectDataEditMgr )
private:
	map<string, function<CObjectDataEditItem*( CUITreeView*, CUITreeView::CTreeViewContent*, uint8*, SClassMetaData*, const char* )> > m_mapCreateFuncs;
};