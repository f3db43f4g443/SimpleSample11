#pragma once
#include "LevelGenerate.h"
#include "UICommon/UITreeView.h"
#include "UICommon/UIViewport.h"
#include "UICommon/UITextBox.h"
#include "GUI/UIUtils.h"

class CChunkPreview : public CEntity
{
public:
	CChunkPreview() : m_pChunk( NULL ), m_region( 0, 0, 0, 0 ) { SET_BASEOBJECT_ID( CChunkPreview ); bVisible = false; }
	CChunkPreview( const SClassCreateContext& context ) : CEntity( context ), m_pChunk( NULL ) { SET_BASEOBJECT_ID( CChunkPreview ); }
	virtual void OnRemovedFromStage() override { Clear(); }
	CLevelGenerateNode* GetNode() { return m_pNode; }
	const TRectangle<int32>& GetRegion() { return m_region; }
	virtual CEntity* GetPreviewRoot();
	virtual void Set( CLevelGenerateNode* pNode, const TRectangle<int32>& region, int8* pData = NULL, int32 nDataSize = 0 );
	virtual void Clear();
protected:
	CReference<CLevelGenerateNode> m_pNode;
	TRectangle<int32> m_region;
	SChunk* m_pChunk;
	CReference<CChunkObject> m_pChunkObject;
};

class CChunkEdit : public CChunkPreview
{
	friend void RegisterGameClasses();
public:
	CChunkEdit( const SClassCreateContext& context ) : CChunkPreview( context ), m_bTempEdit( false ), m_bEditValid( false ) { SET_BASEOBJECT_ID( CChunkEdit ); }
	virtual void Set( CLevelGenerateNode* pNode, const TRectangle<int32>& region, int8* pData = NULL, int32 nDataSize = 0 ) override;
	void SetTempEdit( bool bTempEdit );

	virtual CEntity* GetPreviewRoot() override;
	bool IsEditValid() { return m_bEditValid; }
	void Check();
private:
	CReference<CRenderObject2D> m_pFrameImg[8];

	bool m_bTempEdit;
	bool m_bEditValid;
};

struct SLevelDesignItem : public CReferenceObject
{
	SLevelDesignItem() : bLocked( false ) {}
	~SLevelDesignItem()
	{
		if( pEntity )
			pEntity->SetParentEntity( NULL );
		onRemoved.Trigger( 0, this );
	}

	string strFullName;
	CReference<CLevelGenerateNode> pGenNode;
	CReference<CChunkEdit> pEntity;
	TRectangle<int32> region;
	vector<int8> vecData;
	string strChunkName;
	bool bLocked;
	CEventTrigger<1> onRemoved;
};

struct SLevelDesignContext
{
	SLevelDesignContext( uint32 nWidth, uint32 nHeight ) : nWidth( nWidth ), nHeight( nHeight ), bInited( false )
	{
		items[0].resize( nWidth * nHeight );
		items[1].resize( nWidth * nHeight );
	}
	void Init();

	SLevelDesignItem* AddItem( CLevelGenerateNode* pNode, const TRectangle<int32>& region, bool bAutoErase );
	void RemoveItem( SLevelDesignItem* pItem );
	virtual SLevelDesignItem* Add( const char* szFullName, const TRectangle<int32>& region, IBufReader* pExtraBuffer = NULL );
	virtual void Remove( SLevelDesignItem* pItem ) { RemoveItem( pItem ); }

	CLevelGenerateNode* FindNode( const char* szFullName );
	void GenerateLevel( class CMyLevel* pLevel );

	void New();
	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	uint32 nWidth, nHeight;
	vector<CReference<SLevelDesignItem> > items[2];

	map<string, CReference<CLevelGenerateNode> > mapGenerateNodes;
	bool bInited;
};

class CChunkDetailEdit : public CEntity
{
	friend void RegisterGameClasses();
public:
	CChunkDetailEdit( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CChunkDetailEdit ); }

	void Set( SLevelDesignItem* pItem );
	void Edit( int32 x, int32 y, int32 nType );
	void Refresh();
private:
	void RefreshGrid( int32 x, int32 y );
	CReference<CRenderObject2D> m_pFrameImg[8];
	TResourceRef<CDrawableGroup> m_pGridDrawable;
	uint32 m_nGridTexCols, m_nGridTexRows;

	CReference<SLevelDesignItem> m_pCurItem;
	vector<CReference<CRenderObject2D> > m_vecGrids;
	bool m_bDirty;
};

