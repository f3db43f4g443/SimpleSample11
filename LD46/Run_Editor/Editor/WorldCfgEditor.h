#pragma once
#include "Editor/Editors/ResourceEditor.h"
#include "Game/Stage.h"
#include "Game/World.h"
#include "Game/MyLevel.h"
#include "UICommon/UITreeView.h"
#include "UICommon/UIButton.h"
#include "Editor/Editors/UIComponentUtil.h"
#include <set>

class CWorldCfgEditor : public TResourceEditor<CWorldCfgFile>
{
	typedef TResourceEditor<CWorldCfgFile> Super;
public:
	CWorldCfgEditor() : m_fScale( 0 ), m_pData( NULL ), m_nState( 0 ) {}

	virtual void NewFile( const char* szFileName ) override;
	virtual void Refresh() override;

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CWorldCfgEditor )
protected:
	virtual void OnInited() override;
	virtual void OnInitViewport() override {}
	virtual void Save() override;
	virtual void RefreshPreview() override;
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) override;
	void AutoLayout();
	void Validate( const char* szName );
	void InitRegion( int32 nRegion, map<string, CReference<CPrefab> >& mapLevelPrefabs );
	void InitLevel( int32 nRegion, int32 nLevel, CPrefab* pPrefab );
	void SelectRegion( int32 n );
	void Select( int32 n );
	void OnLevelDataEdit( int32 nRegion, int32 nLevel );
	void RefreshLevelDataLink( int32 nRegion, int32 n, map<CString, TVector2<int32> >* pMap = NULL );
	void RefreshExtLevel( int32 nRegion );
	void ShowLevelTool();
	void BeginNewLevel();
	void EndNewLevel( bool bOK );
	void OnNewLevelOK() { EndNewLevel( true ); }
	void OnNewLevelCancel() { EndNewLevel( false ); }
	void OnBlueprintChange();

	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportDragged( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportMouseWheel( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportKey( SUIKeyEvent* pEvent ) override;
	void OnRegionSelectChanged();

	SWorldCfg* m_pData;

	struct SLevelData
	{
		SLevelData() : nFlag( 0 ), bDirty( false ) {}
		CReference<CPrefabNode> pClonedLevelData;
		CReference<CRenderObject2D> pLevelPreview;
		struct SLink
		{
			int32 nRegion;
			int32 nTarget;
			vector<TVector2<int32> > vecBegin;
		};
		vector<SLink> vecLinks;
		int8 nFlag;
		bool bDirty;
	};
	struct SRegionData
	{
		CReference<CRenderObject2D> pRoot;
		CReference<CRenderObject2D> pBack;
		vector<SLevelData> vecLevelData;
		struct SExtLevelData
		{
			int32 nRegion;
			int32 nLevel;
			CVector2 ofs;
		};
		vector<SExtLevelData> vecExtLevel;
	};
	vector<SRegionData> m_vecRegionData;
	CReference<CUIElement> m_pPanel[2];
	CReference<CDropDownBox> m_pRegionSelect;
	CReference<CFileNameEdit> m_pNewTemplate;
	CReference<CFileNameEdit> m_pBlueprint;
	CReference<CUITextBox> m_pNewLevelName;
	float m_fScale;

	int32 m_nCurRegion;
	int32 m_nCurSelected;
	int8 m_nDragType;
	CVector2 m_worldDragBeginPos;
	CVector2 m_curDisplayOfs0;

	int8 m_nState;

	CReference<CPrefab> m_pNewMapTemplate;
	CVector2 m_newMapBase;
	struct _SLessTile
	{
		bool operator () ( const TVector2<int32>& a, const TVector2<int32>& b ) const
		{
			return memcmp( &a, &b, sizeof( a ) ) < 0;
		}
	};
	set<TVector2<int32>, _SLessTile> m_newMapTiles;

	TClassTrigger<CWorldCfgEditor> m_onBeginNewLevel;
	TClassTrigger<CWorldCfgEditor> m_onSelectRegion;
	TClassTrigger<CWorldCfgEditor> m_onSave;
	TClassTrigger<CWorldCfgEditor> m_onAutoLayout;
	TClassTrigger<CWorldCfgEditor> m_onNewLevel;
	TClassTrigger<CWorldCfgEditor> m_onNewLevelOK;
	TClassTrigger<CWorldCfgEditor> m_onNewLevelCancel;
	TClassTrigger<CWorldCfgEditor> m_onBlueprintChange;

	TClassTrigger1<CWorldCfgEditor, const char*> m_onAddOK;
};