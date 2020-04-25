#pragma once
#include "Editor/Editors/ObjectDataEdit.h"

class CLevelEdit : public CObjectDataEdit
{
public:
	CLevelEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName = NULL );

	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem, const CMatrix2D& transform ) override;
	virtual CObjectDataEditItem* OnViewportStartDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual void OnViewportDragged( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
	virtual void OnViewportStopDrag( class CUIViewport* pViewport, const CVector2& mousePos, const CMatrix2D& transform ) override;
private:
	int32 GetCurOprValueCount();
	void UpdateDrag( class CUIViewport* pViewport, const TVector2<int32>& p, const CMatrix2D& transform );
	virtual void OnEdit( uint32 nParam ) override;
	int8 m_nObjType;
	int8 m_nDragType;
	uint32 m_nDragIndex;
	CVector2 m_dragBegin;
	CVector2 m_dragOfs;
	int32 m_nWidth, m_nHeight;

	int8 m_nCurSelectedOpr;
	int8 m_nCurSelectedValue;
};