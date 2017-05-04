#pragma once
#include "LevelGenerate.h"
#include "UICommon/UITreeView.h"
#include "UICommon/UIViewport.h"

class CChunkPreview : public CEntity
{
public:
	CChunkPreview() : m_pChunk( NULL ) { SET_BASEOBJECT_ID( CChunkPreview ); }
	CChunkPreview( const SClassCreateContext& context ) : CEntity( context ), m_pChunk( NULL ) { SET_BASEOBJECT_ID( CChunkPreview ); }
	virtual void OnRemovedFromStage() override { Clear(); }
	CLevelGenerateNode* GetNode() { return m_pNode; }
	const TRectangle<int32>& GetRegion() { return m_region; }
	virtual CEntity* GetPreviewRoot();
	virtual void Set( CLevelGenerateNode* pNode, const TRectangle<int32>& region );
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
	virtual void Set( CLevelGenerateNode* pNode, const TRectangle<int32>& region ) override;
	void SetTempEdit( bool bTempEdit );

	virtual CEntity* GetPreviewRoot() override;
	bool IsEditValid() { return m_bEditValid; }
private:
	void Check();

	CReference<CRenderObject2D> m_pFrameImg[8];

	bool m_bTempEdit;
	bool m_bEditValid;
};

struct SLevelDesignItem : public CReferenceObject
{
	~SLevelDesignItem()
	{
		if( pEntity )
			pEntity->SetParentEntity( NULL );
	}

	string strFullName;
	CReference<CLevelGenerateNode> pGenNode;
	CReference<CChunkEdit> pEntity;
	TRectangle<int32> region;
};

struct SLevelDesignContext
{
	SLevelDesignContext( uint32 nWidth, uint32 nHeight ) : nWidth( nWidth ), nHeight( nHeight )
	{
		items[0].resize( nWidth * nHeight );
		items[1].resize( nWidth * nHeight );
	}

	SLevelDesignItem* AddItem( CLevelGenerateNode* pNode, const TRectangle<int32>& region );
	void RemoveItem( SLevelDesignItem* pItem );

	uint32 nWidth, nHeight;
	vector<CReference<SLevelDesignItem> > items[2];
};

class CDesignLevel : public CEntity, protected SLevelDesignContext
{
	friend void RegisterGameClasses();
public:
	CDesignLevel( const SClassCreateContext& context ) : CEntity( context ), m_bInited( false ), m_nShowLevelType( 3 ), SLevelDesignContext( 32, 128 ), m_strChunkEditPrefab( context ) { SET_BASEOBJECT_ID( CDesignLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void Add( const char* szFullName, const TRectangle<int32>& region );
	void Remove( SLevelDesignItem* pItem ) { RemoveItem( pItem ); }
	void SetShowLevelType( uint8 nType );

	SLevelDesignItem* GetItemByGrid( const TVector2<int32> grid, bool bPick = false );
	SLevelDesignItem* GetItemByWorldPos( const CVector2& worldPos, bool bPick = false );

	bool BeginEdit( const char* szNode, const CVector2& worldPos );
	void UpdateEdit( const CVector2& worldPos );
	void EndEdit();
	void StopEdit();

	CLevelGenerateNode* FindNode( const char* szFullName );
	void GenerateLevel( class CMyLevel* pLevel );

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	CEntity* GetChunkRoot() { return m_pChunkRoot; }
	CEntity* GetChunkRoot1() { return m_pChunkRoot1; }

	static CDesignLevel* GetInst() { return s_pLevel; }
private:
	bool m_bInited;
	uint8 m_nShowLevelType;

	CString m_strChunkEditPrefab;
	CReference<CPrefab> m_pChunkEditPrefab;

	CReference<CEntity> m_pChunkRoot;
	CReference<CEntity> m_pChunkRoot1;
	CReference<CEntity> m_pChunkEditRoot;
	CReference<CEntity> m_pChunkEditRoot1;

	map<string, CReference<CLevelGenerateNode> > m_mapGenerateNodes;

	string m_strEditNodeName;
	CReference<CChunkEdit> m_pTempChunkEdit;
	CVector2 m_editBeginPos;

	static CDesignLevel* s_pLevel;
};

class CDesignView : public CUIElement
{
public:
	DECLARE_GLOBAL_INST_POINTER_WITH_REFERENCE( CDesignView )

	CDesignView() : m_pSelectedFile( NULL ), m_pSelectedNode( NULL ), m_bIsDeleteMode( false ) {}

	void SelectFile( class CDesignViewFileElem* pItem );
	void SelectNode( class CDesignViewNodeElem* pItem );
	void SetDeleteMode( bool bDelete ) { m_bIsDeleteMode = bDelete; }

	bool IsDeleteMode() { return m_bIsDeleteMode; }
	class CDesignViewNodeElem* GetSelectedNode() { return !m_bIsDeleteMode ? m_pSelectedNode : NULL; }

	void OnDesignVisible( bool bVisible );
protected:
	virtual void OnInited() override;

	virtual void OnChar( uint32 nChar ) override;
private:
	bool m_bIsDeleteMode;

	CReference<CUIViewport> m_pMainViewport;
	CReference<CUITreeView> m_pFileView;
	CReference<CUIScrollView> m_pNodeView;

	class CDesignViewFileElem* m_pSelectedFile;
	class CDesignViewNodeElem* m_pSelectedNode;
};