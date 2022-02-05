#pragma once
#include "Editor/Editors/ResourceEditor.h"
#include "Game/World.h"
#include "Game/MyLevel.h"
#include "Editor/Editors/UIComponentUtil.h"

class CWorldCfgEditor : public TResourceEditor<CWorldCfgFile>
{
	typedef TResourceEditor<CWorldCfgFile> Super;
public:
	CWorldCfgEditor() : m_nState( 0 ), m_fScale( 0 ), m_pData( NULL ), m_z( 0 ), m_nCurSelected( -1 ), m_nDragType( 0 ) {}

	virtual void NewFile( const char* szFileName ) override;
	virtual void OnOpenFile() override;
	virtual void Refresh() override;

	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CWorldCfgEditor )
protected:
	virtual void OnInited() override;
	virtual void OnInitViewport() override;
	virtual void Save() override;
	virtual void RefreshPreview() override;
	void Validate( const char* szName );
	void InitLevels( map<string, CReference<CPrefab> >& mapLevelPrefabs );
	void InitLevel( int32 nLevel, CPrefab* pPrefab );
	void Select( int32 n );
	void RefreshLevelData( int32 nLevel );
	void OnLevelDataEdit( int32 nLevel );
	void ShowLevelTool();
	void OpenLevelFile();
	int32 PickLevel( const CVector2& p );
	void AddOverlap( int32 a, int32 b );
	void RemoveOverlap( int32 a, int32 b );
	void MoveZ( bool bDown );

	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) override;
	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportDragged( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportMouseWheel( SUIMouseEvent* pEvent ) override;
	virtual void OnViewportKey( SUIKeyEvent* pEvent ) override;

	void BeginNewLevel();
	void EndNewLevel( bool bOK );
	void OnNewLevelOK() { EndNewLevel( true ); }
	void OnNewLevelCancel() { EndNewLevel( false ); }

	SWorldCfg* m_pData;
	CReference<CUIElement> m_pPanel[2];
	CReference<CFileNameEdit> m_pNewTemplate;
	CReference<CUITextBox> m_pNewLevelName;
	float m_fScale;

	struct SLevelData
	{
		SLevelData() : nFlag( 0 ), bDirty( false ) {}
		CReference<CPrefabNode> pClonedLevelData;
		CReference<CRenderObject2D> pLevelPreview;
		int8 nFlag;
		bool bDirty;
	};
	vector<SLevelData> m_vecLevelData;
	map<string, int32> m_mapLevelDataIndex;
	
	int32 m_nCurSelected;
	int8 m_nDragType;
	CVector2 m_dragBegin;
	CVector2 m_worldDragBeginPos;
	CVector2 m_curDisplayOfs0;
	int32 m_z;

	int8 m_nState;
	CReference<CPrefab> m_pNewMapTemplate;
	CVector2 m_newMapOfs;
	CRectangle m_newMapSize;

	TClassTrigger<CWorldCfgEditor> m_onSave;
	TClassTrigger<CWorldCfgEditor> m_onNewLevel;
	TClassTrigger<CWorldCfgEditor> m_onNewLevelOK;
	TClassTrigger<CWorldCfgEditor> m_onNewLevelCancel;
};