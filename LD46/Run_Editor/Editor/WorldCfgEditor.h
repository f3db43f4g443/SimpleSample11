#pragma once
#include "Editor/Editors/ResourceEditor.h"
#include "Game/Stage.h"
#include "Game/World.h"
#include "Game/MyLevel.h"
#include "UICommon/UITreeView.h"
#include "UICommon/UIButton.h"
#include "Editor/Editors/UIComponentUtil.h"
#include "LevelEdit.h"

class CWorldCfgEditor : public TResourceEditor<CWorldCfgFile>
{
	typedef TResourceEditor<CWorldCfgFile> Super;
public:
	CWorldCfgEditor() : m_pData( NULL ) {}

	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CWorldCfgEditor )
protected:
	virtual void OnInited() override;
	virtual void Save() override;
	virtual void RefreshPreview() override;
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) override;
	void AutoLayout();
	void Validate( const char* szName );
	void Select( int32 n );
	void OnLevelDateEdit();
	void RefreshLevelDataLink( int32 n );

	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportDragged( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportChar( uint32 nChar ) override;

	SWorldCfg* m_pData;
	struct SLevelData
	{
		SLevelData() : nFlag( 0 ) {}
		CReference<CPrefabNode> pClonedLevelData;
		CReference<CMyLevel> pLevelPreview;
		struct SLink
		{
			int32 nTarget;
			vector<TVector2<int32> > vecBegin;
		};
		vector<SLink> vecLinks;
		int8 nFlag;
	};
	vector<SLevelData> m_vecLevelData;
	CReference<CUITreeView> m_pTreeView;
	CReference<CLevelEdit> m_pCurLevelEdit;
	CReference<CObjectDataEditItem> m_pCurDragEdit;

	int32 m_nCurSelected;
	int8 m_nDragType;
	CVector2 m_worldDragBeginPos;
	CVector2 m_curDisplayOfs0;

	TClassTrigger<CWorldCfgEditor> m_onSave;
	TClassTrigger<CWorldCfgEditor> m_onAutoLayout;
	TClassTrigger<CWorldCfgEditor> m_onLvDataEdit;

	TClassTrigger1<CWorldCfgEditor, const char*> m_onAddOK;
};