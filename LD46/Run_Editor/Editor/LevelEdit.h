#pragma once
#include "Editor/Editors/ObjectDataEdit.h"

class CLevelEdit : public CObjectDataEdit
{
public:
	CLevelEdit( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, uint8* pData, SClassMetaData* pMetaData, const char* szName = NULL );
private:
	void OnLevelTools();
	void OnWorld();
	TClassTrigger<CLevelEdit> m_onLevelTools;
	TClassTrigger<CLevelEdit> m_onWorld;
};