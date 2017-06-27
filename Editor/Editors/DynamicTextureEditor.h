#pragma once
#include "UICommon/UITextBox.h"
#include "UICommon/UIViewport.h"
#include "Render/Canvas.h"
#include "UIComponentUtil.h"
#include "ResourceEditor.h"

class CDynamicTextureEditor : public TResourceEditor<CDynamicTexture>
{
	typedef TResourceEditor<CDynamicTexture> Super;
public:
	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CDynamicTextureEditor )
protected:
	virtual void OnInited() override;
	virtual void RefreshPreview() override;
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) override;

	void ExportBaseTex();

	CReference<CCommonEdit> m_pWidth;
	CReference<CCommonEdit> m_pHeight;
	CReference<CFileNameEdit> m_pPrefabName;
	CReference<CFileNameEdit> m_pBaseTexName;
	CReference<CCommonEdit> m_pExportFileName;

	TClassTrigger<CDynamicTextureEditor> m_onExportBaseTex;
	TClassTrigger<CDynamicTextureEditor> m_onRefreshPreview;
	TClassTrigger<CDynamicTextureEditor> m_onSave;
};