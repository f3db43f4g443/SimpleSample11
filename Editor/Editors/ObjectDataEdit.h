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

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) {}
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) { return NULL; }
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) {}
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) {}

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
	CObjectDataBoolEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData );
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
	CObjectDataCommonEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData );
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
	CObjectDataVectorEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData );
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
	CObjectDataStringEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData );
	~CObjectDataStringEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	void OnEdit();
	CReference<CCommonEdit> m_pEdit;
	TClassTrigger<CObjectDataStringEdit> m_onEdit;
};

class CObjectDataEnumEdit : public CObjectDataEditItem
{
public:
	CObjectDataEnumEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData::SMemberData* pMetaData );
	~CObjectDataEnumEdit() { if( m_onEdit.IsRegistered() ) m_onEdit.Unregister(); }
	virtual void RefreshData() override;
private:
	void OnEdit();
	CReference<CDropDownBox> m_pEdit;
	TClassTrigger<CObjectDataEnumEdit> m_onEdit;
};

class CObjectDataEdit : public CObjectDataEditItem
{
public:
	CObjectDataEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName = NULL );
	~CObjectDataEdit();
	virtual void RefreshData() override;

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
private:
	void OnEdit( uint32 nParam )
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