#pragma once

#include "Editor/Editors/ResourceEditor.h"
#include "Game/Face.h"
#include "Game/Stage.h"
#include "Game/GUI/FaceView.h"
#include "UICommon/UITreeView.h"
#include "UICommon/UIButton.h"
#include "Editor/Editors/UIComponentUtil.h"

class CFaceEditor : public TResourceEditor<CFaceData>
{
	typedef TResourceEditor<CFaceData> Super;
public:
	CFaceEditor() : m_pStage( NULL ) {}

	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CFaceEditor )
protected:
	virtual void OnInited() override;
	virtual void CreateViewport() override;
	virtual void RefreshPreview() override;
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) override;

	virtual void OnSetVisible( bool bVisible ) override;

	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportDragged( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportChar( uint32 nChar ) override;
private:
	CReference<CFace> m_pFace;
	CReference<CPrefab> m_pFacePrefab;
	CReference<CFaceView> m_pFaceView;

	CReference<CUIElement> m_pCreatePanel;
	CReference<CUIElement> m_pEditPanel;

	CStage* m_pStage;

	CReference<CFileNameEdit> m_pFileName;
	void OnCreateOK();

	class CFaceEditItemUI : public CUIButton
	{
	public:
		static CFaceEditItemUI* Create( CFaceEditor* pOwner, CFaceEditItem* pItem );

		CFaceEditItem* GetItem() { return m_pItem; }
	protected:
		virtual void OnInited() override
		{
			m_pName = GetChildByName<CUILabel>( "name" );
		}
		void Refresh( CFaceEditItem* pItem )
		{
			m_pItem = pItem;
			m_pName->SetText( pItem->strName.c_str() );
		}
		virtual void OnClick( const CVector2& mousePos ) override
		{
			m_pOwner->SetSelectedEditItem( this );
		}

		CFaceEditor* m_pOwner;
		CFaceEditItem* m_pItem;
		CReference<CUILabel> m_pName;
	};
	void RefreshToolbox();
	void ClearToolbox();
	void SetSelectedEditItem( CFaceEditItemUI* pSelected );
	CReference<CUITreeView> m_pToolView;
	CReference<CUITreeView::CTreeViewContent> m_pOrgansRoot;
	CReference<CUITreeView::CTreeViewContent> m_pSkinsRoot;
	CReference<CUITreeView::CTreeViewContent> m_pMasksRoot;
	CReference<CFaceEditItemUI> m_pSelected;
	
	TClassTrigger<CFaceEditor> m_onCreateOK;
	TClassTrigger<CFaceEditor> m_onSave;
};