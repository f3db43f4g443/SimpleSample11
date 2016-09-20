#pragma once
#include "Physics/HitProxy.h"
#include "Editor/Editors/ObjectDataEdit.h"

class CHitProxyDataEdit : public CObjectDataEdit
{
public:
	CHitProxyDataEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName = NULL );
	~CHitProxyDataEdit() { m_pHitProxy = NULL; }

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	
	virtual void RefreshData() override { CObjectDataEdit::RefreshData(); Refresh(); }
private:
	void Refresh();
	void OnChangeType();
	CReference<CObjectDataEditItem> m_pHitProxy;
	uint32 m_nCurType;

	CReference<CDropDownBox> m_dropDownBox;
	TClassTrigger<CHitProxyDataEdit> m_onChangeType;

	SHitProxy* m_pDragProxy;
	uint32 m_nDragVertIndex;
	CVector2 m_dragPos;
	CVector2 m_vertPos;
};