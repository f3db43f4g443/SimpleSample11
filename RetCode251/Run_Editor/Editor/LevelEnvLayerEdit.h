#pragma once
#include "MyLevel.h"
#include "Editor/Editors/ObjectDataEdit.h"

class CLevelEnvLayerEdit : public CObjectDataEdit
{
public:
	CLevelEnvLayerEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName = NULL )
		: CObjectDataEdit( pTreeView, pParent, pData, pMetaData, szName ) {}
	~CLevelEnvLayerEdit() { m_pHitProxy = NULL; }

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;

	virtual void RefreshData() override { CObjectDataEdit::RefreshData(); Refresh(); }
private:
	void DebugDrawCtrlPoint( IRenderSystem* pRenderSystem, CUIViewport* pViewport, const CMatrix2D & transform, const SLevelCamCtrlPoint& p, int8 nType );
	void DebugDrawCtrlLink( IRenderSystem* pRenderSystem, CUIViewport* pViewport, const CMatrix2D & transform, const SLevelCamCtrlPointLink& l, const SLevelCamCtrlPoint& p1, const SLevelCamCtrlPoint& p2 );
	void Refresh();
	CReference<CObjectDataEditItem> m_pHitProxy;
	uint32 m_nCurType;

	CReference<CDropDownBox> m_dropDownBox;
	TClassTrigger<CLevelEnvLayerEdit> m_onChangeType;

	SHitProxy* m_pDragProxy;
	uint32 m_nDragVertIndex;
	CVector2 m_dragPos;
	CVector2 m_vertPos;
};