class CDesignLevel : public CEntity, public SLevelDesignContext
{
	friend class CDesignView;
	friend void RegisterGameClasses();
public:
	CDesignLevel( const SClassCreateContext& context ) : CEntity( context ), m_bInited( false )
		, m_bBeginEdit( false ), m_bAutoErase( false ), m_nBrushSize( 1 ), m_brushDims( 0, 0 ), m_nShowLevelType( 3 )
		, SLevelDesignContext( 32, 128 ), m_strChunkEditPrefab( context ), m_curEditPos( 0, 0 ) { SET_BASEOBJECT_ID( CDesignLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual SLevelDesignItem* Add( const char* szFullName, const TRectangle<int32>& region, IBufReader* pExtraBuffer = NULL ) override;
	void CreatePreviewForItem( SLevelDesignItem* pItem );
	void SetShowLevelType( uint8 nType );
	void ToggloShowEditLevel();
	bool IsAutoErase() { return m_bAutoErase; }
	void SetAutoErase( bool bAutoErase );

	SLevelDesignItem* GetItemByGrid( uint8 nLevel, const TVector2<int32> grid );
	SLevelDesignItem* GetItemByGrid( const TVector2<int32> grid, bool bPick = false );
	SLevelDesignItem* GetItemByWorldPos( uint8 nLevel, const CVector2& worldPos );
	SLevelDesignItem* GetItemByWorldPos( const CVector2& worldPos, bool bPick = false );
	CRectangle GetBrushRect();
	uint8 GetBrushSize() { return m_nBrushSize; }
	void SetBrushSize( uint8 nSize );
	CChunkDetailEdit* GetDetailEdit() { return m_pDetailEdit; }

	TVector2<int32> CalcBrushDims( CLevelGenerateNode* pNode );
	TRectangle<int32> CalcEditRegion( CLevelGenerateNode* pNode, CVector2 begin, CVector2 end );
	bool SetEditNode( const char* szNode );
	void RefreshBrush();
	void RefreshBrushEditingFence();
	bool IsEditValid();
	void ApplyBrush();
	bool BeginEdit( const CVector2& worldPos );
	void UpdateEdit( const CVector2& worldPos );
	void EndEdit();
	void StopEdit();
	void ClearLockedItems();

	CEntity* GetChunkRoot( uint8 nLevel ) { return m_pChunkRoot[nLevel - 1]; }

	static CDesignLevel* GetInst() { return s_pLevel; }
private:
	bool m_bInited;
	uint8 m_nShowLevelType;

	CString m_strChunkEditPrefab;
	CReference<CPrefab> m_pChunkEditPrefab;

	CReference<CEntity> m_pChunkRoot[3];
	CReference<CEntity> m_pChunkEditRoot[3];
	CReference<CChunkDetailEdit> m_pDetailEdit;

	CVector2 m_curEditPos;
	bool m_bBeginEdit;
	uint8 m_nBrushSize;
	bool m_bAutoErase;
	TVector2<int32> m_brushDims;
	vector<CReference<CChunkEdit> > m_vecTempChunkEdits;
	string m_strEditNodeName;
	CReference<CLevelGenerateNode> m_pEditNode;
	string m_strPendingEditNodeName;
	CReference<CLevelGenerateNode> m_pPendingEditNode;
	CVector2 m_editBeginPos;
	vector<CReference<SLevelDesignItem> > m_vecLockedItems;

	static CDesignLevel* s_pLevel;
};

class CDesignView : public CUIElement
{
public:
	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CDesignView )

	CDesignView() : m_pSelectedFile( NULL ), m_pSelectedNode( NULL ), m_pSelectedItem( NULL ), m_bIsDeleteMode( false ), m_bHelp( false ), m_onSelectedItemDeleted( this, &CDesignView::OnSelectedItemDeleted ) {}

	void OnHelp();
	void OnNew() { SelectItem( NULL ); CDesignLevel::GetInst()->New(); }
	void OnLoad();
	void OnSave();

	void Load( const char* szFileName );
	void Save( const char* szFileName );

	void SelectFile( class CDesignViewFileElem* pItem );
	void SelectNode( class CDesignViewNodeElem* pItem );
	void SetDeleteMode( bool bDelete ) { m_bIsDeleteMode = bDelete; FormatStateText(); }
	void SelectItem( SLevelDesignItem* pItem );
	void OnPickItem( SLevelDesignItem* pItem, const CVector2& pos );

	bool IsDeleteMode() { return m_bIsDeleteMode; }
	class CDesignViewNodeElem* GetSelectedNode() { return !m_bIsDeleteMode ? m_pSelectedNode : NULL; }

	void OnDesignVisible( bool bVisible );
protected:
	virtual void OnInited() override;
	virtual void OnChar( uint32 nChar ) override;

	void OnSelectedItemDeleted() { SelectItem( NULL ); }
	void OnChunkNameChanged();

	void FormatStateText();
private:
	bool m_bHelp;
	bool m_bIsDeleteMode;

	CReference<CUIViewport> m_pMainViewport;
	CReference<CUITreeView> m_pFileView;
	CReference<CUIScrollView> m_pNodeView;

	CReference<CUITextBox> m_pChunkName;
	CReference<CGameDropDownBox> m_pDetailEditType;

	CReference<CUILabel> m_pStateText;

	CReference<CUIElement> m_pLoadDialog;
	CReference<CUIElement> m_pSaveDialog;

	class CDesignViewFileElem* m_pSelectedFile;
	class CDesignViewNodeElem* m_pSelectedNode;
	SLevelDesignItem* m_pSelectedItem;

	TClassTrigger<CDesignView> m_onNew;
	TClassTrigger<CDesignView> m_onLoad;
	TClassTrigger<CDesignView> m_onSave;

	TClassTrigger<CDesignView> m_onChunkNameChanged;
	TClassTrigger<CDesignView> m_onSelectedItemDeleted;
};